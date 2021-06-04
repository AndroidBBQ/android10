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

#ifndef ANDROID_JAUDIOATTRIBUTES_H
#define ANDROID_JAUDIOATTRIBUTES_H

#include <jni.h>
#include <system/audio.h>

namespace android {

class JAudioAttributes {
public:
    /* Creates a Java AudioAttributes object. */
    static jobject createAudioAttributesObj(JNIEnv *env,
                                            const audio_attributes_t* pAttributes) {

        jclass jBuilderCls = env->FindClass("android/media/AudioAttributes$Builder");
        jmethodID jBuilderCtor = env->GetMethodID(jBuilderCls, "<init>", "()V");
        jobject jBuilderObj = env->NewObject(jBuilderCls, jBuilderCtor);

        if (pAttributes != NULL) {
            // If pAttributes is not null, streamType is ignored.
            jmethodID jSetUsage = env->GetMethodID(
                    jBuilderCls, "setUsage", "(I)Landroid/media/AudioAttributes$Builder;");
            jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetUsage, pAttributes->usage);

            jmethodID jSetContentType = env->GetMethodID(jBuilderCls, "setContentType",
                    "(I)Landroid/media/AudioAttributes$Builder;");
            jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetContentType,
                    pAttributes->content_type);

            // TODO: Java AudioAttributes.Builder.setCapturePreset() is systemApi and hidden.
            // Can we use this method?
//            jmethodID jSetCapturePreset = env->GetMethodID(jBuilderCls, "setCapturePreset",
//                    "(I)Landroid/media/AudioAttributes$Builder;");
//            jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetCapturePreset,
//                    pAttributes->source);

            jmethodID jSetFlags = env->GetMethodID(jBuilderCls, "setFlags",
                    "(I)Landroid/media/AudioAttributes$Builder;");
            jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetFlags, pAttributes->flags);

            // TODO: Handle the 'tags' (char[] to HashSet<String>).
            // How to parse the char[]? Is there any example of it?
            // Also, the addTags() method is hidden.
        }

        jmethodID jBuild = env->GetMethodID(jBuilderCls, "build",
                "()Landroid/media/AudioAttributes;");
        return env->CallObjectMethod(jBuilderObj, jBuild);
    }

};

} // namespace android

#endif // ANDROID_JAUDIOATTRIBUTES_H
