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

/* Volume implementation */

#include "sles_allinclusive.h"


static SLresult IVolume_SetVolumeLevel(SLVolumeItf self, SLmillibel level_)
{
    SL_ENTER_INTERFACE

    int level = level_;
    if (!((SL_MILLIBEL_MIN <= level) && (level <= PLATFORM_MILLIBEL_MAX_VOLUME))) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_exclusive(thiz);
        SLmillibel oldLevel = thiz->mLevel;
        if (oldLevel != level) {
            thiz->mLevel = level;
            interface_unlock_exclusive_attributes(thiz, ATTR_GAIN);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_GetVolumeLevel(SLVolumeItf self, SLmillibel *pLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_shared(thiz);
        SLmillibel level = thiz->mLevel;
        interface_unlock_shared(thiz);
        *pLevel = level;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_GetMaxVolumeLevel(SLVolumeItf self, SLmillibel *pMaxLevel)
{
    SL_ENTER_INTERFACE

    if (NULL == pMaxLevel) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pMaxLevel = PLATFORM_MILLIBEL_MAX_VOLUME;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_SetMute(SLVolumeItf self, SLboolean mute)
{
    SL_ENTER_INTERFACE

    IVolume *thiz = (IVolume *) self;
    mute = SL_BOOLEAN_FALSE != mute; // normalize
    interface_lock_exclusive(thiz);
    SLboolean oldMute = thiz->mMute;
    if (oldMute != mute) {
        thiz->mMute = (SLuint8) mute;
        interface_unlock_exclusive_attributes(thiz, ATTR_GAIN);
    } else {
        interface_unlock_exclusive(thiz);
    }
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_GetMute(SLVolumeItf self, SLboolean *pMute)
{
    SL_ENTER_INTERFACE

    if (NULL == pMute) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_shared(thiz);
        SLboolean mute = thiz->mMute;
        interface_unlock_shared(thiz);
        *pMute = mute;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_EnableStereoPosition(SLVolumeItf self, SLboolean enable)
{
    SL_ENTER_INTERFACE

    IVolume *thiz = (IVolume *) self;
    enable = SL_BOOLEAN_FALSE != enable; // normalize
    interface_lock_exclusive(thiz);
    SLboolean oldEnable = thiz->mEnableStereoPosition;
    if (oldEnable != enable) {
        thiz->mEnableStereoPosition = (SLuint8) enable;
        interface_unlock_exclusive_attributes(thiz, ATTR_GAIN);
    } else {
        interface_unlock_exclusive(thiz);
    }
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_IsEnabledStereoPosition(SLVolumeItf self, SLboolean *pEnable)
{
    SL_ENTER_INTERFACE

    if (NULL == pEnable) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_shared(thiz);
        SLboolean enable = thiz->mEnableStereoPosition;
        interface_unlock_shared(thiz);
        *pEnable = enable;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_SetStereoPosition(SLVolumeItf self, SLpermille stereoPosition)
{
    SL_ENTER_INTERFACE

    if (!((-1000 <= stereoPosition) && (1000 >= stereoPosition))) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_exclusive(thiz);
        SLpermille oldStereoPosition = thiz->mStereoPosition;
        if (oldStereoPosition != stereoPosition) {
            thiz->mStereoPosition = stereoPosition;
            interface_unlock_exclusive_attributes(thiz, ATTR_GAIN);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVolume_GetStereoPosition(SLVolumeItf self, SLpermille *pStereoPosition)
{
    SL_ENTER_INTERFACE

    if (NULL == pStereoPosition) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVolume *thiz = (IVolume *) self;
        interface_lock_shared(thiz);
        SLpermille stereoPosition = thiz->mStereoPosition;
        interface_unlock_shared(thiz);
        *pStereoPosition = stereoPosition;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLVolumeItf_ IVolume_Itf = {
    IVolume_SetVolumeLevel,
    IVolume_GetVolumeLevel,
    IVolume_GetMaxVolumeLevel,
    IVolume_SetMute,
    IVolume_GetMute,
    IVolume_EnableStereoPosition,
    IVolume_IsEnabledStereoPosition,
    IVolume_SetStereoPosition,
    IVolume_GetStereoPosition
};

void IVolume_init(void *self)
{
    IVolume *thiz = (IVolume *) self;
    thiz->mItf = &IVolume_Itf;
    thiz->mLevel = 0;
    thiz->mMute = SL_BOOLEAN_FALSE;
    thiz->mEnableStereoPosition = SL_BOOLEAN_FALSE;
    thiz->mStereoPosition = 0;
}
