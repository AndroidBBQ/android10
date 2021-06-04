/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_TRACK_SHARED_H
#define ANDROID_AUDIO_TRACK_SHARED_H

#include <stdint.h>
#include <sys/types.h>

#include <audio_utils/minifloat.h>
#include <utils/threads.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <audio_utils/roundup.h>
#include <media/AudioResamplerPublic.h>
#include <media/AudioTimestamp.h>
#include <media/Modulo.h>
#include <media/SingleStateQueue.h>

namespace android {

// ----------------------------------------------------------------------------

// for audio_track_cblk_t::mFlags
#define CBLK_UNDERRUN   0x01 // set by server immediately on output underrun, cleared by client
#define CBLK_FORCEREADY 0x02 // set: track is considered ready immediately by AudioFlinger,
                             // clear: track is ready when buffer full
#define CBLK_INVALID    0x04 // track buffer invalidated by AudioFlinger, need to re-create
#define CBLK_DISABLED   0x08 // output track disabled by AudioFlinger due to underrun,
                             // need to re-start.  Unlike CBLK_UNDERRUN, this is not set
                             // immediately, but only after a long string of underruns.
// 0x10 unused
#define CBLK_LOOP_CYCLE 0x20 // set by server each time a loop cycle other than final one completes
#define CBLK_LOOP_FINAL 0x40 // set by server when the final loop cycle completes
#define CBLK_BUFFER_END 0x80 // set by server when the position reaches end of buffer if not looping
#define CBLK_OVERRUN   0x100 // set by server immediately on input overrun, cleared by client
#define CBLK_INTERRUPT 0x200 // set by client on interrupt(), cleared by client in obtainBuffer()
#define CBLK_STREAM_END_DONE 0x400 // set by server on render completion, cleared by client

//EL_FIXME 20 seconds may not be enough and must be reconciled with new obtainBuffer implementation
#define MAX_RUN_OFFLOADED_TIMEOUT_MS 20000 // assuming up to a maximum of 20 seconds of offloaded

struct AudioTrackSharedStreaming {
    // similar to NBAIO MonoPipe
    // in continuously incrementing frame units, take modulo buffer size, which must be a power of 2
    volatile int32_t mFront;    // read by consumer (output: server, input: client)
    volatile int32_t mRear;     // written by producer (output: client, input: server)
    volatile int32_t mFlush;    // incremented by client to indicate a request to flush;
                                // server notices and discards all data between mFront and mRear
    volatile int32_t mStop;     // set by client to indicate a stop frame position; server
                                // will not read beyond this position until start is called.
    volatile uint32_t mUnderrunFrames; // server increments for each unavailable but desired frame
    volatile uint32_t mUnderrunCount;  // server increments for each underrun occurrence
};

// Represents a single state of an AudioTrack that was created in static mode (shared memory buffer
// supplied by the client).  This state needs to be communicated from the client to server.  As this
// state is too large to be updated atomically without a mutex, and mutexes aren't allowed here, the
// state is wrapped by a SingleStateQueue.
struct StaticAudioTrackState {
    // Do not define constructors, destructors, or virtual methods as this is part of a
    // union in shared memory and they will not get called properly.

    // These fields should both be size_t, but since they are located in shared memory we
    // force to 32-bit.  The client and server may have different typedefs for size_t.

    // The state has a sequence counter to indicate whether changes are made to loop or position.
    // The sequence counter also currently indicates whether loop or position is first depending
    // on which is greater; it jumps by max(mLoopSequence, mPositionSequence) + 1.

    uint32_t    mLoopStart;
    uint32_t    mLoopEnd;
    int32_t     mLoopCount;
    uint32_t    mLoopSequence; // a sequence counter to indicate changes to loop
    uint32_t    mPosition;
    uint32_t    mPositionSequence; // a sequence counter to indicate changes to position
};

typedef SingleStateQueue<StaticAudioTrackState> StaticAudioTrackSingleStateQueue;

struct StaticAudioTrackPosLoop {
    // Do not define constructors, destructors, or virtual methods as this is part of a
    // union in shared memory and will not get called properly.

