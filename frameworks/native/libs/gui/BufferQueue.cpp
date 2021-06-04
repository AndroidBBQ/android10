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

#define LOG_TAG "BufferQueue"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#ifndef NO_BUFFERHUB
#include <gui/BufferHubConsumer.h>
#include <gui/BufferHubProducer.h>
#endif

#include <gui/BufferQueue.h>
#include <gui/BufferQueueConsumer.h>
#include <gui/BufferQueueCore.h>
#include <gui/BufferQueueProducer.h>

namespace android {

BufferQueue::ProxyConsumerListener::ProxyConsumerListener(
        const wp<ConsumerListener>& consumerListener):
        mConsumerListener(consumerListener) {}

BufferQueue::ProxyConsumerListener::~ProxyConsumerListener() {}

void BufferQueue::ProxyConsumerListener::onDisconnect() {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->onDisconnect();
    }
}

void BufferQueue::ProxyConsumerListener::onFrameAvailable(
        const BufferItem& item) {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->onFrameAvailable(item);
    }
}

void BufferQueue::ProxyConsumerListener::onFrameReplaced(
        const BufferItem& item) {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->onFrameReplaced(item);
    }
}

void BufferQueue::ProxyConsumerListener::onBuffersReleased() {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->onBuffersReleased();
    }
}

void BufferQueue::ProxyConsumerListener::onSidebandStreamChanged() {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->onSidebandStreamChanged();
    }
}

void BufferQueue::ProxyConsumerListener::addAndGetFrameTimestamps(
        const NewFrameEventsEntry* newTimestamps,
        FrameEventHistoryDelta* outDelta) {
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != nullptr) {
        listener->addAndGetFrameTimestamps(newTimestamps, outDelta);
    }
}

void BufferQueue::createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
        sp<IGraphicBufferConsumer>* outConsumer,
        bool consumerIsSurfaceFlinger) {
    LOG_ALWAYS_FATAL_IF(outProducer == nullptr,
            "BufferQueue: outProducer must not be NULL");
    LOG_ALWAYS_FATAL_IF(outConsumer == nullptr,
            "BufferQueue: outConsumer must not be NULL");

    sp<BufferQueueCore> core(new BufferQueueCore());
    LOG_ALWAYS_FATAL_IF(core == nullptr,
            "BufferQueue: failed to create BufferQueueCore");

    sp<IGraphicBufferProducer> producer(new BufferQueueProducer(core, consumerIsSurfaceFlinger));
    LOG_ALWAYS_FATAL_IF(producer == nullptr,
            "BufferQueue: failed to create BufferQueueProducer");

    sp<IGraphicBufferConsumer> consumer(new BufferQueueConsumer(core));
    LOG_ALWAYS_FATAL_IF(consumer == nullptr,
            "BufferQueue: failed to create BufferQueueConsumer");

    *outProducer = producer;
    *outConsumer = consumer;
}

#ifndef NO_BUFFERHUB
void BufferQueue::createBufferHubQueue(sp<IGraphicBufferProducer>* outProducer,
                                       sp<IGraphicBufferConsumer>* outConsumer) {
    LOG_ALWAYS_FATAL_IF(outProducer == nullptr, "BufferQueue: outProducer must not be NULL");
    LOG_ALWAYS_FATAL_IF(outConsumer == nullptr, "BufferQueue: outConsumer must not be NULL");

    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;

    dvr::ProducerQueueConfigBuilder configBuilder;
    std::shared_ptr<dvr::ProducerQueue> producerQueue =
            dvr::ProducerQueue::Create(configBuilder.Build(), dvr::UsagePolicy{});
    LOG_ALWAYS_FATAL_IF(producerQueue == nullptr, "BufferQueue: failed to create ProducerQueue.");

    std::shared_ptr<dvr::ConsumerQueue> consumerQueue = producerQueue->CreateConsumerQueue();
    LOG_ALWAYS_FATAL_IF(consumerQueue == nullptr, "BufferQueue: failed to create ConsumerQueue.");

    producer = BufferHubProducer::Create(producerQueue);
    consumer = BufferHubConsumer::Create(consumerQueue);

    LOG_ALWAYS_FATAL_IF(producer == nullptr, "BufferQueue: failed to create BufferQueueProducer");
    LOG_ALWAYS_FATAL_IF(consumer == nullptr, "BufferQueue: failed to create BufferQueueConsumer");

    *outProducer = producer;
    *outConsumer = consumer;
}
#endif

}; // namespace android
