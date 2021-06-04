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


/**
 * Used to define the mapping from an OpenSL ES or OpenMAX AL object to an Android
 * media framework object
 */
enum AndroidObjectType {
    INVALID_TYPE                                =-1,
    // audio player, playing from a URI or FD data source
    AUDIOPLAYER_FROM_URIFD                      = 0,
    // audio player, playing PCM buffers in a buffer queue data source
    AUDIOPLAYER_FROM_PCM_BUFFERQUEUE            = 1,
    // audio player, playing transport stream packets in an Android buffer queue data source
    AUDIOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE      = 2,
    // audio player, decoding from a URI or FD data source to a buffer queue data sink in PCM format
    AUDIOPLAYER_FROM_URIFD_TO_PCM_BUFFERQUEUE   = 3,
    // audio video player, playing transport stream packets in an Android buffer queue data source
    AUDIOVIDEOPLAYER_FROM_TS_ANDROIDBUFFERQUEUE = 4,
    // audio video player, playing from a URI or FD data source
    AUDIOVIDEOPLAYER_FROM_URIFD                 = 5,
    // audio recorder, recording from an input device data source, streamed into a
    //   PCM buffer queue data sink
    AUDIORECORDER_FROM_MIC_TO_PCM_BUFFERQUEUE   = 6,
    // audio player, decoding from an Android buffer queue with ADTS data,
    //   to a buffer queue data sink in PCM format
    AUDIOPLAYER_FROM_ADTS_ABQ_TO_PCM_BUFFERQUEUE = 7,
    NUM_AUDIOPLAYER_MAP_TYPES
};


/**
 * Used to define the states of the OpenSL ES / OpenMAX AL object initialization and preparation
 * with regards to the Android-side of the data, in particular as affected by the play state.
 */
enum AndroidObjectState {
    ANDROID_UNINITIALIZED = -1, // never called SetPlayState(PAUSED or PLAYING)
    ANDROID_PREPARING,          // currently preparing due to first SetPlayState(PAUSED or PLAYING)
    ANDROID_READY,              // preparation has completed, and will not be attempted again
    // NUM_ANDROID_STATES       // unused
};

/**
 * Must be one of the supported stream types that can be set through SLAndroidConfigurationItf
 */
#define ANDROID_DEFAULT_OUTPUT_STREAM_TYPE AUDIO_STREAM_MUSIC

#define PLAYER_FAILURE ((int32_t) android::UNKNOWN_ERROR)
#define PLAYER_SUCCESS ((int32_t) android::NO_ERROR)

#define PLAYER_FD_FIND_FILE_SIZE ((int64_t)0xFFFFFFFFFFFFFFFFll)

#define MPEG2_TS_PACKET_SIZE 188
#define MPEG2_TS_PACKET_SYNC 0x47

struct AudioPlayback_Parameters {
    audio_stream_type_t streamType;
    audio_session_t sessionId;
};

/**
 * Structure to maintain the set of audio levels about a player
 */
struct AndroidAudioLevels {
    /**
     * Send level to aux effect, there's a single aux bus, so there's a single level
     */
    // FIXME not used yet, will be used when supporting effects in OpenMAX AL
    //SLmillibel mAuxSendLevel;
    /**
     * Attenuation factor derived from direct level
     */
    // FIXME not used yet, will be used when supporting effects in OpenMAX AL
    //float mAmplFromDirectLevel;
    /**
     * Android Left/Right volume
     * The final volume of an Android AudioTrack or MediaPlayer is a stereo amplification
     * (or attenuation) represented as a float from 0.0f to 1.0f
     */
    float mFinalVolume[STEREO_CHANNELS];
};


/**
 * Event notification callback from Android to SL ES framework
 */
typedef void (*notif_cbf_t)(int event, int data1, int data2, void* notifUser);

/**
 * Audio data push callback from Android objects to SL ES framework
 */
typedef size_t (*data_push_cbf_t)(const uint8_t *data, size_t size, CAudioPlayer* ap);


/**
 * Events sent to mNotifyClient during prepare, prefetch, and playback
 * used in APlayer::notify() and AMessage::findxxx()
 */
