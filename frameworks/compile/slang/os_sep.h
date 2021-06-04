/*
 * Copyright 2011, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_SLANG_OS_SEP_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_OS_SEP_H_

#ifdef _WIN32
/* Define the default path separator for the platform. */
#define OS_PATH_SEPARATOR     '\\'
#define OS_PATH_SEPARATOR_STR "\\"

#else /* not _WIN32 */

/* Define the default path separator for the platform. */
#define OS_PATH_SEPARATOR     '/'
#define OS_PATH_SEPARATOR_STR "/"

#endif

#endif  // _FRAMEWORKS_COMPILE_SLANG_NO_SEP_H_  NOLINT
