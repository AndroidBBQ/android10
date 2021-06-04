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
#define LOG_TAG "statsd_drm"
#include <utils/Log.h>

#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
#include <pwd.h>

#include "MediaAnalyticsService.h"
#include "iface_statsd.h"

#include <statslog.h>

namespace android {

// mediadrm
bool statsd_mediadrm(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;

    char *vendor = NULL;
    (void) item->getCString("vendor", &vendor);
    char *description = NULL;
    (void) item->getCString("description", &description);
    char *serialized_metrics = NULL;
    (void) item->getCString("serialized_metrics", &serialized_metrics);

    if (enabled_statsd) {
        android::util::BytesField bf_serialized(serialized_metrics ? serialized_metrics : NULL,
                                                serialized_metrics ? strlen(serialized_metrics)
                                                                   : 0);
        android::util::stats_write(android::util::MEDIAMETRICS_MEDIADRM_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   vendor, description,
                                   bf_serialized);
    } else {
        ALOGV("NOT sending: mediadrm private data (len=%zu)",
              serialized_metrics ? strlen(serialized_metrics) : 0);
    }

    free(vendor);
    free(description);
    free(serialized_metrics);
    return true;
}

// widevineCDM
bool statsd_widevineCDM(MediaAnalyticsItem *item)
{
    if (item == NULL) return false;

    nsecs_t timestamp = item->getTimestamp();
    std::string pkgName = item->getPkgName();
    int64_t pkgVersionCode = item->getPkgVersionCode();
    int64_t mediaApexVersion = 0;

    char *serialized_metrics = NULL;
    (void) item->getCString("serialized_metrics", &serialized_metrics);

    if (enabled_statsd) {
        android::util::BytesField bf_serialized(serialized_metrics ? serialized_metrics : NULL,
                                                serialized_metrics ? strlen(serialized_metrics)
                                                                   : 0);
        android::util::stats_write(android::util::MEDIAMETRICS_DRM_WIDEVINE_REPORTED,
                                   timestamp, pkgName.c_str(), pkgVersionCode,
                                   mediaApexVersion,
                                   bf_serialized);
    } else {
        ALOGV("NOT sending: widevine private data (len=%zu)",
              serialized_metrics ? strlen(serialized_metrics) : 0);
    }

    free(serialized_metrics);
    return true;
}

} // namespace android
