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

#ifndef ANDROID_ML_NN_RUNTIME_EXTENSION_MANAGER_H
#define ANDROID_ML_NN_RUNTIME_EXTENSION_MANAGER_H

#include "HalInterfaces.h"
#include "Manager.h"

#include <map>
#include <set>
#include <string>

namespace android {
namespace nn {

// Manages runtime operand and operation type information.
//
// This class gathers information about extension types from all devices
// and provides a unified way to access information about any known type.
class TypeManager {
   public:
    static TypeManager* get() {
        static TypeManager manager;
        return &manager;
    }

    // Creates an operand/operation type corresponding to a given extension
    // name and type within extension.
    //
    // Returns false if the extension is unknown.
    bool getExtensionType(const char* extensionName, uint16_t typeWithinExtension, int32_t* type);

    // Looks up information about the extension corresponding to the given prefix
    //
    // Returns false if no extension corresponds to the given prefix.
    bool getExtensionInfo(uint16_t prefix, const Extension** extension) const;

    // Looks up information about an extension operand type
    //
    // Returns false if the extension or type is unknown.
    bool getExtensionOperandTypeInfo(OperandType type,
                                     const Extension::OperandTypeInformation** info) const;

    // Returns true if an operand type is a tensor type.
    //
    // Aborts if the type is an unknown extension type.
    bool isTensorType(OperandType type) const;

    // Returns the amount of space needed to store a value of the dimensions and
    // type of this operand. For a tensor with unspecified rank or at least one
    // unspecified dimension, returns zero.
    //
    // Aborts if the type is an unknown extension type.
    uint32_t getSizeOfData(const Operand& operand) const {
        return getSizeOfData(operand.type, operand.dimensions);
    }

    // Returns the amount of space needed to store a value of the specified
    // dimensions and type. For a tensor with unspecified rank or at least one
    // unspecified dimension, returns zero.
    //
    // Aborts if the type is an unknown extension type.
    uint32_t getSizeOfData(OperandType type, const std::vector<uint32_t>& dimensions) const;

    // Returns true if extensions usage is allowed in current process.
    bool areExtensionsAllowed() const { return mExtensionsAllowed; }

    // This method is intended for use only by internal unit tests.
    //
    // Registers an extension.
    //
    // Returns true if the registration was successful.
    bool forTest_registerExtension(const Extension& extension) {
        return registerExtension(extension, "INTERNAL TEST");
    }

    // This method is intended for use only by internal unit tests.
    //
    // Resets the internal state.
    //
    // After calling forTest_registerExtension() any number of times, call
    // forTest_reset() to return to the state as if forTest_registerExtension()
    // had never been called. Note that forTest_reset() resets all internal
    // state (including assigned prefixes) and re-discovers extensions from
    // available devices.
    void forTest_reset() { *this = TypeManager(); }

    // Collection of app-related arguments for the isExtensionsUseAllowed method.
    struct AppPackageInfo {
        // Path of the binary (/proc/$PID/exe)
        std::string binaryPath;
        // Package name of the Android app (empty string if not Android app).
        std::string appPackageName;
        // Is the app a system app? (false if not an Android app)
        bool appIsSystemApp;
        // Is the app preinstalled on vendor image? (false if not an Android app)
        bool appIsOnVendorImage;
        // Is the app preinstalled on product image? (false if not an Android app)
        bool appIsOnProductImage;
    };

    // Check if NNAPI Vendor extensions are usable in the process with the given app
    // and supplemental infomation.
    //
    // useOnProductImageEnabled - whether apps/binaries preinstalled on /product partition
    // can be enabled for extensions use.
    // allowlist - list of apps/binaries which are allowed to use extensions.
    static bool isExtensionsUseAllowed(const AppPackageInfo& appPackageInfo,
                                       bool useOnProductImageEnabled,
                                       const std::vector<std::string>& allowlist);

   private:
    TypeManager();
    void findAvailableExtensions();
    bool registerExtension(Extension extension, const std::string& deviceName);

    // Returns the numeric "prefix" value corresponding to an extension.
    //
    // Returns false when assigning a new prefix would overflow uint16_t.
    bool getExtensionPrefix(const std::string& extensionName, uint16_t* prefix);

    const DeviceManager* mDeviceManager = DeviceManager::get();

    // Contains all registered extensions.
    std::map<std::string, Extension> mExtensionNameToExtension;

    // Contains the name of the first discovered device that supports an
    // extension. Used for error reporting.
    std::map<std::string, std::string> mExtensionNameToFirstDevice;

    // When multiple devices report conflicting information about an extension,
    // the extension is disabled.
    std::set<std::string> mDisabledExtensions;

    // The fields below are used to support efficient extension name to
    // prefix mapping. New prefixes are created by getExtensionPrefix.
    std::map<std::string, uint16_t> mExtensionNameToPrefix;
    // Entries of mPrefixToExtension point into mExtensionNameToExtension.
    // prefix=0 corresponds to no extension and should never be looked up.
    std::vector<Extension*> mPrefixToExtension = {nullptr};

    // True if Extensions can be used in current process.
    bool mExtensionsAllowed = false;
};

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_RUNTIME_EXTENSION_MANAGER_H
