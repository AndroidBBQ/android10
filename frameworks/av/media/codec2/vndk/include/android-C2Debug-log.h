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

#ifndef C2UTILS_ANDROID_DEBUG_LOG_H_
#define C2UTILS_ANDROID_DEBUG_LOG_H_

/*
 * Android provides logging and debug macros. Redefine them with C2 prefix and add support for
 * opting out of verbose logs.
 */

#ifdef C2_LOG_TAG
#define LOG_TAG C2_LOG_TAG
#endif

#include <android-base/logging.h>

#ifdef C2_LOG_VERBOSE
#define C2_LOG(LEVEL) LOG(::android::base::LEVEL)
#else
/**
 * Use as follows:
 *
 * #define C2_LOG_TAG "tag"
 * //#define C2_LOG_VERBOSE  // enable verbose logs in this file
 * #include <C2Debug.h>
 *
 * C2_LOG(DEBUG) << expr ...;
 *
 * Log levels are: VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL (aborts).
 *
 * No endl is required.
 */
#define C2_LOG(LEVEL) \
        (::android::base::LEVEL != ::android::base::VERBOSE) && LOG(::android::base::LEVEL)
#endif

#define C2_CHECK    CHECK
#define C2_CHECK_LT CHECK_LT
#define C2_CHECK_LE CHECK_LE
#define C2_CHECK_EQ CHECK_EQ
#define C2_CHECK_GE CHECK_GE
#define C2_CHECK_GT CHECK_GT
#define C2_CHECK_NE CHECK_NE

#define C2_DCHECK    DCHECK
#define C2_DCHECK_LT DCHECK_LT
#define C2_DCHECK_LE DCHECK_LE
#define C2_DCHECK_EQ DCHECK_EQ
#define C2_DCHECK_GE DCHECK_GE
#define C2_DCHECK_GT DCHECK_GT
#define C2_DCHECK_NE DCHECK_NE

#endif  // C2UTILS_ANDROID_DEBUG_LOG_H_
