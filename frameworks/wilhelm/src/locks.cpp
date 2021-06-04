/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "sles_allinclusive.h"


// Use this macro to validate a pthread_t before passing it into pthread_gettid_np.
// One of the common reasons for deadlock is trying to lock a mutex for an object
// which has been destroyed (which does memset to 0x00 or 0x55 as the final step).
// To avoid crashing with a SIGSEGV right before we're about to log a deadlock warning,
// we check that the pthread_t is probably valid.  Note that it is theoretically
// possible for something to look like a valid pthread_t but not actually be valid.
// So we might still crash, but only in the case where a deadlock was imminent anyway.
#define LIKELY_VALID(ptr) (((ptr) != (pthread_t) 0) && ((((size_t) (ptr)) & 3) == 0))


/** \brief Exclusively lock an object */

#ifdef USE_DEBUG

void init_time_spec(timespec* ts, long delta) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_nsec += delta;

    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000L;
    }
}

void object_lock_exclusive_(IObject *thiz, const char *file, int line)
{
    int ok;
    ok = pthread_mutex_trylock(&thiz->mMutex);
    if (0 != ok) {
        // not android_atomic_acquire_load because we don't care about relative load/load ordering
        int32_t oldGeneration = thiz->mGeneration;
        // wait up to a total of 250 ms
        static const long nanoBackoffs[] = {
            10 * 1000000, 20 * 1000000, 30 * 1000000, 40 * 1000000, 50 * 1000000, 100 * 1000000};
        unsigned i = 0;
        timespec ts;
        memset(&ts, 0, sizeof(timespec));
        for (;;) {
            init_time_spec(&ts, nanoBackoffs[i]);
            ok = pthread_mutex_timedlock(&thiz->mMutex, &ts);
            if (0 == ok) {
                break;
            }
            if (EBUSY == ok) {
                // this is the expected return value for timeout, and will be handled below
            } else if (EDEADLK == ok) {
                // we don't use the kind of mutex that can return this error, but just in case
                SL_LOGE("%s:%d: recursive lock detected", file, line);
            } else {
                // some other return value
                SL_LOGE("%s:%d: pthread_mutex_lock_timeout_np returned %d", file, line, ok);
            }
            // is anyone else making forward progress?
            int32_t newGeneration = thiz->mGeneration;
            if (newGeneration != oldGeneration) {
                // if we ever see forward progress then lock without timeout (more efficient)
                goto forward_progress;
            }
            // no, then continue trying to lock but with increasing timeouts
            if (++i >= (sizeof(nanoBackoffs) / sizeof(nanoBackoffs[0]))) {
                // the extra block avoids a C++ compiler error about goto past initialization
                {
                    pthread_t me = pthread_self();
                    pthread_t owner = thiz->mOwner;
                    // unlikely, but this could result in a memory fault if owner is corrupt
                    pid_t ownerTid = LIKELY_VALID(owner) ? pthread_gettid_np(owner) : -1;
                    SL_LOGW("%s:%d: pthread %p (tid %d) sees object %p was locked by pthread %p"
                            " (tid %d) at %s:%d\n", file, line, *(void **)&me, gettid(), thiz,
                            *(void **)&owner, ownerTid, thiz->mFile, thiz->mLine);
                }
forward_progress:
                // attempt one more time without timeout; maybe this time we will be successful
                ok = pthread_mutex_lock(&thiz->mMutex);
                assert(0 == ok);
                break;
            }
        }
    }
    // here if mutex was successfully locked
    pthread_t zero;
    memset(&zero, 0, sizeof(pthread_t));
    if (0 != memcmp(&zero, &thiz->mOwner, sizeof(pthread_t))) {
        pthread_t me = pthread_self();
        pthread_t owner = thiz->mOwner;
        pid_t ownerTid = LIKELY_VALID(owner) ? pthread_gettid_np(owner) : -1;
        if (pthread_equal(pthread_self(), owner)) {
            SL_LOGE("%s:%d: pthread %p (tid %d) sees object %p was recursively locked by pthread"
                    " %p (tid %d) at %s:%d\n", file, line, *(void **)&me, gettid(), thiz,
                    *(void **)&owner, ownerTid, thiz->mFile, thiz->mLine);
        } else {
            SL_LOGE("%s:%d: pthread %p (tid %d) sees object %p was left unlocked in unexpected"
                    " state by pthread %p (tid %d) at %s:%d\n", file, line, *(void **)&me, gettid(),
                    thiz, *(void **)&owner, ownerTid, thiz->mFile, thiz->mLine);
        }
        assert(false);
    }
    thiz->mOwner = pthread_self();
    thiz->mFile = file;
    thiz->mLine = line;
    // not android_atomic_inc because we are already holding a mutex
    ++thiz->mGeneration;
}
#else
void object_lock_exclusive(IObject *thiz)
{
    int ok;
    ok = pthread_mutex_lock(&thiz->mMutex);
    assert(0 == ok);
}
#endif


