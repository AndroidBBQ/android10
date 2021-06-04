/*
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

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <sys/statvfs.h>

#include <android-base/logging.h>
#include <android-base/strings.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <cutils/fs.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <private/android_filesystem_config.h>

#include "dexopt_return_codes.h"
#include "globals.h"  // extern variables.

#ifndef LOG_TAG
#define LOG_TAG "installd"
#endif

#define DEBUG_XATTRS 0

using android::base::EndsWith;
using android::base::Fdopendir;
using android::base::StringPrintf;
using android::base::unique_fd;

namespace android {
namespace installd {

/**
 * Check that given string is valid filename, and that it attempts no
 * parent or child directory traversal.
 */
bool is_valid_filename(const std::string& name) {
    if (name.empty() || (name == ".") || (name == "..")
            || (name.find('/') != std::string::npos)) {
        return false;
    } else {
        return true;
    }
}

static void check_package_name(const char* package_name) {
    CHECK(is_valid_filename(package_name));
    CHECK(is_valid_package_name(package_name));
}

static std::string resolve_ce_path_by_inode_or_fallback(const std::string& root_path,
        ino_t ce_data_inode, const std::string& fallback) {
    if (ce_data_inode != 0) {
        DIR* dir = opendir(root_path.c_str());
        if (dir == nullptr) {
            PLOG(ERROR) << "Failed to opendir " << root_path;
            return fallback;
        }

        struct dirent* ent;
        while ((ent = readdir(dir))) {
            if (ent->d_ino == ce_data_inode) {
                auto resolved = StringPrintf("%s/%s", root_path.c_str(), ent->d_name);
                if (resolved != fallback) {
                    LOG(DEBUG) << "Resolved path " << resolved << " for inode " << ce_data_inode
                            << " instead of " << fallback;
                }
                closedir(dir);
                return resolved;
            }
        }
        LOG(WARNING) << "Failed to resolve inode " << ce_data_inode << "; using " << fallback;
        closedir(dir);
        return fallback;
    } else {
        return fallback;
    }
}

/**
 * Create the path name where package app contents should be stored for
 * the given volume UUID and package name.  An empty UUID is assumed to
 * be internal storage.
 */
std::string create_data_app_package_path(const char* volume_uuid,
        const char* package_name) {
    check_package_name(package_name);
    return StringPrintf("%s/%s",
            create_data_app_path(volume_uuid).c_str(), package_name);
}

/**
 * Create the path name where package data should be stored for the given
 * volume UUID, package name, and user ID. An empty UUID is assumed to be
 * internal storage.
 */
std::string create_data_user_ce_package_path(const char* volume_uuid,
        userid_t user, const char* package_name) {
    check_package_name(package_name);
    return StringPrintf("%s/%s",
            create_data_user_ce_path(volume_uuid, user).c_str(), package_name);
}

/**
 * Create the path name where package data should be stored for the given
 * volume UUID, package name, and user ID. An empty UUID is assumed to be
 * internal storage.
 * Compared to create_data_user_ce_package_path this method always return the
 * ".../user/..." directory.
 */
std::string create_data_user_ce_package_path_as_user_link(
        const char* volume_uuid, userid_t userid, const char* package_name) {
    check_package_name(package_name);
    std::string data(create_data_path(volume_uuid));
    return StringPrintf("%s/user/%u/%s", data.c_str(), userid, package_name);
}

std::string create_data_user_ce_package_path(const char* volume_uuid, userid_t user,
        const char* package_name, ino_t ce_data_inode) {
    // For testing purposes, rely on the inode when defined; this could be
    // optimized to use access() in the future.
    auto fallback = create_data_user_ce_package_path(volume_uuid, user, package_name);
    auto user_path = create_data_user_ce_path(volume_uuid, user);
    return resolve_ce_path_by_inode_or_fallback(user_path, ce_data_inode, fallback);
}

std::string create_data_user_de_package_path(const char* volume_uuid,
        userid_t user, const char* package_name) {
    check_package_name(package_name);
    return StringPrintf("%s/%s",
            create_data_user_de_path(volume_uuid, user).c_str(), package_name);
}

std::string create_data_path(const char* volume_uuid) {
    if (volume_uuid == nullptr) {
        return "/data";
    } else if (!strcmp(volume_uuid, "TEST")) {
        CHECK(property_get_bool("ro.debuggable", false));
        return "/data/local/tmp";
    } else {
        CHECK(is_valid_filename(volume_uuid));
        return StringPrintf("/mnt/expand/%s", volume_uuid);
    }
}

/**
 * Create the path name for app data.
 */
std::string create_data_app_path(const char* volume_uuid) {
    return StringPrintf("%s/app", create_data_path(volume_uuid).c_str());
}

