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
#define LOG_TAG "statsd_audiopolicy"
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

bool statsd_audiopolicy(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    // these go into the statsd wrapper
    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;


    // the rest into our own proto
    //
    ::android::stats::mediametrics::AudioPolicyData metrics_proto;

    // flesh out the protobuf we'll hand off with our data
    //
    //int32 char kAudioPolicyStatus[] = "android.media.audiopolicy.status";
    int32_t status = -1;
    if (item->getInt32("android.media.audiopolicy.status", &status)) {
        metrics_proto.set_status(status);
    }
    //string char kAudioPolicyRqstSrc[] = "android.media.audiopolicy.rqst.src";
    char *rqst_src = NULL;
    if (item->getCString("android.media.audiopolicy.rqst.src", &rqst_src)) {
        metrics_proto.set_request_source(rqst_src);
    }
    //string char kAudioPolicyRqstPkg[] = "android.media.audiopolicy.rqst.pkg";
    char *rqst_pkg = NULL;
    if (item->getCString("android.media.audiopolicy.rqst.pkg", &rqst_pkg)) {
        metrics_proto.set_request_package(rqst_pkg);
    }
    //int32 char kAudioPolicyRqstSession[] = "android.media.audiopolicy.rqst.session";
    int32_t rqst_session = -1;
    if (item->getInt32("android.media.audiopolicy.rqst.session", &rqst_session)) {
        metrics_proto.set_request_session(rqst_session);
    }
    //string char kAudioPolicyRqstDevice[] = "android.media.audiopolicy.rqst.device";
    char *rqst_device = NULL;
    if (item->getCString("android.media.audiopolicy.rqst.device", &rqst_device)) {
        metrics_proto.set_request_device(rqst_device);
    }

    //string char kAudioPolicyActiveSrc[] = "android.media.audiopolicy.active.src";
    char *active_src = NULL;
    if (item->getCString("android.media.audiopolicy.active.src", &active_src)) {
        metrics_proto.set_active_source(active_src);
    }
    //string char kAudioPolicyActivePkg[] = "android.media.audiopolicy.active.pkg";
    char *active_pkg = NULL;
    if (item->getCString("android.media.audiopolicy.active.pkg", &active_pkg)) {
        metrics_proto.set_active_package(active_pkg);
    }
    //int32 char kAudioPolicyActiveSession[] = "android.media.audiopolicy.active.session";
    int32_t active_session = -1;
    if (item->getInt32("android.media.audiopolicy.active.session", &active_session)) {
        metrics_proto.set_active_session(active_session);
    }
    //string char kAudioPolicyActiveDevice[] = "android.media.audiopolicy.active.device";
    char *active_device = NULL;
    if (item->getCString("android.media.audiopolicy.active.device", &active_device)) {
        metrics_proto.set_active_device(active_device);
    }


    std::string serialized;
    if (!metrics_proto.SerializeToString(&serialized)) {
        ALOGE("Failed to serialize audipolicy metrics");
        return false;
    }

    if (enabled_statsd) {
        android::util::BytesField bf_serialized( serialized.c_str(), serialized.size());
        (void)android::util::stats_write(android::util::MEDIAMETRICS_AUDIOPOLICY_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);

    } else {
        ALOGV("NOT sending: private data (len=%zu)", strlen(serialized.c_str()));
    }

    // must free the strings that we were given
    free(rqst_src);
    free(rqst_pkg);
    free(rqst_device);
    free(active_src);
    free(active_pkg);
    free(active_device);

    return true;
}

};
