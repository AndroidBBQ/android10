/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_FLINGER_H
#define ANDROID_AUDIO_FLINGER_H

#include "Configuration.h"
#include <atomic>
#include <mutex>
#include <chrono>
#include <deque>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#include <android/os/BnExternalVibrationController.h>
#include <android-base/macros.h>

#include <cutils/atomic.h>
#include <cutils/compiler.h>
#include <cutils/properties.h>

#include <media/IAudioFlinger.h>
#include <media/IAudioFlingerClient.h>
#include <media/IAudioTrack.h>
#include <media/AudioSystem.h>
#include <media/AudioTrack.h>
#include <media/MmapStreamInterface.h>
#include <media/MmapStreamCallback.h>

#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>

#include <binder/AppOpsManager.h>
#include <binder/BinderService.h>
#include <binder/IAppOpsCallback.h>
#include <binder/MemoryDealer.h>

#include <system/audio.h>
#include <system/audio_policy.h>

#include <media/audiohal/EffectBufferHalInterface.h>
#include <media/audiohal/StreamHalInterface.h>
#include <media/AudioBufferProvider.h>
#include <media/AudioMixer.h>
#include <media/ExtendedAudioBufferProvider.h>
#include <media/LinearMap.h>
#include <media/VolumeShaper.h>

#include <audio_utils/clock.h>
#include <audio_utils/FdToString.h>
#include <audio_utils/SimpleLog.h>
#include <audio_utils/TimestampVerifier.h>

#include "FastCapture.h"
#include "FastMixer.h"
#include <media/nbaio/NBAIO.h>
#include "AudioWatchdog.h"
#include "AudioStreamOut.h"
#include "SpdifStreamOut.h"
#include "AudioHwDevice.h"
#include "NBAIO_Tee.h"

#include <powermanager/IPowerManager.h>

#include <media/nblog/NBLog.h>
#include <private/media/AudioEffectShared.h>
#include <private/media/AudioTrackShared.h>

#include <vibrator/ExternalVibration.h>

#include "android/media/BnAudioRecord.h"

namespace android {

class AudioMixer;
class AudioBuffer;
class AudioResampler;
class DeviceHalInterface;
class DevicesFactoryHalInterface;
class EffectsFactoryHalInterface;
class FastMixer;
class PassthruBufferProvider;
class RecordBufferConverter;
class ServerProxy;

// ----------------------------------------------------------------------------

static const nsecs_t kDefaultStandbyTimeInNsecs = seconds(3);

#define INCLUDING_FROM_AUDIOFLINGER_H

class AudioFlinger :
    public BinderService<AudioFlinger>,
    public BnAudioFlinger
{
    friend class BinderService<AudioFlinger>;   // for AudioFlinger()

public:
    static const char* getServiceName() ANDROID_API { return "media.audio_flinger"; }

    virtual     status_t    dump(int fd, const Vector<String16>& args);

    // IAudioFlinger interface, in binder opcode order
    virtual sp<IAudioTrack> createTrack(const CreateTrackInput& input,
                                        CreateTrackOutput& output,
                                        status_t *status);

    virtual sp<media::IAudioRecord> createRecord(const CreateRecordInput& input,
                                                 CreateRecordOutput& output,
                                                 status_t *status);

    virtual     uint32_t    sampleRate(audio_io_handle_t ioHandle) const;
    virtual     audio_format_t format(audio_io_handle_t output) const;
    virtual     size_t      frameCount(audio_io_handle_t ioHandle) const;
    virtual     size_t      frameCountHAL(audio_io_handle_t ioHandle) const;
    virtual     uint32_t    latency(audio_io_handle_t output) const;

    virtual     status_t    setMasterVolume(float value);
    virtual     status_t    setMasterMute(bool muted);

    virtual     float       masterVolume() const;
    virtual     bool        masterMute() const;

    // Balance value must be within -1.f (left only) to 1.f (right only) inclusive.
                status_t    setMasterBalance(float balance) override;
                status_t    getMasterBalance(float *balance) const override;

    virtual     status_t    setStreamVolume(audio_stream_type_t stream, float value,
                                            audio_io_handle_t output);
    virtual     status_t    setStreamMute(audio_stream_type_t stream, bool muted);

    virtual     float       streamVolume(audio_stream_type_t stream,
                                         audio_io_handle_t output) const;
    virtual     bool        streamMute(audio_stream_type_t stream) const;

    virtual     status_t    setMode(audio_mode_t mode);

    virtual     status_t    setMicMute(bool state);
    virtual     bool        getMicMute() const;

    virtual     void        setRecordSilenced(uid_t uid, bool silenced);

    virtual     status_t    setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs);
    virtual     String8     getParameters(audio_io_handle_t ioHandle, const String8& keys) const;

