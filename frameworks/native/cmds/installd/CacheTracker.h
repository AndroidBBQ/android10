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

#ifndef ANDROID_INSTALLD_CACHE_TRACKER_H
#define ANDROID_INSTALLD_CACHE_TRACKER_H

#include <memory>
#include <string>
#include <queue>

#include <sys/types.h>
#include <sys/stat.h>

#include <android-base/macros.h>
#include <cutils/multiuser.h>

#include "CacheItem.h"

namespace android {
namespace installd {

/**
 * Cache tracker for a single UID. Each tracker is used in two modes: first
 * for loading lightweight "stats", and then by loading detailed "items"
 * which can then be purged to free up space.
 */
class CacheTracker {
public:
    CacheTracker(userid_t userId, appid_t appId, const std::string& uuid);
    ~CacheTracker();

    std::string toString();

    void addDataPath(const std::string& dataPath);

    void loadStats();
    void loadItems();

    void ensureItems();

    int getCacheRatio();

    int64_t cacheUsed;
    int64_t cacheQuota;

    std::vector<std::shared_ptr<CacheItem>> items;

private:
    userid_t mUserId;
    appid_t mAppId;
    bool mItemsLoaded;
    const std::string& mUuid;

    std::vector<std::string> mDataPaths;

    bool loadQuotaStats();
    void loadItemsFrom(const std::string& path);

    DISALLOW_COPY_AND_ASSIGN(CacheTracker);
};

}  // namespace installd
}  // namespace android

#endif  // ANDROID_INSTALLD_CACHE_TRACKER_H
