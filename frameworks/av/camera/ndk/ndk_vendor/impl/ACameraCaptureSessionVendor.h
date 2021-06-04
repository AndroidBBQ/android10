/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <string>
#include "utils.h"

struct ACaptureSessionOutput {
    explicit ACaptureSessionOutput(native_handle_t* window, bool isShared = false,
            const char* physicalCameraId = "") :
            mWindow(window), mIsShared(isShared), mPhysicalCameraId(physicalCameraId) {};

    bool operator == (const ACaptureSessionOutput& other) const {
        return (mWindow == other.mWindow);
    }

    bool operator != (const ACaptureSessionOutput& other) const {
        return mWindow != other.mWindow;
    }

    bool operator < (const ACaptureSessionOutput& other) const {
        return mWindow < other.mWindow;
    }

    bool operator > (const ACaptureSessionOutput& other) const {
        return mWindow > other.mWindow;
    }

    android::acam::utils::native_handle_ptr_wrapper mWindow;
    std::set<android::acam::utils::native_handle_ptr_wrapper> mSharedWindows;
    bool           mIsShared;
    int            mRotation = CAMERA3_STREAM_ROTATION_0;
    std::string mPhysicalCameraId;
};


