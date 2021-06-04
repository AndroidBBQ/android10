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

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_set>

#include <stdint.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <media/AudioParameter.h>
#include <media/AudioPolicy.h>
#include <media/PatchBuilder.h>
#include "AudioPolicyInterface.h"

#include <AudioPolicyManagerInterface.h>
#include <AudioPolicyManagerObserver.h>
#include <AudioGain.h>
#include <AudioPolicyConfig.h>
#include <AudioPort.h>
#include <AudioPatch.h>
#include <AudioProfile.h>
#include <DeviceDescriptor.h>
#include <IOProfile.h>
#include <HwModule.h>
#include <AudioInputDescriptor.h>
#include <AudioOutputDescriptor.h>
#include <AudioPolicyMix.h>
#include <EffectDescriptor.h>
#include <SoundTriggerSession.h>
#include "TypeConverter.h"

namespace android {

// ----------------------------------------------------------------------------

// Attenuation applied to STRATEGY_SONIFICATION streams when a headset is connected: 6dB
#define SONIFICATION_HEADSET_VOLUME_FACTOR_DB (-6)
// Min volume for STRATEGY_SONIFICATION streams when limited by music volume: -36dB
#define SONIFICATION_HEADSET_VOLUME_MIN_DB  (-36)
// Max volume difference on A2DP between playing media and STRATEGY_SONIFICATION streams: 12dB
#define SONIFICATION_A2DP_MAX_MEDIA_DIFF_DB (12)

// Time in milliseconds during which we consider that music is still active after a music
// track was stopped - see computeVolume()
#define SONIFICATION_HEADSET_MUSIC_DELAY  5000

// Time in milliseconds during witch some streams are muted while the audio path
// is switched
#define MUTE_TIME_MS 2000

// multiplication factor applied to output latency when calculating a safe mute delay when
// invalidating tracks
#define LATENCY_MUTE_FACTOR 4

#define NUM_TEST_OUTPUTS 5

#define NUM_VOL_CURVE_KNEES 2

// Default minimum length allowed for offloading a compressed track
// Can be overridden by the audio.offload.min.duration.secs property
#define OFFLOAD_DEFAULT_MIN_DURATION_SECS 60

// ----------------------------------------------------------------------------
// AudioPolicyManager implements audio policy manager behavior common to all platforms.
// ----------------------------------------------------------------------------

class AudioPolicyManager : public AudioPolicyInterface, public AudioPolicyManagerObserver
{

public:
        explicit AudioPolicyManager(AudioPolicyClientInterface *clientInterface);
        virtual ~AudioPolicyManager();

        // AudioPolicyInterface
        virtual status_t setDeviceConnectionState(audio_devices_t device,
                                                          audio_policy_dev_state_t state,
                                                          const char *device_address,
                                                          const char *device_name,
                                                          audio_format_t encodedFormat);
        virtual audio_policy_dev_state_t getDeviceConnectionState(audio_devices_t device,
                                                                              const char *device_address);
        virtual status_t handleDeviceConfigChange(audio_devices_t device,
                                                  const char *device_address,
                                                  const char *device_name,
                                                  audio_format_t encodedFormat);
        virtual void setPhoneState(audio_mode_t state);
        virtual void setForceUse(audio_policy_force_use_t usage,
                                 audio_policy_forced_cfg_t config);
        virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage);

        virtual void setSystemProperty(const char* property, const char* value);
        virtual status_t initCheck();
        virtual audio_io_handle_t getOutput(audio_stream_type_t stream);
        status_t getOutputForAttr(const audio_attributes_t *attr,
                                  audio_io_handle_t *output,
                                  audio_session_t session,
                                  audio_stream_type_t *stream,
                                  uid_t uid,
                                  const audio_config_t *config,
                                  audio_output_flags_t *flags,
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
                                         uid_t uid,
                                         const audio_config_base_t *config,
                                         audio_input_flags_t flags,
                                         audio_port_handle_t *selectedDeviceId,
                                         input_type_t *inputType,
                                         audio_port_handle_t *portId);

        // indicates to the audio policy manager that the input starts being used.
        virtual status_t startInput(audio_port_handle_t portId);

