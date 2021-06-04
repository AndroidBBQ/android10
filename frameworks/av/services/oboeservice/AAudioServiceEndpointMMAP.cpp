/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "AAudioServiceEndpointMMAP"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <algorithm>
#include <assert.h>
#include <map>
#include <mutex>
#include <sstream>
#include <utils/Singleton.h>
#include <vector>


#include "AAudioEndpointManager.h"
#include "AAudioServiceEndpoint.h"

#include "core/AudioStreamBuilder.h"
#include "AAudioServiceEndpoint.h"
#include "AAudioServiceStreamShared.h"
#include "AAudioServiceEndpointPlay.h"
#include "AAudioServiceEndpointMMAP.h"


#define AAUDIO_BUFFER_CAPACITY_MIN    4 * 512
#define AAUDIO_SAMPLE_RATE_DEFAULT    48000

// This is an estimate of the time difference between the HW and the MMAP time.
// TODO Get presentation timestamps from the HAL instead of using these estimates.
#define OUTPUT_ESTIMATED_HARDWARE_OFFSET_NANOS  (3 * AAUDIO_NANOS_PER_MILLISECOND)
#define INPUT_ESTIMATED_HARDWARE_OFFSET_NANOS   (-1 * AAUDIO_NANOS_PER_MILLISECOND)

using namespace android;  // TODO just import names needed
using namespace aaudio;   // TODO just import names needed


AAudioServiceEndpointMMAP::AAudioServiceEndpointMMAP(AAudioService &audioService)
        : mMmapStream(nullptr)
        , mAAudioService(audioService) {}

AAudioServiceEndpointMMAP::~AAudioServiceEndpointMMAP() {}

std::string AAudioServiceEndpointMMAP::dump() const {
    std::stringstream result;

    result << "  MMAP: framesTransferred = " << mFramesTransferred.get();
    result << ", HW nanos = " << mHardwareTimeOffsetNanos;
    result << ", port handle = " << mPortHandle;
    result << ", audio data FD = " << mAudioDataFileDescriptor;
    result << "\n";

    result << "    HW Offset Micros:     " <<
                                      (getHardwareTimeOffsetNanos()
                                       / AAUDIO_NANOS_PER_MICROSECOND) << "\n";

    result << AAudioServiceEndpoint::dump();
    return result.str();
}

