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

#include "InstalldNativeService.h"

#define ATRACE_TAG ATRACE_TAG_PACKAGE_MANAGER

#include <algorithm>
#include <errno.h>
#include <fstream>
#include <fts.h>
#include <functional>
#include <inttypes.h>
#include <regex>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/scopeguard.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <cutils/ashmem.h>
#include <cutils/fs.h>
#include <cutils/properties.h>
#include <cutils/sched_policy.h>
#include <log/log.h>               // TODO: Move everything to base/logging.
#include <logwrap/logwrap.h>
#include <private/android_filesystem_config.h>
#include <selinux/android.h>
#include <system/thread_defs.h>
#include <utils/Trace.h>

#include "dexopt.h"
#include "globals.h"
#include "installd_deps.h"
#include "otapreopt_utils.h"
#include "utils.h"
#include "view_compiler.h"

#include "CacheTracker.h"
#include "MatchExtensionGen.h"
#include "QuotaUtils.h"

#ifndef LOG_TAG
#define LOG_TAG "installd"
#endif

using android::base::StringPrintf;
using std::endl;

namespace android {
namespace installd {

// An uuid used in unit tests.
static constexpr const char* kTestUuid = "TEST";

static constexpr const mode_t kRollbackFolderMode = 0700;

static constexpr const char* kCpPath = "/system/bin/cp";
static constexpr const char* kXattrDefault = "user.default";

static constexpr const int MIN_RESTRICTED_HOME_SDK_VERSION = 24; // > M

static constexpr const char* PKG_LIB_POSTFIX = "/lib";
static constexpr const char* CACHE_DIR_POSTFIX = "/cache";
static constexpr const char* CODE_CACHE_DIR_POSTFIX = "/code_cache";

static constexpr const char *kIdMapPath = "/system/bin/idmap";
static constexpr const char* IDMAP_PREFIX = "/data/resource-cache/";
static constexpr const char* IDMAP_SUFFIX = "@idmap";

// fsverity assumes the page size is always 4096. If not, the feature can not be
// enabled.
static constexpr int kVerityPageSize = 4096;
static constexpr size_t kSha256Size = 32;
static constexpr const char* kPropApkVerityMode = "ro.apk_verity.mode";

namespace {

constexpr const char* kDump = "android.permission.DUMP";

static binder::Status ok() {
    return binder::Status::ok();
}

static binder::Status exception(uint32_t code, const std::string& msg) {
    LOG(ERROR) << msg << " (" << code << ")";
    return binder::Status::fromExceptionCode(code, String8(msg.c_str()));
}

static binder::Status error() {
    return binder::Status::fromServiceSpecificError(errno);
}

static binder::Status error(const std::string& msg) {
    PLOG(ERROR) << msg;
    return binder::Status::fromServiceSpecificError(errno, String8(msg.c_str()));
}

static binder::Status error(uint32_t code, const std::string& msg) {
    LOG(ERROR) << msg << " (" << code << ")";
    return binder::Status::fromServiceSpecificError(code, String8(msg.c_str()));
}

binder::Status checkPermission(const char* permission) {
    pid_t pid;
    uid_t uid;

    if (checkCallingPermission(String16(permission), reinterpret_cast<int32_t*>(&pid),
            reinterpret_cast<int32_t*>(&uid))) {
        return ok();
    } else {
        return exception(binder::Status::EX_SECURITY,
                StringPrintf("UID %d / PID %d lacks permission %s", uid, pid, permission));
    }
}

binder::Status checkUid(uid_t expectedUid) {
    uid_t uid = IPCThreadState::self()->getCallingUid();
    if (uid == expectedUid || uid == AID_ROOT) {
        return ok();
    } else {
        return exception(binder::Status::EX_SECURITY,
                StringPrintf("UID %d is not expected UID %d", uid, expectedUid));
    }
}

binder::Status checkArgumentUuid(const std::unique_ptr<std::string>& uuid) {
    if (!uuid || is_valid_filename(*uuid)) {
        return ok();
    } else {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                StringPrintf("UUID %s is malformed", uuid->c_str()));
    }
}

binder::Status checkArgumentUuidTestOrNull(const std::unique_ptr<std::string>& uuid) {
    if (!uuid || strcmp(uuid->c_str(), kTestUuid) == 0) {
        return ok();
    } else {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                StringPrintf("UUID must be null or \"%s\", got: %s", kTestUuid, uuid->c_str()));
    }
}

binder::Status checkArgumentPackageName(const std::string& packageName) {
    if (is_valid_package_name(packageName)) {
        return ok();
    } else {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                StringPrintf("Package name %s is malformed", packageName.c_str()));
    }
}

binder::Status checkArgumentPath(const std::string& path) {
    if (path.empty()) {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT, "Missing path");
    }
    if (path[0] != '/') {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                StringPrintf("Path %s is relative", path.c_str()));
    }
    if ((path + '/').find("/../") != std::string::npos) {
        return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                StringPrintf("Path %s is shady", path.c_str()));
    }
    for (const char& c : path) {
        if (c == '\0' || c == '\n') {
            return exception(binder::Status::EX_ILLEGAL_ARGUMENT,
                    StringPrintf("Path %s is malformed", path.c_str()));
        }
    }
    return ok();
}

binder::Status checkArgumentPath(const std::unique_ptr<std::string>& path) {
    if (path) {
        return checkArgumentPath(*path);
    } else {
        return ok();
    }
}

#define ENFORCE_UID(uid) {                                  \
    binder::Status status = checkUid((uid));                \
    if (!status.isOk()) {                                   \
        return status;                                      \
    }                                                       \
}

#define CHECK_ARGUMENT_UUID(uuid) {                         \
    binder::Status status = checkArgumentUuid((uuid));      \
    if (!status.isOk()) {                                   \
        return status;                                      \
    }                                                       \
}

#define CHECK_ARGUMENT_UUID_IS_TEST_OR_NULL(uuid) {         \
    auto status = checkArgumentUuidTestOrNull(uuid);        \
    if (!status.isOk()) {                                   \
        return status;                                      \
    }                                                       \
}                                                           \

#define CHECK_ARGUMENT_PACKAGE_NAME(packageName) {          \
    binder::Status status =                                 \
            checkArgumentPackageName((packageName));        \
    if (!status.isOk()) {                                   \
        return status;                                      \
    }                                                       \
}

#define CHECK_ARGUMENT_PATH(path) {                         \
    binder::Status status = checkArgumentPath((path));      \
    if (!status.isOk()) {                                   \
        return status;                                      \
    }                                                       \
}

#define ASSERT_PAGE_SIZE_4K() {                             \
    if (getpagesize() != kVerityPageSize) {                 \
        return error("FSVerity only supports 4K pages");     \
    }                                                       \
}

}  // namespace

status_t InstalldNativeService::start() {
    IPCThreadState::self()->disableBackgroundScheduling(true);
    status_t ret = BinderService<InstalldNativeService>::publish();
    if (ret != android::OK) {
        return ret;
    }
    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();
    ps->giveThreadPoolName();
    return android::OK;
}

status_t InstalldNativeService::dump(int fd, const Vector<String16> & /* args */) {
    auto out = std::fstream(StringPrintf("/proc/self/fd/%d", fd));
    const binder::Status dump_permission = checkPermission(kDump);
    if (!dump_permission.isOk()) {
        out << dump_permission.toString8() << endl;
        return PERMISSION_DENIED;
    }
    std::lock_guard<std::recursive_mutex> lock(mLock);

    out << "installd is happy!" << endl;

    {
        std::lock_guard<std::recursive_mutex> lock(mMountsLock);
        out << endl << "Storage mounts:" << endl;
        for (const auto& n : mStorageMounts) {
            out << "    " << n.first << " = " << n.second << endl;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(mQuotasLock);
        out << endl << "Per-UID cache quotas:" << endl;
        for (const auto& n : mCacheQuotas) {
            out << "    " << n.first << " = " << n.second << endl;
        }
    }

    out << endl;
    out.flush();

    return NO_ERROR;
}

/**
 * Perform restorecon of the given path, but only perform recursive restorecon
 * if the label of that top-level file actually changed.  This can save us
 * significant time by avoiding no-op traversals of large filesystem trees.
 */
static int restorecon_app_data_lazy(const std::string& path, const std::string& seInfo, uid_t uid,
        bool existing) {
    int res = 0;
    char* before = nullptr;
    char* after = nullptr;

    // Note that SELINUX_ANDROID_RESTORECON_DATADATA flag is set by
    // libselinux. Not needed here.

    if (lgetfilecon(path.c_str(), &before) < 0) {
        PLOG(ERROR) << "Failed before getfilecon for " << path;
        goto fail;
    }
    if (selinux_android_restorecon_pkgdir(path.c_str(), seInfo.c_str(), uid, 0) < 0) {
        PLOG(ERROR) << "Failed top-level restorecon for " << path;
        goto fail;
    }
    if (lgetfilecon(path.c_str(), &after) < 0) {
        PLOG(ERROR) << "Failed after getfilecon for " << path;
        goto fail;
    }

    // If the initial top-level restorecon above changed the label, then go
    // back and restorecon everything recursively
    if (strcmp(before, after)) {
        if (existing) {
            LOG(DEBUG) << "Detected label change from " << before << " to " << after << " at "
                    << path << "; running recursive restorecon";
        }
        if (selinux_android_restorecon_pkgdir(path.c_str(), seInfo.c_str(), uid,
                SELINUX_ANDROID_RESTORECON_RECURSE) < 0) {
            PLOG(ERROR) << "Failed recursive restorecon for " << path;
            goto fail;
        }
    }

    goto done;
fail:
    res = -1;
done:
    free(before);
    free(after);
    return res;
}

static int restorecon_app_data_lazy(const std::string& parent, const char* name,
        const std::string& seInfo, uid_t uid, bool existing) {
    return restorecon_app_data_lazy(StringPrintf("%s/%s", parent.c_str(), name), seInfo, uid,
            existing);
}

static int prepare_app_dir(const std::string& path, mode_t target_mode, uid_t uid) {
    if (fs_prepare_dir_strict(path.c_str(), target_mode, uid, uid) != 0) {
        PLOG(ERROR) << "Failed to prepare " << path;
        return -1;
    }
    return 0;
}

static bool prepare_app_profile_dir(const std::string& packageName, int32_t appId, int32_t userId) {
    if (!property_get_bool("dalvik.vm.usejitprofiles", false)) {
        return true;
    }

    int32_t uid = multiuser_get_uid(userId, appId);
    int shared_app_gid = multiuser_get_shared_gid(userId, appId);
    if (shared_app_gid == -1) {
        // TODO(calin): this should no longer be possible but do not continue if we don't get
        // a valid shared gid.
        PLOG(WARNING) << "Invalid shared_app_gid for " << packageName;
        return true;
    }

    const std::string profile_dir =
            create_primary_current_profile_package_dir_path(userId, packageName);
    // read-write-execute only for the app user.
    if (fs_prepare_dir_strict(profile_dir.c_str(), 0700, uid, uid) != 0) {
        PLOG(ERROR) << "Failed to prepare " << profile_dir;
        return false;
    }

    const std::string ref_profile_path =
            create_primary_reference_profile_package_dir_path(packageName);

    // Prepare the reference profile directory. Note that we use the non strict version of
    // fs_prepare_dir. This will fix the permission and the ownership to the correct values.
    // This is particularly important given that in O there were some fixes for how the
    // shared_app_gid is computed.
    //
    // Note that by the time we get here we know that we are using a correct uid (otherwise
    // prepare_app_dir and the above fs_prepare_file_strict which check the uid). So we
    // are sure that the gid being used belongs to the owning app and not someone else.
    //
    // dex2oat/profman runs under the shared app gid and it needs to read/write reference profiles.
    if (fs_prepare_dir(ref_profile_path.c_str(), 0770, AID_SYSTEM, shared_app_gid) != 0) {
        PLOG(ERROR) << "Failed to prepare " << ref_profile_path;
        return false;
    }

    return true;
}

binder::Status InstalldNativeService::createAppData(const std::unique_ptr<std::string>& uuid,
        const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
        const std::string& seInfo, int32_t targetSdkVersion, int64_t* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgname = packageName.c_str();

    // Assume invalid inode unless filled in below
    if (_aidl_return != nullptr) *_aidl_return = -1;

    int32_t uid = multiuser_get_uid(userId, appId);
    int32_t cacheGid = multiuser_get_cache_gid(userId, appId);
    mode_t targetMode = targetSdkVersion >= MIN_RESTRICTED_HOME_SDK_VERSION ? 0700 : 0751;

    // If UID doesn't have a specific cache GID, use UID value
    if (cacheGid == -1) {
        cacheGid = uid;
    }

    if (flags & FLAG_STORAGE_CE) {
        auto path = create_data_user_ce_package_path(uuid_, userId, pkgname);
        bool existing = (access(path.c_str(), F_OK) == 0);

        if (prepare_app_dir(path, targetMode, uid) ||
                prepare_app_cache_dir(path, "cache", 02771, uid, cacheGid) ||
                prepare_app_cache_dir(path, "code_cache", 02771, uid, cacheGid)) {
            return error("Failed to prepare " + path);
        }

        // Consider restorecon over contents if label changed
        if (restorecon_app_data_lazy(path, seInfo, uid, existing) ||
                restorecon_app_data_lazy(path, "cache", seInfo, uid, existing) ||
                restorecon_app_data_lazy(path, "code_cache", seInfo, uid, existing)) {
            return error("Failed to restorecon " + path);
        }

        // Remember inode numbers of cache directories so that we can clear
        // contents while CE storage is locked
        if (write_path_inode(path, "cache", kXattrInodeCache) ||
                write_path_inode(path, "code_cache", kXattrInodeCodeCache)) {
            return error("Failed to write_path_inode for " + path);
        }

        // And return the CE inode of the top-level data directory so we can
        // clear contents while CE storage is locked
        if ((_aidl_return != nullptr)
                && get_path_inode(path, reinterpret_cast<ino_t*>(_aidl_return)) != 0) {
            return error("Failed to get_path_inode for " + path);
        }
    }
    if (flags & FLAG_STORAGE_DE) {
        auto path = create_data_user_de_package_path(uuid_, userId, pkgname);
        bool existing = (access(path.c_str(), F_OK) == 0);

        if (prepare_app_dir(path, targetMode, uid) ||
                prepare_app_cache_dir(path, "cache", 02771, uid, cacheGid) ||
                prepare_app_cache_dir(path, "code_cache", 02771, uid, cacheGid)) {
            return error("Failed to prepare " + path);
        }

        // Consider restorecon over contents if label changed
        if (restorecon_app_data_lazy(path, seInfo, uid, existing) ||
                restorecon_app_data_lazy(path, "cache", seInfo, uid, existing) ||
                restorecon_app_data_lazy(path, "code_cache", seInfo, uid, existing)) {
            return error("Failed to restorecon " + path);
        }

        if (!prepare_app_profile_dir(packageName, appId, userId)) {
            return error("Failed to prepare profiles for " + packageName);
        }
    }
    return ok();
}

binder::Status InstalldNativeService::migrateAppData(const std::unique_ptr<std::string>& uuid,
        const std::string& packageName, int32_t userId, int32_t flags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgname = packageName.c_str();

    // This method only exists to upgrade system apps that have requested
    // forceDeviceEncrypted, so their default storage always lives in a
    // consistent location.  This only works on non-FBE devices, since we
    // never want to risk exposing data on a device with real CE/DE storage.

    auto ce_path = create_data_user_ce_package_path(uuid_, userId, pkgname);
    auto de_path = create_data_user_de_package_path(uuid_, userId, pkgname);

    // If neither directory is marked as default, assume CE is default
    if (getxattr(ce_path.c_str(), kXattrDefault, nullptr, 0) == -1
            && getxattr(de_path.c_str(), kXattrDefault, nullptr, 0) == -1) {
        if (setxattr(ce_path.c_str(), kXattrDefault, nullptr, 0, 0) != 0) {
            return error("Failed to mark default storage " + ce_path);
        }
    }

    // Migrate default data location if needed
    auto target = (flags & FLAG_STORAGE_DE) ? de_path : ce_path;
    auto source = (flags & FLAG_STORAGE_DE) ? ce_path : de_path;

    if (getxattr(target.c_str(), kXattrDefault, nullptr, 0) == -1) {
        LOG(WARNING) << "Requested default storage " << target
                << " is not active; migrating from " << source;
        if (delete_dir_contents_and_dir(target) != 0) {
            return error("Failed to delete " + target);
        }
        if (rename(source.c_str(), target.c_str()) != 0) {
            return error("Failed to rename " + source + " to " + target);
        }
    }

    return ok();
}


binder::Status InstalldNativeService::clearAppProfiles(const std::string& packageName,
        const std::string& profileName) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    binder::Status res = ok();
    if (!clear_primary_reference_profile(packageName, profileName)) {
        res = error("Failed to clear reference profile for " + packageName);
    }
    if (!clear_primary_current_profiles(packageName, profileName)) {
        res = error("Failed to clear current profiles for " + packageName);
    }
    return res;
}