    // These fields should both be size_t, but since they are located in shared memory we
    // force to 32-bit.  The client and server may have different typedefs for size_t.

    // This struct information is stored in a single state queue to communicate the
    // static AudioTrack server state to the client while data is consumed.
    // It is smaller than StaticAudioTrackState to prevent unnecessary information from
    // being sent.

    uint32_t mBufferPosition;
    int32_t  mLoopCount;
};

typedef SingleStateQueue<StaticAudioTrackPosLoop> StaticAudioTrackPosLoopQueue;

struct AudioTrackSharedStatic {
    // client requests to the server for loop or position changes.
    StaticAudioTrackSingleStateQueue::Shared
                    mSingleStateQueue;
    // position info updated asynchronously by server and read by client,
    // "for entertainment purposes only"
    StaticAudioTrackPosLoopQueue::Shared
                    mPosLoopQueue;
};

typedef SingleStateQueue<AudioPlaybackRate> PlaybackRateQueue;

typedef SingleStateQueue<ExtendedTimestamp> ExtendedTimestampQueue;

// ----------------------------------------------------------------------------

// Important: do not add any virtual methods, including ~
struct audio_track_cblk_t
{
                // Since the control block is always located in shared memory, this constructor
                // is only used for placement new().  It is never used for regular new() or stack.
                            audio_track_cblk_t();
                /*virtual*/ ~audio_track_cblk_t() { }

                friend class Proxy;
                friend class ClientProxy;
                friend class AudioTrackClientProxy;
                friend class AudioRecordClientProxy;
                friend class ServerProxy;
                friend class AudioTrackServerProxy;
                friend class AudioRecordServerProxy;

    // The data members are grouped so that members accessed frequently and in the same context
    // are in the same line of data cache.

                uint32_t    mServer;    // Number of filled frames consumed by server (mIsOut),
                                        // or filled frames provided by server (!mIsOut).
                                        // It is updated asynchronously by server without a barrier.
                                        // The value should be used
                                        // "for entertainment purposes only",
                                        // which means don't make important decisions based on it.

                uint32_t    mPad1;      // unused

    volatile    int32_t     mFutex;     // event flag: down (P) by client,
                                        // up (V) by server or binderDied() or interrupt()
#define CBLK_FUTEX_WAKE 1               // if event flag bit is set, then a deferred wake is pending

private:

                // This field should be a size_t, but since it is located in shared memory we
                // force to 32-bit.  The client and server may have different typedefs for size_t.
                uint32_t    mMinimum;       // server wakes up client if available >= mMinimum

                // Stereo gains for AudioTrack only, not used by AudioRecord.
                gain_minifloat_packed_t mVolumeLR;

                uint32_t    mSampleRate;    // AudioTrack only: client's requested sample rate in Hz
                                            // or 0 == default. Write-only client, read-only server.

                PlaybackRateQueue::Shared mPlaybackRateQueue;

                // client write-only, server read-only
                uint16_t    mSendLevel;      // Fixed point U4.12 so 0x1000 means 1.0

                uint16_t    mPad2 __attribute__((__unused__)); // unused

                // server write-only, client read
                ExtendedTimestampQueue::Shared mExtendedTimestampQueue;

                // This is set by AudioTrack.setBufferSizeInFrames().
                // A write will not fill the buffer above this limit.
    volatile    uint32_t   mBufferSizeInFrames;  // effective size of the buffer

public:

    volatile    int32_t     mFlags;         // combinations of CBLK_*

public:
                union {
                    AudioTrackSharedStreaming   mStreaming;
                    AudioTrackSharedStatic      mStatic;
                    int                         mAlign[8];
                } u;

                // Cache line boundary (32 bytes)
};

// ----------------------------------------------------------------------------

// Proxy for shared memory control block, to isolate callers from needing to know the details.
// There is exactly one ClientProxy and one ServerProxy per shared memory control block.
// The proxies are located in normal memory, and are not multi-thread safe within a given side.
class Proxy : public RefBase {
protected:
    Proxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount, size_t frameSize, bool isOut,
            bool clientInServer);
    virtual ~Proxy() { }

public:
    struct Buffer {
        size_t  mFrameCount;            // number of frames available in this buffer
        void*   mRaw;                   // pointer to first frame
        size_t  mNonContig;             // number of additional non-contiguous frames available
    };

