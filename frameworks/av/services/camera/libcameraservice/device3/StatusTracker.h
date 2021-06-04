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

#ifndef ANDROID_SERVERS_CAMERA3_STATUSTRACKER_H
#define ANDROID_SERVERS_CAMERA3_STATUSTRACKER_H

#include <utils/Condition.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Thread.h>
#include <utils/KeyedVector.h>
#include <hardware/camera3.h>

#include "common/CameraDeviceBase.h"

namespace android {

class Camera3Device;
class Fence;

namespace camera3 {

/**
 * State tracking for idle and other collective state transitions.
 * Collects idle notifications from different sources and calls the
 * parent when all of them become idle.
 *
 * The parent is responsible for synchronizing the status updates with its
 * internal state correctly, which means the notifyStatus call to the parent may
 * block for a while.
 */
class StatusTracker: public Thread {
  public:
    explicit StatusTracker(wp<Camera3Device> parent);
    ~StatusTracker();

    // An always-invalid component ID
    static const int NO_STATUS_ID = -1;

    // Add a component to track; returns non-negative unique ID for the new
    // component on success, negative error code on failure.
    // New components start in the idle state.
    int addComponent();

    // Remove existing component from idle tracking. Ignores unknown IDs
    void removeComponent(int id);

    // Set the state of a tracked component to be idle. Ignores unknown IDs; can
    // accept a fence to wait on to complete idle.  The fence is merged with any
    // previous fences given, which means they all must signal before the
    // component is considered idle.
    void markComponentIdle(int id, const sp<Fence>& componentFence);

    // Set the state of a tracked component to be active. Ignores unknown IDs.
    void markComponentActive(int id);

    virtual void requestExit();
  protected:

    virtual bool threadLoop();

  private:
    enum ComponentState {
        IDLE,
        ACTIVE
    };

    void markComponent(int id, ComponentState state,
            const sp<Fence>& componentFence);

    // Guards mPendingChange, mPendingStates, mComponentsChanged
    Mutex mPendingLock;

    Condition mPendingChangeSignal;

    struct StateChange {
        int id;
        ComponentState state;
        sp<Fence> fence;
    };
    // A queue of yet-to-be-processed state changes to components
    Vector<StateChange> mPendingChangeQueue;
    bool mComponentsChanged;

    wp<Camera3Device> mParent;

    // Guards rest of internals. Must be locked after mPendingLock if both used.
    Mutex mLock;

    int mNextComponentId;

    // Current component states
    KeyedVector<int, ComponentState> mStates;
    // Merged fence for all processed state changes
    sp<Fence> mIdleFence;
    // Current overall device state
    ComponentState mDeviceState;

    // Private to threadLoop

    // Determine current overall device state
    // We're IDLE iff
    // - All components are currently IDLE
    // - The merged fence for all component updates has signalled
    ComponentState getDeviceStateLocked();

    Vector<ComponentState> mStateTransitions;

    static const nsecs_t kWaitDuration = 250000000LL; // 250 ms
};

} // namespace camera3

} // namespace android

#endif
