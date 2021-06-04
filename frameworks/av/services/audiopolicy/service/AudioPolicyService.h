/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_AUDIOPOLICYSERVICE_H
#define ANDROID_AUDIOPOLICYSERVICE_H

#include <cutils/misc.h>
#include <cutils/config_utils.h>
#include <cutils/compiler.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>
#include <binder/BinderService.h>
#include <binder/IUidObserver.h>
#include <system/audio.h>
#include <system/audio_policy.h>
#include <media/IAudioPolicyService.h>
#include <media/ToneGenerator.h>
#include <media/AudioEffect.h>
#include <media/AudioPolicy.h>
#include <mediautils/ServiceUtilities.h>
#include "AudioPolicyEffects.h"
#include <AudioPolicyInterface.h>
#include <android/hardware/BnSensorPrivacyListener.h>

#include <unordered_map>

namespace android {

// ----------------------------------------------------------------------------

class AudioPolicyService :
    public BinderService<AudioPolicyService>,
    public BnAudioPolicyService,
    public IBinder::DeathRecipient
{
    friend class BinderService<AudioPolicyService>;

public:
    // for BinderService
    static const char *getServiceName() ANDROID_API { return "media.audio_policy"; }

    virtual status_t    dump(int fd, const Vector<String16>& args);

    //
    // BnAudioPolicyService (see AudioPolicyInterface for method descriptions)
    //

    virtual status_t setDeviceConnectionState(audio_devices_t device,
                                              audio_policy_dev_state_t state,
                                              const char *device_address,
                                              const char *device_name,
                                              audio_format_t encodedFormat);
    virtual audio_policy_dev_state_t getDeviceConnectionState(
                                                                audio_devices_t device,
                                                                const char *device_address);
    virtual status_t handleDeviceConfigChange(audio_devices_t device,
                                              const char *device_address,
                                              const char *device_name,
                                              audio_format_t encodedFormat);
    virtual status_t setPhoneState(audio_mode_t state);
    virtual status_t setForceUse(audio_policy_force_use_t usage, audio_policy_forced_cfg_t config);
    virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage);
    virtual audio_io_handle_t getOutput(audio_stream_type_t stream);
    status_t getOutputForAttr(audio_attributes_t *attr,
                              audio_io_handle_t *output,
                              audio_session_t session,
                              audio_stream_type_t *stream,
                              pid_t pid,
                              uid_t uid,
                              const audio_config_t *config,
                              audio_output_flags_t flags,
                              audio_port_handle_t *selectedDeviceId,
                              audio_port_handle_t *portId,
                              std::vector<audio_io_handle_t> *secondaryOutputs) override;
    virtual status_t startOutput(audio_port_handle_t portId);
    virtual status_t stopOutput(audio_port_handle_t portId);
    virtual void releaseOutput(audio_port_handle_t portId);
    virtual status_t getInputForAttr(const audio_attributes_t *attr,
                                     audio_io_handle_t *input,
                                     audio_unique_id_t riid,
                                     audio_session_t session,
                                     pid_t pid,
                                     uid_t uid,
                                     const String16& opPackageName,
                                     const audio_config_base_t *config,
                                     audio_input_flags_t flags,
                                     audio_port_handle_t *selectedDeviceId = NULL,
                                     audio_port_handle_t *portId = NULL);
    virtual status_t startInput(audio_port_handle_t portId);
    virtual status_t stopInput(audio_port_handle_t portId);
    virtual void releaseInput(audio_port_handle_t portId);
    virtual status_t initStreamVolume(audio_stream_type_t stream,
                                      int indexMin,
                                      int indexMax);
    virtual status_t setStreamVolumeIndex(audio_stream_type_t stream,
                                          int index,
                                          audio_devices_t device);
    virtual status_t getStreamVolumeIndex(audio_stream_type_t stream,
                                          int *index,
                                          audio_devices_t device);

