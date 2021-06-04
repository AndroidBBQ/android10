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

#pragma once

#if defined(__ANDROID__)

#include "egldefs.h"

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "egl_trace.h"

namespace angle {

bool initializeAnglePlatform(EGLDisplay dpy);
void resetAnglePlatform(EGLDisplay dpy);

}; // namespace angle

#endif // __ANDROID__
