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

#include <system/window.h>
#include <utils/StrongPointer.h>
#include <gui/Surface.h>

#include "sles_allinclusive.h"
#include "android_prompts.h"
// LocAVPlayer and StreamPlayer derive from GenericMediaPlayer,
//    so no need to #include "android_GenericMediaPlayer.h"
#include "android_LocAVPlayer.h"
#include "android_StreamPlayer.h"

//-----------------------------------------------------------------------------
static void player_handleMediaPlayerEventNotifications(int event, int data1, int data2, void* user)
{

    // FIXME This code is derived from similar code in sfplayer_handlePrefetchEvent.  The two
    // versions are quite similar, but still different enough that they need to be separate.
    // At some point they should be re-factored and merged if feasible.
    // As with other OpenMAX AL implementation code, this copy mostly uses SL_ symbols
    // rather than XA_ unless the difference is significant.

    if (NULL == user) {
        return;
    }

    CMediaPlayer* mp = (CMediaPlayer*) user;
    if (!android::CallbackProtector::enterCbIfOk(mp->mCallbackProtector)) {
        // it is not safe to enter the callback (the media player is about to go away)
        return;
    }
    union {
        char c[sizeof(int)];
        int i;
    } u;
    u.i = event;
    SL_LOGV("player_handleMediaPlayerEventNotifications(event='%c%c%c%c' (%d), data1=%d, data2=%d, "
            "user=%p) from AVPlayer", u.c[3], u.c[2], u.c[1], u.c[0], event, data1, data2, user);
    switch (event) {

      case android::GenericPlayer::kEventPrepared: {
        SL_LOGV("Received GenericPlayer::kEventPrepared for CMediaPlayer %p", mp);

        // assume no callback
        slPrefetchCallback callback = NULL;
        void* callbackPContext;
        XAuint32 events;

        object_lock_exclusive(&mp->mObject);

        // mark object as prepared; same state is used for successful or unsuccessful prepare
        assert(mp->mAndroidObjState == ANDROID_PREPARING);
        mp->mAndroidObjState = ANDROID_READY;

        if (PLAYER_SUCCESS == data1) {
            // Most of successful prepare completion for mp->mAVPlayer
            // is handled by GenericPlayer and its subclasses.
        } else {
            // AVPlayer prepare() failed prefetching, there is no event in XAPrefetchStatus to
            //  indicate a prefetch error, so we signal it by sending simultaneously two events:
            //  - SL_PREFETCHEVENT_FILLLEVELCHANGE with a level of 0
            //  - SL_PREFETCHEVENT_STATUSCHANGE with a status of SL_PREFETCHSTATUS_UNDERFLOW
            SL_LOGE(ERROR_PLAYER_PREFETCH_d, data1);
            if (IsInterfaceInitialized(&mp->mObject, MPH_XAPREFETCHSTATUS)) {
                mp->mPrefetchStatus.mLevel = 0;
                mp->mPrefetchStatus.mStatus = SL_PREFETCHSTATUS_UNDERFLOW;
                if (!(~mp->mPrefetchStatus.mCallbackEventsMask &
                        (SL_PREFETCHEVENT_FILLLEVELCHANGE | SL_PREFETCHEVENT_STATUSCHANGE))) {
                    callback = mp->mPrefetchStatus.mCallback;
                    callbackPContext = mp->mPrefetchStatus.mContext;
                    events = SL_PREFETCHEVENT_FILLLEVELCHANGE | SL_PREFETCHEVENT_STATUSCHANGE;
                }
            }
        }

        object_unlock_exclusive(&mp->mObject);

        // callback with no lock held
        if (NULL != callback) {
            (*callback)(&mp->mPrefetchStatus.mItf, callbackPContext, events);
        }

        break;
      }

      case android::GenericPlayer::kEventHasVideoSize: {
        SL_LOGV("Received AVPlayer::kEventHasVideoSize (%d,%d) for CMediaPlayer %p",
                data1, data2, mp);

        object_lock_exclusive(&mp->mObject);

        // remove an existing video info entry (here we only have one video stream)
        for(size_t i=0 ; i < mp->mStreamInfo.mStreamInfoTable.size() ; i++) {
            if (XA_DOMAINTYPE_VIDEO == mp->mStreamInfo.mStreamInfoTable.itemAt(i).domain) {
                mp->mStreamInfo.mStreamInfoTable.removeAt(i);
                break;
            }
        }
        // update the stream information with a new video info entry
        StreamInfo streamInfo;
        streamInfo.domain = XA_DOMAINTYPE_VIDEO;
        streamInfo.videoInfo.codecId = 0;// unknown, we don't have that info FIXME
        streamInfo.videoInfo.width = (XAuint32)data1;
        streamInfo.videoInfo.height = (XAuint32)data2;
        streamInfo.videoInfo.bitRate = 0;// unknown, we don't have that info FIXME
        streamInfo.videoInfo.frameRate = 0;
        streamInfo.videoInfo.duration = XA_TIME_UNKNOWN;
        StreamInfo &contInfo = mp->mStreamInfo.mStreamInfoTable.editItemAt(0);
        contInfo.containerInfo.numStreams = 1;
        ssize_t index = mp->mStreamInfo.mStreamInfoTable.add(streamInfo);

        // callback is unconditional; there is no bitmask of enabled events
        xaStreamEventChangeCallback callback = mp->mStreamInfo.mCallback;
        void* callbackPContext = mp->mStreamInfo.mContext;

        object_unlock_exclusive(&mp->mObject);

        // enqueue notification (outside of lock) that the stream information has been updated
        if ((NULL != callback) && (index >= 0)) {
#ifndef USE_ASYNCHRONOUS_STREAMCBEVENT_PROPERTYCHANGE_CALLBACK
            (*callback)(&mp->mStreamInfo.mItf, XA_STREAMCBEVENT_PROPERTYCHANGE /*eventId*/,
                    1 /*streamIndex, only one stream supported here, 0 is reserved*/,
                    NULL /*pEventData, always NULL in OpenMAX AL 1.0.1*/,
                    callbackPContext /*pContext*/);
#else
            SLresult res = EnqueueAsyncCallback_piipp(mp, callback,
                    /*p1*/ &mp->mStreamInfo.mItf,
                    /*i1*/ XA_STREAMCBEVENT_PROPERTYCHANGE /*eventId*/,
                    /*i2*/ 1 /*streamIndex, only one stream supported here, 0 is reserved*/,
                    /*p2*/ NULL /*pEventData, always NULL in OpenMAX AL 1.0.1*/,
                    /*p3*/ callbackPContext /*pContext*/);
            ALOGW_IF(SL_RESULT_SUCCESS != res,
                        "Callback %p(%p, XA_STREAMCBEVENT_PROPERTYCHANGE, 1, NULL, %p) dropped",
                        callback, &mp->mStreamInfo.mItf, callbackPContext);
#endif
        }
        break;
      }

      case android::GenericPlayer::kEventEndOfStream: {
        SL_LOGV("Received AVPlayer::kEventEndOfStream for CMediaPlayer %p", mp);

        object_lock_exclusive(&mp->mObject);
        // should be xaPlayCallback but we're sharing the itf between SL and AL
        slPlayCallback playCallback = NULL;
        void * playContext = NULL;
        // XAPlayItf callback or no callback?
        if (mp->mPlay.mEventFlags & XA_PLAYEVENT_HEADATEND) {
            playCallback = mp->mPlay.mCallback;
            playContext = mp->mPlay.mContext;
        }
        mp->mPlay.mState = XA_PLAYSTATE_PAUSED;
        object_unlock_exclusive(&mp->mObject);

        // enqueue callback with no lock held
        if (NULL != playCallback) {
#ifndef USE_ASYNCHRONOUS_PLAY_CALLBACK
            (*playCallback)(&mp->mPlay.mItf, playContext, XA_PLAYEVENT_HEADATEND);
#else
            SLresult res = EnqueueAsyncCallback_ppi(mp, playCallback, &mp->mPlay.mItf, playContext,
                    XA_PLAYEVENT_HEADATEND);
            ALOGW_IF(SL_RESULT_SUCCESS != res,
                    "Callback %p(%p, %p, SL_PLAYEVENT_HEADATEND) dropped", playCallback,
                    &mp->mPlay.mItf, playContext);
#endif
        }
        break;
      }

      case android::GenericPlayer::kEventChannelCount: {
        SL_LOGV("kEventChannelCount channels = %d", data1);
        object_lock_exclusive(&mp->mObject);
        if (UNKNOWN_NUMCHANNELS == mp->mNumChannels && UNKNOWN_NUMCHANNELS != data1) {
            mp->mNumChannels = data1;
            android_Player_volumeUpdate(mp);
        }
        object_unlock_exclusive(&mp->mObject);
      }
      break;

      case android::GenericPlayer::kEventPrefetchFillLevelUpdate: {
        SL_LOGV("kEventPrefetchFillLevelUpdate");
        if (!IsInterfaceInitialized(&mp->mObject, MPH_XAPREFETCHSTATUS)) {
            break;
        }
        slPrefetchCallback callback = NULL;
        void* callbackPContext = NULL;

        // SLPrefetchStatusItf callback or no callback?
        interface_lock_exclusive(&mp->mPrefetchStatus);
        if (mp->mPrefetchStatus.mCallbackEventsMask & SL_PREFETCHEVENT_FILLLEVELCHANGE) {
            callback = mp->mPrefetchStatus.mCallback;
            callbackPContext = mp->mPrefetchStatus.mContext;
        }
        mp->mPrefetchStatus.mLevel = (SLpermille)data1;
        interface_unlock_exclusive(&mp->mPrefetchStatus);

        // callback with no lock held
        if (NULL != callback) {
            (*callback)(&mp->mPrefetchStatus.mItf, callbackPContext,
                    SL_PREFETCHEVENT_FILLLEVELCHANGE);
        }
      }
      break;

      case android::GenericPlayer::kEventPrefetchStatusChange: {
        SL_LOGV("kEventPrefetchStatusChange");
        if (!IsInterfaceInitialized(&mp->mObject, MPH_XAPREFETCHSTATUS)) {
            break;
        }
        slPrefetchCallback callback = NULL;
        void* callbackPContext = NULL;

        // SLPrefetchStatusItf callback or no callback?
        object_lock_exclusive(&mp->mObject);
        if (mp->mPrefetchStatus.mCallbackEventsMask & SL_PREFETCHEVENT_STATUSCHANGE) {
            callback = mp->mPrefetchStatus.mCallback;
            callbackPContext = mp->mPrefetchStatus.mContext;
        }
        if (data1 >= android::kStatusIntermediate) {
            mp->mPrefetchStatus.mStatus = SL_PREFETCHSTATUS_SUFFICIENTDATA;
        } else if (data1 < android::kStatusIntermediate) {
            mp->mPrefetchStatus.mStatus = SL_PREFETCHSTATUS_UNDERFLOW;
        }
        object_unlock_exclusive(&mp->mObject);

        // callback with no lock held
        if (NULL != callback) {
            (*callback)(&mp->mPrefetchStatus.mItf, callbackPContext, SL_PREFETCHEVENT_STATUSCHANGE);
        }
      }
      break;

      case android::GenericPlayer::kEventPlay: {
        SL_LOGV("kEventPlay");

        interface_lock_shared(&mp->mPlay);
        slPlayCallback callback = mp->mPlay.mCallback;
        void* callbackPContext = mp->mPlay.mContext;
        interface_unlock_shared(&mp->mPlay);

        if (NULL != callback) {
            (*callback)(&mp->mPlay.mItf, callbackPContext, (SLuint32) data1); // SL_PLAYEVENT_HEAD*
        }
      }
      break;

      case android::GenericPlayer::kEventErrorAfterPrepare: {
        SL_LOGV("kEventErrorAfterPrepare");

        // assume no callback
        slPrefetchCallback callback = NULL;
        void* callbackPContext = NULL;

        object_lock_exclusive(&mp->mObject);
        if (IsInterfaceInitialized(&mp->mObject, MPH_XAPREFETCHSTATUS)) {
            mp->mPrefetchStatus.mLevel = 0;
            mp->mPrefetchStatus.mStatus = SL_PREFETCHSTATUS_UNDERFLOW;
            if (!(~mp->mPrefetchStatus.mCallbackEventsMask &
                    (SL_PREFETCHEVENT_FILLLEVELCHANGE | SL_PREFETCHEVENT_STATUSCHANGE))) {
                callback = mp->mPrefetchStatus.mCallback;
                callbackPContext = mp->mPrefetchStatus.mContext;
            }
        }
        object_unlock_exclusive(&mp->mObject);

        // FIXME there's interesting information in data1, but no API to convey it to client
        SL_LOGE("Error after prepare: %d", data1);

        // callback with no lock held
        if (NULL != callback) {
            (*callback)(&mp->mPrefetchStatus.mItf, callbackPContext,
                    SL_PREFETCHEVENT_FILLLEVELCHANGE | SL_PREFETCHEVENT_STATUSCHANGE);
        }

      }
      break;

      default: {
        SL_LOGE("Received unknown event %d, data %d from AVPlayer", event, data1);
      }
    }

    mp->mCallbackProtector->exitCb();
}


