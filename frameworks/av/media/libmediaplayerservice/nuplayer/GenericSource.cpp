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

//#define LOG_NDEBUG 0
#define LOG_TAG "GenericSource"

#include "GenericSource.h"
#include "NuPlayerDrm.h"

#include "AnotherPacketSource.h"
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <media/DataSource.h>
#include <media/MediaBufferHolder.h>
#include <media/MediaSource.h>
#include <media/IMediaExtractorService.h>
#include <media/IMediaHTTPService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/InterfaceUtils.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include "../../libstagefright/include/NuCachedSource2.h"
#include "../../libstagefright/include/HTTPBase.h"

namespace android {

static const int kInitialMarkMs        = 5000;  // 5secs

//static const int kPausePlaybackMarkMs  = 2000;  // 2secs
static const int kResumePlaybackMarkMs = 15000;  // 15secs

NuPlayer::GenericSource::GenericSource(
        const sp<AMessage> &notify,
        bool uidValid,
        uid_t uid,
        const sp<MediaClock> &mediaClock)
    : Source(notify),
      mAudioTimeUs(0),
      mAudioLastDequeueTimeUs(0),
      mVideoTimeUs(0),
      mVideoLastDequeueTimeUs(0),
      mPrevBufferPercentage(-1),
      mPollBufferingGeneration(0),
      mSentPauseOnBuffering(false),
      mAudioDataGeneration(0),
      mVideoDataGeneration(0),
      mFetchSubtitleDataGeneration(0),
      mFetchTimedTextDataGeneration(0),
      mDurationUs(-1LL),
      mAudioIsVorbis(false),
      mIsSecure(false),
      mIsStreaming(false),
      mUIDValid(uidValid),
      mUID(uid),
      mMediaClock(mediaClock),
      mFd(-1),
      mBitrate(-1LL),
      mPendingReadBufferTypes(0) {
    ALOGV("GenericSource");
    CHECK(mediaClock != NULL);

    mBufferingSettings.mInitialMarkMs = kInitialMarkMs;
    mBufferingSettings.mResumePlaybackMarkMs = kResumePlaybackMarkMs;
    resetDataSource();
}

void NuPlayer::GenericSource::resetDataSource() {
    ALOGV("resetDataSource");

    mHTTPService.clear();
    {
        Mutex::Autolock _l_d(mDisconnectLock);
        mHttpSource.clear();
        mDisconnected = false;
    }
    mUri.clear();
    mUriHeaders.clear();
    mSources.clear();
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
    mOffset = 0;
    mLength = 0;
    mStarted = false;
    mPreparing = false;

    mIsDrmProtected = false;
    mIsDrmReleased = false;
    mIsSecure = false;
    mMimes.clear();
}

status_t NuPlayer::GenericSource::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    Mutex::Autolock _l(mLock);
    ALOGV("setDataSource url: %s", url);

    resetDataSource();

    mHTTPService = httpService;
    mUri = url;

    if (headers) {
        mUriHeaders = *headers;
    }

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    return OK;
}

status_t NuPlayer::GenericSource::setDataSource(
        int fd, int64_t offset, int64_t length) {
    Mutex::Autolock _l(mLock);
    ALOGV("setDataSource %d/%lld/%lld", fd, (long long)offset, (long long)length);

    resetDataSource();

    mFd = dup(fd);
    mOffset = offset;
    mLength = length;

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    return OK;
}

status_t NuPlayer::GenericSource::setDataSource(const sp<DataSource>& source) {
    Mutex::Autolock _l(mLock);
    ALOGV("setDataSource (source: %p)", source.get());

    resetDataSource();
    {
        Mutex::Autolock _l_d(mDisconnectLock);
        mDataSource = source;
    }
    return OK;
}

sp<MetaData> NuPlayer::GenericSource::getFileFormatMeta() const {
    Mutex::Autolock _l(mLock);
    return mFileMeta;
}

status_t NuPlayer::GenericSource::initFromDataSource() {
    sp<IMediaExtractor> extractor;
    sp<DataSource> dataSource;
    {
        Mutex::Autolock _l_d(mDisconnectLock);
        dataSource = mDataSource;
    }
    CHECK(dataSource != NULL);

    mLock.unlock();
    // This might take long time if data source is not reliable.
    extractor = MediaExtractorFactory::Create(dataSource, NULL);

    if (extractor == NULL) {
        ALOGE("initFromDataSource, cannot create extractor!");
        mLock.lock();
        return UNKNOWN_ERROR;
    }

    sp<MetaData> fileMeta = extractor->getMetaData();

    size_t numtracks = extractor->countTracks();
    if (numtracks == 0) {
        ALOGE("initFromDataSource, source has no track!");
        mLock.lock();
        return UNKNOWN_ERROR;
    }

    mLock.lock();
    mFileMeta = fileMeta;
    if (mFileMeta != NULL) {
        int64_t duration;
        if (mFileMeta->findInt64(kKeyDuration, &duration)) {
            mDurationUs = duration;
        }
    }

    int32_t totalBitrate = 0;

    mMimes.clear();

    for (size_t i = 0; i < numtracks; ++i) {
        sp<IMediaSource> track = extractor->getTrack(i);
        if (track == NULL) {
            continue;
        }

        sp<MetaData> meta = extractor->getTrackMetaData(i);
        if (meta == NULL) {
            ALOGE("no metadata for track %zu", i);
            return UNKNOWN_ERROR;
        }

        const char *mime;
        CHECK(meta->findCString(kKeyMIMEType, &mime));

        ALOGV("initFromDataSource track[%zu]: %s", i, mime);

        // Do the string compare immediately with "mime",
        // we can't assume "mime" would stay valid after another
        // extractor operation, some extractors might modify meta
        // during getTrack() and make it invalid.
        if (!strncasecmp(mime, "audio/", 6)) {
            if (mAudioTrack.mSource == NULL) {
                mAudioTrack.mIndex = i;
                mAudioTrack.mSource = track;
                mAudioTrack.mPackets =
                    new AnotherPacketSource(mAudioTrack.mSource->getFormat());

                if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
                    mAudioIsVorbis = true;
                } else {
                    mAudioIsVorbis = false;
                }

                mMimes.add(String8(mime));
            }
        } else if (!strncasecmp(mime, "video/", 6)) {
            if (mVideoTrack.mSource == NULL) {
                mVideoTrack.mIndex = i;
                mVideoTrack.mSource = track;
                mVideoTrack.mPackets =
                    new AnotherPacketSource(mVideoTrack.mSource->getFormat());

                // video always at the beginning
                mMimes.insertAt(String8(mime), 0);
            }
        }

        mSources.push(track);
        int64_t durationUs;
        if (meta->findInt64(kKeyDuration, &durationUs)) {
            if (durationUs > mDurationUs) {
                mDurationUs = durationUs;
            }
        }

        int32_t bitrate;
        if (totalBitrate >= 0 && meta->findInt32(kKeyBitRate, &bitrate)) {
            totalBitrate += bitrate;
        } else {
            totalBitrate = -1;
        }
    }

    ALOGV("initFromDataSource mSources.size(): %zu  mIsSecure: %d  mime[0]: %s", mSources.size(),
            mIsSecure, (mMimes.isEmpty() ? "NONE" : mMimes[0].string()));

    if (mSources.size() == 0) {
        ALOGE("b/23705695");
        return UNKNOWN_ERROR;
    }

    // Modular DRM: The return value doesn't affect source initialization.
    (void)checkDrmInfo();

    mBitrate = totalBitrate;

    return OK;
}

