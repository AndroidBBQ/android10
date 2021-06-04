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

#ifndef ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_ID_GENERATOR_H
#define ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_ID_GENERATOR_H

#include <mutex>
#include <set>

#include <utils/Mutex.h>

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

// A thread-safe, non-negative, incremental, int id generator.
class BufferHubIdGenerator {
public:
    // Get the singleton instance of this class
    static BufferHubIdGenerator& getInstance();

    // Gets next available id. If next id is greater than std::numeric_limits<int32_t>::max(), it
    // will try to get an id start from 0 again.
    int getId();

    // Free a specific id.
    void freeId(int id);

private:
    BufferHubIdGenerator() = default;
    ~BufferHubIdGenerator() = default;

    // Start from -1 so all valid ids will be >= 0
    int mLastId = -1;

    std::mutex mIdsInUseMutex;
    std::set<int> mIdsInUse GUARDED_BY(mIdsInUseMutex);
};

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android

#endif // ANDROID_FRAMEWORKS_BUFFERHUB_V1_0_ID_GENERATOR_H
