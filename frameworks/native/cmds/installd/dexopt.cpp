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
#define LOG_TAG "installd"

#include <array>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iomanip>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <cutils/fs.h>
#include <cutils/properties.h>
#include <cutils/sched_policy.h>
#include <dex2oat_return_codes.h>
#include <log/log.h>               // TODO: Move everything to base/logging.
#include <openssl/sha.h>
#include <private/android_filesystem_config.h>
#include <processgroup/sched_policy.h>
#include <selinux/android.h>
#include <server_configurable_flags/get_flags.h>
#include <system/thread_defs.h>

#include "dexopt.h"
#include "dexopt_return_codes.h"
#include "globals.h"
#include "installd_deps.h"
#include "otapreopt_utils.h"
#include "utils.h"

using android::base::EndsWith;
using android::base::GetBoolProperty;
using android::base::GetProperty;
using android::base::ReadFdToString;
using android::base::ReadFully;
using android::base::StringPrintf;
using android::base::WriteFully;
using android::base::unique_fd;

namespace android {
namespace installd {

// Should minidebug info be included in compiled artifacts? Even if this value is
// "true," usage might still be conditional to other constraints, e.g., system
// property overrides.
static constexpr bool kEnableMinidebugInfo = true;

static constexpr const char* kMinidebugInfoSystemProperty = "dalvik.vm.dex2oat-minidebuginfo";
static constexpr bool kMinidebugInfoSystemPropertyDefault = false;
static constexpr const char* kMinidebugDex2oatFlag = "--generate-mini-debug-info";
static constexpr const char* kDisableCompactDexFlag = "--compact-dex-level=none";


// Deleter using free() for use with std::unique_ptr<>. See also UniqueCPtr<> below.
struct FreeDelete {
  // NOTE: Deleting a const object is valid but free() takes a non-const pointer.
  void operator()(const void* ptr) const {
    free(const_cast<void*>(ptr));
  }
};

// Alias for std::unique_ptr<> that uses the C function free() to delete objects.
template <typename T>
using UniqueCPtr = std::unique_ptr<T, FreeDelete>;

static unique_fd invalid_unique_fd() {
    return unique_fd(-1);
}

static bool is_debug_runtime() {
    return android::base::GetProperty("persist.sys.dalvik.vm.lib.2", "") == "libartd.so";
}

static bool is_debuggable_build() {
    return android::base::GetBoolProperty("ro.debuggable", false);
}

static bool clear_profile(const std::string& profile) {
    unique_fd ufd(open(profile.c_str(), O_WRONLY | O_NOFOLLOW | O_CLOEXEC));
    if (ufd.get() < 0) {
        if (errno != ENOENT) {
            PLOG(WARNING) << "Could not open profile " << profile;
            return false;
        } else {
            // Nothing to clear. That's ok.
            return true;
        }
    }

    if (flock(ufd.get(), LOCK_EX | LOCK_NB) != 0) {
        if (errno != EWOULDBLOCK) {
            PLOG(WARNING) << "Error locking profile " << profile;
        }
        // This implies that the app owning this profile is running
        // (and has acquired the lock).
        //
        // If we can't acquire the lock bail out since clearing is useless anyway
        // (the app will write again to the profile).
        //
        // Note:
        // This does not impact the this is not an issue for the profiling correctness.
        // In case this is needed because of an app upgrade, profiles will still be
        // eventually cleared by the app itself due to checksum mismatch.
        // If this is needed because profman advised, then keeping the data around
        // until the next run is again not an issue.
        //
        // If the app attempts to acquire a lock while we've held one here,
        // it will simply skip the current write cycle.
        return false;
    }

    bool truncated = ftruncate(ufd.get(), 0) == 0;
    if (!truncated) {
        PLOG(WARNING) << "Could not truncate " << profile;
    }
    if (flock(ufd.get(), LOCK_UN) != 0) {
        PLOG(WARNING) << "Error unlocking profile " << profile;
    }
    return truncated;
}

// Clear the reference profile for the given location.
// The location is the profile name for primary apks or the dex path for secondary dex files.
static bool clear_reference_profile(const std::string& package_name, const std::string& location,
        bool is_secondary_dex) {
    return clear_profile(create_reference_profile_path(package_name, location, is_secondary_dex));
}

// Clear the reference profile for the given location.
// The location is the profile name for primary apks or the dex path for secondary dex files.
static bool clear_current_profile(const std::string& package_name, const std::string& location,
        userid_t user, bool is_secondary_dex) {
    return clear_profile(create_current_profile_path(user, package_name, location,
            is_secondary_dex));
}

// Clear the reference profile for the primary apk of the given package.
// The location is the profile name for primary apks or the dex path for secondary dex files.
bool clear_primary_reference_profile(const std::string& package_name,
        const std::string& location) {
    return clear_reference_profile(package_name, location, /*is_secondary_dex*/false);
}

// Clear all current profile for the primary apk of the given package.
// The location is the profile name for primary apks or the dex path for secondary dex files.
bool clear_primary_current_profiles(const std::string& package_name, const std::string& location) {
    bool success = true;
    // For secondary dex files, we don't really need the user but we use it for sanity checks.
    std::vector<userid_t> users = get_known_users(/*volume_uuid*/ nullptr);
    for (auto user : users) {
        success &= clear_current_profile(package_name, location, user, /*is_secondary_dex*/false);
    }
    return success;
}

// Clear the current profile for the primary apk of the given package and user.
bool clear_primary_current_profile(const std::string& package_name, const std::string& location,
        userid_t user) {
    return clear_current_profile(package_name, location, user, /*is_secondary_dex*/false);
}

static std::vector<std::string> SplitBySpaces(const std::string& str) {
    if (str.empty()) {
        return {};
    }
    return android::base::Split(str, " ");
}

static const char* get_location_from_path(const char* path) {
    static constexpr char kLocationSeparator = '/';
    const char *location = strrchr(path, kLocationSeparator);
    if (location == nullptr) {
        return path;
    } else {
        // Skip the separator character.
        return location + 1;
    }
}

// ExecVHelper prepares and holds pointers to parsed command line arguments so that no allocations
// need to be performed between the fork and exec.
class ExecVHelper {
  public:
    // Store a placeholder for the binary name.
    ExecVHelper() : args_(1u, std::string()) {}

    void PrepareArgs(const std::string& bin) {
        CHECK(!args_.empty());
        CHECK(args_[0].empty());
        args_[0] = bin;
        // Write char* into array.
        for (const std::string& arg : args_) {
            argv_.push_back(arg.c_str());
        }
        argv_.push_back(nullptr);  // Add null terminator.
    }

    [[ noreturn ]]
    void Exec(int exit_code) {
        execv(argv_[0], (char * const *)&argv_[0]);
        PLOG(ERROR) << "execv(" << argv_[0] << ") failed";
        exit(exit_code);
    }

    // Add an arg if it's not empty.
    void AddArg(const std::string& arg) {
        if (!arg.empty()) {
            args_.push_back(arg);
        }
    }

    // Add a runtime arg if it's not empty.
    void AddRuntimeArg(const std::string& arg) {
        if (!arg.empty()) {
            args_.push_back("--runtime-arg");
            args_.push_back(arg);
        }
    }

  protected:
    // Holder arrays for backing arg storage.
    std::vector<std::string> args_;

