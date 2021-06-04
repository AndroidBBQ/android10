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

/* MIDITempo implementation */

#include "sles_allinclusive.h"


static SLresult IMIDITempo_SetTicksPerQuarterNote(SLMIDITempoItf self, SLuint32 tpqn)
{
    SL_ENTER_INTERFACE

    if (!(1 <= tpqn && tpqn <= 32767)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDITempo *thiz = (IMIDITempo *) self;
        interface_lock_poke(thiz);
        thiz->mTicksPerQuarterNote = tpqn;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDITempo_GetTicksPerQuarterNote(SLMIDITempoItf self, SLuint32 *pTpqn)
{
    SL_ENTER_INTERFACE

    if (NULL == pTpqn) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDITempo *thiz = (IMIDITempo *) self;
        interface_lock_peek(thiz);
        SLuint32 ticksPerQuarterNote = thiz->mTicksPerQuarterNote;
        interface_unlock_peek(thiz);
        *pTpqn = ticksPerQuarterNote;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDITempo_SetMicrosecondsPerQuarterNote(SLMIDITempoItf self, SLmicrosecond uspqn)
{
    SL_ENTER_INTERFACE

    // spec says zero, is that correct?
    if (!(1 <= uspqn && uspqn <= 16777215)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDITempo *thiz = (IMIDITempo *) self;
        interface_lock_poke(thiz);
        thiz->mMicrosecondsPerQuarterNote = uspqn;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMIDITempo_GetMicrosecondsPerQuarterNote(SLMIDITempoItf self, SLmicrosecond *uspqn)
{
    SL_ENTER_INTERFACE

    if (NULL == uspqn) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMIDITempo *thiz = (IMIDITempo *) self;
        interface_lock_peek(thiz);
        SLuint32 microsecondsPerQuarterNote = thiz->mMicrosecondsPerQuarterNote;
        interface_unlock_peek(thiz);
        *uspqn = microsecondsPerQuarterNote;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLMIDITempoItf_ IMIDITempo_Itf = {
    IMIDITempo_SetTicksPerQuarterNote,
    IMIDITempo_GetTicksPerQuarterNote,
    IMIDITempo_SetMicrosecondsPerQuarterNote,
    IMIDITempo_GetMicrosecondsPerQuarterNote
};

void IMIDITempo_init(void *self)
{
    IMIDITempo *thiz = (IMIDITempo *) self;
    thiz->mItf = &IMIDITempo_Itf;
    thiz->mTicksPerQuarterNote = 32; // wrong
    thiz->mMicrosecondsPerQuarterNote = 100; // wrong
}
