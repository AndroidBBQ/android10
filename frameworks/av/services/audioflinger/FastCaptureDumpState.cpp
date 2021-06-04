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

#define LOG_TAG "FastCaptureDumpState"
//define LOG_NDEBUG 0

#include "Configuration.h"
#include <utils/Log.h>
#include "FastCaptureDumpState.h"
#include "FastCaptureState.h"

namespace android {

FastCaptureDumpState::FastCaptureDumpState() : FastThreadDumpState(),
    mReadSequence(0), mFramesRead(0), mReadErrors(0), mSampleRate(0), mFrameCount(0)
{
}

FastCaptureDumpState::~FastCaptureDumpState()
{
}

void FastCaptureDumpState::dump(int fd) const
{
    if (mCommand == FastCaptureState::INITIAL) {
        dprintf(fd, "  FastCapture not initialized\n");
        return;
    }
    double measuredWarmupMs = (mMeasuredWarmupTs.tv_sec * 1000.0) +
            (mMeasuredWarmupTs.tv_nsec / 1000000.0);
    double periodSec = (double) mFrameCount / mSampleRate;
    dprintf(fd, "  FastCapture command=%s readSequence=%u framesRead=%u\n"
                "              readErrors=%u sampleRate=%u frameCount=%zu\n"
                "              measuredWarmup=%.3g ms, warmupCycles=%u period=%.2f ms\n",
                FastCaptureState::commandToString(mCommand), mReadSequence, mFramesRead,
                mReadErrors, mSampleRate, mFrameCount, measuredWarmupMs, mWarmupCycles,
                periodSec * 1e3);
}

}   // android
