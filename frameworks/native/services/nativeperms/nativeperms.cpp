/*
 * Copyright 2016 The Android Open Source Project
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

#include <base/at_exit.h>
#include <base/logging.h>
#include <base/message_loop/message_loop.h>
#include <binder/IServiceManager.h>
#include <binder/Status.h>
#include <brillo/binder_watcher.h>
#include <brillo/message_loops/base_message_loop.h>
#include <brillo/syslog_logging.h>
#include <utils/String16.h>

#include "android/os/BnPermissionController.h"

namespace {
static android::String16 serviceName("permission");
}

namespace android {

class PermissionService : public android::os::BnPermissionController {
   public:
    ::android::binder::Status checkPermission(
            const ::android::String16& permission, int32_t pid, int32_t uid,
            bool* _aidl_return) {
        (void)permission;
        (void)pid;
        (void)uid;
        *_aidl_return = true;
        return binder::Status::ok();
    }

    ::android::binder::Status getPackagesForUid(
            int32_t uid, ::std::vector<::android::String16>* _aidl_return) {
        (void)uid;
        // Brillo doesn't currently have installable packages.
        if (_aidl_return) {
            _aidl_return->clear();
        }
        return binder::Status::ok();
    }

    ::android::binder::Status isRuntimePermission(
            const ::android::String16& permission, bool* _aidl_return) {
        (void)permission;
        // Brillo doesn't currently have runtime permissions.
        *_aidl_return = false;
        return binder::Status::ok();
    }
};

}  // namespace android

int main() {
    base::AtExitManager atExitManager;
    brillo::InitLog(brillo::kLogToSyslog);
    // Register the service with servicemanager.
    android::status_t status = android::defaultServiceManager()->addService(
            serviceName, new android::PermissionService());
    CHECK(status == android::OK) << "Failed to get IPermissionController "
                                    "binder from servicemanager.";

    // Create a message loop.
    base::MessageLoopForIO messageLoopForIo;
    brillo::BaseMessageLoop messageLoop{&messageLoopForIo};

    // Initialize a binder watcher.
    brillo::BinderWatcher watcher(&messageLoop);
    watcher.Init();

    // Run the message loop.
    messageLoop.Run();
}
