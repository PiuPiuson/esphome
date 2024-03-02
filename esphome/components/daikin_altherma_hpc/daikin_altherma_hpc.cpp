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

/****************** CLIMATE *****************/
int16_t DaikinAlthermaHPC::data_to_int16(const std::vector<uint8_t> &data) {
  if (data.size() != 2) {
    ESP_LOGW(TAG, "Tried to convert invalid data to unt16");
    return 0;
  }

  return ((static_cast<int16_t>(data[0])) << 8) | data[1];
}

float DaikinAlthermaHPC::data_to_temperature(const std::vector<uint8_t> &data) {
  return this->data_to_int16(data) * 0.1;
}

bool DaikinAlthermaHPC::data_to_bool(const std::vector<uint8_t> &data) { return this->data_to_int16(data) == 1; }

int16_t DaikinAlthermaHPC::temperature_to_int16(float temperature) { return temperature * 10; }

uint16_t DaikinAlthermaHPC::generate_config_data() {
  uint16_t data = 0;
  if (this->fan_mode.has_value()) {
    data |= static_cast<uint8_t>(this->climate_fan_mode_to_fan_mode(this->fan_mode.value()));
  }
  data |= static_cast<uint16_t>(this->lock_controls_) << 4;

  if (this->standby_) {
    data |= 0b10000000;
  }
  return data;
}

void DaikinAlthermaHPC::parse_config_data(uint16_t data) {
  this->standby_ = ((data >> 7) & 0b1);
  this->lock_controls_ = (data >> 4) & 0b1;
  this->fan_mode = fan_mode_to_climate_fan_mode(static_cast<FanMode>(data & 0b111));

  if (this->lock_controls_switch_ != nullptr) {
    this->lock_controls_switch_->publish_state(this->lock_controls_);
  }
}

DaikinAlthermaHPC::HeatCoolMode DaikinAlthermaHPC::climate_mode_to_heat_cool_mode(climate::ClimateMode mode) {
  switch (mode) {
    case climate::ClimateMode::CLIMATE_MODE_HEAT_COOL:
      return HeatCoolMode::Auto;
    case climate::ClimateMode::CLIMATE_MODE_HEAT:
      return HeatCoolMode::Heat;
    case climate::ClimateMode::CLIMATE_MODE_COOL:
      return HeatCoolMode::Cool;
    default:
      ESP_LOGW(TAG, "Tried to convert unsupported ClimateMode (%u) to HeatCoolMode", static_cast<int>(mode));
      return HeatCoolMode::Auto;
  }
}

climate::ClimateMode DaikinAlthermaHPC::heat_cool_mode_to_climate_mode(DaikinAlthermaHPC::HeatCoolMode mode) {
  switch (mode) {
    case HeatCoolMode::Auto:
      return climate::ClimateMode::CLIMATE_MODE_HEAT_COOL;
    case HeatCoolMode::Heat:
      return climate::ClimateMode::CLIMATE_MODE_HEAT;
    case HeatCoolMode::Cool:
      return climate::ClimateMode::CLIMATE_MODE_COOL;
    default:
      ESP_LOGW(TAG, "Tried to convert unsupported HeatCoolMode (%u) to ClimateMode", static_cast<int>(mode));
      return climate::ClimateMode::CLIMATE_MODE_OFF;
  }
}

DaikinAlthermaHPC::FanMode DaikinAlthermaHPC::climate_fan_mode_to_fan_mode(climate::ClimateFanMode mode) {
  switch (mode) {
    case climate::ClimateFanMode::CLIMATE_FAN_OFF:
    case climate::ClimateFanMode::CLIMATE_FAN_AUTO:
      return FanMode::Auto;
    case climate::ClimateFanMode::CLIMATE_FAN_LOW:
      return FanMode::Min;
    case climate::ClimateFanMode::CLIMATE_FAN_QUIET:
      return FanMode::Night;
    case climate::ClimateFanMode::CLIMATE_FAN_HIGH:
      return FanMode::Max;
    default:
      ESP_LOGW(TAG, "Tried to convert unsupported ClimateFanMode (%u) to FanMode", static_cast<int>(mode));
      return FanMode::Auto;
  }
}