    // Argument poiners.
    std::vector<const char*> argv_;
};

static std::string MapPropertyToArg(const std::string& property,
                                    const std::string& format,
                                    const std::string& default_value = "") {
  std::string prop = GetProperty(property, default_value);
  if (!prop.empty()) {
    return StringPrintf(format.c_str(), prop.c_str());
  }
  return "";
}

// Determines which binary we should use for execution (the debug or non-debug version).
// e.g. dex2oatd vs dex2oat
static const char* select_execution_binary(const char* binary, const char* debug_binary,
        bool background_job_compile) {
    return select_execution_binary(
        binary,
        debug_binary,
        background_job_compile,
        is_debug_runtime(),
        (android::base::GetProperty("ro.build.version.codename", "") == "REL"),
        is_debuggable_build());
}

// Determines which binary we should use for execution (the debug or non-debug version).
// e.g. dex2oatd vs dex2oat
// This is convenient method which is much easier to test because it doesn't read
// system properties.
const char* select_execution_binary(
        const char* binary,
        const char* debug_binary,
        bool background_job_compile,
        bool is_debug_runtime,
        bool is_release,
        bool is_debuggable_build) {
    // Do not use debug binaries for release candidates (to give more soak time).
    bool is_debug_bg_job = background_job_compile && is_debuggable_build && !is_release;

    // If the runtime was requested to use libartd.so, we'll run the debug version - assuming
    // the file is present (it may not be on images with very little space available).
    bool useDebug = (is_debug_runtime || is_debug_bg_job) && (access(debug_binary, X_OK) == 0);

    return useDebug ? debug_binary : binary;
}

// Namespace for Android Runtime flags applied during boot time.
static const char* RUNTIME_NATIVE_BOOT_NAMESPACE = "runtime_native_boot";
// Feature flag name for running the JIT in Zygote experiment, b/119800099.
static const char* ENABLE_APEX_IMAGE = "enable_apex_image";
// Location of the apex image.
static const char* kApexImage = "/system/framework/apex.art";

class RunDex2Oat : public ExecVHelper {
  public:
    RunDex2Oat(int zip_fd,
               int oat_fd,
               int input_vdex_fd,
               int output_vdex_fd,
               int image_fd,
               const char* input_file_name,
               const char* output_file_name,
               int swap_fd,
               const char* instruction_set,
               const char* compiler_filter,
               bool debuggable,
               bool post_bootcomplete,
               bool background_job_compile,
               int profile_fd,
               const char* class_loader_context,
               const std::string& class_loader_context_fds,
               int target_sdk_version,
               bool enable_hidden_api_checks,
               bool generate_compact_dex,
               int dex_metadata_fd,
               const char* compilation_reason) {
        // Get the relative path to the input file.
        const char* relative_input_file_name = get_location_from_path(input_file_name);

        std::string dex2oat_Xms_arg = MapPropertyToArg("dalvik.vm.dex2oat-Xms", "-Xms%s");
        std::string dex2oat_Xmx_arg = MapPropertyToArg("dalvik.vm.dex2oat-Xmx", "-Xmx%s");

        const char* threads_property = post_bootcomplete
                ? "dalvik.vm.dex2oat-threads"
                : "dalvik.vm.boot-dex2oat-threads";
        std::string dex2oat_threads_arg = MapPropertyToArg(threads_property, "-j%s");
        const char* cpu_set_property = post_bootcomplete
                ? "dalvik.vm.dex2oat-cpu-set"
                : "dalvik.vm.boot-dex2oat-cpu-set";
        std::string dex2oat_cpu_set_arg = MapPropertyToArg(cpu_set_property, "--cpu-set=%s");

        std::string bootclasspath;
        char* dex2oat_bootclasspath = getenv("DEX2OATBOOTCLASSPATH");
        if (dex2oat_bootclasspath != nullptr) {
            bootclasspath = StringPrintf("-Xbootclasspath:%s", dex2oat_bootclasspath);
        }
        // If DEX2OATBOOTCLASSPATH is not in the environment, dex2oat is going to query
        // BOOTCLASSPATH.

        const std::string dex2oat_isa_features_key =
                StringPrintf("dalvik.vm.isa.%s.features", instruction_set);
        std::string instruction_set_features_arg =
            MapPropertyToArg(dex2oat_isa_features_key, "--instruction-set-features=%s");

        const std::string dex2oat_isa_variant_key =
                StringPrintf("dalvik.vm.isa.%s.variant", instruction_set);
        std::string instruction_set_variant_arg =
            MapPropertyToArg(dex2oat_isa_variant_key, "--instruction-set-variant=%s");

        const char* dex2oat_norelocation = "-Xnorelocate";

        const std::string dex2oat_flags = GetProperty("dalvik.vm.dex2oat-flags", "");
        std::vector<std::string> dex2oat_flags_args = SplitBySpaces(dex2oat_flags);
        ALOGV("dalvik.vm.dex2oat-flags=%s\n", dex2oat_flags.c_str());

        // If we are booting without the real /data, don't spend time compiling.
        std::string vold_decrypt = GetProperty("vold.decrypt", "");
        bool skip_compilation = vold_decrypt == "trigger_restart_min_framework" ||
                                vold_decrypt == "1";

        std::string resolve_startup_string_arg =
                MapPropertyToArg("persist.device_config.runtime.dex2oat_resolve_startup_strings",
                                 "--resolve-startup-const-strings=%s");
        if (resolve_startup_string_arg.empty()) {
          // If empty, fall back to system property.
          resolve_startup_string_arg =
                MapPropertyToArg("dalvik.vm.dex2oat-resolve-startup-strings",
                                 "--resolve-startup-const-strings=%s");
        }

        const std::string image_block_size_arg =
                MapPropertyToArg("dalvik.vm.dex2oat-max-image-block-size",
                                 "--max-image-block-size=%s");

        const bool generate_debug_info = GetBoolProperty("debug.generate-debug-info", false);

        std::string image_format_arg;
        if (image_fd >= 0) {
            image_format_arg = MapPropertyToArg("dalvik.vm.appimageformat", "--image-format=%s");
        }

        std::string dex2oat_large_app_threshold_arg =
            MapPropertyToArg("dalvik.vm.dex2oat-very-large", "--very-large-app-threshold=%s");


        const char* dex2oat_bin = select_execution_binary(
            kDex2oatPath, kDex2oatDebugPath, background_job_compile);

        bool generate_minidebug_info = kEnableMinidebugInfo &&
                GetBoolProperty(kMinidebugInfoSystemProperty, kMinidebugInfoSystemPropertyDefault);

        std::string boot_image;
        std::string use_apex_image =
            server_configurable_flags::GetServerConfigurableFlag(RUNTIME_NATIVE_BOOT_NAMESPACE,
                                                                 ENABLE_APEX_IMAGE,
                                                                 /*default_value=*/ "");
        if (use_apex_image == "true") {
          boot_image = StringPrintf("-Ximage:%s", kApexImage);
        } else {
          boot_image = MapPropertyToArg("dalvik.vm.boot-image", "-Ximage:%s");
        }

        // clang FORTIFY doesn't let us use strlen in constant array bounds, so we
        // use arraysize instead.
        std::string zip_fd_arg = StringPrintf("--zip-fd=%d", zip_fd);
        std::string zip_location_arg = StringPrintf("--zip-location=%s", relative_input_file_name);
        std::string input_vdex_fd_arg = StringPrintf("--input-vdex-fd=%d", input_vdex_fd);
        std::string output_vdex_fd_arg = StringPrintf("--output-vdex-fd=%d", output_vdex_fd);
        std::string oat_fd_arg = StringPrintf("--oat-fd=%d", oat_fd);
        std::string oat_location_arg = StringPrintf("--oat-location=%s", output_file_name);
        std::string instruction_set_arg = StringPrintf("--instruction-set=%s", instruction_set);
        std::string dex2oat_compiler_filter_arg;
        std::string dex2oat_swap_fd;
        std::string dex2oat_image_fd;
        std::string target_sdk_version_arg;
        if (target_sdk_version != 0) {
            target_sdk_version_arg = StringPrintf("-Xtarget-sdk-version:%d", target_sdk_version);
        }
        std::string class_loader_context_arg;
        std::string class_loader_context_fds_arg;
        if (class_loader_context != nullptr) {
            class_loader_context_arg = StringPrintf("--class-loader-context=%s",
                                                    class_loader_context);
            if (!class_loader_context_fds.empty()) {
                class_loader_context_fds_arg = StringPrintf("--class-loader-context-fds=%s",
                                                            class_loader_context_fds.c_str());
            }
        }

        if (swap_fd >= 0) {
            dex2oat_swap_fd = StringPrintf("--swap-fd=%d", swap_fd);
        }
        if (image_fd >= 0) {
            dex2oat_image_fd = StringPrintf("--app-image-fd=%d", image_fd);
        }

        // Compute compiler filter.
        bool have_dex2oat_relocation_skip_flag = false;
        if (skip_compilation) {
            dex2oat_compiler_filter_arg = "--compiler-filter=extract";
            have_dex2oat_relocation_skip_flag = true;
        } else if (compiler_filter != nullptr) {
            dex2oat_compiler_filter_arg = StringPrintf("--compiler-filter=%s", compiler_filter);
        }

        if (dex2oat_compiler_filter_arg.empty()) {
            dex2oat_compiler_filter_arg = MapPropertyToArg("dalvik.vm.dex2oat-filter",
                                                           "--compiler-filter=%s");
        }

        // Check whether all apps should be compiled debuggable.
        if (!debuggable) {
            debuggable = GetProperty("dalvik.vm.always_debuggable", "") == "1";
        }
        std::string profile_arg;
        if (profile_fd != -1) {
            profile_arg = StringPrintf("--profile-file-fd=%d", profile_fd);
        }

        // Get the directory of the apk to pass as a base classpath directory.
        std::string base_dir;
        std::string apk_dir(input_file_name);
        unsigned long dir_index = apk_dir.rfind('/');
        bool has_base_dir = dir_index != std::string::npos;
        if (has_base_dir) {
            apk_dir = apk_dir.substr(0, dir_index);
            base_dir = StringPrintf("--classpath-dir=%s", apk_dir.c_str());
        }

        std::string dex_metadata_fd_arg = "--dm-fd=" + std::to_string(dex_metadata_fd);

        std::string compilation_reason_arg = compilation_reason == nullptr
                ? ""
                : std::string("--compilation-reason=") + compilation_reason;

        ALOGV("Running %s in=%s out=%s\n", dex2oat_bin, relative_input_file_name, output_file_name);

        // Disable cdex if update input vdex is true since this combination of options is not
        // supported.
        const bool disable_cdex = !generate_compact_dex || (input_vdex_fd == output_vdex_fd);

        AddArg(zip_fd_arg);
        AddArg(zip_location_arg);
        AddArg(input_vdex_fd_arg);
        AddArg(output_vdex_fd_arg);
        AddArg(oat_fd_arg);
        AddArg(oat_location_arg);
        AddArg(instruction_set_arg);

        AddArg(instruction_set_variant_arg);
        AddArg(instruction_set_features_arg);

        AddRuntimeArg(boot_image);
        AddRuntimeArg(bootclasspath);
        AddRuntimeArg(dex2oat_Xms_arg);
        AddRuntimeArg(dex2oat_Xmx_arg);

        AddArg(resolve_startup_string_arg);
        AddArg(image_block_size_arg);
        AddArg(dex2oat_compiler_filter_arg);
        AddArg(dex2oat_threads_arg);
        AddArg(dex2oat_cpu_set_arg);
        AddArg(dex2oat_swap_fd);
        AddArg(dex2oat_image_fd);

        if (generate_debug_info) {
            AddArg("--generate-debug-info");
        }
        if (debuggable) {
            AddArg("--debuggable");
        }
        AddArg(image_format_arg);
        AddArg(dex2oat_large_app_threshold_arg);

        if (have_dex2oat_relocation_skip_flag) {
            AddRuntimeArg(dex2oat_norelocation);
        }
        AddArg(profile_arg);
        AddArg(base_dir);
        AddArg(class_loader_context_arg);
        AddArg(class_loader_context_fds_arg);
        if (generate_minidebug_info) {
            AddArg(kMinidebugDex2oatFlag);
        }
        if (disable_cdex) {
            AddArg(kDisableCompactDexFlag);
        }
        AddRuntimeArg(target_sdk_version_arg);
        if (enable_hidden_api_checks) {
            AddRuntimeArg("-Xhidden-api-policy:enabled");
        }

        if (dex_metadata_fd > -1) {
            AddArg(dex_metadata_fd_arg);
        }

        AddArg(compilation_reason_arg);

        // Do not add args after dex2oat_flags, they should override others for debugging.
        args_.insert(args_.end(), dex2oat_flags_args.begin(), dex2oat_flags_args.end());

        PrepareArgs(dex2oat_bin);
    }
};

/*
 * Whether dexopt should use a swap file when compiling an APK.
 *
 * If kAlwaysProvideSwapFile, do this on all devices (dex2oat will make a more informed decision
 * itself, anyways).
 *
 * Otherwise, read "dalvik.vm.dex2oat-swap". If the property exists, return whether it is "true".
 *
 * Otherwise, return true if this is a low-mem device.
 *
 * Otherwise, return default value.
 */
static bool kAlwaysProvideSwapFile = false;
static bool kDefaultProvideSwapFile = true;

static bool ShouldUseSwapFileForDexopt() {
    if (kAlwaysProvideSwapFile) {
        return true;
    }

    // Check the "override" property. If it exists, return value == "true".
    std::string dex2oat_prop_buf = GetProperty("dalvik.vm.dex2oat-swap", "");
    if (!dex2oat_prop_buf.empty()) {
        return dex2oat_prop_buf == "true";
    }

    // Shortcut for default value. This is an implementation optimization for the process sketched
    // above. If the default value is true, we can avoid to check whether this is a low-mem device,
    // as low-mem is never returning false. The compiler will optimize this away if it can.
    if (kDefaultProvideSwapFile) {
        return true;
    }

    if (GetBoolProperty("ro.config.low_ram", false)) {
        return true;
    }

    // Default value must be false here.
    return kDefaultProvideSwapFile;
}

static void SetDex2OatScheduling(bool set_to_bg) {
    if (set_to_bg) {
        if (set_sched_policy(0, SP_BACKGROUND) < 0) {
            PLOG(ERROR) << "set_sched_policy failed";
            exit(DexoptReturnCodes::kSetSchedPolicy);
        }
        if (setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_BACKGROUND) < 0) {
            PLOG(ERROR) << "setpriority failed";
            exit(DexoptReturnCodes::kSetPriority);
        }
    }
}

static unique_fd create_profile(uid_t uid, const std::string& profile, int32_t flags) {
    unique_fd fd(TEMP_FAILURE_RETRY(open(profile.c_str(), flags, 0600)));
    if (fd.get() < 0) {
        if (errno != EEXIST) {
            PLOG(ERROR) << "Failed to create profile " << profile;
            return invalid_unique_fd();
        }
    }
    // Profiles should belong to the app; make sure of that by giving ownership to
    // the app uid. If we cannot do that, there's no point in returning the fd
    // since dex2oat/profman will fail with SElinux denials.
    if (fchown(fd.get(), uid, uid) < 0) {
        PLOG(ERROR) << "Could not chown profile " << profile;
        return invalid_unique_fd();
    }
    return fd;
}

static unique_fd open_profile(uid_t uid, const std::string& profile, int32_t flags) {
    // Do not follow symlinks when opening a profile:
    //   - primary profiles should not contain symlinks in their paths
    //   - secondary dex paths should have been already resolved and validated
    flags |= O_NOFOLLOW;

    // Check if we need to create the profile
    // Reference profiles and snapshots are created on the fly; so they might not exist beforehand.
    unique_fd fd;
    if ((flags & O_CREAT) != 0) {
        fd = create_profile(uid, profile, flags);
    } else {
        fd.reset(TEMP_FAILURE_RETRY(open(profile.c_str(), flags)));
    }

    if (fd.get() < 0) {
        if (errno != ENOENT) {
            // Profiles might be missing for various reasons. For example, in a
            // multi-user environment, the profile directory for one user can be created
            // after we start a merge. In this case the current profile for that user
            // will not be found.
            // Also, the secondary dex profiles might be deleted by the app at any time,
            // so we can't we need to prepare if they are missing.
            PLOG(ERROR) << "Failed to open profile " << profile;
        }
        return invalid_unique_fd();
    }

    return fd;
}

static unique_fd open_current_profile(uid_t uid, userid_t user, const std::string& package_name,
        const std::string& location, bool is_secondary_dex) {
    std::string profile = create_current_profile_path(user, package_name, location,
            is_secondary_dex);
    return open_profile(uid, profile, O_RDONLY);
}

static unique_fd open_reference_profile(uid_t uid, const std::string& package_name,
        const std::string& location, bool read_write, bool is_secondary_dex) {
    std::string profile = create_reference_profile_path(package_name, location, is_secondary_dex);
    return open_profile(uid, profile, read_write ? (O_CREAT | O_RDWR) : O_RDONLY);
}

static unique_fd open_spnashot_profile(uid_t uid, const std::string& package_name,
        const std::string& location) {
    std::string profile = create_snapshot_profile_path(package_name, location);
    return open_profile(uid, profile, O_CREAT | O_RDWR | O_TRUNC);
}

static void open_profile_files(uid_t uid, const std::string& package_name,
            const std::string& location, bool is_secondary_dex,
            /*out*/ std::vector<unique_fd>* profiles_fd, /*out*/ unique_fd* reference_profile_fd) {
    // Open the reference profile in read-write mode as profman might need to save the merge.
    *reference_profile_fd = open_reference_profile(uid, package_name, location,
            /*read_write*/ true, is_secondary_dex);

    // For secondary dex files, we don't really need the user but we use it for sanity checks.
    // Note: the user owning the dex file should be the current user.
    std::vector<userid_t> users;
    if (is_secondary_dex){
        users.push_back(multiuser_get_user_id(uid));
    } else {
        users = get_known_users(/*volume_uuid*/ nullptr);
    }
    for (auto user : users) {
        unique_fd profile_fd = open_current_profile(uid, user, package_name, location,
                is_secondary_dex);
        // Add to the lists only if both fds are valid.
        if (profile_fd.get() >= 0) {
            profiles_fd->push_back(std::move(profile_fd));
        }
    }
}

static constexpr int PROFMAN_BIN_RETURN_CODE_COMPILE = 0;
static constexpr int PROFMAN_BIN_RETURN_CODE_SKIP_COMPILATION = 1;
static constexpr int PROFMAN_BIN_RETURN_CODE_BAD_PROFILES = 2;
static constexpr int PROFMAN_BIN_RETURN_CODE_ERROR_IO = 3;
static constexpr int PROFMAN_BIN_RETURN_CODE_ERROR_LOCKING = 4;

class RunProfman : public ExecVHelper {
  public:
   void SetupArgs(const std::vector<unique_fd>& profile_fds,
                  const unique_fd& reference_profile_fd,
                  const std::vector<unique_fd>& apk_fds,
                  const std::vector<std::string>& dex_locations,
                  bool copy_and_update,
                  bool store_aggregation_counters) {

        // TODO(calin): Assume for now we run in the bg compile job (which is in
        // most of the invocation). With the current data flow, is not very easy or
        // clean to discover this in RunProfman (it will require quite a messy refactoring).
        const char* profman_bin = select_execution_binary(
            kProfmanPath, kProfmanDebugPath, /*background_job_compile=*/ true);

        if (copy_and_update) {
            CHECK_EQ(1u, profile_fds.size());
            CHECK_EQ(1u, apk_fds.size());
        }
        if (reference_profile_fd != -1) {
            AddArg("--reference-profile-file-fd=" + std::to_string(reference_profile_fd.get()));
        }

        for (const unique_fd& fd : profile_fds) {
            AddArg("--profile-file-fd=" + std::to_string(fd.get()));
        }

        for (const unique_fd& fd : apk_fds) {
            AddArg("--apk-fd=" + std::to_string(fd.get()));
        }

        for (const std::string& dex_location : dex_locations) {
            AddArg("--dex-location=" + dex_location);
        }

        if (copy_and_update) {
            AddArg("--copy-and-update-profile-key");
        }

        if (store_aggregation_counters) {
            AddArg("--store-aggregation-counters");
        }

        // Do not add after dex2oat_flags, they should override others for debugging.
        PrepareArgs(profman_bin);
    }

