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

/* Initial global entry points */

#include "sles_allinclusive.h"

#ifdef ANDROID
#include <binder/ProcessState.h>
#endif

/** \brief Internal code shared by slCreateEngine and xaCreateEngine */

LI_API SLresult liCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
    const SLEngineOption *pEngineOptions, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired,
    const ClassTable *pCEngine_class)
{
    SLresult result;

    int ok;
    ok = pthread_mutex_lock(&theOneTrueMutex);
    assert(0 == ok);
    bool needToUnlockTheOneTrueMutex = true;

    do {

        if (NULL == pEngine) {
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }
        *pEngine = NULL;

        if ((0 < numOptions) && (NULL == pEngineOptions)) {
            SL_LOGE("numOptions=%u and pEngineOptions=NULL", numOptions);
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }

        // default values
        SLboolean threadSafe = SL_BOOLEAN_TRUE;
        SLboolean lossOfControlGlobal = SL_BOOLEAN_FALSE;

        // process engine options
        SLuint32 i;
        const SLEngineOption *option = pEngineOptions;
        result = SL_RESULT_SUCCESS;
        for (i = 0; i < numOptions; ++i, ++option) {
            switch (option->feature) {
            case SL_ENGINEOPTION_THREADSAFE:
                threadSafe = SL_BOOLEAN_FALSE != (SLboolean) option->data; // normalize
                break;
            case SL_ENGINEOPTION_LOSSOFCONTROL:
                lossOfControlGlobal = SL_BOOLEAN_FALSE != (SLboolean) option->data; // normalize
                break;
            default:
                SL_LOGE("unknown engine option: feature=%u data=%u",
                    option->feature, option->data);
                result = SL_RESULT_PARAMETER_INVALID;
                break;
            }
        }
        if (SL_RESULT_SUCCESS != result) {
            break;
        }

        unsigned exposedMask;
        assert(NULL != pCEngine_class);
        result = checkInterfaces(pCEngine_class, numInterfaces,
            pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        if (SL_RESULT_SUCCESS != result) {
            break;
        }

        // if an engine already exists, then increment its ref count
        CEngine *thiz = theOneTrueEngine;
        if (NULL != thiz) {
            assert(0 < theOneTrueRefCount);
            ++theOneTrueRefCount;

            // In order to update the engine object, we need to lock it,
            // but that would violate the lock order and potentially deadlock.
            // So we unlock now and note that it should not be unlocked later.
            ok = pthread_mutex_unlock(&theOneTrueMutex);
            assert(0 == ok);
            needToUnlockTheOneTrueMutex = false;
            object_lock_exclusive(&thiz->mObject);

            // now expose additional interfaces not requested by the earlier engine create
            const struct iid_vtable *x = pCEngine_class->mInterfaces;
            SLuint8 *interfaceStateP = thiz->mObject.mInterfaceStates;
            SLuint32 index;
            for (index = 0; index < pCEngine_class->mInterfaceCount; ++index, ++x,
                    exposedMask >>= 1, ++interfaceStateP) {
                switch (*interfaceStateP) {
                case INTERFACE_EXPOSED:         // previously exposed
                    break;
                case INTERFACE_INITIALIZED:     // not exposed during the earlier create
                    if (exposedMask & 1) {
                        const struct MPH_init *mi = &MPH_init_table[x->mMPH];
                        BoolHook expose = mi->mExpose;
                        if ((NULL == expose) || (*expose)((char *) thiz + x->mOffset)) {
                            *interfaceStateP = INTERFACE_EXPOSED;
                        }
                        // FIXME log or report to application that expose hook failed
                    }
                    break;
                case INTERFACE_UNINITIALIZED:   // no init hook
                    break;
                default:                        // impossible
                    assert(false);
                    break;
                }
            }
            object_unlock_exclusive(&thiz->mObject);
            // return the shared engine object
            *pEngine = &thiz->mObject.mItf;
            break;
        }

        // here when creating the first engine reference
        assert(0 == theOneTrueRefCount);

#ifdef ANDROID
        android::ProcessState::self()->startThreadPool();
#endif

        thiz = (CEngine *) construct(pCEngine_class, exposedMask, NULL);
        if (NULL == thiz) {
            result = SL_RESULT_MEMORY_FAILURE;
            break;
        }

        // initialize fields not associated with an interface
        // mThreadPool is initialized in CEngine_Realize
        memset(&thiz->mThreadPool, 0, sizeof(ThreadPool));
        memset(&thiz->mSyncThread, 0, sizeof(pthread_t));
#if defined(ANDROID)
        thiz->mEqNumPresets = 0;
        thiz->mEqPresetNames = NULL;
#endif
        // initialize fields related to an interface
        thiz->mObject.mLossOfControlMask = lossOfControlGlobal ? ~0 : 0;
        thiz->mEngine.mLossOfControlGlobal = lossOfControlGlobal;
        thiz->mEngineCapabilities.mThreadSafe = threadSafe;
        IObject_Publish(&thiz->mObject);
        theOneTrueEngine = thiz;
        theOneTrueRefCount = 1;
        // return the new engine object
        *pEngine = &thiz->mObject.mItf;

    } while(0);

    if (needToUnlockTheOneTrueMutex) {
        ok = pthread_mutex_unlock(&theOneTrueMutex);
        assert(0 == ok);
    }

    return result;
}


/** Internal function for slQuerySupportedEngineInterfaces and xaQuerySupportedEngineInterfaces */

LI_API SLresult liQueryNumSupportedInterfaces(SLuint32 *pNumSupportedInterfaces,
        const ClassTable *clazz)
{
    SLresult result;
    if (NULL == pNumSupportedInterfaces) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        assert(NULL != clazz);
        SLuint32 count = 0;
        SLuint32 i;
        for (i = 0; i < clazz->mInterfaceCount; ++i) {
            switch (clazz->mInterfaces[i].mInterface) {
            case INTERFACE_IMPLICIT:
            case INTERFACE_IMPLICIT_PREREALIZE:
            case INTERFACE_EXPLICIT:
            case INTERFACE_EXPLICIT_PREREALIZE:
            case INTERFACE_DYNAMIC:
                ++count;
                break;
            case INTERFACE_UNAVAILABLE:
                break;
            default:
                assert(false);
                break;
            }
        }
        *pNumSupportedInterfaces = count;
        result = SL_RESULT_SUCCESS;
    }
    return result;
}


/** Internal function for slQuerySupportedEngineInterfaces and xaQuerySupportedEngineInterfaces */

LI_API SLresult liQuerySupportedInterfaces(SLuint32 index, SLInterfaceID *pInterfaceId,
        const ClassTable *clazz)
{
    SLresult result;
    if (NULL == pInterfaceId) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pInterfaceId = NULL;
        assert(NULL != clazz);
        result = SL_RESULT_PARAMETER_INVALID;   // will be reset later
        SLuint32 i;
        for (i = 0; i < clazz->mInterfaceCount; ++i) {
            switch (clazz->mInterfaces[i].mInterface) {
            case INTERFACE_IMPLICIT:
            case INTERFACE_IMPLICIT_PREREALIZE:
            case INTERFACE_EXPLICIT:
            case INTERFACE_EXPLICIT_PREREALIZE:
            case INTERFACE_DYNAMIC:
                break;
            case INTERFACE_UNAVAILABLE:
                continue;
            default:
                assert(false);
                break;
            }
            if (index == 0) {
                *pInterfaceId = &SL_IID_array[clazz->mInterfaces[i].mMPH];
                result = SL_RESULT_SUCCESS;
                break;
            }
            --index;
        }
    }
    return result;
}
