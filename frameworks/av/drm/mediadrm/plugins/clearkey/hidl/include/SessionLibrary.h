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

#ifndef CLEARKEY_SESSION_LIBRARY_H_
#define CLEARKEY_SESSION_LIBRARY_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>

#include "ClearKeyTypes.h"
#include "Session.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::sp;

class SessionLibrary : public RefBase {
public:
    static SessionLibrary* get();

    sp<Session> createSession();

    sp<Session> findSession(
            const std::vector<uint8_t>& sessionId);

    void destroySession(const sp<Session>& session);

    size_t numOpenSessions() const { return mSessions.size(); }

private:
    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(SessionLibrary);

    SessionLibrary() : mNextSessionId(1) {}

    static Mutex sSingletonLock;
    static SessionLibrary* sSingleton;

    Mutex mSessionsLock;
    uint32_t mNextSessionId;
    std::map<std::vector<uint8_t>, sp<Session> > mSessions;
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_SESSION_LIBRARY_H_
