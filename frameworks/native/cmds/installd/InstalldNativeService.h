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

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <inttypes.h>
#include <unistd.h>

#include <vector>
#include <unordered_map>

#include <android-base/macros.h>
#include <binder/BinderService.h>
#include <cutils/multiuser.h>

#include "android/os/BnInstalld.h"
#include "installd_constants.h"

namespace android {
namespace installd {

class InstalldNativeService : public BinderService<InstalldNativeService>, public os::BnInstalld {
public:
    static status_t start();
    static char const* getServiceName() { return "installd"; }
    virtual status_t dump(int fd, const Vector<String16> &args) override;

    binder::Status createUserData(const std::unique_ptr<std::string>& uuid, int32_t userId,
            int32_t userSerial, int32_t flags);
    binder::Status destroyUserData(const std::unique_ptr<std::string>& uuid, int32_t userId,
            int32_t flags);

    binder::Status createAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
            const std::string& seInfo, int32_t targetSdkVersion, int64_t* _aidl_return);
    binder::Status restoreconAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
            const std::string& seInfo);
    binder::Status migrateAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags);
    binder::Status clearAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode);
    binder::Status destroyAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode);

    binder::Status fixupAppData(const std::unique_ptr<std::string>& uuid, int32_t flags);

    binder::Status snapshotAppData(const std::unique_ptr<std::string>& volumeUuid,
            const std::string& packageName, const int32_t user, const int32_t snapshotId,
            int32_t storageFlags, int64_t* _aidl_return);
    binder::Status restoreAppDataSnapshot(const std::unique_ptr<std::string>& volumeUuid,
            const std::string& packageName, const int32_t appId, const std::string& seInfo,
            const int32_t user, const int32_t snapshotId, int32_t storageFlags);
    binder::Status destroyAppDataSnapshot(const std::unique_ptr<std::string> &volumeUuid,
            const std::string& packageName, const int32_t user, const int64_t ceSnapshotInode,
            const int32_t snapshotId, int32_t storageFlags);

    binder::Status getAppSize(const std::unique_ptr<std::string>& uuid,
            const std::vector<std::string>& packageNames, int32_t userId, int32_t flags,
            int32_t appId, const std::vector<int64_t>& ceDataInodes,
            const std::vector<std::string>& codePaths, std::vector<int64_t>* _aidl_return);
    binder::Status getUserSize(const std::unique_ptr<std::string>& uuid,
            int32_t userId, int32_t flags, const std::vector<int32_t>& appIds,
            std::vector<int64_t>* _aidl_return);
    binder::Status getExternalSize(const std::unique_ptr<std::string>& uuid,
            int32_t userId, int32_t flags, const std::vector<int32_t>& appIds,
            std::vector<int64_t>* _aidl_return);

    binder::Status setAppQuota(const std::unique_ptr<std::string>& uuid,
            int32_t userId, int32_t appId, int64_t cacheQuota);

    binder::Status moveCompleteApp(const std::unique_ptr<std::string>& fromUuid,
            const std::unique_ptr<std::string>& toUuid, const std::string& packageName,
            const std::string& dataAppName, int32_t appId, const std::string& seInfo,
            int32_t targetSdkVersion);

    binder::Status dexopt(const std::string& apkPath, int32_t uid,
            const std::unique_ptr<std::string>& packageName, const std::string& instructionSet,
            int32_t dexoptNeeded, const std::unique_ptr<std::string>& outputPath, int32_t dexFlags,
            const std::string& compilerFilter, const std::unique_ptr<std::string>& uuid,
            const std::unique_ptr<std::string>& classLoaderContext,
            const std::unique_ptr<std::string>& seInfo, bool downgrade,
            int32_t targetSdkVersion, const std::unique_ptr<std::string>& profileName,
            const std::unique_ptr<std::string>& dexMetadataPath,
            const std::unique_ptr<std::string>& compilationReason);

    binder::Status compileLayouts(const std::string& apkPath, const std::string& packageName,
                                  const std::string& outDexFile, int uid, bool* _aidl_return);

    binder::Status rmdex(const std::string& codePath, const std::string& instructionSet);

    binder::Status mergeProfiles(int32_t uid, const std::string& packageName,
            const std::string& profileName, bool* _aidl_return);
    binder::Status dumpProfiles(int32_t uid, const std::string& packageName,
            const std::string& profileName, const std::string& codePath, bool* _aidl_return);
    binder::Status copySystemProfile(const std::string& systemProfile,
            int32_t uid, const std::string& packageName, const std::string& profileName,
            bool* _aidl_return);
    binder::Status clearAppProfiles(const std::string& packageName, const std::string& profileName);
    binder::Status destroyAppProfiles(const std::string& packageName);

    binder::Status createProfileSnapshot(int32_t appId, const std::string& packageName,
            const std::string& profileName, const std::string& classpath, bool* _aidl_return);
    binder::Status destroyProfileSnapshot(const std::string& packageName,
            const std::string& profileName);

    binder::Status idmap(const std::string& targetApkPath, const std::string& overlayApkPath,
            int32_t uid);
    binder::Status removeIdmap(const std::string& overlayApkPath);
    binder::Status rmPackageDir(const std::string& packageDir);
    binder::Status freeCache(const std::unique_ptr<std::string>& uuid, int64_t targetFreeBytes,
            int64_t cacheReservedBytes, int32_t flags);
    binder::Status linkNativeLibraryDirectory(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, const std::string& nativeLibPath32, int32_t userId);
    binder::Status createOatDir(const std::string& oatDir, const std::string& instructionSet);
    binder::Status linkFile(const std::string& relativePath, const std::string& fromBase,
            const std::string& toBase);
    binder::Status moveAb(const std::string& apkPath, const std::string& instructionSet,
            const std::string& outputPath);
    binder::Status deleteOdex(const std::string& apkPath, const std::string& instructionSet,
            const std::unique_ptr<std::string>& outputPath);
    binder::Status installApkVerity(const std::string& filePath,
            const ::android::base::unique_fd& verityInput, int32_t contentSize);
    binder::Status assertFsverityRootHashMatches(const std::string& filePath,
            const std::vector<uint8_t>& expectedHash);
    binder::Status reconcileSecondaryDexFile(const std::string& dexPath,
        const std::string& packageName, int32_t uid, const std::vector<std::string>& isa,
        const std::unique_ptr<std::string>& volumeUuid, int32_t storage_flag, bool* _aidl_return);
    binder::Status hashSecondaryDexFile(const std::string& dexPath,
        const std::string& packageName, int32_t uid, const std::unique_ptr<std::string>& volumeUuid,
        int32_t storageFlag, std::vector<uint8_t>* _aidl_return);

    binder::Status invalidateMounts();
    binder::Status isQuotaSupported(const std::unique_ptr<std::string>& volumeUuid,
            bool* _aidl_return);

    binder::Status prepareAppProfile(const std::string& packageName,
            int32_t userId, int32_t appId, const std::string& profileName,
            const std::string& codePath, const std::unique_ptr<std::string>& dexMetadata,
            bool* _aidl_return);

    binder::Status migrateLegacyObbData();

private:
    std::recursive_mutex mLock;

    std::recursive_mutex mMountsLock;
    std::recursive_mutex mQuotasLock;

    /* Map of all storage mounts from source to target */
    std::unordered_map<std::string, std::string> mStorageMounts;

    /* Map from UID to cache quota size */
    std::unordered_map<uid_t, int64_t> mCacheQuotas;

    std::string findDataMediaPath(const std::unique_ptr<std::string>& uuid, userid_t userid);
};

}  // namespace installd
}  // namespace android

#endif  // COMMANDS_H_
