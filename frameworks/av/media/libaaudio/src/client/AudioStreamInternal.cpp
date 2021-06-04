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

// This file is used in both client and server processes.
// This is needed to make sense of the logs more easily.
#define LOG_TAG (mInService ? "AudioStreamInternal_Service" : "AudioStreamInternal_Client")
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include <stdint.h>

#include <binder/IServiceManager.h>

#include <aaudio/AAudio.h>
#include <cutils/properties.h>
#include <utils/String16.h>
#include <utils/Trace.h>

#include "AudioEndpointParcelable.h"
#include "binding/AAudioStreamRequest.h"
#include "binding/AAudioStreamConfiguration.h"
#include "binding/IAAudioService.h"
#include "binding/AAudioServiceMessage.h"
#include "core/AudioStreamBuilder.h"
#include "fifo/FifoBuffer.h"
#include "utility/AudioClock.h"

#include "AudioStreamInternal.h"

using android::String16;
using android::Mutex;
using android::WrappingBuffer;

using namespace aaudio;

#define MIN_TIMEOUT_NANOS        (1000 * AAUDIO_NANOS_PER_MILLISECOND)

// Wait at least this many times longer than the operation should take.
#define MIN_TIMEOUT_OPERATIONS    4

#define LOG_TIMESTAMPS            0

AudioStreamInternal::AudioStreamInternal(AAudioServiceInterface  &serviceInterface, bool inService)
        : AudioStream()
        , mClockModel()
        , mAudioEndpoint()
        , mServiceStreamHandle(AAUDIO_HANDLE_INVALID)
        , mInService(inService)
        , mServiceInterface(serviceInterface)
        , mAtomicInternalTimestamp()
        , mWakeupDelayNanos(AAudioProperty_getWakeupDelayMicros() * AAUDIO_NANOS_PER_MICROSECOND)
        , mMinimumSleepNanos(AAudioProperty_getMinimumSleepMicros() * AAUDIO_NANOS_PER_MICROSECOND)
        {
}

AudioStreamInternal::~AudioStreamInternal() {
}

