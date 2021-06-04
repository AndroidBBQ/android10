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

/**************************************************************************************************
 * Player lifecycle
 ****************************/
extern XAresult android_Player_checkSourceSink(CMediaPlayer *mp);

extern XAresult android_Player_create(CMediaPlayer *mp);

extern XAresult android_Player_realize(CMediaPlayer *mp, SLboolean async);

extern XAresult android_Player_destroy(CMediaPlayer *mp);

extern XAresult android_Player_preDestroy(CMediaPlayer *mp);


/**************************************************************************************************
 * Configuration
 ****************************/

/**
 *  pre-conditions:
 *      pMediaPlayer != NULL
 *      pMediaPlayer->mAVPlayer != 0 (player is realized)
 *      nativeWindow can be NULL, but if NULL it is treated as an error
 */
extern SLresult android_Player_setNativeWindow(CMediaPlayer *pMediaPlayer,
        ANativeWindow *nativeWindow);

/**
 * pre-conditions:
 *    pPlayItf != NULL
 *    pDurMsec, pPosMsec ! = NULL
 *    IObjectToObjectID( ((IPlay *) self)->mThis ) == XA_OBJECTID_MEDIAPLAYER
 */
extern XAresult android_Player_getDuration(IPlay *pPlayItf, SLmillisecond *pDurMsec);
extern XAresult android_Player_getPosition(IPlay *pPlayItf, SLmillisecond *pPosMsec);

/**
 * pre-condition: mp != NULL
 */
extern void android_Player_volumeUpdate(CMediaPlayer *mp);
extern void android_Player_usePlayEventMask(CMediaPlayer *mp);

/**************************************************************************************************
 * Playback control and events
 ****************************/
/**
 * pre-condition: gp != 0
 */
extern XAresult android_Player_setPlayState(const android::sp<android::GenericPlayer> &gp,
        SLuint32 playState,
        AndroidObjectState* pObjState);

/**
 * for all functions below: pre-condition: mp != NULL
 */
extern XAresult android_Player_seek(CMediaPlayer *mp, SLmillisecond posMsec);
extern XAresult android_Player_loop(CMediaPlayer *mp, SLboolean loopEnable);



/**************************************************************************************************
 * Buffer Queue events
 ****************************/

/**************************************************************************************************
 * Android Buffer Queue
 ****************************/

/* must be called with a lock on mp->mThis */
extern void android_Player_androidBufferQueue_clear_l(CMediaPlayer *mp);
/* must be called with a lock on mp->mThis */
extern void android_Player_androidBufferQueue_onRefilled_l(CMediaPlayer *mp);
