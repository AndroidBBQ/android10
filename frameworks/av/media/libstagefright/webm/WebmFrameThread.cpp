/*
 * Copyright (C) 2014 The Android Open Source Project
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
#define LOG_TAG "WebmFrameThread"

#include "WebmConstants.h"
#include "WebmFrameThread.h"

#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/ADebug.h>

#include <utils/Log.h>
#include <inttypes.h>

using namespace webm;

namespace android {

void *WebmFrameThread::wrap(void *arg) {
    WebmFrameThread *worker = reinterpret_cast<WebmFrameThread*>(arg);
    worker->run();
    return NULL;
}

status_t WebmFrameThread::start() {
    status_t err = OK;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if ((err = pthread_create(&mThread, &attr, WebmFrameThread::wrap, this))) {
        mThread = 0;
    }
    pthread_attr_destroy(&attr);
    return err;
}

status_t WebmFrameThread::stop() {
    void *status = nullptr;
    if (mThread) {
        pthread_join(mThread, &status);
        mThread = 0;
    }
    return (status_t)(intptr_t)status;
}

//=================================================================================================

WebmFrameSourceThread::WebmFrameSourceThread(
    int type,
    LinkedBlockingQueue<const sp<WebmFrame> >& sink)
    : mType(type), mSink(sink) {
}

//=================================================================================================

WebmFrameSinkThread::WebmFrameSinkThread(
        const int& fd,
        const uint64_t& off,
        sp<WebmFrameSourceThread> videoThread,
        sp<WebmFrameSourceThread> audioThread,
        List<sp<WebmElement> >& cues)
    : mFd(fd),
      mSegmentDataStart(off),
      mVideoFrames(videoThread->mSink),
      mAudioFrames(audioThread->mSink),
      mCues(cues),
      mStartOffsetTimecode(UINT64_MAX),
      mDone(true) {
}

WebmFrameSinkThread::WebmFrameSinkThread(
        const int& fd,
        const uint64_t& off,
        LinkedBlockingQueue<const sp<WebmFrame> >& videoSource,
        LinkedBlockingQueue<const sp<WebmFrame> >& audioSource,
        List<sp<WebmElement> >& cues)
    : mFd(fd),
      mSegmentDataStart(off),
      mVideoFrames(videoSource),
      mAudioFrames(audioSource),
      mCues(cues),
      mStartOffsetTimecode(UINT64_MAX),
      mDone(true) {
}

// Initializes a webm cluster with its starting timecode.
//
// frames:
//   sequence of input audio/video frames received from the source.
//
// clusterTimecodeL:
//   the starting timecode of the cluster; this is the timecode of the first
//   frame since frames are ordered by timestamp.
//
// children:
//   list to hold child elements in a webm cluster (start timecode and
//   simple blocks).
//
// static
void WebmFrameSinkThread::initCluster(
    List<const sp<WebmFrame> >& frames,
    uint64_t& clusterTimecodeL,
    List<sp<WebmElement> >& children) {
    CHECK(!frames.empty() && children.empty());

    const sp<WebmFrame> f = *(frames.begin());
    clusterTimecodeL = f->mAbsTimecode;
    WebmUnsigned *clusterTimecode = new WebmUnsigned(kMkvTimecode, clusterTimecodeL);
    children.clear();
    children.push_back(clusterTimecode);
}

void WebmFrameSinkThread::writeCluster(List<sp<WebmElement> >& children) {
    // children must contain at least one simpleblock and its timecode
    CHECK_GE(children.size(), 2u);

    uint64_t size;
    sp<WebmElement> cluster = new WebmMaster(kMkvCluster, children);
    cluster->write(mFd, size);
    children.clear();
}

// Write out (possibly multiple) webm cluster(s) from frames split on video key frames.
//
// last:
//   current flush is triggered by EOS instead of a second outstanding video key frame.
void WebmFrameSinkThread::flushFrames(List<const sp<WebmFrame> >& frames, bool last) {
    if (frames.empty()) {
        return;
    }

    uint64_t clusterTimecodeL;
    List<sp<WebmElement> > children;
    initCluster(frames, clusterTimecodeL, children);

    uint64_t cueTime = clusterTimecodeL;
    off_t fpos = ::lseek(mFd, 0, SEEK_CUR);
    size_t n = frames.size();
    if (!last) {
        // If we are not flushing the last sequence of outstanding frames, flushFrames
        // must have been called right after we have pushed a second outstanding video key
        // frame (the last frame), which belongs to the next cluster; also hold back on
        // flushing the second to last frame before we check its type. A audio frame
        // should precede the aforementioned video key frame in the next sequence, a video
        // frame should be the last frame in the current (to-be-flushed) sequence.
        CHECK_GE(n, 2u);
        n -= 2;
    }

    for (size_t i = 0; i < n; i++) {
        const sp<WebmFrame> f = *(frames.begin());
        if (f->mType == kVideoType && f->mKey) {
            cueTime = f->mAbsTimecode;
        }

        if (f->mAbsTimecode - clusterTimecodeL > INT16_MAX) {
            writeCluster(children);
            initCluster(frames, clusterTimecodeL, children);
        }

        frames.erase(frames.begin());
        children.push_back(f->SimpleBlock(clusterTimecodeL));
    }

    // equivalent to last==false
    if (!frames.empty()) {
        // decide whether to write out the second to last frame.
        const sp<WebmFrame> secondLastFrame = *(frames.begin());
        if (secondLastFrame->mType == kVideoType) {
            frames.erase(frames.begin());
            children.push_back(secondLastFrame->SimpleBlock(clusterTimecodeL));
        }
    }

    writeCluster(children);
    sp<WebmElement> cuePoint = WebmElement::CuePointEntry(cueTime, 1, fpos - mSegmentDataStart);
    mCues.push_back(cuePoint);
}

status_t WebmFrameSinkThread::start() {
    mDone = false;
    return WebmFrameThread::start();
}

status_t WebmFrameSinkThread::stop() {
    mDone = true;
    mVideoFrames.push(WebmFrame::EOS);
    mAudioFrames.push(WebmFrame::EOS);
    return WebmFrameThread::stop();
}

void WebmFrameSinkThread::run() {
    int numVideoKeyFrames = 0;
    List<const sp<WebmFrame> > outstandingFrames;
    while (!mDone) {
        ALOGV("wait v frame");
        const sp<WebmFrame> videoFrame = mVideoFrames.peek();
        ALOGV("v frame: %p", videoFrame.get());

        ALOGV("wait a frame");
        const sp<WebmFrame> audioFrame = mAudioFrames.peek();
        ALOGV("a frame: %p", audioFrame.get());

        if (mStartOffsetTimecode == UINT64_MAX) {
            mStartOffsetTimecode =
                    std::min(audioFrame->getAbsTimecode(), videoFrame->getAbsTimecode());
        }

        if (videoFrame->mEos && audioFrame->mEos) {
            break;
        }

        if (*audioFrame < *videoFrame) {
            ALOGV("take a frame");
            mAudioFrames.take();
            audioFrame->updateAbsTimecode(audioFrame->getAbsTimecode() - mStartOffsetTimecode);
            outstandingFrames.push_back(audioFrame);
        } else {
            ALOGV("take v frame");
            mVideoFrames.take();
            videoFrame->updateAbsTimecode(videoFrame->getAbsTimecode() - mStartOffsetTimecode);
            outstandingFrames.push_back(videoFrame);
            if (videoFrame->mKey)
                numVideoKeyFrames++;
        }

        if (numVideoKeyFrames == 2) {
            flushFrames(outstandingFrames, /* last = */ false);
            numVideoKeyFrames--;
        }
    }
    ALOGV("flushing last cluster (size %zu)", outstandingFrames.size());
    flushFrames(outstandingFrames, /* last = */ true);
    mDone = true;
}

