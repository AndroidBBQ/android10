/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef ANDROID_MEDIAPLAYER2_TYPES_H
#define ANDROID_MEDIAPLAYER2_TYPES_H

#include <media/mediaplayer_common.h>

#include <media/MediaSource.h>

namespace android {

typedef MediaSource::ReadOptions::SeekMode MediaPlayer2SeekMode;

enum media2_event_type {
    MEDIA2_NOP               = 0, // interface test message
    MEDIA2_PREPARED          = 1,
    MEDIA2_PLAYBACK_COMPLETE = 2,
    MEDIA2_BUFFERING_UPDATE  = 3,
    MEDIA2_SEEK_COMPLETE     = 4,
    MEDIA2_SET_VIDEO_SIZE    = 5,
    MEDIA2_STARTED           = 6,
    MEDIA2_PAUSED            = 7,
    MEDIA2_SKIPPED           = 8,
    MEDIA2_NOTIFY_TIME       = 98,
    MEDIA2_TIMED_TEXT        = 99,
    MEDIA2_ERROR             = 100,
    MEDIA2_INFO              = 200,
    MEDIA2_SUBTITLE_DATA     = 201,
    MEDIA2_META_DATA         = 202,
    MEDIA2_DRM_INFO          = 210,
};

// Generic error codes for the media player framework.  Errors are fatal, the
// playback must abort.
//
// Errors are communicated back to the client using the
// MediaPlayer2Listener::notify method defined below.
// In this situation, 'notify' is invoked with the following:
//   'msg' is set to MEDIA_ERROR.
//   'ext1' should be a value from the enum media2_error_type.
//   'ext2' contains an implementation dependant error code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   1xx: Android Player errors. Something went wrong inside the MediaPlayer2.
//   2xx: Media errors (e.g Codec not supported). There is a problem with the
//        media itself.
//   3xx: Runtime errors. Some extraordinary condition arose making the playback
//        impossible.
//
enum media2_error_type {
    // 0xx
    MEDIA2_ERROR_UNKNOWN = 1,
    // 1xx
    // MEDIA2_ERROR_SERVER_DIED = 100,
    // 2xx
    MEDIA2_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200,
    // 3xx
    MEDIA2_ERROR_FAILED_TO_SET_DATA_SOURCE = 300,
};


// Info and warning codes for the media player framework.  These are non fatal,
// the playback is going on but there might be some user visible issues.
//
// Info and warning messages are communicated back to the client using the
// MediaPlayer2Listener::notify method defined below.  In this situation,
// 'notify' is invoked with the following:
//   'msg' is set to MEDIA_INFO.
//   'ext1' should be a value from the enum media2_info_type.
//   'ext2' contains an implementation dependant info code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   7xx: Android Player info/warning (e.g player lagging behind.)
//   8xx: Media info/warning (e.g media badly interleaved.)
//
enum media2_info_type {
    // 0xx
    MEDIA2_INFO_UNKNOWN = 1,
    // The player just started the playback of this data source.
    MEDIA2_INFO_DATA_SOURCE_START = 2,
    // The player just pushed the very first video frame for rendering
    MEDIA2_INFO_VIDEO_RENDERING_START = 3,
    // The player just pushed the very first audio frame for rendering
    MEDIA2_INFO_AUDIO_RENDERING_START = 4,
    // The player just completed the playback of this data source
    MEDIA2_INFO_DATA_SOURCE_END = 5,
    // The player just completed the playback of all data sources.
    // But this is not visible in native code. Just keep this entry for completeness.
    MEDIA2_INFO_DATA_SOURCE_LIST_END = 6,
    // The player just completed an iteration of playback loop. This event is sent only when
    // looping is enabled.
    MEDIA2_INFO_DATA_SOURCE_REPEAT = 7,

    //1xx
    // The player just prepared a data source.
    MEDIA2_INFO_PREPARED = 100,
    // The player just completed a call play().
    MEDIA2_INFO_COMPLETE_CALL_PLAY = 101,
    // The player just completed a call pause().
    MEDIA2_INFO_COMPLETE_CALL_PAUSE = 102,
    // The player just completed a call seekTo.
    MEDIA2_INFO_COMPLETE_CALL_SEEK = 103,

