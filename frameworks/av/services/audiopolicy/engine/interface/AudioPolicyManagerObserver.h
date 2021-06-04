/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#include <AudioGain.h>
#include <AudioPort.h>
#include <AudioPatch.h>
#include <IOProfile.h>
#include <DeviceDescriptor.h>
#include <AudioInputDescriptor.h>
#include <AudioOutputDescriptor.h>
#include <AudioPolicyMix.h>
#include <SoundTriggerSession.h>

namespace android {

/**
 * This interface is an observer that the manager shall implement to allows e.g. the engine
 * to access to policy pillars elements (like output / input descritors collections,
 * HwModule collections, AudioMix, ...
 */
class AudioPolicyManagerObserver
{
public:
    virtual const AudioPatchCollection &getAudioPatches() const = 0;

    virtual const SoundTriggerSessionCollection &getSoundTriggerSessionCollection() const = 0;

    virtual const AudioPolicyMixCollection &getAudioPolicyMixCollection() const = 0;

    virtual const SwAudioOutputCollection &getOutputs() const = 0;

    virtual const AudioInputCollection &getInputs() const = 0;

    virtual const DeviceVector getAvailableOutputDevices() const = 0;

    virtual const DeviceVector getAvailableInputDevices() const = 0;

    virtual const sp<DeviceDescriptor> &getDefaultOutputDevice() const = 0;

protected:
    virtual ~AudioPolicyManagerObserver() {}
};

} // namespace android