binder::Status InstalldNativeService::clearAppData(const std::unique_ptr<std::string>& uuid,
        const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgname = packageName.c_str();

    binder::Status res = ok();
    if (flags & FLAG_STORAGE_CE) {
        auto path = create_data_user_ce_package_path(uuid_, userId, pkgname, ceDataInode);
        if (flags & FLAG_CLEAR_CACHE_ONLY) {
            path = read_path_inode(path, "cache", kXattrInodeCache);
        } else if (flags & FLAG_CLEAR_CODE_CACHE_ONLY) {
            path = read_path_inode(path, "code_cache", kXattrInodeCodeCache);
        }
        if (access(path.c_str(), F_OK) == 0) {
            if (delete_dir_contents(path) != 0) {
                res = error("Failed to delete contents of " + path);
            } else if ((flags & (FLAG_CLEAR_CACHE_ONLY | FLAG_CLEAR_CODE_CACHE_ONLY)) == 0) {
                remove_path_xattr(path, kXattrInodeCache);
                remove_path_xattr(path, kXattrInodeCodeCache);
            }
        }
    }
    if (flags & FLAG_STORAGE_DE) {
        std::string suffix = "";
        bool only_cache = false;
        if (flags & FLAG_CLEAR_CACHE_ONLY) {
            suffix = CACHE_DIR_POSTFIX;
            only_cache = true;
        } else if (flags & FLAG_CLEAR_CODE_CACHE_ONLY) {
            suffix = CODE_CACHE_DIR_POSTFIX;
            only_cache = true;
        }

        auto path = create_data_user_de_package_path(uuid_, userId, pkgname) + suffix;
        if (access(path.c_str(), F_OK) == 0) {
            if (delete_dir_contents(path) != 0) {
                res = error("Failed to delete contents of " + path);
            }
        }
    }
    if (flags & FLAG_STORAGE_EXTERNAL) {
        std::lock_guard<std::recursive_mutex> lock(mMountsLock);
        for (const auto& n : mStorageMounts) {
            auto extPath = n.second;
            if (n.first.compare(0, 14, "/mnt/media_rw/") != 0) {
                extPath += StringPrintf("/%d", userId);
            } else if (userId != 0) {
                // TODO: support devices mounted under secondary users
                continue;
            }
            if (flags & FLAG_CLEAR_CACHE_ONLY) {
                // Clear only cached data from shared storage
                auto path = StringPrintf("%s/Android/data/%s/cache", extPath.c_str(), pkgname);
                if (delete_dir_contents(path, true) != 0) {
                    res = error("Failed to delete contents of " + path);
                }
            } else if (flags & FLAG_CLEAR_CODE_CACHE_ONLY) {
                // No code cache on shared storage
            } else {
                // Clear everything on shared storage
                auto path = StringPrintf("%s/Android/data/%s", extPath.c_str(), pkgname);
                if (delete_dir_contents(path, true) != 0) {
                    res = error("Failed to delete contents of " + path);
                }
                path = StringPrintf("%s/Android/media/%s", extPath.c_str(), pkgname);
                if (delete_dir_contents(path, true) != 0) {
                    res = error("Failed to delete contents of " + path);
                }
                path = StringPrintf("%s/Android/obb/%s", extPath.c_str(), pkgname);
                if (delete_dir_contents(path, true) != 0) {
                    res = error("Failed to delete contents of " + path);
                }
            }
        }
    }
    return res;
}

static int destroy_app_reference_profile(const std::string& pkgname) {
    return delete_dir_contents_and_dir(
        create_primary_reference_profile_package_dir_path(pkgname),
        /*ignore_if_missing*/ true);
}

static int destroy_app_current_profiles(const std::string& pkgname, userid_t userid) {
    return delete_dir_contents_and_dir(
        create_primary_current_profile_package_dir_path(userid, pkgname),
        /*ignore_if_missing*/ true);
}

binder::Status InstalldNativeService::destroyAppProfiles(const std::string& packageName) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    binder::Status res = ok();
    std::vector<userid_t> users = get_known_users(/*volume_uuid*/ nullptr);
    for (auto user : users) {
        if (destroy_app_current_profiles(packageName, user) != 0) {
            res = error("Failed to destroy current profiles for " + packageName);
        }
    }
    if (destroy_app_reference_profile(packageName) != 0) {
        res = error("Failed to destroy reference profile for " + packageName);
    }
    return res;
}

binder::Status InstalldNativeService::destroyAppData(const std::unique_ptr<std::string>& uuid,
        const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgname = packageName.c_str();

    binder::Status res = ok();
    if (flags & FLAG_STORAGE_CE) {
        auto path = create_data_user_ce_package_path(uuid_, userId, pkgname, ceDataInode);
        if (delete_dir_contents_and_dir(path) != 0) {
            res = error("Failed to delete " + path);
        }
    }
    if (flags & FLAG_STORAGE_DE) {
        auto path = create_data_user_de_package_path(uuid_, userId, pkgname);
        if (delete_dir_contents_and_dir(path) != 0) {
            res = error("Failed to delete " + path);
        }
        destroy_app_current_profiles(packageName, userId);
        // TODO(calin): If the package is still installed by other users it's probably
        // beneficial to keep the reference profile around.
        // Verify if it's ok to do that.
        destroy_app_reference_profile(packageName);
    }
    if (flags & FLAG_STORAGE_EXTERNAL) {
        std::lock_guard<std::recursive_mutex> lock(mMountsLock);
        for (const auto& n : mStorageMounts) {
            auto extPath = n.second;
            if (n.first.compare(0, 14, "/mnt/media_rw/") != 0) {
                extPath += StringPrintf("/%d", userId);
            } else if (userId != 0) {
                // TODO: support devices mounted under secondary users
                continue;
            }
            auto path = StringPrintf("%s/Android/data/%s", extPath.c_str(), pkgname);
            if (delete_dir_contents_and_dir(path, true) != 0) {
                res = error("Failed to delete contents of " + path);
            }
            path = StringPrintf("%s/Android/media/%s", extPath.c_str(), pkgname);
            if (delete_dir_contents_and_dir(path, true) != 0) {
                res = error("Failed to delete contents of " + path);
            }
            path = StringPrintf("%s/Android/obb/%s", extPath.c_str(), pkgname);
            if (delete_dir_contents_and_dir(path, true) != 0) {
                res = error("Failed to delete contents of " + path);
            }
        }
    }
    return res;
}

static gid_t get_cache_gid(uid_t uid) {
    int32_t gid = multiuser_get_cache_gid(multiuser_get_user_id(uid), multiuser_get_app_id(uid));
    return (gid != -1) ? gid : uid;
}

