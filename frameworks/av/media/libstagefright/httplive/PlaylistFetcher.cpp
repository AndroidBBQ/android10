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
#define LOG_TAG "PlaylistFetcher"
#include <android-base/macros.h>
#include <utils/Log.h>
#include <utils/misc.h>

#include "PlaylistFetcher.h"
#include "HTTPDownloader.h"
#include "LiveSession.h"
#include "M3UParser.h"
#include "include/ID3.h"
#include "mpeg2ts/AnotherPacketSource.h"
#include "mpeg2ts/HlsSampleDecryptor.h"

#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/MediaKeys.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/DataURISource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MetaDataUtils.h>
#include <media/stagefright/Utils.h>

#include <ctype.h>
#include <inttypes.h>

#define FLOGV(fmt, ...) ALOGV("[fetcher-%d] " fmt, mFetcherID, ##__VA_ARGS__)
#define FSLOGV(stream, fmt, ...) ALOGV("[fetcher-%d] [%s] " fmt, mFetcherID, \
         LiveSession::getNameForStream(stream), ##__VA_ARGS__)

namespace android {

// static
const int64_t PlaylistFetcher::kMinBufferedDurationUs = 30000000LL;
const int64_t PlaylistFetcher::kMaxMonitorDelayUs = 3000000LL;
// LCM of 188 (size of a TS packet) & 1k works well
const int32_t PlaylistFetcher::kDownloadBlockSize = 47 * 1024;

struct PlaylistFetcher::DownloadState : public RefBase {
    DownloadState();
    void resetState();
    bool hasSavedState() const;
    void restoreState(
            AString &uri,
            sp<AMessage> &itemMeta,
            sp<ABuffer> &buffer,
            sp<ABuffer> &tsBuffer,
            int32_t &firstSeqNumberInPlaylist,
            int32_t &lastSeqNumberInPlaylist);
    void saveState(
            AString &uri,
            sp<AMessage> &itemMeta,
            sp<ABuffer> &buffer,
            sp<ABuffer> &tsBuffer,
            int32_t &firstSeqNumberInPlaylist,
            int32_t &lastSeqNumberInPlaylist);

private:
    bool mHasSavedState;
    AString mUri;
    sp<AMessage> mItemMeta;
    sp<ABuffer> mBuffer;
    sp<ABuffer> mTsBuffer;
    int32_t mFirstSeqNumberInPlaylist;
    int32_t mLastSeqNumberInPlaylist;
};

PlaylistFetcher::DownloadState::DownloadState() {
    resetState();
}

bool PlaylistFetcher::DownloadState::hasSavedState() const {
    return mHasSavedState;
}

void PlaylistFetcher::DownloadState::resetState() {
    mHasSavedState = false;

    mUri.clear();
    mItemMeta = NULL;
    mBuffer = NULL;
    mTsBuffer = NULL;
    mFirstSeqNumberInPlaylist = 0;
    mLastSeqNumberInPlaylist = 0;
}

void PlaylistFetcher::DownloadState::restoreState(
        AString &uri,
        sp<AMessage> &itemMeta,
        sp<ABuffer> &buffer,
        sp<ABuffer> &tsBuffer,
        int32_t &firstSeqNumberInPlaylist,
        int32_t &lastSeqNumberInPlaylist) {
    if (!mHasSavedState) {
        return;
    }

    uri = mUri;
    itemMeta = mItemMeta;
    buffer = mBuffer;
    tsBuffer = mTsBuffer;
    firstSeqNumberInPlaylist = mFirstSeqNumberInPlaylist;
    lastSeqNumberInPlaylist = mLastSeqNumberInPlaylist;

    resetState();
}

void PlaylistFetcher::DownloadState::saveState(
        AString &uri,
        sp<AMessage> &itemMeta,
        sp<ABuffer> &buffer,
        sp<ABuffer> &tsBuffer,
        int32_t &firstSeqNumberInPlaylist,
        int32_t &lastSeqNumberInPlaylist) {
    mHasSavedState = true;

    mUri = uri;
    mItemMeta = itemMeta;
    mBuffer = buffer;
    mTsBuffer = tsBuffer;
    mFirstSeqNumberInPlaylist = firstSeqNumberInPlaylist;
    mLastSeqNumberInPlaylist = lastSeqNumberInPlaylist;
}

PlaylistFetcher::PlaylistFetcher(
        const sp<AMessage> &notify,
        const sp<LiveSession> &session,
        const char *uri,
        int32_t id,
        int32_t subtitleGeneration)
    : mNotify(notify),
      mSession(session),
      mURI(uri),
      mFetcherID(id),
      mStreamTypeMask(0),
      mStartTimeUs(-1LL),
      mSegmentStartTimeUs(-1LL),
      mDiscontinuitySeq(-1LL),
      mStartTimeUsRelative(false),
      mLastPlaylistFetchTimeUs(-1LL),
      mPlaylistTimeUs(-1LL),
      mSeqNumber(-1),
      mNumRetries(0),
      mNumRetriesForMonitorQueue(0),
      mStartup(true),
      mIDRFound(false),
      mSeekMode(LiveSession::kSeekModeExactPosition),
      mTimeChangeSignaled(false),
      mNextPTSTimeUs(-1LL),
      mMonitorQueueGeneration(0),
      mSubtitleGeneration(subtitleGeneration),
      mLastDiscontinuitySeq(-1LL),
      mRefreshState(INITIAL_MINIMUM_RELOAD_DELAY),
      mFirstPTSValid(false),
      mFirstTimeUs(-1LL),
      mVideoBuffer(new AnotherPacketSource(NULL)),
      mSampleAesKeyItemChanged(false),
      mThresholdRatio(-1.0f),
      mDownloadState(new DownloadState()),
      mHasMetadata(false) {
    memset(mPlaylistHash, 0, sizeof(mPlaylistHash));
    mHTTPDownloader = mSession->getHTTPDownloader();

    memset(mKeyData, 0, sizeof(mKeyData));
    memset(mAESInitVec, 0, sizeof(mAESInitVec));
}

PlaylistFetcher::~PlaylistFetcher() {
}

int32_t PlaylistFetcher::getFetcherID() const {
    return mFetcherID;
}

int64_t PlaylistFetcher::getSegmentStartTimeUs(int32_t seqNumber) const {
    CHECK(mPlaylist != NULL);

    int32_t firstSeqNumberInPlaylist, lastSeqNumberInPlaylist;
    mPlaylist->getSeqNumberRange(
            &firstSeqNumberInPlaylist, &lastSeqNumberInPlaylist);

    CHECK_GE(seqNumber, firstSeqNumberInPlaylist);
    CHECK_LE(seqNumber, lastSeqNumberInPlaylist);

    int64_t segmentStartUs = 0LL;
    for (int32_t index = 0;
            index < seqNumber - firstSeqNumberInPlaylist; ++index) {
        sp<AMessage> itemMeta;
        CHECK(mPlaylist->itemAt(
                    index, NULL /* uri */, &itemMeta));

        int64_t itemDurationUs;
        CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

        segmentStartUs += itemDurationUs;
    }

    return segmentStartUs;
}

int64_t PlaylistFetcher::getSegmentDurationUs(int32_t seqNumber) const {
    CHECK(mPlaylist != NULL);

    int32_t firstSeqNumberInPlaylist, lastSeqNumberInPlaylist;
    mPlaylist->getSeqNumberRange(
            &firstSeqNumberInPlaylist, &lastSeqNumberInPlaylist);

    CHECK_GE(seqNumber, firstSeqNumberInPlaylist);
    CHECK_LE(seqNumber, lastSeqNumberInPlaylist);

    int32_t index = seqNumber - firstSeqNumberInPlaylist;
    sp<AMessage> itemMeta;
    CHECK(mPlaylist->itemAt(
                index, NULL /* uri */, &itemMeta));

    int64_t itemDurationUs;
    CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

    return itemDurationUs;
}

int64_t PlaylistFetcher::delayUsToRefreshPlaylist() const {
    int64_t nowUs = ALooper::GetNowUs();

    if (mPlaylist == NULL || mLastPlaylistFetchTimeUs < 0LL) {
        CHECK_EQ((int)mRefreshState, (int)INITIAL_MINIMUM_RELOAD_DELAY);
        return 0LL;
    }

    if (mPlaylist->isComplete()) {
        return (~0LLU >> 1);
    }

    int64_t targetDurationUs = mPlaylist->getTargetDuration();

    int64_t minPlaylistAgeUs;

    switch (mRefreshState) {
        case INITIAL_MINIMUM_RELOAD_DELAY:
        {
            size_t n = mPlaylist->size();
            if (n > 0) {
                sp<AMessage> itemMeta;
                CHECK(mPlaylist->itemAt(n - 1, NULL /* uri */, &itemMeta));

                int64_t itemDurationUs;
                CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

                minPlaylistAgeUs = itemDurationUs;
                break;
            }

            FALLTHROUGH_INTENDED;
        }

        case FIRST_UNCHANGED_RELOAD_ATTEMPT:
        {
            minPlaylistAgeUs = targetDurationUs / 2;
            break;
        }

        case SECOND_UNCHANGED_RELOAD_ATTEMPT:
        {
            minPlaylistAgeUs = (targetDurationUs * 3) / 2;
            break;
        }

        case THIRD_UNCHANGED_RELOAD_ATTEMPT:
        {
            minPlaylistAgeUs = targetDurationUs * 3;
            break;
        }

        default:
            TRESPASS();
            break;
    }

    int64_t delayUs = mLastPlaylistFetchTimeUs + minPlaylistAgeUs - nowUs;
    return delayUs > 0LL ? delayUs : 0LL;
}

status_t PlaylistFetcher::decryptBuffer(
        size_t playlistIndex, const sp<ABuffer> &buffer,
        bool first) {
    sp<AMessage> itemMeta;
    bool found = false;
    AString method;

    for (ssize_t i = playlistIndex; i >= 0; --i) {
        AString uri;
        CHECK(mPlaylist->itemAt(i, &uri, &itemMeta));

        if (itemMeta->findString("cipher-method", &method)) {
            found = true;
            break;
        }
    }

    // TODO: Revise this when we add support for KEYFORMAT
    // If method has changed (e.g., -> NONE); sufficient to check at the segment boundary
    if (mSampleAesKeyItem != NULL && first && found && method != "SAMPLE-AES") {
        ALOGI("decryptBuffer: resetting mSampleAesKeyItem(%p) with method %s",
                mSampleAesKeyItem.get(), method.c_str());
        mSampleAesKeyItem = NULL;
        mSampleAesKeyItemChanged = true;
    }

    if (!found) {
        method = "NONE";
    }
    buffer->meta()->setString("cipher-method", method.c_str());

    if (method == "NONE") {
        return OK;
    } else if (method == "SAMPLE-AES") {
        ALOGV("decryptBuffer: Non-Widevine SAMPLE-AES is supported now.");
    } else if (!(method == "AES-128")) {
        ALOGE("Unsupported cipher method '%s'", method.c_str());
        return ERROR_UNSUPPORTED;
    }

    AString keyURI;
    if (!itemMeta->findString("cipher-uri", &keyURI)) {
        ALOGE("Missing key uri");
        return ERROR_MALFORMED;
    }

    ssize_t index = mAESKeyForURI.indexOfKey(keyURI);

    sp<ABuffer> key;
    if (index >= 0) {
        key = mAESKeyForURI.valueAt(index);
    } else if (keyURI.startsWith("data:")) {
        sp<DataSource> keySrc = DataURISource::Create(keyURI.c_str());
        off64_t keyLen;
        if (keySrc == NULL || keySrc->getSize(&keyLen) != OK || keyLen < 0) {
            ALOGE("Malformed cipher key data uri.");
            return ERROR_MALFORMED;
        }
        key = new ABuffer(keyLen);
        keySrc->readAt(0, key->data(), keyLen);
        key->setRange(0, keyLen);
    } else {
        ssize_t err = mHTTPDownloader->fetchFile(keyURI.c_str(), &key);

        if (err == ERROR_NOT_CONNECTED) {
            return ERROR_NOT_CONNECTED;
        } else if (err < 0) {
            ALOGE("failed to fetch cipher key from '%s'.", uriDebugString(keyURI).c_str());
            return ERROR_IO;
        } else if (key->size() != 16) {
            ALOGE("key file '%s' wasn't 16 bytes in size.", uriDebugString(keyURI).c_str());
            return ERROR_MALFORMED;
        }

        mAESKeyForURI.add(keyURI, key);
    }

    if (first) {
        // If decrypting the first block in a file, read the iv from the manifest
        // or derive the iv from the file's sequence number.

        unsigned char AESInitVec[AES_BLOCK_SIZE];
        AString iv;
        if (itemMeta->findString("cipher-iv", &iv)) {
            if ((!iv.startsWith("0x") && !iv.startsWith("0X"))
                    || iv.size() > 16 * 2 + 2) {
                ALOGE("malformed cipher IV '%s'.", iv.c_str());
                return ERROR_MALFORMED;
            }

            while (iv.size() < 16 * 2 + 2) {
                iv.insert("0", 1, 2);
            }

            memset(AESInitVec, 0, sizeof(AESInitVec));
            for (size_t i = 0; i < 16; ++i) {
                char c1 = tolower(iv.c_str()[2 + 2 * i]);
                char c2 = tolower(iv.c_str()[3 + 2 * i]);
                if (!isxdigit(c1) || !isxdigit(c2)) {
                    ALOGE("malformed cipher IV '%s'.", iv.c_str());
                    return ERROR_MALFORMED;
                }
                uint8_t nibble1 = isdigit(c1) ? c1 - '0' : c1 - 'a' + 10;
                uint8_t nibble2 = isdigit(c2) ? c2 - '0' : c2 - 'a' + 10;

                AESInitVec[i] = nibble1 << 4 | nibble2;
            }
        } else {
            memset(AESInitVec, 0, sizeof(AESInitVec));
            AESInitVec[15] = mSeqNumber & 0xff;
            AESInitVec[14] = (mSeqNumber >> 8) & 0xff;
            AESInitVec[13] = (mSeqNumber >> 16) & 0xff;
            AESInitVec[12] = (mSeqNumber >> 24) & 0xff;
        }

        bool newKey = memcmp(mKeyData, key->data(), AES_BLOCK_SIZE) != 0;
        bool newInitVec = memcmp(mAESInitVec, AESInitVec, AES_BLOCK_SIZE) != 0;
        bool newSampleAesKeyItem = newKey || newInitVec;
        ALOGV("decryptBuffer: SAMPLE-AES newKeyItem %d/%d (Key %d initVec %d)",
                mSampleAesKeyItemChanged, newSampleAesKeyItem, newKey, newInitVec);

        if (newSampleAesKeyItem) {
            memcpy(mKeyData, key->data(), AES_BLOCK_SIZE);
            memcpy(mAESInitVec, AESInitVec, AES_BLOCK_SIZE);

            if (method == "SAMPLE-AES") {
                mSampleAesKeyItemChanged = true;

                sp<ABuffer> keyDataBuffer = ABuffer::CreateAsCopy(mKeyData, sizeof(mKeyData));
                sp<ABuffer> initVecBuffer = ABuffer::CreateAsCopy(mAESInitVec, sizeof(mAESInitVec));

                // always allocating a new one rather than updating the old message
                // lower layer might still have a reference to the old message
                mSampleAesKeyItem = new AMessage();
                mSampleAesKeyItem->setBuffer("keyData", keyDataBuffer);
                mSampleAesKeyItem->setBuffer("initVec", initVecBuffer);

                ALOGV("decryptBuffer: New SampleAesKeyItem: Key: %s  IV: %s",
                        HlsSampleDecryptor::aesBlockToStr(mKeyData).c_str(),
                        HlsSampleDecryptor::aesBlockToStr(mAESInitVec).c_str());
            } // SAMPLE-AES
        } // newSampleAesKeyItem
    } // first

    if (method == "SAMPLE-AES") {
        ALOGV("decryptBuffer: skipping full-seg decrypt for SAMPLE-AES");
        return OK;
    }


    AES_KEY aes_key;
    if (AES_set_decrypt_key(key->data(), 128, &aes_key) != 0) {
        ALOGE("failed to set AES decryption key.");
        return UNKNOWN_ERROR;
    }

    size_t n = buffer->size();
    if (!n) {
        return OK;
    }

    if (n < 16 || n % 16) {
        ALOGE("not enough or trailing bytes (%zu) in encrypted buffer", n);
        return ERROR_MALFORMED;
    }

    AES_cbc_encrypt(
            buffer->data(), buffer->data(), buffer->size(),
            &aes_key, mAESInitVec, AES_DECRYPT);

    return OK;
}

status_t PlaylistFetcher::checkDecryptPadding(const sp<ABuffer> &buffer) {
    AString method;
    CHECK(buffer->meta()->findString("cipher-method", &method));
    if (method == "NONE" || method == "SAMPLE-AES") {
        return OK;
    }

    uint8_t padding = 0;
    if (buffer->size() > 0) {
        padding = buffer->data()[buffer->size() - 1];
    }

    if (padding > 16) {
        return ERROR_MALFORMED;
    }

    for (size_t i = buffer->size() - padding; i < padding; i++) {
        if (buffer->data()[i] != padding) {
            return ERROR_MALFORMED;
        }
    }

    buffer->setRange(buffer->offset(), buffer->size() - padding);
    return OK;
}

void PlaylistFetcher::postMonitorQueue(int64_t delayUs, int64_t minDelayUs) {
    int64_t maxDelayUs = delayUsToRefreshPlaylist();
    if (maxDelayUs < minDelayUs) {
        maxDelayUs = minDelayUs;
    }
    if (delayUs > maxDelayUs) {
        FLOGV("Need to refresh playlist in %lld", (long long)maxDelayUs);
        delayUs = maxDelayUs;
    }
    sp<AMessage> msg = new AMessage(kWhatMonitorQueue, this);
    msg->setInt32("generation", mMonitorQueueGeneration);
    msg->post(delayUs);
}

void PlaylistFetcher::cancelMonitorQueue() {
    ++mMonitorQueueGeneration;
}

void PlaylistFetcher::setStoppingThreshold(float thresholdRatio, bool disconnect) {
    {
        AutoMutex _l(mThresholdLock);
        mThresholdRatio = thresholdRatio;
    }
    if (disconnect) {
        mHTTPDownloader->disconnect();
    }
}

void PlaylistFetcher::resetStoppingThreshold(bool disconnect) {
    {
        AutoMutex _l(mThresholdLock);
        mThresholdRatio = -1.0f;
    }
    if (disconnect) {
        mHTTPDownloader->disconnect();
    } else {
        // allow reconnect
        mHTTPDownloader->reconnect();
    }
}

float PlaylistFetcher::getStoppingThreshold() {
    AutoMutex _l(mThresholdLock);
    return mThresholdRatio;
}

void PlaylistFetcher::startAsync(
        const sp<AnotherPacketSource> &audioSource,
        const sp<AnotherPacketSource> &videoSource,
        const sp<AnotherPacketSource> &subtitleSource,
        const sp<AnotherPacketSource> &metadataSource,
        int64_t startTimeUs,
        int64_t segmentStartTimeUs,
        int32_t startDiscontinuitySeq,
        LiveSession::SeekMode seekMode) {
    sp<AMessage> msg = new AMessage(kWhatStart, this);

    uint32_t streamTypeMask = 0ul;

    if (audioSource != NULL) {
        msg->setPointer("audioSource", audioSource.get());
        streamTypeMask |= LiveSession::STREAMTYPE_AUDIO;
    }

    if (videoSource != NULL) {
        msg->setPointer("videoSource", videoSource.get());
        streamTypeMask |= LiveSession::STREAMTYPE_VIDEO;
    }

    if (subtitleSource != NULL) {
        msg->setPointer("subtitleSource", subtitleSource.get());
        streamTypeMask |= LiveSession::STREAMTYPE_SUBTITLES;
    }

    if (metadataSource != NULL) {
        msg->setPointer("metadataSource", metadataSource.get());
        // metadataSource does not affect streamTypeMask.
    }

    msg->setInt32("streamTypeMask", streamTypeMask);
    msg->setInt64("startTimeUs", startTimeUs);
    msg->setInt64("segmentStartTimeUs", segmentStartTimeUs);
    msg->setInt32("startDiscontinuitySeq", startDiscontinuitySeq);
    msg->setInt32("seekMode", seekMode);
    msg->post();
}

/*
 * pauseAsync
 *
 * threshold: 0.0f - pause after current fetch block (default 47Kbytes)
 *           -1.0f - pause after finishing current segment
 *        0.0~1.0f - pause if remaining of current segment exceeds threshold
 */
void PlaylistFetcher::pauseAsync(
        float thresholdRatio, bool disconnect) {
    setStoppingThreshold(thresholdRatio, disconnect);

    (new AMessage(kWhatPause, this))->post();
}

void PlaylistFetcher::stopAsync(bool clear) {
    setStoppingThreshold(0.0f, true /* disconncect */);

    sp<AMessage> msg = new AMessage(kWhatStop, this);
    msg->setInt32("clear", clear);
    msg->post();
}

void PlaylistFetcher::resumeUntilAsync(const sp<AMessage> &params) {
    FLOGV("resumeUntilAsync: params=%s", params->debugString().c_str());

    AMessage* msg = new AMessage(kWhatResumeUntil, this);
    msg->setMessage("params", params);
    msg->post();
}

void PlaylistFetcher::fetchPlaylistAsync() {
    (new AMessage(kWhatFetchPlaylist, this))->post();
}

void PlaylistFetcher::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            status_t err = onStart(msg);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatStarted);
            notify->setInt32("err", err);
            notify->post();
            break;
        }

        case kWhatPause:
        {
            onPause();

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatPaused);
            notify->setInt32("seekMode",
                    mDownloadState->hasSavedState()
                    ? LiveSession::kSeekModeNextSample
                    : LiveSession::kSeekModeNextSegment);
            notify->post();
            break;
        }

        case kWhatStop:
        {
            onStop(msg);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatStopped);
            notify->post();
            break;
        }

        case kWhatFetchPlaylist:
        {
            bool unchanged;
            sp<M3UParser> playlist = mHTTPDownloader->fetchPlaylist(
                    mURI.c_str(), NULL /* curPlaylistHash */, &unchanged);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatPlaylistFetched);
            notify->setObject("playlist", playlist);
            notify->post();
            break;
        }

        case kWhatMonitorQueue:
        case kWhatDownloadNext:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            if (generation != mMonitorQueueGeneration) {
                // Stale event
                break;
            }

            if (msg->what() == kWhatMonitorQueue) {
                onMonitorQueue();
            } else {
                onDownloadNext();
            }
            break;
        }

        case kWhatResumeUntil:
        {
            onResumeUntil(msg);
            break;
        }

        default:
            TRESPASS();
    }
}

