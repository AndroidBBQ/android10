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

// Put build-time configuration options here rather than Android.mk,
// so that all build modules will pick up the same options.

#ifndef ANDROID_WILHELM_CONFIGURATION_H
#define ANDROID_WILHELM_CONFIGURATION_H

// uncomment to enable mutex deadlock detection,
// or comment to disable mutex deadlock detection
#define USE_DEBUG

#endif // ANDROID_WILHELM_CONFIGURATION_H
