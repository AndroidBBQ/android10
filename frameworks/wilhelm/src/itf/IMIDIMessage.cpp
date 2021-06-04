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

/* MIDIMessage implementation */

#include "sles_allinclusive.h"


static SLresult IMIDIMessage_SendMessage(SLMIDIMessageItf self, const SLuint8 *data,
    SLuint32 length)
{
    SL_ENTER_INTERFACE

    if (NULL == data) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        //IMIDIMessage *thiz = (IMIDIMessage *) self;
        result = SL_RESULT_FEATURE_UNSUPPORTED;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMessage_RegisterMetaEventCallback(SLMIDIMessageItf self,
    slMetaEventCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IMIDIMessage *thiz = (IMIDIMessage *) self;
    interface_lock_exclusive(thiz);
    thiz->mMetaEventCallback = callback;
    thiz->mMetaEventContext = pContext;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMessage_RegisterMIDIMessageCallback(SLMIDIMessageItf self,
    slMIDIMessageCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IMIDIMessage *thiz = (IMIDIMessage *) self;
    interface_lock_exclusive(thiz);
    thiz->mMessageCallback = callback;
    thiz->mMessageContext = pContext;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMessage_AddMIDIMessageCallbackFilter(SLMIDIMessageItf self,
    SLuint32 messageType)
{
    SL_ENTER_INTERFACE

    switch (messageType) {
    case SL_MIDIMESSAGETYPE_NOTE_ON_OFF:
    case SL_MIDIMESSAGETYPE_POLY_PRESSURE:
    case SL_MIDIMESSAGETYPE_CONTROL_CHANGE:
    case SL_MIDIMESSAGETYPE_PROGRAM_CHANGE:
    case SL_MIDIMESSAGETYPE_CHANNEL_PRESSURE:
    case SL_MIDIMESSAGETYPE_PITCH_BEND:
    case SL_MIDIMESSAGETYPE_SYSTEM_MESSAGE:
        {
        SLuint8 messageTypeMask = 1 << messageType;
        IMIDIMessage *thiz = (IMIDIMessage *) self;
        interface_lock_exclusive(thiz);
        thiz->mMessageTypes |= messageTypeMask;
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


static SLresult IMIDIMessage_ClearMIDIMessageCallbackFilter(SLMIDIMessageItf self)
{
    SL_ENTER_INTERFACE

    IMIDIMessage *thiz = (IMIDIMessage *) self;
    interface_lock_exclusive(thiz);
    thiz->mMessageTypes = 0;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SLMIDIMessageItf_ IMIDIMessage_Itf = {
    IMIDIMessage_SendMessage,
    IMIDIMessage_RegisterMetaEventCallback,
    IMIDIMessage_RegisterMIDIMessageCallback,
    IMIDIMessage_AddMIDIMessageCallbackFilter,
    IMIDIMessage_ClearMIDIMessageCallbackFilter
};

void IMIDIMessage_init(void *self)
{
    IMIDIMessage *thiz = (IMIDIMessage *) self;
    thiz->mItf = &IMIDIMessage_Itf;
    thiz->mMetaEventCallback = NULL;
    thiz->mMetaEventContext = NULL;
    thiz->mMessageCallback = NULL;
    thiz->mMessageContext = NULL;
    thiz->mMessageTypes = 0;
}