binder::Status InstalldNativeService::fixupAppData(const std::unique_ptr<std::string>& uuid,
        int32_t flags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    for (auto user : get_known_users(uuid_)) {
        ATRACE_BEGIN("fixup user");
        FTS* fts;
        FTSENT* p;
        auto ce_path = create_data_user_ce_path(uuid_, user);
        auto de_path = create_data_user_de_path(uuid_, user);
        char *argv[] = { (char*) ce_path.c_str(), (char*) de_path.c_str(), nullptr };
        if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
            return error("Failed to fts_open");
        }
        while ((p = fts_read(fts)) != nullptr) {
            if (p->fts_info == FTS_D && p->fts_level == 1) {
                // Track down inodes of cache directories
                uint64_t raw = 0;
                ino_t inode_cache = 0;
                ino_t inode_code_cache = 0;
                if (getxattr(p->fts_path, kXattrInodeCache, &raw, sizeof(raw)) == sizeof(raw)) {
                    inode_cache = raw;
                }
                if (getxattr(p->fts_path, kXattrInodeCodeCache, &raw, sizeof(raw)) == sizeof(raw)) {
                    inode_code_cache = raw;
                }

                // Figure out expected GID of each child
                FTSENT* child = fts_children(fts, 0);
                while (child != nullptr) {
                    if ((child->fts_statp->st_ino == inode_cache)
                            || (child->fts_statp->st_ino == inode_code_cache)
                            || !strcmp(child->fts_name, "cache")
                            || !strcmp(child->fts_name, "code_cache")) {
                        child->fts_number = get_cache_gid(p->fts_statp->st_uid);
                    } else {
                        child->fts_number = p->fts_statp->st_uid;
                    }
                    child = child->fts_link;
                }
            } else if (p->fts_level >= 2) {
                if (p->fts_level > 2) {
                    // Inherit GID from parent once we're deeper into tree
                    p->fts_number = p->fts_parent->fts_number;
                }

                uid_t uid = p->fts_parent->fts_statp->st_uid;
                gid_t cache_gid = get_cache_gid(uid);
                gid_t expected = p->fts_number;
                gid_t actual = p->fts_statp->st_gid;
                if (actual == expected) {
#if FIXUP_DEBUG
                    LOG(DEBUG) << "Ignoring " << p->fts_path << " with expected GID " << expected;
#endif
                    if (!(flags & FLAG_FORCE)) {
                        fts_set(fts, p, FTS_SKIP);
                    }
                } else if ((actual == uid) || (actual == cache_gid)) {
                    // Only consider fixing up when current GID belongs to app
                    if (p->fts_info != FTS_D) {
                        LOG(INFO) << "Fixing " << p->fts_path << " with unexpected GID " << actual
                                << " instead of " << expected;
                    }
                    switch (p->fts_info) {
                    case FTS_DP:
                        // If we're moving towards cache GID, we need to set S_ISGID
                        if (expected == cache_gid) {
                            if (chmod(p->fts_path, 02771) != 0) {
                                PLOG(WARNING) << "Failed to chmod " << p->fts_path;
                            }
                        }
                        [[fallthrough]]; // also set GID
                    case FTS_F:
                        if (chown(p->fts_path, -1, expected) != 0) {
                            PLOG(WARNING) << "Failed to chown " << p->fts_path;
                        }
                        break;
                    case FTS_SL:
                    case FTS_SLNONE:
                        if (lchown(p->fts_path, -1, expected) != 0) {
                            PLOG(WARNING) << "Failed to chown " << p->fts_path;
                        }
                        break;
                    }
                } else {
                    // Ignore all other GID transitions, since they're kinda shady
                    LOG(WARNING) << "Ignoring " << p->fts_path << " with unexpected GID " << actual
                            << " instead of " << expected;
                    if (!(flags & FLAG_FORCE)) {
                        fts_set(fts, p, FTS_SKIP);
                    }
                }
            }
        }
        fts_close(fts);
        ATRACE_END();
    }
    return ok();
}

static int32_t copy_directory_recursive(const char* from, const char* to) {
    char *argv[] = {
        (char*) kCpPath,
        (char*) "-F", /* delete any existing destination file first (--remove-destination) */
        (char*) "-p", /* preserve timestamps, ownership, and permissions */
        (char*) "-R", /* recurse into subdirectories (DEST must be a directory) */
        (char*) "-P", /* Do not follow symlinks [default] */
        (char*) "-d", /* don't dereference symlinks */
        (char*) from,
        (char*) to
    };

    LOG(DEBUG) << "Copying " << from << " to " << to;
    return android_fork_execvp(ARRAY_SIZE(argv), argv, nullptr, false, true);
}

