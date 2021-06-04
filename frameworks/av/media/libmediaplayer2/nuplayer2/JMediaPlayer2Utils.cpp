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

//#define LOG_NDEBUG 0
#define LOG_TAG "JMediaPlayer2Utils"

#include "JMediaPlayer2Utils.h"
#include <mediaplayer2/JavaVMHelper.h>

#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/Utils.h>
#include <utils/Log.h>

#include "log/log.h"

namespace android {

static const int64_t kOffloadMinDurationSec = 60;

// static
bool JMediaPlayer2Utils::isOffloadedAudioPlaybackSupported(
        const sp<MetaData>& meta, bool hasVideo, bool isStreaming, audio_stream_type_t streamType)
{
    if (hasVideo || streamType != AUDIO_STREAM_MUSIC) {
        return false;
    }

    audio_offload_info_t info = AUDIO_INFO_INITIALIZER;
    if (OK != getAudioOffloadInfo(meta, hasVideo, isStreaming, streamType, &info)) {
        return false;
    }

    if (info.duration_us < kOffloadMinDurationSec * 1000000) {
        return false;
    }

    int32_t audioFormat = audioFormatFromNative(info.format);
    int32_t channelMask = outChannelMaskFromNative(info.channel_mask);
    if (audioFormat == ENCODING_INVALID || channelMask == CHANNEL_INVALID) {
        return false;
    }

    JNIEnv* env = JavaVMHelper::getJNIEnv();
    jclass jMP2UtilsCls = env->FindClass("android/media/MediaPlayer2Utils");
    jmethodID jSetAudioOutputDeviceById = env->GetStaticMethodID(
            jMP2UtilsCls, "isOffloadedAudioPlaybackSupported", "(III)Z");
    jboolean result = env->CallStaticBooleanMethod(
            jMP2UtilsCls, jSetAudioOutputDeviceById, audioFormat, info.sample_rate, channelMask);
    return result;
}

}  // namespace android