        // indicates to the audio policy manager that the input stops being used.
        virtual status_t stopInput(audio_port_handle_t portId);
        virtual void releaseInput(audio_port_handle_t portId);
        virtual void checkCloseInputs();
        /**
         * @brief initStreamVolume: even if the engine volume files provides min and max, keep this
         * api for compatibility reason.
         * AudioServer will get the min and max and may overwrite them if:
         *      -using property (highest priority)
         *      -not defined (-1 by convention), case when still using apm volume tables XML files
         * @param stream to be considered
         * @param indexMin to set
         * @param indexMax to set
         */
        virtual void initStreamVolume(audio_stream_type_t stream, int indexMin, int indexMax);
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
        virtual status_t getMaxVolumeIndexForAttributes(const audio_attributes_t &attr, int &index);

        virtual status_t getMinVolumeIndexForAttributes(const audio_attributes_t &attr, int &index);

        status_t setVolumeCurveIndex(int index,
                                     audio_devices_t device,
                                     IVolumeCurves &volumeCurves);

        status_t getVolumeIndex(const IVolumeCurves &curves, int &index,
                                audio_devices_t device) const;

        // return the strategy corresponding to a given stream type
        virtual uint32_t getStrategyForStream(audio_stream_type_t stream)
        {
            return streamToStrategy(stream);
        }
        product_strategy_t streamToStrategy(audio_stream_type_t stream) const
        {
            auto attributes = mEngine->getAttributesForStreamType(stream);
            return mEngine->getProductStrategyForAttributes(attributes);
        }

        // return the enabled output devices for the given stream type
        virtual audio_devices_t getDevicesForStream(audio_stream_type_t stream);

        virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc = NULL);
        virtual status_t registerEffect(const effect_descriptor_t *desc,
                                        audio_io_handle_t io,
                                        uint32_t strategy,
                                        int session,
                                        int id);
        virtual status_t unregisterEffect(int id);
        virtual status_t setEffectEnabled(int id, bool enabled);
        status_t moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io) override;

        virtual bool isStreamActive(audio_stream_type_t stream, uint32_t inPastMs = 0) const;
        // return whether a stream is playing remotely, override to change the definition of
        //   local/remote playback, used for instance by notification manager to not make
        //   media players lose audio focus when not playing locally
        //   For the base implementation, "remotely" means playing during screen mirroring which
        //   uses an output for playback with a non-empty, non "0" address.
        virtual bool isStreamActiveRemotely(audio_stream_type_t stream,
                                            uint32_t inPastMs = 0) const;

        virtual bool isSourceActive(audio_source_t source) const;

        // helpers for dump(int fd)
        void dumpManualSurroundFormats(String8 *dst) const;
        void dump(String8 *dst) const;

        status_t dump(int fd) override;

        status_t setAllowedCapturePolicy(uid_t uid, audio_flags_mask_t capturePolicy) override;
        virtual bool isOffloadSupported(const audio_offload_info_t& offloadInfo);

        virtual bool isDirectOutputSupported(const audio_config_base_t& config,
                                             const audio_attributes_t& attributes);

        virtual status_t listAudioPorts(audio_port_role_t role,
                                        audio_port_type_t type,
                                        unsigned int *num_ports,
                                        struct audio_port *ports,
                                        unsigned int *generation);
        virtual status_t getAudioPort(struct audio_port *port);
        virtual status_t createAudioPatch(const struct audio_patch *patch,
                                           audio_patch_handle_t *handle,
                                           uid_t uid);
        virtual status_t releaseAudioPatch(audio_patch_handle_t handle,
                                              uid_t uid);
        virtual status_t listAudioPatches(unsigned int *num_patches,
                                          struct audio_patch *patches,
                                          unsigned int *generation);
        virtual status_t setAudioPortConfig(const struct audio_port_config *config);

        virtual void releaseResourcesForUid(uid_t uid);

        virtual status_t acquireSoundTriggerSession(audio_session_t *session,
                                               audio_io_handle_t *ioHandle,
                                               audio_devices_t *device);

        virtual status_t releaseSoundTriggerSession(audio_session_t session)
        {
            return mSoundTriggerSessions.releaseSession(session);
        }

        virtual status_t registerPolicyMixes(const Vector<AudioMix>& mixes);
        virtual status_t unregisterPolicyMixes(Vector<AudioMix> mixes);
        virtual status_t setUidDeviceAffinities(uid_t uid,
                const Vector<AudioDeviceTypeAddr>& devices);
        virtual status_t removeUidDeviceAffinities(uid_t uid);

        virtual status_t startAudioSource(const struct audio_port_config *source,
                                          const audio_attributes_t *attributes,
                                          audio_port_handle_t *portId,
                                          uid_t uid);
        virtual status_t stopAudioSource(audio_port_handle_t portId);

        virtual status_t setMasterMono(bool mono);
        virtual status_t getMasterMono(bool *mono);
        virtual float    getStreamVolumeDB(
                    audio_stream_type_t stream, int index, audio_devices_t device);

        virtual status_t getSurroundFormats(unsigned int *numSurroundFormats,
                                            audio_format_t *surroundFormats,
                                            bool *surroundFormatsEnabled,
                                            bool reported);
        virtual status_t setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled);

        virtual status_t getHwOffloadEncodingFormatsSupportedForA2DP(
                    std::vector<audio_format_t> *formats);

        virtual void setAppState(uid_t uid, app_state_t state);

        virtual bool isHapticPlaybackSupported();

        virtual status_t listAudioProductStrategies(AudioProductStrategyVector &strategies)
        {
            return mEngine->listAudioProductStrategies(strategies);
        }

        virtual status_t getProductStrategyFromAudioAttributes(const AudioAttributes &aa,
                                                               product_strategy_t &productStrategy)
        {
            productStrategy = mEngine->getProductStrategyForAttributes(aa.getAttributes());
            return productStrategy != PRODUCT_STRATEGY_NONE ? NO_ERROR : BAD_VALUE;
        }

        virtual status_t listAudioVolumeGroups(AudioVolumeGroupVector &groups)
        {
            return mEngine->listAudioVolumeGroups(groups);
        }

        virtual status_t getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                           volume_group_t &volumeGroup)
        {
            volumeGroup = mEngine->getVolumeGroupForAttributes(aa.getAttributes());
            return volumeGroup != VOLUME_GROUP_NONE ? NO_ERROR : BAD_VALUE;
        }

