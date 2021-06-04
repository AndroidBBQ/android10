/*
 * Copyright 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BCC_SUPPORT_PROPERTIES_H
#define BCC_SUPPORT_PROPERTIES_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __ANDROID__
#include <sys/system_properties.h>
// TODO: Use rsCppUtils.cpp once the two projects are merged.
// Currently this implementation is copied over from
// frameworks/rs/rsCppUtils.cpp
static inline int property_get(const char *key, char *value, const char *default_value) {
    int len;
    len = __system_property_get(key, value);
    if (len > 0) {
        return len;
    }

    if (default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    }
    return len;
}
#endif

static inline uint32_t getProperty(const char *str) {
#ifdef __ANDROID__
    char buf[PROP_VALUE_MAX];
    property_get(str, buf, "0");
    return atoi(buf);
#else
    return 0;
#endif
}

#endif // BCC_SUPPORT_PROPERTIES_H
