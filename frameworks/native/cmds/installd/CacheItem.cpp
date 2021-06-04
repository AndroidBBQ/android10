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

#include "CacheItem.h"

#include <inttypes.h>
#include <stdint.h>
#include <sys/xattr.h>

#include <android-base/logging.h>
#include <android-base/stringprintf.h>

#include "utils.h"

using android::base::StringPrintf;

namespace android {
namespace installd {

CacheItem::CacheItem(FTSENT* p) {
    level = p->fts_level;
    directory = S_ISDIR(p->fts_statp->st_mode);
    size = p->fts_statp->st_blocks * 512;
    modified = p->fts_statp->st_mtime;

    mParent = static_cast<CacheItem*>(p->fts_parent->fts_pointer);
    if (mParent) {
        group = mParent->group;
        tombstone = mParent->tombstone;
        mName = p->fts_name;
        mName.insert(0, "/");
    } else {
        group = false;
        tombstone = false;
        mName = p->fts_path;
    }
}

CacheItem::~CacheItem() {
}

std::string CacheItem::toString() {
    return StringPrintf("%s size=%" PRId64 " mod=%ld", buildPath().c_str(), size, modified);
}

std::string CacheItem::buildPath() {
    std::string res = mName;
    CacheItem* parent = mParent;
    while (parent) {
        res.insert(0, parent->mName);
        parent = parent->mParent;
    }
    return res;
}

int CacheItem::purge() {
    int res = 0;
    auto path = buildPath();
    if (directory) {
        FTS *fts;
        FTSENT *p;
        char *argv[] = { (char*) path.c_str(), nullptr };
        if (!(fts = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR | FTS_XDEV, nullptr))) {
            PLOG(WARNING) << "Failed to fts_open " << path;
            return -1;
        }
        while ((p = fts_read(fts)) != nullptr) {
            switch (p->fts_info) {
            case FTS_D:
                if (p->fts_level == 0) {
                    p->fts_number = tombstone;
                } else {
                    p->fts_number = p->fts_parent->fts_number
                            | (getxattr(p->fts_path, kXattrCacheTombstone, nullptr, 0) >= 0);
                }
                break;
            case FTS_F:
                if (p->fts_parent->fts_number) {
                    if (truncate(p->fts_path, 0) != 0) {
                        PLOG(WARNING) << "Failed to truncate " << p->fts_path;
                        res = -1;
                    }
                } else {
                    if (unlink(p->fts_path) != 0) {
                        PLOG(WARNING) << "Failed to unlink " << p->fts_path;
                        res = -1;
                    }
                }
                break;
            case FTS_DEFAULT:
            case FTS_SL:
            case FTS_SLNONE:
                if (unlink(p->fts_path) != 0) {
                    PLOG(WARNING) << "Failed to unlink " << p->fts_path;
                    res = -1;
                }
                break;
            case FTS_DP:
                if (rmdir(p->fts_path) != 0) {
                    PLOG(WARNING) << "Failed to rmdir " << p->fts_path;
                    res = -1;
                }
                break;
            }
        }
    } else {
        if (tombstone) {
            if (truncate(path.c_str(), 0) != 0) {
                PLOG(WARNING) << "Failed to truncate " << path;
                res = -1;
            }
        } else {
            if (unlink(path.c_str()) != 0) {
                PLOG(WARNING) << "Failed to unlink " << path;
                res = -1;
            }
        }
    }
    return res;
}

}  // namespace installd
}  // namespace android