aaudio_result_t AAudioServiceEndpointMMAP::open(const aaudio::AAudioStreamRequest &request) {
    aaudio_result_t result = AAUDIO_OK;
    audio_config_base_t config;
    audio_port_handle_t deviceId;

    copyFrom(request.getConstantConfiguration());

    aaudio_direction_t direction = getDirection();

    const audio_content_type_t contentType =
            AAudioConvert_contentTypeToInternal(getContentType());
    // Usage only used for OUTPUT
    const audio_usage_t usage = (direction == AAUDIO_DIRECTION_OUTPUT)
            ? AAudioConvert_usageToInternal(getUsage())
            : AUDIO_USAGE_UNKNOWN;
    const audio_source_t source = (direction == AAUDIO_DIRECTION_INPUT)
            ? AAudioConvert_inputPresetToAudioSource(getInputPreset())
            : AUDIO_SOURCE_DEFAULT;
    const audio_flags_mask_t flags = AUDIO_FLAG_LOW_LATENCY |
            AAudioConvert_allowCapturePolicyToAudioFlagsMask(getAllowedCapturePolicy());

    const audio_attributes_t attributes = {
            .content_type = contentType,
            .usage = usage,
            .source = source,
            .flags = flags,
            .tags = ""
    };

    mMmapClient.clientUid = request.getUserId();
    mMmapClient.clientPid = request.getProcessId();
    mMmapClient.packageName.setTo(String16(""));

    mRequestedDeviceId = deviceId = getDeviceId();

    // Fill in config
    audio_format_t audioFormat = getFormat();
    if (audioFormat == AUDIO_FORMAT_DEFAULT || audioFormat == AUDIO_FORMAT_PCM_FLOAT) {
        audioFormat = AUDIO_FORMAT_PCM_16_BIT;
    }
    config.format = audioFormat;

    int32_t aaudioSampleRate = getSampleRate();
    if (aaudioSampleRate == AAUDIO_UNSPECIFIED) {
        aaudioSampleRate = AAUDIO_SAMPLE_RATE_DEFAULT;
    }
    config.sample_rate = aaudioSampleRate;

    int32_t aaudioSamplesPerFrame = getSamplesPerFrame();

    if (direction == AAUDIO_DIRECTION_OUTPUT) {
        config.channel_mask = (aaudioSamplesPerFrame == AAUDIO_UNSPECIFIED)
                              ? AUDIO_CHANNEL_OUT_STEREO
                              : audio_channel_out_mask_from_count(aaudioSamplesPerFrame);
        mHardwareTimeOffsetNanos = OUTPUT_ESTIMATED_HARDWARE_OFFSET_NANOS; // frames at DAC later

    } else if (direction == AAUDIO_DIRECTION_INPUT) {
        config.channel_mask =  (aaudioSamplesPerFrame == AAUDIO_UNSPECIFIED)
                               ? AUDIO_CHANNEL_IN_STEREO
                               : audio_channel_in_mask_from_count(aaudioSamplesPerFrame);
        mHardwareTimeOffsetNanos = INPUT_ESTIMATED_HARDWARE_OFFSET_NANOS; // frames at ADC earlier

    } else {
        ALOGE("%s() invalid direction = %d", __func__, direction);
        return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
    }

    MmapStreamInterface::stream_direction_t streamDirection =
            (direction == AAUDIO_DIRECTION_OUTPUT)
            ? MmapStreamInterface::DIRECTION_OUTPUT
            : MmapStreamInterface::DIRECTION_INPUT;

    aaudio_session_id_t requestedSessionId = getSessionId();
    audio_session_t sessionId = AAudioConvert_aaudioToAndroidSessionId(requestedSessionId);

    // Open HAL stream. Set mMmapStream
    status_t status = MmapStreamInterface::openMmapStream(streamDirection,
                                                          &attributes,
                                                          &config,
                                                          mMmapClient,
                                                          &deviceId,
                                                          &sessionId,
                                                          this, // callback
                                                          mMmapStream,
                                                          &mPortHandle);
    ALOGD("%s() mMapClient.uid = %d, pid = %d => portHandle = %d\n",
          __func__, mMmapClient.clientUid,  mMmapClient.clientPid, mPortHandle);
    if (status != OK) {
        // This can happen if the resource is busy or the config does
        // not match the hardware.
        ALOGD("%s() - openMmapStream() returned status %d",  __func__, status);
        return AAUDIO_ERROR_UNAVAILABLE;
    }

    if (deviceId == AAUDIO_UNSPECIFIED) {
        ALOGW("%s() - openMmapStream() failed to set deviceId", __func__);
    }
    setDeviceId(deviceId);

    if (sessionId == AUDIO_SESSION_ALLOCATE) {
        ALOGW("%s() - openMmapStream() failed to set sessionId", __func__);
    }

    aaudio_session_id_t actualSessionId =
            (requestedSessionId == AAUDIO_SESSION_ID_NONE)
            ? AAUDIO_SESSION_ID_NONE
            : (aaudio_session_id_t) sessionId;
    setSessionId(actualSessionId);
    ALOGD("%s() deviceId = %d, sessionId = %d", __func__, getDeviceId(), getSessionId());

    // Create MMAP/NOIRQ buffer.
    int32_t minSizeFrames = getBufferCapacity();
    if (minSizeFrames <= 0) { // zero will get rejected
        minSizeFrames = AAUDIO_BUFFER_CAPACITY_MIN;
    }
    status = mMmapStream->createMmapBuffer(minSizeFrames, &mMmapBufferinfo);
    bool isBufferShareable = mMmapBufferinfo.flags & AUDIO_MMAP_APPLICATION_SHAREABLE;
    if (status != OK) {
        ALOGE("%s() - createMmapBuffer() failed with status %d %s",
              __func__, status, strerror(-status));
        result = AAUDIO_ERROR_UNAVAILABLE;
        goto error;
    } else {
        ALOGD("%s() createMmapBuffer() buffer_size = %d fr, burst_size %d fr"
                      ", Sharable FD: %s",
              __func__,
              mMmapBufferinfo.buffer_size_frames,
              mMmapBufferinfo.burst_size_frames,
              isBufferShareable ? "Yes" : "No");
    }

    setBufferCapacity(mMmapBufferinfo.buffer_size_frames);
    if (!isBufferShareable) {
        // Exclusive mode can only be used by the service because the FD cannot be shared.
        uid_t audioServiceUid = getuid();
        if ((mMmapClient.clientUid != audioServiceUid) &&
            getSharingMode() == AAUDIO_SHARING_MODE_EXCLUSIVE) {
            ALOGW("%s() - exclusive FD cannot be used by client", __func__);
            result = AAUDIO_ERROR_UNAVAILABLE;
            goto error;
        }
    }

    // Get information about the stream and pass it back to the caller.
    setSamplesPerFrame((direction == AAUDIO_DIRECTION_OUTPUT)
                       ? audio_channel_count_from_out_mask(config.channel_mask)
                       : audio_channel_count_from_in_mask(config.channel_mask));

    // AAudio creates a copy of this FD and retains ownership of the copy.
    // Assume that AudioFlinger will close the original shared_memory_fd.
    mAudioDataFileDescriptor.reset(dup(mMmapBufferinfo.shared_memory_fd));
    if (mAudioDataFileDescriptor.get() == -1) {
        ALOGE("%s() - could not dup shared_memory_fd", __func__);
        result = AAUDIO_ERROR_INTERNAL;
        goto error;
    }
    mFramesPerBurst = mMmapBufferinfo.burst_size_frames;
    setFormat(config.format);
    setSampleRate(config.sample_rate);

    ALOGD("%s() actual rate = %d, channels = %d"
          ", deviceId = %d, capacity = %d\n",
          __func__, getSampleRate(), getSamplesPerFrame(), deviceId, getBufferCapacity());

    ALOGD("%s() format = 0x%08x, frame size = %d, burst size = %d",
          __func__, getFormat(), calculateBytesPerFrame(), mFramesPerBurst);

    return result;

error:
    close();
    return result;
}

