#pragma once

#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/climate/climate.h"

#include <vector>
#include <queue>
#include <unordered_map>

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

class DaikinAlthermaHPC : public climate::Climate, public PollingComponent, public modbus::ModbusDevice {
 public:
  void setup() override;
  void update() override;
  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

  void set_water_temperature_sensor(sensor::Sensor *sensor) { this->water_temperature_sensor_ = sensor; }
  void set_fan_speed_sensor(sensor::Sensor *sensor) { this->fan_speed_sensor_ = sensor; }

  void set_lock_controls_switch(DaikinAlthermaHPCSwitch *sw) { this->lock_controls_switch_ = sw; }

  void set_air_temperature_offset_number(DaikinAlthermaHPCNumber *num) { this->air_temperature_offset_numer_ = num; }

  void toggle_switch(const std::string &id, bool state);
  void set_number(const std::string &id, float value);
  void press_button(const std::string &id);
  void set_select(const std::string &id, const std::string &option);

 protected:
  climate::ClimateTraits traits() override;

  void control(const climate::ClimateCall &call) override;

  sensor::Sensor *water_temperature_sensor_{nullptr};
  sensor::Sensor *fan_speed_sensor_{nullptr};

  DaikinAlthermaHPCSwitch *lock_controls_switch_{nullptr};

  DaikinAlthermaHPCNumber *air_temperature_offset_numer_{nullptr};

  bool standby_{};
  bool lock_controls_{};

  enum class Register : uint16_t {
    AirTemperature = 0,
    WaterTemperature = 1,
    FanSpeed = 9,
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
  std::unordered_map<Register, uint16_t> modbus_write_map_{};

  void modbus_write_bool(Register reg, bool val);
  void modbus_write_int16(Register reg, int16_t val);

  void clear_modbus_read_queue();

  float data_to_temperature(const std::vector<uint8_t> &data);
  int16_t data_to_int16(const std::vector<uint8_t> &data);
  bool data_to_bool(const std::vector<uint8_t> &data);

  uint16_t generate_config_data();
  void parse_config_data(uint16_t data);

  int16_t temperature_to_int16(float temperature);

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
