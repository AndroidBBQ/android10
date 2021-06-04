/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_AUDIOSYSTEM_H_
#define ANDROID_AUDIOSYSTEM_H_

#include <sys/types.h>

#include <media/AudioPolicy.h>
#include <media/AudioProductStrategy.h>
#include <media/AudioVolumeGroup.h>
#include <media/AudioIoDescriptor.h>
#include <media/IAudioFlingerClient.h>
#include <media/IAudioPolicyServiceClient.h>
#include <media/MicrophoneInfo.h>
#include <system/audio.h>
#include <system/audio_effect.h>
#include <system/audio_policy.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <vector>

namespace android {

typedef void (*audio_error_callback)(status_t err);
typedef void (*dynamic_policy_callback)(int event, String8 regId, int val);
typedef void (*record_config_callback)(int event,
                                       const record_client_info_t *clientInfo,
                                       const audio_config_base_t *clientConfig,
                                       std::vector<effect_descriptor_t> clientEffects,
                                       const audio_config_base_t *deviceConfig,
                                       std::vector<effect_descriptor_t> effects,
                                       audio_patch_handle_t patchHandle,
                                       audio_source_t source);

class IAudioFlinger;
class IAudioPolicyService;
class String8;

class AudioSystem
{
public:

    // FIXME Declare in binder opcode order, similarly to IAudioFlinger.h and IAudioFlinger.cpp

    /* These are static methods to control the system-wide AudioFlinger
     * only privileged processes can have access to them
     */

    // mute/unmute microphone
    static status_t muteMicrophone(bool state);
    static status_t isMicrophoneMuted(bool *state);

    // set/get master volume
    static status_t setMasterVolume(float value);
    static status_t getMasterVolume(float* volume);

    // mute/unmute audio outputs
    static status_t setMasterMute(bool mute);
    static status_t getMasterMute(bool* mute);

    // set/get stream volume on specified output
    static status_t setStreamVolume(audio_stream_type_t stream, float value,
                                    audio_io_handle_t output);
    static status_t getStreamVolume(audio_stream_type_t stream, float* volume,
                                    audio_io_handle_t output);

    // mute/unmute stream
    static status_t setStreamMute(audio_stream_type_t stream, bool mute);
    static status_t getStreamMute(audio_stream_type_t stream, bool* mute);

    // set audio mode in audio hardware
    static status_t setMode(audio_mode_t mode);

    // returns true in *state if tracks are active on the specified stream or have been active
    // in the past inPastMs milliseconds
    static status_t isStreamActive(audio_stream_type_t stream, bool *state, uint32_t inPastMs);
    // returns true in *state if tracks are active for what qualifies as remote playback
    // on the specified stream or have been active in the past inPastMs milliseconds. Remote
    // playback isn't mutually exclusive with local playback.
    static status_t isStreamActiveRemotely(audio_stream_type_t stream, bool *state,
            uint32_t inPastMs);
    // returns true in *state if a recorder is currently recording with the specified source
    static status_t isSourceActive(audio_source_t source, bool *state);

    // set/get audio hardware parameters. The function accepts a list of parameters
    // key value pairs in the form: key1=value1;key2=value2;...
    // Some keys are reserved for standard parameters (See AudioParameter class).
    // The versions with audio_io_handle_t are intended for internal media framework use only.
    static status_t setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs);
    static String8  getParameters(audio_io_handle_t ioHandle, const String8& keys);
    // The versions without audio_io_handle_t are intended for JNI.
    static status_t setParameters(const String8& keyValuePairs);
    static String8  getParameters(const String8& keys);

    static void setErrorCallback(audio_error_callback cb);
    static void setDynPolicyCallback(dynamic_policy_callback cb);
    static void setRecordConfigCallback(record_config_callback);

    // helper function to obtain AudioFlinger service handle
    static const sp<IAudioFlinger> get_audio_flinger();

    static float linearToLog(int volume);
    static int logToLinear(float volume);
    static size_t calculateMinFrameCount(
            uint32_t afLatencyMs, uint32_t afFrameCount, uint32_t afSampleRate,
            uint32_t sampleRate, float speed /*, uint32_t notificationsPerBufferReq*/);

