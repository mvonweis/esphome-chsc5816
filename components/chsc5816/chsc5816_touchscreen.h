#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"


typedef struct __CHSC5816_Header {
    uint16_t fw_ver;
    uint16_t checksum;
    uint32_t sig;
    uint32_t vid_pid;
    uint16_t raw_offet;
    uint16_t dif_offet;
} CHSC5816_Header_t;

union __CHSC5816_PointReg { // 8 bytes
    struct {
        uint8_t status;
        uint8_t fingerNumber;
        uint8_t x_l8;
        uint8_t y_l8;
        uint8_t z;
        uint8_t x_h4: 4;
        uint8_t y_h4: 4;
        uint8_t id: 4;
        uint8_t event: 4;
        uint8_t p2;
    } rp;
    unsigned char data[8];
};

union _rpt_point_t { // 5 bytes
    struct {
      uint8_t x_l8;
      uint8_t y_l8;
      uint8_t z;
      uint8_t x_h4:4;
      uint8_t y_h4:4;
      uint8_t id:4;
      uint8_t event:4;
    };
    uint8_t data[5];
};


namespace esphome {
namespace chsc5816 {

using namespace touchscreen;
using namespace i2c;

static const char *const TAG = "chsc5816.touchscreen";

static const uint8_t CHSC5816_CHIP_ID = 0x2e;

/*
static const uint8_t CHSC5816_REG_STATUS = 0x00;
static const uint8_t CHSC5816_REG_STATUS_TOUCH = 0x00;
static const uint8_t CHSC5816_REG_STATUS_X_COR = 0x02;
static const uint8_t CHSC5816_REG_STATUS_Y_COR = 0x04;
static const uint8_t CHSC5816_REG_STATUS_LEN = 0x05;
*/

class CHSC5816Touchscreen : public Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }

 protected:
  void hard_reset_();
  void update_touches() override;

  bool semi_touch_read_bytes(uint32_t reg, uint8_t* pdata, uint16_t len);
  void semi_touch_write_bytes(uint32_t reg, uint8_t* pdata, uint16_t len);

  bool semi_touch_read_fwid();

  InternalGPIOPin *interrupt_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};

  uint8_t read_byte_(uint8_t addr);
};

}  // namespace chsc5816
}  // namespace esphome
