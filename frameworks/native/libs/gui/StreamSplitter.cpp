/*
 * Copyright 2014 The Android Open Source Project
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

#include <inttypes.h>

#define LOG_TAG "StreamSplitter"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include <gui/BufferItem.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/StreamSplitter.h>

#include <ui/GraphicBuffer.h>

#include <binder/ProcessState.h>

#include <utils/Trace.h>

#include <system/window.h>

namespace android {

status_t StreamSplitter::createSplitter(
        const sp<IGraphicBufferConsumer>& inputQueue,
        sp<StreamSplitter>* outSplitter) {
    if (inputQueue == nullptr) {
        ALOGE("createSplitter: inputQueue must not be NULL");
        return BAD_VALUE;
    }
    if (outSplitter == nullptr) {
        ALOGE("createSplitter: outSplitter must not be NULL");
        return BAD_VALUE;
    }

    sp<StreamSplitter> splitter(new StreamSplitter(inputQueue));
    status_t status = splitter->mInput->consumerConnect(splitter, false);
    if (status == NO_ERROR) {
        splitter->mInput->setConsumerName(String8("StreamSplitter"));
        *outSplitter = splitter;
    }
    return status;
}

StreamSplitter::StreamSplitter(const sp<IGraphicBufferConsumer>& inputQueue)
      : mIsAbandoned(false), mMutex(), mReleaseCondition(),
        mOutstandingBuffers(0), mInput(inputQueue), mOutputs(), mBuffers() {}

StreamSplitter::~StreamSplitter() {
    mInput->consumerDisconnect();
    Vector<sp<IGraphicBufferProducer> >::iterator output = mOutputs.begin();
    for (; output != mOutputs.end(); ++output) {
        (*output)->disconnect(NATIVE_WINDOW_API_CPU);
    }

    if (mBuffers.size() > 0) {
        ALOGE("%zu buffers still being tracked", mBuffers.size());
    }
}

status_t StreamSplitter::addOutput(
        const sp<IGraphicBufferProducer>& outputQueue) {
    if (outputQueue == nullptr) {
        ALOGE("addOutput: outputQueue must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mMutex);

    IGraphicBufferProducer::QueueBufferOutput queueBufferOutput;
    sp<OutputListener> listener(new OutputListener(this, outputQueue));
    IInterface::asBinder(outputQueue)->linkToDeath(listener);
    status_t status = outputQueue->connect(listener, NATIVE_WINDOW_API_CPU,
            /* producerControlledByApp */ false, &queueBufferOutput);
    if (status != NO_ERROR) {
        ALOGE("addOutput: failed to connect (%d)", status);
        return status;
    }

    mOutputs.push_back(outputQueue);

    return NO_ERROR;
}

void StreamSplitter::setName(const String8 &name) {
    Mutex::Autolock lock(mMutex);
    mInput->setConsumerName(name);
}

void StreamSplitter::onFrameAvailable(const BufferItem& /* item */) {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);

    // The current policy is that if any one consumer is consuming buffers too
    // slowly, the splitter will stall the rest of the outputs by not acquiring
    // any more buffers from the input. This will cause back pressure on the
    // input queue, slowing down its producer.

    // If there are too many outstanding buffers, we block until a buffer is
    // released back to the input in onBufferReleased
    while (mOutstandingBuffers >= MAX_OUTSTANDING_BUFFERS) {
        mReleaseCondition.wait(mMutex);

        // If the splitter is abandoned while we are waiting, the release
        // condition variable will be broadcast, and we should just return
        // without attempting to do anything more (since the input queue will
        // also be abandoned).
        if (mIsAbandoned) {
            return;
        }
    }
    ++mOutstandingBuffers;

    // Acquire and detach the buffer from the input
    BufferItem bufferItem;
    status_t status = mInput->acquireBuffer(&bufferItem, /* presentWhen */ 0);
    LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
            "acquiring buffer from input failed (%d)", status);

    ALOGV("acquired buffer %#" PRIx64 " from input",
            bufferItem.mGraphicBuffer->getId());

    status = mInput->detachBuffer(bufferItem.mSlot);
    LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
            "detaching buffer from input failed (%d)", status);

    // Initialize our reference count for this buffer
    mBuffers.add(bufferItem.mGraphicBuffer->getId(),
            new BufferTracker(bufferItem.mGraphicBuffer));

    IGraphicBufferProducer::QueueBufferInput queueInput(
            bufferItem.mTimestamp, bufferItem.mIsAutoTimestamp,
            bufferItem.mDataSpace, bufferItem.mCrop,
            static_cast<int32_t>(bufferItem.mScalingMode),
            bufferItem.mTransform, bufferItem.mFence);

    // Attach and queue the buffer to each of the outputs
    Vector<sp<IGraphicBufferProducer> >::iterator output = mOutputs.begin();
    for (; output != mOutputs.end(); ++output) {
        int slot;
        status = (*output)->attachBuffer(&slot, bufferItem.mGraphicBuffer);
        if (status == NO_INIT) {
            // If we just discovered that this output has been abandoned, note
            // that, increment the release count so that we still release this
            // buffer eventually, and move on to the next output
            onAbandonedLocked();
            mBuffers.editValueFor(bufferItem.mGraphicBuffer->getId())->
                    incrementReleaseCountLocked();
            continue;
        } else {
            LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
                    "attaching buffer to output failed (%d)", status);
        }

        IGraphicBufferProducer::QueueBufferOutput queueOutput;
        status = (*output)->queueBuffer(slot, queueInput, &queueOutput);
        if (status == NO_INIT) {
            // If we just discovered that this output has been abandoned, note
            // that, increment the release count so that we still release this
            // buffer eventually, and move on to the next output
            onAbandonedLocked();
            mBuffers.editValueFor(bufferItem.mGraphicBuffer->getId())->
                    incrementReleaseCountLocked();
            continue;
        } else {
            LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
                    "queueing buffer to output failed (%d)", status);
        }

        ALOGV("queued buffer %#" PRIx64 " to output %p",
                bufferItem.mGraphicBuffer->getId(), output->get());
    }
}

