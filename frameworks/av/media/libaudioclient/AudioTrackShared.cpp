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

#define LOG_TAG "AudioTrackShared"
//#define LOG_NDEBUG 0

#include <android-base/macros.h>
#include <private/media/AudioTrackShared.h>
#include <utils/Log.h>
#include <audio_utils/safe_math.h>

#include <linux/futex.h>
#include <sys/syscall.h>

namespace android {

// used to clamp a value to size_t.  TODO: move to another file.
template <typename T>
size_t clampToSize(T x) {
    return sizeof(T) > sizeof(size_t) && x > (T) SIZE_MAX ? SIZE_MAX : x < 0 ? 0 : (size_t) x;
}

// incrementSequence is used to determine the next sequence value
// for the loop and position sequence counters.  It should return
// a value between "other" + 1 and "other" + INT32_MAX, the choice of
// which needs to be the "least recently used" sequence value for "self".
// In general, this means (new_self) returned is max(self, other) + 1.
__attribute__((no_sanitize("integer")))
static uint32_t incrementSequence(uint32_t self, uint32_t other) {
    int32_t diff = (int32_t) self - (int32_t) other;
    if (diff >= 0 && diff < INT32_MAX) {
        return self + 1; // we're already ahead of other.
    }
    return other + 1; // we're behind, so move just ahead of other.
}

audio_track_cblk_t::audio_track_cblk_t()
    : mServer(0), mFutex(0), mMinimum(0)
    , mVolumeLR(GAIN_MINIFLOAT_PACKED_UNITY), mSampleRate(0), mSendLevel(0)
    , mBufferSizeInFrames(0)
    , mFlags(0)
{
    memset(&u, 0, sizeof(u));
}

// ---------------------------------------------------------------------------

Proxy::Proxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount, size_t frameSize,
        bool isOut, bool clientInServer)
    : mCblk(cblk), mBuffers(buffers), mFrameCount(frameCount), mFrameSize(frameSize),
      mFrameCountP2(roundup(frameCount)), mIsOut(isOut), mClientInServer(clientInServer),
      mIsShutdown(false), mUnreleased(0)
{
}

// ---------------------------------------------------------------------------

ClientProxy::ClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
        size_t frameSize, bool isOut, bool clientInServer)
    : Proxy(cblk, buffers, frameCount, frameSize, isOut, clientInServer)
    , mEpoch(0)
    , mTimestampObserver(&cblk->mExtendedTimestampQueue)
{
    setBufferSizeInFrames(frameCount);
}

const struct timespec ClientProxy::kForever = {INT_MAX /*tv_sec*/, 0 /*tv_nsec*/};
const struct timespec ClientProxy::kNonBlocking = {0 /*tv_sec*/, 0 /*tv_nsec*/};

#define MEASURE_NS 10000000 // attempt to provide accurate timeouts if requested >= MEASURE_NS

// To facilitate quicker recovery from server failure, this value limits the timeout per each futex
// wait.  However it does not protect infinite timeouts.  If defined to be zero, there is no limit.
// FIXME May not be compatible with audio tunneling requirements where timeout should be in the
// order of minutes.
#define MAX_SEC    5

uint32_t ClientProxy::setBufferSizeInFrames(uint32_t size)
{
    // The minimum should be  greater than zero and less than the size
    // at which underruns will occur.
    const uint32_t minimum = 16; // based on AudioMixer::BLOCKSIZE
    const uint32_t maximum = frameCount();
    uint32_t clippedSize = size;
    if (maximum < minimum) {
        clippedSize = maximum;
    } else if (clippedSize < minimum) {
        clippedSize = minimum;
    } else if (clippedSize > maximum) {
        clippedSize = maximum;
    }
    // for server to read
    android_atomic_release_store(clippedSize, (int32_t *)&mCblk->mBufferSizeInFrames);
    // for client to read
    mBufferSizeInFrames = clippedSize;
    return clippedSize;
}

