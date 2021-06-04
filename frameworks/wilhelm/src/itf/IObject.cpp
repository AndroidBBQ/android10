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

/* Object implementation */

#include "sles_allinclusive.h"


// Called by a worker thread to handle an asynchronous Object.Realize.
// Parameter self is the Object.

static void HandleRealize(void *self, void *ignored, int unused)
{

    // validate input parameters
    IObject *thiz = (IObject *) self;
    assert(NULL != thiz);
    const ClassTable *clazz = thiz->mClass;
    assert(NULL != clazz);
    AsyncHook realize = clazz->mRealize;
    SLresult result;
    SLuint8 state;

    // check object state
    object_lock_exclusive(thiz);
    state = thiz->mState;
    switch (state) {

    case SL_OBJECT_STATE_REALIZING_1:   // normal case
        if (NULL != realize) {
            thiz->mState = SL_OBJECT_STATE_REALIZING_2;
            // Note that the mutex is locked on entry to and exit from the realize hook,
            // but the hook is permitted to temporarily unlock the mutex (e.g. for async).
            result = (*realize)(thiz, SL_BOOLEAN_TRUE);
            assert(SL_OBJECT_STATE_REALIZING_2 == thiz->mState);
            state = SL_RESULT_SUCCESS == result ? SL_OBJECT_STATE_REALIZED :
                SL_OBJECT_STATE_UNREALIZED;
        } else {
            result = SL_RESULT_SUCCESS;
            state = SL_OBJECT_STATE_REALIZED;
        }
        break;

    case SL_OBJECT_STATE_REALIZING_1A:  // operation was aborted while on work queue
        result = SL_RESULT_OPERATION_ABORTED;
        state = SL_OBJECT_STATE_UNREALIZED;
        break;

    default:                            // impossible
        assert(SL_BOOLEAN_FALSE);
        result = SL_RESULT_INTERNAL_ERROR;
        break;

    }

    // mutex is locked, update state
    thiz->mState = state;

    // Make a copy of these, so we can call the callback with mutex unlocked
    slObjectCallback callback = thiz->mCallback;
    void *context = thiz->mContext;
    object_unlock_exclusive(thiz);

    // Note that the mutex is unlocked during the callback
    if (NULL != callback) {
        (*callback)(&thiz->mItf, context, SL_OBJECT_EVENT_ASYNC_TERMINATION, result, state, NULL);
    }
}


