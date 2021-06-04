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

#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include "AudioIODescriptorInterface.h"
#include "AudioPort.h"
#include "ClientDescriptor.h"
#include "DeviceDescriptor.h"
#include "EffectDescriptor.h"
#include "IOProfile.h"

namespace android {

class AudioPolicyMix;
class AudioPolicyClientInterface;

// descriptor for audio inputs. Used to maintain current configuration of each opened audio input
// and keep track of the usage of this input.
class AudioInputDescriptor: public AudioPortConfig, public AudioIODescriptorInterface
    , public ClientMapHandler<RecordClientDescriptor>
{
public:
    explicit AudioInputDescriptor(const sp<IOProfile>& profile,
                                  AudioPolicyClientInterface *clientInterface);
    audio_port_handle_t getId() const;
    audio_module_handle_t getModuleHandle() const;

    audio_devices_t getDeviceType() const { return (mDevice != nullptr) ?
                    mDevice->type() : AUDIO_DEVICE_NONE; }
    sp<DeviceDescriptor> getDevice() const { return mDevice; }
    void setDevice(const sp<DeviceDescriptor> &device) { mDevice = device; }
    DeviceVector supportedDevices() const  {
        return mProfile != nullptr ? mProfile->getSupportedDevices() :  DeviceVector(); }

    void dump(String8 *dst) const override;

    audio_io_handle_t   mIoHandle = AUDIO_IO_HANDLE_NONE; // input handle
    wp<AudioPolicyMix>  mPolicyMix;                   // non NULL when used by a dynamic policy
    const sp<IOProfile> mProfile;                     // I/O profile this output derives from

    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
            const struct audio_port_config *srcConfig = NULL) const;
    virtual sp<AudioPort> getAudioPort() const { return mProfile; }
    void toAudioPort(struct audio_port *port) const;
    void setPreemptedSessions(const SortedVector<audio_session_t>& sessions);
    SortedVector<audio_session_t> getPreemptedSessions() const;
    bool hasPreemptedSession(audio_session_t session) const;
    void clearPreemptedSessions();
    bool isActive() const { return mGlobalActiveCount > 0; }
    bool isSourceActive(audio_source_t source) const;
    audio_source_t source() const;
    bool isSoundTrigger() const;
    sp<RecordClientDescriptor> getHighestPriorityClient() const;
    audio_attributes_t getHighestPriorityAttributes() const;
    void setClientActive(const sp<RecordClientDescriptor>& client, bool active);
    int32_t activeCount() { return mGlobalActiveCount; }
    void trackEffectEnabled(const sp<EffectDescriptor> &effect, bool enabled);
    EffectDescriptorCollection getEnabledEffects() const;
    EffectDescriptorCollection getActiveEffects() const; // enabled and not suspended
    // implementation of AudioIODescriptorInterface
    audio_config_base_t getConfig() const override;
    audio_patch_handle_t getPatchHandle() const override;
    void setPatchHandle(audio_patch_handle_t handle) override;

    status_t open(const audio_config_t *config,
                  const sp<DeviceDescriptor> &device,
                  audio_source_t source,
                  audio_input_flags_t flags,
                  audio_io_handle_t *input);
    // Called when a stream is about to be started.
    // Note: called after setClientActive(client, true)
    status_t start();
    // Called after a stream is stopped
    // Note: called after setClientActive(client, false)
    void stop();
    void close();

    RecordClientVector getClientsForSession(audio_session_t session);
    RecordClientVector clientsList(bool activeOnly = false,
        audio_source_t source = AUDIO_SOURCE_DEFAULT, bool preferredDeviceOnly = false) const;

    void setAppState(uid_t uid, app_state_t state);

    // implementation of ClientMapHandler<RecordClientDescriptor>
    void addClient(const sp<RecordClientDescriptor> &client) override;

    // Go over all active clients and suspend or restore effects according highest priority
    // active use case
    void checkSuspendEffects();

 private:

    void updateClientRecordingConfiguration(int event, const sp<RecordClientDescriptor>& client);

    audio_patch_handle_t mPatchHandle = AUDIO_PATCH_HANDLE_NONE;
    audio_port_handle_t  mId = AUDIO_PORT_HANDLE_NONE;
    sp<DeviceDescriptor> mDevice = nullptr; /**< current device this input is routed to */

    // Because a preemptible capture session can preempt another one, we end up in an endless loop
    // situation were each session is allowed to restart after being preempted,
    // thus preempting the other one which restarts and so on.
    // To avoid this situation, we store which audio session was preempted when
    // a particular input started and prevent preemption of this active input by this session.
    // We also inherit sessions from the preempted input to avoid a 3 way preemption loop etc...
    SortedVector<audio_session_t> mPreemptedSessions;
    AudioPolicyClientInterface * const mClientInterface;
    int32_t mGlobalActiveCount = 0;  // non-client-specific activity ref count
    EffectDescriptorCollection mEnabledEffects;
};

class AudioInputCollection :
        public DefaultKeyedVector< audio_io_handle_t, sp<AudioInputDescriptor> >
{
public:
    bool isSourceActive(audio_source_t source) const;

    sp<AudioInputDescriptor> getInputFromId(audio_port_handle_t id) const;

    // count active capture sessions using one of the specified devices.
    // ignore devices if empty vector is passed
    uint32_t activeInputsCountOnDevices(const DeviceVector &devices) const;

    /**
     * return io handle of active input or 0 if no input is active
     * Only considers inputs from physical devices (e.g. main mic, headset mic) when
     * ignoreVirtualInputs is true.
     */
    Vector<sp <AudioInputDescriptor> > getActiveInputs();

    sp<AudioInputDescriptor> getInputForClient(audio_port_handle_t portId);

    void trackEffectEnabled(const sp<EffectDescriptor> &effect, bool enabled);

    /**
    * @brief clearSessionRoutesForDevice: when a device is disconnected, and if this device has
    * been chosen as the preferred device by any client, the policy manager shall
    * prevent from using this device any more by clearing all the session routes involving this
    * device.
    * In other words, the preferred device port id of these clients will be resetted to NONE.
    * @param disconnectedDevice device to be disconnected
    */
    void clearSessionRoutesForDevice(const sp<DeviceDescriptor> &disconnectedDevice);

    void dump(String8 *dst) const;
};


} // namespace android
