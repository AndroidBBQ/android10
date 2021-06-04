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

#ifndef IMEDIA_SOURCE_BASE_H_

#define IMEDIA_SOURCE_BASE_H_

#include <map>

#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <media/MediaSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

class MediaBufferGroup;

class IMediaSource : public IInterface {
public:
    DECLARE_META_INTERFACE(MediaSource);

    enum {
        // Maximum number of buffers would be read in readMultiple.
        kMaxNumReadMultiple = 128,
    };

    // To be called before any other methods on this object, except
    // getFormat().
    virtual status_t start(MetaData *params = NULL) = 0;

    // Any blocking read call returns immediately with a result of NO_INIT.
    // It is an error to call any methods other than start after this call
    // returns. Any buffers the object may be holding onto at the time of
    // the stop() call are released.
    // Also, it is imperative that any buffers output by this object and
    // held onto by callers be released before a call to stop() !!!
    virtual status_t stop() = 0;

    // Returns the format of the data output by this media source.
    virtual sp<MetaData> getFormat() = 0;

    // Returns a new buffer of data. Call blocks until a
    // buffer is available, an error is encountered or the end of the stream
    // is reached.
    // End of stream is signalled by a result of ERROR_END_OF_STREAM.
    // A result of INFO_FORMAT_CHANGED indicates that the format of this
    // MediaSource has changed mid-stream, the client can continue reading
    // but should be prepared for buffers of the new configuration.
    //
    // TODO: consider removing read() in favor of readMultiple().
    virtual status_t read(
            MediaBufferBase **buffer,
            const MediaSource::ReadOptions *options = NULL) = 0;

    // Returns a vector of new buffers of data, where the new buffers are added
    // to the end of the vector.
    // Call blocks until an error is encountered, or the end of the stream is
    // reached, or format change is hit, or |kMaxNumReadMultiple| buffers have
    // been read.
    // End of stream is signaled by a result of ERROR_END_OF_STREAM.
    // A result of INFO_FORMAT_CHANGED indicates that the format of this
    // MediaSource has changed mid-stream, the client can continue reading
    // but should be prepared for buffers of the new configuration.
    //
    // ReadOptions may be specified. Persistent options apply to all reads;
    // non-persistent options (e.g. seek) apply only to the first read.
    virtual status_t readMultiple(
            Vector<MediaBufferBase *> *buffers, uint32_t maxNumBuffers = 1,
            const MediaSource::ReadOptions *options = nullptr) = 0;

    // Returns true if |readMultiple| is supported, otherwise false.
    virtual bool supportReadMultiple() = 0;

    // Returns true if |read| supports nonblocking option, otherwise false.
    // |readMultiple| if supported, always allows the nonblocking option.
    virtual bool supportNonblockingRead() = 0;

    // Causes this source to suspend pulling data from its upstream source
    // until a subsequent read-with-seek. Currently only supported by
    // OMXCodec.
    virtual status_t pause()  = 0;
};

class BnMediaSource: public BnInterface<IMediaSource>
{
public:
    BnMediaSource();

    virtual status_t    onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);

    virtual status_t pause() {
        return ERROR_UNSUPPORTED;
    }

    // TODO: Implement this for local media sources.
    virtual status_t readMultiple(
            Vector<MediaBufferBase *> * /* buffers */, uint32_t /* maxNumBuffers = 1 */,
            const MediaSource::ReadOptions * /* options = nullptr */) {
        return ERROR_UNSUPPORTED;
    }

    virtual bool supportReadMultiple() {
        return false;
    }

    // Override in source if nonblocking reads are supported.
    virtual bool supportNonblockingRead() {
        return false;
    }

    // align buffer count with video request size in NuMediaExtractor::selectTrack()
    static const size_t kBinderMediaBuffers = 8; // buffers managed by BnMediaSource
    static const size_t kTransferSharedAsSharedThreshold = 4 * 1024;  // if >= shared, else inline
    static const size_t kTransferInlineAsSharedThreshold = 8 * 1024; // if >= shared, else inline
    static const size_t kInlineMaxTransfer = 64 * 1024; // Binder size limited to BINDER_VM_SIZE.

protected:
    virtual ~BnMediaSource();

private:
    uint32_t mBuffersSinceStop; // Buffer tracking variable

    std::unique_ptr<MediaBufferGroup> mGroup;

    // To prevent marshalling IMemory with each read transaction, we cache the IMemory pointer
    // into a map.
    //
    // This is converted into an index, which is used to identify the associated memory
    // on the receiving side.  We hold a reference to the IMemory here to ensure it doesn't
    // change underneath us.

    struct IndexCache {
        IndexCache() : mIndex(0) { }

        // Returns the index of the IMemory stored in cache or 0 if not found.
        uint64_t lookup(const sp<IMemory> &mem) {
            auto p = mMemoryToIndex.find(mem.get());
            if (p == mMemoryToIndex.end()) {
                return 0;
            }
            if (MediaBuffer::isDeadObject(p->second.first)) {
                // this object's dead
                ALOGW("Attempting to lookup a dead IMemory");
                (void)mMemoryToIndex.erase(p);
                return 0;
            }
            ALOGW_IF(p->second.first.get() != mem.get(), "Mismatched buffers without reset");
            return p->second.second;
        }

        // Returns the index of the IMemory stored in the index cache.
        uint64_t insert(const sp<IMemory> &mem) {
            auto p = mMemoryToIndex.find(mem.get());
            if (p == mMemoryToIndex.end()) {
                if (mIndex == UINT64_MAX) {
                    ALOGE("Index overflow");
                    mIndex = 1; // skip overflow condition and hope for the best
                } else {
                    ++mIndex;
                }
                (void)mMemoryToIndex.emplace(// C++11 mem.get(), std::make_pair(mem, mIndex))
                        std::piecewise_construct,
                        std::forward_as_tuple(mem.get()), std::forward_as_tuple(mem, mIndex));
                return mIndex;
            }
            ALOGW("IMemory already inserted into cache");
            return p->second.second;
        }

        void reset() {
            mMemoryToIndex.clear();
            mIndex = 0;
        }

        void gc() {
            for (auto it = mMemoryToIndex.begin(); it != mMemoryToIndex.end(); ) {
                if (MediaBuffer::isDeadObject(it->second.first)) {
                    it = mMemoryToIndex.erase(it);
                } else {
                    ++it;
                }
            }
        }

    private:
        uint64_t mIndex;
        // C++14 unordered_map erase on iterator is stable; C++11 has no guarantee.
        // Could key on uintptr_t instead of IMemory *
        std::map<IMemory *, std::pair<sp<IMemory>, uint64_t>> mMemoryToIndex;
    } mIndexCache;
};

}  // namespace android

#endif  // IMEDIA_SOURCE_BASE_H_
