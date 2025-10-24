# Button Library Handler Component

Hardware button support for ESP32 Snapclient using the official [espressif/button](https://components.espressif.com/components/espressif/button) component.

## Features

### Four Configurable Buttons

- 🎵 **Play button** (multi-click detection):
  - Single click: toggle play/pause
  - Double click: next track
  - Triple click: previous track
- 🔊 **Volume up button**: increase volume
- 🔉 **Volume down button**: decrease volume  
- 💤 **Sleep button**: enter deep sleep with ext1 wakeup

## Configuration

All GPIO pins are fully configurable through `idf.py menuconfig`:

Navigate to: **Component config → Button Handler Configuration**

### Board Presets

Choose from pre-configured board presets or define custom GPIO pins:

- 🎛️ **ESP32-LyraT v4.3** - Default GPIO: Play=33, Vol Up=27, Vol Down=13, Sleep=32
- 🎛️ **ESP32-Audio-Kit v2.2** - Default GPIO: Play=23, Vol Up=5, Vol Down=18, Sleep=13
- ⚙️ **Custom/Generic** - Manually define your own GPIO pins

Even after selecting a preset, you can customize individual pins.

### GPIO Pin Configuration

- **Play Button GPIO**: Default varies by preset, set to -1 to disable
- **Volume Up GPIO**: Default varies by preset, set to -1 to disable
- **Volume Down GPIO**: Default varies by preset, set to -1 to disable
- **Sleep Button GPIO**: Default varies by preset, set to -1 to disable

## Usage

1. Enable in menuconfig: **Component config → Button Handler Configuration → Enable Button Handler**
2. Select board preset or configure custom GPIO pins
3. Call `button_lib_handler_init()` in your application's `app_main()`

```c
#include "button_lib_handler.h"

void app_main(void) {
    // ... other initialization code ...
    
    esp_err_t ret = button_lib_handler_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Button handler initialized");
    }
    
    // ... rest of your code ...
}
```

## Implementation Details

- Uses official **espressif/button** component v4.1.4
- Managed dependencies via `idf_component.yml` (no git submodules needed)
- Compatible with **ESP-IDF v5.x**
- Deep sleep/wake detection with `esp_sleep_get_wakeup_cause()`
- Active-low button wiring (press = GND) with internal pull-ups
- TODO markers for snapclient API integration

## Wiring

Buttons should be wired as **active-low** (common cathode):
- Button pin → GPIO
- Button other pin → GND
- Internal pull-ups are enabled automatically

## Deep Sleep

When the sleep button is pressed:
1. System logs the sleep event
2. Configures ext1 wakeup on the sleep button GPIO
3. Enters deep sleep mode
4. Can be woken by pressing the sleep button again

On wake, the component detects the wakeup cause and logs it.

## Snapclient API Integration

Button callbacks currently log actions. To enable actual functionality, wire up the snapclient API calls in `button_lib_handler.c`:

- `play_button_single_click_cb()` → Add play/pause toggle call
- `play_button_double_click_cb()` → Add next track call
- `play_button_triple_click_cb()` → Add previous track call
- `vol_up_button_click_cb()` → Add volume up call
- `vol_down_button_click_cb()` → Add volume down call

## Dependencies

Automatically managed via ESP Component Registry:
- **espressif/button** ^4.1.4
- ESP-IDF >=5.0

## Notes

⚠️ **GPIO defaults are placeholders** - Always verify against your board schematic before deploying.
