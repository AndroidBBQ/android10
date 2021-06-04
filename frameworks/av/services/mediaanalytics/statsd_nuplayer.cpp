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
#define LOG_TAG "statsd_nuplayer"
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

/*
 *  handles nuplayer AND nuplayer2
 *  checks for the union of what the two players generate
 */
bool statsd_nuplayer(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::NuPlayerData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //

    // differentiate between nuplayer and nuplayer2
    metrics_proto.set_whichplayer(item->getKey().c_str());

    char *video_mime = NULL;
    if (item->getCString("android.media.mediaplayer.video.mime", &video_mime)) {
        metrics_proto.set_video_mime(video_mime);
    }
    char *video_codec = NULL;
    if (item->getCString("android.media.mediaplayer.video.codec", &video_codec)) {
        metrics_proto.set_video_codec(video_codec);
    }

    int32_t width = -1;
    if (item->getInt32("android.media.mediaplayer.width", &width)) {
        metrics_proto.set_width(width);
    }
    int32_t height = -1;
    if (item->getInt32("android.media.mediaplayer.height", &height)) {
        metrics_proto.set_height(height);
    }

    int64_t frames = -1;
    if (item->getInt64("android.media.mediaplayer.frames", &frames)) {
        metrics_proto.set_frames(frames);
    }
    int64_t frames_dropped = -1;
    if (item->getInt64("android.media.mediaplayer.dropped", &frames_dropped)) {
        metrics_proto.set_frames_dropped(frames_dropped);
    }
    int64_t frames_dropped_startup = -1;
    if (item->getInt64("android.media.mediaplayer.startupdropped", &frames_dropped_startup)) {
        metrics_proto.set_frames_dropped_startup(frames_dropped_startup);
    }
    double fps = -1.0;
    if (item->getDouble("android.media.mediaplayer.fps", &fps)) {
        metrics_proto.set_framerate(fps);
    }

    char *audio_mime = NULL;
    if (item->getCString("android.media.mediaplayer.audio.mime", &audio_mime)) {
        metrics_proto.set_audio_mime(audio_mime);
    }
    char *audio_codec = NULL;
    if (item->getCString("android.media.mediaplayer.audio.codec", &audio_codec)) {
        metrics_proto.set_audio_codec(audio_codec);
    }

    int64_t duration_ms = -1;
    if (item->getInt64("android.media.mediaplayer.durationMs", &duration_ms)) {
        metrics_proto.set_duration_millis(duration_ms);
    }
    int64_t playing_ms = -1;
    if (item->getInt64("android.media.mediaplayer.playingMs", &playing_ms)) {
        metrics_proto.set_playing_millis(playing_ms);
    }

    int32_t err = -1;
    if (item->getInt32("android.media.mediaplayer.err", &err)) {
        metrics_proto.set_error(err);
    }
    int32_t error_code = -1;
    if (item->getInt32("android.media.mediaplayer.errcode", &error_code)) {
        metrics_proto.set_error_code(error_code);
    }
    char *error_state = NULL;
    if (item->getCString("android.media.mediaplayer.errstate", &error_state)) {
        metrics_proto.set_error_state(error_state);
    }

    char *data_source_type = NULL;
    if (item->getCString("android.media.mediaplayer.dataSource", &data_source_type)) {
        metrics_proto.set_data_source_type(data_source_type);
    }

    int64_t rebufferingMs = -1;
    if (item->getInt64("android.media.mediaplayer.rebufferingMs", &rebufferingMs)) {
        metrics_proto.set_rebuffering_millis(rebufferingMs);
    }
    int32_t rebuffers = -1;
    if (item->getInt32("android.media.mediaplayer.rebuffers", &rebuffers)) {
        metrics_proto.set_rebuffers(rebuffers);
    }
    int32_t rebufferExit = -1;
    if (item->getInt32("android.media.mediaplayer.rebufferExit", &rebufferExit)) {
        metrics_proto.set_rebuffer_at_exit(rebufferExit);
    }


    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize nuplayer metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_NUPLAYER_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(video_mime);
    free(video_codec);
    free(audio_mime);
    free(audio_codec);
    free(error_state);
    free(data_source_type);

    return true;
}

};