status_t PlaylistFetcher::onStart(const sp<AMessage> &msg) {
    mPacketSources.clear();
    mStopParams.clear();
    mStartTimeUsNotify = mNotify->dup();
    mStartTimeUsNotify->setInt32("what", kWhatStartedAt);
    mStartTimeUsNotify->setString("uri", mURI);

    uint32_t streamTypeMask;
    CHECK(msg->findInt32("streamTypeMask", (int32_t *)&streamTypeMask));

    int64_t startTimeUs;
    int64_t segmentStartTimeUs;
    int32_t startDiscontinuitySeq;
    int32_t seekMode;
    CHECK(msg->findInt64("startTimeUs", &startTimeUs));
    CHECK(msg->findInt64("segmentStartTimeUs", &segmentStartTimeUs));
    CHECK(msg->findInt32("startDiscontinuitySeq", &startDiscontinuitySeq));
    CHECK(msg->findInt32("seekMode", &seekMode));

    if (streamTypeMask & LiveSession::STREAMTYPE_AUDIO) {
        void *ptr;
        CHECK(msg->findPointer("audioSource", &ptr));

        mPacketSources.add(
                LiveSession::STREAMTYPE_AUDIO,
                static_cast<AnotherPacketSource *>(ptr));
    }

    if (streamTypeMask & LiveSession::STREAMTYPE_VIDEO) {
        void *ptr;
        CHECK(msg->findPointer("videoSource", &ptr));

        mPacketSources.add(
                LiveSession::STREAMTYPE_VIDEO,
                static_cast<AnotherPacketSource *>(ptr));
    }

    if (streamTypeMask & LiveSession::STREAMTYPE_SUBTITLES) {
        void *ptr;
        CHECK(msg->findPointer("subtitleSource", &ptr));

        mPacketSources.add(
                LiveSession::STREAMTYPE_SUBTITLES,
                static_cast<AnotherPacketSource *>(ptr));
    }

    void *ptr;
    // metadataSource is not part of streamTypeMask
    if ((streamTypeMask & (LiveSession::STREAMTYPE_AUDIO | LiveSession::STREAMTYPE_VIDEO))
            && msg->findPointer("metadataSource", &ptr)) {
        mPacketSources.add(
                LiveSession::STREAMTYPE_METADATA,
                static_cast<AnotherPacketSource *>(ptr));
    }

    mStreamTypeMask = streamTypeMask;

    mSegmentStartTimeUs = segmentStartTimeUs;

    if (startDiscontinuitySeq >= 0) {
        mDiscontinuitySeq = startDiscontinuitySeq;
    }

    mRefreshState = INITIAL_MINIMUM_RELOAD_DELAY;
    mSeekMode = (LiveSession::SeekMode) seekMode;

    if (startTimeUs >= 0 || mSeekMode == LiveSession::kSeekModeNextSample) {
        mStartup = true;
        mIDRFound = false;
        mVideoBuffer->clear();
    }

    if (startTimeUs >= 0) {
        mStartTimeUs = startTimeUs;
        mFirstPTSValid = false;
        mSeqNumber = -1;
        mTimeChangeSignaled = false;
        mDownloadState->resetState();
    }

    postMonitorQueue();

    return OK;
}

