/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "MediaSync"
#include <inttypes.h>

#include <gui/BufferQueue.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>

#include <media/AudioTrack.h>
#include <media/stagefright/MediaClock.h>
#include <media/stagefright/MediaSync.h>
#include <media/stagefright/VideoFrameScheduler.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>

#include <ui/GraphicBuffer.h>

#include <system/window.h>

// Maximum late time allowed for a video frame to be rendered. When a video
// frame arrives later than this number, it will be discarded without rendering.
static const int64_t kMaxAllowedVideoLateTimeUs = 40000LL;

namespace android {

// static
sp<MediaSync> MediaSync::create() {
    sp<MediaSync> sync = new MediaSync();
    sync->mLooper->registerHandler(sync);
    return sync;
}

MediaSync::MediaSync()
      : mIsAbandoned(false),
        mMutex(),
        mReleaseCondition(),
        mNumOutstandingBuffers(0),
        mUsageFlagsFromOutput(0),
        mMaxAcquiredBufferCount(1),
        mReturnPendingInputFrame(false),
        mNativeSampleRateInHz(0),
        mNumFramesWritten(0),
        mHasAudio(false),
        mNextBufferItemMediaUs(-1),
        mPlaybackRate(0.0) {
    mMediaClock = new MediaClock;
    mMediaClock->init();

    // initialize settings
    mPlaybackSettings = AUDIO_PLAYBACK_RATE_DEFAULT;
    mPlaybackSettings.mSpeed = mPlaybackRate;

    mLooper = new ALooper;
    mLooper->setName("MediaSync");
    mLooper->start(false, false, ANDROID_PRIORITY_AUDIO);
}

MediaSync::~MediaSync() {
    if (mInput != NULL) {
        mInput->consumerDisconnect();
    }
    if (mOutput != NULL) {
        mOutput->disconnect(NATIVE_WINDOW_API_MEDIA);
    }

    if (mLooper != NULL) {
        mLooper->unregisterHandler(id());
        mLooper->stop();
    }
}

status_t MediaSync::setSurface(const sp<IGraphicBufferProducer> &output) {
    Mutex::Autolock lock(mMutex);

    if (output == mOutput) {
        return NO_ERROR;  // same output surface.
    }

    if (output == NULL && mSyncSettings.mSource == AVSYNC_SOURCE_VSYNC) {
        ALOGE("setSurface: output surface is used as sync source and cannot be removed.");
        return INVALID_OPERATION;
    }

    if (output != NULL) {
        int newUsage = 0;
        output->query(NATIVE_WINDOW_CONSUMER_USAGE_BITS, &newUsage);

        // Check usage flags only when current output surface has been used to create input surface.
        if (mOutput != NULL && mInput != NULL) {
            int ignoredFlags = (GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_COMPOSER
                    | GRALLOC_USAGE_EXTERNAL_DISP);
            // New output surface is not allowed to add new usage flag except ignored ones.
            if ((newUsage & ~(mUsageFlagsFromOutput | ignoredFlags)) != 0) {
                ALOGE("setSurface: new output surface has new usage flag not used by current one.");
                return BAD_VALUE;
            }
        }

        // Try to connect to new output surface. If failed, current output surface will not
        // be changed.
        IGraphicBufferProducer::QueueBufferOutput queueBufferOutput;
        sp<OutputListener> listener(new OutputListener(this, output));
        IInterface::asBinder(output)->linkToDeath(listener);
        status_t status =
            output->connect(listener,
                            NATIVE_WINDOW_API_MEDIA,
                            true /* producerControlledByApp */,
                            &queueBufferOutput);
        if (status != NO_ERROR) {
            ALOGE("setSurface: failed to connect (%d)", status);
            return status;
        }

        if (mFrameScheduler == NULL) {
            mFrameScheduler = new VideoFrameScheduler();
            mFrameScheduler->init();
        }
    }

    if (mOutput != NULL) {
        mOutput->disconnect(NATIVE_WINDOW_API_MEDIA);
        while (!mBuffersSentToOutput.isEmpty()) {
            returnBufferToInput_l(mBuffersSentToOutput.valueAt(0), Fence::NO_FENCE);
            mBuffersSentToOutput.removeItemsAt(0);
        }
    }

    mOutput = output;

    return NO_ERROR;
}

// |audioTrack| is used only for querying information.
status_t MediaSync::setAudioTrack(const sp<AudioTrack> &audioTrack) {
    Mutex::Autolock lock(mMutex);

    // TODO: support audio track change.
    if (mAudioTrack != NULL) {
        ALOGE("setAudioTrack: audioTrack has already been configured.");
        return INVALID_OPERATION;
    }

    if (audioTrack == NULL && mSyncSettings.mSource == AVSYNC_SOURCE_AUDIO) {
        ALOGE("setAudioTrack: audioTrack is used as sync source and cannot be removed.");
        return INVALID_OPERATION;
    }

    if (audioTrack != NULL) {
        // check if audio track supports the playback settings
        if (mPlaybackSettings.mSpeed != 0.f
                && audioTrack->setPlaybackRate(mPlaybackSettings) != OK) {
            ALOGE("playback settings are not supported by the audio track");
            return INVALID_OPERATION;
        }
        uint32_t nativeSampleRateInHz = audioTrack->getOriginalSampleRate();
        if (nativeSampleRateInHz <= 0) {
            ALOGE("setAudioTrack: native sample rate should be positive.");
            return BAD_VALUE;
        }
        mAudioTrack = audioTrack;
        mNativeSampleRateInHz = nativeSampleRateInHz;
        (void)setPlaybackSettings_l(mPlaybackSettings);
    }
    else {
        mAudioTrack = NULL;
        mNativeSampleRateInHz = 0;
    }

    // potentially resync to new source
    resync_l();
    return OK;
}

status_t MediaSync::createInputSurface(
        sp<IGraphicBufferProducer> *outBufferProducer) {
    if (outBufferProducer == NULL) {
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);

    if (mOutput == NULL) {
        return NO_INIT;
    }

    if (mInput != NULL) {
        return INVALID_OPERATION;
    }

    sp<IGraphicBufferProducer> bufferProducer;
    sp<IGraphicBufferConsumer> bufferConsumer;
    BufferQueue::createBufferQueue(&bufferProducer, &bufferConsumer);

    sp<InputListener> listener(new InputListener(this));
    IInterface::asBinder(bufferConsumer)->linkToDeath(listener);
    status_t status =
        bufferConsumer->consumerConnect(listener, false /* controlledByApp */);
    if (status == NO_ERROR) {
        bufferConsumer->setConsumerName(String8("MediaSync"));
        // propagate usage bits from output surface
        mUsageFlagsFromOutput = 0;
        mOutput->query(NATIVE_WINDOW_CONSUMER_USAGE_BITS, &mUsageFlagsFromOutput);
        bufferConsumer->setConsumerUsageBits(mUsageFlagsFromOutput);
        *outBufferProducer = bufferProducer;
        mInput = bufferConsumer;

        // set undequeued buffer count
        int minUndequeuedBuffers;
        mOutput->query(NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBuffers);
        mMaxAcquiredBufferCount = minUndequeuedBuffers;
        bufferConsumer->setMaxAcquiredBufferCount(mMaxAcquiredBufferCount);
    }
    return status;
}

void MediaSync::resync_l() {
    AVSyncSource src = mSyncSettings.mSource;
    if (src == AVSYNC_SOURCE_DEFAULT) {
        if (mAudioTrack != NULL) {
            src = AVSYNC_SOURCE_AUDIO;
        } else {
            src = AVSYNC_SOURCE_SYSTEM_CLOCK;
        }
    }

    // TODO: resync ourselves to the current clock (e.g. on sync source change)
    updatePlaybackRate_l(mPlaybackRate);
}

void MediaSync::updatePlaybackRate_l(float rate) {
    if (rate > mPlaybackRate) {
        mNextBufferItemMediaUs = -1;
    }
    mPlaybackRate = rate;
    // TODO: update frame scheduler with this info
    mMediaClock->setPlaybackRate(rate);
    onDrainVideo_l();
}

sp<const MediaClock> MediaSync::getMediaClock() {
    return mMediaClock;
}

status_t MediaSync::getPlayTimeForPendingAudioFrames(int64_t *outTimeUs) {
    Mutex::Autolock lock(mMutex);
    // User should check the playback rate if it doesn't want to receive a
    // huge number for play time.
    if (mPlaybackRate == 0.0f) {
        *outTimeUs = INT64_MAX;
        return OK;
    }

    uint32_t numFramesPlayed = 0;
    if (mAudioTrack != NULL) {
        status_t res = mAudioTrack->getPosition(&numFramesPlayed);
        if (res != OK) {
            return res;
        }
    }

    int64_t numPendingFrames = mNumFramesWritten - numFramesPlayed;
    if (numPendingFrames < 0) {
        numPendingFrames = 0;
        ALOGW("getPlayTimeForPendingAudioFrames: pending frame count is negative.");
    }
    double timeUs = numPendingFrames * 1000000.0
            / (mNativeSampleRateInHz * (double)mPlaybackRate);
    if (timeUs > (double)INT64_MAX) {
        // Overflow.
        *outTimeUs = INT64_MAX;
        ALOGW("getPlayTimeForPendingAudioFrames: play time for pending audio frames "
              "is too high, possibly due to super low playback rate(%f)", mPlaybackRate);
    } else {
        *outTimeUs = (int64_t)timeUs;
    }

    return OK;
}

status_t MediaSync::updateQueuedAudioData(
        size_t sizeInBytes, int64_t presentationTimeUs) {
    if (sizeInBytes == 0) {
        return OK;
    }

    Mutex::Autolock lock(mMutex);

    if (mAudioTrack == NULL) {
        ALOGW("updateQueuedAudioData: audioTrack has NOT been configured.");
        return INVALID_OPERATION;
    }

    int64_t numFrames = sizeInBytes / mAudioTrack->frameSize();
    int64_t maxMediaTimeUs = presentationTimeUs
            + getDurationIfPlayedAtNativeSampleRate_l(numFrames);

    int64_t nowUs = ALooper::GetNowUs();
    int64_t nowMediaUs = presentationTimeUs
            - getDurationIfPlayedAtNativeSampleRate_l(mNumFramesWritten)
            + getPlayedOutAudioDurationMedia_l(nowUs);

    mNumFramesWritten += numFrames;

    int64_t oldRealTime = -1;
    if (mNextBufferItemMediaUs != -1) {
        oldRealTime = getRealTime(mNextBufferItemMediaUs, nowUs);
    }

    mMediaClock->updateAnchor(nowMediaUs, nowUs, maxMediaTimeUs);
    mHasAudio = true;

    if (oldRealTime != -1) {
        int64_t newRealTime = getRealTime(mNextBufferItemMediaUs, nowUs);
        if (newRealTime >= oldRealTime) {
            return OK;
        }
    }

    mNextBufferItemMediaUs = -1;
    onDrainVideo_l();
    return OK;
}

void MediaSync::setName(const AString &name) {
    Mutex::Autolock lock(mMutex);
    mInput->setConsumerName(String8(name.c_str()));
}

void MediaSync::flush() {
    Mutex::Autolock lock(mMutex);
    if (mFrameScheduler != NULL) {
        mFrameScheduler->restart();
    }
    while (!mBufferItems.empty()) {
        BufferItem *bufferItem = &*mBufferItems.begin();
        returnBufferToInput_l(bufferItem->mGraphicBuffer, bufferItem->mFence);
        mBufferItems.erase(mBufferItems.begin());
    }
    mNextBufferItemMediaUs = -1;
    mNumFramesWritten = 0;
    mReturnPendingInputFrame = true;
    mReleaseCondition.signal();
    mMediaClock->clearAnchor();
}

status_t MediaSync::setVideoFrameRateHint(float rate) {
    Mutex::Autolock lock(mMutex);
    if (rate < 0.f) {
        return BAD_VALUE;
    }
    if (mFrameScheduler != NULL) {
        mFrameScheduler->init(rate);
    }
    return OK;
}

float MediaSync::getVideoFrameRate() {
    Mutex::Autolock lock(mMutex);
    if (mFrameScheduler != NULL) {
        float fps = mFrameScheduler->getFrameRate();
        if (fps > 0.f) {
            return fps;
        }
    }

    // we don't have or know the frame rate
    return -1.f;
}

status_t MediaSync::setSyncSettings(const AVSyncSettings &syncSettings) {
    // validate settings
    if (syncSettings.mSource >= AVSYNC_SOURCE_MAX
            || syncSettings.mAudioAdjustMode >= AVSYNC_AUDIO_ADJUST_MODE_MAX
            || syncSettings.mTolerance < 0.f
            || syncSettings.mTolerance >= AVSYNC_TOLERANCE_MAX) {
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);

    // verify that we have the sync source
    switch (syncSettings.mSource) {
        case AVSYNC_SOURCE_AUDIO:
            if (mAudioTrack == NULL) {
                ALOGE("setSyncSettings: audio sync source requires an audio track");
                return BAD_VALUE;
            }
            break;
        case AVSYNC_SOURCE_VSYNC:
            if (mOutput == NULL) {
                ALOGE("setSyncSettings: vsync sync source requires an output surface");
                return BAD_VALUE;
            }
            break;
        default:
            break;
    }

    mSyncSettings = syncSettings;
    resync_l();
    return OK;
}

void MediaSync::getSyncSettings(AVSyncSettings *syncSettings) {
    Mutex::Autolock lock(mMutex);
    *syncSettings = mSyncSettings;
}

status_t MediaSync::setPlaybackSettings(const AudioPlaybackRate &rate) {
    Mutex::Autolock lock(mMutex);

    status_t err = setPlaybackSettings_l(rate);
    if (err == OK) {
        // TODO: adjust rate if using VSYNC as source
        updatePlaybackRate_l(rate.mSpeed);
    }
    return err;
}

status_t MediaSync::setPlaybackSettings_l(const AudioPlaybackRate &rate) {
    if (rate.mSpeed < 0.f || rate.mPitch < 0.f) {
        // We don't validate other audio settings.
        // They will be validated when/if audiotrack is set.
        return BAD_VALUE;
    }

    if (mAudioTrack != NULL) {
        if (rate.mSpeed == 0.f) {
            mAudioTrack->pause();
        } else {
            status_t err = mAudioTrack->setPlaybackRate(rate);
            if (err != OK) {
                return BAD_VALUE;
            }

            // ignore errors
            (void)mAudioTrack->start();
        }
    }
    mPlaybackSettings = rate;
    return OK;
}

void MediaSync::getPlaybackSettings(AudioPlaybackRate *rate) {
    Mutex::Autolock lock(mMutex);
    *rate = mPlaybackSettings;
}

int64_t MediaSync::getRealTime(int64_t mediaTimeUs, int64_t nowUs) {
    int64_t realUs;
    if (mMediaClock->getRealTimeFor(mediaTimeUs, &realUs) != OK) {
        // If failed to get current position, e.g. due to audio clock is
        // not ready, then just play out video immediately without delay.
        return nowUs;
    }
    return realUs;
}

int64_t MediaSync::getDurationIfPlayedAtNativeSampleRate_l(int64_t numFrames) {
    return (numFrames * 1000000LL / mNativeSampleRateInHz);
}

int64_t MediaSync::getPlayedOutAudioDurationMedia_l(int64_t nowUs) {
    CHECK(mAudioTrack != NULL);

    uint32_t numFramesPlayed;
    int64_t numFramesPlayedAtUs;
    AudioTimestamp ts;

    status_t res = mAudioTrack->getTimestamp(ts);
    if (res == OK) {
        // case 1: mixing audio tracks.
        numFramesPlayed = ts.mPosition;
        numFramesPlayedAtUs = ts.mTime.tv_sec * 1000000LL + ts.mTime.tv_nsec / 1000;
        //ALOGD("getTimestamp: OK %d %lld",
        //      numFramesPlayed, (long long)numFramesPlayedAtUs);
    } else if (res == WOULD_BLOCK) {
        // case 2: transitory state on start of a new track
        numFramesPlayed = 0;
        numFramesPlayedAtUs = nowUs;
        //ALOGD("getTimestamp: WOULD_BLOCK %d %lld",
        //      numFramesPlayed, (long long)numFramesPlayedAtUs);
    } else {
        // case 3: transitory at new track or audio fast tracks.
        res = mAudioTrack->getPosition(&numFramesPlayed);
        CHECK_EQ(res, (status_t)OK);
        numFramesPlayedAtUs = nowUs;
        numFramesPlayedAtUs += 1000LL * mAudioTrack->latency() / 2; /* XXX */
        //ALOGD("getPosition: %d %lld", numFramesPlayed, (long long)numFramesPlayedAtUs);
    }

    //can't be negative until 12.4 hrs, test.
    //CHECK_EQ(numFramesPlayed & (1 << 31), 0);
    int64_t durationUs =
        getDurationIfPlayedAtNativeSampleRate_l(numFramesPlayed)
            + nowUs - numFramesPlayedAtUs;
    if (durationUs < 0) {
        // Occurs when numFramesPlayed position is very small and the following:
        // (1) In case 1, the time nowUs is computed before getTimestamp() is
        //     called and numFramesPlayedAtUs is greater than nowUs by time more
        //     than numFramesPlayed.
        // (2) In case 3, using getPosition and adding mAudioTrack->latency()
        //     to numFramesPlayedAtUs, by a time amount greater than
        //     numFramesPlayed.
        //
        // Both of these are transitory conditions.
        ALOGV("getPlayedOutAudioDurationMedia_l: negative duration %lld "
              "set to zero", (long long)durationUs);
        durationUs = 0;
    }
    ALOGV("getPlayedOutAudioDurationMedia_l(%lld) nowUs(%lld) frames(%u) "
          "framesAt(%lld)",
          (long long)durationUs, (long long)nowUs, numFramesPlayed,
          (long long)numFramesPlayedAtUs);
    return durationUs;
}

void MediaSync::onDrainVideo_l() {
    if (!isPlaying()) {
        return;
    }

    while (!mBufferItems.empty()) {
        int64_t nowUs = ALooper::GetNowUs();
        BufferItem *bufferItem = &*mBufferItems.begin();
        int64_t itemMediaUs = bufferItem->mTimestamp / 1000;
        int64_t itemRealUs = getRealTime(itemMediaUs, nowUs);

        // adjust video frame PTS based on vsync
        itemRealUs = mFrameScheduler->schedule(itemRealUs * 1000) / 1000;
        int64_t twoVsyncsUs = 2 * (mFrameScheduler->getVsyncPeriod() / 1000);

        // post 2 display refreshes before rendering is due
        if (itemRealUs <= nowUs + twoVsyncsUs) {
            ALOGV("adjusting PTS from %lld to %lld",
                    (long long)bufferItem->mTimestamp / 1000, (long long)itemRealUs);
            bufferItem->mTimestamp = itemRealUs * 1000;
            bufferItem->mIsAutoTimestamp = false;

            if (mHasAudio) {
                if (nowUs - itemRealUs <= kMaxAllowedVideoLateTimeUs) {
                    renderOneBufferItem_l(*bufferItem);
                } else {
                    // too late.
                    returnBufferToInput_l(
                            bufferItem->mGraphicBuffer, bufferItem->mFence);
                    mFrameScheduler->restart();
                }
            } else {
                // always render video buffer in video-only mode.
                renderOneBufferItem_l(*bufferItem);

                // smooth out videos >= 10fps
                mMediaClock->updateAnchor(
                        itemMediaUs, nowUs, itemMediaUs + 100000);
            }

            mBufferItems.erase(mBufferItems.begin());
            mNextBufferItemMediaUs = -1;
        } else {
            if (mNextBufferItemMediaUs == -1
                    || mNextBufferItemMediaUs > itemMediaUs) {
                sp<AMessage> msg = new AMessage(kWhatDrainVideo, this);
                msg->post(itemRealUs - nowUs - twoVsyncsUs);
                mNextBufferItemMediaUs = itemMediaUs;
            }
            break;
        }
    }
}

void MediaSync::onFrameAvailableFromInput() {
    Mutex::Autolock lock(mMutex);

    const static nsecs_t kAcquireWaitTimeout = 2000000000; // 2 seconds

    mReturnPendingInputFrame = false;

    // If there are too many outstanding buffers, wait until a buffer is
    // released back to the input in onBufferReleased.
    // NOTE: BufferQueue allows dequeuing maxAcquiredBufferCount + 1 buffers
    while (mNumOutstandingBuffers > mMaxAcquiredBufferCount
            && !mIsAbandoned && !mReturnPendingInputFrame) {
        if (mReleaseCondition.waitRelative(mMutex, kAcquireWaitTimeout) != OK) {
            ALOGI_IF(mPlaybackRate != 0.f, "still waiting to release a buffer before acquire");
        }

        // If the sync is abandoned while we are waiting, the release
        // condition variable will be broadcast, and we should just return
        // without attempting to do anything more (since the input queue will
        // also be abandoned).
        if (mIsAbandoned) {
            return;
        }
    }

    // Acquire and detach the buffer from the input.
    BufferItem bufferItem;
    status_t status = mInput->acquireBuffer(&bufferItem, 0 /* presentWhen */);
    if (status != NO_ERROR) {
        ALOGE("acquiring buffer from input failed (%d)", status);
        return;
    }
    ++mNumOutstandingBuffers;

    ALOGV("acquired buffer %#llx from input", (long long)bufferItem.mGraphicBuffer->getId());

    status = mInput->detachBuffer(bufferItem.mSlot);
    if (status != NO_ERROR) {
        ALOGE("detaching buffer from input failed (%d)", status);
        if (status == NO_INIT) {
            // If the input has been abandoned, move on.
            onAbandoned_l(true /* isInput */);
        }
        return;
    }

    if (mBuffersFromInput.indexOfKey(bufferItem.mGraphicBuffer->getId()) >= 0) {
        // Something is wrong since this buffer should be at our hands, bail.
        ALOGE("received buffer multiple times from input");
        mInput->consumerDisconnect();
        onAbandoned_l(true /* isInput */);
        return;
    }
    mBuffersFromInput.add(bufferItem.mGraphicBuffer->getId(), bufferItem.mGraphicBuffer);

    // If flush happened while waiting for a buffer to be released, simply return it
    // TRICKY: do it here after it is detached so that we don't have to cache mGraphicBuffer.
    if (mReturnPendingInputFrame) {
        mReturnPendingInputFrame = false;
        returnBufferToInput_l(bufferItem.mGraphicBuffer, bufferItem.mFence);
        return;
    }

    mBufferItems.push_back(bufferItem);

    if (mBufferItems.size() == 1) {
        onDrainVideo_l();
    }
}

void MediaSync::renderOneBufferItem_l(const BufferItem &bufferItem) {
    IGraphicBufferProducer::QueueBufferInput queueInput(
            bufferItem.mTimestamp,
            bufferItem.mIsAutoTimestamp,
            bufferItem.mDataSpace,
            bufferItem.mCrop,
            static_cast<int32_t>(bufferItem.mScalingMode),
            bufferItem.mTransform,
            bufferItem.mFence);

    // Attach and queue the buffer to the output.
    int slot;
    mOutput->setGenerationNumber(bufferItem.mGraphicBuffer->getGenerationNumber());
    status_t status = mOutput->attachBuffer(&slot, bufferItem.mGraphicBuffer);
    ALOGE_IF(status != NO_ERROR, "attaching buffer to output failed (%d)", status);
    if (status == NO_ERROR) {
        IGraphicBufferProducer::QueueBufferOutput queueOutput;
        status = mOutput->queueBuffer(slot, queueInput, &queueOutput);
        ALOGE_IF(status != NO_ERROR, "queueing buffer to output failed (%d)", status);
    }

    if (status != NO_ERROR) {
        returnBufferToInput_l(bufferItem.mGraphicBuffer, bufferItem.mFence);
        if (status == NO_INIT) {
            // If the output has been abandoned, move on.
            onAbandoned_l(false /* isInput */);
        }
        return;
    }

    if (mBuffersSentToOutput.indexOfKey(bufferItem.mGraphicBuffer->getId()) >= 0) {
        // Something is wrong since this buffer should be held by output now, bail.
        mInput->consumerDisconnect();
        onAbandoned_l(true /* isInput */);
        return;
    }
    mBuffersSentToOutput.add(bufferItem.mGraphicBuffer->getId(), bufferItem.mGraphicBuffer);

    ALOGV("queued buffer %#llx to output", (long long)bufferItem.mGraphicBuffer->getId());
}

void MediaSync::onBufferReleasedByOutput(sp<IGraphicBufferProducer> &output) {
    Mutex::Autolock lock(mMutex);

    if (output != mOutput) {
        return;  // This is not the current output, ignore.
    }

    sp<GraphicBuffer> buffer;
    sp<Fence> fence;
    status_t status = mOutput->detachNextBuffer(&buffer, &fence);
    ALOGE_IF(status != NO_ERROR, "detaching buffer from output failed (%d)", status);

    if (status == NO_INIT) {
        // If the output has been abandoned, we can't do anything else,
        // since buffer is invalid.
        onAbandoned_l(false /* isInput */);
        return;
    }

    ALOGV("detached buffer %#llx from output", (long long)buffer->getId());

    // If we've been abandoned, we can't return the buffer to the input, so just
    // move on.
    if (mIsAbandoned) {
        return;
    }

    ssize_t ix = mBuffersSentToOutput.indexOfKey(buffer->getId());
    if (ix < 0) {
        // The buffer is unknown, maybe leftover, ignore.
        return;
    }
    mBuffersSentToOutput.removeItemsAt(ix);

    returnBufferToInput_l(buffer, fence);
}

void MediaSync::returnBufferToInput_l(
        const sp<GraphicBuffer> &buffer, const sp<Fence> &fence) {
    ssize_t ix = mBuffersFromInput.indexOfKey(buffer->getId());
    if (ix < 0) {
        // The buffer is unknown, something is wrong, bail.
        ALOGE("output returned unknown buffer");
        mOutput->disconnect(NATIVE_WINDOW_API_MEDIA);
        onAbandoned_l(false /* isInput */);
        return;
    }
    sp<GraphicBuffer> oldBuffer = mBuffersFromInput.valueAt(ix);
    mBuffersFromInput.removeItemsAt(ix);

    // Attach and release the buffer back to the input.
    int consumerSlot;
    status_t status = mInput->attachBuffer(&consumerSlot, oldBuffer);
    ALOGE_IF(status != NO_ERROR, "attaching buffer to input failed (%d)", status);
    if (status == NO_ERROR) {
        status = mInput->releaseBuffer(consumerSlot, 0 /* frameNumber */,
                EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, fence);
        ALOGE_IF(status != NO_ERROR, "releasing buffer to input failed (%d)", status);
    }

    // Notify any waiting onFrameAvailable calls.
    --mNumOutstandingBuffers;
    mReleaseCondition.signal();

    if (status == NO_ERROR) {
        ALOGV("released buffer %#llx to input", (long long)oldBuffer->getId());
    }
}

void MediaSync::onAbandoned_l(bool isInput) {
    ALOGE("the %s has abandoned me", (isInput ? "input" : "output"));
    if (!mIsAbandoned) {
        if (isInput) {
            mOutput->disconnect(NATIVE_WINDOW_API_MEDIA);
        } else {
            mInput->consumerDisconnect();
        }
        mIsAbandoned = true;
    }
    mReleaseCondition.broadcast();
}

void MediaSync::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatDrainVideo:
        {
            Mutex::Autolock lock(mMutex);
            if (mNextBufferItemMediaUs != -1) {
                int64_t nowUs = ALooper::GetNowUs();
                int64_t itemRealUs = getRealTime(mNextBufferItemMediaUs, nowUs);

                // The message could arrive earlier than expected due to
                // various reasons, e.g., media clock has been changed because
                // of new anchor time or playback rate. In such cases, the
                // message needs to be re-posted.
                if (itemRealUs > nowUs) {
                    msg->post(itemRealUs - nowUs);
                    break;
                }
            }

            onDrainVideo_l();
            break;
        }

