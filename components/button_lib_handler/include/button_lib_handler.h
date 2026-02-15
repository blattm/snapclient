/*
 * Button Library Handler Component for ESP32 Snapclient
 *
 * This component provides hardware button support using the official
 * espressif/button component from the ESP Component Registry.
 *
 * Features:
 * - Configurable GPIO pins via menuconfig
 * - Board-specific presets (LyraT v4.3, Audio-Kit v2.2)
 * - Multi-click detection on play button
 * - Deep sleep support with ext1 wakeup
 *
 * Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */

#ifndef BUTTON_LIB_HANDLER_H
#define BUTTON_LIB_HANDLER_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the button handler component
 *
 * This function initializes all configured buttons based on menuconfig settings.
 * It sets up GPIO pins, button callbacks, and deep sleep wake detection.
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_ERR_INVALID_STATE if button handler is disabled in menuconfig
 *     - ESP_FAIL on initialization failure
 */
esp_err_t button_lib_handler_init(void);

/**
 * @brief Deinitialize the button handler component
 *
 * Cleans up button resources and frees memory.
 *
 * @return
 *     - ESP_OK on success
 *     - ESP_FAIL on failure
 */
esp_err_t button_lib_handler_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_LIB_HANDLER_H */