aaudio_result_t AudioStreamInternal::open(const AudioStreamBuilder &builder) {

    aaudio_result_t result = AAUDIO_OK;
    int32_t capacity;
    int32_t framesPerBurst;
    int32_t framesPerHardwareBurst;
    AAudioStreamRequest request;
    AAudioStreamConfiguration configurationOutput;

    if (getState() != AAUDIO_STREAM_STATE_UNINITIALIZED) {
        ALOGE("%s - already open! state = %d", __func__, getState());
        return AAUDIO_ERROR_INVALID_STATE;
    }

    // Copy requested parameters to the stream.
    result = AudioStream::open(builder);
    if (result < 0) {
        return result;
    }

    const int32_t burstMinMicros = AAudioProperty_getHardwareBurstMinMicros();
    int32_t burstMicros = 0;

    // We have to do volume scaling. So we prefer FLOAT format.
    if (getFormat() == AUDIO_FORMAT_DEFAULT) {
        setFormat(AUDIO_FORMAT_PCM_FLOAT);
    }
    // Request FLOAT for the shared mixer.
    request.getConfiguration().setFormat(AUDIO_FORMAT_PCM_FLOAT);

    // Build the request to send to the server.
    request.setUserId(getuid());
    request.setProcessId(getpid());
    request.setSharingModeMatchRequired(isSharingModeMatchRequired());
    request.setInService(isInService());

    request.getConfiguration().setDeviceId(getDeviceId());
    request.getConfiguration().setSampleRate(getSampleRate());
    request.getConfiguration().setSamplesPerFrame(getSamplesPerFrame());
    request.getConfiguration().setDirection(getDirection());
    request.getConfiguration().setSharingMode(getSharingMode());

    request.getConfiguration().setUsage(getUsage());
    request.getConfiguration().setContentType(getContentType());
    request.getConfiguration().setInputPreset(getInputPreset());

    request.getConfiguration().setBufferCapacity(builder.getBufferCapacity());

    mDeviceChannelCount = getSamplesPerFrame(); // Assume it will be the same. Update if not.

    mServiceStreamHandle = mServiceInterface.openStream(request, configurationOutput);
    if (mServiceStreamHandle < 0
            && request.getConfiguration().getSamplesPerFrame() == 1 // mono?
            && getDirection() == AAUDIO_DIRECTION_OUTPUT
            && !isInService()) {
        // if that failed then try switching from mono to stereo if OUTPUT.
        // Only do this in the client. Otherwise we end up with a mono mixer in the service
        // that writes to a stereo MMAP stream.
        ALOGD("%s() - openStream() returned %d, try switching from MONO to STEREO",
              __func__, mServiceStreamHandle);
        request.getConfiguration().setSamplesPerFrame(2); // stereo
        mServiceStreamHandle = mServiceInterface.openStream(request, configurationOutput);
    }
    if (mServiceStreamHandle < 0) {
        return mServiceStreamHandle;
    }

    result = configurationOutput.validate();
    if (result != AAUDIO_OK) {
        goto error;
    }
    // Save results of the open.
    if (getSamplesPerFrame() == AAUDIO_UNSPECIFIED) {
        setSamplesPerFrame(configurationOutput.getSamplesPerFrame());
    }
    mDeviceChannelCount = configurationOutput.getSamplesPerFrame();

    setSampleRate(configurationOutput.getSampleRate());
    setDeviceId(configurationOutput.getDeviceId());
    setSessionId(configurationOutput.getSessionId());
    setSharingMode(configurationOutput.getSharingMode());

    setUsage(configurationOutput.getUsage());
    setContentType(configurationOutput.getContentType());
    setInputPreset(configurationOutput.getInputPreset());

    // Save device format so we can do format conversion and volume scaling together.
    setDeviceFormat(configurationOutput.getFormat());

    result = mServiceInterface.getStreamDescription(mServiceStreamHandle, mEndPointParcelable);
    if (result != AAUDIO_OK) {
        goto error;
    }

    // Resolve parcelable into a descriptor.
    result = mEndPointParcelable.resolve(&mEndpointDescriptor);
    if (result != AAUDIO_OK) {
        goto error;
    }

    // Configure endpoint based on descriptor.
    result = mAudioEndpoint.configure(&mEndpointDescriptor, getDirection());
    if (result != AAUDIO_OK) {
        goto error;
    }

    framesPerHardwareBurst = mEndpointDescriptor.dataQueueDescriptor.framesPerBurst;

    // Scale up the burst size to meet the minimum equivalent in microseconds.
    // This is to avoid waking the CPU too often when the HW burst is very small
    // or at high sample rates.
    framesPerBurst = framesPerHardwareBurst;
    do {
        if (burstMicros > 0) {  // skip first loop
            framesPerBurst *= 2;
        }
        burstMicros = framesPerBurst * static_cast<int64_t>(1000000) / getSampleRate();
    } while (burstMicros < burstMinMicros);
    ALOGD("%s() original HW burst = %d, minMicros = %d => SW burst = %d\n",
          __func__, framesPerHardwareBurst, burstMinMicros, framesPerBurst);

    // Validate final burst size.
    if (framesPerBurst < MIN_FRAMES_PER_BURST || framesPerBurst > MAX_FRAMES_PER_BURST) {
        ALOGE("%s - framesPerBurst out of range = %d", __func__, framesPerBurst);
        result = AAUDIO_ERROR_OUT_OF_RANGE;
        goto error;
    }
    mFramesPerBurst = framesPerBurst; // only save good value

    capacity = mEndpointDescriptor.dataQueueDescriptor.capacityInFrames;
    if (capacity < mFramesPerBurst || capacity > MAX_BUFFER_CAPACITY_IN_FRAMES) {
        ALOGE("%s - bufferCapacity out of range = %d", __func__, capacity);
        result = AAUDIO_ERROR_OUT_OF_RANGE;
        goto error;
    }

    mClockModel.setSampleRate(getSampleRate());
    mClockModel.setFramesPerBurst(framesPerHardwareBurst);

    if (isDataCallbackSet()) {
        mCallbackFrames = builder.getFramesPerDataCallback();
        if (mCallbackFrames > getBufferCapacity() / 2) {
            ALOGW("%s - framesPerCallback too big = %d, capacity = %d",
                  __func__, mCallbackFrames, getBufferCapacity());
            result = AAUDIO_ERROR_OUT_OF_RANGE;
            goto error;

        } else if (mCallbackFrames < 0) {
            ALOGW("%s - framesPerCallback negative", __func__);
            result = AAUDIO_ERROR_OUT_OF_RANGE;
            goto error;

        }
        if (mCallbackFrames == AAUDIO_UNSPECIFIED) {
            mCallbackFrames = mFramesPerBurst;
        }

        const int32_t callbackBufferSize = mCallbackFrames * getBytesPerFrame();
        mCallbackBuffer = new uint8_t[callbackBufferSize];
    }

    setState(AAUDIO_STREAM_STATE_OPEN);

    return result;

error:
    close();
    return result;
}

