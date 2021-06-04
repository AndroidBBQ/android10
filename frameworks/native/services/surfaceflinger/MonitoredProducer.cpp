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

#include "MonitoredProducer.h"
#include "Layer.h"
#include "SurfaceFlinger.h"

#include "Scheduler/MessageQueue.h"

namespace android {

MonitoredProducer::MonitoredProducer(const sp<IGraphicBufferProducer>& producer,
        const sp<SurfaceFlinger>& flinger,
        const wp<Layer>& layer) :
    mProducer(producer),
    mFlinger(flinger),
    mLayer(layer) {}

MonitoredProducer::~MonitoredProducer() {
    // Remove ourselves from SurfaceFlinger's list. We do this asynchronously
    // because we don't know where this destructor is called from. It could be
    // called with the mStateLock held, leading to a dead-lock (it actually
    // happens).
    sp<LambdaMessage> cleanUpListMessage =
            new LambdaMessage([flinger = mFlinger, asBinder = wp<IBinder>(onAsBinder())]() {
                Mutex::Autolock lock(flinger->mStateLock);
                flinger->mGraphicBufferProducerList.erase(asBinder);
            });

    mFlinger->postMessageAsync(cleanUpListMessage);
}

status_t MonitoredProducer::requestBuffer(int slot, sp<GraphicBuffer>* buf) {
    return mProducer->requestBuffer(slot, buf);
}

status_t MonitoredProducer::setMaxDequeuedBufferCount(
        int maxDequeuedBuffers) {
    return mProducer->setMaxDequeuedBufferCount(maxDequeuedBuffers);
}

status_t MonitoredProducer::setAsyncMode(bool async) {
    return mProducer->setAsyncMode(async);
}

status_t MonitoredProducer::dequeueBuffer(int* slot, sp<Fence>* fence, uint32_t w, uint32_t h,
                                          PixelFormat format, uint64_t usage,
                                          uint64_t* outBufferAge,
                                          FrameEventHistoryDelta* outTimestamps) {
    return mProducer->dequeueBuffer(slot, fence, w, h, format, usage, outBufferAge, outTimestamps);
}

status_t MonitoredProducer::detachBuffer(int slot) {
    return mProducer->detachBuffer(slot);
}

status_t MonitoredProducer::detachNextBuffer(sp<GraphicBuffer>* outBuffer,
        sp<Fence>* outFence) {
    return mProducer->detachNextBuffer(outBuffer, outFence);
}

status_t MonitoredProducer::attachBuffer(int* outSlot,
        const sp<GraphicBuffer>& buffer) {
    return mProducer->attachBuffer(outSlot, buffer);
}

status_t MonitoredProducer::queueBuffer(int slot, const QueueBufferInput& input,
        QueueBufferOutput* output) {
    return mProducer->queueBuffer(slot, input, output);
}

status_t MonitoredProducer::cancelBuffer(int slot, const sp<Fence>& fence) {
    return mProducer->cancelBuffer(slot, fence);
}

int MonitoredProducer::query(int what, int* value) {
    return mProducer->query(what, value);
}

status_t MonitoredProducer::connect(const sp<IProducerListener>& listener,
        int api, bool producerControlledByApp, QueueBufferOutput* output) {
    return mProducer->connect(listener, api, producerControlledByApp, output);
}

status_t MonitoredProducer::disconnect(int api, DisconnectMode mode) {
    return mProducer->disconnect(api, mode);
}

status_t MonitoredProducer::setSidebandStream(const sp<NativeHandle>& stream) {
    return mProducer->setSidebandStream(stream);
}

void MonitoredProducer::allocateBuffers(uint32_t width, uint32_t height,
        PixelFormat format, uint64_t usage) {
    mProducer->allocateBuffers(width, height, format, usage);
}

status_t MonitoredProducer::allowAllocation(bool allow) {
    return mProducer->allowAllocation(allow);
}

status_t MonitoredProducer::setGenerationNumber(uint32_t generationNumber) {
    return mProducer->setGenerationNumber(generationNumber);
}

String8 MonitoredProducer::getConsumerName() const {
    return mProducer->getConsumerName();
}

status_t MonitoredProducer::setSharedBufferMode(bool sharedBufferMode) {
    return mProducer->setSharedBufferMode(sharedBufferMode);
}

status_t MonitoredProducer::setAutoRefresh(bool autoRefresh) {
    return mProducer->setAutoRefresh(autoRefresh);
}

status_t MonitoredProducer::setDequeueTimeout(nsecs_t timeout) {
    return mProducer->setDequeueTimeout(timeout);
}

status_t MonitoredProducer::setLegacyBufferDrop(bool drop) {
    return mProducer->setLegacyBufferDrop(drop);
}

status_t MonitoredProducer::getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer,
        sp<Fence>* outFence, float outTransformMatrix[16]) {
    return mProducer->getLastQueuedBuffer(outBuffer, outFence,
            outTransformMatrix);
}

void MonitoredProducer::getFrameTimestamps(FrameEventHistoryDelta* outDelta) {
    mProducer->getFrameTimestamps(outDelta);
}

status_t MonitoredProducer::getUniqueId(uint64_t* outId) const {
    return mProducer->getUniqueId(outId);
}

status_t MonitoredProducer::getConsumerUsage(uint64_t* outUsage) const {
    return mProducer->getConsumerUsage(outUsage);
}

IBinder* MonitoredProducer::onAsBinder() {
    return this;
}

sp<Layer> MonitoredProducer::getLayer() const {
    return mLayer.promote();
}

// ---------------------------------------------------------------------------
}; // namespace android
