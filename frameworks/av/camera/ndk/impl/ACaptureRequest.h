/*
 * Copyright (C) 2015 The Android Open Source Project
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
#ifndef _ACAPTURE_REQUEST_H
#define _ACAPTURE_REQUEST_H

#include <camera/NdkCaptureRequest.h>
#include <set>
#include <unordered_map>

using namespace android;

#ifdef __ANDROID_VNDK__
#include "ndk_vendor/impl/ACaptureRequestVendor.h"
#else
struct ACameraOutputTarget {
    explicit ACameraOutputTarget(ACameraWindowType* window) : mWindow(window) {};

    bool operator == (const ACameraOutputTarget& other) const {
        return mWindow == other.mWindow;
    }
    bool operator != (const ACameraOutputTarget& other) const {
        return mWindow != other.mWindow;
    }
    bool operator < (const ACameraOutputTarget& other) const {
        return mWindow < other.mWindow;
    }
    bool operator > (const ACameraOutputTarget& other) const {
        return mWindow > other.mWindow;
    }

    ACameraWindowType* mWindow;
};
#endif

struct ACameraOutputTargets {
    std::set<ACameraOutputTarget> mOutputs;
};

struct ACaptureRequest {
    camera_status_t setContext(void* ctx) {
        context = ctx;
        return ACAMERA_OK;
    }

    camera_status_t getContext(void** ctx) const {
        *ctx = context;
        return ACAMERA_OK;
    }

    sp<ACameraMetadata> settings;
    std::unordered_map<std::string, sp<ACameraMetadata>> physicalSettings;
    ACameraOutputTargets* targets;
    void*                 context;
};

#endif // _ACAPTURE_REQUEST_H
