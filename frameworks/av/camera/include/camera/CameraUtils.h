/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ANDROID_CAMERA_CLIENT_CAMERAUTILS_H
#define ANDROID_CAMERA_CLIENT_CAMERAUTILS_H

#include <binder/IMemory.h>
#include <camera/CameraMetadata.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <stdint.h>

namespace android {

/**
 * CameraUtils contains utility methods that are shared between the native
 * camera client, and the camera service.
 */
class CameraUtils {
    public:
        /**
         * Calculate the ANativeWindow transform from the static camera
         * metadata.  This is based on the sensor orientation and lens facing
         * attributes of the camera device.
         *
         * Returns OK on success, or a negative error code.
         */
        static status_t getRotationTransform(const CameraMetadata& staticInfo,
                /*out*/int32_t* transform);

        /**
         * Check if the image data is VideoNativeHandleMetadata, that contains a native handle.
         */
        static bool isNativeHandleMetadata(const sp<IMemory>& imageData);

    private:
        CameraUtils();
};

} /* namespace android */

#endif /* ANDROID_CAMERA_CLIENT_CAMERAUTILS_H */

