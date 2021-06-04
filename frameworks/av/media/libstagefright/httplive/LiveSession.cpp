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

//#define LOG_NDEBUG 0
#define LOG_TAG "LiveSession"
#include <utils/Log.h>

#include "LiveSession.h"
#include "HTTPDownloader.h"
#include "M3UParser.h"
#include "PlaylistFetcher.h"

#include "mpeg2ts/AnotherPacketSource.h"

#include <cutils/properties.h>
#include <media/MediaHTTPService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

#include <utils/Mutex.h>

#include <ctype.h>
#include <inttypes.h>

namespace android {

// static
// Bandwidth Switch Mark Defaults
const int64_t LiveSession::kUpSwitchMarkUs = 15000000LL;
const int64_t LiveSession::kDownSwitchMarkUs = 20000000LL;
const int64_t LiveSession::kUpSwitchMarginUs = 5000000LL;
const int64_t LiveSession::kResumeThresholdUs = 100000LL;

//TODO: redefine this mark to a fair value
// default buffer underflow mark
static const int kUnderflowMarkMs = 1000;  // 1 second

struct LiveSession::BandwidthEstimator : public RefBase {
    BandwidthEstimator();

    void addBandwidthMeasurement(size_t numBytes, int64_t delayUs);
    bool estimateBandwidth(
            int32_t *bandwidth,
            bool *isStable = NULL,
            int32_t *shortTermBps = NULL);

private:
    // Bandwidth estimation parameters
    static const int32_t kShortTermBandwidthItems = 3;
    static const int32_t kMinBandwidthHistoryItems = 20;
    static const int64_t kMinBandwidthHistoryWindowUs = 5000000LL; // 5 sec
    static const int64_t kMaxBandwidthHistoryWindowUs = 30000000LL; // 30 sec
    static const int64_t kMaxBandwidthHistoryAgeUs = 60000000LL; // 60 sec

    struct BandwidthEntry {
        int64_t mTimestampUs;
        int64_t mDelayUs;
        size_t mNumBytes;
    };

    Mutex mLock;
    List<BandwidthEntry> mBandwidthHistory;
    List<int32_t> mPrevEstimates;
    int32_t mShortTermEstimate;
    bool mHasNewSample;
    bool mIsStable;
    int64_t mTotalTransferTimeUs;
    size_t mTotalTransferBytes;

