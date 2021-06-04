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

/** \file CAudioPlayer.c AudioPlayer class */

#include "sles_allinclusive.h"


/** \brief Hook called by Object::Realize when an audio player is realized */

SLresult CAudioPlayer_Realize(void *self, SLboolean async)
{
    CAudioPlayer *thiz = (CAudioPlayer *) self;
    SLresult result = SL_RESULT_SUCCESS;

#ifdef ANDROID
    result = android_audioPlayer_realize(thiz, async);
#endif

#ifdef USE_SNDFILE
    result = SndFile_Realize(thiz);
#endif

    // At this point the channel count and sample rate might still be unknown,
    // depending on the data source and the platform implementation.
    // If they are unknown here, then they will be determined during prefetch.

    return result;
}


/** \brief Hook called by Object::Resume when an audio player is resumed */

SLresult CAudioPlayer_Resume(void *self, SLboolean async)
{
    return SL_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Destroy when an audio player is destroyed */

void CAudioPlayer_Destroy(void *self)
{
    CAudioPlayer *thiz = (CAudioPlayer *) self;
#ifdef ANDROID
    android_audioPlayer_destroy(thiz);
#endif
    freeDataLocatorFormat(&thiz->mDataSource);
    freeDataLocatorFormat(&thiz->mDataSink);
#ifdef USE_SNDFILE
    SndFile_Destroy(thiz);
#endif
}


/** \brief Hook called by Object::Destroy before an audio player is about to be destroyed */

predestroy_t CAudioPlayer_PreDestroy(void *self)
{
    CAudioPlayer *thiz = (CAudioPlayer *) self;
#ifdef ANDROID
    android_audioPlayer_preDestroy(thiz);
#endif

#ifdef USE_OUTPUTMIXEXT
    // Safe to proceed immediately if a track has not yet been assigned
    Track *track = thiz->mTrack;
    if (NULL == track) {
        return predestroy_ok;
    }
    CAudioPlayer *audioPlayer = track->mAudioPlayer;
    if (NULL == audioPlayer) {
        return predestroy_ok;
    }
    assert(audioPlayer == thiz);
    // Request the mixer thread to unlink this audio player's track
    thiz->mDestroyRequested = true;
    while (thiz->mDestroyRequested) {
        object_cond_wait(self);
    }
    // Mixer thread has acknowledged the request
#endif
    return predestroy_ok;
}


/** \brief Given an audio player, return its data sink, which is guaranteed to be a non-NULL output
 *  mix.  This function is used by effect send.
 */

COutputMix *CAudioPlayer_GetOutputMix(CAudioPlayer *audioPlayer)
{
    assert(NULL != audioPlayer);
    assert(SL_DATALOCATOR_OUTPUTMIX == audioPlayer->mDataSink.mLocator.mLocatorType);
    SLObjectItf outputMix = audioPlayer->mDataSink.mLocator.mOutputMix.outputMix;
    assert(NULL != outputMix);
    return (COutputMix *) outputMix;
}