aaudio_result_t AAudioServiceEndpointMMAP::close() {
    if (mMmapStream != 0) {
        // Needs to be explicitly cleared or CTS will fail but it is not clear why.
        mMmapStream.clear();
        // Apparently the above close is asynchronous. An attempt to open a new device
        // right after a close can fail. Also some callbacks may still be in flight!
        // FIXME Make closing synchronous.
        AudioClock::sleepForNanos(100 * AAUDIO_NANOS_PER_MILLISECOND);
    }

    return AAUDIO_OK;
}

aaudio_result_t AAudioServiceEndpointMMAP::startStream(sp<AAudioServiceStreamBase> stream,
                                                   audio_port_handle_t *clientHandle __unused) {
    // Start the client on behalf of the AAudio service.
    // Use the port handle that was provided by openMmapStream().
    audio_port_handle_t tempHandle = mPortHandle;
    aaudio_result_t result = startClient(mMmapClient, &tempHandle);
    // When AudioFlinger is passed a valid port handle then it should not change it.
    LOG_ALWAYS_FATAL_IF(tempHandle != mPortHandle,
                        "%s() port handle not expected to change from %d to %d",
                        __func__, mPortHandle, tempHandle);
    ALOGV("%s() mPortHandle = %d", __func__, mPortHandle);
    return result;
}

aaudio_result_t AAudioServiceEndpointMMAP::stopStream(sp<AAudioServiceStreamBase> stream,
                                                  audio_port_handle_t clientHandle __unused) {
    mFramesTransferred.reset32();

    // Round 64-bit counter up to a multiple of the buffer capacity.
    // This is required because the 64-bit counter is used as an index
    // into a circular buffer and the actual HW position is reset to zero
    // when the stream is stopped.
    mFramesTransferred.roundUp64(getBufferCapacity());

    // Use the port handle that was provided by openMmapStream().
    ALOGV("%s() mPortHandle = %d", __func__, mPortHandle);
    return stopClient(mPortHandle);
}

aaudio_result_t AAudioServiceEndpointMMAP::startClient(const android::AudioClient& client,
                                                       audio_port_handle_t *clientHandle) {
    if (mMmapStream == nullptr) return AAUDIO_ERROR_NULL;
    status_t status = mMmapStream->start(client, clientHandle);
    return AAudioConvert_androidToAAudioResult(status);
}

