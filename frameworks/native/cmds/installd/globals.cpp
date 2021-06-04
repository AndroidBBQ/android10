/*
** Copyright 2015, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "installd"

#include <globals.h>
#include <installd_constants.h>
#include <utils.h>

#include <android-base/logging.h>

#include <stdlib.h>
#include <string.h>

namespace android {
namespace installd {

static constexpr const char* APP_SUBDIR = "app/"; // sub-directory under ANDROID_DATA

static constexpr const char* PRIV_APP_SUBDIR = "priv-app/"; // sub-directory under ANDROID_DATA

static constexpr const char* EPHEMERAL_APP_SUBDIR = "app-ephemeral/"; // sub-directory under
                                                                      // ANDROID_DATA

static constexpr const char* APP_LIB_SUBDIR = "app-lib/"; // sub-directory under ANDROID_DATA

static constexpr const char* MEDIA_SUBDIR = "media/"; // sub-directory under ANDROID_DATA

static constexpr const char* PROFILES_SUBDIR = "misc/profiles"; // sub-directory under ANDROID_DATA

static constexpr const char* PRIVATE_APP_SUBDIR = "app-private/"; // sub-directory under
                                                                  // ANDROID_DATA

static constexpr const char* STAGING_SUBDIR = "app-staging/"; // sub-directory under ANDROID_DATA

std::string android_app_dir;
std::string android_app_ephemeral_dir;
std::string android_app_lib_dir;
std::string android_app_private_dir;
std::string android_asec_dir;
std::string android_data_dir;
std::string android_media_dir;
std::string android_mnt_expand_dir;
std::string android_profiles_dir;
std::string android_root_dir;
std::string android_staging_dir;

std::vector<std::string> android_system_dirs;

bool init_globals_from_data_and_root() {
    const char* data_path = getenv("ANDROID_DATA");
    if (data_path == nullptr) {
        LOG(ERROR) << "Could not find ANDROID_DATA";
        return false;
    }
    const char* root_path = getenv("ANDROID_ROOT");
    if (root_path == nullptr) {
        LOG(ERROR) << "Could not find ANDROID_ROOT";
        return false;
    }
    return init_globals_from_data_and_root(data_path, root_path);
}

static std::string ensure_trailing_slash(const std::string& path) {
    if (path.rfind('/') != path.size() - 1) {
        return path + '/';
    } else {
        return path;
    }
}

bool init_globals_from_data_and_root(const char* data, const char* root) {
    // Get the android data directory.
    android_data_dir = ensure_trailing_slash(data);

    // Get the android root directory.
    android_root_dir = ensure_trailing_slash(root);

    // Get the android app directory.
    android_app_dir = android_data_dir + APP_SUBDIR;

    // Get the android protected app directory.
    android_app_private_dir = android_data_dir + PRIVATE_APP_SUBDIR;

    // Get the android ephemeral app directory.
    android_app_ephemeral_dir = android_data_dir + EPHEMERAL_APP_SUBDIR;

    // Get the android app native library directory.
    android_app_lib_dir = android_data_dir + APP_LIB_SUBDIR;

    // Get the sd-card ASEC mount point.
    android_asec_dir = ensure_trailing_slash(getenv(ASEC_MOUNTPOINT_ENV_NAME));

    // Get the android media directory.
    android_media_dir = android_data_dir + MEDIA_SUBDIR;

    // Get the android external app directory.
    android_mnt_expand_dir = "/mnt/expand/";

    // Get the android profiles directory.
    android_profiles_dir = android_data_dir + PROFILES_SUBDIR;

    // Get the android session staging directory.
    android_staging_dir = android_data_dir + STAGING_SUBDIR;

    // Take note of the system and vendor directories.
    android_system_dirs.clear();
    android_system_dirs.push_back(android_root_dir + APP_SUBDIR);
    android_system_dirs.push_back(android_root_dir + PRIV_APP_SUBDIR);
    android_system_dirs.push_back("/vendor/app/");
    android_system_dirs.push_back("/oem/app/");

    return true;
}

}  // namespace installd
}  // namespace android