binder::Status InstalldNativeService::snapshotAppData(
        const std::unique_ptr<std::string>& volumeUuid,
        const std::string& packageName, int32_t user, int32_t snapshotId,
        int32_t storageFlags, int64_t* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID_IS_TEST_OR_NULL(volumeUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* volume_uuid = volumeUuid ? volumeUuid->c_str() : nullptr;
    const char* package_name = packageName.c_str();

    binder::Status res = ok();
    // Default result to 0, it will be populated with inode of ce data snapshot
    // if FLAG_STORAGE_CE has been passed.
    if (_aidl_return != nullptr) *_aidl_return = 0;

    bool clear_ce_on_exit = false;
    bool clear_de_on_exit = false;

    auto deleter = [&clear_ce_on_exit, &clear_de_on_exit, &volume_uuid, &user, &package_name,
            &snapshotId] {
        if (clear_de_on_exit) {
            auto to = create_data_misc_de_rollback_package_path(volume_uuid, user, snapshotId,
                    package_name);
            if (delete_dir_contents(to.c_str(), 1, nullptr) != 0) {
                LOG(WARNING) << "Failed to delete app data snapshot: " << to;
            }
        }

        if (clear_ce_on_exit) {
            auto to = create_data_misc_ce_rollback_package_path(volume_uuid, user, snapshotId,
                    package_name);
            if (delete_dir_contents(to.c_str(), 1, nullptr) != 0) {
                LOG(WARNING) << "Failed to delete app data snapshot: " << to;
            }
        }
    };

    auto scope_guard = android::base::make_scope_guard(deleter);

    // The app may not have any data at all, in which case it's OK to skip here.
    auto from_ce = create_data_user_ce_package_path(volume_uuid, user, package_name);
    if (access(from_ce.c_str(), F_OK) != 0) {
        LOG(INFO) << "Missing source " << from_ce;
        return ok();
    }

    // ce_data_inode is not needed when FLAG_CLEAR_CACHE_ONLY is set.
    binder::Status clear_cache_result = clearAppData(volumeUuid, packageName, user,
            storageFlags | FLAG_CLEAR_CACHE_ONLY, 0);
    if (!clear_cache_result.isOk()) {
        // It should be fine to continue snapshot if we for some reason failed
        // to clear cache.
        LOG(WARNING) << "Failed to clear cache of app " << packageName;
    }

    // ce_data_inode is not needed when FLAG_CLEAR_CODE_CACHE_ONLY is set.
    binder::Status clear_code_cache_result = clearAppData(volumeUuid, packageName, user,
            storageFlags | FLAG_CLEAR_CODE_CACHE_ONLY, 0);
    if (!clear_code_cache_result.isOk()) {
        // It should be fine to continue snapshot if we for some reason failed
        // to clear code_cache.
        LOG(WARNING) << "Failed to clear code_cache of app " << packageName;
    }

    if (storageFlags & FLAG_STORAGE_DE) {
        auto from = create_data_user_de_package_path(volume_uuid, user, package_name);
        auto to = create_data_misc_de_rollback_path(volume_uuid, user, snapshotId);
        auto rollback_package_path = create_data_misc_de_rollback_package_path(volume_uuid, user,
            snapshotId, package_name);

        int rc = create_dir_if_needed(to.c_str(), kRollbackFolderMode);
        if (rc != 0) {
            return error(rc, "Failed to create folder " + to);
        }

        rc = delete_dir_contents(rollback_package_path, true /* ignore_if_missing */);
        if (rc != 0) {
            return error(rc, "Failed clearing existing snapshot " + rollback_package_path);
        }

        rc = copy_directory_recursive(from.c_str(), to.c_str());
        if (rc != 0) {
            res = error(rc, "Failed copying " + from + " to " + to);
            clear_de_on_exit = true;
            return res;
        }
    }

    if (storageFlags & FLAG_STORAGE_CE) {
        auto from = create_data_user_ce_package_path(volume_uuid, user, package_name);
        auto to = create_data_misc_ce_rollback_path(volume_uuid, user, snapshotId);
        auto rollback_package_path = create_data_misc_ce_rollback_package_path(volume_uuid, user,
            snapshotId, package_name);

        int rc = create_dir_if_needed(to.c_str(), kRollbackFolderMode);
        if (rc != 0) {
            return error(rc, "Failed to create folder " + to);
        }

        rc = delete_dir_contents(rollback_package_path, true /* ignore_if_missing */);
        if (rc != 0) {
            return error(rc, "Failed clearing existing snapshot " + rollback_package_path);
        }

        rc = copy_directory_recursive(from.c_str(), to.c_str());
        if (rc != 0) {
            res = error(rc, "Failed copying " + from + " to " + to);
            clear_ce_on_exit = true;
            return res;
        }
        if (_aidl_return != nullptr) {
            auto ce_snapshot_path = create_data_misc_ce_rollback_package_path(volume_uuid, user,
                    snapshotId, package_name);
            rc = get_path_inode(ce_snapshot_path, reinterpret_cast<ino_t*>(_aidl_return));
            if (rc != 0) {
                res = error(rc, "Failed to get_path_inode for " + ce_snapshot_path);
                clear_ce_on_exit = true;
                return res;
            }
        }
    }

    return res;
}

binder::Status InstalldNativeService::restoreAppDataSnapshot(
        const std::unique_ptr<std::string>& volumeUuid, const std::string& packageName,
        const int32_t appId, const std::string& seInfo, const int32_t user,
        const int32_t snapshotId, int32_t storageFlags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID_IS_TEST_OR_NULL(volumeUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* volume_uuid = volumeUuid ? volumeUuid->c_str() : nullptr;
    const char* package_name = packageName.c_str();

    auto from_ce = create_data_misc_ce_rollback_package_path(volume_uuid,
            user, snapshotId, package_name);
    auto from_de = create_data_misc_de_rollback_package_path(volume_uuid,
            user, snapshotId, package_name);

    const bool needs_ce_rollback = (storageFlags & FLAG_STORAGE_CE) &&
        (access(from_ce.c_str(), F_OK) == 0);
    const bool needs_de_rollback = (storageFlags & FLAG_STORAGE_DE) &&
        (access(from_de.c_str(), F_OK) == 0);

    if (!needs_ce_rollback && !needs_de_rollback) {
        return ok();
    }

    // We know we're going to rollback one of the CE or DE data, so we clear
    // application data first. Note that it's possible that we're asked to
    // restore both CE & DE data but that one of the restores fail. Leaving the
    // app with no data in those cases is arguably better than leaving the app
    // with mismatched / stale data.
    LOG(INFO) << "Clearing app data for " << packageName << " to restore snapshot.";
    // It's fine to pass 0 as ceDataInode here, because restoreAppDataSnapshot
    // can only be called when user unlocks the phone, meaning that CE user data
    // is decrypted.
    binder::Status res = clearAppData(volumeUuid, packageName, user, storageFlags,
            0 /* ceDataInode */);
    if (!res.isOk()) {
        return res;
    }

    if (needs_ce_rollback) {
        auto to_ce = create_data_user_ce_path(volume_uuid, user);
        int rc = copy_directory_recursive(from_ce.c_str(), to_ce.c_str());
        if (rc != 0) {
            res = error(rc, "Failed copying " + from_ce + " to " + to_ce);
            return res;
        }
    }

    if (needs_de_rollback) {
        auto to_de = create_data_user_de_path(volume_uuid, user);
        int rc = copy_directory_recursive(from_de.c_str(), to_de.c_str());
        if (rc != 0) {
            if (needs_ce_rollback) {
                auto ce_data = create_data_user_ce_package_path(volume_uuid, user, package_name);
                LOG(WARNING) << "de_data rollback failed. Erasing rolled back ce_data " << ce_data;
                if (delete_dir_contents(ce_data.c_str(), 1, nullptr) != 0) {
                    LOG(WARNING) << "Failed to delete rolled back ce_data " << ce_data;
                }
            }
            res = error(rc, "Failed copying " + from_de + " to " + to_de);
            return res;
        }
    }

    // Finally, restore the SELinux label on the app data.
    return restoreconAppData(volumeUuid, packageName, user, storageFlags, appId, seInfo);
}

binder::Status InstalldNativeService::destroyAppDataSnapshot(
        const std::unique_ptr<std::string> &volumeUuid, const std::string& packageName,
        const int32_t user, const int64_t ceSnapshotInode, const int32_t snapshotId,
        int32_t storageFlags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID_IS_TEST_OR_NULL(volumeUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* volume_uuid = volumeUuid ? volumeUuid->c_str() : nullptr;
    const char* package_name = packageName.c_str();

    if (storageFlags & FLAG_STORAGE_DE) {
        auto de_snapshot_path = create_data_misc_de_rollback_package_path(volume_uuid,
                user, snapshotId, package_name);

        int res = delete_dir_contents_and_dir(de_snapshot_path, true /* ignore_if_missing */);
        if (res != 0) {
            return error(res, "Failed clearing snapshot " + de_snapshot_path);
        }
    }

    if (storageFlags & FLAG_STORAGE_CE) {
        auto ce_snapshot_path = create_data_misc_ce_rollback_package_path(volume_uuid,
                user, snapshotId, package_name, ceSnapshotInode);
        int res = delete_dir_contents_and_dir(ce_snapshot_path, true /* ignore_if_missing */);
        if (res != 0) {
            return error(res, "Failed clearing snapshot " + ce_snapshot_path);
        }
    }
    return ok();
}


binder::Status InstalldNativeService::moveCompleteApp(const std::unique_ptr<std::string>& fromUuid,
        const std::unique_ptr<std::string>& toUuid, const std::string& packageName,
        const std::string& dataAppName, int32_t appId, const std::string& seInfo,
        int32_t targetSdkVersion) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(fromUuid);
    CHECK_ARGUMENT_UUID(toUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* from_uuid = fromUuid ? fromUuid->c_str() : nullptr;
    const char* to_uuid = toUuid ? toUuid->c_str() : nullptr;
    const char* package_name = packageName.c_str();
    const char* data_app_name = dataAppName.c_str();

    binder::Status res = ok();
    std::vector<userid_t> users = get_known_users(from_uuid);

    // Copy app
    {
        auto from = create_data_app_package_path(from_uuid, data_app_name);
        auto to = create_data_app_package_path(to_uuid, data_app_name);
        auto to_parent = create_data_app_path(to_uuid);

        int rc = copy_directory_recursive(from.c_str(), to_parent.c_str());
        if (rc != 0) {
            res = error(rc, "Failed copying " + from + " to " + to);
            goto fail;
        }

        if (selinux_android_restorecon(to.c_str(), SELINUX_ANDROID_RESTORECON_RECURSE) != 0) {
            res = error("Failed to restorecon " + to);
            goto fail;
        }
    }

    // Copy private data for all known users
    for (auto user : users) {

        // Data source may not exist for all users; that's okay
        auto from_ce = create_data_user_ce_package_path(from_uuid, user, package_name);
        if (access(from_ce.c_str(), F_OK) != 0) {
            LOG(INFO) << "Missing source " << from_ce;
            continue;
        }

        if (!createAppData(toUuid, packageName, user, FLAG_STORAGE_CE | FLAG_STORAGE_DE, appId,
                seInfo, targetSdkVersion, nullptr).isOk()) {
            res = error("Failed to create package target");
            goto fail;
        }

        {
            auto from = create_data_user_de_package_path(from_uuid, user, package_name);
            auto to = create_data_user_de_path(to_uuid, user);

            int rc = copy_directory_recursive(from.c_str(), to.c_str());
            if (rc != 0) {
                res = error(rc, "Failed copying " + from + " to " + to);
                goto fail;
            }
        }
        {
            auto from = create_data_user_ce_package_path(from_uuid, user, package_name);
            auto to = create_data_user_ce_path(to_uuid, user);

            int rc = copy_directory_recursive(from.c_str(), to.c_str());
            if (rc != 0) {
                res = error(rc, "Failed copying " + from + " to " + to);
                goto fail;
            }
        }

        if (!restoreconAppData(toUuid, packageName, user, FLAG_STORAGE_CE | FLAG_STORAGE_DE,
                appId, seInfo).isOk()) {
            res = error("Failed to restorecon");
            goto fail;
        }
    }

    // We let the framework scan the new location and persist that before
    // deleting the data in the old location; this ordering ensures that
    // we can recover from things like battery pulls.
    return ok();

fail:
    // Nuke everything we might have already copied
    {
        auto to = create_data_app_package_path(to_uuid, data_app_name);
        if (delete_dir_contents(to.c_str(), 1, nullptr) != 0) {
            LOG(WARNING) << "Failed to rollback " << to;
        }
    }
    for (auto user : users) {
        {
            auto to = create_data_user_de_package_path(to_uuid, user, package_name);
            if (delete_dir_contents(to.c_str(), 1, nullptr) != 0) {
                LOG(WARNING) << "Failed to rollback " << to;
            }
        }
        {
            auto to = create_data_user_ce_package_path(to_uuid, user, package_name);
            if (delete_dir_contents(to.c_str(), 1, nullptr) != 0) {
                LOG(WARNING) << "Failed to rollback " << to;
            }
        }
    }
    return res;
}

binder::Status InstalldNativeService::createUserData(const std::unique_ptr<std::string>& uuid,
        int32_t userId, int32_t userSerial ATTRIBUTE_UNUSED, int32_t flags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    if (flags & FLAG_STORAGE_DE) {
        if (uuid_ == nullptr) {
            if (ensure_config_user_dirs(userId) != 0) {
                return error(StringPrintf("Failed to ensure dirs for %d", userId));
            }
        }
    }

    return ok();
}

binder::Status InstalldNativeService::destroyUserData(const std::unique_ptr<std::string>& uuid,
        int32_t userId, int32_t flags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    binder::Status res = ok();
    if (flags & FLAG_STORAGE_DE) {
        auto path = create_data_user_de_path(uuid_, userId);
        if (delete_dir_contents_and_dir(path, true) != 0) {
            res = error("Failed to delete " + path);
        }
        if (uuid_ == nullptr) {
            path = create_data_misc_legacy_path(userId);
            if (delete_dir_contents_and_dir(path, true) != 0) {
                res = error("Failed to delete " + path);
            }
            path = create_primary_cur_profile_dir_path(userId);
            if (delete_dir_contents_and_dir(path, true) != 0) {
                res = error("Failed to delete " + path);
            }
        }
    }
    if (flags & FLAG_STORAGE_CE) {
        auto path = create_data_user_ce_path(uuid_, userId);
        if (delete_dir_contents_and_dir(path, true) != 0) {
            res = error("Failed to delete " + path);
        }
        path = findDataMediaPath(uuid, userId);
        if (delete_dir_contents_and_dir(path, true) != 0) {
            res = error("Failed to delete " + path);
        }
    }
    return res;
}

binder::Status InstalldNativeService::freeCache(const std::unique_ptr<std::string>& uuid,
        int64_t targetFreeBytes, int64_t cacheReservedBytes, int32_t flags) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    auto uuidString = uuid ? *uuid : "";
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    auto data_path = create_data_path(uuid_);
    auto noop = (flags & FLAG_FREE_CACHE_NOOP);

    int64_t free = data_disk_free(data_path);
    if (free < 0) {
        return error("Failed to determine free space for " + data_path);
    }

    int64_t cleared = 0;
    int64_t needed = targetFreeBytes - free;
    LOG(DEBUG) << "Device " << data_path << " has " << free << " free; requested "
            << targetFreeBytes << "; needed " << needed;

    if (free >= targetFreeBytes) {
        return ok();
    }

    if (flags & FLAG_FREE_CACHE_V2) {
        // This new cache strategy fairly removes files from UIDs by deleting
        // files from the UIDs which are most over their allocated quota

        // 1. Create trackers for every known UID
        ATRACE_BEGIN("create");
        std::unordered_map<uid_t, std::shared_ptr<CacheTracker>> trackers;
        for (auto user : get_known_users(uuid_)) {
            FTS *fts;
            FTSENT *p;
            auto ce_path = create_data_user_ce_path(uuid_, user);
            auto de_path = create_data_user_de_path(uuid_, user);
            auto media_path = findDataMediaPath(uuid, user) + "/Android/data/";
            char *argv[] = { (char*) ce_path.c_str(), (char*) de_path.c_str(),
                    (char*) media_path.c_str(), nullptr };
            if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
                return error("Failed to fts_open");
            }
            while ((p = fts_read(fts)) != nullptr) {
                if (p->fts_info == FTS_D && p->fts_level == 1) {
                    uid_t uid = p->fts_statp->st_uid;
                    if (multiuser_get_app_id(uid) == AID_MEDIA_RW) {
                        uid = (multiuser_get_app_id(p->fts_statp->st_gid) - AID_EXT_GID_START)
                                + AID_APP_START;
                    }
                    auto search = trackers.find(uid);
                    if (search != trackers.end()) {
                        search->second->addDataPath(p->fts_path);
                    } else {
                        auto tracker = std::shared_ptr<CacheTracker>(new CacheTracker(
                                multiuser_get_user_id(uid), multiuser_get_app_id(uid), uuidString));
                        tracker->addDataPath(p->fts_path);
                        {
                            std::lock_guard<std::recursive_mutex> lock(mQuotasLock);
                            tracker->cacheQuota = mCacheQuotas[uid];
                        }
                        if (tracker->cacheQuota == 0) {
#if MEASURE_DEBUG
                            LOG(WARNING) << "UID " << uid << " has no cache quota; assuming 64MB";
#endif
                            tracker->cacheQuota = 67108864;
                        }
                        trackers[uid] = tracker;
                    }
                    fts_set(fts, p, FTS_SKIP);
                }
            }
            fts_close(fts);
        }
        ATRACE_END();

        // 2. Populate tracker stats and insert into priority queue
        ATRACE_BEGIN("populate");
        int64_t cacheTotal = 0;
        auto cmp = [](std::shared_ptr<CacheTracker> left, std::shared_ptr<CacheTracker> right) {
            return (left->getCacheRatio() < right->getCacheRatio());
        };
        std::priority_queue<std::shared_ptr<CacheTracker>,
                std::vector<std::shared_ptr<CacheTracker>>, decltype(cmp)> queue(cmp);
        for (const auto& it : trackers) {
            it.second->loadStats();
            queue.push(it.second);
            cacheTotal += it.second->cacheUsed;
        }
        ATRACE_END();

        // 3. Bounce across the queue, freeing items from whichever tracker is
        // the most over their assigned quota
        ATRACE_BEGIN("bounce");
        std::shared_ptr<CacheTracker> active;
        while (active || !queue.empty()) {
            // Only look at apps under quota when explicitly requested
            if (active && (active->getCacheRatio() < 10000)
                    && !(flags & FLAG_FREE_CACHE_V2_DEFY_QUOTA)) {
                LOG(DEBUG) << "Active ratio " << active->getCacheRatio()
                        << " isn't over quota, and defy not requested";
                break;
            }

            // Only keep clearing when we haven't pushed into reserved area
            if (cacheReservedBytes > 0 && cleared >= (cacheTotal - cacheReservedBytes)) {
                LOG(DEBUG) << "Refusing to clear cached data in reserved space";
                break;
            }

            // Find the best tracker to work with; this might involve swapping
            // if the active tracker is no longer the most over quota
            bool nextBetter = active && !queue.empty()
                    && active->getCacheRatio() < queue.top()->getCacheRatio();
            if (!active || nextBetter) {
                if (active) {
                    // Current tracker still has items, so we'll consider it
                    // again later once it bubbles up to surface
                    queue.push(active);
                }
                active = queue.top(); queue.pop();
                active->ensureItems();
                continue;
            }

            // If no items remain, go find another tracker
            if (active->items.empty()) {
                active = nullptr;
                continue;
            } else {
                auto item = active->items.back();
                active->items.pop_back();

                LOG(DEBUG) << "Purging " << item->toString() << " from " << active->toString();
                if (!noop) {
                    item->purge();
                }
                active->cacheUsed -= item->size;
                needed -= item->size;
                cleared += item->size;
            }

            // Verify that we're actually done before bailing, since sneaky
            // apps might be using hardlinks
            if (needed <= 0) {
                free = data_disk_free(data_path);
                needed = targetFreeBytes - free;
                if (needed <= 0) {
                    break;
                } else {
                    LOG(WARNING) << "Expected to be done but still need " << needed;
                }
            }
        }
        ATRACE_END();

    } else {
        return error("Legacy cache logic no longer supported");
    }

    free = data_disk_free(data_path);
    if (free >= targetFreeBytes) {
        return ok();
    } else {
        return error(StringPrintf("Failed to free up %" PRId64 " on %s; final free space %" PRId64,
                targetFreeBytes, data_path.c_str(), free));
    }
}

binder::Status InstalldNativeService::rmdex(const std::string& codePath,
        const std::string& instructionSet) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(codePath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    char dex_path[PKG_PATH_MAX];

    const char* path = codePath.c_str();
    const char* instruction_set = instructionSet.c_str();

    if (validate_apk_path(path) && validate_system_app_path(path)) {
        return error("Invalid path " + codePath);
    }

    if (!create_cache_path(dex_path, path, instruction_set)) {
        return error("Failed to create cache path for " + codePath);
    }

    ALOGV("unlink %s\n", dex_path);
    if (unlink(dex_path) < 0) {
        // It's ok if we don't have a dalvik cache path. Report error only when the path exists
        // but could not be unlinked.
        if (errno != ENOENT) {
            return error(StringPrintf("Failed to unlink %s", dex_path));
        }
    }
    return ok();
}

struct stats {
    int64_t codeSize;
    int64_t dataSize;
    int64_t cacheSize;
};

#if MEASURE_DEBUG
static std::string toString(std::vector<int64_t> values) {
    std::stringstream res;
    res << "[";
    for (size_t i = 0; i < values.size(); i++) {
        res << values[i];
        if (i < values.size() - 1) {
            res << ",";
        }
    }
    res << "]";
    return res.str();
}
#endif

static void collectQuotaStats(const std::string& uuid, int32_t userId,
        int32_t appId, struct stats* stats, struct stats* extStats) {
    int64_t space;
    if (stats != nullptr) {
        uid_t uid = multiuser_get_uid(userId, appId);
        if ((space = GetOccupiedSpaceForUid(uuid, uid)) != -1) {
            stats->dataSize += space;
        }

        int cacheGid = multiuser_get_cache_gid(userId, appId);
        if (cacheGid != -1) {
            if ((space = GetOccupiedSpaceForGid(uuid, cacheGid)) != -1) {
                stats->cacheSize += space;
            }
        }

        int sharedGid = multiuser_get_shared_gid(0, appId);
        if (sharedGid != -1) {
            if ((space = GetOccupiedSpaceForGid(uuid, sharedGid)) != -1) {
                stats->codeSize += space;
            }
        }
    }

    if (extStats != nullptr) {
        int extGid = multiuser_get_ext_gid(userId, appId);
        if (extGid != -1) {
            if ((space = GetOccupiedSpaceForGid(uuid, extGid)) != -1) {
                extStats->dataSize += space;
            }
        }

        int extCacheGid = multiuser_get_ext_cache_gid(userId, appId);
        if (extCacheGid != -1) {
            if ((space = GetOccupiedSpaceForGid(uuid, extCacheGid)) != -1) {
                extStats->dataSize += space;
                extStats->cacheSize += space;
            }
        }
    }
}

static void collectManualStats(const std::string& path, struct stats* stats) {
    DIR *d;
    int dfd;
    struct dirent *de;
    struct stat s;

    d = opendir(path.c_str());
    if (d == nullptr) {
        if (errno != ENOENT) {
            PLOG(WARNING) << "Failed to open " << path;
        }
        return;
    }
    dfd = dirfd(d);
    while ((de = readdir(d))) {
        const char *name = de->d_name;

        int64_t size = 0;
        if (fstatat(dfd, name, &s, AT_SYMLINK_NOFOLLOW) == 0) {
            size = s.st_blocks * 512;
        }

        if (de->d_type == DT_DIR) {
            if (!strcmp(name, ".")) {
                // Don't recurse, but still count node size
            } else if (!strcmp(name, "..")) {
                // Don't recurse or count node size
                continue;
            } else {
                // Measure all children nodes
                size = 0;
                calculate_tree_size(StringPrintf("%s/%s", path.c_str(), name), &size);
            }

            if (!strcmp(name, "cache") || !strcmp(name, "code_cache")) {
                stats->cacheSize += size;
            }
        }

        // Legacy symlink isn't owned by app
        if (de->d_type == DT_LNK && !strcmp(name, "lib")) {
            continue;
        }

        // Everything found inside is considered data
        stats->dataSize += size;
    }
    closedir(d);
}

static void collectManualStatsForUser(const std::string& path, struct stats* stats,
        bool exclude_apps = false) {
    DIR *d;
    int dfd;
    struct dirent *de;
    struct stat s;

    d = opendir(path.c_str());
    if (d == nullptr) {
        if (errno != ENOENT) {
            PLOG(WARNING) << "Failed to open " << path;
        }
        return;
    }
    dfd = dirfd(d);
    while ((de = readdir(d))) {
        if (de->d_type == DT_DIR) {
            const char *name = de->d_name;
            if (fstatat(dfd, name, &s, AT_SYMLINK_NOFOLLOW) != 0) {
                continue;
            }
            int32_t user_uid = multiuser_get_app_id(s.st_uid);
            if (!strcmp(name, ".") || !strcmp(name, "..")) {
                continue;
            } else if (exclude_apps && (user_uid >= AID_APP_START && user_uid <= AID_APP_END)) {
                continue;
            } else {
                collectManualStats(StringPrintf("%s/%s", path.c_str(), name), stats);
            }
        }
    }
    closedir(d);
}

static void collectManualExternalStatsForUser(const std::string& path, struct stats* stats) {
    FTS *fts;
    FTSENT *p;
    char *argv[] = { (char*) path.c_str(), nullptr };
    if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
        PLOG(ERROR) << "Failed to fts_open " << path;
        return;
    }
    while ((p = fts_read(fts)) != nullptr) {
        p->fts_number = p->fts_parent->fts_number;
        switch (p->fts_info) {
        case FTS_D:
            if (p->fts_level == 4
                    && !strcmp(p->fts_name, "cache")
                    && !strcmp(p->fts_parent->fts_parent->fts_name, "data")
                    && !strcmp(p->fts_parent->fts_parent->fts_parent->fts_name, "Android")) {
                p->fts_number = 1;
            }
            [[fallthrough]]; // to count the directory
        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
            int64_t size = (p->fts_statp->st_blocks * 512);
            if (p->fts_number == 1) {
                stats->cacheSize += size;
            }
            stats->dataSize += size;
            break;
        }
    }
    fts_close(fts);
}

