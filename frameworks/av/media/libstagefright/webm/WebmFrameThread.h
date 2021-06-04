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

#ifndef WEBMFRAMETHREAD_H_
#define WEBMFRAMETHREAD_H_

#include "WebmFrame.h"
#include "LinkedBlockingQueue.h"

#include <media/MediaSource.h>
#include <media/stagefright/FileSource.h>

#include <utils/List.h>
#include <utils/Errors.h>

#include <pthread.h>

namespace android {

class WebmFrameThread : public LightRefBase<WebmFrameThread> {
public:
    virtual void run() = 0;
    virtual bool running() { return false; }
    virtual status_t start();
    virtual status_t pause() { return OK; }
    virtual status_t resume() { return OK; }
    virtual status_t stop();
    virtual ~WebmFrameThread() { stop(); }
    static void *wrap(void *arg);

protected:
    WebmFrameThread()
        : mThread(0) {
    }

private:
    pthread_t mThread;
    DISALLOW_EVIL_CONSTRUCTORS(WebmFrameThread);
};

//=================================================================================================

class WebmFrameSourceThread;
class WebmFrameSinkThread : public WebmFrameThread {
public:
    WebmFrameSinkThread(
            const int& fd,
            const uint64_t& off,
            sp<WebmFrameSourceThread> videoThread,
            sp<WebmFrameSourceThread> audioThread,
            List<sp<WebmElement> >& cues);

    WebmFrameSinkThread(
            const int& fd,
            const uint64_t& off,
            LinkedBlockingQueue<const sp<WebmFrame> >& videoSource,
            LinkedBlockingQueue<const sp<WebmFrame> >& audioSource,
            List<sp<WebmElement> >& cues);

    void run();
    bool running() {
        return !mDone;
    }
    status_t start();
    status_t stop();

private:
    const int& mFd;
    const uint64_t& mSegmentDataStart;
    LinkedBlockingQueue<const sp<WebmFrame> >& mVideoFrames;
    LinkedBlockingQueue<const sp<WebmFrame> >& mAudioFrames;
    List<sp<WebmElement> >& mCues;
    uint64_t mStartOffsetTimecode;

    volatile bool mDone;

    static void initCluster(
            List<const sp<WebmFrame> >& frames,
            uint64_t& clusterTimecodeL,
            List<sp<WebmElement> >& children);
    void writeCluster(List<sp<WebmElement> >& children);
    void flushFrames(List<const sp<WebmFrame> >& frames, bool last);
};

//=================================================================================================

class WebmFrameSourceThread : public WebmFrameThread {
public:
    WebmFrameSourceThread(int type, LinkedBlockingQueue<const sp<WebmFrame> >& sink);
    virtual int64_t getDurationUs() = 0;
protected:
    const int mType;
    LinkedBlockingQueue<const sp<WebmFrame> >& mSink;

    friend class WebmFrameSinkThread;
};

//=================================================================================================

class WebmFrameEmptySourceThread : public WebmFrameSourceThread {
public:
    WebmFrameEmptySourceThread(int type, LinkedBlockingQueue<const sp<WebmFrame> >& sink)
        : WebmFrameSourceThread(type, sink) {
    }
    void run() { mSink.push(WebmFrame::EOS); }
    int64_t getDurationUs() { return 0; }
};

//=================================================================================================

class WebmFrameMediaSourceThread: public WebmFrameSourceThread {
public:
    WebmFrameMediaSourceThread(
            const sp<MediaSource>& source,
            int type,
            LinkedBlockingQueue<const sp<WebmFrame> >& sink,
            uint64_t timeCodeScale,
            int64_t startTimeRealUs,
            int32_t startTimeOffsetMs,
            int numPeers,
            bool realTimeRecording);

    void run();
    status_t start();
    status_t resume();
    status_t pause();
    status_t stop();
    int64_t getDurationUs() {
        return mTrackDurationUs;
    }

private:
    const sp<MediaSource> mSource;
    const uint64_t mTimeCodeScale;
    uint64_t mStartTimeUs;

    volatile bool mDone;
    volatile bool mPaused;
    volatile bool mResumed;
    volatile bool mStarted;
    volatile bool mReachedEOS;
    int64_t mTrackDurationUs;

    void clearFlags();
};
} /* namespace android */

#endif /* WEBMFRAMETHREAD_H_ */