//=================================================================================================

static const int64_t kInitialDelayTimeUs = 700000LL;

void WebmFrameMediaSourceThread::clearFlags() {
    mDone = false;
    mPaused = false;
    mResumed = false;
    mStarted = false;
    mReachedEOS = false;
}

WebmFrameMediaSourceThread::WebmFrameMediaSourceThread(
        const sp<MediaSource>& source,
        int type,
        LinkedBlockingQueue<const sp<WebmFrame> >& sink,
        uint64_t timeCodeScale,
        int64_t startTimeRealUs,
        int32_t startTimeOffsetMs,
        int numTracks,
        bool realTimeRecording)
    : WebmFrameSourceThread(type, sink),
      mSource(source),
      mTimeCodeScale(timeCodeScale),
      mTrackDurationUs(0) {
    clearFlags();
    mStartTimeUs = startTimeRealUs;
    if (realTimeRecording && numTracks > 1) {
        /*
         * Copied from MPEG4Writer
         *
         * This extra delay of accepting incoming audio/video signals
         * helps to align a/v start time at the beginning of a recording
         * session, and it also helps eliminate the "recording" sound for
         * camcorder applications.
         *
         * If client does not set the start time offset, we fall back to
         * use the default initial delay value.
         */
        int64_t startTimeOffsetUs = startTimeOffsetMs * 1000LL;
        if (startTimeOffsetUs < 0) {  // Start time offset was not set
            startTimeOffsetUs = kInitialDelayTimeUs;
        }
        mStartTimeUs += startTimeOffsetUs;
        ALOGI("Start time offset: %" PRId64 " us", startTimeOffsetUs);
    }
}