    size_t frameCount() const { return mFrameCount; }

protected:
    // These refer to shared memory, and are virtual addresses with respect to the current process.
    // They may have different virtual addresses within the other process.
    audio_track_cblk_t* const   mCblk;  // the control block
    void* const     mBuffers;           // starting address of buffers

    const size_t    mFrameCount;        // not necessarily a power of 2
    const size_t    mFrameSize;         // in bytes
    const size_t    mFrameCountP2;      // mFrameCount rounded to power of 2, streaming mode
    const bool      mIsOut;             // true for AudioTrack, false for AudioRecord
    const bool      mClientInServer;    // true for OutputTrack, false for AudioTrack & AudioRecord
    bool            mIsShutdown;        // latch set to true when shared memory corruption detected
    size_t          mUnreleased;        // unreleased frames remaining from most recent obtainBuffer
};

// ----------------------------------------------------------------------------

// Proxy seen by AudioTrack client and AudioRecord client
class ClientProxy : public Proxy {
public:
    ClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount, size_t frameSize,
            bool isOut, bool clientInServer);
    virtual ~ClientProxy() { }

    static const struct timespec kForever;
    static const struct timespec kNonBlocking;

    // Obtain a buffer with filled frames (reading) or empty frames (writing).
    // It is permitted to call obtainBuffer() multiple times in succession, without any intervening
    // calls to releaseBuffer().  In that case, the final obtainBuffer() is the one that effectively
    // sets or extends the unreleased frame count.
    // On entry:
    //  buffer->mFrameCount should be initialized to maximum number of desired frames,
    //      which must be > 0.
    //  buffer->mNonContig is unused.
    //  buffer->mRaw is unused.
    //  requested is the requested timeout in local monotonic delta time units:
    //      NULL or &kNonBlocking means non-blocking (zero timeout).
    //      &kForever means block forever (infinite timeout).
    //      Other values mean a specific timeout in local monotonic delta time units.
    //  elapsed is a pointer to a location that will hold the total local monotonic time that
    //      elapsed while blocked, or NULL if not needed.
    // On exit:
    //  buffer->mFrameCount has the actual number of contiguous available frames,
    //      which is always 0 when the return status != NO_ERROR.
    //  buffer->mNonContig is the number of additional non-contiguous available frames.
    //  buffer->mRaw is a pointer to the first available frame,
    //      or NULL when buffer->mFrameCount == 0.
    // The return status is one of:
    //  NO_ERROR    Success, buffer->mFrameCount > 0.
    //  WOULD_BLOCK Non-blocking mode and no frames are available.
    //  TIMED_OUT   Timeout occurred before any frames became available.
    //              This can happen even for infinite timeout, due to a spurious wakeup.
    //              In this case, the caller should investigate and then re-try as appropriate.
    //  DEAD_OBJECT Server has died or invalidated, caller should destroy this proxy and re-create.
    //  -EINTR      Call has been interrupted.  Look around to see why, and then perhaps try again.
    //  NO_INIT     Shared memory is corrupt.
    //  NOT_ENOUGH_DATA Server has disabled the track because of underrun: restart the track
    //              if still in active state.
    // Assertion failure on entry, if buffer == NULL or buffer->mFrameCount == 0.
    status_t    obtainBuffer(Buffer* buffer, const struct timespec *requested = NULL,
            struct timespec *elapsed = NULL);

    // Release (some of) the frames last obtained.
    // On entry, buffer->mFrameCount should have the number of frames to release,
    // which must (cumulatively) be <= the number of frames last obtained but not yet released.
    // buffer->mRaw is ignored, but is normally same pointer returned by last obtainBuffer().
    // It is permitted to call releaseBuffer() multiple times to release the frames in chunks.
    // On exit:
    //  buffer->mFrameCount is zero.
    //  buffer->mRaw is NULL.
    void        releaseBuffer(Buffer* buffer);

    // Call after detecting server's death
    void        binderDied();

    // Call to force an obtainBuffer() to return quickly with -EINTR
    void        interrupt();

    Modulo<uint32_t> getPosition() {
        return mEpoch + mCblk->mServer;
    }

    void        setEpoch(const Modulo<uint32_t> &epoch) {
        mEpoch = epoch;
    }

    void        setMinimum(size_t minimum) {
        // This can only happen on a 64-bit client
        if (minimum > UINT32_MAX) {
            minimum = UINT32_MAX;
        }
        mCblk->mMinimum = (uint32_t) minimum;
    }

    // Return the number of frames that would need to be obtained and released
    // in order for the client to be aligned at start of buffer
    virtual size_t  getMisalignment();

    Modulo<uint32_t> getEpoch() const {
        return mEpoch;
    }

    uint32_t      getBufferSizeInFrames() const { return mBufferSizeInFrames; }
    // See documentation for AudioTrack::setBufferSizeInFrames()
    uint32_t      setBufferSizeInFrames(uint32_t requestedSize);

    status_t    getTimestamp(ExtendedTimestamp *timestamp) {
        if (timestamp == nullptr) {
            return BAD_VALUE;
        }
        (void) mTimestampObserver.poll(mTimestamp);
        *timestamp = mTimestamp;
        return OK;
    }

    void        clearTimestamp() {
        mTimestamp.clear();
    }

    virtual void stop() { }; // called by client in AudioTrack::stop()

