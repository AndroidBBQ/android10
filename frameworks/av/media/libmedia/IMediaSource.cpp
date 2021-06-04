/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "BpMediaSource"
#include <utils/Log.h>

#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <media/IMediaSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/MediaSource.h>
#include <media/stagefright/MetaData.h>

namespace android {

enum {
    START = IBinder::FIRST_CALL_TRANSACTION,
    STOP,
    PAUSE,
    GETFORMAT,
    // READ, deprecated
    READMULTIPLE,
    RELEASE_BUFFER,
    SUPPORT_NONBLOCKING_READ,
};

enum {
    NULL_BUFFER,
    SHARED_BUFFER,
    INLINE_BUFFER,
    SHARED_BUFFER_INDEX,
};

class RemoteMediaBufferWrapper : public MediaBuffer {
public:
    RemoteMediaBufferWrapper(const sp<IMemory> &mem)
        : MediaBuffer(mem) {
        ALOGV("RemoteMediaBufferWrapper: creating %p", this);
    }

protected:
    virtual ~RemoteMediaBufferWrapper() {
        // Release our interest in the MediaBuffer's shared memory.
        int32_t old = addRemoteRefcount(-1);
        ALOGV("RemoteMediaBufferWrapper: releasing %p, refcount %d", this, old - 1);
        mMemory.clear(); // don't set the dead object flag.
    }
};

class BpMediaSource : public BpInterface<IMediaSource> {
public:
    explicit BpMediaSource(const sp<IBinder>& impl)
        : BpInterface<IMediaSource>(impl), mBuffersSinceStop(0)
    {
    }

    virtual status_t start(MetaData *params) {
        ALOGV("start");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        if (params) {
            params->writeToParcel(data);
        }
        status_t ret = remote()->transact(START, data, &reply);
        if (ret == NO_ERROR && params) {
            ALOGW("ignoring potentially modified MetaData from start");
            ALOGW("input:");
            params->dumpToLog();
            sp<MetaData> meta = MetaData::createFromParcel(reply);
            ALOGW("output:");
            meta->dumpToLog();
        }
        return ret;
    }

    virtual status_t stop() {
        ALOGV("stop");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        status_t status = remote()->transact(STOP, data, &reply);
        mMemoryCache.reset();
        mBuffersSinceStop = 0;
        return status;
    }

    virtual sp<MetaData> getFormat() {
        ALOGV("getFormat");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        status_t ret = remote()->transact(GETFORMAT, data, &reply);
        if (ret == NO_ERROR) {
            AutoMutex _l(mLock);
            mMetaData = MetaData::createFromParcel(reply);
            return mMetaData;
        }
        return NULL;
    }

    virtual status_t read(MediaBufferBase **buffer,
            const MediaSource::ReadOptions *options) {
        Vector<MediaBufferBase *> buffers;
        status_t ret = readMultiple(&buffers, 1 /* maxNumBuffers */, options);
        *buffer = buffers.size() == 0 ? nullptr : buffers[0];
        ALOGV("read status %d, bufferCount %u, sinceStop %u",
                ret, *buffer != nullptr, mBuffersSinceStop);
        return ret;
    }