void PlaylistFetcher::onPause() {
    cancelMonitorQueue();
    mLastDiscontinuitySeq = mDiscontinuitySeq;

    resetStoppingThreshold(false /* disconnect */);
}

void PlaylistFetcher::onStop(const sp<AMessage> &msg) {
    cancelMonitorQueue();

    int32_t clear;
    CHECK(msg->findInt32("clear", &clear));
    if (clear) {
        for (size_t i = 0; i < mPacketSources.size(); i++) {
            sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);
            packetSource->clear();
        }
    }

    mDownloadState->resetState();
    mPacketSources.clear();
    mStreamTypeMask = 0;

    resetStoppingThreshold(true /* disconnect */);
}

// Resume until we have reached the boundary timestamps listed in `msg`; when
// the remaining time is too short (within a resume threshold) stop immediately
// instead.
status_t PlaylistFetcher::onResumeUntil(const sp<AMessage> &msg) {
    sp<AMessage> params;
    CHECK(msg->findMessage("params", &params));

    mStopParams = params;
    onDownloadNext();

    return OK;
}

void PlaylistFetcher::notifyStopReached() {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatStopReached);
    notify->post();
}

void PlaylistFetcher::notifyError(status_t err) {
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

void PlaylistFetcher::queueDiscontinuity(
        ATSParser::DiscontinuityType type, const sp<AMessage> &extra) {
    for (size_t i = 0; i < mPacketSources.size(); ++i) {
        // do not discard buffer upon #EXT-X-DISCONTINUITY tag
        // (seek will discard buffer by abandoning old fetchers)
        mPacketSources.valueAt(i)->queueDiscontinuity(
                type, extra, false /* discard */);
    }
}

void PlaylistFetcher::onMonitorQueue() {
    // in the middle of an unfinished download, delay
    // playlist refresh as it'll change seq numbers
    if (!mDownloadState->hasSavedState()) {
        status_t err = refreshPlaylist();
        if (err != OK) {
            if (mNumRetriesForMonitorQueue < kMaxNumRetries) {
                ++mNumRetriesForMonitorQueue;
            } else {
                notifyError(err);
            }
            return;
        } else {
            mNumRetriesForMonitorQueue = 0;
        }
    }

    int64_t targetDurationUs = kMinBufferedDurationUs;
    if (mPlaylist != NULL) {
        targetDurationUs = mPlaylist->getTargetDuration();
    }

    int64_t bufferedDurationUs = 0LL;
    status_t finalResult = OK;
    if (mStreamTypeMask == LiveSession::STREAMTYPE_SUBTITLES) {
        sp<AnotherPacketSource> packetSource =
            mPacketSources.valueFor(LiveSession::STREAMTYPE_SUBTITLES);

        bufferedDurationUs =
                packetSource->getBufferedDurationUs(&finalResult);
    } else {
        // Use min stream duration, but ignore streams that never have any packet
        // enqueued to prevent us from waiting on a non-existent stream;
        // when we cannot make out from the manifest what streams are included in
        // a playlist we might assume extra streams.
        bufferedDurationUs = -1LL;
        for (size_t i = 0; i < mPacketSources.size(); ++i) {
            if ((mStreamTypeMask & mPacketSources.keyAt(i)) == 0
                    || mPacketSources[i]->getLatestEnqueuedMeta() == NULL) {
                continue;
            }

            int64_t bufferedStreamDurationUs =
                mPacketSources.valueAt(i)->getBufferedDurationUs(&finalResult);

            FSLOGV(mPacketSources.keyAt(i), "buffered %lld", (long long)bufferedStreamDurationUs);

            if (bufferedDurationUs == -1LL
                 || bufferedStreamDurationUs < bufferedDurationUs) {
                bufferedDurationUs = bufferedStreamDurationUs;
            }
        }
        if (bufferedDurationUs == -1LL) {
            bufferedDurationUs = 0LL;
        }
    }

    if (finalResult == OK && bufferedDurationUs < kMinBufferedDurationUs) {
        FLOGV("monitoring, buffered=%lld < %lld",
                (long long)bufferedDurationUs, (long long)kMinBufferedDurationUs);

        // delay the next download slightly; hopefully this gives other concurrent fetchers
        // a better chance to run.
        // onDownloadNext();
        sp<AMessage> msg = new AMessage(kWhatDownloadNext, this);
        msg->setInt32("generation", mMonitorQueueGeneration);
        msg->post(1000L);
    } else {
        // We'd like to maintain buffering above durationToBufferUs, so try
        // again when buffer just about to go below durationToBufferUs
        // (or after targetDurationUs / 2, whichever is smaller).
        int64_t delayUs = bufferedDurationUs - kMinBufferedDurationUs + 1000000LL;
        if (delayUs > targetDurationUs / 2) {
            delayUs = targetDurationUs / 2;
        }

        FLOGV("pausing for %lld, buffered=%lld > %lld",
                (long long)delayUs,
                (long long)bufferedDurationUs,
                (long long)kMinBufferedDurationUs);

        postMonitorQueue(delayUs);
    }
}

status_t PlaylistFetcher::refreshPlaylist() {
    if (delayUsToRefreshPlaylist() <= 0) {
        bool unchanged;
        sp<M3UParser> playlist = mHTTPDownloader->fetchPlaylist(
                mURI.c_str(), mPlaylistHash, &unchanged);

        if (playlist == NULL) {
            if (unchanged) {
                // We succeeded in fetching the playlist, but it was
                // unchanged from the last time we tried.

                if (mRefreshState != THIRD_UNCHANGED_RELOAD_ATTEMPT) {
                    mRefreshState = (RefreshState)(mRefreshState + 1);
                }
            } else {
                ALOGE("failed to load playlist at url '%s'", uriDebugString(mURI).c_str());
                return ERROR_IO;
            }
        } else {
            mRefreshState = INITIAL_MINIMUM_RELOAD_DELAY;
            mPlaylist = playlist;

            if (mPlaylist->isComplete() || mPlaylist->isEvent()) {
                updateDuration();
            }
            // Notify LiveSession to use target-duration based buffering level
            // for up/down switch. Default LiveSession::kUpSwitchMark may not
            // be reachable for live streams, as our max buffering amount is
            // limited to 3 segments.
            if (!mPlaylist->isComplete()) {
                updateTargetDuration();
            }
            mPlaylistTimeUs = ALooper::GetNowUs();
        }

        mLastPlaylistFetchTimeUs = ALooper::GetNowUs();
    }
    return OK;
}

// static
bool PlaylistFetcher::bufferStartsWithTsSyncByte(const sp<ABuffer>& buffer) {
    return buffer->size() > 0 && buffer->data()[0] == 0x47;
}

bool PlaylistFetcher::shouldPauseDownload() {
    if (mStreamTypeMask == LiveSession::STREAMTYPE_SUBTITLES) {
        // doesn't apply to subtitles
        return false;
    }

    // Calculate threshold to abort current download
    float thresholdRatio = getStoppingThreshold();

    if (thresholdRatio < 0.0f) {
        // never abort
        return false;
    } else if (thresholdRatio == 0.0f) {
        // immediately abort
        return true;
    }

    // now we have a positive thresholdUs, abort if remaining
    // portion to download is over that threshold.
    if (mSegmentFirstPTS < 0) {
        // this means we haven't even find the first access unit,
        // abort now as we must be very far away from the end.
        return true;
    }
    int64_t lastEnqueueUs = mSegmentFirstPTS;
    for (size_t i = 0; i < mPacketSources.size(); ++i) {
        if ((mStreamTypeMask & mPacketSources.keyAt(i)) == 0) {
            continue;
        }
        sp<AMessage> meta = mPacketSources[i]->getLatestEnqueuedMeta();
        int32_t type;
        if (meta == NULL || meta->findInt32("discontinuity", &type)) {
            continue;
        }
        int64_t tmpUs;
        CHECK(meta->findInt64("timeUs", &tmpUs));
        if (tmpUs > lastEnqueueUs) {
            lastEnqueueUs = tmpUs;
        }
    }
    lastEnqueueUs -= mSegmentFirstPTS;

    int64_t targetDurationUs = mPlaylist->getTargetDuration();
    int64_t thresholdUs = thresholdRatio * targetDurationUs;

    FLOGV("%spausing now, thresholdUs %lld, remaining %lld",
            targetDurationUs - lastEnqueueUs > thresholdUs ? "" : "not ",
            (long long)thresholdUs,
            (long long)(targetDurationUs - lastEnqueueUs));

    if (targetDurationUs - lastEnqueueUs > thresholdUs) {
        return true;
    }
    return false;
}

void PlaylistFetcher::initSeqNumberForLiveStream(
        int32_t &firstSeqNumberInPlaylist,
        int32_t &lastSeqNumberInPlaylist) {
    // start at least 3 target durations from the end.
    int64_t timeFromEnd = 0;
    size_t index = mPlaylist->size();
    sp<AMessage> itemMeta;
    int64_t itemDurationUs;
    int32_t targetDuration;
    if (mPlaylist->meta() != NULL
            && mPlaylist->meta()->findInt32("target-duration", &targetDuration)) {
        do {
            --index;
            if (!mPlaylist->itemAt(index, NULL /* uri */, &itemMeta)
                    || !itemMeta->findInt64("durationUs", &itemDurationUs)) {
                ALOGW("item or itemDurationUs missing");
                mSeqNumber = lastSeqNumberInPlaylist - 3;
                break;
            }

            timeFromEnd += itemDurationUs;
            mSeqNumber = firstSeqNumberInPlaylist + index;
        } while (timeFromEnd < targetDuration * 3E6 && index > 0);
    } else {
        ALOGW("target-duration missing");
        mSeqNumber = lastSeqNumberInPlaylist - 3;
    }

    if (mSeqNumber < firstSeqNumberInPlaylist) {
        mSeqNumber = firstSeqNumberInPlaylist;
    }
}

bool PlaylistFetcher::initDownloadState(
        AString &uri,
        sp<AMessage> &itemMeta,
        int32_t &firstSeqNumberInPlaylist,
        int32_t &lastSeqNumberInPlaylist) {
    status_t err = refreshPlaylist();
    firstSeqNumberInPlaylist = 0;
    lastSeqNumberInPlaylist = 0;
    bool discontinuity = false;

    if (mPlaylist != NULL) {
        mPlaylist->getSeqNumberRange(
                &firstSeqNumberInPlaylist, &lastSeqNumberInPlaylist);

        if (mDiscontinuitySeq < 0) {
            mDiscontinuitySeq = mPlaylist->getDiscontinuitySeq();
        }
    }

    mSegmentFirstPTS = -1LL;

    if (mPlaylist != NULL && mSeqNumber < 0) {
        CHECK_GE(mStartTimeUs, 0LL);

        if (mSegmentStartTimeUs < 0) {
            if (!mPlaylist->isComplete() && !mPlaylist->isEvent()) {
                // this is a live session
                initSeqNumberForLiveStream(firstSeqNumberInPlaylist, lastSeqNumberInPlaylist);
            } else {
                // When seeking mSegmentStartTimeUs is unavailable (< 0), we
                // use mStartTimeUs (client supplied timestamp) to determine both start segment
                // and relative position inside a segment
                mSeqNumber = getSeqNumberForTime(mStartTimeUs);
                mStartTimeUs -= getSegmentStartTimeUs(mSeqNumber);
            }
            mStartTimeUsRelative = true;
            FLOGV("Initial sequence number for time %lld is %d from (%d .. %d)",
                    (long long)mStartTimeUs, mSeqNumber, firstSeqNumberInPlaylist,
                    lastSeqNumberInPlaylist);
        } else {
            // When adapting or track switching, mSegmentStartTimeUs (relative
            // to media time 0) is used to determine the start segment; mStartTimeUs (absolute
            // timestamps coming from the media container) is used to determine the position
            // inside a segments.
            if (mStreamTypeMask != LiveSession::STREAMTYPE_SUBTITLES
                    && mSeekMode != LiveSession::kSeekModeNextSample) {
                // avoid double fetch/decode
                // Use (mSegmentStartTimeUs + 1/2 * targetDurationUs) to search
                // for the starting segment in new variant.
                // If the two variants' segments are aligned, this gives the
                // next segment. If they're not aligned, this gives the segment
                // that overlaps no more than 1/2 * targetDurationUs.
                mSeqNumber = getSeqNumberForTime(mSegmentStartTimeUs
                        + mPlaylist->getTargetDuration() / 2);
            } else {
                mSeqNumber = getSeqNumberForTime(mSegmentStartTimeUs);
            }
            ssize_t minSeq = getSeqNumberForDiscontinuity(mDiscontinuitySeq);
            if (mSeqNumber < minSeq) {
                mSeqNumber = minSeq;
            }

            if (mSeqNumber < firstSeqNumberInPlaylist) {
                mSeqNumber = firstSeqNumberInPlaylist;
            }

            if (mSeqNumber > lastSeqNumberInPlaylist) {
                mSeqNumber = lastSeqNumberInPlaylist;
            }
            FLOGV("Initial sequence number is %d from (%d .. %d)",
                    mSeqNumber, firstSeqNumberInPlaylist,
                    lastSeqNumberInPlaylist);
        }
    }

    // if mPlaylist is NULL then err must be non-OK; but the other way around might not be true
    if (mSeqNumber < firstSeqNumberInPlaylist
            || mSeqNumber > lastSeqNumberInPlaylist
            || err != OK) {
        if ((err != OK || !mPlaylist->isComplete()) && mNumRetries < kMaxNumRetries) {
            ++mNumRetries;

            if (mSeqNumber > lastSeqNumberInPlaylist || err != OK) {
                // make sure we reach this retry logic on refresh failures
                // by adding an err != OK clause to all enclosing if's.

                // refresh in increasing fraction (1/2, 1/3, ...) of the
                // playlist's target duration or 3 seconds, whichever is less
                int64_t delayUs = kMaxMonitorDelayUs;
                if (mPlaylist != NULL) {
                    delayUs = mPlaylist->size() * mPlaylist->getTargetDuration()
                            / (1 + mNumRetries);
                }
                if (delayUs > kMaxMonitorDelayUs) {
                    delayUs = kMaxMonitorDelayUs;
                }
                FLOGV("sequence number high: %d from (%d .. %d), "
                      "monitor in %lld (retry=%d)",
                        mSeqNumber, firstSeqNumberInPlaylist,
                        lastSeqNumberInPlaylist, (long long)delayUs, mNumRetries);
                postMonitorQueue(delayUs);
                return false;
            }

            if (err != OK) {
                notifyError(err);
                return false;
            }

            // we've missed the boat, let's start 3 segments prior to the latest sequence
            // number available and signal a discontinuity.

            ALOGI("We've missed the boat, restarting playback."
                  "  mStartup=%d, was  looking for %d in %d-%d",
                    mStartup, mSeqNumber, firstSeqNumberInPlaylist,
                    lastSeqNumberInPlaylist);
            if (mStopParams != NULL) {
                // we should have kept on fetching until we hit the boundaries in mStopParams,
                // but since the segments we are supposed to fetch have already rolled off
                // the playlist, i.e. we have already missed the boat, we inevitably have to
                // skip.
                notifyStopReached();
                return false;
            }
            mSeqNumber = lastSeqNumberInPlaylist - 3;
            if (mSeqNumber < firstSeqNumberInPlaylist) {
                mSeqNumber = firstSeqNumberInPlaylist;
            }
            discontinuity = true;

            // fall through
        } else {
            if (mPlaylist != NULL) {
                if (mSeqNumber >= firstSeqNumberInPlaylist + (int32_t)mPlaylist->size()
                        && !mPlaylist->isComplete()) {
                    // Live playlists
                    ALOGW("sequence number %d not yet available", mSeqNumber);
                    postMonitorQueue(delayUsToRefreshPlaylist());
                    return false;
                }
                ALOGE("Cannot find sequence number %d in playlist "
                     "(contains %d - %d)",
                     mSeqNumber, firstSeqNumberInPlaylist,
                      firstSeqNumberInPlaylist + (int32_t)mPlaylist->size() - 1);

                if (mTSParser != NULL) {
                    mTSParser->signalEOS(ERROR_END_OF_STREAM);
                    // Use an empty buffer; we don't have any new data, just want to extract
                    // potential new access units after flush.  Reset mSeqNumber to
                    // lastSeqNumberInPlaylist such that we set the correct access unit
                    // properties in extractAndQueueAccessUnitsFromTs.
                    sp<ABuffer> buffer = new ABuffer(0);
                    mSeqNumber = lastSeqNumberInPlaylist;
                    extractAndQueueAccessUnitsFromTs(buffer);
                }
                notifyError(ERROR_END_OF_STREAM);
            } else {
                // It's possible that we were never able to download the playlist.
                // In this case we should notify error, instead of EOS, as EOS during
                // prepare means we succeeded in downloading everything.
                ALOGE("Failed to download playlist!");
                notifyError(ERROR_IO);
            }

            return false;
        }
    }

    mNumRetries = 0;

    CHECK(mPlaylist->itemAt(
                mSeqNumber - firstSeqNumberInPlaylist,
                &uri,
                &itemMeta));

    CHECK(itemMeta->findInt32("discontinuity-sequence", &mDiscontinuitySeq));

    int32_t val;
    if (itemMeta->findInt32("discontinuity", &val) && val != 0) {
        discontinuity = true;
    } else if (mLastDiscontinuitySeq >= 0
            && mDiscontinuitySeq != mLastDiscontinuitySeq) {
        // Seek jumped to a new discontinuity sequence. We need to signal
        // a format change to decoder. Decoder needs to shutdown and be
        // created again if seamless format change is unsupported.
        FLOGV("saw discontinuity: mStartup %d, mLastDiscontinuitySeq %d, "
                "mDiscontinuitySeq %d, mStartTimeUs %lld",
                mStartup, mLastDiscontinuitySeq, mDiscontinuitySeq, (long long)mStartTimeUs);
        discontinuity = true;
    }
    mLastDiscontinuitySeq = -1;

    // decrypt a junk buffer to prefetch key; since a session uses only one http connection,
    // this avoids interleaved connections to the key and segment file.
    {
        sp<ABuffer> junk = new ABuffer(16);
        junk->setRange(0, 16);
        status_t err = decryptBuffer(mSeqNumber - firstSeqNumberInPlaylist, junk,
                true /* first */);
        if (err == ERROR_NOT_CONNECTED) {
            return false;
        } else if (err != OK) {
            notifyError(err);
            return false;
        }
    }

    if ((mStartup && !mTimeChangeSignaled) || discontinuity) {
        // We need to signal a time discontinuity to ATSParser on the
        // first segment after start, or on a discontinuity segment.
        // Setting mNextPTSTimeUs informs extractAndQueueAccessUnitsXX()
        // to send the time discontinuity.
        if (mPlaylist->isComplete() || mPlaylist->isEvent()) {
            // If this was a live event this made no sense since
            // we don't have access to all the segment before the current
            // one.
            mNextPTSTimeUs = getSegmentStartTimeUs(mSeqNumber);
        }

        // Setting mTimeChangeSignaled to true, so that if start time
        // searching goes into 2nd segment (without a discontinuity),
        // we don't reset time again. It causes corruption when pending
        // data in ATSParser is cleared.
        mTimeChangeSignaled = true;
    }

    if (discontinuity) {
        ALOGI("queueing discontinuity (explicit=%d)", discontinuity);

        // Signal a format discontinuity to ATSParser to clear partial data
        // from previous streams. Not doing this causes bitstream corruption.
        if (mTSParser != NULL) {
            mTSParser.clear();
        }

        queueDiscontinuity(
                ATSParser::DISCONTINUITY_FORMAT_ONLY,
                NULL /* extra */);

        if (mStartup && mStartTimeUsRelative && mFirstPTSValid) {
            // This means we guessed mStartTimeUs to be in the previous
            // segment (likely very close to the end), but either video or
            // audio has not found start by the end of that segment.
            //
            // If this new segment is not a discontinuity, keep searching.
            //
            // If this new segment even got a discontinuity marker, just
            // set mStartTimeUs=0, and take all samples from now on.
            mStartTimeUs = 0;
            mFirstPTSValid = false;
            mIDRFound = false;
            mVideoBuffer->clear();
        }
    }

    FLOGV("fetching segment %d from (%d .. %d)",
            mSeqNumber, firstSeqNumberInPlaylist, lastSeqNumberInPlaylist);
    return true;
}

void PlaylistFetcher::onDownloadNext() {
    AString uri;
    sp<AMessage> itemMeta;
    sp<ABuffer> buffer;
    sp<ABuffer> tsBuffer;
    int32_t firstSeqNumberInPlaylist = 0;
    int32_t lastSeqNumberInPlaylist = 0;
    bool connectHTTP = true;

    if (mDownloadState->hasSavedState()) {
        mDownloadState->restoreState(
                uri,
                itemMeta,
                buffer,
                tsBuffer,
                firstSeqNumberInPlaylist,
                lastSeqNumberInPlaylist);
        connectHTTP = false;
        FLOGV("resuming: '%s'", uri.c_str());
    } else {
        if (!initDownloadState(
                uri,
                itemMeta,
                firstSeqNumberInPlaylist,
                lastSeqNumberInPlaylist)) {
            return;
        }
        FLOGV("fetching: '%s'", uri.c_str());
    }

    int64_t range_offset, range_length;
    if (!itemMeta->findInt64("range-offset", &range_offset)
            || !itemMeta->findInt64("range-length", &range_length)) {
        range_offset = 0;
        range_length = -1;
    }

    // block-wise download
    bool shouldPause = false;
    ssize_t bytesRead;
    do {
        int64_t startUs = ALooper::GetNowUs();
        bytesRead = mHTTPDownloader->fetchBlock(
                uri.c_str(), &buffer, range_offset, range_length, kDownloadBlockSize,
                NULL /* actualURL */, connectHTTP);
        int64_t delayUs = ALooper::GetNowUs() - startUs;

        if (bytesRead == ERROR_NOT_CONNECTED) {
            return;
        }
        if (bytesRead < 0) {
            status_t err = bytesRead;
            ALOGE("failed to fetch .ts segment at url '%s'", uriDebugString(uri).c_str());
            notifyError(err);
            return;
        }

        // add sample for bandwidth estimation, excluding samples from subtitles (as
        // its too small), or during startup/resumeUntil (when we could have more than
        // one connection open which affects bandwidth)
        if (!mStartup && mStopParams == NULL && bytesRead > 0
                && (mStreamTypeMask
                        & (LiveSession::STREAMTYPE_AUDIO
                        | LiveSession::STREAMTYPE_VIDEO))) {
            mSession->addBandwidthMeasurement(bytesRead, delayUs);
            if (delayUs > 2000000LL) {
                FLOGV("bytesRead %zd took %.2f seconds - abnormal bandwidth dip",
                        bytesRead, (double)delayUs / 1.0e6);
            }
        }

        connectHTTP = false;

        CHECK(buffer != NULL);

        size_t size = buffer->size();
        // Set decryption range.
        buffer->setRange(size - bytesRead, bytesRead);
        status_t err = decryptBuffer(mSeqNumber - firstSeqNumberInPlaylist, buffer,
                buffer->offset() == 0 /* first */);
        // Unset decryption range.
        buffer->setRange(0, size);

        if (err != OK) {
            ALOGE("decryptBuffer failed w/ error %d", err);

            notifyError(err);
            return;
        }

        bool startUp = mStartup; // save current start up state

        err = OK;
        if (bufferStartsWithTsSyncByte(buffer)) {
            // Incremental extraction is only supported for MPEG2 transport streams.
            if (tsBuffer == NULL) {
                tsBuffer = new ABuffer(buffer->data(), buffer->capacity());
                tsBuffer->setRange(0, 0);
            } else if (tsBuffer->capacity() != buffer->capacity()) {
                size_t tsOff = tsBuffer->offset(), tsSize = tsBuffer->size();
                tsBuffer = new ABuffer(buffer->data(), buffer->capacity());
                tsBuffer->setRange(tsOff, tsSize);
            }
            tsBuffer->setRange(tsBuffer->offset(), tsBuffer->size() + bytesRead);
            err = extractAndQueueAccessUnitsFromTs(tsBuffer);
        }

        if (err == -EAGAIN) {
            // starting sequence number too low/high
            mTSParser.clear();
            for (size_t i = 0; i < mPacketSources.size(); i++) {
                sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);
                packetSource->clear();
            }
            postMonitorQueue();
            return;
        } else if (err == ERROR_OUT_OF_RANGE) {
            // reached stopping point
            notifyStopReached();
            return;
        } else if (err != OK) {
            notifyError(err);
            return;
        }
        // If we're switching, post start notification
        // this should only be posted when the last chunk is full processed by TSParser
        if (mSeekMode != LiveSession::kSeekModeExactPosition && startUp != mStartup) {
            CHECK(mStartTimeUsNotify != NULL);
            mStartTimeUsNotify->post();
            mStartTimeUsNotify.clear();
            shouldPause = true;
        }
        if (shouldPause || shouldPauseDownload()) {
            // save state and return if this is not the last chunk,
            // leaving the fetcher in paused state.
            if (bytesRead != 0) {
                mDownloadState->saveState(
                        uri,
                        itemMeta,
                        buffer,
                        tsBuffer,
                        firstSeqNumberInPlaylist,
                        lastSeqNumberInPlaylist);
                return;
            }
            shouldPause = true;
        }
    } while (bytesRead != 0);

    if (bufferStartsWithTsSyncByte(buffer)) {
        // If we don't see a stream in the program table after fetching a full ts segment
        // mark it as nonexistent.
        ATSParser::SourceType srcTypes[] =
                { ATSParser::VIDEO, ATSParser::AUDIO };
        LiveSession::StreamType streamTypes[] =
                { LiveSession::STREAMTYPE_VIDEO, LiveSession::STREAMTYPE_AUDIO };
        const size_t kNumTypes = NELEM(srcTypes);

        for (size_t i = 0; i < kNumTypes; i++) {
            ATSParser::SourceType srcType = srcTypes[i];
            LiveSession::StreamType streamType = streamTypes[i];

            sp<AnotherPacketSource> source =
                static_cast<AnotherPacketSource *>(
                    mTSParser->getSource(srcType).get());

            if (!mTSParser->hasSource(srcType)) {
                ALOGW("MPEG2 Transport stream does not contain %s data.",
                      srcType == ATSParser::VIDEO ? "video" : "audio");

                mStreamTypeMask &= ~streamType;
                mPacketSources.removeItem(streamType);
            }
        }

    }

    if (checkDecryptPadding(buffer) != OK) {
        ALOGE("Incorrect padding bytes after decryption.");
        notifyError(ERROR_MALFORMED);
        return;
    }

    if (tsBuffer != NULL) {
        AString method;
        CHECK(buffer->meta()->findString("cipher-method", &method));
        if ((tsBuffer->size() > 0 && method == "NONE")
                || tsBuffer->size() > 16) {
            ALOGE("MPEG2 transport stream is not an even multiple of 188 "
                    "bytes in length.");
            notifyError(ERROR_MALFORMED);
            return;
        }
    }

    // bulk extract non-ts files
    bool startUp = mStartup;
    if (tsBuffer == NULL) {
        status_t err = extractAndQueueAccessUnits(buffer, itemMeta);
        if (err == -EAGAIN) {
            // starting sequence number too low/high
            postMonitorQueue();
            return;
        } else if (err == ERROR_OUT_OF_RANGE) {
            // reached stopping point
            notifyStopReached();
            return;
        } else if (err != OK) {
            notifyError(err);
            return;
        }
    }

    ++mSeqNumber;

    // if adapting, pause after found the next starting point
    if (mSeekMode != LiveSession::kSeekModeExactPosition && startUp != mStartup) {
        CHECK(mStartTimeUsNotify != NULL);
        mStartTimeUsNotify->post();
        mStartTimeUsNotify.clear();
        shouldPause = true;
    }

    if (!shouldPause) {
        postMonitorQueue();
    }
}

