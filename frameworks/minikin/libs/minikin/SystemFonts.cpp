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

#define LOG_TAG "Minikin"

#include "minikin/SystemFonts.h"

namespace minikin {

SystemFonts& SystemFonts::getInstance() {
    static SystemFonts systemFonts;
    return systemFonts;
}

std::shared_ptr<FontCollection> SystemFonts::findFontCollectionInternal(
        const std::string& familyName) const {
    auto it = mSystemFallbacks.find(familyName);
    if (it != mSystemFallbacks.end()) {
        return it->second;
    }
    // TODO: Lookup by PostScript name.
    return mDefaultFallback;
}

}  // namespace minikin