__attribute__((no_sanitize("integer")))
status_t ClientProxy::obtainBuffer(Buffer* buffer, const struct timespec *requested,
        struct timespec *elapsed)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL || buffer->mFrameCount == 0,
            "%s: null or zero frame buffer, buffer:%p", __func__, buffer);
    struct timespec total;          // total elapsed time spent waiting
    total.tv_sec = 0;
    total.tv_nsec = 0;
    bool measure = elapsed != NULL; // whether to measure total elapsed time spent waiting

    status_t status;
    enum {
        TIMEOUT_ZERO,       // requested == NULL || *requested == 0
        TIMEOUT_INFINITE,   // *requested == infinity
        TIMEOUT_FINITE,     // 0 < *requested < infinity
        TIMEOUT_CONTINUE,   // additional chances after TIMEOUT_FINITE
    } timeout;
    if (requested == NULL) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == 0 && requested->tv_nsec == 0) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == INT_MAX) {
        timeout = TIMEOUT_INFINITE;
    } else {
        timeout = TIMEOUT_FINITE;
        if (requested->tv_sec > 0 || requested->tv_nsec >= MEASURE_NS) {
            measure = true;
        }
    }
    struct timespec before;
    bool beforeIsValid = false;
    audio_track_cblk_t* cblk = mCblk;
    bool ignoreInitialPendingInterrupt = true;
    // check for shared memory corruption
    if (mIsShutdown) {
        status = NO_INIT;
        goto end;
    }
    for (;;) {
        int32_t flags = android_atomic_and(~CBLK_INTERRUPT, &cblk->mFlags);
        // check for track invalidation by server, or server death detection
        if (flags & CBLK_INVALID) {
            ALOGV("Track invalidated");
            status = DEAD_OBJECT;
            goto end;
        }
        if (flags & CBLK_DISABLED) {
            ALOGV("Track disabled");
            status = NOT_ENOUGH_DATA;
            goto end;
        }
        // check for obtainBuffer interrupted by client
        if (!ignoreInitialPendingInterrupt && (flags & CBLK_INTERRUPT)) {
            ALOGV("obtainBuffer() interrupted by client");
            status = -EINTR;
            goto end;
        }
        ignoreInitialPendingInterrupt = false;
        // compute number of frames available to write (AudioTrack) or read (AudioRecord)
        int32_t front;
        int32_t rear;
        if (mIsOut) {
            // The barrier following the read of mFront is probably redundant.
            // We're about to perform a conditional branch based on 'filled',
            // which will force the processor to observe the read of mFront
            // prior to allowing data writes starting at mRaw.
            // However, the processor may support speculative execution,
            // and be unable to undo speculative writes into shared memory.
            // The barrier will prevent such speculative execution.
            front = android_atomic_acquire_load(&cblk->u.mStreaming.mFront);
            rear = cblk->u.mStreaming.mRear;
        } else {
            // On the other hand, this barrier is required.
            rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
            front = cblk->u.mStreaming.mFront;
        }
        // write to rear, read from front
        ssize_t filled = audio_utils::safe_sub_overflow(rear, front);
        // pipe should not be overfull
        if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
            if (mIsOut) {
                ALOGE("Shared memory control block is corrupt (filled=%zd, mFrameCount=%zu); "
                        "shutting down", filled, mFrameCount);
                mIsShutdown = true;
                status = NO_INIT;
                goto end;
            }
            // for input, sync up on overrun
            filled = 0;
            cblk->u.mStreaming.mFront = rear;
            (void) android_atomic_or(CBLK_OVERRUN, &cblk->mFlags);
        }
        // Don't allow filling pipe beyond the user settable size.
        // The calculation for avail can go negative if the buffer size
        // is suddenly dropped below the amount already in the buffer.
        // So use a signed calculation to prevent a numeric overflow abort.
        ssize_t adjustableSize = (ssize_t) getBufferSizeInFrames();
        ssize_t avail =  (mIsOut) ? adjustableSize - filled : filled;
        if (avail < 0) {
            avail = 0;
        } else if (avail > 0) {
            // 'avail' may be non-contiguous, so return only the first contiguous chunk
            size_t part1;
            if (mIsOut) {
                rear &= mFrameCountP2 - 1;
                part1 = mFrameCountP2 - rear;
            } else {
                front &= mFrameCountP2 - 1;
                part1 = mFrameCountP2 - front;
            }
            if (part1 > (size_t)avail) {
                part1 = avail;
            }
            if (part1 > buffer->mFrameCount) {
                part1 = buffer->mFrameCount;
            }
            buffer->mFrameCount = part1;
            buffer->mRaw = part1 > 0 ?
                    &((char *) mBuffers)[(mIsOut ? rear : front) * mFrameSize] : NULL;
            buffer->mNonContig = avail - part1;
            mUnreleased = part1;
            status = NO_ERROR;
            break;
        }
        struct timespec remaining;
        const struct timespec *ts;
        switch (timeout) {
        case TIMEOUT_ZERO:
            status = WOULD_BLOCK;
            goto end;
        case TIMEOUT_INFINITE:
            ts = NULL;
            break;
        case TIMEOUT_FINITE:
            timeout = TIMEOUT_CONTINUE;
            if (MAX_SEC == 0) {
                ts = requested;
                break;
            }
            FALLTHROUGH_INTENDED;
        case TIMEOUT_CONTINUE:
            // FIXME we do not retry if requested < 10ms? needs documentation on this state machine
            if (!measure || requested->tv_sec < total.tv_sec ||
                    (requested->tv_sec == total.tv_sec && requested->tv_nsec <= total.tv_nsec)) {
                status = TIMED_OUT;
                goto end;
            }
            remaining.tv_sec = requested->tv_sec - total.tv_sec;
            if ((remaining.tv_nsec = requested->tv_nsec - total.tv_nsec) < 0) {
                remaining.tv_nsec += 1000000000;
                remaining.tv_sec++;
            }
            if (0 < MAX_SEC && MAX_SEC < remaining.tv_sec) {
                remaining.tv_sec = MAX_SEC;
                remaining.tv_nsec = 0;
            }
            ts = &remaining;
            break;
        default:
            LOG_ALWAYS_FATAL("obtainBuffer() timeout=%d", timeout);
            ts = NULL;
            break;
        }
        int32_t old = android_atomic_and(~CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            if (measure && !beforeIsValid) {
                clock_gettime(CLOCK_MONOTONIC, &before);
                beforeIsValid = true;
            }
            errno = 0;
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAIT_PRIVATE : FUTEX_WAIT, old & ~CBLK_FUTEX_WAKE, ts);
            status_t error = errno; // clock_gettime can affect errno
            // update total elapsed time spent waiting
            if (measure) {
                struct timespec after;
                clock_gettime(CLOCK_MONOTONIC, &after);
                total.tv_sec += after.tv_sec - before.tv_sec;
                // Use auto instead of long to avoid the google-runtime-int warning.
                auto deltaNs = after.tv_nsec - before.tv_nsec;
                if (deltaNs < 0) {
                    deltaNs += 1000000000;
                    total.tv_sec--;
                }
                if ((total.tv_nsec += deltaNs) >= 1000000000) {
                    total.tv_nsec -= 1000000000;
                    total.tv_sec++;
                }
                before = after;
                beforeIsValid = true;
            }
            switch (error) {
            case 0:            // normal wakeup by server, or by binderDied()
            case EWOULDBLOCK:  // benign race condition with server
            case EINTR:        // wait was interrupted by signal or other spurious wakeup
            case ETIMEDOUT:    // time-out expired
                // FIXME these error/non-0 status are being dropped
                break;
            default:
                status = error;
                ALOGE("%s unexpected error %s", __func__, strerror(status));
                goto end;
            }
        }
    }

