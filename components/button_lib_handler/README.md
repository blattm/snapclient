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

Navigate to: **Button Handler Configuration** (at the top level of menuconfig)

### Board Presets

Choose from pre-configured board presets or define custom GPIO pins:

- 🎛️ **ESP32-LyraT v4.3** - Uses GPIO: Play=33, Vol Up=27, Vol Down=13, Sleep=32
- 🎛️ **ESP32-Audio-Kit v2.2** - Uses GPIO: Play=23, Vol Up=5, Vol Down=18, Sleep=13
- ⚙️ **Custom/Generic** - Manually define your own GPIO pins

**Note:** When a preset is selected, the custom GPIO configuration menu is hidden and the preset's GPIO values are used. To configure custom pins, select "Custom/Generic".

### GPIO Pin Configuration (Custom preset only)

When "Custom/Generic" preset is selected:

- **Play Button GPIO**: Set GPIO number or -1 to disable
- **Volume Up GPIO**: Set GPIO number or -1 to disable
- **Volume Down GPIO**: Set GPIO number or -1 to disable
- **Sleep Button GPIO**: Set GPIO number or -1 to disable

### Volume Control Configuration

- **Volume change step (%)**: Default 5%, range 1-50%
  - Determines how much volume changes per button press

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

### Volume Control (Implemented)

Volume up/down buttons are fully functional:
- Reads current volume from `audioDAC_data.volume`
- Adjusts by configured step percentage (default 5%)
- Clamps to 0-100% range
- Applies via `audio_set_volume()`
- Logs: "Volume up: 50% -> 55%"

**Note:** Current implementation sets volume locally. For multi-client setups with sync through snapserver, the snapcast protocol would need to support client-initiated volume change requests.

### Media Control (Not Yet Implemented)

Play button callbacks currently log actions. To enable media control functionality:

- `play_button_single_click_cb()` → Add play/pause toggle call
- `play_button_double_click_cb()` → Add next track call
- `play_button_triple_click_cb()` → Add previous track call

See `VOLUME_INTEGRATION_PLAN.md` for integration architecture details.

## Dependencies

Automatically managed via ESP Component Registry:
- **espressif/button** ^4.1.4
- ESP-IDF >=5.0

## Notes

⚠️ **GPIO defaults are placeholders** - Always verify against your board schematic before deploying.
