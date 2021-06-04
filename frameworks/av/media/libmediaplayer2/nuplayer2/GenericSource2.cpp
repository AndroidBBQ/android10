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

//#define LOG_NDEBUG 0
#define LOG_TAG "GenericSource2"

#include "GenericSource2.h"
#include "NuPlayer2Drm.h"

#include "AnotherPacketSource.h"
#include <cutils/properties.h>
#include <media/DataSource.h>
#include <media/MediaBufferHolder.h>
#include <media/NdkWrapper.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/NdkUtils.h>
#include <media/stagefright/Utils.h>

namespace android {

static const int kInitialMarkMs        = 5000;  // 5secs

//static const int kPausePlaybackMarkMs  = 2000;  // 2secs
static const int kResumePlaybackMarkMs = 15000;  // 15secs

NuPlayer2::GenericSource2::GenericSource2(
        const sp<AMessage> &notify,
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
      mDurationUs(-1ll),
      mAudioIsVorbis(false),
      mIsSecure(false),
      mIsStreaming(false),
      mUID(uid),
      mMediaClock(mediaClock),
      mFd(-1),
      mBitrate(-1ll),
      mPendingReadBufferTypes(0) {
    ALOGV("GenericSource2");
    CHECK(mediaClock != NULL);

    mBufferingSettings.mInitialMarkMs = kInitialMarkMs;
    mBufferingSettings.mResumePlaybackMarkMs = kResumePlaybackMarkMs;
    resetDataSource();
}

void NuPlayer2::GenericSource2::resetDataSource() {
    ALOGV("resetDataSource");

    mDisconnected = false;
    mUri.clear();
    mUriHeaders.clear();
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

status_t NuPlayer2::GenericSource2::setDataSource(
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    Mutex::Autolock _l(mLock);
    ALOGV("setDataSource url: %s", url);

    resetDataSource();

    mUri = url;

    if (headers) {
        mUriHeaders = *headers;
    }

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    return OK;
}

status_t NuPlayer2::GenericSource2::setDataSource(
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

status_t NuPlayer2::GenericSource2::setDataSource(const sp<DataSource>& source) {
    Mutex::Autolock _l(mLock);
    ALOGV("setDataSource (source: %p)", source.get());

    resetDataSource();
    mDataSourceWrapper = new AMediaDataSourceWrapper(source);
    return OK;
}

sp<MetaData> NuPlayer2::GenericSource2::getFileFormatMeta() const {
    Mutex::Autolock _l(mLock);
    return mFileMeta;
}

status_t NuPlayer2::GenericSource2::initFromDataSource() {
    mExtractor = new AMediaExtractorWrapper(AMediaExtractor_new());
    CHECK(mFd >=0 || mDataSourceWrapper != NULL);
    sp<AMediaDataSourceWrapper> aSourceWrapper = mDataSourceWrapper;
    const int fd = mFd;

    mLock.unlock();
    // This might take long time if data source is not reliable.
    status_t err;
    if (aSourceWrapper != NULL) {
        err = mExtractor->setDataSource(aSourceWrapper->getAMediaDataSource());
    } else {
        err = mExtractor->setDataSource(fd, mOffset, mLength);
    }

    if (err != OK) {
        ALOGE("initFromDataSource, failed to set extractor data source!");
        mLock.lock();
        return UNKNOWN_ERROR;
    }

    size_t numtracks = mExtractor->getTrackCount();
    if (numtracks == 0) {
        ALOGE("initFromDataSource, source has no track!");
        mLock.lock();
        return UNKNOWN_ERROR;
    }

    mFileMeta = convertMediaFormatWrapperToMetaData(mExtractor->getFormat());
    mLock.lock();
    if (mFileMeta != NULL) {
        int64_t duration;
        if (mFileMeta->findInt64(kKeyDuration, &duration)) {
            mDurationUs = duration;
        }
    }

    int32_t totalBitrate = 0;

    mMimes.clear();

    for (size_t i = 0; i < numtracks; ++i) {

        sp<AMediaFormatWrapper> trackFormat = mExtractor->getTrackFormat(i);
        if (trackFormat == NULL) {
            ALOGE("no metadata for track %zu", i);
            return UNKNOWN_ERROR;
        }

        sp<AMediaExtractorWrapper> trackExtractor = new AMediaExtractorWrapper(AMediaExtractor_new());
        if (aSourceWrapper != NULL) {
            trackExtractor->setDataSource(aSourceWrapper->getAMediaDataSource());
        } else {
            trackExtractor->setDataSource(fd, mOffset, mLength);
        }

        const char *mime;
        sp<MetaData> meta = convertMediaFormatWrapperToMetaData(trackFormat);
        CHECK(meta->findCString(kKeyMIMEType, &mime));

        ALOGV("initFromDataSource track[%zu]: %s", i, mime);

        // Do the string compare immediately with "mime",
        // we can't assume "mime" would stay valid after another
        // extractor operation, some extractors might modify meta
        // during getTrack() and make it invalid.
        if (!strncasecmp(mime, "audio/", 6)) {
            if (mAudioTrack.mExtractor == NULL) {
                mAudioTrack.mIndex = i;
                mAudioTrack.mExtractor = trackExtractor;
                mAudioTrack.mExtractor->selectTrack(i);
                mAudioTrack.mPackets = new AnotherPacketSource(meta);

                if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
                    mAudioIsVorbis = true;
                } else {
                    mAudioIsVorbis = false;
                }

                mMimes.add(String8(mime));
            }
        } else if (!strncasecmp(mime, "video/", 6)) {
            if (mVideoTrack.mExtractor == NULL) {
                mVideoTrack.mIndex = i;
                mVideoTrack.mExtractor = trackExtractor;
                mVideoTrack.mExtractor->selectTrack(i);
                mVideoTrack.mPackets = new AnotherPacketSource(meta);

                // video always at the beginning
                mMimes.insertAt(String8(mime), 0);
            }
        }

        mExtractors.push(trackExtractor);
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

    ALOGV("initFromDataSource mExtractors.size(): %zu  mIsSecure: %d  mime[0]: %s", mExtractors.size(),
            mIsSecure, (mMimes.isEmpty() ? "NONE" : mMimes[0].string()));

    if (mExtractors.size() == 0) {
        ALOGE("b/23705695");
        return UNKNOWN_ERROR;
    }

    // Modular DRM: The return value doesn't affect source initialization.
    (void)checkDrmInfo();

    mBitrate = totalBitrate;

    return OK;
}

status_t NuPlayer2::GenericSource2::getBufferingSettings(
        BufferingSettings* buffering /* nonnull */) {
    {
        Mutex::Autolock _l(mLock);
        *buffering = mBufferingSettings;
    }

    ALOGV("getBufferingSettings{%s}", buffering->toString().string());
    return OK;
}

status_t NuPlayer2::GenericSource2::setBufferingSettings(const BufferingSettings& buffering) {
    ALOGV("setBufferingSettings{%s}", buffering.toString().string());

    Mutex::Autolock _l(mLock);
    mBufferingSettings = buffering;
    return OK;
}

int64_t NuPlayer2::GenericSource2::getLastReadPosition() {
    if (mAudioTrack.mExtractor != NULL) {
        return mAudioTimeUs;
    } else if (mVideoTrack.mExtractor != NULL) {
        return mVideoTimeUs;
    } else {
        return 0;
    }
}

bool NuPlayer2::GenericSource2::isStreaming() const {
    Mutex::Autolock _l(mLock);
    return mIsStreaming;
}

NuPlayer2::GenericSource2::~GenericSource2() {
    ALOGV("~GenericSource2");
    if (mLooper != NULL) {
        mLooper->unregisterHandler(id());
        mLooper->stop();
    }
    if (mDataSourceWrapper != NULL) {
        mDataSourceWrapper->close();
    }
    resetDataSource();
}

void NuPlayer2::GenericSource2::prepareAsync(int64_t startTimeUs) {
    Mutex::Autolock _l(mLock);
    ALOGV("prepareAsync: (looper: %d)", (mLooper != NULL));

    if (mLooper == NULL) {
        mLooper = new ALooper;
        mLooper->setName("generic2");
        mLooper->start(false, /* runOnCallingThread */
                       true,  /* canCallJava */
                       PRIORITY_DEFAULT);

        mLooper->registerHandler(this);
    }

    sp<AMessage> msg = new AMessage(kWhatPrepareAsync, this);
    msg->setInt64("startTimeUs", startTimeUs);

    msg->post();
}

void NuPlayer2::GenericSource2::onPrepareAsync(int64_t startTimeUs) {
    ALOGV("onPrepareAsync: mFd %d mUri %s mDataSourceWrapper: %p",
            mFd, mUri.c_str(), mDataSourceWrapper.get());

    if (!mUri.empty()) {
        const char* uri = mUri.c_str();
        size_t numheaders = mUriHeaders.size();
        const char **key_values = numheaders ? new const char *[numheaders * 2] : NULL;
        for (size_t i = 0; i < numheaders; ++i) {
            key_values[i * 2] = mUriHeaders.keyAt(i).c_str();
            key_values[i * 2 + 1] =  mUriHeaders.valueAt(i).c_str();
        }
        mLock.unlock();
        AMediaDataSource *aSource = AMediaDataSource_newUri(uri, numheaders, key_values);
        mLock.lock();
        mDataSourceWrapper = aSource ? new AMediaDataSourceWrapper(aSource) : NULL;
        delete[] key_values;
        // For cached streaming cases, we need to wait for enough
        // buffering before reporting prepared.
        mIsStreaming = !strncasecmp("http://", uri, 7) || !strncasecmp("https://", uri, 8);
    }

    if (mDisconnected || (mFd < 0 && mDataSourceWrapper == NULL)) {
        ALOGE("mDisconnected(%d) or Failed to create data source!", mDisconnected);
        notifyPreparedAndCleanup(UNKNOWN_ERROR);
        return;
    }

    // init extractor from data source
    status_t err = initFromDataSource();
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }

    if (err != OK) {
        ALOGE("Failed to init from data source!");
        notifyPreparedAndCleanup(err);
        return;
    }

    if (mVideoTrack.mExtractor != NULL) {
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

    doSeek(startTimeUs, MediaPlayer2SeekMode::SEEK_CLOSEST);
    finishPrepareAsync();

    ALOGV("onPrepareAsync: Done");
}

void NuPlayer2::GenericSource2::finishPrepareAsync() {
    ALOGV("finishPrepareAsync");

    if (mIsStreaming) {
        mPreparing = true;
        ++mPollBufferingGeneration;
        schedulePollBuffering();
    } else {
        notifyPrepared();
    }

    if (mAudioTrack.mExtractor != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_AUDIO);
    }

    if (mVideoTrack.mExtractor != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_VIDEO);
    }
}

void NuPlayer2::GenericSource2::notifyPreparedAndCleanup(status_t err) {
    if (err != OK) {
        mDataSourceWrapper.clear();

        mBitrate = -1;
        mPrevBufferPercentage = -1;
        ++mPollBufferingGeneration;
    }
    notifyPrepared(err);
}

void NuPlayer2::GenericSource2::start() {
    Mutex::Autolock _l(mLock);
    ALOGI("start");

    if (mAudioTrack.mExtractor != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_AUDIO);
    }

    if (mVideoTrack.mExtractor != NULL) {
        postReadBuffer(MEDIA_TRACK_TYPE_VIDEO);
    }

    mStarted = true;
}

void NuPlayer2::GenericSource2::stop() {
    Mutex::Autolock _l(mLock);
    mStarted = false;
}

void NuPlayer2::GenericSource2::pause() {
    Mutex::Autolock _l(mLock);
    mStarted = false;
}

void NuPlayer2::GenericSource2::resume() {
    Mutex::Autolock _l(mLock);
    mStarted = true;
}

void NuPlayer2::GenericSource2::disconnect() {
    {
        Mutex::Autolock _l(mLock);
        mDisconnected = true;
    }
    if (mDataSourceWrapper != NULL) {
        mDataSourceWrapper->close();
    }
}

status_t NuPlayer2::GenericSource2::feedMoreTSData() {
    return OK;
}

void NuPlayer2::GenericSource2::onMessageReceived(const sp<AMessage> &msg) {
    Mutex::Autolock _l(mLock);
    switch (msg->what()) {
      case kWhatPrepareAsync:
      {
          int64_t startTimeUs;
          CHECK(msg->findInt64("startTimeUs", &startTimeUs));
          onPrepareAsync(startTimeUs);
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
          const sp<AMediaExtractorWrapper> extractor = mExtractors.itemAt(trackIndex);

          Track* track;
          AString mime;
          media_track_type trackType, counterpartType;
          sp<AMediaFormatWrapper> format = extractor->getTrackFormat(trackIndex);
          format->getString(AMEDIAFORMAT_KEY_MIME, &mime);
          if (!strncasecmp(mime.c_str(), "audio/", 6)) {
              track = &mAudioTrack;
              trackType = MEDIA_TRACK_TYPE_AUDIO;
              counterpartType = MEDIA_TRACK_TYPE_VIDEO;;
          } else {
              CHECK(!strncasecmp(mime.c_str(), "video/", 6));
              track = &mVideoTrack;
              trackType = MEDIA_TRACK_TYPE_VIDEO;
              counterpartType = MEDIA_TRACK_TYPE_AUDIO;;
          }


          track->mExtractor = extractor;
          track->mExtractor->selectSingleTrack(trackIndex);
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
          readBuffer(trackType, timeUs, MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC /* mode */,
                  &actualTimeUs, formatChange);
          readBuffer(counterpartType, -1, MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC /* mode */,
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

void NuPlayer2::GenericSource2::fetchTextData(
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
    readBuffer(type, timeUs, MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC /* mode */, &subTimeUs);

    status_t eosResult;
    if (!packets->hasBufferAvailable(&eosResult)) {
        return;
    }

    if (msg->what() == kWhatFetchSubtitleData) {
        subTimeUs -= 1000000ll;  // send subtile data one second earlier
    }
    sp<AMessage> msg2 = new AMessage(sendWhat, this);
    msg2->setInt32("generation", msgGeneration);
    mMediaClock->addTimer(msg2, subTimeUs);
}

void NuPlayer2::GenericSource2::sendTextData(
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
    readBuffer(type, -1, MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC /* mode */, &nextSubTimeUs);

    sp<ABuffer> buffer;
    status_t dequeueStatus = packets->dequeueAccessUnit(&buffer);
    if (dequeueStatus == OK) {
        sp<AMessage> notify = dupNotify();
        notify->setInt32("what", what);
        notify->setBuffer("buffer", buffer);
        notify->post();

        if (msg->what() == kWhatSendSubtitleData) {
            nextSubTimeUs -= 1000000ll;  // send subtile data one second earlier
        }
        mMediaClock->addTimer(msg, nextSubTimeUs);
    }
}

void NuPlayer2::GenericSource2::sendGlobalTextData(
        uint32_t what,
        int32_t curGen,
        sp<AMessage> msg) {
    int32_t msgGeneration;
    CHECK(msg->findInt32("generation", &msgGeneration));
    if (msgGeneration != curGen) {
        // stale
        return;
    }

    void *data = NULL;
    size_t size = 0;
    if (mTimedTextTrack.mExtractor->getTrackFormat(mTimedTextTrack.mIndex)->getBuffer(
                    "text", &data, &size)) {
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

sp<AMessage> NuPlayer2::GenericSource2::getFormat(bool audio) {
    Mutex::Autolock _l(mLock);
    return getFormat_l(audio);
}

sp<MetaData> NuPlayer2::GenericSource2::getFormatMeta(bool audio) {
    Mutex::Autolock _l(mLock);
    return getFormatMeta_l(audio);
}

sp<AMessage> NuPlayer2::GenericSource2::getFormat_l(bool audio) {
    sp<AMediaExtractorWrapper> extractor = audio ? mAudioTrack.mExtractor : mVideoTrack.mExtractor;
    size_t trackIndex = audio ? mAudioTrack.mIndex : mVideoTrack.mIndex;

    if (extractor == NULL) {
        return NULL;
    }

    return extractor->getTrackFormat(trackIndex)->toAMessage();
}

sp<MetaData> NuPlayer2::GenericSource2::getFormatMeta_l(bool audio) {
    sp<AMediaExtractorWrapper> extractor = audio ? mAudioTrack.mExtractor : mVideoTrack.mExtractor;
    size_t trackIndex = audio ? mAudioTrack.mIndex : mVideoTrack.mIndex;

    if (extractor == NULL) {
        return NULL;
    }

    return convertMediaFormatWrapperToMetaData(extractor->getTrackFormat(trackIndex));
}

status_t NuPlayer2::GenericSource2::dequeueAccessUnit(
        bool audio, sp<ABuffer> *accessUnit) {
    Mutex::Autolock _l(mLock);
    // If has gone through stop/releaseDrm sequence, we no longer send down any buffer b/c
    // the codec's crypto object has gone away (b/37960096).
    // Note: This will be unnecessary when stop() changes behavior and releases codec (b/35248283).
    if (!mStarted && mIsDrmReleased) {
        return -EWOULDBLOCK;
    }

    Track *track = audio ? &mAudioTrack : &mVideoTrack;

    if (track->mExtractor == NULL) {
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
             mBufferingSettings.mResumePlaybackMarkMs * 1000ll / 2;
        if (finalResult == OK) {
            if (durationUs < restartBufferingMarkUs) {
                postReadBuffer(audio? MEDIA_TRACK_TYPE_AUDIO : MEDIA_TRACK_TYPE_VIDEO);
            }
            if (track->mPackets->getAvailableBufferCount(&finalResult) < 2
                && !mSentPauseOnBuffering && !mPreparing) {
                mSentPauseOnBuffering = true;
                sp<AMessage> notify = dupNotify();
                notify->setInt32("what", kWhatPauseOnBufferingStart);
                notify->post();
            }
        }
    }

    if (result != OK) {
        if (mSubtitleTrack.mExtractor != NULL) {
            mSubtitleTrack.mPackets->clear();
            mFetchSubtitleDataGeneration++;
        }
        if (mTimedTextTrack.mExtractor != NULL) {
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

    if (mSubtitleTrack.mExtractor != NULL
            && !mSubtitleTrack.mPackets->hasBufferAvailable(&eosResult)) {
        sp<AMessage> msg = new AMessage(kWhatFetchSubtitleData, this);
        msg->setInt64("timeUs", timeUs);
        msg->setInt32("generation", mFetchSubtitleDataGeneration);
        msg->post();
    }

    if (mTimedTextTrack.mExtractor != NULL
            && !mTimedTextTrack.mPackets->hasBufferAvailable(&eosResult)) {
        sp<AMessage> msg = new AMessage(kWhatFetchTimedTextData, this);
        msg->setInt64("timeUs", timeUs);
        msg->setInt32("generation", mFetchTimedTextDataGeneration);
        msg->post();
    }

    return result;
}

status_t NuPlayer2::GenericSource2::getDuration(int64_t *durationUs) {
    Mutex::Autolock _l(mLock);
    *durationUs = mDurationUs;
    return OK;
}

size_t NuPlayer2::GenericSource2::getTrackCount() const {
    Mutex::Autolock _l(mLock);
    return mExtractors.size();
}

sp<AMessage> NuPlayer2::GenericSource2::getTrackInfo(size_t trackIndex) const {
    Mutex::Autolock _l(mLock);
    size_t trackCount = mExtractors.size();
    if (trackIndex >= trackCount) {
        return NULL;
    }

    sp<AMessage> format = mExtractors.itemAt(trackIndex)->getTrackFormat(trackIndex)->toAMessage();
    if (format == NULL) {
        ALOGE("no metadata for track %zu", trackIndex);
        return NULL;
    }

    AString mime;
    CHECK(format->findString(AMEDIAFORMAT_KEY_MIME, &mime));

    int32_t trackType;
    if (!strncasecmp(mime.c_str(), "video/", 6)) {
        trackType = MEDIA_TRACK_TYPE_VIDEO;
    } else if (!strncasecmp(mime.c_str(), "audio/", 6)) {
        trackType = MEDIA_TRACK_TYPE_AUDIO;
    } else if (!strcasecmp(mime.c_str(), MEDIA_MIMETYPE_TEXT_3GPP)) {
        trackType = MEDIA_TRACK_TYPE_TIMEDTEXT;
    } else {
        trackType = MEDIA_TRACK_TYPE_UNKNOWN;
    }
    format->setInt32("type", trackType);

    AString lang;
    if (!format->findString("language", &lang)) {
        format->setString("language", "und");
    }

    if (trackType == MEDIA_TRACK_TYPE_SUBTITLE) {
        int32_t isAutoselect = 1, isDefault = 0, isForced = 0;
        format->findInt32(AMEDIAFORMAT_KEY_IS_AUTOSELECT, &isAutoselect);
        format->findInt32(AMEDIAFORMAT_KEY_IS_DEFAULT, &isDefault);
        format->findInt32(AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE, &isForced);

        format->setInt32("auto", !!isAutoselect);
        format->setInt32("default", !!isDefault);
        format->setInt32("forced", !!isForced);
    }

    return format;
}

ssize_t NuPlayer2::GenericSource2::getSelectedTrack(media_track_type type) const {
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

    if (track != NULL && track->mExtractor != NULL) {
        return track->mIndex;
    }

    return -1;
}

status_t NuPlayer2::GenericSource2::selectTrack(size_t trackIndex, bool select, int64_t timeUs) {
    Mutex::Autolock _l(mLock);
    ALOGV("%s track: %zu", select ? "select" : "deselect", trackIndex);

    if (trackIndex >= mExtractors.size()) {
        return BAD_INDEX;
    }

    if (!select) {
        Track* track = NULL;
        if (mSubtitleTrack.mExtractor != NULL && trackIndex == mSubtitleTrack.mIndex) {
            track = &mSubtitleTrack;
            mFetchSubtitleDataGeneration++;
        } else if (mTimedTextTrack.mExtractor != NULL && trackIndex == mTimedTextTrack.mIndex) {
            track = &mTimedTextTrack;
            mFetchTimedTextDataGeneration++;
        }
        if (track == NULL) {
            return INVALID_OPERATION;
        }
        track->mExtractor = NULL;
        track->mPackets->clear();
        return OK;
    }

    const sp<AMediaExtractorWrapper> extractor = mExtractors.itemAt(trackIndex);
    sp<MetaData> meta = convertMediaFormatWrapperToMetaData(extractor->getTrackFormat(trackIndex));
    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));
    if (!strncasecmp(mime, "text/", 5)) {
        bool isSubtitle = strcasecmp(mime, MEDIA_MIMETYPE_TEXT_3GPP);
        Track *track = isSubtitle ? &mSubtitleTrack : &mTimedTextTrack;
        if (track->mExtractor != NULL && track->mIndex == trackIndex) {
            return OK;
        }
        track->mIndex = trackIndex;
        track->mExtractor = mExtractors.itemAt(trackIndex);
        track->mExtractor->selectSingleTrack(trackIndex);
        if (track->mPackets == NULL) {
            track->mPackets = new AnotherPacketSource(meta);
        } else {
            track->mPackets->clear();
            track->mPackets->setFormat(meta);

        }

        if (isSubtitle) {
            mFetchSubtitleDataGeneration++;
        } else {
            mFetchTimedTextDataGeneration++;
        }

        status_t eosResult; // ignored
        if (mSubtitleTrack.mExtractor != NULL
                && !mSubtitleTrack.mPackets->hasBufferAvailable(&eosResult)) {
            sp<AMessage> msg = new AMessage(kWhatFetchSubtitleData, this);
            msg->setInt64("timeUs", timeUs);
            msg->setInt32("generation", mFetchSubtitleDataGeneration);
            msg->post();
        }

        sp<AMessage> msg2 = new AMessage(kWhatSendGlobalTimedTextData, this);
        msg2->setInt32("generation", mFetchTimedTextDataGeneration);
        msg2->post();

        if (mTimedTextTrack.mExtractor != NULL
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
        if (track->mExtractor != NULL && track->mIndex == trackIndex) {
            return OK;
        }

        sp<AMessage> msg = new AMessage(kWhatChangeAVSource, this);
        msg->setInt32("trackIndex", trackIndex);
        msg->post();
        return OK;
    }

    return INVALID_OPERATION;
}

status_t NuPlayer2::GenericSource2::seekTo(int64_t seekTimeUs, MediaPlayer2SeekMode mode) {
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

void NuPlayer2::GenericSource2::onSeek(const sp<AMessage>& msg) {
    int64_t seekTimeUs;
    int32_t mode;
    CHECK(msg->findInt64("seekTimeUs", &seekTimeUs));
    CHECK(msg->findInt32("mode", &mode));

    sp<AMessage> response = new AMessage;
    status_t err = doSeek(seekTimeUs, (MediaPlayer2SeekMode)mode);
    response->setInt32("err", err);

    sp<AReplyToken> replyID;
    CHECK(msg->senderAwaitsResponse(&replyID));
    response->postReply(replyID);
}

status_t NuPlayer2::GenericSource2::doSeek(int64_t seekTimeUs, MediaPlayer2SeekMode mode) {
    if (mVideoTrack.mExtractor != NULL) {
        ++mVideoDataGeneration;

        int64_t actualTimeUs;
        readBuffer(MEDIA_TRACK_TYPE_VIDEO, seekTimeUs, mode, &actualTimeUs);

        if (mode != MediaPlayer2SeekMode::SEEK_CLOSEST) {
            seekTimeUs = actualTimeUs;
        }
        mVideoLastDequeueTimeUs = actualTimeUs;
    }

    if (mAudioTrack.mExtractor != NULL) {
        ++mAudioDataGeneration;
        readBuffer(MEDIA_TRACK_TYPE_AUDIO, seekTimeUs, MediaPlayer2SeekMode::SEEK_CLOSEST);
        mAudioLastDequeueTimeUs = seekTimeUs;
    }

    if (mSubtitleTrack.mExtractor != NULL) {
        mSubtitleTrack.mPackets->clear();
        mFetchSubtitleDataGeneration++;
    }

    if (mTimedTextTrack.mExtractor != NULL) {
        mTimedTextTrack.mPackets->clear();
        mFetchTimedTextDataGeneration++;
    }

    ++mPollBufferingGeneration;
    schedulePollBuffering();
    return OK;
}

sp<ABuffer> NuPlayer2::GenericSource2::mediaBufferToABuffer(
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
        // setMediaBufferBase() interestingly doesn't increment the ref count on its own.
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
        AString mime;
        sp<AMediaExtractorWrapper> extractor = mTimedTextTrack.mExtractor;
        size_t trackIndex = mTimedTextTrack.mIndex;
        CHECK(extractor != NULL
                && extractor->getTrackFormat(trackIndex)->getString(AMEDIAFORMAT_KEY_MIME, &mime));
        meta->setString("mime", mime.c_str());
    }

    int64_t durationUs;
    if (mb->meta_data().findInt64(kKeyDuration, &durationUs)) {
        meta->setInt64("durationUs", durationUs);
    }

    if (trackType == MEDIA_TRACK_TYPE_SUBTITLE) {
        meta->setInt32(AMEDIAFORMAT_KEY_TRACK_INDEX, mSubtitleTrack.mIndex);
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
        meta->setBuffer(AMEDIAFORMAT_KEY_MPEG_USER_DATA, mpegUserData);
    }

    mb->release();
    mb = NULL;

    return ab;
}

int32_t NuPlayer2::GenericSource2::getDataGeneration(media_track_type type) const {
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

void NuPlayer2::GenericSource2::postReadBuffer(media_track_type trackType) {
    if ((mPendingReadBufferTypes & (1 << trackType)) == 0) {
        mPendingReadBufferTypes |= (1 << trackType);
        sp<AMessage> msg = new AMessage(kWhatReadBuffer, this);
        msg->setInt32("trackType", trackType);
        msg->post();
    }
}

void NuPlayer2::GenericSource2::onReadBuffer(const sp<AMessage>& msg) {
    int32_t tmpType;
    CHECK(msg->findInt32("trackType", &tmpType));
    media_track_type trackType = (media_track_type)tmpType;
    mPendingReadBufferTypes &= ~(1 << trackType);
    readBuffer(trackType);
}

void NuPlayer2::GenericSource2::readBuffer(
        media_track_type trackType, int64_t seekTimeUs, MediaPlayer2SeekMode mode,
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

    if (track->mExtractor == NULL) {
        return;
    }

    if (actualTimeUs) {
        *actualTimeUs = seekTimeUs;
    }


    bool seeking = false;
    sp<AMediaExtractorWrapper> extractor = track->mExtractor;
    if (seekTimeUs >= 0) {
        extractor->seekTo(seekTimeUs, mode);
        seeking = true;
    }

    int32_t generation = getDataGeneration(trackType);
    for (size_t numBuffers = 0; numBuffers < maxBuffers; ) {
        Vector<sp<ABuffer> > aBuffers;

        mLock.unlock();

        sp<AMediaFormatWrapper> format;
        ssize_t sampleSize = -1;
        status_t err = extractor->getSampleFormat(format);
        if (err == OK) {
            sampleSize = extractor->getSampleSize();
        }

        if (err != OK || sampleSize < 0) {
            mLock.lock();
            track->mPackets->signalEOS(err != OK ? err : ERROR_END_OF_STREAM);
            break;
        }

        sp<ABuffer> abuf = new ABuffer(sampleSize);
        sampleSize = extractor->readSampleData(abuf);
        mLock.lock();

        // in case track has been changed since we don't have lock for some time.
        if (generation != getDataGeneration(trackType)) {
            break;
        }

        int64_t timeUs = extractor->getSampleTime();
        if (timeUs < 0) {
            track->mPackets->signalEOS(ERROR_MALFORMED);
            break;
        }

        sp<AMessage> meta = abuf->meta();
        format->writeToAMessage(meta);
        meta->setInt64("timeUs", timeUs);
        if (trackType == MEDIA_TRACK_TYPE_AUDIO) {
            mAudioTimeUs = timeUs;
        } else if (trackType == MEDIA_TRACK_TYPE_VIDEO) {
            mVideoTimeUs = timeUs;
        }

        sp<AMediaCodecCryptoInfoWrapper> cryptInfo = extractor->getSampleCryptoInfo();
        if (cryptInfo != NULL) {
            meta->setObject("cryptInfo", cryptInfo);
        }

        queueDiscontinuityIfNeeded(seeking, formatChange, trackType, track);

        if (numBuffers == 0 && actualTimeUs != nullptr) {
            *actualTimeUs = timeUs;
        }
        if (seeking) {
            if (meta != nullptr && mode == MediaPlayer2SeekMode::SEEK_CLOSEST
                    && seekTimeUs > timeUs) {
                sp<AMessage> extra = new AMessage;
                extra->setInt64("resume-at-mediaTimeUs", seekTimeUs);
                meta->setMessage("extra", extra);
            }
        }

        track->mPackets->queueAccessUnit(abuf);
        formatChange = false;
        seeking = false;
        ++numBuffers;
        extractor->advance();

    }

    if (mIsStreaming
        && (trackType == MEDIA_TRACK_TYPE_VIDEO || trackType == MEDIA_TRACK_TYPE_AUDIO)) {
        status_t finalResult;
        int64_t durationUs = track->mPackets->getBufferedDurationUs(&finalResult);

        // TODO: maxRebufferingMarkMs could be larger than
        // mBufferingSettings.mResumePlaybackMarkMs
        int64_t markUs = (mPreparing ? mBufferingSettings.mInitialMarkMs
            : mBufferingSettings.mResumePlaybackMarkMs) * 1000ll;
        if (finalResult == ERROR_END_OF_STREAM || durationUs >= markUs) {
            if (mPreparing || mSentPauseOnBuffering) {
                Track *counterTrack =
                    (trackType == MEDIA_TRACK_TYPE_VIDEO ? &mAudioTrack : &mVideoTrack);
                if (counterTrack->mExtractor != NULL) {
                    durationUs = counterTrack->mPackets->getBufferedDurationUs(&finalResult);
                }
                if (finalResult == ERROR_END_OF_STREAM || durationUs >= markUs) {
                    if (mPreparing) {
                        notifyPrepared();
                        mPreparing = false;
                    } else {
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

void NuPlayer2::GenericSource2::queueDiscontinuityIfNeeded(
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

void NuPlayer2::GenericSource2::notifyBufferingUpdate(int32_t percentage) {
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

void NuPlayer2::GenericSource2::schedulePollBuffering() {
    if (mIsStreaming) {
        sp<AMessage> msg = new AMessage(kWhatPollBuffering, this);
        msg->setInt32("generation", mPollBufferingGeneration);
        // Enquires buffering status every second.
        msg->post(1000000ll);
    }
}

void NuPlayer2::GenericSource2::onPollBuffering() {
    int64_t cachedDurationUs = -1ll;

    sp<AMediaExtractorWrapper> extractor;
    if (mVideoTrack.mExtractor != NULL) {
        extractor = mVideoTrack.mExtractor;
    } else if (mAudioTrack.mExtractor != NULL) {
        extractor = mAudioTrack.mExtractor;
    }

    if (extractor != NULL) {
        cachedDurationUs = extractor->getCachedDuration();
    }

    if (cachedDurationUs >= 0ll) {
        ssize_t sampleSize = extractor->getSampleSize();
        if (sampleSize >= 0ll) {
            int64_t cachedPosUs = getLastReadPosition() + cachedDurationUs;
            int percentage = 100.0 * cachedPosUs / mDurationUs;
            if (percentage > 100) {
                percentage = 100;
            }

            notifyBufferingUpdate(percentage);
            ALOGV("onPollBuffering: cachedDurationUs %.1f sec", cachedDurationUs / 1000000.0f);
        } else {
            notifyBufferingUpdate(100);
            ALOGV("onPollBuffering: EOS");
        }
    }

    schedulePollBuffering();
}

// Modular DRM
status_t NuPlayer2::GenericSource2::prepareDrm(
        const uint8_t uuid[16],
        const Vector<uint8_t> &drmSessionId,
        sp<AMediaCryptoWrapper> *outCrypto) {
    Mutex::Autolock _l(mLock);
    ALOGV("prepareDrm");

    mIsDrmProtected = false;
    mIsDrmReleased = false;
    mIsSecure = false;

    status_t status = OK;
    sp<AMediaCryptoWrapper> crypto =
        new AMediaCryptoWrapper(uuid, drmSessionId.array(), drmSessionId.size());
    if (crypto == NULL) {
        ALOGE("prepareDrm: failed to create crypto.");
        return UNKNOWN_ERROR;
    }
    ALOGV("prepareDrm: crypto created for uuid: %s",
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
        ALOGV("prepareDrm: mCrypto: %p", outCrypto->get());
        ALOGD("prepareDrm ret: %d ", status);
    } else {
        ALOGE("prepareDrm err: %d", status);
    }
    return status;
}

status_t NuPlayer2::GenericSource2::releaseDrm() {
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

status_t NuPlayer2::GenericSource2::checkDrmInfo()
{
    // clearing the flag at prepare in case the player is reused after stop/releaseDrm with the
    // same source without being reset (called by prepareAsync/initFromDataSource)
    mIsDrmReleased = false;

    if (mExtractor == NULL) {
        ALOGV("checkDrmInfo: No extractor");
        return OK; // letting the caller responds accordingly
    }

    PsshInfo *psshInfo = mExtractor->getPsshInfo();
    if (psshInfo == NULL) {
        ALOGV("checkDrmInfo: No PSSH");
        return OK; // source without DRM info
    }

    PlayerMessage playerMsg;
    status_t ret = NuPlayer2Drm::retrieveDrmInfo(psshInfo, &playerMsg);
    ALOGV("checkDrmInfo: MEDIA_DRM_INFO PSSH drm info size: %d", (int)playerMsg.ByteSize());

    if (ret != OK) {
        ALOGE("checkDrmInfo: failed to retrive DrmInfo %d", ret);
        return UNKNOWN_ERROR;
    }

    int size = playerMsg.ByteSize();
    sp<ABuffer> drmInfoBuf = new ABuffer(size);
    playerMsg.SerializeToArray(drmInfoBuf->data(), size);
    drmInfoBuf->setRange(0, size);
    notifyDrmInfo(drmInfoBuf);

    return OK;
}

void NuPlayer2::GenericSource2::signalBufferReturned(MediaBufferBase *buffer)
{
    //ALOGV("signalBufferReturned %p  refCount: %d", buffer, buffer->localRefcount());

    buffer->setObserver(NULL);
    buffer->release(); // this leads to delete since that there is no observor
}

}  // namespace android
