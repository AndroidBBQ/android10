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

/* DeviceVolume implementation */

#include "sles_allinclusive.h"


static SLresult IDeviceVolume_GetVolumeScale(SLDeviceVolumeItf self, SLuint32 deviceID,
    SLint32 *pMinValue, SLint32 *pMaxValue, SLboolean *pIsMillibelScale)
{
    SL_ENTER_INTERFACE

    switch (deviceID) {
    case SL_DEFAULTDEVICEID_AUDIOINPUT:
    case SL_DEFAULTDEVICEID_AUDIOOUTPUT:
    case DEVICE_ID_HEADSET:
    case DEVICE_ID_HANDSFREE:
        if (NULL != pMinValue)
            *pMinValue = 0;
        if (NULL != pMaxValue)
            *pMaxValue = 10;
        if (NULL != pIsMillibelScale)
            *pIsMillibelScale = SL_BOOLEAN_FALSE;
        result = SL_RESULT_SUCCESS;
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IDeviceVolume_SetVolume(SLDeviceVolumeItf self, SLuint32 deviceID, SLint32 volume)
{
    SL_ENTER_INTERFACE

    switch (deviceID) {
    // These are treated same as generic audio output for now
    case DEVICE_ID_HEADSET:
    case DEVICE_ID_HANDSFREE:
        deviceID = SL_DEFAULTDEVICEID_AUDIOOUTPUT;
        FALLTHROUGH_INTENDED;
    case SL_DEFAULTDEVICEID_AUDIOINPUT:
    case SL_DEFAULTDEVICEID_AUDIOOUTPUT:
        {
        IDeviceVolume *thiz = (IDeviceVolume *) self;
        interface_lock_poke(thiz);
        thiz->mVolume[~deviceID] = volume;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
        }
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IDeviceVolume_GetVolume(SLDeviceVolumeItf self, SLuint32 deviceID, SLint32 *pVolume)
{
    SL_ENTER_INTERFACE

    if (NULL == pVolume) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        switch (deviceID) {
        // These are treated same as generic audio output for now
        case DEVICE_ID_HEADSET:
        case DEVICE_ID_HANDSFREE:
            deviceID = SL_DEFAULTDEVICEID_AUDIOOUTPUT;
            FALLTHROUGH_INTENDED;
        case SL_DEFAULTDEVICEID_AUDIOINPUT:
        case SL_DEFAULTDEVICEID_AUDIOOUTPUT:
            {
            IDeviceVolume *thiz = (IDeviceVolume *) self;
            interface_lock_peek(thiz);
            SLint32 volume = thiz->mVolume[~deviceID];
            interface_unlock_peek(thiz);
            *pVolume = volume;
            result = SL_RESULT_SUCCESS;
            }
            break;
        default:
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }
    }

    SL_LEAVE_INTERFACE
}


static const struct SLDeviceVolumeItf_ IDeviceVolume_Itf = {
    IDeviceVolume_GetVolumeScale,
    IDeviceVolume_SetVolume,
    IDeviceVolume_GetVolume
};

void IDeviceVolume_init(void *self)
{
    IDeviceVolume *thiz = (IDeviceVolume *) self;
    thiz->mItf = &IDeviceVolume_Itf;
    unsigned i;
    for (i = 0; i < MAX_DEVICE; ++i) {
        thiz->mVolume[i] = 10;
    }
}
