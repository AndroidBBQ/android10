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

#ifndef MINIKIN_LOCALE_LIST_CACHE_H
#define MINIKIN_LOCALE_LIST_CACHE_H

#include <mutex>
#include <unordered_map>

#include "minikin/Macros.h"

#include "Locale.h"

namespace minikin {

class LocaleListCache {
public:
    // A special ID for the empty locale list.
    // This value must be 0 since the empty locale list is inserted into mLocaleLists by
    // default.
    const static uint32_t kEmptyListId = 0;

    // A special ID for the invalid locale list.
    const static uint32_t kInvalidListId = (uint32_t)(-1);

    // Returns the locale list ID for the given string representation of LocaleList.
    // Caller should acquire a lock before calling the method.
    static inline uint32_t getId(const std::string& locales) {
        return getInstance().getIdInternal(locales);
    }

    // Caller should acquire a lock before calling the method.
    static inline const LocaleList& getById(uint32_t id) {
        return getInstance().getByIdInternal(id);
    }

private:
    LocaleListCache();  // Singleton
    ~LocaleListCache() {}

    uint32_t getIdInternal(const std::string& locales);
    const LocaleList& getByIdInternal(uint32_t id);

    // Caller should acquire a lock before calling the method.
    static LocaleListCache& getInstance() {
        static LocaleListCache instance;
        return instance;
    }

    std::vector<LocaleList> mLocaleLists GUARDED_BY(mMutex);

    // A map from the string representation of the font locale list to the ID.
    std::unordered_map<std::string, uint32_t> mLocaleListLookupTable GUARDED_BY(mMutex);

    std::mutex mMutex;
};

}  // namespace minikin

#endif  // MINIKIN_LOCALE_LIST_CACHE_H
