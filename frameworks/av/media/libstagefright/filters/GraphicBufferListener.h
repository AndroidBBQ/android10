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

#ifndef GRAPHIC_BUFFER_LISTENER_H_
#define GRAPHIC_BUFFER_LISTENER_H_

#include <gui/BufferQueue.h>

namespace android {

struct AMessage;

struct GraphicBufferListener : public BufferQueue::ConsumerListener {
public:
    GraphicBufferListener() {};

    status_t init(
            const sp<AMessage> &notify,
            size_t bufferWidth, size_t bufferHeight, size_t bufferCount);

    virtual void onFrameAvailable(const BufferItem& item);
    virtual void onBuffersReleased();
    virtual void onSidebandStreamChanged();

    // Returns the handle to the producer side of the BufferQueue.  Buffers
    // queued on this will be received by GraphicBufferListener.
    sp<IGraphicBufferProducer> getIGraphicBufferProducer() const {
        return mProducer;
    }

    BufferItem getBufferItem();
    sp<GraphicBuffer> getBuffer(BufferItem item);
    status_t releaseBuffer(BufferItem item);

    enum {
        kWhatFrameAvailable = 'frav',
    };

private:
    sp<AMessage> mNotify;
    size_t mNumFramesAvailable;

    mutable Mutex mMutex;

    // Our BufferQueue interfaces. mProducer is passed to the producer through
    // getIGraphicBufferProducer, and mConsumer is used internally to retrieve
    // the buffers queued by the producer.
    sp<IGraphicBufferProducer> mProducer;
    sp<IGraphicBufferConsumer> mConsumer;

    // Cache of GraphicBuffers from the buffer queue.
    sp<GraphicBuffer> mBufferSlot[BufferQueue::NUM_BUFFER_SLOTS];
};

}   // namespace android

#endif  // GRAPHIC_BUFFER_LISTENER_H
