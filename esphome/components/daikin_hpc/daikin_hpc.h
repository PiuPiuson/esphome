#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

#include <vector>
#include <queue>

namespace esphome {
namespace daikin_hpc {

class DaikinHpcClimate : public PollingComponent, public modbus::ModbusDevice {
 public:
  void setup() override;
  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

 protected:
  DaikinHpcClimate *daikin_hpc_;

  sensor::Sensor *waterTemperature_ = new sensor::Sensor();
  sensor::Sensor *airTemperature_ = new sensor::Sensor();
  sensor::Sensor *motorSpeed_ = new sensor::Sensor();

  switch_::Switch *controlLock_ = new switch_::Switch();
  switch_::Switch *onOff_ = new switch_::Switch();

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

  struct __attribute__((packed, aligned(1))) ConfigRegister {
    FanMode fanMode : 3;
    uint8_t : 1;
    bool lock : 1;
    uint8_t : 2;
    bool onOff : 1;
  };

  std::queue<Register> modbusSendQueue{};

  void readNextRegister();

  float dataToTemperature(const std::vector<uint8_t> &data);
  uint16_t dataToUint16(const std::vector<uint8_t> &data);
};

}  // namespace daikin_hpc
}  // namespace esphome
