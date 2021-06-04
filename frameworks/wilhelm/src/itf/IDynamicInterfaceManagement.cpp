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

/* DynamicInterfaceManagement implementation */

#include "sles_allinclusive.h"


// Called by a worker thread to handle an asynchronous AddInterface.
// Parameter self is the DynamicInterface, and MPH specifies which interface to add.

static void HandleAdd(void *self, void *ignored, int MPH)
{

    // validate input parameters
    IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
    assert(NULL != thiz);
    IObject *thisObject = InterfaceToIObject(thiz);
    assert(NULL != thisObject);
    assert(0 <= MPH && MPH < MPH_MAX);
    const ClassTable *clazz = thisObject->mClass;
    assert(NULL != clazz);
    int index = clazz->mMPH_to_index[MPH];
    assert(0 <= index && index < (int) clazz->mInterfaceCount);
    SLuint8 *interfaceStateP = &thisObject->mInterfaceStates[index];
    SLresult result;

    // check interface state
    object_lock_exclusive(thisObject);
    SLuint8 state = *interfaceStateP;
    switch (state) {

    case INTERFACE_ADDING_1:    // normal case
        {
        // change state to indicate we are now adding the interface
        *interfaceStateP = INTERFACE_ADDING_2;
        object_unlock_exclusive(thisObject);

        // this section runs with mutex unlocked
        const struct iid_vtable *x = &clazz->mInterfaces[index];
        size_t offset = x->mOffset;
        void *thisItf = (char *) thisObject + offset;
        BoolHook expose = MPH_init_table[MPH].mExpose;
        // call the optional expose hook
        if ((NULL == expose) || (*expose)(thisItf)) {
            result = SL_RESULT_SUCCESS;
        } else {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        }

        // re-lock mutex to update state
        object_lock_exclusive(thisObject);
        assert(INTERFACE_ADDING_2 == *interfaceStateP);
        if (SL_RESULT_SUCCESS == result) {
            ((size_t *) thisItf)[0] ^= ~0;
            state = INTERFACE_ADDED;
        } else {
            state = INTERFACE_INITIALIZED;
        }
        }
        break;

    case INTERFACE_ADDING_1A:   // operation was aborted while on work queue
        result = SL_RESULT_OPERATION_ABORTED;
        state = INTERFACE_INITIALIZED;
        break;

    default:                    // impossible
        assert(SL_BOOLEAN_FALSE);
        result = SL_RESULT_INTERNAL_ERROR;
        break;

    }

    // mutex is locked, update state
    *interfaceStateP = state;

    // Make a copy of these, so we can call the callback with mutex unlocked
    slDynamicInterfaceManagementCallback callback = thiz->mCallback;
    void *context = thiz->mContext;
    object_unlock_exclusive(thisObject);

    // Note that the mutex is unlocked during the callback
    if (NULL != callback) {
        const SLInterfaceID iid = &SL_IID_array[MPH]; // equal but not == to the original IID
        (*callback)(&thiz->mItf, context, SL_DYNAMIC_ITF_EVENT_ASYNC_TERMINATION, result, iid);
    }

}


