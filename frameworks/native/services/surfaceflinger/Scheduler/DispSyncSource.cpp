/*
 * Copyright 2018 The Android Open Source Project
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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "DispSyncSource.h"

#include <android-base/stringprintf.h>
#include <utils/Trace.h>
#include <mutex>

#include "DispSync.h"
#include "EventThread.h"

namespace android {

DispSyncSource::DispSyncSource(DispSync* dispSync, nsecs_t phaseOffset,
                               nsecs_t offsetThresholdForNextVsync, bool traceVsync,
                               const char* name)
      : mName(name),
        mTraceVsync(traceVsync),
        mVsyncOnLabel(base::StringPrintf("VsyncOn-%s", name)),
        mVsyncEventLabel(base::StringPrintf("VSYNC-%s", name)),
        mVsyncOffsetLabel(base::StringPrintf("VsyncOffset-%s", name)),
        mVsyncNegativeOffsetLabel(base::StringPrintf("VsyncNegativeOffset-%s", name)),
        mDispSync(dispSync),
        mPhaseOffset(phaseOffset),
        mOffsetThresholdForNextVsync(offsetThresholdForNextVsync) {}

void DispSyncSource::setVSyncEnabled(bool enable) {
    std::lock_guard lock(mVsyncMutex);
    if (enable) {
        tracePhaseOffset();
        status_t err = mDispSync->addEventListener(mName, mPhaseOffset,
                                                   static_cast<DispSync::Callback*>(this),
                                                   mLastCallbackTime);
        if (err != NO_ERROR) {
            ALOGE("error registering vsync callback: %s (%d)", strerror(-err), err);
        }
        // ATRACE_INT(mVsyncOnLabel.c_str(), 1);
    } else {
        status_t err = mDispSync->removeEventListener(static_cast<DispSync::Callback*>(this),
                                                      &mLastCallbackTime);
        if (err != NO_ERROR) {
            ALOGE("error unregistering vsync callback: %s (%d)", strerror(-err), err);
        }
        // ATRACE_INT(mVsyncOnLabel.c_str(), 0);
    }
    mEnabled = enable;
}

void DispSyncSource::setCallback(VSyncSource::Callback* callback) {
    std::lock_guard lock(mCallbackMutex);
    mCallback = callback;
}

void DispSyncSource::setPhaseOffset(nsecs_t phaseOffset) {
    std::lock_guard lock(mVsyncMutex);
    const nsecs_t period = mDispSync->getPeriod();
    // Check if offset should be handled as negative
    if (phaseOffset >= mOffsetThresholdForNextVsync) {
        phaseOffset -= period;
    }

    // Normalize phaseOffset to [-period, period)
    const int numPeriods = phaseOffset / period;
    phaseOffset -= numPeriods * period;
    if (mPhaseOffset == phaseOffset) {
        return;
    }

    mPhaseOffset = phaseOffset;
    tracePhaseOffset();

    // If we're not enabled, we don't need to mess with the listeners
    if (!mEnabled) {
        return;
    }

    status_t err =
            mDispSync->changePhaseOffset(static_cast<DispSync::Callback*>(this), mPhaseOffset);
    if (err != NO_ERROR) {
        ALOGE("error changing vsync offset: %s (%d)", strerror(-err), err);
    }
}

void DispSyncSource::onDispSyncEvent(nsecs_t when) {
    VSyncSource::Callback* callback;
    {
        std::lock_guard lock(mCallbackMutex);
        callback = mCallback;
    }

    if (mTraceVsync) {
        mValue = (mValue + 1) % 2;
        ATRACE_INT(mVsyncEventLabel.c_str(), mValue);
    }

    if (callback != nullptr) {
        callback->onVSyncEvent(when);
    }
}

void DispSyncSource::tracePhaseOffset() {
    if (mPhaseOffset > 0) {
        ATRACE_INT(mVsyncOffsetLabel.c_str(), mPhaseOffset);
        ATRACE_INT(mVsyncNegativeOffsetLabel.c_str(), 0);
    } else {
        ATRACE_INT(mVsyncOffsetLabel.c_str(), 0);
        ATRACE_INT(mVsyncNegativeOffsetLabel.c_str(), -mPhaseOffset);
    }
}

} // namespace android
