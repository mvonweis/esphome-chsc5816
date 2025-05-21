# esphome-chsc5816

This is an ESPHome component for the CHSC5816 touchscreen controller, which can be found e.g. in the [LilyGo T-Encoder Pro](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/tree/arduino-esp32-libs_V3.0.7).

Our code is based on the snippets provided in the [CHSC5816 Touch Control Chip User Manual V1](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CHSC5816%E8%A7%A6%E6%8E%A7%E8%8A%AF%E7%89%87%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8EV1-20221114.pdf) provided by the chip manufacturer, Chipsemi. We also studied [LilyGo's example .ino](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/blob/arduino-esp32-libs_V3.0.7/examples/CHSC5816/CHSC5816.ino) but found it to be less useful.

## Shortcomings

- The component works and is stable, but the code hasn't been cleaned up.
- The component can currently only read touch points from the chip.
- Swiping is implemented by tracking the touch points in ESPHome, although this is less responsive than we would like. The chip apparently supports gestures in hardware and we should utilize that.


## YAML snippet for ESPHome

The pinouts below are for the LilyGo T-Encoder Pro. Other ESP32 devices may use different pinouts and may not have the button and rotary encoder found on the LilyGo. 

```
# Include this CHSC5816 component.

external_components:
  - source: github://mvonweis/esphome-chsc5816
    components: [ chsc5816 ]

# The touchscreen part. First set up the internal I2C bus on pins 5 and 6, then the
# CHSC5816 chip on pins 8 and 9. Add two triggers, one for touch and one for swipe.
# Note that there is another I2C bus on pins 15 and 16 connected to one of the QWIIC
# connectors.

i2c:
  - id: i2c_bus_touch
    sda: GPIO5
    scl: GPIO6

touchscreen:
  - platform: chsc5816
    id: device_lilygo_tencoderpro_touch
    i2c_id: i2c_bus_touch
    # address: 0x2E # Not needed, since this seems to be the only device on this bus.
    display: device_lilygo_display
    reset_pin: GPIO8
    interrupt_pin: GPIO9
    on_touch: # Just logging
      - lambda: |-
          ESP_LOGI("Touch on_touch:", "id=%d x=%d, y=%d", touch.id, touch.x, touch.y);
    on_update: # Our best attempt at detecting swipes and triggering scripts
      - lambda: |-
          for (auto touch: touches)  {
            if (touch.state == 2) {
              ESP_LOGI("Touch on_update:", "id=%d, s=%d, x=%d, y=%d", touch.id, touch.state, touch.x, touch.y);
            } else if (touch.state == 6) {
              if (touch.x_org > touch.x_prev) {
                ESP_LOGE("Touch on_update", "Detected RIGHT to LEFT swipe");
                id(swipe_left).execute();
              } else if (touch.x_org < touch.x_prev) {
                ESP_LOGE("Touch on_update", "Detected LEFT to RIGHT swipe");
                id(swipe_right).execute();
              }
            }
          }

# Now set up the LilyGo display on the quad SPI bus. It requires an init sequence.

spi:
  id: display_qspi
  type: quad
  clk_pin: GPIO12
  data_pins: [GPIO11, GPIO13, GPIO7, GPIO14]

display:
  - platform: qspi_dbi
    id: device_lilygo_display
    dimensions:
      width: 390
      height: 390
    model: CUSTOM
    cs_pin: GPIO10
    reset_pin: GPIO4
    enable_pin:
      number: GPIO3
      ignore_strapping_warning: true
    rotation: 0
    invert_colors: false
    draw_from_origin: true
    auto_clear_enabled: false
    update_interval: never
    init_sequence:
      - [0x01]  # Software Reset
      - delay 200ms
      - [0x11]  # Sleep Out
      - delay 120ms
      - [0x13]  # Normal Display Mode On
      - [0x20]  # Display Inversion Off
      - [0x3A, 0x05] # Interface Pixel Format 16bit/pixel
      - [0x29]  # Display On
      - [0x53, 0x28]  # Brightness Control On and Display Dimming On
      - [0x51, 0x00]  # Brightness Adjustment
      # High contrast mode (Sunlight Readability Enhancement)
      - [0x58, 0x00]  # High Contrast Mode Off
      # - [0x58, 0x05]  # High Contrast Mode Low
      # - [0x58, 0x06]  # High Contrast Mode Med
      # - [0x58, 0x07]  # High Contrast Mode High
      - delay 10ms

# Set up the rotary encoder and push button on the Lilygo T-Encoder Pro.

sensor:
  - platform: rotary_encoder
    id: device_rotary_encoder
    pin_a: GPIO1
    pin_b: GPIO2
    resolution: 2 # Seems to work best with resolution 2 (tried 1 and 4).

binary_sensor:
  - platform: gpio
    id: device_button_press
    pin:
      number: GPIO0
      ignore_strapping_warning: True

# Two scripts triggered by touchscreen swipes. 

script:
  - id: swipe_left
    then:
      - lvgl.page.previous:
          animation: OUT_LEFT
          time: 300ms

  - id: swipe_right
    then:
      - lvgl.page.next:
            animation: OUT_RIGHT
            time: 300ms

```
