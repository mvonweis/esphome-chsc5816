# esphome-chsc5816

This is an [ESPHome](https://esphome.io/) component for the CHSC5816 touchscreen controller. The controller can be found e.g. in the [LilyGo T-Encoder Pro](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/tree/arduino-esp32-libs_V3.0.7) ESP32 MCU.

Our code is based on the snippets provided in the [CHSC5816 Touch Control Chip User Manual V1](https://github.com/lewisxhe/SensorLib/blob/master/datasheet/CHSC5816%E8%A7%A6%E6%8E%A7%E8%8A%AF%E7%89%87%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8EV1-20221114.pdf) provided by the chip manufacturer, Chipsemi. We also studied [LilyGo's example .ino](https://github.com/Xinyuan-LilyGO/T-Encoder-Pro/blob/arduino-esp32-libs_V3.0.7/examples/CHSC5816/CHSC5816.ino) as well as [SensorLib](https://www.arduinolibraries.info/libraries/sensor-lib) but found these to to be less useful.

According to Chipsemi's docs, the CHSC5816 supports touch pressure and also recognizes swipes in the four cardinal directions. Unfortunately the ESPHome [touchscreen](https://esphome.io/components/touchscreen/index.html) component doesn't carry this information through.

The component works and seems to be stable.

## TODOs

- Clean up the code and remove unnecessary lines.
- Check if hardware swipe functionality can be utilized in ESPHome.

## YAML snippets for ESPHome

There is a sample file `lilygo-touchscreen-test.yaml` that compiles and installs on the Lilygo T-Encoder Pro using ESPHome 2025.6.0.

### Touchscreen setup

First, include the CHSC5816 component from this github repo.

Next, set up the internal I2C bus for the touch screen on pins 5 and 6, then the CHSC5816 chip on pins 8 and 9 (these pins are specific to the T-Encoder Pro). Touches trigger an interrupt, so there's no need to poll. 

Finally, set up an `on_touch` trigger for information logging and a swipe-detecting `on_update` trigger that calls some scripts. If you're not interested in swipe detection, you should remove the second trigger, as we haven't tested how it interferes with the normal UI in LVGL.

```
external_components:
  - source: github://mvonweis/esphome-chsc5816
    components: [ chsc5816 ]

i2c:
  - id: i2c_bus_touch
    sda: GPIO5
    scl: GPIO6

touchscreen:
  - platform: chsc5816
    id: device_lilygo_tencoderpro_touch
    i2c_id: i2c_bus_touch
    # address: 0x2E # Not needed on the T-Encoder Pro, since there's only one device on this bus.
    display: device_lilygo_display
    reset_pin: GPIO8
    interrupt_pin: GPIO9

# Log the touches (change this to ESP_LOGD for debug-level logging).
    on_touch:
      - lambda: |-
          ESP_LOGI("Touch on_touch:", "id=%d x=%d, y=%d", touch.id, touch.x, touch.y);
          
# This is our best attempt at detecting swipes and triggering scripts. 30 pixels translates to
# around 3 mm on the T-Encoder screen.
    on_update:
      - lambda: |-
          for (auto touch: touches)  {
            if (touch.state == 2) { # touch active, coordinates updated 
              ESP_LOGI("Touch on_update:", "id=%d, s=%d, x=%d, y=%d", touch.id, touch.state, touch.x, touch.y);
            } else if (touch.state == 6) { # finger lifted, check coordinates
              if (touch.x_prev < touch.x_org - 30) {
                ESP_LOGI("Touch on_update", "Detected RIGHT to LEFT swipe");
                id(swipe_left).execute();
              } else if (touch.x_prev > touch.x_org + 30) {
                ESP_LOGI("Touch on_update", "Detected LEFT to RIGHT swipe");
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

