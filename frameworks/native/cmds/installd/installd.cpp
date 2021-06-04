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
#define LOG_TAG "installd"

#include <fcntl.h>
#include <selinux/android.h>
#include <selinux/avc.h>
#include <sys/capability.h>
#include <sys/fsuid.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <android-base/logging.h>
#include <cutils/fs.h>
#include <cutils/properties.h>
#include <log/log.h>              // TODO: Move everything to base::logging.
#include <private/android_filesystem_config.h>

#include "InstalldNativeService.h"
#include "dexopt.h"
#include "globals.h"
#include "installd_constants.h"
#include "installd_deps.h"  // Need to fill in requirements of commands.
#include "utils.h"

namespace android {
namespace installd {

// Check that installd-deps sizes match cutils sizes.
static_assert(kPropertyKeyMax == PROPERTY_KEY_MAX, "Size mismatch.");
static_assert(kPropertyValueMax == PROPERTY_VALUE_MAX, "Size mismatch.");

////////////////////////
// Plug-in functions. //
////////////////////////

int get_property(const char *key, char *value, const char *default_value) {
    return property_get(key, value, default_value);
}

bool calculate_oat_file_path(char path[PKG_PATH_MAX], const char *oat_dir, const char *apk_path,
        const char *instruction_set) {
    return calculate_oat_file_path_default(path, oat_dir, apk_path, instruction_set);
}

bool calculate_odex_file_path(char path[PKG_PATH_MAX], const char *apk_path,
        const char *instruction_set) {
    return calculate_odex_file_path_default(path, apk_path, instruction_set);
}

bool create_cache_path(char path[PKG_PATH_MAX], const char *src, const char *instruction_set) {
    return create_cache_path_default(path, src, instruction_set);
}

static bool initialize_globals() {
    return init_globals_from_data_and_root();
}

static int initialize_directories() {
    int res = -1;

    // Read current filesystem layout version to handle upgrade paths
    char version_path[PATH_MAX];
    snprintf(version_path, PATH_MAX, "%s.layout_version", android_data_dir.c_str());

    int oldVersion;
    if (fs_read_atomic_int(version_path, &oldVersion) == -1) {
        oldVersion = 0;
    }
    int version = oldVersion;

    if (version < 2) {
        SLOGD("Assuming that device has multi-user storage layout; upgrade no longer supported");
        version = 2;
    }

    if (ensure_config_user_dirs(0) == -1) {
        SLOGE("Failed to setup misc for user 0");
        goto fail;
    }

    if (version == 2) {
        SLOGD("Upgrading to /data/misc/user directories");

        char misc_dir[PATH_MAX];
        snprintf(misc_dir, PATH_MAX, "%smisc", android_data_dir.c_str());

        char keychain_added_dir[PATH_MAX];
        snprintf(keychain_added_dir, PATH_MAX, "%s/keychain/cacerts-added", misc_dir);

        char keychain_removed_dir[PATH_MAX];
        snprintf(keychain_removed_dir, PATH_MAX, "%s/keychain/cacerts-removed", misc_dir);

        DIR *dir;
        struct dirent *dirent;
        dir = opendir("/data/user");
        if (dir != nullptr) {
            while ((dirent = readdir(dir))) {
                const char *name = dirent->d_name;

                // skip "." and ".."
                if (name[0] == '.') {
                    if (name[1] == 0) continue;
                    if ((name[1] == '.') && (name[2] == 0)) continue;
                }

                uint32_t user_id = std::stoi(name);

                // /data/misc/user/<user_id>
                if (ensure_config_user_dirs(user_id) == -1) {
                    goto fail;
                }

                char misc_added_dir[PATH_MAX];
                snprintf(misc_added_dir, PATH_MAX, "%s/user/%s/cacerts-added", misc_dir, name);

                char misc_removed_dir[PATH_MAX];
                snprintf(misc_removed_dir, PATH_MAX, "%s/user/%s/cacerts-removed", misc_dir, name);

                uid_t uid = multiuser_get_uid(user_id, AID_SYSTEM);
                gid_t gid = uid;
                if (access(keychain_added_dir, F_OK) == 0) {
                    if (copy_dir_files(keychain_added_dir, misc_added_dir, uid, gid) != 0) {
                        SLOGE("Some files failed to copy");
                    }
                }
                if (access(keychain_removed_dir, F_OK) == 0) {
                    if (copy_dir_files(keychain_removed_dir, misc_removed_dir, uid, gid) != 0) {
                        SLOGE("Some files failed to copy");
                    }
                }
            }
            closedir(dir);

            if (access(keychain_added_dir, F_OK) == 0) {
                delete_dir_contents(keychain_added_dir, 1, nullptr);
            }
            if (access(keychain_removed_dir, F_OK) == 0) {
                delete_dir_contents(keychain_removed_dir, 1, nullptr);
            }
        }

        version = 3;
    }

    // Persist layout version if changed
    if (version != oldVersion) {
        if (fs_write_atomic_int(version_path, version) == -1) {
            SLOGE("Failed to save version to %s: %s", version_path, strerror(errno));
            goto fail;
        }
    }

    // Success!
    res = 0;

fail:
    return res;
}

static int log_callback(int type, const char *fmt, ...) { // NOLINT
    va_list ap;
    int priority;

    switch (type) {
    case SELINUX_WARNING:
        priority = ANDROID_LOG_WARN;
        break;
    case SELINUX_INFO:
        priority = ANDROID_LOG_INFO;
        break;
    default:
        priority = ANDROID_LOG_ERROR;
        break;
    }
    va_start(ap, fmt);
    LOG_PRI_VA(priority, "SELinux", fmt, ap);
    va_end(ap);
    return 0;
}

static int installd_main(const int argc ATTRIBUTE_UNUSED, char *argv[]) {
    int ret;
    int selinux_enabled = (is_selinux_enabled() > 0);

    setenv("ANDROID_LOG_TAGS", "*:v", 1);
    android::base::InitLogging(argv);

    SLOGI("installd firing up");

    union selinux_callback cb;
    cb.func_log = log_callback;
    selinux_set_callback(SELINUX_CB_LOG, cb);

    if (!initialize_globals()) {
        SLOGE("Could not initialize globals; exiting.\n");
        exit(1);
    }

    if (initialize_directories() < 0) {
        SLOGE("Could not create directories; exiting.\n");
        exit(1);
    }

    if (selinux_enabled && selinux_status_open(true) < 0) {
        SLOGE("Could not open selinux status; exiting.\n");
        exit(1);
    }

    if ((ret = InstalldNativeService::start()) != android::OK) {
        SLOGE("Unable to start InstalldNativeService: %d", ret);
        exit(1);
    }

    IPCThreadState::self()->joinThreadPool();

    LOG(INFO) << "installd shutting down";

    return 0;
}

}  // namespace installd
}  // namespace android

int main(const int argc, char *argv[]) {
    return android::installd::installd_main(argc, argv);
}
