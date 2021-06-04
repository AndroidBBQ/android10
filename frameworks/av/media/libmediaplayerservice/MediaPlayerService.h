/*
**
** Copyright 2008, The Android Open Source Project
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

#ifndef ANDROID_MEDIAPLAYERSERVICE_H
#define ANDROID_MEDIAPLAYERSERVICE_H

#include <arpa/inet.h>

#include <utils/threads.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <media/MediaPlayerInterface.h>
#include <media/Metadata.h>
#include <media/stagefright/foundation/ABase.h>

#include <system/audio.h>

namespace android {

struct AudioPlaybackRate;
class AudioTrack;
struct AVSyncSettings;
class DeathNotifier;
class IDataSource;
class IMediaRecorder;
class IMediaMetadataRetriever;
class IRemoteDisplay;
class IRemoteDisplayClient;
class MediaRecorderClient;

#define CALLBACK_ANTAGONIZER 0
#if CALLBACK_ANTAGONIZER
class Antagonizer {
public:
    Antagonizer(const sp<MediaPlayerBase::Listener> &listener);
    void start() { mActive = true; }
    void stop() { mActive = false; }
    void kill();
private:
    static const int interval;
    Antagonizer();
    static int callbackThread(void* cookie);
    Mutex                         mLock;
    Condition                     mCondition;
    bool                          mExit;
    bool                          mActive;
    sp<MediaPlayerBase::Listener> mListener;
};
#endif

class MediaPlayerService : public BnMediaPlayerService
{
    class Client;

    class AudioOutput : public MediaPlayerBase::AudioSink
    {
        class CallbackData;

     public:
                                AudioOutput(
                                        audio_session_t sessionId,
                                        uid_t uid,
                                        int pid,
                                        const audio_attributes_t * attr,
                                        const sp<AudioSystem::AudioDeviceCallback>& deviceCallback);
        virtual                 ~AudioOutput();

        virtual bool            ready() const { return mTrack != 0; }
        virtual ssize_t         bufferSize() const;
        virtual ssize_t         frameCount() const;
        virtual ssize_t         channelCount() const;
        virtual ssize_t         frameSize() const;
        virtual uint32_t        latency() const;
        virtual float           msecsPerFrame() const;
        virtual status_t        getPosition(uint32_t *position) const;
        virtual status_t        getTimestamp(AudioTimestamp &ts) const;
        virtual int64_t         getPlayedOutDurationUs(int64_t nowUs) const;
        virtual status_t        getFramesWritten(uint32_t *frameswritten) const;
        virtual audio_session_t getSessionId() const;
        virtual uint32_t        getSampleRate() const;
        virtual int64_t         getBufferDurationInUs() const;

        virtual status_t        open(
                uint32_t sampleRate, int channelCount, audio_channel_mask_t channelMask,
                audio_format_t format, int bufferCount,
                AudioCallback cb, void *cookie,
                audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                const audio_offload_info_t *offloadInfo = NULL,
                bool doNotReconnect = false,
                uint32_t suggestedFrameCount = 0);

        virtual status_t        start();
        virtual ssize_t         write(const void* buffer, size_t size, bool blocking = true);
        virtual void            stop();
        virtual void            flush();
        virtual void            pause();
        virtual void            close();
                void            setAudioStreamType(audio_stream_type_t streamType);
        virtual audio_stream_type_t getAudioStreamType() const { return mStreamType; }
                void            setAudioAttributes(const audio_attributes_t * attributes);

                void            setVolume(float left, float right);
        virtual status_t        setPlaybackRate(const AudioPlaybackRate& rate);
        virtual status_t        getPlaybackRate(AudioPlaybackRate* rate /* nonnull */);

                status_t        setAuxEffectSendLevel(float level);
                status_t        attachAuxEffect(int effectId);
        virtual status_t        dump(int fd, const Vector<String16>& args) const;

        static bool             isOnEmulator();
        static int              getMinBufferCount();
                void            setNextOutput(const sp<AudioOutput>& nextOutput);
                void            switchToNextOutput();
        virtual bool            needsTrailingPadding() { return mNextOutput == NULL; }
        virtual status_t        setParameters(const String8& keyValuePairs);
        virtual String8         getParameters(const String8& keys);

        virtual media::VolumeShaper::Status applyVolumeShaper(
                                        const sp<media::VolumeShaper::Configuration>& configuration,
                                        const sp<media::VolumeShaper::Operation>& operation) override;
        virtual sp<media::VolumeShaper::State> getVolumeShaperState(int id) override;

        // AudioRouting
        virtual status_t        setOutputDevice(audio_port_handle_t deviceId);
        virtual status_t        getRoutedDeviceId(audio_port_handle_t* deviceId);
        virtual status_t        enableAudioDeviceCallback(bool enabled);

    private:
        static void             setMinBufferCount();
        static void             CallbackWrapper(
                int event, void *me, void *info);
               void             deleteRecycledTrack_l();
               void             close_l();
           status_t             updateTrack();

        sp<AudioTrack>          mTrack;
        sp<AudioTrack>          mRecycledTrack;
        sp<AudioOutput>         mNextOutput;
        AudioCallback           mCallback;
        void *                  mCallbackCookie;
        CallbackData *          mCallbackData;
        audio_stream_type_t     mStreamType;
        audio_attributes_t *    mAttributes;
        float                   mLeftVolume;
        float                   mRightVolume;
        AudioPlaybackRate       mPlaybackRate;
        uint32_t                mSampleRateHz; // sample rate of the content, as set in open()
        float                   mMsecsPerFrame;
        size_t                  mFrameSize;
        audio_session_t         mSessionId;
        uid_t                   mUid;
        int                     mPid;
        float                   mSendLevel;
        int                     mAuxEffectId;
        audio_output_flags_t    mFlags;
        sp<media::VolumeHandler>       mVolumeHandler;
        audio_port_handle_t     mSelectedDeviceId;
        audio_port_handle_t     mRoutedDeviceId;
        bool                    mDeviceCallbackEnabled;
        wp<AudioSystem::AudioDeviceCallback>        mDeviceCallback;
        mutable Mutex           mLock;

        // static variables below not protected by mutex
        static bool             mIsOnEmulator;
        static int              mMinBufferCount;  // 12 for emulator; otherwise 4

        // CallbackData is what is passed to the AudioTrack as the "user" data.
        // We need to be able to target this to a different Output on the fly,
        // so we can't use the Output itself for this.
        class CallbackData {
            friend AudioOutput;
        public:
            explicit CallbackData(AudioOutput *cookie) {
                mData = cookie;
                mSwitching = false;
            }
            AudioOutput *   getOutput() const { return mData; }
            void            setOutput(AudioOutput* newcookie) { mData = newcookie; }
            // lock/unlock are used by the callback before accessing the payload of this object
            void            lock() const { mLock.lock(); }
            void            unlock() const { mLock.unlock(); }

            // tryBeginTrackSwitch/endTrackSwitch are used when the CallbackData is handed over
            // to the next sink.

            // tryBeginTrackSwitch() returns true only if it obtains the lock.
            bool            tryBeginTrackSwitch() {
                LOG_ALWAYS_FATAL_IF(mSwitching, "tryBeginTrackSwitch() already called");
                if (mLock.tryLock() != OK) {
                    return false;
                }
                mSwitching = true;
                return true;
            }
            void            endTrackSwitch() {
                if (mSwitching) {
                    mLock.unlock();
                }
                mSwitching = false;
            }
        private:
            AudioOutput *   mData;
            mutable Mutex   mLock; // a recursive mutex might make this unnecessary.
            bool            mSwitching;
            DISALLOW_EVIL_CONSTRUCTORS(CallbackData);
        };

    }; // AudioOutput

