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
#define LOG_TAG "statsd_codec"
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

bool statsd_codec(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::CodecData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //
    // android.media.mediacodec.codec   string
    char *codec = NULL;
    if (item->getCString("android.media.mediacodec.codec", &codec)) {
        metrics_proto.set_codec(codec);
    }
    // android.media.mediacodec.mime    string
    char *mime = NULL;
    if (item->getCString("android.media.mediacodec.mime", &mime)) {
        metrics_proto.set_mime(mime);
    }
    // android.media.mediacodec.mode    string
    char *mode = NULL;
    if ( item->getCString("android.media.mediacodec.mode", &mode)) {
        metrics_proto.set_mode(mode);
    }
    // android.media.mediacodec.encoder int32
    int32_t encoder = -1;
    if ( item->getInt32("android.media.mediacodec.encoder", &encoder)) {
        metrics_proto.set_encoder(encoder);
    }
    // android.media.mediacodec.secure  int32
    int32_t secure = -1;
    if ( item->getInt32("android.media.mediacodec.secure", &secure)) {
        metrics_proto.set_secure(secure);
    }
    // android.media.mediacodec.width   int32
    int32_t width = -1;
    if ( item->getInt32("android.media.mediacodec.width", &width)) {
        metrics_proto.set_width(width);
    }
    // android.media.mediacodec.height  int32
    int32_t height = -1;
    if ( item->getInt32("android.media.mediacodec.height", &height)) {
        metrics_proto.set_height(height);
    }
    // android.media.mediacodec.rotation-degrees        int32
    int32_t rotation = -1;
    if ( item->getInt32("android.media.mediacodec.rotation-degrees", &rotation)) {
        metrics_proto.set_rotation(rotation);
    }
    // android.media.mediacodec.crypto  int32 (although missing if not needed
    int32_t crypto = -1;
    if ( item->getInt32("android.media.mediacodec.crypto", &crypto)) {
        metrics_proto.set_crypto(crypto);
    }
    // android.media.mediacodec.profile int32
    int32_t profile = -1;
    if ( item->getInt32("android.media.mediacodec.profile", &profile)) {
        metrics_proto.set_profile(profile);
    }
    // android.media.mediacodec.level   int32
    int32_t level = -1;
    if ( item->getInt32("android.media.mediacodec.level", &level)) {
        metrics_proto.set_level(level);
    }
    // android.media.mediacodec.maxwidth        int32
    int32_t maxwidth = -1;
    if ( item->getInt32("android.media.mediacodec.maxwidth", &maxwidth)) {
        metrics_proto.set_max_width(maxwidth);
    }
    // android.media.mediacodec.maxheight       int32
    int32_t maxheight = -1;
    if ( item->getInt32("android.media.mediacodec.maxheight", &maxheight)) {
        metrics_proto.set_max_height(maxheight);
    }
    // android.media.mediacodec.errcode         int32
    int32_t errcode = -1;
    if ( item->getInt32("android.media.mediacodec.errcode", &errcode)) {
        metrics_proto.set_error_code(errcode);
    }
    // android.media.mediacodec.errstate        string
    char *errstate = NULL;
    if ( item->getCString("android.media.mediacodec.errstate", &errstate)) {
        metrics_proto.set_error_state(errstate);
    }
    // android.media.mediacodec.latency.max  int64
    int64_t latency_max = -1;
    if ( item->getInt64("android.media.mediacodec.latency.max", &latency_max)) {
        metrics_proto.set_latency_max(latency_max);
    }
    // android.media.mediacodec.latency.min  int64
    int64_t latency_min = -1;
    if ( item->getInt64("android.media.mediacodec.latency.min", &latency_min)) {
        metrics_proto.set_latency_min(latency_min);
    }
    // android.media.mediacodec.latency.avg  int64
    int64_t latency_avg = -1;
    if ( item->getInt64("android.media.mediacodec.latency.avg", &latency_avg)) {
        metrics_proto.set_latency_avg(latency_avg);
    }
    // android.media.mediacodec.latency.n    int64
    int64_t latency_count = -1;
    if ( item->getInt64("android.media.mediacodec.latency.n", &latency_count)) {
        metrics_proto.set_latency_count(latency_count);
    }
    // android.media.mediacodec.latency.unknown    int64
    int64_t latency_unknown = -1;
    if ( item->getInt64("android.media.mediacodec.latency.unknown", &latency_unknown)) {
        metrics_proto.set_latency_unknown(latency_unknown);
    }
    // android.media.mediacodec.latency.hist    NOT EMITTED

    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize codec metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_CODEC_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(codec);
    free(mime);
    free(mode);
    free(errstate);

    return true;
}

};