static SLresult IObject_Realize(SLObjectItf self, SLboolean async)
{
    SL_ENTER_INTERFACE

    IObject *thiz = (IObject *) self;
    SLuint8 state;
    const ClassTable *clazz = thiz->mClass;
    bool isSharedEngine = false;
    object_lock_exclusive(thiz);
    // note that SL_OBJECTID_ENGINE and XA_OBJECTID_ENGINE map to same class
    if (clazz == objectIDtoClass(SL_OBJECTID_ENGINE)) {
        // important: the lock order is engine followed by theOneTrueMutex
        int ok = pthread_mutex_lock(&theOneTrueMutex);
        assert(0 == ok);
        isSharedEngine = 1 < theOneTrueRefCount;
        ok = pthread_mutex_unlock(&theOneTrueMutex);
        assert(0 == ok);
    }
    state = thiz->mState;
    // Reject redundant calls to Realize, except on a shared engine
    if (SL_OBJECT_STATE_UNREALIZED != state) {
        object_unlock_exclusive(thiz);
        // redundant realize on the shared engine is permitted
        if (isSharedEngine && (SL_OBJECT_STATE_REALIZED == state)) {
            result = SL_RESULT_SUCCESS;
        } else {
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        }
    } else {
        // Asynchronous: mark operation pending and cancellable
        if (async && (SL_OBJECTID_ENGINE != clazz->mSLObjectID)) {
            state = SL_OBJECT_STATE_REALIZING_1;
        // Synchronous: mark operation pending and non-cancellable
        } else {
            state = SL_OBJECT_STATE_REALIZING_2;
        }
        thiz->mState = state;
        switch (state) {
        case SL_OBJECT_STATE_REALIZING_1: // asynchronous on non-Engine
            object_unlock_exclusive(thiz);
            assert(async);
            result = ThreadPool_add_ppi(&thiz->mEngine->mThreadPool, HandleRealize, thiz, NULL, 0);
            if (SL_RESULT_SUCCESS != result) {
                // Engine was destroyed during realize, or insufficient memory
                object_lock_exclusive(thiz);
                thiz->mState = SL_OBJECT_STATE_UNREALIZED;
                object_unlock_exclusive(thiz);
            }
            break;
        case SL_OBJECT_STATE_REALIZING_2: // synchronous, or asynchronous on Engine
            {
            AsyncHook realize = clazz->mRealize;
            // Note that the mutex is locked on entry to and exit from the realize hook,
            // but the hook is permitted to temporarily unlock the mutex (e.g. for async).
            result = (NULL != realize) ? (*realize)(thiz, async) : SL_RESULT_SUCCESS;
            assert(SL_OBJECT_STATE_REALIZING_2 == thiz->mState);
            state = (SL_RESULT_SUCCESS == result) ? SL_OBJECT_STATE_REALIZED :
                SL_OBJECT_STATE_UNREALIZED;
            thiz->mState = state;
            slObjectCallback callback = thiz->mCallback;
            void *context = thiz->mContext;
            object_unlock_exclusive(thiz);
            // asynchronous Realize on an Engine is actually done synchronously, but still has
            // callback because there is no thread pool yet to do it asynchronously.
            if (async && (NULL != callback)) {
                (*callback)(&thiz->mItf, context, SL_OBJECT_EVENT_ASYNC_TERMINATION, result, state,
                    NULL);
            }
            }
            break;
        default:                          // impossible
            object_unlock_exclusive(thiz);
            assert(SL_BOOLEAN_FALSE);
            break;
        }
    }

    SL_LEAVE_INTERFACE
}


// Called by a worker thread to handle an asynchronous Object.Resume.
// Parameter self is the Object.

static void HandleResume(void *self, void *ignored, int unused)
{

    // valid input parameters
    IObject *thiz = (IObject *) self;
    assert(NULL != thiz);
    const ClassTable *clazz = thiz->mClass;
    assert(NULL != clazz);
    AsyncHook resume = clazz->mResume;
    SLresult result;
    SLuint8 state;

    // check object state
    object_lock_exclusive(thiz);
    state = thiz->mState;
    switch (state) {

    case SL_OBJECT_STATE_RESUMING_1:    // normal case
        if (NULL != resume) {
            thiz->mState = SL_OBJECT_STATE_RESUMING_2;
            // Note that the mutex is locked on entry to and exit from the resume hook,
            // but the hook is permitted to temporarily unlock the mutex (e.g. for async).
            result = (*resume)(thiz, SL_BOOLEAN_TRUE);
            assert(SL_OBJECT_STATE_RESUMING_2 == thiz->mState);
            state = SL_RESULT_SUCCESS == result ? SL_OBJECT_STATE_REALIZED :
                SL_OBJECT_STATE_SUSPENDED;
        } else {
            result = SL_RESULT_SUCCESS;
            state = SL_OBJECT_STATE_REALIZED;
        }
        break;

    case SL_OBJECT_STATE_RESUMING_1A:   // operation was aborted while on work queue
        result = SL_RESULT_OPERATION_ABORTED;
        state = SL_OBJECT_STATE_SUSPENDED;
        break;

    default:                            // impossible
        assert(SL_BOOLEAN_FALSE);
        result = SL_RESULT_INTERNAL_ERROR;
        break;

    }

    // mutex is unlocked, update state
    thiz->mState = state;

    // Make a copy of these, so we can call the callback with mutex unlocked
    slObjectCallback callback = thiz->mCallback;
    void *context = thiz->mContext;
    object_unlock_exclusive(thiz);

    // Note that the mutex is unlocked during the callback
    if (NULL != callback) {
        (*callback)(&thiz->mItf, context, SL_OBJECT_EVENT_ASYNC_TERMINATION, result, state, NULL);
    }
}


