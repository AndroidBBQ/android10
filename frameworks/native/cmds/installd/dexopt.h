/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef DEXOPT_H_
#define DEXOPT_H_

#include "installd_constants.h"

#include <sys/types.h>

#include <cutils/multiuser.h>

namespace android {
namespace installd {

/* dexopt needed flags matching those in dalvik.system.DexFile */
static constexpr int NO_DEXOPT_NEEDED            = 0;
static constexpr int DEX2OAT_FROM_SCRATCH        = 1;
static constexpr int DEX2OAT_FOR_BOOT_IMAGE      = 2;
static constexpr int DEX2OAT_FOR_FILTER          = 3;

#define ANDROID_RUNTIME_APEX_BIN "/apex/com.android.runtime/bin"
// Location of binaries in the Android Runtime APEX.
static constexpr const char* kDex2oatPath = ANDROID_RUNTIME_APEX_BIN "/dex2oat";
static constexpr const char* kDex2oatDebugPath = ANDROID_RUNTIME_APEX_BIN "/dex2oatd";
static constexpr const char* kProfmanPath = ANDROID_RUNTIME_APEX_BIN "/profman";
static constexpr const char* kProfmanDebugPath = ANDROID_RUNTIME_APEX_BIN "/profmand";
static constexpr const char* kDexoptanalyzerPath = ANDROID_RUNTIME_APEX_BIN "/dexoptanalyzer";
static constexpr const char* kDexoptanalyzerDebugPath = ANDROID_RUNTIME_APEX_BIN "/dexoptanalyzerd";
#undef ANDROID_RUNTIME_APEX_BIN

// Clear the reference profile identified by the given profile name.
bool clear_primary_reference_profile(const std::string& pkgname, const std::string& profile_name);
// Clear the current profile identified by the given profile name (for single user).
bool clear_primary_current_profile(const std::string& pkgname, const std::string& profile_name,
         userid_t user);
// Clear all current profiles identified by the given profile name (all users).
bool clear_primary_current_profiles(const std::string& pkgname, const std::string& profile_name);

// Decide if profile guided compilation is needed or not based on existing profiles.
// The analysis is done for a single profile name (which corresponds to a single code path).
// Returns true if there is enough information in the current profiles that makes it
// worth to recompile the package.
// If the return value is true all the current profiles would have been merged into
// the reference profiles accessible with open_reference_profile().
bool analyze_primary_profiles(uid_t uid,
                              const std::string& pkgname,
                              const std::string& profile_name);

// Create a snapshot of the profile information for the given package profile.
// If appId is -1, the method creates the profile snapshot for the boot image.
//
// The profile snapshot is the aggregation of all existing profiles (all current user
// profiles & the reference profile) and is meant to capture the all the profile information
// without performing a merge into the reference profile which might impact future dex2oat
// compilations.
// The snapshot is created next to the reference profile of the package and the
// ownership is assigned to AID_SYSTEM.
// The snapshot location is reference_profile_location.snapshot. If a snapshot is already
// there, it will be truncated and overwritten.
//
// The classpath acts as filter: only profiling data belonging to elements of the classpath
// will end up in the snapshot.
bool create_profile_snapshot(int32_t app_id,
                             const std::string& package,
                             const std::string& profile_name,
                             const std::string& classpath);

bool dump_profiles(int32_t uid,
                   const std::string& pkgname,
                   const std::string& profile_name,
                   const std::string& code_path);

bool copy_system_profile(const std::string& system_profile,
                         uid_t packageUid,
                         const std::string& pkgname,
                         const std::string& profile_name);

// Prepare the app profile for the given code path:
//  - create the current profile using profile_name
//  - merge the profile from the dex metadata file (if present) into
//    the reference profile.
bool prepare_app_profile(const std::string& package_name,
                         userid_t user_id,
                         appid_t app_id,
                         const std::string& profile_name,
                         const std::string& code_path,
                         const std::unique_ptr<std::string>& dex_metadata);

bool delete_odex(const char* apk_path, const char* instruction_set, const char* output_path);

bool reconcile_secondary_dex_file(const std::string& dex_path,
        const std::string& pkgname, int uid, const std::vector<std::string>& isas,
        const std::unique_ptr<std::string>& volumeUuid, int storage_flag,
        /*out*/bool* out_secondary_dex_exists);

bool hash_secondary_dex_file(const std::string& dex_path,
        const std::string& pkgname, int uid, const std::unique_ptr<std::string>& volume_uuid,
        int storage_flag, std::vector<uint8_t>* out_secondary_dex_hash);

int dexopt(const char *apk_path, uid_t uid, const char *pkgName, const char *instruction_set,
        int dexopt_needed, const char* oat_dir, int dexopt_flags, const char* compiler_filter,
        const char* volume_uuid, const char* class_loader_context, const char* se_info,
        bool downgrade, int target_sdk_version, const char* profile_name,
        const char* dexMetadataPath, const char* compilation_reason, std::string* error_msg);

bool calculate_oat_file_path_default(char path[PKG_PATH_MAX], const char *oat_dir,
        const char *apk_path, const char *instruction_set);

bool calculate_odex_file_path_default(char path[PKG_PATH_MAX], const char *apk_path,
        const char *instruction_set);

bool create_cache_path_default(char path[PKG_PATH_MAX], const char *src,
        const char *instruction_set);

bool move_ab(const char* apk_path, const char* instruction_set, const char* output_path);

const char* select_execution_binary(
        const char* binary,
        const char* debug_binary,
        bool background_job_compile,
        bool is_debug_runtime,
        bool is_release,
        bool is_debuggable_build);

}  // namespace installd
}  // namespace android

#endif  // DEXOPT_H_
