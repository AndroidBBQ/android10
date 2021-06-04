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

#ifndef OMX_2_IGRAPHICBUFFERSOURCE_H_
#define OMX_2_IGRAPHICBUFFERSOURCE_H_

#include <android/BnGraphicBufferSource.h>
#include <media/stagefright/omx/OmxGraphicBufferSource.h>

namespace android {

using BnStatus = ::android::binder::Status;

struct Omx2IGraphicBufferSource : public BnGraphicBufferSource {
    sp<OmxGraphicBufferSource> mBase;
    sp<IOMXBufferSource> mOMXBufferSource;
    Omx2IGraphicBufferSource(sp<OmxGraphicBufferSource> const& base);
    BnStatus configure(const sp<IOMXNode>& omxNode, int32_t dataSpace) override;
    BnStatus setSuspend(bool suspend, int64_t timeUs) override;
    BnStatus setRepeatPreviousFrameDelayUs(int64_t repeatAfterUs) override;
    BnStatus setMaxFps(float maxFps) override;
    BnStatus setTimeLapseConfig(double fps, double captureFps) override;
    BnStatus setStartTimeUs(int64_t startTimeUs) override;
    BnStatus setStopTimeUs(int64_t stopTimeUs) override;
    BnStatus getStopTimeOffsetUs(int64_t *stopTimeOffsetUs) override;
    BnStatus setColorAspects(int32_t aspects) override;
    BnStatus setTimeOffsetUs(int64_t timeOffsetsUs) override;
    BnStatus signalEndOfInputStream() override;
};

} // namespace android

#endif // OMX_2_IGRAPHICBUFFERSOURCE_H_

