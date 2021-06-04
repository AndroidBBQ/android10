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

/* Play implementation */

#include "sles_allinclusive.h"


static SLresult IPlay_SetPlayState(SLPlayItf self, SLuint32 state)
{
    SL_ENTER_INTERFACE

    switch (state) {
    case SL_PLAYSTATE_STOPPED:
    case SL_PLAYSTATE_PAUSED:
    case SL_PLAYSTATE_PLAYING:
        {
        IPlay *thiz = (IPlay *) self;
        unsigned attr = ATTR_NONE;
        result = SL_RESULT_SUCCESS;
#ifdef USE_OUTPUTMIXEXT
        CAudioPlayer *audioPlayer = (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) ?
            (CAudioPlayer *) thiz->mThis : NULL;
#endif
        interface_lock_exclusive(thiz);
        SLuint32 oldState = thiz->mState;
        if (state != oldState) {
#ifdef USE_OUTPUTMIXEXT
          for (;; interface_cond_wait(thiz)) {

            // We are comparing the old state (left) vs. new state (right).
            // Note that the old state is 3 bits wide, but new state is only 2 bits wide.
            // That is why the old state is on the left and new state is on the right.
            switch ((oldState << 2) | state) {

            case (SL_PLAYSTATE_STOPPED  << 2) | SL_PLAYSTATE_STOPPED:
            case (SL_PLAYSTATE_PAUSED   << 2) | SL_PLAYSTATE_PAUSED:
            case (SL_PLAYSTATE_PLAYING  << 2) | SL_PLAYSTATE_PLAYING:
               // no-op, and unreachable due to earlier "if (state != oldState)"
                break;

            case (SL_PLAYSTATE_STOPPED  << 2) | SL_PLAYSTATE_PLAYING:
            case (SL_PLAYSTATE_PAUSED   << 2) | SL_PLAYSTATE_PLAYING:
                attr = ATTR_PLAY_STATE;
                // set enqueue attribute if queue is non-empty and state becomes PLAYING
                if ((NULL != audioPlayer) && (audioPlayer->mBufferQueue.mFront !=
                    audioPlayer->mBufferQueue.mRear)) {
                    // note that USE_OUTPUTMIXEXT does not support ATTR_ABQ_ENQUEUE
                    attr |= ATTR_BQ_ENQUEUE;
                }
                FALLTHROUGH_INTENDED;

            case (SL_PLAYSTATE_STOPPED  << 2) | SL_PLAYSTATE_PAUSED:
            case (SL_PLAYSTATE_PLAYING  << 2) | SL_PLAYSTATE_PAUSED:
                // easy
                thiz->mState = state;
                break;

            case (SL_PLAYSTATE_STOPPING << 2) | SL_PLAYSTATE_STOPPED:
                // either spurious wakeup, or someone else requested same transition
                continue;

            case (SL_PLAYSTATE_STOPPING << 2) | SL_PLAYSTATE_PAUSED:
            case (SL_PLAYSTATE_STOPPING << 2) | SL_PLAYSTATE_PLAYING:
                // wait for other guy to finish his transition, then retry ours
                continue;

            case (SL_PLAYSTATE_PAUSED   << 2) | SL_PLAYSTATE_STOPPED:
            case (SL_PLAYSTATE_PLAYING  << 2) | SL_PLAYSTATE_STOPPED:
                // tell mixer to stop, then wait for mixer to acknowledge the request to stop
                thiz->mState = SL_PLAYSTATE_STOPPING;
                continue;

            default:
                // unexpected state
                assert(SL_BOOLEAN_FALSE);
                result = SL_RESULT_INTERNAL_ERROR;
                break;

            }

            break;
          }
#else
          // Here life looks easy for an Android, but there are other troubles in play land
          thiz->mState = state;
          attr = ATTR_PLAY_STATE;
          // no need to set ATTR_BQ_ENQUEUE or ATTR_ABQ_ENQUEUE
#endif
        }
        // SL_LOGD("set play state %d", state);
        interface_unlock_exclusive_attributes(thiz, attr);
        }
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_GetPlayState(SLPlayItf self, SLuint32 *pState)
{
    SL_ENTER_INTERFACE

    if (NULL == pState) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        interface_lock_shared(thiz);
        SLuint32 state = thiz->mState;
        interface_unlock_shared(thiz);
        result = SL_RESULT_SUCCESS;
#ifdef USE_OUTPUTMIXEXT
        switch (state) {
        case SL_PLAYSTATE_STOPPED:  // as is
        case SL_PLAYSTATE_PAUSED:
        case SL_PLAYSTATE_PLAYING:
            break;
        case SL_PLAYSTATE_STOPPING: // these states require re-mapping
            state = SL_PLAYSTATE_STOPPED;
            break;
        default:                    // impossible
            assert(SL_BOOLEAN_FALSE);
            state = SL_PLAYSTATE_STOPPED;
            result = SL_RESULT_INTERNAL_ERROR;
            break;
        }
#endif
        *pState = state;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_GetDuration(SLPlayItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        result = SL_RESULT_SUCCESS;
        IPlay *thiz = (IPlay *) self;
        // even though this is a getter, it can modify state due to caching
        interface_lock_exclusive(thiz);
        SLmillisecond duration = thiz->mDuration;
#ifdef ANDROID
        if (SL_TIME_UNKNOWN == duration) {
            SLmillisecond temp;
            switch (InterfaceToObjectID(thiz)) {
            case SL_OBJECTID_AUDIOPLAYER:
                result = android_audioPlayer_getDuration(thiz, &temp);
                break;
            case XA_OBJECTID_MEDIAPLAYER:
                result = android_Player_getDuration(thiz, &temp);
                break;
            default:
                result = SL_RESULT_FEATURE_UNSUPPORTED;
                break;
            }
            if (SL_RESULT_SUCCESS == result) {
                duration = temp;
                thiz->mDuration = duration;
            }
        }
#else
        // will be set by containing AudioPlayer or MidiPlayer object at Realize, if known,
        // otherwise the duration will be updated each time a new maximum position is detected
#endif
        interface_unlock_exclusive(thiz);
        *pMsec = duration;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_GetPosition(SLPlayItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        SLmillisecond position;
        interface_lock_shared(thiz);
#ifdef ANDROID
        // Android does not use the mPosition field for audio and media players
        //  and doesn't cache the position
        switch (IObjectToObjectID((thiz)->mThis)) {
          case SL_OBJECTID_AUDIOPLAYER:
            android_audioPlayer_getPosition(thiz, &position);
            break;
          case XA_OBJECTID_MEDIAPLAYER:
            android_Player_getPosition(thiz, &position);
            break;
          default:
            // we shouldn'be here
            assert(SL_BOOLEAN_FALSE);
        }
#else
        // on other platforms we depend on periodic updates to the current position
        position = thiz->mPosition;
        // if a seek is pending, then lie about current position so the seek appears synchronous
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
            CAudioPlayer *audioPlayer = (CAudioPlayer *) thiz->mThis;
            SLmillisecond pos = audioPlayer->mSeek.mPos;
            if (SL_TIME_UNKNOWN != pos) {
                position = pos;
            }
        }
#endif
        interface_unlock_shared(thiz);
        *pMsec = position;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_RegisterCallback(SLPlayItf self, slPlayCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IPlay *thiz = (IPlay *) self;
    interface_lock_exclusive(thiz);
    thiz->mCallback = callback;
    thiz->mContext = pContext;
    // omits _attributes b/c noone cares deeply enough about these fields to need quick notification
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_SetCallbackEventsMask(SLPlayItf self, SLuint32 eventFlags)
{
    SL_ENTER_INTERFACE

    if (eventFlags & ~(SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADATMARKER |
            SL_PLAYEVENT_HEADATNEWPOS | SL_PLAYEVENT_HEADMOVING | SL_PLAYEVENT_HEADSTALLED)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        interface_lock_exclusive(thiz);
        if (thiz->mEventFlags != eventFlags) {
#ifdef USE_OUTPUTMIXEXT
            // enabling the "head at new position" play event will postpone the next update event
            if (!(thiz->mEventFlags & SL_PLAYEVENT_HEADATNEWPOS) &&
                    (eventFlags & SL_PLAYEVENT_HEADATNEWPOS)) {
                thiz->mFramesSincePositionUpdate = 0;
            }
#endif
            thiz->mEventFlags = eventFlags;
            interface_unlock_exclusive_attributes(thiz, ATTR_TRANSPORT);
        } else {
            interface_unlock_exclusive(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_GetCallbackEventsMask(SLPlayItf self, SLuint32 *pEventFlags)
{
    SL_ENTER_INTERFACE

    if (NULL == pEventFlags) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        interface_lock_shared(thiz);
        SLuint32 eventFlags = thiz->mEventFlags;
        interface_unlock_shared(thiz);
        *pEventFlags = eventFlags;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IPlay_SetMarkerPosition(SLPlayItf self, SLmillisecond mSec)
{
    SL_ENTER_INTERFACE

    if (SL_TIME_UNKNOWN == mSec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        bool significant = false;
        interface_lock_exclusive(thiz);
        if (thiz->mMarkerPosition != mSec) {
            thiz->mMarkerPosition = mSec;
            if (thiz->mEventFlags & SL_PLAYEVENT_HEADATMARKER) {
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


static SLresult IPlay_ClearMarkerPosition(SLPlayItf self)
{
    SL_ENTER_INTERFACE

    IPlay *thiz = (IPlay *) self;
    bool significant = false;
    interface_lock_exclusive(thiz);
    // clearing the marker position is equivalent to setting the marker to SL_TIME_UNKNOWN
    if (thiz->mMarkerPosition != SL_TIME_UNKNOWN) {
        thiz->mMarkerPosition = SL_TIME_UNKNOWN;
        if (thiz->mEventFlags & SL_PLAYEVENT_HEADATMARKER) {
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


static SLresult IPlay_GetMarkerPosition(SLPlayItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
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


static SLresult IPlay_SetPositionUpdatePeriod(SLPlayItf self, SLmillisecond mSec)
{
    SL_ENTER_INTERFACE

    if (0 == mSec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        bool significant = false;
        interface_lock_exclusive(thiz);
        if (thiz->mPositionUpdatePeriod != mSec) {
            thiz->mPositionUpdatePeriod = mSec;
#ifdef USE_OUTPUTMIXEXT
            if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
                CAudioPlayer *audioPlayer = (CAudioPlayer *) thiz->mThis;
                SLuint32 frameUpdatePeriod = ((long long) mSec *
                    (long long) audioPlayer->mSampleRateMilliHz) / 1000000LL;
                if (0 == frameUpdatePeriod) {
                    frameUpdatePeriod = ~0;
                }
                thiz->mFrameUpdatePeriod = frameUpdatePeriod;
                // setting a new update period postpones the next callback
                thiz->mFramesSincePositionUpdate = 0;
            }
#endif
            if (thiz->mEventFlags & SL_PLAYEVENT_HEADATNEWPOS) {
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


static SLresult IPlay_GetPositionUpdatePeriod(SLPlayItf self, SLmillisecond *pMsec)
{
    SL_ENTER_INTERFACE

    if (NULL == pMsec) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IPlay *thiz = (IPlay *) self;
        interface_lock_shared(thiz);
        SLmillisecond positionUpdatePeriod = thiz->mPositionUpdatePeriod;
        interface_unlock_shared(thiz);
        *pMsec = positionUpdatePeriod;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLPlayItf_ IPlay_Itf = {
    IPlay_SetPlayState,
    IPlay_GetPlayState,
    IPlay_GetDuration,
    IPlay_GetPosition,
    IPlay_RegisterCallback,
    IPlay_SetCallbackEventsMask,
    IPlay_GetCallbackEventsMask,
    IPlay_SetMarkerPosition,
    IPlay_ClearMarkerPosition,
    IPlay_GetMarkerPosition,
    IPlay_SetPositionUpdatePeriod,
    IPlay_GetPositionUpdatePeriod
};

void IPlay_init(void *self)
{
    IPlay *thiz = (IPlay *) self;
    thiz->mItf = &IPlay_Itf;
    thiz->mState = SL_PLAYSTATE_STOPPED;
    thiz->mDuration = SL_TIME_UNKNOWN;  // will be set by containing player object
    thiz->mPosition = (SLmillisecond) 0;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mEventFlags = 0;
    thiz->mMarkerPosition = SL_TIME_UNKNOWN;
    thiz->mPositionUpdatePeriod = 1000; // per spec
#ifdef USE_OUTPUTMIXEXT
    thiz->mFrameUpdatePeriod = 0;   // because we don't know the sample rate yet
    thiz->mLastSeekPosition = 0;
    thiz->mFramesSinceLastSeek = 0;
    thiz->mFramesSincePositionUpdate = 0;
#endif
}
