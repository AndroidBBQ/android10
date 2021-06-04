// Copyright 2015, The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AV_SERVICES_MINIJAIL_MINIJAIL
#define AV_SERVICES_MINIJAIL_MINIJAIL

#include <string>

namespace android {
int WritePolicyToPipe(const std::string& base_policy_content,
                      const std::string& additional_policy_content);
void SetUpMinijail(const std::string& base_policy_path, const std::string& additional_policy_path);
}

#endif  // AV_SERVICES_MINIJAIL_MINIJAIL
