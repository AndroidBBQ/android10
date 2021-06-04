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

#ifndef C2UTILS_PARAM_UTILS_H_
#define C2UTILS_PARAM_UTILS_H_

#include <utility>
#include <vector>

/** \file
 * Utilities for parameter handling to be used by Codec2 implementations.
 */

/// \cond INTERNAL

struct C2Param;

class C2ParamUtils {
    friend class C2UtilTest_ParamUtilsTest_Test;

public:

    /// safe(r) parsing from parameter blob
    static
    C2Param *ParseFirst(const uint8_t *blob, size_t size);
};

/// \endcond

#endif  // C2UTILS_PARAM_UTILS_H_

