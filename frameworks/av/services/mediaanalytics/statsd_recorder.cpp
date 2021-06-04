/*
 * Copyright (C) 2019 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "statsd_recorder"
#include <utils/Log.h>

#include <dirent.h>
#include <inttypes.h>
#include <pthread.h>
#include <pwd.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <statslog.h>

#include "MediaAnalyticsService.h"
#include "frameworks/base/core/proto/android/stats/mediametrics/mediametrics.pb.h"
#include "iface_statsd.h"

namespace android {

bool statsd_recorder(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::RecorderData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //

    // string kRecorderAudioMime = "android.media.mediarecorder.audio.mime";
    char *audio_mime = NULL;
    if (item->getCString("android.media.mediarecorder.audio.mime", &audio_mime)) {
        metrics_proto.set_audio_mime(audio_mime);
    }
    // string kRecorderVideoMime = "android.media.mediarecorder.video.mime";
    char *video_mime = NULL;
    if (item->getCString("android.media.mediarecorder.video.mime", &video_mime)) {
        metrics_proto.set_video_mime(video_mime);
    }
    // int32 kRecorderVideoProfile = "android.media.mediarecorder.video-encoder-profile";
    int32_t videoProfile = -1;
    if (item->getInt32("android.media.mediarecorder.video-encoder-profile", &videoProfile)) {
        metrics_proto.set_video_profile(videoProfile);
    }
    // int32 kRecorderVideoLevel = "android.media.mediarecorder.video-encoder-level";
    int32_t videoLevel = -1;
    if (item->getInt32("android.media.mediarecorder.video-encoder-level", &videoLevel)) {
        metrics_proto.set_video_level(videoLevel);
    }
    // int32 kRecorderWidth = "android.media.mediarecorder.width";
    int32_t width = -1;
    if (item->getInt32("android.media.mediarecorder.width", &width)) {
        metrics_proto.set_width(width);
    }
    // int32 kRecorderHeight = "android.media.mediarecorder.height";
    int32_t height = -1;
    if (item->getInt32("android.media.mediarecorder.height", &height)) {
        metrics_proto.set_height(height);
    }
    // int32 kRecorderRotation = "android.media.mediarecorder.rotation";
    int32_t rotation = -1;                      // default to 0?
    if (item->getInt32("android.media.mediarecorder.rotation", &rotation)) {
        metrics_proto.set_rotation(rotation);
    }
    // int32 kRecorderFrameRate = "android.media.mediarecorder.frame-rate";
    int32_t framerate = -1;
    if (item->getInt32("android.media.mediarecorder.frame-rate", &framerate)) {
        metrics_proto.set_framerate(framerate);
    }

    // int32 kRecorderCaptureFps = "android.media.mediarecorder.capture-fps";
    int32_t captureFps = -1;
    if (item->getInt32("android.media.mediarecorder.capture-fps", &captureFps)) {
        metrics_proto.set_capture_fps(captureFps);
    }
    // double kRecorderCaptureFpsEnable = "android.media.mediarecorder.capture-fpsenable";
    double captureFpsEnable = -1;
    if (item->getDouble("android.media.mediarecorder.capture-fpsenable", &captureFpsEnable)) {
        metrics_proto.set_capture_fps_enable(captureFpsEnable);
    }

    // int64 kRecorderDurationMs = "android.media.mediarecorder.durationMs";
    int64_t durationMs = -1;
    if (item->getInt64("android.media.mediarecorder.durationMs", &durationMs)) {
        metrics_proto.set_duration_millis(durationMs);
    }
    // int64 kRecorderPaused = "android.media.mediarecorder.pausedMs";
    int64_t pausedMs = -1;
    if (item->getInt64("android.media.mediarecorder.pausedMs", &pausedMs)) {
        metrics_proto.set_paused_millis(pausedMs);
    }
    // int32 kRecorderNumPauses = "android.media.mediarecorder.NPauses";
    int32_t pausedCount = -1;
    if (item->getInt32("android.media.mediarecorder.NPauses", &pausedCount)) {
        metrics_proto.set_paused_count(pausedCount);
    }

    // int32 kRecorderAudioBitrate = "android.media.mediarecorder.audio-bitrate";
    int32_t audioBitrate = -1;
    if (item->getInt32("android.media.mediarecorder.audio-bitrate", &audioBitrate)) {
        metrics_proto.set_audio_bitrate(audioBitrate);
    }
    // int32 kRecorderAudioChannels = "android.media.mediarecorder.audio-channels";
    int32_t audioChannels = -1;
    if (item->getInt32("android.media.mediarecorder.audio-channels", &audioChannels)) {
        metrics_proto.set_audio_channels(audioChannels);
    }
    // int32 kRecorderAudioSampleRate = "android.media.mediarecorder.audio-samplerate";
    int32_t audioSampleRate = -1;
    if (item->getInt32("android.media.mediarecorder.audio-samplerate", &audioSampleRate)) {
        metrics_proto.set_audio_samplerate(audioSampleRate);
    }

    // int32 kRecorderMovieTimescale = "android.media.mediarecorder.movie-timescale";
    int32_t movieTimescale = -1;
    if (item->getInt32("android.media.mediarecorder.movie-timescale", &movieTimescale)) {
        metrics_proto.set_movie_timescale(movieTimescale);
    }
    // int32 kRecorderAudioTimescale = "android.media.mediarecorder.audio-timescale";
    int32_t audioTimescale = -1;
    if (item->getInt32("android.media.mediarecorder.audio-timescale", &audioTimescale)) {
        metrics_proto.set_audio_timescale(audioTimescale);
    }
    // int32 kRecorderVideoTimescale = "android.media.mediarecorder.video-timescale";
    int32_t videoTimescale = -1;
    if (item->getInt32("android.media.mediarecorder.video-timescale", &videoTimescale)) {
        metrics_proto.set_video_timescale(videoTimescale);
    }

    // int32 kRecorderVideoBitrate = "android.media.mediarecorder.video-bitrate";
    int32_t videoBitRate = -1;
    if (item->getInt32("android.media.mediarecorder.video-bitrate", &videoBitRate)) {
        metrics_proto.set_video_bitrate(videoBitRate);
    }
    // int32 kRecorderVideoIframeInterval = "android.media.mediarecorder.video-iframe-interval";
    int32_t iFrameInterval = -1;
    if (item->getInt32("android.media.mediarecorder.video-iframe-interval", &iFrameInterval)) {
        metrics_proto.set_iframe_interval(iFrameInterval);
    }

    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize recorder metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_RECORDER_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(audio_mime);
    free(video_mime);

    return true;
}

};
