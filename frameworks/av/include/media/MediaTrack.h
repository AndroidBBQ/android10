/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef MEDIA_SOURCE_BASE_H_

#define MEDIA_SOURCE_BASE_H_

#include <sys/types.h>

#include <binder/IMemory.h>
#include <binder/MemoryDealer.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/MediaExtractorPluginApi.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

namespace android {

class MediaBufferBase;
struct CMediaTrack;

class SourceBaseAllocTracker {
public:
    SourceBaseAllocTracker() {
        ALOGD("sourcebase allocated: %p", this);
    }
    virtual ~SourceBaseAllocTracker() {
        ALOGD("sourcebase freed: %p", this);
    }
};

struct MediaTrack
//    : public SourceBaseAllocTracker
{
    MediaTrack();

    // To be called before any other methods on this object, except
    // getFormat().
    virtual status_t start() = 0;

    // Any blocking read call returns immediately with a result of NO_INIT.
    // It is an error to call any methods other than start after this call
    // returns. Any buffers the object may be holding onto at the time of
    // the stop() call are released.
    // Also, it is imperative that any buffers output by this object and
    // held onto by callers be released before a call to stop() !!!
    virtual status_t stop() = 0;

    // Returns the format of the data output by this media track.
    virtual status_t getFormat(MetaDataBase& format) = 0;

    // Options that modify read() behaviour. The default is to
    // a) not request a seek
    // b) not be late, i.e. lateness_us = 0
    struct ReadOptions {
        enum SeekMode : int32_t {
            SEEK_PREVIOUS_SYNC = CMediaTrackReadOptions::SEEK_PREVIOUS_SYNC,
            SEEK_NEXT_SYNC = CMediaTrackReadOptions::SEEK_NEXT_SYNC,
            SEEK_CLOSEST_SYNC = CMediaTrackReadOptions::SEEK_CLOSEST_SYNC,
            SEEK_CLOSEST = CMediaTrackReadOptions::SEEK_CLOSEST,
            SEEK_FRAME_INDEX = CMediaTrackReadOptions::SEEK_FRAME_INDEX,
        };

        ReadOptions() {
            reset();
        }

        // Reset everything back to defaults.
        void reset() {
            mOptions = 0;
            mSeekTimeUs = 0;
            mNonBlocking = false;
        }

        void setSeekTo(int64_t time_us, SeekMode mode = SEEK_CLOSEST_SYNC);
        void clearSeekTo() {
            mOptions &= ~kSeekTo_Option;
            mSeekTimeUs = 0;
            mSeekMode = SEEK_CLOSEST_SYNC;
        }
        bool getSeekTo(int64_t *time_us, SeekMode *mode) const;

        void setNonBlocking();
        void clearNonBlocking();
        bool getNonBlocking() const;

        // Used to clear all non-persistent options for multiple buffer reads.
        void clearNonPersistent() {
            clearSeekTo();
        }

    private:
        enum Options {
            kSeekTo_Option      = 1,
        };

        uint32_t mOptions;
        int64_t mSeekTimeUs;
        SeekMode mSeekMode;
        bool mNonBlocking;
    } __attribute__((packed)); // sent through Binder

    // Returns a new buffer of data. Call blocks until a
    // buffer is available, an error is encountered of the end of the stream
    // is reached.
    // End of stream is signalled by a result of ERROR_END_OF_STREAM.
    // A result of INFO_FORMAT_CHANGED indicates that the format of this
    // MediaSource has changed mid-stream, the client can continue reading
    // but should be prepared for buffers of the new configuration.
    virtual status_t read(
            MediaBufferBase **buffer, const ReadOptions *options = NULL) = 0;

    // Returns true if |read| supports nonblocking option, otherwise false.
    // |readMultiple| if supported, always allows the nonblocking option.
    virtual bool supportNonblockingRead() {
        return false;
    }

    virtual ~MediaTrack();

private:
    MediaTrack(const MediaTrack &);
    MediaTrack &operator=(const MediaTrack &);
};

class MediaTrackCUnwrapper : public MediaTrack {
public:
    static MediaTrackCUnwrapper *create(CMediaTrack *wrapper);

    virtual status_t start();
    virtual status_t stop();
    virtual status_t getFormat(MetaDataBase& format);
    virtual status_t read(MediaBufferBase **buffer, const ReadOptions *options = NULL);

    virtual bool supportNonblockingRead();

protected:
    virtual ~MediaTrackCUnwrapper();

private:
    explicit MediaTrackCUnwrapper(CMediaTrack *wrapper);
    CMediaTrack *wrapper;
    MediaBufferGroup *bufferGroup;
};

}  // namespace android

#endif  // MEDIA_SOURCE_BASE_H_