public:
    static  void                instantiate();

    // IMediaPlayerService interface
    virtual sp<IMediaRecorder>  createMediaRecorder(const String16 &opPackageName);
    void    removeMediaRecorderClient(const wp<MediaRecorderClient>& client);
    virtual sp<IMediaMetadataRetriever> createMetadataRetriever();

    virtual sp<IMediaPlayer>    create(const sp<IMediaPlayerClient>& client,
                                       audio_session_t audioSessionId);

    virtual sp<IMediaCodecList> getCodecList() const;

    virtual sp<IRemoteDisplay> listenForRemoteDisplay(const String16 &opPackageName,
            const sp<IRemoteDisplayClient>& client, const String8& iface);
    virtual status_t            dump(int fd, const Vector<String16>& args);

            void                removeClient(const wp<Client>& client);
            bool                hasClient(wp<Client> client);

    enum {
        MEDIASERVER_PROCESS_DEATH = 0,
        MEDIAEXTRACTOR_PROCESS_DEATH = 1,
        MEDIACODEC_PROCESS_DEATH = 2,
        AUDIO_PROCESS_DEATH = 3,   // currently no need to track this
        CAMERA_PROCESS_DEATH = 4
    };

    // Collect info of the codec usage from media player and media recorder
    virtual void                addBatteryData(uint32_t params);
    // API for the Battery app to pull the data of codecs usage
    virtual status_t            pullBatteryData(Parcel* reply);
