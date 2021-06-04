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

namespace android {

extern bool enabled_statsd;

// component specific dumpers
extern bool statsd_audiopolicy(MediaAnalyticsItem *);
extern bool statsd_audiorecord(MediaAnalyticsItem *);
extern bool statsd_audiothread(MediaAnalyticsItem *);
extern bool statsd_audiotrack(MediaAnalyticsItem *);
extern bool statsd_codec(MediaAnalyticsItem *);
extern bool statsd_extractor(MediaAnalyticsItem *);
extern bool statsd_nuplayer(MediaAnalyticsItem *);
extern bool statsd_recorder(MediaAnalyticsItem *);

extern bool statsd_mediadrm(MediaAnalyticsItem *);
extern bool statsd_widevineCDM(MediaAnalyticsItem *);

} // namespace android
