#include "gree.h"
#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace gree {

static const char *const TAG = "gree.climate";

void GreeClimate::set_model(Model model) { this->model_ = model; }

void GreeClimate::transmit_state() {
  uint8_t remote_state[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00};

  remote_state[0] = this->fan_speed_() | this->operation_mode_();
  remote_state[1] = this->temperature_();

  if (this->model_ == GREE_YAN) {
    remote_state[2] = 0x60;
    remote_state[3] = 0x50;
    remote_state[4] = this->vertical_swing_();
  }

  if (this->model_ == GREE_YAC) {
    remote_state[4] |= (this->horizontal_swing_() << 4);
  }

  if (this->model_ == GREE_YAA || this->model_ == GREE_YAC) {
    remote_state[2] = 0x20;  // bits 0..3 always 0000, bits 4..7 TURBO,LIGHT,HEALTH,X-FAN
    remote_state[3] = 0x50;  // bits 4..7 always 0101
    remote_state[6] = 0x20;  // YAA1FB, FAA1FB1, YB1F2 bits 4..7 always 0010

    if (this->vertical_swing_() == GREE_VDIR_SWING) {
      remote_state[0] |= (1 << 6);  // Enable swing by setting bit 6
    } else if (this->vertical_swing_() != GREE_VDIR_AUTO) {
      remote_state[5] = this->vertical_swing_();
    }
  }

  remote_state[7] = calculate_checksum_(remote_state);

  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(GREE_IR_FREQUENCY);

  data->mark(GREE_HEADER_MARK);
  data->space(GREE_HEADER_SPACE);

  for (int i = 0; i < 4; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {  // iterate through bit mask
      data->mark(GREE_BIT_MARK);
      bool bit = remote_state[i] & mask;
      data->space(bit ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  data->mark(GREE_BIT_MARK);
  data->space(GREE_ZERO_SPACE);
  data->mark(GREE_BIT_MARK);
  data->space(GREE_ONE_SPACE);
  data->mark(GREE_BIT_MARK);
  data->space(GREE_ZERO_SPACE);

  data->mark(GREE_BIT_MARK);
  data->space(GREE_MESSAGE_SPACE);

  for (int i = 4; i < 8; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {  // iterate through bit mask
      data->mark(GREE_BIT_MARK);
      bool bit = remote_state[i] & mask;
      data->space(bit ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  data->mark(GREE_BIT_MARK);
  data->space(0);

  transmit.perform();
}

uint8_t GreeClimate::calculate_checksum_(const uint8_t state[]) {
  if (this->model_ == GREE_YAN) {
    return ((state[0] << 4) + (state[1] << 4) + 0xC0);
  }

  return ((((state[0] & 0x0F) + (state[1] & 0x0F) + (state[2] & 0x0F) + (state[3] & 0x0F) + ((state[5] & 0xF0) >> 4) +
            ((state[6] & 0xF0) >> 4) + ((state[7] & 0xF0) >> 4) + 0x0A) &
           0x0F)
          << 4) |
         (state[7] & 0x0F);
}

uint8_t GreeClimate::operation_mode_() {
  uint8_t operating_mode = GREE_MODE_ON;

  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
      operating_mode |= GREE_MODE_COOL;
      break;
    case climate::CLIMATE_MODE_DRY:
      operating_mode |= GREE_MODE_DRY;
      break;
    case climate::CLIMATE_MODE_HEAT:
      operating_mode |= GREE_MODE_HEAT;
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
      operating_mode |= GREE_MODE_AUTO;
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
      operating_mode |= GREE_MODE_FAN;
      break;
    case climate::CLIMATE_MODE_OFF:
    default:
      operating_mode = GREE_MODE_OFF;
      break;
  }

  return operating_mode;
}

uint8_t GreeClimate::fan_speed_() {
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_LOW:
      return GREE_FAN_1;
    case climate::CLIMATE_FAN_MEDIUM:
      return GREE_FAN_2;
    case climate::CLIMATE_FAN_HIGH:
      return GREE_FAN_3;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return GREE_FAN_AUTO;
  }
}

uint8_t GreeClimate::horizontal_swing_() {
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_HORIZONTAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_HDIR_SWING;
    default:
      return GREE_HDIR_MANUAL;
  }
}

uint8_t GreeClimate::vertical_swing_() {
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_VDIR_SWING;
    default:
      return GREE_VDIR_MANUAL;
  }
}

uint8_t GreeClimate::temperature_() {
  return (uint8_t) roundf(clamp<float>(this->target_temperature, GREE_TEMP_MIN, GREE_TEMP_MAX));
}

// bool GreeClimate::parse_state_frame_(const uint8_t frame[]) {
//   // uint8_t checksum = 0;

//   // for (int i = 0; i < (GREE_STATE_FRAME_SIZE - 1); i++) {
//   //   checksum += frame[i];
//   // }

//   // if (frame[GREE_STATE_FRAME_SIZE - 1] != checksum) {
//   //   return false;
//   // }

//   // uint8_t mode = frame[5];
//   // if (mode & GREE_MODE_ON) {
//   //   switch (mode & 0xF0) {
//   //     case GREE_MODE_COOL:
//   //       this->mode = climate::CLIMATE_MODE_COOL;
//   //       break;
//   //     case GREE_MODE_DRY:
//   //       this->mode = climate::CLIMATE_MODE_DRY;
//   //       break;
//   //     case GREE_MODE_HEAT:
//   //       this->mode = climate::CLIMATE_MODE_HEAT;
//   //       break;
//   //     case GREE_MODE_AUTO:
//   //       this->mode = climate::CLIMATE_MODE_HEAT_COOL;
//   //       break;
//   //     case GREE_MODE_FAN:
//   //       this->mode = climate::CLIMATE_MODE_FAN_ONLY;
//   //       break;
//   //   }
//   // } else {
//   //   this->mode = climate::CLIMATE_MODE_OFF;
//   // }
//   // uint8_t temperature = frame[6];
//   // if (!(temperature & 0xC0)) {
//   //   this->target_temperature = temperature >> 1;
//   // }
//   // uint8_t fan_mode = frame[8];
//   // uint8_t swing_mode = frame[9];
//   // if (fan_mode & 0xF && swing_mode & 0xF) {
//   //   this->swing_mode = climate::CLIMATE_SWING_BOTH;
//   // } else if (fan_mode & 0xF) {
//   //   this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
//   // } else if (swing_mode & 0xF) {
//   //   this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
//   // } else {
//   //   this->swing_mode = climate::CLIMATE_SWING_OFF;
//   // }
//   // switch (fan_mode & 0xF0) {
//   //   case GREE_FAN_1:
//   //   case GREE_FAN_2:
//   //   case GREE_FAN_SILENT:
//   //     this->fan_mode = climate::CLIMATE_FAN_LOW;
//   //     break;
//   //   case GREE_FAN_3:
//   //     this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
//   //     break;
//   //   case GREE_FAN_4:
//   //   case GREE_FAN_5:
//   //     this->fan_mode = climate::CLIMATE_FAN_HIGH;
//   //     break;
//   //   case GREE_FAN_AUTO:
//   //     this->fan_mode = climate::CLIMATE_FAN_AUTO;
//   //     break;
//   // }
//   // this->publish_state();
//   return false;
// }

bool GreeClimate::on_receive(remote_base::RemoteReceiveData data) {
  if (!data.expect_item(GREE_HEADER_MARK, GREE_HEADER_SPACE)) {
    return false;
  }

  ESP_LOGI(TAG, "Header OK");

  uint8_t state_frame[GREE_STATE_FRAME_SIZE]{};

  for (int pos = 0; pos < 4; pos++) {
    for (int8_t bit = 0; bit < 8; bit++) {
      if (data.expect_item(GREE_BIT_MARK, GREE_ONE_SPACE)) {
        state_frame[pos] |= 1 << bit;
        continue;
      }

      if (!data.expect_item(GREE_BIT_MARK, GREE_ZERO_SPACE)) {
        ESP_LOGI(TAG, "Byte %d bit %d fail", pos, bit);
        return false;
      }
    }
  }

  if (!data.expect_item(GREE_BIT_MARK, GREE_ZERO_SPACE)) {
    ESP_LOGI(TAG, "Invalid data received 0");
    return false;
  }
  if (!data.expect_item(GREE_BIT_MARK, GREE_ONE_SPACE)) {
    ESP_LOGI(TAG, "Invalid data received 1");
    return false;
  }
  if (!data.expect_item(GREE_BIT_MARK, GREE_ZERO_SPACE)) {
    ESP_LOGI(TAG, "Invalid data received 2");
    return false;
  }
  if (!data.expect_item(GREE_BIT_MARK, GREE_MESSAGE_SPACE)) {
    ESP_LOGI(TAG, "Invalid data received 3");
    return false;
  }

  ESP_LOGI(TAG, "Mid data OK");

  ESP_LOGI(TAG, "Received: %02X %02X %02X %02X %02X %02X %02X %02X", state_frame[0], state_frame[1], state_frame[2],
           state_frame[3], state_frame[4], state_frame[5], state_frame[6], state_frame[7]);

  uint8_t fan_speed = state_frame[0] & 0xF0;
  switch (fan_speed) {
    case GREE_FAN_1:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case GREE_FAN_2:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    case GREE_FAN_3:
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
      break;
    case GREE_FAN_AUTO:
    default:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
  }

  uint8_t operation_mode = state_frame[0] & 0x0F;
  if ((operation_mode & GREE_MODE_ON) == GREE_MODE_ON) {
    switch (operation_mode) {
      case GREE_MODE_COOL:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case GREE_MODE_DRY:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case GREE_MODE_HEAT:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case GREE_MODE_AUTO:
        this->mode = climate::CLIMATE_MODE_AUTO;
        break;
      case GREE_MODE_FAN:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      default:
        this->mode = climate::CLIMATE_MODE_OFF;
    }
  } else {
    ESP_LOGI("TAG", "OpMode %02X %02X", operation_mode, operation_mode & GREE_MODE_ON);
    this->mode = climate::CLIMATE_MODE_OFF;
  }

  uint8_t temp = state_frame[1];
  if (temp < (uint8_t) this->minimum_temperature_) {
    temp += 16;
  }
  this->target_temperature = temp;

  this->publish_state();
  return true;

  for (int pos = 4; pos < 8; pos++) {
    for (int8_t bit = 0; bit < 8; bit++) {
      if (data.expect_item(GREE_BIT_MARK, GREE_ONE_SPACE)) {
        state_frame[pos] |= 1 << bit;
        continue;
      }

      if (!data.expect_item(GREE_BIT_MARK, GREE_ZERO_SPACE)) {
        ESP_LOGI(TAG, "Byte %d bit %d fail", pos, bit);
        return false;
      }
    }
  }

  uint8_t checksum = calculate_checksum_(state_frame);

  ESP_LOGI(TAG, "Checksum: %02X", checksum);

  return false;
}

}  // namespace gree
}  // namespace esphome