private:
    // This is a copy of mCblk->mBufferSizeInFrames
    uint32_t   mBufferSizeInFrames;  // effective size of the buffer

    Modulo<uint32_t> mEpoch;

    // The shared buffer contents referred to by the timestamp observer
    // is initialized when the server proxy created.  A local zero timestamp
    // is initialized by the client constructor.
    ExtendedTimestampQueue::Observer mTimestampObserver;
    ExtendedTimestamp mTimestamp; // initialized by constructor
};

// ----------------------------------------------------------------------------

// Proxy used by AudioTrack client, which also includes AudioFlinger::PlaybackThread::OutputTrack
class AudioTrackClientProxy : public ClientProxy {
public:
    AudioTrackClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize, bool clientInServer = false)
        : ClientProxy(cblk, buffers, frameCount, frameSize, true /*isOut*/,
          clientInServer),
          mPlaybackRateMutator(&cblk->mPlaybackRateQueue) {
    }

    virtual ~AudioTrackClientProxy() { }

    // No barriers on the following operations, so the ordering of loads/stores
    // with respect to other parameters is UNPREDICTABLE. That's considered safe.

    // caller must limit to 0.0 <= sendLevel <= 1.0
    void        setSendLevel(float sendLevel) {
        mCblk->mSendLevel = uint16_t(sendLevel * 0x1000);
    }

    // set stereo gains
    void        setVolumeLR(gain_minifloat_packed_t volumeLR) {
        mCblk->mVolumeLR = volumeLR;
    }

    void        setSampleRate(uint32_t sampleRate) {
        mCblk->mSampleRate = sampleRate;
    }

    void        setPlaybackRate(const AudioPlaybackRate& playbackRate) {
        mPlaybackRateMutator.push(playbackRate);
    }

    // Sends flush and stop position information from the client to the server,
    // used by streaming AudioTrack flush() or stop().
    void sendStreamingFlushStop(bool flush);

    virtual void flush();

            void stop() override;

    virtual uint32_t    getUnderrunFrames() const {
        return mCblk->u.mStreaming.mUnderrunFrames;
    }
    virtual uint32_t    getUnderrunCount() const {
        return mCblk->u.mStreaming.mUnderrunCount;
    }

    bool        clearStreamEndDone();   // and return previous value

    bool        getStreamEndDone() const;

    status_t    waitStreamEndDone(const struct timespec *requested);

private:
    PlaybackRateQueue::Mutator   mPlaybackRateMutator;
};

