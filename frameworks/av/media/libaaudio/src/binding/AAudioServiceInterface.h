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

#ifndef ANDROID_AAUDIO_BINDING_AAUDIO_SERVICE_INTERFACE_H
#define ANDROID_AAUDIO_BINDING_AAUDIO_SERVICE_INTERFACE_H

#include <utils/StrongPointer.h>
#include <media/AudioClient.h>

#include "binding/AAudioServiceDefinitions.h"
#include "binding/AAudioStreamRequest.h"
#include "binding/AAudioStreamConfiguration.h"
#include "binding/AudioEndpointParcelable.h"
#include "binding/IAAudioClient.h"

/**
 * This has the same methods as IAAudioService but without the Binder features.
 *
 * It allows us to abstract the Binder interface and use an AudioStreamInternal
 * both in the client and in the service.
 */
namespace aaudio {

class AAudioServiceInterface {
public:

    AAudioServiceInterface() {};
    virtual ~AAudioServiceInterface() = default;

    virtual void registerClient(const android::sp<android::IAAudioClient>& client) = 0;

    /**
     * @param request info needed to create the stream
     * @param configuration contains information about the created stream
     * @return handle to the stream or a negative error
     */
    virtual aaudio_handle_t openStream(const AAudioStreamRequest &request,
                                       AAudioStreamConfiguration &configuration) = 0;

    virtual aaudio_result_t closeStream(aaudio_handle_t streamHandle) = 0;

    /* Get an immutable description of the in-memory queues
    * used to communicate with the underlying HAL or Service.
    */
    virtual aaudio_result_t getStreamDescription(aaudio_handle_t streamHandle,
                                                 AudioEndpointParcelable &parcelable) = 0;

    /**
     * Start the flow of data.
     */
    virtual aaudio_result_t startStream(aaudio_handle_t streamHandle) = 0;

    /**
     * Stop the flow of data such that start() can resume without loss of data.
     */
    virtual aaudio_result_t pauseStream(aaudio_handle_t streamHandle) = 0;

    /**
     * Stop the flow of data after data currently inthe buffer has played.
     */
    virtual aaudio_result_t stopStream(aaudio_handle_t streamHandle) = 0;

    /**
     *  Discard any data held by the underlying HAL or Service.
     */
    virtual aaudio_result_t flushStream(aaudio_handle_t streamHandle) = 0;

    /**
     * Manage the specified thread as a low latency audio thread.
     */
    virtual aaudio_result_t registerAudioThread(aaudio_handle_t streamHandle,
                                                pid_t clientThreadId,
                                                int64_t periodNanoseconds) = 0;

    virtual aaudio_result_t unregisterAudioThread(aaudio_handle_t streamHandle,
                                                  pid_t clientThreadId) = 0;

    virtual aaudio_result_t startClient(aaudio_handle_t streamHandle,
                                      const android::AudioClient& client,
                                      audio_port_handle_t *clientHandle) = 0;

    virtual aaudio_result_t stopClient(aaudio_handle_t streamHandle,
                                       audio_port_handle_t clientHandle) = 0;
};

} /* namespace aaudio */

#endif //ANDROID_AAUDIO_BINDING_AAUDIO_SERVICE_INTERFACE_H
