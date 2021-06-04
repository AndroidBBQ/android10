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

/* ThreadPool */

#include "sles_allinclusive.h"

// Entry point for each worker thread

static void *ThreadPool_start(void *context)
{
    ThreadPool *tp = (ThreadPool *) context;
    assert(NULL != tp);
    for (;;) {
        Closure *pClosure = ThreadPool_remove(tp);
        // closure is NULL when thread pool is being destroyed
        if (NULL == pClosure) {
            break;
        }
        // make a copy of parameters, then free the parameters
        const Closure closure = *pClosure;
        free(pClosure);
        // extract parameters and call the right method depending on kind
        ClosureKind kind = closure.mKind;
        void *context1 = closure.mContext1;
        void *context2 = closure.mContext2;
        int parameter1 = closure.mParameter1;
        switch (kind) {
          case CLOSURE_KIND_PPI:
            {
            ClosureHandler_ppi handler_ppi = closure.mHandler.mHandler_ppi;
            assert(NULL != handler_ppi);
            (*handler_ppi)(context1, context2, parameter1);
            }
            break;
          case CLOSURE_KIND_PPII:
            {
            ClosureHandler_ppii handler_ppii = closure.mHandler.mHandler_ppii;
            assert(NULL != handler_ppii);
            int parameter2 = closure.mParameter2;
            (*handler_ppii)(context1, context2, parameter1, parameter2);
            }
            break;
          case CLOSURE_KIND_PIIPP:
            {
            ClosureHandler_piipp handler_piipp = closure.mHandler.mHandler_piipp;
            assert(NULL != handler_piipp);
            int parameter2 = closure.mParameter2;
            void *context3 = closure.mContext3;
            (*handler_piipp)(context1, parameter1, parameter2, context2, context3);
            }
            break;
          default:
            SL_LOGE("Unexpected callback kind %d", kind);
            assert(false);
            break;
        }
    }
    return NULL;
}

#define INITIALIZED_NONE         0
#define INITIALIZED_MUTEX        1
#define INITIALIZED_CONDNOTFULL  2
#define INITIALIZED_CONDNOTEMPTY 4
#define INITIALIZED_ALL          7

static void ThreadPool_deinit_internal(ThreadPool *tp, unsigned initialized, unsigned nThreads);

// Initialize a ThreadPool
// maxClosures defaults to CLOSURE_TYPICAL if 0
// maxThreads defaults to THREAD_TYPICAL if 0

SLresult ThreadPool_init(ThreadPool *tp, unsigned maxClosures, unsigned maxThreads)
{
    assert(NULL != tp);
    memset(tp, 0, sizeof(ThreadPool));
    tp->mShutdown = SL_BOOLEAN_FALSE;
    unsigned initialized = INITIALIZED_NONE;    // which objects were successfully initialized
    unsigned nThreads = 0;                      // number of threads successfully created
    int err;
    SLresult result;

    // initialize mutex and condition variables
    err = pthread_mutex_init(&tp->mMutex, (const pthread_mutexattr_t *) NULL);
    result = err_to_result(err);
    if (SL_RESULT_SUCCESS != result)
        goto fail;
    initialized |= INITIALIZED_MUTEX;
    err = pthread_cond_init(&tp->mCondNotFull, (const pthread_condattr_t *) NULL);
    result = err_to_result(err);
    if (SL_RESULT_SUCCESS != result)
        goto fail;
    initialized |= INITIALIZED_CONDNOTFULL;
    err = pthread_cond_init(&tp->mCondNotEmpty, (const pthread_condattr_t *) NULL);
    result = err_to_result(err);
    if (SL_RESULT_SUCCESS != result)
        goto fail;
    initialized |= INITIALIZED_CONDNOTEMPTY;

    // use default values for parameters, if not specified explicitly
    tp->mWaitingNotFull = 0;
    tp->mWaitingNotEmpty = 0;
    if (0 == maxClosures)
        maxClosures = CLOSURE_TYPICAL;
    tp->mMaxClosures = maxClosures;
    if (0 == maxThreads)
        maxThreads = THREAD_TYPICAL;
    tp->mMaxThreads = maxThreads;

    // initialize circular buffer for closures
    if (CLOSURE_TYPICAL >= maxClosures) {
        tp->mClosureArray = tp->mClosureTypical;
    } else {
        tp->mClosureArray = (Closure **) malloc((maxClosures + 1) * sizeof(Closure *));
        if (NULL == tp->mClosureArray) {
            result = SL_RESULT_RESOURCE_ERROR;
            goto fail;
        }
    }
    tp->mClosureFront = tp->mClosureArray;
    tp->mClosureRear = tp->mClosureArray;

    // initialize thread pool
    if (THREAD_TYPICAL >= maxThreads) {
        tp->mThreadArray = tp->mThreadTypical;
    } else {
        tp->mThreadArray = (pthread_t *) malloc(maxThreads * sizeof(pthread_t));
        if (NULL == tp->mThreadArray) {
            result = SL_RESULT_RESOURCE_ERROR;
            goto fail;
        }
    }
    unsigned i;
    for (i = 0; i < maxThreads; ++i) {
        int err = pthread_create(&tp->mThreadArray[i], (const pthread_attr_t *) NULL,
            ThreadPool_start, tp);
        result = err_to_result(err);
        if (SL_RESULT_SUCCESS != result)
            goto fail;
        ++nThreads;
    }
    tp->mInitialized = initialized;

    // done
    return SL_RESULT_SUCCESS;

    // here on any kind of error
fail:
    ThreadPool_deinit_internal(tp, initialized, nThreads);
    return result;
}

