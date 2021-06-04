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

#ifndef MINIKIN_HYPHENATOR_MAP_H
#define MINIKIN_HYPHENATOR_MAP_H

#include <map>
#include <mutex>

#include "minikin/Hyphenator.h"
#include "minikin/Macros.h"

#include "Locale.h"

namespace minikin {

class HyphenatorMap {
public:
    // This map doesn't take ownership of the hyphenator but we don't need to care about the
    // ownership. In Android, the Hyphenator is allocated in Zygote and never gets released.
    static void add(const std::string& localeStr, const Hyphenator* hyphenator) {
        getInstance().addInternal(localeStr, hyphenator);
    }

    static void addAlias(const std::string& fromLocaleStr, const std::string& toLocaleStr) {
        getInstance().addAliasInternal(fromLocaleStr, toLocaleStr);
    }

    // Remove all hyphenators from the map. This is test only method.
    static void clear() { getInstance().clearInternal(); }

    // The returned pointer is never a dangling pointer. If nothing found for a given locale,
    // returns a hyphenator which only processes soft hyphens.
    //
    // The Hyphenator lookup works with the following rules:
    // 1. Search for the Hyphenator with the given locale.
    // 2. If not found, try again with language + script + region + variant.
    // 3. If not found, try again with language + region + variant.
    // 4. If not found, try again with language + variant.
    // 5. If not found, try again with language.
    // 6. If not found, try again with script.
    static const Hyphenator* lookup(const Locale& locale) {
        return getInstance().lookupInternal(locale);
    }

protected:
    // The following five methods are protected for testing purposes.
    HyphenatorMap();  // Use getInstance() instead.
    void addInternal(const std::string& localeStr, const Hyphenator* hyphenator);
    void addAliasInternal(const std::string& fromLocaleStr, const std::string& toLocaleStr);
    const Hyphenator* lookupInternal(const Locale& locale);

private:
    static HyphenatorMap& getInstance() {  // Singleton.
        static HyphenatorMap map;
        return map;
    }

    void clearInternal();

    const Hyphenator* lookupByIdentifier(uint64_t id) const EXCLUSIVE_LOCKS_REQUIRED(mMutex);
    const Hyphenator* lookupBySubtag(const Locale& locale, SubtagBits bits) const
            EXCLUSIVE_LOCKS_REQUIRED(mMutex);

    const Hyphenator* mSoftHyphenOnlyHyphenator;
    std::map<uint64_t, const Hyphenator*> mMap GUARDED_BY(mMutex);

    std::mutex mMutex;
};

}  // namespace minikin

#endif  // MINIKIN_HYPHENATOR_MAP_H
