# esphome-chsc5816

This is an [ESPHome](https://esphome.io/) component for the CHSC5816 touchscreen controller. The controller can be found e.g. in the [LilyGo T-Encoder Pro](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/tree/arduino-esp32-libs_V3.0.7) ESP32 MCU.

Our code is based on the snippets provided in the [CHSC5816 Touch Control Chip User Manual V1](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CHSC5816%E8%A7%A6%E6%8E%A7%E8%8A%AF%E7%89%87%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8EV1-20221114.pdf) provided by the chip manufacturer, Chipsemi. We also studied [LilyGo's example .ino](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/blob/arduino-esp32-libs_V3.0.7/examples/CHSC5816/CHSC5816.ino) as well as [SensorLib](https://www.arduinolibraries.info/libraries/sensor-lib) but found these to to be less useful.

The CHSC5816 supports touch pressure and also some kind of swiping. Unfortunately the ESPHome [touchscreen](https://esphome.io/components/touchscreen/index.html) component doesn't carry this information through.

The component works and seems to be stable.

## TODOs

- Clean up the code and remove unnecessary lines.
- Check if hardware swipe functionality can be utilized in ESPHome.


## YAML snippets for ESPHome

### Touchscreen setup

Include the CHSC5816 component from this github repo.

```
external_components:
  - source: github://mvonweis/esphome-chsc5816
    components: [ chsc5816 ]
```

Set up the internal I2C bus for the touch screen on pins 5 and 6, then the CHSC5816 chip on pins 8 and 9 (these pins are specific to the T-Encoder Pro). Touches trigger an interrupt, so there's no need to poll. 

There are two triggers, one for `on_touch` which does basic logging. The second, `on_update`, is used to recognize swipes, which is what we need this component for. You may want to remove the triggers.

```
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

# Log the touches (change this to ESP_LOGD for debug-level logging).
    on_touch:
      - lambda: |-
          ESP_LOGI("Touch on_touch:", "id=%d x=%d, y=%d", touch.id, touch.x, touch.y);
          
# This is our best attempt at detecting swipes and triggering scripts.
    on_update:
      - lambda: |-
          for (auto touch: touches)  {
            if (touch.state == 2) { # touch active, coordinates updated 
              ESP_LOGI("Touch on_update:", "id=%d, s=%d, x=%d, y=%d", touch.id, touch.state, touch.x, touch.y);
            } else if (touch.state == 6) { # finger lifted, check coordinates
              if (touch.x_org > touch.x_prev) {
                ESP_LOGE("Touch on_update", "Detected RIGHT to LEFT swipe");
                id(swipe_left).execute();
              } else if (touch.x_org < touch.x_prev) {
                ESP_LOGE("Touch on_update", "Detected LEFT to RIGHT swipe");
                id(swipe_right).execute();
              }
            }
          }
```

Two scripts triggered by touchscreen swipes in the `on_update` section above. They simply change pages in LVGL.

```
script:
  - id: swipe_left
    then:
      - lvgl.page.next:
          animation: OUT_LEFT
          time: 300ms

  - id: swipe_right
    then:
      - lvgl.page.previous:
            animation: OUT_RIGHT
            time: 300ms
```


### Display config for the T-Encoder Pro

This is specific to the T-Encoder Pro and included for convenience. If your MCU is different, use your own display configuration.

The T-Encoder Pro uses a quad SPI chip on specific pins and with a specific init sequence. 

```
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
```

The T-Encoder Pro also has a rotary encoder and a push button.

```
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
```
