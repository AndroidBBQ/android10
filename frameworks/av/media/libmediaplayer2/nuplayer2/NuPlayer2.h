/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef NU_PLAYER2_H_

#define NU_PLAYER2_H_

#include <media/AudioResamplerPublic.h>
#include <media/stagefright/foundation/AHandler.h>

#include <mediaplayer2/MediaPlayer2Interface.h>
#include <mediaplayer2/JObjectHolder.h>

#include "mediaplayer2.pb.h"

using android::media::MediaPlayer2Proto::PlayerMessage;

namespace android {

struct ABuffer;
struct AMediaCryptoWrapper;
struct AMessage;
struct ANativeWindowWrapper;
struct AudioPlaybackRate;
struct AVSyncSettings;
struct DataSourceDesc;
struct MediaClock;
struct MediaHTTPService;
class MetaData;
struct NuPlayer2Driver;

struct NuPlayer2 : public AHandler {
    explicit NuPlayer2(pid_t pid, uid_t uid,
            const sp<MediaClock> &mediaClock, const sp<JObjectHolder> &context);

    void setDriver(const wp<NuPlayer2Driver> &driver);

    void setDataSourceAsync(const sp<DataSourceDesc> &dsd);
    void prepareNextDataSourceAsync(const sp<DataSourceDesc> &dsd);
    void playNextDataSource(int64_t srcId);

    status_t getBufferingSettings(BufferingSettings* buffering /* nonnull */);
    status_t setBufferingSettings(const BufferingSettings& buffering);

    void prepareAsync();

    void setVideoSurfaceTextureAsync(const sp<ANativeWindowWrapper> &nww);

    void setAudioSink(const sp<MediaPlayer2Interface::AudioSink> &sink);
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
            MediaPlayer2SeekMode mode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC,
            bool needNotify = false);
    void rewind();

    status_t setVideoScalingMode(int32_t mode);
    status_t getTrackInfo(int64_t srcId, PlayerMessage* reply) const;
    status_t getSelectedTrack(int64_t srcId, int32_t type, PlayerMessage* reply) const;
    status_t selectTrack(int64_t srcId, size_t trackIndex, bool select, int64_t timeUs);
    status_t getCurrentPosition(int64_t *mediaUs);
    void getStats(Vector<sp<AMessage> > *mTrackStats);

    sp<MetaData> getFileMeta();
    float getFrameRate();

    // Modular DRM
    status_t prepareDrm(int64_t srcId, const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId);
    status_t releaseDrm(int64_t srcId);

    const char *getDataSourceType();

protected:
    virtual ~NuPlayer2();

