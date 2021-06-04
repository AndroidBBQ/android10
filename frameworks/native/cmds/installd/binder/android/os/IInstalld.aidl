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

package android.os;

/** {@hide} */
interface IInstalld {
    void createUserData(@nullable @utf8InCpp String uuid, int userId, int userSerial, int flags);
    void destroyUserData(@nullable @utf8InCpp String uuid, int userId, int flags);

    long createAppData(@nullable @utf8InCpp String uuid, in @utf8InCpp String packageName,
            int userId, int flags, int appId, in @utf8InCpp String seInfo, int targetSdkVersion);
    void restoreconAppData(@nullable @utf8InCpp String uuid, @utf8InCpp String packageName,
            int userId, int flags, int appId, @utf8InCpp String seInfo);
    void migrateAppData(@nullable @utf8InCpp String uuid, @utf8InCpp String packageName,
            int userId, int flags);
    void clearAppData(@nullable @utf8InCpp String uuid, @utf8InCpp String packageName,
            int userId, int flags, long ceDataInode);
    void destroyAppData(@nullable @utf8InCpp String uuid, @utf8InCpp String packageName,
            int userId, int flags, long ceDataInode);

    void fixupAppData(@nullable @utf8InCpp String uuid, int flags);

    long[] getAppSize(@nullable @utf8InCpp String uuid, in @utf8InCpp String[] packageNames,
            int userId, int flags, int appId, in long[] ceDataInodes,
            in @utf8InCpp String[] codePaths);
    long[] getUserSize(@nullable @utf8InCpp String uuid, int userId, int flags, in int[] appIds);
    long[] getExternalSize(@nullable @utf8InCpp String uuid, int userId, int flags, in int[] appIds);

    void setAppQuota(@nullable @utf8InCpp String uuid, int userId, int appId, long cacheQuota);

    void moveCompleteApp(@nullable @utf8InCpp String fromUuid, @nullable @utf8InCpp String toUuid,
            @utf8InCpp String packageName, @utf8InCpp String dataAppName, int appId,
            @utf8InCpp String seInfo, int targetSdkVersion);

    void dexopt(@utf8InCpp String apkPath, int uid, @nullable @utf8InCpp String packageName,
            @utf8InCpp String instructionSet, int dexoptNeeded,
            @nullable @utf8InCpp String outputPath, int dexFlags,
            @utf8InCpp String compilerFilter, @nullable @utf8InCpp String uuid,
            @nullable @utf8InCpp String sharedLibraries,
            @nullable @utf8InCpp String seInfo, boolean downgrade, int targetSdkVersion,
            @nullable @utf8InCpp String profileName,
            @nullable @utf8InCpp String dexMetadataPath,
            @nullable @utf8InCpp String compilationReason);
    boolean compileLayouts(@utf8InCpp String apkPath, @utf8InCpp String packageName,
            @utf8InCpp String outDexFile, int uid);

    void rmdex(@utf8InCpp String codePath, @utf8InCpp String instructionSet);

    boolean mergeProfiles(int uid, @utf8InCpp String packageName, @utf8InCpp String profileName);
    boolean dumpProfiles(int uid, @utf8InCpp String packageName, @utf8InCpp String  profileName,
            @utf8InCpp String codePath);
    boolean copySystemProfile(@utf8InCpp String systemProfile, int uid,
            @utf8InCpp String packageName, @utf8InCpp String profileName);
    void clearAppProfiles(@utf8InCpp String packageName, @utf8InCpp String profileName);
    void destroyAppProfiles(@utf8InCpp String packageName);

    boolean createProfileSnapshot(int appId, @utf8InCpp String packageName,
            @utf8InCpp String profileName, @utf8InCpp String classpath);
    void destroyProfileSnapshot(@utf8InCpp String packageName, @utf8InCpp String profileName);

    void idmap(@utf8InCpp String targetApkPath, @utf8InCpp String overlayApkPath, int uid);
    void removeIdmap(@utf8InCpp String overlayApkPath);
    void rmPackageDir(@utf8InCpp String packageDir);
    void freeCache(@nullable @utf8InCpp String uuid, long targetFreeBytes,
            long cacheReservedBytes, int flags);
    void linkNativeLibraryDirectory(@nullable @utf8InCpp String uuid,
            @utf8InCpp String packageName, @utf8InCpp String nativeLibPath32, int userId);
    void createOatDir(@utf8InCpp String oatDir, @utf8InCpp String instructionSet);
    void linkFile(@utf8InCpp String relativePath, @utf8InCpp String fromBase,
            @utf8InCpp String toBase);
    void moveAb(@utf8InCpp String apkPath, @utf8InCpp String instructionSet,
            @utf8InCpp String outputPath);
    void deleteOdex(@utf8InCpp String apkPath, @utf8InCpp String instructionSet,
            @nullable @utf8InCpp String outputPath);
    void installApkVerity(@utf8InCpp String filePath, in FileDescriptor verityInput,
            int contentSize);
    void assertFsverityRootHashMatches(@utf8InCpp String filePath, in byte[] expectedHash);

    boolean reconcileSecondaryDexFile(@utf8InCpp String dexPath, @utf8InCpp String pkgName,
        int uid, in @utf8InCpp String[] isas, @nullable @utf8InCpp String volume_uuid,
        int storage_flag);

    byte[] hashSecondaryDexFile(@utf8InCpp String dexPath, @utf8InCpp String pkgName,
        int uid, @nullable @utf8InCpp String volumeUuid, int storageFlag);

    void invalidateMounts();
    boolean isQuotaSupported(@nullable @utf8InCpp String uuid);

    boolean prepareAppProfile(@utf8InCpp String packageName,
        int userId, int appId, @utf8InCpp String profileName, @utf8InCpp String codePath,
        @nullable @utf8InCpp String dexMetadata);

    long snapshotAppData(@nullable @utf8InCpp String uuid, in @utf8InCpp String packageName,
            int userId, int snapshotId, int storageFlags);
    void restoreAppDataSnapshot(@nullable @utf8InCpp String uuid, in @utf8InCpp String packageName,
            int appId, @utf8InCpp String seInfo, int user, int snapshotId, int storageflags);
    void destroyAppDataSnapshot(@nullable @utf8InCpp String uuid, @utf8InCpp String packageName,
            int userId, long ceSnapshotInode, int snapshotId, int storageFlags);

    void migrateLegacyObbData();

    const int FLAG_STORAGE_DE = 0x1;
    const int FLAG_STORAGE_CE = 0x2;
    const int FLAG_STORAGE_EXTERNAL = 0x4;

    const int FLAG_CLEAR_CACHE_ONLY = 0x10;
    const int FLAG_CLEAR_CODE_CACHE_ONLY = 0x20;

    const int FLAG_FREE_CACHE_V2 = 0x100;
    const int FLAG_FREE_CACHE_V2_DEFY_QUOTA = 0x200;
    const int FLAG_FREE_CACHE_NOOP = 0x400;

    const int FLAG_USE_QUOTA = 0x1000;
    const int FLAG_FORCE = 0x2000;
}