    DISALLOW_EVIL_CONSTRUCTORS(BandwidthEstimator);
};

LiveSession::BandwidthEstimator::BandwidthEstimator() :
    mShortTermEstimate(0),
    mHasNewSample(false),
    mIsStable(true),
    mTotalTransferTimeUs(0),
    mTotalTransferBytes(0) {
}

void LiveSession::BandwidthEstimator::addBandwidthMeasurement(
        size_t numBytes, int64_t delayUs) {
    AutoMutex autoLock(mLock);

    int64_t nowUs = ALooper::GetNowUs();
    BandwidthEntry entry;
    entry.mTimestampUs = nowUs;
    entry.mDelayUs = delayUs;
    entry.mNumBytes = numBytes;
    mTotalTransferTimeUs += delayUs;
    mTotalTransferBytes += numBytes;
    mBandwidthHistory.push_back(entry);
    mHasNewSample = true;

    // Remove no more than 10% of total transfer time at a time
    // to avoid sudden jump on bandwidth estimation. There might
    // be long blocking reads that takes up signification time,
    // we have to keep a longer window in that case.
    int64_t bandwidthHistoryWindowUs = mTotalTransferTimeUs * 9 / 10;
    if (bandwidthHistoryWindowUs < kMinBandwidthHistoryWindowUs) {
        bandwidthHistoryWindowUs = kMinBandwidthHistoryWindowUs;
    } else if (bandwidthHistoryWindowUs > kMaxBandwidthHistoryWindowUs) {
        bandwidthHistoryWindowUs = kMaxBandwidthHistoryWindowUs;
    }
    // trim old samples, keeping at least kMaxBandwidthHistoryItems samples,
    // and total transfer time at least kMaxBandwidthHistoryWindowUs.
    while (mBandwidthHistory.size() > kMinBandwidthHistoryItems) {
        List<BandwidthEntry>::iterator it = mBandwidthHistory.begin();
        // remove sample if either absolute age or total transfer time is
        // over kMaxBandwidthHistoryWindowUs
        if (nowUs - it->mTimestampUs < kMaxBandwidthHistoryAgeUs &&
                mTotalTransferTimeUs - it->mDelayUs < bandwidthHistoryWindowUs) {
            break;
        }
        mTotalTransferTimeUs -= it->mDelayUs;
        mTotalTransferBytes -= it->mNumBytes;
        mBandwidthHistory.erase(mBandwidthHistory.begin());
    }
}

bool LiveSession::BandwidthEstimator::estimateBandwidth(
        int32_t *bandwidthBps, bool *isStable, int32_t *shortTermBps) {
    AutoMutex autoLock(mLock);

    if (mBandwidthHistory.size() < 2) {
        return false;
    }

    if (!mHasNewSample) {
        *bandwidthBps = *(--mPrevEstimates.end());
        if (isStable) {
            *isStable = mIsStable;
        }
        if (shortTermBps) {
            *shortTermBps = mShortTermEstimate;
        }
        return true;
    }

    *bandwidthBps = ((double)mTotalTransferBytes * 8E6 / mTotalTransferTimeUs);
    mPrevEstimates.push_back(*bandwidthBps);
    while (mPrevEstimates.size() > 3) {
        mPrevEstimates.erase(mPrevEstimates.begin());
    }
    mHasNewSample = false;

    int64_t totalTimeUs = 0;
    size_t totalBytes = 0;
    if (mBandwidthHistory.size() >= kShortTermBandwidthItems) {
        List<BandwidthEntry>::iterator it = --mBandwidthHistory.end();
        for (size_t i = 0; i < kShortTermBandwidthItems; i++, it--) {
            totalTimeUs += it->mDelayUs;
            totalBytes += it->mNumBytes;
        }
    }
    mShortTermEstimate = totalTimeUs > 0 ?
            (totalBytes * 8E6 / totalTimeUs) : *bandwidthBps;
    if (shortTermBps) {
        *shortTermBps = mShortTermEstimate;
    }

    int64_t minEstimate = -1, maxEstimate = -1;
    List<int32_t>::iterator it;
    for (it = mPrevEstimates.begin(); it != mPrevEstimates.end(); it++) {
        int32_t estimate = *it;
        if (minEstimate < 0 || minEstimate > estimate) {
            minEstimate = estimate;
        }
        if (maxEstimate < 0 || maxEstimate < estimate) {
            maxEstimate = estimate;
        }
    }
    // consider it stable if long-term average is not jumping a lot
    // and short-term average is not much lower than long-term average
    mIsStable = (maxEstimate <= minEstimate * 4 / 3)
            && mShortTermEstimate > minEstimate * 7 / 10;
    if (isStable) {
        *isStable = mIsStable;
    }

#if 0
    {
        char dumpStr[1024] = {0};
        size_t itemIdx = 0;
        size_t histSize = mBandwidthHistory.size();
        sprintf(dumpStr, "estimate bps=%d stable=%d history (n=%d): {",
            *bandwidthBps, mIsStable, histSize);
        List<BandwidthEntry>::iterator it = mBandwidthHistory.begin();
        for (; it != mBandwidthHistory.end(); ++it) {
            if (itemIdx > 50) {
                sprintf(dumpStr + strlen(dumpStr),
                        "...(%zd more items)... }", histSize - itemIdx);
                break;
            }
            sprintf(dumpStr + strlen(dumpStr), "%dk/%.3fs%s",
                it->mNumBytes / 1024,
                (double)it->mDelayUs * 1.0e-6,
                (it == (--mBandwidthHistory.end())) ? "}" : ", ");
            itemIdx++;
        }
        ALOGE(dumpStr);
    }
#endif
    return true;
}

//static
const char *LiveSession::getKeyForStream(StreamType type) {
    switch (type) {
        case STREAMTYPE_VIDEO:
            return "timeUsVideo";
        case STREAMTYPE_AUDIO:
            return "timeUsAudio";
        case STREAMTYPE_SUBTITLES:
            return "timeUsSubtitle";
        case STREAMTYPE_METADATA:
            return "timeUsMetadata"; // unused
        default:
            TRESPASS();
    }
    return NULL;
}

//static
const char *LiveSession::getNameForStream(StreamType type) {
    switch (type) {
        case STREAMTYPE_VIDEO:
            return "video";
        case STREAMTYPE_AUDIO:
            return "audio";
        case STREAMTYPE_SUBTITLES:
            return "subs";
        case STREAMTYPE_METADATA:
            return "metadata";
        default:
            break;
    }
    return "unknown";
}

//static
ATSParser::SourceType LiveSession::getSourceTypeForStream(StreamType type) {
    switch (type) {
        case STREAMTYPE_VIDEO:
            return ATSParser::VIDEO;
        case STREAMTYPE_AUDIO:
            return ATSParser::AUDIO;
        case STREAMTYPE_METADATA:
            return ATSParser::META;
        case STREAMTYPE_SUBTITLES:
        default:
            TRESPASS();
    }
    return ATSParser::NUM_SOURCE_TYPES; // should not reach here
}

LiveSession::LiveSession(
        const sp<AMessage> &notify, uint32_t flags,
        const sp<MediaHTTPService> &httpService)
    : mNotify(notify),
      mFlags(flags),
      mHTTPService(httpService),
      mBuffering(false),
      mInPreparationPhase(true),
      mPollBufferingGeneration(0),
      mPrevBufferPercentage(-1),
      mCurBandwidthIndex(-1),
      mOrigBandwidthIndex(-1),
      mLastBandwidthBps(-1LL),
      mLastBandwidthStable(false),
      mBandwidthEstimator(new BandwidthEstimator()),
      mMaxWidth(720),
      mMaxHeight(480),
      mStreamMask(0),
      mNewStreamMask(0),
      mSwapMask(0),
      mSwitchGeneration(0),
      mSubtitleGeneration(0),
      mLastDequeuedTimeUs(0LL),
      mRealTimeBaseUs(0LL),
      mReconfigurationInProgress(false),
      mSwitchInProgress(false),
      mUpSwitchMark(kUpSwitchMarkUs),
      mDownSwitchMark(kDownSwitchMarkUs),
      mUpSwitchMargin(kUpSwitchMarginUs),
      mFirstTimeUsValid(false),
      mFirstTimeUs(0),
      mLastSeekTimeUs(0),
      mHasMetadata(false) {
    mStreams[kAudioIndex] = StreamItem("audio");
    mStreams[kVideoIndex] = StreamItem("video");
    mStreams[kSubtitleIndex] = StreamItem("subtitles");

    for (size_t i = 0; i < kNumSources; ++i) {
        mPacketSources.add(indexToType(i), new AnotherPacketSource(NULL /* meta */));
        mPacketSources2.add(indexToType(i), new AnotherPacketSource(NULL /* meta */));
    }
}

LiveSession::~LiveSession() {
    if (mFetcherLooper != NULL) {
        mFetcherLooper->stop();
    }
}

int64_t LiveSession::calculateMediaTimeUs(
        int64_t firstTimeUs, int64_t timeUs, int32_t discontinuitySeq) {
    if (timeUs >= firstTimeUs) {
        timeUs -= firstTimeUs;
    } else {
        timeUs = 0;
    }
    timeUs += mLastSeekTimeUs;
    if (mDiscontinuityOffsetTimesUs.indexOfKey(discontinuitySeq) >= 0) {
        timeUs += mDiscontinuityOffsetTimesUs.valueFor(discontinuitySeq);
    }
    return timeUs;
}

status_t LiveSession::dequeueAccessUnit(
        StreamType stream, sp<ABuffer> *accessUnit) {
    status_t finalResult = OK;
    sp<AnotherPacketSource> packetSource = mPacketSources.valueFor(stream);

    ssize_t streamIdx = typeToIndex(stream);
    if (streamIdx < 0) {
        return BAD_VALUE;
    }
    const char *streamStr = getNameForStream(stream);
    // Do not let client pull data if we don't have data packets yet.
    // We might only have a format discontinuity queued without data.
    // When NuPlayerDecoder dequeues the format discontinuity, it will
    // immediately try to getFormat. If we return NULL, NuPlayerDecoder
    // thinks it can do seamless change, so will not shutdown decoder.
    // When the actual format arrives, it can't handle it and get stuck.
    if (!packetSource->hasDataBufferAvailable(&finalResult)) {
        ALOGV("[%s] dequeueAccessUnit: no buffer available (finalResult=%d)",
                streamStr, finalResult);

        if (finalResult == OK) {
            return -EAGAIN;
        } else {
            return finalResult;
        }
    }

    // Let the client dequeue as long as we have buffers available
    // Do not make pause/resume decisions here.

    status_t err = packetSource->dequeueAccessUnit(accessUnit);

    if (err == INFO_DISCONTINUITY) {
        // adaptive streaming, discontinuities in the playlist
        int32_t type;
        CHECK((*accessUnit)->meta()->findInt32("discontinuity", &type));

        sp<AMessage> extra;
        if (!(*accessUnit)->meta()->findMessage("extra", &extra)) {
            extra.clear();
        }

        ALOGI("[%s] read discontinuity of type %d, extra = %s",
              streamStr,
              type,
              extra == NULL ? "NULL" : extra->debugString().c_str());
    } else if (err == OK) {

        if (stream == STREAMTYPE_AUDIO || stream == STREAMTYPE_VIDEO) {
            int64_t timeUs, originalTimeUs;
            int32_t discontinuitySeq = 0;
            StreamItem& strm = mStreams[streamIdx];
            CHECK((*accessUnit)->meta()->findInt64("timeUs",  &timeUs));
            originalTimeUs = timeUs;
            (*accessUnit)->meta()->findInt32("discontinuitySeq", &discontinuitySeq);
            if (discontinuitySeq > (int32_t) strm.mCurDiscontinuitySeq) {
                int64_t offsetTimeUs;
                if (mDiscontinuityOffsetTimesUs.indexOfKey(strm.mCurDiscontinuitySeq) >= 0) {
                    offsetTimeUs = mDiscontinuityOffsetTimesUs.valueFor(strm.mCurDiscontinuitySeq);
                } else {
                    offsetTimeUs = 0;
                }

                if (mDiscontinuityAbsStartTimesUs.indexOfKey(strm.mCurDiscontinuitySeq) >= 0
                        && strm.mLastDequeuedTimeUs >= 0) {
                    int64_t firstTimeUs;
                    firstTimeUs = mDiscontinuityAbsStartTimesUs.valueFor(strm.mCurDiscontinuitySeq);
                    offsetTimeUs += strm.mLastDequeuedTimeUs - firstTimeUs;
                    offsetTimeUs += strm.mLastSampleDurationUs;
                } else {
                    offsetTimeUs += strm.mLastSampleDurationUs;
                }

                mDiscontinuityOffsetTimesUs.add(discontinuitySeq, offsetTimeUs);
                strm.mCurDiscontinuitySeq = discontinuitySeq;
            }

            int32_t discard = 0;
            int64_t firstTimeUs;
            if (mDiscontinuityAbsStartTimesUs.indexOfKey(strm.mCurDiscontinuitySeq) >= 0) {
                int64_t durUs; // approximate sample duration
                if (timeUs > strm.mLastDequeuedTimeUs) {
                    durUs = timeUs - strm.mLastDequeuedTimeUs;
                } else {
                    durUs = strm.mLastDequeuedTimeUs - timeUs;
                }
                strm.mLastSampleDurationUs = durUs;
                firstTimeUs = mDiscontinuityAbsStartTimesUs.valueFor(strm.mCurDiscontinuitySeq);
            } else if ((*accessUnit)->meta()->findInt32("discard", &discard) && discard) {
                firstTimeUs = timeUs;
            } else {
                mDiscontinuityAbsStartTimesUs.add(strm.mCurDiscontinuitySeq, timeUs);
                firstTimeUs = timeUs;
            }

            strm.mLastDequeuedTimeUs = timeUs;
            timeUs = calculateMediaTimeUs(firstTimeUs, timeUs, discontinuitySeq);

            ALOGV("[%s] dequeueAccessUnit: time %lld us, original %lld us",
                    streamStr, (long long)timeUs, (long long)originalTimeUs);
            (*accessUnit)->meta()->setInt64("timeUs",  timeUs);
            mLastDequeuedTimeUs = timeUs;
            mRealTimeBaseUs = ALooper::GetNowUs() - timeUs;
        } else if (stream == STREAMTYPE_SUBTITLES) {
            int32_t subtitleGeneration;
            if ((*accessUnit)->meta()->findInt32("subtitleGeneration", &subtitleGeneration)
                    && subtitleGeneration != mSubtitleGeneration) {
               return -EAGAIN;
            };
            (*accessUnit)->meta()->setInt32(
                    "track-index", mPlaylist->getSelectedIndex());
            (*accessUnit)->meta()->setInt64("baseUs", mRealTimeBaseUs);
        } else if (stream == STREAMTYPE_METADATA) {
            HLSTime mdTime((*accessUnit)->meta());
            if (mDiscontinuityAbsStartTimesUs.indexOfKey(mdTime.mSeq) < 0) {
                packetSource->requeueAccessUnit((*accessUnit));
                return -EAGAIN;
            } else {
                int64_t firstTimeUs = mDiscontinuityAbsStartTimesUs.valueFor(mdTime.mSeq);
                int64_t timeUs = calculateMediaTimeUs(firstTimeUs, mdTime.mTimeUs, mdTime.mSeq);
                (*accessUnit)->meta()->setInt64("timeUs",  timeUs);
                (*accessUnit)->meta()->setInt64("baseUs", mRealTimeBaseUs);
            }
        }
    } else {
        ALOGI("[%s] encountered error %d", streamStr, err);
    }

    return err;
}

status_t LiveSession::getStreamFormatMeta(StreamType stream, sp<MetaData> *meta) {
    if (!(mStreamMask & stream)) {
        return UNKNOWN_ERROR;
    }

    sp<AnotherPacketSource> packetSource = mPacketSources.valueFor(stream);

    *meta = packetSource->getFormat();

    if (*meta == NULL) {
        return -EWOULDBLOCK;
    }

    if (stream == STREAMTYPE_AUDIO) {
        // set AAC input buffer size to 32K bytes (256kbps x 1sec)
        (*meta)->setInt32(kKeyMaxInputSize, 32 * 1024);
    } else if (stream == STREAMTYPE_VIDEO) {
        (*meta)->setInt32(kKeyMaxWidth, mMaxWidth);
        (*meta)->setInt32(kKeyMaxHeight, mMaxHeight);
    }

    return OK;
}

sp<HTTPDownloader> LiveSession::getHTTPDownloader() {
    return new HTTPDownloader(mHTTPService, mExtraHeaders);
}

void LiveSession::setBufferingSettings(
        const BufferingSettings &buffering) {
    sp<AMessage> msg = new AMessage(kWhatSetBufferingSettings, this);
    writeToAMessage(msg, buffering);
    msg->post();
}

void LiveSession::connectAsync(
        const char *url, const KeyedVector<String8, String8> *headers) {
    sp<AMessage> msg = new AMessage(kWhatConnect, this);
    msg->setString("url", url);

    if (headers != NULL) {
        msg->setPointer(
                "headers",
                new KeyedVector<String8, String8>(*headers));
    }

    msg->post();
}

status_t LiveSession::disconnect() {
    sp<AMessage> msg = new AMessage(kWhatDisconnect, this);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t LiveSession::seekTo(int64_t timeUs, MediaPlayerSeekMode mode) {
    sp<AMessage> msg = new AMessage(kWhatSeek, this);
    msg->setInt64("timeUs", timeUs);
    msg->setInt32("mode", mode);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

bool LiveSession::checkSwitchProgress(
        sp<AMessage> &stopParams, int64_t delayUs, bool *needResumeUntil) {
    AString newUri;
    CHECK(stopParams->findString("uri", &newUri));

    *needResumeUntil = false;
    sp<AMessage> firstNewMeta[kMaxStreams];
    for (size_t i = 0; i < kMaxStreams; ++i) {
        StreamType stream = indexToType(i);
        if (!(mSwapMask & mNewStreamMask & stream)
            || (mStreams[i].mNewUri != newUri)) {
            continue;
        }
        if (stream == STREAMTYPE_SUBTITLES) {
            continue;
        }
        sp<AnotherPacketSource> &source = mPacketSources.editValueAt(i);

        // First, get latest dequeued meta, which is where the decoder is at.
        // (when upswitching, we take the meta after a certain delay, so that
        // the decoder is left with some cushion)
        sp<AMessage> lastDequeueMeta, lastEnqueueMeta;
        if (delayUs > 0) {
            lastDequeueMeta = source->getMetaAfterLastDequeued(delayUs);
            if (lastDequeueMeta == NULL) {
                // this means we don't have enough cushion, try again later
                ALOGV("[%s] up switching failed due to insufficient buffer",
                        getNameForStream(stream));
                return false;
            }
        } else {
            // It's okay for lastDequeueMeta to be NULL here, it means the
            // decoder hasn't even started dequeueing
            lastDequeueMeta = source->getLatestDequeuedMeta();
        }
        // Then, trim off packets at beginning of mPacketSources2 that's before
        // the latest dequeued time. These samples are definitely too late.
        firstNewMeta[i] = mPacketSources2.editValueAt(i)
                            ->trimBuffersBeforeMeta(lastDequeueMeta);

        // Now firstNewMeta[i] is the first sample after the trim.
        // If it's NULL, we failed because dequeue already past all samples
        // in mPacketSource2, we have to try again.
        if (firstNewMeta[i] == NULL) {
            HLSTime dequeueTime(lastDequeueMeta);
            ALOGV("[%s] dequeue time (%d, %lld) past start time",
                    getNameForStream(stream),
                    dequeueTime.mSeq, (long long) dequeueTime.mTimeUs);
            return false;
        }

        // Otherwise, we check if mPacketSources2 overlaps with what old fetcher
        // already fetched, and see if we need to resumeUntil
        lastEnqueueMeta = source->getLatestEnqueuedMeta();
        // lastEnqueueMeta == NULL means old fetcher stopped at a discontinuity
        // boundary, no need to resume as the content will look different anyways
        if (lastEnqueueMeta != NULL) {
            HLSTime lastTime(lastEnqueueMeta), startTime(firstNewMeta[i]);

            // no need to resume old fetcher if new fetcher started in different
            // discontinuity sequence, as the content will look different.
            *needResumeUntil |= (startTime.mSeq == lastTime.mSeq
                    && startTime.mTimeUs - lastTime.mTimeUs > kResumeThresholdUs);

            // update the stopTime for resumeUntil
            stopParams->setInt32("discontinuitySeq", startTime.mSeq);
            stopParams->setInt64(getKeyForStream(stream), startTime.mTimeUs);
        }
    }

    // if we're here, it means dequeue progress hasn't passed some samples in
    // mPacketSource2, we can trim off the excess in mPacketSource.
    // (old fetcher might still need to resumeUntil the start time of new fetcher)
    for (size_t i = 0; i < kMaxStreams; ++i) {
        StreamType stream = indexToType(i);
        if (!(mSwapMask & mNewStreamMask & stream)
            || (newUri != mStreams[i].mNewUri)
            || stream == STREAMTYPE_SUBTITLES) {
            continue;
        }
        mPacketSources.valueFor(stream)->trimBuffersAfterMeta(firstNewMeta[i]);
    }

    // no resumeUntil if already underflow
    *needResumeUntil &= !mBuffering;

    return true;
}

void LiveSession::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSetBufferingSettings:
        {
            readFromAMessage(msg, &mBufferingSettings);
            break;
        }

        case kWhatConnect:
        {
            onConnect(msg);
            break;
        }

        case kWhatDisconnect:
        {
            CHECK(msg->senderAwaitsResponse(&mDisconnectReplyID));

            if (mReconfigurationInProgress) {
                break;
            }

            finishDisconnect();
            break;
        }

        case kWhatSeek:
        {
            if (mReconfigurationInProgress) {
                msg->post(50000);
                break;
            }

            CHECK(msg->senderAwaitsResponse(&mSeekReplyID));
            mSeekReply = new AMessage;

            onSeek(msg);
            break;
        }

        case kWhatFetcherNotify:
        {
            int32_t what;
            CHECK(msg->findInt32("what", &what));

            switch (what) {
                case PlaylistFetcher::kWhatStarted:
                    break;
                case PlaylistFetcher::kWhatPaused:
                case PlaylistFetcher::kWhatStopped:
                {
                    AString uri;
                    CHECK(msg->findString("uri", &uri));
                    ssize_t index = mFetcherInfos.indexOfKey(uri);
                    if (index < 0) {
                        // ignore msgs from fetchers that's already gone
                        break;
                    }

                    ALOGV("fetcher-%d %s",
                            mFetcherInfos[index].mFetcher->getFetcherID(),
                            what == PlaylistFetcher::kWhatPaused ?
                                    "paused" : "stopped");

                    if (what == PlaylistFetcher::kWhatStopped) {
                        mFetcherLooper->unregisterHandler(
                                mFetcherInfos[index].mFetcher->id());
                        mFetcherInfos.removeItemsAt(index);
                    } else if (what == PlaylistFetcher::kWhatPaused) {
                        int32_t seekMode;
                        CHECK(msg->findInt32("seekMode", &seekMode));
                        for (size_t i = 0; i < kMaxStreams; ++i) {
                            if (mStreams[i].mUri == uri) {
                                mStreams[i].mSeekMode = (SeekMode) seekMode;
                            }
                        }
                    }

                    if (mContinuation != NULL) {
                        CHECK_GT(mContinuationCounter, 0u);
                        if (--mContinuationCounter == 0) {
                            mContinuation->post();
                        }
                        ALOGV("%zu fetcher(s) left", mContinuationCounter);
                    }
                    break;
                }

                case PlaylistFetcher::kWhatDurationUpdate:
                {
                    AString uri;
                    CHECK(msg->findString("uri", &uri));

                    int64_t durationUs;
                    CHECK(msg->findInt64("durationUs", &durationUs));

                    ssize_t index = mFetcherInfos.indexOfKey(uri);
                    if (index >= 0) {
                        FetcherInfo *info = &mFetcherInfos.editValueFor(uri);
                        info->mDurationUs = durationUs;
                    }
                    break;
                }

                case PlaylistFetcher::kWhatTargetDurationUpdate:
                {
                    int64_t targetDurationUs;
                    CHECK(msg->findInt64("targetDurationUs", &targetDurationUs));
                    mUpSwitchMark = min(kUpSwitchMarkUs, targetDurationUs * 7 / 4);
                    mDownSwitchMark = min(kDownSwitchMarkUs, targetDurationUs * 9 / 4);
                    mUpSwitchMargin = min(kUpSwitchMarginUs, targetDurationUs);
                    break;
                }

                case PlaylistFetcher::kWhatError:
                {
                    status_t err;
                    CHECK(msg->findInt32("err", &err));

                    ALOGE("XXX Received error %d from PlaylistFetcher.", err);

                    // handle EOS on subtitle tracks independently
                    AString uri;
                    if (err == ERROR_END_OF_STREAM && msg->findString("uri", &uri)) {
                        ssize_t i = mFetcherInfos.indexOfKey(uri);
                        if (i >= 0) {
                            const sp<PlaylistFetcher> &fetcher = mFetcherInfos.valueAt(i).mFetcher;
                            if (fetcher != NULL) {
                                uint32_t type = fetcher->getStreamTypeMask();
                                if (type == STREAMTYPE_SUBTITLES) {
                                    mPacketSources.valueFor(
                                            STREAMTYPE_SUBTITLES)->signalEOS(err);;
                                    break;
                                }
                            }
                        }
                    }

                    // remember the failure index (as mCurBandwidthIndex will be restored
                    // after cancelBandwidthSwitch()), and record last fail time
                    size_t failureIndex = mCurBandwidthIndex;
                    mBandwidthItems.editItemAt(
                            failureIndex).mLastFailureUs = ALooper::GetNowUs();

                    if (mSwitchInProgress) {
                        // if error happened when we switch to a variant, try fallback
                        // to other variant to save the session
                        if (tryBandwidthFallback()) {
                            break;
                        }
                    }

                    if (mInPreparationPhase) {
                        postPrepared(err);
                    }

                    cancelBandwidthSwitch();

                    mPacketSources.valueFor(STREAMTYPE_AUDIO)->signalEOS(err);

                    mPacketSources.valueFor(STREAMTYPE_VIDEO)->signalEOS(err);

                    mPacketSources.valueFor(
                            STREAMTYPE_SUBTITLES)->signalEOS(err);

                    postError(err);
                    break;
                }

                case PlaylistFetcher::kWhatStopReached:
                {
                    ALOGV("kWhatStopReached");

                    AString oldUri;
                    CHECK(msg->findString("uri", &oldUri));

                    ssize_t index = mFetcherInfos.indexOfKey(oldUri);
                    if (index < 0) {
                        break;
                    }

                    tryToFinishBandwidthSwitch(oldUri);
                    break;
                }

                case PlaylistFetcher::kWhatStartedAt:
                {
                    int32_t switchGeneration;
                    CHECK(msg->findInt32("switchGeneration", &switchGeneration));

                    ALOGV("kWhatStartedAt: switchGen=%d, mSwitchGen=%d",
                            switchGeneration, mSwitchGeneration);

                    if (switchGeneration != mSwitchGeneration) {
                        break;
                    }

                    AString uri;
                    CHECK(msg->findString("uri", &uri));

                    // mark new fetcher mToBeResumed
                    ssize_t index = mFetcherInfos.indexOfKey(uri);
                    if (index >= 0) {
                        mFetcherInfos.editValueAt(index).mToBeResumed = true;
                    }

                    // temporarily disable packet sources to be swapped to prevent
                    // NuPlayerDecoder from dequeuing while we check progress
                    for (size_t i = 0; i < mPacketSources.size(); ++i) {
                        if ((mSwapMask & mPacketSources.keyAt(i))
                                && uri == mStreams[i].mNewUri) {
                            mPacketSources.editValueAt(i)->enable(false);
                        }
                    }
                    bool switchUp = (mCurBandwidthIndex > mOrigBandwidthIndex);
                    // If switching up, require a cushion bigger than kUnderflowMark
                    // to avoid buffering immediately after the switch.
                    // (If we don't have that cushion we'd rather cancel and try again.)
                    int64_t delayUs =
                        switchUp ?
                            (kUnderflowMarkMs * 1000LL + 1000000LL)
                            : 0;
                    bool needResumeUntil = false;
                    sp<AMessage> stopParams = msg;
                    if (checkSwitchProgress(stopParams, delayUs, &needResumeUntil)) {
                        // playback time hasn't passed startAt time
                        if (!needResumeUntil) {
                            ALOGV("finish switch");
                            for (size_t i = 0; i < kMaxStreams; ++i) {
                                if ((mSwapMask & indexToType(i))
                                        && uri == mStreams[i].mNewUri) {
                                    // have to make a copy of mStreams[i].mUri because
                                    // tryToFinishBandwidthSwitch is modifying mStreams[]
                                    AString oldURI = mStreams[i].mUri;
                                    tryToFinishBandwidthSwitch(oldURI);
                                    break;
                                }
                            }
                        } else {
                            // startAt time is after last enqueue time
                            // Resume fetcher for the original variant; the resumed fetcher should
                            // continue until the timestamps found in msg, which is stored by the
                            // new fetcher to indicate where the new variant has started buffering.
                            ALOGV("finish switch with resumeUntilAsync");
                            for (size_t i = 0; i < mFetcherInfos.size(); i++) {
                                const FetcherInfo &info = mFetcherInfos.valueAt(i);
                                if (info.mToBeRemoved) {
                                    info.mFetcher->resumeUntilAsync(stopParams);
                                }
                            }
                        }
                    } else {
                        // playback time passed startAt time
                        if (switchUp) {
                            // if switching up, cancel and retry if condition satisfies again
                            ALOGV("cancel up switch because we're too late");
                            cancelBandwidthSwitch(true /* resume */);
                        } else {
                            ALOGV("retry down switch at next sample");
                            resumeFetcher(uri, mSwapMask, -1, true /* newUri */);
                        }
                    }
                    // re-enable all packet sources
                    for (size_t i = 0; i < mPacketSources.size(); ++i) {
                        mPacketSources.editValueAt(i)->enable(true);
                    }

                    break;
                }

                case PlaylistFetcher::kWhatPlaylistFetched:
                {
                    onMasterPlaylistFetched(msg);
                    break;
                }

                case PlaylistFetcher::kWhatMetadataDetected:
                {
                    if (!mHasMetadata) {
                        mHasMetadata = true;
                        sp<AMessage> notify = mNotify->dup();
                        notify->setInt32("what", kWhatMetadataDetected);
                        notify->post();
                    }
                    break;
                }

                default:
                    TRESPASS();
            }

            break;
        }

        case kWhatChangeConfiguration:
        {
            onChangeConfiguration(msg);
            break;
        }

        case kWhatChangeConfiguration2:
        {
            onChangeConfiguration2(msg);
            break;
        }

        case kWhatChangeConfiguration3:
        {
            onChangeConfiguration3(msg);
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
            TRESPASS();
            break;
    }
}

// static
bool LiveSession::isBandwidthValid(const BandwidthItem &item) {
    static const int64_t kBlacklistWindowUs = 300 * 1000000LL;
    return item.mLastFailureUs < 0
            || ALooper::GetNowUs() - item.mLastFailureUs > kBlacklistWindowUs;
}

// static
int LiveSession::SortByBandwidth(const BandwidthItem *a, const BandwidthItem *b) {
    if (a->mBandwidth < b->mBandwidth) {
        return -1;
    } else if (a->mBandwidth == b->mBandwidth) {
        return 0;
    }

    return 1;
}

// static
LiveSession::StreamType LiveSession::indexToType(int idx) {
    CHECK(idx >= 0 && idx < kNumSources);
    return (StreamType)(1 << idx);
}

// static
ssize_t LiveSession::typeToIndex(int32_t type) {
    switch (type) {
        case STREAMTYPE_AUDIO:
            return 0;
        case STREAMTYPE_VIDEO:
            return 1;
        case STREAMTYPE_SUBTITLES:
            return 2;
        case STREAMTYPE_METADATA:
            return 3;
        default:
            return -1;
    };
    return -1;
}

void LiveSession::onConnect(const sp<AMessage> &msg) {
    CHECK(msg->findString("url", &mMasterURL));

    // TODO currently we don't know if we are coming here from incognito mode
    ALOGI("onConnect %s", uriDebugString(mMasterURL).c_str());

    KeyedVector<String8, String8> *headers = NULL;
    if (!msg->findPointer("headers", (void **)&headers)) {
        mExtraHeaders.clear();
    } else {
        mExtraHeaders = *headers;

        delete headers;
        headers = NULL;
    }

    // create looper for fetchers
    if (mFetcherLooper == NULL) {
        mFetcherLooper = new ALooper();

        mFetcherLooper->setName("Fetcher");
        mFetcherLooper->start(false, /* runOnCallingThread */
                              true  /* canCallJava */);
    }

    // create fetcher to fetch the master playlist
    addFetcher(mMasterURL.c_str())->fetchPlaylistAsync();
}

void LiveSession::onMasterPlaylistFetched(const sp<AMessage> &msg) {
    AString uri;
    CHECK(msg->findString("uri", &uri));
    ssize_t index = mFetcherInfos.indexOfKey(uri);
    if (index < 0) {
        ALOGW("fetcher for master playlist is gone.");
        return;
    }

    // no longer useful, remove
    mFetcherLooper->unregisterHandler(mFetcherInfos[index].mFetcher->id());
    mFetcherInfos.removeItemsAt(index);

    CHECK(msg->findObject("playlist", (sp<RefBase> *)&mPlaylist));
    if (mPlaylist == NULL) {
        ALOGE("unable to fetch master playlist %s.",
                uriDebugString(mMasterURL).c_str());

        postPrepared(ERROR_IO);
        return;
    }
    // We trust the content provider to make a reasonable choice of preferred
    // initial bandwidth by listing it first in the variant playlist.
    // At startup we really don't have a good estimate on the available
    // network bandwidth since we haven't tranferred any data yet. Once
    // we have we can make a better informed choice.
    size_t initialBandwidth = 0;
    size_t initialBandwidthIndex = 0;

    int32_t maxWidth = 0;
    int32_t maxHeight = 0;

    if (mPlaylist->isVariantPlaylist()) {
        Vector<BandwidthItem> itemsWithVideo;
        for (size_t i = 0; i < mPlaylist->size(); ++i) {
            BandwidthItem item;

            item.mPlaylistIndex = i;
            item.mLastFailureUs = -1LL;

            sp<AMessage> meta;
            AString uri;
            mPlaylist->itemAt(i, &uri, &meta);

            CHECK(meta->findInt32("bandwidth", (int32_t *)&item.mBandwidth));

            int32_t width, height;
            if (meta->findInt32("width", &width)) {
                maxWidth = max(maxWidth, width);
            }
            if (meta->findInt32("height", &height)) {
                maxHeight = max(maxHeight, height);
            }

            mBandwidthItems.push(item);
            if (mPlaylist->hasType(i, "video")) {
                itemsWithVideo.push(item);
            }
        }
        // remove the audio-only variants if we have at least one with video
        if (!itemsWithVideo.empty()
                && itemsWithVideo.size() < mBandwidthItems.size()) {
            mBandwidthItems.clear();
            for (size_t i = 0; i < itemsWithVideo.size(); ++i) {
                mBandwidthItems.push(itemsWithVideo[i]);
            }
        }

        CHECK_GT(mBandwidthItems.size(), 0u);
        initialBandwidth = mBandwidthItems[0].mBandwidth;

        mBandwidthItems.sort(SortByBandwidth);

        for (size_t i = 0; i < mBandwidthItems.size(); ++i) {
            if (mBandwidthItems.itemAt(i).mBandwidth == initialBandwidth) {
                initialBandwidthIndex = i;
                break;
            }
        }
    } else {
        // dummy item.
        BandwidthItem item;
        item.mPlaylistIndex = 0;
        item.mBandwidth = 0;
        mBandwidthItems.push(item);
    }

    mMaxWidth = maxWidth > 0 ? maxWidth : mMaxWidth;
    mMaxHeight = maxHeight > 0 ? maxHeight : mMaxHeight;

    mPlaylist->pickRandomMediaItems();
    changeConfiguration(
            0LL /* timeUs */, initialBandwidthIndex, false /* pickTrack */);
}

void LiveSession::finishDisconnect() {
    ALOGV("finishDisconnect");

    // No reconfiguration is currently pending, make sure none will trigger
    // during disconnection either.
    cancelBandwidthSwitch();

    // cancel buffer polling
    cancelPollBuffering();

    // TRICKY: don't wait for all fetcher to be stopped when disconnecting
    //
    // Some fetchers might be stuck in connect/getSize at this point. These
    // operations will eventually timeout (as we have a timeout set in
    // MediaHTTPConnection), but we don't want to block the main UI thread
    // until then. Here we just need to make sure we clear all references
    // to the fetchers, so that when they finally exit from the blocking
    // operation, they can be destructed.
    //
    // There is one very tricky point though. For this scheme to work, the
    // fecther must hold a reference to LiveSession, so that LiveSession is
    // destroyed after fetcher. Otherwise LiveSession would get stuck in its
    // own destructor when it waits for mFetcherLooper to stop, which still
    // blocks main UI thread.
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        mFetcherInfos.valueAt(i).mFetcher->stopAsync();
        mFetcherLooper->unregisterHandler(
                mFetcherInfos.valueAt(i).mFetcher->id());
    }
    mFetcherInfos.clear();

    mPacketSources.valueFor(STREAMTYPE_AUDIO)->signalEOS(ERROR_END_OF_STREAM);
    mPacketSources.valueFor(STREAMTYPE_VIDEO)->signalEOS(ERROR_END_OF_STREAM);

    mPacketSources.valueFor(
            STREAMTYPE_SUBTITLES)->signalEOS(ERROR_END_OF_STREAM);

    sp<AMessage> response = new AMessage;
    response->setInt32("err", OK);

    response->postReply(mDisconnectReplyID);
    mDisconnectReplyID.clear();
}

sp<PlaylistFetcher> LiveSession::addFetcher(const char *uri) {
    ssize_t index = mFetcherInfos.indexOfKey(uri);

    if (index >= 0) {
        return NULL;
    }

    sp<AMessage> notify = new AMessage(kWhatFetcherNotify, this);
    notify->setString("uri", uri);
    notify->setInt32("switchGeneration", mSwitchGeneration);

    FetcherInfo info;
    info.mFetcher = new PlaylistFetcher(
            notify, this, uri, mCurBandwidthIndex, mSubtitleGeneration);
    info.mDurationUs = -1LL;
    info.mToBeRemoved = false;
    info.mToBeResumed = false;
    mFetcherLooper->registerHandler(info.mFetcher);

    mFetcherInfos.add(uri, info);

    return info.mFetcher;
}

#if 0
static double uniformRand() {
    return (double)rand() / RAND_MAX;
}
#endif

bool LiveSession::UriIsSameAsIndex(const AString &uri, int32_t i, bool newUri) {
    ALOGV("[timed_id3] i %d UriIsSameAsIndex newUri %s, %s", i,
            newUri ? "true" : "false",
            newUri ? mStreams[i].mNewUri.c_str() : mStreams[i].mUri.c_str());
    return i >= 0
            && ((!newUri && uri == mStreams[i].mUri)
            || (newUri && uri == mStreams[i].mNewUri));
}

sp<AnotherPacketSource> LiveSession::getPacketSourceForStreamIndex(
        size_t trackIndex, bool newUri) {
    StreamType type = indexToType(trackIndex);
    sp<AnotherPacketSource> source = NULL;
    if (newUri) {
        source = mPacketSources2.valueFor(type);
        source->clear();
    } else {
        source = mPacketSources.valueFor(type);
    };
    return source;
}

sp<AnotherPacketSource> LiveSession::getMetadataSource(
        sp<AnotherPacketSource> sources[kNumSources], uint32_t streamMask, bool newUri) {
    // todo: One case where the following strategy can fail is when audio and video
    // are in separate playlists, both are transport streams, and the metadata
    // is actually contained in the audio stream.
    ALOGV("[timed_id3] getMetadataSourceForUri streamMask %x newUri %s",
            streamMask, newUri ? "true" : "false");

    if ((sources[kVideoIndex] != NULL) // video fetcher; or ...
            || (!(streamMask & STREAMTYPE_VIDEO) && sources[kAudioIndex] != NULL)) {
            // ... audio fetcher for audio only variant
        return getPacketSourceForStreamIndex(kMetaDataIndex, newUri);
    }

    return NULL;
}

bool LiveSession::resumeFetcher(
        const AString &uri, uint32_t streamMask, int64_t timeUs, bool newUri) {
    ssize_t index = mFetcherInfos.indexOfKey(uri);
    if (index < 0) {
        ALOGE("did not find fetcher for uri: %s", uriDebugString(uri).c_str());
        return false;
    }

    bool resume = false;
    sp<AnotherPacketSource> sources[kNumSources];
    for (size_t i = 0; i < kMaxStreams; ++i) {
        if ((streamMask & indexToType(i)) && UriIsSameAsIndex(uri, i, newUri)) {
            resume = true;
            sources[i] = getPacketSourceForStreamIndex(i, newUri);
        }
    }

    if (resume) {
        sp<PlaylistFetcher> &fetcher = mFetcherInfos.editValueAt(index).mFetcher;
        SeekMode seekMode = newUri ? kSeekModeNextSample : kSeekModeExactPosition;

        ALOGV("resuming fetcher-%d, timeUs=%lld, seekMode=%d",
                fetcher->getFetcherID(), (long long)timeUs, seekMode);

        fetcher->startAsync(
                sources[kAudioIndex],
                sources[kVideoIndex],
                sources[kSubtitleIndex],
                getMetadataSource(sources, streamMask, newUri),
                timeUs, -1, -1, seekMode);
    }

    return resume;
}

float LiveSession::getAbortThreshold(
        ssize_t currentBWIndex, ssize_t targetBWIndex) const {
    float abortThreshold = -1.0f;
    if (currentBWIndex > 0 && targetBWIndex < currentBWIndex) {
        /*
           If we're switching down, we need to decide whether to

           1) finish last segment of high-bandwidth variant, or
           2) abort last segment of high-bandwidth variant, and fetch an
              overlapping portion from low-bandwidth variant.

           Here we try to maximize the amount of buffer left when the
           switch point is met. Given the following parameters:

           B: our current buffering level in seconds
           T: target duration in seconds
           X: sample duration in seconds remain to fetch in last segment
           bw0: bandwidth of old variant (as specified in playlist)
           bw1: bandwidth of new variant (as specified in playlist)
           bw: measured bandwidth available

           If we choose 1), when switch happens at the end of current
           segment, our buffering will be
                  B + X - X * bw0 / bw

           If we choose 2), when switch happens where we aborted current
           segment, our buffering will be
                  B - (T - X) * bw1 / bw

           We should only choose 1) if
                  X/T < bw1 / (bw1 + bw0 - bw)
        */

        // abort old bandwidth immediately if bandwidth is fluctuating a lot.
        // our estimate could be far off, and fetching old bandwidth could
        // take too long.
        if (!mLastBandwidthStable) {
            return 0.0f;
        }

        // Taking the measured current bandwidth at 50% face value only,
        // as our bandwidth estimation is a lagging indicator. Being
        // conservative on this, we prefer switching to lower bandwidth
        // unless we're really confident finishing up the last segment
        // of higher bandwidth will be fast.
        CHECK(mLastBandwidthBps >= 0);
        abortThreshold =
                (float)mBandwidthItems.itemAt(targetBWIndex).mBandwidth
             / ((float)mBandwidthItems.itemAt(targetBWIndex).mBandwidth
              + (float)mBandwidthItems.itemAt(currentBWIndex).mBandwidth
              - (float)mLastBandwidthBps * 0.5f);
        if (abortThreshold < 0.0f) {
            abortThreshold = -1.0f; // do not abort
        }
        ALOGV("Switching Down: bps %ld => %ld, measured %d, abort ratio %.2f",
                mBandwidthItems.itemAt(currentBWIndex).mBandwidth,
                mBandwidthItems.itemAt(targetBWIndex).mBandwidth,
                mLastBandwidthBps,
                abortThreshold);
    }
    return abortThreshold;
}

void LiveSession::addBandwidthMeasurement(size_t numBytes, int64_t delayUs) {
    mBandwidthEstimator->addBandwidthMeasurement(numBytes, delayUs);
}

ssize_t LiveSession::getLowestValidBandwidthIndex() const {
    for (size_t index = 0; index < mBandwidthItems.size(); index++) {
        if (isBandwidthValid(mBandwidthItems[index])) {
            return index;
        }
    }
    // if playlists are all blacklisted, return 0 and hope it's alive
    return 0;
}

size_t LiveSession::getBandwidthIndex(int32_t bandwidthBps) {
    if (mBandwidthItems.size() < 2) {
        // shouldn't be here if we only have 1 bandwidth, check
        // logic to get rid of redundant bandwidth polling
        ALOGW("getBandwidthIndex() called for single bandwidth playlist!");
        return 0;
    }

#if 1
    char value[PROPERTY_VALUE_MAX];
    ssize_t index = -1;
    if (property_get("media.httplive.bw-index", value, NULL)) {
        char *end;
        index = strtol(value, &end, 10);
        CHECK(end > value && *end == '\0');

        if (index >= 0 && (size_t)index >= mBandwidthItems.size()) {
            index = mBandwidthItems.size() - 1;
        }
    }

    if (index < 0) {
        char value[PROPERTY_VALUE_MAX];
        if (property_get("media.httplive.max-bw", value, NULL)) {
            char *end;
            long maxBw = strtoul(value, &end, 10);
            if (end > value && *end == '\0') {
                if (maxBw > 0 && bandwidthBps > maxBw) {
                    ALOGV("bandwidth capped to %ld bps", maxBw);
                    bandwidthBps = maxBw;
                }
            }
        }

        // Pick the highest bandwidth stream that's not currently blacklisted
        // below or equal to estimated bandwidth.

        index = mBandwidthItems.size() - 1;
        ssize_t lowestBandwidth = getLowestValidBandwidthIndex();
        while (index > lowestBandwidth) {
            // be conservative (70%) to avoid overestimating and immediately
            // switching down again.
            size_t adjustedBandwidthBps = bandwidthBps * .7f;
            const BandwidthItem &item = mBandwidthItems[index];
            if (item.mBandwidth <= adjustedBandwidthBps
                    && isBandwidthValid(item)) {
                break;
            }
            --index;
        }
    }
#elif 0
    // Change bandwidth at random()
    size_t index = uniformRand() * mBandwidthItems.size();
#elif 0
    // There's a 50% chance to stay on the current bandwidth and
    // a 50% chance to switch to the next higher bandwidth (wrapping around
    // to lowest)
    const size_t kMinIndex = 0;

    static ssize_t mCurBandwidthIndex = -1;

    size_t index;
    if (mCurBandwidthIndex < 0) {
        index = kMinIndex;
    } else if (uniformRand() < 0.5) {
        index = (size_t)mCurBandwidthIndex;
    } else {
        index = mCurBandwidthIndex + 1;
        if (index == mBandwidthItems.size()) {
            index = kMinIndex;
        }
    }
    mCurBandwidthIndex = index;
#elif 0
    // Pick the highest bandwidth stream below or equal to 1.2 Mbit/sec

    size_t index = mBandwidthItems.size() - 1;
    while (index > 0 && mBandwidthItems.itemAt(index).mBandwidth > 1200000) {
        --index;
    }
#elif 1
    char value[PROPERTY_VALUE_MAX];
    size_t index;
    if (property_get("media.httplive.bw-index", value, NULL)) {
        char *end;
        index = strtoul(value, &end, 10);
        CHECK(end > value && *end == '\0');

        if (index >= mBandwidthItems.size()) {
            index = mBandwidthItems.size() - 1;
        }
    } else {
        index = 0;
    }
#else
    size_t index = mBandwidthItems.size() - 1;  // Highest bandwidth stream
#endif

    CHECK_GE(index, 0);

    return index;
}

HLSTime LiveSession::latestMediaSegmentStartTime() const {
    HLSTime audioTime(mPacketSources.valueFor(
                    STREAMTYPE_AUDIO)->getLatestDequeuedMeta());

    HLSTime videoTime(mPacketSources.valueFor(
                    STREAMTYPE_VIDEO)->getLatestDequeuedMeta());

    return audioTime < videoTime ? videoTime : audioTime;
}

void LiveSession::onSeek(const sp<AMessage> &msg) {
    int64_t timeUs;
    int32_t mode;
    CHECK(msg->findInt64("timeUs", &timeUs));
    CHECK(msg->findInt32("mode", &mode));
    // TODO: add "mode" to changeConfiguration.
    changeConfiguration(timeUs/* , (MediaPlayerSeekMode)mode */);
}

status_t LiveSession::getDuration(int64_t *durationUs) const {
    int64_t maxDurationUs = -1LL;
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        int64_t fetcherDurationUs = mFetcherInfos.valueAt(i).mDurationUs;

        if (fetcherDurationUs > maxDurationUs) {
            maxDurationUs = fetcherDurationUs;
        }
    }