/**
 * Create the path name for user data for a certain userid.
 * Keep same implementation as vold to minimize path walking overhead
 */
std::string create_data_user_ce_path(const char* volume_uuid, userid_t userid) {
    std::string data(create_data_path(volume_uuid));
    if (volume_uuid == nullptr && userid == 0) {
        std::string legacy = StringPrintf("%s/data", data.c_str());
        struct stat sb;
        if (lstat(legacy.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
            /* /data/data is dir, return /data/data for legacy system */
            return legacy;
        }
    }
    return StringPrintf("%s/user/%u", data.c_str(), userid);
}

/**
 * Create the path name for device encrypted user data for a certain userid.
 */
std::string create_data_user_de_path(const char* volume_uuid, userid_t userid) {
    std::string data(create_data_path(volume_uuid));
    return StringPrintf("%s/user_de/%u", data.c_str(), userid);
}

std::string create_data_misc_ce_rollback_base_path(const char* volume_uuid, userid_t user) {
    return StringPrintf("%s/misc_ce/%u/rollback", create_data_path(volume_uuid).c_str(), user);
}

std::string create_data_misc_de_rollback_base_path(const char* volume_uuid, userid_t user) {
    return StringPrintf("%s/misc_de/%u/rollback", create_data_path(volume_uuid).c_str(), user);
}

std::string create_data_misc_ce_rollback_path(const char* volume_uuid, userid_t user,
        int32_t snapshot_id) {
    return StringPrintf("%s/%d", create_data_misc_ce_rollback_base_path(volume_uuid, user).c_str(),
          snapshot_id);
}

std::string create_data_misc_de_rollback_path(const char* volume_uuid, userid_t user,
        int32_t snapshot_id) {
    return StringPrintf("%s/%d", create_data_misc_de_rollback_base_path(volume_uuid, user).c_str(),
          snapshot_id);
}

std::string create_data_misc_ce_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name) {
    return StringPrintf("%s/%s",
           create_data_misc_ce_rollback_path(volume_uuid, user, snapshot_id).c_str(), package_name);
}

std::string create_data_misc_ce_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name, ino_t ce_rollback_inode) {
    auto fallback = create_data_misc_ce_rollback_package_path(volume_uuid, user, snapshot_id,
            package_name);
    auto user_path = create_data_misc_ce_rollback_path(volume_uuid, user, snapshot_id);
    return resolve_ce_path_by_inode_or_fallback(user_path, ce_rollback_inode, fallback);
}

std::string create_data_misc_de_rollback_package_path(const char* volume_uuid,
        userid_t user, int32_t snapshot_id, const char* package_name) {
    return StringPrintf("%s/%s",
           create_data_misc_de_rollback_path(volume_uuid, user, snapshot_id).c_str(), package_name);
}

/**
 * Create the path name for media for a certain userid.
 */
std::string create_data_media_path(const char* volume_uuid, userid_t userid) {
    return StringPrintf("%s/media/%u", create_data_path(volume_uuid).c_str(), userid);
}

std::string create_data_media_package_path(const char* volume_uuid, userid_t userid,
        const char* data_type, const char* package_name) {
    return StringPrintf("%s/Android/%s/%s", create_data_media_path(volume_uuid, userid).c_str(),
            data_type, package_name);
}

std::string create_data_misc_legacy_path(userid_t userid) {
    return StringPrintf("%s/misc/user/%u", create_data_path(nullptr).c_str(), userid);
}

std::string create_primary_cur_profile_dir_path(userid_t userid) {
    return StringPrintf("%s/cur/%u", android_profiles_dir.c_str(), userid);
}

std::string create_primary_current_profile_package_dir_path(userid_t user,
        const std::string& package_name) {
    check_package_name(package_name.c_str());
    return StringPrintf("%s/%s",
            create_primary_cur_profile_dir_path(user).c_str(), package_name.c_str());
}

std::string create_primary_ref_profile_dir_path() {
    return StringPrintf("%s/ref", android_profiles_dir.c_str());
}

std::string create_primary_reference_profile_package_dir_path(const std::string& package_name) {
    check_package_name(package_name.c_str());
    return StringPrintf("%s/ref/%s", android_profiles_dir.c_str(), package_name.c_str());
}

std::string create_data_dalvik_cache_path() {
    return "/data/dalvik-cache";
}

// Keep profile paths in sync with ActivityThread and LoadedApk.
const std::string PROFILE_EXT = ".prof";
const std::string CURRENT_PROFILE_EXT = ".cur";
const std::string SNAPSHOT_PROFILE_EXT = ".snapshot";