    virtual     void        registerClient(const sp<IAudioFlingerClient>& client);

    virtual     size_t      getInputBufferSize(uint32_t sampleRate, audio_format_t format,
                                               audio_channel_mask_t channelMask) const;

    virtual status_t openOutput(audio_module_handle_t module,
                                audio_io_handle_t *output,
                                audio_config_t *config,
                                audio_devices_t *devices,
                                const String8& address,
                                uint32_t *latencyMs,
                                audio_output_flags_t flags);

    virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1,
                                                  audio_io_handle_t output2);

    virtual status_t closeOutput(audio_io_handle_t output);

    virtual status_t suspendOutput(audio_io_handle_t output);

    virtual status_t restoreOutput(audio_io_handle_t output);

    virtual status_t openInput(audio_module_handle_t module,
                               audio_io_handle_t *input,
                               audio_config_t *config,
                               audio_devices_t *device,
                               const String8& address,
                               audio_source_t source,
                               audio_input_flags_t flags);

    virtual status_t closeInput(audio_io_handle_t input);

    virtual status_t invalidateStream(audio_stream_type_t stream);

    virtual status_t setVoiceVolume(float volume);

    virtual status_t getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames,
                                       audio_io_handle_t output) const;

    virtual uint32_t getInputFramesLost(audio_io_handle_t ioHandle) const;

    // This is the binder API.  For the internal API see nextUniqueId().
    virtual audio_unique_id_t newAudioUniqueId(audio_unique_id_use_t use);

    virtual void acquireAudioSessionId(audio_session_t audioSession, pid_t pid);

    virtual void releaseAudioSessionId(audio_session_t audioSession, pid_t pid);

    virtual status_t queryNumberEffects(uint32_t *numEffects) const;

    virtual status_t queryEffect(uint32_t index, effect_descriptor_t *descriptor) const;

    virtual status_t getEffectDescriptor(const effect_uuid_t *pUuid,
                                         const effect_uuid_t *pTypeUuid,
                                         uint32_t preferredTypeFlag,
                                         effect_descriptor_t *descriptor) const;

    virtual sp<IEffect> createEffect(
                        effect_descriptor_t *pDesc,
                        const sp<IEffectClient>& effectClient,
                        int32_t priority,
                        audio_io_handle_t io,
                        audio_session_t sessionId,
                        const String16& opPackageName,
                        pid_t pid,
                        status_t *status /*non-NULL*/,
                        int *id,
                        int *enabled);

    virtual status_t moveEffects(audio_session_t sessionId, audio_io_handle_t srcOutput,
                        audio_io_handle_t dstOutput);

            void setEffectSuspended(int effectId,
                                    audio_session_t sessionId,
                                    bool suspended) override;

    virtual audio_module_handle_t loadHwModule(const char *name);

    virtual uint32_t getPrimaryOutputSamplingRate();
    virtual size_t getPrimaryOutputFrameCount();

    virtual status_t setLowRamDevice(bool isLowRamDevice, int64_t totalMemory) override;

    /* List available audio ports and their attributes */
    virtual status_t listAudioPorts(unsigned int *num_ports,
                                    struct audio_port *ports);

    /* Get attributes for a given audio port */
    virtual status_t getAudioPort(struct audio_port *port);

    /* Create an audio patch between several source and sink ports */
    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle);

    /* Release an audio patch */
    virtual status_t releaseAudioPatch(audio_patch_handle_t handle);

    /* List existing audio patches */
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches);

    /* Set audio port configuration */
    virtual status_t setAudioPortConfig(const struct audio_port_config *config);

    /* Get the HW synchronization source used for an audio session */
    virtual audio_hw_sync_t getAudioHwSyncForSession(audio_session_t sessionId);

    /* Indicate JAVA services are ready (scheduling, power management ...) */
    virtual status_t systemReady();

    virtual status_t getMicrophones(std::vector<media::MicrophoneInfo> *microphones);

    virtual     status_t    onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags);

    // end of IAudioFlinger interface

    sp<NBLog::Writer>   newWriter_l(size_t size, const char *name);
    void                unregisterWriter(const sp<NBLog::Writer>& writer);
    sp<EffectsFactoryHalInterface> getEffectsFactory();

    status_t openMmapStream(MmapStreamInterface::stream_direction_t direction,
                            const audio_attributes_t *attr,
                            audio_config_base_t *config,
                            const AudioClient& client,
                            audio_port_handle_t *deviceId,
                            audio_session_t *sessionId,
                            const sp<MmapStreamCallback>& callback,
                            sp<MmapStreamInterface>& interface,
                            audio_port_handle_t *handle);

    static int onExternalVibrationStart(const sp<os::ExternalVibration>& externalVibration);
    static void onExternalVibrationStop(const sp<os::ExternalVibration>& externalVibration);