    // Returned samplingRate and frameCount output values are guaranteed
    // to be non-zero if status == NO_ERROR
    // FIXME This API assumes a route, and so should be deprecated.
    static status_t getOutputSamplingRate(uint32_t* samplingRate,
            audio_stream_type_t stream);
    // FIXME This API assumes a route, and so should be deprecated.
    static status_t getOutputFrameCount(size_t* frameCount,
            audio_stream_type_t stream);
    // FIXME This API assumes a route, and so should be deprecated.
    static status_t getOutputLatency(uint32_t* latency,
            audio_stream_type_t stream);
    // returns the audio HAL sample rate
    static status_t getSamplingRate(audio_io_handle_t ioHandle,
                                          uint32_t* samplingRate);
    // For output threads with a fast mixer, returns the number of frames per normal mixer buffer.
    // For output threads without a fast mixer, or for input, this is same as getFrameCountHAL().
    static status_t getFrameCount(audio_io_handle_t ioHandle,
                                  size_t* frameCount);
    // returns the audio output latency in ms. Corresponds to
    // audio_stream_out->get_latency()
    static status_t getLatency(audio_io_handle_t output,
                               uint32_t* latency);

    // return status NO_ERROR implies *buffSize > 0
    // FIXME This API assumes a route, and so should deprecated.
    static status_t getInputBufferSize(uint32_t sampleRate, audio_format_t format,
        audio_channel_mask_t channelMask, size_t* buffSize);

    static status_t setVoiceVolume(float volume);

    // return the number of audio frames written by AudioFlinger to audio HAL and
    // audio dsp to DAC since the specified output has exited standby.
    // returned status (from utils/Errors.h) can be:
    // - NO_ERROR: successful operation, halFrames and dspFrames point to valid data
    // - INVALID_OPERATION: Not supported on current hardware platform
    // - BAD_VALUE: invalid parameter
    // NOTE: this feature is not supported on all hardware platforms and it is
    // necessary to check returned status before using the returned values.
    static status_t getRenderPosition(audio_io_handle_t output,
                                      uint32_t *halFrames,
                                      uint32_t *dspFrames);

    // return the number of input frames lost by HAL implementation, or 0 if the handle is invalid
    static uint32_t getInputFramesLost(audio_io_handle_t ioHandle);

    // Allocate a new unique ID for use as an audio session ID or I/O handle.
    // If unable to contact AudioFlinger, returns AUDIO_UNIQUE_ID_ALLOCATE instead.
    // FIXME If AudioFlinger were to ever exhaust the unique ID namespace,
    //       this method could fail by returning either a reserved ID like AUDIO_UNIQUE_ID_ALLOCATE
    //       or an unspecified existing unique ID.
    static audio_unique_id_t newAudioUniqueId(audio_unique_id_use_t use);

    static void acquireAudioSessionId(audio_session_t audioSession, pid_t pid);
    static void releaseAudioSessionId(audio_session_t audioSession, pid_t pid);

    // Get the HW synchronization source used for an audio session.
    // Return a valid source or AUDIO_HW_SYNC_INVALID if an error occurs
    // or no HW sync source is used.
    static audio_hw_sync_t getAudioHwSyncForSession(audio_session_t sessionId);

    // Indicate JAVA services are ready (scheduling, power management ...)
    static status_t systemReady();

    // Returns the number of frames per audio HAL buffer.
    // Corresponds to audio_stream->get_buffer_size()/audio_stream_in_frame_size() for input.
    // See also getFrameCount().
    static status_t getFrameCountHAL(audio_io_handle_t ioHandle,
                                     size_t* frameCount);

    // Events used to synchronize actions between audio sessions.
    // For instance SYNC_EVENT_PRESENTATION_COMPLETE can be used to delay recording start until
    // playback is complete on another audio session.
    // See definitions in MediaSyncEvent.java
    enum sync_event_t {
        SYNC_EVENT_SAME = -1,             // used internally to indicate restart with same event
        SYNC_EVENT_NONE = 0,
        SYNC_EVENT_PRESENTATION_COMPLETE,

        //
        // Define new events here: SYNC_EVENT_START, SYNC_EVENT_STOP, SYNC_EVENT_TIME ...
        //
        SYNC_EVENT_CNT,
    };

    // Timeout for synchronous record start. Prevents from blocking the record thread forever
    // if the trigger event is not fired.
    static const uint32_t kSyncRecordStartTimeOutMs = 30000;