end:
    if (status != NO_ERROR) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
    }
    if (elapsed != NULL) {
        *elapsed = total;
    }
    if (requested == NULL) {
        requested = &kNonBlocking;
    }
    if (measure) {
        ALOGV("requested %ld.%03ld elapsed %ld.%03ld",
              requested->tv_sec, requested->tv_nsec / 1000000,
              total.tv_sec, total.tv_nsec / 1000000);
    }
    return status;
}

__attribute__((no_sanitize("integer")))
void ClientProxy::releaseBuffer(Buffer* buffer)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL);
    size_t stepCount = buffer->mFrameCount;
    if (stepCount == 0 || mIsShutdown) {
        // prevent accidental re-use of buffer
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount),
            "%s: mUnreleased out of range, "
            "!(stepCount:%zu <= mUnreleased:%zu <= mFrameCount:%zu), BufferSizeInFrames:%u",
            __func__, stepCount, mUnreleased, mFrameCount, getBufferSizeInFrames());
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    // Both of these barriers are required
    if (mIsOut) {
        int32_t rear = cblk->u.mStreaming.mRear;
        android_atomic_release_store(stepCount + rear, &cblk->u.mStreaming.mRear);
    } else {
        int32_t front = cblk->u.mStreaming.mFront;
        android_atomic_release_store(stepCount + front, &cblk->u.mStreaming.mFront);
    }
}

void ClientProxy::binderDied()
{
    audio_track_cblk_t* cblk = mCblk;
    if (!(android_atomic_or(CBLK_INVALID, &cblk->mFlags) & CBLK_INVALID)) {
        android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        // it seems that a FUTEX_WAKE_PRIVATE will not wake a FUTEX_WAIT, even within same process
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
}

void ClientProxy::interrupt()
{
    audio_track_cblk_t* cblk = mCblk;
    if (!(android_atomic_or(CBLK_INTERRUPT, &cblk->mFlags) & CBLK_INTERRUPT)) {
        android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
}

__attribute__((no_sanitize("integer")))
size_t ClientProxy::getMisalignment()
{
    audio_track_cblk_t* cblk = mCblk;
    return (mFrameCountP2 - (mIsOut ? cblk->u.mStreaming.mRear : cblk->u.mStreaming.mFront)) &
            (mFrameCountP2 - 1);
}

// ---------------------------------------------------------------------------

void AudioTrackClientProxy::flush()
{
    sendStreamingFlushStop(true /* flush */);
}

void AudioTrackClientProxy::stop()
{
    sendStreamingFlushStop(false /* flush */);
}

// Sets the client-written mFlush and mStop positions, which control server behavior.
//
// @param flush indicates whether the operation is a flush or stop.
// A client stop sets mStop to the current write position;
// the server will not read past this point until start() or subsequent flush().
// A client flush sets both mStop and mFlush to the current write position.
// This advances the server read limit (if previously set) and on the next
// server read advances the server read position to this limit.
//
void AudioTrackClientProxy::sendStreamingFlushStop(bool flush)
{
    // TODO: Replace this by 64 bit counters - avoids wrap complication.
    // This works for mFrameCountP2 <= 2^30
    // mFlush is 32 bits concatenated as [ flush_counter ] [ newfront_offset ]
    // Should newFlush = cblk->u.mStreaming.mRear?  Only problem is
    // if you want to flush twice to the same rear location after a 32 bit wrap.

    const size_t increment = mFrameCountP2 << 1;
    const size_t mask = increment - 1;
    // No need for client atomic synchronization on mRear, mStop, mFlush
    // as AudioTrack client only read/writes to them under client lock. Server only reads.
    const int32_t rearMasked = mCblk->u.mStreaming.mRear & mask;

    // update stop before flush so that the server front
    // never advances beyond a (potential) previous stop's rear limit.
    int32_t stopBits; // the following add can overflow
    __builtin_add_overflow(mCblk->u.mStreaming.mStop & ~mask, increment, &stopBits);
    android_atomic_release_store(rearMasked | stopBits, &mCblk->u.mStreaming.mStop);

    if (flush) {
        int32_t flushBits; // the following add can overflow
        __builtin_add_overflow(mCblk->u.mStreaming.mFlush & ~mask, increment, &flushBits);
        android_atomic_release_store(rearMasked | flushBits, &mCblk->u.mStreaming.mFlush);
    }
}

bool AudioTrackClientProxy::clearStreamEndDone() {
    return (android_atomic_and(~CBLK_STREAM_END_DONE, &mCblk->mFlags) & CBLK_STREAM_END_DONE) != 0;
}

bool AudioTrackClientProxy::getStreamEndDone() const {
    return (mCblk->mFlags & CBLK_STREAM_END_DONE) != 0;
}

status_t AudioTrackClientProxy::waitStreamEndDone(const struct timespec *requested)
{
    struct timespec total;          // total elapsed time spent waiting
    total.tv_sec = 0;
    total.tv_nsec = 0;
    audio_track_cblk_t* cblk = mCblk;
    status_t status;
    enum {
        TIMEOUT_ZERO,       // requested == NULL || *requested == 0
        TIMEOUT_INFINITE,   // *requested == infinity
        TIMEOUT_FINITE,     // 0 < *requested < infinity
        TIMEOUT_CONTINUE,   // additional chances after TIMEOUT_FINITE
    } timeout;
    if (requested == NULL) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == 0 && requested->tv_nsec == 0) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == INT_MAX) {
        timeout = TIMEOUT_INFINITE;
    } else {
        timeout = TIMEOUT_FINITE;
    }
    for (;;) {
        int32_t flags = android_atomic_and(~(CBLK_INTERRUPT|CBLK_STREAM_END_DONE), &cblk->mFlags);
        // check for track invalidation by server, or server death detection
        if (flags & CBLK_INVALID) {
            ALOGV("Track invalidated");
            status = DEAD_OBJECT;
            goto end;
        }
        // a track is not supposed to underrun at this stage but consider it done
        if (flags & (CBLK_STREAM_END_DONE | CBLK_DISABLED)) {
            ALOGV("stream end received");
            status = NO_ERROR;
            goto end;
        }
        // check for obtainBuffer interrupted by client
        if (flags & CBLK_INTERRUPT) {
            ALOGV("waitStreamEndDone() interrupted by client");
            status = -EINTR;
            goto end;
        }
        struct timespec remaining;
        const struct timespec *ts;
        switch (timeout) {
        case TIMEOUT_ZERO:
            status = WOULD_BLOCK;
            goto end;
        case TIMEOUT_INFINITE:
            ts = NULL;
            break;
        case TIMEOUT_FINITE:
            timeout = TIMEOUT_CONTINUE;
            if (MAX_SEC == 0) {
                ts = requested;
                break;
            }
            FALLTHROUGH_INTENDED;
        case TIMEOUT_CONTINUE:
            // FIXME we do not retry if requested < 10ms? needs documentation on this state machine
            if (requested->tv_sec < total.tv_sec ||
                    (requested->tv_sec == total.tv_sec && requested->tv_nsec <= total.tv_nsec)) {
                status = TIMED_OUT;
                goto end;
            }
            remaining.tv_sec = requested->tv_sec - total.tv_sec;
            if ((remaining.tv_nsec = requested->tv_nsec - total.tv_nsec) < 0) {
                remaining.tv_nsec += 1000000000;
                remaining.tv_sec++;
            }
            if (0 < MAX_SEC && MAX_SEC < remaining.tv_sec) {
                remaining.tv_sec = MAX_SEC;
                remaining.tv_nsec = 0;
            }
            ts = &remaining;
            break;
        default:
            LOG_ALWAYS_FATAL("waitStreamEndDone() timeout=%d", timeout);
            ts = NULL;
            break;
        }
        int32_t old = android_atomic_and(~CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            errno = 0;
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAIT_PRIVATE : FUTEX_WAIT, old & ~CBLK_FUTEX_WAKE, ts);
            switch (errno) {
            case 0:            // normal wakeup by server, or by binderDied()
            case EWOULDBLOCK:  // benign race condition with server
            case EINTR:        // wait was interrupted by signal or other spurious wakeup
            case ETIMEDOUT:    // time-out expired
                break;
            default:
                status = errno;
                ALOGE("%s unexpected error %s", __func__, strerror(status));
                goto end;
            }
        }
    }

