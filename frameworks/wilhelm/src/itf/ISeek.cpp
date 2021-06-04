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

/* Seek implementation */

#include "sles_allinclusive.h"


static SLresult ISeek_SetPosition(SLSeekItf self, SLmillisecond pos, SLuint32 seekMode)
{
    SL_ENTER_INTERFACE

    switch (seekMode) {
    case SL_SEEKMODE_FAST:
    case SL_SEEKMODE_ACCURATE:
        {
        // maximum position is a special value that indicates a seek is not pending
        if (SL_TIME_UNKNOWN == pos) {
            pos = SL_TIME_UNKNOWN - 1;
        }
        ISeek *thiz = (ISeek *) self;
        interface_lock_exclusive(thiz);
        thiz->mPos = pos;
        // at this point the seek is merely pending, so do not yet update other fields
        interface_unlock_exclusive_attributes(thiz, ATTR_POSITION);
        result = SL_RESULT_SUCCESS;
        }
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult ISeek_SetLoop(SLSeekItf self, SLboolean loopEnable,
    SLmillisecond startPos, SLmillisecond endPos)
{
    SL_ENTER_INTERFACE

    if (!(startPos < endPos)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        ISeek *thiz = (ISeek *) self;
        interface_lock_exclusive(thiz);
#ifdef ANDROID
        if ((startPos != 0) && (endPos != SL_TIME_UNKNOWN)) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            switch (IObjectToObjectID((thiz)->mThis)) {
              case SL_OBJECTID_AUDIOPLAYER: {
                CAudioPlayer *ap = InterfaceToCAudioPlayer(thiz);
                if (NULL != ap) {
                    result = android_audioPlayer_loop(ap, loopEnable);
                } else {
                    result = SL_RESULT_PARAMETER_INVALID;
                }
                break;
              }
              case XA_OBJECTID_MEDIAPLAYER: {
                CMediaPlayer *mp = InterfaceToCMediaPlayer(thiz);
                if (NULL != mp) {
                    result = android_Player_loop(mp, loopEnable);
                } else {
                    result = SL_RESULT_PARAMETER_INVALID;
                }
                break;
              }
              default: {
                result = SL_RESULT_PARAMETER_INVALID;
              }
            }
            if (SL_RESULT_SUCCESS == result) {
                thiz->mLoopEnabled = SL_BOOLEAN_FALSE != loopEnable; // normalize
                // start and end positions already initialized to [0, end of stream]
                /*thiz->mStartPos = 0;
                  thiz->mEndPos = (SLmillisecond) SL_TIME_UNKNOWN;*/
            }
        }
#else
        thiz->mLoopEnabled = SL_BOOLEAN_FALSE != loopEnable; // normalize
        thiz->mStartPos = startPos;
        thiz->mEndPos = endPos;
        result = SL_RESULT_SUCCESS;
#endif
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult ISeek_GetLoop(SLSeekItf self, SLboolean *pLoopEnabled,
    SLmillisecond *pStartPos, SLmillisecond *pEndPos)
{
    SL_ENTER_INTERFACE

    if (NULL == pLoopEnabled || NULL == pStartPos || NULL == pEndPos) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        ISeek *thiz = (ISeek *) self;
        interface_lock_shared(thiz);
        SLboolean loopEnabled = thiz->mLoopEnabled;
        SLmillisecond startPos = thiz->mStartPos;
        SLmillisecond endPos = thiz->mEndPos;
        interface_unlock_shared(thiz);
        *pLoopEnabled = loopEnabled;
        *pStartPos = startPos;
        *pEndPos = endPos;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLSeekItf_ ISeek_Itf = {
    ISeek_SetPosition,
    ISeek_SetLoop,
    ISeek_GetLoop
};

void ISeek_init(void *self)
{
    ISeek *thiz = (ISeek *) self;
    thiz->mItf = &ISeek_Itf;
    thiz->mPos = (SLmillisecond) SL_TIME_UNKNOWN;
    thiz->mStartPos = (SLmillisecond) 0;
    thiz->mEndPos = (SLmillisecond) SL_TIME_UNKNOWN;
    thiz->mLoopEnabled = SL_BOOLEAN_FALSE;
}