    void SetupMerge(const std::vector<unique_fd>& profiles_fd,
                    const unique_fd& reference_profile_fd,
                    const std::vector<unique_fd>& apk_fds = std::vector<unique_fd>(),
                    const std::vector<std::string>& dex_locations = std::vector<std::string>(),
                    bool store_aggregation_counters = false) {
        SetupArgs(profiles_fd,
                  reference_profile_fd,
                  apk_fds,
                  dex_locations,
                  /*copy_and_update=*/false,
                  store_aggregation_counters);
    }

    void SetupCopyAndUpdate(unique_fd&& profile_fd,
                            unique_fd&& reference_profile_fd,
                            unique_fd&& apk_fd,
                            const std::string& dex_location) {
        // The fds need to stay open longer than the scope of the function, so put them into a local
        // variable vector.
        profiles_fd_.push_back(std::move(profile_fd));
        apk_fds_.push_back(std::move(apk_fd));
        reference_profile_fd_ = std::move(reference_profile_fd);
        std::vector<std::string> dex_locations = {dex_location};
        SetupArgs(profiles_fd_,
                  reference_profile_fd_,
                  apk_fds_,
                  dex_locations,
                  /*copy_and_update=*/true,
                  /*store_aggregation_counters=*/false);
    }

    void SetupDump(const std::vector<unique_fd>& profiles_fd,
                   const unique_fd& reference_profile_fd,
                   const std::vector<std::string>& dex_locations,
                   const std::vector<unique_fd>& apk_fds,
                   const unique_fd& output_fd) {
        AddArg("--dump-only");
        AddArg(StringPrintf("--dump-output-to-fd=%d", output_fd.get()));
        SetupArgs(profiles_fd,
                  reference_profile_fd,
                  apk_fds,
                  dex_locations,
                  /*copy_and_update=*/false,
                  /*store_aggregation_counters=*/false);
    }

    void Exec() {
        ExecVHelper::Exec(DexoptReturnCodes::kProfmanExec);
    }

  private:
    unique_fd reference_profile_fd_;
    std::vector<unique_fd> profiles_fd_;
    std::vector<unique_fd> apk_fds_;
};



// Decides if profile guided compilation is needed or not based on existing profiles.
// The location is the package name for primary apks or the dex path for secondary dex files.
// Returns true if there is enough information in the current profiles that makes it
// worth to recompile the given location.
// If the return value is true all the current profiles would have been merged into
// the reference profiles accessible with open_reference_profile().
static bool analyze_profiles(uid_t uid, const std::string& package_name,
        const std::string& location, bool is_secondary_dex) {
    std::vector<unique_fd> profiles_fd;
    unique_fd reference_profile_fd;
    open_profile_files(uid, package_name, location, is_secondary_dex,
        &profiles_fd, &reference_profile_fd);
    if (profiles_fd.empty() || (reference_profile_fd.get() < 0)) {
        // Skip profile guided compilation because no profiles were found.
        // Or if the reference profile info couldn't be opened.
        return false;
    }

    RunProfman profman_merge;
    profman_merge.SetupMerge(profiles_fd, reference_profile_fd);
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(uid);
        profman_merge.Exec();
    }
    /* parent */
    int return_code = wait_child(pid);
    bool need_to_compile = false;
    bool should_clear_current_profiles = false;
    bool should_clear_reference_profile = false;
    if (!WIFEXITED(return_code)) {
        LOG(WARNING) << "profman failed for location " << location << ": " << return_code;
    } else {
        return_code = WEXITSTATUS(return_code);
        switch (return_code) {
            case PROFMAN_BIN_RETURN_CODE_COMPILE:
                need_to_compile = true;
                should_clear_current_profiles = true;
                should_clear_reference_profile = false;
                break;
            case PROFMAN_BIN_RETURN_CODE_SKIP_COMPILATION:
                need_to_compile = false;
                should_clear_current_profiles = false;
                should_clear_reference_profile = false;
                break;
            case PROFMAN_BIN_RETURN_CODE_BAD_PROFILES:
                LOG(WARNING) << "Bad profiles for location " << location;
                need_to_compile = false;
                should_clear_current_profiles = true;
                should_clear_reference_profile = true;
                break;
            case PROFMAN_BIN_RETURN_CODE_ERROR_IO:  // fall-through
            case PROFMAN_BIN_RETURN_CODE_ERROR_LOCKING:
                // Temporary IO problem (e.g. locking). Ignore but log a warning.
                LOG(WARNING) << "IO error while reading profiles for location " << location;
                need_to_compile = false;
                should_clear_current_profiles = false;
                should_clear_reference_profile = false;
                break;
           default:
                // Unknown return code or error. Unlink profiles.
                LOG(WARNING) << "Unknown error code while processing profiles for location "
                        << location << ": " << return_code;
                need_to_compile = false;
                should_clear_current_profiles = true;
                should_clear_reference_profile = true;
                break;
        }
    }

    if (should_clear_current_profiles) {
        if (is_secondary_dex) {
            // For secondary dex files, the owning user is the current user.
            clear_current_profile(package_name, location, multiuser_get_user_id(uid),
                    is_secondary_dex);
        } else  {
            clear_primary_current_profiles(package_name, location);
        }
    }
    if (should_clear_reference_profile) {
        clear_reference_profile(package_name, location, is_secondary_dex);
    }
    return need_to_compile;
}

// Decides if profile guided compilation is needed or not based on existing profiles.
// The analysis is done for the primary apks of the given package.
// Returns true if there is enough information in the current profiles that makes it
// worth to recompile the package.
// If the return value is true all the current profiles would have been merged into
// the reference profiles accessible with open_reference_profile().
bool analyze_primary_profiles(uid_t uid, const std::string& package_name,
        const std::string& profile_name) {
    return analyze_profiles(uid, package_name, profile_name, /*is_secondary_dex*/false);
}

bool dump_profiles(int32_t uid, const std::string& pkgname, const std::string& profile_name,
        const std::string& code_path) {
    std::vector<unique_fd> profile_fds;
    unique_fd reference_profile_fd;
    std::string out_file_name = StringPrintf("/data/misc/profman/%s-%s.txt",
        pkgname.c_str(), profile_name.c_str());

    open_profile_files(uid, pkgname, profile_name, /*is_secondary_dex*/false,
            &profile_fds, &reference_profile_fd);

    const bool has_reference_profile = (reference_profile_fd.get() != -1);
    const bool has_profiles = !profile_fds.empty();

    if (!has_reference_profile && !has_profiles) {
        LOG(ERROR)  << "profman dump: no profiles to dump for " << pkgname;
        return false;
    }

    unique_fd output_fd(open(out_file_name.c_str(),
            O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644));
    if (fchmod(output_fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0) {
        LOG(ERROR) << "installd cannot chmod file for dump_profile" << out_file_name;
        return false;
    }

    std::vector<std::string> dex_locations;
    std::vector<unique_fd> apk_fds;
    unique_fd apk_fd(open(code_path.c_str(), O_RDONLY | O_NOFOLLOW));
    if (apk_fd == -1) {
        PLOG(ERROR) << "installd cannot open " << code_path.c_str();
        return false;
    }
    dex_locations.push_back(get_location_from_path(code_path.c_str()));
    apk_fds.push_back(std::move(apk_fd));


    RunProfman profman_dump;
    profman_dump.SetupDump(profile_fds, reference_profile_fd, dex_locations, apk_fds, output_fd);
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(uid);
        profman_dump.Exec();
    }
    /* parent */
    int return_code = wait_child(pid);
    if (!WIFEXITED(return_code)) {
        LOG(WARNING) << "profman failed for package " << pkgname << ": "
                << return_code;
        return false;
    }
    return true;
}

