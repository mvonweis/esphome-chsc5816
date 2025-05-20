# esphome-chsc5816

This is an ESPHome component for the CHSC5816 touchscreen driver, which can be found e.g. in the LilyGo T-Encoder Pro. The code is based on the Arduino ESP32 libraries (v3.0.7).

The component works, but the code doesn't look very nice.


## YAML snippet for ESPHome

```
substitutions:
  touch_pin_i2c_sda: "GPIO5" # For the LilyGo T-Encoder Pro
  touch_pin_i2c_scl: "GPIO6"
  touch_pin_i2c_int: "GPIO9"
  touch_pin_i2c_rst: "GPIO8"


i2c:
  - id: i2c_bus_touch
    sda: ${touch_pin_i2c_sda}
    scl: ${touch_pin_i2c_scl}

touchscreen:
  - platform: chsc5816
    id: device_lilygo_tencoderpro_touch
 #   address: 0x2E
    i2c_id: i2c_bus_touch
    display: device_lilygo_display
    interrupt_pin: ${touch_pin_i2c_int}
    reset_pin: ${touch_pin_i2c_rst}

display:
  - platform: qspi_dbi
    id: device_lilygo_display
    # Etc.
```
