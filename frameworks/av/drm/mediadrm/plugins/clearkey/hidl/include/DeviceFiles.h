// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
#ifndef CLEARKEY_DEVICE_FILES_H_
#define CLEARKEY_DEVICE_FILES_H_

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <set>
#include <string>
#include <vector>

#include "protos/DeviceFiles.pb.h"
#include "ClearKeyTypes.h"
#include "MemoryFileSystem.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

class DeviceFiles {
 public:
    typedef enum {
        kLicenseStateUnknown,
        kLicenseStateActive,
        kLicenseStateReleasing,
    } LicenseState;

    DeviceFiles() {};
    virtual ~DeviceFiles() {};

    virtual bool StoreLicense(const std::string& keySetId, LicenseState state,
            const std::string& keyResponse);

    virtual bool RetrieveLicense(
            const std::string& key_set_id, LicenseState* state, std::string* offlineLicense);

    virtual bool LicenseExists(const std::string& keySetId);

    virtual std::vector<std::string> ListLicenses() const;

    virtual bool DeleteLicense(const std::string& keySetId);

    virtual bool DeleteAllLicenses();

 private:
    bool FileExists(const std::string& path) const;
    ssize_t GetFileSize(const std::string& fileName) const;
    bool RemoveFile(const std::string& fileName);

    bool RetrieveHashedFile(const std::string& fileName, OfflineFile* deSerializedFile);
    bool StoreFileRaw(const std::string& fileName, const std::string& serializedFile);
    bool StoreFileWithHash(const std::string& fileName, const std::string& serializedFile);

    MemoryFileSystem mFileHandle;

    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(DeviceFiles);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif  // CLEARKEY_DEVICE_FILES_H_