class StaticAudioTrackClientProxy : public AudioTrackClientProxy {
public:
    StaticAudioTrackClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize);
    virtual ~StaticAudioTrackClientProxy() { }

    virtual void    flush();

    void stop() override;

#define MIN_LOOP    16  // minimum length of each loop iteration in frames

            // setLoop(), setBufferPosition(), and setBufferPositionAndLoop() set the
            // static buffer position and looping parameters.  These commands are not
            // synchronous (they do not wait or block); instead they take effect at the
            // next buffer data read from the server side. However, the client side
            // getters will read a cached version of the position and loop variables
            // until the setting takes effect.
            //
            // setBufferPositionAndLoop() is equivalent to calling, in order, setLoop() and
            // setBufferPosition().
            //
            // The functions should not be relied upon to do parameter or state checking.
            // That is done at the AudioTrack level.

            void    setLoop(size_t loopStart, size_t loopEnd, int loopCount);
            void    setBufferPosition(size_t position);
            void    setBufferPositionAndLoop(size_t position, size_t loopStart, size_t loopEnd,
                                             int loopCount);
            size_t  getBufferPosition();
                    // getBufferPositionAndLoopCount() provides the proper snapshot of
                    // position and loopCount together.
            void    getBufferPositionAndLoopCount(size_t *position, int *loopCount);

    virtual size_t  getMisalignment() {
        return 0;
    }

    virtual uint32_t getUnderrunFrames() const override {
        return 0;
    }

    virtual uint32_t getUnderrunCount() const override {
        return 0;
    }

private:
    StaticAudioTrackSingleStateQueue::Mutator   mMutator;
    StaticAudioTrackPosLoopQueue::Observer      mPosLoopObserver;
                        StaticAudioTrackState   mState;   // last communicated state to server
                        StaticAudioTrackPosLoop mPosLoop; // snapshot of position and loop.
};

// ----------------------------------------------------------------------------

// Proxy used by AudioRecord client
class AudioRecordClientProxy : public ClientProxy {
public:
    AudioRecordClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize)
        : ClientProxy(cblk, buffers, frameCount, frameSize,
            false /*isOut*/, false /*clientInServer*/) { }
    ~AudioRecordClientProxy() { }

    // Advances the client read pointer to the server write head pointer
    // effectively flushing the client read buffer. The effect is
    // instantaneous. Returns the number of frames flushed.
    uint32_t    flush() {
        int32_t rear = android_atomic_acquire_load(&mCblk->u.mStreaming.mRear);
        int32_t front = mCblk->u.mStreaming.mFront;
        android_atomic_release_store(rear, &mCblk->u.mStreaming.mFront);
        return (Modulo<int32_t>(rear) - front).unsignedValue();
    }
};

// ----------------------------------------------------------------------------

// Proxy used by AudioFlinger server
class ServerProxy : public Proxy {
protected:
    ServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount, size_t frameSize,
            bool isOut, bool clientInServer);
