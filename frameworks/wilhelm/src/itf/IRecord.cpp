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

/* Record implementation */

#include "sles_allinclusive.h"


static SLresult IRecord_SetRecordState(SLRecordItf self, SLuint32 state)
{
    SL_ENTER_INTERFACE

    switch (state) {
    case SL_RECORDSTATE_STOPPED:
    case SL_RECORDSTATE_PAUSED:
    case SL_RECORDSTATE_RECORDING:
        {
        IRecord *thiz = (IRecord *) self;
        interface_lock_exclusive(thiz);
        thiz->mState = state;
#ifdef ANDROID
        android_audioRecorder_setRecordState(InterfaceToCAudioRecorder(thiz), state);
#endif
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
        }
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_GetRecordState(SLRecordItf self, SLuint32 *pState)
{
    SL_ENTER_INTERFACE

    IRecord *thiz = (IRecord *) self;
    if (NULL == pState) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_shared(thiz);
        SLuint32 state = thiz->mState;
        interface_unlock_shared(thiz);
        *pState = state;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_SetDurationLimit(SLRecordItf self, SLmillisecond msec)
{
    SL_ENTER_INTERFACE

    IRecord *thiz = (IRecord *) self;
    interface_lock_exclusive(thiz);
    if (thiz->mDurationLimit != msec) {
        thiz->mDurationLimit = msec;
        interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
    } else {
        interface_unlock_exclusive(thiz);
    }
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_GetPosition(SLRecordItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        SLmillisecond position;
        interface_lock_shared(thiz);
#ifdef ANDROID
        // Android does not use the mPosition field for audio recorders
        if (SL_OBJECTID_AUDIORECORDER == InterfaceToObjectID(thiz)) {
            android_audioRecorder_getPosition(InterfaceToCAudioRecorder(thiz), &position);
        } else {
            position = thiz->mPosition;
        }
#else
        position = thiz->mPosition;
#endif
        interface_unlock_shared(thiz);
        *pMsec = position;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_RegisterCallback(SLRecordItf self, slRecordCallback callback,
    void *pContext)
{
    SL_ENTER_INTERFACE

    IRecord *thiz = (IRecord *) self;
    interface_lock_exclusive(thiz);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_SetCallbackEventsMask(SLRecordItf self, SLuint32 eventFlags)
{
    SL_ENTER_INTERFACE

    if (eventFlags & ~(
        SL_RECORDEVENT_HEADATLIMIT  |
        SL_RECORDEVENT_HEADATMARKER |
        SL_RECORDEVENT_HEADATNEWPOS |
        SL_RECORDEVENT_HEADMOVING   |
        SL_RECORDEVENT_HEADSTALLED  |
        SL_RECORDEVENT_BUFFER_FULL)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        interface_lock_exclusive(thiz);
        if (thiz->mCallbackEventsMask != eventFlags) {
            thiz->mCallbackEventsMask = eventFlags;
            interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_GetCallbackEventsMask(SLRecordItf self, SLuint32 *pEventFlags)
{
    SL_ENTER_INTERFACE

    if (NULL == pEventFlags) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        interface_lock_shared(thiz);
        SLuint32 callbackEventsMask = thiz->mCallbackEventsMask;
        interface_unlock_shared(thiz);
        *pEventFlags = callbackEventsMask;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_SetMarkerPosition(SLRecordItf self, SLmillisecond mSec)
{
    SL_ENTER_INTERFACE

    if (SL_TIME_UNKNOWN == mSec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        bool significant = false;
        interface_lock_exclusive(thiz);
        if (thiz->mMarkerPosition != mSec) {
            thiz->mMarkerPosition = mSec;
            if (thiz->mCallbackEventsMask & SL_PLAYEVENT_HEADATMARKER) {
                significant = true;
            }
        }
        if (significant) {
            interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_ClearMarkerPosition(SLRecordItf self)
{
    SL_ENTER_INTERFACE

    IRecord *thiz = (IRecord *) self;
    bool significant = false;
    interface_lock_exclusive(thiz);
    // clearing the marker position is equivalent to setting the marker to SL_TIME_UNKNOWN
    if (thiz->mMarkerPosition != SL_TIME_UNKNOWN) {
        thiz->mMarkerPosition = SL_TIME_UNKNOWN;
        if (thiz->mCallbackEventsMask & SL_PLAYEVENT_HEADATMARKER) {
            significant = true;
        }
    }
    if (significant) {
        interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
    } else {
        interface_unlock_exclusive(thiz);
    }
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_GetMarkerPosition(SLRecordItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        interface_lock_shared(thiz);
        SLmillisecond markerPosition = thiz->mMarkerPosition;
        interface_unlock_shared(thiz);
        *pMsec = markerPosition;
        if (SL_TIME_UNKNOWN == markerPosition) {
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        } else {
            result = SL_RESULT_SUCCESS;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_SetPositionUpdatePeriod(SLRecordItf self, SLmillisecond mSec)
{
    SL_ENTER_INTERFACE

    if (0 == mSec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        interface_lock_exclusive(thiz);
        if (thiz->mPositionUpdatePeriod != mSec) {
            thiz->mPositionUpdatePeriod = mSec;
            interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IRecord_GetPositionUpdatePeriod(SLRecordItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IRecord *thiz = (IRecord *) self;
        interface_lock_shared(thiz);
        SLmillisecond positionUpdatePeriod = thiz->mPositionUpdatePeriod;
        interface_unlock_shared(thiz);
        *pMsec = positionUpdatePeriod;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLRecordItf_ IRecord_Itf = {
    IRecord_SetRecordState,
    IRecord_GetRecordState,
    IRecord_SetDurationLimit,
    IRecord_GetPosition,
    IRecord_RegisterCallback,
    IRecord_SetCallbackEventsMask,
    IRecord_GetCallbackEventsMask,
    IRecord_SetMarkerPosition,
    IRecord_ClearMarkerPosition,
    IRecord_GetMarkerPosition,
    IRecord_SetPositionUpdatePeriod,
    IRecord_GetPositionUpdatePeriod
};

void IRecord_init(void *self)
{
    IRecord *thiz = (IRecord *) self;
    thiz->mItf = &IRecord_Itf;
    thiz->mState = SL_RECORDSTATE_STOPPED;
    thiz->mDurationLimit = 0;
    thiz->mPosition = (SLmillisecond) 0;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mCallbackEventsMask = 0;
    thiz->mMarkerPosition = SL_TIME_UNKNOWN;
    thiz->mPositionUpdatePeriod = 1000; // per spec
}
