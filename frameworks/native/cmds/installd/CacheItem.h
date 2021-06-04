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

#ifndef ANDROID_INSTALLD_CACHE_ITEM_H
#define ANDROID_INSTALLD_CACHE_ITEM_H

#include <memory>
#include <string>

#include <fts.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <android-base/macros.h>

namespace android {
namespace installd {

/**
 * Single cache item that can be purged to free up space. This may be an
 * isolated file, or an entire directory tree that should be deleted as a
 * group.
 */
class CacheItem {
public:
    CacheItem(FTSENT* p);
    ~CacheItem();

    std::string toString();
    std::string buildPath();

    int purge();

    short level;
    bool directory;
    bool group;
    bool tombstone;
    int64_t size;
    time_t modified;

private:
    CacheItem* mParent;
    std::string mName;

    DISALLOW_COPY_AND_ASSIGN(CacheItem);
};

}  // namespace installd
}  // namespace android

#endif  // ANDROID_INSTALLD_CACHE_ITEM_H