status_t NuPlayer::GenericSource::getBufferingSettings(
        BufferingSettings* buffering /* nonnull */) {
    {
        Mutex::Autolock _l(mLock);
        *buffering = mBufferingSettings;
    }

    ALOGV("getBufferingSettings{%s}", buffering->toString().string());
    return OK;
}

status_t NuPlayer::GenericSource::setBufferingSettings(const BufferingSettings& buffering) {
    ALOGV("setBufferingSettings{%s}", buffering.toString().string());

    Mutex::Autolock _l(mLock);
    mBufferingSettings = buffering;
    return OK;
}

status_t NuPlayer::GenericSource::startSources() {
    // Start the selected A/V tracks now before we start buffering.
    // Widevine sources might re-initialize crypto when starting, if we delay
    // this to start(), all data buffered during prepare would be wasted.
    // (We don't actually start reading until start().)
    //
    // TODO: this logic may no longer be relevant after the removal of widevine
    // support
    if (mAudioTrack.mSource != NULL && mAudioTrack.mSource->start() != OK) {
        ALOGE("failed to start audio track!");
        return UNKNOWN_ERROR;
    }

    if (mVideoTrack.mSource != NULL && mVideoTrack.mSource->start() != OK) {
        ALOGE("failed to start video track!");
        return UNKNOWN_ERROR;
    }

    return OK;
}

int64_t NuPlayer::GenericSource::getLastReadPosition() {
    if (mAudioTrack.mSource != NULL) {
        return mAudioTimeUs;
    } else if (mVideoTrack.mSource != NULL) {
        return mVideoTimeUs;
    } else {
        return 0;
    }
}

bool NuPlayer::GenericSource::isStreaming() const {
    Mutex::Autolock _l(mLock);
    return mIsStreaming;
}

NuPlayer::GenericSource::~GenericSource() {
    ALOGV("~GenericSource");
    if (mLooper != NULL) {
        mLooper->unregisterHandler(id());
        mLooper->stop();
    }
    resetDataSource();
}

void NuPlayer::GenericSource::prepareAsync() {
    Mutex::Autolock _l(mLock);
    ALOGV("prepareAsync: (looper: %d)", (mLooper != NULL));

    if (mLooper == NULL) {
        mLooper = new ALooper;
        mLooper->setName("generic");
        mLooper->start();

        mLooper->registerHandler(this);
    }

    sp<AMessage> msg = new AMessage(kWhatPrepareAsync, this);
    msg->post();
}

void NuPlayer::GenericSource::onPrepareAsync() {
    mDisconnectLock.lock();
    ALOGV("onPrepareAsync: mDataSource: %d", (mDataSource != NULL));

    // delayed data source creation
    if (mDataSource == NULL) {
        // set to false first, if the extractor
        // comes back as secure, set it to true then.
        mIsSecure = false;

        if (!mUri.empty()) {
            const char* uri = mUri.c_str();
            String8 contentType;

            if (!strncasecmp("http://", uri, 7) || !strncasecmp("https://", uri, 8)) {
                sp<DataSource> httpSource;
                mDisconnectLock.unlock();
                httpSource = DataSourceFactory::CreateMediaHTTP(mHTTPService);
                if (httpSource == NULL) {
                    ALOGE("Failed to create http source!");
                    notifyPreparedAndCleanup(UNKNOWN_ERROR);
                    return;
                }
                mDisconnectLock.lock();

                if (!mDisconnected) {
                    mHttpSource = httpSource;
                }
            }

            mLock.unlock();
            mDisconnectLock.unlock();
            // This might take long time if connection has some issue.
            sp<DataSource> dataSource = DataSourceFactory::CreateFromURI(
                   mHTTPService, uri, &mUriHeaders, &contentType,
                   static_cast<HTTPBase *>(mHttpSource.get()));
            mDisconnectLock.lock();
            mLock.lock();
            if (!mDisconnected) {
                mDataSource = dataSource;
            }
        } else {
            if (property_get_bool("media.stagefright.extractremote", true) &&
                    !FileSource::requiresDrm(mFd, mOffset, mLength, nullptr /* mime */)) {
                sp<IBinder> binder =
                        defaultServiceManager()->getService(String16("media.extractor"));
                if (binder != nullptr) {
                    ALOGD("FileSource remote");
                    sp<IMediaExtractorService> mediaExService(
                            interface_cast<IMediaExtractorService>(binder));
                    sp<IDataSource> source =
                            mediaExService->makeIDataSource(mFd, mOffset, mLength);
                    ALOGV("IDataSource(FileSource): %p %d %lld %lld",
                            source.get(), mFd, (long long)mOffset, (long long)mLength);
                    if (source.get() != nullptr) {
                        mDataSource = CreateDataSourceFromIDataSource(source);
                        if (mDataSource != nullptr) {
                            // Close the local file descriptor as it is not needed anymore.
                            close(mFd);
                            mFd = -1;
                        }
                    } else {
                        ALOGW("extractor service cannot make data source");
                    }
                } else {
                    ALOGW("extractor service not running");
                }
            }
            if (mDataSource == nullptr) {
                ALOGD("FileSource local");
                mDataSource = new FileSource(mFd, mOffset, mLength);
            }
            // TODO: close should always be done on mFd, see the lines following
            // CreateDataSourceFromIDataSource above,
            // and the FileSource constructor should dup the mFd argument as needed.
            mFd = -1;
        }

        if (mDataSource == NULL) {
            ALOGE("Failed to create data source!");
            mDisconnectLock.unlock();
            notifyPreparedAndCleanup(UNKNOWN_ERROR);
            return;
        }
    }

    if (mDataSource->flags() & DataSource::kIsCachingDataSource) {
        mCachedSource = static_cast<NuCachedSource2 *>(mDataSource.get());
    }

    mDisconnectLock.unlock();

    // For cached streaming cases, we need to wait for enough
    // buffering before reporting prepared.
    mIsStreaming = (mCachedSource != NULL);

    // init extractor from data source
    status_t err = initFromDataSource();

    if (err != OK) {
        ALOGE("Failed to init from data source!");
        notifyPreparedAndCleanup(err);
        return;
    }

    if (mVideoTrack.mSource != NULL) {
        sp<MetaData> meta = getFormatMeta_l(false /* audio */);
        sp<AMessage> msg = new AMessage;
        err = convertMetaDataToMessage(meta, &msg);
        if(err != OK) {
            notifyPreparedAndCleanup(err);
            return;
        }
        notifyVideoSizeChanged(msg);
    }

    notifyFlagsChanged(
            // FLAG_SECURE will be known if/when prepareDrm is called by the app
            // FLAG_PROTECTED will be known if/when prepareDrm is called by the app
            FLAG_CAN_PAUSE |
            FLAG_CAN_SEEK_BACKWARD |
            FLAG_CAN_SEEK_FORWARD |
            FLAG_CAN_SEEK);

    finishPrepareAsync();

    ALOGV("onPrepareAsync: Done");
}

