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

#ifndef INSTALLD_CONSTANTS_H_
#define INSTALLD_CONSTANTS_H_

namespace android {
namespace installd {

/* elements combined with a valid package name to form paths */

constexpr const char* PRIMARY_USER_PREFIX = "data/";
constexpr const char* SECONDARY_USER_PREFIX = "user/";

// This is used as a string literal, can't be constants. TODO: std::string...
#define DALVIK_CACHE "dalvik-cache"
constexpr const char* DALVIK_CACHE_POSTFIX = "@classes.dex";

constexpr size_t PKG_NAME_MAX = 128u;   /* largest allowed package name */
constexpr size_t PKG_PATH_MAX = 1024u;  /* max size of any path we use */

/****************************************************************************
 * IMPORTANT: These values are passed from Java code. Keep them in sync with
 * frameworks/base/services/core/java/com/android/server/pm/Installer.java
 ***************************************************************************/
constexpr int DEXOPT_PUBLIC         = 1 << 1;
constexpr int DEXOPT_DEBUGGABLE     = 1 << 2;
constexpr int DEXOPT_BOOTCOMPLETE   = 1 << 3;
constexpr int DEXOPT_PROFILE_GUIDED = 1 << 4;
constexpr int DEXOPT_SECONDARY_DEX  = 1 << 5;
// DEXOPT_FORCE, DEXOPT_STORAGE_CE, DEXOPT_STORAGE_DE are exposed for secondary
// dex files only. Primary apks are analyzed in PackageManager and installd
// does not need to know if the compilation is forced or on what kind of storage
// the dex files are.
constexpr int DEXOPT_FORCE          = 1 << 6;
constexpr int DEXOPT_STORAGE_CE     = 1 << 7;
constexpr int DEXOPT_STORAGE_DE     = 1 << 8;
// Tells the compiler that it is invoked from the background service.  This
// controls whether extra debugging flags can be used (taking more compile time.)
constexpr int DEXOPT_IDLE_BACKGROUND_JOB = 1 << 9;
constexpr int DEXOPT_ENABLE_HIDDEN_API_CHECKS = 1 << 10;
constexpr int DEXOPT_GENERATE_COMPACT_DEX = 1 << 11;
constexpr int DEXOPT_GENERATE_APP_IMAGE = 1 << 12;

/* all known values for dexopt flags */
constexpr int DEXOPT_MASK =
    DEXOPT_PUBLIC
    | DEXOPT_DEBUGGABLE
    | DEXOPT_BOOTCOMPLETE
    | DEXOPT_PROFILE_GUIDED
    | DEXOPT_SECONDARY_DEX
    | DEXOPT_FORCE
    | DEXOPT_STORAGE_CE
    | DEXOPT_STORAGE_DE
    | DEXOPT_IDLE_BACKGROUND_JOB
    | DEXOPT_ENABLE_HIDDEN_API_CHECKS
    | DEXOPT_GENERATE_COMPACT_DEX
    | DEXOPT_GENERATE_APP_IMAGE;

// NOTE: keep in sync with StorageManager
constexpr int FLAG_STORAGE_DE = 1 << 0;
constexpr int FLAG_STORAGE_CE = 1 << 1;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

}  // namespace installd
}  // namespace android

#endif  // INSTALLD_CONSTANTS_H_