end:
    if (requested == NULL) {
        requested = &kNonBlocking;
    }
    return status;
}

// ---------------------------------------------------------------------------

StaticAudioTrackClientProxy::StaticAudioTrackClientProxy(audio_track_cblk_t* cblk, void *buffers,
        size_t frameCount, size_t frameSize)
    : AudioTrackClientProxy(cblk, buffers, frameCount, frameSize),
      mMutator(&cblk->u.mStatic.mSingleStateQueue),
      mPosLoopObserver(&cblk->u.mStatic.mPosLoopQueue)
{
    memset(&mState, 0, sizeof(mState));
    memset(&mPosLoop, 0, sizeof(mPosLoop));
}

void StaticAudioTrackClientProxy::flush()
{
    LOG_ALWAYS_FATAL("static flush");
}

void StaticAudioTrackClientProxy::stop()
{
    ; // no special handling required for static tracks.
}

void StaticAudioTrackClientProxy::setLoop(size_t loopStart, size_t loopEnd, int loopCount)
{
    // This can only happen on a 64-bit client
    if (loopStart > UINT32_MAX || loopEnd > UINT32_MAX) {
        // FIXME Should return an error status
        return;
    }
    mState.mLoopStart = (uint32_t) loopStart;
    mState.mLoopEnd = (uint32_t) loopEnd;
    mState.mLoopCount = loopCount;
    mState.mLoopSequence = incrementSequence(mState.mLoopSequence, mState.mPositionSequence);
    // set patch-up variables until the mState is acknowledged by the ServerProxy.
    // observed buffer position and loop count will freeze until then to give the
    // illusion of a synchronous change.
    getBufferPositionAndLoopCount(NULL, NULL);
    // preserve behavior to restart at mState.mLoopStart if position exceeds mState.mLoopEnd.
    if (mState.mLoopCount != 0 && mPosLoop.mBufferPosition >= mState.mLoopEnd) {
        mPosLoop.mBufferPosition = mState.mLoopStart;
    }
    mPosLoop.mLoopCount = mState.mLoopCount;
    (void) mMutator.push(mState);
}

void StaticAudioTrackClientProxy::setBufferPosition(size_t position)
{
    // This can only happen on a 64-bit client
    if (position > UINT32_MAX) {
        // FIXME Should return an error status
        return;
    }
    mState.mPosition = (uint32_t) position;
    mState.mPositionSequence = incrementSequence(mState.mPositionSequence, mState.mLoopSequence);
    // set patch-up variables until the mState is acknowledged by the ServerProxy.
    // observed buffer position and loop count will freeze until then to give the
    // illusion of a synchronous change.
    if (mState.mLoopCount > 0) {  // only check if loop count is changing
        getBufferPositionAndLoopCount(NULL, NULL); // get last position
    }
    mPosLoop.mBufferPosition = position;
    if (position >= mState.mLoopEnd) {
        // no ongoing loop is possible if position is greater than loopEnd.
        mPosLoop.mLoopCount = 0;
    }
    (void) mMutator.push(mState);
}