climate::ClimateFanMode DaikinAlthermaHPC::fan_mode_to_climate_fan_mode(DaikinAlthermaHPC::FanMode mode) {
  switch (mode) {
    case FanMode::Auto:
      return climate::ClimateFanMode::CLIMATE_FAN_AUTO;
    case FanMode::Min:
      return climate::ClimateFanMode::CLIMATE_FAN_LOW;
    case FanMode::Night:
      return climate::ClimateFanMode::CLIMATE_FAN_QUIET;
    case FanMode::Max:
      return climate::ClimateFanMode::CLIMATE_FAN_HIGH;
    default:
      ESP_LOGW(TAG, "Tried to convert unsupported FanMode (%u) to ClimateFanMode", static_cast<int>(mode));
      return climate::ClimateFanMode::CLIMATE_FAN_AUTO;
  }
}

void DaikinAlthermaHPC::on_modbus_data(const std::vector<uint8_t> &data) {
  if (data.size() == 2) {
    this->process_read_queue(data);
    this->read_next_queue_item();
    return;
  }

  if (data.size() != 4) {
    return;
  }

  auto reg = static_cast<Register>(this->data_to_int16({data[0], data[1]}));
  auto val = this->data_to_int16({data[2], data[3]});

  this->modbus_write_queue_.pop();

  this->clear_modbus_read_queue();
  this->modbus_read_queue_.push(reg);

  this->process_read_queue({data[2], data[3]});
  this->write_next_queue_item();
}

void DaikinAlthermaHPC::process_read_queue(const std::vector<uint8_t> &data) {
  if (this->modbus_read_queue_.empty()) {
    return;
  }

  switch (this->modbus_read_queue_.front()) {
    case Register::WaterTemperature:
      if (this->water_temperature_sensor_ != nullptr) {
        this->water_temperature_sensor_->publish_state(this->data_to_temperature(data));
      }
      break;

    case Register::AirTemperature:
      this->current_temperature = this->data_to_temperature(data);
      break;

    case Register::AirTemperatureOffset:
      if (this->air_temperature_offset_numer_ != nullptr) {
        this->air_temperature_offset_numer_->publish_state(this->data_to_temperature(data));
      }
      break;

    case Register::FanSpeed:
      if (this->fan_speed_sensor_ != nullptr) {
        this->fan_speed_sensor_->publish_state(this->data_to_int16(data));
      }
      break;

    case Register::SetPoint:
      this->target_temperature = this->data_to_temperature(data);
      break;

    case Register::Config:
      this->parse_config_data(this->data_to_int16(data));
      break;

    case Register::HeatCoolSelect:
      if (this->standby_) {
        this->mode = climate::ClimateMode::CLIMATE_MODE_OFF;
      } else {
        this->mode = this->heat_cool_mode_to_climate_mode(static_cast<HeatCoolMode>(this->data_to_int16(data)));
      }
      this->publish_state();
      break;

    default:
      break;
  }

  this->modbus_read_queue_.pop();
}

void DaikinAlthermaHPC::update() {
  this->clear_modbus_read_queue();

  if (!this->modbus_write_queue_.empty()) {
    this->write_next_queue_item();
    return;
  }

  this->modbus_read_queue_.push(Register::AirTemperature);
  this->modbus_read_queue_.push(Register::WaterTemperature);
  this->modbus_read_queue_.push(Register::FanSpeed);
  this->modbus_read_queue_.push(Register::AirTemperatureOffset);
  this->modbus_read_queue_.push(Register::SetPoint);
  this->modbus_read_queue_.push(Register::Config);
  this->modbus_read_queue_.push(Register::HeatCoolSelect);

  this->read_next_queue_item();
}

void DaikinAlthermaHPC::read_next_queue_item() {
  if (this->modbus_read_queue_.empty()) {
    return;
  }

  this->send(MODBUS_CMD_READ_REGISTER, static_cast<uint16_t>(this->modbus_read_queue_.front()), 1);
}

void DaikinAlthermaHPC::write_next_queue_item() {
  if (this->modbus_write_queue_.empty()) {
    return;
  }

  auto [reg, value] = modbus_write_queue_.front();
  this->modbus_write_int16(reg, value);
}

void DaikinAlthermaHPC::modbus_write_bool(DaikinAlthermaHPC::Register reg, bool val) {
  uint8_t data[2] = {0};
  data[1] = val & 1;
  this->send(MODBUS_CMD_WRITE_REGISTER, static_cast<uint16_t>(reg), 1, 2, data);

  this->clear_modbus_read_queue();
}