aaudio_result_t AAudioServiceEndpointMMAP::stopClient(audio_port_handle_t clientHandle) {
    if (mMmapStream == nullptr) return AAUDIO_ERROR_NULL;
    aaudio_result_t result = AAudioConvert_androidToAAudioResult(mMmapStream->stop(clientHandle));
    return result;
}

// Get free-running DSP or DMA hardware position from the HAL.
aaudio_result_t AAudioServiceEndpointMMAP::getFreeRunningPosition(int64_t *positionFrames,
                                                                int64_t *timeNanos) {
    struct audio_mmap_position position;
    if (mMmapStream == nullptr) {
        return AAUDIO_ERROR_NULL;
    }
    status_t status = mMmapStream->getMmapPosition(&position);
    ALOGV("%s() status= %d, pos = %d, nanos = %lld\n",
          __func__, status, position.position_frames, (long long) position.time_nanoseconds);
    aaudio_result_t result = AAudioConvert_androidToAAudioResult(status);
    if (result == AAUDIO_ERROR_UNAVAILABLE) {
        ALOGW("%s(): getMmapPosition() has no position data available", __func__);
    } else if (result != AAUDIO_OK) {
        ALOGE("%s(): getMmapPosition() returned status %d", __func__, status);
    } else {
        // Convert 32-bit position to 64-bit position.
        mFramesTransferred.update32(position.position_frames);
        *positionFrames = mFramesTransferred.get();
        *timeNanos = position.time_nanoseconds;
    }
    return result;
}

aaudio_result_t AAudioServiceEndpointMMAP::getTimestamp(int64_t *positionFrames,
                                                    int64_t *timeNanos) {
    return 0; // TODO
}

// This is called by AudioFlinger when it wants to destroy a stream.
void AAudioServiceEndpointMMAP::onTearDown(audio_port_handle_t portHandle) {
    ALOGD("%s(portHandle = %d) called", __func__, portHandle);
    // Are we tearing down the EXCLUSIVE MMAP stream?
    if (isStreamRegistered(portHandle)) {
        ALOGD("%s(%d) tearing down this entire MMAP endpoint", __func__, portHandle);
        disconnectRegisteredStreams();
    } else {
        // Must be a SHARED stream?
        ALOGD("%s(%d) disconnect a specific stream", __func__, portHandle);
        aaudio_result_t result = mAAudioService.disconnectStreamByPortHandle(portHandle);
        ALOGD("%s(%d) disconnectStreamByPortHandle returned %d", __func__, portHandle, result);
    }
};

void AAudioServiceEndpointMMAP::onVolumeChanged(audio_channel_mask_t channels,
                                              android::Vector<float> values) {
    // TODO Do we really need a different volume for each channel?
    // We get called with an array filled with a single value!
    float volume = values[0];
    ALOGD("%s() volume[0] = %f", __func__, volume);
    std::lock_guard<std::mutex> lock(mLockStreams);
    for(const auto& stream : mRegisteredStreams) {
        stream->onVolumeChanged(volume);
    }
};

void AAudioServiceEndpointMMAP::onRoutingChanged(audio_port_handle_t deviceId) {
    ALOGD("%s() called with dev %d, old = %d", __func__, deviceId, getDeviceId());
    if (getDeviceId() != AUDIO_PORT_HANDLE_NONE  && getDeviceId() != deviceId) {
        disconnectRegisteredStreams();
    }
    setDeviceId(deviceId);
};

/**
 * Get an immutable description of the data queue from the HAL.
 */
aaudio_result_t AAudioServiceEndpointMMAP::getDownDataDescription(AudioEndpointParcelable &parcelable)
{
    // Gather information on the data queue based on HAL info.
    int32_t bytesPerFrame = calculateBytesPerFrame();
    int32_t capacityInBytes = getBufferCapacity() * bytesPerFrame;
    int fdIndex = parcelable.addFileDescriptor(mAudioDataFileDescriptor, capacityInBytes);
    parcelable.mDownDataQueueParcelable.setupMemory(fdIndex, 0, capacityInBytes);
    parcelable.mDownDataQueueParcelable.setBytesPerFrame(bytesPerFrame);
    parcelable.mDownDataQueueParcelable.setFramesPerBurst(mFramesPerBurst);
    parcelable.mDownDataQueueParcelable.setCapacityInFrames(getBufferCapacity());
    return AAUDIO_OK;
}