void StaticAudioTrackClientProxy::setBufferPositionAndLoop(size_t position, size_t loopStart,
        size_t loopEnd, int loopCount)
{
    setLoop(loopStart, loopEnd, loopCount);
    setBufferPosition(position);
}

size_t StaticAudioTrackClientProxy::getBufferPosition()
{
    getBufferPositionAndLoopCount(NULL, NULL);
    return mPosLoop.mBufferPosition;
}

void StaticAudioTrackClientProxy::getBufferPositionAndLoopCount(
        size_t *position, int *loopCount)
{
    if (mMutator.ack() == StaticAudioTrackSingleStateQueue::SSQ_DONE) {
         if (mPosLoopObserver.poll(mPosLoop)) {
             ; // a valid mPosLoop should be available if ackDone is true.
         }
    }
    if (position != NULL) {
        *position = mPosLoop.mBufferPosition;
    }
    if (loopCount != NULL) {
        *loopCount = mPosLoop.mLoopCount;
    }
}

// ---------------------------------------------------------------------------

ServerProxy::ServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
        size_t frameSize, bool isOut, bool clientInServer)
    : Proxy(cblk, buffers, frameCount, frameSize, isOut, clientInServer),
      mAvailToClient(0), mFlush(0), mReleased(0), mFlushed(0)
    , mTimestampMutator(&cblk->mExtendedTimestampQueue)
{
    cblk->mBufferSizeInFrames = frameCount;
}

__attribute__((no_sanitize("integer")))
void ServerProxy::flushBufferIfNeeded()
{
    audio_track_cblk_t* cblk = mCblk;
    // The acquire_load is not really required. But since the write is a release_store in the
    // client, using acquire_load here makes it easier for people to maintain the code,
    // and the logic for communicating ipc variables seems somewhat standard,
    // and there really isn't much penalty for 4 or 8 byte atomics.
    int32_t flush = android_atomic_acquire_load(&cblk->u.mStreaming.mFlush);
    if (flush != mFlush) {
        ALOGV("ServerProxy::flushBufferIfNeeded() mStreaming.mFlush = 0x%x, mFlush = 0x%0x",
                flush, mFlush);
        // shouldn't matter, but for range safety use mRear instead of getRear().
        int32_t rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
        int32_t front = cblk->u.mStreaming.mFront;

        // effectively obtain then release whatever is in the buffer
        const size_t overflowBit = mFrameCountP2 << 1;
        const size_t mask = overflowBit - 1;
        int32_t newFront = (front & ~mask) | (flush & mask);
        ssize_t filled = audio_utils::safe_sub_overflow(rear, newFront);
        if (filled >= (ssize_t)overflowBit) {
            // front and rear offsets span the overflow bit of the p2 mask
            // so rebasing newFront on the front offset is off by the overflow bit.
            // adjust newFront to match rear offset.
            ALOGV("flush wrap: filled %zx >= overflowBit %zx", filled, overflowBit);
            newFront += overflowBit;
            filled -= overflowBit;
        }
        // Rather than shutting down on a corrupt flush, just treat it as a full flush
        if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
            ALOGE("mFlush %#x -> %#x, front %#x, rear %#x, mask %#x, newFront %#x, "
                    "filled %zd=%#x",
                    mFlush, flush, front, rear,
                    (unsigned)mask, newFront, filled, (unsigned)filled);
            newFront = rear;
        }
        mFlush = flush;
        android_atomic_release_store(newFront, &cblk->u.mStreaming.mFront);
        // There is no danger from a false positive, so err on the side of caution
        if (true /*front != newFront*/) {
            int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
            if (!(old & CBLK_FUTEX_WAKE)) {
                (void) syscall(__NR_futex, &cblk->mFutex,
                        mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE, 1);
            }
        }
        mFlushed += (newFront - front) & mask;
    }
}

__attribute__((no_sanitize("integer")))
int32_t AudioTrackServerProxy::getRear() const
{
    const int32_t stop = android_atomic_acquire_load(&mCblk->u.mStreaming.mStop);
    const int32_t rear = android_atomic_acquire_load(&mCblk->u.mStreaming.mRear);
    const int32_t stopLast = mStopLast.load(std::memory_order_acquire);
    if (stop != stopLast) {
        const int32_t front = mCblk->u.mStreaming.mFront;
        const size_t overflowBit = mFrameCountP2 << 1;
        const size_t mask = overflowBit - 1;
        int32_t newRear = (rear & ~mask) | (stop & mask);
        ssize_t filled = audio_utils::safe_sub_overflow(newRear, front);
        // overflowBit is unsigned, so cast to signed for comparison.
        if (filled >= (ssize_t)overflowBit) {
            // front and rear offsets span the overflow bit of the p2 mask
            // so rebasing newRear on the rear offset is off by the overflow bit.
            ALOGV("stop wrap: filled %zx >= overflowBit %zx", filled, overflowBit);
            newRear -= overflowBit;
            filled -= overflowBit;
        }
        if (0 <= filled && (size_t) filled <= mFrameCount) {
            // we're stopped, return the stop level as newRear
            return newRear;
        }

        // A corrupt stop. Log error and ignore.
        ALOGE("mStopLast %#x -> stop %#x, front %#x, rear %#x, mask %#x, newRear %#x, "
                "filled %zd=%#x",
                stopLast, stop, front, rear,
                (unsigned)mask, newRear, filled, (unsigned)filled);
        // Don't reset mStopLast as this is const.
    }
    return rear;
}

void AudioTrackServerProxy::start()
{
    mStopLast = android_atomic_acquire_load(&mCblk->u.mStreaming.mStop);
}

