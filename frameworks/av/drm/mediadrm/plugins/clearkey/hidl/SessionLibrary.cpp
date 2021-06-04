/*
 * Copyright (C) 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "hidl_ClearKeySessionLibrary"
#include <utils/Log.h>

#include "SessionLibrary.h"
#include "Utils.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;

Mutex SessionLibrary::sSingletonLock;
SessionLibrary* SessionLibrary::sSingleton = NULL;

SessionLibrary* SessionLibrary::get() {
    Mutex::Autolock lock(sSingletonLock);

    if (sSingleton == NULL) {
        ALOGD("Instantiating Session Library Singleton.");
        sSingleton = new SessionLibrary();
    }

    return sSingleton;
}

sp<Session> SessionLibrary::createSession() {
    Mutex::Autolock lock(mSessionsLock);

    char sessionIdRaw[16];
    snprintf(sessionIdRaw, sizeof(sessionIdRaw), "%u", mNextSessionId);

    mNextSessionId += 1;

    std::vector<uint8_t> sessionId;
    sessionId.insert(sessionId.end(), sessionIdRaw,
            sessionIdRaw + sizeof(sessionIdRaw) / sizeof(uint8_t));

    mSessions.insert(std::pair<std::vector<uint8_t>,
            sp<Session> >(sessionId, new Session(sessionId)));
    std::map<std::vector<uint8_t>, sp<Session> >::iterator itr =
            mSessions.find(sessionId);
    if (itr != mSessions.end()) {
        return itr->second;
    } else {
        return nullptr;
    }
}

sp<Session> SessionLibrary::findSession(
        const std::vector<uint8_t>& sessionId) {
    Mutex::Autolock lock(mSessionsLock);
    std::map<std::vector<uint8_t>, sp<Session> >::iterator itr =
            mSessions.find(sessionId);
    if (itr != mSessions.end()) {
        return itr->second;
    } else {
        return nullptr;
    }
}

void SessionLibrary::destroySession(const sp<Session>& session) {
    Mutex::Autolock lock(mSessionsLock);
    mSessions.erase(session->sessionId());
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android