//-----------------------------------------------------------------------------
XAresult android_Player_checkSourceSink(CMediaPlayer *mp) {

    XAresult result = XA_RESULT_SUCCESS;

    const SLDataSource *pSrc    = &mp->mDataSource.u.mSource;
    const SLDataSink *pAudioSnk = &mp->mAudioSink.u.mSink;

    // format check:
    const SLuint32 sourceLocatorType = *(SLuint32 *)pSrc->pLocator;
    const SLuint32 sourceFormatType  = *(SLuint32 *)pSrc->pFormat;
    const SLuint32 audioSinkLocatorType = *(SLuint32 *)pAudioSnk->pLocator;
    //const SLuint32 sinkFormatType = *(SLuint32 *)pAudioSnk->pFormat;

    // Source check
    switch (sourceLocatorType) {

    case XA_DATALOCATOR_ANDROIDBUFFERQUEUE: {
        switch (sourceFormatType) {
        case XA_DATAFORMAT_MIME: {
            SLDataFormat_MIME *df_mime = (SLDataFormat_MIME *) pSrc->pFormat;
            if (SL_CONTAINERTYPE_MPEG_TS != df_mime->containerType) {
                SL_LOGE("Cannot create player with XA_DATALOCATOR_ANDROIDBUFFERQUEUE data source "
                        "that is not fed MPEG-2 TS data");
                return SL_RESULT_CONTENT_UNSUPPORTED;
            }
        } break;
        default:
            SL_LOGE("Cannot create player with XA_DATALOCATOR_ANDROIDBUFFERQUEUE data source "
                    "without SL_DATAFORMAT_MIME format");
            return XA_RESULT_CONTENT_UNSUPPORTED;
        }
    } break;

    case XA_DATALOCATOR_URI:
        FALLTHROUGH_INTENDED;
    case XA_DATALOCATOR_ANDROIDFD:
        break;

    default:
        SL_LOGE("Cannot create media player with data locator type 0x%x",
                (unsigned) sourceLocatorType);
        return SL_RESULT_PARAMETER_INVALID;
    }// switch (locatorType)

    // Audio sink check: only playback is supported here
    switch (audioSinkLocatorType) {

    case XA_DATALOCATOR_OUTPUTMIX:
        break;

    default:
        SL_LOGE("Cannot create media player with audio sink data locator of type 0x%x",
                (unsigned) audioSinkLocatorType);
        return XA_RESULT_PARAMETER_INVALID;
    }// switch (locaaudioSinkLocatorTypeorType)

    return result;
}