__attribute__((no_sanitize("integer")))
status_t ServerProxy::obtainBuffer(Buffer* buffer, bool ackFlush)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL || buffer->mFrameCount == 0,
            "%s: null or zero frame buffer, buffer:%p", __func__, buffer);
    if (mIsShutdown) {
        goto no_init;
    }
    {
    audio_track_cblk_t* cblk = mCblk;
    // compute number of frames available to write (AudioTrack) or read (AudioRecord),
    // or use previous cached value from framesReady(), with added barrier if it omits.
    int32_t front;
    int32_t rear;
    // See notes on barriers at ClientProxy::obtainBuffer()
    if (mIsOut) {
        flushBufferIfNeeded(); // might modify mFront
        rear = getRear();
        front = cblk->u.mStreaming.mFront;
    } else {
        front = android_atomic_acquire_load(&cblk->u.mStreaming.mFront);
        rear = cblk->u.mStreaming.mRear;
    }
    ssize_t filled = audio_utils::safe_sub_overflow(rear, front);
    // pipe should not already be overfull
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        ALOGE("Shared memory control block is corrupt (filled=%zd, mFrameCount=%zu); shutting down",
                filled, mFrameCount);
        mIsShutdown = true;
    }
    if (mIsShutdown) {
        goto no_init;
    }
    // don't allow filling pipe beyond the nominal size
    size_t availToServer;
    if (mIsOut) {
        availToServer = filled;
        mAvailToClient = mFrameCount - filled;
    } else {
        availToServer = mFrameCount - filled;
        mAvailToClient = filled;
    }
    // 'availToServer' may be non-contiguous, so return only the first contiguous chunk
    size_t part1;
    if (mIsOut) {
        front &= mFrameCountP2 - 1;
        part1 = mFrameCountP2 - front;
    } else {
        rear &= mFrameCountP2 - 1;
        part1 = mFrameCountP2 - rear;
    }
    if (part1 > availToServer) {
        part1 = availToServer;
    }
    size_t ask = buffer->mFrameCount;
    if (part1 > ask) {
        part1 = ask;
    }
    // is assignment redundant in some cases?
    buffer->mFrameCount = part1;
    buffer->mRaw = part1 > 0 ?
            &((char *) mBuffers)[(mIsOut ? front : rear) * mFrameSize] : NULL;
    buffer->mNonContig = availToServer - part1;
    // After flush(), allow releaseBuffer() on a previously obtained buffer;
    // see "Acknowledge any pending flush()" in audioflinger/Tracks.cpp.
    if (!ackFlush) {
        mUnreleased = part1;
    }
    return part1 > 0 ? NO_ERROR : WOULD_BLOCK;
    }
no_init:
    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
    mUnreleased = 0;
    return NO_INIT;
}

__attribute__((no_sanitize("integer")))
void ServerProxy::releaseBuffer(Buffer* buffer)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL);
    size_t stepCount = buffer->mFrameCount;
    if (stepCount == 0 || mIsShutdown) {
        // prevent accidental re-use of buffer
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount),
            "%s: mUnreleased out of range, "
            "!(stepCount:%zu <= mUnreleased:%zu <= mFrameCount:%zu)",
            __func__, stepCount, mUnreleased, mFrameCount);
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    if (mIsOut) {
        int32_t front = cblk->u.mStreaming.mFront;
        android_atomic_release_store(stepCount + front, &cblk->u.mStreaming.mFront);
    } else {
        int32_t rear = cblk->u.mStreaming.mRear;
        android_atomic_release_store(stepCount + rear, &cblk->u.mStreaming.mRear);
    }

    cblk->mServer += stepCount;
    mReleased += stepCount;

    size_t half = mFrameCount / 2;
    if (half == 0) {
        half = 1;
    }
    size_t minimum = (size_t) cblk->mMinimum;
    if (minimum == 0) {
        minimum = mIsOut ? half : 1;
    } else if (minimum > half) {
        minimum = half;
    }
    // FIXME AudioRecord wakeup needs to be optimized; it currently wakes up client every time
    if (!mIsOut || (mAvailToClient + stepCount >= minimum)) {
        ALOGV("mAvailToClient=%zu stepCount=%zu minimum=%zu", mAvailToClient, stepCount, minimum);
        int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE, 1);
        }
    }

    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
}

// ---------------------------------------------------------------------------

__attribute__((no_sanitize("integer")))
size_t AudioTrackServerProxy::framesReady()
{
    LOG_ALWAYS_FATAL_IF(!mIsOut);

    if (mIsShutdown) {
        return 0;
    }
    audio_track_cblk_t* cblk = mCblk;

    int32_t flush = cblk->u.mStreaming.mFlush;
    if (flush != mFlush) {
        // FIXME should return an accurate value, but over-estimate is better than under-estimate
        return mFrameCount;
    }
    const int32_t rear = getRear();
    ssize_t filled = audio_utils::safe_sub_overflow(rear, cblk->u.mStreaming.mFront);
    // pipe should not already be overfull
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        ALOGE("Shared memory control block is corrupt (filled=%zd, mFrameCount=%zu); shutting down",
                filled, mFrameCount);
        mIsShutdown = true;
        return 0;
    }
    //  cache this value for later use by obtainBuffer(), with added barrier
    //  and racy if called by normal mixer thread
    // ignores flush(), so framesReady() may report a larger mFrameCount than obtainBuffer()
    return filled;
}

__attribute__((no_sanitize("integer")))
size_t AudioTrackServerProxy::framesReadySafe() const
{
    if (mIsShutdown) {
        return 0;
    }
    const audio_track_cblk_t* cblk = mCblk;
    const int32_t flush = android_atomic_acquire_load(&cblk->u.mStreaming.mFlush);
    if (flush != mFlush) {
        return mFrameCount;
    }
    const int32_t rear = getRear();
    const ssize_t filled = audio_utils::safe_sub_overflow(rear, cblk->u.mStreaming.mFront);
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        return 0; // error condition, silently return 0.
    }
    return filled;
}

