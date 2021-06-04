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

#ifndef GENERIC_SOURCE2_H_

#define GENERIC_SOURCE2_H_

#include "NuPlayer2.h"
#include "NuPlayer2Source.h"

#include "ATSParser.h"

#include <media/stagefright/MediaBuffer.h>
#include <mediaplayer2/mediaplayer2.h>
#include <media/NdkMediaDataSource.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkWrapper.h>

namespace android {

class DecryptHandle;
struct AnotherPacketSource;
struct ARTSPController;
class DataSource;
class IDataSource;
class IMediaSource;
struct MediaSource;
class MediaBuffer;
struct MediaClock;

struct NuPlayer2::GenericSource2 : public NuPlayer2::Source,
                                   public MediaBufferObserver // Modular DRM
{
    GenericSource2(const sp<AMessage> &notify, uid_t uid,
                   const sp<MediaClock> &mediaClock);

    status_t setDataSource(
            const char *url,
            const KeyedVector<String8, String8> *headers);

    status_t setDataSource(int fd, int64_t offset, int64_t length);

    status_t setDataSource(const sp<DataSource>& dataSource);

    virtual status_t getBufferingSettings(
            BufferingSettings* buffering /* nonnull */) override;
    virtual status_t setBufferingSettings(const BufferingSettings& buffering) override;

    virtual void prepareAsync(int64_t startTimeUs);

    virtual void start();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    virtual void disconnect();

    virtual status_t feedMoreTSData();

    virtual sp<MetaData> getFileFormatMeta() const;

    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);

    virtual status_t getDuration(int64_t *durationUs);
    virtual size_t getTrackCount() const;
    virtual sp<AMessage> getTrackInfo(size_t trackIndex) const;
    virtual ssize_t getSelectedTrack(media_track_type type) const;
    virtual status_t selectTrack(size_t trackIndex, bool select, int64_t timeUs);
    virtual status_t seekTo(
        int64_t seekTimeUs,
        MediaPlayer2SeekMode mode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC) override;

    virtual bool isStreaming() const;

    // Modular DRM
    virtual void signalBufferReturned(MediaBufferBase *buffer);

    virtual status_t prepareDrm(
            const uint8_t uuid[16],
            const Vector<uint8_t> &drmSessionId,
            sp<AMediaCryptoWrapper> *outCrypto);

    virtual status_t releaseDrm();


protected:
    virtual ~GenericSource2();

    virtual void onMessageReceived(const sp<AMessage> &msg);

    virtual sp<AMessage> getFormat(bool audio);
    virtual sp<MetaData> getFormatMeta(bool audio);

private:
    enum {
        kWhatPrepareAsync,
        kWhatFetchSubtitleData,
        kWhatFetchTimedTextData,
        kWhatSendSubtitleData,
        kWhatSendGlobalTimedTextData,
        kWhatSendTimedTextData,
        kWhatChangeAVSource,
        kWhatPollBuffering,
        kWhatSeek,
        kWhatReadBuffer,
        kWhatStart,
        kWhatResume,
        kWhatSecureDecodersInstantiated,
    };

    struct Track {
        size_t mIndex;
        sp<AMediaExtractorWrapper> mExtractor;
        sp<AnotherPacketSource> mPackets;
    };

    int64_t mAudioTimeUs;
    int64_t mAudioLastDequeueTimeUs;
    int64_t mVideoTimeUs;
    int64_t mVideoLastDequeueTimeUs;

    BufferingSettings mBufferingSettings;
    int32_t mPrevBufferPercentage;
    int32_t mPollBufferingGeneration;
    bool mSentPauseOnBuffering;

    int32_t mAudioDataGeneration;
    int32_t mVideoDataGeneration;
    int32_t mFetchSubtitleDataGeneration;
    int32_t mFetchTimedTextDataGeneration;
    int64_t mDurationUs;
    bool mAudioIsVorbis;
    // Secure codec is required.
    bool mIsSecure;
    bool mIsStreaming;
    uid_t mUID;
    const sp<MediaClock> mMediaClock;
    AString mUri;
    KeyedVector<String8, String8> mUriHeaders;
    int mFd;
    int64_t mOffset;
    int64_t mLength;

    bool mDisconnected;
    sp<MetaData> mFileMeta;
    sp<AMediaDataSourceWrapper> mDataSourceWrapper;
    sp<AMediaExtractorWrapper> mExtractor;
    Vector<sp<AMediaExtractorWrapper> > mExtractors;
    bool mStarted;
    bool mPreparing;
    int64_t mBitrate;
    uint32_t mPendingReadBufferTypes;
    sp<ABuffer> mGlobalTimedText;

    Track mVideoTrack;
    Track mAudioTrack;
    Track mSubtitleTrack;
    Track mTimedTextTrack;

    mutable Mutex mLock;

    sp<ALooper> mLooper;

    void resetDataSource();

    status_t initFromDataSource();
    int64_t getLastReadPosition();

    void notifyPreparedAndCleanup(status_t err);
    void onSecureDecodersInstantiated(status_t err);
    void finishPrepareAsync();
    status_t startSources();

    void onSeek(const sp<AMessage>& msg);
    status_t doSeek(int64_t seekTimeUs, MediaPlayer2SeekMode mode);

    void onPrepareAsync(int64_t startTimeUs);

    void fetchTextData(
            uint32_t what, media_track_type type,
            int32_t curGen, const sp<AnotherPacketSource>& packets, const sp<AMessage>& msg);

    void sendGlobalTextData(
            uint32_t what,
            int32_t curGen, sp<AMessage> msg);

    void sendTextData(
            uint32_t what, media_track_type type,
            int32_t curGen, const sp<AnotherPacketSource>& packets, const sp<AMessage>& msg);

    sp<ABuffer> mediaBufferToABuffer(
            MediaBufferBase *mbuf,
            media_track_type trackType);

    void postReadBuffer(media_track_type trackType);
    void onReadBuffer(const sp<AMessage>& msg);
    // When |mode| is MediaPlayer2SeekMode::SEEK_CLOSEST, the buffer read shall
    // include an item indicating skipping rendering all buffers with timestamp
    // earlier than |seekTimeUs|.
    // For other modes, the buffer read will not include the item as above in order
    // to facilitate fast seek operation.
    void readBuffer(
            media_track_type trackType,
            int64_t seekTimeUs = -1ll,
            MediaPlayer2SeekMode mode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC,
            int64_t *actualTimeUs = NULL, bool formatChange = false);

    void queueDiscontinuityIfNeeded(
            bool seeking, bool formatChange, media_track_type trackType, Track *track);

    void schedulePollBuffering();
    void onPollBuffering();
    void notifyBufferingUpdate(int32_t percentage);

    sp<AMessage> getFormat_l(bool audio);
    sp<MetaData> getFormatMeta_l(bool audio);
    int32_t getDataGeneration(media_track_type type) const;

    // Modular DRM
    // The source is DRM protected and is prepared for DRM.
    bool mIsDrmProtected;
    // releaseDrm has been processed.
    bool mIsDrmReleased;
    Vector<String8> mMimes;

    status_t checkDrmInfo();

    DISALLOW_EVIL_CONSTRUCTORS(GenericSource2);
};

}  // namespace android

#endif  // GENERIC_SOURCE2_H_
