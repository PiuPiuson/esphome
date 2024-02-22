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

 protected:
  template<typename... Ts> friend class ResetEnergyAction;
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *energy_sensor_{nullptr};
  sensor::Sensor *frequency_sensor_{nullptr};
  sensor::Sensor *power_factor_sensor_{nullptr};
};

template<typename... Ts> class ResetEnergyAction : public Action<Ts...> {
 public:
  ResetEnergyAction(DAIKIN_HPC *daikin_hpc) : daikin_hpc_(daikin_hpc) {}

  void play(Ts... x) override { this->daikin_hpc_->reset_energy_(); }

 protected:
  DAIKIN_HPC *daikin_hpc_;
};

}  // namespace daikin_hpc
}  // namespace esphome