static SLresult IDynamicInterfaceManagement_AddInterface(SLDynamicInterfaceManagementItf self,
    const SLInterfaceID iid, SLboolean async)
{
    SL_ENTER_INTERFACE

    // validate input parameters
    if (NULL == iid) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
        IObject *thisObject = InterfaceToIObject(thiz);
        const ClassTable *clazz = thisObject->mClass;
        int MPH, index;
        if ((0 > (MPH = IID_to_MPH(iid))) ||
                // no need to check for an initialization hook
                // (NULL == MPH_init_table[MPH].mInit) ||
                (0 > (index = clazz->mMPH_to_index[MPH]))) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            assert(index < (int) clazz->mInterfaceCount);
            SLuint8 *interfaceStateP = &thisObject->mInterfaceStates[index];

            // check interface state
            object_lock_exclusive(thisObject);
            switch (*interfaceStateP) {

            case INTERFACE_INITIALIZED: // normal case
                if (async) {
                    // Asynchronous: mark operation pending and cancellable
                    *interfaceStateP = INTERFACE_ADDING_1;
                    object_unlock_exclusive(thisObject);

                    // this section runs with mutex unlocked
                    result = ThreadPool_add_ppi(&thisObject->mEngine->mThreadPool, HandleAdd, thiz,
                        NULL, MPH);
                    if (SL_RESULT_SUCCESS != result) {
                        // Engine was destroyed during add, or insufficient memory,
                        // so restore mInterfaceStates state to prior value
                        object_lock_exclusive(thisObject);
                        switch (*interfaceStateP) {
                        case INTERFACE_ADDING_1:    // normal
                        case INTERFACE_ADDING_1A:   // operation aborted while mutex unlocked
                            *interfaceStateP = INTERFACE_INITIALIZED;
                            break;
                        default:                    // unexpected
                            // leave state alone
                            break;
                        }
                    }

                } else {
                    // Synchronous: mark operation pending to prevent duplication
                    *interfaceStateP = INTERFACE_ADDING_2;
                    object_unlock_exclusive(thisObject);

                    // this section runs with mutex unlocked
                    const struct iid_vtable *x = &clazz->mInterfaces[index];
                    size_t offset = x->mOffset;
                    void *thisItf = (char *) thisObject + offset;
                    // call the optional expose hook
                    BoolHook expose = MPH_init_table[MPH].mExpose;
                    if ((NULL == expose) || (*expose)(thisItf)) {
                        result = SL_RESULT_SUCCESS;
                    } else {
                        result = SL_RESULT_FEATURE_UNSUPPORTED;
                    }

                    // re-lock mutex to update state
                    object_lock_exclusive(thisObject);
                    assert(INTERFACE_ADDING_2 == *interfaceStateP);
                    if (SL_RESULT_SUCCESS == result) {
                        *interfaceStateP = INTERFACE_ADDED;
                    } else {
                        *interfaceStateP = INTERFACE_INITIALIZED;
                    }
                }

                // mutex is still locked
                break;

            default:    // disallow adding of (partially) initialized interfaces
                result = SL_RESULT_PRECONDITIONS_VIOLATED;
                break;

            }

            object_unlock_exclusive(thisObject);

        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IDynamicInterfaceManagement_RemoveInterface(
    SLDynamicInterfaceManagementItf self, const SLInterfaceID iid)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_BASE
    // validate input parameters
    if (NULL == iid) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
        IObject *thisObject = InterfaceToIObject(thiz);
        const ClassTable *clazz = thisObject->mClass;
        int MPH, index;
        if ((0 > (MPH = IID_to_MPH(iid))) ||
                // no need to check for an initialization hook
                // (NULL == MPH_init_table[MPH].mInit) ||
                (0 > (index = clazz->mMPH_to_index[MPH]))) {
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        } else {
            SLuint8 *interfaceStateP = &thisObject->mInterfaceStates[index];

            // check interface state
            object_lock_exclusive(thisObject);
            switch (*interfaceStateP) {

            case INTERFACE_ADDED:       // normal cases
            case INTERFACE_SUSPENDED:
                {
                // Compute address of the interface
                const struct iid_vtable *x = &clazz->mInterfaces[index];
                size_t offset = x->mOffset;
                void *thisItf = (char *) thisObject + offset;

                // Mark operation pending (not necessary; remove is synchronous with mutex locked)
                *interfaceStateP = INTERFACE_REMOVING;

                // Check if application ever called Object::GetInterface
                unsigned mask = 1 << index;
                if (thisObject->mGottenMask & mask) {
                    thisObject->mGottenMask &= ~mask;
                    // This trickery invalidates the v-table
                    ((size_t *) thisItf)[0] ^= ~0;
                }

                // The remove hook is called with mutex locked
                VoidHook remove = MPH_init_table[MPH].mRemove;
                if (NULL != remove) {
                    (*remove)(thisItf);
                }
                result = SL_RESULT_SUCCESS;

                assert(INTERFACE_REMOVING == *interfaceStateP);
                *interfaceStateP = INTERFACE_INITIALIZED;
                }

                // mutex is still locked
                break;

            default:
                // disallow removal of non-dynamic interfaces, or interfaces which are
                // currently being resumed (will not auto-cancel an asynchronous resume)
                result = SL_RESULT_PRECONDITIONS_VIOLATED;
                break;

            }

            object_unlock_exclusive(thisObject);
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


// Called by a worker thread to handle an asynchronous ResumeInterface.
// Parameter self is the DynamicInterface, and MPH specifies which interface to resume.

static void HandleResume(void *self, void *ignored, int MPH)
{

    // validate input parameters
    IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
    assert(NULL != thiz);
    IObject *thisObject = InterfaceToIObject(thiz);
    assert(NULL != thisObject);
    assert(0 <= MPH && MPH < MPH_MAX);
    const ClassTable *clazz = thisObject->mClass;
    assert(NULL != clazz);
    int index = clazz->mMPH_to_index[MPH];
    assert(0 <= index && index < (int) clazz->mInterfaceCount);
    SLuint8 *interfaceStateP = &thisObject->mInterfaceStates[index];
    SLresult result;

    // check interface state
    object_lock_exclusive(thisObject);
    SLuint8 state = *interfaceStateP;
    switch (state) {

    case INTERFACE_RESUMING_1:      // normal case
        {
        // change state to indicate we are now resuming the interface
        *interfaceStateP = INTERFACE_RESUMING_2;
        object_unlock_exclusive(thisObject);

        // this section runs with mutex unlocked
        const struct iid_vtable *x = &clazz->mInterfaces[index];
        size_t offset = x->mOffset;
        void *thisItf = (char *) thisObject + offset;
        VoidHook resume = MPH_init_table[MPH].mResume;
        if (NULL != resume) {
            (*resume)(thisItf);
        }
        result = SL_RESULT_SUCCESS;

        // re-lock mutex to update state
        object_lock_exclusive(thisObject);
        assert(INTERFACE_RESUMING_2 == *interfaceStateP);
        state = INTERFACE_ADDED;
        }
        break;

    case INTERFACE_RESUMING_1A:     // operation was aborted while on work queue
        result = SL_RESULT_OPERATION_ABORTED;
        state = INTERFACE_SUSPENDED;
        break;

    default:                        // impossible
        assert(SL_BOOLEAN_FALSE);
        result = SL_RESULT_INTERNAL_ERROR;
        break;

    }

    // mutex is locked, update state
    *interfaceStateP = state;

    // Make a copy of these, so we can call the callback with mutex unlocked
    slDynamicInterfaceManagementCallback callback = thiz->mCallback;
    void *context = thiz->mContext;
    object_unlock_exclusive(thisObject);

    // Note that the mutex is unlocked during the callback
    if (NULL != callback) {
        const SLInterfaceID iid = &SL_IID_array[MPH]; // equal but not == to the original IID
        (*callback)(&thiz->mItf, context, SL_DYNAMIC_ITF_EVENT_ASYNC_TERMINATION, result, iid);
    }
}


static SLresult IDynamicInterfaceManagement_ResumeInterface(SLDynamicInterfaceManagementItf self,
    const SLInterfaceID iid, SLboolean async)
{
    SL_ENTER_INTERFACE

    // validate input parameters
    if (NULL == iid) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
        IObject *thisObject = InterfaceToIObject(thiz);
        const ClassTable *clazz = thisObject->mClass;
        int MPH, index;
        if ((0 > (MPH = IID_to_MPH(iid))) ||
                // no need to check for an initialization hook
                // (NULL == MPH_init_table[MPH].mInit) ||
                (0 > (index = clazz->mMPH_to_index[MPH]))) {
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        } else {
            assert(index < (int) clazz->mInterfaceCount);
            SLuint8 *interfaceStateP = &thisObject->mInterfaceStates[index];

            // check interface state
            object_lock_exclusive(thisObject);
            switch (*interfaceStateP) {

            case INTERFACE_SUSPENDED:   // normal case
                if (async) {
                    // Asynchronous: mark operation pending and cancellable
                    *interfaceStateP = INTERFACE_RESUMING_1;
                    object_unlock_exclusive(thisObject);

                    // this section runs with mutex unlocked
                    result = ThreadPool_add_ppi(&thisObject->mEngine->mThreadPool, HandleResume,
                        thiz, NULL, MPH);
                    if (SL_RESULT_SUCCESS != result) {
                        // Engine was destroyed during resume, or insufficient memory,
                        // so restore mInterfaceStates state to prior value
                        object_lock_exclusive(thisObject);
                        switch (*interfaceStateP) {
                        case INTERFACE_RESUMING_1:  // normal
                        case INTERFACE_RESUMING_1A: // operation aborted while mutex unlocked
                            *interfaceStateP = INTERFACE_SUSPENDED;
                            break;
                        default:                    // unexpected
                            // leave state alone
                            break;
                        }
                    }

                } else {
                    // Synchronous: mark operation pending to prevent duplication
                    *interfaceStateP = INTERFACE_RESUMING_2;
                    object_unlock_exclusive(thisObject);

                    // this section runs with mutex unlocked
                    const struct iid_vtable *x = &clazz->mInterfaces[index];
                    size_t offset = x->mOffset;
                    void *thisItf = (char *) thiz + offset;
                    VoidHook resume = MPH_init_table[MPH].mResume;
                    if (NULL != resume) {
                        (*resume)(thisItf);
                    }
                    result = SL_RESULT_SUCCESS;

                    // re-lock mutex to update state
                    object_lock_exclusive(thisObject);
                    assert(INTERFACE_RESUMING_2 == *interfaceStateP);
                    *interfaceStateP = INTERFACE_ADDED;
                }

                // mutex is now locked
                break;

            default:    // disallow resumption of non-suspended interfaces
                result = SL_RESULT_PRECONDITIONS_VIOLATED;
                break;
            }

            object_unlock_exclusive(thisObject);
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IDynamicInterfaceManagement_RegisterCallback(SLDynamicInterfaceManagementItf self,
    slDynamicInterfaceManagementCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
    IObject *thisObject = InterfaceToIObject(thiz);
    object_lock_exclusive(thisObject);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    object_unlock_exclusive(thisObject);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SLDynamicInterfaceManagementItf_ IDynamicInterfaceManagement_Itf = {
    IDynamicInterfaceManagement_AddInterface,
    IDynamicInterfaceManagement_RemoveInterface,
    IDynamicInterfaceManagement_ResumeInterface,
    IDynamicInterfaceManagement_RegisterCallback
};

void IDynamicInterfaceManagement_init(void *self)
{
    IDynamicInterfaceManagement *thiz = (IDynamicInterfaceManagement *) self;
    thiz->mItf = &IDynamicInterfaceManagement_Itf;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
}