bool  AudioTrackServerProxy::setStreamEndDone() {
    audio_track_cblk_t* cblk = mCblk;
    bool old =
            (android_atomic_or(CBLK_STREAM_END_DONE, &cblk->mFlags) & CBLK_STREAM_END_DONE) != 0;
    if (!old) {
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
    return old;
}

__attribute__((no_sanitize("integer")))
void AudioTrackServerProxy::tallyUnderrunFrames(uint32_t frameCount)
{
    audio_track_cblk_t* cblk = mCblk;
    if (frameCount > 0) {
        cblk->u.mStreaming.mUnderrunFrames += frameCount;

        if (!mUnderrunning) { // start of underrun?
            mUnderrunCount++;
            cblk->u.mStreaming.mUnderrunCount = mUnderrunCount;
            mUnderrunning = true;
            ALOGV("tallyUnderrunFrames(%3u) at uf = %u, bump mUnderrunCount = %u",
                frameCount, cblk->u.mStreaming.mUnderrunFrames, mUnderrunCount);
        }

        // FIXME also wake futex so that underrun is noticed more quickly
        (void) android_atomic_or(CBLK_UNDERRUN, &cblk->mFlags);
    } else {
        ALOGV_IF(mUnderrunning,
            "tallyUnderrunFrames(%3u) at uf = %u, underrun finished",
            frameCount, cblk->u.mStreaming.mUnderrunFrames);
        mUnderrunning = false; // so we can detect the next edge
    }
}

AudioPlaybackRate AudioTrackServerProxy::getPlaybackRate()
{   // do not call from multiple threads without holding lock
    mPlaybackRateObserver.poll(mPlaybackRate);
    return mPlaybackRate;
}

// ---------------------------------------------------------------------------

StaticAudioTrackServerProxy::StaticAudioTrackServerProxy(audio_track_cblk_t* cblk, void *buffers,
        size_t frameCount, size_t frameSize)
    : AudioTrackServerProxy(cblk, buffers, frameCount, frameSize),
      mObserver(&cblk->u.mStatic.mSingleStateQueue),
      mPosLoopMutator(&cblk->u.mStatic.mPosLoopQueue),
      mFramesReadySafe(frameCount), mFramesReady(frameCount),
      mFramesReadyIsCalledByMultipleThreads(false)
{
    memset(&mState, 0, sizeof(mState));
}

void StaticAudioTrackServerProxy::framesReadyIsCalledByMultipleThreads()
{
    mFramesReadyIsCalledByMultipleThreads = true;
}

size_t StaticAudioTrackServerProxy::framesReady()
{
    // Can't call pollPosition() from multiple threads.
    if (!mFramesReadyIsCalledByMultipleThreads) {
        (void) pollPosition();
    }
    return mFramesReadySafe;
}

size_t StaticAudioTrackServerProxy::framesReadySafe() const
{
    return mFramesReadySafe;
}

status_t StaticAudioTrackServerProxy::updateStateWithLoop(
        StaticAudioTrackState *localState, const StaticAudioTrackState &update) const
{
    if (localState->mLoopSequence != update.mLoopSequence) {
        bool valid = false;
        const size_t loopStart = update.mLoopStart;
        const size_t loopEnd = update.mLoopEnd;
        size_t position = localState->mPosition;
        if (update.mLoopCount == 0) {
            valid = true;
        } else if (update.mLoopCount >= -1) {
            if (loopStart < loopEnd && loopEnd <= mFrameCount &&
                    loopEnd - loopStart >= MIN_LOOP) {
                // If the current position is greater than the end of the loop
                // we "wrap" to the loop start. This might cause an audible pop.
                if (position >= loopEnd) {
                    position = loopStart;
                }
                valid = true;
            }
        }
        if (!valid || position > mFrameCount) {
            return NO_INIT;
        }
        localState->mPosition = position;
        localState->mLoopCount = update.mLoopCount;
        localState->mLoopEnd = loopEnd;
        localState->mLoopStart = loopStart;
        localState->mLoopSequence = update.mLoopSequence;
    }
    return OK;
}

status_t StaticAudioTrackServerProxy::updateStateWithPosition(
        StaticAudioTrackState *localState, const StaticAudioTrackState &update) const
{
    if (localState->mPositionSequence != update.mPositionSequence) {
        if (update.mPosition > mFrameCount) {
            return NO_INIT;
        } else if (localState->mLoopCount != 0 && update.mPosition >= localState->mLoopEnd) {
            localState->mLoopCount = 0; // disable loop count if position is beyond loop end.
        }
        localState->mPosition = update.mPosition;
        localState->mPositionSequence = update.mPositionSequence;
    }
    return OK;
}

ssize_t StaticAudioTrackServerProxy::pollPosition()
{
    StaticAudioTrackState state;
    if (mObserver.poll(state)) {
        StaticAudioTrackState trystate = mState;
        bool result;
        const int32_t diffSeq = (int32_t) state.mLoopSequence - (int32_t) state.mPositionSequence;

        if (diffSeq < 0) {
            result = updateStateWithLoop(&trystate, state) == OK &&
                    updateStateWithPosition(&trystate, state) == OK;
        } else {
            result = updateStateWithPosition(&trystate, state) == OK &&
                    updateStateWithLoop(&trystate, state) == OK;
        }
        if (!result) {
            mObserver.done();
            // caution: no update occurs so server state will be inconsistent with client state.
            ALOGE("%s client pushed an invalid state, shutting down", __func__);
            mIsShutdown = true;
            return (ssize_t) NO_INIT;
        }
        mState = trystate;
        if (mState.mLoopCount == -1) {
            mFramesReady = INT64_MAX;
        } else if (mState.mLoopCount == 0) {
            mFramesReady = mFrameCount - mState.mPosition;
        } else if (mState.mLoopCount > 0) {
            // TODO: Later consider fixing overflow, but does not seem needed now
            // as will not overflow if loopStart and loopEnd are Java "ints".
            mFramesReady = int64_t(mState.mLoopCount) * (mState.mLoopEnd - mState.mLoopStart)
                    + mFrameCount - mState.mPosition;
        }
        mFramesReadySafe = clampToSize(mFramesReady);
        // This may overflow, but client is not supposed to rely on it
        StaticAudioTrackPosLoop posLoop;

        posLoop.mLoopCount = (int32_t) mState.mLoopCount;
        posLoop.mBufferPosition = (uint32_t) mState.mPosition;
        mPosLoopMutator.push(posLoop);
        mObserver.done(); // safe to read mStatic variables.
    }
    return (ssize_t) mState.mPosition;
}

__attribute__((no_sanitize("integer")))
status_t StaticAudioTrackServerProxy::obtainBuffer(Buffer* buffer, bool ackFlush)
{
    if (mIsShutdown) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
        return NO_INIT;
    }
    ssize_t positionOrStatus = pollPosition();
    if (positionOrStatus < 0) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
        return (status_t) positionOrStatus;
    }
    size_t position = (size_t) positionOrStatus;
    size_t end = mState.mLoopCount != 0 ? mState.mLoopEnd : mFrameCount;
    size_t avail;
    if (position < end) {
        avail = end - position;
        size_t wanted = buffer->mFrameCount;
        if (avail < wanted) {
            buffer->mFrameCount = avail;
        } else {
            avail = wanted;
        }
        buffer->mRaw = &((char *) mBuffers)[position * mFrameSize];
    } else {
        avail = 0;
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
    }
    // As mFramesReady is the total remaining frames in the static audio track,
    // it is always larger or equal to avail.
    LOG_ALWAYS_FATAL_IF(mFramesReady < (int64_t) avail,
            "%s: mFramesReady out of range, mFramesReady:%lld < avail:%zu",
            __func__, (long long)mFramesReady, avail);
    buffer->mNonContig = mFramesReady == INT64_MAX ? SIZE_MAX : clampToSize(mFramesReady - avail);
    if (!ackFlush) {
        mUnreleased = avail;
    }
    return NO_ERROR;
}