    *durationUs = maxDurationUs;

    return OK;
}

bool LiveSession::isSeekable() const {
    int64_t durationUs;
    return getDuration(&durationUs) == OK && durationUs >= 0;
}

bool LiveSession::hasDynamicDuration() const {
    return false;
}

size_t LiveSession::getTrackCount() const {
    if (mPlaylist == NULL) {
        return 0;
    } else {
        return mPlaylist->getTrackCount() + (mHasMetadata ? 1 : 0);
    }
}

sp<AMessage> LiveSession::getTrackInfo(size_t trackIndex) const {
    if (mPlaylist == NULL) {
        return NULL;
    } else {
        if (trackIndex == mPlaylist->getTrackCount() && mHasMetadata) {
            sp<AMessage> format = new AMessage();
            format->setInt32("type", MEDIA_TRACK_TYPE_METADATA);
            format->setString("language", "und");
            format->setString("mime", MEDIA_MIMETYPE_DATA_TIMED_ID3);
            return format;
        }
        return mPlaylist->getTrackInfo(trackIndex);
    }
}

status_t LiveSession::selectTrack(size_t index, bool select) {
    if (mPlaylist == NULL) {
        return INVALID_OPERATION;
    }

    ALOGV("selectTrack: index=%zu, select=%d, mSubtitleGen=%d++",
            index, select, mSubtitleGeneration);

    ++mSubtitleGeneration;
    status_t err = mPlaylist->selectTrack(index, select);
    if (err == OK) {
        sp<AMessage> msg = new AMessage(kWhatChangeConfiguration, this);
        msg->setInt32("pickTrack", select);
        msg->post();
    }
    return err;
}