binder::Status InstalldNativeService::getAppSize(const std::unique_ptr<std::string>& uuid,
        const std::vector<std::string>& packageNames, int32_t userId, int32_t flags,
        int32_t appId, const std::vector<int64_t>& ceDataInodes,
        const std::vector<std::string>& codePaths, std::vector<int64_t>* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    for (const auto& packageName : packageNames) {
        CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    }
    for (const auto& codePath : codePaths) {
        CHECK_ARGUMENT_PATH(codePath);
    }
    // NOTE: Locking is relaxed on this method, since it's limited to
    // read-only measurements without mutation.

    // When modifying this logic, always verify using tests:
    // runtest -x frameworks/base/services/tests/servicestests/src/com/android/server/pm/InstallerTest.java -m testGetAppSize

#if MEASURE_DEBUG
    LOG(INFO) << "Measuring user " << userId << " app " << appId;
#endif

    // Here's a summary of the common storage locations across the platform,
    // and how they're each tagged:
    //
    // /data/app/com.example                           UID system
    // /data/app/com.example/oat                       UID system
    // /data/user/0/com.example                        UID u0_a10      GID u0_a10
    // /data/user/0/com.example/cache                  UID u0_a10      GID u0_a10_cache
    // /data/media/0/foo.txt                           UID u0_media_rw
    // /data/media/0/bar.jpg                           UID u0_media_rw GID u0_media_image
    // /data/media/0/Android/data/com.example          UID u0_media_rw GID u0_a10_ext
    // /data/media/0/Android/data/com.example/cache    UID u0_media_rw GID u0_a10_ext_cache
    // /data/media/obb/com.example                     UID system

    struct stats stats;
    struct stats extStats;
    memset(&stats, 0, sizeof(stats));
    memset(&extStats, 0, sizeof(extStats));

    auto uuidString = uuid ? *uuid : "";
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;

    if (!IsQuotaSupported(uuidString)) {
        flags &= ~FLAG_USE_QUOTA;
    }

    ATRACE_BEGIN("obb");
    for (const auto& packageName : packageNames) {
        auto obbCodePath = create_data_media_package_path(uuid_, userId,
                "obb", packageName.c_str());
        calculate_tree_size(obbCodePath, &extStats.codeSize);
    }
    ATRACE_END();

    if (flags & FLAG_USE_QUOTA && appId >= AID_APP_START) {
        ATRACE_BEGIN("code");
        for (const auto& codePath : codePaths) {
            calculate_tree_size(codePath, &stats.codeSize, -1,
                    multiuser_get_shared_gid(0, appId));
        }
        ATRACE_END();

        ATRACE_BEGIN("quota");
        collectQuotaStats(uuidString, userId, appId, &stats, &extStats);
        ATRACE_END();
    } else {
        ATRACE_BEGIN("code");
        for (const auto& codePath : codePaths) {
            calculate_tree_size(codePath, &stats.codeSize);
        }
        ATRACE_END();

        for (size_t i = 0; i < packageNames.size(); i++) {
            const char* pkgname = packageNames[i].c_str();

            ATRACE_BEGIN("data");
            auto cePath = create_data_user_ce_package_path(uuid_, userId, pkgname, ceDataInodes[i]);
            collectManualStats(cePath, &stats);
            auto dePath = create_data_user_de_package_path(uuid_, userId, pkgname);
            collectManualStats(dePath, &stats);
            ATRACE_END();

            if (!uuid) {
                ATRACE_BEGIN("profiles");
                calculate_tree_size(
                        create_primary_current_profile_package_dir_path(userId, pkgname),
                        &stats.dataSize);
                calculate_tree_size(
                        create_primary_reference_profile_package_dir_path(pkgname),
                        &stats.codeSize);
                ATRACE_END();
            }

            ATRACE_BEGIN("external");
            auto extPath = create_data_media_package_path(uuid_, userId, "data", pkgname);
            collectManualStats(extPath, &extStats);
            auto mediaPath = create_data_media_package_path(uuid_, userId, "media", pkgname);
            calculate_tree_size(mediaPath, &extStats.dataSize);
            ATRACE_END();
        }

        if (!uuid) {
            ATRACE_BEGIN("dalvik");
            int32_t sharedGid = multiuser_get_shared_gid(0, appId);
            if (sharedGid != -1) {
                calculate_tree_size(create_data_dalvik_cache_path(), &stats.codeSize,
                        sharedGid, -1);
            }
            ATRACE_END();
        }
    }

    std::vector<int64_t> ret;
    ret.push_back(stats.codeSize);
    ret.push_back(stats.dataSize);
    ret.push_back(stats.cacheSize);
    ret.push_back(extStats.codeSize);
    ret.push_back(extStats.dataSize);
    ret.push_back(extStats.cacheSize);
#if MEASURE_DEBUG
    LOG(DEBUG) << "Final result " << toString(ret);
#endif
    *_aidl_return = ret;
    return ok();
}