private:
    // FIXME The 400 is temporarily too high until a leak of writers in media.log is fixed.
    static const size_t kLogMemorySize = 400 * 1024;
    sp<MemoryDealer>    mLogMemoryDealer;   // == 0 when NBLog is disabled
    // When a log writer is unregistered, it is done lazily so that media.log can continue to see it
    // for as long as possible.  The memory is only freed when it is needed for another log writer.
    Vector< sp<NBLog::Writer> > mUnregisteredWriters;
    Mutex               mUnregisteredWritersLock;

public:

    class SyncEvent;

    typedef void (*sync_event_callback_t)(const wp<SyncEvent>& event) ;

    class SyncEvent : public RefBase {
    public:
        SyncEvent(AudioSystem::sync_event_t type,
                  audio_session_t triggerSession,
                  audio_session_t listenerSession,
                  sync_event_callback_t callBack,
                  wp<RefBase> cookie)
        : mType(type), mTriggerSession(triggerSession), mListenerSession(listenerSession),
          mCallback(callBack), mCookie(cookie)
        {}

        virtual ~SyncEvent() {}

        void trigger() { Mutex::Autolock _l(mLock); if (mCallback) mCallback(this); }
        bool isCancelled() const { Mutex::Autolock _l(mLock); return (mCallback == NULL); }
        void cancel() { Mutex::Autolock _l(mLock); mCallback = NULL; }
        AudioSystem::sync_event_t type() const { return mType; }
        audio_session_t triggerSession() const { return mTriggerSession; }
        audio_session_t listenerSession() const { return mListenerSession; }
        wp<RefBase> cookie() const { return mCookie; }

    private:
          const AudioSystem::sync_event_t mType;
          const audio_session_t mTriggerSession;
          const audio_session_t mListenerSession;
          sync_event_callback_t mCallback;
          const wp<RefBase> mCookie;
          mutable Mutex mLock;
    };

    sp<SyncEvent> createSyncEvent(AudioSystem::sync_event_t type,
                                        audio_session_t triggerSession,
                                        audio_session_t listenerSession,
                                        sync_event_callback_t callBack,
                                        const wp<RefBase>& cookie);

    bool        btNrecIsOff() const { return mBtNrecIsOff.load(); }


