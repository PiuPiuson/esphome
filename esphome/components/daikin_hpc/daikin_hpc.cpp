#include "daikin_hpc.h"

#include "esphome/core/log.h"

namespace esphome {
namespace daikin_hpc {

static const char *const TAG = "daikin_hpc";

void DaikinHpcClimate::setup() {
  waterTemperature_->set_icon("mdi:thermometer");
  waterTemperature_->set_unit_of_measurement("°C");
  waterTemperature_->set_accuracy_decimals(1);
  waterTemperature_->set_name("Water Temperature");
  waterTemperature_->set_entity_category(EntityCategory::ENTITY_CATEGORY_DIAGNOSTIC);
}

void DaikinHpcClimate::on_modbus_data(const std::vector<uint8_t> &data) {}

void DaikinHpcClimate::update() { waterTemperature_->publish_state(2); }

void DaikinHpcClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "DaikinHpcClimate:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

}  // namespace daikin_hpc
}  // namespace esphome