binder::Status InstalldNativeService::getUserSize(const std::unique_ptr<std::string>& uuid,
        int32_t userId, int32_t flags, const std::vector<int32_t>& appIds,
        std::vector<int64_t>* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    // NOTE: Locking is relaxed on this method, since it's limited to
    // read-only measurements without mutation.

    // When modifying this logic, always verify using tests:
    // runtest -x frameworks/base/services/tests/servicestests/src/com/android/server/pm/InstallerTest.java -m testGetUserSize

#if MEASURE_DEBUG
    LOG(INFO) << "Measuring user " << userId;
#endif

    struct stats stats;
    struct stats extStats;
    memset(&stats, 0, sizeof(stats));
    memset(&extStats, 0, sizeof(extStats));

    auto uuidString = uuid ? *uuid : "";
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;

    if (!IsQuotaSupported(uuidString)) {
        flags &= ~FLAG_USE_QUOTA;
    }

    if (flags & FLAG_USE_QUOTA) {
        int64_t space;

        ATRACE_BEGIN("obb");
        if ((space = GetOccupiedSpaceForGid(uuidString, AID_MEDIA_OBB)) != -1) {
            extStats.codeSize += space;
        }
        ATRACE_END();

        ATRACE_BEGIN("code");
        calculate_tree_size(create_data_app_path(uuid_), &stats.codeSize, -1, -1, true);
        ATRACE_END();

        ATRACE_BEGIN("data");
        auto cePath = create_data_user_ce_path(uuid_, userId);
        collectManualStatsForUser(cePath, &stats, true);
        auto dePath = create_data_user_de_path(uuid_, userId);
        collectManualStatsForUser(dePath, &stats, true);
        ATRACE_END();

        if (!uuid) {
            ATRACE_BEGIN("profile");
            auto userProfilePath = create_primary_cur_profile_dir_path(userId);
            calculate_tree_size(userProfilePath, &stats.dataSize, -1, -1, true);
            auto refProfilePath = create_primary_ref_profile_dir_path();
            calculate_tree_size(refProfilePath, &stats.codeSize, -1, -1, true);
            ATRACE_END();
        }

        ATRACE_BEGIN("external");
        uid_t uid = multiuser_get_uid(userId, AID_MEDIA_RW);
        if ((space = GetOccupiedSpaceForUid(uuidString, uid)) != -1) {
            extStats.dataSize += space;
        }
        ATRACE_END();

        if (!uuid) {
            ATRACE_BEGIN("dalvik");
            calculate_tree_size(create_data_dalvik_cache_path(), &stats.codeSize,
                    -1, -1, true);
            calculate_tree_size(create_primary_cur_profile_dir_path(userId), &stats.dataSize,
                    -1, -1, true);
            ATRACE_END();
        }

        ATRACE_BEGIN("quota");
        int64_t dataSize = extStats.dataSize;
        for (auto appId : appIds) {
            if (appId >= AID_APP_START) {
                collectQuotaStats(uuidString, userId, appId, &stats, &extStats);

#if MEASURE_DEBUG
                // Sleep to make sure we don't lose logs
                usleep(1);
#endif
            }
        }
        extStats.dataSize = dataSize;
        ATRACE_END();
    } else {
        ATRACE_BEGIN("obb");
        auto obbPath = create_data_path(uuid_) + "/media/obb";
        calculate_tree_size(obbPath, &extStats.codeSize);
        ATRACE_END();

        ATRACE_BEGIN("code");
        calculate_tree_size(create_data_app_path(uuid_), &stats.codeSize);
        ATRACE_END();

        ATRACE_BEGIN("data");
        auto cePath = create_data_user_ce_path(uuid_, userId);
        collectManualStatsForUser(cePath, &stats);
        auto dePath = create_data_user_de_path(uuid_, userId);
        collectManualStatsForUser(dePath, &stats);
        ATRACE_END();

        if (!uuid) {
            ATRACE_BEGIN("profile");
            auto userProfilePath = create_primary_cur_profile_dir_path(userId);
            calculate_tree_size(userProfilePath, &stats.dataSize);
            auto refProfilePath = create_primary_ref_profile_dir_path();
            calculate_tree_size(refProfilePath, &stats.codeSize);
            ATRACE_END();
        }

        ATRACE_BEGIN("external");
        auto dataMediaPath = create_data_media_path(uuid_, userId);
        collectManualExternalStatsForUser(dataMediaPath, &extStats);
#if MEASURE_DEBUG
        LOG(DEBUG) << "Measured external data " << extStats.dataSize << " cache "
                << extStats.cacheSize;
#endif
        ATRACE_END();

        if (!uuid) {
            ATRACE_BEGIN("dalvik");
            calculate_tree_size(create_data_dalvik_cache_path(), &stats.codeSize);
            calculate_tree_size(create_primary_cur_profile_dir_path(userId), &stats.dataSize);
            ATRACE_END();
        }
    }

    std::vector<int64_t> ret;
    ret.push_back(stats.codeSize);
    ret.push_back(stats.dataSize);
    ret.push_back(stats.cacheSize);
    ret.push_back(extStats.codeSize);
    ret.push_back(extStats.dataSize);
    ret.push_back(extStats.cacheSize);
#if MEASURE_DEBUG
    LOG(DEBUG) << "Final result " << toString(ret);
#endif
    *_aidl_return = ret;
    return ok();
}

binder::Status InstalldNativeService::getExternalSize(const std::unique_ptr<std::string>& uuid,
        int32_t userId, int32_t flags, const std::vector<int32_t>& appIds,
        std::vector<int64_t>* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    // NOTE: Locking is relaxed on this method, since it's limited to
    // read-only measurements without mutation.

    // When modifying this logic, always verify using tests:
    // runtest -x frameworks/base/services/tests/servicestests/src/com/android/server/pm/InstallerTest.java -m testGetExternalSize

#if MEASURE_DEBUG
    LOG(INFO) << "Measuring external " << userId;
#endif

    auto uuidString = uuid ? *uuid : "";
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;

    int64_t totalSize = 0;
    int64_t audioSize = 0;
    int64_t videoSize = 0;
    int64_t imageSize = 0;
    int64_t appSize = 0;
    int64_t obbSize = 0;

    if (!IsQuotaSupported(uuidString)) {
        flags &= ~FLAG_USE_QUOTA;
    }

    if (flags & FLAG_USE_QUOTA) {
        int64_t space;

        ATRACE_BEGIN("quota");
        uid_t uid = multiuser_get_uid(userId, AID_MEDIA_RW);
        if ((space = GetOccupiedSpaceForUid(uuidString, uid)) != -1) {
            totalSize = space;
        }

        gid_t audioGid = multiuser_get_uid(userId, AID_MEDIA_AUDIO);
        if ((space = GetOccupiedSpaceForGid(uuidString, audioGid)) != -1) {
            audioSize = space;
        }
        gid_t videoGid = multiuser_get_uid(userId, AID_MEDIA_VIDEO);
        if ((space = GetOccupiedSpaceForGid(uuidString, videoGid)) != -1) {
            videoSize = space;
        }
        gid_t imageGid = multiuser_get_uid(userId, AID_MEDIA_IMAGE);
        if ((space = GetOccupiedSpaceForGid(uuidString, imageGid)) != -1) {
            imageSize = space;
        }
        if ((space = GetOccupiedSpaceForGid(uuidString, AID_MEDIA_OBB)) != -1) {
            obbSize = space;
        }
        ATRACE_END();

        ATRACE_BEGIN("apps");
        struct stats extStats;
        memset(&extStats, 0, sizeof(extStats));
        for (auto appId : appIds) {
            if (appId >= AID_APP_START) {
                collectQuotaStats(uuidString, userId, appId, nullptr, &extStats);
            }
        }
        appSize = extStats.dataSize;
        ATRACE_END();
    } else {
        ATRACE_BEGIN("manual");
        FTS *fts;
        FTSENT *p;
        auto path = create_data_media_path(uuid_, userId);
        char *argv[] = { (char*) path.c_str(), nullptr };
        if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
            return error("Failed to fts_open " + path);
        }
        while ((p = fts_read(fts)) != nullptr) {
            char* ext;
            int64_t size = (p->fts_statp->st_blocks * 512);
            switch (p->fts_info) {
            case FTS_F:
                // Only categorize files not belonging to apps
                if (p->fts_parent->fts_number == 0) {
                    ext = strrchr(p->fts_name, '.');
                    if (ext != nullptr) {
                        switch (MatchExtension(++ext)) {
                        case AID_MEDIA_AUDIO: audioSize += size; break;
                        case AID_MEDIA_VIDEO: videoSize += size; break;
                        case AID_MEDIA_IMAGE: imageSize += size; break;
                        }
                    }
                }
                [[fallthrough]]; // always count against total
            case FTS_D:
                // Ignore data belonging to specific apps
                p->fts_number = p->fts_parent->fts_number;
                if (p->fts_level == 1 && !strcmp(p->fts_name, "Android")) {
                    p->fts_number = 1;
                }
                [[fallthrough]]; // always count against total
            case FTS_DEFAULT:
            case FTS_SL:
            case FTS_SLNONE:
                if (p->fts_parent->fts_number == 1) {
                    appSize += size;
                }
                totalSize += size;
                break;
            }
        }
        fts_close(fts);
        ATRACE_END();

        ATRACE_BEGIN("obb");
        auto obbPath = StringPrintf("%s/Android/obb",
                create_data_media_path(uuid_, userId).c_str());
        calculate_tree_size(obbPath, &obbSize);
        ATRACE_END();
    }

    std::vector<int64_t> ret;
    ret.push_back(totalSize);
    ret.push_back(audioSize);
    ret.push_back(videoSize);
    ret.push_back(imageSize);
    ret.push_back(appSize);
    ret.push_back(obbSize);
#if MEASURE_DEBUG
    LOG(DEBUG) << "Final result " << toString(ret);
#endif
    *_aidl_return = ret;
    return ok();
}

binder::Status InstalldNativeService::setAppQuota(const std::unique_ptr<std::string>& uuid,
        int32_t userId, int32_t appId, int64_t cacheQuota) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    std::lock_guard<std::recursive_mutex> lock(mQuotasLock);

    int32_t uid = multiuser_get_uid(userId, appId);
    mCacheQuotas[uid] = cacheQuota;

    return ok();
}

// Dumps the contents of a profile file, using pkgname's dex files for pretty
// printing the result.
binder::Status InstalldNativeService::dumpProfiles(int32_t uid, const std::string& packageName,
        const std::string& profileName, const std::string& codePath, bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    CHECK_ARGUMENT_PATH(codePath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    *_aidl_return = dump_profiles(uid, packageName, profileName, codePath);
    return ok();
}

// Copy the contents of a system profile over the data profile.
binder::Status InstalldNativeService::copySystemProfile(const std::string& systemProfile,
        int32_t packageUid, const std::string& packageName, const std::string& profileName,
        bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);
    *_aidl_return = copy_system_profile(systemProfile, packageUid, packageName, profileName);
    return ok();
}

// TODO: Consider returning error codes.
binder::Status InstalldNativeService::mergeProfiles(int32_t uid, const std::string& packageName,
        const std::string& profileName, bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    *_aidl_return = analyze_primary_profiles(uid, packageName, profileName);
    return ok();
}

binder::Status InstalldNativeService::createProfileSnapshot(int32_t appId,
        const std::string& packageName, const std::string& profileName,
        const std::string& classpath, bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    *_aidl_return = create_profile_snapshot(appId, packageName, profileName, classpath);
    return ok();
}

binder::Status InstalldNativeService::destroyProfileSnapshot(const std::string& packageName,
        const std::string& profileName) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    std::string snapshot = create_snapshot_profile_path(packageName, profileName);
    if ((unlink(snapshot.c_str()) != 0) && (errno != ENOENT)) {
        return error("Failed to destroy profile snapshot for " + packageName + ":" + profileName);
    }
    return ok();
}

static const char* getCStr(const std::unique_ptr<std::string>& data,
        const char* default_value = nullptr) {
    return data == nullptr ? default_value : data->c_str();
}
binder::Status InstalldNativeService::dexopt(const std::string& apkPath, int32_t uid,
        const std::unique_ptr<std::string>& packageName, const std::string& instructionSet,
        int32_t dexoptNeeded, const std::unique_ptr<std::string>& outputPath, int32_t dexFlags,
        const std::string& compilerFilter, const std::unique_ptr<std::string>& uuid,
        const std::unique_ptr<std::string>& classLoaderContext,
        const std::unique_ptr<std::string>& seInfo, bool downgrade, int32_t targetSdkVersion,
        const std::unique_ptr<std::string>& profileName,
        const std::unique_ptr<std::string>& dexMetadataPath,
        const std::unique_ptr<std::string>& compilationReason) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PATH(apkPath);
    if (packageName && *packageName != "*") {
        CHECK_ARGUMENT_PACKAGE_NAME(*packageName);
    }
    CHECK_ARGUMENT_PATH(outputPath);
    CHECK_ARGUMENT_PATH(dexMetadataPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* apk_path = apkPath.c_str();
    const char* pkgname = getCStr(packageName, "*");
    const char* instruction_set = instructionSet.c_str();
    const char* oat_dir = getCStr(outputPath);
    const char* compiler_filter = compilerFilter.c_str();
    const char* volume_uuid = getCStr(uuid);
    const char* class_loader_context = getCStr(classLoaderContext);
    const char* se_info = getCStr(seInfo);
    const char* profile_name = getCStr(profileName);
    const char* dm_path = getCStr(dexMetadataPath);
    const char* compilation_reason = getCStr(compilationReason);
    std::string error_msg;
    int res = android::installd::dexopt(apk_path, uid, pkgname, instruction_set, dexoptNeeded,
            oat_dir, dexFlags, compiler_filter, volume_uuid, class_loader_context, se_info,
            downgrade, targetSdkVersion, profile_name, dm_path, compilation_reason, &error_msg);
    return res ? error(res, error_msg) : ok();
}