//-----------------------------------------------------------------------------
XAresult android_Player_create(CMediaPlayer *mp) {

    XAresult result = XA_RESULT_SUCCESS;

    // FIXME verify data source
    const SLDataSource *pDataSrc = &mp->mDataSource.u.mSource;
    // FIXME verify audio data sink
    // const SLDataSink *pAudioSnk = &mp->mAudioSink.u.mSink;
    // FIXME verify image data sink
    // const SLDataSink *pVideoSnk = &mp->mImageVideoSink.u.mSink;

    XAuint32 sourceLocator = *(XAuint32 *)pDataSrc->pLocator;
    switch (sourceLocator) {
    // FIXME support Android simple buffer queue as well
    case XA_DATALOCATOR_ANDROIDBUFFERQUEUE:
        mp->mAndroidObjType = AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE;
        break;
    case XA_DATALOCATOR_URI:
        FALLTHROUGH_INTENDED;
    case SL_DATALOCATOR_ANDROIDFD:
        mp->mAndroidObjType = AUDIOVIDEOPLAYER_FROM_URIFD;
        break;
    case XA_DATALOCATOR_ADDRESS:
        FALLTHROUGH_INTENDED;
    default:
        mp->mAndroidObjType = INVALID_TYPE;
        SL_LOGE("Unable to create MediaPlayer for data source locator 0x%x", sourceLocator);
        result = XA_RESULT_PARAMETER_INVALID;
        break;
    }

    // FIXME duplicates an initialization also done by higher level
    mp->mAndroidObjState = ANDROID_UNINITIALIZED;
    mp->mStreamType = ANDROID_DEFAULT_OUTPUT_STREAM_TYPE;
    mp->mSessionId = (audio_session_t) android::AudioSystem::newAudioUniqueId(
            AUDIO_UNIQUE_ID_USE_SESSION);

    // placeholder: not necessary yet as session ID lifetime doesn't extend beyond player
    // android::AudioSystem::acquireAudioSessionId(mp->mSessionId);

    mp->mCallbackProtector = new android::CallbackProtector();

    return result;
}


