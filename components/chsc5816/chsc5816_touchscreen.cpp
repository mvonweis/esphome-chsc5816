#include "chsc5816_touchscreen.h"

namespace esphome {
namespace chsc5816 {


bool CHSC5816Touchscreen::semi_touch_read_fwid()
{
  uint8_t hdr0[4] = {
    0x20, 0x00, 0x00, 0x00
  };
  uint8_t hdr1[16] = {
    0xFF, 0x16, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0xe9
  };

  uint8_t pdata[16];

//  a_register = convert_big_endian(a_register);
  ErrorCode err = this->write(reinterpret_cast<const uint8_t *>(hdr0), 4, true);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "CHSC5816 semi_touch_read_fwid write 1 failed: %d", err);
    return false;
  }
  err = this->write(reinterpret_cast<const uint8_t *>(hdr1), 16, true);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "CHSC5816 semi_touch_read_fwid write 2 failed: %d", err);
    return false;
  }
//  delay(200);
  vTaskDelay(pdMS_TO_TICKS(200));
  err = this->write(reinterpret_cast<const uint8_t *>(hdr0), 4, true);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "CHSC5816 semi_touch_read_fwid write 3 failed: %d", err);
    return false;
  }

  err = bus_->read(address_, reinterpret_cast<uint8_t *>(&(pdata[0])), 16);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "CHSC5816 semi_touch_read_fwid read from %x failed: %d", address_, err);
    return false;
  }  
//  ESP_LOGCONFIG(TAG, "  Got fwid: %x %x %x %x", pdata[0], pdata[0], pdata[0], pdata[0]);

//  for (size_t i = 0; i < len; i++) pdata[i] = i2ctohs(pdata[i]);
  return true;
}

void CHSC5816Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CHSC5816 Touchscreen...");
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->attach_interrupt_(this->interrupt_pin_, gpio::INTERRUPT_FALLING_EDGE);
  }  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->hard_reset_();
  }

  // Get touch resolution
  if (this->x_raw_max_ == this->x_raw_min_) {
    this->x_raw_max_ = this->display_->get_native_width();
  }
  if (this->y_raw_max_ == this->y_raw_min_) {
    this->y_raw_max_ = this->display_->get_native_height();
  }

  ESP_LOGI(TAG, "CHSC5816 touch driver started");
  LOG_I2C_DEVICE(this);
  
  // (void)this->semi_touch_read_fwid();
  
/*    ESP_LOGE(TAG, "CHSC5816 touch driver failed to start"); */
}

void CHSC5816Touchscreen::hard_reset_() {
  if (this->reset_pin_ != nullptr) {
    ESP_LOGI(TAG, "CHSC5816 hard reset");
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
  }
}

bool CHSC5816Touchscreen::semi_touch_read_bytes(uint32_t a_register, uint8_t* pdata, uint16_t len)
{
  a_register = convert_big_endian(a_register);

  ErrorCode err = this->write(reinterpret_cast<const uint8_t *>(&a_register), 4, false);
  if (err != ERROR_OK)
    return false;
  err = this->read(pdata, len);
  if (err != ERROR_OK)
    return false;

//  ESP_LOGD(TAG, "Read: %x %x %x %x", pdata[0], pdata[1], pdata[2], pdata[7]);

  return true;
}

void CHSC5816Touchscreen::semi_touch_write_bytes(uint32_t reg, uint8_t* pdata, uint16_t len)
{

}


void CHSC5816Touchscreen::update_touches() {
  uint8_t data[8];
  int pointed = 0;
  int x, y;
  union _rpt_point_t *ppt;
  
  if (!this->semi_touch_read_bytes(0x2000002c, data, sizeof(data))) {
    ESP_LOGE(TAG, "CHSC5816 touch error: %x", data);
    return;
  }

  ppt = (union _rpt_point_t*)&data[2];

  if ((data[0] == 0xff) && (data[1] <= 2)) {
    if (data [1] > 0) {
      pointed = 1;
      x = (unsigned int) (ppt->x_h4 << 8) | ppt->x_l8;
      y = (unsigned int) (ppt->y_h4 << 8) | ppt->y_l8;
      ESP_LOGD(TAG, "CHSC5816 touch point at: %d %d", x, y);
      this->add_raw_touch_position_(0, x, y);
    }
  } else {
    ESP_LOGE(TAG, "CHSC5816 touch, could not parse data");
    return;
  }
  
}

void CHSC5816Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "CHSC5816 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  ESP_LOGCONFIG(TAG, "  Touch timeout: %d", this->touch_timeout_);
  ESP_LOGCONFIG(TAG, "  x_raw_max_: %d", this->x_raw_max_);
  ESP_LOGCONFIG(TAG, "  y_raw_max_: %d", this->y_raw_max_);
}

uint8_t CHSC5816Touchscreen::read_byte_(uint8_t addr) {
  uint8_t byte = 0;
  this->read_byte(addr, &byte);
  return byte;
}


}  // namespace chsc5816
}  // namespace esphome