/*
 * returns true if we need to adjust mSeqNumber
 */
bool PlaylistFetcher::adjustSeqNumberWithAnchorTime(int64_t anchorTimeUs) {
    int32_t firstSeqNumberInPlaylist = mPlaylist->getFirstSeqNumber();

    int64_t minDiffUs, maxDiffUs;
    if (mSeekMode == LiveSession::kSeekModeNextSample) {
        // if the previous fetcher paused in the middle of a segment, we
        // want to start at a segment that overlaps the last sample
        minDiffUs = -mPlaylist->getTargetDuration();
        maxDiffUs = 0LL;
    } else {
        // if the previous fetcher paused at the end of a segment, ideally
        // we want to start at the segment that's roughly aligned with its
        // next segment, but if the two variants are not well aligned we
        // adjust the diff to within (-T/2, T/2)
        minDiffUs = -mPlaylist->getTargetDuration() / 2;
        maxDiffUs = mPlaylist->getTargetDuration() / 2;
    }

    int32_t oldSeqNumber = mSeqNumber;
    ssize_t index = mSeqNumber - firstSeqNumberInPlaylist;

    // adjust anchorTimeUs to within (minDiffUs, maxDiffUs) from mStartTimeUs
    int64_t diffUs = anchorTimeUs - mStartTimeUs;
    if (diffUs > maxDiffUs) {
        while (index > 0 && diffUs > maxDiffUs) {
            --index;

            sp<AMessage> itemMeta;
            CHECK(mPlaylist->itemAt(index, NULL /* uri */, &itemMeta));

            int64_t itemDurationUs;
            CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

            diffUs -= itemDurationUs;
        }
    } else if (diffUs < minDiffUs) {
        while (index + 1 < (ssize_t) mPlaylist->size()
                && diffUs < minDiffUs) {
            ++index;

            sp<AMessage> itemMeta;
            CHECK(mPlaylist->itemAt(index, NULL /* uri */, &itemMeta));

            int64_t itemDurationUs;
            CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

            diffUs += itemDurationUs;
        }
    }

    mSeqNumber = firstSeqNumberInPlaylist + index;

    if (mSeqNumber != oldSeqNumber) {
        FLOGV("guessed wrong seg number: diff %lld out of [%lld, %lld]",
                (long long) anchorTimeUs - mStartTimeUs,
                (long long) minDiffUs,
                (long long) maxDiffUs);
        return true;
    }
    return false;
}

