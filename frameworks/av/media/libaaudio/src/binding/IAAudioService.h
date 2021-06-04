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

#ifndef ANDROID_AAUDIO_IAAUDIO_SERVICE_H
#define ANDROID_AAUDIO_IAAUDIO_SERVICE_H

#include <stdint.h>
#include <utils/RefBase.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>

#include <aaudio/AAudio.h>

#include "binding/AAudioCommon.h"
#include "binding/AAudioServiceDefinitions.h"
#include "binding/AAudioStreamConfiguration.h"
#include "binding/AAudioStreamRequest.h"
#include "binding/AudioEndpointParcelable.h"
#include "binding/IAAudioClient.h"

namespace android {

#define AAUDIO_SERVICE_NAME  "media.aaudio"

// Interface (our AIDL) - service methods called by client
class IAAudioService : public IInterface {
public:

    DECLARE_META_INTERFACE(AAudioService);

    // Register an object to receive audio input/output change and track notifications.
    // For a given calling pid, AAudio service disregards any registrations after the first.
    // Thus the IAAudioClient must be a singleton per process.
    virtual void registerClient(const sp<IAAudioClient>& client) = 0;

    /**
     * @param request info needed to create the stream
     * @param configuration contains information about the created stream
     * @return handle to the stream or a negative error
     */
    virtual aaudio::aaudio_handle_t openStream(const aaudio::AAudioStreamRequest &request,
                                     aaudio::AAudioStreamConfiguration &configurationOutput) = 0;

    virtual aaudio_result_t closeStream(aaudio::aaudio_handle_t streamHandle) = 0;

    /* Get an immutable description of the in-memory queues
    * used to communicate with the underlying HAL or Service.
    */
    virtual aaudio_result_t getStreamDescription(aaudio::aaudio_handle_t streamHandle,
                                               aaudio::AudioEndpointParcelable &parcelable) = 0;

    /**
     * Start the flow of data.
     * This is asynchronous. When complete, the service will send a STARTED event.
     */
    virtual aaudio_result_t startStream(aaudio::aaudio_handle_t streamHandle) = 0;

    /**
     * Stop the flow of data such that start() can resume without loss of data.
     * This is asynchronous. When complete, the service will send a PAUSED event.
     */
    virtual aaudio_result_t pauseStream(aaudio::aaudio_handle_t streamHandle) = 0;

    /**
     * Stop the flow of data such that the data currently in the buffer is played.
     * This is asynchronous. When complete, the service will send a STOPPED event.
     */
    virtual aaudio_result_t stopStream(aaudio::aaudio_handle_t streamHandle) = 0;

    /**
     *  Discard any data held by the underlying HAL or Service.
     * This is asynchronous. When complete, the service will send a FLUSHED event.
     */
    virtual aaudio_result_t flushStream(aaudio::aaudio_handle_t streamHandle) = 0;

    /**
     * Manage the specified thread as a low latency audio thread.
     */
    virtual aaudio_result_t registerAudioThread(aaudio::aaudio_handle_t streamHandle,
                                              pid_t clientThreadId,
                                              int64_t periodNanoseconds) = 0;

    virtual aaudio_result_t unregisterAudioThread(aaudio::aaudio_handle_t streamHandle,
                                                pid_t clientThreadId) = 0;
};

class BnAAudioService : public BnInterface<IAAudioService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
                                Parcel* reply, uint32_t flags = 0);

};

} /* namespace android */

#endif //ANDROID_AAUDIO_IAAUDIO_SERVICE_H
