/*
 * Copyright 2019 The Android Open Source Project
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

#include <cstdlib>
#include <string>

#include "Log.h"

#include <sys/system_properties.h>

#define LOG_TAG "SysProp"

std::string getSystemPropViaGet(const char* key, std::string default_value = "") {
    char buffer[PROP_VALUE_MAX + 1];
    int bufferLen = __system_property_get(key, buffer);
    if (bufferLen > PROP_VALUE_MAX || bufferLen == 0) {
        return default_value;
    }
    return std::string(buffer, bufferLen);
}

int getSystemPropViaGetAsInt(const char* key, int default_value = 0) {
    char buffer[PROP_VALUE_MAX + 1];
    int bufferLen = __system_property_get(key, buffer);
    if (bufferLen > PROP_VALUE_MAX || bufferLen == 0) {
        return default_value;
    }
    return ::atoi(buffer);
}

int getSystemPropViaGetAsBool(const char* key, bool default_value = false) {
    return getSystemPropViaGetAsInt(key, default_value) != 0;
}