protected:
        // A constructor that allows more fine-grained control over initialization process,
        // used in automatic tests.
        AudioPolicyManager(AudioPolicyClientInterface *clientInterface, bool forTesting);

        // These methods should be used when finer control over APM initialization
        // is needed, e.g. in tests. Must be used in conjunction with the constructor
        // that only performs fields initialization. The public constructor comprises
        // these steps in the following sequence:
        //   - field initializing constructor;
        //   - loadConfig;
        //   - initialize.
        AudioPolicyConfig& getConfig() { return mConfig; }
        void loadConfig();
        status_t initialize();

        // From AudioPolicyManagerObserver
        virtual const AudioPatchCollection &getAudioPatches() const
        {
            return mAudioPatches;
        }
        virtual const SoundTriggerSessionCollection &getSoundTriggerSessionCollection() const
        {
            return mSoundTriggerSessions;
        }
        virtual const AudioPolicyMixCollection &getAudioPolicyMixCollection() const
        {
            return mPolicyMixes;
        }
        virtual const SwAudioOutputCollection &getOutputs() const
        {
            return mOutputs;
        }
        virtual const AudioInputCollection &getInputs() const
        {
            return mInputs;
        }
        virtual const DeviceVector getAvailableOutputDevices() const
        {
            return mAvailableOutputDevices.filterForEngine();
        }
        virtual const DeviceVector getAvailableInputDevices() const
        {
            // legacy and non-legacy remote-submix are managed by the engine, do not filter
            return mAvailableInputDevices;
        }
        virtual const sp<DeviceDescriptor> &getDefaultOutputDevice() const
        {
            return mDefaultOutputDevice;
        }

        std::vector<volume_group_t> getVolumeGroups() const
        {
            return mEngine->getVolumeGroups();
        }

        VolumeSource toVolumeSource(volume_group_t volumeGroup) const
        {
            return static_cast<VolumeSource>(volumeGroup);
        }
        VolumeSource toVolumeSource(const audio_attributes_t &attributes) const
        {
            return toVolumeSource(mEngine->getVolumeGroupForAttributes(attributes));
        }
        VolumeSource toVolumeSource(audio_stream_type_t stream) const
        {
            return toVolumeSource(mEngine->getVolumeGroupForStreamType(stream));
        }
        IVolumeCurves &getVolumeCurves(VolumeSource volumeSource)
        {
          auto *curves = mEngine->getVolumeCurvesForVolumeGroup(
              static_cast<volume_group_t>(volumeSource));
          ALOG_ASSERT(curves != nullptr, "No curves for volume source %d", volumeSource);
          return *curves;
        }
        IVolumeCurves &getVolumeCurves(const audio_attributes_t &attr)
        {
            auto *curves = mEngine->getVolumeCurvesForAttributes(attr);
            ALOG_ASSERT(curves != nullptr, "No curves for attributes %s", toString(attr).c_str());
            return *curves;
        }
        IVolumeCurves &getVolumeCurves(audio_stream_type_t stream)
        {
            auto *curves = mEngine->getVolumeCurvesForStreamType(stream);
            ALOG_ASSERT(curves != nullptr, "No curves for stream %s", toString(stream).c_str());
            return *curves;
        }

        void addOutput(audio_io_handle_t output, const sp<SwAudioOutputDescriptor>& outputDesc);
        void removeOutput(audio_io_handle_t output);
        void addInput(audio_io_handle_t input, const sp<AudioInputDescriptor>& inputDesc);

        // change the route of the specified output. Returns the number of ms we have slept to
        // allow new routing to take effect in certain cases.
        uint32_t setOutputDevices(const sp<SwAudioOutputDescriptor>& outputDesc,
                                  const DeviceVector &device,
                                  bool force = false,
                                  int delayMs = 0,
                                  audio_patch_handle_t *patchHandle = NULL,
                                  bool requiresMuteCheck = true);
        status_t resetOutputDevice(const sp<AudioOutputDescriptor>& outputDesc,
                                   int delayMs = 0,
                                   audio_patch_handle_t *patchHandle = NULL);
        status_t setInputDevice(audio_io_handle_t input,
                                const sp<DeviceDescriptor> &device,
                                bool force = false,
                                audio_patch_handle_t *patchHandle = NULL);
        status_t resetInputDevice(audio_io_handle_t input,
                                  audio_patch_handle_t *patchHandle = NULL);

        // compute the actual volume for a given stream according to the requested index and a particular
        // device
        virtual float computeVolume(IVolumeCurves &curves,
                                    VolumeSource volumeSource,
                                    int index,
                                    audio_devices_t device);

        // rescale volume index from srcStream within range of dstStream
        int rescaleVolumeIndex(int srcIndex,
                               VolumeSource fromVolumeSource,
                               VolumeSource toVolumeSource);
        // check that volume change is permitted, compute and send new volume to audio hardware
        virtual status_t checkAndSetVolume(IVolumeCurves &curves,
                                           VolumeSource volumeSource, int index,
                                           const sp<AudioOutputDescriptor>& outputDesc,
                                           audio_devices_t device,
                                           int delayMs = 0, bool force = false);

        // apply all stream volumes to the specified output and device
        void applyStreamVolumes(const sp<AudioOutputDescriptor>& outputDesc,
                                audio_devices_t device, int delayMs = 0, bool force = false);

        /**
         * @brief setStrategyMute Mute or unmute all active clients on the considered output
         * following the given strategy.
         * @param strategy to be considered
         * @param on true for mute, false for unmute
         * @param outputDesc to be considered
         * @param delayMs
         * @param device
         */
        void setStrategyMute(product_strategy_t strategy,
                             bool on,
                             const sp<AudioOutputDescriptor>& outputDesc,
                             int delayMs = 0,
                             audio_devices_t device = AUDIO_DEVICE_NONE);

        /**
         * @brief setVolumeSourceMute Mute or unmute the volume source on the specified output
         * @param volumeSource to be muted/unmute (may host legacy streams or by extension set of
         * audio attributes)
         * @param on true to mute, false to umute
         * @param outputDesc on which the client following the volume group shall be muted/umuted
         * @param delayMs
         * @param device
         */
        void setVolumeSourceMute(VolumeSource volumeSource,
                                 bool on,
                                 const sp<AudioOutputDescriptor>& outputDesc,
                                 int delayMs = 0,
                                 audio_devices_t device = AUDIO_DEVICE_NONE);

        audio_mode_t getPhoneState();

        // true if device is in a telephony or VoIP call
        virtual bool isInCall();
        // true if given state represents a device in a telephony or VoIP call
        virtual bool isStateInCall(int state);

        // when a device is connected, checks if an open output can be routed
        // to this device. If none is open, tries to open one of the available outputs.
        // Returns an output suitable to this device or 0.
        // when a device is disconnected, checks if an output is not used any more and
        // returns its handle if any.
        // transfers the audio tracks and effects from one output thread to another accordingly.
        status_t checkOutputsForDevice(const sp<DeviceDescriptor>& device,
                                       audio_policy_dev_state_t state,
                                       SortedVector<audio_io_handle_t>& outputs);

        status_t checkInputsForDevice(const sp<DeviceDescriptor>& device,
                                      audio_policy_dev_state_t state);

        // close an output and its companion duplicating output.
        void closeOutput(audio_io_handle_t output);

        // close an input.
        void closeInput(audio_io_handle_t input);

        // runs all the checks required for accomodating changes in devices and outputs
        // if 'onOutputsChecked' callback is provided, it is executed after the outputs
        // check via 'checkOutputForAllStrategies'. If the callback returns 'true',
        // A2DP suspend status is rechecked.
        void checkForDeviceAndOutputChanges(std::function<bool()> onOutputsChecked = nullptr);

        /**
         * @brief checkOutputForAttributes checks and if necessary changes outputs used for the
         * given audio attributes.
         * must be called every time a condition that affects the output choice for a given
         * attributes changes: connected device, phone state, force use...
         * Must be called before updateDevicesAndOutputs()
         * @param attr to be considered
         */
        void checkOutputForAttributes(const audio_attributes_t &attr);

        bool followsSameRouting(const audio_attributes_t &lAttr,
                                const audio_attributes_t &rAttr) const;

        /**
         * @brief checkOutputForAllStrategies Same as @see checkOutputForAttributes()
         *      but for a all product strategies in order of priority
         */
        void checkOutputForAllStrategies();

        // Same as checkOutputForStrategy but for secondary outputs. Make sure if a secondary
        // output condition changes, the track is properly rerouted
        void checkSecondaryOutputs();

        // manages A2DP output suspend/restore according to phone state and BT SCO usage
        void checkA2dpSuspend();

        // selects the most appropriate device on output for current state
        // must be called every time a condition that affects the device choice for a given output is
        // changed: connected device, phone state, force use, output start, output stop..
        // see getDeviceForStrategy() for the use of fromCache parameter
        DeviceVector getNewOutputDevices(const sp<SwAudioOutputDescriptor>& outputDesc,
                                         bool fromCache);

        /**
         * @brief updateDevicesAndOutputs: updates cache of devices of the engine
         * must be called every time a condition that affects the device choice is changed:
         * connected device, phone state, force use...
         * cached values are used by getOutputDevicesForStream()/getDevicesForAttributes if
         * parameter fromCache is true.
         * Must be called after checkOutputForAllStrategies()
         */
        void updateDevicesAndOutputs();

        // selects the most appropriate device on input for current state
        sp<DeviceDescriptor> getNewInputDevice(const sp<AudioInputDescriptor>& inputDesc);

        virtual uint32_t getMaxEffectsCpuLoad()
        {
            return mEffects.getMaxEffectsCpuLoad();
        }

        virtual uint32_t getMaxEffectsMemory()
        {
            return mEffects.getMaxEffectsMemory();
        }

        SortedVector<audio_io_handle_t> getOutputsForDevices(
                const DeviceVector &devices, const SwAudioOutputCollection& openOutputs);

        /**
         * @brief checkDeviceMuteStrategies mute/unmute strategies
         *      using an incompatible device combination.
         *      if muting, wait for the audio in pcm buffer to be drained before proceeding
         *      if unmuting, unmute only after the specified delay
         * @param outputDesc
         * @param prevDevice
         * @param delayMs
         * @return the number of ms waited
         */
        virtual uint32_t checkDeviceMuteStrategies(const sp<AudioOutputDescriptor>& outputDesc,
                                                   const DeviceVector &prevDevices,
                                                   uint32_t delayMs);

        audio_io_handle_t selectOutput(const SortedVector<audio_io_handle_t>& outputs,
                                       audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                                       audio_format_t format = AUDIO_FORMAT_INVALID,
                                       audio_channel_mask_t channelMask = AUDIO_CHANNEL_NONE,
                                       uint32_t samplingRate = 0);
        // samplingRate, format, channelMask are in/out and so may be modified
        sp<IOProfile> getInputProfile(const sp<DeviceDescriptor> & device,
                                      uint32_t& samplingRate,
                                      audio_format_t& format,
                                      audio_channel_mask_t& channelMask,
                                      audio_input_flags_t flags);
        /**
         * @brief getProfileForOutput
         * @param devices vector of descriptors, may be empty if ignoring the device is required
         * @param samplingRate
         * @param format
         * @param channelMask
         * @param flags
         * @param directOnly
         * @return IOProfile to be used if found, nullptr otherwise
         */
        sp<IOProfile> getProfileForOutput(const DeviceVector &devices,
                                          uint32_t samplingRate,
                                          audio_format_t format,
                                          audio_channel_mask_t channelMask,
                                          audio_output_flags_t flags,
                                          bool directOnly);

        audio_io_handle_t selectOutputForMusicEffects();

        virtual status_t addAudioPatch(audio_patch_handle_t handle, const sp<AudioPatch>& patch)
        {
            return mAudioPatches.addAudioPatch(handle, patch);
        }
        virtual status_t removeAudioPatch(audio_patch_handle_t handle)
        {
            return mAudioPatches.removeAudioPatch(handle);
        }

        bool isPrimaryModule(const sp<HwModule> &module) const
        {
            if (module == 0 || !hasPrimaryOutput()) {
                return false;
            }
            return module->getHandle() == mPrimaryOutput->getModuleHandle();
        }
        DeviceVector availablePrimaryOutputDevices() const
        {
            if (!hasPrimaryOutput()) {
                return DeviceVector();
            }
            return mAvailableOutputDevices.filter(mPrimaryOutput->supportedDevices());
        }
        DeviceVector availablePrimaryModuleInputDevices() const
        {
            if (!hasPrimaryOutput()) {
                return DeviceVector();
            }
            return mAvailableInputDevices.getDevicesFromHwModule(
                    mPrimaryOutput->getModuleHandle());
        }
        /**
         * @brief getFirstDeviceId of the Device Vector
         * @return if the collection is not empty, it returns the first device Id,
         *         otherwise AUDIO_PORT_HANDLE_NONE
         */
        audio_port_handle_t getFirstDeviceId(const DeviceVector &devices) const
        {
            return (devices.size() > 0) ? devices.itemAt(0)->getId() : AUDIO_PORT_HANDLE_NONE;
        }
        String8 getFirstDeviceAddress(const DeviceVector &devices) const
        {
            return (devices.size() > 0) ? devices.itemAt(0)->address() : String8("");
        }

        uint32_t updateCallRouting(const DeviceVector &rxDevices, uint32_t delayMs = 0);
        sp<AudioPatch> createTelephonyPatch(bool isRx, const sp<DeviceDescriptor> &device,
                                            uint32_t delayMs);
        sp<DeviceDescriptor> findDevice(
                const DeviceVector& devices, audio_devices_t device) const;
        audio_devices_t getModuleDeviceTypes(
                const DeviceVector& devices, const char *moduleId) const;
        bool isDeviceOfModule(const sp<DeviceDescriptor>& devDesc, const char *moduleId) const;

        status_t startSource(const sp<SwAudioOutputDescriptor>& outputDesc,
                             const sp<TrackClientDescriptor>& client,
                             uint32_t *delayMs);
        status_t stopSource(const sp<SwAudioOutputDescriptor>& outputDesc,
                            const sp<TrackClientDescriptor>& client);

        void clearAudioPatches(uid_t uid);
        void clearSessionRoutes(uid_t uid);

        /**
         * @brief checkStrategyRoute: when an output is beeing rerouted, reconsider each output
         * that may host a strategy playing on the considered output.
         * @param ps product strategy that initiated the rerouting
         * @param ouptutToSkip output that initiated the rerouting
         */
        void checkStrategyRoute(product_strategy_t ps, audio_io_handle_t ouptutToSkip);

        status_t hasPrimaryOutput() const { return mPrimaryOutput != 0; }

        status_t connectAudioSource(const sp<SourceClientDescriptor>& sourceDesc);
        status_t disconnectAudioSource(const sp<SourceClientDescriptor>& sourceDesc);

        sp<SourceClientDescriptor> getSourceForAttributesOnOutput(audio_io_handle_t output,
                                                                  const audio_attributes_t &attr);

        void cleanUpForDevice(const sp<DeviceDescriptor>& deviceDesc);

        void clearAudioSources(uid_t uid);

        static bool streamsMatchForvolume(audio_stream_type_t stream1,
                                          audio_stream_type_t stream2);

        void closeActiveClients(const sp<AudioInputDescriptor>& input);
        void closeClient(audio_port_handle_t portId);

        const uid_t mUidCached;                         // AID_AUDIOSERVER
        AudioPolicyClientInterface *mpClientInterface;  // audio policy client interface
        sp<SwAudioOutputDescriptor> mPrimaryOutput;     // primary output descriptor
        // list of descriptors for outputs currently opened

        SwAudioOutputCollection mOutputs;
        // copy of mOutputs before setDeviceConnectionState() opens new outputs
        // reset to mOutputs when updateDevicesAndOutputs() is called.
        SwAudioOutputCollection mPreviousOutputs;
        AudioInputCollection mInputs;     // list of input descriptors

        DeviceVector  mAvailableOutputDevices; // all available output devices
        DeviceVector  mAvailableInputDevices;  // all available input devices

        bool    mLimitRingtoneVolume;        // limit ringtone volume to music volume if headset connected

        float   mLastVoiceVolume;            // last voice volume value sent to audio HAL
        bool    mA2dpSuspended;  // true if A2DP output is suspended

        EffectDescriptorCollection mEffects;  // list of registered audio effects
        sp<DeviceDescriptor> mDefaultOutputDevice; // output device selected by default at boot time
        HwModuleCollection mHwModules; // contains only modules that have been loaded successfully
        HwModuleCollection mHwModulesAll; // normally not needed, used during construction and for
                                          // dumps

        AudioPolicyConfig mConfig;

        std::atomic<uint32_t> mAudioPortGeneration;

        AudioPatchCollection mAudioPatches;

        SoundTriggerSessionCollection mSoundTriggerSessions;

        sp<AudioPatch> mCallTxPatch;
        sp<AudioPatch> mCallRxPatch;

        HwAudioOutputCollection mHwOutputs;
        SourceClientCollection mAudioSources;

        // for supporting "beacon" streams, i.e. streams that only play on speaker, and never
        // when something other than STREAM_TTS (a.k.a. "Transmitted Through Speaker") is playing
        enum {
            STARTING_OUTPUT,
            STARTING_BEACON,
            STOPPING_OUTPUT,
            STOPPING_BEACON
        };
        uint32_t mBeaconMuteRefCount;   // ref count for stream that would mute beacon
        uint32_t mBeaconPlayingRefCount;// ref count for the playing beacon streams
        bool mBeaconMuted;              // has STREAM_TTS been muted
        bool mTtsOutputAvailable;       // true if a dedicated output for TTS stream is available

        bool mMasterMono;               // true if we wish to force all outputs to mono
        AudioPolicyMixCollection mPolicyMixes; // list of registered mixes
        audio_io_handle_t mMusicEffectOutput;     // output selected for music effects

        uint32_t nextAudioPortGeneration();

        // Audio Policy Engine Interface.
        AudioPolicyManagerInterface *mEngine;

        // Surround formats that are enabled manually. Taken into account when
        // "encoded surround" is forced into "manual" mode.
        std::unordered_set<audio_format_t> mManualSurroundFormats;

        std::unordered_map<uid_t, audio_flags_mask_t> mAllowedCapturePolicies;