// This must be called under mStreamLock.
aaudio_result_t AudioStreamInternal::close() {
    aaudio_result_t result = AAUDIO_OK;
    ALOGV("%s(): mServiceStreamHandle = 0x%08X", __func__, mServiceStreamHandle);
    if (mServiceStreamHandle != AAUDIO_HANDLE_INVALID) {
        // Don't close a stream while it is running.
        aaudio_stream_state_t currentState = getState();
        // Don't close a stream while it is running. Stop it first.
        // If DISCONNECTED then we should still try to stop in case the
        // error callback is still running.
        if (isActive() || currentState == AAUDIO_STREAM_STATE_DISCONNECTED) {
            requestStop();
        }
        setState(AAUDIO_STREAM_STATE_CLOSING);
        aaudio_handle_t serviceStreamHandle = mServiceStreamHandle;
        mServiceStreamHandle = AAUDIO_HANDLE_INVALID;

        mServiceInterface.closeStream(serviceStreamHandle);
        delete[] mCallbackBuffer;
        mCallbackBuffer = nullptr;

        setState(AAUDIO_STREAM_STATE_CLOSED);
        result = mEndPointParcelable.close();
        aaudio_result_t result2 = AudioStream::close();
        return (result != AAUDIO_OK) ? result : result2;
    } else {
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
}

static void *aaudio_callback_thread_proc(void *context)
{
    AudioStreamInternal *stream = (AudioStreamInternal *)context;
    //LOGD("oboe_callback_thread, stream = %p", stream);
    if (stream != NULL) {
        return stream->callbackLoop();
    } else {
        return NULL;
    }
}

/*
 * It normally takes about 20-30 msec to start a stream on the server.
 * But the first time can take as much as 200-300 msec. The HW
 * starts right away so by the time the client gets a chance to write into
 * the buffer, it is already in a deep underflow state. That can cause the
 * XRunCount to be non-zero, which could lead an app to tune its latency higher.
 * To avoid this problem, we set a request for the processing code to start the
 * client stream at the same position as the server stream.
 * The processing code will then save the current offset
 * between client and server and apply that to any position given to the app.
 */
aaudio_result_t AudioStreamInternal::requestStart()
{
    int64_t startTime;
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGD("requestStart() mServiceStreamHandle invalid");
        return AAUDIO_ERROR_INVALID_STATE;
    }
    if (isActive()) {
        ALOGD("requestStart() already active");
        return AAUDIO_ERROR_INVALID_STATE;
    }

    aaudio_stream_state_t originalState = getState();
    if (originalState == AAUDIO_STREAM_STATE_DISCONNECTED) {
        ALOGD("requestStart() but DISCONNECTED");
        return AAUDIO_ERROR_DISCONNECTED;
    }
    setState(AAUDIO_STREAM_STATE_STARTING);

    // Clear any stale timestamps from the previous run.
    drainTimestampsFromService();

    aaudio_result_t result = mServiceInterface.startStream(mServiceStreamHandle);

    startTime = AudioClock::getNanoseconds();
    mClockModel.start(startTime);
    mNeedCatchUp.request();  // Ask data processing code to catch up when first timestamp received.

    // Start data callback thread.
    if (result == AAUDIO_OK && isDataCallbackSet()) {
        // Launch the callback loop thread.
        int64_t periodNanos = mCallbackFrames
                              * AAUDIO_NANOS_PER_SECOND
                              / getSampleRate();
        mCallbackEnabled.store(true);
        result = createThread(periodNanos, aaudio_callback_thread_proc, this);
    }
    if (result != AAUDIO_OK) {
        setState(originalState);
    }
    return result;
}