int32_t PlaylistFetcher::getSeqNumberForDiscontinuity(size_t discontinuitySeq) const {
    int32_t firstSeqNumberInPlaylist = mPlaylist->getFirstSeqNumber();

    size_t index = 0;
    while (index < mPlaylist->size()) {
        sp<AMessage> itemMeta;
        CHECK(mPlaylist->itemAt( index, NULL /* uri */, &itemMeta));
        size_t curDiscontinuitySeq;
        CHECK(itemMeta->findInt32("discontinuity-sequence", (int32_t *)&curDiscontinuitySeq));
        int32_t seqNumber = firstSeqNumberInPlaylist + index;
        if (curDiscontinuitySeq == discontinuitySeq) {
            return seqNumber;
        } else if (curDiscontinuitySeq > discontinuitySeq) {
            return seqNumber <= 0 ? 0 : seqNumber - 1;
        }

        ++index;
    }

    return firstSeqNumberInPlaylist + mPlaylist->size();
}

int32_t PlaylistFetcher::getSeqNumberForTime(int64_t timeUs) const {
    size_t index = 0;
    int64_t segmentStartUs = 0;
    while (index < mPlaylist->size()) {
        sp<AMessage> itemMeta;
        CHECK(mPlaylist->itemAt(
                    index, NULL /* uri */, &itemMeta));

        int64_t itemDurationUs;
        CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

        if (timeUs < segmentStartUs + itemDurationUs) {
            break;
        }

        segmentStartUs += itemDurationUs;
        ++index;
    }

    if (index >= mPlaylist->size()) {
        index = mPlaylist->size() - 1;
    }

    return mPlaylist->getFirstSeqNumber() + index;
}

