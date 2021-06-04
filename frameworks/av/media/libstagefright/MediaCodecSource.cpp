/*
 * Copyright 2014, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaCodecSource"
#define DEBUG_DRIFT_TIME 0

#include <inttypes.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/MediaBufferHolder.h>
#include <media/MediaCodecBuffer.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaCodecSource.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

namespace android {

const int32_t kDefaultSwVideoEncoderFormat = HAL_PIXEL_FORMAT_YCbCr_420_888;
const int32_t kDefaultHwVideoEncoderFormat = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
const int32_t kDefaultVideoEncoderDataSpace = HAL_DATASPACE_V0_BT709;

const int kStopTimeoutUs = 300000; // allow 1 sec for shutting down encoder
// allow maximum 1 sec for stop time offset. This limits the the delay in the
// input source.
const int kMaxStopTimeOffsetUs = 1000000;

struct MediaCodecSource::Puller : public AHandler {
    explicit Puller(const sp<MediaSource> &source);

    void interruptSource();
    status_t start(const sp<MetaData> &meta, const sp<AMessage> &notify);
    void stop();
    void stopSource();
    void pause();
    void resume();
    status_t setStopTimeUs(int64_t stopTimeUs);
    bool readBuffer(MediaBufferBase **buffer);

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Puller();

private:
    enum {
        kWhatStart = 'msta',
        kWhatStop,
        kWhatPull,
        kWhatSetStopTimeUs,
    };

    sp<MediaSource> mSource;
    sp<AMessage> mNotify;
    sp<ALooper> mLooper;
    bool mIsAudio;

    struct Queue {
        Queue()
            : mReadPendingSince(0),
              mPaused(false),
              mPulling(false) { }
        int64_t mReadPendingSince;
        bool mPaused;
        bool mPulling;
        Vector<MediaBufferBase *> mReadBuffers;

        void flush();
        // if queue is empty, return false and set *|buffer| to NULL . Otherwise, pop
        // buffer from front of the queue, place it into *|buffer| and return true.
        bool readBuffer(MediaBufferBase **buffer);
        // add a buffer to the back of the queue
        void pushBuffer(MediaBufferBase *mbuf);
    };
    Mutexed<Queue> mQueue;

    status_t postSynchronouslyAndReturnError(const sp<AMessage> &msg);
    void schedulePull();
    void handleEOS();

    DISALLOW_EVIL_CONSTRUCTORS(Puller);
};

MediaCodecSource::Puller::Puller(const sp<MediaSource> &source)
    : mSource(source),
      mLooper(new ALooper()),
      mIsAudio(false)
{
    sp<MetaData> meta = source->getFormat();
    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    mIsAudio = !strncasecmp(mime, "audio/", 6);

    mLooper->setName("pull_looper");
}

MediaCodecSource::Puller::~Puller() {
    mLooper->unregisterHandler(id());
    mLooper->stop();
}

void MediaCodecSource::Puller::Queue::pushBuffer(MediaBufferBase *mbuf) {
    mReadBuffers.push_back(mbuf);
}

bool MediaCodecSource::Puller::Queue::readBuffer(MediaBufferBase **mbuf) {
    if (mReadBuffers.empty()) {
        *mbuf = NULL;
        return false;
    }
    *mbuf = *mReadBuffers.begin();
    mReadBuffers.erase(mReadBuffers.begin());
    return true;
}

void MediaCodecSource::Puller::Queue::flush() {
    MediaBufferBase *mbuf;
    while (readBuffer(&mbuf)) {
        // there are no null buffers in the queue
        mbuf->release();
    }
}

bool MediaCodecSource::Puller::readBuffer(MediaBufferBase **mbuf) {
    Mutexed<Queue>::Locked queue(mQueue);
    return queue->readBuffer(mbuf);
}

status_t MediaCodecSource::Puller::postSynchronouslyAndReturnError(
        const sp<AMessage> &msg) {
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        return err;
    }

    if (!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

status_t MediaCodecSource::Puller::setStopTimeUs(int64_t stopTimeUs) {
    sp<AMessage> msg = new AMessage(kWhatSetStopTimeUs, this);
    msg->setInt64("stop-time-us", stopTimeUs);
    return postSynchronouslyAndReturnError(msg);
}

status_t MediaCodecSource::Puller::start(const sp<MetaData> &meta, const sp<AMessage> &notify) {
    ALOGV("puller (%s) start", mIsAudio ? "audio" : "video");
    mLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
    mLooper->registerHandler(this);
    mNotify = notify;

    sp<AMessage> msg = new AMessage(kWhatStart, this);
    msg->setObject("meta", meta);
    return postSynchronouslyAndReturnError(msg);
}

void MediaCodecSource::Puller::stop() {
    bool interrupt = false;
    {
        // mark stopping before actually reaching kWhatStop on the looper, so the pulling will
        // stop.
        Mutexed<Queue>::Locked queue(mQueue);
        queue->mPulling = false;
        interrupt = queue->mReadPendingSince && (queue->mReadPendingSince < ALooper::GetNowUs() - 1000000);
        queue->flush(); // flush any unprocessed pulled buffers
    }

    if (interrupt) {
        interruptSource();
    }
}

void MediaCodecSource::Puller::interruptSource() {
    // call source->stop if read has been pending for over a second
    // We have to call this outside the looper as looper is pending on the read.
    mSource->stop();
}

void MediaCodecSource::Puller::stopSource() {
    sp<AMessage> msg = new AMessage(kWhatStop, this);
    (void)postSynchronouslyAndReturnError(msg);
}

void MediaCodecSource::Puller::pause() {
    Mutexed<Queue>::Locked queue(mQueue);
    queue->mPaused = true;
}

void MediaCodecSource::Puller::resume() {
    Mutexed<Queue>::Locked queue(mQueue);
    queue->mPaused = false;
}

void MediaCodecSource::Puller::schedulePull() {
    (new AMessage(kWhatPull, this))->post();
}

void MediaCodecSource::Puller::handleEOS() {
    ALOGV("puller (%s) posting EOS", mIsAudio ? "audio" : "video");
    sp<AMessage> msg = mNotify->dup();
    msg->setInt32("eos", 1);
    msg->post();
}

void MediaCodecSource::Puller::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            sp<RefBase> obj;
            CHECK(msg->findObject("meta", &obj));

            {
                Mutexed<Queue>::Locked queue(mQueue);
                queue->mPulling = true;
            }

            status_t err = mSource->start(static_cast<MetaData *>(obj.get()));

            if (err == OK) {
                schedulePull();
            }

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);

            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            response->postReply(replyID);
            break;
        }

        case kWhatSetStopTimeUs:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            int64_t stopTimeUs;
            CHECK(msg->findInt64("stop-time-us", &stopTimeUs));
            status_t err = mSource->setStopTimeUs(stopTimeUs);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatStop:
        {
            mSource->stop();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", OK);

            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            response->postReply(replyID);
            break;
        }

        case kWhatPull:
        {
            Mutexed<Queue>::Locked queue(mQueue);
            queue->mReadPendingSince = ALooper::GetNowUs();
            if (!queue->mPulling) {
                handleEOS();
                break;
            }

            queue.unlock();
            MediaBufferBase *mbuf = NULL;
            status_t err = mSource->read(&mbuf);
            queue.lock();

            queue->mReadPendingSince = 0;
            // if we need to discard buffer
            if (!queue->mPulling || queue->mPaused || err != OK) {
                if (mbuf != NULL) {
                    mbuf->release();
                    mbuf = NULL;
                }
                if (queue->mPulling && err == OK) {
                    msg->post(); // if simply paused, keep pulling source
                    break;
                } else if (err == ERROR_END_OF_STREAM) {
                    ALOGV("stream ended, mbuf %p", mbuf);
                } else if (err != OK) {
                    ALOGE("error %d reading stream.", err);
                }
            }

            if (mbuf != NULL) {
                queue->pushBuffer(mbuf);
            }

            queue.unlock();

            if (mbuf != NULL) {
                mNotify->post();
                msg->post();
            } else {
                handleEOS();
            }
            break;
        }

        default:
            TRESPASS();
    }
}

MediaCodecSource::Output::Output()
    : mEncoderReachedEOS(false),
      mErrorCode(OK) {
}

// static
sp<MediaCodecSource> MediaCodecSource::Create(
        const sp<ALooper> &looper,
        const sp<AMessage> &format,
        const sp<MediaSource> &source,
        const sp<PersistentSurface> &persistentSurface,
        uint32_t flags) {
    sp<MediaCodecSource> mediaSource = new MediaCodecSource(
            looper, format, source, persistentSurface, flags);

    if (mediaSource->init() == OK) {
        return mediaSource;
    }
    return NULL;
}

status_t MediaCodecSource::setInputBufferTimeOffset(int64_t timeOffsetUs) {
    sp<AMessage> msg = new AMessage(kWhatSetInputBufferTimeOffset, mReflector);
    msg->setInt64(PARAMETER_KEY_OFFSET_TIME, timeOffsetUs);
    return postSynchronouslyAndReturnError(msg);
}

int64_t MediaCodecSource::getFirstSampleSystemTimeUs() {
    sp<AMessage> msg = new AMessage(kWhatGetFirstSampleSystemTimeUs, mReflector);
    sp<AMessage> response;
    msg->postAndAwaitResponse(&response);
    int64_t timeUs;
    if (!response->findInt64("time-us", &timeUs)) {
        timeUs = -1LL;
    }
    return timeUs;
}

status_t MediaCodecSource::start(MetaData* params) {
    sp<AMessage> msg = new AMessage(kWhatStart, mReflector);
    msg->setObject("meta", params);
    return postSynchronouslyAndReturnError(msg);
}

status_t MediaCodecSource::stop() {
    sp<AMessage> msg = new AMessage(kWhatStop, mReflector);
    return postSynchronouslyAndReturnError(msg);
}


status_t MediaCodecSource::setStopTimeUs(int64_t stopTimeUs) {
    sp<AMessage> msg = new AMessage(kWhatSetStopTimeUs, mReflector);
    msg->setInt64("stop-time-us", stopTimeUs);
    return postSynchronouslyAndReturnError(msg);
}

status_t MediaCodecSource::pause(MetaData* params) {
    sp<AMessage> msg = new AMessage(kWhatPause, mReflector);
    msg->setObject("meta", params);
    msg->post();
    return OK;
}

sp<MetaData> MediaCodecSource::getFormat() {
    Mutexed<sp<MetaData>>::Locked meta(mMeta);
    return *meta;
}

sp<IGraphicBufferProducer> MediaCodecSource::getGraphicBufferProducer() {
    CHECK(mFlags & FLAG_USE_SURFACE_INPUT);
    return mGraphicBufferProducer;
}

status_t MediaCodecSource::read(
        MediaBufferBase** buffer, const ReadOptions* /* options */) {
    Mutexed<Output>::Locked output(mOutput);

    *buffer = NULL;
    while (output->mBufferQueue.size() == 0 && !output->mEncoderReachedEOS) {
        output.waitForCondition(output->mCond);
    }
    if (!output->mEncoderReachedEOS) {
        *buffer = *output->mBufferQueue.begin();
        output->mBufferQueue.erase(output->mBufferQueue.begin());
        return OK;
    }
    return output->mErrorCode;
}

