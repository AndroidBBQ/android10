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

#include <SLES/OpenSLES.h>
#include <OMXAL/OpenMAXAL.h>
#include "attr.h"
#include "handlers.h"

#define NULL 0

/* The entries in this table are sorted first by object ID, and second
 * by attribute mask. The holes in the table are deliberate to permit
 * direct lookup. Don't cross streams!
 */
const AttributeHandler handlerTable[1 + XA_OBJECTID_CAMERADEVICE +
        (SL_OBJECTID_METADATAEXTRACTOR - SL_OBJECTID_ENGINE) + 1][ATTR_INDEX_MAX] = {

// XA IDs map directly to table indices

    [XA_OBJECTID_MEDIAPLAYER] = {
        [ATTR_INDEX_GAIN]        = handler_MediaPlayer_gain,
        [ATTR_INDEX_TRANSPORT]   = handler_MediaPlayer_transport,
        [ATTR_INDEX_POSITION]    = handler_MediaPlayer_position,
        [ATTR_INDEX_ABQ_ENQUEUE] = handler_MediaPlayer_abq_enqueue,
        [ATTR_INDEX_PLAY_STATE]  = handler_MediaPlayer_play_state},

// SL IDs need a little arithmetic to make them contiguous with XA IDs
#define _(id) ((id) - SL_OBJECTID_ENGINE + XA_OBJECTID_CAMERADEVICE + 1)

    [_(SL_OBJECTID_AUDIOPLAYER)] = {
#ifdef ANDROID
        [ATTR_INDEX_GAIN]        = handler_AudioPlayer_gain,
#endif
        [ATTR_INDEX_TRANSPORT]   = handler_AudioPlayer_transport,
        [ATTR_INDEX_POSITION]    = handler_AudioPlayer_position,
        [ATTR_INDEX_BQ_ENQUEUE]  = handler_AudioPlayer_bq_enqueue,
        [ATTR_INDEX_ABQ_ENQUEUE] = handler_AudioPlayer_abq_enqueue,
        [ATTR_INDEX_PLAY_STATE]  = handler_AudioPlayer_play_state},

    [_(SL_OBJECTID_AUDIORECORDER)] = {
        [ATTR_INDEX_TRANSPORT]   = handler_AudioRecorder_transport},

    [_(SL_OBJECTID_MIDIPLAYER)] = {
        [ATTR_INDEX_GAIN]        = handler_MidiPlayer_gain,
        [ATTR_INDEX_POSITION]    = handler_MidiPlayer_position},

    [_(SL_OBJECTID_OUTPUTMIX)] = {
        [ATTR_INDEX_GAIN]        = handler_OutputMix_gain},

};
