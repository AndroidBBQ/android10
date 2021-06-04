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

#ifndef MediaCodecSource_H_
#define MediaCodecSource_H_

#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/Mutexed.h>
#include <media/stagefright/PersistentSurface.h>

namespace android {

struct ALooper;
struct AMessage;
struct AReplyToken;
class IGraphicBufferProducer;
struct MediaCodec;

struct MediaCodecSource : public MediaSource,
                          public MediaBufferObserver {
    enum FlagBits {
        FLAG_USE_SURFACE_INPUT      = 1,
        FLAG_PREFER_SOFTWARE_CODEC  = 4,  // used for testing only
    };

    static sp<MediaCodecSource> Create(
            const sp<ALooper> &looper,
            const sp<AMessage> &format,
            const sp<MediaSource> &source,
            const sp<PersistentSurface> &persistentSurface = NULL,
            uint32_t flags = 0);

    bool isVideo() const { return mIsVideo; }
    sp<IGraphicBufferProducer> getGraphicBufferProducer();
    status_t setInputBufferTimeOffset(int64_t timeOffsetUs);
    int64_t getFirstSampleSystemTimeUs();

    // MediaSource
    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual status_t pause() { return pause(NULL); }
    virtual status_t pause(MetaData *params);
    virtual sp<MetaData> getFormat();
    virtual status_t read(
            MediaBufferBase **buffer,
            const ReadOptions *options = NULL);
    virtual status_t setStopTimeUs(int64_t stopTimeUs);


    // MediaBufferObserver
    virtual void signalBufferReturned(MediaBufferBase *buffer);

    // for AHandlerReflector
    void onMessageReceived(const sp<AMessage> &msg);



protected:
    virtual ~MediaCodecSource();

private:
    struct Puller;

    enum {
        kWhatPullerNotify,
        kWhatEncoderActivity,
        kWhatStart,
        kWhatStop,
        kWhatPause,
        kWhatSetInputBufferTimeOffset,
        kWhatSetStopTimeUs,
        kWhatGetFirstSampleSystemTimeUs,
        kWhatStopStalled,
    };

    MediaCodecSource(
            const sp<ALooper> &looper,
            const sp<AMessage> &outputFormat,
            const sp<MediaSource> &source,
            const sp<PersistentSurface> &persistentSurface,
            uint32_t flags = 0);

    status_t onStart(MetaData *params);

    // Pause the source at pauseStartTimeUs. For non-surface input,
    // buffers will be dropped immediately. For surface input, buffers
    // with timestamp smaller than pauseStartTimeUs will still be encoded.
    // Buffers with timestamp larger or queal to pauseStartTimeUs will be
    // dropped. pauseStartTimeUs uses SYSTEM_TIME_MONOTONIC time base.
    void onPause(int64_t pauseStartTimeUs);

    status_t init();
    status_t initEncoder();
    void releaseEncoder();
    status_t feedEncoderInputBuffers();
    // Resume GraphicBufferSource at resumeStartTimeUs. Buffers
    // from GraphicBufferSource with timestamp larger or equal to
    // resumeStartTimeUs will be encoded. resumeStartTimeUs uses
    // SYSTEM_TIME_MONOTONIC time base.
    void resume(int64_t resumeStartTimeUs = -1ll);
    void signalEOS(status_t err = ERROR_END_OF_STREAM);
    bool reachedEOS();
    status_t postSynchronouslyAndReturnError(const sp<AMessage> &msg);

    sp<ALooper> mLooper;
    sp<ALooper> mCodecLooper;
    sp<AHandlerReflector<MediaCodecSource> > mReflector;
    sp<AMessage> mOutputFormat;
    Mutexed<sp<MetaData>> mMeta;
    sp<Puller> mPuller;
    sp<MediaCodec> mEncoder;
    uint32_t mFlags;
    List<sp<AReplyToken>> mStopReplyIDQueue;
    bool mIsVideo;
    bool mStarted;
    bool mStopping;
    bool mDoMoreWorkPending;
    bool mSetEncoderFormat;
    int32_t mEncoderFormat;
    int32_t mEncoderDataSpace;
    sp<AMessage> mEncoderActivityNotify;
    sp<IGraphicBufferProducer> mGraphicBufferProducer;
    sp<PersistentSurface> mPersistentSurface;
    List<MediaBufferBase *> mInputBufferQueue;
    List<size_t> mAvailEncoderInputIndices;
    List<int64_t> mDecodingTimeQueue; // decoding time (us) for video
    int64_t mInputBufferTimeOffsetUs;
    int64_t mFirstSampleSystemTimeUs;
    bool mPausePending;

    // audio drift time
    int64_t mFirstSampleTimeUs;
    List<int64_t> mDriftTimeQueue;

    struct Output {
        Output();
        List<MediaBufferBase*> mBufferQueue;
        bool mEncoderReachedEOS;
        status_t mErrorCode;
        Condition mCond;
    };
    Mutexed<Output> mOutput;

    int32_t mGeneration;

    DISALLOW_EVIL_CONSTRUCTORS(MediaCodecSource);
};

} // namespace android

#endif /* MediaCodecSource_H_ */