private:

               audio_mode_t getMode() const { return mMode; }

                            AudioFlinger() ANDROID_API;
    virtual                 ~AudioFlinger();

    // call in any IAudioFlinger method that accesses mPrimaryHardwareDev
    status_t                initCheck() const { return mPrimaryHardwareDev == NULL ?
                                                        NO_INIT : NO_ERROR; }

    // RefBase
    virtual     void        onFirstRef();

    AudioHwDevice*          findSuitableHwDev_l(audio_module_handle_t module,
                                                audio_devices_t devices);

    // Set kEnableExtendedChannels to true to enable greater than stereo output
    // for the MixerThread and device sink.  Number of channels allowed is
    // FCC_2 <= channels <= AudioMixer::MAX_NUM_CHANNELS.
    static const bool kEnableExtendedChannels = true;

    // Returns true if channel mask is permitted for the PCM sink in the MixerThread
    static inline bool isValidPcmSinkChannelMask(audio_channel_mask_t channelMask) {
        switch (audio_channel_mask_get_representation(channelMask)) {
        case AUDIO_CHANNEL_REPRESENTATION_POSITION: {
            // Haptic channel mask is only applicable for channel position mask.
            const uint32_t channelCount = audio_channel_count_from_out_mask(
                    channelMask & ~AUDIO_CHANNEL_HAPTIC_ALL);
            const uint32_t maxChannelCount = kEnableExtendedChannels
                    ? AudioMixer::MAX_NUM_CHANNELS : FCC_2;
            if (channelCount < FCC_2 // mono is not supported at this time
                    || channelCount > maxChannelCount) {
                return false;
            }
            // check that channelMask is the "canonical" one we expect for the channelCount.
            return audio_channel_position_mask_is_out_canonical(channelMask);
            }
        case AUDIO_CHANNEL_REPRESENTATION_INDEX:
            if (kEnableExtendedChannels) {
                const uint32_t channelCount = audio_channel_count_from_out_mask(channelMask);
                if (channelCount >= FCC_2 // mono is not supported at this time
                        && channelCount <= AudioMixer::MAX_NUM_CHANNELS) {
                    return true;
                }
            }
            return false;
        default:
            return false;
        }
    }

    // Set kEnableExtendedPrecision to true to use extended precision in MixerThread
    static const bool kEnableExtendedPrecision = true;

    // Returns true if format is permitted for the PCM sink in the MixerThread
    static inline bool isValidPcmSinkFormat(audio_format_t format) {
        switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            return true;
        case AUDIO_FORMAT_PCM_FLOAT:
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        case AUDIO_FORMAT_PCM_32_BIT:
        case AUDIO_FORMAT_PCM_8_24_BIT:
            return kEnableExtendedPrecision;
        default:
            return false;
        }
    }

    // standby delay for MIXER and DUPLICATING playback threads is read from property
    // ro.audio.flinger_standbytime_ms or defaults to kDefaultStandbyTimeInNsecs
    static nsecs_t          mStandbyTimeInNsecs;

    // incremented by 2 when screen state changes, bit 0 == 1 means "off"
    // AudioFlinger::setParameters() updates, other threads read w/o lock
    static uint32_t         mScreenState;

    // Internal dump utilities.
    static const int kDumpLockTimeoutNs = 1 * NANOS_PER_SECOND;
    static bool dumpTryLock(Mutex& mutex);
    void dumpPermissionDenial(int fd, const Vector<String16>& args);
    void dumpClients(int fd, const Vector<String16>& args);
    void dumpInternals(int fd, const Vector<String16>& args);

    SimpleLog mThreadLog{16}; // 16 Thread history limit

    class ThreadBase;
    void dumpToThreadLog_l(const sp<ThreadBase> &thread);

    // --- Client ---
    class Client : public RefBase {
    public:
                            Client(const sp<AudioFlinger>& audioFlinger, pid_t pid);
        virtual             ~Client();
        sp<MemoryDealer>    heap() const;
        pid_t               pid() const { return mPid; }
        sp<AudioFlinger>    audioFlinger() const { return mAudioFlinger; }

    private:
        DISALLOW_COPY_AND_ASSIGN(Client);

        const sp<AudioFlinger> mAudioFlinger;
              sp<MemoryDealer> mMemoryDealer;
        const pid_t         mPid;
    };

    // --- Notification Client ---
    class NotificationClient : public IBinder::DeathRecipient {
    public:
                            NotificationClient(const sp<AudioFlinger>& audioFlinger,
                                                const sp<IAudioFlingerClient>& client,
                                                pid_t pid);
        virtual             ~NotificationClient();

                sp<IAudioFlingerClient> audioFlingerClient() const { return mAudioFlingerClient; }

                // IBinder::DeathRecipient
                virtual     void        binderDied(const wp<IBinder>& who);

    private:
        DISALLOW_COPY_AND_ASSIGN(NotificationClient);

        const sp<AudioFlinger>  mAudioFlinger;
        const pid_t             mPid;
        const sp<IAudioFlingerClient> mAudioFlingerClient;
    };

    // --- MediaLogNotifier ---
    // Thread in charge of notifying MediaLogService to start merging.
    // Receives requests from AudioFlinger's binder activity. It is used to reduce the amount of
    // binder calls to MediaLogService in case of bursts of AudioFlinger binder calls.
    class MediaLogNotifier : public Thread {
    public:
        MediaLogNotifier();

        // Requests a MediaLogService notification. It's ignored if there has recently been another
        void requestMerge();
    private:
        // Every iteration blocks waiting for a request, then interacts with MediaLogService to
        // start merging.
        // As every MediaLogService binder call is expensive, once it gets a request it ignores the
        // following ones for a period of time.
        virtual bool threadLoop() override;

        bool mPendingRequests;

        // Mutex and condition variable around mPendingRequests' value
        Mutex       mMutex;
        Condition   mCond;

        // Duration of the sleep period after a processed request
        static const int kPostTriggerSleepPeriod = 1000000;
    };

    const sp<MediaLogNotifier> mMediaLogNotifier;

    // This is a helper that is called during incoming binder calls.
    void requestLogMerge();

    class TrackHandle;
    class RecordHandle;
    class RecordThread;
    class PlaybackThread;
    class MixerThread;
    class DirectOutputThread;
    class OffloadThread;
    class DuplicatingThread;
    class AsyncCallbackThread;
    class Track;
    class RecordTrack;
    class EffectModule;
    class EffectHandle;
    class EffectChain;

    struct AudioStreamIn;
    struct TeePatch;
    using TeePatches = std::vector<TeePatch>;


    struct  stream_type_t {
        stream_type_t()
            :   volume(1.0f),
                mute(false)
        {
        }
        float       volume;
        bool        mute;
    };

    // --- PlaybackThread ---