    //
    // IAudioPolicyService interface (see AudioPolicyInterface for method descriptions)
    //
    static status_t setDeviceConnectionState(audio_devices_t device, audio_policy_dev_state_t state,
                                             const char *device_address, const char *device_name,
                                             audio_format_t encodedFormat);
    static audio_policy_dev_state_t getDeviceConnectionState(audio_devices_t device,
                                                                const char *device_address);
    static status_t handleDeviceConfigChange(audio_devices_t device,
                                             const char *device_address,
                                             const char *device_name,
                                             audio_format_t encodedFormat);
    static status_t setPhoneState(audio_mode_t state);
    static status_t setForceUse(audio_policy_force_use_t usage, audio_policy_forced_cfg_t config);
    static audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage);

    static status_t getOutputForAttr(audio_attributes_t *attr,
                                     audio_io_handle_t *output,
                                     audio_session_t session,
                                     audio_stream_type_t *stream,
                                     pid_t pid,
                                     uid_t uid,
                                     const audio_config_t *config,
                                     audio_output_flags_t flags,
                                     audio_port_handle_t *selectedDeviceId,
                                     audio_port_handle_t *portId,
                                     std::vector<audio_io_handle_t> *secondaryOutputs);
    static status_t startOutput(audio_port_handle_t portId);
    static status_t stopOutput(audio_port_handle_t portId);
    static void releaseOutput(audio_port_handle_t portId);

    // Client must successfully hand off the handle reference to AudioFlinger via createRecord(),
    // or release it with releaseInput().
    static status_t getInputForAttr(const audio_attributes_t *attr,
                                    audio_io_handle_t *input,
                                    audio_unique_id_t riid,
                                    audio_session_t session,
                                    pid_t pid,
                                    uid_t uid,
                                    const String16& opPackageName,
                                    const audio_config_base_t *config,
                                    audio_input_flags_t flags,
                                    audio_port_handle_t *selectedDeviceId,
                                    audio_port_handle_t *portId);

    static status_t startInput(audio_port_handle_t portId);
    static status_t stopInput(audio_port_handle_t portId);
    static void releaseInput(audio_port_handle_t portId);
    static status_t initStreamVolume(audio_stream_type_t stream,
                                      int indexMin,
                                      int indexMax);
    static status_t setStreamVolumeIndex(audio_stream_type_t stream,
                                         int index,
                                         audio_devices_t device);
    static status_t getStreamVolumeIndex(audio_stream_type_t stream,
                                         int *index,
                                         audio_devices_t device);

    static status_t setVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                int index,
                                                audio_devices_t device);
    static status_t getVolumeIndexForAttributes(const audio_attributes_t &attr,
                                                int &index,
                                                audio_devices_t device);

    static status_t getMaxVolumeIndexForAttributes(const audio_attributes_t &attr, int &index);

    static status_t getMinVolumeIndexForAttributes(const audio_attributes_t &attr, int &index);

    static uint32_t getStrategyForStream(audio_stream_type_t stream);
    static audio_devices_t getDevicesForStream(audio_stream_type_t stream);

    static audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc);
    static status_t registerEffect(const effect_descriptor_t *desc,
                                    audio_io_handle_t io,
                                    uint32_t strategy,
                                    audio_session_t session,
                                    int id);
    static status_t unregisterEffect(int id);
    static status_t setEffectEnabled(int id, bool enabled);
    static status_t moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io);

    // clear stream to output mapping cache (gStreamOutputMap)
    // and output configuration cache (gOutputs)
    static void clearAudioConfigCache();

    static const sp<IAudioPolicyService> get_audio_policy_service();

    // helpers for android.media.AudioManager.getProperty(), see description there for meaning
    static uint32_t getPrimaryOutputSamplingRate();
    static size_t getPrimaryOutputFrameCount();

    static status_t setLowRamDevice(bool isLowRamDevice, int64_t totalMemory);

    static status_t setAllowedCapturePolicy(uid_t uid, audio_flags_mask_t flags);

    // Check if hw offload is possible for given format, stream type, sample rate,
    // bit rate, duration, video and streaming or offload property is enabled
    static bool isOffloadSupported(const audio_offload_info_t& info);

    // check presence of audio flinger service.
    // returns NO_ERROR if binding to service succeeds, DEAD_OBJECT otherwise
    static status_t checkAudioFlinger();

    /* List available audio ports and their attributes */
    static status_t listAudioPorts(audio_port_role_t role,
                                   audio_port_type_t type,
                                   unsigned int *num_ports,
                                   struct audio_port *ports,
                                   unsigned int *generation);

    /* Get attributes for a given audio port */
    static status_t getAudioPort(struct audio_port *port);

    /* Create an audio patch between several source and sink ports */
    static status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle);

    /* Release an audio patch */
    static status_t releaseAudioPatch(audio_patch_handle_t handle);

    /* List existing audio patches */
    static status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches,
                                      unsigned int *generation);
    /* Set audio port configuration */
    static status_t setAudioPortConfig(const struct audio_port_config *config);


    static status_t acquireSoundTriggerSession(audio_session_t *session,
                                           audio_io_handle_t *ioHandle,
                                           audio_devices_t *device);
    static status_t releaseSoundTriggerSession(audio_session_t session);

    static audio_mode_t getPhoneState();

    static status_t registerPolicyMixes(const Vector<AudioMix>& mixes, bool registration);

    static status_t setUidDeviceAffinities(uid_t uid, const Vector<AudioDeviceTypeAddr>& devices);

    static status_t removeUidDeviceAffinities(uid_t uid);

    static status_t startAudioSource(const struct audio_port_config *source,
                                     const audio_attributes_t *attributes,
                                     audio_port_handle_t *portId);
    static status_t stopAudioSource(audio_port_handle_t portId);

    static status_t setMasterMono(bool mono);
    static status_t getMasterMono(bool *mono);

    static status_t setMasterBalance(float balance);
    static status_t getMasterBalance(float *balance);

    static float    getStreamVolumeDB(
            audio_stream_type_t stream, int index, audio_devices_t device);

    static status_t getMicrophones(std::vector<media::MicrophoneInfo> *microphones);

    static status_t getHwOffloadEncodingFormatsSupportedForA2DP(
                                    std::vector<audio_format_t> *formats);

    // numSurroundFormats holds the maximum number of formats and bool value allowed in the array.
    // When numSurroundFormats is 0, surroundFormats and surroundFormatsEnabled will not be
    // populated. The actual number of surround formats should be returned at numSurroundFormats.
    static status_t getSurroundFormats(unsigned int *numSurroundFormats,
                                       audio_format_t *surroundFormats,
                                       bool *surroundFormatsEnabled,
                                       bool reported);
    static status_t setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled);

    static status_t setAssistantUid(uid_t uid);
    static status_t setA11yServicesUids(const std::vector<uid_t>& uids);

    static bool     isHapticPlaybackSupported();

    static status_t listAudioProductStrategies(AudioProductStrategyVector &strategies);
    static status_t getProductStrategyFromAudioAttributes(const AudioAttributes &aa,
                                                        product_strategy_t &productStrategy);

    static audio_attributes_t streamTypeToAttributes(audio_stream_type_t stream);
    static audio_stream_type_t attributesToStreamType(const audio_attributes_t &attr);

    static status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups);

    static status_t getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                      volume_group_t &volumeGroup);

    static status_t setRttEnabled(bool enabled);

    // ----------------------------------------------------------------------------

    class AudioVolumeGroupCallback : public RefBase
    {
    public:

        AudioVolumeGroupCallback() {}
        virtual ~AudioVolumeGroupCallback() {}

        virtual void onAudioVolumeGroupChanged(volume_group_t group, int flags) = 0;
        virtual void onServiceDied() = 0;

    };

    static status_t addAudioVolumeGroupCallback(const sp<AudioVolumeGroupCallback>& callback);
    static status_t removeAudioVolumeGroupCallback(const sp<AudioVolumeGroupCallback>& callback);

    class AudioPortCallback : public RefBase
    {
    public:

                AudioPortCallback() {}
        virtual ~AudioPortCallback() {}

        virtual void onAudioPortListUpdate() = 0;
        virtual void onAudioPatchListUpdate() = 0;
        virtual void onServiceDied() = 0;

    };

    static status_t addAudioPortCallback(const sp<AudioPortCallback>& callback);
    static status_t removeAudioPortCallback(const sp<AudioPortCallback>& callback);

    class AudioDeviceCallback : public RefBase
    {
    public:

                AudioDeviceCallback() {}
        virtual ~AudioDeviceCallback() {}

        virtual void onAudioDeviceUpdate(audio_io_handle_t audioIo,
                                         audio_port_handle_t deviceId) = 0;
    };

    static status_t addAudioDeviceCallback(const wp<AudioDeviceCallback>& callback,
                                           audio_io_handle_t audioIo,
                                           audio_port_handle_t portId);
    static status_t removeAudioDeviceCallback(const wp<AudioDeviceCallback>& callback,
                                              audio_io_handle_t audioIo,
                                              audio_port_handle_t portId);

    static audio_port_handle_t getDeviceIdForIo(audio_io_handle_t audioIo);

