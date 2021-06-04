/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef _J_MEDIAPLAYER2_UTILS2_H_
#define _J_MEDIAPLAYER2_UTILS2_H_

#include <media/stagefright/MetaData.h>

#include "jni.h"
#include "android_media_AudioFormat.h"

namespace android {

struct JMediaPlayer2Utils {
    static bool isOffloadedAudioPlaybackSupported(
            const sp<MetaData>& meta, bool hasVideo, bool isStreaming,
            audio_stream_type_t streamType);
};

}  // namespace android

#endif  // _J_MEDIAPLAYER2_UTILS2_H_
