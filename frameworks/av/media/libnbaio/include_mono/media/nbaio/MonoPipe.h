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

#ifndef ANDROID_AUDIO_MONO_PIPE_H
#define ANDROID_AUDIO_MONO_PIPE_H

#include <time.h>
#include <audio_utils/fifo.h>
#include <media/SingleStateQueue.h>
#include <media/nbaio/NBAIO.h>

namespace android {

typedef SingleStateQueue<ExtendedTimestamp> ExtendedTimestampSingleStateQueue;

// MonoPipe is similar to Pipe except:
//  - supports only a single reader, called MonoPipeReader
//  - write() cannot overrun; instead it will return a short actual count if insufficient space
//  - write() can optionally block if the pipe is full
// Like Pipe, it is not multi-thread safe for either writer or reader
// but writer and reader can be different threads.
class MonoPipe : public NBAIO_Sink {

    friend class MonoPipeReader;

public:
    // reqFrames will be rounded up to a power of 2, and all slots are available. Must be >= 2.
    // Note: whatever shares this object with another thread needs to do so in an SMP-safe way (like
    // creating it the object before creating the other thread, or storing the object with a
    // release_store). Otherwise the other thread could see a partially-constructed object.
    MonoPipe(size_t reqFrames, const NBAIO_Format& format, bool writeCanBlock = false);
    virtual ~MonoPipe();

    // NBAIO_Port interface

    //virtual ssize_t negotiate(const NBAIO_Format offers[], size_t numOffers,
    //                          NBAIO_Format counterOffers[], size_t& numCounterOffers);
    //virtual NBAIO_Format format() const;

    // NBAIO_Sink interface

    //virtual int64_t framesWritten() const;
    //virtual int64_t framesUnderrun() const;
    //virtual int64_t underruns() const;

    // returns n where 0 <= n <= mMaxFrames, or a negative status_t
    // including the private status codes in NBAIO.h
    virtual ssize_t availableToWrite();

    virtual ssize_t write(const void *buffer, size_t count);
    //virtual ssize_t writeVia(writeVia_t via, size_t total, void *user, size_t block);

            // average number of frames present in the pipe under normal conditions.
            // See throttling mechanism in MonoPipe::write()
            size_t  getAvgFrames() const { return mSetpoint; }
            void    setAvgFrames(size_t setpoint);
            size_t  maxFrames() const { return mMaxFrames; }

            // Set the shutdown state for the write side of a pipe.
            // This may be called by an unrelated thread.  When shutdown state is 'true',
            // a write that would otherwise block instead returns a short transfer count.
            // There is no guarantee how long it will take for the shutdown to be recognized,
            // but it will not be an unbounded amount of time.
            // The state can be restored to normal by calling shutdown(false).
            void    shutdown(bool newState = true);

            // Return true if the write side of a pipe is currently shutdown.
            bool    isShutdown();

            // Return NO_ERROR if there is a timestamp available
            status_t getTimestamp(ExtendedTimestamp &timestamp);

private:
    const size_t    mMaxFrames;     // as requested in constructor, rounded up to a power of 2
    void * const    mBuffer;
    audio_utils_fifo        mFifo;
    audio_utils_fifo_writer mFifoWriter;
    bool            mWriteTsValid;  // whether mWriteTs is valid
    struct timespec mWriteTs;       // time that the previous write() completed
    size_t          mSetpoint;      // target value for pipe fill depth
    const bool      mWriteCanBlock; // whether write() should block if the pipe is full

    bool            mIsShutdown;    // whether shutdown(true) was called, no barriers are needed

    ExtendedTimestampSingleStateQueue::Shared      mTimestampShared;
    ExtendedTimestampSingleStateQueue::Mutator     mTimestampMutator;
    ExtendedTimestampSingleStateQueue::Observer    mTimestampObserver;
};

}   // namespace android

#endif  // ANDROID_AUDIO_MONO_PIPE_H
