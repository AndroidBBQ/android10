/*
 ** Copyright 2016, The Android Open Source Project
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

#include "otapreopt_parameters.h"

#include <cstring>

#include <android-base/logging.h>

#include "dexopt.h"
#include "installd_constants.h"
#include "otapreopt_utils.h"

#ifndef LOG_TAG
#define LOG_TAG "otapreopt"
#endif

namespace android {
namespace installd {

static bool ParseBool(const char* in) {
    if (strcmp(in, "true") == 0) {
        return true;
    }
    return false;
}

static const char* ParseNull(const char* arg) {
    return (strcmp(arg, "!") == 0) ? nullptr : arg;
}

static bool ParseUInt(const char* in, uint32_t* out) {
    char* end;
    long long int result = strtoll(in, &end, 0);
    if (in == end || *end != '\0') {
        return false;
    }
    if (result < std::numeric_limits<uint32_t>::min() ||
            std::numeric_limits<uint32_t>::max() < result) {
        return false;
    }
    *out = static_cast<uint32_t>(result);
    return true;
}

bool OTAPreoptParameters::ReadArguments(int argc, const char** argv) {
    // Expected command line:
    //   target-slot [version] dexopt {DEXOPT_PARAMETERS}

    const char* target_slot_arg = argv[1];
    if (target_slot_arg == nullptr) {
        LOG(ERROR) << "Missing parameters";
        return false;
    }
    // Sanitize value. Only allow (a-zA-Z0-9_)+.
    target_slot = target_slot_arg;
    if (!ValidateTargetSlotSuffix(target_slot)) {
        LOG(ERROR) << "Target slot suffix not legal: " << target_slot;
        return false;
    }

    // Check for version or "dexopt" next.
    if (argv[2] == nullptr) {
        LOG(ERROR) << "Missing parameters";
        return false;
    }

    if (std::string("dexopt").compare(argv[2]) == 0) {
        // This is version 1 (N) or pre-versioning version 2.
        constexpr int kV2ArgCount =   1   // "otapreopt"
                                    + 1   // slot
                                    + 1   // "dexopt"
                                    + 1   // apk_path
                                    + 1   // uid
                                    + 1   // pkg
                                    + 1   // isa
                                    + 1   // dexopt_needed
                                    + 1   // oat_dir
                                    + 1   // dexopt_flags
                                    + 1   // filter
                                    + 1   // volume
                                    + 1   // libs
                                    + 1;  // seinfo
        if (argc == kV2ArgCount) {
            return ReadArgumentsPostV1(2, argv, false);
        } else {
            return ReadArgumentsV1(argv);
        }
    }

    uint32_t version;
    if (!ParseUInt(argv[2], &version)) {
        LOG(ERROR) << "Could not parse version: " << argv[2];
        return false;
    }

    return ReadArgumentsPostV1(version, argv, true);
}

static int ReplaceMask(int input, int old_mask, int new_mask) {
    return (input & old_mask) != 0 ? new_mask : 0;
}

void OTAPreoptParameters::SetDefaultsForPostV1Arguments() {
    // Set se_info to null. It is only relevant for secondary dex files, which we won't
    // receive from a v1 A side.
    se_info = nullptr;

    // Set downgrade to false. It is only relevant when downgrading compiler
    // filter, which is not the case during ota.
    downgrade = false;

    // Set target_sdk_version to 0, ie the platform SDK version. This is
    // conservative and may force some classes to verify at runtime.
    target_sdk_version = 0;

    // Set the profile name to the primary apk profile.
    profile_name = "primary.prof";

    // By default we don't have a dex metadata file.
    dex_metadata_path = nullptr;

    // The compilation reason is ab-ota (match the system property pm.dexopt.ab-ota)
    compilation_reason = "ab-ota";

    // Flag is enabled by default for A/B otas.
    dexopt_flags = DEXOPT_GENERATE_COMPACT_DEX;
}

bool OTAPreoptParameters::ReadArgumentsV1(const char** argv) {
    // Check for "dexopt".
    if (argv[2] == nullptr) {
        LOG(ERROR) << "Missing parameters";
        return false;
    }
    if (std::string("dexopt").compare(argv[2]) != 0) {
        LOG(ERROR) << "Expected \"dexopt\" but found: " << argv[2];
        return false;
    }

    SetDefaultsForPostV1Arguments();

    size_t param_index = 0;
    for (;; ++param_index) {
        const char* param = argv[3 + param_index];
        if (param == nullptr) {
            break;
        }

        switch (param_index) {
            case 0:
                apk_path = param;
                break;

            case 1:
                uid = atoi(param);
                break;

            case 2:
                pkgName = param;
                break;

            case 3:
                instruction_set = param;
                break;

            case 4: {
                // Version 1 had:
                //   DEXOPT_DEX2OAT_NEEDED       = 1
                //   DEXOPT_PATCHOAT_NEEDED      = 2
                //   DEXOPT_SELF_PATCHOAT_NEEDED = 3
                // We will simply use DEX2OAT_FROM_SCRATCH.
                dexopt_needed = DEX2OAT_FROM_SCRATCH;
                break;
            }

            case 5:
                oat_dir = param;
                break;

            case 6: {
                // Version 1 had:
                constexpr int OLD_DEXOPT_PUBLIC         = 1 << 1;
                // Note: DEXOPT_SAFEMODE has been removed.
                // constexpr int OLD_DEXOPT_SAFEMODE       = 1 << 2;
                constexpr int OLD_DEXOPT_DEBUGGABLE     = 1 << 3;
                constexpr int OLD_DEXOPT_BOOTCOMPLETE   = 1 << 4;
                constexpr int OLD_DEXOPT_PROFILE_GUIDED = 1 << 5;
                constexpr int OLD_DEXOPT_OTA            = 1 << 6;
                static_assert(DEXOPT_GENERATE_COMPACT_DEX > OLD_DEXOPT_OTA, "must not overlap");
                int input = atoi(param);
                dexopt_flags |=
                        ReplaceMask(input, OLD_DEXOPT_PUBLIC, DEXOPT_PUBLIC) |
                        ReplaceMask(input, OLD_DEXOPT_DEBUGGABLE, DEXOPT_DEBUGGABLE) |
                        ReplaceMask(input, OLD_DEXOPT_BOOTCOMPLETE, DEXOPT_BOOTCOMPLETE) |
                        ReplaceMask(input, OLD_DEXOPT_PROFILE_GUIDED, DEXOPT_PROFILE_GUIDED) |
                        ReplaceMask(input, OLD_DEXOPT_OTA, 0);
                break;
            }

            case 7:
                compiler_filter = param;
                break;

            case 8:
                volume_uuid = ParseNull(param);
                break;

            case 9:
                shared_libraries = ParseNull(param);
                break;

            default:
                LOG(ERROR) << "Too many arguments, got " << param;
                return false;
        }
    }

    if (param_index != 10) {
        LOG(ERROR) << "Not enough parameters";
        return false;
    }

    return true;
}

bool OTAPreoptParameters::ReadArgumentsPostV1(uint32_t version, const char** argv, bool versioned) {
    size_t num_args_expected = 0;
    switch (version) {
        case 2: num_args_expected = 11; break;
        case 3: num_args_expected = 12; break;
        case 4: num_args_expected = 13; break;
        case 5: num_args_expected = 14; break;
        case 6: num_args_expected = 15; break;
        case 7:
        // Version 8 adds a new dexopt flag: DEXOPT_GENERATE_COMPACT_DEX
        case 8: num_args_expected = 16; break;
        // Version 9 adds a new dexopt flag: DEXOPT_GENERATE_APP_IMAGE
        case 9: num_args_expected = 16; break;
        // Version 10 is a compatibility bump.
        case 10: num_args_expected = 16; break;
        default:
            LOG(ERROR) << "Don't know how to read arguments for version " << version;
            return false;
    }
    size_t dexopt_index = versioned ? 3 : 2;

    // Check for "dexopt".
    if (argv[dexopt_index] == nullptr) {
        LOG(ERROR) << "Missing parameters";
        return false;
    }
    if (std::string("dexopt").compare(argv[dexopt_index]) != 0) {
        LOG(ERROR) << "Expected \"dexopt\" but found: " << argv[dexopt_index];
        return false;
    }

    // Validate the number of arguments.
    size_t num_args_actual = 0;
    while (argv[dexopt_index + 1 + num_args_actual] != nullptr) {
        num_args_actual++;
    }

    if (num_args_actual != num_args_expected) {
        LOG(ERROR) << "Invalid number of arguments. expected="
                << num_args_expected << " actual=" << num_args_actual;
        return false;
    }

    // The number of arguments is OK.
    // Configure the default values for the parameters that were added after V1.
    // The default values will be overwritten in case they are passed as arguments.
    SetDefaultsForPostV1Arguments();

    for (size_t param_index = 0; param_index < num_args_actual; ++param_index) {
        const char* param = argv[dexopt_index + 1 + param_index];
        switch (param_index) {
            case 0:
                apk_path = param;
                break;

            case 1:
                uid = atoi(param);
                break;

            case 2:
                pkgName = param;
                break;

            case 3:
                instruction_set = param;
                break;

            case 4:
                dexopt_needed = atoi(param);
                break;

            case 5:
                oat_dir = param;
                break;

            case 6:
                dexopt_flags = atoi(param);
                // Add CompactDex generation flag for versions less than 8 since it wasn't passed
                // from the package manager. Only conditionally set the flag here so that it can
                // be fully controlled by the package manager.
                dexopt_flags |= (version < 8) ? DEXOPT_GENERATE_COMPACT_DEX : 0u;
                break;

            case 7:
                compiler_filter = param;
                break;

            case 8:
                volume_uuid = ParseNull(param);
                break;

            case 9:
                shared_libraries = ParseNull(param);
                break;

            case 10:
                se_info = ParseNull(param);
                break;

            case 11:
                downgrade = ParseBool(param);
                break;

            case 12:
                target_sdk_version = atoi(param);
                break;

            case 13:
                profile_name = ParseNull(param);
                break;

            case 14:
                dex_metadata_path = ParseNull(param);
                break;

            case 15:
                compilation_reason = ParseNull(param);
                break;

            default:
                LOG(FATAL) << "Should not get here. Did you call ReadArguments "
                        << "with the right expectation? index=" << param_index
                        << " num_args=" << num_args_actual;
                return false;
        }
    }

    if (version < 10) {
        // Do not accept '&' as shared libraries from versions prior to 10. These may lead
        // to runtime crashes. The server side of version 10+ should send the correct
        // context in almost all cases (e.g., only for actual shared packages).
        if (shared_libraries != nullptr && std::string("&") == shared_libraries) {
            return false;
        }
    }

    return true;
}

}  // namespace installd
}  // namespace android
