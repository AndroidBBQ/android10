/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef LE_FX_ENGINE_COMMON_CORE_OS_H_
#define LE_FX_ENGINE_COMMON_CORE_OS_H_

// -----------------------------------------------------------------------------
// OS Identification:
// -----------------------------------------------------------------------------

#define LE_FX_OS_UNIX
#if defined(__ANDROID__)
#    define LE_FX_OS_ANDROID
#endif  // Android

#endif // LE_FX_ENGINE_COMMON_CORE_OS_H_
