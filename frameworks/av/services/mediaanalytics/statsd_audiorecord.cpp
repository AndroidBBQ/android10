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
#define LOG_TAG "statsd_audiorecord"
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

bool statsd_audiorecord(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::AudioRecordData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //
    char *encoding = NULL;
    if (item->getCString("android.media.audiorecord.encoding", &encoding)) {
        metrics_proto.set_encoding(encoding);
    }

    char *source = NULL;
    if (item->getCString("android.media.audiorecord.source", &source)) {
        metrics_proto.set_source(source);
    }

    int32_t latency = -1;
    if (item->getInt32("android.media.audiorecord.latency", &latency)) {
        metrics_proto.set_latency(latency);
    }

    int32_t samplerate = -1;
    if (item->getInt32("android.media.audiorecord.samplerate", &samplerate)) {
        metrics_proto.set_samplerate(samplerate);
    }

    int32_t channels = -1;
    if (item->getInt32("android.media.audiorecord.channels", &channels)) {
        metrics_proto.set_channels(channels);
    }

    int64_t createdMs = -1;
    if (item->getInt64("android.media.audiorecord.createdMs", &createdMs)) {
        metrics_proto.set_created_millis(createdMs);
    }

    int64_t durationMs = -1;
    if (item->getInt64("android.media.audiorecord.durationMs", &durationMs)) {
        metrics_proto.set_duration_millis(durationMs);
    }

    int32_t count = -1;
    if (item->getInt32("android.media.audiorecord.n", &count)) {
        metrics_proto.set_count(count);
    }

    int32_t errcode = -1;
    if (item->getInt32("android.media.audiorecord.errcode", &errcode)) {
        metrics_proto.set_error_code(errcode);
    } else if (item->getInt32("android.media.audiorecord.lastError.code", &errcode)) {
        metrics_proto.set_error_code(errcode);
    }

    char *errfunc = NULL;
    if (item->getCString("android.media.audiorecord.errfunc", &errfunc)) {
        metrics_proto.set_error_function(errfunc);
    } else if (item->getCString("android.media.audiorecord.lastError.at", &errfunc)) {
        metrics_proto.set_error_function(errfunc);
    }

    // portId (int32)
    int32_t port_id = -1;
    if (item->getInt32("android.media.audiorecord.portId", &port_id)) {
        metrics_proto.set_port_id(count);
    }
    // frameCount (int32)
    int32_t frameCount = -1;
    if (item->getInt32("android.media.audiorecord.frameCount", &frameCount)) {
        metrics_proto.set_frame_count(frameCount);
    }
    // attributes (string)
    char *attributes = NULL;
    if (item->getCString("android.media.audiorecord.attributes", &attributes)) {
        metrics_proto.set_attributes(attributes);
    }
    // channelMask (int64)
    int64_t channelMask = -1;
    if (item->getInt64("android.media.audiorecord.channelMask", &channelMask)) {
        metrics_proto.set_channel_mask(channelMask);
    }
    // startcount (int64)
    int64_t startcount = -1;
    if (item->getInt64("android.media.audiorecord.startcount", &startcount)) {
        metrics_proto.set_start_count(startcount);
    }


    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize audiorecord metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_AUDIORECORD_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(encoding);
    free(source);
    free(errfunc);
    free(attributes);

    return true;
}

};