    virtual status_t readMultiple(
            Vector<MediaBufferBase *> *buffers, uint32_t maxNumBuffers,
            const MediaSource::ReadOptions *options) {
        ALOGV("readMultiple");
        if (buffers == NULL || !buffers->isEmpty()) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        data.writeUint32(maxNumBuffers);
        if (options != nullptr) {
            data.writeByteArray(sizeof(*options), (uint8_t*) options);
        }
        status_t ret = remote()->transact(READMULTIPLE, data, &reply);
        mMemoryCache.gc();
        if (ret != NO_ERROR) {
            return ret;
        }
        // wrap the returned data in a vector of MediaBuffers
        int32_t buftype;
        uint32_t bufferCount = 0;
        while ((buftype = reply.readInt32()) != NULL_BUFFER) {
            LOG_ALWAYS_FATAL_IF(bufferCount >= maxNumBuffers,
                    "Received %u+ buffers and requested %u buffers",
                    bufferCount + 1, maxNumBuffers);
            MediaBuffer *buf;
            if (buftype == SHARED_BUFFER || buftype == SHARED_BUFFER_INDEX) {
                uint64_t index = reply.readUint64();
                ALOGV("Received %s index %llu",
                        buftype == SHARED_BUFFER ? "SHARED_BUFFER" : "SHARED_BUFFER_INDEX",
                        (unsigned long long) index);
                sp<IMemory> mem;
                if (buftype == SHARED_BUFFER) {
                    sp<IBinder> binder = reply.readStrongBinder();
                    mem = interface_cast<IMemory>(binder);
                    LOG_ALWAYS_FATAL_IF(mem.get() == nullptr,
                            "Received NULL IMemory for shared buffer");
                    mMemoryCache.insert(index, mem);
                } else {
                    mem = mMemoryCache.lookup(index);
                    LOG_ALWAYS_FATAL_IF(mem.get() == nullptr,
                            "Received invalid IMemory index for shared buffer: %llu",
                            (unsigned long long)index);
                }
                size_t offset = reply.readInt32();
                size_t length = reply.readInt32();
                buf = new RemoteMediaBufferWrapper(mem);
                buf->set_range(offset, length);
                buf->meta_data().updateFromParcel(reply);
            } else { // INLINE_BUFFER
                int32_t len = reply.readInt32();
                ALOGV("INLINE_BUFFER status %d and len %d", ret, len);
                buf = new MediaBuffer(len);
                reply.read(buf->data(), len);
                buf->meta_data().updateFromParcel(reply);
            }
            buffers->push_back(buf);
            ++bufferCount;
            ++mBuffersSinceStop;
        }
        ret = reply.readInt32();
        ALOGV("readMultiple status %d, bufferCount %u, sinceStop %u",
                ret, bufferCount, mBuffersSinceStop);
        if (bufferCount && ret == WOULD_BLOCK) {
            ret = OK;
        }
        return ret;
    }

    // Binder proxy adds readMultiple support.
    virtual bool supportReadMultiple() {
        return true;
    }

    virtual bool supportNonblockingRead() {
        ALOGV("supportNonblockingRead");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        status_t ret = remote()->transact(SUPPORT_NONBLOCKING_READ, data, &reply);
        if (ret == NO_ERROR) {
            return reply.readInt32() != 0;
        }
        return false;
    }

    virtual status_t pause() {
        ALOGV("pause");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaSource::getInterfaceDescriptor());
        return remote()->transact(PAUSE, data, &reply);
    }

private:

    uint32_t mBuffersSinceStop; // Buffer tracking variable

    // NuPlayer passes pointers-to-metadata around, so we use this to keep the metadata alive
    // XXX: could we use this for caching, or does metadata change on the fly?
    sp<MetaData> mMetaData;
    // ensure synchronize access to mMetaData
    Mutex mLock;

    // Cache all IMemory objects received from MediaExtractor.
    // We gc IMemory objects that are no longer active (referenced by a MediaBuffer).

    struct MemoryCache {
        sp<IMemory> lookup(uint64_t index) {
            auto p = mIndexToMemory.find(index);
            if (p == mIndexToMemory.end()) {
                ALOGE("cannot find index!");
                return nullptr;
            }
            return p->second;
        }

        void insert(uint64_t index, const sp<IMemory> &mem) {
            if (mIndexToMemory.find(index) != mIndexToMemory.end()) {
                ALOGE("index %llu already present", (unsigned long long)index);
                return;
            }
            (void)mIndexToMemory.emplace(index, mem);
        }

        void reset() {
            mIndexToMemory.clear();
        }