void NuPlayer::GenericSource::finishPrepareAsync() {
    ALOGV("finishPrepareAsync");

    status_t err = startSources();
    if (err != OK) {
        ALOGE("Failed to init start data source!");
        notifyPreparedAndCleanup(err);
        return;
    }

    if (mIsStreaming) {
        mCachedSource->resumeFetchingIfNecessary();
        mPreparing = true;
        schedulePollBuffering();
    } else {
        notifyPrepared();
    }

    if (mAudioTrack.mSource != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_AUDIO);
    }

    if (mVideoTrack.mSource != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_VIDEO);
    }
}

void NuPlayer::GenericSource::notifyPreparedAndCleanup(status_t err) {
    if (err != OK) {
        {
            Mutex::Autolock _l_d(mDisconnectLock);
            mDataSource.clear();
            mHttpSource.clear();
        }

        mCachedSource.clear();

        mBitrate = -1;
        mPrevBufferPercentage = -1;
        ++mPollBufferingGeneration;
    }
    notifyPrepared(err);
}

void NuPlayer::GenericSource::start() {
    Mutex::Autolock _l(mLock);
    ALOGI("start");

    if (mAudioTrack.mSource != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_AUDIO);
    }

    if (mVideoTrack.mSource != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_VIDEO);
    }

    mStarted = true;
}

void NuPlayer::GenericSource::stop() {
    Mutex::Autolock _l(mLock);
    mStarted = false;
}

void NuPlayer::GenericSource::pause() {
    Mutex::Autolock _l(mLock);
    mStarted = false;
}

void NuPlayer::GenericSource::resume() {
    Mutex::Autolock _l(mLock);
    mStarted = true;
}

void NuPlayer::GenericSource::disconnect() {
    sp<DataSource> dataSource, httpSource;
    {
        Mutex::Autolock _l_d(mDisconnectLock);
        dataSource = mDataSource;
        httpSource = mHttpSource;
        mDisconnected = true;
    }

    if (dataSource != NULL) {
        // disconnect data source
        if (dataSource->flags() & DataSource::kIsCachingDataSource) {
            static_cast<NuCachedSource2 *>(dataSource.get())->disconnect();
        }
    } else if (httpSource != NULL) {
        static_cast<HTTPBase *>(httpSource.get())->disconnect();
    }
}

status_t NuPlayer::GenericSource::feedMoreTSData() {
    return OK;
}

void NuPlayer::GenericSource::sendCacheStats() {
    int32_t kbps = 0;
    status_t err = UNKNOWN_ERROR;

    if (mCachedSource != NULL) {
        err = mCachedSource->getEstimatedBandwidthKbps(&kbps);
    }

    if (err == OK) {
        sp<AMessage> notify = dupNotify();
        notify->setInt32("what", kWhatCacheStats);
        notify->setInt32("bandwidth", kbps);
        notify->post();
    }
}

void NuPlayer::GenericSource::onMessageReceived(const sp<AMessage> &msg) {
    Mutex::Autolock _l(mLock);
    switch (msg->what()) {
      case kWhatPrepareAsync:
      {
          onPrepareAsync();
          break;
      }
      case kWhatFetchSubtitleData:
      {
          fetchTextData(kWhatSendSubtitleData, MEDIA_TRACK_TYPE_SUBTITLE,
                  mFetchSubtitleDataGeneration, mSubtitleTrack.mPackets, msg);
          break;
      }

      case kWhatFetchTimedTextData:
      {
          fetchTextData(kWhatSendTimedTextData, MEDIA_TRACK_TYPE_TIMEDTEXT,
                  mFetchTimedTextDataGeneration, mTimedTextTrack.mPackets, msg);
          break;
      }

      case kWhatSendSubtitleData:
      {
          sendTextData(kWhatSubtitleData, MEDIA_TRACK_TYPE_SUBTITLE,
                  mFetchSubtitleDataGeneration, mSubtitleTrack.mPackets, msg);
          break;
      }

      case kWhatSendGlobalTimedTextData:
      {
          sendGlobalTextData(kWhatTimedTextData, mFetchTimedTextDataGeneration, msg);
          break;
      }
      case kWhatSendTimedTextData:
      {
          sendTextData(kWhatTimedTextData, MEDIA_TRACK_TYPE_TIMEDTEXT,
                  mFetchTimedTextDataGeneration, mTimedTextTrack.mPackets, msg);
          break;
      }

      case kWhatChangeAVSource:
      {
          int32_t trackIndex;
          CHECK(msg->findInt32("trackIndex", &trackIndex));
          const sp<IMediaSource> source = mSources.itemAt(trackIndex);

          Track* track;
          const char *mime;
          media_track_type trackType, counterpartType;
          sp<MetaData> meta = source->getFormat();
          meta->findCString(kKeyMIMEType, &mime);
          if (!strncasecmp(mime, "audio/", 6)) {
              track = &mAudioTrack;
              trackType = MEDIA_TRACK_TYPE_AUDIO;
              counterpartType = MEDIA_TRACK_TYPE_VIDEO;;
          } else {
              CHECK(!strncasecmp(mime, "video/", 6));
              track = &mVideoTrack;
              trackType = MEDIA_TRACK_TYPE_VIDEO;
              counterpartType = MEDIA_TRACK_TYPE_AUDIO;;
          }


          if (track->mSource != NULL) {
              track->mSource->stop();
          }
          track->mSource = source;
          track->mSource->start();
          track->mIndex = trackIndex;
          ++mAudioDataGeneration;
          ++mVideoDataGeneration;

          int64_t timeUs, actualTimeUs;
          const bool formatChange = true;
          if (trackType == MEDIA_TRACK_TYPE_AUDIO) {
              timeUs = mAudioLastDequeueTimeUs;
          } else {
              timeUs = mVideoLastDequeueTimeUs;
          }
          readBuffer(trackType, timeUs, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */,
                  &actualTimeUs, formatChange);
          readBuffer(counterpartType, -1, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */,
                  NULL, !formatChange);
          ALOGV("timeUs %lld actualTimeUs %lld", (long long)timeUs, (long long)actualTimeUs);

          break;
      }

      case kWhatSeek:
      {
          onSeek(msg);
          break;
      }

      case kWhatReadBuffer:
      {
          onReadBuffer(msg);
          break;
      }

      case kWhatPollBuffering:
      {
          int32_t generation;
          CHECK(msg->findInt32("generation", &generation));
          if (generation == mPollBufferingGeneration) {
              onPollBuffering();
          }
          break;
      }

      default:
          Source::onMessageReceived(msg);
          break;
    }
}