#ifdef FLOAT_EFFECT_CHAIN
#define EFFECT_BUFFER_FORMAT AUDIO_FORMAT_PCM_FLOAT
using effect_buffer_t = float;
#else
#define EFFECT_BUFFER_FORMAT AUDIO_FORMAT_PCM_16_BIT
using effect_buffer_t = int16_t;
#endif

#include "Threads.h"

#include "Effects.h"

#include "PatchPanel.h"

    // Find io handle by session id.
    // Preference is given to an io handle with a matching effect chain to session id.
    // If none found, AUDIO_IO_HANDLE_NONE is returned.
    template <typename T>
    static audio_io_handle_t findIoHandleBySessionId_l(
            audio_session_t sessionId, const T& threads) {
        audio_io_handle_t io = AUDIO_IO_HANDLE_NONE;

        for (size_t i = 0; i < threads.size(); i++) {
            const uint32_t sessionType = threads.valueAt(i)->hasAudioSession(sessionId);
            if (sessionType != 0) {
                io = threads.keyAt(i);
                if ((sessionType & AudioFlinger::ThreadBase::EFFECT_SESSION) != 0) {
                    break; // effect chain here.
                }
            }
        }
        return io;
    }

    // server side of the client's IAudioTrack
    class TrackHandle : public android::BnAudioTrack {
    public:
        explicit            TrackHandle(const sp<PlaybackThread::Track>& track);
        virtual             ~TrackHandle();
        virtual sp<IMemory> getCblk() const;
        virtual status_t    start();
        virtual void        stop();
        virtual void        flush();
        virtual void        pause();
        virtual status_t    attachAuxEffect(int effectId);
        virtual status_t    setParameters(const String8& keyValuePairs);
        virtual status_t    selectPresentation(int presentationId, int programId);
        virtual media::VolumeShaper::Status applyVolumeShaper(
                const sp<media::VolumeShaper::Configuration>& configuration,
                const sp<media::VolumeShaper::Operation>& operation) override;
        virtual sp<media::VolumeShaper::State> getVolumeShaperState(int id) override;
        virtual status_t    getTimestamp(AudioTimestamp& timestamp);
        virtual void        signal(); // signal playback thread for a change in control block

        virtual status_t onTransact(
            uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);

    private:
        const sp<PlaybackThread::Track> mTrack;
    };

    // server side of the client's IAudioRecord
    class RecordHandle : public android::media::BnAudioRecord {
    public:
        explicit RecordHandle(const sp<RecordThread::RecordTrack>& recordTrack);
        virtual             ~RecordHandle();
        virtual binder::Status    start(int /*AudioSystem::sync_event_t*/ event,
                int /*audio_session_t*/ triggerSession);
        virtual binder::Status   stop();
        virtual binder::Status   getActiveMicrophones(
                std::vector<media::MicrophoneInfo>* activeMicrophones);
        virtual binder::Status   setPreferredMicrophoneDirection(
                int /*audio_microphone_direction_t*/ direction);
        virtual binder::Status   setPreferredMicrophoneFieldDimension(float zoom);

    private:
        const sp<RecordThread::RecordTrack> mRecordTrack;

        // for use from destructor
        void                stop_nonvirtual();
    };

    // Mmap stream control interface implementation. Each MmapThreadHandle controls one
    // MmapPlaybackThread or MmapCaptureThread instance.
    class MmapThreadHandle : public MmapStreamInterface {
    public:
        explicit            MmapThreadHandle(const sp<MmapThread>& thread);
        virtual             ~MmapThreadHandle();

        // MmapStreamInterface virtuals
        virtual status_t createMmapBuffer(int32_t minSizeFrames,
                                          struct audio_mmap_buffer_info *info);
        virtual status_t getMmapPosition(struct audio_mmap_position *position);
        virtual status_t start(const AudioClient& client,
                                         audio_port_handle_t *handle);
        virtual status_t stop(audio_port_handle_t handle);
        virtual status_t standby();

    private:
        const sp<MmapThread> mThread;
    };

              ThreadBase *checkThread_l(audio_io_handle_t ioHandle) const;
              PlaybackThread *checkPlaybackThread_l(audio_io_handle_t output) const;
              MixerThread *checkMixerThread_l(audio_io_handle_t output) const;
              RecordThread *checkRecordThread_l(audio_io_handle_t input) const;
              MmapThread *checkMmapThread_l(audio_io_handle_t io) const;
              VolumeInterface *getVolumeInterface_l(audio_io_handle_t output) const;
              Vector <VolumeInterface *> getAllVolumeInterfaces_l() const;

              sp<ThreadBase> openInput_l(audio_module_handle_t module,
                                           audio_io_handle_t *input,
                                           audio_config_t *config,
                                           audio_devices_t device,
                                           const String8& address,
                                           audio_source_t source,
                                           audio_input_flags_t flags,
                                           audio_devices_t outputDevice,
                                           const String8& outputDeviceAddress);
              sp<ThreadBase> openOutput_l(audio_module_handle_t module,
                                              audio_io_handle_t *output,
                                              audio_config_t *config,
                                              audio_devices_t devices,
                                              const String8& address,
                                              audio_output_flags_t flags);

              void closeOutputFinish(const sp<PlaybackThread>& thread);
              void closeInputFinish(const sp<RecordThread>& thread);

              // no range check, AudioFlinger::mLock held
              bool streamMute_l(audio_stream_type_t stream) const
                                { return mStreamTypes[stream].mute; }
              void ioConfigChanged(audio_io_config_event event,
                                   const sp<AudioIoDescriptor>& ioDesc,
                                   pid_t pid = 0);

              // Allocate an audio_unique_id_t.
              // Specific types are audio_io_handle_t, audio_session_t, effect ID (int),
              // audio_module_handle_t, and audio_patch_handle_t.
              // They all share the same ID space, but the namespaces are actually independent
              // because there are separate KeyedVectors for each kind of ID.
              // The return value is cast to the specific type depending on how the ID will be used.
              // FIXME This API does not handle rollover to zero (for unsigned IDs),
              //       or from positive to negative (for signed IDs).
              //       Thus it may fail by returning an ID of the wrong sign,
              //       or by returning a non-unique ID.
              // This is the internal API.  For the binder API see newAudioUniqueId().
              audio_unique_id_t nextUniqueId(audio_unique_id_use_t use);

              status_t moveEffectChain_l(audio_session_t sessionId,
                                     PlaybackThread *srcThread,
                                     PlaybackThread *dstThread);

              status_t moveAuxEffectToIo(int EffectId,
                                         const sp<PlaybackThread>& dstThread,
                                         sp<PlaybackThread> *srcThread);

              // return thread associated with primary hardware device, or NULL
              PlaybackThread *primaryPlaybackThread_l() const;
              audio_devices_t primaryOutputDevice_l() const;

              // return the playback thread with smallest HAL buffer size, and prefer fast
              PlaybackThread *fastPlaybackThread_l() const;

              sp<ThreadBase> getEffectThread_l(audio_session_t sessionId, int effectId);


                void        removeClient_l(pid_t pid);
                void        removeNotificationClient(pid_t pid);
                bool isNonOffloadableGlobalEffectEnabled_l();
                void onNonOffloadableGlobalEffectEnable();
                bool isSessionAcquired_l(audio_session_t audioSession);

                // Store an effect chain to mOrphanEffectChains keyed vector.
                // Called when a thread exits and effects are still attached to it.
                // If effects are later created on the same session, they will reuse the same
                // effect chain and same instances in the effect library.
                // return ALREADY_EXISTS if a chain with the same session already exists in
                // mOrphanEffectChains. Note that this should never happen as there is only one
                // chain for a given session and it is attached to only one thread at a time.
                status_t        putOrphanEffectChain_l(const sp<EffectChain>& chain);
                // Get an effect chain for the specified session in mOrphanEffectChains and remove
                // it if found. Returns 0 if not found (this is the most common case).
                sp<EffectChain> getOrphanEffectChain_l(audio_session_t session);
                // Called when the last effect handle on an effect instance is removed. If this
                // effect belongs to an effect chain in mOrphanEffectChains, the chain is updated
                // and removed from mOrphanEffectChains if it does not contain any effect.
                // Return true if the effect was found in mOrphanEffectChains, false otherwise.
                bool            updateOrphanEffectChains(const sp<EffectModule>& effect);

                std::vector< sp<EffectModule> > purgeStaleEffects_l();

                void broacastParametersToRecordThreads_l(const String8& keyValuePairs);
                void forwardParametersToDownstreamPatches_l(
                        audio_io_handle_t upStream, const String8& keyValuePairs,
                        std::function<bool(const sp<PlaybackThread>&)> useThread = nullptr);

    // AudioStreamIn is immutable, so their fields are const.
    // For emphasis, we could also make all pointers to them be "const *",
    // but that would clutter the code unnecessarily.

    struct AudioStreamIn {
        AudioHwDevice* const audioHwDev;
        sp<StreamInHalInterface> stream;
        audio_input_flags_t flags;

        sp<DeviceHalInterface> hwDev() const { return audioHwDev->hwDevice(); }

        AudioStreamIn(AudioHwDevice *dev, sp<StreamInHalInterface> in, audio_input_flags_t flags) :
            audioHwDev(dev), stream(in), flags(flags) {}
    };

    struct TeePatch {
        sp<RecordThread::PatchRecord> patchRecord;
        sp<PlaybackThread::PatchTrack> patchTrack;
    };

    // for mAudioSessionRefs only
    struct AudioSessionRef {
        AudioSessionRef(audio_session_t sessionid, pid_t pid) :
            mSessionid(sessionid), mPid(pid), mCnt(1) {}
        const audio_session_t mSessionid;
        const pid_t mPid;
        int         mCnt;
    };

    mutable     Mutex                               mLock;
                // protects mClients and mNotificationClients.
                // must be locked after mLock and ThreadBase::mLock if both must be locked
                // avoids acquiring AudioFlinger::mLock from inside thread loop.
    mutable     Mutex                               mClientLock;
                // protected by mClientLock
                DefaultKeyedVector< pid_t, wp<Client> >     mClients;   // see ~Client()

                mutable     Mutex                   mHardwareLock;
                // NOTE: If both mLock and mHardwareLock mutexes must be held,
                // always take mLock before mHardwareLock

                // These two fields are immutable after onFirstRef(), so no lock needed to access
                AudioHwDevice*                      mPrimaryHardwareDev; // mAudioHwDevs[0] or NULL
                DefaultKeyedVector<audio_module_handle_t, AudioHwDevice*>  mAudioHwDevs;

                sp<DevicesFactoryHalInterface> mDevicesFactoryHal;

    // for dump, indicates which hardware operation is currently in progress (but not stream ops)
    enum hardware_call_state {
        AUDIO_HW_IDLE = 0,              // no operation in progress
        AUDIO_HW_INIT,                  // init_check
        AUDIO_HW_OUTPUT_OPEN,           // open_output_stream
        AUDIO_HW_OUTPUT_CLOSE,          // unused
        AUDIO_HW_INPUT_OPEN,            // unused
        AUDIO_HW_INPUT_CLOSE,           // unused
        AUDIO_HW_STANDBY,               // unused
        AUDIO_HW_SET_MASTER_VOLUME,     // set_master_volume
        AUDIO_HW_GET_ROUTING,           // unused
        AUDIO_HW_SET_ROUTING,           // unused
        AUDIO_HW_GET_MODE,              // unused
        AUDIO_HW_SET_MODE,              // set_mode
        AUDIO_HW_GET_MIC_MUTE,          // get_mic_mute
        AUDIO_HW_SET_MIC_MUTE,          // set_mic_mute
        AUDIO_HW_SET_VOICE_VOLUME,      // set_voice_volume
        AUDIO_HW_SET_PARAMETER,         // set_parameters
        AUDIO_HW_GET_INPUT_BUFFER_SIZE, // get_input_buffer_size
        AUDIO_HW_GET_MASTER_VOLUME,     // get_master_volume
        AUDIO_HW_GET_PARAMETER,         // get_parameters
        AUDIO_HW_SET_MASTER_MUTE,       // set_master_mute
        AUDIO_HW_GET_MASTER_MUTE,       // get_master_mute
    };

    mutable     hardware_call_state                 mHardwareStatus;    // for dump only


                DefaultKeyedVector< audio_io_handle_t, sp<PlaybackThread> >  mPlaybackThreads;
                stream_type_t                       mStreamTypes[AUDIO_STREAM_CNT];

                // member variables below are protected by mLock
                float                               mMasterVolume;
                bool                                mMasterMute;
                float                               mMasterBalance = 0.f;
                // end of variables protected by mLock

                DefaultKeyedVector< audio_io_handle_t, sp<RecordThread> >    mRecordThreads;

                // protected by mClientLock
                DefaultKeyedVector< pid_t, sp<NotificationClient> >    mNotificationClients;

                // updated by atomic_fetch_add_explicit
                volatile atomic_uint_fast32_t       mNextUniqueIds[AUDIO_UNIQUE_ID_USE_MAX];

                audio_mode_t                        mMode;
                std::atomic_bool                    mBtNrecIsOff;

                // protected by mLock
                Vector<AudioSessionRef*> mAudioSessionRefs;

                float       masterVolume_l() const;
                float       getMasterBalance_l() const;
                bool        masterMute_l() const;
                audio_module_handle_t loadHwModule_l(const char *name);

                Vector < sp<SyncEvent> > mPendingSyncEvents; // sync events awaiting for a session
                                                             // to be created

                // Effect chains without a valid thread
                DefaultKeyedVector< audio_session_t , sp<EffectChain> > mOrphanEffectChains;

                // list of sessions for which a valid HW A/V sync ID was retrieved from the HAL
                DefaultKeyedVector< audio_session_t , audio_hw_sync_t >mHwAvSyncIds;

                // list of MMAP stream control threads. Those threads allow for wake lock, routing
                // and volume control for activity on the associated MMAP stream at the HAL.
                // Audio data transfer is directly handled by the client creating the MMAP stream
                DefaultKeyedVector< audio_io_handle_t, sp<MmapThread> >  mMmapThreads;