__attribute__((no_sanitize("integer")))
void StaticAudioTrackServerProxy::releaseBuffer(Buffer* buffer)
{
    size_t stepCount = buffer->mFrameCount;
    LOG_ALWAYS_FATAL_IF(!((int64_t) stepCount <= mFramesReady),
            "%s: stepCount out of range, "
            "!(stepCount:%zu <= mFramesReady:%lld)",
            __func__, stepCount, (long long)mFramesReady);
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased),
            "%s: stepCount out of range, "
            "!(stepCount:%zu <= mUnreleased:%zu)",
            __func__, stepCount, mUnreleased);
    if (stepCount == 0) {
        // prevent accidental re-use of buffer
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    size_t position = mState.mPosition;
    size_t newPosition = position + stepCount;
    int32_t setFlags = 0;
    if (!(position <= newPosition && newPosition <= mFrameCount)) {
        ALOGW("%s newPosition %zu outside [%zu, %zu]", __func__, newPosition, position,
                mFrameCount);
        newPosition = mFrameCount;
    } else if (mState.mLoopCount != 0 && newPosition == mState.mLoopEnd) {
        newPosition = mState.mLoopStart;
        if (mState.mLoopCount == -1 || --mState.mLoopCount != 0) {
            setFlags = CBLK_LOOP_CYCLE;
        } else {
            setFlags = CBLK_LOOP_FINAL;
        }
    }
    if (newPosition == mFrameCount) {
        setFlags |= CBLK_BUFFER_END;
    }
    mState.mPosition = newPosition;
    if (mFramesReady != INT64_MAX) {
        mFramesReady -= stepCount;
    }
    mFramesReadySafe = clampToSize(mFramesReady);

    cblk->mServer += stepCount;
    mReleased += stepCount;

    // This may overflow, but client is not supposed to rely on it
    StaticAudioTrackPosLoop posLoop;
    posLoop.mBufferPosition = mState.mPosition;
    posLoop.mLoopCount = mState.mLoopCount;
    mPosLoopMutator.push(posLoop);
    if (setFlags != 0) {
        (void) android_atomic_or(setFlags, &cblk->mFlags);
        // this would be a good place to wake a futex
    }

    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
}

void StaticAudioTrackServerProxy::tallyUnderrunFrames(uint32_t frameCount)
{
    // Unlike AudioTrackServerProxy::tallyUnderrunFrames() used for streaming tracks,
    // we don't have a location to count underrun frames.  The underrun frame counter
    // only exists in AudioTrackSharedStreaming.  Fortunately, underruns are not
    // possible for static buffer tracks other than at end of buffer, so this is not a loss.

    // FIXME also wake futex so that underrun is noticed more quickly
    if (frameCount > 0) {
        (void) android_atomic_or(CBLK_UNDERRUN, &mCblk->mFlags);
    }
}

int32_t StaticAudioTrackServerProxy::getRear() const
{
    LOG_ALWAYS_FATAL("getRear() not permitted for static tracks");
    return 0;
}

__attribute__((no_sanitize("integer")))
size_t AudioRecordServerProxy::framesReadySafe() const
{
    if (mIsShutdown) {
        return 0;
    }
    const int32_t front = android_atomic_acquire_load(&mCblk->u.mStreaming.mFront);
    const int32_t rear = mCblk->u.mStreaming.mRear;
    const ssize_t filled = audio_utils::safe_sub_overflow(rear, front);
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        return 0; // error condition, silently return 0.
    }
    return filled;
}

// ---------------------------------------------------------------------------

}   // namespace android