void DaikinAlthermaHPC::modbus_write_int16(DaikinAlthermaHPC::Register reg, int16_t val) {
  // ESP_LOGW(TAG, "Writing register %u %u", static_cast<uint16_t>(reg), val);
  uint8_t data[2] = {0};
  data[0] = val >> 8;
  data[1] = val & 0xFF;
  this->send(MODBUS_CMD_WRITE_REGISTER, static_cast<uint16_t>(reg), 1, 2, data);

  this->clear_modbus_read_queue();
}

climate::ClimateTraits DaikinAlthermaHPC::traits() {
  climate::ClimateTraits traits;
  traits.set_supported_modes({climate::ClimateMode::CLIMATE_MODE_HEAT_COOL, climate::ClimateMode::CLIMATE_MODE_HEAT,
                              climate::ClimateMode::CLIMATE_MODE_COOL, climate::ClimateMode::CLIMATE_MODE_OFF});
  traits.set_supports_two_point_target_temperature(false);
  traits.set_supports_target_humidity(false);
  traits.set_supports_current_temperature(true);
  traits.set_supported_fan_modes({
      climate::ClimateFanMode::CLIMATE_FAN_AUTO,
      climate::ClimateFanMode::CLIMATE_FAN_HIGH,
      climate::ClimateFanMode::CLIMATE_FAN_LOW,
      climate::ClimateFanMode::CLIMATE_FAN_QUIET,
  });
  return traits;
}

void DaikinAlthermaHPC::setup() {
  this->set_visual_max_temperature_override(28);
  this->set_visual_min_temperature_override(16);
  this->set_visual_temperature_step_override(0.1, 0.5);
}

void DaikinAlthermaHPC::control(const climate::ClimateCall &call) {
  this->fan_mode = call.get_fan_mode();

  if (call.get_mode().has_value()) {
    this->mode = call.get_mode().value();
    this->standby_ = this->mode == climate::ClimateMode::CLIMATE_MODE_OFF;
  }

  if (call.get_target_temperature().has_value()) {
    this->target_temperature = call.get_target_temperature().value();
  }

  this->publish_state();
  this->write_state();
}

void DaikinAlthermaHPC::write_state() {
  this->clear_modbus_write_queue();
  this->clear_modbus_read_queue();

  if (this->mode != climate::ClimateMode::CLIMATE_MODE_OFF) {
    auto daikin_mode = this->climate_mode_to_heat_cool_mode(this->mode);
    this->modbus_write_queue_.push({Register::HeatCoolSelect, static_cast<uint16_t>(daikin_mode)});
  }

  auto config_data = this->generate_config_data();
  this->modbus_write_queue_.push({Register::Config, config_data});

  this->modbus_write_queue_.push({Register::SetPoint, this->temperature_to_int16(this->target_temperature)});

  this->write_next_queue_item();
}

void DaikinAlthermaHPC::toggle_switch(const std::string &id, bool state) {
  if (id == "lock_controls") {
    this->lock_controls_ = state;
    this->write_state();
  }
}

// make it so that writeInt and writeBool actually push into the queue.
// THe queue should be a map, we iterate through it and send all values

void DaikinAlthermaHPC::set_number(const std::string &id, float value) {
  if (id == "air_temperature_offset") {
    this->modbus_write_queue_.push({Register::AirTemperatureOffset, this->temperature_to_int16(value)});
  }
}

void DaikinAlthermaHPC::press_button(const std::string &id) {
  if (id == "reset_filter_hours") {
    // this->modbus_write_int16(Register::MultifunctionSettings, 1);
  }
}

void DaikinAlthermaHPC::set_select(const std::string &id, const std::string &option) {
  if (id == "filter_alarm_interval") {
    // auto index = this->filter_alarm_interval_select_->index_of(option);
    // if (index.has_value()) {
    //  this->modbus_write_int16(Register::FilterAlarmTimer, index.value());
    // }
  }
}

void DaikinAlthermaHPC::clear_modbus_read_queue() {
  while (!this->modbus_read_queue_.empty()) {
    this->modbus_read_queue_.pop();
  }
}

void DaikinAlthermaHPC::clear_modbus_write_queue() {
  while (!this->modbus_write_queue_.empty()) {
    this->modbus_write_queue_.pop();
  }
}

void DaikinAlthermaHPC::dump_config() {
  ESP_LOGCONFIG(TAG, "DaikinAlthermaHPC:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
  LOG_CLIMATE(TAG, " Climate", this);
}

}  // namespace daikin_altherma_hpc
}  // namespace esphome