    // 7xx
    // The video is too complex for the decoder: it can't decode frames fast
    // enough. Possibly only the audio plays fine at this stage.
    MEDIA2_INFO_VIDEO_TRACK_LAGGING = 700,
    // MediaPlayer2 is temporarily pausing playback internally in order to
    // buffer more data.
    MEDIA2_INFO_BUFFERING_START = 701,
    // MediaPlayer2 is resuming playback after filling buffers.
    MEDIA2_INFO_BUFFERING_END = 702,
    // Bandwidth in recent past
    MEDIA2_INFO_NETWORK_BANDWIDTH = 703,

    // 8xx
    // Bad interleaving means that a media has been improperly interleaved or not
    // interleaved at all, e.g has all the video samples first then all the audio
    // ones. Video is playing but a lot of disk seek may be happening.
    MEDIA2_INFO_BAD_INTERLEAVING = 800,
    // The media is not seekable (e.g live stream).
    MEDIA2_INFO_NOT_SEEKABLE = 801,
    // New media metadata is available.
    MEDIA2_INFO_METADATA_UPDATE = 802,
    // Audio can not be played.
    MEDIA2_INFO_PLAY_AUDIO_ERROR = 804,
    // Video can not be played.
    MEDIA2_INFO_PLAY_VIDEO_ERROR = 805,

    //9xx
    MEDIA2_INFO_TIMED_TEXT_ERROR = 900,
};

// Do not change these values without updating their counterparts in MediaPlayer2.java
enum mediaplayer2_states {
    MEDIAPLAYER2_STATE_IDLE         = 1001,
    MEDIAPLAYER2_STATE_PREPARED     = 1002,
    MEDIAPLAYER2_STATE_PAUSED       = 1003,
    MEDIAPLAYER2_STATE_PLAYING      = 1004,
    MEDIAPLAYER2_STATE_ERROR        = 1005,
};

enum media_player2_internal_states {
    MEDIA_PLAYER2_STATE_ERROR        = 0,
    MEDIA_PLAYER2_IDLE               = 1 << 0,
    MEDIA_PLAYER2_INITIALIZED        = 1 << 1,
    MEDIA_PLAYER2_PREPARING          = 1 << 2,
    MEDIA_PLAYER2_PREPARED           = 1 << 3,
    MEDIA_PLAYER2_STARTED            = 1 << 4,
    MEDIA_PLAYER2_PAUSED             = 1 << 5,
    MEDIA_PLAYER2_PLAYBACK_COMPLETE  = 1 << 6
};

// Keep KEY_PARAMETER_* in sync with MediaPlayer2.java.
// The same enum space is used for both set and get, in case there are future keys that
// can be both set and get.  But as of now, all parameters are either set only or get only.
enum media2_parameter_keys {
    // Streaming/buffering parameters
    MEDIA2_KEY_PARAMETER_CACHE_STAT_COLLECT_FREQ_MS = 1100,            // set only

    // Return a Parcel containing a single int, which is the channel count of the
    // audio track, or zero for error (e.g. no audio track) or unknown.
    MEDIA2_KEY_PARAMETER_AUDIO_CHANNEL_COUNT = 1200,                   // get only

    // Playback rate expressed in permille (1000 is normal speed), saved as int32_t, with negative
    // values used for rewinding or reverse playback.
    MEDIA2_KEY_PARAMETER_PLAYBACK_RATE_PERMILLE = 1300,                // set only

    // Set a Parcel containing the value of a parcelled Java AudioAttribute instance
    MEDIA2_KEY_PARAMETER_AUDIO_ATTRIBUTES = 1400                       // set only
};

// Keep INVOKE_ID_* in sync with MediaPlayer2.java.
enum media_player2_invoke_ids {
    MEDIA_PLAYER2_INVOKE_ID_GET_TRACK_INFO = 1,
    MEDIA_PLAYER2_INVOKE_ID_ADD_EXTERNAL_SOURCE = 2,
    MEDIA_PLAYER2_INVOKE_ID_ADD_EXTERNAL_SOURCE_FD = 3,
    MEDIA_PLAYER2_INVOKE_ID_SELECT_TRACK = 4,
    MEDIA_PLAYER2_INVOKE_ID_UNSELECT_TRACK = 5,
    MEDIA_PLAYER2_INVOKE_ID_SET_VIDEO_SCALING_MODE = 6,
    MEDIA_PLAYER2_INVOKE_ID_GET_SELECTED_TRACK = 7
};

}; // namespace android

#endif // ANDROID_MEDIAPLAYER2_TYPES_H