//-----------------------------------------------------------------------------
// FIXME abstract out the diff between CMediaPlayer and CAudioPlayer
XAresult android_Player_realize(CMediaPlayer *mp, SLboolean async) {
    SL_LOGV("android_Player_realize_l(%p)", mp);
    XAresult result = XA_RESULT_SUCCESS;

    AudioPlayback_Parameters ap_params;
    ap_params.sessionId = mp->mSessionId;
    ap_params.streamType = mp->mStreamType;

    switch (mp->mAndroidObjType) {
    case AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE: {
        mp->mAVPlayer = new android::StreamPlayer(&ap_params, true /*hasVideo*/,
                &mp->mAndroidBufferQueue, mp->mCallbackProtector);
        mp->mAVPlayer->init(player_handleMediaPlayerEventNotifications, (void*)mp);
        }
        break;
    case AUDIOVIDEOPLAYER_FROM_URIFD: {
        mp->mAVPlayer = new android::LocAVPlayer(&ap_params, true /*hasVideo*/);
        mp->mAVPlayer->init(player_handleMediaPlayerEventNotifications, (void*)mp);
        switch (mp->mDataSource.mLocator.mLocatorType) {
        case XA_DATALOCATOR_URI:
            ((android::LocAVPlayer*)mp->mAVPlayer.get())->setDataSource(
                    (const char*)mp->mDataSource.mLocator.mURI.URI);
            break;
        case XA_DATALOCATOR_ANDROIDFD: {
            int64_t offset = (int64_t)mp->mDataSource.mLocator.mFD.offset;
            ((android::LocAVPlayer*)mp->mAVPlayer.get())->setDataSource(
                    (int)mp->mDataSource.mLocator.mFD.fd,
                    offset == SL_DATALOCATOR_ANDROIDFD_USE_FILE_SIZE ?
                            (int64_t)PLAYER_FD_FIND_FILE_SIZE : offset,
                    (int64_t)mp->mDataSource.mLocator.mFD.length);
            }
            break;
        default:
            SL_LOGE("Invalid or unsupported data locator type %u for data source",
                    mp->mDataSource.mLocator.mLocatorType);
            result = XA_RESULT_PARAMETER_INVALID;
        }
        }
        break;
    case INVALID_TYPE:
        FALLTHROUGH_INTENDED;
    default:
        SL_LOGE("Unable to realize MediaPlayer, invalid internal Android object type");
        result = XA_RESULT_PARAMETER_INVALID;
        break;
    }

    if (XA_RESULT_SUCCESS == result) {

        // if there is a video sink
        if (XA_DATALOCATOR_NATIVEDISPLAY ==
                mp->mImageVideoSink.mLocator.mLocatorType) {
            ANativeWindow *nativeWindow = (ANativeWindow *)
                    mp->mImageVideoSink.mLocator.mNativeDisplay.hWindow;
            // we already verified earlier that hWindow is non-NULL
            assert(nativeWindow != NULL);
            result = android_Player_setNativeWindow(mp, nativeWindow);
        }

    }

    return result;
}

