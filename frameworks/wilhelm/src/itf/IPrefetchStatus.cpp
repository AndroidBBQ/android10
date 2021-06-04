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

/* PrefetchStatus implementation */

#include "sles_allinclusive.h"


static SLresult IPrefetchStatus_GetPrefetchStatus(SLPrefetchStatusItf self, SLuint32 *pStatus)
{
    SL_ENTER_INTERFACE

    if (NULL == pStatus) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_shared(thiz);
        SLuint32 status = thiz->mStatus;
        interface_unlock_shared(thiz);
        *pStatus = status;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_GetFillLevel(SLPrefetchStatusItf self, SLpermille *pLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_shared(thiz);
        SLpermille level = thiz->mLevel;
        interface_unlock_shared(thiz);
        *pLevel = level;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_RegisterCallback(SLPrefetchStatusItf self,
    slPrefetchCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IPrefetchStatus *thiz = (IPrefetchStatus *) self;
    interface_lock_exclusive(thiz);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_SetCallbackEventsMask(SLPrefetchStatusItf self, SLuint32 eventFlags)
{
    SL_ENTER_INTERFACE

    if (eventFlags & ~(SL_PREFETCHEVENT_STATUSCHANGE | SL_PREFETCHEVENT_FILLLEVELCHANGE)) {
        result = SL_RESULT_PARAMETER_INVALID;

    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_exclusive(thiz);
        thiz->mCallbackEventsMask = eventFlags;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;

    }

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_GetCallbackEventsMask(SLPrefetchStatusItf self,
    SLuint32 *pEventFlags)
{
    SL_ENTER_INTERFACE

    if (NULL == pEventFlags) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_shared(thiz);
        SLuint32 callbackEventsMask = thiz->mCallbackEventsMask;
        interface_unlock_shared(thiz);
        *pEventFlags = callbackEventsMask;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_SetFillUpdatePeriod(SLPrefetchStatusItf self, SLpermille period)
{
    SL_ENTER_INTERFACE

    if (0 == period) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_exclusive(thiz);
        thiz->mFillUpdatePeriod = period;
#ifdef ANDROID
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
            CAudioPlayer *ap = (CAudioPlayer *) thiz->mThis;
            android_audioPlayer_setBufferingUpdateThresholdPerMille(ap, period);
        }
#endif
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPrefetchStatus_GetFillUpdatePeriod(SLPrefetchStatusItf self, SLpermille *pPeriod)
{
    SL_ENTER_INTERFACE

    if (NULL == pPeriod) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPrefetchStatus *thiz = (IPrefetchStatus *) self;
        interface_lock_shared(thiz);
        SLpermille fillUpdatePeriod = thiz->mFillUpdatePeriod;
        interface_unlock_shared(thiz);
        *pPeriod = fillUpdatePeriod;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLPrefetchStatusItf_ IPrefetchStatus_Itf = {
    IPrefetchStatus_GetPrefetchStatus,
    IPrefetchStatus_GetFillLevel,
    IPrefetchStatus_RegisterCallback,
    IPrefetchStatus_SetCallbackEventsMask,
    IPrefetchStatus_GetCallbackEventsMask,
    IPrefetchStatus_SetFillUpdatePeriod,
    IPrefetchStatus_GetFillUpdatePeriod
};

void IPrefetchStatus_init(void *self)
{
    IPrefetchStatus *thiz = (IPrefetchStatus *) self;
    thiz->mItf = &IPrefetchStatus_Itf;
    thiz->mStatus = SL_PREFETCHSTATUS_UNDERFLOW;
    thiz->mLevel = 0;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mCallbackEventsMask = 0;
    thiz->mFillUpdatePeriod = 100;
#ifdef ANDROID
    thiz->mDeferredPrefetchCallback = NULL;
    thiz->mDeferredPrefetchContext  = NULL;
    thiz->mDeferredPrefetchEvents   = SL_PREFETCHEVENT_NONE;
#endif
}
