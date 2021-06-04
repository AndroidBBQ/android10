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

#ifndef AAUDIO_SERVICE_ENDPOINT_MMAP_H
#define AAUDIO_SERVICE_ENDPOINT_MMAP_H

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

#include "client/AudioStreamInternal.h"
#include "client/AudioStreamInternalPlay.h"
#include "binding/AAudioServiceMessage.h"
#include "AAudioServiceEndpointShared.h"
#include "AAudioServiceStreamShared.h"
#include "AAudioServiceStreamMMAP.h"
#include "AAudioMixer.h"
#include "AAudioService.h"

namespace aaudio {

/**
 * This is used by AAudioServiceStreamMMAP to access the MMAP devices
 * through AudioFlinger.
 */
class AAudioServiceEndpointMMAP
        : public AAudioServiceEndpoint
        , public android::MmapStreamCallback {

public:
    explicit AAudioServiceEndpointMMAP(android::AAudioService &audioService);

    virtual ~AAudioServiceEndpointMMAP();

    std::string dump() const override;

    aaudio_result_t open(const aaudio::AAudioStreamRequest &request) override;

    aaudio_result_t close() override;

    aaudio_result_t startStream(android::sp<AAudioServiceStreamBase> stream,
                                audio_port_handle_t *clientHandle) override;

    aaudio_result_t stopStream(android::sp<AAudioServiceStreamBase> stream,
                               audio_port_handle_t clientHandle) override;

    aaudio_result_t startClient(const android::AudioClient& client,
                                        audio_port_handle_t *clientHandle)  override;

    aaudio_result_t stopClient(audio_port_handle_t clientHandle)  override;

    aaudio_result_t getFreeRunningPosition(int64_t *positionFrames, int64_t *timeNanos) override;

    aaudio_result_t getTimestamp(int64_t *positionFrames, int64_t *timeNanos) override;

    // -------------- Callback functions for MmapStreamCallback ---------------------
    void onTearDown(audio_port_handle_t handle) override;

    void onVolumeChanged(audio_channel_mask_t channels,
                         android::Vector<float> values) override;

    void onRoutingChanged(audio_port_handle_t deviceId) override;
    // ------------------------------------------------------------------------------

    aaudio_result_t getDownDataDescription(AudioEndpointParcelable &parcelable);

    int64_t getHardwareTimeOffsetNanos() const {
        return mHardwareTimeOffsetNanos;
    }

private:
    MonotonicCounter                          mFramesTransferred;

    // Interface to the AudioFlinger MMAP support.
    android::sp<android::MmapStreamInterface> mMmapStream;
    struct audio_mmap_buffer_info             mMmapBufferinfo;

    // There is only one port associated with an MMAP endpoint.
    audio_port_handle_t                       mPortHandle = AUDIO_PORT_HANDLE_NONE;

    android::AAudioService                    &mAAudioService;

    android::base::unique_fd                  mAudioDataFileDescriptor;

    int64_t                                   mHardwareTimeOffsetNanos = 0; // TODO get from HAL

};

} /* namespace aaudio */

#endif //AAUDIO_SERVICE_ENDPOINT_MMAP_H

