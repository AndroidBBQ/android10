/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_H
#define ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_H

#include <stdint.h>
#include <aaudio/AAudio.h>

#include "binding/IAAudioService.h"
#include "binding/AudioEndpointParcelable.h"
#include "binding/AAudioServiceInterface.h"
#include "client/IsochronousClockModel.h"
#include "client/AudioEndpoint.h"
#include "core/AudioStream.h"
#include "utility/AudioClock.h"

using android::sp;
using android::IAAudioService;

namespace aaudio {

    // These are intended to be outside the range of what is normally encountered.
    // TODO MAXes should probably be much bigger.
    constexpr int32_t MIN_FRAMES_PER_BURST = 16; // arbitrary
    constexpr int32_t MAX_FRAMES_PER_BURST = 16 * 1024;  // arbitrary
    constexpr int32_t MAX_BUFFER_CAPACITY_IN_FRAMES = 32 * 1024;  // arbitrary

// A stream that talks to the AAudioService or directly to a HAL.
class AudioStreamInternal : public AudioStream {

public:
    AudioStreamInternal(AAudioServiceInterface  &serviceInterface, bool inService);
    virtual ~AudioStreamInternal();

    aaudio_result_t requestStart() override;

    aaudio_result_t requestStop() override;

    aaudio_result_t getTimestamp(clockid_t clockId,
                                       int64_t *framePosition,
                                       int64_t *timeNanoseconds) override;

    virtual aaudio_result_t updateStateMachine() override;

    aaudio_result_t open(const AudioStreamBuilder &builder) override;

    aaudio_result_t close() override;

    aaudio_result_t setBufferSize(int32_t requestedFrames) override;

    int32_t getBufferSize() const override;

    int32_t getBufferCapacity() const override;

    int32_t getFramesPerBurst() const override;

    int32_t getXRunCount() const override {
        return mXRunCount;
    }

    aaudio_result_t registerThread() override;

    aaudio_result_t unregisterThread() override;

    aaudio_result_t joinThread(void** returnArg);

    // Called internally from 'C'
    virtual void *callbackLoop() = 0;


    bool isMMap() override {
        return true;
    }

    // Calculate timeout based on framesPerBurst
    int64_t calculateReasonableTimeout();

    aaudio_result_t startClient(const android::AudioClient& client,
                                audio_port_handle_t *clientHandle);

    aaudio_result_t stopClient(audio_port_handle_t clientHandle);

    aaudio_handle_t getServiceHandle() const {
        return mServiceStreamHandle;
    }

protected:

    aaudio_result_t processData(void *buffer,
                         int32_t numFrames,
                         int64_t timeoutNanoseconds);

/**
 * Low level data processing that will not block. It will just read or write as much as it can.
 *
 * It passed back a recommended time to wake up if wakeTimePtr is not NULL.
 *
 * @return the number of frames processed or a negative error code.
 */
    virtual aaudio_result_t processDataNow(void *buffer,
                            int32_t numFrames,
                            int64_t currentTimeNanos,
                            int64_t *wakeTimePtr) = 0;

    aaudio_result_t drainTimestampsFromService();

    aaudio_result_t processCommands();

    aaudio_result_t stopCallback();

    virtual void advanceClientToMatchServerPosition() = 0;

    virtual void onFlushFromServer() {}

    aaudio_result_t onEventFromServer(AAudioServiceMessage *message);

    aaudio_result_t onTimestampService(AAudioServiceMessage *message);

    aaudio_result_t onTimestampHardware(AAudioServiceMessage *message);

    void logTimestamp(AAudioServiceMessage &message);

    // Calculate timeout for an operation involving framesPerOperation.
    int64_t calculateReasonableTimeout(int32_t framesPerOperation);

    int32_t getDeviceChannelCount() const { return mDeviceChannelCount; }

    /**
     * @return true if running in audio service, versus in app process
     */
    bool isInService() const { return mInService; }

    /**
     * Is the service FIFO position currently controlled by the AAudio service or HAL,
     * or set based on the Clock Model.
     *
     * @return true if the ClockModel is currently determining the FIFO position
     */
    bool isClockModelInControl() const;

    IsochronousClockModel    mClockModel;      // timing model for chasing the HAL

    AudioEndpoint            mAudioEndpoint;   // source for reads or sink for writes
    aaudio_handle_t          mServiceStreamHandle; // opaque handle returned from service

    int32_t                  mFramesPerBurst = MIN_FRAMES_PER_BURST; // frames per HAL transfer
    int32_t                  mXRunCount = 0;      // how many underrun events?

    // Offset from underlying frame position.
    int64_t                  mFramesOffsetFromService = 0; // offset for timestamps

    uint8_t                 *mCallbackBuffer = nullptr;
    int32_t                  mCallbackFrames = 0;

    // The service uses this for SHARED mode.
    bool                     mInService = false;  // Is this running in the client or the service?

    AAudioServiceInterface  &mServiceInterface;   // abstract interface to the service

    SimpleDoubleBuffer<Timestamp>  mAtomicInternalTimestamp;

    AtomicRequestor          mNeedCatchUp;   // Ask read() or write() to sync on first timestamp.

    float                    mStreamVolume = 1.0f;

private:
    /*
     * Asynchronous write with data conversion.
     * @param buffer
     * @param numFrames
     * @return fdrames written or negative error
     */
    aaudio_result_t writeNowWithConversion(const void *buffer,
                                     int32_t numFrames);

    // Adjust timing model based on timestamp from service.
    void processTimestamp(uint64_t position, int64_t time);

    // Thread on other side of FIFO will have wakeup jitter.
    // By delaying slightly we can avoid waking up before other side is ready.
    const int32_t            mWakeupDelayNanos; // delay past typical wakeup jitter
    const int32_t            mMinimumSleepNanos; // minimum sleep while polling

    AudioEndpointParcelable  mEndPointParcelable; // description of the buffers filled by service
    EndpointDescriptor       mEndpointDescriptor; // buffer description with resolved addresses

    int64_t                  mServiceLatencyNanos = 0;

    // Sometimes the hardware is operating with a different channel count from the app.
    // Then we require conversion in AAudio.
    int32_t                  mDeviceChannelCount = 0;
};

} /* namespace aaudio */

#endif //ANDROID_AAUDIO_AUDIO_STREAM_INTERNAL_H