    virtual status_t setVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                 int index,
                                                 audio_devices_t device);
    virtual status_t getVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                 int &index,
                                                 audio_devices_t device);
    virtual status_t getMinVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                    int &index);
    virtual status_t getMaxVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                    int &index);

    virtual uint32_t getStrategyForStream(audio_stream_type_t stream);
    virtual audio_devices_t getDevicesForStream(audio_stream_type_t stream);

    virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc);
    virtual status_t registerEffect(const effect_descriptor_t *desc,
                                    audio_io_handle_t io,
                                    uint32_t strategy,
                                    audio_session_t session,
                                    int id);
    virtual status_t unregisterEffect(int id);
    virtual status_t setEffectEnabled(int id, bool enabled);
    status_t moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io) override;
    virtual bool isStreamActive(audio_stream_type_t stream, uint32_t inPastMs = 0) const;
    virtual bool isStreamActiveRemotely(audio_stream_type_t stream, uint32_t inPastMs = 0) const;
    virtual bool isSourceActive(audio_source_t source) const;

    virtual status_t queryDefaultPreProcessing(audio_session_t audioSession,
                                              effect_descriptor_t *descriptors,
                                              uint32_t *count);
    virtual status_t addSourceDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_source_t source,
                                            audio_unique_id_t* id);
    virtual status_t addStreamDefaultEffect(const effect_uuid_t *type,
                                            const String16& opPackageName,
                                            const effect_uuid_t *uuid,
                                            int32_t priority,
                                            audio_usage_t usage,
                                            audio_unique_id_t* id);
    virtual status_t removeSourceDefaultEffect(audio_unique_id_t id);
    virtual status_t removeStreamDefaultEffect(audio_unique_id_t id);

    virtual     status_t    onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags);

    // IBinder::DeathRecipient
    virtual     void        binderDied(const wp<IBinder>& who);

    // RefBase
    virtual     void        onFirstRef();

    //
    // Helpers for the struct audio_policy_service_ops implementation.
    // This is used by the audio policy manager for certain operations that
    // are implemented by the policy service.
    //
    virtual void setParameters(audio_io_handle_t ioHandle,
                               const char *keyValuePairs,
                               int delayMs);

    virtual status_t setStreamVolume(audio_stream_type_t stream,
                                     float volume,
                                     audio_io_handle_t output,
                                     int delayMs = 0);
    virtual status_t setVoiceVolume(float volume, int delayMs = 0);
    status_t setAllowedCapturePolicy(uint_t uid, audio_flags_mask_t capturePolicy) override;
    virtual bool isOffloadSupported(const audio_offload_info_t &config);
    virtual bool isDirectOutputSupported(const audio_config_base_t& config,
                                         const audio_attributes_t& attributes);

    virtual status_t listAudioPorts(audio_port_role_t role,
                                    audio_port_type_t type,
                                    unsigned int *num_ports,
                                    struct audio_port *ports,
                                    unsigned int *generation);
    virtual status_t getAudioPort(struct audio_port *port);
    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle);
    virtual status_t releaseAudioPatch(audio_patch_handle_t handle);
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches,
                                      unsigned int *generation);
    virtual status_t setAudioPortConfig(const struct audio_port_config *config);

    virtual void registerClient(const sp<IAudioPolicyServiceClient>& client);

    virtual void setAudioPortCallbacksEnabled(bool enabled);

    virtual void setAudioVolumeGroupCallbacksEnabled(bool enabled);

    virtual status_t acquireSoundTriggerSession(audio_session_t *session,
                                           audio_io_handle_t *ioHandle,
                                           audio_devices_t *device);

    virtual status_t releaseSoundTriggerSession(audio_session_t session);

    virtual audio_mode_t getPhoneState();

    virtual status_t registerPolicyMixes(const Vector<AudioMix>& mixes, bool registration);

    virtual status_t setUidDeviceAffinities(uid_t uid, const Vector<AudioDeviceTypeAddr>& devices);

    virtual status_t removeUidDeviceAffinities(uid_t uid);

    virtual status_t startAudioSource(const struct audio_port_config *source,
                                      const audio_attributes_t *attributes,
                                      audio_port_handle_t *portId);
    virtual status_t stopAudioSource(audio_port_handle_t portId);

    virtual status_t setMasterMono(bool mono);
    virtual status_t getMasterMono(bool *mono);

    virtual float    getStreamVolumeDB(
                audio_stream_type_t stream, int index, audio_devices_t device);

    virtual status_t getSurroundFormats(unsigned int *numSurroundFormats,
                                        audio_format_t *surroundFormats,
                                        bool *surroundFormatsEnabled,
                                        bool reported);
    virtual status_t getHwOffloadEncodingFormatsSupportedForA2DP(
                                        std::vector<audio_format_t> *formats);
    virtual status_t setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled);

    virtual status_t setAssistantUid(uid_t uid);
    virtual status_t setA11yServicesUids(const std::vector<uid_t>& uids);

    virtual bool     isHapticPlaybackSupported();

    virtual status_t listAudioProductStrategies(AudioProductStrategyVector &strategies);
    virtual status_t getProductStrategyFromAudioAttributes(const AudioAttributes &aa,
                                                           product_strategy_t &productStrategy);

    virtual status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups);

    virtual status_t getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                       volume_group_t &volumeGroup);

    virtual status_t setRttEnabled(bool enabled);

            status_t doStopOutput(audio_port_handle_t portId);
            void doReleaseOutput(audio_port_handle_t portId);

            status_t clientCreateAudioPatch(const struct audio_patch *patch,
                                      audio_patch_handle_t *handle,
                                      int delayMs);
            status_t clientReleaseAudioPatch(audio_patch_handle_t handle,
                                             int delayMs);
            virtual status_t clientSetAudioPortConfig(const struct audio_port_config *config,
                                                      int delayMs);

            void removeNotificationClient(uid_t uid, pid_t pid);
            void onAudioPortListUpdate();
            void doOnAudioPortListUpdate();
            void onAudioPatchListUpdate();
            void doOnAudioPatchListUpdate();

            void onDynamicPolicyMixStateUpdate(const String8& regId, int32_t state);
            void doOnDynamicPolicyMixStateUpdate(const String8& regId, int32_t state);
            void onRecordingConfigurationUpdate(int event,
                                                const record_client_info_t *clientInfo,
                                                const audio_config_base_t *clientConfig,
                                                std::vector<effect_descriptor_t> clientEffects,
                                                const audio_config_base_t *deviceConfig,
                                                std::vector<effect_descriptor_t> effects,
                                                audio_patch_handle_t patchHandle,
                                                audio_source_t source);
            void doOnRecordingConfigurationUpdate(int event,
                                                  const record_client_info_t *clientInfo,
                                                  const audio_config_base_t *clientConfig,
                                                  std::vector<effect_descriptor_t> clientEffects,
                                                  const audio_config_base_t *deviceConfig,
                                                  std::vector<effect_descriptor_t> effects,
                                                  audio_patch_handle_t patchHandle,
                                                  audio_source_t source);

            void onAudioVolumeGroupChanged(volume_group_t group, int flags);
            void doOnAudioVolumeGroupChanged(volume_group_t group, int flags);
            void setEffectSuspended(int effectId,
                                    audio_session_t sessionId,
                                    bool suspended);