void MediaCodecSource::signalBufferReturned(MediaBufferBase *buffer) {
    buffer->setObserver(0);
    buffer->release();
}

MediaCodecSource::MediaCodecSource(
        const sp<ALooper> &looper,
        const sp<AMessage> &outputFormat,
        const sp<MediaSource> &source,
        const sp<PersistentSurface> &persistentSurface,
        uint32_t flags)
    : mLooper(looper),
      mOutputFormat(outputFormat),
      mMeta(new MetaData),
      mFlags(flags),
      mIsVideo(false),
      mStarted(false),
      mStopping(false),
      mDoMoreWorkPending(false),
      mSetEncoderFormat(false),
      mEncoderFormat(0),
      mEncoderDataSpace(0),
      mPersistentSurface(persistentSurface),
      mInputBufferTimeOffsetUs(0),
      mFirstSampleSystemTimeUs(-1LL),
      mPausePending(false),
      mFirstSampleTimeUs(-1LL),
      mGeneration(0) {
    CHECK(mLooper != NULL);

    if (!(mFlags & FLAG_USE_SURFACE_INPUT)) {
        mPuller = new Puller(source);
    }
}

MediaCodecSource::~MediaCodecSource() {
    releaseEncoder();

    mCodecLooper->stop();
    mLooper->unregisterHandler(mReflector->id());
}