const sp<ABuffer> &PlaylistFetcher::setAccessUnitProperties(
        const sp<ABuffer> &accessUnit, const sp<AnotherPacketSource> &source, bool discard) {
    sp<MetaData> format = source->getFormat();
    if (format != NULL) {
        // for simplicity, store a reference to the format in each unit
        accessUnit->meta()->setObject("format", format);
    }

    if (discard) {
        accessUnit->meta()->setInt32("discard", discard);
    }

    accessUnit->meta()->setInt32("discontinuitySeq", mDiscontinuitySeq);
    accessUnit->meta()->setInt64("segmentStartTimeUs", getSegmentStartTimeUs(mSeqNumber));
    accessUnit->meta()->setInt64("segmentFirstTimeUs", mSegmentFirstPTS);
    accessUnit->meta()->setInt64("segmentDurationUs", getSegmentDurationUs(mSeqNumber));
    if (!mPlaylist->isComplete() && !mPlaylist->isEvent()) {
        accessUnit->meta()->setInt64("playlistTimeUs", mPlaylistTimeUs);
    }
    return accessUnit;
}

bool PlaylistFetcher::isStartTimeReached(int64_t timeUs) {
    if (!mFirstPTSValid) {
        mFirstTimeUs = timeUs;
        mFirstPTSValid = true;
    }
    bool startTimeReached = true;
    if (mStartTimeUsRelative) {
        FLOGV("startTimeUsRelative, timeUs (%lld) - %lld = %lld",
                (long long)timeUs,
                (long long)mFirstTimeUs,
                (long long)(timeUs - mFirstTimeUs));
        timeUs -= mFirstTimeUs;
        if (timeUs < 0) {
            FLOGV("clamp negative timeUs to 0");
            timeUs = 0;
        }
        startTimeReached = (timeUs >= mStartTimeUs);
    }
    return startTimeReached;
}

