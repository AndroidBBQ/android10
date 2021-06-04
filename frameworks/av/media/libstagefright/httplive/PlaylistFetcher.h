/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef PLAYLIST_FETCHER_H_

#define PLAYLIST_FETCHER_H_

#include <media/stagefright/foundation/AHandler.h>
#include <openssl/aes.h>

#include "mpeg2ts/ATSParser.h"
#include "LiveSession.h"

namespace android {

struct ABuffer;
struct AnotherPacketSource;
class DataSource;
struct HTTPBase;
struct LiveDataSource;
struct M3UParser;
class String8;

struct PlaylistFetcher : public AHandler {
    static const int64_t kMinBufferedDurationUs;
    static const int32_t kDownloadBlockSize;
    static const int64_t kFetcherResumeThreshold;

    enum {
        kWhatStarted,
        kWhatPaused,
        kWhatStopped,
        kWhatError,
        kWhatDurationUpdate,
        kWhatTargetDurationUpdate,
        kWhatPrepared,
        kWhatPreparationFailed,
        kWhatStartedAt,
        kWhatStopReached,
        kWhatPlaylistFetched,
        kWhatMetadataDetected,
    };

    PlaylistFetcher(
            const sp<AMessage> &notify,
            const sp<LiveSession> &session,
            const char *uri,
            int32_t id,
            int32_t subtitleGeneration);

    int32_t getFetcherID() const;

    void startAsync(
            const sp<AnotherPacketSource> &audioSource,
            const sp<AnotherPacketSource> &videoSource,
            const sp<AnotherPacketSource> &subtitleSource,
            const sp<AnotherPacketSource> &metadataSource,
            int64_t startTimeUs = -1ll,         // starting timestamps
            int64_t segmentStartTimeUs = -1ll, // starting position within playlist
            // startTimeUs!=segmentStartTimeUs only when playlist is live
            int32_t startDiscontinuitySeq = -1,
            LiveSession::SeekMode seekMode = LiveSession::kSeekModeExactPosition);

    void pauseAsync(float thresholdRatio, bool disconnect);

    void stopAsync(bool clear = true);

    void resumeUntilAsync(const sp<AMessage> &params);

    void fetchPlaylistAsync();

    uint32_t getStreamTypeMask() const {
        return mStreamTypeMask;
    }

protected:
    virtual ~PlaylistFetcher();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kMaxNumRetries         = 5,
    };

    enum {
        kWhatStart          = 'strt',
        kWhatPause          = 'paus',
        kWhatStop           = 'stop',
        kWhatMonitorQueue   = 'moni',
        kWhatResumeUntil    = 'rsme',
        kWhatDownloadNext   = 'dlnx',
        kWhatFetchPlaylist  = 'flst'
    };

    struct DownloadState;

    static const int64_t kMaxMonitorDelayUs;
    static const int32_t kNumSkipFrames;

    static bool bufferStartsWithTsSyncByte(const sp<ABuffer>& buffer);
    static bool bufferStartsWithWebVTTMagicSequence(const sp<ABuffer>& buffer);

    // notifications to mSession
    sp<AMessage> mNotify;
    sp<AMessage> mStartTimeUsNotify;

    sp<HTTPDownloader> mHTTPDownloader;
    sp<LiveSession> mSession;
    AString mURI;

    int32_t mFetcherID;

    uint32_t mStreamTypeMask;
    int64_t mStartTimeUs;

    // Start time relative to the beginning of the first segment in the initial
    // playlist. It's value is initialized to a non-negative value only when we are
    // adapting or switching tracks.
    int64_t mSegmentStartTimeUs;

    int32_t mDiscontinuitySeq;
    bool mStartTimeUsRelative;
    sp<AMessage> mStopParams; // message containing the latest timestamps we should fetch.

    KeyedVector<LiveSession::StreamType, sp<AnotherPacketSource> >
        mPacketSources;

    KeyedVector<AString, sp<ABuffer> > mAESKeyForURI;

    int64_t mLastPlaylistFetchTimeUs;
    int64_t mPlaylistTimeUs;
    sp<M3UParser> mPlaylist;
    int32_t mSeqNumber;
    int32_t mNumRetries;
    int32_t mNumRetriesForMonitorQueue;
    bool mStartup;
    bool mIDRFound;
    int32_t mSeekMode;
    bool mTimeChangeSignaled;
    int64_t mNextPTSTimeUs;

