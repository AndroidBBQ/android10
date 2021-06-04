/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "ANativeWindow"

#include <grallocusage/GrallocUsageConversion.h>
// from nativewindow/includes/system/window.h
// (not to be confused with the compatibility-only window.h from system/core/includes)
#include <system/window.h>

#include <private/android/AHardwareBufferHelpers.h>

#include <ui/GraphicBuffer.h>

using namespace android;

static int32_t query(ANativeWindow* window, int what) {
    int value;
    int res = window->query(window, what, &value);
    return res < 0 ? res : value;
}

static bool isDataSpaceValid(ANativeWindow* window, int32_t dataSpace) {
    bool supported = false;
    switch (dataSpace) {
        case HAL_DATASPACE_UNKNOWN:
        case HAL_DATASPACE_V0_SRGB:
            return true;
        // These data space need wide gamut support.
        case HAL_DATASPACE_V0_SCRGB_LINEAR:
        case HAL_DATASPACE_V0_SCRGB:
        case HAL_DATASPACE_DISPLAY_P3:
            native_window_get_wide_color_support(window, &supported);
            return supported;
        // These data space need HDR support.
        case HAL_DATASPACE_BT2020_PQ:
            native_window_get_hdr_support(window, &supported);
            return supported;
        default:
            return false;
    }
}

/**************************************************************************************************
 * NDK
 **************************************************************************************************/

void ANativeWindow_acquire(ANativeWindow* window) {
    // incStrong/decStrong token must be the same, doesn't matter what it is
    window->incStrong((void*)ANativeWindow_acquire);
}

void ANativeWindow_release(ANativeWindow* window) {
    // incStrong/decStrong token must be the same, doesn't matter what it is
    window->decStrong((void*)ANativeWindow_acquire);
}

int32_t ANativeWindow_getWidth(ANativeWindow* window) {
    return query(window, NATIVE_WINDOW_WIDTH);
}

int32_t ANativeWindow_getHeight(ANativeWindow* window) {
    return query(window, NATIVE_WINDOW_HEIGHT);
}

int32_t ANativeWindow_getFormat(ANativeWindow* window) {
    return query(window, NATIVE_WINDOW_FORMAT);
}

int32_t ANativeWindow_setBuffersGeometry(ANativeWindow* window,
        int32_t width, int32_t height, int32_t format) {
    int32_t err = native_window_set_buffers_format(window, format);
    if (!err) {
        err = native_window_set_buffers_user_dimensions(window, width, height);
        if (!err) {
            int mode = NATIVE_WINDOW_SCALING_MODE_FREEZE;
            if (width && height) {
                mode = NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW;
            }
            err = native_window_set_scaling_mode(window, mode);
        }
    }
    return err;
}

int32_t ANativeWindow_lock(ANativeWindow* window, ANativeWindow_Buffer* outBuffer,
        ARect* inOutDirtyBounds) {
    return window->perform(window, NATIVE_WINDOW_LOCK, outBuffer, inOutDirtyBounds);
}

int32_t ANativeWindow_unlockAndPost(ANativeWindow* window) {
    return window->perform(window, NATIVE_WINDOW_UNLOCK_AND_POST);
}

int32_t ANativeWindow_setBuffersTransform(ANativeWindow* window, int32_t transform) {
    static_assert(ANATIVEWINDOW_TRANSFORM_MIRROR_HORIZONTAL == NATIVE_WINDOW_TRANSFORM_FLIP_H);
    static_assert(ANATIVEWINDOW_TRANSFORM_MIRROR_VERTICAL == NATIVE_WINDOW_TRANSFORM_FLIP_V);
    static_assert(ANATIVEWINDOW_TRANSFORM_ROTATE_90 == NATIVE_WINDOW_TRANSFORM_ROT_90);

    constexpr int32_t kAllTransformBits =
            ANATIVEWINDOW_TRANSFORM_MIRROR_HORIZONTAL |
            ANATIVEWINDOW_TRANSFORM_MIRROR_VERTICAL |
            ANATIVEWINDOW_TRANSFORM_ROTATE_90 |
            // We don't expose INVERSE_DISPLAY as an NDK constant, but someone could have read it
            // from a buffer already set by Camera framework, so we allow it to be forwarded.
            NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
    if (!window || !query(window, NATIVE_WINDOW_IS_VALID))
        return -EINVAL;
    if ((transform & ~kAllTransformBits) != 0)
        return -EINVAL;

    return native_window_set_buffers_transform(window, transform);
}