void StreamSplitter::onBufferReleasedByOutput(
        const sp<IGraphicBufferProducer>& from) {
    ATRACE_CALL();
    Mutex::Autolock lock(mMutex);

    sp<GraphicBuffer> buffer;
    sp<Fence> fence;
    status_t status = from->detachNextBuffer(&buffer, &fence);
    if (status == NO_INIT) {
        // If we just discovered that this output has been abandoned, note that,
        // but we can't do anything else, since buffer is invalid
        onAbandonedLocked();
        return;
    } else {
        LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
                "detaching buffer from output failed (%d)", status);
    }

    ALOGV("detached buffer %#" PRIx64 " from output %p",
          buffer->getId(), from.get());

    const sp<BufferTracker>& tracker = mBuffers.editValueFor(buffer->getId());

    // Merge the release fence of the incoming buffer so that the fence we send
    // back to the input includes all of the outputs' fences
    tracker->mergeFence(fence);

    // Check to see if this is the last outstanding reference to this buffer
    size_t releaseCount = tracker->incrementReleaseCountLocked();
    ALOGV("buffer %#" PRIx64 " reference count %zu (of %zu)", buffer->getId(),
            releaseCount, mOutputs.size());
    if (releaseCount < mOutputs.size()) {
        return;
    }

    // If we've been abandoned, we can't return the buffer to the input, so just
    // stop tracking it and move on
    if (mIsAbandoned) {
        mBuffers.removeItem(buffer->getId());
        return;
    }

    // Attach and release the buffer back to the input
    int consumerSlot;
    status = mInput->attachBuffer(&consumerSlot, tracker->getBuffer());
    LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
            "attaching buffer to input failed (%d)", status);

    status = mInput->releaseBuffer(consumerSlot, /* frameNumber */ 0,
            EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, tracker->getMergedFence());
    LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
            "releasing buffer to input failed (%d)", status);

    ALOGV("released buffer %#" PRIx64 " to input", buffer->getId());

    // We no longer need to track the buffer once it has been returned to the
    // input
    mBuffers.removeItem(buffer->getId());

    // Notify any waiting onFrameAvailable calls
    --mOutstandingBuffers;
    mReleaseCondition.signal();
}

void StreamSplitter::onAbandonedLocked() {
    ALOGE("one of my outputs has abandoned me");
    if (!mIsAbandoned) {
        mInput->consumerDisconnect();
    }
    mIsAbandoned = true;
    mReleaseCondition.broadcast();
}

StreamSplitter::OutputListener::OutputListener(
        const sp<StreamSplitter>& splitter,
        const sp<IGraphicBufferProducer>& output)
      : mSplitter(splitter), mOutput(output) {}

StreamSplitter::OutputListener::~OutputListener() {}

void StreamSplitter::OutputListener::onBufferReleased() {
    mSplitter->onBufferReleasedByOutput(mOutput);
}

void StreamSplitter::OutputListener::binderDied(const wp<IBinder>& /* who */) {
    Mutex::Autolock lock(mSplitter->mMutex);
    mSplitter->onAbandonedLocked();
}

StreamSplitter::BufferTracker::BufferTracker(const sp<GraphicBuffer>& buffer)
      : mBuffer(buffer), mMergedFence(Fence::NO_FENCE), mReleaseCount(0) {}

StreamSplitter::BufferTracker::~BufferTracker() {}

void StreamSplitter::BufferTracker::mergeFence(const sp<Fence>& with) {
    mMergedFence = Fence::merge(String8("StreamSplitter"), mMergedFence, with);
}

} // namespace android
