/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HARDWARE_LEGACY_WIFI_H
#define HARDWARE_LEGACY_WIFI_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * Load the Wi-Fi driver.
 *
 * @return 0 on success, < 0 on failure.
 */
int wifi_load_driver();

/**
 * Unload the Wi-Fi driver.
 *
 * @return 0 on success, < 0 on failure.
 */
int wifi_unload_driver();

/**
 * Check if the Wi-Fi driver is loaded.
 * Check if the Wi-Fi driver is loaded.

 * @return 0 on success, < 0 on failure.
 */
int is_wifi_driver_loaded();

/**
 * Return the path to requested firmware
 */
#define WIFI_GET_FW_PATH_STA  0
#define WIFI_GET_FW_PATH_AP 1
#define WIFI_GET_FW_PATH_P2P  2
const char *wifi_get_fw_path(int fw_type);

/**
 * Change the path to firmware for the wlan driver
 */
int wifi_change_fw_path(const char *fwpath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* HARDWARE_LEGACY_WIFI_H */
