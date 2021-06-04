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

#ifndef ANDROID_AAUDIO_AAUDIO_BINDER_CLIENT_H
#define ANDROID_AAUDIO_AAUDIO_BINDER_CLIENT_H

#include <utils/RefBase.h>
#include <utils/Singleton.h>

#include <aaudio/AAudio.h>
#include "AAudioServiceDefinitions.h"
#include "AAudioServiceInterface.h"
#include "binding/AAudioStreamRequest.h"
#include "binding/AAudioStreamConfiguration.h"
#include "binding/AudioEndpointParcelable.h"
#include "binding/IAAudioService.h"

/**
 * Implements the AAudioServiceInterface by talking to the service through Binder.
 */

namespace aaudio {

class AAudioBinderClient : public virtual android::RefBase
        , public AAudioServiceInterface
        , public android::Singleton<AAudioBinderClient> {

public:

    AAudioBinderClient();

    virtual ~AAudioBinderClient();

    const android::sp<android::IAAudioService> getAAudioService();

    void dropAAudioService();

    void registerClient(const android::sp<android::IAAudioClient>& client __unused) override {}

    /**
     * @param request info needed to create the stream
     * @param configuration contains resulting information about the created stream
     * @return handle to the stream or a negative error
     */
    aaudio_handle_t openStream(const AAudioStreamRequest &request,
                               AAudioStreamConfiguration &configurationOutput) override;

    aaudio_result_t closeStream(aaudio_handle_t streamHandle) override;

    /* Get an immutable description of the in-memory queues
    * used to communicate with the underlying HAL or Service.
    */
    aaudio_result_t getStreamDescription(aaudio_handle_t streamHandle,
                                                 AudioEndpointParcelable &parcelable) override;

    /**
     * Start the flow of data.
     * This is asynchronous. When complete, the service will send a STARTED event.
     */
    aaudio_result_t startStream(aaudio_handle_t streamHandle) override;

    /**
     * Stop the flow of data such that start() can resume without loss of data.
     * This is asynchronous. When complete, the service will send a PAUSED event.
     */
    aaudio_result_t pauseStream(aaudio_handle_t streamHandle) override;

    aaudio_result_t stopStream(aaudio_handle_t streamHandle) override;

    /**
     *  Discard any data held by the underlying HAL or Service.
     * This is asynchronous. When complete, the service will send a FLUSHED event.
     */
    aaudio_result_t flushStream(aaudio_handle_t streamHandle) override;

    /**
     * Manage the specified thread as a low latency audio thread.
     * TODO Consider passing this information as part of the startStream() call.
     */
    aaudio_result_t registerAudioThread(aaudio_handle_t streamHandle,
                                                pid_t clientThreadId,
                                                int64_t periodNanoseconds) override;

    aaudio_result_t unregisterAudioThread(aaudio_handle_t streamHandle,
                                                  pid_t clientThreadId) override;

    aaudio_result_t startClient(aaudio_handle_t streamHandle __unused,
                                      const android::AudioClient& client __unused,
                                      audio_port_handle_t *clientHandle) override {
        return AAUDIO_ERROR_UNAVAILABLE;
    }

    aaudio_result_t stopClient(aaudio_handle_t streamHandle __unused,
                               audio_port_handle_t clientHandle __unused)  override {
        return AAUDIO_ERROR_UNAVAILABLE;
    }

    void onStreamChange(aaudio_handle_t handle, int32_t opcode, int32_t value) {
        // TODO This is just a stub so we can have a client Binder to pass to the service.
        // TODO Implemented in a later CL.
        ALOGW("onStreamChange called!");
    }

    class AAudioClient : public android::IBinder::DeathRecipient , public android::BnAAudioClient
    {
    public:
        AAudioClient(android::wp<AAudioBinderClient> aaudioBinderClient)
                : mBinderClient(aaudioBinderClient) {
        }

        // implement DeathRecipient
        virtual void binderDied(const android::wp<android::IBinder>& who __unused) {
            android::sp<AAudioBinderClient> client = mBinderClient.promote();
            if (client.get() != nullptr) {
                client->dropAAudioService();
            }
            ALOGW("AAudio service binderDied()!");
        }

        // implement BnAAudioClient
        void onStreamChange(aaudio_handle_t handle, int32_t opcode, int32_t value) {
            android::sp<AAudioBinderClient> client = mBinderClient.promote();
            if (client.get() != nullptr) {
                client->onStreamChange(handle, opcode, value);
            }
        }
    private:
        android::wp<AAudioBinderClient> mBinderClient;
    };

private:

    android::Mutex                  mServiceLock;
    android::sp<android::IAAudioService>  mAAudioService;
    android::sp<AAudioClient>       mAAudioClient;

};


} /* namespace aaudio */

#endif //ANDROID_AAUDIO_AAUDIO_BINDER_CLIENT_H
