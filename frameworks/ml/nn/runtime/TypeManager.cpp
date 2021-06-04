/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define LOG_TAG "TypeManager"

#include "TypeManager.h"

#include "Utils.h"

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android/content/pm/IPackageManagerNative.h>
#include <binder/IServiceManager.h>
#include <procpartition/procpartition.h>
#include <algorithm>
#include <string_view>

namespace android {
namespace nn {

// Replacement function for std::string_view::starts_with()
// which shall be available in C++20.
#if __cplusplus >= 202000L
#error "When upgrading to C++20, remove this error and file a bug to remove this workaround."
#endif
inline bool StartsWith(std::string_view sv, std::string_view prefix) {
    return sv.substr(0u, prefix.size()) == prefix;
}

namespace {

const uint8_t kLowBitsType = static_cast<uint8_t>(Model::ExtensionTypeEncoding::LOW_BITS_TYPE);
const uint32_t kMaxPrefix =
        (1 << static_cast<uint8_t>(Model::ExtensionTypeEncoding::HIGH_BITS_PREFIX)) - 1;

// Checks if the two structures contain the same information. The order of
// operand types within the structures does not matter.
bool equal(const Extension& a, const Extension& b) {
    NN_RET_CHECK_EQ(a.name, b.name);
    // Relies on the fact that TypeManager sorts operandTypes.
    NN_RET_CHECK(a.operandTypes == b.operandTypes);
    return true;
}

// Property for disabling NNAPI vendor extensions on product image (used on GSI /product image,
// which can't use NNAPI vendor extensions).
const char kVExtProductDeny[] = "ro.nnapi.extensions.deny_on_product";
bool isNNAPIVendorExtensionsUseAllowedInProductImage() {
    const std::string vExtProductDeny = android::base::GetProperty(kVExtProductDeny, "");
    return vExtProductDeny.empty();
}

// The file containing the list of Android apps and binaries allowed to use vendor extensions.
// Each line of the file contains new entry. If entry is prefixed by
// '/' slash, then it's a native binary path (e.g. '/data/foo'). If not, it's a name
// of Android app package (e.g. 'com.foo.bar').
const char kAppAllowlistPath[] = "/vendor/etc/nnapi_extensions_app_allowlist";
const char kCtsAllowlist[] = "/data/local/tmp/CTSNNAPITestCases";
std::vector<std::string> getVendorExtensionAllowlistedApps() {
    std::string data;
    // Allowlist CTS by default.
    std::vector<std::string> allowlist = {kCtsAllowlist};

    if (!android::base::ReadFileToString(kAppAllowlistPath, &data)) {
        // Return default allowlist (no app can use extensions).
        LOG(INFO) << "Failed to read " << kAppAllowlistPath
                  << " ; No app allowlisted for vendor extensions use.";
        return allowlist;
    }

    std::istringstream streamData(data);
    std::string line;
    while (std::getline(streamData, line)) {
        // Do some basic sanity check on entry, it's either
        // fs path or package name.
        if (StartsWith(line, "/") || line.find('.') != std::string::npos) {
            allowlist.push_back(line);
        } else {
            LOG(ERROR) << kAppAllowlistPath << " - Invalid entry: " << line;
        }
    }
    return allowlist;
}

// Query PackageManagerNative service about Android app properties.
// On success, it will populate appPackageInfo->app* fields.
bool fetchAppPackageLocationInfo(uid_t uid, TypeManager::AppPackageInfo* appPackageInfo) {
    sp<::android::IServiceManager> sm(::android::defaultServiceManager());
    sp<::android::IBinder> binder(sm->getService(String16("package_native")));
    if (binder == nullptr) {
        LOG(ERROR) << "getService package_native failed";
        return false;
    }

    sp<content::pm::IPackageManagerNative> packageMgr =
            interface_cast<content::pm::IPackageManagerNative>(binder);
    std::vector<int> uids{static_cast<int>(uid)};
    std::vector<std::string> names;
    binder::Status status = packageMgr->getNamesForUids(uids, &names);
    if (!status.isOk()) {
        LOG(ERROR) << "package_native::getNamesForUids failed: "
                   << status.exceptionMessage().c_str();
        return false;
    }
    const std::string& packageName = names[0];

    appPackageInfo->appPackageName = packageName;
    int flags = 0;
    status = packageMgr->getLocationFlags(packageName, &flags);
    if (!status.isOk()) {
        LOG(ERROR) << "package_native::getLocationFlags failed: "
                   << status.exceptionMessage().c_str();
        return false;
    }
    // isSystemApp()
    appPackageInfo->appIsSystemApp =
            ((flags & content::pm::IPackageManagerNative::LOCATION_SYSTEM) != 0);
    // isVendor()
    appPackageInfo->appIsOnVendorImage =
            ((flags & content::pm::IPackageManagerNative::LOCATION_VENDOR) != 0);
    // isProduct()
    appPackageInfo->appIsOnProductImage =
            ((flags & content::pm::IPackageManagerNative::LOCATION_PRODUCT) != 0);
    return true;
}

// Check if this process is allowed to use NNAPI Vendor extensions.
bool isNNAPIVendorExtensionsUseAllowed(const std::vector<std::string>& allowlist) {
    TypeManager::AppPackageInfo appPackageInfo = {
            .binaryPath = ::android::procpartition::getExe(getpid()),
            .appPackageName = "",
            .appIsSystemApp = false,
            .appIsOnVendorImage = false,
            .appIsOnProductImage = false};

    if (appPackageInfo.binaryPath == "/system/bin/app_process64" ||
        appPackageInfo.binaryPath == "/system/bin/app_process32") {
        if (!fetchAppPackageLocationInfo(getuid(), &appPackageInfo)) {
            LOG(ERROR) << "Failed to get app information from package_manager_native";
            return false;
        }
    }
    return TypeManager::isExtensionsUseAllowed(
            appPackageInfo, isNNAPIVendorExtensionsUseAllowedInProductImage(), allowlist);
}

}  // namespace

TypeManager::TypeManager() {
    VLOG(MANAGER) << "TypeManager::TypeManager";
    mExtensionsAllowed = isNNAPIVendorExtensionsUseAllowed(getVendorExtensionAllowlistedApps());
    VLOG(MANAGER) << "NNAPI Vendor extensions enabled: " << mExtensionsAllowed;
    findAvailableExtensions();
}

bool TypeManager::isExtensionsUseAllowed(const AppPackageInfo& appPackageInfo,
                                         bool useOnProductImageEnabled,
                                         const std::vector<std::string>& allowlist) {
    // Only selected partitions and user-installed apps (/data)
    // are allowed to use extensions.
    if (StartsWith(appPackageInfo.binaryPath, "/vendor/") ||
        StartsWith(appPackageInfo.binaryPath, "/odm/") ||
        StartsWith(appPackageInfo.binaryPath, "/data/") ||
        (StartsWith(appPackageInfo.binaryPath, "/product/") && useOnProductImageEnabled)) {
#ifdef NN_DEBUGGABLE
        // Only on userdebug and eng builds.
        // When running tests with mma and adb push.
        if (StartsWith(appPackageInfo.binaryPath, "/data/nativetest") ||
            // When running tests with Atest.
            StartsWith(appPackageInfo.binaryPath, "/data/local/tmp/NeuralNetworksTest_")) {
            return true;
        }
#endif  // NN_DEBUGGABLE

        return std::find(allowlist.begin(), allowlist.end(), appPackageInfo.binaryPath) !=
               allowlist.end();
    } else if (appPackageInfo.binaryPath == "/system/bin/app_process64" ||
               appPackageInfo.binaryPath == "/system/bin/app_process32") {
        // App is not system app OR vendor app OR (product app AND product enabled)
        // AND app is on allowlist.
        return (!appPackageInfo.appIsSystemApp || appPackageInfo.appIsOnVendorImage ||
                (appPackageInfo.appIsOnProductImage && useOnProductImageEnabled)) &&
               std::find(allowlist.begin(), allowlist.end(), appPackageInfo.appPackageName) !=
                       allowlist.end();
    }
    return false;
}

void TypeManager::findAvailableExtensions() {
    for (const std::shared_ptr<Device>& device : mDeviceManager->getDrivers()) {
        for (const Extension extension : device->getSupportedExtensions()) {
            registerExtension(extension, device->getName());
        }
    }
}

bool TypeManager::registerExtension(Extension extension, const std::string& deviceName) {
    if (mDisabledExtensions.find(extension.name) != mDisabledExtensions.end()) {
        LOG(ERROR) << "Extension " << extension.name << " is disabled";
        return false;
    }

    std::sort(extension.operandTypes.begin(), extension.operandTypes.end(),
              [](const Extension::OperandTypeInformation& a,
                 const Extension::OperandTypeInformation& b) {
                  return static_cast<uint16_t>(a.type) < static_cast<uint16_t>(b.type);
              });

    std::map<std::string, Extension>::iterator it;
    bool isNew;
    std::tie(it, isNew) = mExtensionNameToExtension.emplace(extension.name, extension);
    if (isNew) {
        VLOG(MANAGER) << "Registered extension " << extension.name;
        mExtensionNameToFirstDevice.emplace(extension.name, deviceName);
    } else if (!equal(extension, it->second)) {
        LOG(ERROR) << "Devices " << mExtensionNameToFirstDevice[extension.name] << " and "
                   << deviceName << " provide inconsistent information for extension "
                   << extension.name << ", which is therefore disabled";
        mExtensionNameToExtension.erase(it);
        mDisabledExtensions.insert(extension.name);
        return false;
    }
    return true;
}

bool TypeManager::getExtensionPrefix(const std::string& extensionName, uint16_t* prefix) {
    auto it = mExtensionNameToPrefix.find(extensionName);
    if (it != mExtensionNameToPrefix.end()) {
        *prefix = it->second;
    } else {
        NN_RET_CHECK_LE(mPrefixToExtension.size(), kMaxPrefix) << "Too many extensions in use";
        *prefix = mPrefixToExtension.size();
        mExtensionNameToPrefix[extensionName] = *prefix;
        mPrefixToExtension.push_back(&mExtensionNameToExtension[extensionName]);
    }
    return true;
}

bool TypeManager::getExtensionType(const char* extensionName, uint16_t typeWithinExtension,
                                   int32_t* type) {
    uint16_t prefix;
    NN_RET_CHECK(getExtensionPrefix(extensionName, &prefix));
    *type = (prefix << kLowBitsType) | typeWithinExtension;
    return true;
}

bool TypeManager::getExtensionInfo(uint16_t prefix, const Extension** extension) const {
    NN_RET_CHECK_NE(prefix, 0u) << "prefix=0 does not correspond to an extension";
    NN_RET_CHECK_LT(prefix, mPrefixToExtension.size()) << "Unknown extension prefix";
    *extension = mPrefixToExtension[prefix];
    return true;
}

bool TypeManager::getExtensionOperandTypeInfo(
        OperandType type, const Extension::OperandTypeInformation** info) const {
    uint32_t operandType = static_cast<uint32_t>(type);
    uint16_t prefix = operandType >> kLowBitsType;
    uint16_t typeWithinExtension = operandType & ((1 << kLowBitsType) - 1);
    const Extension* extension;
    NN_RET_CHECK(getExtensionInfo(prefix, &extension))
            << "Cannot find extension corresponding to prefix " << prefix;
    auto it = std::lower_bound(
            extension->operandTypes.begin(), extension->operandTypes.end(), typeWithinExtension,
            [](const Extension::OperandTypeInformation& info, uint32_t typeSought) {
                return static_cast<uint16_t>(info.type) < typeSought;
            });
    NN_RET_CHECK(it != extension->operandTypes.end() &&
                 static_cast<uint16_t>(it->type) == typeWithinExtension)
            << "Cannot find operand type " << typeWithinExtension << " in extension "
            << extension->name;
    *info = &*it;
    return true;
}

bool TypeManager::isTensorType(OperandType type) const {
    if (!isExtensionOperandType(type)) {
        return !nonExtensionOperandTypeIsScalar(static_cast<int>(type));
    }
    const Extension::OperandTypeInformation* info;
    CHECK(getExtensionOperandTypeInfo(type, &info));
    return info->isTensor;
}

uint32_t TypeManager::getSizeOfData(OperandType type,
                                    const std::vector<uint32_t>& dimensions) const {
    if (!isExtensionOperandType(type)) {
        return nonExtensionOperandSizeOfData(type, dimensions);
    }

    const Extension::OperandTypeInformation* info;
    CHECK(getExtensionOperandTypeInfo(type, &info));

    if (!info->isTensor) {
        return info->byteSize;
    }

    if (dimensions.empty()) {
        return 0;
    }

    uint32_t size = info->byteSize;
    for (auto dimension : dimensions) {
        size *= dimension;
    }
    return size;
}

}  // namespace nn
}  // namespace android
