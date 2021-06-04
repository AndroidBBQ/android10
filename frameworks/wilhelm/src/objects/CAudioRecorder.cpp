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

/* \file CAudioRecorder.c AudioRecorder class */

#include "sles_allinclusive.h"


/** \brief Hook called by Object::Realize when an audio recorder is realized */

SLresult CAudioRecorder_Realize(void *self, SLboolean async)
{
    SLresult result = SL_RESULT_SUCCESS;

#ifdef ANDROID
    CAudioRecorder *thiz = (CAudioRecorder *) self;
    result = android_audioRecorder_realize(thiz, async);
#endif

    return result;
}


/** \brief Hook called by Object::Resume when an audio recorder is resumed */

SLresult CAudioRecorder_Resume(void *self, SLboolean async)
{
    return SL_RESULT_SUCCESS;
}


/** \brief Hook called by Object::Destroy when an audio recorder is destroyed */

void CAudioRecorder_Destroy(void *self)
{
    CAudioRecorder *thiz = (CAudioRecorder *) self;
    freeDataLocatorFormat(&thiz->mDataSource);
    freeDataLocatorFormat(&thiz->mDataSink);
#ifdef ANDROID
    android_audioRecorder_destroy(thiz);
#endif
}


/** \brief Hook called by Object::Destroy before an audio recorder is about to be destroyed */

predestroy_t CAudioRecorder_PreDestroy(void *self)
{
    CAudioRecorder *thiz = (CAudioRecorder *) self;
#ifdef ANDROID
    android_audioRecorder_preDestroy(thiz);
#endif
    return predestroy_ok;
}