private:
        // Add or remove AC3 DTS encodings based on user preferences.
        void modifySurroundFormats(const sp<DeviceDescriptor>& devDesc, FormatVector *formatsPtr);
        void modifySurroundChannelMasks(ChannelsVector *channelMasksPtr);

        // Support for Multi-Stream Decoder (MSD) module
        sp<DeviceDescriptor> getMsdAudioInDevice() const;
        DeviceVector getMsdAudioOutDevices() const;
        const AudioPatchCollection getMsdPatches() const;
        status_t getBestMsdAudioProfileFor(const sp<DeviceDescriptor> &outputDevice,
                                           bool hwAvSync,
                                           audio_port_config *sourceConfig,
                                           audio_port_config *sinkConfig) const;
        PatchBuilder buildMsdPatch(const sp<DeviceDescriptor> &outputDevice) const;
        status_t setMsdPatch(const sp<DeviceDescriptor> &outputDevice = nullptr);

        // If any, resolve any "dynamic" fields of an Audio Profiles collection
        void updateAudioProfiles(const sp<DeviceDescriptor>& devDesc, audio_io_handle_t ioHandle,
                AudioProfileVector &profiles);

        // Notify the policy client of any change of device state with AUDIO_IO_HANDLE_NONE,
        // so that the client interprets it as global to audio hardware interfaces.
        // It can give a chance to HAL implementer to retrieve dynamic capabilities associated
        // to this device for example.
        // TODO avoid opening stream to retrieve capabilities of a profile.
        void broadcastDeviceConnectionState(const sp<DeviceDescriptor> &device,
                                            audio_policy_dev_state_t state);

        // updates device caching and output for streams that can influence the
        //    routing of notifications
        void handleNotificationRoutingForStream(audio_stream_type_t stream);
        uint32_t curAudioPortGeneration() const { return mAudioPortGeneration; }
        // internal method, get audio_attributes_t from either a source audio_attributes_t
        // or audio_stream_type_t, respectively.
        status_t getAudioAttributes(audio_attributes_t *dstAttr,
                const audio_attributes_t *srcAttr,
                audio_stream_type_t srcStream);
        // internal method, called by getOutputForAttr() and connectAudioSource.
        status_t getOutputForAttrInt(audio_attributes_t *resultAttr,
                audio_io_handle_t *output,
                audio_session_t session,
                const audio_attributes_t *attr,
                audio_stream_type_t *stream,
                uid_t uid,
                const audio_config_t *config,
                audio_output_flags_t *flags,
                audio_port_handle_t *selectedDeviceId,
                bool *isRequestedDeviceForExclusiveUse,
                std::vector<sp<SwAudioOutputDescriptor>> *secondaryDescs);
        // internal method to return the output handle for the given device and format
        audio_io_handle_t getOutputForDevices(
                const DeviceVector &devices,
                audio_session_t session,
                audio_stream_type_t stream,
                const audio_config_t *config,
                audio_output_flags_t *flags,
                bool forceMutingHaptic = false);

        /**
         * @brief getInputForDevice selects an input handle for a given input device and
         * requester context
         * @param device to be used by requester, selected by policy mix rules or engine
         * @param session requester session id
         * @param uid requester uid
         * @param attributes requester audio attributes (e.g. input source and tags matter)
         * @param config requester audio configuration (e.g. sample rate, format, channel mask).
         * @param flags requester input flags
         * @param policyMix may be null, policy rules to be followed by the requester
         * @return input io handle aka unique input identifier selected for this device.
         */
        audio_io_handle_t getInputForDevice(const sp<DeviceDescriptor> &device,
                audio_session_t session,
                const audio_attributes_t &attributes,
                const audio_config_base_t *config,
                audio_input_flags_t flags,
                const sp<AudioPolicyMix> &policyMix);

        // event is one of STARTING_OUTPUT, STARTING_BEACON, STOPPING_OUTPUT, STOPPING_BEACON
        // returns 0 if no mute/unmute event happened, the largest latency of the device where
        //   the mute/unmute happened
        uint32_t handleEventForBeacon(int event);
        uint32_t setBeaconMute(bool mute);
        bool     isValidAttributes(const audio_attributes_t *paa);

        // Called by setDeviceConnectionState().
        status_t setDeviceConnectionStateInt(audio_devices_t deviceType,
                                             audio_policy_dev_state_t state,
                                             const char *device_address,
                                             const char *device_name,
                                             audio_format_t encodedFormat);

        void setEngineDeviceConnectionState(const sp<DeviceDescriptor> device,
                                      audio_policy_dev_state_t state);

        void updateMono(audio_io_handle_t output) {
            AudioParameter param;
            param.addInt(String8(AudioParameter::keyMonoOutput), (int)mMasterMono);
            mpClientInterface->setParameters(output, param.toString());
        }
        status_t installPatch(const char *caller,
                audio_patch_handle_t *patchHandle,
                AudioIODescriptorInterface *ioDescriptor,
                const struct audio_patch *patch,
                int delayMs);
        status_t installPatch(const char *caller,
                ssize_t index,
                audio_patch_handle_t *patchHandle,
                const struct audio_patch *patch,
                int delayMs,
                uid_t uid,
                sp<AudioPatch> *patchDescPtr);

        void cleanUpEffectsForIo(audio_io_handle_t io);
};

};
