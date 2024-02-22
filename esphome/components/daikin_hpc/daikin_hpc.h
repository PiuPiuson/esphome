#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"

#include <vector>

namespace esphome {
namespace daikin_hpc {

class DAIKIN_HPC : public PollingComponent, public modbus::ModbusDevice {
 public:
  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

 protected:
  DAIKIN_HPC *daikin_hpc_;

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

  struct ConfigRegister __attribute__((packed, aligned(1))) {
    FanMode fanMode : 3;
    void : 1;
    bool lock : 1;
    void : 2;
    bool onOff : 1;
  };
};

}  // namespace daikin_hpc
}  // namespace esphome
