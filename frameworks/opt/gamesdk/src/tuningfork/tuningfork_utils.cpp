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

#include "tuningfork_utils.h"

#include <sys/stat.h>
#include <errno.h>

#define LOG_TAG "TuningFork"
#include "Log.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace tuningfork {

namespace apk_utils {

    // Get an asset from this APK's asset directory.
    // Returns NULL if the asset could not be found.
    // Asset_close must be called once the asset is no longer needed.
    AAsset* GetAsset(JNIEnv* env, jobject activity, const char* name) {
        jclass cls = env->FindClass("android/content/Context");
        jmethodID get_assets = env->GetMethodID(cls, "getAssets",
                                                "()Landroid/content/res/AssetManager;");
        if(get_assets==nullptr) {
            ALOGE("No Context.getAssets() method");
            return nullptr;
        }
        auto javaMgr = env->CallObjectMethod(activity, get_assets);
        if (javaMgr == nullptr) {
            ALOGE("No java asset manager");
            return nullptr;
        }
        AAssetManager* mgr = AAssetManager_fromJava(env, javaMgr);
        if (mgr == nullptr) {
            ALOGE("No asset manager");
            return nullptr;
        }
        AAsset* asset = AAssetManager_open(mgr, name,
                                           AASSET_MODE_BUFFER);
        if (asset == nullptr) {
            ALOGW("Can't find %s in APK", name);
            return nullptr;
        }
        return asset;
    }


    // Get the app's version code. Also fills packageNameStr with the package name
    //  if it is non-null.
    int GetVersionCode(JNIEnv *env, jobject context, std::string* packageNameStr) {
        jstring packageName;
        jobject packageManagerObj;
        jobject packageInfoObj;
        jclass contextClass =  env->GetObjectClass( context);
        jmethodID getPackageNameMid = env->GetMethodID( contextClass, "getPackageName",
            "()Ljava/lang/String;");
        jmethodID getPackageManager =  env->GetMethodID( contextClass, "getPackageManager",
            "()Landroid/content/pm/PackageManager;");

        jclass packageManagerClass = env->FindClass("android/content/pm/PackageManager");
        jmethodID getPackageInfo = env->GetMethodID( packageManagerClass, "getPackageInfo",
            "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

        jclass packageInfoClass = env->FindClass("android/content/pm/PackageInfo");
        jfieldID versionCodeFid = env->GetFieldID( packageInfoClass, "versionCode", "I");

        packageName =  (jstring)env->CallObjectMethod(context, getPackageNameMid);

        if (packageNameStr != nullptr) {
            // Fill packageNameStr with the package name
            const char* packageName_cstr = env->GetStringUTFChars(packageName, NULL);
            *packageNameStr = std::string(packageName_cstr);
            env->ReleaseStringUTFChars(packageName, packageName_cstr);
        }
        // Get version code from package info
        packageManagerObj = env->CallObjectMethod(context, getPackageManager);
        packageInfoObj = env->CallObjectMethod(packageManagerObj,getPackageInfo,
                                               packageName, 0x0);
        int versionCode = env->GetIntField( packageInfoObj, versionCodeFid);
        return versionCode;
    }

} // namespace apk_utils

namespace file_utils {

    // Creates the directory if it does not exist. Returns true if the directory
    //  already existed or could be created.
    bool CheckAndCreateDir(const std::string& path) {
        struct stat sb;
        int32_t res = stat(path.c_str(), &sb);
        if (0 == res && sb.st_mode & S_IFDIR) {
            ALOGV("Directory %s already exists", path.c_str());
            return true;
        } else if (ENOENT == errno) {
            ALOGI("Creating directory %s", path.c_str());
            res = mkdir(path.c_str(), 0770);
            if(res!=0)
                ALOGW("Error creating directory %s: %d", path.c_str(), res);
            return res==0;
        }
        return false;
    }
    bool FileExists(const std::string& fname) {
        struct stat buffer;
        return (stat(fname.c_str(), &buffer)==0);
    }
    std::string GetAppCacheDir(JNIEnv* env, jobject activity) {
        jclass activityClass = env->FindClass( "android/app/NativeActivity" );
        jmethodID getCacheDir = env->GetMethodID( activityClass, "getCacheDir",
            "()Ljava/io/File;" );
        jobject cache_dir = env->CallObjectMethod( activity, getCacheDir );

        jclass fileClass = env->FindClass( "java/io/File" );
        jmethodID getPath = env->GetMethodID( fileClass, "getPath", "()Ljava/lang/String;" );
        jstring path_string = (jstring)env->CallObjectMethod( cache_dir, getPath );

        const char *path_chars = env->GetStringUTFChars( path_string, NULL );
        std::string temp_folder( path_chars );
        env->ReleaseStringUTFChars( path_string, path_chars );

        return temp_folder;
    }

} // namespace file_utils

std::string UniqueId(JNIEnv* env) {
    jclass uuid_class = env->FindClass("java/util/UUID");
    jmethodID randomUUID = env->GetStaticMethodID( uuid_class, "randomUUID",
            "()Ljava/util/UUID;");
    jobject uuid = env->CallStaticObjectMethod(uuid_class, randomUUID);
    jmethodID toString = env->GetMethodID( uuid_class, "toString", "()Ljava/lang/String;");
    jstring uuid_string = (jstring)env->CallObjectMethod(uuid, toString);
    const char *uuid_chars = env->GetStringUTFChars( uuid_string, NULL );
    std::string temp_uuid( uuid_chars );
    env->ReleaseStringUTFChars( uuid_string, uuid_chars );
    return temp_uuid;
}

} // namespace tuningfork