int64_t AudioStreamInternal::calculateReasonableTimeout(int32_t framesPerOperation) {

    // Wait for at least a second or some number of callbacks to join the thread.
    int64_t timeoutNanoseconds = (MIN_TIMEOUT_OPERATIONS
                                  * framesPerOperation
                                  * AAUDIO_NANOS_PER_SECOND)
                                  / getSampleRate();
    if (timeoutNanoseconds < MIN_TIMEOUT_NANOS) { // arbitrary number of seconds
        timeoutNanoseconds = MIN_TIMEOUT_NANOS;
    }
    return timeoutNanoseconds;
}

int64_t AudioStreamInternal::calculateReasonableTimeout() {
    return calculateReasonableTimeout(getFramesPerBurst());
}

// This must be called under mStreamLock.
aaudio_result_t AudioStreamInternal::stopCallback()
{
    if (isDataCallbackSet()
            && (isActive() || getState() == AAUDIO_STREAM_STATE_DISCONNECTED)) {
        mCallbackEnabled.store(false);
        return joinThread(NULL); // may temporarily unlock mStreamLock
    } else {
        return AAUDIO_OK;
    }
}

// This must be called under mStreamLock.
aaudio_result_t AudioStreamInternal::requestStop() {
    aaudio_result_t result = stopCallback();
    if (result != AAUDIO_OK) {
        return result;
    }
    // The stream may have been unlocked temporarily to let a callback finish
    // and the callback may have stopped the stream.
    // Check to make sure the stream still needs to be stopped.
    // See also AudioStream::safeStop().
    if (!(isActive() || getState() == AAUDIO_STREAM_STATE_DISCONNECTED)) {
        return AAUDIO_OK;
    }

    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGW("%s() mServiceStreamHandle invalid = 0x%08X",
              __func__, mServiceStreamHandle);
        return AAUDIO_ERROR_INVALID_STATE;
    }

    mClockModel.stop(AudioClock::getNanoseconds());
    setState(AAUDIO_STREAM_STATE_STOPPING);
    mAtomicInternalTimestamp.clear();

    return mServiceInterface.stopStream(mServiceStreamHandle);
}

aaudio_result_t AudioStreamInternal::registerThread() {
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGW("%s() mServiceStreamHandle invalid", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    return mServiceInterface.registerAudioThread(mServiceStreamHandle,
                                              gettid(),
                                              getPeriodNanoseconds());
}

aaudio_result_t AudioStreamInternal::unregisterThread() {
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGW("%s() mServiceStreamHandle invalid", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    return mServiceInterface.unregisterAudioThread(mServiceStreamHandle, gettid());
}

aaudio_result_t AudioStreamInternal::startClient(const android::AudioClient& client,
                                                 audio_port_handle_t *portHandle) {
    ALOGV("%s() called", __func__);
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        return AAUDIO_ERROR_INVALID_STATE;
    }
    aaudio_result_t result =  mServiceInterface.startClient(mServiceStreamHandle,
                                                            client, portHandle);
    ALOGV("%s(%d) returning %d", __func__, *portHandle, result);
    return result;
}

aaudio_result_t AudioStreamInternal::stopClient(audio_port_handle_t portHandle) {
    ALOGV("%s(%d) called", __func__, portHandle);
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        return AAUDIO_ERROR_INVALID_STATE;
    }
    aaudio_result_t result = mServiceInterface.stopClient(mServiceStreamHandle, portHandle);
    ALOGV("%s(%d) returning %d", __func__, portHandle, result);
    return result;
}

