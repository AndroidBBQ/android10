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

/* AudioEncoder implementation */

#include "sles_allinclusive.h"


static SLresult IAudioEncoder_SetEncoderSettings(SLAudioEncoderItf self,
    SLAudioEncoderSettings  *pSettings)
{
    SL_ENTER_INTERFACE

    if (NULL == pSettings) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAudioEncoder *thiz = (IAudioEncoder *) self;
        SLAudioEncoderSettings settings = *pSettings;
        interface_lock_exclusive(thiz);
        thiz->mSettings = settings;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAudioEncoder_GetEncoderSettings(SLAudioEncoderItf self,
    SLAudioEncoderSettings *pSettings)
{
    SL_ENTER_INTERFACE

    if (NULL == pSettings) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAudioEncoder *thiz = (IAudioEncoder *) self;
        interface_lock_shared(thiz);
        SLAudioEncoderSettings settings = thiz->mSettings;
        interface_unlock_shared(thiz);
        *pSettings = settings;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLAudioEncoderItf_ IAudioEncoder_Itf = {
    IAudioEncoder_SetEncoderSettings,
    IAudioEncoder_GetEncoderSettings
};

void IAudioEncoder_init(void *self)
{
    IAudioEncoder *thiz = (IAudioEncoder *) self;
    thiz->mItf = &IAudioEncoder_Itf;
    memset(&thiz->mSettings, 0, sizeof(SLAudioEncoderSettings));
}