/** \brief Exclusively unlock an object and do not report any updates */

#ifdef USE_DEBUG
void object_unlock_exclusive_(IObject *thiz, const char *file, int line)
{
    assert(pthread_equal(pthread_self(), thiz->mOwner));
    assert(NULL != thiz->mFile);
    assert(0 != thiz->mLine);
    memset(&thiz->mOwner, 0, sizeof(pthread_t));
    thiz->mFile = file;
    thiz->mLine = line;
    int ok;
    ok = pthread_mutex_unlock(&thiz->mMutex);
    assert(0 == ok);
}
#else
void object_unlock_exclusive(IObject *thiz)
{
    int ok;
    ok = pthread_mutex_unlock(&thiz->mMutex);
    assert(0 == ok);
}
#endif


/** \brief Exclusively unlock an object and report updates to the specified bit-mask of
 *  attributes
 */

#ifdef USE_DEBUG
void object_unlock_exclusive_attributes_(IObject *thiz, unsigned attributes,
    const char *file, int line)
#else
void object_unlock_exclusive_attributes(IObject *thiz, unsigned attributes)
#endif
{

#ifdef USE_DEBUG
    assert(pthread_equal(pthread_self(), thiz->mOwner));
    assert(NULL != thiz->mFile);
    assert(0 != thiz->mLine);
#endif

    int ok;

    // make SL object IDs be contiguous with XA object IDs
    SLuint32 objectID = IObjectToObjectID(thiz);
    SLuint32 index = objectID;
    if ((XA_OBJECTID_ENGINE <= index) && (index <= XA_OBJECTID_CAMERADEVICE)) {
        ;
    } else if ((SL_OBJECTID_ENGINE <= index) && (index <= SL_OBJECTID_METADATAEXTRACTOR)) {
        index -= SL_OBJECTID_ENGINE - XA_OBJECTID_CAMERADEVICE - 1;
    } else {
        assert(false);
        index = 0;
    }

    // first synchronously handle updates to attributes here, while object is still locked.
    // This appears to be a loop, but actually typically runs through the loop only once.
    unsigned asynchronous = attributes;
    while (attributes) {
        // this sequence is carefully crafted to be O(1); tread carefully when making changes
        unsigned bit = ctz(attributes);
        // ATTR_INDEX_MAX == next bit position after the last attribute
        assert(ATTR_INDEX_MAX > bit);
        // compute the entry in the handler table using object ID and bit number
        AttributeHandler handler = handlerTable[index][bit];
        if (NULL != handler) {
            asynchronous &= ~(*handler)(thiz);
        }
        attributes &= ~(1 << bit);
    }

    // any remaining attributes are handled asynchronously in the sync thread
    if (asynchronous) {
        unsigned oldAttributesMask = thiz->mAttributesMask;
        thiz->mAttributesMask = oldAttributesMask | asynchronous;
        if (oldAttributesMask) {
            asynchronous = ATTR_NONE;
        }
    }

#ifdef ANDROID
    // FIXME hack to safely handle a post-unlock PrefetchStatus callback and/or AudioTrack::start()
    slPrefetchCallback prefetchCallback = NULL;
    void *prefetchContext = NULL;
    SLuint32 prefetchEvents = SL_PREFETCHEVENT_NONE;
    android::sp<android::AudioTrack> audioTrack;
    if (SL_OBJECTID_AUDIOPLAYER == objectID) {
        CAudioPlayer *ap = (CAudioPlayer *) thiz;
        prefetchCallback = ap->mPrefetchStatus.mDeferredPrefetchCallback;
        prefetchContext  = ap->mPrefetchStatus.mDeferredPrefetchContext;
        prefetchEvents   = ap->mPrefetchStatus.mDeferredPrefetchEvents;
        ap->mPrefetchStatus.mDeferredPrefetchCallback = NULL;
        // clearing these next two fields is not required, but avoids stale data during debugging
        ap->mPrefetchStatus.mDeferredPrefetchContext  = NULL;
        ap->mPrefetchStatus.mDeferredPrefetchEvents   = SL_PREFETCHEVENT_NONE;
        if (ap->mDeferredStart) {
            audioTrack = ap->mTrackPlayer->mAudioTrack;
            ap->mDeferredStart = false;
        }
    }
#endif

#ifdef USE_DEBUG
    memset(&thiz->mOwner, 0, sizeof(pthread_t));
    thiz->mFile = file;
    thiz->mLine = line;
#endif
    ok = pthread_mutex_unlock(&thiz->mMutex);
    assert(0 == ok);

#ifdef ANDROID
    // FIXME call the prefetch status callback while not holding the mutex on AudioPlayer
    if (NULL != prefetchCallback) {
        // note these are synchronous by the application's thread as it is about to return from API
        assert(prefetchEvents != SL_PREFETCHEVENT_NONE);
        CAudioPlayer *ap = (CAudioPlayer *) thiz;
        // spec requires separate callbacks for each event
        if (SL_PREFETCHEVENT_STATUSCHANGE & prefetchEvents) {
            (*prefetchCallback)(&ap->mPrefetchStatus.mItf, prefetchContext,
                    SL_PREFETCHEVENT_STATUSCHANGE);
        }
        if (SL_PREFETCHEVENT_FILLLEVELCHANGE & prefetchEvents) {
            (*prefetchCallback)(&ap->mPrefetchStatus.mItf, prefetchContext,
                    SL_PREFETCHEVENT_FILLLEVELCHANGE);
        }
    }

    // call AudioTrack::start() while not holding the mutex on AudioPlayer
    if (audioTrack != 0) {
        audioTrack->start();
        audioTrack.clear();
    }
#endif

    // first update to this interface since previous sync
    if (ATTR_NONE != asynchronous) {
        unsigned id = thiz->mInstanceID;
        if (0 != id) {
            --id;
            assert(MAX_INSTANCE > id);
            IEngine *thisEngine = &thiz->mEngine->mEngine;
            // FIXME atomic or here
            interface_lock_exclusive(thisEngine);
            thisEngine->mChangedMask |= 1 << id;
            interface_unlock_exclusive(thisEngine);
        }
    }

}


