#include "somfy_protocol.h"
#include "esphome/core/log.h"

#define SYMBOL 640

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.somfy";

void SomfyProtocol::_build_frame(uint8_t *frame, SomfyData data) {
  const uint8_t button = static_cast<uint8_t>(data.command);

  this->_rolling_code += 1;

  ESP_LOGD(TAG, "Creating frame for address %08X command %01X and rolling code %d", data.address, data.command,
           this->_rolling_code);

  frame[0] = 0xA7;                      // Encryption key. Doesn't matter much
  frame[1] = button << 4;               // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = this->_rolling_code >> 8;  // Rolling code (big endian)
  frame[3] = this->_rolling_code;       // Rolling code
  frame[4] = data.address >> 16;        // Remote address
  frame[5] = data.address >> 8;         // Remote address
  frame[6] = data.address;              // Remote address

  // Checksum calculation: a XOR of all the nibbles
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111;  // We keep the last 4 bits only

  // Checksum integration
  frame[1] |= checksum;

  // Obfuscation: a XOR of all the bytes
  for (uint8_t i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }
}

void SomfyProtocol::_send_frame(RemoteTransmitData *dst, uint8_t *frame, uint8_t sync) {
  if (sync == 2) {  // Only with the first frame.
    // Wake-up pulse & Silence
    dst->item(9415, 9656);
    delay(80);
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    dst->item(4 * SYMBOL, 4 * SYMBOL);
  }

  // Software sync
  dst->item(4550, SYMBOL);

  // Data: bits are sent one by one, starting with the MSB.
  for (uint8_t i = 0; i < 56; i++) {
    if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1) {
      dst->space(SYMBOL);
      dst->mark(SYMBOL);
    } else {
      dst->mark(SYMBOL);
      dst->space(SYMBOL);
    }
  }

  // Inter-frame silence
  dst->space(415);
  delay(30);
}

void SomfyProtocol::encode(RemoteTransmitData *dst, const SomfyData &data) {
  uint8_t frame[7] = {0};
  this->_build_frame(frame, data);

  dst->set_carrier_frequency(433420);
  dst->reserve(7);

  this->_send_frame(dst, frame, 2);

  for (int i = 0; i < 4; i++) {
    this->_send_frame(dst, frame, 7);
  }
}

optional<SomfyData> SomfyProtocol::decode(RemoteReceiveData src) {
  //   SomfyData out{
  //       .data = 0,
  //       .nbits = 0,
  //   };
  //   if (!src.expect_item(HEADER_HIGH_US, HEADER_LOW_US))
  //     return {};

  //   for (; out.nbits < 20; out.nbits++) {
  //     uint32_t bit;
  //     if (src.expect_mark(BIT_ONE_HIGH_US)) {
  //       bit = 1;
  //     } else if (src.expect_mark(BIT_ZERO_HIGH_US)) {
  //       bit = 0;
  //     } else if (out.nbits == 12 || out.nbits == 15) {
  //       return out;
  //     } else {
  //       return {};
  //     }

  //     out.data = (out.data << 1UL) | bit;
  //     if (src.expect_space(BIT_LOW_US)) {
  //       // nothing needs to be done
  //     } else if (src.peek_space_at_least(BIT_LOW_US)) {
  //       out.nbits += 1;
  //       if (out.nbits == 12 || out.nbits == 15 || out.nbits == 20)
  //         return out;
  //       return {};
  //     } else {
  //       return {};
  //     }
  //   }

  return nullopt;
}
void SomfyProtocol::dump(const SomfyData &data) {
  ESP_LOGI(TAG, "Received Somfy: address=0x%08" PRIX32 ", command=%d", data.address, data.command);
}

}  // namespace remote_base
}  // namespace esphome
