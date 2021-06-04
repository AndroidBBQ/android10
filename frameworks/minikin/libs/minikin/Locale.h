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

#ifndef MINIKIN_LOCALE_LIST_H
#define MINIKIN_LOCALE_LIST_H

#include <string>
#include <vector>

#include <hb.h>

#include "StringPiece.h"

namespace minikin {

// Due to the limits in font fallback score calculation, we can't use anything more than 12 locales.
const size_t FONT_LOCALE_LIMIT = 12;

// The language or region code is encoded to 15 bits.
constexpr uint16_t NO_LANGUAGE = 0x7fff;
constexpr uint16_t NO_REGION = 0x7fff;
// The script code is encoded to 20 bits.
constexpr uint32_t NO_SCRIPT = 0xfffff;

class LocaleList;

// Enum for making sub-locale from FontLangauge.
enum class SubtagBits : uint8_t {
    EMPTY = 0b00000000,
    LANGUAGE = 0b00000001,
    SCRIPT = 0b00000010,
    REGION = 0b00000100,
    VARIANT = 0b00001000,
    EMOJI = 0b00010000,
    ALL = 0b00011111,
};

inline constexpr SubtagBits operator&(SubtagBits l, SubtagBits r) {
    return static_cast<SubtagBits>(static_cast<uint8_t>(l) & static_cast<uint8_t>(r));
}
inline constexpr SubtagBits operator|(SubtagBits l, SubtagBits r) {
    return static_cast<SubtagBits>(static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
}

// Enum for emoji style.
enum class EmojiStyle : uint8_t {
    EMPTY = 0,    // No emoji style is specified.
    DEFAULT = 1,  // Default emoji style is specified.
    EMOJI = 2,    // Emoji (color) emoji style is specified.
    TEXT = 3,     // Text (black/white) emoji style is specified.
};

// Enum for line break style.
enum class LineBreakStyle : uint8_t {
    EMPTY = 0,   // No line break style is specified.
    LOOSE = 1,   // line break style is loose.
    NORMAL = 2,  // line break style is normal.
    STRICT = 3,  // line break style is strict.
};

// Locale is a compact representation of a BCP 47 language tag.
// It does not capture all possible information, only what directly affects text layout:
// font rendering, hyphenation, word breaking, etc.
struct Locale {
public:
    enum class Variant : uint16_t {
        NO_VARIANT = 0x0000,
        GERMAN_1901_ORTHOGRAPHY = 0x0001,
        GERMAN_1996_ORTHOGRAPHY = 0x0002,
    };

    // Default constructor creates the unsupported locale.
    Locale()
            : mScript(NO_SCRIPT),
              mLanguage(NO_LANGUAGE),
              mRegion(NO_REGION),
              mSubScriptBits(0ul),
              mVariant(Variant::NO_VARIANT),
              mEmojiStyle(EmojiStyle::EMPTY),
              mLBStyle(LineBreakStyle::EMPTY) {}

    // Parse from string
    Locale(const StringPiece& buf);

    bool operator==(const Locale other) const {
        return !isUnsupported() && isEqualScript(other) && mLanguage == other.mLanguage &&
               mRegion == other.mRegion && mVariant == other.mVariant &&
               mLBStyle == other.mLBStyle && mEmojiStyle == other.mEmojiStyle;
    }

    bool operator!=(const Locale other) const { return !(*this == other); }

    inline bool hasLanguage() const { return mLanguage != NO_LANGUAGE; }
    inline bool hasScript() const { return mScript != NO_SCRIPT; }
    inline bool hasRegion() const { return mRegion != NO_REGION; }
    inline bool hasVariant() const { return mVariant != Variant::NO_VARIANT; }
    inline bool hasLBStyle() const { return mLBStyle != LineBreakStyle::EMPTY; }
    inline bool hasEmojiStyle() const { return mEmojiStyle != EmojiStyle::EMPTY; }

    inline bool isSupported() const {
        return hasLanguage() || hasScript() || hasRegion() || hasVariant() || hasLBStyle() ||
               hasEmojiStyle();
    }

    inline bool isUnsupported() const { return !isSupported(); }

    EmojiStyle getEmojiStyle() const { return mEmojiStyle; }

    bool isEqualScript(const Locale& other) const;

    // Returns true if this script supports the given script. For example, ja-Jpan supports Hira,
    // ja-Hira doesn't support Jpan.
    bool supportsHbScript(hb_script_t script) const;

    std::string getString() const;

    // Calculates a matching score. This score represents how well the input locales cover this
    // locale. The maximum score in the locale list is returned.
    // 0 = no match, 1 = script match, 2 = script and primary language match.
    int calcScoreFor(const LocaleList& supported) const;

    // Identifier pattern:
    // |-------|-------|-------|-------|-------|-------|-------|-------|
    // lllllllllllllll                                                   Language Code
    //                ssssssssssssssssssss                               Script Code
    //                                    rrrrrrrrrrrrrrr                Region Code
    //                                                   ee              Emoji Style
    //                                                     bb            Line Break Style
    //                                                       XXXXXXXX    Free
    //                                                               vv  German Variant
    uint64_t getIdentifier() const {
        return ((uint64_t)mLanguage << 49) | ((uint64_t)mScript << 29) | ((uint64_t)mRegion << 14) |
               ((uint64_t)mEmojiStyle << 12) | ((uint64_t)mLBStyle << 10) | (uint64_t)mVariant;
    }

    Locale getPartialLocale(SubtagBits bits) const;

private:
    friend class LocaleList;  // for LocaleList constructor

    // ISO 15924 compliant script code. The 4 chars script code are packed into a 20 bit integer.
    // If not specified, this is kInvalidScript.
    uint32_t mScript;

    // ISO 639-1 or ISO 639-2 compliant language code.
    // The two- or three-letter language code is packed into a 15 bit integer.
    // mLanguage = 0 means the Locale is unsupported.
    uint16_t mLanguage;

    // ISO 3166-1 or UN M.49 compliant region code. The two-letter or three-digit region code is
    // packed into a 15 bit integer.
    uint16_t mRegion;

    // For faster comparing, use 7 bits for specific scripts.
    static const uint8_t kBopomofoFlag = 1u;
    static const uint8_t kHanFlag = 1u << 1;
    static const uint8_t kHangulFlag = 1u << 2;
    static const uint8_t kHiraganaFlag = 1u << 3;
    static const uint8_t kKatakanaFlag = 1u << 4;
    static const uint8_t kSimplifiedChineseFlag = 1u << 5;
    static const uint8_t kTraditionalChineseFlag = 1u << 6;
    uint8_t mSubScriptBits;

    Variant mVariant;

    EmojiStyle mEmojiStyle;
    LineBreakStyle mLBStyle;

    void resolveUnicodeExtension(const char* buf, size_t length);

    static uint8_t scriptToSubScriptBits(uint32_t rawScript);

    static LineBreakStyle resolveLineBreakStyle(const char* buf, size_t length);
    static EmojiStyle resolveEmojiStyle(const char* buf, size_t length);
    static EmojiStyle scriptToEmojiStyle(uint32_t script);

    // Returns true if the provide subscript bits has the requested subscript bits.
    // Note that this function returns false if the requested subscript bits are empty.
    static bool supportsScript(uint8_t providedBits, uint8_t requestedBits);
};

// An immutable list of locale.
class LocaleList {
public:
    explicit LocaleList(std::vector<Locale>&& locales);
    LocaleList()
            : mUnionOfSubScriptBits(0),
              mIsAllTheSameLocale(false),
              mEmojiStyle(EmojiStyle::EMPTY) {}
    LocaleList(LocaleList&&) = default;

    size_t size() const { return mLocales.size(); }
    bool empty() const { return mLocales.empty(); }
    const Locale& operator[](size_t n) const { return mLocales[n]; }

    hb_language_t getHbLanguage(size_t n) const { return mHbLangs[n]; }

    // Returns an effective emoji style of this locale list.
    // The effective means the first non empty emoji style in the list.
    EmojiStyle getEmojiStyle() const { return mEmojiStyle; }

private:
    friend struct Locale;  // for calcScoreFor

    std::vector<Locale> mLocales;

    // The languages to be passed to HarfBuzz shaper.
    std::vector<hb_language_t> mHbLangs;
    uint8_t mUnionOfSubScriptBits;
    bool mIsAllTheSameLocale;
    EmojiStyle mEmojiStyle;

    uint8_t getUnionOfSubScriptBits() const { return mUnionOfSubScriptBits; }
    bool isAllTheSameLocale() const { return mIsAllTheSameLocale; }

    // Do not copy and assign.
    LocaleList(const LocaleList&) = delete;
    void operator=(const LocaleList&) = delete;
};

}  // namespace minikin

#endif  // MINIKIN_LOCALE_LIST_H