int32_t ANativeWindow_setBuffersDataSpace(ANativeWindow* window, int32_t dataSpace) {
    static_assert(static_cast<int>(ADATASPACE_UNKNOWN) == static_cast<int>(HAL_DATASPACE_UNKNOWN));
    static_assert(static_cast<int>(ADATASPACE_SCRGB_LINEAR) == static_cast<int>(HAL_DATASPACE_V0_SCRGB_LINEAR));
    static_assert(static_cast<int>(ADATASPACE_SRGB) == static_cast<int>(HAL_DATASPACE_V0_SRGB));
    static_assert(static_cast<int>(ADATASPACE_SCRGB) == static_cast<int>(HAL_DATASPACE_V0_SCRGB));
    static_assert(static_cast<int>(ADATASPACE_DISPLAY_P3) == static_cast<int>(HAL_DATASPACE_DISPLAY_P3));
    static_assert(static_cast<int>(ADATASPACE_BT2020_PQ) == static_cast<int>(HAL_DATASPACE_BT2020_PQ));

    if (!window || !query(window, NATIVE_WINDOW_IS_VALID) ||
            !isDataSpaceValid(window, dataSpace)) {
        return -EINVAL;
    }
    return native_window_set_buffers_data_space(window,
                                                static_cast<android_dataspace_t>(dataSpace));
}

int32_t ANativeWindow_getBuffersDataSpace(ANativeWindow* window) {
    if (!window || !query(window, NATIVE_WINDOW_IS_VALID))
        return -EINVAL;
    return query(window, NATIVE_WINDOW_DATASPACE);
}

/**************************************************************************************************
 * vndk-stable
 **************************************************************************************************/

AHardwareBuffer* ANativeWindowBuffer_getHardwareBuffer(ANativeWindowBuffer* anwb) {
    return AHardwareBuffer_from_GraphicBuffer(static_cast<GraphicBuffer*>(anwb));
}

int ANativeWindow_OemStorageSet(ANativeWindow* window, uint32_t slot, intptr_t value) {
    if (slot < 4) {
        window->oem[slot] = value;
        return 0;
    }
    return -EINVAL;
}

int ANativeWindow_OemStorageGet(ANativeWindow* window, uint32_t slot, intptr_t* value) {
    if (slot >= 4) {
        *value = window->oem[slot];
        return 0;
    }
    return -EINVAL;
}


int ANativeWindow_setSwapInterval(ANativeWindow* window, int interval) {
    return window->setSwapInterval(window, interval);
}

int ANativeWindow_query(const ANativeWindow* window, ANativeWindowQuery what, int* value) {
    switch (what) {
        case ANATIVEWINDOW_QUERY_MIN_UNDEQUEUED_BUFFERS:
        case ANATIVEWINDOW_QUERY_DEFAULT_WIDTH:
        case ANATIVEWINDOW_QUERY_DEFAULT_HEIGHT:
        case ANATIVEWINDOW_QUERY_TRANSFORM_HINT:
            // these are part of the VNDK API
            break;
        case ANATIVEWINDOW_QUERY_MIN_SWAP_INTERVAL:
            *value = window->minSwapInterval;
            return 0;
        case ANATIVEWINDOW_QUERY_MAX_SWAP_INTERVAL:
            *value = window->maxSwapInterval;
            return 0;
        case ANATIVEWINDOW_QUERY_XDPI:
            *value = (int)window->xdpi;
            return 0;
        case ANATIVEWINDOW_QUERY_YDPI:
            *value = (int)window->ydpi;
            return 0;
        default:
            // asked for an invalid query(), one that isn't part of the VNDK
            return -EINVAL;
    }
    return window->query(window, int(what), value);
}

int ANativeWindow_queryf(const ANativeWindow* window, ANativeWindowQuery what, float* value) {
    switch (what) {
        case ANATIVEWINDOW_QUERY_XDPI:
            *value = window->xdpi;
            return 0;
        case ANATIVEWINDOW_QUERY_YDPI:
            *value = window->ydpi;
            return 0;
        default:
            break;
    }

    int i;
    int e = ANativeWindow_query(window, what, &i);
    if (e == 0) {
        *value = (float)i;
    }
    return e;
}

int ANativeWindow_dequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd) {
    return window->dequeueBuffer(window, buffer, fenceFd);
}

int ANativeWindow_queueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    return window->queueBuffer(window, buffer, fenceFd);
}

int ANativeWindow_cancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    return window->cancelBuffer(window, buffer, fenceFd);
}

int ANativeWindow_setUsage(ANativeWindow* window, uint64_t usage) {
    return native_window_set_usage(window, usage);
}

int ANativeWindow_setBufferCount(ANativeWindow* window, size_t bufferCount) {
    return native_window_set_buffer_count(window, bufferCount);
}

int ANativeWindow_setBuffersDimensions(ANativeWindow* window, uint32_t w, uint32_t h) {
    return native_window_set_buffers_dimensions(window, (int)w, (int)h);
}

int ANativeWindow_setBuffersFormat(ANativeWindow* window, int format) {
    return native_window_set_buffers_format(window, format);
}

int ANativeWindow_setBuffersTimestamp(ANativeWindow* window, int64_t timestamp) {
    return native_window_set_buffers_timestamp(window, timestamp);
}

int ANativeWindow_setSharedBufferMode(ANativeWindow* window, bool sharedBufferMode) {
    return native_window_set_shared_buffer_mode(window, sharedBufferMode);
}

int ANativeWindow_setAutoRefresh(ANativeWindow* window, bool autoRefresh) {
    return native_window_set_auto_refresh(window, autoRefresh);
}
