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

#ifndef MINIKIN_SYSTEM_FONTS_H
#define MINIKIN_SYSTEM_FONTS_H

#include <map>
#include <memory>
#include <string>

#include "minikin/FontCollection.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

// Provides a system font mapping.
class SystemFonts {
public:
    static std::shared_ptr<FontCollection> findFontCollection(const std::string& familyName) {
        return getInstance().findFontCollectionInternal(familyName);
    }

    // Do not call this function outside Zygote process.
    static void registerFallback(const std::string& familyName,
                                 const std::shared_ptr<FontCollection>& fc) {
        return getInstance().registerFallbackInternal(familyName, fc);
    }

    // Do not call this function outside Zygote process.
    static void registerDefault(const std::shared_ptr<FontCollection>& fc) {
        return getInstance().registerDefaultInternal(fc);
    }

protected:
    // Visible for testing purposes.
    SystemFonts() {}
    virtual ~SystemFonts() {}

    std::shared_ptr<FontCollection> findFontCollectionInternal(const std::string& familyName) const;
    void registerFallbackInternal(const std::string& familyName,
                                  const std::shared_ptr<FontCollection>& fc) {
        mSystemFallbacks.insert(std::make_pair(familyName, fc));
    }

    void registerDefaultInternal(const std::shared_ptr<FontCollection>& fc) {
        mDefaultFallback = fc;
    }

private:
    static SystemFonts& getInstance();

    // There is no mutex guard here since registerFallback is designed to be
    // called only in Zygote.
    std::map<std::string, std::shared_ptr<FontCollection>> mSystemFallbacks;
    std::shared_ptr<FontCollection> mDefaultFallback;
};

}  // namespace minikin

#endif  // MINIKIN_SYSTEM_FONTS_H