void NuPlayer::GenericSource::fetchTextData(
        uint32_t sendWhat,
        media_track_type type,
        int32_t curGen,
        const sp<AnotherPacketSource>& packets,
        const sp<AMessage>& msg) {
    int32_t msgGeneration;
    CHECK(msg->findInt32("generation", &msgGeneration));
    if (msgGeneration != curGen) {
        // stale
        return;
    }

    int32_t avail;
    if (packets->hasBufferAvailable(&avail)) {
        return;
    }

    int64_t timeUs;
    CHECK(msg->findInt64("timeUs", &timeUs));

    int64_t subTimeUs = 0;
    readBuffer(type, timeUs, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */, &subTimeUs);

    status_t eosResult;
    if (!packets->hasBufferAvailable(&eosResult)) {
        return;
    }

    if (msg->what() == kWhatFetchSubtitleData) {
        subTimeUs -= 1000000LL;  // send subtile data one second earlier
    }
    sp<AMessage> msg2 = new AMessage(sendWhat, this);
    msg2->setInt32("generation", msgGeneration);
    mMediaClock->addTimer(msg2, subTimeUs);
}

void NuPlayer::GenericSource::sendTextData(
        uint32_t what,
        media_track_type type,
        int32_t curGen,
        const sp<AnotherPacketSource>& packets,
        const sp<AMessage>& msg) {
    int32_t msgGeneration;
    CHECK(msg->findInt32("generation", &msgGeneration));
    if (msgGeneration != curGen) {
        // stale
        return;
    }

    int64_t subTimeUs;
    if (packets->nextBufferTime(&subTimeUs) != OK) {
        return;
    }

    int64_t nextSubTimeUs;
    readBuffer(type, -1, MediaPlayerSeekMode::SEEK_PREVIOUS_SYNC /* mode */, &nextSubTimeUs);

    sp<ABuffer> buffer;
    status_t dequeueStatus = packets->dequeueAccessUnit(&buffer);
    if (dequeueStatus == OK) {
        sp<AMessage> notify = dupNotify();
        notify->setInt32("what", what);
        notify->setBuffer("buffer", buffer);
        notify->post();

        if (msg->what() == kWhatSendSubtitleData) {
            nextSubTimeUs -= 1000000LL;  // send subtile data one second earlier
        }
        mMediaClock->addTimer(msg, nextSubTimeUs);
    }
}

void NuPlayer::GenericSource::sendGlobalTextData(
        uint32_t what,
        int32_t curGen,
        sp<AMessage> msg) {
    int32_t msgGeneration;
    CHECK(msg->findInt32("generation", &msgGeneration));
    if (msgGeneration != curGen) {
        // stale
        return;
    }

    uint32_t textType;
    const void *data;
    size_t size = 0;
    if (mTimedTextTrack.mSource->getFormat()->findData(
                    kKeyTextFormatData, &textType, &data, &size)) {
        mGlobalTimedText = new ABuffer(size);
        if (mGlobalTimedText->data()) {
            memcpy(mGlobalTimedText->data(), data, size);
            sp<AMessage> globalMeta = mGlobalTimedText->meta();
            globalMeta->setInt64("timeUs", 0);
            globalMeta->setString("mime", MEDIA_MIMETYPE_TEXT_3GPP);
            globalMeta->setInt32("global", 1);
            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", what);
            notify->setBuffer("buffer", mGlobalTimedText);
            notify->post();
        }
    }
}

sp<MetaData> NuPlayer::GenericSource::getFormatMeta(bool audio) {
    Mutex::Autolock _l(mLock);
    return getFormatMeta_l(audio);
}

sp<MetaData> NuPlayer::GenericSource::getFormatMeta_l(bool audio) {
    sp<IMediaSource> source = audio ? mAudioTrack.mSource : mVideoTrack.mSource;

    if (source == NULL) {
        return NULL;
    }

    return source->getFormat();
}

status_t NuPlayer::GenericSource::dequeueAccessUnit(
        bool audio, sp<ABuffer> *accessUnit) {
    Mutex::Autolock _l(mLock);
    // If has gone through stop/releaseDrm sequence, we no longer send down any buffer b/c
    // the codec's crypto object has gone away (b/37960096).
    // Note: This will be unnecessary when stop() changes behavior and releases codec (b/35248283).
    if (!mStarted && mIsDrmReleased) {
        return -EWOULDBLOCK;
    }

    Track *track = audio ? &mAudioTrack : &mVideoTrack;

    if (track->mSource == NULL) {
        return -EWOULDBLOCK;
    }

    status_t finalResult;
    if (!track->mPackets->hasBufferAvailable(&finalResult)) {
        if (finalResult == OK) {
            postReadBuffer(
                    audio ? MEDIA_TRACK_TYPE_AUDIO : MEDIA_TRACK_TYPE_VIDEO);
            return -EWOULDBLOCK;
        }
        return finalResult;
    }

    status_t result = track->mPackets->dequeueAccessUnit(accessUnit);

    // start pulling in more buffers if cache is running low
    // so that decoder has less chance of being starved
    if (!mIsStreaming) {
        if (track->mPackets->getAvailableBufferCount(&finalResult) < 2) {
            postReadBuffer(audio? MEDIA_TRACK_TYPE_AUDIO : MEDIA_TRACK_TYPE_VIDEO);
        }
    } else {
        int64_t durationUs = track->mPackets->getBufferedDurationUs(&finalResult);
        // TODO: maxRebufferingMarkMs could be larger than
        // mBufferingSettings.mResumePlaybackMarkMs
        int64_t restartBufferingMarkUs =
             mBufferingSettings.mResumePlaybackMarkMs * 1000LL / 2;
        if (finalResult == OK) {
            if (durationUs < restartBufferingMarkUs) {
                postReadBuffer(audio? MEDIA_TRACK_TYPE_AUDIO : MEDIA_TRACK_TYPE_VIDEO);
            }
            if (track->mPackets->getAvailableBufferCount(&finalResult) < 2
                && !mSentPauseOnBuffering && !mPreparing) {
                mCachedSource->resumeFetchingIfNecessary();
                sendCacheStats();
                mSentPauseOnBuffering = true;
                sp<AMessage> notify = dupNotify();
                notify->setInt32("what", kWhatPauseOnBufferingStart);
                notify->post();
            }
        }
    }

    if (result != OK) {
        if (mSubtitleTrack.mSource != NULL) {
            mSubtitleTrack.mPackets->clear();
            mFetchSubtitleDataGeneration++;
        }
        if (mTimedTextTrack.mSource != NULL) {
            mTimedTextTrack.mPackets->clear();
            mFetchTimedTextDataGeneration++;
        }
        return result;
    }

    int64_t timeUs;
    status_t eosResult; // ignored
    CHECK((*accessUnit)->meta()->findInt64("timeUs", &timeUs));
    if (audio) {
        mAudioLastDequeueTimeUs = timeUs;
    } else {
        mVideoLastDequeueTimeUs = timeUs;
    }

    if (mSubtitleTrack.mSource != NULL
            && !mSubtitleTrack.mPackets->hasBufferAvailable(&eosResult)) {
        sp<AMessage> msg = new AMessage(kWhatFetchSubtitleData, this);
        msg->setInt64("timeUs", timeUs);
        msg->setInt32("generation", mFetchSubtitleDataGeneration);
        msg->post();
    }

    if (mTimedTextTrack.mSource != NULL
            && !mTimedTextTrack.mPackets->hasBufferAvailable(&eosResult)) {
        sp<AMessage> msg = new AMessage(kWhatFetchTimedTextData, this);
        msg->setInt64("timeUs", timeUs);
        msg->setInt32("generation", mFetchTimedTextDataGeneration);
        msg->post();
    }

    return result;
}

