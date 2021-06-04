/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_SENSOR_SERVICE_UTIL
#define ANDROID_SENSOR_SERVICE_UTIL

#include <cstddef>
#include <string>

namespace android {
namespace SensorServiceUtil {

class Dumpable {
public:
    virtual std::string dump() const = 0;
    virtual void setFormat(std::string ) {}
    virtual ~Dumpable() {}
};

size_t eventSizeBySensorType(int type);

} // namespace SensorServiceUtil
} // namespace android;

#endif // ANDROID_SENSOR_SERVICE_UTIL
