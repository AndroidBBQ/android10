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

#include <android_runtime/AndroidRuntime.h>
#include <log/log.h>

#include <string>

// Use reflection to query the default cache dir.
// The function is NOT thread-safe. It is expected to be called
// at most once per process.
extern "C" const char* rsQueryCacheDir() {
    static std::string cacheDir;
    // First check if we have JavaVM running in this process.
    if (android::AndroidRuntime::getJavaVM()) {
        JNIEnv* env = android::AndroidRuntime::getJNIEnv();
        if (env) {
            jclass cacheDirClass = env->FindClass("android/renderscript/RenderScriptCacheDir");
            jfieldID cacheDirID = env->GetStaticFieldID(cacheDirClass, "mCacheDir", "Ljava/io/File;");
            jobject cache_dir = env->GetStaticObjectField(cacheDirClass, cacheDirID);

            if (cache_dir) {
                jclass fileClass = env->FindClass("java/io/File");
                jmethodID getPath = env->GetMethodID(fileClass, "getPath", "()Ljava/lang/String;");
                jstring path_string = (jstring)env->CallObjectMethod(cache_dir, getPath);
                const char *path_chars = env->GetStringUTFChars(path_string, NULL);

                ALOGD("Successfully queried cache dir: %s", path_chars);
                cacheDir = std::string(path_chars);
                env->ReleaseStringUTFChars(path_string, path_chars);
            } else {
                ALOGD("Cache dir not initialized");
            }
        } else {
            ALOGD("Failed to query the default cache dir.");
        }
    }

    return cacheDir.c_str();
}