static void ThreadPool_deinit_internal(ThreadPool *tp, unsigned initialized, unsigned nThreads)
{
    int ok;

    assert(NULL != tp);
    // Destroy all threads
    if (0 < nThreads) {
        assert(INITIALIZED_ALL == initialized);
        ok = pthread_mutex_lock(&tp->mMutex);
        assert(0 == ok);
        tp->mShutdown = SL_BOOLEAN_TRUE;
        ok = pthread_cond_broadcast(&tp->mCondNotEmpty);
        assert(0 == ok);
        ok = pthread_cond_broadcast(&tp->mCondNotFull);
        assert(0 == ok);
        ok = pthread_mutex_unlock(&tp->mMutex);
        assert(0 == ok);
        unsigned i;
        for (i = 0; i < nThreads; ++i) {
            ok = pthread_join(tp->mThreadArray[i], (void **) NULL);
            assert(ok == 0);
        }

        // Empty out the circular buffer of closures
        ok = pthread_mutex_lock(&tp->mMutex);
        assert(0 == ok);
        Closure **oldFront = tp->mClosureFront;
        while (oldFront != tp->mClosureRear) {
            Closure **newFront = oldFront;
            if (++newFront == &tp->mClosureArray[tp->mMaxClosures + 1])
                newFront = tp->mClosureArray;
            Closure *pClosure = *oldFront;
            assert(NULL != pClosure);
            *oldFront = NULL;
            tp->mClosureFront = newFront;
            ok = pthread_mutex_unlock(&tp->mMutex);
            assert(0 == ok);
            free(pClosure);
            ok = pthread_mutex_lock(&tp->mMutex);
            assert(0 == ok);
        }
        ok = pthread_mutex_unlock(&tp->mMutex);
        assert(0 == ok);
        // Note that we can't be sure when mWaitingNotFull will drop to zero
    }

    // destroy the mutex and condition variables
    if (initialized & INITIALIZED_CONDNOTEMPTY) {
        ok = pthread_cond_destroy(&tp->mCondNotEmpty);
        assert(0 == ok);
    }
    if (initialized & INITIALIZED_CONDNOTFULL) {
        ok = pthread_cond_destroy(&tp->mCondNotFull);
        assert(0 == ok);
    }
    if (initialized & INITIALIZED_MUTEX) {
        ok = pthread_mutex_destroy(&tp->mMutex);
        assert(0 == ok);
    }
    tp->mInitialized = INITIALIZED_NONE;

    // release the closure circular buffer
    if (tp->mClosureTypical != tp->mClosureArray && NULL != tp->mClosureArray) {
        free(tp->mClosureArray);
        tp->mClosureArray = NULL;
    }

    // release the thread pool
    if (tp->mThreadTypical != tp->mThreadArray && NULL != tp->mThreadArray) {
        free(tp->mThreadArray);
        tp->mThreadArray = NULL;
    }

}

void ThreadPool_deinit(ThreadPool *tp)
{
    ThreadPool_deinit_internal(tp, tp->mInitialized, tp->mMaxThreads);
}

