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

/* \file COutputMix.c OutputMix class */

#include "sles_allinclusive.h"


/** \brief Hook called by Object::Realize when an output mix is realized */

SLresult COutputMix_Realize(void *self, SLboolean async)
{
    SLresult result = SL_RESULT_SUCCESS;

#ifdef ANDROID
    COutputMix *thiz = (COutputMix *) self;
    result = android_outputMix_realize(thiz, async);
#endif

    return result;
}


/** \brief Hook called by Object::Resume when an output mix is resumed */

SLresult COutputMix_Resume(void *self, SLboolean async)
{
    return SL_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Destroy when an output mix is destroyed */

void COutputMix_Destroy(void *self)
{
#ifdef ANDROID
    COutputMix *thiz = (COutputMix *) self;
    android_outputMix_destroy(thiz);
#endif
}


/** \brief Hook called by Object::Destroy before an output mix is about to be destroyed */

predestroy_t COutputMix_PreDestroy(void *self)
{
    // Ignore destroy requests if there are any players attached to this output mix
    COutputMix *outputMix = (COutputMix *) self;
    // See design document for explanation
    if (0 == outputMix->mObject.mStrongRefCount) {
#ifdef USE_OUTPUTMIXEXT
        // We only support a single active output mix per engine, so check if this is the active mix
        IEngine *thisEngine = &outputMix->mObject.mEngine->mEngine;
        interface_lock_exclusive(thisEngine);
        bool thisIsTheActiveOutputMix = false;
        if (outputMix == thisEngine->mOutputMix) {
            thisIsTheActiveOutputMix = true;
        }
        interface_unlock_exclusive(thisEngine);
        if (thisIsTheActiveOutputMix) {
            // Tell the asynchronous mixer callback that we want to destroy the output mix
            outputMix->mOutputMixExt.mDestroyRequested = true;
            while (outputMix->mOutputMixExt.mDestroyRequested) {
                object_cond_wait(&outputMix->mObject);
            }
#ifdef USE_SDL
            // Mixer callback has acknowledged our request and unlinked output mix from engine.
            // Disable SDL_callback from being called periodically by SDL's internal thread.
            SDL_PauseAudio(1);
#endif
        }
#endif
        return predestroy_ok;
    }
    SL_LOGE("Object::Destroy(%p) for OutputMix ignored; %u players attached", outputMix,
        outputMix->mObject.mStrongRefCount);
    return predestroy_error;
}
