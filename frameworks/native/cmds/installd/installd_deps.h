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

#ifndef INSTALLD_DEPS_H_
#define INSTALLD_DEPS_H_

#include <inttypes.h>

#include <installd_constants.h>

namespace android {
namespace installd {

// Dependencies for a full binary. These functions need to be provided to
// figure out parts of the configuration.

// Retrieve a system property. Same API as cutils, just renamed.
extern int get_property(const char *key,
                        char *value,
                        const char *default_value);
// Size constants. Should be checked to be equal to the cutils requirements.
constexpr size_t kPropertyKeyMax = 32u;
constexpr size_t kPropertyValueMax = 92u;

// Compute the output path for dex2oat.
extern bool calculate_oat_file_path(char path[PKG_PATH_MAX],
                                    const char *oat_dir,
                                    const char *apk_path,
                                    const char *instruction_set);
// Compute the output path for patchoat.
//
// Computes the odex file for the given apk_path and instruction_set, e.g.,
// /system/framework/whatever.jar -> /system/framework/oat/<isa>/whatever.odex
//
// Returns false if it failed to determine the odex file path.
//
extern bool calculate_odex_file_path(char path[PKG_PATH_MAX],
                                     const char *apk_path,
                                     const char *instruction_set);

// Compute the output path into the dalvik cache.
extern bool create_cache_path(char path[PKG_PATH_MAX],
                              const char *src,
                              const char *instruction_set);

}  // namespace installd
}  // namespace android

#endif  // INSTALLD_DEPS_H_