status_t MediaCodecSource::init() {
    status_t err = initEncoder();

    if (err != OK) {
        releaseEncoder();
    }

    return err;
}

status_t MediaCodecSource::initEncoder() {

    mReflector = new AHandlerReflector<MediaCodecSource>(this);
    mLooper->registerHandler(mReflector);

    mCodecLooper = new ALooper;
    mCodecLooper->setName("codec_looper");
    mCodecLooper->start();

    if (mFlags & FLAG_USE_SURFACE_INPUT) {
        mOutputFormat->setInt32(KEY_CREATE_INPUT_SURFACE_SUSPENDED, 1);
    }

    AString outputMIME;
    CHECK(mOutputFormat->findString("mime", &outputMIME));
    mIsVideo = outputMIME.startsWithIgnoreCase("video/");

    AString name;
    status_t err = NO_INIT;
    if (mOutputFormat->findString("testing-name", &name)) {
        mEncoder = MediaCodec::CreateByComponentName(mCodecLooper, name);

        mEncoderActivityNotify = new AMessage(kWhatEncoderActivity, mReflector);
        mEncoder->setCallback(mEncoderActivityNotify);

        err = mEncoder->configure(
                    mOutputFormat,
                    NULL /* nativeWindow */,
                    NULL /* crypto */,
                    MediaCodec::CONFIGURE_FLAG_ENCODE);
    } else {
        Vector<AString> matchingCodecs;
        MediaCodecList::findMatchingCodecs(
                outputMIME.c_str(), true /* encoder */,
                ((mFlags & FLAG_PREFER_SOFTWARE_CODEC) ? MediaCodecList::kPreferSoftwareCodecs : 0),
                &matchingCodecs);

        for (size_t ix = 0; ix < matchingCodecs.size(); ++ix) {
            mEncoder = MediaCodec::CreateByComponentName(
                    mCodecLooper, matchingCodecs[ix]);

            if (mEncoder == NULL) {
                continue;
            }

            ALOGV("output format is '%s'", mOutputFormat->debugString(0).c_str());

            mEncoderActivityNotify = new AMessage(kWhatEncoderActivity, mReflector);
            mEncoder->setCallback(mEncoderActivityNotify);

            err = mEncoder->configure(
                        mOutputFormat,
                        NULL /* nativeWindow */,
                        NULL /* crypto */,
                        MediaCodec::CONFIGURE_FLAG_ENCODE);

            if (err == OK) {
                break;
            }
            mEncoder->release();
            mEncoder = NULL;
        }
    }

    if (err != OK) {
        return err;
    }

    mEncoder->getOutputFormat(&mOutputFormat);
    sp<MetaData> meta = new MetaData;
    convertMessageToMetaData(mOutputFormat, meta);
    mMeta.lock().set(meta);

    if (mFlags & FLAG_USE_SURFACE_INPUT) {
        CHECK(mIsVideo);

        if (mPersistentSurface != NULL) {
            // When using persistent surface, we are only interested in the
            // consumer, but have to use PersistentSurface as a wrapper to
            // pass consumer over messages (similar to BufferProducerWrapper)
            err = mEncoder->setInputSurface(mPersistentSurface);
        } else {
            err = mEncoder->createInputSurface(&mGraphicBufferProducer);
        }

        if (err != OK) {
            return err;
        }
    }

    sp<AMessage> inputFormat;
    int32_t usingSwReadOften;
    mSetEncoderFormat = false;
    if (mEncoder->getInputFormat(&inputFormat) == OK) {
        mSetEncoderFormat = true;
        if (inputFormat->findInt32("using-sw-read-often", &usingSwReadOften)
                && usingSwReadOften) {
            // this is a SW encoder; signal source to allocate SW readable buffers
            mEncoderFormat = kDefaultSwVideoEncoderFormat;
        } else {
            mEncoderFormat = kDefaultHwVideoEncoderFormat;
        }
        if (!inputFormat->findInt32("android._dataspace", &mEncoderDataSpace)) {
            mEncoderDataSpace = kDefaultVideoEncoderDataSpace;
        }
        ALOGV("setting dataspace %#x, format %#x", mEncoderDataSpace, mEncoderFormat);
    }

    err = mEncoder->start();

    if (err != OK) {
        return err;
    }

    {
        Mutexed<Output>::Locked output(mOutput);
        output->mEncoderReachedEOS = false;
        output->mErrorCode = OK;
    }

    return OK;
}