status_t NuPlayer::GenericSource::getDuration(int64_t *durationUs) {
    Mutex::Autolock _l(mLock);
    *durationUs = mDurationUs;
    return OK;
}

size_t NuPlayer::GenericSource::getTrackCount() const {
    Mutex::Autolock _l(mLock);
    return mSources.size();
}

sp<AMessage> NuPlayer::GenericSource::getTrackInfo(size_t trackIndex) const {
    Mutex::Autolock _l(mLock);
    size_t trackCount = mSources.size();
    if (trackIndex >= trackCount) {
        return NULL;
    }

    sp<AMessage> format = new AMessage();
    sp<MetaData> meta = mSources.itemAt(trackIndex)->getFormat();
    if (meta == NULL) {
        ALOGE("no metadata for track %zu", trackIndex);
        format->setInt32("type", MEDIA_TRACK_TYPE_UNKNOWN);
        format->setString("mime", "application/octet-stream");
        format->setString("language", "und");

        return format;
    }

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));
    format->setString("mime", mime);

    int32_t trackType;
    if (!strncasecmp(mime, "video/", 6)) {
        trackType = MEDIA_TRACK_TYPE_VIDEO;
    } else if (!strncasecmp(mime, "audio/", 6)) {
        trackType = MEDIA_TRACK_TYPE_AUDIO;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_TEXT_3GPP)) {
        trackType = MEDIA_TRACK_TYPE_TIMEDTEXT;
    } else {
        trackType = MEDIA_TRACK_TYPE_UNKNOWN;
    }
    format->setInt32("type", trackType);

    const char *lang;
    if (!meta->findCString(kKeyMediaLanguage, &lang)) {
        lang = "und";
    }
    format->setString("language", lang);

    if (trackType == MEDIA_TRACK_TYPE_SUBTITLE) {
        int32_t isAutoselect = 1, isDefault = 0, isForced = 0;
        meta->findInt32(kKeyTrackIsAutoselect, &isAutoselect);
        meta->findInt32(kKeyTrackIsDefault, &isDefault);
        meta->findInt32(kKeyTrackIsForced, &isForced);

        format->setInt32("auto", !!isAutoselect);
        format->setInt32("default", !!isDefault);
        format->setInt32("forced", !!isForced);
    }

    return format;
}

ssize_t NuPlayer::GenericSource::getSelectedTrack(media_track_type type) const {
    Mutex::Autolock _l(mLock);
    const Track *track = NULL;
    switch (type) {
    case MEDIA_TRACK_TYPE_VIDEO:
        track = &mVideoTrack;
        break;
    case MEDIA_TRACK_TYPE_AUDIO:
        track = &mAudioTrack;
        break;
    case MEDIA_TRACK_TYPE_TIMEDTEXT:
        track = &mTimedTextTrack;
        break;
    case MEDIA_TRACK_TYPE_SUBTITLE:
        track = &mSubtitleTrack;
        break;
    default:
        break;
    }

    if (track != NULL && track->mSource != NULL) {
        return track->mIndex;
    }

    return -1;
}

status_t NuPlayer::GenericSource::selectTrack(size_t trackIndex, bool select, int64_t timeUs) {
    Mutex::Autolock _l(mLock);
    ALOGV("%s track: %zu", select ? "select" : "deselect", trackIndex);

    if (trackIndex >= mSources.size()) {
        return BAD_INDEX;
    }

    if (!select) {
        Track* track = NULL;
        if (mSubtitleTrack.mSource != NULL && trackIndex == mSubtitleTrack.mIndex) {
            track = &mSubtitleTrack;
            mFetchSubtitleDataGeneration++;
        } else if (mTimedTextTrack.mSource != NULL && trackIndex == mTimedTextTrack.mIndex) {
            track = &mTimedTextTrack;
            mFetchTimedTextDataGeneration++;
        }
        if (track == NULL) {
            return INVALID_OPERATION;
        }
        track->mSource->stop();
        track->mSource = NULL;
        track->mPackets->clear();
        return OK;
    }

    const sp<IMediaSource> source = mSources.itemAt(trackIndex);
    sp<MetaData> meta = source->getFormat();
    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));
    if (!strncasecmp(mime, "text/", 5)) {
        bool isSubtitle = strcasecmp(mime, MEDIA_MIMETYPE_TEXT_3GPP);
        Track *track = isSubtitle ? &mSubtitleTrack : &mTimedTextTrack;
        if (track->mSource != NULL && track->mIndex == trackIndex) {
            return OK;
        }
        track->mIndex = trackIndex;
        if (track->mSource != NULL) {
            track->mSource->stop();
        }
        track->mSource = mSources.itemAt(trackIndex);
        track->mSource->start();
        if (track->mPackets == NULL) {
            track->mPackets = new AnotherPacketSource(track->mSource->getFormat());
        } else {
            track->mPackets->clear();
            track->mPackets->setFormat(track->mSource->getFormat());

        }

        if (isSubtitle) {
            mFetchSubtitleDataGeneration++;
        } else {
            mFetchTimedTextDataGeneration++;
        }

        status_t eosResult; // ignored
        if (mSubtitleTrack.mSource != NULL
                && !mSubtitleTrack.mPackets->hasBufferAvailable(&eosResult)) {
            sp<AMessage> msg = new AMessage(kWhatFetchSubtitleData, this);
            msg->setInt64("timeUs", timeUs);
            msg->setInt32("generation", mFetchSubtitleDataGeneration);
            msg->post();
        }

        sp<AMessage> msg2 = new AMessage(kWhatSendGlobalTimedTextData, this);
        msg2->setInt32("generation", mFetchTimedTextDataGeneration);
        msg2->post();

        if (mTimedTextTrack.mSource != NULL
                && !mTimedTextTrack.mPackets->hasBufferAvailable(&eosResult)) {
            sp<AMessage> msg = new AMessage(kWhatFetchTimedTextData, this);
            msg->setInt64("timeUs", timeUs);
            msg->setInt32("generation", mFetchTimedTextDataGeneration);
            msg->post();
        }

        return OK;
    } else if (!strncasecmp(mime, "audio/", 6) || !strncasecmp(mime, "video/", 6)) {
        bool audio = !strncasecmp(mime, "audio/", 6);
        Track *track = audio ? &mAudioTrack : &mVideoTrack;
        if (track->mSource != NULL && track->mIndex == trackIndex) {
            return OK;
        }

        sp<AMessage> msg = new AMessage(kWhatChangeAVSource, this);
        msg->setInt32("trackIndex", trackIndex);
        msg->post();
        return OK;
    }

    return INVALID_OPERATION;
}

status_t NuPlayer::GenericSource::seekTo(int64_t seekTimeUs, MediaPlayerSeekMode mode) {
    ALOGV("seekTo: %lld, %d", (long long)seekTimeUs, mode);
    sp<AMessage> msg = new AMessage(kWhatSeek, this);
    msg->setInt64("seekTimeUs", seekTimeUs);
    msg->setInt32("mode", mode);

    // Need to call readBuffer on |mLooper| to ensure the calls to
    // IMediaSource::read* are serialized. Note that IMediaSource::read*
    // is called without |mLock| acquired and MediaSource is not thread safe.
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    if (err == OK && response != NULL) {
        CHECK(response->findInt32("err", &err));
    }

    return err;
}

