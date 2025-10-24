/*
 * Button Library Handler Component for ESP32 Snapclient
 *
 * This component provides hardware button support using the official
 * espressif/button component from the ESP Component Registry.
 *
 * Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */

#include "button_lib_handler.h"

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "sdkconfig.h"

static const char *TAG = "BUTTON_HANDLER";

#ifdef CONFIG_BUTTON_HANDLER_ENABLE

// Button handles
static button_handle_t play_button = NULL;
static button_handle_t vol_up_button = NULL;
static button_handle_t vol_down_button = NULL;
static button_handle_t sleep_button = NULL;

/**
 * @brief Play button single click callback - Toggle play/pause
 */
static void play_button_single_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Play button: Single click - Toggle play/pause");
  // TODO: Wire up snapclient API call to toggle play/pause
  // Example: snapclient_toggle_play_pause();
}

/**
 * @brief Play button double click callback - Next track
 */
static void play_button_double_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Play button: Double click - Next track");
  // TODO: Wire up snapclient API call to skip to next track
  // Example: snapclient_next_track();
}

/**
 * @brief Play button triple click callback - Previous track
 */
static void play_button_triple_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Play button: Triple click - Previous track");
  // TODO: Wire up snapclient API call to go to previous track
  // Example: snapclient_previous_track();
}

/**
 * @brief Volume up button click callback
 */
static void vol_up_button_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Volume up button pressed");
  // TODO: Wire up snapclient API call to increase volume
  // Example: snapclient_volume_up();
}

/**
 * @brief Volume down button click callback
 */
static void vol_down_button_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Volume down button pressed");
  // TODO: Wire up snapclient API call to decrease volume
  // Example: snapclient_volume_down();
}

/**
 * @brief Sleep button click callback - Enter deep sleep
 */
static void sleep_button_click_cb(void *arg, void *data) {
  ESP_LOGI(TAG, "Sleep button pressed - Entering deep sleep");

  // Configure ext1 wakeup on the sleep button GPIO
#if CONFIG_BUTTON_HANDLER_SLEEP_GPIO >= 0
  const uint64_t ext_wakeup_pin_mask = 1ULL << CONFIG_BUTTON_HANDLER_SLEEP_GPIO;
  ESP_LOGI(TAG, "Enabling ext1 wakeup on GPIO %d",
           CONFIG_BUTTON_HANDLER_SLEEP_GPIO);

  // Configure ext1 wakeup - wake on LOW (button press pulls to GND)
  esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_LOW);

  // Give some time for the log to be printed
  vTaskDelay(pdMS_TO_TICKS(100));

  // Enter deep sleep
  esp_deep_sleep_start();
#endif
}

/**
 * @brief Create and configure a button
 */
static esp_err_t create_button(int gpio_num, button_handle_t *button_handle,
                               const char *name) {
  if (gpio_num < 0) {
    ESP_LOGW(TAG, "%s button disabled (GPIO = %d)", name, gpio_num);
    return ESP_ERR_INVALID_ARG;
  }

  button_config_t btn_cfg = {
      .type = BUTTON_TYPE_GPIO,
      .gpio_button_config =
          {
              .gpio_num = gpio_num,
              .active_level = 0,  // Active low (button press pulls to GND)
          },
  };

  *button_handle = iot_button_create(&btn_cfg);
  if (*button_handle == NULL) {
    ESP_LOGE(TAG, "Failed to create %s button on GPIO %d", name, gpio_num);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "%s button created on GPIO %d", name, gpio_num);
  return ESP_OK;
}