/** \brief Wait on the condition variable associated with the object; see pthread_cond_wait */

#ifdef USE_DEBUG
void object_cond_wait_(IObject *thiz, const char *file, int line)
{
    // note that this will unlock the mutex, so we have to clear the owner
    assert(pthread_equal(pthread_self(), thiz->mOwner));
    assert(NULL != thiz->mFile);
    assert(0 != thiz->mLine);
    memset(&thiz->mOwner, 0, sizeof(pthread_t));
    thiz->mFile = file;
    thiz->mLine = line;
    // alas we don't know the new owner's identity
    int ok;
    ok = pthread_cond_wait(&thiz->mCond, &thiz->mMutex);
    assert(0 == ok);
    // restore my ownership
    thiz->mOwner = pthread_self();
    thiz->mFile = file;
    thiz->mLine = line;
}
#else
void object_cond_wait(IObject *thiz)
{
    int ok;
    ok = pthread_cond_wait(&thiz->mCond, &thiz->mMutex);
    assert(0 == ok);
}
#endif


/** \brief Signal the condition variable associated with the object; see pthread_cond_signal */

void object_cond_signal(IObject *thiz)
{
    int ok;
    ok = pthread_cond_signal(&thiz->mCond);
    assert(0 == ok);
}


/** \brief Broadcast the condition variable associated with the object;
 *  see pthread_cond_broadcast
 */

void object_cond_broadcast(IObject *thiz)
{
    int ok;
    ok = pthread_cond_broadcast(&thiz->mCond);
    assert(0 == ok);
}