private:

    class AudioFlingerClient: public IBinder::DeathRecipient, public BnAudioFlingerClient
    {
    public:
        AudioFlingerClient() :
            mInBuffSize(0), mInSamplingRate(0),
            mInFormat(AUDIO_FORMAT_DEFAULT), mInChannelMask(AUDIO_CHANNEL_NONE) {
        }

        void clearIoCache();
        status_t getInputBufferSize(uint32_t sampleRate, audio_format_t format,
                                    audio_channel_mask_t channelMask, size_t* buffSize);
        sp<AudioIoDescriptor> getIoDescriptor(audio_io_handle_t ioHandle);

        // DeathRecipient
        virtual void binderDied(const wp<IBinder>& who);

        // IAudioFlingerClient

        // indicate a change in the configuration of an output or input: keeps the cached
        // values for output/input parameters up-to-date in client process
        virtual void ioConfigChanged(audio_io_config_event event,
                                     const sp<AudioIoDescriptor>& ioDesc);


        status_t addAudioDeviceCallback(const wp<AudioDeviceCallback>& callback,
                                               audio_io_handle_t audioIo,
                                               audio_port_handle_t portId);
        status_t removeAudioDeviceCallback(const wp<AudioDeviceCallback>& callback,
                                           audio_io_handle_t audioIo,
                                           audio_port_handle_t portId);

        audio_port_handle_t getDeviceIdForIo(audio_io_handle_t audioIo);

    private:
        Mutex                               mLock;
        DefaultKeyedVector<audio_io_handle_t, sp<AudioIoDescriptor> >   mIoDescriptors;

        std::map<audio_io_handle_t, std::map<audio_port_handle_t, wp<AudioDeviceCallback>>>
                mAudioDeviceCallbacks;
        // cached values for recording getInputBufferSize() queries
        size_t                              mInBuffSize;    // zero indicates cache is invalid
        uint32_t                            mInSamplingRate;
        audio_format_t                      mInFormat;
        audio_channel_mask_t                mInChannelMask;
        sp<AudioIoDescriptor> getIoDescriptor_l(audio_io_handle_t ioHandle);
    };

    class AudioPolicyServiceClient: public IBinder::DeathRecipient,
                                    public BnAudioPolicyServiceClient
    {
    public:
        AudioPolicyServiceClient() {
        }

        int addAudioPortCallback(const sp<AudioPortCallback>& callback);
        int removeAudioPortCallback(const sp<AudioPortCallback>& callback);
        bool isAudioPortCbEnabled() const { return (mAudioPortCallbacks.size() != 0); }

        int addAudioVolumeGroupCallback(const sp<AudioVolumeGroupCallback>& callback);
        int removeAudioVolumeGroupCallback(const sp<AudioVolumeGroupCallback>& callback);
        bool isAudioVolumeGroupCbEnabled() const { return (mAudioVolumeGroupCallback.size() != 0); }

        // DeathRecipient
        virtual void binderDied(const wp<IBinder>& who);

        // IAudioPolicyServiceClient
        virtual void onAudioPortListUpdate();
        virtual void onAudioPatchListUpdate();
        virtual void onAudioVolumeGroupChanged(volume_group_t group, int flags);
        virtual void onDynamicPolicyMixStateUpdate(String8 regId, int32_t state);
        virtual void onRecordingConfigurationUpdate(int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source);

    private:
        Mutex                               mLock;
        Vector <sp <AudioPortCallback> >    mAudioPortCallbacks;
        Vector <sp <AudioVolumeGroupCallback> > mAudioVolumeGroupCallback;
    };

    static audio_io_handle_t getOutput(audio_stream_type_t stream);
    static const sp<AudioFlingerClient> getAudioFlingerClient();
    static sp<AudioIoDescriptor> getIoDescriptor(audio_io_handle_t ioHandle);

    static sp<AudioFlingerClient> gAudioFlingerClient;
    static sp<AudioPolicyServiceClient> gAudioPolicyServiceClient;
    friend class AudioFlingerClient;
    friend class AudioPolicyServiceClient;

    static Mutex gLock;      // protects gAudioFlinger and gAudioErrorCallback,
    static Mutex gLockAPS;   // protects gAudioPolicyService and gAudioPolicyServiceClient
    static sp<IAudioFlinger> gAudioFlinger;
    static audio_error_callback gAudioErrorCallback;
    static dynamic_policy_callback gDynPolicyCallback;
    static record_config_callback gRecordConfigCallback;

    static size_t gInBuffSize;
    // previous parameters for recording buffer size queries
    static uint32_t gPrevInSamplingRate;
    static audio_format_t gPrevInFormat;
    static audio_channel_mask_t gPrevInChannelMask;

    static sp<IAudioPolicyService> gAudioPolicyService;
};

};  // namespace android

#endif  /*ANDROID_AUDIOSYSTEM_H_*/
