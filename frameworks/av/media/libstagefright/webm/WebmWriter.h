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

#ifndef WEBMWRITER_H_
#define WEBMWRITER_H_

#include "WebmConstants.h"
#include "WebmFrameThread.h"
#include "LinkedBlockingQueue.h"

#include <media/MediaSource.h>
#include <media/stagefright/MediaWriter.h>

#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>

#include <stdint.h>

using namespace webm;

namespace android {

class WebmWriter : public MediaWriter {
public:
    explicit WebmWriter(int fd);
    ~WebmWriter() { reset(); }


    virtual status_t addSource(const sp<MediaSource> &source);
    virtual status_t start(MetaData *param = NULL);
    virtual status_t stop();
    virtual status_t pause();
    virtual bool reachedEOS();

    virtual void setStartTimeOffsetMs(int ms) { mStartTimeOffsetMs = ms; }
    virtual int32_t getStartTimeOffsetMs() const { return mStartTimeOffsetMs; }

private:
    int mFd;
    status_t mInitCheck;

    uint64_t mTimeCodeScale;
    int64_t mStartTimestampUs;
    int32_t mStartTimeOffsetMs;

    uint64_t mSegmentOffset;
    uint64_t mSegmentDataStart;
    uint64_t mInfoOffset;
    uint64_t mInfoSize;
    uint64_t mTracksOffset;
    uint64_t mCuesOffset;

    bool mPaused;
    bool mStarted;
    bool mIsFileSizeLimitExplicitlyRequested;
    bool mIsRealTimeRecording;
    bool mStreamableFile;
    uint64_t mEstimatedCuesSize;

    Mutex mLock;
    List<sp<WebmElement> > mCuePoints;

    enum {
        kAudioIndex     =  0,
        kVideoIndex     =  1,
        kMaxStreams     =  2,
    };

    struct WebmStream {
        int mType;
        const char *mName;
        sp<WebmElement> (*mMakeTrack)(const sp<MetaData>&);

        sp<MediaSource> mSource;
        sp<WebmElement> mTrackEntry;
        sp<WebmFrameSourceThread> mThread;
        LinkedBlockingQueue<const sp<WebmFrame> > mSink;

        WebmStream()
            : mType(kInvalidType),
              mName("Invalid"),
              mMakeTrack(NULL) {
        }

        WebmStream(int type, const char *name, sp<WebmElement> (*makeTrack)(const sp<MetaData>&))
            : mType(type),
              mName(name),
              mMakeTrack(makeTrack) {
        }

        WebmStream &operator=(const WebmStream &other) {
            mType = other.mType;
            mName = other.mName;
            mMakeTrack = other.mMakeTrack;
            return *this;
        }
    };
    WebmStream mStreams[kMaxStreams];
    Vector<sp<WebmElement>> mStreamsInOrder;

    sp<WebmFrameSinkThread> mSinkThread;

    size_t numTracks();
    uint64_t estimateCuesSize(int32_t bitRate);
    void initStream(size_t idx);
    void release();
    status_t reset();

    static sp<WebmElement> videoTrack(const sp<MetaData>& md);
    static sp<WebmElement> audioTrack(const sp<MetaData>& md);

    DISALLOW_EVIL_CONSTRUCTORS(WebmWriter);
};

} /* namespace android */
#endif /* WEBMWRITER_H_ */