aaudio_result_t AudioStreamInternal::getTimestamp(clockid_t clockId,
                           int64_t *framePosition,
                           int64_t *timeNanoseconds) {
    // Generated in server and passed to client. Return latest.
    if (mAtomicInternalTimestamp.isValid()) {
        Timestamp timestamp = mAtomicInternalTimestamp.read();
        int64_t position = timestamp.getPosition() + mFramesOffsetFromService;
        if (position >= 0) {
            *framePosition = position;
            *timeNanoseconds = timestamp.getNanoseconds();
            return AAUDIO_OK;
        }
    }
    return AAUDIO_ERROR_INVALID_STATE;
}

aaudio_result_t AudioStreamInternal::updateStateMachine() {
    if (isDataCallbackActive()) {
        return AAUDIO_OK; // state is getting updated by the callback thread read/write call
    }
    return processCommands();
}

void AudioStreamInternal::logTimestamp(AAudioServiceMessage &command) {
    static int64_t oldPosition = 0;
    static int64_t oldTime = 0;
    int64_t framePosition = command.timestamp.position;
    int64_t nanoTime = command.timestamp.timestamp;
    ALOGD("logTimestamp: timestamp says framePosition = %8lld at nanoTime %lld",
         (long long) framePosition,
         (long long) nanoTime);
    int64_t nanosDelta = nanoTime - oldTime;
    if (nanosDelta > 0 && oldTime > 0) {
        int64_t framesDelta = framePosition - oldPosition;
        int64_t rate = (framesDelta * AAUDIO_NANOS_PER_SECOND) / nanosDelta;
        ALOGD("logTimestamp:     framesDelta = %8lld, nanosDelta = %8lld, rate = %lld",
              (long long) framesDelta, (long long) nanosDelta, (long long) rate);
    }
    oldPosition = framePosition;
    oldTime = nanoTime;
}

aaudio_result_t AudioStreamInternal::onTimestampService(AAudioServiceMessage *message) {
#if LOG_TIMESTAMPS
    logTimestamp(*message);
#endif
    processTimestamp(message->timestamp.position, message->timestamp.timestamp);
    return AAUDIO_OK;
}

aaudio_result_t AudioStreamInternal::onTimestampHardware(AAudioServiceMessage *message) {
    Timestamp timestamp(message->timestamp.position, message->timestamp.timestamp);
    mAtomicInternalTimestamp.write(timestamp);
    return AAUDIO_OK;
}

aaudio_result_t AudioStreamInternal::onEventFromServer(AAudioServiceMessage *message) {
    aaudio_result_t result = AAUDIO_OK;
    switch (message->event.event) {
        case AAUDIO_SERVICE_EVENT_STARTED:
            ALOGD("%s - got AAUDIO_SERVICE_EVENT_STARTED", __func__);
            if (getState() == AAUDIO_STREAM_STATE_STARTING) {
                setState(AAUDIO_STREAM_STATE_STARTED);
            }
            break;
        case AAUDIO_SERVICE_EVENT_PAUSED:
            ALOGD("%s - got AAUDIO_SERVICE_EVENT_PAUSED", __func__);
            if (getState() == AAUDIO_STREAM_STATE_PAUSING) {
                setState(AAUDIO_STREAM_STATE_PAUSED);
            }
            break;
        case AAUDIO_SERVICE_EVENT_STOPPED:
            ALOGD("%s - got AAUDIO_SERVICE_EVENT_STOPPED", __func__);
            if (getState() == AAUDIO_STREAM_STATE_STOPPING) {
                setState(AAUDIO_STREAM_STATE_STOPPED);
            }
            break;
        case AAUDIO_SERVICE_EVENT_FLUSHED:
            ALOGD("%s - got AAUDIO_SERVICE_EVENT_FLUSHED", __func__);
            if (getState() == AAUDIO_STREAM_STATE_FLUSHING) {
                setState(AAUDIO_STREAM_STATE_FLUSHED);
                onFlushFromServer();
            }
            break;
        case AAUDIO_SERVICE_EVENT_DISCONNECTED:
            // Prevent hardware from looping on old data and making buzzing sounds.
            if (getDirection() == AAUDIO_DIRECTION_OUTPUT) {
                mAudioEndpoint.eraseDataMemory();
            }
            result = AAUDIO_ERROR_DISCONNECTED;
            setState(AAUDIO_STREAM_STATE_DISCONNECTED);
            ALOGW("%s - AAUDIO_SERVICE_EVENT_DISCONNECTED - FIFO cleared", __func__);
            break;
        case AAUDIO_SERVICE_EVENT_VOLUME:
            ALOGD("%s - AAUDIO_SERVICE_EVENT_VOLUME %lf", __func__, message->event.dataDouble);
            mStreamVolume = (float)message->event.dataDouble;
            doSetVolume();
            break;
        case AAUDIO_SERVICE_EVENT_XRUN:
            mXRunCount = static_cast<int32_t>(message->event.dataLong);
            break;
        default:
            ALOGE("%s - Unrecognized event = %d", __func__, (int) message->event.event);
            break;
    }
    return result;
}

