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

//#define LOG_NDEBUG 0
#define LOG_TAG "DrmSessionManager"
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IProcessInfoService.h>
#include <binder/IServiceManager.h>
#include <media/stagefright/ProcessInfo.h>
#include <mediadrm/DrmSessionClientInterface.h>
#include <mediadrm/DrmSessionManager.h>
#include <unistd.h>
#include <utils/String8.h>

namespace android {

static String8 GetSessionIdString(const Vector<uint8_t> &sessionId) {
    String8 sessionIdStr;
    for (size_t i = 0; i < sessionId.size(); ++i) {
        sessionIdStr.appendFormat("%u ", sessionId[i]);
    }
    return sessionIdStr;
}

bool isEqualSessionId(const Vector<uint8_t> &sessionId1, const Vector<uint8_t> &sessionId2) {
    if (sessionId1.size() != sessionId2.size()) {
        return false;
    }
    for (size_t i = 0; i < sessionId1.size(); ++i) {
        if (sessionId1[i] != sessionId2[i]) {
            return false;
        }
    }
    return true;
}

sp<DrmSessionManager> DrmSessionManager::Instance() {
    static sp<DrmSessionManager> drmSessionManager = new DrmSessionManager();
    return drmSessionManager;
}

DrmSessionManager::DrmSessionManager()
    : mProcessInfo(new ProcessInfo()),
      mTime(0) {}

DrmSessionManager::DrmSessionManager(sp<ProcessInfoInterface> processInfo)
    : mProcessInfo(processInfo),
      mTime(0) {}

DrmSessionManager::~DrmSessionManager() {}

void DrmSessionManager::addSession(
        int pid, const sp<DrmSessionClientInterface>& drm, const Vector<uint8_t> &sessionId) {
    ALOGV("addSession(pid %d, drm %p, sessionId %s)", pid, drm.get(),
            GetSessionIdString(sessionId).string());

    Mutex::Autolock lock(mLock);
    SessionInfo info;
    info.drm = drm;
    info.sessionId = sessionId;
    info.timeStamp = getTime_l();
    ssize_t index = mSessionMap.indexOfKey(pid);
    if (index < 0) {
        // new pid
        SessionInfos infosForPid;
        infosForPid.push_back(info);
        mSessionMap.add(pid, infosForPid);
    } else {
        mSessionMap.editValueAt(index).push_back(info);
    }
}

void DrmSessionManager::useSession(const Vector<uint8_t> &sessionId) {
    ALOGV("useSession(%s)", GetSessionIdString(sessionId).string());

    Mutex::Autolock lock(mLock);
    for (size_t i = 0; i < mSessionMap.size(); ++i) {
        SessionInfos& infos = mSessionMap.editValueAt(i);
        for (size_t j = 0; j < infos.size(); ++j) {
            SessionInfo& info = infos.editItemAt(j);
            if (isEqualSessionId(sessionId, info.sessionId)) {
                info.timeStamp = getTime_l();
                return;
            }
        }
    }
}

void DrmSessionManager::removeSession(const Vector<uint8_t> &sessionId) {
    ALOGV("removeSession(%s)", GetSessionIdString(sessionId).string());

    Mutex::Autolock lock(mLock);
    for (size_t i = 0; i < mSessionMap.size(); ++i) {
        SessionInfos& infos = mSessionMap.editValueAt(i);
        for (size_t j = 0; j < infos.size(); ++j) {
            if (isEqualSessionId(sessionId, infos[j].sessionId)) {
                infos.removeAt(j);
                return;
            }
        }
    }
}

void DrmSessionManager::removeDrm(const sp<DrmSessionClientInterface>& drm) {
    ALOGV("removeDrm(%p)", drm.get());

    Mutex::Autolock lock(mLock);
    bool found = false;
    for (size_t i = 0; i < mSessionMap.size(); ++i) {
        SessionInfos& infos = mSessionMap.editValueAt(i);
        for (size_t j = 0; j < infos.size();) {
            if (infos[j].drm == drm) {
                ALOGV("removed session (%s)", GetSessionIdString(infos[j].sessionId).string());
                j = infos.removeAt(j);
                found = true;
            } else {
                ++j;
            }
        }
        if (found) {
            break;
        }
    }
}

bool DrmSessionManager::reclaimSession(int callingPid) {
    ALOGV("reclaimSession(%d)", callingPid);

    sp<DrmSessionClientInterface> drm;
    Vector<uint8_t> sessionId;
    int lowestPriorityPid;
    int lowestPriority;
    {
        Mutex::Autolock lock(mLock);
        int callingPriority;
        if (!mProcessInfo->getPriority(callingPid, &callingPriority)) {
            return false;
        }
        if (!getLowestPriority_l(&lowestPriorityPid, &lowestPriority)) {
            return false;
        }
        if (lowestPriority <= callingPriority) {
            return false;
        }

        if (!getLeastUsedSession_l(lowestPriorityPid, &drm, &sessionId)) {
            return false;
        }
    }

    if (drm == NULL) {
        return false;
    }

    ALOGV("reclaim session(%s) opened by pid %d",
            GetSessionIdString(sessionId).string(), lowestPriorityPid);

    return drm->reclaimSession(sessionId);
}

int64_t DrmSessionManager::getTime_l() {
    return mTime++;
}

bool DrmSessionManager::getLowestPriority_l(int* lowestPriorityPid, int* lowestPriority) {
    int pid = -1;
    int priority = -1;
    for (size_t i = 0; i < mSessionMap.size(); ++i) {
        if (mSessionMap.valueAt(i).size() == 0) {
            // no opened session by this process.
            continue;
        }
        int tempPid = mSessionMap.keyAt(i);
        int tempPriority;
        if (!mProcessInfo->getPriority(tempPid, &tempPriority)) {
            // shouldn't happen.
            return false;
        }
        if (pid == -1) {
            pid = tempPid;
            priority = tempPriority;
        } else {
            if (tempPriority > priority) {
                pid = tempPid;
                priority = tempPriority;
            }
        }
    }
    if (pid != -1) {
        *lowestPriorityPid = pid;
        *lowestPriority = priority;
    }
    return (pid != -1);
}

bool DrmSessionManager::getLeastUsedSession_l(
        int pid, sp<DrmSessionClientInterface>* drm, Vector<uint8_t>* sessionId) {
    ssize_t index = mSessionMap.indexOfKey(pid);
    if (index < 0) {
        return false;
    }

    int leastUsedIndex = -1;
    int64_t minTs = LLONG_MAX;
    const SessionInfos& infos = mSessionMap.valueAt(index);
    for (size_t j = 0; j < infos.size(); ++j) {
        if (leastUsedIndex == -1) {
            leastUsedIndex = j;
            minTs = infos[j].timeStamp;
        } else {
            if (infos[j].timeStamp < minTs) {
                leastUsedIndex = j;
                minTs = infos[j].timeStamp;
            }
        }
    }
    if (leastUsedIndex != -1) {
        *drm = infos[leastUsedIndex].drm;
        *sessionId = infos[leastUsedIndex].sessionId;
    }
    return (leastUsedIndex != -1);
}

}  // namespace android