status_t WebmFrameMediaSourceThread::start() {
    sp<MetaData> meta = new MetaData;
    meta->setInt64(kKeyTime, mStartTimeUs);
    status_t err = mSource->start(meta.get());
    if (err != OK) {
        mDone = true;
        mReachedEOS = true;
        return err;
    } else {
        mStarted = true;
        return WebmFrameThread::start();
    }
}

status_t WebmFrameMediaSourceThread::resume() {
    if (!mDone && mPaused) {
        mPaused = false;
        mResumed = true;
    }
    return OK;
}

status_t WebmFrameMediaSourceThread::pause() {
    if (mStarted) {
        mPaused = true;
    }
    return OK;
}

status_t WebmFrameMediaSourceThread::stop() {
    if (mStarted) {
        mStarted = false;
        mDone = true;
        mSource->stop();
        return WebmFrameThread::stop();
    }
    return OK;
}

void WebmFrameMediaSourceThread::run() {
    int32_t count = 0;
    int64_t timestampUs = 0xdeadbeef;
    int64_t lastTimestampUs = 0; // Previous sample time stamp
    int64_t lastDurationUs = 0; // Previous sample duration
    int64_t previousPausedDurationUs = 0;

    const uint64_t kUninitialized = 0xffffffffffffffffL;
    mStartTimeUs = kUninitialized;

    status_t err = OK;
    MediaBufferBase *buffer;
    while (!mDone && (err = mSource->read(&buffer, NULL)) == OK) {
        if (buffer->range_length() == 0) {
            buffer->release();
            buffer = NULL;
            continue;
        }

        MetaDataBase &md = buffer->meta_data();
        CHECK(md.findInt64(kKeyTime, &timestampUs));
        if (mStartTimeUs == kUninitialized) {
            mStartTimeUs = timestampUs;
        }

        if (mPaused && !mResumed) {
            lastDurationUs = timestampUs - lastTimestampUs;
            lastTimestampUs = timestampUs;
            buffer->release();
            buffer = NULL;
            continue;
        }
        ++count;

        // adjust time-stamps after pause/resume
        if (mResumed) {
            int64_t durExcludingEarlierPausesUs = timestampUs - previousPausedDurationUs;
            CHECK_GE(durExcludingEarlierPausesUs, 0LL);
            int64_t pausedDurationUs = durExcludingEarlierPausesUs - mTrackDurationUs;
            CHECK_GE(pausedDurationUs, lastDurationUs);
            previousPausedDurationUs += pausedDurationUs - lastDurationUs;
            mResumed = false;
        }
        timestampUs -= previousPausedDurationUs;
        CHECK_GE(timestampUs, 0LL);

        int32_t isSync = false;
        md.findInt32(kKeyIsSyncFrame, &isSync);
        const sp<WebmFrame> f = new WebmFrame(
            mType,
            isSync,
            timestampUs * 1000 / mTimeCodeScale,
            buffer);
        mSink.push(f);

        ALOGV(
            "%s %s frame at %" PRId64 " size %zu\n",
            mType == kVideoType ? "video" : "audio",
            isSync ? "I" : "P",
            timestampUs * 1000 / mTimeCodeScale,
            buffer->range_length());

        buffer->release();
        buffer = NULL;

        if (timestampUs > mTrackDurationUs) {
            mTrackDurationUs = timestampUs;
        }
        lastDurationUs = timestampUs - lastTimestampUs;
        lastTimestampUs = timestampUs;
    }

    mTrackDurationUs += lastDurationUs;
    mSink.push(WebmFrame::EOS);
}
}
