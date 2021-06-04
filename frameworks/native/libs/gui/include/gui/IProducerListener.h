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

#ifndef ANDROID_GUI_IPRODUCERLISTENER_H
#define ANDROID_GUI_IPRODUCERLISTENER_H

#include <vector>

#include <android/hardware/graphics/bufferqueue/1.0/IProducerListener.h>
#include <android/hardware/graphics/bufferqueue/2.0/IProducerListener.h>
#include <binder/IInterface.h>
#include <hidl/HybridInterface.h>
#include <utils/RefBase.h>

namespace android {

// ProducerListener is the interface through which the BufferQueue notifies the
// producer of events that the producer may wish to react to. Because the
// producer will generally have a mutex that is locked during calls from the
// producer to the BufferQueue, these calls from the BufferQueue to the
// producer *MUST* be called only when the BufferQueue mutex is NOT locked.

class ProducerListener : public virtual RefBase
{
public:
    ProducerListener() {}
    virtual ~ProducerListener();

    // onBufferReleased is called from IGraphicBufferConsumer::releaseBuffer to
    // notify the producer that a new buffer is free and ready to be dequeued.
    //
    // This is called without any lock held and can be called concurrently by
    // multiple threads.
    virtual void onBufferReleased() = 0; // Asynchronous
    virtual bool needsReleaseNotify() = 0;
    // onBuffersFreed is called from IGraphicBufferConsumer::discardFreeBuffers
    // to notify the producer that certain free buffers are discarded by the consumer.
    virtual void onBuffersDiscarded(const std::vector<int32_t>& slots) = 0; // Asynchronous
};

class IProducerListener : public ProducerListener, public IInterface
{
public:
    using HProducerListener1 =
            ::android::hardware::graphics::bufferqueue::V1_0::IProducerListener;
    using HProducerListener2 =
            ::android::hardware::graphics::bufferqueue::V2_0::IProducerListener;
    DECLARE_HYBRID_META_INTERFACE(
            ProducerListener,
            HProducerListener1,
            HProducerListener2)
};

class BnProducerListener : public BnInterface<IProducerListener>
{
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
    virtual bool needsReleaseNotify();
    virtual void onBuffersDiscarded(const std::vector<int32_t>& slots);
};

class DummyProducerListener : public BnProducerListener
{
public:
    virtual ~DummyProducerListener();
    virtual void onBufferReleased() {}
    virtual bool needsReleaseNotify() { return false; }
};

} // namespace android

#endif
