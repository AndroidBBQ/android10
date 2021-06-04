/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROID_MEDIAUTILS_SERVICEUTILITIES_H
#define ANDROID_MEDIAUTILS_SERVICEUTILITIES_H

#include <unistd.h>

#include <android/content/pm/IPackageManagerNative.h>
#include <binder/IMemory.h>
#include <binder/PermissionController.h>
#include <cutils/multiuser.h>
#include <private/android_filesystem_config.h>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace android {

// Audio permission utilities

// Used for calls that should originate from system services.
// We allow that some services might have separate processes to
// handle multiple users, e.g. u10_system, u10_bluetooth, u10_radio.
static inline bool isServiceUid(uid_t uid) {
    return multiuser_get_app_id(uid) < AID_APP_START;
}

// Used for calls that should originate from audioserver.
static inline bool isAudioServerUid(uid_t uid) {
    return uid == AID_AUDIOSERVER;
}

// Used for some permission checks.
// AID_ROOT is OK for command-line tests.  Native audioserver always OK.
static inline bool isAudioServerOrRootUid(uid_t uid) {
    return uid == AID_AUDIOSERVER || uid == AID_ROOT;
}

// Used for calls that should come from system server or internal.
// Note: system server is multiprocess for multiple users.  audioserver is not.
static inline bool isAudioServerOrSystemServerUid(uid_t uid) {
    return multiuser_get_app_id(uid) == AID_SYSTEM || uid == AID_AUDIOSERVER;
}

// used for calls that should come from system_server or audio_server and
// include AID_ROOT for command-line tests.
static inline bool isAudioServerOrSystemServerOrRootUid(uid_t uid) {
    return multiuser_get_app_id(uid) == AID_SYSTEM || uid == AID_AUDIOSERVER || uid == AID_ROOT;
}

// Mediaserver may forward the client PID and UID as part of a binder interface call;
// otherwise the calling UID must be equal to the client UID.
static inline bool isAudioServerOrMediaServerUid(uid_t uid) {
    switch (uid) {
    case AID_MEDIA:
    case AID_AUDIOSERVER:
        return true;
    default:
        return false;
    }
}

bool recordingAllowed(const String16& opPackageName, pid_t pid, uid_t uid);
bool startRecording(const String16& opPackageName, pid_t pid, uid_t uid);
void finishRecording(const String16& opPackageName, uid_t uid);
bool captureAudioOutputAllowed(pid_t pid, uid_t uid);
bool captureMediaOutputAllowed(pid_t pid, uid_t uid);
bool captureHotwordAllowed(const String16& opPackageName, pid_t pid, uid_t uid);
bool settingsAllowed();
bool modifyAudioRoutingAllowed();
bool modifyDefaultAudioEffectsAllowed();
bool dumpAllowed();
bool modifyPhoneStateAllowed(pid_t pid, uid_t uid);
bool bypassInterruptionPolicyAllowed(pid_t pid, uid_t uid);

status_t checkIMemory(const sp<IMemory>& iMemory);

class MediaPackageManager {
public:
    /** Query the PackageManager to check if all apps of an UID allow playback capture. */
    bool allowPlaybackCapture(uid_t uid) {
        auto result = doIsAllowed(uid);
        if (!result) {
            mPackageManagerErrors++;
        }
        return result.value_or(false);
    }
    void dump(int fd, int spaces = 0) const;
private:
    static constexpr const char* nativePackageManagerName = "package_native";
    std::optional<bool> doIsAllowed(uid_t uid);
    sp<content::pm::IPackageManagerNative> retreivePackageManager();
    sp<content::pm::IPackageManagerNative> mPackageManager; // To check apps manifest
    uint_t mPackageManagerErrors = 0;
    struct Package {
        std::string name;
        bool playbackCaptureAllowed = false;
    };
    using Packages = std::vector<Package>;
    std::map<uid_t, Packages> mDebugLog;
};
}

#endif // ANDROID_MEDIAUTILS_SERVICEUTILITIES_H
