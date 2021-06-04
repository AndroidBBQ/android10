/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "sles_allinclusive.h"

#ifdef ANDROID
/** \brief Log messages are prefixed by this tag */
const char slLogTag[] = "libOpenSLES";
#endif

#if 0
// There is no support for configuring the logging level at runtime.
// If that was needed, it could be done like this:
// #define SL_LOGx(...)  do { if (slLogLevel <= ...) ... } while (0)

/** \brief Default runtime log level */

SLAndroidLogLevel slLogLevel = USE_LOG_RUNTIME;

/** \brief Set the runtime log level */

SL_API void SLAPIENTRY slAndroidSetLogLevel(SLAndroidLogLevel logLevel)
{
    // Errors can't be disabled
    if (logLevel > SLAndroidLogLevel_Error)
        logLevel = SLAndroidLogLevel_Error;
    slLogLevel = logLevel;
}
#endif
