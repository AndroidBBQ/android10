/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <media/stagefright/foundation/MediaKeys.h>

namespace android {

const char *const kIStreamListenerKeyDiscontinuityMask = "discontinuity-mask";
const char *const kATSParserKeyResumeAtPTS = "resume-at-PTS";
const char *const kATSParserKeyMediaTimeUs = "media-time-us";
const char *const kATSParserKeyRecentMediaTimeUs = "recent-media-time-us";

}  // namespace android
