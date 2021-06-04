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

#include "DeviceDescriptor.h"
#include <utils/RefBase.h>
#include <media/AudioPolicy.h>
#include <utils/Vector.h>
#include <system/audio.h>
#include <utils/String8.h>

#include <DeviceDescriptor.h>
#include <AudioOutputDescriptor.h>

namespace android {

/**
 * custom mix entry in mPolicyMixes
 */
class AudioPolicyMix : public AudioMix, public RefBase {
public:
    AudioPolicyMix(const AudioMix &mix) : AudioMix(mix) {}
    AudioPolicyMix(const AudioPolicyMix&) = delete;
    AudioPolicyMix& operator=(const AudioPolicyMix&) = delete;

    const sp<SwAudioOutputDescriptor> &getOutput() const { return mOutput; }
    void setOutput(const sp<SwAudioOutputDescriptor> &output) { mOutput = output; }
    void clearOutput() { mOutput.clear(); }

    void dump(String8 *dst, int spaces, int index) const;

private:
    sp<SwAudioOutputDescriptor> mOutput;  // Corresponding output stream
};


class AudioPolicyMixCollection : public Vector<sp<AudioPolicyMix>>
{
public:
    status_t getAudioPolicyMix(audio_devices_t deviceType,
            const String8& address, sp<AudioPolicyMix> &policyMix) const;

    status_t registerMix(AudioMix mix, sp<SwAudioOutputDescriptor> desc);

    status_t unregisterMix(const AudioMix& mix);

    void closeOutput(sp<SwAudioOutputDescriptor> &desc);

    /**
     * Try to find an output descriptor for the given attributes.
     *
     * @param[in] attributes to consider fowr the research of output descriptor.
     * @param[out] desc to return if an primary output could be found.
     * @param[out] secondaryDesc other desc that the audio should be routed to.
     * @return OK if the request is valid
     *         otherwise if the request is not supported
     */
    status_t getOutputForAttr(const audio_attributes_t& attributes, uid_t uid,
                              audio_output_flags_t flags,
                              sp<SwAudioOutputDescriptor> &primaryDesc,
                              std::vector<sp<SwAudioOutputDescriptor>> *secondaryDescs);

    sp<DeviceDescriptor> getDeviceAndMixForInputSource(audio_source_t inputSource,
                                                       const DeviceVector &availableDeviceTypes,
                                                       sp<AudioPolicyMix> *policyMix) const;

    /**
     * @brief try to find a matching mix for a given output descriptor and returns the associated
     * output device.
     * @param output to be considered
     * @param availableOutputDevices list of output devices currently reachable
     * @return device selected from the mix attached to the output, null pointer otherwise
     */
    sp<DeviceDescriptor> getDeviceAndMixForOutput(const sp<SwAudioOutputDescriptor> &output,
                                                  const DeviceVector &availableOutputDevices);

    status_t getInputMixForAttr(audio_attributes_t attr, sp<AudioPolicyMix> *policyMix);

    /**
     * Updates the mix rules in order to make streams associated with the given uid
     * be routed to the given audio devices.
     * @param uid the uid for which the device affinity is set
     * @param devices the vector of devices that this uid may be routed to. A typical
     *    use is to pass the devices associated with a given zone in a multi-zone setup.
     * @return NO_ERROR if the update was successful, INVALID_OPERATION otherwise.
     *    An example of failure is when there are already rules in place to restrict
     *    a mix to the given uid (i.e. when a MATCH_UID rule was set for it).
     */
    status_t setUidDeviceAffinities(uid_t uid, const Vector<AudioDeviceTypeAddr>& devices);
    status_t removeUidDeviceAffinities(uid_t uid);
    status_t getDevicesForUid(uid_t uid, Vector<AudioDeviceTypeAddr>& devices) const;

    void dump(String8 *dst) const;

private:
    enum class MixMatchStatus { MATCH, NO_MATCH, INVALID_MIX };
    MixMatchStatus mixMatch(const AudioMix* mix, size_t mixIndex,
                            const audio_attributes_t& attributes, uid_t uid);
};

} // namespace android
