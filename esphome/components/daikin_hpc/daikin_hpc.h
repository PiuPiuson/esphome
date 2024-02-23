#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

#include "daikin_hpc_switch.h"

#include <vector>
#include <queue>

namespace esphome {
namespace daikin_hpc {

class DaikinHpcClimate : public PollingComponent, public modbus::ModbusDevice, public sensor::Sensor {
 public:
  void setup() override;
  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

  void set_water_temperature_sensor(sensor::Sensor *waterTemperatureSensor) {
    ESP_LOGW(TAG, "set water temp");
    waterTemperatureSensor_ = waterTemperatureSensor;
  }
  void set_air_temperature_sensor(sensor::Sensor *airTemperatureSensor) {
    airTemperatureSensor_ = airTemperatureSensor;
  }
  void set_motor_speed_sensor(sensor::Sensor *motorSpeedSensor) { motorSpeedSensor_ = motorSpeedSensor; }
  void set_absolute_set_point_sensor(sensor::Sensor *absoluteSetPointSensor) {
    absoluteSetPointSensor_ = absoluteSetPointSensor;
  }

 protected:
  DaikinHpcClimate *daikin_hpc_;

  sensor::Sensor *waterTemperatureSensor_{nullptr};
  sensor::Sensor *airTemperatureSensor_{nullptr};
  sensor::Sensor *motorSpeedSensor_{nullptr};

  // switch_::Switch *controlLock_ = new switch_::Switch();
  DaikinHpcSwitch *onOff_ = new DaikinHpcSwitch();

  sensor::Sensor *absoluteSetPointSensor_{nullptr};

 private:
  enum class Register : uint8_t {
    AirTemperature = 0,
    WaterTemperature = 1,
    MotorSpeed = 9,
    Config = 201,
    AbsoluteSetPoint = 231,
  };

  enum class FanMode : uint8_t {
    Auto = 0,
    Min = 1,
    Night = 2,
    Max = 3,
  };

  struct __attribute__((packed, aligned(2))) ConfigRegister {
    FanMode fanMode : 3;
    uint8_t : 1;
    bool lock : 1;
    uint8_t : 2;
    bool onOff : 1;
    uint8_t : 8;
  };

  std::queue<Register> modbusSendQueue{};

  void readNextRegister();

  float dataToTemperature(const std::vector<uint8_t> &data);
  uint16_t dataToUint16(const std::vector<uint8_t> &data);

  void parseConfigData(const std::vector<uint8_t> &data);
};

}  // namespace daikin_hpc
}  // namespace esphome