// Gets the parent directory and the file name for the given secondary dex path.
// Returns true on success, false on failure (if the dex_path does not have the expected
// structure).
static bool get_secondary_dex_location(const std::string& dex_path,
        std::string* out_dir_name, std::string* out_file_name) {
   size_t dirIndex = dex_path.rfind('/');
   if (dirIndex == std::string::npos) {
        return false;
   }
   if (dirIndex == dex_path.size() - 1) {
        return false;
   }
   *out_dir_name = dex_path.substr(0, dirIndex);
   *out_file_name = dex_path.substr(dirIndex + 1);

   return true;
}

std::string create_current_profile_path(userid_t user, const std::string& package_name,
        const std::string& location, bool is_secondary_dex) {
    if (is_secondary_dex) {
        // Secondary dex current profiles are stored next to the dex files under the oat folder.
        std::string dex_dir;
        std::string dex_name;
        CHECK(get_secondary_dex_location(location, &dex_dir, &dex_name))
                << "Unexpected dir structure for secondary dex " << location;
        return StringPrintf("%s/oat/%s%s%s",
                dex_dir.c_str(), dex_name.c_str(), CURRENT_PROFILE_EXT.c_str(),
                PROFILE_EXT.c_str());
    } else {
        // Profiles for primary apks are under /data/misc/profiles/cur.
        std::string profile_dir = create_primary_current_profile_package_dir_path(
                user, package_name);
        return StringPrintf("%s/%s", profile_dir.c_str(), location.c_str());
    }
}

std::string create_reference_profile_path(const std::string& package_name,
        const std::string& location, bool is_secondary_dex) {
    if (is_secondary_dex) {
        // Secondary dex reference profiles are stored next to the dex files under the oat folder.
        std::string dex_dir;
        std::string dex_name;
        CHECK(get_secondary_dex_location(location, &dex_dir, &dex_name))
                << "Unexpected dir structure for secondary dex " << location;
        return StringPrintf("%s/oat/%s%s",
                dex_dir.c_str(), dex_name.c_str(), PROFILE_EXT.c_str());
    } else {
        // Reference profiles for primary apks are stored in /data/misc/profile/ref.
        std::string profile_dir = create_primary_reference_profile_package_dir_path(package_name);
        return StringPrintf("%s/%s", profile_dir.c_str(), location.c_str());
    }
}

std::string create_snapshot_profile_path(const std::string& package,
        const std::string& profile_name) {
    std::string ref_profile = create_reference_profile_path(package, profile_name,
            /*is_secondary_dex*/ false);
    return ref_profile + SNAPSHOT_PROFILE_EXT;
}

std::vector<userid_t> get_known_users(const char* volume_uuid) {
    std::vector<userid_t> users;

    // We always have an owner
    users.push_back(0);

    std::string path(create_data_path(volume_uuid) + "/" + SECONDARY_USER_PREFIX);
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        // Unable to discover other users, but at least return owner
        PLOG(ERROR) << "Failed to opendir " << path;
        return users;
    }

    struct dirent* ent;
    while ((ent = readdir(dir))) {
        if (ent->d_type != DT_DIR) {
            continue;
        }

        char* end;
        userid_t user = strtol(ent->d_name, &end, 10);
        if (*end == '\0' && user != 0) {
            LOG(DEBUG) << "Found valid user " << user;
            users.push_back(user);
        }
    }
    closedir(dir);

    return users;
}

int calculate_tree_size(const std::string& path, int64_t* size,
        int32_t include_gid, int32_t exclude_gid, bool exclude_apps) {
    FTS *fts;
    FTSENT *p;
    int64_t matchedSize = 0;
    char *argv[] = { (char*) path.c_str(), nullptr };
    if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
        if (errno != ENOENT) {
            PLOG(ERROR) << "Failed to fts_open " << path;
        }
        return -1;
    }
    while ((p = fts_read(fts)) != nullptr) {
        switch (p->fts_info) {
        case FTS_D:
        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
            int32_t uid = p->fts_statp->st_uid;
            int32_t gid = p->fts_statp->st_gid;
            int32_t user_uid = multiuser_get_app_id(uid);
            int32_t user_gid = multiuser_get_app_id(gid);
            if (exclude_apps && ((user_uid >= AID_APP_START && user_uid <= AID_APP_END)
                    || (user_gid >= AID_CACHE_GID_START && user_gid <= AID_CACHE_GID_END)
                    || (user_gid >= AID_SHARED_GID_START && user_gid <= AID_SHARED_GID_END))) {
                // Don't traverse inside or measure
                fts_set(fts, p, FTS_SKIP);
                break;
            }
            if (include_gid != -1 && gid != include_gid) {
                break;
            }
            if (exclude_gid != -1 && gid == exclude_gid) {
                break;
            }
            matchedSize += (p->fts_statp->st_blocks * 512);
            break;
        }
    }
    fts_close(fts);