binder::Status InstalldNativeService::compileLayouts(const std::string& apkPath,
                                                     const std::string& packageName,
                                                     const std ::string& outDexFile, int uid,
                                                     bool* _aidl_return) {
    const char* apk_path = apkPath.c_str();
    const char* package_name = packageName.c_str();
    const char* out_dex_file = outDexFile.c_str();
    *_aidl_return = android::installd::view_compiler(apk_path, package_name, out_dex_file, uid);
    return *_aidl_return ? ok() : error("viewcompiler failed");
}

binder::Status InstalldNativeService::linkNativeLibraryDirectory(
        const std::unique_ptr<std::string>& uuid, const std::string& packageName,
        const std::string& nativeLibPath32, int32_t userId) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    CHECK_ARGUMENT_PATH(nativeLibPath32);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgname = packageName.c_str();
    const char* asecLibDir = nativeLibPath32.c_str();
    struct stat s, libStat;
    binder::Status res = ok();

    auto _pkgdir = create_data_user_ce_package_path(uuid_, userId, pkgname);
    auto _libsymlink = _pkgdir + PKG_LIB_POSTFIX;

    const char* pkgdir = _pkgdir.c_str();
    const char* libsymlink = _libsymlink.c_str();

    if (stat(pkgdir, &s) < 0) {
        return error("Failed to stat " + _pkgdir);
    }

    char *con = nullptr;
    if (lgetfilecon(pkgdir, &con) < 0) {
        return error("Failed to lgetfilecon " + _pkgdir);
    }

    if (chown(pkgdir, AID_INSTALL, AID_INSTALL) < 0) {
        res = error("Failed to chown " + _pkgdir);
        goto out;
    }

    if (chmod(pkgdir, 0700) < 0) {
        res = error("Failed to chmod " + _pkgdir);
        goto out;
    }

    if (lstat(libsymlink, &libStat) < 0) {
        if (errno != ENOENT) {
            res = error("Failed to stat " + _libsymlink);
            goto out;
        }
    } else {
        if (S_ISDIR(libStat.st_mode)) {
            if (delete_dir_contents(libsymlink, 1, nullptr) < 0) {
                res = error("Failed to delete " + _libsymlink);
                goto out;
            }
        } else if (S_ISLNK(libStat.st_mode)) {
            if (unlink(libsymlink) < 0) {
                res = error("Failed to unlink " + _libsymlink);
                goto out;
            }
        }
    }

    if (symlink(asecLibDir, libsymlink) < 0) {
        res = error("Failed to symlink " + _libsymlink + " to " + nativeLibPath32);
        goto out;
    }

    if (lsetfilecon(libsymlink, con) < 0) {
        res = error("Failed to lsetfilecon " + _libsymlink);
        goto out;
    }

out:
    free(con);
    if (chmod(pkgdir, s.st_mode) < 0) {
        auto msg = "Failed to cleanup chmod " + _pkgdir;
        if (res.isOk()) {
            res = error(msg);
        } else {
            PLOG(ERROR) << msg;
        }
    }

    if (chown(pkgdir, s.st_uid, s.st_gid) < 0) {
        auto msg = "Failed to cleanup chown " + _pkgdir;
        if (res.isOk()) {
            res = error(msg);
        } else {
            PLOG(ERROR) << msg;
        }
    }

    return res;
}

static void run_idmap(const char *target_apk, const char *overlay_apk, int idmap_fd)
{
    execl(kIdMapPath, kIdMapPath, "--fd", target_apk, overlay_apk,
            StringPrintf("%d", idmap_fd).c_str(), (char*)nullptr);
    PLOG(ERROR) << "execl (" << kIdMapPath << ") failed";
}

static void run_verify_idmap(const char *target_apk, const char *overlay_apk, int idmap_fd)
{
    execl(kIdMapPath, kIdMapPath, "--verify", target_apk, overlay_apk,
            StringPrintf("%d", idmap_fd).c_str(), (char*)nullptr);
    PLOG(ERROR) << "execl (" << kIdMapPath << ") failed";
}

static bool delete_stale_idmap(const char* target_apk, const char* overlay_apk,
        const char* idmap_path, int32_t uid) {
    int idmap_fd = open(idmap_path, O_RDWR);
    if (idmap_fd < 0) {
        PLOG(ERROR) << "idmap open failed: " << idmap_path;
        unlink(idmap_path);
        return true;
    }

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        if (setgid(uid) != 0) {
            LOG(ERROR) << "setgid(" << uid << ") failed during idmap";
            exit(1);
        }
        if (setuid(uid) != 0) {
            LOG(ERROR) << "setuid(" << uid << ") failed during idmap";
            exit(1);
        }
        if (flock(idmap_fd, LOCK_EX | LOCK_NB) != 0) {
            PLOG(ERROR) << "flock(" << idmap_path << ") failed during idmap";
            exit(1);
        }

        run_verify_idmap(target_apk, overlay_apk, idmap_fd);
        exit(1); /* only if exec call to deleting stale idmap failed */
    } else {
        int status = wait_child(pid);
        close(idmap_fd);

        if (status != 0) {
            // Failed on verifying if idmap is made from target_apk and overlay_apk.
            LOG(DEBUG) << "delete stale idmap: " << idmap_path;
            unlink(idmap_path);
            return true;
        }
    }
    return false;
}

// Transform string /a/b/c.apk to (prefix)/a@b@c.apk@(suffix)
// eg /a/b/c.apk to /data/resource-cache/a@b@c.apk@idmap
static int flatten_path(const char *prefix, const char *suffix,
        const char *overlay_path, char *idmap_path, size_t N)
{
    if (overlay_path == nullptr || idmap_path == nullptr) {
        return -1;
    }
    const size_t len_overlay_path = strlen(overlay_path);
    // will access overlay_path + 1 further below; requires absolute path
    if (len_overlay_path < 2 || *overlay_path != '/') {
        return -1;
    }
    const size_t len_idmap_root = strlen(prefix);
    const size_t len_suffix = strlen(suffix);
    if (SIZE_MAX - len_idmap_root < len_overlay_path ||
            SIZE_MAX - (len_idmap_root + len_overlay_path) < len_suffix) {
        // additions below would cause overflow
        return -1;
    }
    if (N < len_idmap_root + len_overlay_path + len_suffix) {
        return -1;
    }
    memset(idmap_path, 0, N);
    snprintf(idmap_path, N, "%s%s%s", prefix, overlay_path + 1, suffix);
    char *ch = idmap_path + len_idmap_root;
    while (*ch != '\0') {
        if (*ch == '/') {
            *ch = '@';
        }
        ++ch;
    }
    return 0;
}

binder::Status InstalldNativeService::idmap(const std::string& targetApkPath,
        const std::string& overlayApkPath, int32_t uid) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(targetApkPath);
    CHECK_ARGUMENT_PATH(overlayApkPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* target_apk = targetApkPath.c_str();
    const char* overlay_apk = overlayApkPath.c_str();
    ALOGV("idmap target_apk=%s overlay_apk=%s uid=%d\n", target_apk, overlay_apk, uid);

    int idmap_fd = -1;
    char idmap_path[PATH_MAX];
    struct stat idmap_stat;
    bool outdated = false;

    if (flatten_path(IDMAP_PREFIX, IDMAP_SUFFIX, overlay_apk,
                idmap_path, sizeof(idmap_path)) == -1) {
        ALOGE("idmap cannot generate idmap path for overlay %s\n", overlay_apk);
        goto fail;
    }

    if (stat(idmap_path, &idmap_stat) < 0) {
        outdated = true;
    } else {
        outdated = delete_stale_idmap(target_apk, overlay_apk, idmap_path, uid);
    }

    if (outdated) {
        idmap_fd = open(idmap_path, O_RDWR | O_CREAT | O_EXCL, 0644);
    } else {
        idmap_fd = open(idmap_path, O_RDWR);
    }

    if (idmap_fd < 0) {
        ALOGE("idmap cannot open '%s' for output: %s\n", idmap_path, strerror(errno));
        goto fail;
    }
    if (fchown(idmap_fd, AID_SYSTEM, uid) < 0) {
        ALOGE("idmap cannot chown '%s'\n", idmap_path);
        goto fail;
    }
    if (fchmod(idmap_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        ALOGE("idmap cannot chmod '%s'\n", idmap_path);
        goto fail;
    }

    if (!outdated) {
        close(idmap_fd);
        return ok();
    }

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        if (setgid(uid) != 0) {
            ALOGE("setgid(%d) failed during idmap\n", uid);
            exit(1);
        }
        if (setuid(uid) != 0) {
            ALOGE("setuid(%d) failed during idmap\n", uid);
            exit(1);
        }
        if (flock(idmap_fd, LOCK_EX | LOCK_NB) != 0) {
            ALOGE("flock(%s) failed during idmap: %s\n", idmap_path, strerror(errno));
            exit(1);
        }

        run_idmap(target_apk, overlay_apk, idmap_fd);
        exit(1); /* only if exec call to idmap failed */
    } else {
        int status = wait_child(pid);
        if (status != 0) {
            ALOGE("idmap failed, status=0x%04x\n", status);
            goto fail;
        }
    }

    close(idmap_fd);
    return ok();
fail:
    if (idmap_fd >= 0) {
        close(idmap_fd);
        unlink(idmap_path);
    }
    return error();
}

binder::Status InstalldNativeService::removeIdmap(const std::string& overlayApkPath) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(overlayApkPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* overlay_apk = overlayApkPath.c_str();
    char idmap_path[PATH_MAX];

    if (flatten_path(IDMAP_PREFIX, IDMAP_SUFFIX, overlay_apk,
                idmap_path, sizeof(idmap_path)) == -1) {
        ALOGE("idmap cannot generate idmap path for overlay %s\n", overlay_apk);
        return error();
    }
    if (unlink(idmap_path) < 0) {
        ALOGE("couldn't unlink idmap file %s\n", idmap_path);
        return error();
    }
    return ok();
}

binder::Status InstalldNativeService::restoreconAppData(const std::unique_ptr<std::string>& uuid,
        const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
        const std::string& seInfo) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(uuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    binder::Status res = ok();

    // SELINUX_ANDROID_RESTORECON_DATADATA flag is set by libselinux. Not needed here.
    unsigned int seflags = SELINUX_ANDROID_RESTORECON_RECURSE;
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    const char* pkgName = packageName.c_str();
    const char* seinfo = seInfo.c_str();

    uid_t uid = multiuser_get_uid(userId, appId);
    if (flags & FLAG_STORAGE_CE) {
        auto path = create_data_user_ce_package_path(uuid_, userId, pkgName);
        if (selinux_android_restorecon_pkgdir(path.c_str(), seinfo, uid, seflags) < 0) {
            res = error("restorecon failed for " + path);
        }
    }
    if (flags & FLAG_STORAGE_DE) {
        auto path = create_data_user_de_package_path(uuid_, userId, pkgName);
        if (selinux_android_restorecon_pkgdir(path.c_str(), seinfo, uid, seflags) < 0) {
            res = error("restorecon failed for " + path);
        }
    }
    return res;
}

