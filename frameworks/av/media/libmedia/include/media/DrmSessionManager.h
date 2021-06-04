/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef DRM_SESSION_MANAGER_H_

#define DRM_SESSION_MANAGER_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <utils/Vector.h>

namespace android {

class DrmSessionManagerTest;
struct DrmSessionClientInterface;
struct ProcessInfoInterface;

bool isEqualSessionId(const Vector<uint8_t> &sessionId1, const Vector<uint8_t> &sessionId2);

struct SessionInfo {
    sp<DrmSessionClientInterface> drm;
    Vector<uint8_t> sessionId;
    int64_t timeStamp;
};

typedef Vector<SessionInfo > SessionInfos;
typedef KeyedVector<int, SessionInfos > PidSessionInfosMap;

struct DrmSessionManager : public RefBase {
    static sp<DrmSessionManager> Instance();

    DrmSessionManager();
    explicit DrmSessionManager(sp<ProcessInfoInterface> processInfo);

    void addSession(int pid, const sp<DrmSessionClientInterface>& drm, const Vector<uint8_t>& sessionId);
    void useSession(const Vector<uint8_t>& sessionId);
    void removeSession(const Vector<uint8_t>& sessionId);
    void removeDrm(const sp<DrmSessionClientInterface>& drm);
    bool reclaimSession(int callingPid);

protected:
    virtual ~DrmSessionManager();

private:
    friend class DrmSessionManagerTest;

    int64_t getTime_l();
    bool getLowestPriority_l(int* lowestPriorityPid, int* lowestPriority);
    bool getLeastUsedSession_l(
            int pid, sp<DrmSessionClientInterface>* drm, Vector<uint8_t>* sessionId);

    sp<ProcessInfoInterface> mProcessInfo;
    mutable Mutex mLock;
    PidSessionInfosMap mSessionMap;
    int64_t mTime;

    DISALLOW_EVIL_CONSTRUCTORS(DrmSessionManager);
};

}  // namespace android

#endif  // DRM_SESSION_MANAGER_H_
