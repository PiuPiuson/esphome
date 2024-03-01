#include "daikin_altherma_hpc.h"

#include "esphome/core/log.h"

namespace esphome {
namespace daikin_altherma_hpc {

static const char *const TAG = "daikin_altherma_hpc";

static constexpr uint8_t MODBUS_CMD_READ_REGISTER = 3;
static constexpr uint8_t MODBUS_CMD_WRITE_REGISTER = 6;

/****************** SWITCH *****************/
void DaikinAlthermaHPCSwitch::dump_config() { LOG_SWITCH(TAG, " Switch", this); }

void DaikinAlthermaHPCSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->toggle_switch(this->id_, state);
}

/****************** NUMBER *****************/
void DaikinAlthermaHPCNumber::dump_config() { LOG_NUMBER(TAG, " Number", this); }

void DaikinAlthermaHPCNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_number(this->id_, value);
}

/****************** BUTTON *****************/
void DaikinAlthermaHPCButton::dump_config() { LOG_BUTTON(TAG, " Button", this); }

void DaikinAlthermaHPCButton::press_action() { this->parent_->press_button(this->id_); }

/****************** SELECT ****************/
void DaikinAlthermaHPCSelect::dump_config() { LOG_SELECT(TAG, " Select", this); }

void DaikinAlthermaHPCSelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_select(this->id_, value);
}

/****************** HRU *****************/
uint16_t DaikinAlthermaHPC::data_to_uint16(const std::vector<uint8_t> &data) {
  if (data.size() != 2) {
    ESP_LOGW(TAG, "Tried to convert invalid data to unt16");
    return 0;
  }

  return ((static_cast<int16_t>(data[0])) << 8) | data[1];
}

float DaikinAlthermaHPC::data_to_temperature(const std::vector<uint8_t> &data) {
  return this->data_to_uint16(data) * 0.1;
}

bool DaikinAlthermaHPC::data_to_bool(const std::vector<uint8_t> &data) { return this->data_to_uint16(data) == 1; }

uint16_t DaikinAlthermaHPC::temperature_to_uint16(float temperature) { return temperature * 10; }

void DaikinAlthermaHPC::process_flag_data(const std::vector<uint8_t> &data) {
  auto dataInt = this->data_to_uint16(data);

  // if (this->fire_alarm_sensor_ != nullptr) {
  //   this->fire_alarm_sensor_->publish_state(dataInt & 1U);
  // }

  // if (this->bypass_open_sensor_ != nullptr) {
  //   this->bypass_open_sensor_->publish_state((dataInt >> 1) & 1U);
  // }

  // if (this->defrosting_sensor_ != nullptr) {
  //   this->defrosting_sensor_->publish_state((dataInt >> 3) & 1U);
  // }
}

void DaikinAlthermaHPC::on_modbus_data(const std::vector<uint8_t> &data) {
  if (this->modbus_send_queue.empty()) {
    if (data.size() == 4) {
      auto reg = static_cast<Register>(this->data_to_uint16({data[0], data[1]}));
      auto val = this->data_to_uint16({data[2], data[3]});
      ESP_LOGD(TAG, "Received confirmation register %d value %u (0x%02X%02X)", static_cast<int>(reg), val, data[2],
               data[3]);

      this->modbus_send_queue.push(reg);
      this->process_register_queue({data[2], data[3]});
    }

  } else {
    this->process_register_queue(data);
  }
}

void DaikinAlthermaHPC::process_register_queue(const std::vector<uint8_t> &data) {
  switch (this->modbus_send_queue.front()) {
    case Register::WaterTemperature:
      if (this->water_temperature_sensor_ != nullptr) {
        this->water_temperature_sensor_->publish_state(this->data_to_temperature(data));
      }
      break;

    case Register::AirTemperature:
      this->current_temperature = this->data_to_temperature(data);
      this->publish_state();

    case Register::SetPoint:
      this->target_temperature = this->data_to_temperature(data);
      this->publish_state();
    default:
      break;
  }

  this->modbus_send_queue.pop();
  this->read_next_register();
}

void DaikinAlthermaHPC::update() {
  this->clear_modbus_send_queue();

  this->modbus_send_queue.push(Register::AirTemperature);
  this->modbus_send_queue.push(Register::WaterTemperature);
  this->modbus_send_queue.push(Register::SetPoint);

  this->read_next_register();
}

void DaikinAlthermaHPC::read_next_register() {
  if (this->modbus_send_queue.empty()) {
    return;
  }

  this->send(MODBUS_CMD_READ_REGISTER, static_cast<uint16_t>(this->modbus_send_queue.front()), 1);
}

void DaikinAlthermaHPC::modbus_write_bool(DaikinAlthermaHPC::Register reg, bool val) {
  uint8_t data[2] = {0};
  data[1] = val & 1;
  this->send(MODBUS_CMD_WRITE_REGISTER, static_cast<uint16_t>(reg), 1, 2, data);

  this->clear_modbus_send_queue();
}

void DaikinAlthermaHPC::modbus_write_uint16(DaikinAlthermaHPC::Register reg, uint16_t val) {
  ESP_LOGW(TAG, "Writing register %u %u", static_cast<uint16_t>(reg), val);
  uint8_t data[2] = {0};
  data[0] = val >> 8;
  data[1] = val & 0xFF;
  this->send(MODBUS_CMD_WRITE_REGISTER, static_cast<uint16_t>(reg), 1, 2, data);

  this->clear_modbus_send_queue();
}

void DaikinAlthermaHPC::control(const climate::ClimateCall &call) {}

void DaikinAlthermaHPC::toggle_switch(const std::string &id, bool state) {
  if (id == "heater_installed") {
    // this->modbus_write_bool(Register::HeaterValidOrInvalid, state);
  }
}

void DaikinAlthermaHPC::set_number(const std::string &id, float value) {
  if (id == "bypass_open_temperature") {
    // this->modbus_write_uint16(Register::BypassOpenTemperature, value);
  }
}

void DaikinAlthermaHPC::press_button(const std::string &id) {
  if (id == "reset_filter_hours") {
    // this->modbus_write_uint16(Register::MultifunctionSettings, 1);
  }
}

void DaikinAlthermaHPC::set_select(const std::string &id, const std::string &option) {
  if (id == "filter_alarm_interval") {
    // auto index = this->filter_alarm_interval_select_->index_of(option);
    // if (index.has_value()) {
    //  this->modbus_write_uint16(Register::FilterAlarmTimer, index.value());
    // }
  }
}

void DaikinAlthermaHPC::clear_modbus_send_queue() {
  while (!this->modbus_send_queue.empty()) {
    this->modbus_send_queue.pop();
  }
}

void DaikinAlthermaHPC::dump_config() {
  ESP_LOGCONFIG(TAG, "DaikinAlthermaHPC:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
  LOG_CLIMATE(TAG, " Climate", this);
}

}  // namespace daikin_altherma_hpc
}  // namespace esphome