bool copy_system_profile(const std::string& system_profile,
        uid_t packageUid, const std::string& package_name, const std::string& profile_name) {
    unique_fd in_fd(open(system_profile.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    unique_fd out_fd(open_reference_profile(packageUid,
                     package_name,
                     profile_name,
                     /*read_write*/ true,
                     /*secondary*/ false));
    if (in_fd.get() < 0) {
        PLOG(WARNING) << "Could not open profile " << system_profile;
        return false;
    }
    if (out_fd.get() < 0) {
        PLOG(WARNING) << "Could not open profile " << package_name;
        return false;
    }

    // As a security measure we want to write the profile information with the reduced capabilities
    // of the package user id. So we fork and drop capabilities in the child.
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(packageUid);

        if (flock(out_fd.get(), LOCK_EX | LOCK_NB) != 0) {
            if (errno != EWOULDBLOCK) {
                PLOG(WARNING) << "Error locking profile " << package_name;
            }
            // This implies that the app owning this profile is running
            // (and has acquired the lock).
            //
            // The app never acquires the lock for the reference profiles of primary apks.
            // Only dex2oat from installd will do that. Since installd is single threaded
            // we should not see this case. Nevertheless be prepared for it.
            PLOG(WARNING) << "Failed to flock " << package_name;
            return false;
        }

        bool truncated = ftruncate(out_fd.get(), 0) == 0;
        if (!truncated) {
            PLOG(WARNING) << "Could not truncate " << package_name;
        }

        // Copy over data.
        static constexpr size_t kBufferSize = 4 * 1024;
        char buffer[kBufferSize];
        while (true) {
            ssize_t bytes = read(in_fd.get(), buffer, kBufferSize);
            if (bytes == 0) {
                break;
            }
            write(out_fd.get(), buffer, bytes);
        }
        if (flock(out_fd.get(), LOCK_UN) != 0) {
            PLOG(WARNING) << "Error unlocking profile " << package_name;
        }
        // Use _exit since we don't want to run the global destructors in the child.
        // b/62597429
        _exit(0);
    }
    /* parent */
    int return_code = wait_child(pid);
    return return_code == 0;
}

static std::string replace_file_extension(const std::string& oat_path, const std::string& new_ext) {
  // A standard dalvik-cache entry. Replace ".dex" with `new_ext`.
  if (EndsWith(oat_path, ".dex")) {
    std::string new_path = oat_path;
    new_path.replace(new_path.length() - strlen(".dex"), strlen(".dex"), new_ext);
    CHECK(EndsWith(new_path, new_ext));
    return new_path;
  }

  // An odex entry. Not that this may not be an extension, e.g., in the OTA
  // case (where the base name will have an extension for the B artifact).
  size_t odex_pos = oat_path.rfind(".odex");
  if (odex_pos != std::string::npos) {
    std::string new_path = oat_path;
    new_path.replace(odex_pos, strlen(".odex"), new_ext);
    CHECK_NE(new_path.find(new_ext), std::string::npos);
    return new_path;
  }

  // Don't know how to handle this.
  return "";
}

// Translate the given oat path to an art (app image) path. An empty string
// denotes an error.
static std::string create_image_filename(const std::string& oat_path) {
    return replace_file_extension(oat_path, ".art");
}

// Translate the given oat path to a vdex path. An empty string denotes an error.
static std::string create_vdex_filename(const std::string& oat_path) {
    return replace_file_extension(oat_path, ".vdex");
}

static int open_output_file(const char* file_name, bool recreate, int permissions) {
    int flags = O_RDWR | O_CREAT;
    if (recreate) {
        if (unlink(file_name) < 0) {
            if (errno != ENOENT) {
                PLOG(ERROR) << "open_output_file: Couldn't unlink " << file_name;
            }
        }
        flags |= O_EXCL;
    }
    return open(file_name, flags, permissions);
}

static bool set_permissions_and_ownership(
        int fd, bool is_public, int uid, const char* path, bool is_secondary_dex) {
    // Primary apks are owned by the system. Secondary dex files are owned by the app.
    int owning_uid = is_secondary_dex ? uid : AID_SYSTEM;
    if (fchmod(fd,
               S_IRUSR|S_IWUSR|S_IRGRP |
               (is_public ? S_IROTH : 0)) < 0) {
        ALOGE("installd cannot chmod '%s' during dexopt\n", path);
        return false;
    } else if (fchown(fd, owning_uid, uid) < 0) {
        ALOGE("installd cannot chown '%s' during dexopt\n", path);
        return false;
    }
    return true;
}

static bool IsOutputDalvikCache(const char* oat_dir) {
  // InstallerConnection.java (which invokes installd) transforms Java null arguments
  // into '!'. Play it safe by handling it both.
  // TODO: ensure we never get null.
  // TODO: pass a flag instead of inferring if the output is dalvik cache.
  return oat_dir == nullptr || oat_dir[0] == '!';
}

// Best-effort check whether we can fit the the path into our buffers.
// Note: the cache path will require an additional 5 bytes for ".swap", but we'll try to run
// without a swap file, if necessary. Reference profiles file also add an extra ".prof"
// extension to the cache path (5 bytes).
// TODO(calin): move away from char* buffers and PKG_PATH_MAX.
static bool validate_dex_path_size(const std::string& dex_path) {
    if (dex_path.size() >= (PKG_PATH_MAX - 8)) {
        LOG(ERROR) << "dex_path too long: " << dex_path;
        return false;
    }
    return true;
}

static bool create_oat_out_path(const char* apk_path, const char* instruction_set,
            const char* oat_dir, bool is_secondary_dex, /*out*/ char* out_oat_path) {
    if (!validate_dex_path_size(apk_path)) {
        return false;
    }

    if (!IsOutputDalvikCache(oat_dir)) {
        // Oat dirs for secondary dex files are already validated.
        if (!is_secondary_dex && validate_apk_path(oat_dir)) {
            ALOGE("cannot validate apk path with oat_dir '%s'\n", oat_dir);
            return false;
        }
        if (!calculate_oat_file_path(out_oat_path, oat_dir, apk_path, instruction_set)) {
            return false;
        }
    } else {
        if (!create_cache_path(out_oat_path, apk_path, instruction_set)) {
            return false;
        }
    }
    return true;
}

// Helper for fd management. This is similar to a unique_fd in that it closes the file descriptor
// on destruction. It will also run the given cleanup (unless told not to) after closing.
//
// Usage example:
//
//   Dex2oatFileWrapper file(open(...),
//                                                   [name]() {
//                                                       unlink(name.c_str());
//                                                   });
//   // Note: care needs to be taken about name, as it needs to have a lifetime longer than the
//            wrapper if captured as a reference.
//
//   if (file.get() == -1) {
//       // Error opening...
//   }
//
//   ...
//   if (error) {
//       // At this point, when the Dex2oatFileWrapper is destructed, the cleanup function will run
//       // and delete the file (after the fd is closed).
//       return -1;
//   }
//
//   (Success case)
//   file.SetCleanup(false);
//   // At this point, when the Dex2oatFileWrapper is destructed, the cleanup function will not run
//   // (leaving the file around; after the fd is closed).
//
class Dex2oatFileWrapper {
 public:
    Dex2oatFileWrapper() : value_(-1), cleanup_(), do_cleanup_(true), auto_close_(true) {
    }

    Dex2oatFileWrapper(int value, std::function<void ()> cleanup)
            : value_(value), cleanup_(cleanup), do_cleanup_(true), auto_close_(true) {}

    Dex2oatFileWrapper(Dex2oatFileWrapper&& other) {
        value_ = other.value_;
        cleanup_ = other.cleanup_;
        do_cleanup_ = other.do_cleanup_;
        auto_close_ = other.auto_close_;
        other.release();
    }

    Dex2oatFileWrapper& operator=(Dex2oatFileWrapper&& other) {
        value_ = other.value_;
        cleanup_ = other.cleanup_;
        do_cleanup_ = other.do_cleanup_;
        auto_close_ = other.auto_close_;
        other.release();
        return *this;
    }

    ~Dex2oatFileWrapper() {
        reset(-1);
    }

    int get() {
        return value_;
    }

    void SetCleanup(bool cleanup) {
        do_cleanup_ = cleanup;
    }

    void reset(int new_value) {
        if (auto_close_ && value_ >= 0) {
            close(value_);
        }
        if (do_cleanup_ && cleanup_ != nullptr) {
            cleanup_();
        }

        value_ = new_value;
    }

    void reset(int new_value, std::function<void ()> new_cleanup) {
        if (auto_close_ && value_ >= 0) {
            close(value_);
        }
        if (do_cleanup_ && cleanup_ != nullptr) {
            cleanup_();
        }

        value_ = new_value;
        cleanup_ = new_cleanup;
    }

    void DisableAutoClose() {
        auto_close_ = false;
    }

 private:
    void release() {
        value_ = -1;
        do_cleanup_ = false;
        cleanup_ = nullptr;
    }
    int value_;
    std::function<void ()> cleanup_;
    bool do_cleanup_;
    bool auto_close_;
};

// (re)Creates the app image if needed.
Dex2oatFileWrapper maybe_open_app_image(const char* out_oat_path,
        bool generate_app_image, bool is_public, int uid, bool is_secondary_dex) {

    // We don't create an image for secondary dex files.
    if (is_secondary_dex) {
        return Dex2oatFileWrapper();
    }

    const std::string image_path = create_image_filename(out_oat_path);
    if (image_path.empty()) {
        // Happens when the out_oat_path has an unknown extension.
        return Dex2oatFileWrapper();
    }

    // In case there is a stale image, remove it now. Ignore any error.
    unlink(image_path.c_str());

    // Not enabled, exit.
    if (!generate_app_image) {
        return Dex2oatFileWrapper();
    }
    std::string app_image_format = GetProperty("dalvik.vm.appimageformat", "");
    if (app_image_format.empty()) {
        return Dex2oatFileWrapper();
    }
    // Recreate is true since we do not want to modify a mapped image. If the app is
    // already running and we modify the image file, it can cause crashes (b/27493510).
    Dex2oatFileWrapper wrapper_fd(
            open_output_file(image_path.c_str(), true /*recreate*/, 0600 /*permissions*/),
            [image_path]() { unlink(image_path.c_str()); });
    if (wrapper_fd.get() < 0) {
        // Could not create application image file. Go on since we can compile without it.
        LOG(ERROR) << "installd could not create '" << image_path
                << "' for image file during dexopt";
         // If we have a valid image file path but no image fd, explicitly erase the image file.
        if (unlink(image_path.c_str()) < 0) {
            if (errno != ENOENT) {
                PLOG(ERROR) << "Couldn't unlink image file " << image_path;
            }
        }
    } else if (!set_permissions_and_ownership(
                wrapper_fd.get(), is_public, uid, image_path.c_str(), is_secondary_dex)) {
        ALOGE("installd cannot set owner '%s' for image during dexopt\n", image_path.c_str());
        wrapper_fd.reset(-1);
    }

    return wrapper_fd;
}

// Creates the dexopt swap file if necessary and return its fd.
// Returns -1 if there's no need for a swap or in case of errors.
unique_fd maybe_open_dexopt_swap_file(const char* out_oat_path) {
    if (!ShouldUseSwapFileForDexopt()) {
        return invalid_unique_fd();
    }
    auto swap_file_name = std::string(out_oat_path) + ".swap";
    unique_fd swap_fd(open_output_file(
            swap_file_name.c_str(), /*recreate*/true, /*permissions*/0600));
    if (swap_fd.get() < 0) {
        // Could not create swap file. Optimistically go on and hope that we can compile
        // without it.
        ALOGE("installd could not create '%s' for swap during dexopt\n", swap_file_name.c_str());
    } else {
        // Immediately unlink. We don't really want to hit flash.
        if (unlink(swap_file_name.c_str()) < 0) {
            PLOG(ERROR) << "Couldn't unlink swap file " << swap_file_name;
        }
    }
    return swap_fd;
}

// Opens the reference profiles if needed.
// Note that the reference profile might not exist so it's OK if the fd will be -1.
Dex2oatFileWrapper maybe_open_reference_profile(const std::string& pkgname,
        const std::string& dex_path, const char* profile_name, bool profile_guided,
        bool is_public, int uid, bool is_secondary_dex) {
    // If we are not profile guided compilation, or we are compiling system server
    // do not bother to open the profiles; we won't be using them.
    if (!profile_guided || (pkgname[0] == '*')) {
        return Dex2oatFileWrapper();
    }

    // If this is a secondary dex path which is public do not open the profile.
    // We cannot compile public secondary dex paths with profiles. That's because
    // it will expose how the dex files are used by their owner.
    //
    // Note that the PackageManager is responsible to set the is_public flag for
    // primary apks and we do not check it here. In some cases, e.g. when
    // compiling with a public profile from the .dm file the PackageManager will
    // set is_public toghether with the profile guided compilation.
    if (is_secondary_dex && is_public) {
        return Dex2oatFileWrapper();
    }

    // Open reference profile in read only mode as dex2oat does not get write permissions.
    std::string location;
    if (is_secondary_dex) {
        location = dex_path;
    } else {
        if (profile_name == nullptr) {
            // This path is taken for system server re-compilation lunched from ZygoteInit.
            return Dex2oatFileWrapper();
        } else {
            location = profile_name;
        }
    }
    unique_fd ufd = open_reference_profile(uid, pkgname, location, /*read_write*/false,
            is_secondary_dex);
    const auto& cleanup = [pkgname, location, is_secondary_dex]() {
        clear_reference_profile(pkgname, location, is_secondary_dex);
    };
    return Dex2oatFileWrapper(ufd.release(), cleanup);
}

// Opens the vdex files and assigns the input fd to in_vdex_wrapper_fd and the output fd to
// out_vdex_wrapper_fd. Returns true for success or false in case of errors.
bool open_vdex_files_for_dex2oat(const char* apk_path, const char* out_oat_path, int dexopt_needed,
        const char* instruction_set, bool is_public, int uid, bool is_secondary_dex,
        bool profile_guided, Dex2oatFileWrapper* in_vdex_wrapper_fd,
        Dex2oatFileWrapper* out_vdex_wrapper_fd) {
    CHECK(in_vdex_wrapper_fd != nullptr);
    CHECK(out_vdex_wrapper_fd != nullptr);
    // Open the existing VDEX. We do this before creating the new output VDEX, which will
    // unlink the old one.
    char in_odex_path[PKG_PATH_MAX];
    int dexopt_action = abs(dexopt_needed);
    bool is_odex_location = dexopt_needed < 0;
    std::string in_vdex_path_str;

    // Infer the name of the output VDEX.
    const std::string out_vdex_path_str = create_vdex_filename(out_oat_path);
    if (out_vdex_path_str.empty()) {
        return false;
    }

    bool update_vdex_in_place = false;
    if (dexopt_action != DEX2OAT_FROM_SCRATCH) {
        // Open the possibly existing vdex. If none exist, we pass -1 to dex2oat for input-vdex-fd.
        const char* path = nullptr;
        if (is_odex_location) {
            if (calculate_odex_file_path(in_odex_path, apk_path, instruction_set)) {
                path = in_odex_path;
            } else {
                ALOGE("installd cannot compute input vdex location for '%s'\n", apk_path);
                return false;
            }
        } else {
            path = out_oat_path;
        }
        in_vdex_path_str = create_vdex_filename(path);
        if (in_vdex_path_str.empty()) {
            ALOGE("installd cannot compute input vdex location for '%s'\n", path);
            return false;
        }
        // We can update in place when all these conditions are met:
        // 1) The vdex location to write to is the same as the vdex location to read (vdex files
        //    on /system typically cannot be updated in place).
        // 2) We dex2oat due to boot image change, because we then know the existing vdex file
        //    cannot be currently used by a running process.
        // 3) We are not doing a profile guided compilation, because dexlayout requires two
        //    different vdex files to operate.
        update_vdex_in_place =
            (in_vdex_path_str == out_vdex_path_str) &&
            (dexopt_action == DEX2OAT_FOR_BOOT_IMAGE) &&
            !profile_guided;
        if (update_vdex_in_place) {
            // Open the file read-write to be able to update it.
            in_vdex_wrapper_fd->reset(open(in_vdex_path_str.c_str(), O_RDWR, 0));
            if (in_vdex_wrapper_fd->get() == -1) {
                // If we failed to open the file, we cannot update it in place.
                update_vdex_in_place = false;
            }
        } else {
            in_vdex_wrapper_fd->reset(open(in_vdex_path_str.c_str(), O_RDONLY, 0));
        }
    }

    // If we are updating the vdex in place, we do not need to recreate a vdex,
    // and can use the same existing one.
    if (update_vdex_in_place) {
        // We unlink the file in case the invocation of dex2oat fails, to ensure we don't
        // have bogus stale vdex files.
        out_vdex_wrapper_fd->reset(
              in_vdex_wrapper_fd->get(),
              [out_vdex_path_str]() { unlink(out_vdex_path_str.c_str()); });
        // Disable auto close for the in wrapper fd (it will be done when destructing the out
        // wrapper).
        in_vdex_wrapper_fd->DisableAutoClose();
    } else {
        out_vdex_wrapper_fd->reset(
              open_output_file(out_vdex_path_str.c_str(), /*recreate*/true, /*permissions*/0644),
              [out_vdex_path_str]() { unlink(out_vdex_path_str.c_str()); });
        if (out_vdex_wrapper_fd->get() < 0) {
            ALOGE("installd cannot open vdex'%s' during dexopt\n", out_vdex_path_str.c_str());
            return false;
        }
    }
    if (!set_permissions_and_ownership(out_vdex_wrapper_fd->get(), is_public, uid,
            out_vdex_path_str.c_str(), is_secondary_dex)) {
        ALOGE("installd cannot set owner '%s' for vdex during dexopt\n", out_vdex_path_str.c_str());
        return false;
    }

    // If we got here we successfully opened the vdex files.
    return true;
}

// Opens the output oat file for the given apk.
// If successful it stores the output path into out_oat_path and returns true.
Dex2oatFileWrapper open_oat_out_file(const char* apk_path, const char* oat_dir,
        bool is_public, int uid, const char* instruction_set, bool is_secondary_dex,
        char* out_oat_path) {
    if (!create_oat_out_path(apk_path, instruction_set, oat_dir, is_secondary_dex, out_oat_path)) {
        return Dex2oatFileWrapper();
    }
    const std::string out_oat_path_str(out_oat_path);
    Dex2oatFileWrapper wrapper_fd(
            open_output_file(out_oat_path, /*recreate*/true, /*permissions*/0644),
            [out_oat_path_str]() { unlink(out_oat_path_str.c_str()); });
    if (wrapper_fd.get() < 0) {
        PLOG(ERROR) << "installd cannot open output during dexopt" <<  out_oat_path;
    } else if (!set_permissions_and_ownership(
                wrapper_fd.get(), is_public, uid, out_oat_path, is_secondary_dex)) {
        ALOGE("installd cannot set owner '%s' for output during dexopt\n", out_oat_path);
        wrapper_fd.reset(-1);
    }
    return wrapper_fd;
}

// Creates RDONLY fds for oat and vdex files, if exist.
// Returns false if it fails to create oat out path for the given apk path.
// Note that the method returns true even if the files could not be opened.
bool maybe_open_oat_and_vdex_file(const std::string& apk_path,
                                  const std::string& oat_dir,
                                  const std::string& instruction_set,
                                  bool is_secondary_dex,
                                  unique_fd* oat_file_fd,
                                  unique_fd* vdex_file_fd) {
    char oat_path[PKG_PATH_MAX];
    if (!create_oat_out_path(apk_path.c_str(),
                             instruction_set.c_str(),
                             oat_dir.c_str(),
                             is_secondary_dex,
                             oat_path)) {
        LOG(ERROR) << "Could not create oat out path for "
                << apk_path << " with oat dir " << oat_dir;
        return false;
    }
    oat_file_fd->reset(open(oat_path, O_RDONLY));
    if (oat_file_fd->get() < 0) {
        PLOG(INFO) << "installd cannot open oat file during dexopt" <<  oat_path;
    }

    std::string vdex_filename = create_vdex_filename(oat_path);
    vdex_file_fd->reset(open(vdex_filename.c_str(), O_RDONLY));
    if (vdex_file_fd->get() < 0) {
        PLOG(INFO) << "installd cannot open vdex file during dexopt" <<  vdex_filename;
    }

    return true;
}

// Updates the access times of out_oat_path based on those from apk_path.
void update_out_oat_access_times(const char* apk_path, const char* out_oat_path) {
    struct stat input_stat;
    memset(&input_stat, 0, sizeof(input_stat));
    if (stat(apk_path, &input_stat) != 0) {
        PLOG(ERROR) << "Could not stat " << apk_path << " during dexopt";
        return;
    }

    struct utimbuf ut;
    ut.actime = input_stat.st_atime;
    ut.modtime = input_stat.st_mtime;
    if (utime(out_oat_path, &ut) != 0) {
        PLOG(WARNING) << "Could not update access times for " << apk_path << " during dexopt";
    }
}

// Runs (execv) dexoptanalyzer on the given arguments.
// The analyzer will check if the dex_file needs to be (re)compiled to match the compiler_filter.
// If this is for a profile guided compilation, profile_was_updated will tell whether or not
// the profile has changed.
class RunDexoptAnalyzer : public ExecVHelper {
 public:
    RunDexoptAnalyzer(const std::string& dex_file,
                      int vdex_fd,
                      int oat_fd,
                      int zip_fd,
                      const std::string& instruction_set,
                      const std::string& compiler_filter,
                      bool profile_was_updated,
                      bool downgrade,
                      const char* class_loader_context,
                      const std::string& class_loader_context_fds) {
        CHECK_GE(zip_fd, 0);

        // We always run the analyzer in the background job.
        const char* dexoptanalyzer_bin = select_execution_binary(
             kDexoptanalyzerPath, kDexoptanalyzerDebugPath, /*background_job_compile=*/ true);

        std::string dex_file_arg = "--dex-file=" + dex_file;
        std::string oat_fd_arg = "--oat-fd=" + std::to_string(oat_fd);
        std::string vdex_fd_arg = "--vdex-fd=" + std::to_string(vdex_fd);
        std::string zip_fd_arg = "--zip-fd=" + std::to_string(zip_fd);
        std::string isa_arg = "--isa=" + instruction_set;
        std::string compiler_filter_arg = "--compiler-filter=" + compiler_filter;
        const char* assume_profile_changed = "--assume-profile-changed";
        const char* downgrade_flag = "--downgrade";
        std::string class_loader_context_arg = "--class-loader-context=";
        if (class_loader_context != nullptr) {
            class_loader_context_arg += class_loader_context;
        }
        std::string class_loader_context_fds_arg = "--class-loader-context-fds=";
        if (!class_loader_context_fds.empty()) {
            class_loader_context_fds_arg += class_loader_context_fds;
        }

        // program name, dex file, isa, filter
        AddArg(dex_file_arg);
        AddArg(isa_arg);
        AddArg(compiler_filter_arg);
        if (oat_fd >= 0) {
            AddArg(oat_fd_arg);
        }
        if (vdex_fd >= 0) {
            AddArg(vdex_fd_arg);
        }
        AddArg(zip_fd_arg);
        if (profile_was_updated) {
            AddArg(assume_profile_changed);
        }
        if (downgrade) {
            AddArg(downgrade_flag);
        }
        if (class_loader_context != nullptr) {
            AddArg(class_loader_context_arg);
            if (!class_loader_context_fds.empty()) {
                AddArg(class_loader_context_fds_arg);
            }
        }

        PrepareArgs(dexoptanalyzer_bin);
    }

    // Dexoptanalyzer mode which flattens the given class loader context and
    // prints a list of its dex files in that flattened order.
    RunDexoptAnalyzer(const char* class_loader_context) {
        CHECK(class_loader_context != nullptr);

        // We always run the analyzer in the background job.
        const char* dexoptanalyzer_bin = select_execution_binary(
             kDexoptanalyzerPath, kDexoptanalyzerDebugPath, /*background_job_compile=*/ true);

        AddArg("--flatten-class-loader-context");
        AddArg(std::string("--class-loader-context=") + class_loader_context);
        PrepareArgs(dexoptanalyzer_bin);
    }
};

// Prepares the oat dir for the secondary dex files.
static bool prepare_secondary_dex_oat_dir(const std::string& dex_path, int uid,
        const char* instruction_set) {
    unsigned long dirIndex = dex_path.rfind('/');
    if (dirIndex == std::string::npos) {
        LOG(ERROR ) << "Unexpected dir structure for secondary dex " << dex_path;
        return false;
    }
    std::string dex_dir = dex_path.substr(0, dirIndex);

    // Create oat file output directory.
    mode_t oat_dir_mode = S_IRWXU | S_IRWXG | S_IXOTH;
    if (prepare_app_cache_dir(dex_dir, "oat", oat_dir_mode, uid, uid) != 0) {
        LOG(ERROR) << "Could not prepare oat dir for secondary dex: " << dex_path;
        return false;
    }

    char oat_dir[PKG_PATH_MAX];
    snprintf(oat_dir, PKG_PATH_MAX, "%s/oat", dex_dir.c_str());

    if (prepare_app_cache_dir(oat_dir, instruction_set, oat_dir_mode, uid, uid) != 0) {
        LOG(ERROR) << "Could not prepare oat/isa dir for secondary dex: " << dex_path;
        return false;
    }

    return true;
}

// Return codes for identifying the reason why dexoptanalyzer was not invoked when processing
// secondary dex files. This return codes are returned by the child process created for
// analyzing secondary dex files in process_secondary_dex_dexopt.

enum DexoptAnalyzerSkipCodes {
  // The dexoptanalyzer was not invoked because of validation or IO errors.
  // Specific errors are encoded in the name.
  kSecondaryDexDexoptAnalyzerSkippedValidatePath = 200,
  kSecondaryDexDexoptAnalyzerSkippedOpenZip = 201,
  kSecondaryDexDexoptAnalyzerSkippedPrepareDir = 202,
  kSecondaryDexDexoptAnalyzerSkippedOpenOutput = 203,
  kSecondaryDexDexoptAnalyzerSkippedFailExec = 204,
  // The dexoptanalyzer was not invoked because the dex file does not exist anymore.
  kSecondaryDexDexoptAnalyzerSkippedNoFile = 205,
};

// Verifies the result of analyzing secondary dex files from process_secondary_dex_dexopt.
// If the result is valid returns true and sets dexopt_needed_out to a valid value.
// Returns false for errors or unexpected result values.
// The result is expected to be either one of SECONDARY_DEX_* codes or a valid exit code
// of dexoptanalyzer.
static bool process_secondary_dexoptanalyzer_result(const std::string& dex_path, int result,
            int* dexopt_needed_out, std::string* error_msg) {
    // The result values are defined in dexoptanalyzer.
    switch (result) {
        case 0:  // dexoptanalyzer: no_dexopt_needed
            *dexopt_needed_out = NO_DEXOPT_NEEDED; return true;
        case 1:  // dexoptanalyzer: dex2oat_from_scratch
            *dexopt_needed_out = DEX2OAT_FROM_SCRATCH; return true;
        case 4:  // dexoptanalyzer: dex2oat_for_bootimage_odex
            *dexopt_needed_out = -DEX2OAT_FOR_BOOT_IMAGE; return true;
        case 5:  // dexoptanalyzer: dex2oat_for_filter_odex
            *dexopt_needed_out = -DEX2OAT_FOR_FILTER; return true;
        case 2:  // dexoptanalyzer: dex2oat_for_bootimage_oat
        case 3:  // dexoptanalyzer: dex2oat_for_filter_oat
            *error_msg = StringPrintf("Dexoptanalyzer return the status of an oat file."
                                      " Expected odex file status for secondary dex %s"
                                      " : dexoptanalyzer result=%d",
                                      dex_path.c_str(),
                                      result);
            return false;
    }

    // Use a second switch for enum switch-case analysis.
    switch (static_cast<DexoptAnalyzerSkipCodes>(result)) {
        case kSecondaryDexDexoptAnalyzerSkippedNoFile:
            // If the file does not exist there's no need for dexopt.
            *dexopt_needed_out = NO_DEXOPT_NEEDED;
            return true;

        case kSecondaryDexDexoptAnalyzerSkippedValidatePath:
            *error_msg = "Dexoptanalyzer path validation failed";
            return false;
        case kSecondaryDexDexoptAnalyzerSkippedOpenZip:
            *error_msg = "Dexoptanalyzer open zip failed";
            return false;
        case kSecondaryDexDexoptAnalyzerSkippedPrepareDir:
            *error_msg = "Dexoptanalyzer dir preparation failed";
            return false;
        case kSecondaryDexDexoptAnalyzerSkippedOpenOutput:
            *error_msg = "Dexoptanalyzer open output failed";
            return false;
        case kSecondaryDexDexoptAnalyzerSkippedFailExec:
            *error_msg = "Dexoptanalyzer failed to execute";
            return false;
    }

    *error_msg = StringPrintf("Unexpected result from analyzing secondary dex %s result=%d",
                              dex_path.c_str(),
                              result);
    return false;
}

enum SecondaryDexAccess {
    kSecondaryDexAccessReadOk = 0,
    kSecondaryDexAccessDoesNotExist = 1,
    kSecondaryDexAccessPermissionError = 2,
    kSecondaryDexAccessIOError = 3
};

static SecondaryDexAccess check_secondary_dex_access(const std::string& dex_path) {
    // Check if the path exists and can be read. If not, there's nothing to do.
    if (access(dex_path.c_str(), R_OK) == 0) {
        return kSecondaryDexAccessReadOk;
    } else {
        if (errno == ENOENT) {
            LOG(INFO) << "Secondary dex does not exist: " <<  dex_path;
            return kSecondaryDexAccessDoesNotExist;
        } else {
            PLOG(ERROR) << "Could not access secondary dex " << dex_path;
            return errno == EACCES
                ? kSecondaryDexAccessPermissionError
                : kSecondaryDexAccessIOError;
        }
    }
}

static bool is_file_public(const std::string& filename) {
    struct stat file_stat;
    if (stat(filename.c_str(), &file_stat) == 0) {
        return (file_stat.st_mode & S_IROTH) != 0;
    }
    return false;
}

// Create the oat file structure for the secondary dex 'dex_path' and assign
// the individual path component to the 'out_' parameters.
static bool create_secondary_dex_oat_layout(const std::string& dex_path, const std::string& isa,
        char* out_oat_dir, char* out_oat_isa_dir, char* out_oat_path, std::string* error_msg) {
    size_t dirIndex = dex_path.rfind('/');
    if (dirIndex == std::string::npos) {
        *error_msg = std::string("Unexpected dir structure for dex file ").append(dex_path);
        return false;
    }
    // TODO(calin): we have similar computations in at lest 3 other places
    // (InstalldNativeService, otapropt and dexopt). Unify them and get rid of snprintf by
    // using string append.
    std::string apk_dir = dex_path.substr(0, dirIndex);
    snprintf(out_oat_dir, PKG_PATH_MAX, "%s/oat", apk_dir.c_str());
    snprintf(out_oat_isa_dir, PKG_PATH_MAX, "%s/%s", out_oat_dir, isa.c_str());

    if (!create_oat_out_path(dex_path.c_str(), isa.c_str(), out_oat_dir,
            /*is_secondary_dex*/true, out_oat_path)) {
        *error_msg = std::string("Could not create oat path for secondary dex ").append(dex_path);
        return false;
    }
    return true;
}

// Validate that the dexopt_flags contain a valid storage flag and convert that to an installd
// recognized storage flags (FLAG_STORAGE_CE or FLAG_STORAGE_DE).
static bool validate_dexopt_storage_flags(int dexopt_flags,
                                          int* out_storage_flag,
                                          std::string* error_msg) {
    if ((dexopt_flags & DEXOPT_STORAGE_CE) != 0) {
        *out_storage_flag = FLAG_STORAGE_CE;
        if ((dexopt_flags & DEXOPT_STORAGE_DE) != 0) {
            *error_msg = "Ambiguous secondary dex storage flag. Both, CE and DE, flags are set";
            return false;
        }
    } else if ((dexopt_flags & DEXOPT_STORAGE_DE) != 0) {
        *out_storage_flag = FLAG_STORAGE_DE;
    } else {
        *error_msg = "Secondary dex storage flag must be set";
        return false;
    }
    return true;
}

static bool get_class_loader_context_dex_paths(const char* class_loader_context, int uid,
        /* out */ std::vector<std::string>* context_dex_paths) {
    if (class_loader_context == nullptr) {
      return true;
    }

    LOG(DEBUG) << "Getting dex paths for context " << class_loader_context;

    // Pipe to get the hash result back from our child process.
    unique_fd pipe_read, pipe_write;
    if (!Pipe(&pipe_read, &pipe_write)) {
        PLOG(ERROR) << "Failed to create pipe";
        return false;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // child -- drop privileges before continuing.
        drop_capabilities(uid);

        // Route stdout to `pipe_write`
        while ((dup2(pipe_write, STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        pipe_write.reset();
        pipe_read.reset();

        RunDexoptAnalyzer run_dexopt_analyzer(class_loader_context);
        run_dexopt_analyzer.Exec(kSecondaryDexDexoptAnalyzerSkippedFailExec);
    }

    /* parent */
    pipe_write.reset();

    std::string str_dex_paths;
    if (!ReadFdToString(pipe_read, &str_dex_paths)) {
        PLOG(ERROR) << "Failed to read from pipe";
        return false;
    }
    pipe_read.reset();

    int return_code = wait_child(pid);
    if (!WIFEXITED(return_code)) {
        PLOG(ERROR) << "Error waiting for child dexoptanalyzer process";
        return false;
    }

    constexpr int kFlattenClassLoaderContextSuccess = 50;
    return_code = WEXITSTATUS(return_code);
    if (return_code != kFlattenClassLoaderContextSuccess) {
        LOG(ERROR) << "Dexoptanalyzer could not flatten class loader context, code=" << return_code;
        return false;
    }

    if (!str_dex_paths.empty()) {
        *context_dex_paths = android::base::Split(str_dex_paths, ":");
    }
    return true;
}

static int open_dex_paths(const std::vector<std::string>& dex_paths,
        /* out */ std::vector<unique_fd>* zip_fds, /* out */ std::string* error_msg) {
    for (const std::string& dex_path : dex_paths) {
        zip_fds->emplace_back(open(dex_path.c_str(), O_RDONLY));
        if (zip_fds->back().get() < 0) {
            *error_msg = StringPrintf(
                    "installd cannot open '%s' for input during dexopt", dex_path.c_str());
            if (errno == ENOENT) {
                return kSecondaryDexDexoptAnalyzerSkippedNoFile;
            } else {
                return kSecondaryDexDexoptAnalyzerSkippedOpenZip;
            }
        }
    }
    return 0;
}

static std::string join_fds(const std::vector<unique_fd>& fds) {
    std::stringstream ss;
    bool is_first = true;
    for (const unique_fd& fd : fds) {
        if (is_first) {
            is_first = false;
        } else {
            ss << ":";
        }
        ss << fd.get();
    }
    return ss.str();
}

// Processes the dex_path as a secondary dex files and return true if the path dex file should
// be compiled. Returns false for errors (logged) or true if the secondary dex path was process
// successfully.
// When returning true, the output parameters will be:
//   - is_public_out: whether or not the oat file should not be made public
//   - dexopt_needed_out: valid OatFileAsssitant::DexOptNeeded
//   - oat_dir_out: the oat dir path where the oat file should be stored
static bool process_secondary_dex_dexopt(const std::string& dex_path, const char* pkgname,
        int dexopt_flags, const char* volume_uuid, int uid, const char* instruction_set,
        const char* compiler_filter, bool* is_public_out, int* dexopt_needed_out,
        std::string* oat_dir_out, bool downgrade, const char* class_loader_context,
        const std::vector<std::string>& context_dex_paths, /* out */ std::string* error_msg) {
    LOG(DEBUG) << "Processing secondary dex path " << dex_path;
    int storage_flag;
    if (!validate_dexopt_storage_flags(dexopt_flags, &storage_flag, error_msg)) {
        LOG(ERROR) << *error_msg;
        return false;
    }
    // Compute the oat dir as it's not easy to extract it from the child computation.
    char oat_path[PKG_PATH_MAX];
    char oat_dir[PKG_PATH_MAX];
    char oat_isa_dir[PKG_PATH_MAX];
    if (!create_secondary_dex_oat_layout(
            dex_path, instruction_set, oat_dir, oat_isa_dir, oat_path, error_msg)) {
        LOG(ERROR) << "Could not create secondary odex layout: " << *error_msg;
        return false;
    }
    oat_dir_out->assign(oat_dir);

    pid_t pid = fork();
    if (pid == 0) {
        // child -- drop privileges before continuing.
        drop_capabilities(uid);

        // Validate the path structure.
        if (!validate_secondary_dex_path(pkgname, dex_path, volume_uuid, uid, storage_flag)) {
            LOG(ERROR) << "Could not validate secondary dex path " << dex_path;
            _exit(kSecondaryDexDexoptAnalyzerSkippedValidatePath);
        }

        // Open the dex file.
        unique_fd zip_fd;
        zip_fd.reset(open(dex_path.c_str(), O_RDONLY));
        if (zip_fd.get() < 0) {
            if (errno == ENOENT) {
                _exit(kSecondaryDexDexoptAnalyzerSkippedNoFile);
            } else {
                _exit(kSecondaryDexDexoptAnalyzerSkippedOpenZip);
            }
        }

        // Open class loader context dex files.
        std::vector<unique_fd> context_zip_fds;
        int open_dex_paths_rc = open_dex_paths(context_dex_paths, &context_zip_fds, error_msg);
        if (open_dex_paths_rc != 0) {
            _exit(open_dex_paths_rc);
        }

        // Prepare the oat directories.
        if (!prepare_secondary_dex_oat_dir(dex_path, uid, instruction_set)) {
            _exit(kSecondaryDexDexoptAnalyzerSkippedPrepareDir);
        }

        // Open the vdex/oat files if any.
        unique_fd oat_file_fd;
        unique_fd vdex_file_fd;
        if (!maybe_open_oat_and_vdex_file(dex_path,
                                          *oat_dir_out,
                                          instruction_set,
                                          true /* is_secondary_dex */,
                                          &oat_file_fd,
                                          &vdex_file_fd)) {
            _exit(kSecondaryDexDexoptAnalyzerSkippedOpenOutput);
        }

        // Analyze profiles.
        bool profile_was_updated = analyze_profiles(uid, pkgname, dex_path,
                /*is_secondary_dex*/true);

        // Run dexoptanalyzer to get dexopt_needed code. This is not expected to return.
        // Note that we do not do it before the fork since opening the files is required to happen
        // after forking.
        RunDexoptAnalyzer run_dexopt_analyzer(dex_path,
                                              vdex_file_fd.get(),
                                              oat_file_fd.get(),
                                              zip_fd.get(),
                                              instruction_set,
                                              compiler_filter, profile_was_updated,
                                              downgrade,
                                              class_loader_context,
                                              join_fds(context_zip_fds));
        run_dexopt_analyzer.Exec(kSecondaryDexDexoptAnalyzerSkippedFailExec);
    }

    /* parent */
    int result = wait_child(pid);
    if (!WIFEXITED(result)) {
        *error_msg = StringPrintf("dexoptanalyzer failed for path %s: 0x%04x",
                                  dex_path.c_str(),
                                  result);
        LOG(ERROR) << *error_msg;
        return false;
    }
    result = WEXITSTATUS(result);
    // Check that we successfully executed dexoptanalyzer.
    bool success = process_secondary_dexoptanalyzer_result(dex_path,
                                                           result,
                                                           dexopt_needed_out,
                                                           error_msg);
    if (!success) {
        LOG(ERROR) << *error_msg;
    }

    LOG(DEBUG) << "Processed secondary dex file " << dex_path << " result=" << result;

    // Run dexopt only if needed or forced.
    // Note that dexoptanalyzer is executed even if force compilation is enabled (because it
    // makes the code simpler; force compilation is only needed during tests).
    if (success &&
        (result != kSecondaryDexDexoptAnalyzerSkippedNoFile) &&
        ((dexopt_flags & DEXOPT_FORCE) != 0)) {
        *dexopt_needed_out = DEX2OAT_FROM_SCRATCH;
    }

    // Check if we should make the oat file public.
    // Note that if the dex file is not public the compiled code cannot be made public.
    // It is ok to check this flag outside in the parent process.
    *is_public_out = ((dexopt_flags & DEXOPT_PUBLIC) != 0) && is_file_public(dex_path);

    return success;
}

static std::string format_dexopt_error(int status, const char* dex_path) {
  if (WIFEXITED(status)) {
    int int_code = WEXITSTATUS(status);
    const char* code_name = get_return_code_name(static_cast<DexoptReturnCodes>(int_code));
    if (code_name != nullptr) {
      return StringPrintf("Dex2oat invocation for %s failed: %s", dex_path, code_name);
    }
  }
  return StringPrintf("Dex2oat invocation for %s failed with 0x%04x", dex_path, status);
}

int dexopt(const char* dex_path, uid_t uid, const char* pkgname, const char* instruction_set,
        int dexopt_needed, const char* oat_dir, int dexopt_flags, const char* compiler_filter,
        const char* volume_uuid, const char* class_loader_context, const char* se_info,
        bool downgrade, int target_sdk_version, const char* profile_name,
        const char* dex_metadata_path, const char* compilation_reason, std::string* error_msg) {
    CHECK(pkgname != nullptr);
    CHECK(pkgname[0] != 0);
    CHECK(error_msg != nullptr);
    CHECK_EQ(dexopt_flags & ~DEXOPT_MASK, 0)
        << "dexopt flags contains unknown fields: " << dexopt_flags;

    if (!validate_dex_path_size(dex_path)) {
        *error_msg = StringPrintf("Failed to validate %s", dex_path);
        return -1;
    }

    if (class_loader_context != nullptr && strlen(class_loader_context) > PKG_PATH_MAX) {
        *error_msg = StringPrintf("Class loader context exceeds the allowed size: %s",
                                  class_loader_context);
        LOG(ERROR) << *error_msg;
        return -1;
    }

    bool is_public = (dexopt_flags & DEXOPT_PUBLIC) != 0;
    bool debuggable = (dexopt_flags & DEXOPT_DEBUGGABLE) != 0;
    bool boot_complete = (dexopt_flags & DEXOPT_BOOTCOMPLETE) != 0;
    bool profile_guided = (dexopt_flags & DEXOPT_PROFILE_GUIDED) != 0;
    bool is_secondary_dex = (dexopt_flags & DEXOPT_SECONDARY_DEX) != 0;
    bool background_job_compile = (dexopt_flags & DEXOPT_IDLE_BACKGROUND_JOB) != 0;
    bool enable_hidden_api_checks = (dexopt_flags & DEXOPT_ENABLE_HIDDEN_API_CHECKS) != 0;
    bool generate_compact_dex = (dexopt_flags & DEXOPT_GENERATE_COMPACT_DEX) != 0;
    bool generate_app_image = (dexopt_flags & DEXOPT_GENERATE_APP_IMAGE) != 0;

    // Check if we're dealing with a secondary dex file and if we need to compile it.
    std::string oat_dir_str;
    std::vector<std::string> context_dex_paths;
    if (is_secondary_dex) {
        if (!get_class_loader_context_dex_paths(class_loader_context, uid, &context_dex_paths)) {
            *error_msg = "Failed acquiring context dex paths";
            return -1;  // We had an error, logged in the process method.
        }

        if (process_secondary_dex_dexopt(dex_path, pkgname, dexopt_flags, volume_uuid, uid,
                instruction_set, compiler_filter, &is_public, &dexopt_needed, &oat_dir_str,
                downgrade, class_loader_context, context_dex_paths, error_msg)) {
            oat_dir = oat_dir_str.c_str();
            if (dexopt_needed == NO_DEXOPT_NEEDED) {
                return 0;  // Nothing to do, report success.
            }
        } else {
            if (error_msg->empty()) {  // TODO: Make this a CHECK.
                *error_msg = "Failed processing secondary.";
            }
            return -1;  // We had an error, logged in the process method.
        }
    } else {
        // Currently these flags are only used for secondary dex files.
        // Verify that they are not set for primary apks.
        CHECK((dexopt_flags & DEXOPT_STORAGE_CE) == 0);
        CHECK((dexopt_flags & DEXOPT_STORAGE_DE) == 0);
    }

    // Open the input file.
    unique_fd input_fd(open(dex_path, O_RDONLY, 0));
    if (input_fd.get() < 0) {
        *error_msg = StringPrintf("installd cannot open '%s' for input during dexopt", dex_path);
        LOG(ERROR) << *error_msg;
        return -1;
    }

    // Open class loader context dex files.
    std::vector<unique_fd> context_input_fds;
    if (open_dex_paths(context_dex_paths, &context_input_fds, error_msg) != 0) {
        LOG(ERROR) << *error_msg;
        return -1;
    }

    // Create the output OAT file.
    char out_oat_path[PKG_PATH_MAX];
    Dex2oatFileWrapper out_oat_fd = open_oat_out_file(dex_path, oat_dir, is_public, uid,
            instruction_set, is_secondary_dex, out_oat_path);
    if (out_oat_fd.get() < 0) {
        *error_msg = "Could not open out oat file.";
        return -1;
    }

    // Open vdex files.
    Dex2oatFileWrapper in_vdex_fd;
    Dex2oatFileWrapper out_vdex_fd;
    if (!open_vdex_files_for_dex2oat(dex_path, out_oat_path, dexopt_needed, instruction_set,
            is_public, uid, is_secondary_dex, profile_guided, &in_vdex_fd, &out_vdex_fd)) {
        *error_msg = "Could not open vdex files.";
        return -1;
    }

    // Ensure that the oat dir and the compiler artifacts of secondary dex files have the correct
    // selinux context (we generate them on the fly during the dexopt invocation and they don't
    // fully inherit their parent context).
    // Note that for primary apk the oat files are created before, in a separate installd
    // call which also does the restorecon. TODO(calin): unify the paths.
    if (is_secondary_dex) {
        if (selinux_android_restorecon_pkgdir(oat_dir, se_info, uid,
                SELINUX_ANDROID_RESTORECON_RECURSE)) {
            *error_msg = std::string("Failed to restorecon ").append(oat_dir);
            LOG(ERROR) << *error_msg;
            return -1;
        }
    }

    // Create a swap file if necessary.
    unique_fd swap_fd = maybe_open_dexopt_swap_file(out_oat_path);

    // Create the app image file if needed.
    Dex2oatFileWrapper image_fd = maybe_open_app_image(
            out_oat_path, generate_app_image, is_public, uid, is_secondary_dex);

    // Open the reference profile if needed.
    Dex2oatFileWrapper reference_profile_fd = maybe_open_reference_profile(
            pkgname, dex_path, profile_name, profile_guided, is_public, uid, is_secondary_dex);

    unique_fd dex_metadata_fd;
    if (dex_metadata_path != nullptr) {
        dex_metadata_fd.reset(TEMP_FAILURE_RETRY(open(dex_metadata_path, O_RDONLY | O_NOFOLLOW)));
        if (dex_metadata_fd.get() < 0) {
            PLOG(ERROR) << "Failed to open dex metadata file " << dex_metadata_path;
        }
    }

    LOG(VERBOSE) << "DexInv: --- BEGIN '" << dex_path << "' ---";

    RunDex2Oat runner(input_fd.get(),
                      out_oat_fd.get(),
                      in_vdex_fd.get(),
                      out_vdex_fd.get(),
                      image_fd.get(),
                      dex_path,
                      out_oat_path,
                      swap_fd.get(),
                      instruction_set,
                      compiler_filter,
                      debuggable,
                      boot_complete,
                      background_job_compile,
                      reference_profile_fd.get(),
                      class_loader_context,
                      join_fds(context_input_fds),
                      target_sdk_version,
                      enable_hidden_api_checks,
                      generate_compact_dex,
                      dex_metadata_fd.get(),
                      compilation_reason);

    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(uid);

        SetDex2OatScheduling(boot_complete);
        if (flock(out_oat_fd.get(), LOCK_EX | LOCK_NB) != 0) {
            PLOG(ERROR) << "flock(" << out_oat_path << ") failed";
            _exit(DexoptReturnCodes::kFlock);
        }

        runner.Exec(DexoptReturnCodes::kDex2oatExec);
    } else {
        int res = wait_child(pid);
        if (res == 0) {
            LOG(VERBOSE) << "DexInv: --- END '" << dex_path << "' (success) ---";
        } else {
            LOG(VERBOSE) << "DexInv: --- END '" << dex_path << "' --- status=0x"
                         << std::hex << std::setw(4) << res << ", process failed";
            *error_msg = format_dexopt_error(res, dex_path);
            return res;
        }
    }

    update_out_oat_access_times(dex_path, out_oat_path);

    // We've been successful, don't delete output.
    out_oat_fd.SetCleanup(false);
    out_vdex_fd.SetCleanup(false);
    image_fd.SetCleanup(false);
    reference_profile_fd.SetCleanup(false);

    return 0;
}

// Try to remove the given directory. Log an error if the directory exists
// and is empty but could not be removed.
static bool rmdir_if_empty(const char* dir) {
    if (rmdir(dir) == 0) {
        return true;
    }
    if (errno == ENOENT || errno == ENOTEMPTY) {
        return true;
    }
    PLOG(ERROR) << "Failed to remove dir: " << dir;
    return false;
}

// Try to unlink the given file. Log an error if the file exists and could not
// be unlinked.
static bool unlink_if_exists(const std::string& file) {
    if (unlink(file.c_str()) == 0) {
        return true;
    }
    if (errno == ENOENT) {
        return true;

    }
    PLOG(ERROR) << "Could not unlink: " << file;
    return false;
}

enum ReconcileSecondaryDexResult {
    kReconcileSecondaryDexExists = 0,
    kReconcileSecondaryDexCleanedUp = 1,
    kReconcileSecondaryDexValidationError = 2,
    kReconcileSecondaryDexCleanUpError = 3,
    kReconcileSecondaryDexAccessIOError = 4,
};

// Reconcile the secondary dex 'dex_path' and its generated oat files.
// Return true if all the parameters are valid and the secondary dex file was
//   processed successfully (i.e. the dex_path either exists, or if not, its corresponding
//   oat/vdex/art files where deleted successfully). In this case, out_secondary_dex_exists
//   will be true if the secondary dex file still exists. If the secondary dex file does not exist,
//   the method cleans up any previously generated compiler artifacts (oat, vdex, art).
// Return false if there were errors during processing. In this case
//   out_secondary_dex_exists will be set to false.
bool reconcile_secondary_dex_file(const std::string& dex_path,
        const std::string& pkgname, int uid, const std::vector<std::string>& isas,
        const std::unique_ptr<std::string>& volume_uuid, int storage_flag,
        /*out*/bool* out_secondary_dex_exists) {
    *out_secondary_dex_exists = false;  // start by assuming the file does not exist.
    if (isas.size() == 0) {
        LOG(ERROR) << "reconcile_secondary_dex_file called with empty isas vector";
        return false;
    }

    if (storage_flag != FLAG_STORAGE_CE && storage_flag != FLAG_STORAGE_DE) {
        LOG(ERROR) << "reconcile_secondary_dex_file called with invalid storage_flag: "
                << storage_flag;
        return false;
    }

    // As a security measure we want to unlink art artifacts with the reduced capabilities
    // of the package user id. So we fork and drop capabilities in the child.
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(uid);

        const char* volume_uuid_cstr = volume_uuid == nullptr ? nullptr : volume_uuid->c_str();
        if (!validate_secondary_dex_path(pkgname, dex_path, volume_uuid_cstr,
                uid, storage_flag)) {
            LOG(ERROR) << "Could not validate secondary dex path " << dex_path;
            _exit(kReconcileSecondaryDexValidationError);
        }

        SecondaryDexAccess access_check = check_secondary_dex_access(dex_path);
        switch (access_check) {
            case kSecondaryDexAccessDoesNotExist:
                 // File does not exist. Proceed with cleaning.
                break;
            case kSecondaryDexAccessReadOk: _exit(kReconcileSecondaryDexExists);
            case kSecondaryDexAccessIOError: _exit(kReconcileSecondaryDexAccessIOError);
            case kSecondaryDexAccessPermissionError: _exit(kReconcileSecondaryDexValidationError);
            default:
                LOG(ERROR) << "Unexpected result from check_secondary_dex_access: " << access_check;
                _exit(kReconcileSecondaryDexValidationError);
        }

        // The secondary dex does not exist anymore or it's. Clear any generated files.
        char oat_path[PKG_PATH_MAX];
        char oat_dir[PKG_PATH_MAX];
        char oat_isa_dir[PKG_PATH_MAX];
        bool result = true;
        for (size_t i = 0; i < isas.size(); i++) {
            std::string error_msg;
            if (!create_secondary_dex_oat_layout(
                    dex_path,isas[i], oat_dir, oat_isa_dir, oat_path, &error_msg)) {
                LOG(ERROR) << error_msg;
                _exit(kReconcileSecondaryDexValidationError);
            }

            // Delete oat/vdex/art files.
            result = unlink_if_exists(oat_path) && result;
            result = unlink_if_exists(create_vdex_filename(oat_path)) && result;
            result = unlink_if_exists(create_image_filename(oat_path)) && result;

            // Delete profiles.
            std::string current_profile = create_current_profile_path(
                multiuser_get_user_id(uid), pkgname, dex_path, /*is_secondary*/true);
            std::string reference_profile = create_reference_profile_path(
                pkgname, dex_path, /*is_secondary*/true);
            result = unlink_if_exists(current_profile) && result;
            result = unlink_if_exists(reference_profile) && result;

            // We upgraded once the location of current profile for secondary dex files.
            // Check for any previous left-overs and remove them as well.
            std::string old_current_profile = dex_path + ".prof";
            result = unlink_if_exists(old_current_profile);

            // Try removing the directories as well, they might be empty.
            result = rmdir_if_empty(oat_isa_dir) && result;
            result = rmdir_if_empty(oat_dir) && result;
        }
        if (!result) {
            PLOG(ERROR) << "Failed to clean secondary dex artifacts for location " << dex_path;
        }
        _exit(result ? kReconcileSecondaryDexCleanedUp : kReconcileSecondaryDexAccessIOError);
    }

    int return_code = wait_child(pid);
    if (!WIFEXITED(return_code)) {
        LOG(WARNING) << "reconcile dex failed for location " << dex_path << ": " << return_code;
    } else {
        return_code = WEXITSTATUS(return_code);
    }

    LOG(DEBUG) << "Reconcile secondary dex path " << dex_path << " result=" << return_code;

    switch (return_code) {
        case kReconcileSecondaryDexCleanedUp:
        case kReconcileSecondaryDexValidationError:
            // If we couldn't validate assume the dex file does not exist.
            // This will purge the entry from the PM records.
            *out_secondary_dex_exists = false;
            return true;
        case kReconcileSecondaryDexExists:
            *out_secondary_dex_exists = true;
            return true;
        case kReconcileSecondaryDexAccessIOError:
            // We had an access IO error.
            // Return false so that we can try again.
            // The value of out_secondary_dex_exists does not matter in this case and by convention
            // is set to false.
            *out_secondary_dex_exists = false;
            return false;
        default:
            LOG(ERROR) << "Unexpected code from reconcile_secondary_dex_file: " << return_code;
            *out_secondary_dex_exists = false;
            return false;
    }
}

// Compute and return the hash (SHA-256) of the secondary dex file at dex_path.
// Returns true if all parameters are valid and the hash successfully computed and stored in
// out_secondary_dex_hash.
// Also returns true with an empty hash if the file does not currently exist or is not accessible to
// the app.
// For any other errors (e.g. if any of the parameters are invalid) returns false.
bool hash_secondary_dex_file(const std::string& dex_path, const std::string& pkgname, int uid,
        const std::unique_ptr<std::string>& volume_uuid, int storage_flag,
        std::vector<uint8_t>* out_secondary_dex_hash) {
    out_secondary_dex_hash->clear();

    const char* volume_uuid_cstr = volume_uuid == nullptr ? nullptr : volume_uuid->c_str();

    if (storage_flag != FLAG_STORAGE_CE && storage_flag != FLAG_STORAGE_DE) {
        LOG(ERROR) << "hash_secondary_dex_file called with invalid storage_flag: "
                << storage_flag;
        return false;
    }

    // Pipe to get the hash result back from our child process.
    unique_fd pipe_read, pipe_write;
    if (!Pipe(&pipe_read, &pipe_write)) {
        PLOG(ERROR) << "Failed to create pipe";
        return false;
    }

    // Fork so that actual access to the files is done in the app's own UID, to ensure we only
    // access data the app itself can access.
    pid_t pid = fork();
    if (pid == 0) {
        // child -- drop privileges before continuing
        drop_capabilities(uid);
        pipe_read.reset();

        if (!validate_secondary_dex_path(pkgname, dex_path, volume_uuid_cstr, uid, storage_flag)) {
            LOG(ERROR) << "Could not validate secondary dex path " << dex_path;
            _exit(DexoptReturnCodes::kHashValidatePath);
        }

        unique_fd fd(TEMP_FAILURE_RETRY(open(dex_path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW)));
        if (fd == -1) {
            if (errno == EACCES || errno == ENOENT) {
                // Not treated as an error.
                _exit(0);
            }
            PLOG(ERROR) << "Failed to open secondary dex " << dex_path;
            _exit(DexoptReturnCodes::kHashOpenPath);
        }

        SHA256_CTX ctx;
        SHA256_Init(&ctx);

        std::vector<uint8_t> buffer(65536);
        while (true) {
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer.data(), buffer.size()));
            if (bytes_read == 0) {
                break;
            } else if (bytes_read == -1) {
                PLOG(ERROR) << "Failed to read secondary dex " << dex_path;
                _exit(DexoptReturnCodes::kHashReadDex);
            }

            SHA256_Update(&ctx, buffer.data(), bytes_read);
        }

        std::array<uint8_t, SHA256_DIGEST_LENGTH> hash;
        SHA256_Final(hash.data(), &ctx);
        if (!WriteFully(pipe_write, hash.data(), hash.size())) {
            _exit(DexoptReturnCodes::kHashWrite);
        }

        _exit(0);
    }

    // parent
    pipe_write.reset();

    out_secondary_dex_hash->resize(SHA256_DIGEST_LENGTH);
    if (!ReadFully(pipe_read, out_secondary_dex_hash->data(), out_secondary_dex_hash->size())) {
        out_secondary_dex_hash->clear();
    }
    return wait_child(pid) == 0;
}

