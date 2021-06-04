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

/** \file CMediaPlayer.c MediaPlayer class */

#include "sles_allinclusive.h"

#ifdef ANDROID
#include "android/android_GenericMediaPlayer.h"
using namespace android;
#endif


XAresult CMediaPlayer_Realize(void *self, XAboolean async)
{
    XAresult result = XA_RESULT_SUCCESS;

#ifdef ANDROID
    CMediaPlayer *thiz = (CMediaPlayer *) self;

    // realize player
    result = android_Player_realize(thiz, async);
#endif

    return result;
}

XAresult CMediaPlayer_Resume(void *self, XAboolean async)
{
    return XA_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Destroy when a media player is destroyed */

void CMediaPlayer_Destroy(void *self)
{
    CMediaPlayer *thiz = (CMediaPlayer *) self;
    freeDataLocatorFormat(&thiz->mDataSource);
    freeDataLocatorFormat(&thiz->mBankSource);
    freeDataLocatorFormat(&thiz->mAudioSink);
    freeDataLocatorFormat(&thiz->mImageVideoSink);
    freeDataLocatorFormat(&thiz->mVibraSink);
    freeDataLocatorFormat(&thiz->mLEDArraySink);
#ifdef ANDROID
    android_Player_destroy(thiz);
#endif
}


predestroy_t CMediaPlayer_PreDestroy(void *self)
{
#ifdef ANDROID
    CMediaPlayer *thiz = (CMediaPlayer *) self;
    android_Player_preDestroy(thiz);
#endif
    return predestroy_ok;
}
