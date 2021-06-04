// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include <utils/Log.h>

#include <string>
#include <sys/stat.h>

#include "DeviceFiles.h"
#include "Utils.h"

#include <openssl/sha.h>

// Protobuf generated classes.
using android::hardware::drm::V1_2::clearkey::OfflineFile;
using android::hardware::drm::V1_2::clearkey::HashedFile;
using android::hardware::drm::V1_2::clearkey::License;
using android::hardware::drm::V1_2::clearkey::License_LicenseState_ACTIVE;
using android::hardware::drm::V1_2::clearkey::License_LicenseState_RELEASING;

namespace {
const char kLicenseFileNameExt[] = ".lic";

bool Hash(const std::string& data, std::string* hash) {
    if (!hash) return false;

    hash->resize(SHA256_DIGEST_LENGTH);

    const unsigned char* input = reinterpret_cast<const unsigned char*>(data.data());
    unsigned char* output = reinterpret_cast<unsigned char*>(&(*hash)[0]);
    SHA256(input, data.size(), output);
    return true;
}

}  // namespace

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

bool DeviceFiles::StoreLicense(
        const std::string& keySetId, LicenseState state,
        const std::string& licenseResponse) {

    OfflineFile file;
    file.set_type(OfflineFile::LICENSE);
    file.set_version(OfflineFile::VERSION_1);

    License* license = file.mutable_license();
    switch (state) {
        case kLicenseStateActive:
            license->set_state(License_LicenseState_ACTIVE);
            license->set_license(licenseResponse);
            break;
        case kLicenseStateReleasing:
            license->set_state(License_LicenseState_RELEASING);
            license->set_license(licenseResponse);
            break;
        default:
            ALOGW("StoreLicense: Unknown license state: %u", state);
            return false;
    }

    std::string serializedFile;
    file.SerializeToString(&serializedFile);

    return StoreFileWithHash(keySetId + kLicenseFileNameExt, serializedFile);
}

bool DeviceFiles::StoreFileWithHash(const std::string& fileName,
        const std::string& serializedFile) {
    std::string hash;
    if (!Hash(serializedFile, &hash)) {
        ALOGE("StoreFileWithHash: Failed to compute hash");
        return false;
    }

    HashedFile hashFile;
    hashFile.set_file(serializedFile);
    hashFile.set_hash(hash);

    std::string serializedHashFile;
    hashFile.SerializeToString(&serializedHashFile);

    return StoreFileRaw(fileName, serializedHashFile);
}

bool DeviceFiles::StoreFileRaw(const std::string& fileName, const std::string& serializedHashFile) {
    MemoryFileSystem::MemoryFile memFile;
    memFile.setFileName(fileName);
    memFile.setContent(serializedHashFile);
    memFile.setFileSize(serializedHashFile.size());
    size_t len = mFileHandle.Write(fileName, memFile);

    if (len != static_cast<size_t>(serializedHashFile.size())) {
        ALOGE("StoreFileRaw: Failed to write %s", fileName.c_str());
        ALOGD("StoreFileRaw: expected=%zd, actual=%zu", serializedHashFile.size(), len);
        return false;
    }

    ALOGD("StoreFileRaw: wrote %zu bytes to %s", serializedHashFile.size(), fileName.c_str());
    return true;
}

bool DeviceFiles::RetrieveLicense(
    const std::string& keySetId, LicenseState* state, std::string* offlineLicense) {

    OfflineFile file;
    if (!RetrieveHashedFile(keySetId + kLicenseFileNameExt, &file)) {
        return false;
    }

    if (file.type() != OfflineFile::LICENSE) {
        ALOGE("RetrieveLicense: Invalid file type");
        return false;
    }

    if (file.version() != OfflineFile::VERSION_1) {
        ALOGE("RetrieveLicense: Invalid file version");
        return false;
    }

    if (!file.has_license()) {
        ALOGE("RetrieveLicense: License not present");
        return false;
    }

    License license = file.license();
    switch (license.state()) {
        case License_LicenseState_ACTIVE:
            *state = kLicenseStateActive;
            break;
        case License_LicenseState_RELEASING:
            *state = kLicenseStateReleasing;
            break;
        default:
            ALOGW("RetrieveLicense: Unrecognized license state: %u",
                    kLicenseStateUnknown);
            *state = kLicenseStateUnknown;
            break;
    }
    *offlineLicense = license.license();
    return true;
}