        default:
            TRESPASS();
            break;
    }
}

MediaSync::InputListener::InputListener(const sp<MediaSync> &sync)
      : mSync(sync) {}

MediaSync::InputListener::~InputListener() {}

void MediaSync::InputListener::onFrameAvailable(const BufferItem &/* item */) {
    mSync->onFrameAvailableFromInput();
}

// We don't care about sideband streams, since we won't relay them.
void MediaSync::InputListener::onSidebandStreamChanged() {
    ALOGE("onSidebandStreamChanged: got sideband stream unexpectedly.");
}


void MediaSync::InputListener::binderDied(const wp<IBinder> &/* who */) {
    Mutex::Autolock lock(mSync->mMutex);
    mSync->onAbandoned_l(true /* isInput */);
}

MediaSync::OutputListener::OutputListener(const sp<MediaSync> &sync,
        const sp<IGraphicBufferProducer> &output)
      : mSync(sync),
        mOutput(output) {}

MediaSync::OutputListener::~OutputListener() {}

void MediaSync::OutputListener::onBufferReleased() {
    mSync->onBufferReleasedByOutput(mOutput);
}

void MediaSync::OutputListener::binderDied(const wp<IBinder> &/* who */) {
    Mutex::Autolock lock(mSync->mMutex);
    mSync->onAbandoned_l(false /* isInput */);
}

} // namespace android
