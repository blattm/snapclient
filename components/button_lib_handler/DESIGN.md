# Button Handler Feature Documentation

## Overview

The button handler component provides hardware button support for the ESP32 Snapclient using the official Espressif button component library.

## Architecture

```
┌─────────────────────────────────────────┐
│         ESP32 Application               │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │     Button Handler Component      │ │
│  │                                   │ │
│  │  - Play button (multi-click)      │ │
│  │  - Volume up                      │ │
│  │  - Volume down                    │ │
│  │  - Sleep button                   │ │
│  └──────────┬────────────────────────┘ │
│             │                           │
│  ┌──────────▼────────────────────────┐ │
│  │  espressif/button v4.1.4          │ │
│  │  (ESP Component Registry)         │ │
│  └──────────┬────────────────────────┘ │
│             │                           │
└─────────────┼───────────────────────────┘
              │
       ┌──────▼──────┐
       │   GPIO Pins │
       └──────┬──────┘
              │
       ┌──────▼──────┐
       │   Buttons   │
       │  (Active-Low)│
       └─────────────┘
```

## Component Structure

```
components/button_lib_handler/
├── CMakeLists.txt              # Build configuration
├── Kconfig                     # menuconfig options
├── idf_component.yml           # Component dependencies
├── README.md                   # Component documentation
├── button_lib_handler.c        # Implementation
└── include/
    └── button_lib_handler.h    # Public API
```

## Configuration Flow

1. User runs `idf.py menuconfig`
2. Navigates to "Button Handler Configuration" (at top level, not under Component config)
3. Enables button handler
4. Selects board preset:
   - LyraT v4.3: Uses preset GPIO pins (33, 27, 13, 32)
   - Audio-Kit v2.2: Uses preset GPIO pins (23, 5, 18, 13)
   - Custom/Generic: Opens custom GPIO configuration menu
5. If Custom selected, configures individual GPIO pins
6. Configures volume step percentage (default 5%)
7. Builds project with `idf.py build`

## Runtime Flow

### Initialization
```
app_main()
  └─> button_lib_handler_init()
      ├─> Check wakeup cause (deep sleep)
      ├─> Create play button
      │   ├─> Register single click callback
      │   ├─> Register double click callback
      │   └─> Register triple click callback
      ├─> Create volume up button
      │   └─> Register press callback
      ├─> Create volume down button
      │   └─> Register press callback
      └─> Create sleep button
          └─> Register click callback
```

### Button Press Flow
```
User presses button
  └─> GPIO interrupt
      └─> espressif/button library
          └─> Detect click pattern
              └─> Call registered callback
                  └─> Log action (TODO: snapclient API)
```

### Deep Sleep Flow
```
User presses sleep button
  └─> sleep_button_click_cb()
      ├─> Log message
      ├─> Configure ext1 wakeup
      ├─> Enter deep sleep
      └─> (ESP32 powered down)

User presses sleep button again
  └─> ESP32 wakes up
      └─> app_main() runs
          └─> button_lib_handler_init()
              └─> Detects ESP_SLEEP_WAKEUP_EXT1
                  └─> Logs wakeup message
```

## Board Presets

### ESP32-LyraT v4.3
- Play: GPIO 33
- Volume Up: GPIO 27
- Volume Down: GPIO 13
- Sleep: GPIO 32

### ESP32-Audio-Kit v2.2
- Play: GPIO 23
- Volume Up: GPIO 5
- Volume Down: GPIO 18
- Sleep: GPIO 13

### Custom
- All GPIOs user-configurable
- Set to -1 to disable individual buttons

## Integration Points

### Volume Control (Implemented)

Volume up/down buttons are fully integrated with snapclient:

```c
// In button_lib_handler.c

static void vol_up_button_click_cb(void *arg, void *data) {
    int current_volume = audioDAC_data.volume;
    int new_volume = current_volume + CONFIG_BUTTON_HANDLER_VOLUME_STEP;
    if (new_volume > 100) new_volume = 100;
    
    ESP_LOGI(TAG, "Volume up: %d%% -> %d%%", current_volume, new_volume);
    audio_set_volume(new_volume);
}
```

**External Dependencies:**
- `audioDAC_data` - Global structure containing current volume and mute state
- `audio_set_volume(int)` - Function to apply volume changes

**Architecture Note:**
Current implementation sets volume locally via `audio_set_volume()`. In the snapcast client-server architecture, volume state is managed by the server. A complete implementation would require:
1. Send volume change request to snapserver
2. Server updates its state
3. Server broadcasts SERVER_SETTINGS message to all clients
4. Clients apply the new volume

For single-client setups, the current local-only approach works well. Multi-client synchronization would require protocol extensions. See `VOLUME_INTEGRATION_PLAN.md` for details.

### Media Control (Future Integration)

Play button callbacks currently log actions. To implement:

```c
static void play_button_single_click_cb(void *arg, void *data) {
    snapclient_toggle_play_pause();  // Call snapclient API
}
```

## Testing

### Unit Testing
Component can be tested standalone by:
1. Enabling in menuconfig
2. Configuring GPIO pins
3. Building and flashing
4. Monitoring console output
5. Pressing buttons to verify callbacks

### Console Output
Expected log messages:
```
I (xxx) BUTTON_HANDLER: Initializing button handler component
I (xxx) BUTTON_HANDLER: Play button created on GPIO 33
I (xxx) BUTTON_HANDLER: Play button handlers registered (single/double/triple click)
I (xxx) BUTTON_HANDLER: Volume up button created on GPIO 27
I (xxx) BUTTON_HANDLER: Volume down button created on GPIO 13
I (xxx) BUTTON_HANDLER: Sleep button created on GPIO 32
I (xxx) BUTTON_HANDLER: Button handler component initialized successfully
```

Button press output:
```
I (xxx) BUTTON_HANDLER: Play button: Single click - Toggle play/pause
I (xxx) BUTTON_HANDLER: Play button: Double click - Next track
I (xxx) BUTTON_HANDLER: Volume up button pressed
```

## Dependencies

### Component Registry
- **espressif/button** ^4.1.4
  - Managed via idf_component.yml
  - Auto-downloaded during build
  - No git submodules required

### ESP-IDF
- Requires ESP-IDF >= 5.0
- Uses standard ESP-IDF APIs:
  - driver/gpio.h
  - esp_sleep.h
  - esp_log.h
  - freertos/FreeRTOS.h

## Safety Features

1. **Disabled by default** - Must be explicitly enabled in menuconfig
2. **GPIO validation** - Invalid GPIOs (-1) are safely ignored
3. **Error handling** - Initialization failures are logged
4. **Graceful degradation** - Failed button creation doesn't stop other buttons
5. **Conditional compilation** - Code only compiled when enabled

## Known Limitations

1. **GPIO defaults are placeholders** - Users must verify against board schematic
2. **No snapclient API integration** - Callbacks have TODO markers
3. **Fixed button timing** - Long press and short press times configurable via menuconfig but not runtime
4. **No button state persistence** - Configuration lost on power cycle (use NVS if needed)

## Future Enhancements

Potential improvements:
- [ ] Add long press detection for additional functions
- [ ] Implement button combination detection
- [ ] Add runtime button reconfiguration API
- [ ] Store button configuration in NVS
- [ ] Add LED feedback for button presses
- [ ] Support for matrix keyboards
- [ ] Implement debounce time configuration

## References

- [ESP Component Registry - button](https://components.espressif.com/components/espressif/button)
- [ESP-IDF GPIO API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [ESP-IDF Sleep Modes](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html)