// Helper for move_ab, so that we can have common failure-case cleanup.
static bool unlink_and_rename(const char* from, const char* to) {
    // Check whether "from" exists, and if so whether it's regular. If it is, unlink. Otherwise,
    // return a failure.
    struct stat s;
    if (stat(to, &s) == 0) {
        if (!S_ISREG(s.st_mode)) {
            LOG(ERROR) << from << " is not a regular file to replace for A/B.";
            return false;
        }
        if (unlink(to) != 0) {
            LOG(ERROR) << "Could not unlink " << to << " to move A/B.";
            return false;
        }
    } else {
        // This may be a permission problem. We could investigate the error code, but we'll just
        // let the rename failure do the work for us.
    }

    // Try to rename "to" to "from."
    if (rename(from, to) != 0) {
        PLOG(ERROR) << "Could not rename " << from << " to " << to;
        return false;
    }
    return true;
}

// Move/rename a B artifact (from) to an A artifact (to).
static bool move_ab_path(const std::string& b_path, const std::string& a_path) {
    // Check whether B exists.
    {
        struct stat s;
        if (stat(b_path.c_str(), &s) != 0) {
            // Silently ignore for now. The service calling this isn't smart enough to understand
            // lack of artifacts at the moment.
            return false;
        }
        if (!S_ISREG(s.st_mode)) {
            LOG(ERROR) << "A/B artifact " << b_path << " is not a regular file.";
            // Try to unlink, but swallow errors.
            unlink(b_path.c_str());
            return false;
        }
    }

    // Rename B to A.
    if (!unlink_and_rename(b_path.c_str(), a_path.c_str())) {
        // Delete the b_path so we don't try again (or fail earlier).
        if (unlink(b_path.c_str()) != 0) {
            PLOG(ERROR) << "Could not unlink " << b_path;
        }

        return false;
    }

    return true;
}

