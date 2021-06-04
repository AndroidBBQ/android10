/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SURFACEFLINGERPROPERTIES_H_
#define SURFACEFLINGERPROPERTIES_H_

#include <SurfaceFlingerProperties.sysprop.h>
#include <android/hardware/configstore/1.1/ISurfaceFlingerConfigs.h>
#include <android/hardware/graphics/common/1.2/types.h>
#include <ui/ConfigStoreTypes.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace android {
namespace sysprop {

int64_t vsync_event_phase_offset_ns(int64_t defaultValue);

int64_t vsync_sf_event_phase_offset_ns(int64_t defaultValue);

bool use_context_priority(bool defaultValue);

int64_t max_frame_buffer_acquired_buffers(int64_t defaultValue);

bool has_wide_color_display(bool defaultValue);

bool running_without_sync_framework(bool defaultValue);

bool has_HDR_display(bool defaultValue);

int64_t present_time_offset_from_vsync_ns(int64_t defaultValue);

bool force_hwc_copy_for_virtual_displays(bool defaultValue);

int64_t max_virtual_display_dimension(int64_t defaultValue);

bool use_vr_flinger(bool defaultValue);

bool start_graphics_allocator_service(bool defaultValue);

SurfaceFlingerProperties::primary_display_orientation_values primary_display_orientation(
        SurfaceFlingerProperties::primary_display_orientation_values defaultValue);

bool use_color_management(bool defaultValue);

int64_t default_composition_dataspace(
        android::hardware::graphics::common::V1_2::Dataspace defaultValue);

int32_t default_composition_pixel_format(
        android::hardware::graphics::common::V1_2::PixelFormat defaultValue);

int64_t wcg_composition_dataspace(
        android::hardware::graphics::common::V1_2::Dataspace defaultValue);

int32_t wcg_composition_pixel_format(
        android::hardware::graphics::common::V1_2::PixelFormat defaultValue);

int64_t color_space_agnostic_dataspace(
        android::hardware::graphics::common::V1_2::Dataspace defaultValue);

bool refresh_rate_switching(bool defaultValue);

int32_t set_idle_timer_ms(int32_t defaultValue);

int32_t set_touch_timer_ms(int32_t defaultValue);

int32_t set_display_power_timer_ms(int32_t defaultValue);

bool use_smart_90_for_video(bool defaultValue);

bool enable_protected_contents(bool defaultValue);

bool support_kernel_idle_timer(bool defaultValue);

android::ui::DisplayPrimaries getDisplayNativePrimaries();
} // namespace sysprop
} // namespace android
#endif // SURFACEFLINGERPROPERTIES_H_
