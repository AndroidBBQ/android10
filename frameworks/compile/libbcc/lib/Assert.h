/*
 * Copyright 2011-2012, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_LIBBCC_INCLUDE_BCC_ASSERT_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_LIBBCC_INCLUDE_BCC_ASSERT_H_

#ifdef __cplusplus
#include <cstdlib>
#include <cstdio>
#else
#include <stdlib.h>
#include <stdio.h>
#endif  // __cplusplus

#ifdef __DISABLE_ASSERTS

#define bccAssert(v) do {} while (0)

#else

#ifndef LOG_TAG
#   define LOG_TAG "bcc"
#   include <log/log.h>
#endif

#define __ABORT_ON_FAILURES 1

#define bccAssert(v)                                \
  do {                                              \
    if (!(v)) {                                     \
      ALOGE("bccAssert failed at %s:%d - '%s'\n",   \
          __FILE__, __LINE__, #v);                  \
      if (__ABORT_ON_FAILURES) {                    \
        abort();                                    \
      }                                             \
    }                                               \
  } while (0)

#endif  // __DISABLE_ASSERTS

#endif  // _FRAMEWORKS_COMPILE_LIBBCC_INCLUDE_BCC_ASSERT_H_  NOLINT