bool move_ab(const char* apk_path, const char* instruction_set, const char* oat_dir) {
    // Get the current slot suffix. No suffix, no A/B.
    const std::string slot_suffix = GetProperty("ro.boot.slot_suffix", "");
    if (slot_suffix.empty()) {
        return false;
    }

    if (!ValidateTargetSlotSuffix(slot_suffix)) {
        LOG(ERROR) << "Target slot suffix not legal: " << slot_suffix;
        return false;
    }

    // Validate other inputs.
    if (validate_apk_path(apk_path) != 0) {
        LOG(ERROR) << "Invalid apk_path: " << apk_path;
        return false;
    }
    if (validate_apk_path(oat_dir) != 0) {
        LOG(ERROR) << "Invalid oat_dir: " << oat_dir;
        return false;
    }

    char a_path[PKG_PATH_MAX];
    if (!calculate_oat_file_path(a_path, oat_dir, apk_path, instruction_set)) {
        return false;
    }
    const std::string a_vdex_path = create_vdex_filename(a_path);
    const std::string a_image_path = create_image_filename(a_path);

    // B path = A path + slot suffix.
    const std::string b_path = StringPrintf("%s.%s", a_path, slot_suffix.c_str());
    const std::string b_vdex_path = StringPrintf("%s.%s", a_vdex_path.c_str(), slot_suffix.c_str());
    const std::string b_image_path = StringPrintf("%s.%s",
                                                  a_image_path.c_str(),
                                                  slot_suffix.c_str());

    bool success = true;
    if (move_ab_path(b_path, a_path)) {
        if (move_ab_path(b_vdex_path, a_vdex_path)) {
            // Note: we can live without an app image. As such, ignore failure to move the image file.
            //       If we decide to require the app image, or the app image being moved correctly,
            //       then change accordingly.
            constexpr bool kIgnoreAppImageFailure = true;

            if (!a_image_path.empty()) {
                if (!move_ab_path(b_image_path, a_image_path)) {
                    unlink(a_image_path.c_str());
                    if (!kIgnoreAppImageFailure) {
                        success = false;
                    }
                }
            }
        } else {
            // Cleanup: delete B image, ignore errors.
            unlink(b_image_path.c_str());
            success = false;
        }
    } else {
        // Cleanup: delete B image, ignore errors.
        unlink(b_vdex_path.c_str());
        unlink(b_image_path.c_str());
        success = false;
    }
    return success;
}

