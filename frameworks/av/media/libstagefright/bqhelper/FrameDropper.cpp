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

//#define LOG_NDEBUG 0
#define LOG_TAG "FrameDropper"
#include <utils/Log.h>

#include <media/stagefright/bqhelper/FrameDropper.h>
#include <media/stagefright/foundation/ADebug.h>

namespace android {

static const int64_t kMaxJitterUs = 2000;

FrameDropper::FrameDropper()
    : mDesiredMinTimeUs(-1),
      mMinIntervalUs(0) {
}

FrameDropper::~FrameDropper() {
}

status_t FrameDropper::setMaxFrameRate(float maxFrameRate) {
    if (maxFrameRate < 0) {
        mMinIntervalUs = -1LL;
        return OK;
    }

    if (maxFrameRate == 0) {
        ALOGE("framerate should be positive but got %f.", maxFrameRate);
        return BAD_VALUE;
    }
    mMinIntervalUs = (int64_t) (1000000.0f / maxFrameRate);
    return OK;
}

bool FrameDropper::shouldDrop(int64_t timeUs) {
    if (mMinIntervalUs <= 0) {
        return false;
    }

    if (mDesiredMinTimeUs < 0) {
        mDesiredMinTimeUs = timeUs + mMinIntervalUs;
        ALOGV("first frame %lld, next desired frame %lld",
                (long long)timeUs, (long long)mDesiredMinTimeUs);
        return false;
    }

    if (timeUs < (mDesiredMinTimeUs - kMaxJitterUs)) {
        ALOGV("drop frame %lld, desired frame %lld, diff %lld",
                (long long)timeUs, (long long)mDesiredMinTimeUs,
                (long long)(mDesiredMinTimeUs - timeUs));
        return true;
    }

    int64_t n = (timeUs - mDesiredMinTimeUs + kMaxJitterUs) / mMinIntervalUs;
    mDesiredMinTimeUs += (n + 1) * mMinIntervalUs;
    ALOGV("keep frame %lld, next desired frame %lld, diff %lld",
            (long long)timeUs, (long long)mDesiredMinTimeUs,
            (long long)(mDesiredMinTimeUs - timeUs));
    return false;
}

}  // namespace android