// Enqueue a closure to be executed later by a worker thread.
// Note that this raw interface requires an explicit "kind" and full parameter list.
// There are convenience methods below that make this easier to use.
SLresult ThreadPool_add(ThreadPool *tp, ClosureKind kind, ClosureHandler_generic handler,
        void *context1, void *context2, void *context3, int parameter1, int parameter2)
{
    assert(NULL != tp);
    assert(NULL != handler);
    Closure *closure = (Closure *) malloc(sizeof(Closure));
    if (NULL == closure) {
        return SL_RESULT_RESOURCE_ERROR;
    }
    closure->mKind = kind;
    switch (kind) {
      case CLOSURE_KIND_PPI:
        closure->mHandler.mHandler_ppi = (ClosureHandler_ppi)handler;
        break;
      case CLOSURE_KIND_PPII:
        closure->mHandler.mHandler_ppii = (ClosureHandler_ppii)handler;
        break;
      case CLOSURE_KIND_PIIPP:
        closure->mHandler.mHandler_piipp = (ClosureHandler_piipp)handler;
        break;
      default:
        SL_LOGE("ThreadPool_add() invalid closure kind %d", kind);
        assert(false);
    }
    closure->mContext1 = context1;
    closure->mContext2 = context2;
    closure->mContext3 = context3;
    closure->mParameter1 = parameter1;
    closure->mParameter2 = parameter2;
    int ok;
    ok = pthread_mutex_lock(&tp->mMutex);
    assert(0 == ok);
    // can't enqueue while thread pool shutting down
    if (tp->mShutdown) {
        ok = pthread_mutex_unlock(&tp->mMutex);
        assert(0 == ok);
        free(closure);
        return SL_RESULT_PRECONDITIONS_VIOLATED;
    }
    for (;;) {
        Closure **oldRear = tp->mClosureRear;
        Closure **newRear = oldRear;
        if (++newRear == &tp->mClosureArray[tp->mMaxClosures + 1])
            newRear = tp->mClosureArray;
        // if closure circular buffer is full, then wait for it to become non-full
        if (newRear == tp->mClosureFront) {
            ++tp->mWaitingNotFull;
            ok = pthread_cond_wait(&tp->mCondNotFull, &tp->mMutex);
            assert(0 == ok);
            // can't enqueue while thread pool shutting down
            if (tp->mShutdown) {
                assert(0 < tp->mWaitingNotFull);
                --tp->mWaitingNotFull;
                ok = pthread_mutex_unlock(&tp->mMutex);
                assert(0 == ok);
                free(closure);
                return SL_RESULT_PRECONDITIONS_VIOLATED;
            }
            continue;
        }
        assert(NULL == *oldRear);
        *oldRear = closure;
        tp->mClosureRear = newRear;
        // if a worker thread was waiting to dequeue, then suggest that it try again
        if (0 < tp->mWaitingNotEmpty) {
            --tp->mWaitingNotEmpty;
            ok = pthread_cond_signal(&tp->mCondNotEmpty);
            assert(0 == ok);
        }
        break;
    }
    ok = pthread_mutex_unlock(&tp->mMutex);
    assert(0 == ok);
    return SL_RESULT_SUCCESS;
}

// Called by a worker thread when it is ready to accept the next closure to execute
Closure *ThreadPool_remove(ThreadPool *tp)
{
    Closure *pClosure;
    int ok;
    ok = pthread_mutex_lock(&tp->mMutex);
    assert(0 == ok);
    for (;;) {
        // fail if thread pool is shutting down
        if (tp->mShutdown) {
            pClosure = NULL;
            break;
        }
        Closure **oldFront = tp->mClosureFront;
        // if closure circular buffer is empty, then wait for it to become non-empty
        if (oldFront == tp->mClosureRear) {
            ++tp->mWaitingNotEmpty;
            ok = pthread_cond_wait(&tp->mCondNotEmpty, &tp->mMutex);
            assert(0 == ok);
            // try again
            continue;
        }
        // dequeue the closure at front of circular buffer
        Closure **newFront = oldFront;
        if (++newFront == &tp->mClosureArray[tp->mMaxClosures + 1]) {
            newFront = tp->mClosureArray;
        }
        pClosure = *oldFront;
        assert(NULL != pClosure);
        *oldFront = NULL;
        tp->mClosureFront = newFront;
        // if a client thread was waiting to enqueue, then suggest that it try again
        if (0 < tp->mWaitingNotFull) {
            --tp->mWaitingNotFull;
            ok = pthread_cond_signal(&tp->mCondNotFull);
            assert(0 == ok);
        }
        break;
    }
    ok = pthread_mutex_unlock(&tp->mMutex);
    assert(0 == ok);
    return pClosure;
}

// Convenience methods for applications
SLresult ThreadPool_add_ppi(ThreadPool *tp, ClosureHandler_ppi handler,
        void *context1, void *context2, int parameter1)
{
    // function pointers are the same size so this is a safe cast
    return ThreadPool_add(tp, CLOSURE_KIND_PPI, (ClosureHandler_generic) handler,
            context1, context2, NULL, parameter1, 0);
}

SLresult ThreadPool_add_ppii(ThreadPool *tp, ClosureHandler_ppii handler,
        void *context1, void *context2, int parameter1, int parameter2)
{
    // function pointers are the same size so this is a safe cast
    return ThreadPool_add(tp, CLOSURE_KIND_PPII, (ClosureHandler_generic) handler,
            context1, context2, NULL, parameter1, parameter2);
}

SLresult ThreadPool_add_piipp(ThreadPool *tp, ClosureHandler_piipp handler,
        void *cntxt1, int param1, int param2, void *cntxt2, void *cntxt3)
{
    // function pointers are the same size so this is a safe cast
    return ThreadPool_add(tp, CLOSURE_KIND_PIIPP, (ClosureHandler_generic) handler,
            cntxt1, cntxt2, cntxt3, param1, param2);
}