void NuPlayer::GenericSource::onSeek(const sp<AMessage>& msg) {
    int64_t seekTimeUs;
    int32_t mode;
    CHECK(msg->findInt64("seekTimeUs", &seekTimeUs));
    CHECK(msg->findInt32("mode", &mode));

    sp<AMessage> response = new AMessage;
    status_t err = doSeek(seekTimeUs, (MediaPlayerSeekMode)mode);
    response->setInt32("err", err);

    sp<AReplyToken> replyID;
    CHECK(msg->senderAwaitsResponse(&replyID));
    response->postReply(replyID);
}

status_t NuPlayer::GenericSource::doSeek(int64_t seekTimeUs, MediaPlayerSeekMode mode) {
    if (mVideoTrack.mSource != NULL) {
        ++mVideoDataGeneration;

        int64_t actualTimeUs;
        readBuffer(MEDIA_TRACK_TYPE_VIDEO, seekTimeUs, mode, &actualTimeUs);

        if (mode != MediaPlayerSeekMode::SEEK_CLOSEST) {
            seekTimeUs = actualTimeUs;
        }
        mVideoLastDequeueTimeUs = actualTimeUs;
    }

    if (mAudioTrack.mSource != NULL) {
        ++mAudioDataGeneration;
        readBuffer(MEDIA_TRACK_TYPE_AUDIO, seekTimeUs, MediaPlayerSeekMode::SEEK_CLOSEST);
        mAudioLastDequeueTimeUs = seekTimeUs;
    }

    if (mSubtitleTrack.mSource != NULL) {
        mSubtitleTrack.mPackets->clear();
        mFetchSubtitleDataGeneration++;
    }

    if (mTimedTextTrack.mSource != NULL) {
        mTimedTextTrack.mPackets->clear();
        mFetchTimedTextDataGeneration++;
    }

    ++mPollBufferingGeneration;
    schedulePollBuffering();
    return OK;
}

sp<ABuffer> NuPlayer::GenericSource::mediaBufferToABuffer(
        MediaBufferBase* mb,
        media_track_type trackType) {
    bool audio = trackType == MEDIA_TRACK_TYPE_AUDIO;
    size_t outLength = mb->range_length();

    if (audio && mAudioIsVorbis) {
        outLength += sizeof(int32_t);
    }

    sp<ABuffer> ab;

    if (mIsDrmProtected)   {
        // Modular DRM
        // Enabled for both video/audio so 1) media buffer is reused without extra copying
        // 2) meta data can be retrieved in onInputBufferFetched for calling queueSecureInputBuffer.

        // data is already provided in the buffer
        ab = new ABuffer(NULL, mb->range_length());
        ab->meta()->setObject("mediaBufferHolder", new MediaBufferHolder(mb));

        // Modular DRM: Required b/c of the above add_ref.
        // If ref>0, there must be an observer, or it'll crash at release().
        // TODO: MediaBuffer might need to be revised to ease such need.
        mb->setObserver(this);
        // Extra increment (since we want to keep mb alive and attached to ab beyond this function
        // call. This is to counter the effect of mb->release() towards the end.
        mb->add_ref();

    } else {
        ab = new ABuffer(outLength);
        memcpy(ab->data(),
               (const uint8_t *)mb->data() + mb->range_offset(),
               mb->range_length());
    }

    if (audio && mAudioIsVorbis) {
        int32_t numPageSamples;
        if (!mb->meta_data().findInt32(kKeyValidSamples, &numPageSamples)) {
            numPageSamples = -1;
        }

        uint8_t* abEnd = ab->data() + mb->range_length();
        memcpy(abEnd, &numPageSamples, sizeof(numPageSamples));
    }

    sp<AMessage> meta = ab->meta();

    int64_t timeUs;
    CHECK(mb->meta_data().findInt64(kKeyTime, &timeUs));
    meta->setInt64("timeUs", timeUs);

    if (trackType == MEDIA_TRACK_TYPE_VIDEO) {
        int32_t layerId;
        if (mb->meta_data().findInt32(kKeyTemporalLayerId, &layerId)) {
            meta->setInt32("temporal-layer-id", layerId);
        }
    }

    if (trackType == MEDIA_TRACK_TYPE_TIMEDTEXT) {
        const char *mime;
        CHECK(mTimedTextTrack.mSource != NULL
                && mTimedTextTrack.mSource->getFormat()->findCString(kKeyMIMEType, &mime));
        meta->setString("mime", mime);
    }

    int64_t durationUs;
    if (mb->meta_data().findInt64(kKeyDuration, &durationUs)) {
        meta->setInt64("durationUs", durationUs);
    }

    if (trackType == MEDIA_TRACK_TYPE_SUBTITLE) {
        meta->setInt32("track-index", mSubtitleTrack.mIndex);
    }

    uint32_t dataType; // unused
    const void *seiData;
    size_t seiLength;
    if (mb->meta_data().findData(kKeySEI, &dataType, &seiData, &seiLength)) {
        sp<ABuffer> sei = ABuffer::CreateAsCopy(seiData, seiLength);;
        meta->setBuffer("sei", sei);
    }

    const void *mpegUserDataPointer;
    size_t mpegUserDataLength;
    if (mb->meta_data().findData(
            kKeyMpegUserData, &dataType, &mpegUserDataPointer, &mpegUserDataLength)) {
        sp<ABuffer> mpegUserData = ABuffer::CreateAsCopy(mpegUserDataPointer, mpegUserDataLength);
        meta->setBuffer("mpeg-user-data", mpegUserData);
    }

    mb->release();
    mb = NULL;

    return ab;
}

int32_t NuPlayer::GenericSource::getDataGeneration(media_track_type type) const {
    int32_t generation = -1;
    switch (type) {
    case MEDIA_TRACK_TYPE_VIDEO:
        generation = mVideoDataGeneration;
        break;
    case MEDIA_TRACK_TYPE_AUDIO:
        generation = mAudioDataGeneration;
        break;
    case MEDIA_TRACK_TYPE_TIMEDTEXT:
        generation = mFetchTimedTextDataGeneration;
        break;
    case MEDIA_TRACK_TYPE_SUBTITLE:
        generation = mFetchSubtitleDataGeneration;
        break;
    default:
        break;
    }

    return generation;
}

void NuPlayer::GenericSource::postReadBuffer(media_track_type trackType) {
    if ((mPendingReadBufferTypes & (1 << trackType)) == 0) {
        mPendingReadBufferTypes |= (1 << trackType);
        sp<AMessage> msg = new AMessage(kWhatReadBuffer, this);
        msg->setInt32("trackType", trackType);
        msg->post();
    }
}

void NuPlayer::GenericSource::onReadBuffer(const sp<AMessage>& msg) {
    int32_t tmpType;
    CHECK(msg->findInt32("trackType", &tmpType));
    media_track_type trackType = (media_track_type)tmpType;
    mPendingReadBufferTypes &= ~(1 << trackType);
    readBuffer(trackType);
}

