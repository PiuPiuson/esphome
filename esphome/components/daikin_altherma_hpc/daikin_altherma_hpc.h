#pragma once

#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/select/select.h"
#include "esphome/components/climate/climate.h"

#include <vector>
#include <queue>

namespace esphome {
namespace daikin_altherma_hpc {

class DaikinAlthermaHPC;

class DaikinAlthermaHPCSwitch : public switch_::Switch, public Component {
 public:
  void setup() override {}
  void dump_config() override;

  void set_parent(DaikinAlthermaHPC *parent) { this->parent_ = parent; }
  void set_id(const char *id) { this->id_ = id; }

 protected:
  void write_state(bool state) override;

  DaikinAlthermaHPC *parent_;
  std::string id_;
};

class DaikinAlthermaHPCNumber : public number::Number, public Component {
 public:
  void setup() override {}
  void dump_config() override;

  void set_parent(DaikinAlthermaHPC *parent) { this->parent_ = parent; }
  void set_id(const char *id) { this->id_ = id; }

 protected:
  void control(float value) override;

  DaikinAlthermaHPC *parent_;
  std::string id_;
};

class DaikinAlthermaHPCButton : public button::Button, public Component {
 public:
  void setup() override {}
  void dump_config() override;

  void set_parent(DaikinAlthermaHPC *parent) { this->parent_ = parent; }
  void set_id(const char *id) { this->id_ = id; }

 protected:
  void press_action() override;

  DaikinAlthermaHPC *parent_;
  std::string id_;
};

class DaikinAlthermaHPCSelect : public select::Select, public Component {
 public:
  void setup() override {}
  void dump_config() override;

  void set_parent(DaikinAlthermaHPC *parent) { this->parent_ = parent; }
  void set_id(const char *id) { this->id_ = id; }

 protected:
  void control(const std::string &value) override;

  DaikinAlthermaHPC *parent_;
  std::string id_;
};

class DaikinAlthermaHPC : public climate::Climate, public PollingComponent, public modbus::ModbusDevice {
 public:
  void setup() override;
  void update() override;
  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

  void set_water_temperature_sensor(sensor::Sensor *sensor) { water_temperature_sensor_ = sensor; }

  void toggle_switch(const std::string &id, bool state);
  void set_number(const std::string &id, float value);
  void press_button(const std::string &id);
  void set_select(const std::string &id, const std::string &option);

 protected:
  climate::ClimateTraits traits() override;

  void control(const climate::ClimateCall &call) override;

  sensor::Sensor *water_temperature_sensor_{nullptr};

  bool standby_{};
  bool lock_{};

  enum class Register : uint16_t {
    AirTemperature = 0,
    WaterTemperature = 1,
    MotorSpeed = 9,
    Config = 201,
    SetPoint = 231,
    HeatCoolSelect = 233,
    AirTemperatureOffset = 242,
  };

  enum class FanMode : uint8_t {
    Auto = 0,
    Min = 1,
    Night = 2,
    Max = 3,
  };

  enum class HeatCoolMode : uint8_t {
    Auto = 0,
    Heat = 3,
    Cool = 5,
  };

  std::queue<Register> modbus_read_queue_{};
  std::queue<std::pair<Register, uint16_t>> modbus_write_queue_{};

  void modbus_write_bool(Register reg, bool val);
  void modbus_write_uint16(Register reg, uint16_t val);

  void clear_modbus_read_queue();
  void clear_modbus_write_queue();

  float data_to_temperature(const std::vector<uint8_t> &data);
  uint16_t data_to_uint16(const std::vector<uint8_t> &data);
  bool data_to_bool(const std::vector<uint8_t> &data);

  uint16_t generate_config_data();
  void parse_config_data(uint16_t data);

  uint16_t temperature_to_uint16(float temperature);

  HeatCoolMode climate_mode_to_heat_cool_mode(climate::ClimateMode mode);
  climate::ClimateMode heat_cool_mode_to_climate_mode(HeatCoolMode mode);

  FanMode climate_fan_mode_to_fan_mode(climate::ClimateFanMode mode);
  climate::ClimateFanMode fan_mode_to_climate_fan_mode(FanMode mode);

  void process_read_queue(const std::vector<uint8_t> &data);
  void read_next_queue_item();

  void write_next_queue_item();

  void write_state();
};

}  // namespace daikin_altherma_hpc
}  // namespace esphome