// Called with a lock on MediaPlayer, and blocks until safe to destroy
XAresult android_Player_preDestroy(CMediaPlayer *mp) {
    SL_LOGV("android_Player_preDestroy(%p)", mp);

    // Not yet clear why this order is important, but it reduces detected deadlocks
    object_unlock_exclusive(&mp->mObject);
    if (mp->mCallbackProtector != 0) {
        mp->mCallbackProtector->requestCbExitAndWait();
    }
    object_lock_exclusive(&mp->mObject);

    if (mp->mAVPlayer != 0) {
        mp->mAVPlayer->preDestroy();
    }
    SL_LOGV("android_Player_preDestroy(%p) after mAVPlayer->preDestroy()", mp);

    return XA_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------
XAresult android_Player_destroy(CMediaPlayer *mp) {
    SL_LOGV("android_Player_destroy(%p)", mp);

    mp->mAVPlayer.clear();

    // placeholder: not necessary yet as session ID lifetime doesn't extend beyond player
    // android::AudioSystem::releaseAudioSessionId(mp->mSessionId);

    mp->mCallbackProtector.clear();

    // explicit destructor
    mp->mAVPlayer.~sp();
    mp->mCallbackProtector.~sp();

    return XA_RESULT_SUCCESS;
}


void android_Player_usePlayEventMask(CMediaPlayer *mp) {
    if (mp->mAVPlayer != 0) {
        IPlay *pPlayItf = &mp->mPlay;
        mp->mAVPlayer->setPlayEvents((int32_t) pPlayItf->mEventFlags,
                (int32_t) pPlayItf->mMarkerPosition, (int32_t) pPlayItf->mPositionUpdatePeriod);
    }
}


XAresult android_Player_getDuration(IPlay *pPlayItf, XAmillisecond *pDurMsec) {
    CMediaPlayer *avp = (CMediaPlayer *)pPlayItf->mThis;

    switch (avp->mAndroidObjType) {

    case AUDIOVIDEOPLAYER_FROM_URIFD: {
        int dur = ANDROID_UNKNOWN_TIME;
        if (avp->mAVPlayer != 0) {
            avp->mAVPlayer->getDurationMsec(&dur);
        }
        if (dur == ANDROID_UNKNOWN_TIME) {
            *pDurMsec = XA_TIME_UNKNOWN;
        } else {
            *pDurMsec = (XAmillisecond)dur;
        }
    } break;

    case AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE:
        FALLTHROUGH_INTENDED;
    default:
        *pDurMsec = XA_TIME_UNKNOWN;
        break;
    }

    return XA_RESULT_SUCCESS;
}


XAresult android_Player_getPosition(IPlay *pPlayItf, XAmillisecond *pPosMsec) {
    SL_LOGD("android_Player_getPosition()");
    XAresult result = XA_RESULT_SUCCESS;
    CMediaPlayer *avp = (CMediaPlayer *)pPlayItf->mThis;

    switch (avp->mAndroidObjType) {

    case AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE:
        FALLTHROUGH_INTENDED;
    case AUDIOVIDEOPLAYER_FROM_URIFD: {
        int pos = ANDROID_UNKNOWN_TIME;
        if (avp->mAVPlayer != 0) {
            avp->mAVPlayer->getPositionMsec(&pos);
        }
        if (pos == ANDROID_UNKNOWN_TIME) {
            *pPosMsec = 0;
        } else {
            *pPosMsec = (XAmillisecond)pos;
        }
    } break;

    default:
        // we shouldn't be here
        assert(false);
        break;
    }

    return result;
}


//-----------------------------------------------------------------------------
/**
 * pre-condition: mp != NULL
 */
void android_Player_volumeUpdate(CMediaPlayer* mp)
{
    android::GenericPlayer* avp = mp->mAVPlayer.get();
    if (avp != NULL) {
        float volumes[2];
        // MediaPlayer does not currently support EffectSend or MuteSolo
        android_player_volumeUpdate(volumes, &mp->mVolume, mp->mNumChannels, 1.0f, NULL);
        float leftVol = volumes[0], rightVol = volumes[1];
        avp->setVolume(leftVol, rightVol);
    }
}

//-----------------------------------------------------------------------------
/**
 * pre-condition: gp != 0
 */
XAresult android_Player_setPlayState(const android::sp<android::GenericPlayer> &gp,
        SLuint32 playState,
        AndroidObjectState* pObjState)
{
    XAresult result = XA_RESULT_SUCCESS;
    AndroidObjectState objState = *pObjState;

    switch (playState) {
     case SL_PLAYSTATE_STOPPED: {
         SL_LOGV("setting AVPlayer to SL_PLAYSTATE_STOPPED");
         gp->stop();
         }
         break;
     case SL_PLAYSTATE_PAUSED: {
         SL_LOGV("setting AVPlayer to SL_PLAYSTATE_PAUSED");
         switch (objState) {
         case ANDROID_UNINITIALIZED:
             *pObjState = ANDROID_PREPARING;
             gp->prepare();
             break;
         case ANDROID_PREPARING:
             break;
         case ANDROID_READY:
             gp->pause();
             break;
         default:
             SL_LOGE("Android object in invalid state");
             break;
         }
         }
         break;
     case SL_PLAYSTATE_PLAYING: {
         SL_LOGV("setting AVPlayer to SL_PLAYSTATE_PLAYING");
         switch (objState) {
         case ANDROID_UNINITIALIZED:
             *pObjState = ANDROID_PREPARING;
             gp->prepare();
             FALLTHROUGH_INTENDED;
         case ANDROID_PREPARING:
             FALLTHROUGH_INTENDED;
         case ANDROID_READY:
             gp->play();
             break;
         default:
             SL_LOGE("Android object in invalid state");
             break;
         }
         }
         break;
     default:
         // checked by caller, should not happen
         break;
     }

    return result;
}


/**
 * pre-condition: mp != NULL
 */
XAresult android_Player_seek(CMediaPlayer *mp, SLmillisecond posMsec) {
    XAresult result = XA_RESULT_SUCCESS;
    switch (mp->mAndroidObjType) {
      case AUDIOVIDEOPLAYER_FROM_URIFD:
        if (mp->mAVPlayer !=0) {
            mp->mAVPlayer->seek(posMsec);
        }
        break;
      case AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE:
        FALLTHROUGH_INTENDED;
      default: {
          result = XA_RESULT_FEATURE_UNSUPPORTED;
      }
    }
    return result;
}


/**
 * pre-condition: mp != NULL
 */
XAresult android_Player_loop(CMediaPlayer *mp, SLboolean loopEnable) {
    XAresult result = XA_RESULT_SUCCESS;
    switch (mp->mAndroidObjType) {
      case AUDIOVIDEOPLAYER_FROM_URIFD:
        if (mp->mAVPlayer !=0) {
            mp->mAVPlayer->loop(loopEnable);
        }
        break;
      case AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE:
        FALLTHROUGH_INTENDED;
      default: {
          result = XA_RESULT_FEATURE_UNSUPPORTED;
      }
    }
    return result;
}


//-----------------------------------------------------------------------------
void android_Player_androidBufferQueue_clear_l(CMediaPlayer *mp) {
    if ((mp->mAndroidObjType == AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE)
            && (mp->mAVPlayer != 0)) {
        android::StreamPlayer* splr = static_cast<android::StreamPlayer*>(mp->mAVPlayer.get());
        splr->appClear_l();
    }
}


void android_Player_androidBufferQueue_onRefilled_l(CMediaPlayer *mp) {
    if ((mp->mAndroidObjType == AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE)
            && (mp->mAVPlayer != 0)) {
        android::StreamPlayer* splr = static_cast<android::StreamPlayer*>(mp->mAVPlayer.get());
        splr->queueRefilled();
    }
}


/*
 *  pre-conditions:
 *      mp != NULL
 *      mp->mAVPlayer != 0 (player is realized)
 *      nativeWindow can be NULL, but if NULL it is treated as an error
 */
SLresult android_Player_setNativeWindow(CMediaPlayer *mp, ANativeWindow *nativeWindow)
{
    assert(mp != NULL);
    assert(mp->mAVPlayer != 0);
    if (nativeWindow == NULL) {
        SL_LOGE("ANativeWindow is NULL");
        return SL_RESULT_PARAMETER_INVALID;
    }
    SLresult result;
    int err;
    int value;
    // this could crash if app passes in a bad parameter, but that's OK
    err = (*nativeWindow->query)(nativeWindow, NATIVE_WINDOW_CONCRETE_TYPE, &value);
    if (0 != err) {
        SL_LOGE("Query NATIVE_WINDOW_CONCRETE_TYPE on ANativeWindow * %p failed; "
                "errno %d", nativeWindow, err);
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        switch (value) {
        case NATIVE_WINDOW_SURFACE: { // Surface
            SL_LOGV("Displaying on ANativeWindow of type NATIVE_WINDOW_SURFACE");
            android::sp<android::Surface> surface(
                    static_cast<android::Surface *>(nativeWindow));
            android::sp<android::IGraphicBufferProducer> nativeSurfaceTexture(
                    surface->getIGraphicBufferProducer());
            mp->mAVPlayer->setVideoSurfaceTexture(nativeSurfaceTexture);
            result = SL_RESULT_SUCCESS;
            } break;
        case NATIVE_WINDOW_FRAMEBUFFER:              // FramebufferNativeWindow
            FALLTHROUGH_INTENDED;
        default:
            SL_LOGE("ANativeWindow * %p has unknown or unsupported concrete type %d",
                    nativeWindow, value);
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }
    }
    return result;
}
