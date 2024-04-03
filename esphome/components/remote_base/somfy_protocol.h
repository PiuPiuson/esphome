#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

enum class SomfyCommand : uint8_t {
  My = 0x1,
  Up = 0x2,
  MyUp = 0x3,
  Down = 0x4,
  MyDown = 0x5,
  UpDown = 0x6,
  Prog = 0x8,
  SunFlag = 0x9,
  Flag = 0xA
};

struct SomfyData {
  SomfyCommand command;
  uint32_t address;

  bool operator==(const SomfyData &rhs) const { return command == rhs.command; }
};

class SomfyProtocol : public RemoteProtocol<SomfyData> {
 public:
  void encode(RemoteTransmitData *dst, const SomfyData &data) override;
  optional<SomfyData> decode(RemoteReceiveData src) override;
  void dump(const SomfyData &data) override;

 protected:
  uint16_t _rolling_code = 0;

  void _build_frame(uint8_t *frame, SomfyData command);
  void _send_frame(RemoteTransmitData *dst, uint8_t *frame, uint8_t sync);
};

DECLARE_REMOTE_PROTOCOL(Somfy)

template<typename... Ts> class SomfyAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint32_t, address)
  TEMPLATABLE_VALUE(uint8_t, command)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    SomfyData data{};
    data.address = this->address_.value(x...);
    data.command = static_cast<SomfyCommand>(this->command_.value(x...));
    SomfyProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
