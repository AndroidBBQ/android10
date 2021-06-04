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

//-----------------------------------------------------------------------------
// Android general purpose errors
//----------------------
#define ERROR_SHOULDNT_BE_HERE_S \
        "Reached an invalid code path in %s"

//-----------------------------------------------------------------------------
// Android AudioPlayer errors
//----------------------
#define ERROR_PLAYERSTREAMTYPE_SET_UNKNOWN_TYPE \
        "Cannot set stream type: unknown or invalid stream type"
#define ERROR_PLAYERSTREAMTYPE_REALIZED \
        "Cannot set stream type: audio player already realized"
#define ERROR_PLAYERREALIZE_UNKNOWN_DATASOURCE_LOCATOR \
        "Cannot realize AudioPlayer: with unknown data source locator"
#define ERROR_PLAYERREALIZE_UNEXPECTED_OBJECT_TYPE_D \
        "Cannot realize AudioPlayer: unexpected object type \%d"
#define ERROR_PLAYERDESTROY_UNEXPECTED_OBJECT_TYPE_D \
        "Cannot destroy AudioPlayer: unexpected object type \%d"
#define ERROR_PLAYERSETPLAYSTATE_UNEXPECTED_OBJECT_TYPE_D \
        "Cannot set AudioPlayer playstate: unexpected object type \%d"
#define ERROR_PLAYERSETPLAYSTATE_INVALID_OBJECT_STATE_D \
        "Cannot set AudioPlayer playstate to %d: Android object in invalid state"
#define ERROR_PLAYER_NEW_NULL_TRACK \
        "Internal error: new AudioTrack shouldn't be NULL"
#define ERROR_PLAYER_PREFETCH_d \
        "Error (%d) encountered while prefetching"

//-----------------------------------------------------------------------------
// Android AudioRecorder errors
//----------------------
#define ERROR_RECORDER_SINK_MUST_BE_ANDROIDSIMPLEBUFFERQUEUE \
        "Cannot create AudioRecorder: data sink must be SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE"
#define ERROR_RECORDER_SINK_FORMAT_MUST_BE_PCM \
        "Cannot create AudioRecorder: data sink must be in PCM format"
#define ERROR_RECORDER_SOURCE_MUST_BE_IODEVICE \
        "Cannot create AudioRecorder: data source must be SL_DATALOCATOR_IODEVICE"
#define ERROR_RECORDER_IODEVICE_MUST_BE_AUDIOINPUT \
        "Cannot create AudioRecorder: data source device type must be SL_IODEVICE_AUDIOINPUT"
#define ERROR_RECORDER_INPUT_ID_MUST_BE_DEFAULT \
        "Cannot create AudioRecorder: data source device ID must be SL_DEFAULTDEVICEID_AUDIOINPUT"
#define ERROR_RECORDERPRESET_SET_UNKNOWN_PRESET \
        "Cannot set recording preset: unknown or invalid preset"
#define ERROR_RECORDERPRESET_REALIZED \
        "Cannot set recording preset: audio recorder already realized"

//-----------------------------------------------------------------------------
// Android Configuration errors
//----------------------
#define ERROR_CONFIG_UNKNOWN_KEY \
        "Configuration error: unknown key"
#define ERROR_CONFIG_VALUESIZE_TOO_LOW \
        "Configuration error: value size too low to store valid value"
#define ERROR_CONFIG_NULL_PARAM \
        "Configuration error: invalid NULL parameter"
#define ERROR_CONFIG_PERF_MODE_UNKNOWN \
        "Cannot set performance mode: unknown or invalid mode"
#define ERROR_CONFIG_PERF_MODE_REALIZED \
        "Cannot set performance mode in: player/recorder realized"
