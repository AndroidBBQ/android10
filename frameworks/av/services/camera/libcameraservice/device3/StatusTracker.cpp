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

#define LOG_TAG "Camera3-Status"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Trace.h>
#include <ui/Fence.h>

#include "device3/StatusTracker.h"
#include "device3/Camera3Device.h"

namespace android {

namespace camera3 {

StatusTracker::StatusTracker(wp<Camera3Device> parent) :
        mComponentsChanged(false),
        mParent(parent),
        mNextComponentId(0),
        mIdleFence(new Fence()),
        mDeviceState(IDLE) {
}

StatusTracker::~StatusTracker() {
}

int StatusTracker::addComponent() {
    int id;
    ssize_t err;
    {
        Mutex::Autolock l(mLock);
        id = mNextComponentId++;
        ALOGV("%s: Adding new component %d", __FUNCTION__, id);

        err = mStates.add(id, IDLE);
        ALOGE_IF(err < 0, "%s: Can't add new component %d: %s (%zd)",
                __FUNCTION__, id, strerror(-err), err);
    }

    if (err >= 0) {
        Mutex::Autolock pl(mPendingLock);
        mComponentsChanged = true;
        mPendingChangeSignal.signal();
    }

    return err < 0 ? err : id;
}

void StatusTracker::removeComponent(int id) {
    ssize_t idx;
    {
        Mutex::Autolock l(mLock);
        ALOGV("%s: Removing component %d", __FUNCTION__, id);
        idx = mStates.removeItem(id);
    }

    if (idx >= 0) {
        Mutex::Autolock pl(mPendingLock);
        mComponentsChanged = true;
        mPendingChangeSignal.signal();
    }

    return;
}


void StatusTracker::markComponentIdle(int id, const sp<Fence>& componentFence) {
    markComponent(id, IDLE, componentFence);
}

void StatusTracker::markComponentActive(int id) {
    markComponent(id, ACTIVE, Fence::NO_FENCE);
}

void StatusTracker::markComponent(int id, ComponentState state,
        const sp<Fence>& componentFence) {
    ALOGV("%s: Component %d is now %s", __FUNCTION__, id,
            state == IDLE ? "idle" : "active");
    Mutex::Autolock l(mPendingLock);

    StateChange newState = {
        id,
        state,
        componentFence
    };

    mPendingChangeQueue.add(newState);
    mPendingChangeSignal.signal();
}

void StatusTracker::requestExit() {
    // First mark thread dead
    Thread::requestExit();
    // Then exit any waits
    mPendingChangeSignal.signal();
}

StatusTracker::ComponentState StatusTracker::getDeviceStateLocked() {
    for (size_t i = 0; i < mStates.size(); i++) {
        if (mStates.valueAt(i) == ACTIVE) {
            ALOGV("%s: Component %d not idle", __FUNCTION__,
                    mStates.keyAt(i));
            return ACTIVE;
        }
    }
    // - If not yet signaled, getSignalTime returns INT64_MAX
    // - If invalid fence or error, returns -1
    // - Otherwise returns time of signalling.
    // Treat -1 as 'signalled', since HAL may not be using fences, and want
    // to be able to idle in case of errors.
    nsecs_t signalTime = mIdleFence->getSignalTime();
    bool fencesDone = signalTime != INT64_MAX;

    ALOGV_IF(!fencesDone, "%s: Fences still to wait on", __FUNCTION__);

    return fencesDone ? IDLE : ACTIVE;
}

bool StatusTracker::threadLoop() {
    status_t res;

    // Wait for state updates
    {
        Mutex::Autolock pl(mPendingLock);
        while (mPendingChangeQueue.size() == 0 && !mComponentsChanged) {
            res = mPendingChangeSignal.waitRelative(mPendingLock,
                    kWaitDuration);
            if (exitPending()) return false;
            if (res != OK) {
                if (res != TIMED_OUT) {
                    ALOGE("%s: Error waiting on state changes: %s (%d)",
                            __FUNCTION__, strerror(-res), res);
                }
                // TIMED_OUT is expected
                break;
            }
        }
    }

    // After new pending states appear, or timeout, check if we're idle.  Even
    // with timeout, need to check to account for fences that may still be
    // clearing out
    sp<Camera3Device> parent;
    {
        Mutex::Autolock pl(mPendingLock);
        Mutex::Autolock l(mLock);

        // Collect all pending state updates and see if the device
        // collectively transitions between idle and active for each one

        // First pass for changed components or fence completions
        ComponentState prevState = getDeviceStateLocked();
        if (prevState != mDeviceState) {
            // Only collect changes to overall device state
            mStateTransitions.add(prevState);
        }
        // For each pending component state update, check if we've transitioned
        // to a new overall device state
        for (size_t i = 0; i < mPendingChangeQueue.size(); i++) {
            const StateChange &newState = mPendingChangeQueue[i];
            ssize_t idx = mStates.indexOfKey(newState.id);
            // Ignore notices for unknown components
            if (idx >= 0) {
                // Update single component state
                mStates.replaceValueAt(idx, newState.state);
                mIdleFence = Fence::merge(String8("idleFence"),
                        mIdleFence, newState.fence);
                // .. and see if overall device state has changed
                ComponentState newState = getDeviceStateLocked();
                if (newState != prevState) {
                    mStateTransitions.add(newState);
                }
                prevState = newState;
            }
        }
        mPendingChangeQueue.clear();
        mComponentsChanged = false;

        // Store final state after all pending state changes are done with

        mDeviceState = prevState;
        parent = mParent.promote();
    }

    // Notify parent for all intermediate transitions
    if (mStateTransitions.size() > 0 && parent.get()) {
        for (size_t i = 0; i < mStateTransitions.size(); i++) {
            bool idle = (mStateTransitions[i] == IDLE);
            ALOGV("Camera device is now %s", idle ? "idle" : "active");
            parent->notifyStatus(idle);
        }
    }
    mStateTransitions.clear();

    return true;
}

} // namespace android

} // namespace camera3
