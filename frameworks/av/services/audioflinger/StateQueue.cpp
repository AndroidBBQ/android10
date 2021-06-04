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

#define LOG_TAG "StateQueue"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#include <time.h>
#include <cutils/atomic.h>
#include <utils/Log.h>
#include "StateQueue.h"

namespace android {

#ifdef STATE_QUEUE_DUMP
void StateQueueObserverDump::dump(int fd)
{
    dprintf(fd, "State queue observer: stateChanges=%u\n", mStateChanges);
}

void StateQueueMutatorDump::dump(int fd)
{
    dprintf(fd, "State queue mutator: pushDirty=%u pushAck=%u blockedSequence=%u\n",
            mPushDirty, mPushAck, mBlockedSequence);
}
#endif

// Constructor and destructor

template<typename T> StateQueue<T>::StateQueue() :
    mAck(NULL), mCurrent(NULL),
    mMutating(&mStates[0]), mExpecting(NULL),
    mInMutation(false), mIsDirty(false), mIsInitialized(false)
#ifdef STATE_QUEUE_DUMP
    , mObserverDump(&mObserverDummyDump), mMutatorDump(&mMutatorDummyDump)
#endif
{
    atomic_init(&mNext, static_cast<uintptr_t>(0));
}

template<typename T> StateQueue<T>::~StateQueue()
{
}

// Observer APIs

template<typename T> const T* StateQueue<T>::poll()
{
    const T *next = (const T *) atomic_load_explicit(&mNext, memory_order_acquire);

    if (next != mCurrent) {
        mAck = next;    // no additional barrier needed
        mCurrent = next;
#ifdef STATE_QUEUE_DUMP
        mObserverDump->mStateChanges++;
#endif
    }
    return next;
}

// Mutator APIs

template<typename T> T* StateQueue<T>::begin()
{
    ALOG_ASSERT(!mInMutation, "begin() called when in a mutation");
    mInMutation = true;
    return mMutating;
}

template<typename T> void StateQueue<T>::end(bool didModify)
{
    ALOG_ASSERT(mInMutation, "end() called when not in a mutation");
    ALOG_ASSERT(mIsInitialized || didModify, "first end() must modify for initialization");
    if (didModify) {
        mIsDirty = true;
        mIsInitialized = true;
    }
    mInMutation = false;
}

template<typename T> bool StateQueue<T>::push(StateQueue<T>::block_t block)
{
#define PUSH_BLOCK_ACK_NS    3000000L   // 3 ms: time between checks for ack in push()
                                        //       FIXME should be configurable
    static const struct timespec req = {0, PUSH_BLOCK_ACK_NS};

    ALOG_ASSERT(!mInMutation, "push() called when in a mutation");

#ifdef STATE_QUEUE_DUMP
    if (block == BLOCK_UNTIL_ACKED) {
        mMutatorDump->mPushAck++;
    }
#endif

    if (mIsDirty) {

#ifdef STATE_QUEUE_DUMP
        mMutatorDump->mPushDirty++;
#endif

        // wait for prior push to be acknowledged
        if (mExpecting != NULL) {
#ifdef STATE_QUEUE_DUMP
            unsigned count = 0;
#endif
            for (;;) {
                const T *ack = (const T *) mAck;    // no additional barrier needed
                if (ack == mExpecting) {
                    // unnecessary as we're about to rewrite
                    //mExpecting = NULL;
                    break;
                }
                if (block == BLOCK_NEVER) {
                    return false;
                }
#ifdef STATE_QUEUE_DUMP
                if (count == 1) {
                    mMutatorDump->mBlockedSequence++;
                }
                ++count;
#endif
                nanosleep(&req, NULL);
            }
#ifdef STATE_QUEUE_DUMP
            if (count > 1) {
                mMutatorDump->mBlockedSequence++;
            }
#endif
        }

        // publish
        atomic_store_explicit(&mNext, (uintptr_t)mMutating, memory_order_release);
        mExpecting = mMutating;

        // copy with circular wraparound
        if (++mMutating >= &mStates[kN]) {
            mMutating = &mStates[0];
        }
        *mMutating = *mExpecting;
        mIsDirty = false;

    }

    // optionally wait for this push or a prior push to be acknowledged
    if (block == BLOCK_UNTIL_ACKED) {
        if (mExpecting != NULL) {
#ifdef STATE_QUEUE_DUMP
            unsigned count = 0;
#endif
            for (;;) {
                const T *ack = (const T *) mAck;    // no additional barrier needed
                if (ack == mExpecting) {
                    mExpecting = NULL;
                    break;
                }
#ifdef STATE_QUEUE_DUMP
                if (count == 1) {
                    mMutatorDump->mBlockedSequence++;
                }
                ++count;
#endif
                nanosleep(&req, NULL);
            }
#ifdef STATE_QUEUE_DUMP
            if (count > 1) {
                mMutatorDump->mBlockedSequence++;
            }
#endif
        }
    }

    return true;
}

}   // namespace android

// hack for gcc
#ifdef STATE_QUEUE_INSTANTIATIONS
#include STATE_QUEUE_INSTANTIATIONS
#endif
