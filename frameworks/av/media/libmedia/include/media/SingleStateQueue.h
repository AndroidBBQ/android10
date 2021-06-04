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

#ifndef SINGLE_STATE_QUEUE_H
#define SINGLE_STATE_QUEUE_H

// Non-blocking single element state queue, or
// Non-blocking single-reader / single-writer multi-word atomic load / store

#include <stdint.h>
#include <cutils/atomic.h>

namespace android {

template<typename T> class SingleStateQueue {

public:

    class Mutator;
    class Observer;

    enum SSQ_STATUS {
        SSQ_PENDING, /* = 0 */
        SSQ_READ,
        SSQ_DONE,
    };

    struct Shared {
        // needs to be part of a union so don't define constructor or destructor

        friend class Mutator;
        friend class Observer;

private:
        void                init() { mAck = 0; mSequence = 0; }

        volatile int32_t    mAck;
        volatile int32_t    mSequence;
        T                   mValue;
    };

    class Mutator {
    public:
        Mutator(Shared *shared)
            : mSequence(0), mShared(shared)
        {
            // exactly one of Mutator and Observer must initialize, currently it is Observer
            // shared->init();
        }

        // push new value onto state queue, overwriting previous value;
        // returns a sequence number which can be used with ack()
        int32_t push(const T& value)
        {
            Shared *shared = mShared;
            int32_t sequence = mSequence;
            sequence++;
            android_atomic_acquire_store(sequence, &shared->mSequence);
            shared->mValue = value;
            sequence++;
            android_atomic_release_store(sequence, &shared->mSequence);
            mSequence = sequence;
            // consider signalling a futex here, if we know that observer is waiting
            return sequence;
        }

        // returns the status of the last state push.  This may be a stale value.
        //
        // SSQ_PENDING, or 0, means it has not been observed
        // SSQ_READ means it has been read
        // SSQ_DONE means it has been acted upon, after Observer::done() is called
        enum SSQ_STATUS ack() const
        {
            // in the case of SSQ_DONE, prevent any subtle data-races of subsequent reads
            // being performed (out-of-order) before the ack read, should the caller be
            // depending on sequentiality of reads.
            const int32_t ack = android_atomic_acquire_load(&mShared->mAck);
            return ack - mSequence & ~1 ? SSQ_PENDING /* seq differ */ :
                    ack & 1 ? SSQ_DONE : SSQ_READ;
        }

        // return true if a push with specified sequence number or later has been observed
        bool ack(int32_t sequence) const
        {
            // this relies on 2's complement rollover to detect an ancient sequence number
            return mShared->mAck - sequence >= 0;
        }

        // returns the last value written (or the contents of the shared buffer after initialization
        // if no value was written).
        T last() const
        {   // assume no sequence check required - we are the writer.
            return mShared->mValue;
        }

    private:
        int32_t     mSequence;
        Shared * const mShared;
    };

    class Observer {
    public:
        Observer(Shared *shared)
            : mSequence(0), mSeed(1), mShared(shared)
        {
            // exactly one of Mutator and Observer must initialize, currently it is Observer
            shared->init();
        }

        // return true if value has changed
        bool poll(T& value)
        {
            Shared *shared = mShared;
            int32_t before = shared->mSequence;
            if (before == mSequence) {
                return false;
            }
            for (int tries = 0; ; ) {
                const int MAX_TRIES = 5;
                if (before & 1) {
                    if (++tries >= MAX_TRIES) {
                        return false;
                    }
                    before = shared->mSequence;
                } else {
                    android_memory_barrier();
                    T temp = shared->mValue;
                    int32_t after = android_atomic_release_load(&shared->mSequence);
                    if (after == before) {
                        value = temp;
                        shared->mAck = before;
                        mSequence = before; // mSequence is even after poll success
                        return true;
                    }
                    if (++tries >= MAX_TRIES) {
                        return false;
                    }
                    before = after;
                }
            }
        }

        // (optional) used to indicate to the Mutator that the state that has been polled
        // has also been acted upon.
        void done()
        {
            const int32_t ack = mShared->mAck + 1;
            // ensure all previous writes have been performed.
            android_atomic_release_store(ack, &mShared->mAck); // mSequence is odd after "done"
        }

    private:
        int32_t     mSequence;
        int         mSeed;  // for PRNG
        Shared * const mShared;
    };

#if 0
    SingleStateQueue(void /*Shared*/ *shared);
    /*virtual*/ ~SingleStateQueue() { }

    static size_t size() { return sizeof(Shared); }
#endif

};

}   // namespace android

#endif  // SINGLE_STATE_QUEUE_H
