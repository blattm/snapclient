# Volume Control Integration Plan

## Current Architecture

The snapclient uses a client-server architecture where:
- **Server** manages the authoritative state (volume, mute, etc.)
- **Client** receives SERVER_SETTINGS messages with current volume
- Volume changes must go through the server to maintain sync

## Problem

Button presses need to:
1. Get current volume from server state
2. Calculate new volume (±5% by default)
3. Send volume change request to server
4. Wait for server to send updated SERVER_SETTINGS message back
5. Apply the new volume locally

## Proposed Solution

### Option 1: Direct Integration (Simpler, but tight coupling)

**Pros:**
- Simple implementation
- Direct access to snapcast state
- No additional APIs needed

**Cons:**
- Tight coupling between button handler and snapclient
- Button handler needs to know about snapcast internals

**Implementation:**
```c
// In button_lib_handler.c
extern snapcastSetting_t scSet;  // Declared in main.c
extern void audio_set_volume(int volume);  // Declared in main.c

static void vol_up_button_click_cb(void *arg, void *data) {
    int current_volume = scSet.volume;
    int new_volume = current_volume + CONFIG_BUTTON_HANDLER_VOLUME_STEP;
    if (new_volume > 100) new_volume = 100;
    
    // TODO: Send volume change request to server
    // For now, just log and set locally (will be overwritten by server)
    ESP_LOGI(TAG, "Volume up: %d -> %d", current_volume, new_volume);
    audio_set_volume(new_volume);
}
```

### Option 2: API-based Integration (Better design)

Create a snapclient control API that button handler can use.

**Pros:**
- Clean separation of concerns
- Testable
- Reusable for other control interfaces (HTTP, etc.)

**Cons:**
- More code to write
- Need to create new API layer

**Implementation:**
```c
// snapclient_control.h (new file)
esp_err_t snapclient_volume_up(int step_percent);
esp_err_t snapclient_volume_down(int step_percent);
int snapclient_get_current_volume(void);

// button_lib_handler.c
#include "snapclient_control.h"

static void vol_up_button_click_cb(void *arg, void *data) {
    snapclient_volume_up(CONFIG_BUTTON_HANDLER_VOLUME_STEP);
}
```

### Option 3: Event-based (Most decoupled, but complex)

Use ESP-IDF event loop to decouple button handler from snapclient.

**Pros:**
- Maximum decoupling
- Multiple listeners possible
- Follows ESP-IDF patterns

**Cons:**
- Most complex
- Overkill for this use case

## Recommendation: Option 1 for MVP

For the initial implementation, use Option 1:
- It's the simplest and fastest to implement
- Provides immediate functionality
- Can be refactored to Option 2 later if needed

## Implementation Steps

1. **In button_lib_handler.c:**
   - Declare extern references to `scSet` and `audio_set_volume()`
   - Update volume button callbacks to:
     - Read current volume from `scSet.volume`
     - Calculate new volume (current ± step)
     - Clamp to 0-100 range
     - Call `audio_set_volume(new_volume)`
     - Log the change

2. **Server Communication (Future Work):**
   Currently, snapclient doesn't send volume change requests back to the server.
   The architecture assumes the server controls volume through SERVER_SETTINGS messages.
   
   To properly implement this:
   - Need to extend snapcast protocol to support client-initiated volume change requests
   - Or use an external control mechanism (e.g., HTTP API to snapserver)
   
   For now, local volume changes will work but won't sync to other clients.

## Volume Step Configuration

Already added in Kconfig.projbuild:
```
config BUTTON_HANDLER_VOLUME_STEP
    int "Volume change step (%)"
    default 5
    range 1 50
```

## Testing Plan

1. Enable button handler in menuconfig
2. Set volume step to 5%
3. Press volume up/down buttons
4. Verify console logs show volume changes
5. Verify audio output volume changes
6. (Future) Verify volume changes sync through server
