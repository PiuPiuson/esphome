#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"

#include <vector>

namespace esphome {
namespace daikin_hpc {

template<typename... Ts> class ResetEnergyAction;

class DAIKIN_HPC : public PollingComponent, public modbus::ModbusDevice {
 public:
  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

  /// Set use of Fahrenheit units
  void set_fahrenheit(bool value) { this->fahrenheit_ = value; }

 protected:
  bool fahrenheit_{false};
  DAIKIN_HPC *daikin_hpc_;
};

}  // namespace daikin_hpc
}  // namespace esphome
