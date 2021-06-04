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

#define ANDROID_DEFAULT_AUDIOTRACK_BUFFER_SIZE 4096

/**************************************************************************************************
 * AudioPlayer lifecycle
 ****************************/
/*
 * Checks that the combination of source and sink parameters is supported in this implementation.
 * Return
 *     SL_RESULT_SUCCESS
 *     SL_PARAMETER_INVALID
 */
extern SLresult android_audioPlayer_checkSourceSink(CAudioPlayer *pAudioPlayer);

/*
 * Finish the Android-specific pre-Realize initialization of a CAudioPlayer.
 */
extern void android_audioPlayer_create(CAudioPlayer *pAudioPlayer);

/*
 * Allocates and initializes the Android media framework objects intended to be used with the
 * given CAudioPlayer data
 * Return
 *     SL_RESULT_SUCCESS
 *     SL_RESULT_CONTENT_UNSUPPORTED if an error occurred during the allocation and initialization
 *         of the Android resources
 */
extern SLresult android_audioPlayer_realize(CAudioPlayer *pAudioPlayer, SLboolean async);

/*
 * Return
 *     SL_RESULT_SUCCESS
 *     SL_RESULT_PARAMETER_INVALID
 *     SL_RESULT_INTERNAL_ERROR
 *     SL_RESULT_PRECONDITIONS_VIOLATED
 */
extern SLresult android_audioPlayer_setConfig(CAudioPlayer *pAudioPlayer, const SLchar *configKey,
        const void *pConfigValue, SLuint32 valueSize);

/*
 * if pConfigValue is NULL, pValueSize contains the size required for the given key
 *
 * Return
 *     SL_RESULT_SUCCESS
 *     SL_RESULT_PARAMETER_INVALID
 *     SL_RESULT_INTERNAL_ERROR
 *     SL_RESULT_PRECONDITIONS_VIOLATED
 */
extern SLresult android_audioPlayer_getConfig(CAudioPlayer *pAudioPlayer, const SLchar *configKey,
        SLuint32* pValueSize, void *pConfigValue);

extern SLresult android_audioPlayer_preDestroy(CAudioPlayer *pAudioPlayer);

extern SLresult android_audioPlayer_destroy(CAudioPlayer *pAudioPlayer);

/**************************************************************************************************
 * Configuration
 ****************************/
extern SLresult android_audioPlayer_setPlaybackRateAndConstraints(CAudioPlayer *pAudioPlayer,
        SLpermille rate, SLuint32 constraints);

extern SLresult android_audioPlayer_getDuration(IPlay *pPlayItf, SLmillisecond *pDurMsec);

extern void android_audioPlayer_volumeUpdate(CAudioPlayer *pAudioPlayer);

extern SLresult android_audioPlayer_setBufferingUpdateThresholdPerMille(CAudioPlayer *pAudioPlayer,
        SLpermille threshold);

/**************************************************************************************************
 * Metadata Extraction
 ****************************/
/*
 * For all metadata extraction functions:
 * Precondition:
 *     no lock held
 *     pAudioPlayer != NULL
 *     input pointers != NULL (pItemCount, pKeySize, pKey, pValueSize, pValue)
 * Return:
 *     SL_RESULT_SUCCESS
 *     SL_RESULT_PARAMETER_INVALID
 */
extern SLresult android_audioPlayer_metadata_getItemCount(CAudioPlayer *pAudioPlayer,
        SLuint32 *pItemCount);

extern SLresult android_audioPlayer_metadata_getKeySize(CAudioPlayer *pAudioPlayer,
        SLuint32 index, SLuint32 *pKeySize);

extern SLresult android_audioPlayer_metadata_getKey(CAudioPlayer *pAudioPlayer,
        SLuint32 index, SLuint32 size, SLMetadataInfo *pKey);

extern SLresult android_audioPlayer_metadata_getValueSize(CAudioPlayer *pAudioPlayer,
        SLuint32 index, SLuint32 *pValueSize);

extern SLresult android_audioPlayer_metadata_getValue(CAudioPlayer *pAudioPlayer,
        SLuint32 index, SLuint32 size, SLMetadataInfo *pValue);

/**************************************************************************************************
 * Playback control and events
 ****************************/
extern void android_audioPlayer_setPlayState(CAudioPlayer *pAudioPlayer);

extern void android_audioPlayer_usePlayEventMask(CAudioPlayer *pAudioPlayer);

extern SLresult android_audioPlayer_seek(CAudioPlayer *pAudioPlayer, SLmillisecond posMsec);

extern SLresult android_audioPlayer_loop(CAudioPlayer *pAudioPlayer, SLboolean loopEnable);

extern void android_audioPlayer_getPosition(IPlay *pPlayItf, SLmillisecond *pPosMsec);

/**************************************************************************************************
 * Buffer Queue events
 ****************************/
extern void android_audioPlayer_bufferQueue_onRefilled_l(CAudioPlayer *pAudioPlayer);

extern SLresult android_audioPlayer_bufferQueue_onClear(CAudioPlayer *pAudioPlayer);

/**************************************************************************************************
 * Android Buffer Queue
 ****************************/
/* must be called with a lock on pAudioPlayer->mThis */
extern void android_audioPlayer_androidBufferQueue_clear_l(CAudioPlayer *pAudioPlayer);
/* must be called with a lock on pAudioPlayer->mThis */
extern void android_audioPlayer_androidBufferQueue_onRefilled_l(CAudioPlayer *pAudioPlayer);
