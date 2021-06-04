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

/* MIDIMuteSolo implementation */

#include "sles_allinclusive.h"


static SLresult IMIDIMuteSolo_SetChannelMute(SLMIDIMuteSoloItf self, SLuint8 channel,
    SLboolean mute)
{
    SL_ENTER_INTERFACE

    if (channel > 15) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
        SLuint16 mask = 1 << channel;
        interface_lock_exclusive(thiz);
        if (mute)
            thiz->mChannelMuteMask |= mask;
        else
            thiz->mChannelMuteMask &= ~mask;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_GetChannelMute(SLMIDIMuteSoloItf self, SLuint8 channel,
    SLboolean *pMute)
{
    SL_ENTER_INTERFACE

    if (channel > 15 || (NULL == pMute)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
        interface_lock_peek(thiz);
        SLuint16 mask = thiz->mChannelMuteMask;
        interface_unlock_peek(thiz);
        *pMute = (mask >> channel) & 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_SetChannelSolo(SLMIDIMuteSoloItf self, SLuint8 channel,
    SLboolean solo)
{
    SL_ENTER_INTERFACE

    if (channel > 15) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
        SLuint16 mask = 1 << channel;
        interface_lock_exclusive(thiz);
        if (solo)
            thiz->mChannelSoloMask |= mask;
        else
            thiz->mChannelSoloMask &= ~mask;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_GetChannelSolo(SLMIDIMuteSoloItf self, SLuint8 channel,
    SLboolean *pSolo)
{
    SL_ENTER_INTERFACE

    if (channel > 15 || (NULL == pSolo)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
        interface_lock_peek(thiz);
        SLuint16 mask = thiz->mChannelSoloMask;
        interface_unlock_peek(thiz);
        *pSolo = (mask >> channel) & 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_GetTrackCount(SLMIDIMuteSoloItf self, SLuint16 *pCount)
{
    SL_ENTER_INTERFACE

    if (NULL == pCount) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
        // const, so no lock needed
        SLuint16 trackCount = thiz->mTrackCount;
        *pCount = trackCount;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_SetTrackMute(SLMIDIMuteSoloItf self, SLuint16 track, SLboolean mute)
{
    SL_ENTER_INTERFACE

    IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
    // const
    if (!(track < thiz->mTrackCount)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        SLuint32 mask = 1 << track;
        interface_lock_exclusive(thiz);
        if (mute)
            thiz->mTrackMuteMask |= mask;
        else
            thiz->mTrackMuteMask &= ~mask;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_GetTrackMute(SLMIDIMuteSoloItf self, SLuint16 track, SLboolean *pMute)
{
    SL_ENTER_INTERFACE

    IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
    // const, no lock needed
    if (!(track < thiz->mTrackCount) || NULL == pMute) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_peek(thiz);
        SLuint32 mask = thiz->mTrackMuteMask;
        interface_unlock_peek(thiz);
        *pMute = (mask >> track) & 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_SetTrackSolo(SLMIDIMuteSoloItf self, SLuint16 track, SLboolean solo)
{
    SL_ENTER_INTERFACE

    IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
    // const
    if (!(track < thiz->mTrackCount)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        SLuint32 mask = 1 << track; interface_lock_exclusive(thiz);
        if (solo)
            thiz->mTrackSoloMask |= mask;
        else
            thiz->mTrackSoloMask &= ~mask;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDIMuteSolo_GetTrackSolo(SLMIDIMuteSoloItf self, SLuint16 track, SLboolean *pSolo)
{
    SL_ENTER_INTERFACE

    IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
    // const, no lock needed
    if (!(track < thiz->mTrackCount) || NULL == pSolo) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        interface_lock_peek(thiz);
        SLuint32 mask = thiz->mTrackSoloMask;
        interface_unlock_peek(thiz);
        *pSolo = (mask >> track) & 1;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLMIDIMuteSoloItf_ IMIDIMuteSolo_Itf = {
    IMIDIMuteSolo_SetChannelMute,
    IMIDIMuteSolo_GetChannelMute,
    IMIDIMuteSolo_SetChannelSolo,
    IMIDIMuteSolo_GetChannelSolo,
    IMIDIMuteSolo_GetTrackCount,
    IMIDIMuteSolo_SetTrackMute,
    IMIDIMuteSolo_GetTrackMute,
    IMIDIMuteSolo_SetTrackSolo,
    IMIDIMuteSolo_GetTrackSolo
};

void IMIDIMuteSolo_init(void *self)
{
    IMIDIMuteSolo *thiz = (IMIDIMuteSolo *) self;
    thiz->mItf = &IMIDIMuteSolo_Itf;
    thiz->mChannelMuteMask = 0;
    thiz->mChannelSoloMask = 0;
    thiz->mTrackMuteMask = 0;
    thiz->mTrackSoloMask = 0;
    // const
    thiz->mTrackCount = 32; // wrong
}