    int32_t mMonitorQueueGeneration;
    const int32_t mSubtitleGeneration;

    int32_t mLastDiscontinuitySeq;

    enum RefreshState {
        INITIAL_MINIMUM_RELOAD_DELAY,
        FIRST_UNCHANGED_RELOAD_ATTEMPT,
        SECOND_UNCHANGED_RELOAD_ATTEMPT,
        THIRD_UNCHANGED_RELOAD_ATTEMPT
    };
    RefreshState mRefreshState;

    uint8_t mPlaylistHash[16];

    sp<ATSParser> mTSParser;

    bool mFirstPTSValid;
    int64_t mFirstTimeUs;
    int64_t mSegmentFirstPTS;
    sp<AnotherPacketSource> mVideoBuffer;

    // Stores the initialization vector to decrypt the next block of cipher text, which can
    // either be derived from the sequence number, read from the manifest, or copied from
    // the last block of cipher text (cipher-block chaining).
    unsigned char mAESInitVec[AES_BLOCK_SIZE];
    unsigned char mKeyData[AES_BLOCK_SIZE];
    bool mSampleAesKeyItemChanged;
    sp<AMessage> mSampleAesKeyItem;

    Mutex mThresholdLock;
    float mThresholdRatio;

    sp<DownloadState> mDownloadState;

    bool mHasMetadata;

    // Set first to true if decrypting the first segment of a playlist segment. When
    // first is true, reset the initialization vector based on the available
    // information in the manifest; otherwise, use the initialization vector as
    // updated by the last call to AES_cbc_encrypt.
    //
    // For the input to decrypt correctly, decryptBuffer must be called on
    // consecutive byte ranges on block boundaries, e.g. 0..15, 16..47, 48..63,
    // and so on.
    status_t decryptBuffer(
            size_t playlistIndex, const sp<ABuffer> &buffer,
            bool first = true);
    status_t checkDecryptPadding(const sp<ABuffer> &buffer);

    void postMonitorQueue(int64_t delayUs = 0, int64_t minDelayUs = 0);
    void cancelMonitorQueue();
    void setStoppingThreshold(float thresholdRatio, bool disconnect);
    void resetStoppingThreshold(bool disconnect);
    float getStoppingThreshold();
    bool shouldPauseDownload();

    int64_t delayUsToRefreshPlaylist() const;
    status_t refreshPlaylist();

    // Returns the media time in us of the segment specified by seqNumber.
    // This is computed by summing the durations of all segments before it.
    int64_t getSegmentStartTimeUs(int32_t seqNumber) const;
    // Returns the duration time in us of the segment specified.
    int64_t getSegmentDurationUs(int32_t seqNumber) const;

    status_t onStart(const sp<AMessage> &msg);
    void onPause();
    void onStop(const sp<AMessage> &msg);
    void onMonitorQueue();
    void onDownloadNext();
    void initSeqNumberForLiveStream(
            int32_t &firstSeqNumberInPlaylist,
            int32_t &lastSeqNumberInPlaylist);
    bool initDownloadState(
            AString &uri,
            sp<AMessage> &itemMeta,
            int32_t &firstSeqNumberInPlaylist,
            int32_t &lastSeqNumberInPlaylist);

    // Resume a fetcher to continue until the stopping point stored in msg.
    status_t onResumeUntil(const sp<AMessage> &msg);

    const sp<ABuffer> &setAccessUnitProperties(
            const sp<ABuffer> &accessUnit,
            const sp<AnotherPacketSource> &source,
            bool discard = false);
    bool isStartTimeReached(int64_t timeUs);
    status_t extractAndQueueAccessUnitsFromTs(const sp<ABuffer> &buffer);

    status_t extractAndQueueAccessUnits(
            const sp<ABuffer> &buffer, const sp<AMessage> &itemMeta);

    void notifyStopReached();
    void notifyError(status_t err);

    void queueDiscontinuity(
            ATSParser::DiscontinuityType type, const sp<AMessage> &extra);

    bool adjustSeqNumberWithAnchorTime(int64_t anchorTimeUs);
    int32_t getSeqNumberForDiscontinuity(size_t discontinuitySeq) const;
    int32_t getSeqNumberForTime(int64_t timeUs) const;

    void updateDuration();
    void updateTargetDuration();

    DISALLOW_EVIL_CONSTRUCTORS(PlaylistFetcher);
};

}  // namespace android

#endif  // PLAYLIST_FETCHER_H_

