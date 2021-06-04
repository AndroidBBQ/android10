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
#define LOG_TAG "iface_statsd"
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

// set of routines that crack a MediaAnalyticsItem
// and send it off to statsd with the appropriate hooks
//
// each MediaAnalyticsItem type (extractor, codec, nuplayer, etc)
// has its own routine to handle this.
//

bool enabled_statsd = true;

struct statsd_hooks {
    const char *key;
    bool (*handler)(MediaAnalyticsItem *);
};

// keep this sorted, so we can do binary searches
struct statsd_hooks  statsd_handlers[] =
{
    { "audiopolicy", statsd_audiopolicy },
    { "audiorecord", statsd_audiorecord },
    { "audiothread", statsd_audiothread },
    { "audiotrack", statsd_audiotrack },
    { "codec", statsd_codec},
    { "drm.vendor.Google.WidevineCDM", statsd_widevineCDM },
    { "extractor", statsd_extractor },
    { "mediadrm", statsd_mediadrm },
    { "nuplayer", statsd_nuplayer },
    { "nuplayer2", statsd_nuplayer },
    { "recorder", statsd_recorder },
};


// give me a record, i'll look at the type and upload appropriately
bool dump2Statsd(MediaAnalyticsItem *item) {
    if (item == NULL) return false;

    // get the key
    std::string key = item->getKey();

    if (!enabled_statsd) {
        ALOGV("statsd logging disabled for record key=%s", key.c_str());
        return false;
    }

    int i;
    for(i = 0;i < sizeof(statsd_handlers) / sizeof(statsd_handlers[0]) ; i++) {
        if (key == statsd_handlers[i].key) {
            return (*statsd_handlers[i].handler)(item);
        }
    }
    return false;
}

} // namespace android
