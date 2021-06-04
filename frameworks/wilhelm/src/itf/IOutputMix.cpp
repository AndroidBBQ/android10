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

/* OutputMix implementation */

#include "sles_allinclusive.h"


static SLresult IOutputMix_GetDestinationOutputDeviceIDs(SLOutputMixItf self,
   SLint32 *pNumDevices, SLuint32 *pDeviceIDs)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumDevices) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        result = SL_RESULT_SUCCESS;
        // The application can set pDeviceIDs == NULL in order to find out number of devices.
        // Then the application can allocate a proper-sized device ID array and try again.
        if (NULL != pDeviceIDs) {
            if (1 > *pNumDevices) {
                result = SL_RESULT_BUFFER_INSUFFICIENT;
            } else {
                pDeviceIDs[0] = SL_DEFAULTDEVICEID_AUDIOOUTPUT;
            }
        }
        *pNumDevices = 1;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IOutputMix_RegisterDeviceChangeCallback(SLOutputMixItf self,
    slMixDeviceChangeCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IOutputMix *thiz = (IOutputMix *) self;
    interface_lock_exclusive(thiz);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IOutputMix_ReRoute(SLOutputMixItf self, SLint32 numOutputDevices,
    SLuint32 *pOutputDeviceIDs)
{
    SL_ENTER_INTERFACE

    if ((1 != numOutputDevices) || (NULL == pOutputDeviceIDs)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        switch (pOutputDeviceIDs[0]) {
        case SL_DEFAULTDEVICEID_AUDIOOUTPUT:
        case DEVICE_ID_HEADSET:
            result = SL_RESULT_SUCCESS;
            break;
        default:
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }
    }

    SL_LEAVE_INTERFACE
}


static const struct SLOutputMixItf_ IOutputMix_Itf = {
    IOutputMix_GetDestinationOutputDeviceIDs,
    IOutputMix_RegisterDeviceChangeCallback,
    IOutputMix_ReRoute
};

void IOutputMix_init(void *self)
{
    IOutputMix *thiz = (IOutputMix *) self;
    thiz->mItf = &IOutputMix_Itf;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
}