bool delete_odex(const char* apk_path, const char* instruction_set, const char* oat_dir) {
    // Delete the oat/odex file.
    char out_path[PKG_PATH_MAX];
    if (!create_oat_out_path(apk_path, instruction_set, oat_dir,
            /*is_secondary_dex*/false, out_path)) {
        return false;
    }

    // In case of a permission failure report the issue. Otherwise just print a warning.
    auto unlink_and_check = [](const char* path) -> bool {
        int result = unlink(path);
        if (result != 0) {
            if (errno == EACCES || errno == EPERM) {
                PLOG(ERROR) << "Could not unlink " << path;
                return false;
            }
            PLOG(WARNING) << "Could not unlink " << path;
        }
        return true;
    };

    // Delete the oat/odex file.
    bool return_value_oat = unlink_and_check(out_path);

    // Derive and delete the app image.
    bool return_value_art = unlink_and_check(create_image_filename(out_path).c_str());

    // Derive and delete the vdex file.
    bool return_value_vdex = unlink_and_check(create_vdex_filename(out_path).c_str());

    // Report success.
    return return_value_oat && return_value_art && return_value_vdex;
}

static bool is_absolute_path(const std::string& path) {
    if (path.find('/') != 0 || path.find("..") != std::string::npos) {
        LOG(ERROR) << "Invalid absolute path " << path;
        return false;
    } else {
        return true;
    }
}

