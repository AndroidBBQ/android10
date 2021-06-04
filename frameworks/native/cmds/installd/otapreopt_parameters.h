/*
 ** Copyright 2018, The Android Open Source Project
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

#ifndef OTAPREOPT_PARAMETERS_H_
#define OTAPREOPT_PARAMETERS_H_

#include <string>
#include <sys/types.h>

namespace android {
namespace installd {

class OTAPreoptParameters {
  public:
    bool ReadArguments(int argc, const char** argv);

  private:
    bool ReadArgumentsV1(const char** argv);
    bool ReadArgumentsPostV1(uint32_t version, const char** argv, bool versioned);

    void SetDefaultsForPostV1Arguments();
    const char* apk_path;
    uid_t uid;
    const char* pkgName;
    const char* instruction_set;
    int dexopt_needed;
    const char* oat_dir;
    int dexopt_flags;
    const char* compiler_filter;
    const char* volume_uuid;
    const char* shared_libraries;
    const char* se_info;
    bool downgrade;
    int target_sdk_version;
    const char* profile_name;
    const char* dex_metadata_path;
    const char* compilation_reason;

    std::string target_slot;

    friend class OTAPreoptService;
    friend class OTAPreoptTest;
};

}  // namespace installd
}  // namespace android

#endif  //  OTAPREOPT_PARAMETERS_H_