#define PLAYEREVENT_PREPARED                "prep"
#define PLAYEREVENT_PREFETCHSTATUSCHANGE    "prsc"
#define PLAYEREVENT_PREFETCHFILLLEVELUPDATE "pflu"
#define PLAYEREVENT_ENDOFSTREAM             "eos"
#define PLAYEREVENT_VIDEO_SIZE_UPDATE       "vsiz"
#define PLAYEREVENT_CHANNEL_COUNT           "ccnt"  // channel count is now known
#define PLAYEREVENT_PLAY                    "play"  // SL_PLAYEVENT_*
#define PLAYEREVENT_ERRORAFTERPREPARE       "easp"  // error after successful prepare


/**
 * Time value when time is unknown. Used for instance for duration or playback position
 */
#define ANDROID_UNKNOWN_TIME (-1)

/**
 * Event mask for MPEG-2 TS events associated with TS data
 */
#define ANDROID_MP2TSEVENT_NONE          ((SLuint32) 0x0)
// buffer is at End Of Stream
#define ANDROID_MP2TSEVENT_EOS           ((SLuint32) 0x1)
// buffer marks a discontinuity with previous TS data, resume display as soon as possible
#define ANDROID_MP2TSEVENT_DISCONTINUITY ((SLuint32) 0x1 << 1)
// buffer marks a discontinuity with previous TS data, resume display upon reaching the
// associated presentation time stamp
#define ANDROID_MP2TSEVENT_DISCON_NEWPTS ((SLuint32) 0x1 << 2)
// buffer marks a format change with previous TS data, resume display as soon as possible
#define ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL  ((SLuint32) 0x1 << 3)
#define ANDROID_MP2TSEVENT_FORMAT_CHANGE_VIDEO ((SLuint32) 0x1 << 4)

/**
 * Event mask for AAC ADTS events associated with ADTS data
 */
#define ANDROID_ADTSEVENT_NONE           ANDROID_MP2TSEVENT_NONE
// buffer is at End Of Stream
#define ANDROID_ADTSEVENT_EOS            ANDROID_MP2TSEVENT_EOS

/**
 * Types of buffers stored in Android Buffer Queues, see IAndroidBufferQueue.mBufferType
 */
enum AndroidBufferType_type {
    kAndroidBufferTypeInvalid = ((SLuint16) 0x0),
    kAndroidBufferTypeMpeg2Ts = ((SLuint16) 0x1),
    kAndroidBufferTypeAacadts = ((SLuint16) 0x2),
};

/**
 * MIME types required for data in Android Buffer Queues
 */
// The preferred MIME types for new applications are in ../../include/*/OpenMAXAL_Android.h
// The legacy MIME type used elsewhere in the Android framework for AAC ADTS is below:
#define ANDROID_MIME_AACADTS_ANDROID_FRAMEWORK "audio/aac-adts"

/**
 * Notification thresholds relative to content duration in the cache
 */
#define DURATION_CACHED_HIGH_MS  30000 // 30s
#define DURATION_CACHED_MED_MS   10000 // 10s
#define DURATION_CACHED_LOW_MS    2000 //  2s


namespace android {

/**
 * Prefetch cache status
 */
enum CacheStatus_t {
        kStatusUnknown = -1,
        kStatusEmpty   = 0,
        kStatusLow,
        kStatusIntermediate,
        kStatusEnough,
        kStatusHigh
};

enum {
    kDataLocatorNone = 0,
    kDataLocatorUri  = 1,
    kDataLocatorFd   = 2,
};

struct FdInfo {
    int fd;
    int64_t offset;
    int64_t length;
    bool mCloseAfterUse;    // whether to close fd after last reference to fd
};

// TODO currently used by SfPlayer, to replace by DataLocator2
union DataLocator {
    char* uri;
    FdInfo fdi;
};

union DataLocator2 {
    const char* uriRef;
    FdInfo fdi;
};


#define ANDROID_PERFORMANCE_MODE_NONE            ((SLuint32) 0x1 << 0)
#define ANDROID_PERFORMANCE_MODE_LATENCY         ((SLuint32) 0x1 << 1)
#define ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS ((SLuint32) 0x1 << 2)
#define ANDROID_PERFORMANCE_MODE_POWER_SAVING    ((SLuint32) 0x1 << 3)

#define ANDROID_PERFORMANCE_MODE_DEFAULT ANDROID_PERFORMANCE_MODE_LATENCY
#define ANDROID_PERFORMANCE_MODE_ALL (ANDROID_PERFORMANCE_MODE_LATENCY | \
                                      ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS | \
                                      ANDROID_PERFORMANCE_MODE_NONE | \
                                      ANDROID_PERFORMANCE_MODE_POWER_SAVING)


} // namespace android