static bool is_valid_instruction_set(const std::string& instruction_set) {
    // TODO: add explicit whitelisting of instruction sets
    if (instruction_set.find('/') != std::string::npos) {
        LOG(ERROR) << "Invalid instruction set " << instruction_set;
        return false;
    } else {
        return true;
    }
}

bool calculate_oat_file_path_default(char path[PKG_PATH_MAX], const char *oat_dir,
        const char *apk_path, const char *instruction_set) {
    std::string oat_dir_ = oat_dir;
    std::string apk_path_ = apk_path;
    std::string instruction_set_ = instruction_set;

    if (!is_absolute_path(oat_dir_)) return false;
    if (!is_absolute_path(apk_path_)) return false;
    if (!is_valid_instruction_set(instruction_set_)) return false;

    std::string::size_type end = apk_path_.rfind('.');
    std::string::size_type start = apk_path_.rfind('/', end);
    if (end == std::string::npos || start == std::string::npos) {
        LOG(ERROR) << "Invalid apk_path " << apk_path_;
        return false;
    }

    std::string res_ = oat_dir_ + '/' + instruction_set + '/'
            + apk_path_.substr(start + 1, end - start - 1) + ".odex";
    const char* res = res_.c_str();
    if (strlen(res) >= PKG_PATH_MAX) {
        LOG(ERROR) << "Result too large";
        return false;
    } else {
        strlcpy(path, res, PKG_PATH_MAX);
        return true;
    }
}

bool calculate_odex_file_path_default(char path[PKG_PATH_MAX], const char *apk_path,
        const char *instruction_set) {
    std::string apk_path_ = apk_path;
    std::string instruction_set_ = instruction_set;

    if (!is_absolute_path(apk_path_)) return false;
    if (!is_valid_instruction_set(instruction_set_)) return false;

    std::string::size_type end = apk_path_.rfind('.');
    std::string::size_type start = apk_path_.rfind('/', end);
    if (end == std::string::npos || start == std::string::npos) {
        LOG(ERROR) << "Invalid apk_path " << apk_path_;
        return false;
    }

    std::string oat_dir = apk_path_.substr(0, start + 1) + "oat";
    return calculate_oat_file_path_default(path, oat_dir.c_str(), apk_path, instruction_set);
}

bool create_cache_path_default(char path[PKG_PATH_MAX], const char *src,
        const char *instruction_set) {
    std::string src_ = src;
    std::string instruction_set_ = instruction_set;

    if (!is_absolute_path(src_)) return false;
    if (!is_valid_instruction_set(instruction_set_)) return false;

    for (auto it = src_.begin() + 1; it < src_.end(); ++it) {
        if (*it == '/') {
            *it = '@';
        }
    }

    std::string res_ = android_data_dir + DALVIK_CACHE + '/' + instruction_set_ + src_
            + DALVIK_CACHE_POSTFIX;
    const char* res = res_.c_str();
    if (strlen(res) >= PKG_PATH_MAX) {
        LOG(ERROR) << "Result too large";
        return false;
    } else {
        strlcpy(path, res, PKG_PATH_MAX);
        return true;
    }
}

bool open_classpath_files(const std::string& classpath, std::vector<unique_fd>* apk_fds,
        std::vector<std::string>* dex_locations) {
    std::vector<std::string> classpaths_elems = base::Split(classpath, ":");
    for (const std::string& elem : classpaths_elems) {
        unique_fd fd(TEMP_FAILURE_RETRY(open(elem.c_str(), O_RDONLY)));
        if (fd < 0) {
            PLOG(ERROR) << "Could not open classpath elem " << elem;
            return false;
        } else {
            apk_fds->push_back(std::move(fd));
            dex_locations->push_back(elem);
        }
    }
    return true;
}

static bool create_app_profile_snapshot(int32_t app_id,
                                        const std::string& package_name,
                                        const std::string& profile_name,
                                        const std::string& classpath) {
    int app_shared_gid = multiuser_get_shared_gid(/*user_id*/ 0, app_id);

    unique_fd snapshot_fd = open_spnashot_profile(AID_SYSTEM, package_name, profile_name);
    if (snapshot_fd < 0) {
        return false;
    }

    std::vector<unique_fd> profiles_fd;
    unique_fd reference_profile_fd;
    open_profile_files(app_shared_gid, package_name, profile_name, /*is_secondary_dex*/ false,
            &profiles_fd, &reference_profile_fd);
    if (profiles_fd.empty() || (reference_profile_fd.get() < 0)) {
        return false;
    }

    profiles_fd.push_back(std::move(reference_profile_fd));

    // Open the class paths elements. These will be used to filter out profile data that does
    // not belong to the classpath during merge.
    std::vector<unique_fd> apk_fds;
    std::vector<std::string> dex_locations;
    if (!open_classpath_files(classpath, &apk_fds, &dex_locations)) {
        return false;
    }

    RunProfman args;
    args.SetupMerge(profiles_fd, snapshot_fd, apk_fds, dex_locations);
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        drop_capabilities(app_shared_gid);
        args.Exec();
    }

    /* parent */
    int return_code = wait_child(pid);
    if (!WIFEXITED(return_code)) {
        LOG(WARNING) << "profman failed for " << package_name << ":" << profile_name;
        return false;
    }

    return true;
}

static bool create_boot_image_profile_snapshot(const std::string& package_name,
                                               const std::string& profile_name,
                                               const std::string& classpath) {
    // The reference profile directory for the android package might not be prepared. Do it now.
    const std::string ref_profile_dir =
            create_primary_reference_profile_package_dir_path(package_name);
    if (fs_prepare_dir(ref_profile_dir.c_str(), 0770, AID_SYSTEM, AID_SYSTEM) != 0) {
        PLOG(ERROR) << "Failed to prepare " << ref_profile_dir;
        return false;
    }

    // Return false for empty class path since it may otherwise return true below if profiles is
    // empty.
    if (classpath.empty()) {
        PLOG(ERROR) << "Class path is empty";
        return false;
    }

    // Open and create the snapshot profile.
    unique_fd snapshot_fd = open_spnashot_profile(AID_SYSTEM, package_name, profile_name);

    // Collect all non empty profiles.
    // The collection will traverse all applications profiles and find the non empty files.
    // This has the potential of inspecting a large number of files and directories (depending
    // on the number of applications and users). So there is a slight increase in the chance
    // to get get occasionally I/O errors (e.g. for opening the file). When that happens do not
    // fail the snapshot and aggregate whatever profile we could open.
    //
    // The profile snapshot is a best effort based on available data it's ok if some data
    // from some apps is missing. It will be counter productive for the snapshot to fail
    // because we could not open or read some of the files.
    std::vector<std::string> profiles;
    if (!collect_profiles(&profiles)) {
        LOG(WARNING) << "There were errors while collecting the profiles for the boot image.";
    }

    // If we have no profiles return early.
    if (profiles.empty()) {
        return true;
    }

    // Open the classpath elements. These will be used to filter out profile data that does
    // not belong to the classpath during merge.
    std::vector<unique_fd> apk_fds;
    std::vector<std::string> dex_locations;
    if (!open_classpath_files(classpath, &apk_fds, &dex_locations)) {
        return false;
    }

    // If we could not open any files from the classpath return an error.
    if (apk_fds.empty()) {
        LOG(ERROR) << "Could not open any of the classpath elements.";
        return false;
    }

    // Aggregate the profiles in batches of kAggregationBatchSize.
    // We do this to avoid opening a huge a amount of files.
    static constexpr size_t kAggregationBatchSize = 10;

    std::vector<unique_fd> profiles_fd;
    for (size_t i = 0; i < profiles.size(); )  {
        for (size_t k = 0; k < kAggregationBatchSize && i < profiles.size(); k++, i++) {
            unique_fd fd = open_profile(AID_SYSTEM, profiles[i], O_RDONLY);
            if (fd.get() >= 0) {
                profiles_fd.push_back(std::move(fd));
            }
        }
        RunProfman args;
        args.SetupMerge(profiles_fd,
                        snapshot_fd,
                        apk_fds,
                        dex_locations,
                        /*store_aggregation_counters=*/true);
        pid_t pid = fork();
        if (pid == 0) {
            /* child -- drop privileges before continuing */
            drop_capabilities(AID_SYSTEM);

            // The introduction of new access flags into boot jars causes them to
            // fail dex file verification.
            args.Exec();
        }

        /* parent */
        int return_code = wait_child(pid);
        if (!WIFEXITED(return_code)) {
            PLOG(WARNING) << "profman failed for " << package_name << ":" << profile_name;
            return false;
        }
        return true;
    }
    return true;
}

bool create_profile_snapshot(int32_t app_id, const std::string& package_name,
        const std::string& profile_name, const std::string& classpath) {
    if (app_id == -1) {
        return create_boot_image_profile_snapshot(package_name, profile_name, classpath);
    } else {
        return create_app_profile_snapshot(app_id, package_name, profile_name, classpath);
    }
}

bool prepare_app_profile(const std::string& package_name,
                         userid_t user_id,
                         appid_t app_id,
                         const std::string& profile_name,
                         const std::string& code_path,
                         const std::unique_ptr<std::string>& dex_metadata) {
    // Prepare the current profile.
    std::string cur_profile  = create_current_profile_path(user_id, package_name, profile_name,
            /*is_secondary_dex*/ false);
    uid_t uid = multiuser_get_uid(user_id, app_id);
    if (fs_prepare_file_strict(cur_profile.c_str(), 0600, uid, uid) != 0) {
        PLOG(ERROR) << "Failed to prepare " << cur_profile;
        return false;
    }

    // Check if we need to install the profile from the dex metadata.
    if (dex_metadata == nullptr) {
        return true;
    }

    // We have a dex metdata. Merge the profile into the reference profile.
    unique_fd ref_profile_fd = open_reference_profile(uid, package_name, profile_name,
            /*read_write*/ true, /*is_secondary_dex*/ false);
    unique_fd dex_metadata_fd(TEMP_FAILURE_RETRY(
            open(dex_metadata->c_str(), O_RDONLY | O_NOFOLLOW)));
    unique_fd apk_fd(TEMP_FAILURE_RETRY(open(code_path.c_str(), O_RDONLY | O_NOFOLLOW)));
    if (apk_fd < 0) {
        PLOG(ERROR) << "Could not open code path " << code_path;
        return false;
    }

    RunProfman args;
    args.SetupCopyAndUpdate(std::move(dex_metadata_fd),
                            std::move(ref_profile_fd),
                            std::move(apk_fd),
                            code_path);
    pid_t pid = fork();
    if (pid == 0) {
        /* child -- drop privileges before continuing */
        gid_t app_shared_gid = multiuser_get_shared_gid(user_id, app_id);
        drop_capabilities(app_shared_gid);

        // The copy and update takes ownership over the fds.
        args.Exec();
    }

    /* parent */
    int return_code = wait_child(pid);
    if (!WIFEXITED(return_code)) {
        PLOG(WARNING) << "profman failed for " << package_name << ":" << profile_name;
        return false;
    }
    return true;
}

}  // namespace installd
}  // namespace android
