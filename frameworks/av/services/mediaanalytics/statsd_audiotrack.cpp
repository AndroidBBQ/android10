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
#define LOG_TAG "statsd_audiotrack"
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

bool statsd_audiotrack(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::AudioTrackData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //

    // static constexpr char kAudioTrackStreamType[] = "android.media.audiotrack.streamtype";
    // optional string streamType;
    char *streamtype = NULL;
    if (item->getCString("android.media.audiotrack.streamtype", &streamtype)) {
        metrics_proto.set_stream_type(streamtype);
    }

    // static constexpr char kAudioTrackContentType[] = "android.media.audiotrack.type";
    // optional string contentType;
    char *contenttype = NULL;
    if (item->getCString("android.media.audiotrack.type", &contenttype)) {
        metrics_proto.set_content_type(contenttype);
    }

    // static constexpr char kAudioTrackUsage[] = "android.media.audiotrack.usage";
    // optional string trackUsage;
    char *trackusage = NULL;
    if (item->getCString("android.media.audiotrack.usage", &trackusage)) {
        metrics_proto.set_track_usage(trackusage);
    }

    // static constexpr char kAudioTrackSampleRate[] = "android.media.audiotrack.samplerate";
    // optional int32 samplerate;
    int32_t samplerate = -1;
    if (item->getInt32("android.media.audiotrack.samplerate", &samplerate)) {
        metrics_proto.set_sample_rate(samplerate);
    }

    // static constexpr char kAudioTrackChannelMask[] = "android.media.audiotrack.channelmask";
    // optional int64 channelMask;
    int64_t channelMask = -1;
    if (item->getInt64("android.media.audiotrack.channelmask", &channelMask)) {
        metrics_proto.set_channel_mask(channelMask);
    }

    // NB: These are not yet exposed as public Java API constants.
    // static constexpr char kAudioTrackUnderrunFrames[] = "android.media.audiotrack.underrunframes";
    // optional int32 underrunframes;
    int32_t underrunframes = -1;
    if (item->getInt32("android.media.audiotrack.underrunframes", &underrunframes)) {
        metrics_proto.set_underrun_frames(underrunframes);
    }

    // static constexpr char kAudioTrackStartupGlitch[] = "android.media.audiotrack.glitch.startup";
    // optional int32 startupglitch;
    int32_t startupglitch = -1;
    if (item->getInt32("android.media.audiotrack.glitch.startup", &startupglitch)) {
        metrics_proto.set_startup_glitch(startupglitch);
    }

    // portId (int32)
    int32_t port_id = -1;
    if (item->getInt32("android.media.audiotrack.portId", &port_id)) {
        metrics_proto.set_port_id(port_id);
    }
    // encoding (string)
    char *encoding = NULL;
    if (item->getCString("android.media.audiotrack.encoding", &encoding)) {
        metrics_proto.set_encoding(encoding);
    }
    // frameCount (int32)
    int32_t frame_count = -1;
    if (item->getInt32("android.media.audiotrack.frameCount", &frame_count)) {
        metrics_proto.set_frame_count(frame_count);
    }
    // attributes (string)
    char *attributes = NULL;
    if (item->getCString("android.media.audiotrack.attributes", &attributes)) {
        metrics_proto.set_attributes(attributes);
    }

    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize audiotrack metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_AUDIOTRACK_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(streamtype);
    free(contenttype);
    free(trackusage);
    free(encoding);
    free(attributes);

    return true;
}

};
