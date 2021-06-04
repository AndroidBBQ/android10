/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_INSTALLD_QUOTA_UTILS_H_
#define ANDROID_INSTALLD_QUOTA_UTILS_H_

#include <memory>
#include <string>

namespace android {
namespace installd {

/* Clear and recompute the reverse mounts map */
bool InvalidateQuotaMounts();

/* Whether quota is supported in the device with the given uuid */
bool IsQuotaSupported(const std::string& uuid);

/* Get the current occupied space in bytes for a uid or -1 if fails */
int64_t GetOccupiedSpaceForUid(const std::string& uuid, uid_t uid);

/* Get the current occupied space in bytes for a gid or -1 if fails */
int64_t GetOccupiedSpaceForGid(const std::string& uuid, gid_t gid);

}  // namespace installd
}  // namespace android

#endif  // ANDROID_INSTALLD_QUOTA_UTILS_H_