#if MEASURE_DEBUG
    if ((include_gid == -1) && (exclude_gid == -1)) {
        LOG(DEBUG) << "Measured " << path << " size " << matchedSize;
    } else {
        LOG(DEBUG) << "Measured " << path << " size " << matchedSize << "; include " << include_gid
                << " exclude " << exclude_gid;
    }
#endif
    *size += matchedSize;
    return 0;
}

/**
 * Checks whether the package name is valid. Returns -1 on error and
 * 0 on success.
 */
bool is_valid_package_name(const std::string& packageName) {
    // This logic is borrowed from PackageParser.java
    bool hasSep = false;
    bool front = true;

    auto it = packageName.begin();
    for (; it != packageName.end() && *it != '-'; it++) {
        char c = *it;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            front = false;
            continue;
        }
        if (!front) {
            if ((c >= '0' && c <= '9') || c == '_') {
                continue;
            }
        }
        if (c == '.') {
            hasSep = true;
            front = true;
            continue;
        }
        LOG(WARNING) << "Bad package character " << c << " in " << packageName;
        return false;
    }

    if (front) {
        LOG(WARNING) << "Missing separator in " << packageName;
        return false;
    }

    for (; it != packageName.end(); it++) {
        char c = *it;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) continue;
        if ((c >= '0' && c <= '9') || c == '_' || c == '-' || c == '=') continue;
        LOG(WARNING) << "Bad suffix character " << c << " in " << packageName;
        return false;
    }

    return true;
}