status_t PlaylistFetcher::extractAndQueueAccessUnitsFromTs(const sp<ABuffer> &buffer) {
    if (mTSParser == NULL) {
        // Use TS_TIMESTAMPS_ARE_ABSOLUTE so pts carry over between fetchers.
        mTSParser = new ATSParser(ATSParser::TS_TIMESTAMPS_ARE_ABSOLUTE);
    }

    if (mNextPTSTimeUs >= 0LL) {
        sp<AMessage> extra = new AMessage;
        // Since we are using absolute timestamps, signal an offset of 0 to prevent
        // ATSParser from skewing the timestamps of access units.
        extra->setInt64(kATSParserKeyMediaTimeUs, 0);

        // When adapting, signal a recent media time to the parser,
        // so that PTS wrap around is handled for the new variant.
        if (mStartTimeUs >= 0 && !mStartTimeUsRelative) {
            extra->setInt64(kATSParserKeyRecentMediaTimeUs, mStartTimeUs);
        }

        mTSParser->signalDiscontinuity(
                ATSParser::DISCONTINUITY_TIME, extra);

        mNextPTSTimeUs = -1LL;
    }

    if (mSampleAesKeyItemChanged) {
        mTSParser->signalNewSampleAesKey(mSampleAesKeyItem);
        mSampleAesKeyItemChanged = false;
    }

    size_t offset = 0;
    while (offset + 188 <= buffer->size()) {
        status_t err = mTSParser->feedTSPacket(buffer->data() + offset, 188);

        if (err != OK) {
            return err;
        }

        offset += 188;
    }
    // setRange to indicate consumed bytes.
    buffer->setRange(buffer->offset() + offset, buffer->size() - offset);

    if (mSegmentFirstPTS < 0LL) {
        // get the smallest first PTS from all streams present in this parser
        for (size_t i = mPacketSources.size(); i > 0;) {
            i--;
            const LiveSession::StreamType stream = mPacketSources.keyAt(i);
            if (stream == LiveSession::STREAMTYPE_SUBTITLES) {
                ALOGE("MPEG2 Transport streams do not contain subtitles.");
                return ERROR_MALFORMED;
            }
            if (stream == LiveSession::STREAMTYPE_METADATA) {
                continue;
            }
            ATSParser::SourceType type =LiveSession::getSourceTypeForStream(stream);
            sp<AnotherPacketSource> source =
                static_cast<AnotherPacketSource *>(
                        mTSParser->getSource(type).get());

            if (source == NULL) {
                continue;
            }
            sp<AMessage> meta = source->getMetaAfterLastDequeued(0);
            if (meta != NULL) {
                int64_t timeUs;
                CHECK(meta->findInt64("timeUs", &timeUs));
                if (mSegmentFirstPTS < 0LL || timeUs < mSegmentFirstPTS) {
                    mSegmentFirstPTS = timeUs;
                }
            }
        }
        if (mSegmentFirstPTS < 0LL) {
            // didn't find any TS packet, can return early
            return OK;
        }
        if (!mStartTimeUsRelative) {
            // mStartup
            //   mStartup is true until we have queued a packet for all the streams
            //   we are fetching. We queue packets whose timestamps are greater than
            //   mStartTimeUs.
            // mSegmentStartTimeUs >= 0
            //   mSegmentStartTimeUs is non-negative when adapting or switching tracks
            // adjustSeqNumberWithAnchorTime(timeUs) == true
            //   we guessed a seq number that's either too large or too small.
            // If this happens, we'll adjust mSeqNumber and restart fetching from new
            // location. Note that we only want to adjust once, so set mSegmentStartTimeUs
            // to -1 so that we don't enter this chunk next time.
            if (mStartup && mSegmentStartTimeUs >= 0
                    && adjustSeqNumberWithAnchorTime(mSegmentFirstPTS)) {
                mStartTimeUsNotify = mNotify->dup();
                mStartTimeUsNotify->setInt32("what", kWhatStartedAt);
                mStartTimeUsNotify->setString("uri", mURI);
                mIDRFound = false;
                mSegmentStartTimeUs = -1;
                return -EAGAIN;
            }
        }
    }

    status_t err = OK;
    for (size_t i = mPacketSources.size(); i > 0;) {
        i--;
        sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);

        const LiveSession::StreamType stream = mPacketSources.keyAt(i);
        if (stream == LiveSession::STREAMTYPE_SUBTITLES) {
            ALOGE("MPEG2 Transport streams do not contain subtitles.");
            return ERROR_MALFORMED;
        }

        const char *key = LiveSession::getKeyForStream(stream);
        ATSParser::SourceType type =LiveSession::getSourceTypeForStream(stream);

        sp<AnotherPacketSource> source =
            static_cast<AnotherPacketSource *>(
                    mTSParser->getSource(type).get());

        if (source == NULL) {
            continue;
        }

        const char *mime;
        sp<MetaData> format  = source->getFormat();
        bool isAvc = format != NULL && format->findCString(kKeyMIMEType, &mime)
                && !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC);

        sp<ABuffer> accessUnit;
        status_t finalResult;
        while (source->hasBufferAvailable(&finalResult)
                && source->dequeueAccessUnit(&accessUnit) == OK) {

            int64_t timeUs;
            CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));

            if (mStartup) {
                bool startTimeReached = isStartTimeReached(timeUs);

                if (!startTimeReached || (isAvc && !mIDRFound)) {
                    // buffer up to the closest preceding IDR frame in the next segement,
                    // or the closest succeeding IDR frame after the exact position
                    FSLOGV(stream, "timeUs(%lld)-mStartTimeUs(%lld)=%lld, mIDRFound=%d",
                            (long long)timeUs,
                            (long long)mStartTimeUs,
                            (long long)timeUs - mStartTimeUs,
                            mIDRFound);
                    if (isAvc) {
                        if (IsIDR(accessUnit->data(), accessUnit->size())) {
                            mVideoBuffer->clear();
                            FSLOGV(stream, "found IDR, clear mVideoBuffer");
                            mIDRFound = true;
                        }
                        if (mIDRFound && mStartTimeUsRelative && !startTimeReached) {
                            mVideoBuffer->queueAccessUnit(accessUnit);
                            FSLOGV(stream, "saving AVC video AccessUnit");
                        }
                    }
                    if (!startTimeReached || (isAvc && !mIDRFound)) {
                        continue;
                    }
                }
            }

            if (mStartTimeUsNotify != NULL) {
                uint32_t streamMask = 0;
                mStartTimeUsNotify->findInt32("streamMask", (int32_t *) &streamMask);
                if ((mStreamTypeMask & mPacketSources.keyAt(i))
                        && !(streamMask & mPacketSources.keyAt(i))) {
                    streamMask |= mPacketSources.keyAt(i);
                    mStartTimeUsNotify->setInt32("streamMask", streamMask);
                    FSLOGV(stream, "found start point, timeUs=%lld, streamMask becomes %x",
                            (long long)timeUs, streamMask);

                    if (streamMask == mStreamTypeMask) {
                        FLOGV("found start point for all streams");
                        mStartup = false;
                    }
                }
            }

            if (mStopParams != NULL) {
                int32_t discontinuitySeq;
                int64_t stopTimeUs;
                if (!mStopParams->findInt32("discontinuitySeq", &discontinuitySeq)
                        || discontinuitySeq > mDiscontinuitySeq
                        || !mStopParams->findInt64(key, &stopTimeUs)
                        || (discontinuitySeq == mDiscontinuitySeq
                                && timeUs >= stopTimeUs)) {
                    FSLOGV(stream, "reached stop point, timeUs=%lld", (long long)timeUs);
                    mStreamTypeMask &= ~stream;
                    mPacketSources.removeItemsAt(i);
                    break;
                }
            }

            if (stream == LiveSession::STREAMTYPE_VIDEO) {
                const bool discard = true;
                status_t status;
                while (mVideoBuffer->hasBufferAvailable(&status)) {
                    sp<ABuffer> videoBuffer;
                    mVideoBuffer->dequeueAccessUnit(&videoBuffer);
                    setAccessUnitProperties(videoBuffer, source, discard);
                    packetSource->queueAccessUnit(videoBuffer);
                    int64_t bufferTimeUs;
                    CHECK(videoBuffer->meta()->findInt64("timeUs", &bufferTimeUs));
                    FSLOGV(stream, "queueAccessUnit (saved), timeUs=%lld",
                            (long long)bufferTimeUs);
                }
            } else if (stream == LiveSession::STREAMTYPE_METADATA && !mHasMetadata) {
                mHasMetadata = true;
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatMetadataDetected);
                notify->post();
            }

            setAccessUnitProperties(accessUnit, source);
            packetSource->queueAccessUnit(accessUnit);
            FSLOGV(stream, "queueAccessUnit, timeUs=%lld", (long long)timeUs);
        }

        if (err != OK) {
            break;
        }
    }

    if (err != OK) {
        for (size_t i = mPacketSources.size(); i > 0;) {
            i--;
            sp<AnotherPacketSource> packetSource = mPacketSources.valueAt(i);
            packetSource->clear();
        }
        return err;
    }

    if (!mStreamTypeMask) {
        // Signal gap is filled between original and new stream.
        FLOGV("reached stop point for all streams");
        return ERROR_OUT_OF_RANGE;
    }

    return OK;
}