        void gc() {
            for (auto it = mIndexToMemory.begin(); it != mIndexToMemory.end(); ) {
                if (MediaBuffer::isDeadObject(it->second)) {
                    it = mIndexToMemory.erase(it);
                } else {
                    ++it;
                }
            }
        }
    private:
        // C++14 unordered_map erase on iterator is stable; C++11 has no guarantee.
        std::map<uint64_t, sp<IMemory>> mIndexToMemory;
    } mMemoryCache;
};

IMPLEMENT_META_INTERFACE(MediaSource, "android.media.IMediaSource");

#undef LOG_TAG
#define LOG_TAG "BnMediaSource"

BnMediaSource::BnMediaSource()
    : mBuffersSinceStop(0)
    , mGroup(new MediaBufferGroup(kBinderMediaBuffers /* growthLimit */)) {
}

BnMediaSource::~BnMediaSource() {
}

status_t BnMediaSource::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case START: {
            ALOGV("start");
            CHECK_INTERFACE(IMediaSource, data, reply);
            sp<MetaData> meta;
            if (data.dataAvail()) {
                meta = MetaData::createFromParcel(data);
            }
            status_t ret = start(meta.get());
            if (ret == NO_ERROR && meta != NULL) {
                meta->writeToParcel(*reply);
            }
            return ret;
        }
        case STOP: {
            ALOGV("stop");
            CHECK_INTERFACE(IMediaSource, data, reply);
            mGroup->signalBufferReturned(nullptr);
            status_t status = stop();
            mIndexCache.reset();
            mBuffersSinceStop = 0;
            return status;
        }
        case PAUSE: {
            ALOGV("pause");
            CHECK_INTERFACE(IMediaSource, data, reply);
            mGroup->signalBufferReturned(nullptr);
            return pause();
        }
        case GETFORMAT: {
            ALOGV("getFormat");
            CHECK_INTERFACE(IMediaSource, data, reply);
            sp<MetaData> meta = getFormat();
            if (meta != NULL) {
                meta->writeToParcel(*reply);
                return NO_ERROR;
            }
            return UNKNOWN_ERROR;
        }
        case READMULTIPLE: {
            ALOGV("readMultiple");
            CHECK_INTERFACE(IMediaSource, data, reply);

            // Get max number of buffers to read.
            uint32_t maxNumBuffers;
            data.readUint32(&maxNumBuffers);
            if (maxNumBuffers > kMaxNumReadMultiple) {
                maxNumBuffers = kMaxNumReadMultiple;
            }

            // Get read options, if any.
            MediaSource::ReadOptions opts;
            uint32_t len;
            const bool useOptions =
                    data.readUint32(&len) == NO_ERROR
                    && len == sizeof(opts)
                    && data.read((void *)&opts, len) == NO_ERROR;

            mGroup->signalBufferReturned(nullptr);
            mIndexCache.gc();
            size_t inlineTransferSize = 0;
            status_t ret = NO_ERROR;
            uint32_t bufferCount = 0;
            for (; bufferCount < maxNumBuffers; ++bufferCount, ++mBuffersSinceStop) {
                MediaBuffer *buf = nullptr;
                ret = read((MediaBufferBase **)&buf, useOptions ? &opts : nullptr);
                opts.clearNonPersistent(); // Remove options that only apply to first buffer.
                if (ret != NO_ERROR || buf == nullptr) {
                    break;
                }

                // Even if we're using shared memory, we might not want to use it, since for small
                // sizes it's faster to copy data through the Binder transaction
                // On the other hand, if the data size is large enough, it's better to use shared
                // memory. When data is too large, binder can't handle it.
                //
                // TODO: reduce MediaBuffer::kSharedMemThreshold
                MediaBuffer *transferBuf = nullptr;
                const size_t length = buf->range_length();
                size_t offset = buf->range_offset();
                if (length >= (supportNonblockingRead() && buf->mMemory != nullptr ?
                        kTransferSharedAsSharedThreshold : kTransferInlineAsSharedThreshold)) {
                    if (buf->mMemory != nullptr) {
                        ALOGV("Use shared memory: %zu", length);
                        transferBuf = buf;
                    } else {
                        ALOGV("Large buffer %zu without IMemory!", length);
                        ret = mGroup->acquire_buffer(
                                (MediaBufferBase **)&transferBuf, false /* nonBlocking */, length);
                        if (ret != OK
                                || transferBuf == nullptr
                                || transferBuf->mMemory == nullptr) {
                            ALOGV("Failed to acquire shared memory, size %zu, ret %d",
                                    length, ret);
                            if (transferBuf != nullptr) {
                                transferBuf->release();
                                transferBuf = nullptr;
                            }
                            // Current buffer transmit inline; no more additional buffers.
                            maxNumBuffers = 0;
                        } else {
                            memcpy(transferBuf->data(), (uint8_t*)buf->data() + offset, length);
                            offset = 0;
                            if (!mGroup->has_buffers()) {
                                maxNumBuffers = 0; // No more MediaBuffers, stop readMultiple.
                            }
                        }
                    }
                }
                if (transferBuf != nullptr) { // Using shared buffers.
                    if (!transferBuf->isObserved() && transferBuf != buf) {
                        // Transfer buffer must be part of a MediaBufferGroup.
                        ALOGV("adding shared memory buffer %p to local group", transferBuf);
                        mGroup->add_buffer(transferBuf);
                        transferBuf->add_ref(); // We have already acquired buffer.
                    }
                    uint64_t index = mIndexCache.lookup(transferBuf->mMemory);
                    if (index == 0) {
                        index = mIndexCache.insert(transferBuf->mMemory);
                        reply->writeInt32(SHARED_BUFFER);
                        reply->writeUint64(index);
                        reply->writeStrongBinder(IInterface::asBinder(transferBuf->mMemory));
                        ALOGV("SHARED_BUFFER(%p) %llu",
                                transferBuf, (unsigned long long)index);
                    } else {
                        reply->writeInt32(SHARED_BUFFER_INDEX);
                        reply->writeUint64(index);
                        ALOGV("SHARED_BUFFER_INDEX(%p) %llu",
                                transferBuf, (unsigned long long)index);
                    }
                    reply->writeInt32(offset);
                    reply->writeInt32(length);
                    buf->meta_data().writeToParcel(*reply);
                    transferBuf->addRemoteRefcount(1);
                    if (transferBuf != buf) {
                        transferBuf->release(); // release local ref
                    } else if (!supportNonblockingRead()) {
                        maxNumBuffers = 0; // stop readMultiple with one shared buffer.
                    }
                } else {
                    ALOGV_IF(buf->mMemory != nullptr,
                            "INLINE(%p) %zu shared mem available, but only %zu used",
                            buf, buf->mMemory->size(), length);
                    reply->writeInt32(INLINE_BUFFER);
                    reply->writeByteArray(length, (uint8_t*)buf->data() + offset);
                    buf->meta_data().writeToParcel(*reply);
                    inlineTransferSize += length;
                    if (inlineTransferSize > kInlineMaxTransfer) {
                        maxNumBuffers = 0; // stop readMultiple if inline transfer is too large.
                    }
                }
                buf->release();
            }
            reply->writeInt32(NULL_BUFFER); // Indicate no more MediaBuffers.
            reply->writeInt32(ret);
            ALOGV("readMultiple status %d, bufferCount %u, sinceStop %u",
                    ret, bufferCount, mBuffersSinceStop);
            return NO_ERROR;
        }
        case SUPPORT_NONBLOCKING_READ: {
            ALOGV("supportNonblockingRead");
            CHECK_INTERFACE(IMediaSource, data, reply);
            reply->writeInt32((int32_t)supportNonblockingRead());
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}  // namespace android