aaudio_result_t AudioStreamInternal::drainTimestampsFromService() {
    aaudio_result_t result = AAUDIO_OK;

    while (result == AAUDIO_OK) {
        AAudioServiceMessage message;
        if (mAudioEndpoint.readUpCommand(&message) != 1) {
            break; // no command this time, no problem
        }
        switch (message.what) {
            // ignore most messages
            case AAudioServiceMessage::code::TIMESTAMP_SERVICE:
            case AAudioServiceMessage::code::TIMESTAMP_HARDWARE:
                break;

            case AAudioServiceMessage::code::EVENT:
                result = onEventFromServer(&message);
                break;

            default:
                ALOGE("%s - unrecognized message.what = %d", __func__, (int) message.what);
                result = AAUDIO_ERROR_INTERNAL;
                break;
        }
    }
    return result;
}

// Process all the commands coming from the server.
aaudio_result_t AudioStreamInternal::processCommands() {
    aaudio_result_t result = AAUDIO_OK;

    while (result == AAUDIO_OK) {
        AAudioServiceMessage message;
        if (mAudioEndpoint.readUpCommand(&message) != 1) {
            break; // no command this time, no problem
        }
        switch (message.what) {
        case AAudioServiceMessage::code::TIMESTAMP_SERVICE:
            result = onTimestampService(&message);
            break;

        case AAudioServiceMessage::code::TIMESTAMP_HARDWARE:
            result = onTimestampHardware(&message);
            break;

        case AAudioServiceMessage::code::EVENT:
            result = onEventFromServer(&message);
            break;

        default:
            ALOGE("%s - unrecognized message.what = %d", __func__, (int) message.what);
            result = AAUDIO_ERROR_INTERNAL;
            break;
        }
    }
    return result;
}