public:
    virtual ~ServerProxy() { }

    // Obtain a buffer with filled frames (writing) or empty frames (reading).
    // It is permitted to call obtainBuffer() multiple times in succession, without any intervening
    // calls to releaseBuffer().  In that case, the final obtainBuffer() is the one that effectively
    // sets or extends the unreleased frame count.
    // Always non-blocking.
    // On entry:
    //  buffer->mFrameCount should be initialized to maximum number of desired frames,
    //      which must be > 0.
    //  buffer->mNonContig is unused.
    //  buffer->mRaw is unused.
    //  ackFlush is true iff being called from Track::start to acknowledge a pending flush.
    // On exit:
    //  buffer->mFrameCount has the actual number of contiguous available frames,
    //      which is always 0 when the return status != NO_ERROR.
    //  buffer->mNonContig is the number of additional non-contiguous available frames.
    //  buffer->mRaw is a pointer to the first available frame,
    //      or NULL when buffer->mFrameCount == 0.
    // The return status is one of:
    //  NO_ERROR    Success, buffer->mFrameCount > 0.
    //  WOULD_BLOCK No frames are available.
    //  NO_INIT     Shared memory is corrupt.
    virtual status_t    obtainBuffer(Buffer* buffer, bool ackFlush = false);

    // Release (some of) the frames last obtained.
    // On entry, buffer->mFrameCount should have the number of frames to release,
    // which must (cumulatively) be <= the number of frames last obtained but not yet released.
    // It is permitted to call releaseBuffer() multiple times to release the frames in chunks.
    // buffer->mRaw is ignored, but is normally same pointer returned by last obtainBuffer().
    // On exit:
    //  buffer->mFrameCount is zero.
    //  buffer->mRaw is NULL.
    virtual void        releaseBuffer(Buffer* buffer);

    // Return the total number of frames that AudioFlinger has obtained and released
    virtual int64_t     framesReleased() const { return mReleased; }

    // Expose timestamp to client proxy. Should only be called by a single thread.
    virtual void        setTimestamp(const ExtendedTimestamp &timestamp) {
        mTimestampMutator.push(timestamp);
    }

    virtual ExtendedTimestamp getTimestamp() const {
        return mTimestampMutator.last();
    }

    // Flushes the shared ring buffer if the client had requested it using mStreaming.mFlush.
    // If flush occurs then:
    //   cblk->u.mStreaming.mFront, ServerProxy::mFlush and ServerProxy::mFlushed will be modified
    //   client will be notified via Futex
    virtual void    flushBufferIfNeeded();

    // Returns the rear position of the AudioTrack shared ring buffer, limited by
    // the stop frame position level.
    virtual int32_t getRear() const = 0;

    // Total count of the number of flushed frames since creation (never reset).
    virtual int64_t     framesFlushed() const { return mFlushed; }

    // Safe frames ready query with no side effects.
    virtual size_t      framesReadySafe() const = 0;

    // Get dynamic buffer size from the shared control block.
    uint32_t            getBufferSizeInFrames() const {
        return android_atomic_acquire_load((int32_t *)&mCblk->mBufferSizeInFrames);
    }

protected:
    size_t      mAvailToClient; // estimated frames available to client prior to releaseBuffer()
    int32_t     mFlush;         // our copy of cblk->u.mStreaming.mFlush, for streaming output only
    int64_t     mReleased;      // our copy of cblk->mServer, at 64 bit resolution
    int64_t     mFlushed;       // flushed frames to account for client-server discrepancy
    ExtendedTimestampQueue::Mutator mTimestampMutator;
};

// Proxy used by AudioFlinger for servicing AudioTrack
class AudioTrackServerProxy : public ServerProxy {
public:
    AudioTrackServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize, bool clientInServer = false, uint32_t sampleRate = 0)
        : ServerProxy(cblk, buffers, frameCount, frameSize, true /*isOut*/, clientInServer),
          mPlaybackRateObserver(&cblk->mPlaybackRateQueue),
          mUnderrunCount(0), mUnderrunning(false), mDrained(true) {
        mCblk->mSampleRate = sampleRate;
        mPlaybackRate = AUDIO_PLAYBACK_RATE_DEFAULT;
    }
protected:
    virtual ~AudioTrackServerProxy() { }