void NuPlayer::GenericSource::readBuffer(
        media_track_type trackType, int64_t seekTimeUs, MediaPlayerSeekMode mode,
        int64_t *actualTimeUs, bool formatChange) {
    Track *track;
    size_t maxBuffers = 1;
    switch (trackType) {
        case MEDIA_TRACK_TYPE_VIDEO:
            track = &mVideoTrack;
            maxBuffers = 8;  // too large of a number may influence seeks
            break;
        case MEDIA_TRACK_TYPE_AUDIO:
            track = &mAudioTrack;
            maxBuffers = 64;
            break;
        case MEDIA_TRACK_TYPE_SUBTITLE:
            track = &mSubtitleTrack;
            break;
        case MEDIA_TRACK_TYPE_TIMEDTEXT:
            track = &mTimedTextTrack;
            break;
        default:
            TRESPASS();
    }

    if (track->mSource == NULL) {
        return;
    }

    if (actualTimeUs) {
        *actualTimeUs = seekTimeUs;
    }

    MediaSource::ReadOptions options;

    bool seeking = false;
    if (seekTimeUs >= 0) {
        options.setSeekTo(seekTimeUs, mode);
        seeking = true;
    }

    const bool couldReadMultiple = (track->mSource->supportReadMultiple());

    if (couldReadMultiple) {
        options.setNonBlocking();
    }

    int32_t generation = getDataGeneration(trackType);
    for (size_t numBuffers = 0; numBuffers < maxBuffers; ) {
        Vector<MediaBufferBase *> mediaBuffers;
        status_t err = NO_ERROR;

        sp<IMediaSource> source = track->mSource;
        mLock.unlock();
        if (couldReadMultiple) {
            err = source->readMultiple(
                    &mediaBuffers, maxBuffers - numBuffers, &options);
        } else {
            MediaBufferBase *mbuf = NULL;
            err = source->read(&mbuf, &options);
            if (err == OK && mbuf != NULL) {
                mediaBuffers.push_back(mbuf);
            }
        }
        mLock.lock();

        options.clearNonPersistent();

        size_t id = 0;
        size_t count = mediaBuffers.size();

        // in case track has been changed since we don't have lock for some time.
        if (generation != getDataGeneration(trackType)) {
            for (; id < count; ++id) {
                mediaBuffers[id]->release();
            }
            break;
        }

        for (; id < count; ++id) {
            int64_t timeUs;
            MediaBufferBase *mbuf = mediaBuffers[id];
            if (!mbuf->meta_data().findInt64(kKeyTime, &timeUs)) {
                mbuf->meta_data().dumpToLog();
                track->mPackets->signalEOS(ERROR_MALFORMED);
                break;
            }
            if (trackType == MEDIA_TRACK_TYPE_AUDIO) {
                mAudioTimeUs = timeUs;
            } else if (trackType == MEDIA_TRACK_TYPE_VIDEO) {
                mVideoTimeUs = timeUs;
            }

            queueDiscontinuityIfNeeded(seeking, formatChange, trackType, track);

            sp<ABuffer> buffer = mediaBufferToABuffer(mbuf, trackType);
            if (numBuffers == 0 && actualTimeUs != nullptr) {
                *actualTimeUs = timeUs;
            }
            if (seeking && buffer != nullptr) {
                sp<AMessage> meta = buffer->meta();
                if (meta != nullptr && mode == MediaPlayerSeekMode::SEEK_CLOSEST
                        && seekTimeUs > timeUs) {
                    sp<AMessage> extra = new AMessage;
                    extra->setInt64("resume-at-mediaTimeUs", seekTimeUs);
                    meta->setMessage("extra", extra);
                }
            }

            track->mPackets->queueAccessUnit(buffer);
            formatChange = false;
            seeking = false;
            ++numBuffers;
        }
        if (id < count) {
            // Error, some mediaBuffer doesn't have kKeyTime.
            for (; id < count; ++id) {
                mediaBuffers[id]->release();
            }
            break;
        }

        if (err == WOULD_BLOCK) {
            break;
        } else if (err == INFO_FORMAT_CHANGED) {
#if 0
            track->mPackets->queueDiscontinuity(
                    ATSParser::DISCONTINUITY_FORMATCHANGE,
                    NULL,
                    false /* discard */);
#endif
        } else if (err != OK) {
            queueDiscontinuityIfNeeded(seeking, formatChange, trackType, track);
            track->mPackets->signalEOS(err);
            break;
        }
    }

    if (mIsStreaming
        && (trackType == MEDIA_TRACK_TYPE_VIDEO || trackType == MEDIA_TRACK_TYPE_AUDIO)) {
        status_t finalResult;
        int64_t durationUs = track->mPackets->getBufferedDurationUs(&finalResult);

        // TODO: maxRebufferingMarkMs could be larger than
        // mBufferingSettings.mResumePlaybackMarkMs
        int64_t markUs = (mPreparing ? mBufferingSettings.mInitialMarkMs
            : mBufferingSettings.mResumePlaybackMarkMs) * 1000LL;
        if (finalResult == ERROR_END_OF_STREAM || durationUs >= markUs) {
            if (mPreparing || mSentPauseOnBuffering) {
                Track *counterTrack =
                    (trackType == MEDIA_TRACK_TYPE_VIDEO ? &mAudioTrack : &mVideoTrack);
                if (counterTrack->mSource != NULL) {
                    durationUs = counterTrack->mPackets->getBufferedDurationUs(&finalResult);
                }
                if (finalResult == ERROR_END_OF_STREAM || durationUs >= markUs) {
                    if (mPreparing) {
                        notifyPrepared();
                        mPreparing = false;
                    } else {
                        sendCacheStats();
                        mSentPauseOnBuffering = false;
                        sp<AMessage> notify = dupNotify();
                        notify->setInt32("what", kWhatResumeOnBufferingEnd);
                        notify->post();
                    }
                }
            }
            return;
        }

        postReadBuffer(trackType);
    }
}

void NuPlayer::GenericSource::queueDiscontinuityIfNeeded(
        bool seeking, bool formatChange, media_track_type trackType, Track *track) {
    // formatChange && seeking: track whose source is changed during selection
    // formatChange && !seeking: track whose source is not changed during selection
    // !formatChange: normal seek
    if ((seeking || formatChange)
            && (trackType == MEDIA_TRACK_TYPE_AUDIO
            || trackType == MEDIA_TRACK_TYPE_VIDEO)) {
        ATSParser::DiscontinuityType type = (formatChange && seeking)
                ? ATSParser::DISCONTINUITY_FORMATCHANGE
                : ATSParser::DISCONTINUITY_NONE;
        track->mPackets->queueDiscontinuity(type, NULL /* extra */, true /* discard */);
    }
}

void NuPlayer::GenericSource::notifyBufferingUpdate(int32_t percentage) {
    // Buffering percent could go backward as it's estimated from remaining
    // data and last access time. This could cause the buffering position
    // drawn on media control to jitter slightly. Remember previously reported
    // percentage and don't allow it to go backward.
    if (percentage < mPrevBufferPercentage) {
        percentage = mPrevBufferPercentage;
    } else if (percentage > 100) {
        percentage = 100;
    }

    mPrevBufferPercentage = percentage;

    ALOGV("notifyBufferingUpdate: buffering %d%%", percentage);

    sp<AMessage> notify = dupNotify();
    notify->setInt32("what", kWhatBufferingUpdate);
    notify->setInt32("percentage", percentage);
    notify->post();
}