static int _delete_dir_contents(DIR *d,
                                int (*exclusion_predicate)(const char *name, const int is_dir))
{
    int result = 0;
    struct dirent *de;
    int dfd;

    dfd = dirfd(d);

    if (dfd < 0) return -1;

    while ((de = readdir(d))) {
        const char *name = de->d_name;

            /* check using the exclusion predicate, if provided */
        if (exclusion_predicate && exclusion_predicate(name, (de->d_type == DT_DIR))) {
            continue;
        }

        if (de->d_type == DT_DIR) {
            int subfd;
            DIR *subdir;

                /* always skip "." and ".." */
            if (name[0] == '.') {
                if (name[1] == 0) continue;
                if ((name[1] == '.') && (name[2] == 0)) continue;
            }

            subfd = openat(dfd, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (subfd < 0) {
                ALOGE("Couldn't openat %s: %s\n", name, strerror(errno));
                result = -1;
                continue;
            }
            subdir = fdopendir(subfd);
            if (subdir == nullptr) {
                ALOGE("Couldn't fdopendir %s: %s\n", name, strerror(errno));
                close(subfd);
                result = -1;
                continue;
            }
            if (_delete_dir_contents(subdir, exclusion_predicate)) {
                result = -1;
            }
            closedir(subdir);
            if (unlinkat(dfd, name, AT_REMOVEDIR) < 0) {
                ALOGE("Couldn't unlinkat %s: %s\n", name, strerror(errno));
                result = -1;
            }
        } else {
            if (unlinkat(dfd, name, 0) < 0) {
                ALOGE("Couldn't unlinkat %s: %s\n", name, strerror(errno));
                result = -1;
            }
        }
    }

    return result;
}

int create_dir_if_needed(const std::string& pathname, mode_t perms) {
    struct stat st;

    int rc;
    if ((rc = stat(pathname.c_str(), &st)) != 0) {
        if (errno == ENOENT) {
            return mkdir(pathname.c_str(), perms);
        } else {
            return rc;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        LOG(DEBUG) << pathname << " is not a folder";
        return -1;
    }

    mode_t actual_perms = st.st_mode & ALLPERMS;
    if (actual_perms != perms) {
        LOG(WARNING) << pathname << " permissions " << actual_perms << " expected " << perms;
        return -1;
    }

    return 0;
}

int delete_dir_contents(const std::string& pathname, bool ignore_if_missing) {
    return delete_dir_contents(pathname.c_str(), 0, nullptr, ignore_if_missing);
}

int delete_dir_contents_and_dir(const std::string& pathname, bool ignore_if_missing) {
    return delete_dir_contents(pathname.c_str(), 1, nullptr, ignore_if_missing);
}

int delete_dir_contents(const char *pathname,
                        int also_delete_dir,
                        int (*exclusion_predicate)(const char*, const int),
                        bool ignore_if_missing)
{
    int res = 0;
    DIR *d;

    d = opendir(pathname);
    if (d == nullptr) {
        if (ignore_if_missing && (errno == ENOENT)) {
            return 0;
        }
        ALOGE("Couldn't opendir %s: %s\n", pathname, strerror(errno));
        return -errno;
    }
    res = _delete_dir_contents(d, exclusion_predicate);
    closedir(d);
    if (also_delete_dir) {
        if (rmdir(pathname)) {
            ALOGE("Couldn't rmdir %s: %s\n", pathname, strerror(errno));
            res = -1;
        }
    }
    return res;
}

int delete_dir_contents_fd(int dfd, const char *name)
{
    int fd, res;
    DIR *d;

    fd = openat(dfd, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("Couldn't openat %s: %s\n", name, strerror(errno));
        return -1;
    }
    d = fdopendir(fd);
    if (d == nullptr) {
        ALOGE("Couldn't fdopendir %s: %s\n", name, strerror(errno));
        close(fd);
        return -1;
    }
    res = _delete_dir_contents(d, nullptr);
    closedir(d);
    return res;
}

static int _copy_owner_permissions(int srcfd, int dstfd)
{
    struct stat st;
    if (fstat(srcfd, &st) != 0) {
        return -1;
    }
    if (fchmod(dstfd, st.st_mode) != 0) {
        return -1;
    }
    return 0;
}

static int _copy_dir_files(int sdfd, int ddfd, uid_t owner, gid_t group)
{
    int result = 0;
    if (_copy_owner_permissions(sdfd, ddfd) != 0) {
        ALOGE("_copy_dir_files failed to copy dir permissions\n");
    }
    if (fchown(ddfd, owner, group) != 0) {
        ALOGE("_copy_dir_files failed to change dir owner\n");
    }

    DIR *ds = fdopendir(sdfd);
    if (ds == nullptr) {
        ALOGE("Couldn't fdopendir: %s\n", strerror(errno));
        return -1;
    }
    struct dirent *de;
    while ((de = readdir(ds))) {
        if (de->d_type != DT_REG) {
            continue;
        }

        const char *name = de->d_name;
        int fsfd = openat(sdfd, name, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        int fdfd = openat(ddfd, name, O_WRONLY | O_NOFOLLOW | O_CLOEXEC | O_CREAT, 0600);
        if (fsfd == -1 || fdfd == -1) {
            ALOGW("Couldn't copy %s: %s\n", name, strerror(errno));
        } else {
            if (_copy_owner_permissions(fsfd, fdfd) != 0) {
                ALOGE("Failed to change file permissions\n");
            }
            if (fchown(fdfd, owner, group) != 0) {
                ALOGE("Failed to change file owner\n");
            }

            char buf[8192];
            ssize_t size;
            while ((size = read(fsfd, buf, sizeof(buf))) > 0) {
                write(fdfd, buf, size);
            }
            if (size < 0) {
                ALOGW("Couldn't copy %s: %s\n", name, strerror(errno));
                result = -1;
            }
        }
        close(fdfd);
        close(fsfd);
    }

    return result;
}

int copy_dir_files(const char *srcname,
                   const char *dstname,
                   uid_t owner,
                   uid_t group)
{
    int res = 0;
    DIR *ds = nullptr;
    DIR *dd = nullptr;

    ds = opendir(srcname);
    if (ds == nullptr) {
        ALOGE("Couldn't opendir %s: %s\n", srcname, strerror(errno));
        return -errno;
    }

    mkdir(dstname, 0600);
    dd = opendir(dstname);
    if (dd == nullptr) {
        ALOGE("Couldn't opendir %s: %s\n", dstname, strerror(errno));
        closedir(ds);
        return -errno;
    }

    int sdfd = dirfd(ds);
    int ddfd = dirfd(dd);
    if (sdfd != -1 && ddfd != -1) {
        res = _copy_dir_files(sdfd, ddfd, owner, group);
    } else {
        res = -errno;
    }
    closedir(dd);
    closedir(ds);
    return res;
}

int64_t data_disk_free(const std::string& data_path) {
    struct statvfs sfs;
    if (statvfs(data_path.c_str(), &sfs) == 0) {
        return static_cast<int64_t>(sfs.f_bavail) * sfs.f_frsize;
    } else {
        PLOG(ERROR) << "Couldn't statvfs " << data_path;
        return -1;
    }
}

int get_path_inode(const std::string& path, ino_t *inode) {
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (stat(path.c_str(), &buf) != 0) {
        PLOG(WARNING) << "Failed to stat " << path;
        return -1;
    } else {
        *inode = buf.st_ino;
        return 0;
    }
}

/**
 * Write the inode of a specific child file into the given xattr on the
 * parent directory. This allows you to find the child later, even if its
 * name is encrypted.
 */
int write_path_inode(const std::string& parent, const char* name, const char* inode_xattr) {
    ino_t inode = 0;
    uint64_t inode_raw = 0;
    auto path = StringPrintf("%s/%s", parent.c_str(), name);

    if (get_path_inode(path, &inode) != 0) {
        // Path probably doesn't exist yet; ignore
        return 0;
    }

    // Check to see if already set correctly
    if (getxattr(parent.c_str(), inode_xattr, &inode_raw, sizeof(inode_raw)) == sizeof(inode_raw)) {
        if (inode_raw == inode) {
            // Already set correctly; skip writing
            return 0;
        } else {
            PLOG(WARNING) << "Mismatched inode value; found " << inode
                    << " on disk but marked value was " << inode_raw << "; overwriting";
        }
    }

    inode_raw = inode;
    if (setxattr(parent.c_str(), inode_xattr, &inode_raw, sizeof(inode_raw), 0) != 0 && errno != EOPNOTSUPP) {
        PLOG(ERROR) << "Failed to write xattr " << inode_xattr << " at " << parent;
        return -1;
    } else {
        return 0;
    }
}

/**
 * Read the inode of a specific child file from the given xattr on the
 * parent directory. Returns a currently valid path for that child, which
 * might have an encrypted name.
 */
std::string read_path_inode(const std::string& parent, const char* name, const char* inode_xattr) {
    ino_t inode = 0;
    uint64_t inode_raw = 0;
    auto fallback = StringPrintf("%s/%s", parent.c_str(), name);

    // Lookup the inode value written earlier
    if (getxattr(parent.c_str(), inode_xattr, &inode_raw, sizeof(inode_raw)) == sizeof(inode_raw)) {
        inode = inode_raw;
    }

    // For testing purposes, rely on the inode when defined; this could be
    // optimized to use access() in the future.
    if (inode != 0) {
        DIR* dir = opendir(parent.c_str());
        if (dir == nullptr) {
            PLOG(ERROR) << "Failed to opendir " << parent;
            return fallback;
        }

        struct dirent* ent;
        while ((ent = readdir(dir))) {
            if (ent->d_ino == inode) {
                auto resolved = StringPrintf("%s/%s", parent.c_str(), ent->d_name);
#if DEBUG_XATTRS
                if (resolved != fallback) {
                    LOG(DEBUG) << "Resolved path " << resolved << " for inode " << inode
                            << " instead of " << fallback;
                }
#endif
                closedir(dir);
                return resolved;
            }
        }
        LOG(WARNING) << "Failed to resolve inode " << inode << "; using " << fallback;
        closedir(dir);
        return fallback;
    } else {
        return fallback;
    }
}

void remove_path_xattr(const std::string& path, const char* inode_xattr) {
    if (removexattr(path.c_str(), inode_xattr) && errno != ENODATA) {
        PLOG(ERROR) << "Failed to remove xattr " << inode_xattr << " at " << path;
    }
}

/**
 * Validate that the path is valid in the context of the provided directory.
 * The path is allowed to have at most one subdirectory and no indirections
 * to top level directories (i.e. have "..").
 */
static int validate_path(const std::string& dir, const std::string& path, int maxSubdirs) {
    // Argument sanity checking
    if (dir.find('/') != 0 || dir.rfind('/') != dir.size() - 1
            || dir.find("..") != std::string::npos) {
        LOG(ERROR) << "Invalid directory " << dir;
        return -1;
    }
    if (path.find("..") != std::string::npos) {
        LOG(ERROR) << "Invalid path " << path;
        return -1;
    }

    if (path.compare(0, dir.size(), dir) != 0) {
        // Common case, path isn't under directory
        return -1;
    }

    // Count number of subdirectories
    auto pos = path.find('/', dir.size());
    int count = 0;
    while (pos != std::string::npos) {
        auto next = path.find('/', pos + 1);
        if (next > pos + 1) {
            count++;
        }
        pos = next;
    }

    if (count > maxSubdirs) {
        LOG(ERROR) << "Invalid path depth " << path << " when tested against " << dir;
        return -1;
    }

    return 0;
}

/**
 * Checks whether a path points to a system app (.apk file). Returns 0
 * if it is a system app or -1 if it is not.
 */
int validate_system_app_path(const char* path) {
    std::string path_ = path;
    for (const auto& dir : android_system_dirs) {
        if (validate_path(dir, path, 1) == 0) {
            return 0;
        }
    }
    return -1;
}

bool validate_secondary_dex_path(const std::string& pkgname, const std::string& dex_path,
        const char* volume_uuid, int uid, int storage_flag) {
    CHECK(storage_flag == FLAG_STORAGE_CE || storage_flag == FLAG_STORAGE_DE);

    // Empty paths are not allowed.
    if (dex_path.empty()) { return false; }
    // First character should always be '/'. No relative paths.
    if (dex_path[0] != '/') { return false; }
    // The last character should not be '/'.
    if (dex_path[dex_path.size() - 1] == '/') { return false; }
    // There should be no '.' after the directory marker.
    if (dex_path.find("/.") != std::string::npos) { return false; }
    // The path should be at most PKG_PATH_MAX long.
    if (dex_path.size() > PKG_PATH_MAX) { return false; }

    // The dex_path should be under the app data directory.
    std::string app_private_dir = storage_flag == FLAG_STORAGE_CE
            ? create_data_user_ce_package_path(
                    volume_uuid, multiuser_get_user_id(uid), pkgname.c_str())
            : create_data_user_de_package_path(
                    volume_uuid, multiuser_get_user_id(uid), pkgname.c_str());

    if (strncmp(dex_path.c_str(), app_private_dir.c_str(), app_private_dir.size()) != 0) {
        // The check above might fail if the dex file is accessed via the /data/user/0 symlink.
        // If that's the case, attempt to validate against the user data link.
        std::string app_private_dir_symlink = create_data_user_ce_package_path_as_user_link(
                volume_uuid, multiuser_get_user_id(uid), pkgname.c_str());
        if (strncmp(dex_path.c_str(), app_private_dir_symlink.c_str(),
                app_private_dir_symlink.size()) != 0) {
            return false;
        }
    }

    // If we got here we have a valid path.
    return true;
}

/**
 * Check whether path points to a valid path for an APK file. The path must
 * begin with a whitelisted prefix path and must be no deeper than |maxSubdirs| within
 * that path. Returns -1 when an invalid path is encountered and 0 when a valid path
 * is encountered.
 */
static int validate_apk_path_internal(const std::string& path, int maxSubdirs) {
    if (validate_path(android_app_dir, path, maxSubdirs) == 0) {
        return 0;
    } else if (validate_path(android_staging_dir, path, maxSubdirs) == 0) {
        return 0;
    } else if (validate_path(android_app_private_dir, path, maxSubdirs) == 0) {
        return 0;
    } else if (validate_path(android_app_ephemeral_dir, path, maxSubdirs) == 0) {
        return 0;
    } else if (validate_path(android_asec_dir, path, maxSubdirs) == 0) {
        return 0;
    } else if (android::base::StartsWith(path, android_mnt_expand_dir)) {
        // Rewrite the path as if it were on internal storage, and test that
        size_t end = path.find('/', android_mnt_expand_dir.size() + 1);
        if (end != std::string::npos) {
            auto modified = path;
            modified.replace(0, end + 1, android_data_dir);
            return validate_apk_path_internal(modified, maxSubdirs);
        }
    }
    return -1;
}

int validate_apk_path(const char* path) {
    return validate_apk_path_internal(path, 1 /* maxSubdirs */);
}

int validate_apk_path_subdirs(const char* path) {
    return validate_apk_path_internal(path, 3 /* maxSubdirs */);
}

int ensure_config_user_dirs(userid_t userid) {
    // writable by system, readable by any app within the same user
    const int uid = multiuser_get_uid(userid, AID_SYSTEM);
    const int gid = multiuser_get_uid(userid, AID_EVERYBODY);

    // Ensure /data/misc/user/<userid> exists
    auto path = create_data_misc_legacy_path(userid);
    return fs_prepare_dir(path.c_str(), 0750, uid, gid);
}

int wait_child(pid_t pid)
{
    int status;
    pid_t got_pid;

    while (1) {
        got_pid = waitpid(pid, &status, 0);
        if (got_pid == -1 && errno == EINTR) {
            printf("waitpid interrupted, retrying\n");
        } else {
            break;
        }
    }
    if (got_pid != pid) {
        ALOGW("waitpid failed: wanted %d, got %d: %s\n",
            (int) pid, (int) got_pid, strerror(errno));
        return 1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    } else {
        return status;      /* always nonzero */
    }
}

/**
 * Prepare an app cache directory, which offers to fix-up the GID and
 * directory mode flags during a platform upgrade.
 * The app cache directory path will be 'parent'/'name'.
 */
int prepare_app_cache_dir(const std::string& parent, const char* name, mode_t target_mode,
        uid_t uid, gid_t gid) {
    auto path = StringPrintf("%s/%s", parent.c_str(), name);
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            // This is fine, just create it
            if (fs_prepare_dir_strict(path.c_str(), target_mode, uid, gid) != 0) {
                PLOG(ERROR) << "Failed to prepare " << path;
                return -1;
            } else {
                return 0;
            }
        } else {
            PLOG(ERROR) << "Failed to stat " << path;
            return -1;
        }
    }

    mode_t actual_mode = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID);
    if (st.st_uid != uid) {
        // Mismatched UID is real trouble; we can't recover
        LOG(ERROR) << "Mismatched UID at " << path << ": found " << st.st_uid
                << " but expected " << uid;
        return -1;
    } else if (st.st_gid == gid && actual_mode == target_mode) {
        // Everything looks good!
        return 0;
    } else {
        // Mismatched GID/mode is recoverable; fall through to update
        LOG(DEBUG) << "Mismatched cache GID/mode at " << path << ": found " << st.st_gid
                << "/" << actual_mode << " but expected " << gid << "/" << target_mode;
    }

    // Directory is owned correctly, but GID or mode mismatch means it's
    // probably a platform upgrade so we need to fix them
    FTS *fts;
    FTSENT *p;
    char *argv[] = { (char*) path.c_str(), nullptr };
    if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
        PLOG(ERROR) << "Failed to fts_open " << path;
        return -1;
    }
    while ((p = fts_read(fts)) != nullptr) {
        switch (p->fts_info) {
        case FTS_DP:
            if (chmod(p->fts_path, target_mode) != 0) {
                PLOG(WARNING) << "Failed to chmod " << p->fts_path;
            }
            [[fallthrough]]; // to also set GID
        case FTS_F:
            if (chown(p->fts_path, -1, gid) != 0) {
                PLOG(WARNING) << "Failed to chown " << p->fts_path;
            }
            break;
        case FTS_SL:
        case FTS_SLNONE:
            if (lchown(p->fts_path, -1, gid) != 0) {
                PLOG(WARNING) << "Failed to chown " << p->fts_path;
            }
            break;
        }
    }
    fts_close(fts);
    return 0;
}

