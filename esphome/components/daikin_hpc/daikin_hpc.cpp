#include "daikin_hpc.h"

#include "esphome/core/log.h"

namespace esphome {
namespace daikin_hpc {

static const char *const TAG = "daikin_hpc";

static constexpr uint8_t MODBUS_CMD_READ_REGISTER = 3;
static constexpr uint8_t MODBUS_CMD_WRITE_REGISTER = 6;

void DaikinHpcClimate::setup() {
  waterTemperature_->set_icon("mdi:thermometer");
  waterTemperature_->set_unit_of_measurement("°C");
  waterTemperature_->set_accuracy_decimals(1);
  waterTemperature_->set_name("Water Temperature");
  waterTemperature_->set_entity_category(EntityCategory::ENTITY_CATEGORY_NONE);
  waterTemperature_->set_internal(false);
}

void DaikinHpcClimate::on_modbus_data(const std::vector<uint8_t> &data) {
  if (data.size() != 2) {
    ESP_LOGW(TAG, "Received data length != 2");
    return;
  }

  int16_t temp = (static_cast<int16_t>(data[0]) << 8) | data[1];
  waterTemperature_->publish_state(temp * 0.1);
}

void DaikinHpcClimate::update() {
  this->send(MODBUS_CMD_READ_REGISTER, static_cast<uint16_t>(Register::WaterTemperature), 1);
}

void DaikinHpcClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "DaikinHpcClimate:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

}  // namespace daikin_hpc
}  // namespace esphome
