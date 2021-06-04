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

/** \file CEngine.c Engine class */

#include "sles_allinclusive.h"
#ifdef ANDROID
#include <binder/IServiceManager.h>
#include <utils/StrongPointer.h>
#include <audiomanager/AudioManager.h>
#include <audiomanager/IAudioManager.h>
#endif

/* This implementation supports at most one engine, identical for both OpenSL ES and OpenMAX AL */

CEngine *theOneTrueEngine = NULL;
pthread_mutex_t theOneTrueMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned theOneTrueRefCount = 0;
// incremented by slCreateEngine or xaCreateEngine, decremented by Object::Destroy on engine


/** \brief Called by dlopen when .so is loaded */

__attribute__((constructor)) static void onDlOpen(void)
{
}


/** \brief Called by dlclose when .so is unloaded */

__attribute__((destructor)) static void onDlClose(void)
{
    // a memory barrier would be sufficient, but the mutex is easier
    (void) pthread_mutex_lock(&theOneTrueMutex);
    if ((NULL != theOneTrueEngine) || (0 < theOneTrueRefCount)) {
        SL_LOGE("Object::Destroy omitted for engine %p", theOneTrueEngine);
    }
    (void) pthread_mutex_unlock(&theOneTrueMutex);
}


/** \brief Hook called by Object::Realize when an engine is realized */

SLresult CEngine_Realize(void *self, SLboolean async)
{
    CEngine *thiz = (CEngine *) self;
    SLresult result;
#ifndef ANDROID
    // create the sync thread
    int err = pthread_create(&thiz->mSyncThread, (const pthread_attr_t *) NULL, sync_start, thiz);
    result = err_to_result(err);
    if (SL_RESULT_SUCCESS != result)
        return result;
#endif
    // initialize the thread pool for asynchronous operations
    result = ThreadPool_init(&thiz->mThreadPool, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        thiz->mEngine.mShutdown = SL_BOOLEAN_TRUE;
        (void) pthread_join(thiz->mSyncThread, (void **) NULL);
        return result;
    }
#ifdef ANDROID
    // use checkService() to avoid blocking if audio service is not up yet
    android::sp<android::IBinder> binder =
            android::defaultServiceManager()->checkService(android::String16("audio"));
    if (binder == 0) {
        SL_LOGE("CEngine_Realize: binding to audio service failed, service up?");
    } else {
        thiz->mAudioManager = android::interface_cast<android::IAudioManager>(binder);
    }
#endif
#ifdef USE_SDL
    SDL_open(&thiz->mEngine);
#endif
    return SL_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Resume when an engine is resumed */

SLresult CEngine_Resume(void *self, SLboolean async)
{
    return SL_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Destroy when an engine is destroyed */

void CEngine_Destroy(void *self)
{
    CEngine *thiz = (CEngine *) self;

    // Verify that there are no extant objects
    unsigned instanceCount = thiz->mEngine.mInstanceCount;
    unsigned instanceMask = thiz->mEngine.mInstanceMask;
    if ((0 < instanceCount) || (0 != instanceMask)) {
        SL_LOGE("Object::Destroy(%p) for engine ignored; %u total active objects",
            thiz, instanceCount);
        while (0 != instanceMask) {
            unsigned i = ctz(instanceMask);
            assert(MAX_INSTANCE > i);
            SL_LOGE("Object::Destroy(%p) for engine ignored; active object ID %u at %p",
                thiz, i + 1, thiz->mEngine.mInstances[i]);
            instanceMask &= ~(1 << i);
        }
    }

    // If engine was created but not realized, there will be no sync thread yet
    pthread_t zero;
    memset(&zero, 0, sizeof(pthread_t));
    if (0 != memcmp(&zero, &thiz->mSyncThread, sizeof(pthread_t))) {

        // Announce to the sync thread that engine is shutting down; it polls so should see it soon
        thiz->mEngine.mShutdown = SL_BOOLEAN_TRUE;
        // Wait for the sync thread to acknowledge the shutdown
        while (!thiz->mEngine.mShutdownAck) {
            object_cond_wait(&thiz->mObject);
        }
        // The sync thread should have exited by now, so collect it by joining
        (void) pthread_join(thiz->mSyncThread, (void **) NULL);

    }

    // Shutdown the thread pool used for asynchronous operations (there should not be any)
    ThreadPool_deinit(&thiz->mThreadPool);

#if defined(ANDROID)
    if (thiz->mAudioManager != 0) {
        thiz->mAudioManager.clear();
    }

    // free equalizer preset names
    if (NULL != thiz->mEqPresetNames) {
        for (unsigned i = 0; i < thiz->mEqNumPresets; ++i) {
            if (NULL != thiz->mEqPresetNames[i]) {
                delete[] thiz->mEqPresetNames[i];
                thiz->mEqPresetNames[i] = NULL;
            }
        }
        delete[] thiz->mEqPresetNames;
        thiz->mEqPresetNames = NULL;
    }
    thiz->mEqNumPresets = 0;
#endif

#ifdef USE_SDL
    SDL_close();
#endif

}


/** \brief Hook called by Object::Destroy before an engine is about to be destroyed */

predestroy_t CEngine_PreDestroy(void *self)
{
    predestroy_t ret;
    (void) pthread_mutex_lock(&theOneTrueMutex);
    assert(self == theOneTrueEngine);
    switch (theOneTrueRefCount) {
    case 0:
        assert(false);
        ret = predestroy_error;
        break;
    case 1:
        ret = predestroy_ok;
        break;
    default:
        --theOneTrueRefCount;
        ret = predestroy_again;
        break;
    }
    (void) pthread_mutex_unlock(&theOneTrueMutex);
    return ret;
}


/** \brief Called by IObject::Destroy after engine is destroyed. The parameter refers to the
 *  previous engine, which is now undefined memory.
 */

void CEngine_Destroyed(CEngine *self)
{
    int ok;
    ok = pthread_mutex_lock(&theOneTrueMutex);
    assert(0 == ok);
    assert(self == theOneTrueEngine);
    theOneTrueEngine = NULL;
    assert(1 == theOneTrueRefCount);
    theOneTrueRefCount = 0;
    ok = pthread_mutex_unlock(&theOneTrueMutex);
    assert(0 == ok);
}