/* static */
bool PlaylistFetcher::bufferStartsWithWebVTTMagicSequence(
        const sp<ABuffer> &buffer) {
    size_t pos = 0;

    // skip possible BOM
    if (buffer->size() >= pos + 3 &&
            !memcmp("\xef\xbb\xbf", buffer->data() + pos, 3)) {
        pos += 3;
    }

    // accept WEBVTT followed by SPACE, TAB or (CR) LF
    if (buffer->size() < pos + 6 ||
            memcmp("WEBVTT", buffer->data() + pos, 6)) {
        return false;
    }
    pos += 6;

    if (buffer->size() == pos) {
        return true;
    }

    uint8_t sep = buffer->data()[pos];
    return sep == ' ' || sep == '\t' || sep == '\n' || sep == '\r';
}

status_t PlaylistFetcher::extractAndQueueAccessUnits(
        const sp<ABuffer> &buffer, const sp<AMessage> &itemMeta) {
    if (bufferStartsWithWebVTTMagicSequence(buffer)) {
        if (mStreamTypeMask != LiveSession::STREAMTYPE_SUBTITLES) {
            ALOGE("This stream only contains subtitles.");
            return ERROR_MALFORMED;
        }

        const sp<AnotherPacketSource> packetSource =
            mPacketSources.valueFor(LiveSession::STREAMTYPE_SUBTITLES);

        int64_t durationUs;
        CHECK(itemMeta->findInt64("durationUs", &durationUs));
        buffer->meta()->setInt64("timeUs", getSegmentStartTimeUs(mSeqNumber));
        buffer->meta()->setInt64("durationUs", durationUs);
        buffer->meta()->setInt64("segmentStartTimeUs", getSegmentStartTimeUs(mSeqNumber));
        buffer->meta()->setInt32("discontinuitySeq", mDiscontinuitySeq);
        buffer->meta()->setInt32("subtitleGeneration", mSubtitleGeneration);
        packetSource->queueAccessUnit(buffer);
        return OK;
    }

    if (mNextPTSTimeUs >= 0LL) {
        mNextPTSTimeUs = -1LL;
    }

    // This better be an ISO 13818-7 (AAC) or ISO 13818-1 (MPEG) audio
    // stream prefixed by an ID3 tag.

    bool firstID3Tag = true;
    uint64_t PTS = 0;

    for (;;) {
        // Make sure to skip all ID3 tags preceding the audio data.
        // At least one must be present to provide the PTS timestamp.

        ID3 id3(buffer->data(), buffer->size(), true /* ignoreV1 */);
        if (!id3.isValid()) {
            if (firstID3Tag) {
                ALOGE("Unable to parse ID3 tag.");
                return ERROR_MALFORMED;
            } else {
                break;
            }
        }

        if (firstID3Tag) {
            bool found = false;

            ID3::Iterator it(id3, "PRIV");
            while (!it.done()) {
                size_t length;
                const uint8_t *data = it.getData(&length);
                if (!data) {
                    return ERROR_MALFORMED;
                }

                static const char *kMatchName =
                    "com.apple.streaming.transportStreamTimestamp";
                static const size_t kMatchNameLen = strlen(kMatchName);

                if (length == kMatchNameLen + 1 + 8
                        && !strncmp((const char *)data, kMatchName, kMatchNameLen)) {
                    found = true;
                    PTS = U64_AT(&data[kMatchNameLen + 1]);
                }

                it.next();
            }

            if (!found) {
                ALOGE("Unable to extract transportStreamTimestamp from ID3 tag.");
                return ERROR_MALFORMED;
            }
        }

        // skip the ID3 tag
        buffer->setRange(
                buffer->offset() + id3.rawSize(), buffer->size() - id3.rawSize());

        firstID3Tag = false;
    }

    if (mStreamTypeMask != LiveSession::STREAMTYPE_AUDIO) {
        ALOGW("This stream only contains audio data!");

        mStreamTypeMask &= LiveSession::STREAMTYPE_AUDIO;

        if (mStreamTypeMask == 0) {
            return OK;
        }
    }

    sp<AnotherPacketSource> packetSource =
        mPacketSources.valueFor(LiveSession::STREAMTYPE_AUDIO);

    if (packetSource->getFormat() == NULL && buffer->size() >= 7) {
        ABitReader bits(buffer->data(), buffer->size());

        // adts_fixed_header

        CHECK_EQ(bits.getBits(12), 0xfffu);
        bits.skipBits(3);  // ID, layer
        bool protection_absent __unused = bits.getBits(1) != 0;

        unsigned profile = bits.getBits(2);
        CHECK_NE(profile, 3u);
        unsigned sampling_freq_index = bits.getBits(4);
        bits.getBits(1);  // private_bit
        unsigned channel_configuration = bits.getBits(3);
        CHECK_NE(channel_configuration, 0u);
        bits.skipBits(2);  // original_copy, home

        sp<MetaData> meta = new MetaData();
        MakeAACCodecSpecificData(*meta,
                profile, sampling_freq_index, channel_configuration);

        meta->setInt32(kKeyIsADTS, true);

        packetSource->setFormat(meta);
    }

    int64_t numSamples = 0LL;
    int32_t sampleRate;
    CHECK(packetSource->getFormat()->findInt32(kKeySampleRate, &sampleRate));

    int64_t timeUs = (PTS * 100LL) / 9LL;
    if (mStartup && !mFirstPTSValid) {
        mFirstPTSValid = true;
        mFirstTimeUs = timeUs;
    }

    if (mSegmentFirstPTS < 0LL) {
        mSegmentFirstPTS = timeUs;
        if (!mStartTimeUsRelative) {
            // Duplicated logic from how we handle .ts playlists.
            if (mStartup && mSegmentStartTimeUs >= 0
                    && adjustSeqNumberWithAnchorTime(timeUs)) {
                mSegmentStartTimeUs = -1;
                return -EAGAIN;
            }
        }
    }

    sp<HlsSampleDecryptor> sampleDecryptor = NULL;
    if (mSampleAesKeyItem != NULL) {
        ALOGV("extractAndQueueAccessUnits[%d] SampleAesKeyItem: Key: %s  IV: %s",
                mSeqNumber,
                HlsSampleDecryptor::aesBlockToStr(mKeyData).c_str(),
                HlsSampleDecryptor::aesBlockToStr(mAESInitVec).c_str());

        sampleDecryptor = new HlsSampleDecryptor(mSampleAesKeyItem);
    }

    int frameId = 0;

    size_t offset = 0;
    while (offset < buffer->size()) {
        const uint8_t *adtsHeader = buffer->data() + offset;
        if (buffer->size() <= offset+5) {
            ALOGV("buffer does not contain a complete header");
            return ERROR_MALFORMED;
        }
        // non-const pointer for decryption if needed
        uint8_t *adtsFrame = buffer->data() + offset;

        unsigned aac_frame_length =
            ((adtsHeader[3] & 3) << 11)
            | (adtsHeader[4] << 3)
            | (adtsHeader[5] >> 5);

        if (aac_frame_length == 0) {
            const uint8_t *id3Header = adtsHeader;
            if (!memcmp(id3Header, "ID3", 3)) {
                ID3 id3(id3Header, buffer->size() - offset, true);
                if (id3.isValid()) {
                    offset += id3.rawSize();
                    continue;
                };
            }
            return ERROR_MALFORMED;
        }

        CHECK_LE(offset + aac_frame_length, buffer->size());

        int64_t unitTimeUs = timeUs + numSamples * 1000000LL / sampleRate;
        offset += aac_frame_length;

        // Each AAC frame encodes 1024 samples.
        numSamples += 1024;

        if (mStartup) {
            int64_t startTimeUs = unitTimeUs;
            if (mStartTimeUsRelative) {
                startTimeUs -= mFirstTimeUs;
                if (startTimeUs  < 0) {
                    startTimeUs = 0;
                }
            }
            if (startTimeUs < mStartTimeUs) {
                continue;
            }

            if (mStartTimeUsNotify != NULL) {
                mStartTimeUsNotify->setInt32("streamMask", LiveSession::STREAMTYPE_AUDIO);
                mStartup = false;
            }
        }

        if (mStopParams != NULL) {
            int32_t discontinuitySeq;
            int64_t stopTimeUs;
            if (!mStopParams->findInt32("discontinuitySeq", &discontinuitySeq)
                    || discontinuitySeq > mDiscontinuitySeq
                    || !mStopParams->findInt64("timeUsAudio", &stopTimeUs)
                    || (discontinuitySeq == mDiscontinuitySeq && unitTimeUs >= stopTimeUs)) {
                mStreamTypeMask = 0;
                mPacketSources.clear();
                return ERROR_OUT_OF_RANGE;
            }
        }

        if (sampleDecryptor != NULL) {
            bool protection_absent = (adtsHeader[1] & 0x1);
            size_t headerSize = protection_absent ? 7 : 9;
            if (frameId == 0) {
                ALOGV("extractAndQueueAAC[%d] protection_absent %d (%02x) headerSize %zu",
                        mSeqNumber, protection_absent, adtsHeader[1], headerSize);
            }

            sampleDecryptor->processAAC(headerSize, adtsFrame, aac_frame_length);
        }
        frameId++;

        sp<ABuffer> unit = new ABuffer(aac_frame_length);
        memcpy(unit->data(), adtsHeader, aac_frame_length);

        unit->meta()->setInt64("timeUs", unitTimeUs);
        setAccessUnitProperties(unit, packetSource);
        packetSource->queueAccessUnit(unit);
    }

    return OK;
}

void PlaylistFetcher::updateDuration() {
    int64_t durationUs = 0LL;
    for (size_t index = 0; index < mPlaylist->size(); ++index) {
        sp<AMessage> itemMeta;
        CHECK(mPlaylist->itemAt(
                    index, NULL /* uri */, &itemMeta));

        int64_t itemDurationUs;
        CHECK(itemMeta->findInt64("durationUs", &itemDurationUs));

        durationUs += itemDurationUs;
    }

    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatDurationUpdate);
    msg->setInt64("durationUs", durationUs);
    msg->post();
}

void PlaylistFetcher::updateTargetDuration() {
    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("what", kWhatTargetDurationUpdate);
    msg->setInt64("targetDurationUs", mPlaylist->getTargetDuration());
    msg->post();
}

}  // namespace android
