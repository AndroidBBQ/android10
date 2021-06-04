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

#include <audio_utils/roundup.h>
#include "FastThreadDumpState.h"

namespace android {

FastThreadDumpState::FastThreadDumpState() :
    mCommand(FastThreadState::INITIAL), mUnderruns(0), mOverruns(0),
    /* mMeasuredWarmupTs({0, 0}), */
    mWarmupCycles(0)
#ifdef FAST_THREAD_STATISTICS
    , mSamplingN(0), mBounds(0)
#endif
{
    mMeasuredWarmupTs.tv_sec = 0;
    mMeasuredWarmupTs.tv_nsec = 0;
#ifdef FAST_THREAD_STATISTICS
    increaseSamplingN(1);
#endif
}

FastThreadDumpState::~FastThreadDumpState()
{
}

#ifdef FAST_THREAD_STATISTICS
void FastThreadDumpState::increaseSamplingN(uint32_t samplingN)
{
    if (samplingN <= mSamplingN || samplingN > kSamplingN || roundup(samplingN) != samplingN) {
        return;
    }
    uint32_t additional = samplingN - mSamplingN;
    // sample arrays aren't accessed atomically with respect to the bounds,
    // so clearing reduces chance for dumpsys to read random uninitialized samples
    memset(&mMonotonicNs[mSamplingN], 0, sizeof(mMonotonicNs[0]) * additional);
    memset(&mLoadNs[mSamplingN], 0, sizeof(mLoadNs[0]) * additional);
#ifdef CPU_FREQUENCY_STATISTICS
    memset(&mCpukHz[mSamplingN], 0, sizeof(mCpukHz[0]) * additional);
#endif
    mSamplingN = samplingN;
}
#endif

}   // android
