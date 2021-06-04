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

#ifndef AAUDIO_AAUDIO_SERVICE_H
#define AAUDIO_AAUDIO_SERVICE_H

#include <time.h>
#include <pthread.h>

#include <binder/BinderService.h>
#include <media/AudioClient.h>

#include <aaudio/AAudio.h>

#include "binding/AAudioCommon.h"
#include "binding/AAudioServiceInterface.h"
#include "binding/IAAudioService.h"

#include "AAudioServiceStreamBase.h"
#include "AAudioStreamTracker.h"

namespace android {

class AAudioService :
    public BinderService<AAudioService>,
    public BnAAudioService,
    public aaudio::AAudioServiceInterface
{
    friend class BinderService<AAudioService>;

public:
    AAudioService();
    virtual ~AAudioService();

    static const char* getServiceName() { return AAUDIO_SERVICE_NAME; }

    virtual status_t        dump(int fd, const Vector<String16>& args) override;

    virtual void            registerClient(const sp<IAAudioClient>& client);

    aaudio::aaudio_handle_t openStream(const aaudio::AAudioStreamRequest &request,
                                       aaudio::AAudioStreamConfiguration &configurationOutput)
                                       override;

    aaudio_result_t closeStream(aaudio::aaudio_handle_t streamHandle) override;

    aaudio_result_t getStreamDescription(
                aaudio::aaudio_handle_t streamHandle,
                aaudio::AudioEndpointParcelable &parcelable) override;

    aaudio_result_t startStream(aaudio::aaudio_handle_t streamHandle) override;

    aaudio_result_t pauseStream(aaudio::aaudio_handle_t streamHandle) override;

    aaudio_result_t stopStream(aaudio::aaudio_handle_t streamHandle) override;

    aaudio_result_t flushStream(aaudio::aaudio_handle_t streamHandle) override;

    aaudio_result_t registerAudioThread(aaudio::aaudio_handle_t streamHandle,
                                                pid_t tid,
                                                int64_t periodNanoseconds) override;

    aaudio_result_t unregisterAudioThread(aaudio::aaudio_handle_t streamHandle,
                                                  pid_t tid) override;

    aaudio_result_t startClient(aaudio::aaudio_handle_t streamHandle,
                                      const android::AudioClient& client,
                                      audio_port_handle_t *clientHandle) override;

    aaudio_result_t stopClient(aaudio::aaudio_handle_t streamHandle,
                                       audio_port_handle_t clientHandle) override;

    aaudio_result_t disconnectStreamByPortHandle(audio_port_handle_t portHandle);

private:

    /**
     * Lookup stream and then validate access to the stream.
     * @param streamHandle
     * @return
     */
    sp<aaudio::AAudioServiceStreamBase> convertHandleToServiceStream(
            aaudio::aaudio_handle_t streamHandle);



    bool releaseStream(const sp<aaudio::AAudioServiceStreamBase> &serviceStream);

    aaudio_result_t checkForPendingClose(const sp<aaudio::AAudioServiceStreamBase> &serviceStream,
                                         aaudio_result_t defaultResult);

    android::AudioClient            mAudioClient;

    aaudio::AAudioStreamTracker     mStreamTracker;

    enum constants {
        DEFAULT_AUDIO_PRIORITY = 2
    };
};

} /* namespace android */

#endif //AAUDIO_AAUDIO_SERVICE_H