ssize_t LiveSession::getSelectedTrack(media_track_type type) const {
    if (mPlaylist == NULL) {
        return -1;
    } else {
        return mPlaylist->getSelectedTrack(type);
    }
}

void LiveSession::changeConfiguration(
        int64_t timeUs, ssize_t bandwidthIndex, bool pickTrack) {
    ALOGV("changeConfiguration: timeUs=%lld us, bwIndex=%zd, pickTrack=%d",
          (long long)timeUs, bandwidthIndex, pickTrack);

    cancelBandwidthSwitch();

    CHECK(!mReconfigurationInProgress);
    mReconfigurationInProgress = true;
    if (bandwidthIndex >= 0) {
        mOrigBandwidthIndex = mCurBandwidthIndex;
        mCurBandwidthIndex = bandwidthIndex;
        if (mOrigBandwidthIndex != mCurBandwidthIndex) {
            ALOGI("#### Starting Bandwidth Switch: %zd => %zd",
                    mOrigBandwidthIndex, mCurBandwidthIndex);
        }
    }
    CHECK_LT((size_t)mCurBandwidthIndex, mBandwidthItems.size());
    const BandwidthItem &item = mBandwidthItems.itemAt(mCurBandwidthIndex);

    uint32_t streamMask = 0; // streams that should be fetched by the new fetcher
    uint32_t resumeMask = 0; // streams that should be fetched by the original fetcher

    AString URIs[kMaxStreams];
    for (size_t i = 0; i < kMaxStreams; ++i) {
        if (mPlaylist->getTypeURI(item.mPlaylistIndex, mStreams[i].mType, &URIs[i])) {
            streamMask |= indexToType(i);
        }
    }

    // Step 1, stop and discard fetchers that are no longer needed.
    // Pause those that we'll reuse.
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        // skip fetchers that are marked mToBeRemoved,
        // these are done and can't be reused
        if (mFetcherInfos[i].mToBeRemoved) {
            continue;
        }

        const AString &uri = mFetcherInfos.keyAt(i);
        sp<PlaylistFetcher> &fetcher = mFetcherInfos.editValueAt(i).mFetcher;

        bool discardFetcher = true, delayRemoval = false;
        for (size_t j = 0; j < kMaxStreams; ++j) {
            StreamType type = indexToType(j);
            if ((streamMask & type) && uri == URIs[j]) {
                resumeMask |= type;
                streamMask &= ~type;
                discardFetcher = false;
            }
        }
        // Delay fetcher removal if not picking tracks, AND old fetcher
        // has stream mask that overlaps new variant. (Okay to discard
        // old fetcher now, if completely no overlap.)
        if (discardFetcher && timeUs < 0LL && !pickTrack
                && (fetcher->getStreamTypeMask() & streamMask)) {
            discardFetcher = false;
            delayRemoval = true;
        }

        if (discardFetcher) {
            ALOGV("discarding fetcher-%d", fetcher->getFetcherID());
            fetcher->stopAsync();
        } else {
            float threshold = 0.0f; // default to pause after current block (47Kbytes)
            bool disconnect = false;
            if (timeUs >= 0LL) {
                // seeking, no need to finish fetching
                disconnect = true;
            } else if (delayRemoval) {
                // adapting, abort if remaining of current segment is over threshold
                threshold = getAbortThreshold(
                        mOrigBandwidthIndex, mCurBandwidthIndex);
            }

            ALOGV("pausing fetcher-%d, threshold=%.2f",
                    fetcher->getFetcherID(), threshold);
            fetcher->pauseAsync(threshold, disconnect);
        }
    }

    sp<AMessage> msg;
    if (timeUs < 0LL) {
        // skip onChangeConfiguration2 (decoder destruction) if not seeking.
        msg = new AMessage(kWhatChangeConfiguration3, this);
    } else {
        msg = new AMessage(kWhatChangeConfiguration2, this);
    }
    msg->setInt32("streamMask", streamMask);
    msg->setInt32("resumeMask", resumeMask);
    msg->setInt32("pickTrack", pickTrack);
    msg->setInt64("timeUs", timeUs);
    for (size_t i = 0; i < kMaxStreams; ++i) {
        if ((streamMask | resumeMask) & indexToType(i)) {
            msg->setString(mStreams[i].uriKey().c_str(), URIs[i].c_str());
        }
    }

    // Every time a fetcher acknowledges the stopAsync or pauseAsync request
    // we'll decrement mContinuationCounter, once it reaches zero, i.e. all
    // fetchers have completed their asynchronous operation, we'll post
    // mContinuation, which then is handled below in onChangeConfiguration2.
    mContinuationCounter = mFetcherInfos.size();
    mContinuation = msg;

    if (mContinuationCounter == 0) {
        msg->post();
    }
}

