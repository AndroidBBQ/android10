/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "sles_allinclusive.h"

// Handlers should return a mask of the attributes which they actually handled,
// or ATTR_NONE if they did not completely handle the attribute change.

#ifdef ANDROID

// SL_OBJECTID_AUDIOPLAYER, ATTR_GAIN
unsigned handler_AudioPlayer_gain(IObject *thiz)
{
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    android_audioPlayer_volumeUpdate(ap);
    return ATTR_GAIN;
}


// SL_OBJECTID_OUTPUTMIX, ATTR_GAIN
unsigned handler_OutputMix_gain(IObject *thiz)
{
    // FIXME update gains on all players attached to this outputmix
    SL_LOGD("[ FIXME: gain update on an SL_OBJECTID_OUTPUTMIX to be implemented ]");
    return ATTR_GAIN;
}


// SL_OBJECTID_MIDIPLAYER, ATTR_GAIN
unsigned handler_MidiPlayer_gain(IObject *thiz)
{
    SL_LOGD("[ FIXME: gain update on an SL_OBJECTID_MIDIPLAYER to be implemented ]");
    return ATTR_GAIN;
}


// XA_OBJECTID_MEDIAPLAYER, ATTR_GAIN
unsigned handler_MediaPlayer_gain(IObject *thiz)
{
    CMediaPlayer *mp = (CMediaPlayer *) thiz;
    android_Player_volumeUpdate(mp);
    return ATTR_GAIN;
}


// SL_OBJECTID_AUDIOPLAYER, ATTR_POSITION
unsigned handler_AudioPlayer_position(IObject *thiz)
{
    CAudioPlayer *ap;
    ap = (CAudioPlayer *) thiz;
    // FIXME provide means to return result for invalid use cases (e.g. buffer queue data source)
    (void) android_audioPlayer_seek(ap, ap->mSeek.mPos);
    return ATTR_POSITION;
}


// SL_OBJECTID_MIDIPLAYER, ATTR_POSITION
unsigned handler_MidiPlayer_position(IObject *thiz)
{
    SL_LOGD("[ FIXME: position update on an SL_OBJECTID_MIDIPLAYER to be implemented ]");
    return ATTR_POSITION;
}


// SL_OBJECTID_AUDIOPLAYER, ATTR_TRANSPORT
unsigned handler_AudioPlayer_transport(IObject *thiz)
{
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    android_audioPlayer_usePlayEventMask(ap);
    return ATTR_TRANSPORT;
}


// SL_OBJECTID_AUDIOPLAYER, ATTR_PLAY_STATE
unsigned handler_AudioPlayer_play_state(IObject *thiz)
{
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    android_audioPlayer_setPlayState(ap);
    return ATTR_PLAY_STATE;
}


// SL_OBJECTID_AUDIORECORDER, ATTR_TRANSPORT
unsigned handler_AudioRecorder_transport(IObject *thiz)
{
    CAudioRecorder* ar = (CAudioRecorder *) thiz;
    android_audioRecorder_useRecordEventMask(ar);
    return ATTR_TRANSPORT;
}


// XA_OBJECTID_MEDIAPLAYER, ATTR_TRANSPORT
unsigned handler_MediaPlayer_transport(IObject *thiz)
{
    CMediaPlayer *mp = (CMediaPlayer *) thiz;
    android_Player_usePlayEventMask(mp);
    return ATTR_TRANSPORT;
}


// XA_OBJECTID_MEDIAPLAYER, ATTR_PLAY_STATE
unsigned handler_MediaPlayer_play_state(IObject *thiz)
{
    CMediaPlayer *mp = (CMediaPlayer *) thiz;
    android::GenericPlayer* avp = mp->mAVPlayer.get();
    if (avp != NULL) {
        android_Player_setPlayState(avp, mp->mPlay.mState, &mp->mAndroidObjState);
    }
    return ATTR_PLAY_STATE;
}


// SL_OBJECTID_AUDIOPLAYER, ATTR_BQ_ENQUEUE
unsigned handler_AudioPlayer_bq_enqueue(IObject *thiz)
{
    // ( buffer queue count is non-empty and play state == PLAYING ) became true
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    assert(SL_PLAYSTATE_PLAYING == ap->mPlay.mState);
    android_audioPlayer_bufferQueue_onRefilled_l(ap);
    return ATTR_BQ_ENQUEUE;
}


// SL_OBJECTID_AUDIOPLAYER, ATTR_ABQ_ENQUEUE
unsigned handler_AudioPlayer_abq_enqueue(IObject *thiz)
{
    // (Android buffer queue count is non-empty and play state == PLAYING ) became true
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    assert(SL_PLAYSTATE_PLAYING == ap->mPlay.mState);
    android_audioPlayer_androidBufferQueue_onRefilled_l(ap);
    return ATTR_ABQ_ENQUEUE;
}


// XA_OBJECTID_MEDIAPLAYER, ATTR_ABQ_ENQUEUE
unsigned handler_MediaPlayer_abq_enqueue(IObject *thiz)
{
    CMediaPlayer* mp = (CMediaPlayer *)thiz;
    if (SL_PLAYSTATE_PLAYING == mp->mPlay.mState) {
        android_Player_androidBufferQueue_onRefilled_l(mp);
    }
    return ATTR_ABQ_ENQUEUE;
}

// XA_OBJECTID_MEDIAPLAYER, ATTR_POSITION
unsigned handler_MediaPlayer_position(IObject *thiz)
{
    CMediaPlayer *mp = (CMediaPlayer *) thiz;
    // FIXME provide means to return result for invalid use cases (e.g. buffer queue data source)
    (void) android_Player_seek(mp, mp->mSeek.mPos);
    return ATTR_POSITION;
}

#else // !defined(ANDROID)

// SL_OBJECTID_AUDIOPLAYER, ATTR_GAIN
unsigned handler_AudioPlayer_gain(IObject *thiz)
{
    CAudioPlayer *ap = (CAudioPlayer *) thiz;
    audioPlayerGainUpdate(ap);
    return ATTR_GAIN;
}

#endif
