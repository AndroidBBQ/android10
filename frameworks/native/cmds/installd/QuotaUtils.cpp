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

#include "QuotaUtils.h"

#include <fstream>
#include <unordered_map>

#include <sys/quota.h>

#include <android-base/logging.h>

#include "utils.h"

namespace android {
namespace installd {

namespace {

std::recursive_mutex mMountsLock;

/* Map of all quota mounts from target to source */
std::unordered_map<std::string, std::string> mQuotaReverseMounts;

std::string& FindQuotaDeviceForUuid(const std::string& uuid) {
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);
    auto path = create_data_path(uuid.empty() ? nullptr : uuid.c_str());
    return mQuotaReverseMounts[path];
}

} // namespace

bool InvalidateQuotaMounts() {
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);

    mQuotaReverseMounts.clear();

    std::ifstream in("/proc/mounts");
    if (!in.is_open()) {
        return false;
    }

    std::string source;
    std::string target;
    std::string ignored;
    while (!in.eof()) {
        std::getline(in, source, ' ');
        std::getline(in, target, ' ');
        std::getline(in, ignored);

        if (source.compare(0, 11, "/dev/block/") == 0) {
            struct dqblk dq;
            if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), source.c_str(), 0,
                    reinterpret_cast<char*>(&dq)) == 0) {
                LOG(DEBUG) << "Found quota mount " << source << " at " << target;
                mQuotaReverseMounts[target] = source;
            }
        }
    }
    return true;
}

bool IsQuotaSupported(const std::string& uuid) {
    return !FindQuotaDeviceForUuid(uuid).empty();
}

int64_t GetOccupiedSpaceForUid(const std::string& uuid, uid_t uid) {
    const std::string device = FindQuotaDeviceForUuid(uuid);
    if (device == "") {
        return -1;
    }
    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid,
            reinterpret_cast<char*>(&dq)) != 0) {
        if (errno != ESRCH) {
            PLOG(ERROR) << "Failed to quotactl " << device << " for UID " << uid;
        }
        return -1;
    } else {
#if MEASURE_DEBUG
        LOG(DEBUG) << "quotactl() for UID " << uid << " " << dq.dqb_curspace;
#endif
        return dq.dqb_curspace;
    }
}

int64_t GetOccupiedSpaceForGid(const std::string& uuid, gid_t gid) {
    const std::string device = FindQuotaDeviceForUuid(uuid);
    if (device == "") {
        return -1;
    }
    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, GRPQUOTA), device.c_str(), gid,
            reinterpret_cast<char*>(&dq)) != 0) {
        if (errno != ESRCH) {
            PLOG(ERROR) << "Failed to quotactl " << device << " for GID " << gid;
        }
        return -1;
    } else {
#if MEASURE_DEBUG
        LOG(DEBUG) << "quotactl() for GID " << gid << " " << dq.dqb_curspace;
#endif
        return dq.dqb_curspace;
    }

}

}  // namespace installd
}  // namespace android