void LiveSession::onChangeConfiguration(const sp<AMessage> &msg) {
    ALOGV("onChangeConfiguration");

    if (!mReconfigurationInProgress) {
        int32_t pickTrack = 0;
        msg->findInt32("pickTrack", &pickTrack);
        changeConfiguration(-1LL /* timeUs */, -1, pickTrack);
    } else {
        msg->post(1000000LL); // retry in 1 sec
    }
}

void LiveSession::onChangeConfiguration2(const sp<AMessage> &msg) {
    ALOGV("onChangeConfiguration2");

    mContinuation.clear();

    // All fetchers are either suspended or have been removed now.

    // If we're seeking, clear all packet sources before we report
    // seek complete, to prevent decoder from pulling stale data.
    int64_t timeUs;
    CHECK(msg->findInt64("timeUs", &timeUs));

    if (timeUs >= 0) {
        mLastSeekTimeUs = timeUs;
        mLastDequeuedTimeUs = timeUs;

        for (size_t i = 0; i < mPacketSources.size(); i++) {
            sp<AnotherPacketSource> packetSource = mPacketSources.editValueAt(i);
            sp<MetaData> format = packetSource->getFormat();
            packetSource->clear();
            // Set a tentative format here such that HTTPLiveSource will always have
            // a format available when NuPlayer queries. Without an available video
            // format when setting a surface NuPlayer might disable video decoding
            // altogether. The tentative format will be overwritten by the
            // authoritative (and possibly same) format once content from the new
            // position is dequeued.
            packetSource->setFormat(format);
        }

        for (size_t i = 0; i < kMaxStreams; ++i) {
            mStreams[i].reset();
        }

        mDiscontinuityOffsetTimesUs.clear();
        mDiscontinuityAbsStartTimesUs.clear();

        if (mSeekReplyID != NULL) {
            CHECK(mSeekReply != NULL);
            mSeekReply->setInt32("err", OK);
            mSeekReply->postReply(mSeekReplyID);
            mSeekReplyID.clear();
            mSeekReply.clear();
        }

        // restart buffer polling after seek becauese previous
        // buffering position is no longer valid.
        restartPollBuffering();
    }

    uint32_t streamMask, resumeMask;
    CHECK(msg->findInt32("streamMask", (int32_t *)&streamMask));
    CHECK(msg->findInt32("resumeMask", (int32_t *)&resumeMask));

    streamMask |= resumeMask;

    AString URIs[kMaxStreams];
    for (size_t i = 0; i < kMaxStreams; ++i) {
        if (streamMask & indexToType(i)) {
            const AString &uriKey = mStreams[i].uriKey();
            CHECK(msg->findString(uriKey.c_str(), &URIs[i]));
            ALOGV("%s = '%s'", uriKey.c_str(), URIs[i].c_str());
        }
    }

    uint32_t changedMask = 0;
    for (size_t i = 0; i < kMaxStreams && i != kSubtitleIndex; ++i) {
        // stream URI could change even if onChangeConfiguration2 is only
        // used for seek. Seek could happen during a bw switch, in this
        // case bw switch will be cancelled, but the seekTo position will
        // fetch from the new URI.
        if ((mStreamMask & streamMask & indexToType(i))
                && !mStreams[i].mUri.empty()
                && !(URIs[i] == mStreams[i].mUri)) {
            ALOGV("stream %zu changed: oldURI %s, newURI %s", i,
                    mStreams[i].mUri.c_str(), URIs[i].c_str());
            sp<AnotherPacketSource> source = mPacketSources.valueFor(indexToType(i));
            if (source->getLatestDequeuedMeta() != NULL) {
                source->queueDiscontinuity(
                        ATSParser::DISCONTINUITY_FORMATCHANGE, NULL, true);
            }
        }
        // Determine which decoders to shutdown on the player side,
        // a decoder has to be shutdown if its streamtype was active
        // before but now longer isn't.
        if ((mStreamMask & ~streamMask & indexToType(i))) {
            changedMask |= indexToType(i);
        }
    }

    if (changedMask == 0) {
        // If nothing changed as far as the audio/video decoders
        // are concerned we can proceed.
        onChangeConfiguration3(msg);
        return;
    }

    // Something changed, inform the player which will shutdown the
    // corresponding decoders and will post the reply once that's done.
    // Handling the reply will continue executing below in
    // onChangeConfiguration3.
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatStreamsChanged);
    notify->setInt32("changedMask", changedMask);

    msg->setWhat(kWhatChangeConfiguration3);
    msg->setTarget(this);

    notify->setMessage("reply", msg);
    notify->post();
}

