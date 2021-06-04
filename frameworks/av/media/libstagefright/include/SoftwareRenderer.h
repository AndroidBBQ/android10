/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef SOFTWARE_RENDERER_H_

#define SOFTWARE_RENDERER_H_

#include <media/stagefright/ColorConverter.h>
#include <media/stagefright/FrameRenderTracker.h>
#include <utils/RefBase.h>
#include <system/window.h>
#include <media/hardware/VideoAPI.h>

#include <list>

namespace android {

struct AMessage;

class SoftwareRenderer {
public:
    explicit SoftwareRenderer(
            const sp<ANativeWindow> &nativeWindow, int32_t rotation = 0);

    ~SoftwareRenderer();

    std::list<FrameRenderTracker::Info> render(
            const void *data, size_t size, int64_t mediaTimeUs, nsecs_t renderTimeNs,
            size_t numOutputBuffers, const sp<AMessage> &format);
    void clearTracker();

private:
    enum YUVMode {
        None,
    };

    OMX_COLOR_FORMATTYPE mColorFormat;
    ColorConverter *mConverter;
    YUVMode mYUVMode;
    sp<ANativeWindow> mNativeWindow;
    int32_t mWidth, mHeight, mStride;
    int32_t mCropLeft, mCropTop, mCropRight, mCropBottom;
    int32_t mCropWidth, mCropHeight;
    int32_t mRotationDegrees;
    android_dataspace mDataSpace;
    HDRStaticInfo mHDRStaticInfo;
    FrameRenderTracker mRenderTracker;

    void resetFormatIfChanged(
            const sp<AMessage> &format, size_t numOutputBuffers);

    SoftwareRenderer(const SoftwareRenderer &);
    SoftwareRenderer &operator=(const SoftwareRenderer &);
};

}  // namespace android

#endif  // SOFTWARE_RENDERER_H_
