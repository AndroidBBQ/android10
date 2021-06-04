/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef NU_PLAYER_H_

#define NU_PLAYER_H_

#include <media/AudioResamplerPublic.h>
#include <media/ICrypto.h>
#include <media/MediaPlayerInterface.h>
#include <media/stagefright/foundation/AHandler.h>

namespace android {

struct ABuffer;
struct AMessage;
struct AudioPlaybackRate;
struct AVSyncSettings;
class IDataSource;
struct MediaClock;
class MetaData;
struct NuPlayerDriver;

struct NuPlayer : public AHandler {
    explicit NuPlayer(pid_t pid, const sp<MediaClock> &mediaClock);

    void setUID(uid_t uid);

    void init(const wp<NuPlayerDriver> &driver);

    void setDataSourceAsync(const sp<IStreamSource> &source);

    void setDataSourceAsync(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    void setDataSourceAsync(int fd, int64_t offset, int64_t length);

    void setDataSourceAsync(const sp<DataSource> &source);

    status_t getBufferingSettings(BufferingSettings* buffering /* nonnull */);
    status_t setBufferingSettings(const BufferingSettings& buffering);

    void prepareAsync();

    void setVideoSurfaceTextureAsync(
            const sp<IGraphicBufferProducer> &bufferProducer);

    void setAudioSink(const sp<MediaPlayerBase::AudioSink> &sink);
    status_t setPlaybackSettings(const AudioPlaybackRate &rate);
    status_t getPlaybackSettings(AudioPlaybackRate *rate /* nonnull */);
    status_t setSyncSettings(const AVSyncSettings &sync, float videoFpsHint);
    status_t getSyncSettings(AVSyncSettings *sync /* nonnull */, float *videoFps /* nonnull */);

    void start();

    void pause();

    // Will notify the driver through "notifyResetComplete" once finished.
    void resetAsync();

    // Request a notification when specified media time is reached.
    status_t notifyAt(int64_t mediaTimeUs);

    // Will notify the driver through "notifySeekComplete" once finished
    // and needNotify is true.
    void seekToAsync(
            int64_t seekTimeUs,
            MediaPlayerSeekMode mode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC,
            bool needNotify = false);

    status_t setVideoScalingMode(int32_t mode);
    status_t getTrackInfo(Parcel* reply) const;
    status_t getSelectedTrack(int32_t type, Parcel* reply) const;
    status_t selectTrack(size_t trackIndex, bool select, int64_t timeUs);
    status_t getCurrentPosition(int64_t *mediaUs);
    void getStats(Vector<sp<AMessage> > *trackStats);

    sp<MetaData> getFileMeta();
    float getFrameRate();

    // Modular DRM
    status_t prepareDrm(const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId);
    status_t releaseDrm();

    const char *getDataSourceType();

    void updateInternalTimers();

protected:
    virtual ~NuPlayer();

    virtual void onMessageReceived(const sp<AMessage> &msg);

public:
    struct NuPlayerStreamListener;
    struct Source;

private:
    struct Decoder;
    struct DecoderBase;
    struct DecoderPassThrough;
    struct CCDecoder;
    struct GenericSource;
    struct HTTPLiveSource;
    struct Renderer;
    struct RTSPSource;
    struct StreamingSource;
    struct Action;
    struct SeekAction;
    struct SetSurfaceAction;
    struct ResumeDecoderAction;
    struct FlushDecoderAction;
    struct PostMessageAction;
    struct SimpleAction;

    enum {
        kWhatSetDataSource              = '=DaS',
        kWhatPrepare                    = 'prep',
        kWhatSetVideoSurface            = '=VSu',
        kWhatSetAudioSink               = '=AuS',
        kWhatMoreDataQueued             = 'more',
        kWhatConfigPlayback             = 'cfPB',
        kWhatConfigSync                 = 'cfSy',
        kWhatGetPlaybackSettings        = 'gPbS',
        kWhatGetSyncSettings            = 'gSyS',
        kWhatStart                      = 'strt',
        kWhatScanSources                = 'scan',
        kWhatVideoNotify                = 'vidN',
        kWhatAudioNotify                = 'audN',
        kWhatClosedCaptionNotify        = 'capN',
        kWhatRendererNotify             = 'renN',
        kWhatReset                      = 'rset',
        kWhatNotifyTime                 = 'nfyT',
        kWhatSeek                       = 'seek',
        kWhatPause                      = 'paus',
        kWhatResume                     = 'rsme',
        kWhatPollDuration               = 'polD',
        kWhatSourceNotify               = 'srcN',
        kWhatGetTrackInfo               = 'gTrI',
        kWhatGetSelectedTrack           = 'gSel',
        kWhatSelectTrack                = 'selT',
        kWhatGetBufferingSettings       = 'gBus',
        kWhatSetBufferingSettings       = 'sBuS',
        kWhatPrepareDrm                 = 'pDrm',
        kWhatReleaseDrm                 = 'rDrm',
        kWhatMediaClockNotify           = 'mckN',
    };

    wp<NuPlayerDriver> mDriver;
    bool mUIDValid;
    uid_t mUID;
    pid_t mPID;
    const sp<MediaClock> mMediaClock;
    Mutex mSourceLock;  // guard |mSource|.
    sp<Source> mSource;
    uint32_t mSourceFlags;
    sp<Surface> mSurface;
    sp<MediaPlayerBase::AudioSink> mAudioSink;
    sp<DecoderBase> mVideoDecoder;
    bool mOffloadAudio;
    sp<DecoderBase> mAudioDecoder;
    Mutex mDecoderLock;  // guard |mAudioDecoder| and |mVideoDecoder|.
    sp<CCDecoder> mCCDecoder;
    sp<Renderer> mRenderer;
    sp<ALooper> mRendererLooper;
    int32_t mAudioDecoderGeneration;
    int32_t mVideoDecoderGeneration;
    int32_t mRendererGeneration;

    Mutex mPlayingTimeLock;
    int64_t mLastStartedPlayingTimeNs;
    void updatePlaybackTimer(bool stopping, const char *where);
    void startPlaybackTimer(const char *where);

    int64_t mLastStartedRebufferingTimeNs;
    void startRebufferingTimer();
    void updateRebufferingTimer(bool stopping, bool exitingPlayback);

    int64_t mPreviousSeekTimeUs;

    List<sp<Action> > mDeferredActions;

    bool mAudioEOS;
    bool mVideoEOS;

    bool mScanSourcesPending;
    int32_t mScanSourcesGeneration;

    int32_t mPollDurationGeneration;
    int32_t mTimedTextGeneration;

    enum FlushStatus {
        NONE,
        FLUSHING_DECODER,
        FLUSHING_DECODER_SHUTDOWN,
        SHUTTING_DOWN_DECODER,
        FLUSHED,
        SHUT_DOWN,
    };

    enum FlushCommand {
        FLUSH_CMD_NONE,
        FLUSH_CMD_FLUSH,
        FLUSH_CMD_SHUTDOWN,
    };

    // Status of flush responses from the decoder and renderer.
    bool mFlushComplete[2][2];

    FlushStatus mFlushingAudio;
    FlushStatus mFlushingVideo;

    // Status of flush responses from the decoder and renderer.
    bool mResumePending;

    int32_t mVideoScalingMode;

    AudioPlaybackRate mPlaybackSettings;
    AVSyncSettings mSyncSettings;
    float mVideoFpsHint;
    bool mStarted;
    bool mPrepared;
    bool mResetting;
    bool mSourceStarted;
    bool mAudioDecoderError;
    bool mVideoDecoderError;

    // Actual pause state, either as requested by client or due to buffering.
    bool mPaused;

    // Pause state as requested by client. Note that if mPausedByClient is
    // true, mPaused is always true; if mPausedByClient is false, mPaused could
    // still become true, when we pause internally due to buffering.
    bool mPausedByClient;

    // Pause state as requested by source (internally) due to buffering
    bool mPausedForBuffering;

    // Modular DRM
    sp<ICrypto> mCrypto;
    bool mIsDrmProtected;

    typedef enum {
        DATA_SOURCE_TYPE_NONE,
        DATA_SOURCE_TYPE_HTTP_LIVE,
        DATA_SOURCE_TYPE_RTSP,
        DATA_SOURCE_TYPE_GENERIC_URL,
        DATA_SOURCE_TYPE_GENERIC_FD,
        DATA_SOURCE_TYPE_MEDIA,
        DATA_SOURCE_TYPE_STREAM,
    } DATA_SOURCE_TYPE;

    std::atomic<DATA_SOURCE_TYPE> mDataSourceType;

    inline const sp<DecoderBase> &getDecoder(bool audio) {
        return audio ? mAudioDecoder : mVideoDecoder;
    }

    inline void clearFlushComplete() {
        mFlushComplete[0][0] = false;
        mFlushComplete[0][1] = false;
        mFlushComplete[1][0] = false;
        mFlushComplete[1][1] = false;
    }

    void tryOpenAudioSinkForOffload(
            const sp<AMessage> &format, const sp<MetaData> &audioMeta, bool hasVideo);
    void closeAudioSink();
    void restartAudio(
            int64_t currentPositionUs, bool forceNonOffload, bool needsToCreateAudioDecoder);
    void determineAudioModeChange(const sp<AMessage> &audioFormat);

    status_t instantiateDecoder(
            bool audio, sp<DecoderBase> *decoder, bool checkAudioModeChange = true);

    status_t onInstantiateSecureDecoders();

    void updateVideoSize(
            const sp<AMessage> &inputFormat,
            const sp<AMessage> &outputFormat = NULL);

    void notifyListener(int msg, int ext1, int ext2, const Parcel *in = NULL);

    void handleFlushComplete(bool audio, bool isDecoder);
    void finishFlushIfPossible();

    void onStart(
            int64_t startPositionUs = -1,
            MediaPlayerSeekMode mode = MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC);
    void onResume();
    void onPause();

    bool audioDecoderStillNeeded();

    void flushDecoder(bool audio, bool needShutdown);

    void finishResume();
    void notifyDriverSeekComplete();

    void postScanSources();

    void schedulePollDuration();
    void cancelPollDuration();

    void processDeferredActions();

    void performSeek(int64_t seekTimeUs, MediaPlayerSeekMode mode);
    void performDecoderFlush(FlushCommand audio, FlushCommand video);
    void performReset();
    void performScanSources();
    void performSetSurface(const sp<Surface> &wrapper);
    void performResumeDecoders(bool needNotify);

    void onSourceNotify(const sp<AMessage> &msg);
    void onClosedCaptionNotify(const sp<AMessage> &msg);

    void queueDecoderShutdown(
            bool audio, bool video, const sp<AMessage> &reply);

    void sendSubtitleData(const sp<ABuffer> &buffer, int32_t baseIndex);
    void sendTimedMetaData(const sp<ABuffer> &buffer);
    void sendTimedTextData(const sp<ABuffer> &buffer);

    void writeTrackInfo(Parcel* reply, const sp<AMessage>& format) const;

    status_t onPrepareDrm(const sp<AMessage> &msg);
    status_t onReleaseDrm();

    DISALLOW_EVIL_CONSTRUCTORS(NuPlayer);
};

}  // namespace android

#endif  // NU_PLAYER_H_
