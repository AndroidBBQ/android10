/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef C2UTILS_DEBUG_BASE_H_
#define C2UTILS_DEBUG_BASE_H_

#include <iostream>

/**
 * Debug print utilities for Codec 2.0 base objects.
 */

#define C2_DECLARE_AS_STRING_AND_DEFINE_STREAM_OUT(type) \
const char *asString(type i, const char *def = "??"); \
inline std::ostream& operator<<(std::ostream& os, const type &i) { \
    return os << asString(i); \
}

C2_DECLARE_AS_STRING_AND_DEFINE_STREAM_OUT(c2_status_t)


#endif  // C2UTILS_DEBUG_BASE_H_