void LiveSession::onChangeConfiguration3(const sp<AMessage> &msg) {
    mContinuation.clear();
    // All remaining fetchers are still suspended, the player has shutdown
    // any decoders that needed it.

    uint32_t streamMask, resumeMask;
    CHECK(msg->findInt32("streamMask", (int32_t *)&streamMask));
    CHECK(msg->findInt32("resumeMask", (int32_t *)&resumeMask));

    mNewStreamMask = streamMask | resumeMask;

    int64_t timeUs;
    int32_t pickTrack;
    bool switching = false;
    CHECK(msg->findInt64("timeUs", &timeUs));
    CHECK(msg->findInt32("pickTrack", &pickTrack));

    if (timeUs < 0LL) {
        if (!pickTrack) {
            // mSwapMask contains streams that are in both old and new variant,
            // (in mNewStreamMask & mStreamMask) but with different URIs
            // (not in resumeMask).
            // For example, old variant has video and audio in two separate
            // URIs, and new variant has only audio with unchanged URI. mSwapMask
            // should be 0 as there is nothing to swap. We only need to stop video,
            // and resume audio.
            mSwapMask =  mNewStreamMask & mStreamMask & ~resumeMask;
            switching = (mSwapMask != 0);
        }
        mRealTimeBaseUs = ALooper::GetNowUs() - mLastDequeuedTimeUs;
    } else {
        mRealTimeBaseUs = ALooper::GetNowUs() - timeUs;
    }

    ALOGV("onChangeConfiguration3: timeUs=%lld, switching=%d, pickTrack=%d, "
            "mStreamMask=0x%x, mNewStreamMask=0x%x, mSwapMask=0x%x",
            (long long)timeUs, switching, pickTrack,
            mStreamMask, mNewStreamMask, mSwapMask);

    for (size_t i = 0; i < kMaxStreams; ++i) {
        if (streamMask & indexToType(i)) {
            if (switching) {
                CHECK(msg->findString(mStreams[i].uriKey().c_str(), &mStreams[i].mNewUri));
            } else {
                CHECK(msg->findString(mStreams[i].uriKey().c_str(), &mStreams[i].mUri));
            }
        }
    }

    // Of all existing fetchers:
    // * Resume fetchers that are still needed and assign them original packet sources.
    // * Mark otherwise unneeded fetchers for removal.
    ALOGV("resuming fetchers for mask 0x%08x", resumeMask);
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        const AString &uri = mFetcherInfos.keyAt(i);
        if (!resumeFetcher(uri, resumeMask, timeUs)) {
            ALOGV("marking fetcher-%d to be removed",
                    mFetcherInfos[i].mFetcher->getFetcherID());

            mFetcherInfos.editValueAt(i).mToBeRemoved = true;
        }
    }

    // streamMask now only contains the types that need a new fetcher created.
    if (streamMask != 0) {
        ALOGV("creating new fetchers for mask 0x%08x", streamMask);
    }

    // Find out when the original fetchers have buffered up to and start the new fetchers
    // at a later timestamp.
    for (size_t i = 0; i < kMaxStreams; i++) {
        if (!(indexToType(i) & streamMask)) {
            continue;
        }

        AString uri;
        uri = switching ? mStreams[i].mNewUri : mStreams[i].mUri;

        sp<PlaylistFetcher> fetcher = addFetcher(uri.c_str());
        CHECK(fetcher != NULL);

        HLSTime startTime;
        SeekMode seekMode = kSeekModeExactPosition;
        sp<AnotherPacketSource> sources[kNumSources];

        if (i == kSubtitleIndex || (!pickTrack && !switching)) {
            startTime = latestMediaSegmentStartTime();
        }

        // TRICKY: looping from i as earlier streams are already removed from streamMask
        for (size_t j = i; j < kMaxStreams; ++j) {
            const AString &streamUri = switching ? mStreams[j].mNewUri : mStreams[j].mUri;
            if ((streamMask & indexToType(j)) && uri == streamUri) {
                sources[j] = mPacketSources.valueFor(indexToType(j));

                if (timeUs >= 0) {
                    startTime.mTimeUs = timeUs;
                } else {
                    int32_t type;
                    sp<AMessage> meta;
                    if (!switching) {
                        // selecting, or adapting but no swap required
                        meta = sources[j]->getLatestDequeuedMeta();
                    } else {
                        // adapting and swap required
                        meta = sources[j]->getLatestEnqueuedMeta();
                        if (meta != NULL && mCurBandwidthIndex > mOrigBandwidthIndex) {
                            // switching up
                            meta = sources[j]->getMetaAfterLastDequeued(mUpSwitchMargin);
                        }
                    }

                    if ((j == kAudioIndex || j == kVideoIndex)
                            && meta != NULL && !meta->findInt32("discontinuity", &type)) {
                        HLSTime tmpTime(meta);
                        if (startTime < tmpTime) {
                            startTime = tmpTime;
                        }
                    }

                    if (!switching) {
                        // selecting, or adapting but no swap required
                        sources[j]->clear();
                        if (j == kSubtitleIndex) {
                            break;
                        }

                        ALOGV("stream[%zu]: queue format change", j);
                        sources[j]->queueDiscontinuity(
                                ATSParser::DISCONTINUITY_FORMAT_ONLY, NULL, true);
                    } else {
                        // switching, queue discontinuities after resume
                        sources[j] = mPacketSources2.valueFor(indexToType(j));
                        sources[j]->clear();
                        // the new fetcher might be providing streams that used to be
                        // provided by two different fetchers,  if one of the fetcher
                        // paused in the middle while the other somehow paused in next
                        // seg, we have to start from next seg.
                        if (seekMode < mStreams[j].mSeekMode) {
                            seekMode = mStreams[j].mSeekMode;
                        }
                    }
                }

                streamMask &= ~indexToType(j);
            }
        }

        ALOGV("[fetcher-%d] startAsync: startTimeUs %lld mLastSeekTimeUs %lld "
                "segmentStartTimeUs %lld seekMode %d",
                fetcher->getFetcherID(),
                (long long)startTime.mTimeUs,
                (long long)mLastSeekTimeUs,
                (long long)startTime.getSegmentTimeUs(),
                seekMode);

        // Set the target segment start time to the middle point of the
        // segment where the last sample was.
        // This gives a better guess if segments of the two variants are not
        // perfectly aligned. (If the corresponding segment in new variant
        // starts slightly later than that in the old variant, we still want
        // to pick that segment, not the one before)
        fetcher->startAsync(
                sources[kAudioIndex],
                sources[kVideoIndex],
                sources[kSubtitleIndex],
                getMetadataSource(sources, mNewStreamMask, switching),
                startTime.mTimeUs < 0 ? mLastSeekTimeUs : startTime.mTimeUs,
                startTime.getSegmentTimeUs(),
                startTime.mSeq,
                seekMode);
    }

    // All fetchers have now been started, the configuration change
    // has completed.

    mReconfigurationInProgress = false;
    if (switching) {
        mSwitchInProgress = true;
    } else {
        mStreamMask = mNewStreamMask;
        if (mOrigBandwidthIndex != mCurBandwidthIndex) {
            ALOGV("#### Finished Bandwidth Switch Early: %zd => %zd",
                    mOrigBandwidthIndex, mCurBandwidthIndex);
            mOrigBandwidthIndex = mCurBandwidthIndex;
        }
    }

    ALOGV("onChangeConfiguration3: mSwitchInProgress %d, mStreamMask 0x%x",
            mSwitchInProgress, mStreamMask);

    if (mDisconnectReplyID != NULL) {
        finishDisconnect();
    }
}

