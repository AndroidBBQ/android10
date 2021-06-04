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

#ifndef AAUDIO_SERVICE_ENDPOINT_PLAY_H
#define AAUDIO_SERVICE_ENDPOINT_PLAY_H

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
 * Contains a mixer and a stream for writing the result of the mix.
 */
class AAudioServiceEndpointPlay : public AAudioServiceEndpointShared {
public:
    explicit AAudioServiceEndpointPlay(android::AAudioService &audioService);

    aaudio_result_t open(const aaudio::AAudioStreamRequest &request) override;

    void *callbackLoop() override;

private:
    AudioStreamInternalPlay  mStreamInternalPlay; // for playing output of mixer
    bool                     mLatencyTuningEnabled = false; // TODO implement tuning
    AAudioMixer              mMixer;    //
};

} /* namespace aaudio */

#endif //AAUDIO_SERVICE_ENDPOINT_PLAY_H
