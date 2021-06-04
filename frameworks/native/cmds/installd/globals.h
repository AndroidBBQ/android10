/*
**
** Copyright 2008, The Android Open Source Project
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

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <inttypes.h>
#include <string>
#include <vector>

namespace android {
namespace installd {

// Name of the environment variable that contains the asec mountpoint.
static constexpr const char* ASEC_MOUNTPOINT_ENV_NAME = "ASEC_MOUNTPOINT";

extern std::string android_app_dir;
extern std::string android_app_ephemeral_dir;
extern std::string android_app_lib_dir;
extern std::string android_app_private_dir;
extern std::string android_asec_dir;
extern std::string android_data_dir;
extern std::string android_media_dir;
extern std::string android_mnt_expand_dir;
extern std::string android_profiles_dir;
extern std::string android_root_dir;
extern std::string android_staging_dir;

extern std::vector<std::string> android_system_dirs;

bool init_globals_from_data_and_root();
bool init_globals_from_data_and_root(const char* data, const char* root);

}  // namespace installd
}  // namespace android

#endif  // GLOBALS_H_
