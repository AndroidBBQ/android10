/*
 * Copyright (C) 2012 The Android Open Source Project
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

#pragma once

#include <stddef.h>

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>

#include <ui/FenceTime.h>

#include <memory>

namespace android {

class FenceTime;

class DispSync {
public:
    class Callback {
    public:
        Callback() = default;
        virtual ~Callback();
        virtual void onDispSyncEvent(nsecs_t when) = 0;

    protected:
        Callback(Callback const&) = delete;
        Callback& operator=(Callback const&) = delete;
    };

    DispSync() = default;
    virtual ~DispSync();

    virtual void reset() = 0;
    virtual bool addPresentFence(const std::shared_ptr<FenceTime>&) = 0;
    virtual void beginResync() = 0;
    virtual bool addResyncSample(nsecs_t timestamp, bool* periodFlushed) = 0;
    virtual void endResync() = 0;
    virtual void setPeriod(nsecs_t period) = 0;
    virtual nsecs_t getPeriod() = 0;
    virtual void setRefreshSkipCount(int count) = 0;
    virtual status_t addEventListener(const char* name, nsecs_t phase, Callback* callback,
                                      nsecs_t lastCallbackTime) = 0;
    virtual status_t removeEventListener(Callback* callback, nsecs_t* outLastCallback) = 0;
    virtual status_t changePhaseOffset(Callback* callback, nsecs_t phase) = 0;
    virtual nsecs_t computeNextRefresh(int periodOffset) const = 0;
    virtual void setIgnorePresentFences(bool ignore) = 0;
    virtual nsecs_t expectedPresentTime() = 0;

    virtual void dump(std::string& result) const = 0;

protected:
    DispSync(DispSync const&) = delete;
    DispSync& operator=(DispSync const&) = delete;
};

namespace impl {

class DispSyncThread;

// DispSync maintains a model of the periodic hardware-based vsync events of a
// display and uses that model to execute period callbacks at specific phase
// offsets from the hardware vsync events.  The model is constructed by
// feeding consecutive hardware event timestamps to the DispSync object via
// the addResyncSample method.
//
// The model is validated using timestamps from Fence objects that are passed
// to the DispSync object via the addPresentFence method.  These fence
// timestamps should correspond to a hardware vsync event, but they need not
// be consecutive hardware vsync times.  If this method determines that the
// current model accurately represents the hardware event times it will return
// false to indicate that a resynchronization (via addResyncSample) is not
// needed.
class DispSync : public android::DispSync {
public:
    explicit DispSync(const char* name);
    ~DispSync() override;

    void init(bool hasSyncFramework, int64_t dispSyncPresentTimeOffset);

    // reset clears the resync samples and error value.
    void reset() override;

    // addPresentFence adds a fence for use in validating the current vsync
    // event model.  The fence need not be signaled at the time
    // addPresentFence is called.  When the fence does signal, its timestamp
    // should correspond to a hardware vsync event.  Unlike the
    // addResyncSample method, the timestamps of consecutive fences need not
    // correspond to consecutive hardware vsync events.
    //
    // This method should be called with the retire fence from each HWComposer
    // set call that affects the display.
    bool addPresentFence(const std::shared_ptr<FenceTime>& fenceTime) override;

    // The beginResync, addResyncSample, and endResync methods are used to re-
    // synchronize the DispSync's model to the hardware vsync events.  The re-
    // synchronization process involves first calling beginResync, then
    // calling addResyncSample with a sequence of consecutive hardware vsync
    // event timestamps, and finally calling endResync when addResyncSample
    // indicates that no more samples are needed by returning false.
    //
    // This resynchronization process should be performed whenever the display
    // is turned on (i.e. once immediately after it's turned on) and whenever
    // addPresentFence returns true indicating that the model has drifted away
    // from the hardware vsync events.
    void beginResync() override;
    // Adds a vsync sample to the dispsync model. The timestamp is the time
    // of the vsync event that fired. periodFlushed will return true if the
    // vsync period was detected to have changed to mPendingPeriod.
    //
    // This method will return true if more vsync samples are needed to lock
    // down the DispSync model, and false otherwise.
    // periodFlushed will be set to true if mPendingPeriod is flushed to
    // mIntendedPeriod, and false otherwise.
    bool addResyncSample(nsecs_t timestamp, bool* periodFlushed) override;
    void endResync() override;

    // The setPeriod method sets the vsync event model's period to a specific
    // value. This should be used to prime the model when a display is first
    // turned on, or when a refresh rate change is requested.
    void setPeriod(nsecs_t period) override;

    // The getPeriod method returns the current vsync period.
    nsecs_t getPeriod() override;

    // setRefreshSkipCount specifies an additional number of refresh
    // cycles to skip.  For example, on a 60Hz display, a skip count of 1
    // will result in events happening at 30Hz.  Default is zero.  The idea
    // is to sacrifice smoothness for battery life.
    void setRefreshSkipCount(int count) override;

    // addEventListener registers a callback to be called repeatedly at the
    // given phase offset from the hardware vsync events.  The callback is
    // called from a separate thread and it should return reasonably quickly
    // (i.e. within a few hundred microseconds).
    // If the callback was previously registered, and the last clock time the
    // callback was invoked was known to the caller (e.g. via removeEventListener),
    // then the caller may pass that through to lastCallbackTime, so that
    // callbacks do not accidentally double-fire if they are unregistered and
    // reregistered in rapid succession.
    status_t addEventListener(const char* name, nsecs_t phase, Callback* callback,
                              nsecs_t lastCallbackTime) override;

    // removeEventListener removes an already-registered event callback.  Once
    // this method returns that callback will no longer be called by the
    // DispSync object.
    // outLastCallbackTime will contain the last time that the callback was invoked.
    // If the caller wishes to reregister the same callback, they should pass the
    // callback time back into lastCallbackTime (see addEventListener).
    status_t removeEventListener(Callback* callback, nsecs_t* outLastCallbackTime) override;

    // changePhaseOffset changes the phase offset of an already-registered event callback. The
    // method will make sure that there is no skipping or double-firing on the listener per frame,
    // even when changing the offsets multiple times.
    status_t changePhaseOffset(Callback* callback, nsecs_t phase) override;

    // computeNextRefresh computes when the next refresh is expected to begin.
    // The periodOffset value can be used to move forward or backward; an
    // offset of zero is the next refresh, -1 is the previous refresh, 1 is
    // the refresh after next. etc.
    nsecs_t computeNextRefresh(int periodOffset) const override;

    // In certain situations the present fences aren't a good indicator of vsync
    // time, e.g. when vr flinger is active, or simply aren't available,
    // e.g. when the sync framework isn't present. Use this method to toggle
    // whether or not DispSync ignores present fences. If present fences are
    // ignored, DispSync will always ask for hardware vsync events by returning
    // true from addPresentFence() and addResyncSample().
    void setIgnorePresentFences(bool ignore) override;

    // Determine the expected present time when a buffer acquired now will be displayed.
    nsecs_t expectedPresentTime();

    // dump appends human-readable debug info to the result string.
    void dump(std::string& result) const override;

private:
    void updateModelLocked();
    void updateErrorLocked();
    void resetLocked();
    void resetErrorLocked();

    enum { MAX_RESYNC_SAMPLES = 32 };
    enum { MIN_RESYNC_SAMPLES_FOR_UPDATE = 6 };
    enum { NUM_PRESENT_SAMPLES = 8 };
    enum { MAX_RESYNC_SAMPLES_WITHOUT_PRESENT = 4 };
    enum { ACCEPTABLE_ZERO_ERR_SAMPLES_COUNT = 64 };

    const char* const mName;

    // mPeriod is the computed period of the modeled vsync events in
    // nanoseconds.
    nsecs_t mPeriod;

    // mIntendedPeriod is the intended period of the modeled vsync events in
    // nanoseconds. Under ideal conditions this should be similar if not the
    // same as mPeriod, plus or minus an observed error.
    nsecs_t mIntendedPeriod = 0;

    // mPendingPeriod is the proposed period change in nanoseconds.
    // If mPendingPeriod differs from mPeriod and is nonzero, it will
    // be flushed to mPeriod when we detect that the hardware switched
    // vsync frequency.
    nsecs_t mPendingPeriod = 0;

    // mPhase is the phase offset of the modeled vsync events.  It is the
    // number of nanoseconds from time 0 to the first vsync event.
    nsecs_t mPhase;

    // mReferenceTime is the reference time of the modeled vsync events.
    // It is the nanosecond timestamp of the first vsync event after a resync.
    nsecs_t mReferenceTime;

    // mError is the computed model error.  It is based on the difference
    // between the estimated vsync event times and those observed in the
    // mPresentFences array.
    nsecs_t mError;

    // mZeroErrSamplesCount keeps track of how many times in a row there were
    // zero timestamps available in the mPresentFences array.
    // Used to sanity check that we are able to calculate the model error.
    size_t mZeroErrSamplesCount;

    // Whether we have updated the vsync event model since the last resync.
    bool mModelUpdated;

    // These member variables are the state used during the resynchronization
    // process to store information about the hardware vsync event times used
    // to compute the model.
    nsecs_t mResyncSamples[MAX_RESYNC_SAMPLES] = {0};
    size_t mFirstResyncSample = 0;
    size_t mNumResyncSamples = 0;
    int mNumResyncSamplesSincePresent;

    // These member variables store information about the present fences used
    // to validate the currently computed model.
    std::shared_ptr<FenceTime> mPresentFences[NUM_PRESENT_SAMPLES]{FenceTime::NO_FENCE};
    size_t mPresentSampleOffset;

    int mRefreshSkipCount;

    // mThread is the thread from which all the callbacks are called.
    sp<DispSyncThread> mThread;

    // mMutex is used to protect access to all member variables.
    mutable Mutex mMutex;

    // This is the offset from the present fence timestamps to the corresponding
    // vsync event.
    int64_t mPresentTimeOffset;

    // Ignore present (retire) fences if the device doesn't have support for the
    // sync framework
    bool mIgnorePresentFences;

    std::unique_ptr<Callback> mZeroPhaseTracer;

    // Flag to turn on logging in systrace.
    bool mTraceDetailedInfo = false;
};

} // namespace impl

} // namespace android
