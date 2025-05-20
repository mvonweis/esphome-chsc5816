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
  ESP_LOGCONFIG(TAG, "  Got fwid: %x %x %x %x", pdata[0], pdata[0], pdata[0], pdata[0]);

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


// ErrorCode readv(uint8_t address, ReadBuffer *buffers, size_t cnt);
// ErrorCode writev(uint8_t address, WriteBuffer *buffers, size_t cnt, bool stop);


bool CHSC5816Touchscreen::semi_touch_read_bytes(uint32_t a_register, uint8_t* pdata, uint16_t len)
{
  a_register = convert_big_endian(a_register);

  ErrorCode err = this->write(reinterpret_cast<const uint8_t *>(&a_register), 4, false);
  if (err != ERROR_OK)
    return false;
  err = this->read(pdata, len);
  if (err != ERROR_OK)
    return false;

  // for (size_t i = 0; i < len; i++) pdata[i] = i2ctohs(pdata[i]);
  ESP_LOGD(TAG, "Read: %x %x %x %x", pdata[0], pdata[1], pdata[2], pdata[7]);

  return true;
}




void CHSC5816Touchscreen::semi_touch_write_bytes(uint32_t reg, uint8_t* pdata, uint16_t len)
{

/*  a_reg = convert_big_endian(reg);
  WriteBuffer bufs[4];
  buf[0].data = 

  this->write_register(reg)



  uint8_t regs[4];
  memcpy(regs, reg, 4);
  this->write_byte(regs[0]);
  this->write_byte(regs[1]);
  this->write_byte(regs[2]);
  this->write_byte(regs[3]);

  for (i=0; i < len; i++) {
    this->write_byte(pdata[0]);
  }
  */
}

/*
    bool checkOnline()
    {
        Header_t first;
        Header_t second;

        memset(&second, 0, sizeof(Header_t));
        memset(&first, 0, sizeof(Header_t));

        // CHSC5816_REG_BOOT_STATE 0x20000018
        uint8_t CHSC5816_REG_BOOT_STATE[] = {0x20, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00};
        if (comm->writeBuffer(CHSC5816_REG_BOOT_STATE, arraySize(CHSC5816_REG_BOOT_STATE)) < 0) {
            log_e("comm->writeBuffer clean boot state failed!\n");
            return false;
        }

        reset();

        for (int i = 0; i < 10; ++i) {
            hal->delay(10);
            // CHSC5816_REG_IMG_HEAD 0x20000014
            uint8_t CHSC5816_REG_IMG_HEAD[] = {0x20, 0x00, 0x00, 0x14};
            if (comm->writeThenRead(CHSC5816_REG_IMG_HEAD,
                                    arraySize(CHSC5816_REG_IMG_HEAD),
                                    (uint8_t *)&first,
                                    sizeof(Header_t)) < 0) {
                return false;
            }

            if (comm->writeThenRead(CHSC5816_REG_IMG_HEAD,
                                    arraySize(CHSC5816_REG_IMG_HEAD),
                                    (uint8_t *)&second,
                                    sizeof(Header_t)) < 0) {
                return false;
            }

            if (memcmp(&second, &first, sizeof(Header_t)) != 0 ) {
                continue;
            }
            if (first.sig == CHSC5816_SIG_VALUE) {
                return true;
            }
        }
        return false;
    }
*/



/*
bool CHSC5816Touchscreen::getResolution(int16_t *width, int16_t *height) {
  //TODO: NEED TEST
  uint8_t CHSC5816_REG_FW[] = {
    0x20, 0x00, 0x00, 0x00, // CHSC5816_REG_CMD_BUFF
    0xFC, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xe9
  };
  
  if (!this->read_bytes())
  if (comm->writeThenRead(CHSC5816_REG_FW,
                          arraySize(CHSC5816_REG_FW),
                          CHSC5816_REG_FW,
                          arraySize(CHSC5816_REG_FW)) < 0)
  {
      return false;
  }

  SensorLibDumpBuffer(CHSC5816_REG_FW, arraySize(CHSC5816_REG_FW));

        int16_t res_w =  (CHSC5816_REG_FW[2] << 8) | CHSC5816_REG_FW[3];
        int16_t res_h =  (CHSC5816_REG_FW[4] << 8) | CHSC5816_REG_FW[5];
        if (width) {
            *width = res_w;
        }
        if (height) {
            *height = res_h;
        }
        return true;
    }
    */
    


void CHSC5816Touchscreen::update_touches() {
  uint8_t data[8];
  int pointed = 0;
  int x, y;
  union _rpt_point_t *ppt;
  
  if (!this->semi_touch_read_bytes(0x2000002c, data, sizeof(data))) {
    ESP_LOGD(TAG, "CHSC5816 touch error: %x", data);
    return;
  }
  ESP_LOGD(TAG, "CHSC5816 touch detected: %x %x %x %x", data[0], data[1], data[2], data[7]);

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
    ESP_LOGD(TAG, "CHSC5816 touch, could not parse data");
    return;
  }
  
  
  /*
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01D3AA08008000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01E0A208008000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01ED9A08008000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01FC9108008000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF010A8808018000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01177D08018000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF01227508018000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
[20:00:12][VV][touchscreen:039]: << Do Touch loop >>
[20:00:12][VV][i2c.idf:205]: 0x2E TX 2000002C
[20:00:12][VV][i2c.idf:181]: 0x2E RX FF012C6D08018000
[20:00:12][D][chsc5816.touchscreen:235]: CHSC5816 touch detected: 0 0 0 0
[20:00:12][D][chsc5816.touchscreen:251]: CHSC5816 touch, could not parse data
[20:00:12][V][touchscreen:116]: Touch status: is_touched=0, was_touched=0
  */
  
  
  
/*
  The INT pin of CHSC5816 triggers an interrupt and the MCU reads the data once.
  Write device address 0x5C and write four register addresses.
    0x5C 0x20 0x00 0x00 0x2C
  Write device address 0x5D and read the required length of touch data.
    0x5D 0xFF 0x01 0x3E 0xB6 0x08 0x01 0x80 0x00
  

  uint8_t data[CHSC5816_REG_STATUS_LEN];

  if (!this->read_bytes(CHSC5816_REG_STATUS, data, sizeof(data))) {
    ESP_LOGCONFIG(TAG, "CHSC5816 touch error: %x", data);
    return;
  }
  ESP_LOGCONFIG(TAG, "CHSC5816 touch success: %x", data);

  uint8_t num_of_touches = data[CHSC5816_REG_STATUS_TOUCH];

  if (num_of_touches == 1) {
    uint16_t x = data[CHSC5816_REG_STATUS_X_COR];
    uint16_t y = data[CHSC5816_REG_STATUS_Y_COR];
    this->add_raw_touch_position_(0, x, y);
  }
*/
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
