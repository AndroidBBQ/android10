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

#define LOG_TAG "AAudioServiceStreamMMAP"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <atomic>
#include <iomanip>
#include <iostream>
#include <stdint.h>

#include <utils/String16.h>
#include <media/nbaio/AudioStreamOutSink.h>
#include <media/MmapStreamInterface.h>

#include "binding/AudioEndpointParcelable.h"
#include "utility/AAudioUtilities.h"

#include "AAudioServiceEndpointMMAP.h"
#include "AAudioServiceStreamBase.h"
#include "AAudioServiceStreamMMAP.h"
#include "SharedMemoryProxy.h"

using android::base::unique_fd;
using namespace android;
using namespace aaudio;

/**
 * Service Stream that uses an MMAP buffer.
 */

AAudioServiceStreamMMAP::AAudioServiceStreamMMAP(android::AAudioService &aAudioService,
                                                 bool inService)
        : AAudioServiceStreamBase(aAudioService)
        , mInService(inService) {
}

aaudio_result_t AAudioServiceStreamMMAP::close() {
    if (getState() == AAUDIO_STREAM_STATE_CLOSED) {
        return AAUDIO_OK;
    }

    stop();

    return AAudioServiceStreamBase::close();
}

// Open stream on HAL and pass information about the shared memory buffer back to the client.
aaudio_result_t AAudioServiceStreamMMAP::open(const aaudio::AAudioStreamRequest &request) {

    sp<AAudioServiceStreamMMAP> keep(this);

    if (request.getConstantConfiguration().getSharingMode() != AAUDIO_SHARING_MODE_EXCLUSIVE) {
        ALOGE("%s() sharingMode mismatch %d", __func__,
              request.getConstantConfiguration().getSharingMode());
        return AAUDIO_ERROR_INTERNAL;
    }

    aaudio_result_t result = AAudioServiceStreamBase::open(request);
    if (result != AAUDIO_OK) {
        return result;
    }

    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }

    result = endpoint->registerStream(keep);
    if (result != AAUDIO_OK) {
        return result;
    }

    setState(AAUDIO_STREAM_STATE_OPEN);

    return AAUDIO_OK;
}

// Start the flow of data.
aaudio_result_t AAudioServiceStreamMMAP::startDevice() {
    aaudio_result_t result = AAudioServiceStreamBase::startDevice();
    if (!mInService && result == AAUDIO_OK) {
        // Note that this can sometimes take 200 to 300 msec for a cold start!
        result = startClient(mMmapClient, &mClientHandle);
    }
    return result;
}

// Stop the flow of data such that start() can resume with loss of data.
aaudio_result_t AAudioServiceStreamMMAP::pause() {
    if (!isRunning()) {
        return AAUDIO_OK;
    }
    aaudio_result_t result = AAudioServiceStreamBase::pause();
    // TODO put before base::pause()?
    if (!mInService) {
        (void) stopClient(mClientHandle);
    }
    return result;
}

aaudio_result_t AAudioServiceStreamMMAP::stop() {
    if (!isRunning()) {
        return AAUDIO_OK;
    }
    aaudio_result_t result = AAudioServiceStreamBase::stop();
    // TODO put before base::stop()?
    if (!mInService) {
        (void) stopClient(mClientHandle);
    }
    return result;
}

aaudio_result_t AAudioServiceStreamMMAP::startClient(const android::AudioClient& client,
                                                       audio_port_handle_t *clientHandle) {
    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    // Start the client on behalf of the application. Generate a new porthandle.
    aaudio_result_t result = endpoint->startClient(client, clientHandle);
    return result;
}

aaudio_result_t AAudioServiceStreamMMAP::stopClient(audio_port_handle_t clientHandle) {
    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    aaudio_result_t result = endpoint->stopClient(clientHandle);
    return result;
}

// Get free-running DSP or DMA hardware position from the HAL.
aaudio_result_t AAudioServiceStreamMMAP::getFreeRunningPosition(int64_t *positionFrames,
                                                                  int64_t *timeNanos) {
    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    sp<AAudioServiceEndpointMMAP> serviceEndpointMMAP =
            static_cast<AAudioServiceEndpointMMAP *>(endpoint.get());

    aaudio_result_t result = serviceEndpointMMAP->getFreeRunningPosition(positionFrames, timeNanos);
    if (result == AAUDIO_OK) {
        Timestamp timestamp(*positionFrames, *timeNanos);
        mAtomicStreamTimestamp.write(timestamp);
        *positionFrames = timestamp.getPosition();
        *timeNanos = timestamp.getNanoseconds();
    } else if (result != AAUDIO_ERROR_UNAVAILABLE) {
        disconnect();
    }
    return result;
}

// Get timestamp that was written by getFreeRunningPosition()
aaudio_result_t AAudioServiceStreamMMAP::getHardwareTimestamp(int64_t *positionFrames,
                                                                int64_t *timeNanos) {

    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    sp<AAudioServiceEndpointMMAP> serviceEndpointMMAP =
            static_cast<AAudioServiceEndpointMMAP *>(endpoint.get());

    // TODO Get presentation timestamp from the HAL
    if (mAtomicStreamTimestamp.isValid()) {
        Timestamp timestamp = mAtomicStreamTimestamp.read();
        *positionFrames = timestamp.getPosition();
        *timeNanos = timestamp.getNanoseconds() + serviceEndpointMMAP->getHardwareTimeOffsetNanos();
        return AAUDIO_OK;
    } else {
        return AAUDIO_ERROR_UNAVAILABLE;
    }
}

// Get an immutable description of the data queue from the HAL.
aaudio_result_t AAudioServiceStreamMMAP::getAudioDataDescription(
        AudioEndpointParcelable &parcelable)
{
    sp<AAudioServiceEndpoint> endpoint = mServiceEndpointWeak.promote();
    if (endpoint == nullptr) {
        ALOGE("%s() has no endpoint", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }
    sp<AAudioServiceEndpointMMAP> serviceEndpointMMAP =
            static_cast<AAudioServiceEndpointMMAP *>(endpoint.get());
    return serviceEndpointMMAP->getDownDataDescription(parcelable);
}
