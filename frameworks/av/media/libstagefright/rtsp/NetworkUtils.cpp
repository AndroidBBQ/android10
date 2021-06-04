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
#define LOG_TAG "NetworkUtils"
#include <utils/Log.h>

#include "NetworkUtils.h"
#include <cutils/qtaguid.h>
#include <NetdClient.h>

namespace android {

// static
void NetworkUtils::RegisterSocketUserTag(int sockfd, uid_t uid, uint32_t kTag) {
    int res = qtaguid_tagSocket(sockfd, kTag, uid);
    if (res != 0) {
        ALOGE("Failed tagging socket %d for uid %d (My UID=%d)", sockfd, uid, geteuid());
    }
}

// static
void NetworkUtils::UnRegisterSocketUserTag(int sockfd) {
    int res = qtaguid_untagSocket(sockfd);
    if (res != 0) {
        ALOGE("Failed untagging socket %d (My UID=%d)", sockfd, geteuid());
    }
}

// static
void NetworkUtils::RegisterSocketUserMark(int sockfd, uid_t uid) {
    setNetworkForUser(uid, sockfd);
}

// static
void NetworkUtils::UnRegisterSocketUserMark(int sockfd) {
    RegisterSocketUserMark(sockfd, geteuid());
}

}  // namespace android