void NuPlayer::GenericSource::schedulePollBuffering() {
    sp<AMessage> msg = new AMessage(kWhatPollBuffering, this);
    msg->setInt32("generation", mPollBufferingGeneration);
    // Enquires buffering status every second.
    msg->post(1000000LL);
}

void NuPlayer::GenericSource::onPollBuffering() {
    status_t finalStatus = UNKNOWN_ERROR;
    int64_t cachedDurationUs = -1LL;
    ssize_t cachedDataRemaining = -1;

    if (mCachedSource != NULL) {
        cachedDataRemaining = mCachedSource->approxDataRemaining(&finalStatus);

        if (finalStatus == OK) {
            off64_t size;
            int64_t bitrate = 0LL;
            if (mDurationUs > 0 && mCachedSource->getSize(&size) == OK) {
                // |bitrate| uses bits/second unit, while size is number of bytes.
                bitrate = size * 8000000LL / mDurationUs;
            } else if (mBitrate > 0) {
                bitrate = mBitrate;
            }
            if (bitrate > 0) {
                cachedDurationUs = cachedDataRemaining * 8000000LL / bitrate;
            }
        }
    }

    if (finalStatus != OK) {
        ALOGV("onPollBuffering: EOS (finalStatus = %d)", finalStatus);

        if (finalStatus == ERROR_END_OF_STREAM) {
            notifyBufferingUpdate(100);
        }

        if (mPreparing) {
            notifyPreparedAndCleanup(finalStatus == ERROR_END_OF_STREAM ? OK : finalStatus);
            mPreparing = false;
        } else if (mSentPauseOnBuffering) {
            sendCacheStats();
            mSentPauseOnBuffering = false;
            sp<AMessage> notify = dupNotify();
            notify->setInt32("what", kWhatResumeOnBufferingEnd);
            notify->post();
        }
        return;
    }

    if (cachedDurationUs >= 0LL) {
        if (mDurationUs > 0LL) {
            int64_t cachedPosUs = getLastReadPosition() + cachedDurationUs;
            int percentage = 100.0 * cachedPosUs / mDurationUs;
            if (percentage > 100) {
                percentage = 100;
            }

            notifyBufferingUpdate(percentage);
        }

        ALOGV("onPollBuffering: cachedDurationUs %.1f sec", cachedDurationUs / 1000000.0f);
    }

    schedulePollBuffering();
}

// Modular DRM
status_t NuPlayer::GenericSource::prepareDrm(
        const uint8_t uuid[16], const Vector<uint8_t> &drmSessionId, sp<ICrypto> *outCrypto) {
    Mutex::Autolock _l(mLock);
    ALOGV("prepareDrm");

    mIsDrmProtected = false;
    mIsDrmReleased = false;
    mIsSecure = false;

    status_t status = OK;
    sp<ICrypto> crypto = NuPlayerDrm::createCryptoAndPlugin(uuid, drmSessionId, status);
    if (crypto == NULL) {
        ALOGE("prepareDrm: createCrypto failed. status: %d", status);
        return status;
    }
    ALOGV("prepareDrm: createCryptoAndPlugin succeeded for uuid: %s",
            DrmUUID::toHexString(uuid).string());

    *outCrypto = crypto;
    // as long a there is an active crypto
    mIsDrmProtected = true;

    if (mMimes.size() == 0) {
        status = UNKNOWN_ERROR;
        ALOGE("prepareDrm: Unexpected. Must have at least one track. status: %d", status);
        return status;
    }

    // first mime in this list is either the video track, or the first audio track
    const char *mime = mMimes[0].string();
    mIsSecure = crypto->requiresSecureDecoderComponent(mime);
    ALOGV("prepareDrm: requiresSecureDecoderComponent mime: %s  isSecure: %d",
            mime, mIsSecure);

    // Checking the member flags while in the looper to send out the notification.
    // The legacy mDecryptHandle!=NULL check (for FLAG_PROTECTED) is equivalent to mIsDrmProtected.
    notifyFlagsChanged(
            (mIsSecure ? FLAG_SECURE : 0) |
            // Setting "protected screen" only for L1: b/38390836
            (mIsSecure ? FLAG_PROTECTED : 0) |
            FLAG_CAN_PAUSE |
            FLAG_CAN_SEEK_BACKWARD |
            FLAG_CAN_SEEK_FORWARD |
            FLAG_CAN_SEEK);

    if (status == OK) {
        ALOGV("prepareDrm: mCrypto: %p (%d)", outCrypto->get(),
                (*outCrypto != NULL ? (*outCrypto)->getStrongCount() : 0));
        ALOGD("prepareDrm ret: %d ", status);
    } else {
        ALOGE("prepareDrm err: %d", status);
    }
    return status;
}

status_t NuPlayer::GenericSource::releaseDrm() {
    Mutex::Autolock _l(mLock);
    ALOGV("releaseDrm");

    if (mIsDrmProtected) {
        mIsDrmProtected = false;
        // to prevent returning any more buffer after stop/releaseDrm (b/37960096)
        mIsDrmReleased = true;
        ALOGV("releaseDrm: mIsDrmProtected is reset.");
    } else {
        ALOGE("releaseDrm: mIsDrmProtected is already false.");
    }

    return OK;
}

status_t NuPlayer::GenericSource::checkDrmInfo()
{
    // clearing the flag at prepare in case the player is reused after stop/releaseDrm with the
    // same source without being reset (called by prepareAsync/initFromDataSource)
    mIsDrmReleased = false;

    if (mFileMeta == NULL) {
        ALOGI("checkDrmInfo: No metadata");
        return OK; // letting the caller responds accordingly
    }

    uint32_t type;
    const void *pssh;
    size_t psshsize;

    if (!mFileMeta->findData(kKeyPssh, &type, &pssh, &psshsize)) {
        ALOGV("checkDrmInfo: No PSSH");
        return OK; // source without DRM info
    }

    Parcel parcel;
    NuPlayerDrm::retrieveDrmInfo(pssh, psshsize, &parcel);
    ALOGV("checkDrmInfo: MEDIA_DRM_INFO PSSH size: %d  Parcel size: %d  objects#: %d",
          (int)psshsize, (int)parcel.dataSize(), (int)parcel.objectsCount());

    if (parcel.dataSize() == 0) {
        ALOGE("checkDrmInfo: Unexpected parcel size: 0");
        return UNKNOWN_ERROR;
    }

    // Can't pass parcel as a message to the player. Converting Parcel->ABuffer to pass it
    // to the Player's onSourceNotify then back to Parcel for calling driver's notifyListener.
    sp<ABuffer> drmInfoBuffer = ABuffer::CreateAsCopy(parcel.data(), parcel.dataSize());
    notifyDrmInfo(drmInfoBuffer);

    return OK;
}

void NuPlayer::GenericSource::signalBufferReturned(MediaBufferBase *buffer)
{
    //ALOGV("signalBufferReturned %p  refCount: %d", buffer, buffer->localRefcount());

    buffer->setObserver(NULL);
    buffer->release(); // this leads to delete since that there is no observor
}

}  // namespace android
