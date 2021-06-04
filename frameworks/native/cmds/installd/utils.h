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

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>

#include <dirent.h>
#include <inttypes.h>
#include <unistd.h>
#include <utime.h>

#include <cutils/multiuser.h>

#include <installd_constants.h>

#define MEASURE_DEBUG 0
#define FIXUP_DEBUG 0

#define BYPASS_QUOTA 0
#define BYPASS_SDCARDFS 0

namespace android {
namespace installd {

constexpr const char* kXattrInodeCache = "user.inode_cache";
constexpr const char* kXattrInodeCodeCache = "user.inode_code_cache";
constexpr const char* kXattrCacheGroup = "user.cache_group";
constexpr const char* kXattrCacheTombstone = "user.cache_tombstone";

std::string create_data_path(const char* volume_uuid);

std::string create_data_app_path(const char* volume_uuid);
std::string create_data_app_package_path(const char* volume_uuid, const char* package_name);

std::string create_data_user_ce_path(const char* volume_uuid, userid_t userid);
std::string create_data_user_de_path(const char* volume_uuid, userid_t userid);

std::string create_data_user_ce_package_path(const char* volume_uuid,
        userid_t user, const char* package_name);
std::string create_data_user_ce_package_path(const char* volume_uuid,
        userid_t user, const char* package_name, ino_t ce_data_inode);
std::string create_data_user_de_package_path(const char* volume_uuid,
        userid_t user, const char* package_name);
std::string create_data_user_ce_package_path_as_user_link(
        const char* volume_uuid, userid_t userid, const char* package_name);

std::string create_data_misc_ce_rollback_base_path(const char* volume_uuid, userid_t user);
std::string create_data_misc_de_rollback_base_path(const char* volume_uuid, userid_t user);
std::string create_data_misc_ce_rollback_path(const char* volume_uuid, userid_t user,
        int32_t snapshot_id);
std::string create_data_misc_de_rollback_path(const char* volume_uuid, userid_t user,
        int32_t snapshot_id);
std::string create_data_misc_ce_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name);
std::string create_data_misc_ce_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name, ino_t ce_rollback_inode);
std::string create_data_misc_de_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name);

std::string create_data_media_path(const char* volume_uuid, userid_t userid);
std::string create_data_media_package_path(const char* volume_uuid, userid_t userid,
        const char* data_type, const char* package_name);

std::string create_data_misc_legacy_path(userid_t userid);

std::string create_data_dalvik_cache_path();

std::string create_primary_cur_profile_dir_path(userid_t userid);
std::string create_primary_current_profile_package_dir_path(
        userid_t user, const std::string& package_name);

std::string create_primary_ref_profile_dir_path();
std::string create_primary_reference_profile_package_dir_path(const std::string& package_name);

std::string create_current_profile_path(
        userid_t user,
        const std::string& package_name,
        const std::string& location,
        bool is_secondary_dex);
std::string create_reference_profile_path(
        const std::string& package_name,
        const std::string& location,
        bool is_secondary_dex);
std::string create_snapshot_profile_path(
        const std::string& package,
        const std::string& profile_name);

std::vector<userid_t> get_known_users(const char* volume_uuid);

int calculate_tree_size(const std::string& path, int64_t* size,
        int32_t include_gid = -1, int32_t exclude_gid = -1, bool exclude_apps = false);

int create_user_config_path(char path[PKG_PATH_MAX], userid_t userid);

bool is_valid_filename(const std::string& name);
bool is_valid_package_name(const std::string& packageName);

int create_dir_if_needed(const std::string& pathname, mode_t mode);

int delete_dir_contents(const std::string& pathname, bool ignore_if_missing = false);
int delete_dir_contents_and_dir(const std::string& pathname, bool ignore_if_missing = false);

int delete_dir_contents(const char *pathname,
                        int also_delete_dir,
                        int (*exclusion_predicate)(const char *name, const int is_dir),
                        bool ignore_if_missing = false);

int delete_dir_contents_fd(int dfd, const char *name);

int rm_package_dir(const std::string& package_dir);

int copy_dir_files(const char *srcname, const char *dstname, uid_t owner, gid_t group);

int64_t data_disk_free(const std::string& data_path);

int get_path_inode(const std::string& path, ino_t *inode);

int write_path_inode(const std::string& parent, const char* name, const char* inode_xattr);
std::string read_path_inode(const std::string& parent, const char* name, const char* inode_xattr);
void remove_path_xattr(const std::string& path, const char* inode_xattr);

int validate_system_app_path(const char* path);
bool validate_secondary_dex_path(const std::string& pkgname, const std::string& dex_path,
        const char* volume_uuid, int uid, int storage_flag);

int validate_apk_path(const char *path);
int validate_apk_path_subdirs(const char *path);

int ensure_config_user_dirs(userid_t userid);

int wait_child(pid_t pid);

int prepare_app_cache_dir(const std::string& parent, const char* name, mode_t target_mode,
        uid_t uid, gid_t gid);

// Collect all non empty profiles from the global profile directory and
// put then into profile_paths. The profiles are identified based on PROFILE_EXT extension.
// If a subdirectory or profile file cannot be opened the method logs a warning and moves on.
// It returns true if there were no errors at all, and false otherwise.
bool collect_profiles(std::vector<std::string>* profiles_paths);

void drop_capabilities(uid_t uid);

}  // namespace installd
}  // namespace android

#endif  // UTILS_H_