static SLresult IObject_Resume(SLObjectItf self, SLboolean async)
{
    SL_ENTER_INTERFACE

    IObject *thiz = (IObject *) self;
    const ClassTable *clazz = thiz->mClass;
    SLuint8 state;
    object_lock_exclusive(thiz);
    state = thiz->mState;
    // Reject redundant calls to Resume
    if (SL_OBJECT_STATE_SUSPENDED != state) {
        object_unlock_exclusive(thiz);
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else {
        // Asynchronous: mark operation pending and cancellable
        if (async) {
            state = SL_OBJECT_STATE_RESUMING_1;
        // Synchronous: mark operatio pending and non-cancellable
        } else {
            state = SL_OBJECT_STATE_RESUMING_2;
        }
        thiz->mState = state;
        switch (state) {
        case SL_OBJECT_STATE_RESUMING_1: // asynchronous
            object_unlock_exclusive(thiz);
            assert(async);
            result = ThreadPool_add_ppi(&thiz->mEngine->mThreadPool, HandleResume, thiz, NULL, 0);
            if (SL_RESULT_SUCCESS != result) {
                // Engine was destroyed during resume, or insufficient memory
                object_lock_exclusive(thiz);
                thiz->mState = SL_OBJECT_STATE_SUSPENDED;
                object_unlock_exclusive(thiz);
            }
            break;
        case SL_OBJECT_STATE_RESUMING_2: // synchronous
            {
            AsyncHook resume = clazz->mResume;
            // Note that the mutex is locked on entry to and exit from the resume hook,
            // but the hook is permitted to temporarily unlock the mutex (e.g. for async).
            result = (NULL != resume) ? (*resume)(thiz, SL_BOOLEAN_FALSE) : SL_RESULT_SUCCESS;
            assert(SL_OBJECT_STATE_RESUMING_2 == thiz->mState);
            thiz->mState = (SL_RESULT_SUCCESS == result) ? SL_OBJECT_STATE_REALIZED :
                SL_OBJECT_STATE_SUSPENDED;
            object_unlock_exclusive(thiz);
            }
            break;
        default:                        // impossible
            object_unlock_exclusive(thiz);
            assert(SL_BOOLEAN_FALSE);
            break;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IObject_GetState(SLObjectItf self, SLuint32 *pState)
{
    SL_ENTER_INTERFACE

    if (NULL == pState) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IObject *thiz = (IObject *) self;
        object_lock_shared(thiz);
        SLuint8 state = thiz->mState;
        object_unlock_shared(thiz);
        // Re-map the realizing, resuming, and suspending states
        switch (state) {
        case SL_OBJECT_STATE_REALIZING_1:
        case SL_OBJECT_STATE_REALIZING_1A:
        case SL_OBJECT_STATE_REALIZING_2:
        case SL_OBJECT_STATE_DESTROYING:    // application shouldn't call GetState after Destroy
            state = SL_OBJECT_STATE_UNREALIZED;
            break;
        case SL_OBJECT_STATE_RESUMING_1:
        case SL_OBJECT_STATE_RESUMING_1A:
        case SL_OBJECT_STATE_RESUMING_2:
        case SL_OBJECT_STATE_SUSPENDING:
            state = SL_OBJECT_STATE_SUSPENDED;
            break;
        case SL_OBJECT_STATE_UNREALIZED:
        case SL_OBJECT_STATE_REALIZED:
        case SL_OBJECT_STATE_SUSPENDED:
            // These are the "official" object states, return them as is
            break;
        default:
            assert(SL_BOOLEAN_FALSE);
            break;
        }
        *pState = state;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}

static SLresult IObject_GetInterface(SLObjectItf self, const SLInterfaceID iid, void *pInterface)
{
    SL_ENTER_INTERFACE

    if (NULL == pInterface) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        void *interface = NULL;
        if (NULL == iid) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            IObject *thiz = (IObject *) self;
            const ClassTable *clazz = thiz->mClass;
            int MPH, index;
            if ((0 > (MPH = IID_to_MPH(iid))) ||
                    // no need to check for an initialization hook
                    // (NULL == MPH_init_table[MPH].mInit) ||
                    (0 > (index = clazz->mMPH_to_index[MPH]))) {
                result = SL_RESULT_FEATURE_UNSUPPORTED;
            } else {
                unsigned mask = 1 << index;
                object_lock_exclusive(thiz);
                if ((SL_OBJECT_STATE_REALIZED != thiz->mState) &&
                        !(INTERFACE_PREREALIZE & clazz->mInterfaces[index].mInterface)) {
                    // Can't get interface on an unrealized object unless pre-realize is ok
                    result = SL_RESULT_PRECONDITIONS_VIOLATED;
                } else if ((MPH_MUTESOLO == MPH) && (SL_OBJECTID_AUDIOPLAYER ==
                        clazz->mSLObjectID) && (1 == ((CAudioPlayer *) thiz)->mNumChannels)) {
                    // Can't get the MuteSolo interface of an audio player if the channel count is
                    // mono, but _can_ get the MuteSolo interface if the channel count is unknown
                    result = SL_RESULT_FEATURE_UNSUPPORTED;
                } else {
                    switch (thiz->mInterfaceStates[index]) {
                    case INTERFACE_EXPOSED:
                    case INTERFACE_ADDED:
                        interface = (char *) thiz + clazz->mInterfaces[index].mOffset;
                        // Note that interface has been gotten,
                        // for debugger and to detect incorrect use of interfaces
                        if (!(thiz->mGottenMask & mask)) {
                            thiz->mGottenMask |= mask;
                            // This trickery validates the v-table
                            ((size_t *) interface)[0] ^= ~0;
                        }
                        result = SL_RESULT_SUCCESS;
                        break;
                    // Can't get interface if uninitialized, initialized, suspended,
                    // suspending, resuming, adding, or removing
                    default:
                        result = SL_RESULT_FEATURE_UNSUPPORTED;
                        break;
                    }
                }
                object_unlock_exclusive(thiz);
            }
        }
        *(void **)pInterface = interface;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IObject_RegisterCallback(SLObjectItf self,
    slObjectCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IObject *thiz = (IObject *) self;
    object_lock_exclusive(thiz);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    object_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


/** \brief This is internal common code for Abort and Destroy.
 *  Note: called with mutex unlocked, and returns with mutex locked.
 */

static void Abort_internal(IObject *thiz)
{
    const ClassTable *clazz = thiz->mClass;
    bool anyAsync = false;
    object_lock_exclusive(thiz);

    // Abort asynchronous operations on the object
    switch (thiz->mState) {
    case SL_OBJECT_STATE_REALIZING_1:   // Realize
        thiz->mState = SL_OBJECT_STATE_REALIZING_1A;
        anyAsync = true;
        break;
    case SL_OBJECT_STATE_RESUMING_1:    // Resume
        thiz->mState = SL_OBJECT_STATE_RESUMING_1A;
        anyAsync = true;
        break;
    case SL_OBJECT_STATE_REALIZING_1A:  // Realize
    case SL_OBJECT_STATE_REALIZING_2:
    case SL_OBJECT_STATE_RESUMING_1A:   // Resume
    case SL_OBJECT_STATE_RESUMING_2:
        anyAsync = true;
        break;
    case SL_OBJECT_STATE_DESTROYING:
        assert(false);
        break;
    default:
        break;
    }

    // Abort asynchronous operations on interfaces
    SLuint8 *interfaceStateP = thiz->mInterfaceStates;
    unsigned index;
    for (index = 0; index < clazz->mInterfaceCount; ++index, ++interfaceStateP) {
        switch (*interfaceStateP) {
        case INTERFACE_ADDING_1:    // AddInterface
            *interfaceStateP = INTERFACE_ADDING_1A;
            anyAsync = true;
            break;
        case INTERFACE_RESUMING_1:  // ResumeInterface
            *interfaceStateP = INTERFACE_RESUMING_1A;
            anyAsync = true;
            break;
        case INTERFACE_ADDING_1A:   // AddInterface
        case INTERFACE_ADDING_2:
        case INTERFACE_RESUMING_1A: // ResumeInterface
        case INTERFACE_RESUMING_2:
        case INTERFACE_REMOVING:    // not observable: RemoveInterface is synchronous & mutex locked
            anyAsync = true;
            break;
        default:
            break;
        }
    }

    // Wait until all asynchronous operations either complete normally or recognize the abort
    while (anyAsync) {
        object_unlock_exclusive(thiz);
        // FIXME should use condition variable instead of polling
        usleep(20000);
        anyAsync = false;
        object_lock_exclusive(thiz);
        switch (thiz->mState) {
        case SL_OBJECT_STATE_REALIZING_1:   // state 1 means it cycled during the usleep window
        case SL_OBJECT_STATE_RESUMING_1:
        case SL_OBJECT_STATE_REALIZING_1A:
        case SL_OBJECT_STATE_REALIZING_2:
        case SL_OBJECT_STATE_RESUMING_1A:
        case SL_OBJECT_STATE_RESUMING_2:
            anyAsync = true;
            break;
        case SL_OBJECT_STATE_DESTROYING:
            assert(false);
            break;
        default:
            break;
        }
        interfaceStateP = thiz->mInterfaceStates;
        for (index = 0; index < clazz->mInterfaceCount; ++index, ++interfaceStateP) {
            switch (*interfaceStateP) {
            case INTERFACE_ADDING_1:    // state 1 means it cycled during the usleep window
            case INTERFACE_RESUMING_1:
            case INTERFACE_ADDING_1A:
            case INTERFACE_ADDING_2:
            case INTERFACE_RESUMING_1A:
            case INTERFACE_RESUMING_2:
            case INTERFACE_REMOVING:
                anyAsync = true;
                break;
            default:
                break;
            }
        }
    }

    // At this point there are no pending asynchronous operations
}


static void IObject_AbortAsyncOperation(SLObjectItf self)
{
    SL_ENTER_INTERFACE_VOID

    IObject *thiz = (IObject *) self;
    Abort_internal(thiz);
    object_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE_VOID
}


void IObject_Destroy(SLObjectItf self)
{
    SL_ENTER_INTERFACE_VOID

    IObject *thiz = (IObject *) self;
    // mutex is unlocked
    Abort_internal(thiz);
    // mutex is locked
    const ClassTable *clazz = thiz->mClass;
    PreDestroyHook preDestroy = clazz->mPreDestroy;
    // The pre-destroy hook is called with mutex locked, and should block until it is safe to
    // destroy.  It is OK to unlock the mutex temporarily, as it long as it re-locks the mutex
    // before returning.
    if (NULL != preDestroy) {
        predestroy_t okToDestroy = (*preDestroy)(thiz);
        switch (okToDestroy) {
        case predestroy_ok:
            break;
        case predestroy_error:
            SL_LOGE("Object::Destroy(%p) not allowed", thiz);
            FALLTHROUGH_INTENDED;
        case predestroy_again:
            object_unlock_exclusive(thiz);
            // unfortunately Destroy doesn't return a result
            SL_LEAVE_INTERFACE_VOID
            // unreachable
        default:
            assert(false);
            break;
        }
    }
    thiz->mState = SL_OBJECT_STATE_DESTROYING;
    VoidHook destroy = clazz->mDestroy;
    // const, no lock needed
    IEngine *thisEngine = &thiz->mEngine->mEngine;
    unsigned i = thiz->mInstanceID;
    assert(MAX_INSTANCE >= i);
    // avoid a recursive lock on the engine when destroying the engine itself
    if (thisEngine->mThis != thiz) {
        interface_lock_exclusive(thisEngine);
    }
    // An unpublished object has a slot reserved, but the ID hasn't been chosen yet
    assert(0 < thisEngine->mInstanceCount);
    --thisEngine->mInstanceCount;
    // If object is published, then remove it from exposure to sync thread and debugger
    if (0 != i) {
        --i;
        unsigned mask = 1 << i;
        assert(thisEngine->mInstanceMask & mask);
        thisEngine->mInstanceMask &= ~mask;
        assert(thisEngine->mInstances[i] == thiz);
        thisEngine->mInstances[i] = NULL;
    }
    // avoid a recursive unlock on the engine when destroying the engine itself
    if (thisEngine->mThis != thiz) {
        interface_unlock_exclusive(thisEngine);
    }
    // The destroy hook is called with mutex locked
    if (NULL != destroy) {
        (*destroy)(thiz);
    }
    // Call the deinitializer for each currently initialized interface,
    // whether it is implicit, explicit, optional, or dynamically added.
    // The deinitializers are called in the reverse order that the
    // initializers were called, so that IObject_deinit is called last.
    unsigned index = clazz->mInterfaceCount;
    const struct iid_vtable *x = &clazz->mInterfaces[index];
    SLuint8 *interfaceStateP = &thiz->mInterfaceStates[index];
    for ( ; index > 0; --index) {
        --x;
        size_t offset = x->mOffset;
        void *thisItf = (char *) thiz + offset;
        SLuint32 state = *--interfaceStateP;
        switch (state) {
        case INTERFACE_UNINITIALIZED:
            break;
        case INTERFACE_EXPOSED:     // quiescent states
        case INTERFACE_ADDED:
        case INTERFACE_SUSPENDED:
            // The remove hook is called with mutex locked
            {
            VoidHook remove = MPH_init_table[x->mMPH].mRemove;
            if (NULL != remove) {
                (*remove)(thisItf);
            }
            *interfaceStateP = INTERFACE_INITIALIZED;
            }
            FALLTHROUGH_INTENDED;
        case INTERFACE_INITIALIZED:
            {
            VoidHook deinit = MPH_init_table[x->mMPH].mDeinit;
            if (NULL != deinit) {
                (*deinit)(thisItf);
            }
            *interfaceStateP = INTERFACE_UNINITIALIZED;
            }
            break;
        case INTERFACE_ADDING_1:    // active states indicate incorrect use of API
        case INTERFACE_ADDING_1A:
        case INTERFACE_ADDING_2:
        case INTERFACE_RESUMING_1:
        case INTERFACE_RESUMING_1A:
        case INTERFACE_RESUMING_2:
        case INTERFACE_REMOVING:
        case INTERFACE_SUSPENDING:
            SL_LOGE("Object::Destroy(%p) while interface %u active", thiz, index);
            break;
        default:
            assert(SL_BOOLEAN_FALSE);
            break;
        }
    }
    // The mutex is unlocked and destroyed by IObject_deinit, which is the last deinitializer
    memset(thiz, 0x55, clazz->mSize); // catch broken applications that continue using interfaces
                                        // was ifdef USE_DEBUG but safer to do this unconditionally
    if (SL_OBJECTID_ENGINE == clazz->mSLObjectID) {
        CEngine_Destroyed((CEngine *) thiz);
    }
    free(thiz);

    SL_LEAVE_INTERFACE_VOID
}


static SLresult IObject_SetPriority(SLObjectItf self, SLint32 priority, SLboolean preemptable)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_BASE
    IObject *thiz = (IObject *) self;
    object_lock_exclusive(thiz);
    thiz->mPriority = priority;
    thiz->mPreemptable = SL_BOOLEAN_FALSE != preemptable; // normalize
    object_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IObject_GetPriority(SLObjectItf self, SLint32 *pPriority, SLboolean *pPreemptable)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_BASE
    if (NULL == pPriority || NULL == pPreemptable) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IObject *thiz = (IObject *) self;
        object_lock_shared(thiz);
        SLint32 priority = thiz->mPriority;
        SLboolean preemptable = thiz->mPreemptable;
        object_unlock_shared(thiz);
        *pPriority = priority;
        *pPreemptable = preemptable;
        result = SL_RESULT_SUCCESS;
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IObject_SetLossOfControlInterfaces(SLObjectItf self,
    SLint16 numInterfaces, SLInterfaceID *pInterfaceIDs, SLboolean enabled)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_BASE
    result = SL_RESULT_SUCCESS;
    if (0 < numInterfaces) {
        SLuint32 i;
        if (NULL == pInterfaceIDs) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            IObject *thiz = (IObject *) self;
            const ClassTable *clazz = thiz->mClass;
            unsigned lossOfControlMask = 0;
            // The cast is due to a typo in the spec, bug 6482
            for (i = 0; i < (SLuint32) numInterfaces; ++i) {
                SLInterfaceID iid = pInterfaceIDs[i];
                if (NULL == iid) {
                    result = SL_RESULT_PARAMETER_INVALID;
                    goto out;
                }
                int MPH, index;
                // We ignore without error any invalid MPH or index, but spec is unclear
                if ((0 <= (MPH = IID_to_MPH(iid))) &&
                        // no need to check for an initialization hook
                        // (NULL == MPH_init_table[MPH].mInit) ||
                        (0 <= (index = clazz->mMPH_to_index[MPH]))) {
                    lossOfControlMask |= (1 << index);
                }
            }
            object_lock_exclusive(thiz);
            if (enabled) {
                thiz->mLossOfControlMask |= lossOfControlMask;
            } else {
                thiz->mLossOfControlMask &= ~lossOfControlMask;
            }
            object_unlock_exclusive(thiz);
        }
    }
out:
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static const struct SLObjectItf_ IObject_Itf = {
    IObject_Realize,
    IObject_Resume,
    IObject_GetState,
    IObject_GetInterface,
    IObject_RegisterCallback,
    IObject_AbortAsyncOperation,
    IObject_Destroy,
    IObject_SetPriority,
    IObject_GetPriority,
    IObject_SetLossOfControlInterfaces
};


/** \brief This must be the first initializer called for an object */

void IObject_init(void *self)
{
    IObject *thiz = (IObject *) self;
    thiz->mItf = &IObject_Itf;
    // initialized in construct:
    // mClass
    // mInstanceID
    // mLossOfControlMask
    // mEngine
    // mInterfaceStates
    thiz->mState = SL_OBJECT_STATE_UNREALIZED;
    thiz->mGottenMask = 1;  // IObject
    thiz->mAttributesMask = 0;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
#if USE_PROFILES & USE_PROFILES_BASE
    thiz->mPriority = SL_PRIORITY_NORMAL;
    thiz->mPreemptable = SL_BOOLEAN_FALSE;
#endif
    thiz->mStrongRefCount = 0;
    int ok;
    ok = pthread_mutex_init(&thiz->mMutex, (const pthread_mutexattr_t *) NULL);
    assert(0 == ok);
#ifdef USE_DEBUG
    memset(&thiz->mOwner, 0, sizeof(pthread_t));
    thiz->mFile = NULL;
    thiz->mLine = 0;
    thiz->mGeneration = 0;
#endif
    ok = pthread_cond_init(&thiz->mCond, (const pthread_condattr_t *) NULL);
    assert(0 == ok);
}


/** \brief This must be the last deinitializer called for an object */

void IObject_deinit(void *self)
{
    IObject *thiz = (IObject *) self;
#ifdef USE_DEBUG
    assert(pthread_equal(pthread_self(), thiz->mOwner));
#endif
    int ok;
    ok = pthread_cond_destroy(&thiz->mCond);
    assert(0 == ok);
    // equivalent to object_unlock_exclusive, but without the rigmarole
    ok = pthread_mutex_unlock(&thiz->mMutex);
    assert(0 == ok);
    ok = pthread_mutex_destroy(&thiz->mMutex);
    assert(0 == ok);
    // redundant: thiz->mState = SL_OBJECT_STATE_UNREALIZED;
}


/** \brief Publish a new object after it is fully initialized.
 *  Publishing will expose the object to sync thread and debugger,
 *  and make it safe to return the SLObjectItf to the application.
 */

void IObject_Publish(IObject *thiz)
{
    IEngine *thisEngine = &thiz->mEngine->mEngine;
    interface_lock_exclusive(thisEngine);
    // construct earlier reserved a pending slot, but did not choose the actual slot number
    unsigned availMask = ~thisEngine->mInstanceMask;
    assert(availMask);
    unsigned i = ctz(availMask);
    assert(MAX_INSTANCE > i);
    assert(NULL == thisEngine->mInstances[i]);
    thisEngine->mInstances[i] = thiz;
    thisEngine->mInstanceMask |= 1 << i;
    // avoid zero as a valid instance ID
    thiz->mInstanceID = i + 1;
    interface_unlock_exclusive(thisEngine);
}