private:
                        AudioPolicyService() ANDROID_API;
    virtual             ~AudioPolicyService();

            status_t dumpInternals(int fd);

    // Handles binder shell commands
    virtual status_t shellCommand(int in, int out, int err, Vector<String16>& args);

    // Sets whether the given UID records only silence
    virtual void setAppState_l(uid_t uid, app_state_t state);

    // Overrides the UID state as if it is idle
    status_t handleSetUidState(Vector<String16>& args, int err);

    // Clears the override for the UID state
    status_t handleResetUidState(Vector<String16>& args, int err);

    // Gets the UID state
    status_t handleGetUidState(Vector<String16>& args, int out, int err);

    // Prints the shell command help
    status_t printHelp(int out);

    std::string getDeviceTypeStrForPortId(audio_port_handle_t portId);

    status_t getAudioPolicyEffects(sp<AudioPolicyEffects>& audioPolicyEffects);

    app_state_t apmStatFromAmState(int amState);

    void updateUidStates();
    void updateUidStates_l();

    void silenceAllRecordings_l();

    static bool isPrivacySensitiveSource(audio_source_t source);
    static bool isVirtualSource(audio_source_t source);

    // If recording we need to make sure the UID is allowed to do that. If the UID is idle
    // then it cannot record and gets buffers with zeros - silence. As soon as the UID
    // transitions to an active state we will start reporting buffers with data. This approach
    // transparently handles recording while the UID transitions between idle/active state
    // avoiding to get stuck in a state receiving non-empty buffers while idle or in a state
    // receiving empty buffers while active.
    class UidPolicy : public BnUidObserver, public virtual IBinder::DeathRecipient {
    public:
        explicit UidPolicy(wp<AudioPolicyService> service)
                : mService(service), mObserverRegistered(false),
                  mAssistantUid(0), mRttEnabled(false) {}

        void registerSelf();
        void unregisterSelf();

        // IBinder::DeathRecipient implementation
        void binderDied(const wp<IBinder> &who) override;

        bool isUidActive(uid_t uid);
        int getUidState(uid_t uid);
        void setAssistantUid(uid_t uid) { mAssistantUid = uid; }
        bool isAssistantUid(uid_t uid) { return uid == mAssistantUid; }
        void setA11yUids(const std::vector<uid_t>& uids) { mA11yUids.clear(); mA11yUids = uids; }
        bool isA11yUid(uid_t uid);
        bool isA11yOnTop();
        void setRttEnabled(bool enabled) { mRttEnabled = enabled; }
        bool isRttEnabled() { return mRttEnabled; }

        // BnUidObserver implementation
        void onUidActive(uid_t uid) override;
        void onUidGone(uid_t uid, bool disabled) override;
        void onUidIdle(uid_t uid, bool disabled) override;
        void onUidStateChanged(uid_t uid, int32_t procState, int64_t procStateSeq);

        void addOverrideUid(uid_t uid, bool active) { updateOverrideUid(uid, active, true); }
        void removeOverrideUid(uid_t uid) { updateOverrideUid(uid, false, false); }

        void updateUid(std::unordered_map<uid_t, std::pair<bool, int>> *uids,
                       uid_t uid, bool active, int state, bool insert);

     private:
        void notifyService();
        void updateOverrideUid(uid_t uid, bool active, bool insert);
        void updateUidLocked(std::unordered_map<uid_t, std::pair<bool, int>> *uids,
                             uid_t uid, bool active, int state, bool insert);
        void checkRegistered();

        wp<AudioPolicyService> mService;
        Mutex mLock;
        bool mObserverRegistered;
        std::unordered_map<uid_t, std::pair<bool, int>> mOverrideUids;
        std::unordered_map<uid_t, std::pair<bool, int>> mCachedUids;
        uid_t mAssistantUid;
        std::vector<uid_t> mA11yUids;
        bool mRttEnabled;
    };

    // If sensor privacy is enabled then all apps, including those that are active, should be
    // prevented from recording. This is handled similar to idle UIDs, any app that attempts
    // to record while sensor privacy is enabled will receive buffers with zeros. As soon as
    // sensor privacy is disabled active apps will receive the expected data when recording.
    class SensorPrivacyPolicy : public hardware::BnSensorPrivacyListener {
        public:
            explicit SensorPrivacyPolicy(wp<AudioPolicyService> service)
                    : mService(service) {}

            void registerSelf();
            void unregisterSelf();

            bool isSensorPrivacyEnabled();

            binder::Status onSensorPrivacyChanged(bool enabled);

        private:
            wp<AudioPolicyService> mService;
            std::atomic_bool mSensorPrivacyEnabled;
    };

    // Thread used to send audio config commands to audio flinger
    // For audio config commands, it is necessary because audio flinger requires that the calling
    // process (user) has permission to modify audio settings.
    class AudioCommandThread : public Thread {
        class AudioCommand;
    public:

        // commands for tone AudioCommand
        enum {
            SET_VOLUME,
            SET_PARAMETERS,
            SET_VOICE_VOLUME,
            STOP_OUTPUT,
            RELEASE_OUTPUT,
            CREATE_AUDIO_PATCH,
            RELEASE_AUDIO_PATCH,
            UPDATE_AUDIOPORT_LIST,
            UPDATE_AUDIOPATCH_LIST,
            CHANGED_AUDIOVOLUMEGROUP,
            SET_AUDIOPORT_CONFIG,
            DYN_POLICY_MIX_STATE_UPDATE,
            RECORDING_CONFIGURATION_UPDATE,
            SET_EFFECT_SUSPENDED,
        };

        AudioCommandThread (String8 name, const wp<AudioPolicyService>& service);
        virtual             ~AudioCommandThread();

                    status_t    dump(int fd);

        // Thread virtuals
        virtual     void        onFirstRef();
        virtual     bool        threadLoop();

                    void        exit();
                    status_t    volumeCommand(audio_stream_type_t stream, float volume,
                                            audio_io_handle_t output, int delayMs = 0);
                    status_t    parametersCommand(audio_io_handle_t ioHandle,
                                            const char *keyValuePairs, int delayMs = 0);
                    status_t    voiceVolumeCommand(float volume, int delayMs = 0);
                    void        stopOutputCommand(audio_port_handle_t portId);
                    void        releaseOutputCommand(audio_port_handle_t portId);
                    status_t    sendCommand(sp<AudioCommand>& command, int delayMs = 0);
                    void        insertCommand_l(sp<AudioCommand>& command, int delayMs = 0);
                    status_t    createAudioPatchCommand(const struct audio_patch *patch,
                                                        audio_patch_handle_t *handle,
                                                        int delayMs);
                    status_t    releaseAudioPatchCommand(audio_patch_handle_t handle,
                                                         int delayMs);
                    void        updateAudioPortListCommand();
                    void        updateAudioPatchListCommand();
                    void        changeAudioVolumeGroupCommand(volume_group_t group, int flags);
                    status_t    setAudioPortConfigCommand(const struct audio_port_config *config,
                                                          int delayMs);
                    void        dynamicPolicyMixStateUpdateCommand(const String8& regId,
                                                                   int32_t state);
                    void        recordingConfigurationUpdateCommand(
                                                    int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source);
                    void        setEffectSuspendedCommand(int effectId,
                                                          audio_session_t sessionId,
                                                          bool suspended);
                    void        insertCommand_l(AudioCommand *command, int delayMs = 0);
    private:
        class AudioCommandData;

        // descriptor for requested tone playback event
        class AudioCommand: public RefBase {

        public:
            AudioCommand()
            : mCommand(-1), mStatus(NO_ERROR), mWaitStatus(false) {}

            void dump(char* buffer, size_t size);

            int mCommand;   // SET_VOLUME, SET_PARAMETERS...
            nsecs_t mTime;  // time stamp
            Mutex mLock;    // mutex associated to mCond
            Condition mCond; // condition for status return
            status_t mStatus; // command status
            bool mWaitStatus; // true if caller is waiting for status
            sp<AudioCommandData> mParam;     // command specific parameter data
        };

        class AudioCommandData: public RefBase {
        public:
            virtual ~AudioCommandData() {}
        protected:
            AudioCommandData() {}
        };

        class VolumeData : public AudioCommandData {
        public:
            audio_stream_type_t mStream;
            float mVolume;
            audio_io_handle_t mIO;
        };

        class ParametersData : public AudioCommandData {
        public:
            audio_io_handle_t mIO;
            String8 mKeyValuePairs;
        };

        class VoiceVolumeData : public AudioCommandData {
        public:
            float mVolume;
        };

        class StopOutputData : public AudioCommandData {
        public:
            audio_port_handle_t mPortId;
        };

        class ReleaseOutputData : public AudioCommandData {
        public:
            audio_port_handle_t mPortId;
        };

        class CreateAudioPatchData : public AudioCommandData {
        public:
            struct audio_patch mPatch;
            audio_patch_handle_t mHandle;
        };

        class ReleaseAudioPatchData : public AudioCommandData {
        public:
            audio_patch_handle_t mHandle;
        };

        class AudioVolumeGroupData : public AudioCommandData {
        public:
            volume_group_t mGroup;
            int mFlags;
        };

        class SetAudioPortConfigData : public AudioCommandData {
        public:
            struct audio_port_config mConfig;
        };

        class DynPolicyMixStateUpdateData : public AudioCommandData {
        public:
            String8 mRegId;
            int32_t mState;
        };

        class RecordingConfigurationUpdateData : public AudioCommandData {
        public:
            int mEvent;
            record_client_info_t mClientInfo;
            struct audio_config_base mClientConfig;
            std::vector<effect_descriptor_t> mClientEffects;
            struct audio_config_base mDeviceConfig;
            std::vector<effect_descriptor_t> mEffects;
            audio_patch_handle_t mPatchHandle;
            audio_source_t mSource;
        };

        class SetEffectSuspendedData : public AudioCommandData {
        public:
            int mEffectId;
            audio_session_t mSessionId;
            bool mSuspended;
        };

        Mutex   mLock;
        Condition mWaitWorkCV;
        Vector < sp<AudioCommand> > mAudioCommands; // list of pending commands
        sp<AudioCommand> mLastCommand;      // last processed command (used by dump)
        String8 mName;                      // string used by wake lock fo delayed commands
        wp<AudioPolicyService> mService;
    };

    class AudioPolicyClient : public AudioPolicyClientInterface
    {
     public:
        explicit AudioPolicyClient(AudioPolicyService *service) : mAudioPolicyService(service) {}
        virtual ~AudioPolicyClient() {}

        //
        // Audio HW module functions
        //

        // loads a HW module.
        virtual audio_module_handle_t loadHwModule(const char *name);

        //
        // Audio output Control functions
        //

        // opens an audio output with the requested parameters. The parameter values can indicate to use the default values
        // in case the audio policy manager has no specific requirements for the output being opened.
        // When the function returns, the parameter values reflect the actual values used by the audio hardware output stream.
        // The audio policy manager can check if the proposed parameters are suitable or not and act accordingly.
        virtual status_t openOutput(audio_module_handle_t module,
                                    audio_io_handle_t *output,
                                    audio_config_t *config,
                                    audio_devices_t *devices,
                                    const String8& address,
                                    uint32_t *latencyMs,
                                    audio_output_flags_t flags);
        // creates a special output that is duplicated to the two outputs passed as arguments. The duplication is performed by
        // a special mixer thread in the AudioFlinger.
        virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1, audio_io_handle_t output2);
        // closes the output stream
        virtual status_t closeOutput(audio_io_handle_t output);
        // suspends the output. When an output is suspended, the corresponding audio hardware output stream is placed in
        // standby and the AudioTracks attached to the mixer thread are still processed but the output mix is discarded.
        virtual status_t suspendOutput(audio_io_handle_t output);
        // restores a suspended output.
        virtual status_t restoreOutput(audio_io_handle_t output);

        //
        // Audio input Control functions
        //

        // opens an audio input
        virtual audio_io_handle_t openInput(audio_module_handle_t module,
                                            audio_io_handle_t *input,
                                            audio_config_t *config,
                                            audio_devices_t *devices,
                                            const String8& address,
                                            audio_source_t source,
                                            audio_input_flags_t flags);
        // closes an audio input
        virtual status_t closeInput(audio_io_handle_t input);
        //
        // misc control functions
        //

        // set a stream volume for a particular output. For the same user setting, a given stream type can have different volumes
        // for each output (destination device) it is attached to.
        virtual status_t setStreamVolume(audio_stream_type_t stream, float volume, audio_io_handle_t output, int delayMs = 0);

        // invalidate a stream type, causing a reroute to an unspecified new output
        virtual status_t invalidateStream(audio_stream_type_t stream);

        // function enabling to send proprietary informations directly from audio policy manager to audio hardware interface.
        virtual void setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs, int delayMs = 0);
        // function enabling to receive proprietary informations directly from audio hardware interface to audio policy manager.
        virtual String8 getParameters(audio_io_handle_t ioHandle, const String8& keys);

        // set down link audio volume.
        virtual status_t setVoiceVolume(float volume, int delayMs = 0);

        // move effect to the specified output
        virtual status_t moveEffects(audio_session_t session,
                                         audio_io_handle_t srcOutput,
                                         audio_io_handle_t dstOutput);

                void setEffectSuspended(int effectId,
                                        audio_session_t sessionId,
                                        bool suspended) override;

        /* Create a patch between several source and sink ports */
        virtual status_t createAudioPatch(const struct audio_patch *patch,
                                           audio_patch_handle_t *handle,
                                           int delayMs);

        /* Release a patch */
        virtual status_t releaseAudioPatch(audio_patch_handle_t handle,
                                           int delayMs);

        /* Set audio port configuration */
        virtual status_t setAudioPortConfig(const struct audio_port_config *config, int delayMs);

        virtual void onAudioPortListUpdate();
        virtual void onAudioPatchListUpdate();
        virtual void onDynamicPolicyMixStateUpdate(String8 regId, int32_t state);
        virtual void onRecordingConfigurationUpdate(int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source);

        virtual void onAudioVolumeGroupChanged(volume_group_t group, int flags);

        virtual audio_unique_id_t newAudioUniqueId(audio_unique_id_use_t use);

     private:
        AudioPolicyService *mAudioPolicyService;
    };

    // --- Notification Client ---
    class NotificationClient : public IBinder::DeathRecipient {
    public:
                            NotificationClient(const sp<AudioPolicyService>& service,
                                                const sp<IAudioPolicyServiceClient>& client,
                                                uid_t uid, pid_t pid);
        virtual             ~NotificationClient();

                            void      onAudioPortListUpdate();
                            void      onAudioPatchListUpdate();
                            void      onDynamicPolicyMixStateUpdate(const String8& regId,
                                                                    int32_t state);
                            void      onAudioVolumeGroupChanged(volume_group_t group, int flags);
                            void      onRecordingConfigurationUpdate(
                                                    int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source);
                            void      setAudioPortCallbacksEnabled(bool enabled);
                            void setAudioVolumeGroupCallbacksEnabled(bool enabled);

                            uid_t uid() {
                                return mUid;
                            }

                // IBinder::DeathRecipient
                virtual     void        binderDied(const wp<IBinder>& who);

    private:
                            NotificationClient(const NotificationClient&);
                            NotificationClient& operator = (const NotificationClient&);

        const wp<AudioPolicyService>        mService;
        const uid_t                         mUid;
        const pid_t                         mPid;
        const sp<IAudioPolicyServiceClient> mAudioPolicyServiceClient;
              bool                          mAudioPortCallbacksEnabled;
              bool                          mAudioVolumeGroupCallbacksEnabled;
    };

    class AudioClient : public virtual RefBase {
    public:
                AudioClient(const audio_attributes_t attributes,
                            const audio_io_handle_t io, uid_t uid, pid_t pid,
                            const audio_session_t session, const audio_port_handle_t deviceId) :
                                attributes(attributes), io(io), uid(uid), pid(pid),
                                session(session), deviceId(deviceId), active(false) {}
                ~AudioClient() override = default;


        const audio_attributes_t attributes; // source, flags ...
        const audio_io_handle_t io;          // audio HAL stream IO handle
        const uid_t uid;                     // client UID
        const pid_t pid;                     // client PID
        const audio_session_t session;       // audio session ID
        const audio_port_handle_t deviceId;  // selected input device port ID
              bool active;                   // Playback/Capture is active or inactive
    };

    // --- AudioRecordClient ---
    // Information about each registered AudioRecord client
    // (between calls to getInputForAttr() and releaseInput())
    class AudioRecordClient : public AudioClient {
    public:
                AudioRecordClient(const audio_attributes_t attributes,
                          const audio_io_handle_t io, uid_t uid, pid_t pid,
                          const audio_session_t session, const audio_port_handle_t deviceId,
                          const String16& opPackageName,
                          bool canCaptureOutput, bool canCaptureHotword) :
                    AudioClient(attributes, io, uid, pid, session, deviceId),
                    opPackageName(opPackageName), startTimeNs(0),
                    canCaptureOutput(canCaptureOutput), canCaptureHotword(canCaptureHotword) {}
                ~AudioRecordClient() override = default;

        const String16 opPackageName;        // client package name
        nsecs_t startTimeNs;
        const bool canCaptureOutput;
        const bool canCaptureHotword;
    };

    // --- AudioPlaybackClient ---
    // Information about each registered AudioTrack client
    // (between calls to getOutputForAttr() and releaseOutput())
    class AudioPlaybackClient : public AudioClient {
    public:
                AudioPlaybackClient(const audio_attributes_t attributes,
                      const audio_io_handle_t io, uid_t uid, pid_t pid,
                            const audio_session_t session, audio_port_handle_t deviceId,
                            audio_stream_type_t stream) :
                    AudioClient(attributes, io, uid, pid, session, deviceId), stream(stream) {}
                ~AudioPlaybackClient() override = default;

        const audio_stream_type_t stream;
    };

    void getPlaybackClientAndEffects(audio_port_handle_t portId,
                                     sp<AudioPlaybackClient>& client,
                                     sp<AudioPolicyEffects>& effects,
                                     const char *context);


    // A class automatically clearing and restoring binder caller identity inside
    // a code block (scoped variable)
    // Declare one systematically before calling AudioPolicyManager methods so that they are
    // executed with the same level of privilege as audioserver process.
    class AutoCallerClear {
    public:
            AutoCallerClear() :
                mToken(IPCThreadState::self()->clearCallingIdentity()) {}
            ~AutoCallerClear() {
                IPCThreadState::self()->restoreCallingIdentity(mToken);
            }

    private:
        const   int64_t mToken;
    };

    // Internal dump utilities.
    status_t dumpPermissionDenial(int fd);


    mutable Mutex mLock;    // prevents concurrent access to AudioPolicy manager functions changing
                            // device connection state  or routing
    // Note: lock acquisition order is always mLock > mEffectsLock:
    // mLock protects AudioPolicyManager methods that can call into audio flinger
    // and possibly back in to audio policy service and acquire mEffectsLock.
    sp<AudioCommandThread> mAudioCommandThread;     // audio commands thread
    sp<AudioCommandThread> mOutputCommandThread;    // process stop and release output
    struct audio_policy_device *mpAudioPolicyDev;
    struct audio_policy *mpAudioPolicy;
    AudioPolicyInterface *mAudioPolicyManager;
    AudioPolicyClient *mAudioPolicyClient;

    DefaultKeyedVector< int64_t, sp<NotificationClient> >    mNotificationClients;
    Mutex mNotificationClientsLock;  // protects mNotificationClients
    // Manage all effects configured in audio_effects.conf
    // never hold AudioPolicyService::mLock when calling AudioPolicyEffects methods as
    // those can call back into AudioPolicyService methods and try to acquire the mutex
    sp<AudioPolicyEffects> mAudioPolicyEffects;
    audio_mode_t mPhoneState;

    sp<UidPolicy> mUidPolicy;
    sp<SensorPrivacyPolicy> mSensorPrivacyPolicy;

    DefaultKeyedVector< audio_port_handle_t, sp<AudioRecordClient> >   mAudioRecordClients;
    DefaultKeyedVector< audio_port_handle_t, sp<AudioPlaybackClient> >   mAudioPlaybackClients;

    MediaPackageManager mPackageManager; // To check allowPlaybackCapture
};

} // namespace android

#endif // ANDROID_AUDIOPOLICYSERVICE_H
