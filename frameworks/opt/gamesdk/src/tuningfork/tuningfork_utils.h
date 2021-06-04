/*
 * Copyright 2019 The Android Open Source Project
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

#include <string>
#include <jni.h>

class AAsset;

namespace tuningfork {

namespace apk_utils {

    // Get an asset from this APK's asset directory.
    // Returns NULL if the asset could not be found.
    // Asset_close must be called once the asset is no longer needed.
    AAsset* GetAsset(JNIEnv* env, jobject activity, const char* name);

    // Get the app's version code. Also fills packageNameStr, if not null, with
    // the package name.
    int GetVersionCode(JNIEnv *env, jobject context, std::string* packageNameStr = nullptr);

} // namespace apk_utils

namespace file_utils {

    // Creates the directory if it does not exist. Returns true if the directory
    //  already existed or could be created.
    bool CheckAndCreateDir(const std::string& path);

    bool FileExists(const std::string& fname);

    // Call NativeActivity.getCacheDir via JNI
    std::string GetAppCacheDir(JNIEnv* env, jobject activity);

} // namespace file_utils

// Get a unique identifier using java.util.UUID
std::string UniqueId(JNIEnv* env);

} // namespace tuningfork
