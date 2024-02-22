#include "daikin_hpc.h"
#include "esphome/core/log.h"

namespace esphome {
namespace daikin_hpc {

static const char *const TAG = "daikin_hpc";

static const uint8_t PZEM_CMD_READ_IN_REGISTERS = 0x04;
static const uint8_t PZEM_CMD_RESET_ENERGY = 0x42;
static const uint8_t PZEM_REGISTER_COUNT = 10;  // 10x 16-bit registers

void DAIKIN_HPC::on_modbus_data(const std::vector<uint8_t> &data) {}

void DAIKIN_HPC::update() {}

void DAIKIN_HPC::dump_config() {
  ESP_LOGCONFIG(TAG, "DAIKIN_HPC:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

}  // namespace daikin_hpc
}  // namespace esphome