binder::Status InstalldNativeService::createOatDir(const std::string& oatDir,
        const std::string& instructionSet) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(oatDir);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* oat_dir = oatDir.c_str();
    const char* instruction_set = instructionSet.c_str();
    char oat_instr_dir[PKG_PATH_MAX];

    if (validate_apk_path(oat_dir)) {
        return error("Invalid path " + oatDir);
    }
    if (fs_prepare_dir(oat_dir, S_IRWXU | S_IRWXG | S_IXOTH, AID_SYSTEM, AID_INSTALL)) {
        return error("Failed to prepare " + oatDir);
    }
    if (selinux_android_restorecon(oat_dir, 0)) {
        return error("Failed to restorecon " + oatDir);
    }
    snprintf(oat_instr_dir, PKG_PATH_MAX, "%s/%s", oat_dir, instruction_set);
    if (fs_prepare_dir(oat_instr_dir, S_IRWXU | S_IRWXG | S_IXOTH, AID_SYSTEM, AID_INSTALL)) {
        return error(StringPrintf("Failed to prepare %s", oat_instr_dir));
    }
    return ok();
}

binder::Status InstalldNativeService::rmPackageDir(const std::string& packageDir) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(packageDir);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    if (validate_apk_path(packageDir.c_str())) {
        return error("Invalid path " + packageDir);
    }
    if (rm_package_dir(packageDir) != 0) {
        return error("Failed to delete " + packageDir);
    }
    return ok();
}

binder::Status InstalldNativeService::linkFile(const std::string& relativePath,
        const std::string& fromBase, const std::string& toBase) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(fromBase);
    CHECK_ARGUMENT_PATH(toBase);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* relative_path = relativePath.c_str();
    const char* from_base = fromBase.c_str();
    const char* to_base = toBase.c_str();
    char from_path[PKG_PATH_MAX];
    char to_path[PKG_PATH_MAX];
    snprintf(from_path, PKG_PATH_MAX, "%s/%s", from_base, relative_path);
    snprintf(to_path, PKG_PATH_MAX, "%s/%s", to_base, relative_path);

    if (validate_apk_path_subdirs(from_path)) {
        return error(StringPrintf("Invalid from path %s", from_path));
    }

    if (validate_apk_path_subdirs(to_path)) {
        return error(StringPrintf("Invalid to path %s", to_path));
    }

    if (link(from_path, to_path) < 0) {
        return error(StringPrintf("Failed to link from %s to %s", from_path, to_path));
    }

    return ok();
}

binder::Status InstalldNativeService::moveAb(const std::string& apkPath,
        const std::string& instructionSet, const std::string& outputPath) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(apkPath);
    CHECK_ARGUMENT_PATH(outputPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* apk_path = apkPath.c_str();
    const char* instruction_set = instructionSet.c_str();
    const char* oat_dir = outputPath.c_str();

    bool success = move_ab(apk_path, instruction_set, oat_dir);
    return success ? ok() : error();
}

binder::Status InstalldNativeService::deleteOdex(const std::string& apkPath,
        const std::string& instructionSet, const std::unique_ptr<std::string>& outputPath) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(apkPath);
    CHECK_ARGUMENT_PATH(outputPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    const char* apk_path = apkPath.c_str();
    const char* instruction_set = instructionSet.c_str();
    const char* oat_dir = outputPath ? outputPath->c_str() : nullptr;

    bool res = delete_odex(apk_path, instruction_set, oat_dir);
    return res ? ok() : error();
}

// This kernel feature is experimental.
// TODO: remove local definition once upstreamed
#ifndef FS_IOC_ENABLE_VERITY

#define FS_IOC_ENABLE_VERITY           _IO('f', 133)
#define FS_IOC_SET_VERITY_MEASUREMENT  _IOW('f', 134, struct fsverity_measurement)

#define FS_VERITY_ALG_SHA256           1

struct fsverity_measurement {
    __u16 digest_algorithm;
    __u16 digest_size;
    __u32 reserved1;
    __u64 reserved2[3];
    __u8 digest[];
};

#endif

binder::Status InstalldNativeService::installApkVerity(const std::string& filePath,
        const ::android::base::unique_fd& verityInputAshmem, int32_t contentSize) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(filePath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    if (!android::base::GetBoolProperty(kPropApkVerityMode, false)) {
        return ok();
    }
#ifndef NDEBUG
    ASSERT_PAGE_SIZE_4K();
#endif
    // TODO: also check fsverity support in the current file system if compiled with DEBUG.
    // TODO: change ashmem to some temporary file to support huge apk.
    if (!ashmem_valid(verityInputAshmem.get())) {
        return error("FD is not an ashmem");
    }

    // 1. Seek to the next page boundary beyond the end of the file.
    ::android::base::unique_fd wfd(open(filePath.c_str(), O_WRONLY));
    if (wfd.get() < 0) {
        return error("Failed to open " + filePath);
    }
    struct stat st;
    if (fstat(wfd.get(), &st) < 0) {
        return error("Failed to stat " + filePath);
    }
    // fsverity starts from the block boundary.
    off_t padding = kVerityPageSize - st.st_size % kVerityPageSize;
    if (padding == kVerityPageSize) {
        padding = 0;
    }
    if (lseek(wfd.get(), st.st_size + padding, SEEK_SET) < 0) {
        return error("Failed to lseek " + filePath);
    }

    // 2. Write everything in the ashmem to the file.  Note that allocated
    //    ashmem size is multiple of page size, which is different from the
    //    actual content size.
    int shmSize = ashmem_get_size_region(verityInputAshmem.get());
    if (shmSize < 0) {
        return error("Failed to get ashmem size: " + std::to_string(shmSize));
    }
    if (contentSize < 0) {
        return error("Invalid content size: " + std::to_string(contentSize));
    }
    if (contentSize > shmSize) {
        return error("Content size overflow: " + std::to_string(contentSize) + " > " +
                     std::to_string(shmSize));
    }
    auto data = std::unique_ptr<void, std::function<void (void *)>>(
        mmap(nullptr, contentSize, PROT_READ, MAP_SHARED, verityInputAshmem.get(), 0),
        [contentSize] (void* ptr) {
          if (ptr != MAP_FAILED) {
            munmap(ptr, contentSize);
          }
        });

    if (data.get() == MAP_FAILED) {
        return error("Failed to mmap the ashmem");
    }
    char* cursor = reinterpret_cast<char*>(data.get());
    int remaining = contentSize;
    while (remaining > 0) {
        int ret = TEMP_FAILURE_RETRY(write(wfd.get(), cursor, remaining));
        if (ret < 0) {
            return error("Failed to write to " + filePath + " (" + std::to_string(remaining) +
                         + "/" + std::to_string(contentSize) + ")");
        }
        cursor += ret;
        remaining -= ret;
    }
    wfd.reset();

    // 3. Enable fsverity (needs readonly fd. Once it's done, the file becomes immutable.
    ::android::base::unique_fd rfd(open(filePath.c_str(), O_RDONLY));
    if (ioctl(rfd.get(), FS_IOC_ENABLE_VERITY, nullptr) < 0) {
        return error("Failed to enable fsverity on " + filePath);
    }
    return ok();
}

binder::Status InstalldNativeService::assertFsverityRootHashMatches(const std::string& filePath,
        const std::vector<uint8_t>& expectedHash) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PATH(filePath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    if (!android::base::GetBoolProperty(kPropApkVerityMode, false)) {
        return ok();
    }
    // TODO: also check fsverity support in the current file system if compiled with DEBUG.
    if (expectedHash.size() != kSha256Size) {
        return error("verity hash size should be " + std::to_string(kSha256Size) + " but is " +
                     std::to_string(expectedHash.size()));
    }

    ::android::base::unique_fd fd(open(filePath.c_str(), O_RDONLY));
    if (fd.get() < 0) {
        return error("Failed to open " + filePath + ": " + strerror(errno));
    }

    unsigned int buffer_size = sizeof(fsverity_measurement) + kSha256Size;
    std::vector<char> buffer(buffer_size, 0);

    fsverity_measurement* config = reinterpret_cast<fsverity_measurement*>(buffer.data());
    config->digest_algorithm = FS_VERITY_ALG_SHA256;
    config->digest_size = kSha256Size;
    memcpy(config->digest, expectedHash.data(), kSha256Size);
    if (ioctl(fd.get(), FS_IOC_SET_VERITY_MEASUREMENT, config) < 0) {
        // This includes an expected failure case with no FSVerity setup. It normally happens when
        // the apk does not contains the Merkle tree root hash.
        return error("Failed to measure fsverity on " + filePath + ": " + strerror(errno));
    }
    return ok();  // hashes match
}

binder::Status InstalldNativeService::reconcileSecondaryDexFile(
        const std::string& dexPath, const std::string& packageName, int32_t uid,
        const std::vector<std::string>& isas, const std::unique_ptr<std::string>& volumeUuid,
        int32_t storage_flag, bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(volumeUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    CHECK_ARGUMENT_PATH(dexPath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    bool result = android::installd::reconcile_secondary_dex_file(
            dexPath, packageName, uid, isas, volumeUuid, storage_flag, _aidl_return);
    return result ? ok() : error();
}

binder::Status InstalldNativeService::hashSecondaryDexFile(
        const std::string& dexPath, const std::string& packageName, int32_t uid,
        const std::unique_ptr<std::string>& volumeUuid, int32_t storageFlag,
        std::vector<uint8_t>* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_UUID(volumeUuid);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    CHECK_ARGUMENT_PATH(dexPath);

    // mLock is not taken here since we will never modify the file system.
    // If a file is modified just as we are reading it this may result in an
    // anomalous hash, but that's ok.
    bool result = android::installd::hash_secondary_dex_file(
        dexPath, packageName, uid, volumeUuid, storageFlag, _aidl_return);
    return result ? ok() : error();
}

binder::Status InstalldNativeService::invalidateMounts() {
    ENFORCE_UID(AID_SYSTEM);
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);

    mStorageMounts.clear();

#if !BYPASS_QUOTA
    if (!InvalidateQuotaMounts()) {
        return error("Failed to read mounts");
    }
#endif

    std::ifstream in("/proc/mounts");
    if (!in.is_open()) {
        return error("Failed to read mounts");
    }

    std::string source;
    std::string target;
    std::string ignored;
    while (!in.eof()) {
        std::getline(in, source, ' ');
        std::getline(in, target, ' ');
        std::getline(in, ignored);

#if !BYPASS_SDCARDFS
        if (target.compare(0, 21, "/mnt/runtime/default/") == 0) {
            LOG(DEBUG) << "Found storage mount " << source << " at " << target;
            mStorageMounts[source] = target;
        }
#endif
    }
    return ok();
}

std::string InstalldNativeService::findDataMediaPath(
        const std::unique_ptr<std::string>& uuid, userid_t userid) {
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);
    const char* uuid_ = uuid ? uuid->c_str() : nullptr;
    auto path = StringPrintf("%s/media", create_data_path(uuid_).c_str());
    auto resolved = mStorageMounts[path];
    if (resolved.empty()) {
        LOG(WARNING) << "Failed to find storage mount for " << path;
        resolved = path;
    }
    return StringPrintf("%s/%u", resolved.c_str(), userid);
}

binder::Status InstalldNativeService::isQuotaSupported(
        const std::unique_ptr<std::string>& uuid, bool* _aidl_return) {
    auto uuidString = uuid ? *uuid : "";
    *_aidl_return = IsQuotaSupported(uuidString);
    return ok();
}

binder::Status InstalldNativeService::prepareAppProfile(const std::string& packageName,
        int32_t userId, int32_t appId, const std::string& profileName, const std::string& codePath,
        const std::unique_ptr<std::string>& dexMetadata, bool* _aidl_return) {
    ENFORCE_UID(AID_SYSTEM);
    CHECK_ARGUMENT_PACKAGE_NAME(packageName);
    CHECK_ARGUMENT_PATH(codePath);
    std::lock_guard<std::recursive_mutex> lock(mLock);

    *_aidl_return = prepare_app_profile(packageName, userId, appId, profileName, codePath,
        dexMetadata);
    return ok();
}

binder::Status InstalldNativeService::migrateLegacyObbData() {
    ENFORCE_UID(AID_SYSTEM);
    // NOTE: The lint warning doesn't apply to the use of system(3) with
    // absolute parse and no command line arguments.
    if (system("/system/bin/migrate_legacy_obb_data.sh") != 0) { // NOLINT(cert-env33-c)
        LOG(ERROR) << "Unable to migrate legacy obb data";
    }

    return ok();
}

}  // namespace installd
}  // namespace android