private:
    sp<Client>  registerPid(pid_t pid);    // always returns non-0

    // for use from destructor
    status_t    closeOutput_nonvirtual(audio_io_handle_t output);
    void        closeThreadInternal_l(const sp<PlaybackThread>& thread);
    status_t    closeInput_nonvirtual(audio_io_handle_t input);
    void        closeThreadInternal_l(const sp<RecordThread>& thread);
    void        setAudioHwSyncForSession_l(PlaybackThread *thread, audio_session_t sessionId);

    status_t    checkStreamType(audio_stream_type_t stream) const;

    void        filterReservedParameters(String8& keyValuePairs, uid_t callingUid);
    void        logFilteredParameters(size_t originalKVPSize, const String8& originalKVPs,
                                      size_t rejectedKVPSize, const String8& rejectedKVPs,
                                      uid_t callingUid);

public:
    // These methods read variables atomically without mLock,
    // though the variables are updated with mLock.
    bool    isLowRamDevice() const { return mIsLowRamDevice; }
    size_t getClientSharedHeapSize() const;

private:
    std::atomic<bool> mIsLowRamDevice;
    bool    mIsDeviceTypeKnown;
    int64_t mTotalMemory;
    std::atomic<size_t> mClientSharedHeapSize;
    static constexpr size_t kMinimumClientSharedHeapSizeBytes = 1024 * 1024; // 1MB

    nsecs_t mGlobalEffectEnableTime;  // when a global effect was last enabled

    // protected by mLock
    PatchPanel mPatchPanel;
    sp<EffectsFactoryHalInterface> mEffectsFactoryHal;

    bool       mSystemReady;

    SimpleLog  mRejectedSetParameterLog;
    SimpleLog  mAppSetParameterLog;
    SimpleLog  mSystemSetParameterLog;
};

#undef INCLUDING_FROM_AUDIOFLINGER_H

std::string formatToString(audio_format_t format);
std::string inputFlagsToString(audio_input_flags_t flags);
std::string outputFlagsToString(audio_output_flags_t flags);
std::string devicesToString(audio_devices_t devices);
const char *sourceToString(audio_source_t source);

// ----------------------------------------------------------------------------

} // namespace android

#endif // ANDROID_AUDIO_FLINGER_H
