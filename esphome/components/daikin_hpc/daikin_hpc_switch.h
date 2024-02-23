#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace daikin_hpc {

class DaikinHpcSwitch : public switch_::Switch, public Component {
 public:
  void setup() override { this->publish_state(false); }

 protected:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace daikin_hpc
}  // namespace esphome