// Collect all non empty profiles from the given directory and puts then into profile_paths.
// The profiles are identified based on PROFILE_EXT extension.
// If a subdirectory or profile file cannot be opened the method logs a warning and moves on.
// It returns true if there were no errors at all, and false otherwise.
static bool collect_profiles(DIR* d,
                             const std::string& current_path,
                             std::vector<std::string>* profiles_paths) {
    int32_t dir_fd = dirfd(d);
    if (dir_fd < 0) {
        return false;
    }

    bool result = true;
    struct dirent* dir_entry;
    while ((dir_entry = readdir(d))) {
        std::string name = dir_entry->d_name;
        std::string local_path = current_path + "/" + name;

        if (dir_entry->d_type == DT_REG) {
            // Check if this is a non empty profile file.
            if (EndsWith(name, PROFILE_EXT)) {
                struct stat st;
                if (stat(local_path.c_str(), &st) != 0) {
                    PLOG(WARNING) << "Cannot stat local path " << local_path;
                    result = false;
                    continue;
                } else if (st.st_size > 0) {
                    profiles_paths->push_back(local_path);
                }
            }
        } else if (dir_entry->d_type == DT_DIR) {
            // always skip "." and ".."
            if (name == "." || name == "..") {
                continue;
            }

            unique_fd subdir_fd(openat(dir_fd, name.c_str(),
                    O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
            if (subdir_fd < 0) {
                PLOG(WARNING) << "Could not open dir path " << local_path;
                result = false;
                continue;
            }

            DIR* subdir = Fdopendir(std::move(subdir_fd));
            if (subdir == nullptr) {
                PLOG(WARNING) << "Could not open dir path " << local_path;
                result = false;
                continue;
            }
            bool new_result = collect_profiles(subdir, local_path, profiles_paths);
            result = result && new_result;
            if (closedir(subdir) != 0) {
                PLOG(WARNING) << "Could not close dir path " << local_path;
            }
        }
    }

    return result;
}

bool collect_profiles(std::vector<std::string>* profiles_paths) {
    DIR* d = opendir(android_profiles_dir.c_str());
    if (d == nullptr) {
        return false;
    } else {
        return collect_profiles(d, android_profiles_dir, profiles_paths);
    }
}

void drop_capabilities(uid_t uid) {
    if (setgid(uid) != 0) {
        PLOG(ERROR) << "setgid(" << uid << ") failed in installd during dexopt";
        exit(DexoptReturnCodes::kSetGid);
    }
    if (setuid(uid) != 0) {
        PLOG(ERROR) << "setuid(" << uid << ") failed in installd during dexopt";
        exit(DexoptReturnCodes::kSetUid);
    }
    // drop capabilities
    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];
    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    if (capset(&capheader, &capdata[0]) < 0) {
        PLOG(ERROR) << "capset failed";
        exit(DexoptReturnCodes::kCapSet);
    }
}

}  // namespace installd
}  // namespace android
