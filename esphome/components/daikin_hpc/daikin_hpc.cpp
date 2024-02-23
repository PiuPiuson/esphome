#include "daikin_hpc.h"

#include "esphome/core/log.h"

namespace esphome {
namespace daikin_hpc {

static const char *const TAG = "daikin_hpc";

static constexpr uint8_t MODBUS_CMD_READ_REGISTER = 3;
static constexpr uint8_t MODBUS_CMD_WRITE_REGISTER = 6;

uint16_t DaikinHpcClimate::dataToUint16(const std::vector<uint8_t> &data) {
  if (data.size() != 2) {
    ESP_LOGW(TAG, "Tried to convert invalid data to unt16");
    return 0;
  }

  return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

void DaikinHpcClimate::set_water_temperature_sensor(sensor::Sensor *waterTemperatureSensor) {
  ESP_LOGW(TAG, "set water temp");
  waterTemperatureSensor_ = waterTemperatureSensor;
}

float DaikinHpcClimate::dataToTemperature(const std::vector<uint8_t> &data) { return dataToUint16(data) * 0.1; }

void DaikinHpcClimate::parseConfigData(const std::vector<uint8_t> &data) {
  const auto raw = dataToUint16(data);

  const bool onOff = (raw >> 7) & 0b1;
  const bool lock = (raw >> 4) & 0b1;
  const uint8_t fanMode = raw & 0b111;

  if (onOff_ != nullptr) {
    onOff_->publish_state(onOff);
  }
}

void DaikinHpcClimate::setup() {
  // waterTemperatureSensor_->set_icon("mdi:thermometer-water");
  // waterTemperatureSensor_->set_unit_of_measurement("°C");
  // waterTemperatureSensor_->set_accuracy_decimals(1);
  // waterTemperatureSensor_->set_name("Water Temperature");
  // waterTemperatureSensor_->set_entity_category(EntityCategory::ENTITY_CATEGORY_DIAGNOSTIC);
  // waterTemperatureSensor_->set_internal(false);

  // airTemperatureSensor_->set_icon("mdi:thermometer");
  // airTemperatureSensor_->set_unit_of_measurement("°C");
  // airTemperatureSensor_->set_accuracy_decimals(1);
  // airTemperatureSensor_->set_name("Air Temperature");
  // airTemperatureSensor_->set_entity_category(EntityCategory::ENTITY_CATEGORY_NONE);
  // airTemperatureSensor_->set_internal(false);

  // motorSpeedSensor_->set_icon("mdi:fan");
  // motorSpeedSensor_->set_unit_of_measurement("rpm");
  // motorSpeedSensor_->set_accuracy_decimals(0);
  // motorSpeedSensor_->set_name("Fan Speed");
  // motorSpeedSensor_->set_entity_category(EntityCategory::ENTITY_CATEGORY_DIAGNOSTIC);

  // controlLock_->set_icon("mdi:lock");
  // controlLock_->set_name("Control Lock");

  onOff_->set_icon("mdi:power");
  onOff_->set_name("On / Off");
  onOff_->set_inverted(true);
}

void DaikinHpcClimate::on_modbus_data(const std::vector<uint8_t> &data) {
  switch (modbusSendQueue.front()) {
    case Register::WaterTemperature:
      if (waterTemperatureSensor_ != nullptr) {
        waterTemperatureSensor_->publish_state(dataToTemperature(data));
      }
      break;

    case Register::AirTemperature:
      if (airTemperatureSensor_ != nullptr) {
        airTemperatureSensor_->publish_state(dataToTemperature(data));
      }
      break;

    case Register::MotorSpeed:
      if (motorSpeedSensor_ != nullptr) {
        motorSpeedSensor_->publish_state(dataToUint16(data));
      }
      break;

    case Register::Config:
      parseConfigData(data);
      break;

    case Register::AbsoluteSetPoint:
      if (absoluteSetPointSensor_ != nullptr) {
        absoluteSetPointSensor_->publish_state(dataToTemperature(data));
      }
      break;
  }

  modbusSendQueue.pop();
  readNextRegister();
}

void DaikinHpcClimate::update() {
  while (!modbusSendQueue.empty()) {
    modbusSendQueue.pop();
  }

  modbusSendQueue.push(Register::WaterTemperature);
  modbusSendQueue.push(Register::AirTemperature);
  modbusSendQueue.push(Register::MotorSpeed);
  modbusSendQueue.push(Register::Config);
  modbusSendQueue.push(Register::AbsoluteSetPoint);

  readNextRegister();
}

void DaikinHpcClimate::readNextRegister() {
  if (modbusSendQueue.empty()) {
    return;
  }

  this->send(MODBUS_CMD_READ_REGISTER, static_cast<uint16_t>(modbusSendQueue.front()), 1);
}

void DaikinHpcClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "DaikinHpcClimate:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

}  // namespace daikin_hpc
}  // namespace esphome