void LiveSession::swapPacketSource(StreamType stream) {
    ALOGV("[%s] swapPacketSource", getNameForStream(stream));

    // transfer packets from source2 to source
    sp<AnotherPacketSource> &aps = mPacketSources.editValueFor(stream);
    sp<AnotherPacketSource> &aps2 = mPacketSources2.editValueFor(stream);

    // queue discontinuity in mPacketSource
    aps->queueDiscontinuity(ATSParser::DISCONTINUITY_FORMAT_ONLY, NULL, false);

    // queue packets in mPacketSource2 to mPacketSource
    status_t finalResult = OK;
    sp<ABuffer> accessUnit;
    while (aps2->hasBufferAvailable(&finalResult) && finalResult == OK &&
          OK == aps2->dequeueAccessUnit(&accessUnit)) {
        aps->queueAccessUnit(accessUnit);
    }
    aps2->clear();
}

void LiveSession::tryToFinishBandwidthSwitch(const AString &oldUri) {
    if (!mSwitchInProgress) {
        return;
    }

    ssize_t index = mFetcherInfos.indexOfKey(oldUri);
    if (index < 0 || !mFetcherInfos[index].mToBeRemoved) {
        return;
    }

    // Swap packet source of streams provided by old variant
    for (size_t idx = 0; idx < kMaxStreams; idx++) {
        StreamType stream = indexToType(idx);
        if ((mSwapMask & stream) && (oldUri == mStreams[idx].mUri)) {
            swapPacketSource(stream);

            if ((mNewStreamMask & stream) && mStreams[idx].mNewUri.empty()) {
                ALOGW("swapping stream type %d %s to empty stream",
                        stream, uriDebugString(mStreams[idx].mUri).c_str());
            }
            mStreams[idx].mUri = mStreams[idx].mNewUri;
            mStreams[idx].mNewUri.clear();

            mSwapMask &= ~stream;
        }
    }

    mFetcherInfos.editValueAt(index).mFetcher->stopAsync(false /* clear */);

    ALOGV("tryToFinishBandwidthSwitch: mSwapMask=0x%x", mSwapMask);
    if (mSwapMask != 0) {
        return;
    }

    // Check if new variant contains extra streams.
    uint32_t extraStreams = mNewStreamMask & (~mStreamMask);
    while (extraStreams) {
        StreamType stream = (StreamType) (extraStreams & ~(extraStreams - 1));
        extraStreams &= ~stream;

        swapPacketSource(stream);

        ssize_t idx = typeToIndex(stream);
        CHECK(idx >= 0);
        if (mStreams[idx].mNewUri.empty()) {
            ALOGW("swapping extra stream type %d %s to empty stream",
                    stream, uriDebugString(mStreams[idx].mUri).c_str());
        }
        mStreams[idx].mUri = mStreams[idx].mNewUri;
        mStreams[idx].mNewUri.clear();
    }

    // Restart new fetcher (it was paused after the first 47k block)
    // and let it fetch into mPacketSources (not mPacketSources2)
    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        FetcherInfo &info = mFetcherInfos.editValueAt(i);
        if (info.mToBeResumed) {
            resumeFetcher(mFetcherInfos.keyAt(i), mNewStreamMask);
            info.mToBeResumed = false;
        }
    }

    ALOGI("#### Finished Bandwidth Switch: %zd => %zd",
            mOrigBandwidthIndex, mCurBandwidthIndex);

    mStreamMask = mNewStreamMask;
    mSwitchInProgress = false;
    mOrigBandwidthIndex = mCurBandwidthIndex;

    restartPollBuffering();
}

void LiveSession::schedulePollBuffering() {
    sp<AMessage> msg = new AMessage(kWhatPollBuffering, this);
    msg->setInt32("generation", mPollBufferingGeneration);
    msg->post(1000000LL);
}

void LiveSession::cancelPollBuffering() {
    ++mPollBufferingGeneration;
    mPrevBufferPercentage = -1;
}

void LiveSession::restartPollBuffering() {
    cancelPollBuffering();
    onPollBuffering();
}

void LiveSession::onPollBuffering() {
    ALOGV("onPollBuffering: mSwitchInProgress %d, mReconfigurationInProgress %d, "
            "mInPreparationPhase %d, mCurBandwidthIndex %zd, mStreamMask 0x%x",
        mSwitchInProgress, mReconfigurationInProgress,
        mInPreparationPhase, mCurBandwidthIndex, mStreamMask);

    bool underflow, ready, down, up;
    if (checkBuffering(underflow, ready, down, up)) {
        if (mInPreparationPhase) {
            // Allow down switch even if we're still preparing.
            //
            // Some streams have a high bandwidth index as default,
            // when bandwidth is low, it takes a long time to buffer
            // to ready mark, then it immediately pauses after start
            // as we have to do a down switch. It's better experience
            // to restart from a lower index, if we detect low bw.
            if (!switchBandwidthIfNeeded(false /* up */, down) && ready) {
                postPrepared(OK);
            }
        }

        if (!mInPreparationPhase) {
            if (ready) {
                stopBufferingIfNecessary();
            } else if (underflow) {
                startBufferingIfNecessary();
            }
            switchBandwidthIfNeeded(up, down);
        }
    }

    schedulePollBuffering();
}