bool DeviceFiles::DeleteLicense(const std::string& keySetId) {
    return mFileHandle.RemoveFile(keySetId + kLicenseFileNameExt);
}

bool DeviceFiles::DeleteAllLicenses() {
    return mFileHandle.RemoveAllFiles();
}

bool DeviceFiles::LicenseExists(const std::string& keySetId) {
    return mFileHandle.FileExists(keySetId + kLicenseFileNameExt);
}

std::vector<std::string> DeviceFiles::ListLicenses() const {
    std::vector<std::string> licenses = mFileHandle.ListFiles();
    for (size_t i = 0; i < licenses.size(); i++) {
        std::string& license = licenses[i];
        license = license.substr(0, license.size() - strlen(kLicenseFileNameExt));
    }
    return licenses;
}

bool DeviceFiles::RetrieveHashedFile(const std::string& fileName, OfflineFile* deSerializedFile) {
    if (!deSerializedFile) {
        ALOGE("RetrieveHashedFile: invalid file parameter");
        return false;
    }

    if (!FileExists(fileName)) {
        ALOGE("RetrieveHashedFile: %s does not exist", fileName.c_str());
        return false;
    }

    ssize_t bytes = GetFileSize(fileName);
    if (bytes <= 0) {
        ALOGE("RetrieveHashedFile: invalid file size: %s", fileName.c_str());
        // Remove the corrupted file so the caller will not get the same error
        // when trying to access the file repeatedly, causing the system to stall.
        RemoveFile(fileName);
        return false;
    }

    std::string serializedHashFile;
    serializedHashFile.resize(bytes);
    bytes = mFileHandle.Read(fileName, &serializedHashFile);

    if (bytes != static_cast<ssize_t>(serializedHashFile.size())) {
        ALOGE("RetrieveHashedFile: Failed to read from %s", fileName.c_str());
        ALOGV("RetrieveHashedFile: expected: %zd, actual: %zd", serializedHashFile.size(), bytes);
        // Remove the corrupted file so the caller will not get the same error
        // when trying to access the file repeatedly, causing the system to stall.
        RemoveFile(fileName);
        return false;
    }

    ALOGV("RetrieveHashedFile: read %zd from %s", bytes, fileName.c_str());

    HashedFile hashFile;
    if (!hashFile.ParseFromString(serializedHashFile)) {
        ALOGE("RetrieveHashedFile: Unable to parse hash file");
        // Remove corrupt file.
        RemoveFile(fileName);
        return false;
    }

    std::string hash;
    if (!Hash(hashFile.file(), &hash)) {
        ALOGE("RetrieveHashedFile: Hash computation failed");
        return false;
    }

    if (hash != hashFile.hash()) {
        ALOGE("RetrieveHashedFile: Hash mismatch");
        // Remove corrupt file.
        RemoveFile(fileName);
        return false;
    }

    if (!deSerializedFile->ParseFromString(hashFile.file())) {
        ALOGE("RetrieveHashedFile: Unable to parse file");
        // Remove corrupt file.
        RemoveFile(fileName);
        return false;
    }

    return true;
}

bool DeviceFiles::FileExists(const std::string& fileName) const {
    return mFileHandle.FileExists(fileName);
}

bool DeviceFiles::RemoveFile(const std::string& fileName) {
    return mFileHandle.RemoveFile(fileName);
}

ssize_t DeviceFiles::GetFileSize(const std::string& fileName) const {
    return mFileHandle.GetFileSize(fileName);
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android
