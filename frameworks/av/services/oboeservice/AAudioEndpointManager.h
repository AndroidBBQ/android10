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

#ifndef AAUDIO_AAUDIO_ENDPOINT_MANAGER_H
#define AAUDIO_AAUDIO_ENDPOINT_MANAGER_H

#include <map>
#include <mutex>
#include <utils/Singleton.h>

#include "binding/AAudioServiceMessage.h"
#include "AAudioServiceEndpoint.h"
#include "AAudioServiceEndpointCapture.h"
#include "AAudioServiceEndpointMMAP.h"
#include "AAudioServiceEndpointPlay.h"

namespace aaudio {

class AAudioEndpointManager : public android::Singleton<AAudioEndpointManager> {
public:
    AAudioEndpointManager();
    ~AAudioEndpointManager() = default;

    /**
     * Returns information about the state of the this class.
     *
     * Will attempt to get the object lock, but will proceed
     * even if it cannot.
     *
     * Each line of information ends with a newline.
     *
     * @return a string with useful information
     */
    std::string dump() const;

    /**
     * Find a service endpoint for the given deviceId, sessionId and direction.
     * If an endpoint does not already exist then try to create one.
     *
     * @param audioService
     * @param request
     * @param sharingMode
     * @return endpoint or null
     */
    android::sp<AAudioServiceEndpoint> openEndpoint(android::AAudioService &audioService,
                                        const aaudio::AAudioStreamRequest &request);

    void closeEndpoint(android::sp<AAudioServiceEndpoint> serviceEndpoint);

private:
    android::sp<AAudioServiceEndpoint> openExclusiveEndpoint(android::AAudioService &aaudioService,
                                                 const aaudio::AAudioStreamRequest &request);

    android::sp<AAudioServiceEndpoint> openSharedEndpoint(android::AAudioService &aaudioService,
                                              const aaudio::AAudioStreamRequest &request);

    android::sp<AAudioServiceEndpoint> findExclusiveEndpoint_l(
            const AAudioStreamConfiguration& configuration);

    android::sp<AAudioServiceEndpointShared> findSharedEndpoint_l(
            const AAudioStreamConfiguration& configuration);

    void closeExclusiveEndpoint(android::sp<AAudioServiceEndpoint> serviceEndpoint);
    void closeSharedEndpoint(android::sp<AAudioServiceEndpoint> serviceEndpoint);

    // Use separate locks because opening a Shared endpoint requires opening an Exclusive one.
    // That could cause a recursive lock.
    // Lock mSharedLock before mExclusiveLock.
    // it is OK to only lock mExclusiveLock.
    mutable std::mutex                                     mSharedLock;
    std::vector<android::sp<AAudioServiceEndpointShared>>  mSharedStreams;

    mutable std::mutex                                     mExclusiveLock;
    std::vector<android::sp<AAudioServiceEndpointMMAP>>    mExclusiveStreams;

    // Modified under a lock.
    int32_t mExclusiveSearchCount = 0; // number of times we SEARCHED for an exclusive endpoint
    int32_t mExclusiveFoundCount  = 0; // number of times we FOUND an exclusive endpoint
    int32_t mExclusiveOpenCount   = 0; // number of times we OPENED an exclusive endpoint
    int32_t mExclusiveCloseCount  = 0; // number of times we CLOSED an exclusive endpoint
    // Same as above but for SHARED endpoints.
    int32_t mSharedSearchCount    = 0;
    int32_t mSharedFoundCount     = 0;
    int32_t mSharedOpenCount      = 0;
    int32_t mSharedCloseCount     = 0;
};
} /* namespace aaudio */

#endif //AAUDIO_AAUDIO_ENDPOINT_MANAGER_H