private:
    struct BatteryTracker {
        BatteryTracker();
        // Collect info of the codec usage from media player and media recorder
        void addBatteryData(uint32_t params);
        // API for the Battery app to pull the data of codecs usage
        status_t pullBatteryData(Parcel* reply);

    private:
        // For battery usage tracking purpose
        struct BatteryUsageInfo {
            // how many streams are being played by one UID
            int     refCount;
            // a temp variable to store the duration(ms) of audio codecs
            // when we start a audio codec, we minus the system time from audioLastTime
            // when we pause it, we add the system time back to the audioLastTime
            // so after the pause, audioLastTime = pause time - start time
            // if multiple audio streams are played (or recorded), then audioLastTime
            // = the total playing time of all the streams
            int32_t audioLastTime;
            // when all the audio streams are being paused, we assign audioLastTime to
            // this variable, so this value could be provided to the battery app
            // in the next pullBatteryData call
            int32_t audioTotalTime;

            int32_t videoLastTime;
            int32_t videoTotalTime;
        };
        KeyedVector<int, BatteryUsageInfo>    mBatteryData;

        enum {
            SPEAKER,
            OTHER_AUDIO_DEVICE,
            SPEAKER_AND_OTHER,
            NUM_AUDIO_DEVICES
        };

        struct BatteryAudioFlingerUsageInfo {
            int refCount; // how many audio streams are being played
            int deviceOn[NUM_AUDIO_DEVICES]; // whether the device is currently used
            int32_t lastTime[NUM_AUDIO_DEVICES]; // in ms
            // totalTime[]: total time of audio output devices usage
            int32_t totalTime[NUM_AUDIO_DEVICES]; // in ms
        };

        // This varialble is used to record the usage of audio output device
        // for battery app
        BatteryAudioFlingerUsageInfo mBatteryAudio;

        mutable Mutex mLock;
    };
    BatteryTracker mBatteryTracker;

    class Client : public BnMediaPlayer {
        // IMediaPlayer interface
        virtual void            disconnect();
        virtual status_t        setVideoSurfaceTexture(
                                        const sp<IGraphicBufferProducer>& bufferProducer);
        virtual status_t        setBufferingSettings(const BufferingSettings& buffering) override;
        virtual status_t        getBufferingSettings(
                                        BufferingSettings* buffering /* nonnull */) override;
        virtual status_t        prepareAsync();
        virtual status_t        start();
        virtual status_t        stop();
        virtual status_t        pause();
        virtual status_t        isPlaying(bool* state);
        virtual status_t        setPlaybackSettings(const AudioPlaybackRate& rate);
        virtual status_t        getPlaybackSettings(AudioPlaybackRate* rate /* nonnull */);
        virtual status_t        setSyncSettings(const AVSyncSettings& rate, float videoFpsHint);
        virtual status_t        getSyncSettings(AVSyncSettings* rate /* nonnull */,
                                                float* videoFps /* nonnull */);
        virtual status_t        seekTo(
                int msec,
                MediaPlayerSeekMode mode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC);
        virtual status_t        getCurrentPosition(int* msec);
        virtual status_t        getDuration(int* msec);
        virtual status_t        reset();
        virtual status_t        notifyAt(int64_t mediaTimeUs);
        virtual status_t        setAudioStreamType(audio_stream_type_t type);
        virtual status_t        setLooping(int loop);
        virtual status_t        setVolume(float leftVolume, float rightVolume);
        virtual status_t        invoke(const Parcel& request, Parcel *reply);
        virtual status_t        setMetadataFilter(const Parcel& filter);
        virtual status_t        getMetadata(bool update_only,
                                            bool apply_filter,
                                            Parcel *reply);
        virtual status_t        setAuxEffectSendLevel(float level);
        virtual status_t        attachAuxEffect(int effectId);
        virtual status_t        setParameter(int key, const Parcel &request);
        virtual status_t        getParameter(int key, Parcel *reply);
        virtual status_t        setRetransmitEndpoint(const struct sockaddr_in* endpoint);
        virtual status_t        getRetransmitEndpoint(struct sockaddr_in* endpoint);
        virtual status_t        setNextPlayer(const sp<IMediaPlayer>& player);

        virtual media::VolumeShaper::Status applyVolumeShaper(
                                        const sp<media::VolumeShaper::Configuration>& configuration,
                                        const sp<media::VolumeShaper::Operation>& operation) override;
        virtual sp<media::VolumeShaper::State> getVolumeShaperState(int id) override;

        sp<MediaPlayerBase>     createPlayer(player_type playerType);

        virtual status_t        setDataSource(
                        const sp<IMediaHTTPService> &httpService,
                        const char *url,
                        const KeyedVector<String8, String8> *headers);

        virtual status_t        setDataSource(int fd, int64_t offset, int64_t length);

        virtual status_t        setDataSource(const sp<IStreamSource> &source);
        virtual status_t        setDataSource(const sp<IDataSource> &source);


        sp<MediaPlayerBase>     setDataSource_pre(player_type playerType);
        status_t                setDataSource_post(const sp<MediaPlayerBase>& p,
                                                   status_t status);

                void            notify(int msg, int ext1, int ext2, const Parcel *obj);

                pid_t           pid() const { return mPid; }
        virtual status_t        dump(int fd, const Vector<String16>& args);

                audio_session_t getAudioSessionId() { return mAudioSessionId; }
        // Modular DRM
        virtual status_t prepareDrm(const uint8_t uuid[16], const Vector<uint8_t>& drmSessionId);
        virtual status_t releaseDrm();
        // AudioRouting
        virtual status_t setOutputDevice(audio_port_handle_t deviceId);
        virtual status_t getRoutedDeviceId(audio_port_handle_t* deviceId);
        virtual status_t enableAudioDeviceCallback(bool enabled);

    private:
        class AudioDeviceUpdatedNotifier: public AudioSystem::AudioDeviceCallback
        {
        public:
            AudioDeviceUpdatedNotifier(const sp<MediaPlayerBase>& listener) {
                mListener = listener;
            }
            ~AudioDeviceUpdatedNotifier() {}

            virtual void onAudioDeviceUpdate(audio_io_handle_t audioIo,
                                             audio_port_handle_t deviceId);

        private:
            wp<MediaPlayerBase> mListener;
        };

        friend class MediaPlayerService;
                                Client( const sp<MediaPlayerService>& service,
                                        pid_t pid,
                                        int32_t connId,
                                        const sp<IMediaPlayerClient>& client,
                                        audio_session_t audioSessionId,
                                        uid_t uid);
                                Client();
        virtual                 ~Client();

                void            deletePlayer();

        sp<MediaPlayerBase>     getPlayer() const { Mutex::Autolock lock(mLock); return mPlayer; }



        // @param type Of the metadata to be tested.
        // @return true if the metadata should be dropped according to
        //              the filters.
        bool shouldDropMetadata(media::Metadata::Type type) const;

        // Add a new element to the set of metadata updated. Noop if
        // the element exists already.
        // @param type Of the metadata to be recorded.
        void addNewMetadataUpdate(media::Metadata::Type type);

        // Disconnect from the currently connected ANativeWindow.
        void disconnectNativeWindow_l();

        status_t setAudioAttributes_l(const Parcel &request);

        class Listener : public MediaPlayerBase::Listener {
        public:
            Listener(const wp<Client> &client) : mClient(client) {}
            virtual ~Listener() {}
            virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) {
                sp<Client> client = mClient.promote();
                if (client != NULL) {
                    client->notify(msg, ext1, ext2, obj);
                }
            }
        private:
            wp<Client> mClient;
        };

        mutable     Mutex                         mLock;
                    sp<MediaPlayerBase>           mPlayer;
                    sp<MediaPlayerService>        mService;
                    sp<IMediaPlayerClient>        mClient;
                    sp<AudioOutput>               mAudioOutput;
                    pid_t                         mPid;
                    status_t                      mStatus;
                    bool                          mLoop;
                    int32_t                       mConnId;
                    audio_session_t               mAudioSessionId;
                    audio_attributes_t *          mAudioAttributes;
                    uid_t                         mUid;
                    sp<ANativeWindow>             mConnectedWindow;
                    sp<IBinder>                   mConnectedWindowBinder;
                    struct sockaddr_in            mRetransmitEndpoint;
                    bool                          mRetransmitEndpointValid;
                    sp<Client>                    mNextClient;
                    sp<MediaPlayerBase::Listener> mListener;

        // Metadata filters.
        media::Metadata::Filter mMetadataAllow;  // protected by mLock
        media::Metadata::Filter mMetadataDrop;  // protected by mLock

        // Metadata updated. For each MEDIA_INFO_METADATA_UPDATE
        // notification we try to update mMetadataUpdated which is a
        // set: no duplicate.
        // getMetadata clears this set.
        media::Metadata::Filter mMetadataUpdated;  // protected by mLock

        std::vector<DeathNotifier> mDeathNotifiers;
        sp<AudioDeviceUpdatedNotifier> mAudioDeviceUpdatedListener;
#if CALLBACK_ANTAGONIZER
                    Antagonizer*                  mAntagonizer;
#endif
    }; // Client

// ----------------------------------------------------------------------------

                            MediaPlayerService();
    virtual                 ~MediaPlayerService();

    mutable     Mutex                       mLock;
                SortedVector< wp<Client> >  mClients;
                SortedVector< wp<MediaRecorderClient> > mMediaRecorderClients;
                int32_t                     mNextConnId;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_MEDIAPLAYERSERVICE_H
