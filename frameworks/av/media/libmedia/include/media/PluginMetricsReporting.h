/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef PLUGIN_METRICS_REPORTING_H_

#define PLUGIN_METRICS_REPORTING_H_

#include <utils/Errors.h>
#include <utils/String8.h>

namespace android {

status_t reportDrmPluginMetrics(const std::string& b64EncodedMetrics,
                                const String8& vendorName,
                                const String8& description,
                                const String8& appPackageName);

}  // namespace android

#endif  // PLUGIN_METRICS_REPORTING_H_