esp_err_t button_lib_handler_init(void) {
  ESP_LOGI(TAG, "Initializing button handler component");

  // Check if woken up from deep sleep
  esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
  if (wakeup_cause == ESP_SLEEP_WAKEUP_EXT1) {
    ESP_LOGI(TAG, "Woke up from deep sleep via ext1 (button press)");
  }

  esp_err_t ret;
  int buttons_initialized = 0;

  // Initialize play button with multi-click detection
#if CONFIG_BUTTON_HANDLER_PLAY_GPIO >= 0
  ret = create_button(CONFIG_BUTTON_HANDLER_PLAY_GPIO, &play_button, "Play");
  if (ret == ESP_OK) {
    // Register single click callback
    ret = iot_button_register_cb(play_button, BUTTON_SINGLE_CLICK,
                                 play_button_single_click_cb, NULL);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register play button single click callback: %d",
               ret);
      iot_button_delete(play_button);
      play_button = NULL;
    } else {
      // Register double click callback
      ret = iot_button_register_cb(play_button, BUTTON_DOUBLE_CLICK,
                                   play_button_double_click_cb, NULL);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "Failed to register play button double click callback: %d",
                 ret);
      }

      // Register multiple click callback for triple click
      button_event_config_t multi_click_cfg = {
          .event = BUTTON_MULTIPLE_CLICK,
          .event_data.multiple_clicks.clicks = 3,
      };
      ret = iot_button_register_event_cb(play_button, multi_click_cfg,
                                         play_button_triple_click_cb, NULL);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "Failed to register play button triple click callback: %d",
                 ret);
      }

      ESP_LOGI(TAG,
               "Play button handlers registered (single/double/triple click)");
      buttons_initialized++;
    }
  }
#endif

  // Initialize volume up button
#if CONFIG_BUTTON_HANDLER_VOL_UP_GPIO >= 0
  ret = create_button(CONFIG_BUTTON_HANDLER_VOL_UP_GPIO, &vol_up_button,
                      "Volume Up");
  if (ret == ESP_OK) {
    ret = iot_button_register_cb(vol_up_button, BUTTON_PRESS_DOWN,
                                 vol_up_button_click_cb, NULL);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register volume up button callback: %d", ret);
      iot_button_delete(vol_up_button);
      vol_up_button = NULL;
    } else {
      ESP_LOGI(TAG, "Volume up button handler registered");
      buttons_initialized++;
    }
  }
#endif

  // Initialize volume down button
#if CONFIG_BUTTON_HANDLER_VOL_DOWN_GPIO >= 0
  ret = create_button(CONFIG_BUTTON_HANDLER_VOL_DOWN_GPIO, &vol_down_button,
                      "Volume Down");
  if (ret == ESP_OK) {
    ret = iot_button_register_cb(vol_down_button, BUTTON_PRESS_DOWN,
                                 vol_down_button_click_cb, NULL);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register volume down button callback: %d", ret);
      iot_button_delete(vol_down_button);
      vol_down_button = NULL;
    } else {
      ESP_LOGI(TAG, "Volume down button handler registered");
      buttons_initialized++;
    }
  }
#endif

  // Initialize sleep button
#if CONFIG_BUTTON_HANDLER_SLEEP_GPIO >= 0
  ret = create_button(CONFIG_BUTTON_HANDLER_SLEEP_GPIO, &sleep_button, "Sleep");
  if (ret == ESP_OK) {
    ret = iot_button_register_cb(sleep_button, BUTTON_SINGLE_CLICK,
                                 sleep_button_click_cb, NULL);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register sleep button callback: %d", ret);
      iot_button_delete(sleep_button);
      sleep_button = NULL;
    } else {
      ESP_LOGI(TAG, "Sleep button handler registered");
      buttons_initialized++;
    }
  }
#endif

  if (buttons_initialized > 0) {
    ESP_LOGI(TAG,
             "Button handler component initialized successfully (%d button(s) "
             "active)",
             buttons_initialized);
    return ESP_OK;
  } else {
    ESP_LOGW(TAG, "No buttons were initialized successfully");
    return ESP_ERR_INVALID_STATE;
  }
}

esp_err_t button_lib_handler_deinit(void) {
  ESP_LOGI(TAG, "Deinitializing button handler component");

  if (play_button != NULL) {
    iot_button_delete(play_button);
    play_button = NULL;
  }

  if (vol_up_button != NULL) {
    iot_button_delete(vol_up_button);
    vol_up_button = NULL;
  }

  if (vol_down_button != NULL) {
    iot_button_delete(vol_down_button);
    vol_down_button = NULL;
  }

  if (sleep_button != NULL) {
    iot_button_delete(sleep_button);
    sleep_button = NULL;
  }

  ESP_LOGI(TAG, "Button handler component deinitialized");
  return ESP_OK;
}

#else

// Stub implementations when button handler is disabled
esp_err_t button_lib_handler_init(void) {
  ESP_LOGW(TAG, "Button handler is disabled in menuconfig");
  return ESP_ERR_INVALID_STATE;
}

esp_err_t button_lib_handler_deinit(void) { return ESP_OK; }

#endif /* CONFIG_BUTTON_HANDLER_ENABLE */