public:
    // return value of these methods must be validated by the caller
    uint32_t    getSampleRate() const { return mCblk->mSampleRate; }
    uint16_t    getSendLevel_U4_12() const { return mCblk->mSendLevel; }
    gain_minifloat_packed_t getVolumeLR() const { return mCblk->mVolumeLR; }

    // estimated total number of filled frames available to server to read,
    // which may include non-contiguous frames
    virtual size_t      framesReady();

    size_t              framesReadySafe() const override; // frames available to read by server.

    // Currently AudioFlinger will call framesReady() for a fast track from two threads:
    // FastMixer thread, and normal mixer thread.  This is dangerous, as the proxy is intended
    // to be called from at most one thread of server, and one thread of client.
    // As a temporary workaround, this method informs the proxy implementation that it
    // should avoid doing a state queue poll from within framesReady().
    // FIXME Change AudioFlinger to not call framesReady() from normal mixer thread.
    virtual void        framesReadyIsCalledByMultipleThreads() { }

    bool     setStreamEndDone();    // and return previous value

    // Add to the tally of underrun frames, and inform client of underrun
    virtual void        tallyUnderrunFrames(uint32_t frameCount);

    // Return the total number of frames which AudioFlinger desired but were unavailable,
    // and thus which resulted in an underrun.
    virtual uint32_t    getUnderrunFrames() const { return mCblk->u.mStreaming.mUnderrunFrames; }

    // Return the playback speed and pitch read atomically. Not multi-thread safe on server side.
    AudioPlaybackRate getPlaybackRate();

    // Set the internal drain state of the track buffer from the timestamp received.
    virtual void        setDrained(bool drained) {
        mDrained.store(drained);
    }

    // Check if the internal drain state of the track buffer.
    // This is not a guarantee, but advisory for determining whether the track is
    // fully played out.
    virtual bool        isDrained() const {
        return mDrained.load();
    }

    int32_t             getRear() const override;

    // Called on server side track start().
    virtual void        start();

private:
    AudioPlaybackRate             mPlaybackRate;  // last observed playback rate
    PlaybackRateQueue::Observer   mPlaybackRateObserver;

    // Last client stop-at position when start() was called. Used for streaming AudioTracks.
    std::atomic<int32_t>          mStopLast{0};

    // The server keeps a copy here where it is safe from the client.
    uint32_t                      mUnderrunCount; // echoed to mCblk
    bool                          mUnderrunning;  // used to detect edge of underrun

    std::atomic<bool>             mDrained; // is the track buffer drained
};

class StaticAudioTrackServerProxy : public AudioTrackServerProxy {
public:
    StaticAudioTrackServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize);
protected:
    virtual ~StaticAudioTrackServerProxy() { }

public:
    virtual size_t      framesReady();
    virtual size_t      framesReadySafe() const override;
    virtual void        framesReadyIsCalledByMultipleThreads();
    virtual status_t    obtainBuffer(Buffer* buffer, bool ackFlush);
    virtual void        releaseBuffer(Buffer* buffer);
    virtual void        tallyUnderrunFrames(uint32_t frameCount);
    virtual uint32_t    getUnderrunFrames() const { return 0; }

    int32_t getRear() const override;

    void start() override { } // ignore for static tracks

private:
    status_t            updateStateWithLoop(StaticAudioTrackState *localState,
                                            const StaticAudioTrackState &update) const;
    status_t            updateStateWithPosition(StaticAudioTrackState *localState,
                                                const StaticAudioTrackState &update) const;
    ssize_t             pollPosition(); // poll for state queue update, and return current position
    StaticAudioTrackSingleStateQueue::Observer  mObserver;
    StaticAudioTrackPosLoopQueue::Mutator       mPosLoopMutator;
    size_t              mFramesReadySafe; // Assuming size_t read/writes are atomic on 32 / 64 bit
                                          // processors, this is a thread-safe version of
                                          // mFramesReady.
    int64_t             mFramesReady;     // The number of frames ready in the static buffer
                                          // including loops.  This is 64 bits since loop mode
                                          // can cause a track to appear to have a large number
                                          // of frames. INT64_MAX means an infinite loop.
    bool                mFramesReadyIsCalledByMultipleThreads;
    StaticAudioTrackState mState;         // Server side state. Any updates from client must be
                                          // passed by the mObserver SingleStateQueue.
};

// Proxy used by AudioFlinger for servicing AudioRecord
class AudioRecordServerProxy : public ServerProxy {
public:
    AudioRecordServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
            size_t frameSize, bool clientInServer)
        : ServerProxy(cblk, buffers, frameCount, frameSize, false /*isOut*/, clientInServer) { }

    int32_t getRear() const override {
        return mCblk->u.mStreaming.mRear; // For completeness only; mRear written by server.
    }

    size_t framesReadySafe() const override; // frames available to read by client.

protected:
    virtual ~AudioRecordServerProxy() { }
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_AUDIO_TRACK_SHARED_H