void MediaCodecSource::releaseEncoder() {
    if (mEncoder == NULL) {
        return;
    }

    mEncoder->release();
    mEncoder.clear();
}

status_t MediaCodecSource::postSynchronouslyAndReturnError(
        const sp<AMessage> &msg) {
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err != OK) {
        return err;
    }

    if (!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

void MediaCodecSource::signalEOS(status_t err) {
    bool reachedEOS = false;
    {
        Mutexed<Output>::Locked output(mOutput);
        reachedEOS = output->mEncoderReachedEOS;
        if (!reachedEOS) {
            ALOGV("encoder (%s) reached EOS", mIsVideo ? "video" : "audio");
            // release all unread media buffers
            for (List<MediaBufferBase*>::iterator it = output->mBufferQueue.begin();
                    it != output->mBufferQueue.end(); it++) {
                (*it)->release();
            }
            output->mBufferQueue.clear();
            output->mEncoderReachedEOS = true;
            output->mErrorCode = err;
            if (!(mFlags & FLAG_USE_SURFACE_INPUT)) {
                mStopping = true;
                mPuller->stop();
            }
            output->mCond.signal();

            reachedEOS = true;
            output.unlock();
            releaseEncoder();
        }
    }

    if (mStopping && reachedEOS) {
        ALOGI("encoder (%s) stopped", mIsVideo ? "video" : "audio");
        if (mPuller != NULL) {
            mPuller->stopSource();
        }
        ALOGV("source (%s) stopped", mIsVideo ? "video" : "audio");
        // posting reply to everyone that's waiting
        List<sp<AReplyToken>>::iterator it;
        for (it = mStopReplyIDQueue.begin();
                it != mStopReplyIDQueue.end(); it++) {
            (new AMessage)->postReply(*it);
        }
        mStopReplyIDQueue.clear();
        mStopping = false;
        ++mGeneration;
    }
}

void MediaCodecSource::resume(int64_t resumeStartTimeUs) {
    CHECK(mFlags & FLAG_USE_SURFACE_INPUT);
    if (mEncoder != NULL) {
        sp<AMessage> params = new AMessage;
        params->setInt32(PARAMETER_KEY_SUSPEND, false);
        if (resumeStartTimeUs > 0) {
            params->setInt64(PARAMETER_KEY_SUSPEND_TIME, resumeStartTimeUs);
        }
        mEncoder->setParameters(params);
    }
}

status_t MediaCodecSource::feedEncoderInputBuffers() {
    MediaBufferBase* mbuf = NULL;
    while (!mAvailEncoderInputIndices.empty() && mPuller->readBuffer(&mbuf)) {
        size_t bufferIndex = *mAvailEncoderInputIndices.begin();
        mAvailEncoderInputIndices.erase(mAvailEncoderInputIndices.begin());

        int64_t timeUs = 0LL;
        uint32_t flags = 0;
        size_t size = 0;

        if (mbuf != NULL) {
            CHECK(mbuf->meta_data().findInt64(kKeyTime, &timeUs));
            if (mFirstSampleSystemTimeUs < 0LL) {
                mFirstSampleSystemTimeUs = systemTime() / 1000;
                if (mPausePending) {
                    mPausePending = false;
                    onPause(mFirstSampleSystemTimeUs);
                    mbuf->release();
                    mAvailEncoderInputIndices.push_back(bufferIndex);
                    return OK;
                }
            }

            timeUs += mInputBufferTimeOffsetUs;

            // push decoding time for video, or drift time for audio
            if (mIsVideo) {
                mDecodingTimeQueue.push_back(timeUs);
            } else {
#if DEBUG_DRIFT_TIME
                if (mFirstSampleTimeUs < 0ll) {
                    mFirstSampleTimeUs = timeUs;
                }
                int64_t driftTimeUs = 0;
                if (mbuf->meta_data().findInt64(kKeyDriftTime, &driftTimeUs)
                        && driftTimeUs) {
                    driftTimeUs = timeUs - mFirstSampleTimeUs - driftTimeUs;
                }
                mDriftTimeQueue.push_back(driftTimeUs);
#endif // DEBUG_DRIFT_TIME
            }

            sp<MediaCodecBuffer> inbuf;
            status_t err = mEncoder->getInputBuffer(bufferIndex, &inbuf);

            if (err != OK || inbuf == NULL || inbuf->data() == NULL
                    || mbuf->data() == NULL || mbuf->size() == 0) {
                mbuf->release();
                signalEOS();
                break;
            }

            size = mbuf->size();

            memcpy(inbuf->data(), mbuf->data(), size);

            if (mIsVideo) {
                // video encoder will release MediaBuffer when done
                // with underlying data.
                inbuf->meta()->setObject("mediaBufferHolder", new MediaBufferHolder(mbuf));
                mbuf->release();
            } else {
                mbuf->release();
            }
        } else {
            flags = MediaCodec::BUFFER_FLAG_EOS;
        }

        status_t err = mEncoder->queueInputBuffer(
                bufferIndex, 0, size, timeUs, flags);

        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t MediaCodecSource::onStart(MetaData *params) {
    if (mStopping || mOutput.lock()->mEncoderReachedEOS) {
        ALOGE("Failed to start while we're stopping or encoder already stopped due to EOS error");
        return INVALID_OPERATION;
    }
    int64_t startTimeUs;
    if (params == NULL || !params->findInt64(kKeyTime, &startTimeUs)) {
        startTimeUs = -1LL;
    }

    if (mStarted) {
        ALOGI("MediaCodecSource (%s) resuming", mIsVideo ? "video" : "audio");
        if (mPausePending) {
            mPausePending = false;
            return OK;
        }
        if (mIsVideo) {
            mEncoder->requestIDRFrame();
        }
        if (mFlags & FLAG_USE_SURFACE_INPUT) {
            resume(startTimeUs);
        } else {
            CHECK(mPuller != NULL);
            mPuller->resume();
        }
        return OK;
    }

    ALOGI("MediaCodecSource (%s) starting", mIsVideo ? "video" : "audio");

    status_t err = OK;

    if (mFlags & FLAG_USE_SURFACE_INPUT) {
        if (mEncoder != NULL) {
            sp<AMessage> params = new AMessage;
            params->setInt32(PARAMETER_KEY_SUSPEND, false);
            if (startTimeUs >= 0) {
                params->setInt64("skip-frames-before", startTimeUs);
            }
            mEncoder->setParameters(params);
        }
    } else {
        CHECK(mPuller != NULL);
        sp<MetaData> meta = params;
        if (mSetEncoderFormat) {
            if (meta == NULL) {
                meta = new MetaData;
            }
            meta->setInt32(kKeyPixelFormat, mEncoderFormat);
            meta->setInt32(kKeyColorSpace, mEncoderDataSpace);
        }

        sp<AMessage> notify = new AMessage(kWhatPullerNotify, mReflector);
        err = mPuller->start(meta.get(), notify);
        if (err != OK) {
            return err;
        }
    }

    ALOGI("MediaCodecSource (%s) started", mIsVideo ? "video" : "audio");

    mStarted = true;
    return OK;
}

void MediaCodecSource::onPause(int64_t pauseStartTimeUs) {
    if (mStopping || mOutput.lock()->mEncoderReachedEOS) {
        // Nothing to do
    } else if ((mFlags & FLAG_USE_SURFACE_INPUT) && (mEncoder != NULL)) {
        sp<AMessage> params = new AMessage;
        params->setInt32(PARAMETER_KEY_SUSPEND, true);
        params->setInt64(PARAMETER_KEY_SUSPEND_TIME, pauseStartTimeUs);
        mEncoder->setParameters(params);
    } else {
        CHECK(mPuller != NULL);
        mPuller->pause();
    }
}

void MediaCodecSource::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
    case kWhatPullerNotify:
    {
        int32_t eos = 0;
        if (msg->findInt32("eos", &eos) && eos) {
            ALOGV("puller (%s) reached EOS", mIsVideo ? "video" : "audio");
            signalEOS();
            break;
        }

        if (mEncoder == NULL) {
            ALOGV("got msg '%s' after encoder shutdown.", msg->debugString().c_str());
            break;
        }

        feedEncoderInputBuffers();
        break;
    }
    case kWhatEncoderActivity:
    {
        if (mEncoder == NULL) {
            break;
        }

        int32_t cbID;
        CHECK(msg->findInt32("callbackID", &cbID));
        if (cbID == MediaCodec::CB_INPUT_AVAILABLE) {
            int32_t index;
            CHECK(msg->findInt32("index", &index));

            mAvailEncoderInputIndices.push_back(index);
            feedEncoderInputBuffers();
        } else if (cbID == MediaCodec::CB_OUTPUT_FORMAT_CHANGED) {
            status_t err = mEncoder->getOutputFormat(&mOutputFormat);
            if (err != OK) {
                signalEOS(err);
                break;
            }
            sp<MetaData> meta = new MetaData;
            convertMessageToMetaData(mOutputFormat, meta);
            mMeta.lock().set(meta);
        } else if (cbID == MediaCodec::CB_OUTPUT_AVAILABLE) {
            int32_t index;
            size_t offset;
            size_t size;
            int64_t timeUs;
            int32_t flags;

            CHECK(msg->findInt32("index", &index));
            CHECK(msg->findSize("offset", &offset));
            CHECK(msg->findSize("size", &size));
            CHECK(msg->findInt64("timeUs", &timeUs));
            CHECK(msg->findInt32("flags", &flags));

            if (flags & MediaCodec::BUFFER_FLAG_EOS) {
                mEncoder->releaseOutputBuffer(index);
                signalEOS();
                break;
            }

            sp<MediaCodecBuffer> outbuf;
            status_t err = mEncoder->getOutputBuffer(index, &outbuf);
            if (err != OK || outbuf == NULL || outbuf->data() == NULL
                || outbuf->size() == 0) {
                signalEOS();
                break;
            }

            MediaBufferBase *mbuf = new MediaBuffer(outbuf->size());
            mbuf->setObserver(this);
            mbuf->add_ref();

            if (!(flags & MediaCodec::BUFFER_FLAG_CODECCONFIG)) {
                if (mIsVideo) {
                    int64_t decodingTimeUs;
                    if (mFlags & FLAG_USE_SURFACE_INPUT) {
                        if (mFirstSampleSystemTimeUs < 0LL) {
                            mFirstSampleSystemTimeUs = systemTime() / 1000;
                            if (mPausePending) {
                                mPausePending = false;
                                onPause(mFirstSampleSystemTimeUs);
                                mbuf->release();
                                break;
                            }
                        }
                        // Timestamp offset is already adjusted in GraphicBufferSource.
                        // GraphicBufferSource is supposed to discard samples
                        // queued before start, and offset timeUs by start time
                        CHECK_GE(timeUs, 0LL);
                        // TODO:
                        // Decoding time for surface source is unavailable,
                        // use presentation time for now. May need to move
                        // this logic into MediaCodec.
                        decodingTimeUs = timeUs;
                    } else {
                        CHECK(!mDecodingTimeQueue.empty());
                        decodingTimeUs = *(mDecodingTimeQueue.begin());
                        mDecodingTimeQueue.erase(mDecodingTimeQueue.begin());
                    }
                    mbuf->meta_data().setInt64(kKeyDecodingTime, decodingTimeUs);

                    ALOGV("[video] time %" PRId64 " us (%.2f secs), dts/pts diff %" PRId64,
                            timeUs, timeUs / 1E6, decodingTimeUs - timeUs);
                } else {
                    int64_t driftTimeUs = 0;
#if DEBUG_DRIFT_TIME
                    CHECK(!mDriftTimeQueue.empty());
                    driftTimeUs = *(mDriftTimeQueue.begin());
                    mDriftTimeQueue.erase(mDriftTimeQueue.begin());
                    mbuf->meta_data().setInt64(kKeyDriftTime, driftTimeUs);
#endif // DEBUG_DRIFT_TIME
                    ALOGV("[audio] time %" PRId64 " us (%.2f secs), drift %" PRId64,
                            timeUs, timeUs / 1E6, driftTimeUs);
                }
                mbuf->meta_data().setInt64(kKeyTime, timeUs);
            } else {
                mbuf->meta_data().setInt64(kKeyTime, 0LL);
                mbuf->meta_data().setInt32(kKeyIsCodecConfig, true);
            }
            if (flags & MediaCodec::BUFFER_FLAG_SYNCFRAME) {
                mbuf->meta_data().setInt32(kKeyIsSyncFrame, true);
            }
            memcpy(mbuf->data(), outbuf->data(), outbuf->size());

            {
                Mutexed<Output>::Locked output(mOutput);
                output->mBufferQueue.push_back(mbuf);
                output->mCond.signal();
            }

            mEncoder->releaseOutputBuffer(index);
       } else if (cbID == MediaCodec::CB_ERROR) {
            status_t err;
            CHECK(msg->findInt32("err", &err));
            ALOGE("Encoder (%s) reported error : 0x%x",
                    mIsVideo ? "video" : "audio", err);
            if (!(mFlags & FLAG_USE_SURFACE_INPUT)) {
                mStopping = true;
                mPuller->stop();
            }
            signalEOS();
       }
       break;
    }
    case kWhatStart:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<RefBase> obj;
        CHECK(msg->findObject("meta", &obj));
        MetaData *params = static_cast<MetaData *>(obj.get());

        sp<AMessage> response = new AMessage;
        response->setInt32("err", onStart(params));
        response->postReply(replyID);
        break;
    }
    case kWhatStop:
    {
        ALOGI("encoder (%s) stopping", mIsVideo ? "video" : "audio");

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if (mOutput.lock()->mEncoderReachedEOS) {
            // if we already reached EOS, reply and return now
            ALOGI("encoder (%s) already stopped",
                    mIsVideo ? "video" : "audio");
            (new AMessage)->postReply(replyID);
            break;
        }

        mStopReplyIDQueue.push_back(replyID);
        if (mStopping) {
            // nothing to do if we're already stopping, reply will be posted
            // to all when we're stopped.
            break;
        }

        mStopping = true;

        int64_t timeoutUs = kStopTimeoutUs;
        // if using surface, signal source EOS and wait for EOS to come back.
        // otherwise, stop puller (which also clears the input buffer queue)
        // and wait for the EOS message. We cannot call source->stop() because
        // the encoder may still be processing input buffers.
        if (mFlags & FLAG_USE_SURFACE_INPUT) {
            mEncoder->signalEndOfInputStream();
            // Increase the timeout if there is delay in the GraphicBufferSource
            sp<AMessage> inputFormat;
            int64_t stopTimeOffsetUs;
            if (mEncoder->getInputFormat(&inputFormat) == OK &&
                    inputFormat->findInt64("android._stop-time-offset-us", &stopTimeOffsetUs) &&
                    stopTimeOffsetUs > 0) {
                if (stopTimeOffsetUs > kMaxStopTimeOffsetUs) {
                    ALOGW("Source stopTimeOffsetUs %lld too large, limit at %lld us",
                        (long long)stopTimeOffsetUs, (long long)kMaxStopTimeOffsetUs);
                    stopTimeOffsetUs = kMaxStopTimeOffsetUs;
                }
                timeoutUs += stopTimeOffsetUs;
            } else {
                // Use kMaxStopTimeOffsetUs if stop time offset is not provided by input source
                timeoutUs = kMaxStopTimeOffsetUs;
            }
        } else {
            mPuller->stop();
        }

        // complete stop even if encoder/puller stalled
        sp<AMessage> timeoutMsg = new AMessage(kWhatStopStalled, mReflector);
        timeoutMsg->setInt32("generation", mGeneration);
        timeoutMsg->post(timeoutUs);
        break;
    }

    case kWhatStopStalled:
    {
        int32_t generation;
        CHECK(msg->findInt32("generation", &generation));
        if (generation != mGeneration) {
             break;
        }

        if (!(mFlags & FLAG_USE_SURFACE_INPUT)) {
            ALOGV("source (%s) stopping", mIsVideo ? "video" : "audio");
            mPuller->interruptSource();
            ALOGV("source (%s) stopped", mIsVideo ? "video" : "audio");
        }
        signalEOS();
        break;
    }

    case kWhatPause:
    {
        if (mFirstSampleSystemTimeUs < 0) {
            mPausePending = true;
        } else {
            sp<RefBase> obj;
            CHECK(msg->findObject("meta", &obj));
            MetaData *params = static_cast<MetaData *>(obj.get());
            int64_t pauseStartTimeUs = -1;
            if (params == NULL || !params->findInt64(kKeyTime, &pauseStartTimeUs)) {
                pauseStartTimeUs = -1LL;
            }
            onPause(pauseStartTimeUs);
        }
        break;
    }
    case kWhatSetInputBufferTimeOffset:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        status_t err = OK;
        CHECK(msg->findInt64(PARAMETER_KEY_OFFSET_TIME, &mInputBufferTimeOffsetUs));

        // Propagate the timestamp offset to GraphicBufferSource.
        if (mFlags & FLAG_USE_SURFACE_INPUT) {
            sp<AMessage> params = new AMessage;
            params->setInt64(PARAMETER_KEY_OFFSET_TIME, mInputBufferTimeOffsetUs);
            err = mEncoder->setParameters(params);
        }

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);
        response->postReply(replyID);
        break;
    }
    case kWhatSetStopTimeUs:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        status_t err = OK;
        int64_t stopTimeUs;
        CHECK(msg->findInt64("stop-time-us", &stopTimeUs));

        // Propagate the stop time to GraphicBufferSource.
        if (mFlags & FLAG_USE_SURFACE_INPUT) {
            sp<AMessage> params = new AMessage;
            params->setInt64("stop-time-us", stopTimeUs);
            err = mEncoder->setParameters(params);
        } else {
            err = mPuller->setStopTimeUs(stopTimeUs);
        }

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);
        response->postReply(replyID);
        break;
    }
    case kWhatGetFirstSampleSystemTimeUs:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;
        response->setInt64("time-us", mFirstSampleSystemTimeUs);
        response->postReply(replyID);
        break;
    }
    default:
        TRESPASS();
    }
}

} // namespace android
