/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef ANDROID_JAUDIOFORMAT_H
#define ANDROID_JAUDIOFORMAT_H

#include <android_media_AudioFormat.h>
#include <jni.h>

namespace android {

class JAudioFormat {
public:
    /* Creates a Java AudioFormat object. */
    static jobject createAudioFormatObj(JNIEnv *env,
                                        uint32_t sampleRate,
                                        audio_format_t format,
                                        audio_channel_mask_t channelMask) {

        jclass jBuilderCls = env->FindClass("android/media/AudioFormat$Builder");
        jmethodID jBuilderCtor = env->GetMethodID(jBuilderCls, "<init>", "()V");
        jobject jBuilderObj = env->NewObject(jBuilderCls, jBuilderCtor);

        if (sampleRate == 0) {
            jclass jAudioFormatCls = env->FindClass("android/media/AudioFormat");
            jfieldID jSampleRateUnspecified =
                    env->GetStaticFieldID(jAudioFormatCls, "SAMPLE_RATE_UNSPECIFIED", "I");
            sampleRate = env->GetStaticIntField(jAudioFormatCls, jSampleRateUnspecified);
        }

        jmethodID jSetEncoding = env->GetMethodID(jBuilderCls, "setEncoding",
                "(I)Landroid/media/AudioFormat$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetEncoding,
                audioFormatFromNative(format));

        jmethodID jSetSampleRate = env->GetMethodID(jBuilderCls, "setSampleRate",
                "(I)Landroid/media/AudioFormat$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetSampleRate, sampleRate);

        jmethodID jSetChannelMask = env->GetMethodID(jBuilderCls, "setChannelMask",
                "(I)Landroid/media/AudioFormat$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetChannelMask,
                outChannelMaskFromNative(channelMask));

        jmethodID jBuild = env->GetMethodID(jBuilderCls, "build", "()Landroid/media/AudioFormat;");
        return env->CallObjectMethod(jBuilderObj, jBuild);
    }

};

} // namespace android

#endif // ANDROID_JAUDIOFORMAT_H
