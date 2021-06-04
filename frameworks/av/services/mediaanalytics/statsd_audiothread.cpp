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
#define LOG_TAG "statsd_audiothread"
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

bool statsd_audiothread(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::AudioThreadData metrics_proto;

#define	MM_PREFIX "android.media.audiothread."

    // flesh out the protobuf we'll hand off with our data
    //
    char *mytype = NULL;
    if (item->getCString(MM_PREFIX "type", &mytype)) {
        metrics_proto.set_type(mytype);
    }
    int32_t framecount = -1;
    if (item->getInt32(MM_PREFIX "framecount", &framecount)) {
        metrics_proto.set_framecount(framecount);
    }
    int32_t samplerate = -1;
    if (item->getInt32(MM_PREFIX "samplerate", &samplerate)) {
        metrics_proto.set_samplerate(samplerate);
    }
    char *workhist = NULL;
    if (item->getCString(MM_PREFIX "workMs.hist", &workhist)) {
        metrics_proto.set_work_millis_hist(workhist);
    }
    char *latencyhist = NULL;
    if (item->getCString(MM_PREFIX "latencyMs.hist", &latencyhist)) {
        metrics_proto.set_latency_millis_hist(latencyhist);
    }
    char *warmuphist = NULL;
    if (item->getCString(MM_PREFIX "warmupMs.hist", &warmuphist)) {
        metrics_proto.set_warmup_millis_hist(warmuphist);
    }
    int64_t underruns = -1;
    if (item->getInt64(MM_PREFIX "underruns", &underruns)) {
        metrics_proto.set_underruns(underruns);
    }
    int64_t overruns = -1;
    if (item->getInt64(MM_PREFIX "overruns", &overruns)) {
        metrics_proto.set_overruns(overruns);
    }
    int64_t activeMs = -1;
    if (item->getInt64(MM_PREFIX "activeMs", &activeMs)) {
        metrics_proto.set_active_millis(activeMs);
    }
    int64_t durationMs = -1;
    if (item->getInt64(MM_PREFIX "durationMs", &durationMs)) {
        metrics_proto.set_duration_millis(durationMs);
    }

    // item->setInt32(MM_PREFIX "id", (int32_t)mId); // IO handle
    int32_t id = -1;
    if (item->getInt32(MM_PREFIX "id", &id)) {
        metrics_proto.set_id(id);
    }
    // item->setInt32(MM_PREFIX "portId", (int32_t)mPortId);
    int32_t port_id = -1;
    if (item->getInt32(MM_PREFIX "portId", &id)) {
        metrics_proto.set_port_id(port_id);
    }
    // item->setCString(MM_PREFIX "type", threadTypeToString(mType));
    char *type = NULL;
    if (item->getCString(MM_PREFIX "type", &type)) {
        metrics_proto.set_type(type);
    }
    // item->setInt32(MM_PREFIX "sampleRate", (int32_t)mSampleRate);
    int32_t sample_rate = -1;
    if (item->getInt32(MM_PREFIX "sampleRate", &sample_rate)) {
        metrics_proto.set_sample_rate(sample_rate);
    }
    // item->setInt64(MM_PREFIX "channelMask", (int64_t)mChannelMask);
    int32_t channel_mask = -1;
    if (item->getInt32(MM_PREFIX "channelMask", &channel_mask)) {
        metrics_proto.set_channel_mask(channel_mask);
    }
    // item->setCString(MM_PREFIX "encoding", toString(mFormat).c_str());
    char *encoding = NULL;
    if (item->getCString(MM_PREFIX "encoding", &encoding)) {
        metrics_proto.set_encoding(encoding);
    }
    // item->setInt32(MM_PREFIX "frameCount", (int32_t)mFrameCount);
    int32_t frame_count = -1;
    if (item->getInt32(MM_PREFIX "frameCount", &frame_count)) {
        metrics_proto.set_frame_count(frame_count);
    }
    // item->setCString(MM_PREFIX "outDevice", toString(mOutDevice).c_str());
    char *outDevice = NULL;
    if (item->getCString(MM_PREFIX "outDevice", &outDevice)) {
        metrics_proto.set_output_device(outDevice);
    }
    // item->setCString(MM_PREFIX "inDevice", toString(mInDevice).c_str());
    char *inDevice = NULL;
    if (item->getCString(MM_PREFIX "inDevice", &inDevice)) {
        metrics_proto.set_input_device(inDevice);
    }
    // item->setDouble(MM_PREFIX "ioJitterMs.mean", mIoJitterMs.getMean());
    double iojitters_ms_mean = -1;
    if (item->getDouble(MM_PREFIX "ioJitterMs.mean", &iojitters_ms_mean)) {
        metrics_proto.set_io_jitter_mean_millis(iojitters_ms_mean);
    }
    // item->setDouble(MM_PREFIX "ioJitterMs.std", mIoJitterMs.getStdDev());
    double iojitters_ms_std = -1;
    if (item->getDouble(MM_PREFIX "ioJitterMs.std", &iojitters_ms_std)) {
        metrics_proto.set_io_jitter_stddev_millis(iojitters_ms_std);
    }
    // item->setDouble(MM_PREFIX "processTimeMs.mean", mProcessTimeMs.getMean());
    double process_time_ms_mean = -1;
    if (item->getDouble(MM_PREFIX "processTimeMs.mean", &process_time_ms_mean)) {
        metrics_proto.set_process_time_mean_millis(process_time_ms_mean);
    }
    // item->setDouble(MM_PREFIX "processTimeMs.std", mProcessTimeMs.getStdDev());
    double process_time_ms_std = -1;
    if (item->getDouble(MM_PREFIX "processTimeMs.std", &process_time_ms_std)) {
        metrics_proto.set_process_time_stddev_millis(process_time_ms_std);
    }
    // item->setDouble(MM_PREFIX "timestampJitterMs.mean", tsjitter.getMean());
    double timestamp_jitter_ms_mean = -1;
    if (item->getDouble(MM_PREFIX "timestampJitterMs.mean", &timestamp_jitter_ms_mean)) {
        metrics_proto.set_timestamp_jitter_mean_millis(timestamp_jitter_ms_mean);
    }
    // item->setDouble(MM_PREFIX "timestampJitterMs.std", tsjitter.getStdDev());
    double timestamp_jitter_ms_stddev = -1;
    if (item->getDouble(MM_PREFIX "timestampJitterMs.std", &timestamp_jitter_ms_stddev)) {
        metrics_proto.set_timestamp_jitter_stddev_millis(timestamp_jitter_ms_stddev);
    }
    // item->setDouble(MM_PREFIX "latencyMs.mean", mLatencyMs.getMean());
    double latency_ms_mean = -1;
    if (item->getDouble(MM_PREFIX "latencyMs.mean", &latency_ms_mean)) {
        metrics_proto.set_latency_mean_millis(latency_ms_mean);
    }
    // item->setDouble(MM_PREFIX "latencyMs.std", mLatencyMs.getStdDev());
    double latency_ms_stddev = -1;
    if (item->getDouble(MM_PREFIX "latencyMs.std", &latency_ms_stddev)) {
        metrics_proto.set_latency_stddev_millis(latency_ms_stddev);
    }


    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize audiothread metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_AUDIOTHREAD_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(mytype);
    free(workhist);
    free(latencyhist);
    free(warmuphist);
    free(type);
    free(encoding);
    free(inDevice);
    free(outDevice);

    return true;
}

};