void LiveSession::cancelBandwidthSwitch(bool resume) {
    ALOGV("cancelBandwidthSwitch: mSwitchGen(%d)++, orig %zd, cur %zd",
            mSwitchGeneration, mOrigBandwidthIndex, mCurBandwidthIndex);
    if (!mSwitchInProgress) {
        return;
    }

    for (size_t i = 0; i < mFetcherInfos.size(); ++i) {
        FetcherInfo& info = mFetcherInfos.editValueAt(i);
        if (info.mToBeRemoved) {
            info.mToBeRemoved = false;
            if (resume) {
                resumeFetcher(mFetcherInfos.keyAt(i), mSwapMask);
            }
        }
    }

    for (size_t i = 0; i < kMaxStreams; ++i) {
        AString newUri = mStreams[i].mNewUri;
        if (!newUri.empty()) {
            // clear all mNewUri matching this newUri
            for (size_t j = i; j < kMaxStreams; ++j) {
                if (mStreams[j].mNewUri == newUri) {
                    mStreams[j].mNewUri.clear();
                }
            }
            ALOGV("stopping newUri = %s", newUri.c_str());
            ssize_t index = mFetcherInfos.indexOfKey(newUri);
            if (index < 0) {
                ALOGE("did not find fetcher for newUri: %s", uriDebugString(newUri).c_str());
                continue;
            }
            FetcherInfo &info = mFetcherInfos.editValueAt(index);
            info.mToBeRemoved = true;
            info.mFetcher->stopAsync();
        }
    }

    ALOGI("#### Canceled Bandwidth Switch: %zd => %zd",
            mOrigBandwidthIndex, mCurBandwidthIndex);

    mSwitchGeneration++;
    mSwitchInProgress = false;
    mCurBandwidthIndex = mOrigBandwidthIndex;
    mSwapMask = 0;
}

bool LiveSession::checkBuffering(
        bool &underflow, bool &ready, bool &down, bool &up) {
    underflow = ready = down = up = false;

    if (mReconfigurationInProgress) {
        ALOGV("Switch/Reconfig in progress, defer buffer polling");
        return false;
    }

    size_t activeCount, underflowCount, readyCount, downCount, upCount;
    activeCount = underflowCount = readyCount = downCount = upCount =0;
    int32_t minBufferPercent = -1;
    int64_t durationUs;
    if (getDuration(&durationUs) != OK) {
        durationUs = -1;
    }
    for (size_t i = 0; i < mPacketSources.size(); ++i) {
        // we don't check subtitles for buffering level
        if (!(mStreamMask & mPacketSources.keyAt(i)
                & (STREAMTYPE_AUDIO | STREAMTYPE_VIDEO))) {
            continue;
        }
        // ignore streams that never had any packet queued.
        // (it's possible that the variant only has audio or video)
        sp<AMessage> meta = mPacketSources[i]->getLatestEnqueuedMeta();
        if (meta == NULL) {
            continue;
        }

        status_t finalResult;
        int64_t bufferedDurationUs =
                mPacketSources[i]->getBufferedDurationUs(&finalResult);
        ALOGV("[%s] buffered %lld us",
                getNameForStream(mPacketSources.keyAt(i)),
                (long long)bufferedDurationUs);
        if (durationUs >= 0) {
            int32_t percent;
            if (mPacketSources[i]->isFinished(0 /* duration */)) {
                percent = 100;
            } else {
                percent = (int32_t)(100.0 *
                        (mLastDequeuedTimeUs + bufferedDurationUs) / durationUs);
            }
            if (minBufferPercent < 0 || percent < minBufferPercent) {
                minBufferPercent = percent;
            }
        }

        ++activeCount;
        int64_t readyMarkUs =
            (mInPreparationPhase ?
                mBufferingSettings.mInitialMarkMs :
                mBufferingSettings.mResumePlaybackMarkMs) * 1000LL;
        if (bufferedDurationUs > readyMarkUs
                || mPacketSources[i]->isFinished(0)) {
            ++readyCount;
        }
        if (!mPacketSources[i]->isFinished(0)) {
            if (bufferedDurationUs < kUnderflowMarkMs * 1000LL) {
                ++underflowCount;
            }
            if (bufferedDurationUs > mUpSwitchMark) {
                ++upCount;
            }
            if (bufferedDurationUs < mDownSwitchMark) {
                ++downCount;
            }
        }
    }

    if (minBufferPercent >= 0) {
        notifyBufferingUpdate(minBufferPercent);
    }

    if (activeCount > 0) {
        up        = (upCount == activeCount);
        down      = (downCount > 0);
        ready     = (readyCount == activeCount);
        underflow = (underflowCount > 0);
        return true;
    }

    return false;
}

void LiveSession::startBufferingIfNecessary() {
    ALOGV("startBufferingIfNecessary: mInPreparationPhase=%d, mBuffering=%d",
            mInPreparationPhase, mBuffering);
    if (!mBuffering) {
        mBuffering = true;

        sp<AMessage> notify = mNotify->dup();
        notify->setInt32("what", kWhatBufferingStart);
        notify->post();
    }
}

void LiveSession::stopBufferingIfNecessary() {
    ALOGV("stopBufferingIfNecessary: mInPreparationPhase=%d, mBuffering=%d",
            mInPreparationPhase, mBuffering);

    if (mBuffering) {
        mBuffering = false;

        sp<AMessage> notify = mNotify->dup();
        notify->setInt32("what", kWhatBufferingEnd);
        notify->post();
    }
}

void LiveSession::notifyBufferingUpdate(int32_t percentage) {
    if (percentage < mPrevBufferPercentage) {
        percentage = mPrevBufferPercentage;
    } else if (percentage > 100) {
        percentage = 100;
    }

    mPrevBufferPercentage = percentage;

    ALOGV("notifyBufferingUpdate: percentage=%d%%", percentage);

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatBufferingUpdate);
    notify->setInt32("percentage", percentage);
    notify->post();
}

bool LiveSession::tryBandwidthFallback() {
    if (mInPreparationPhase || mReconfigurationInProgress) {
        // Don't try fallback during prepare or reconfig.
        // If error happens there, it's likely unrecoverable.
        return false;
    }
    if (mCurBandwidthIndex > mOrigBandwidthIndex) {
        // if we're switching up, simply cancel and resume old variant
        cancelBandwidthSwitch(true /* resume */);
        return true;
    } else {
        // if we're switching down, we're likely about to underflow (if
        // not already underflowing). try the lowest viable bandwidth if
        // not on that variant already.
        ssize_t lowestValid = getLowestValidBandwidthIndex();
        if (mCurBandwidthIndex > lowestValid) {
            cancelBandwidthSwitch();
            changeConfiguration(-1LL, lowestValid);
            return true;
        }
    }
    // return false if we couldn't find any fallback
    return false;
}

/*
 * returns true if a bandwidth switch is actually needed (and started),
 * returns false otherwise
 */
bool LiveSession::switchBandwidthIfNeeded(bool bufferHigh, bool bufferLow) {
    // no need to check bandwidth if we only have 1 bandwidth settings
    if (mBandwidthItems.size() < 2) {
        return false;
    }

    if (mSwitchInProgress) {
        if (mBuffering) {
            tryBandwidthFallback();
        }
        return false;
    }

    int32_t bandwidthBps, shortTermBps;
    bool isStable;
    if (mBandwidthEstimator->estimateBandwidth(
            &bandwidthBps, &isStable, &shortTermBps)) {
        ALOGV("bandwidth estimated at %.2f kbps, "
                "stable %d, shortTermBps %.2f kbps",
                bandwidthBps / 1024.0f, isStable, shortTermBps / 1024.0f);
        mLastBandwidthBps = bandwidthBps;
        mLastBandwidthStable = isStable;
    } else {
        ALOGV("no bandwidth estimate.");
        return false;
    }

    int32_t curBandwidth = mBandwidthItems.itemAt(mCurBandwidthIndex).mBandwidth;
    // canSwithDown and canSwitchUp can't both be true.
    // we only want to switch up when measured bw is 120% higher than current variant,
    // and we only want to switch down when measured bw is below current variant.
    bool canSwitchDown = bufferLow
            && (bandwidthBps < (int32_t)curBandwidth);
    bool canSwitchUp = bufferHigh
            && (bandwidthBps > (int32_t)curBandwidth * 12 / 10);

    if (canSwitchDown || canSwitchUp) {
        // bandwidth estimating has some delay, if we have to downswitch when
        // it hasn't stabilized, use the short term to guess real bandwidth,
        // since it may be dropping too fast.
        // (note this doesn't apply to upswitch, always use longer average there)
        if (!isStable && canSwitchDown) {
            if (shortTermBps < bandwidthBps) {
                bandwidthBps = shortTermBps;
            }
        }

        ssize_t bandwidthIndex = getBandwidthIndex(bandwidthBps);

        // it's possible that we're checking for canSwitchUp case, but the returned
        // bandwidthIndex is < mCurBandwidthIndex, as getBandwidthIndex() only uses 70%
        // of measured bw. In that case we don't want to do anything, since we have
        // both enough buffer and enough bw.
        if ((canSwitchUp && bandwidthIndex > mCurBandwidthIndex)
         || (canSwitchDown && bandwidthIndex < mCurBandwidthIndex)) {
            // if not yet prepared, just restart again with new bw index.
            // this is faster and playback experience is cleaner.
            changeConfiguration(
                    mInPreparationPhase ? 0 : -1LL, bandwidthIndex);
            return true;
        }
    }
    return false;
}

void LiveSession::postError(status_t err) {
    // if we reached EOS, notify buffering of 100%
    if (err == ERROR_END_OF_STREAM) {
        notifyBufferingUpdate(100);
    }
    // we'll stop buffer polling now, before that notify
    // stop buffering to stop the spinning icon
    stopBufferingIfNecessary();
    cancelPollBuffering();

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

void LiveSession::postPrepared(status_t err) {
    CHECK(mInPreparationPhase);

    sp<AMessage> notify = mNotify->dup();
    if (err == OK || err == ERROR_END_OF_STREAM) {
        notify->setInt32("what", kWhatPrepared);
    } else {
        cancelPollBuffering();

        notify->setInt32("what", kWhatPreparationFailed);
        notify->setInt32("err", err);
    }

    notify->post();

    mInPreparationPhase = false;
}


}  // namespace android

