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

#define ATRACE_TAG ATRACE_TAG_PACKAGE_MANAGER

#include "CacheTracker.h"

#include <fts.h>
#include <sys/xattr.h>
#include <utils/Trace.h>

#include <android-base/logging.h>
#include <android-base/stringprintf.h>

#include "QuotaUtils.h"
#include "utils.h"

using android::base::StringPrintf;

namespace android {
namespace installd {

CacheTracker::CacheTracker(userid_t userId, appid_t appId, const std::string& uuid)
      : cacheUsed(0),
        cacheQuota(0),
        mUserId(userId),
        mAppId(appId),
        mItemsLoaded(false),
        mUuid(uuid) {
}

CacheTracker::~CacheTracker() {
}

std::string CacheTracker::toString() {
    return StringPrintf("UID=%d used=%" PRId64 " quota=%" PRId64 " ratio=%d",
            multiuser_get_uid(mUserId, mAppId), cacheUsed, cacheQuota, getCacheRatio());
}

void CacheTracker::addDataPath(const std::string& dataPath) {
    mDataPaths.push_back(dataPath);
}

void CacheTracker::loadStats() {
    ATRACE_BEGIN("loadStats quota");
    cacheUsed = 0;
    if (loadQuotaStats()) {
        return;
    }
    ATRACE_END();

    ATRACE_BEGIN("loadStats tree");
    cacheUsed = 0;
    for (const auto& path : mDataPaths) {
        auto cachePath = read_path_inode(path, "cache", kXattrInodeCache);
        auto codeCachePath = read_path_inode(path, "code_cache", kXattrInodeCodeCache);
        calculate_tree_size(cachePath, &cacheUsed);
        calculate_tree_size(codeCachePath, &cacheUsed);
    }
    ATRACE_END();
}

bool CacheTracker::loadQuotaStats() {
    int cacheGid = multiuser_get_cache_gid(mUserId, mAppId);
    int extCacheGid = multiuser_get_ext_cache_gid(mUserId, mAppId);
    if (IsQuotaSupported(mUuid) && cacheGid != -1 && extCacheGid != -1) {
        int64_t space;
        if ((space = GetOccupiedSpaceForGid(mUuid, cacheGid)) != -1) {
            cacheUsed += space;
        } else {
            return false;
        }

        if ((space = GetOccupiedSpaceForGid(mUuid, extCacheGid)) != -1) {
            cacheUsed += space;
        } else {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

void CacheTracker::loadItemsFrom(const std::string& path) {
    FTS *fts;
    FTSENT *p;
    char *argv[] = { (char*) path.c_str(), nullptr };
    if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
        PLOG(WARNING) << "Failed to fts_open " << path;
        return;
    }
    while ((p = fts_read(fts)) != nullptr) {
        if (p->fts_level == 0) continue;

        // Create tracking nodes for everything we encounter
        switch (p->fts_info) {
        case FTS_D:
        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE: {
            auto item = std::shared_ptr<CacheItem>(new CacheItem(p));
            p->fts_pointer = static_cast<void*>(item.get());
            items.push_back(item);
        }
        }

        switch (p->fts_info) {
        case FTS_D: {
            auto item = static_cast<CacheItem*>(p->fts_pointer);
            item->group |= (getxattr(p->fts_path, kXattrCacheGroup, nullptr, 0) >= 0);
            item->tombstone |= (getxattr(p->fts_path, kXattrCacheTombstone, nullptr, 0) >= 0);

            // When group, immediately collect all files under tree
            if (item->group) {
                while ((p = fts_read(fts)) != nullptr) {
                    if (p->fts_info == FTS_DP && p->fts_level == item->level) break;
                    switch (p->fts_info) {
                    case FTS_D:
                    case FTS_DEFAULT:
                    case FTS_F:
                    case FTS_SL:
                    case FTS_SLNONE:
                        item->size += p->fts_statp->st_blocks * 512;
                        item->modified = std::max(item->modified, p->fts_statp->st_mtime);
                    }
                }
            }
        }
        }

        // Bubble up modified time to parent
        CHECK(p != nullptr);
        switch (p->fts_info) {
        case FTS_DP:
        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE: {
            auto item = static_cast<CacheItem*>(p->fts_pointer);
            auto parent = static_cast<CacheItem*>(p->fts_parent->fts_pointer);
            if (parent) {
                parent->modified = std::max(parent->modified, item->modified);
            }
        }
        }
    }
    fts_close(fts);
}

void CacheTracker::loadItems() {
    items.clear();

    ATRACE_BEGIN("loadItems");
    for (const auto& path : mDataPaths) {
        loadItemsFrom(read_path_inode(path, "cache", kXattrInodeCache));
        loadItemsFrom(read_path_inode(path, "code_cache", kXattrInodeCodeCache));
    }
    ATRACE_END();

    ATRACE_BEGIN("sortItems");
    auto cmp = [](std::shared_ptr<CacheItem> left, std::shared_ptr<CacheItem> right) {
        // TODO: sort dotfiles last
        // TODO: sort code_cache last
        if (left->modified != right->modified) {
            return (left->modified > right->modified);
        }
        if (left->level != right->level) {
            return (left->level < right->level);
        }
        return left->directory;
    };
    std::stable_sort(items.begin(), items.end(), cmp);
    ATRACE_END();
}

void CacheTracker::ensureItems() {
    if (mItemsLoaded) {
        return;
    } else {
        loadItems();
        mItemsLoaded = true;
    }
}

int CacheTracker::getCacheRatio() {
    if (cacheQuota == 0) {
        return 0;
    } else {
        return (cacheUsed * 10000) / cacheQuota;
    }
}

}  // namespace installd
}  // namespace android
