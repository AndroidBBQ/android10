/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA3_STREAMBUFFERLISTENER_H
#define ANDROID_SERVERS_CAMERA3_STREAMBUFFERLISTENER_H

#include <camera/CameraMetadata.h>
#include <gui/Surface.h>
#include <utils/RefBase.h>

namespace android {

namespace camera3 {

class Camera3StreamBufferListener : public virtual RefBase {
public:

    struct BufferInfo {
        bool mOutput; // if false then input buffer
        Rect mCrop;
        uint32_t mTransform;
        uint32_t mScalingMode;
        int64_t mTimestamp;
        uint64_t mFrameNumber;
        bool mError;
    };

    // Buffer was acquired by the HAL
    virtual void onBufferAcquired(const BufferInfo& bufferInfo) = 0;
    // Buffer was released by the HAL
    virtual void onBufferReleased(const BufferInfo& bufferInfo) = 0;
    // Notify about incoming buffer request frame number
    virtual void onBufferRequestForFrameNumber(uint64_t frameNumber, int streamId,
            const CameraMetadata& settings) = 0;
};

}; //namespace camera3
}; //namespace android

#endif
