/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "MockSessionLibrary"

#include <utils/Log.h>
#include <utils/String8.h>
#include "MockSessionLibrary.h"

namespace android {

Mutex MockSessionLibrary::sSingletonLock;
MockSessionLibrary* MockSessionLibrary::sSingleton = NULL;

MockSessionLibrary* MockSessionLibrary::get() {
    Mutex::Autolock lock(sSingletonLock);

    if (sSingleton == NULL) {
        ALOGD("Instantiating Session Library Singleton.");
        sSingleton = new MockSessionLibrary();
    }

    return sSingleton;
}

MockSessionLibrary::MockSessionLibrary() : mNextSessionId(1) {}

status_t MockSessionLibrary::addSession(
        CasPlugin *plugin, CasSessionId *sessionId) {
    Mutex::Autolock lock(mSessionsLock);

    sp<MockCasSession> session = new MockCasSession(plugin);

    uint8_t *byteArray = (uint8_t *) &mNextSessionId;
    sessionId->push_back(byteArray[3]);
    sessionId->push_back(byteArray[2]);
    sessionId->push_back(byteArray[1]);
    sessionId->push_back(byteArray[0]);
    mNextSessionId++;

    mIDToSessionMap.add(*sessionId, session);
    return OK;
}

sp<MockCasSession> MockSessionLibrary::findSession(
        const CasSessionId& sessionId) {
    Mutex::Autolock lock(mSessionsLock);

    ssize_t index = mIDToSessionMap.indexOfKey(sessionId);
    if (index < 0) {
        return NULL;
    }
    return mIDToSessionMap.valueFor(sessionId);
}

void MockSessionLibrary::destroySession(const CasSessionId& sessionId) {
    Mutex::Autolock lock(mSessionsLock);

    ssize_t index = mIDToSessionMap.indexOfKey(sessionId);
    if (index < 0) {
        return;
    }

    sp<MockCasSession> session = mIDToSessionMap.valueAt(index);
    mIDToSessionMap.removeItemsAt(index);
}

void MockSessionLibrary::destroyPlugin(CasPlugin *plugin) {
    Mutex::Autolock lock(mSessionsLock);

    for (ssize_t index = mIDToSessionMap.size() - 1; index >= 0; index--) {
        sp<MockCasSession> session = mIDToSessionMap.valueAt(index);
        if (session->getPlugin() == plugin) {
            mIDToSessionMap.removeItemsAt(index);
        }
    }
}

} // namespace android