    virtual void onMessageReceived(const sp<AMessage> &msg);

public:
    struct StreamListener;
    struct Source;

private:
    struct Decoder;
    struct DecoderBase;
    struct DecoderPassThrough;
    struct CCDecoder;
    struct GenericSource2;
    struct HTTPLiveSource2;
    struct Renderer;
    struct RTSPSource2;
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
        kWhatPrepareNextDataSource      = 'pNDS',
        kWhatPlayNextDataSource         = 'plNS',
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
        kWhatRewind                     = 'reWd',
        kWhatEOSMonitor                 = 'eosM',
    };

    typedef enum {
        DATA_SOURCE_TYPE_NONE,
        DATA_SOURCE_TYPE_HTTP_LIVE,
        DATA_SOURCE_TYPE_RTSP,
        DATA_SOURCE_TYPE_GENERIC_URL,
        DATA_SOURCE_TYPE_GENERIC_FD,
        DATA_SOURCE_TYPE_MEDIA,
    } DATA_SOURCE_TYPE;

    struct SourceInfo {
        SourceInfo();
        SourceInfo &operator=(const SourceInfo &);

        sp<Source> mSource;
        std::atomic<DATA_SOURCE_TYPE> mDataSourceType;
        int64_t mSrcId;
        uint32_t mSourceFlags;
        int64_t mStartTimeUs;
        int64_t mEndTimeUs;
        // Modular DRM
        sp<AMediaCryptoWrapper> mCrypto;
        bool mIsDrmProtected = false;
    };

    wp<NuPlayer2Driver> mDriver;
    pid_t mPID;
    uid_t mUID;
    const sp<MediaClock> mMediaClock;
    Mutex mSourceLock;  // guard |mSource|.
    SourceInfo mCurrentSourceInfo;
    SourceInfo mNextSourceInfo;
    sp<ANativeWindowWrapper> mNativeWindow;
    sp<MediaPlayer2Interface::AudioSink> mAudioSink;
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
    int32_t mEOSMonitorGeneration;

    Mutex mPlayingTimeLock;
    int64_t mLastStartedPlayingTimeNs;
    void stopPlaybackTimer(const char *where);
    void startPlaybackTimer(const char *where);

    int64_t mLastStartedRebufferingTimeNs;
    void startRebufferingTimer();
    void stopRebufferingTimer(bool exitingPlayback);

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

    // Passed from JAVA
    const sp<JObjectHolder> mContext;

    inline const sp<DecoderBase> &getDecoder(bool audio) {
        return audio ? mAudioDecoder : mVideoDecoder;
    }

    inline void clearFlushComplete() {
        mFlushComplete[0][0] = false;
        mFlushComplete[0][1] = false;
        mFlushComplete[1][0] = false;
        mFlushComplete[1][1] = false;
    }

    void disconnectSource();

    status_t createNuPlayer2Source(const sp<DataSourceDesc> &dsd,
                                   sp<Source> *source,
                                   DATA_SOURCE_TYPE *dataSourceType);

    void tryOpenAudioSinkForOffload(
            const sp<AMessage> &format, const sp<MetaData> &audioMeta, bool hasVideo);
    void closeAudioSink();
    void restartAudio(
            int64_t currentPositionUs, bool forceNonOffload, bool needsToCreateAudioDecoder);
    void determineAudioModeChange(const sp<AMessage> &audioFormat);

    status_t instantiateDecoder(
            bool audio, sp<DecoderBase> *decoder, bool checkAudioModeChange = true);

    void updateVideoSize(
            int64_t srcId,
            const sp<AMessage> &inputFormat,
            const sp<AMessage> &outputFormat = NULL);

    void notifyListener(int64_t srcId, int msg, int ext1, int ext2, const PlayerMessage *in = NULL);

    void addEndTimeMonitor();

    void handleFlushComplete(bool audio, bool isDecoder);
    void finishFlushIfPossible();

    void onStart(bool play);
    void onResume();
    void onPause();

    bool audioDecoderStillNeeded();

    void flushDecoder(bool audio, bool needShutdown);

    void finishResume();
    void notifyDriverSeekComplete(int64_t srcId);

    void postScanSources();

    void schedulePollDuration();
    void cancelPollDuration();

    void processDeferredActions();

    void performSeek(int64_t seekTimeUs, MediaPlayer2SeekMode mode);
    void performDecoderFlush(FlushCommand audio, FlushCommand video);
    void performReset();
    void performPlayNextDataSource();
    void performScanSources();
    void performSetSurface(const sp<ANativeWindowWrapper> &nw);
    void performResumeDecoders(bool needNotify);

    void onSourceNotify(const sp<AMessage> &msg);
    void onClosedCaptionNotify(const sp<AMessage> &msg);

    void queueDecoderShutdown(
            bool audio, bool video, const sp<AMessage> &reply);

    void sendSubtitleData(const sp<ABuffer> &buffer, int32_t baseIndex);
    void sendTimedMetaData(const sp<ABuffer> &buffer);
    void sendTimedTextData(const sp<ABuffer> &buffer);

    void writeTrackInfo(PlayerMessage* reply, const sp<AMessage>& format) const;

    status_t onPrepareDrm(const sp<AMessage> &msg);
    status_t onReleaseDrm(const sp<AMessage> &msg);

    SourceInfo* getSourceInfoByIdInMsg(const sp<AMessage> &msg);
    void resetSourceInfo(SourceInfo &srcInfo);

    DISALLOW_EVIL_CONSTRUCTORS(NuPlayer2);
};

}  // namespace android

#endif  // NU_PLAYER2_H_