// Read or write the data, block if needed and timeoutMillis > 0
aaudio_result_t AudioStreamInternal::processData(void *buffer, int32_t numFrames,
                                                 int64_t timeoutNanoseconds)
{
    const char * traceName = "aaProc";
    const char * fifoName = "aaRdy";
    ATRACE_BEGIN(traceName);
    if (ATRACE_ENABLED()) {
        int32_t fullFrames = mAudioEndpoint.getFullFramesAvailable();
        ATRACE_INT(fifoName, fullFrames);
    }

    aaudio_result_t result = AAUDIO_OK;
    int32_t loopCount = 0;
    uint8_t* audioData = (uint8_t*)buffer;
    int64_t currentTimeNanos = AudioClock::getNanoseconds();
    const int64_t entryTimeNanos = currentTimeNanos;
    const int64_t deadlineNanos = currentTimeNanos + timeoutNanoseconds;
    int32_t framesLeft = numFrames;

    // Loop until all the data has been processed or until a timeout occurs.
    while (framesLeft > 0) {
        // The call to processDataNow() will not block. It will just process as much as it can.
        int64_t wakeTimeNanos = 0;
        aaudio_result_t framesProcessed = processDataNow(audioData, framesLeft,
                                                  currentTimeNanos, &wakeTimeNanos);
        if (framesProcessed < 0) {
            result = framesProcessed;
            break;
        }
        framesLeft -= (int32_t) framesProcessed;
        audioData += framesProcessed * getBytesPerFrame();

        // Should we block?
        if (timeoutNanoseconds == 0) {
            break; // don't block
        } else if (framesLeft > 0) {
            if (!mAudioEndpoint.isFreeRunning()) {
                // If there is software on the other end of the FIFO then it may get delayed.
                // So wake up just a little after we expect it to be ready.
                wakeTimeNanos += mWakeupDelayNanos;
            }

            currentTimeNanos = AudioClock::getNanoseconds();
            int64_t earliestWakeTime = currentTimeNanos + mMinimumSleepNanos;
            // Guarantee a minimum sleep time.
            if (wakeTimeNanos < earliestWakeTime) {
                wakeTimeNanos = earliestWakeTime;
            }

            if (wakeTimeNanos > deadlineNanos) {
                // If we time out, just return the framesWritten so far.
                // TODO remove after we fix the deadline bug
                ALOGW("processData(): entered at %lld nanos, currently %lld",
                      (long long) entryTimeNanos, (long long) currentTimeNanos);
                ALOGW("processData(): TIMEOUT after %lld nanos",
                      (long long) timeoutNanoseconds);
                ALOGW("processData(): wakeTime = %lld, deadline = %lld nanos",
                      (long long) wakeTimeNanos, (long long) deadlineNanos);
                ALOGW("processData(): past deadline by %d micros",
                      (int)((wakeTimeNanos - deadlineNanos) / AAUDIO_NANOS_PER_MICROSECOND));
                mClockModel.dump();
                mAudioEndpoint.dump();
                break;
            }

            if (ATRACE_ENABLED()) {
                int32_t fullFrames = mAudioEndpoint.getFullFramesAvailable();
                ATRACE_INT(fifoName, fullFrames);
                int64_t sleepForNanos = wakeTimeNanos - currentTimeNanos;
                ATRACE_INT("aaSlpNs", (int32_t)sleepForNanos);
            }

            AudioClock::sleepUntilNanoTime(wakeTimeNanos);
            currentTimeNanos = AudioClock::getNanoseconds();
        }
    }

    if (ATRACE_ENABLED()) {
        int32_t fullFrames = mAudioEndpoint.getFullFramesAvailable();
        ATRACE_INT(fifoName, fullFrames);
    }

    // return error or framesProcessed
    (void) loopCount;
    ATRACE_END();
    return (result < 0) ? result : numFrames - framesLeft;
}

void AudioStreamInternal::processTimestamp(uint64_t position, int64_t time) {
    mClockModel.processTimestamp(position, time);
}

aaudio_result_t AudioStreamInternal::setBufferSize(int32_t requestedFrames) {
    int32_t adjustedFrames = requestedFrames;
    int32_t actualFrames = 0;
    int32_t maximumSize = getBufferCapacity();

    // Clip to minimum size so that rounding up will work better.
    if (adjustedFrames < 1) {
        adjustedFrames = 1;
    }

    if (adjustedFrames > maximumSize) {
        // Clip to maximum size.
        adjustedFrames = maximumSize;
    } else {
        // Round to the next highest burst size.
        int32_t numBursts = (adjustedFrames + mFramesPerBurst - 1) / mFramesPerBurst;
        adjustedFrames = numBursts * mFramesPerBurst;
        // Rounding may have gone above maximum.
        if (adjustedFrames > maximumSize) {
            adjustedFrames = maximumSize;
        }
    }

    aaudio_result_t result = mAudioEndpoint.setBufferSizeInFrames(adjustedFrames, &actualFrames);
    if (result < 0) {
        return result;
    } else {
        return (aaudio_result_t) actualFrames;
    }
}

int32_t AudioStreamInternal::getBufferSize() const {
    return mAudioEndpoint.getBufferSizeInFrames();
}

int32_t AudioStreamInternal::getBufferCapacity() const {
    return mAudioEndpoint.getBufferCapacityInFrames();
}

int32_t AudioStreamInternal::getFramesPerBurst() const {
    return mFramesPerBurst;
}

// This must be called under mStreamLock.
aaudio_result_t AudioStreamInternal::joinThread(void** returnArg) {
    return AudioStream::joinThread(returnArg, calculateReasonableTimeout(getFramesPerBurst()));
}

bool AudioStreamInternal::isClockModelInControl() const {
    return isActive() && mAudioEndpoint.isFreeRunning() && mClockModel.isRunning();
}
