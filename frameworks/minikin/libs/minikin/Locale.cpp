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

#include "Locale.h"

#include <algorithm>

#include <hb.h>

#include "minikin/LocaleList.h"

#include "LocaleListCache.h"
#include "MinikinInternal.h"
#include "StringPiece.h"

namespace minikin {

constexpr uint32_t FIVE_BITS = 0x1f;

uint32_t registerLocaleList(const std::string& locales) {
    return LocaleListCache::getId(locales);
}

// Check if a language code supports extension such as emoji and line break etc. according to its
// subtag
static bool isSubtag(const char* buf, size_t bufLen, const char* subtag, size_t subtagLen) {
    if (bufLen < subtagLen) {
        return false;
    }
    if (strncmp(buf, subtag, subtagLen) != 0) {
        return false;  // no match between two strings
    }
    return (bufLen == subtagLen || buf[subtagLen] == '\0' || buf[subtagLen] == '-' ||
            buf[subtagLen] == '_');
}

// Pack the three letter code into 15 bits and stored to 16 bit integer. The highest bit is 0.
// For the region code, the letters must be all digits in three letter case, so the number of
// possible values are 10. For the language code, the letters must be all small alphabets, so the
// number of possible values are 26. Thus, 5 bits are sufficient for each case and we can pack the
// three letter language code or region code to 15 bits.
//
// In case of two letter code, use fullbit(0x1f) for the first letter instead.
static uint16_t packLanguageOrRegion(const StringPiece& in, uint8_t twoLetterBase,
                                     uint8_t threeLetterBase) {
    if (in.length() == 2) {
        return 0x7c00u |  // 0x1fu << 10
               (uint16_t)(in[0] - twoLetterBase) << 5 | (uint16_t)(in[1] - twoLetterBase);
    } else {
        return ((uint16_t)(in[0] - threeLetterBase) << 10) |
               (uint16_t)(in[1] - threeLetterBase) << 5 | (uint16_t)(in[2] - threeLetterBase);
    }
}

static size_t unpackLanguageOrRegion(uint16_t in, char* out, uint8_t twoLetterBase,
                                     uint8_t threeLetterBase) {
    uint8_t first = (in >> 10) & FIVE_BITS;
    uint8_t second = (in >> 5) & FIVE_BITS;
    uint8_t third = in & FIVE_BITS;

    if (first == 0x1f) {
        out[0] = second + twoLetterBase;
        out[1] = third + twoLetterBase;
        return 2;
    } else {
        out[0] = first + threeLetterBase;
        out[1] = second + threeLetterBase;
        out[2] = third + threeLetterBase;
        return 3;
    }
}

static uint16_t packLanguage(const StringPiece& in) {
    return packLanguageOrRegion(in, 'a', 'a');
}

static size_t unpackLanguage(uint16_t in, char* out) {
    return unpackLanguageOrRegion(in, out, 'a', 'a');
}

constexpr uint32_t packScript(char c1, char c2, char c3, char c4) {
    constexpr char FIRST_LETTER_BASE = 'A';
    constexpr char REST_LETTER_BASE = 'a';
    return ((uint32_t)(c1 - FIRST_LETTER_BASE) << 15) | (uint32_t)(c2 - REST_LETTER_BASE) << 10 |
           ((uint32_t)(c3 - REST_LETTER_BASE) << 5) | (uint32_t)(c4 - REST_LETTER_BASE);
}

constexpr uint32_t packScript(uint32_t script) {
    return packScript(script >> 24, (script >> 16) & 0xff, (script >> 8) & 0xff, script & 0xff);
}

constexpr uint32_t unpackScript(uint32_t packedScript) {
    constexpr char FIRST_LETTER_BASE = 'A';
    constexpr char REST_LETTER_BASE = 'a';
    const uint32_t first = (packedScript >> 15) + FIRST_LETTER_BASE;
    const uint32_t second = ((packedScript >> 10) & FIVE_BITS) + REST_LETTER_BASE;
    const uint32_t third = ((packedScript >> 5) & FIVE_BITS) + REST_LETTER_BASE;
    const uint32_t fourth = (packedScript & FIVE_BITS) + REST_LETTER_BASE;

    return first << 24 | second << 16 | third << 8 | fourth;
}

static uint16_t packRegion(const StringPiece& in) {
    return packLanguageOrRegion(in, 'A', '0');
}

static size_t unpackRegion(uint16_t in, char* out) {
    return unpackLanguageOrRegion(in, out, 'A', '0');
}

static inline bool isLowercase(char c) {
    return 'a' <= c && c <= 'z';
}

static inline bool isUppercase(char c) {
    return 'A' <= c && c <= 'Z';
}

static inline bool isDigit(char c) {
    return '0' <= c && c <= '9';
}

// Returns true if the buffer is valid for language code.
static inline bool isValidLanguageCode(const StringPiece& buffer) {
    if (buffer.length() != 2 && buffer.length() != 3) return false;
    if (!isLowercase(buffer[0])) return false;
    if (!isLowercase(buffer[1])) return false;
    if (buffer.length() == 3 && !isLowercase(buffer[2])) return false;
    return true;
}

// Returns true if buffer is valid for script code. The length of buffer must be 4.
static inline bool isValidScriptCode(const StringPiece& buffer) {
    return buffer.size() == 4 && isUppercase(buffer[0]) && isLowercase(buffer[1]) &&
           isLowercase(buffer[2]) && isLowercase(buffer[3]);
}

// Returns true if the buffer is valid for region code.
static inline bool isValidRegionCode(const StringPiece& buffer) {
    return (buffer.size() == 2 && isUppercase(buffer[0]) && isUppercase(buffer[1])) ||
           (buffer.size() == 3 && isDigit(buffer[0]) && isDigit(buffer[1]) && isDigit(buffer[2]));
}

// Parse BCP 47 language identifier into internal structure
Locale::Locale(const StringPiece& input) : Locale() {
    SplitIterator it(input, '-');

    StringPiece language = it.next();
    if (isValidLanguageCode(language)) {
        mLanguage = packLanguage(language);
    } else {
        // We don't understand anything other than two-letter or three-letter
        // language codes, so we skip parsing the rest of the string.
        return;
    }

    if (!it.hasNext()) {
        return;  // Language code only.
    }
    StringPiece token = it.next();

    if (isValidScriptCode(token)) {
        mScript = packScript(token[0], token[1], token[2], token[3]);
        mSubScriptBits = scriptToSubScriptBits(mScript);

        if (!it.hasNext()) {
            goto finalize;  // No variant, emoji subtag and region code.
        }
        token = it.next();
    }

    if (isValidRegionCode(token)) {
        mRegion = packRegion(token);

        if (!it.hasNext()) {
            goto finalize;  // No variant or emoji subtag.
        }
        token = it.next();
    }

    if (language == "de") {  // We are only interested in German variants.
        if (token == "1901") {
            mVariant = Variant::GERMAN_1901_ORTHOGRAPHY;
        } else if (token == "1996") {
            mVariant = Variant::GERMAN_1996_ORTHOGRAPHY;
        }

        if (mVariant != Variant::NO_VARIANT) {
            if (!it.hasNext()) {
                goto finalize;  // No emoji subtag.
            }

            token = it.next();
        }
    }

    resolveUnicodeExtension(input.data(), input.length());

finalize:
    if (mEmojiStyle == EmojiStyle::EMPTY) {
        mEmojiStyle = scriptToEmojiStyle(mScript);
    }
}

void Locale::resolveUnicodeExtension(const char* buf, size_t length) {
    static const char kPrefix[] = "-u-";
    const char* pos = std::search(buf, buf + length, kPrefix, kPrefix + strlen(kPrefix));
    if (pos != buf + length) {
        pos += strlen(kPrefix);
        const size_t remainingLength = length - (pos - buf);
        mLBStyle = resolveLineBreakStyle(pos, remainingLength);
        mEmojiStyle = resolveEmojiStyle(pos, remainingLength);
    }
}

// static
// Lookup line break subtag and determine the line break style.
LineBreakStyle Locale::resolveLineBreakStyle(const char* buf, size_t length) {
    // 8 is the length of "-u-lb-loose", which is the shortest line break subtag,
    // unnecessary comparison can be avoided if total length is smaller than 11.
    const size_t kMinSubtagLength = 8;
    if (length >= kMinSubtagLength) {
        static const char kPrefix[] = "lb-";
        const char* pos = std::search(buf, buf + length, kPrefix, kPrefix + strlen(kPrefix));
        if (pos != buf + length) {  // found
            pos += strlen(kPrefix);
            const size_t remainingLength = length - (pos - buf);
            if (isSubtag(pos, remainingLength, "loose", 5)) {
                return LineBreakStyle::LOOSE;
            } else if (isSubtag(pos, remainingLength, "normal", 6)) {
                return LineBreakStyle::NORMAL;
            } else if (isSubtag(pos, remainingLength, "strict", 6)) {
                return LineBreakStyle::STRICT;
            }
        }
    }
    return LineBreakStyle::EMPTY;
}

// static
// Lookup emoji subtag and determine the emoji style.
EmojiStyle Locale::resolveEmojiStyle(const char* buf, size_t length) {
    // 7 is the length of "-u-em-text", which is the shortest emoji subtag,
    // unnecessary comparison can be avoided if total length is smaller than 10.
    const size_t kMinSubtagLength = 7;
    if (length >= kMinSubtagLength) {
        static const char kPrefix[] = "em-";
        const char* pos = std::search(buf, buf + length, kPrefix, kPrefix + strlen(kPrefix));
        if (pos != buf + length) {  // found
            pos += strlen(kPrefix);
            const size_t remainingLength = length - (pos - buf);
            if (isSubtag(pos, remainingLength, "emoji", 5)) {
                return EmojiStyle::EMOJI;
            } else if (isSubtag(pos, remainingLength, "text", 4)) {
                return EmojiStyle::TEXT;
            } else if (isSubtag(pos, remainingLength, "default", 7)) {
                return EmojiStyle::DEFAULT;
            }
        }
    }
    return EmojiStyle::EMPTY;
}

EmojiStyle Locale::scriptToEmojiStyle(uint32_t script) {
    // If no emoji subtag was provided, resolve the emoji style from script code.
    if (script == packScript('Z', 's', 'y', 'e')) {
        return EmojiStyle::EMOJI;
    } else if (script == packScript('Z', 's', 'y', 'm')) {
        return EmojiStyle::TEXT;
    }
    return EmojiStyle::EMPTY;
}

// static
uint8_t Locale::scriptToSubScriptBits(uint32_t script) {
    uint8_t subScriptBits = 0u;
    switch (script) {
        case packScript('B', 'o', 'p', 'o'):
            subScriptBits = kBopomofoFlag;
            break;
        case packScript('H', 'a', 'n', 'g'):
            subScriptBits = kHangulFlag;
            break;
        case packScript('H', 'a', 'n', 'b'):
            // Bopomofo is almost exclusively used in Taiwan.
            subScriptBits = kHanFlag | kBopomofoFlag;
            break;
        case packScript('H', 'a', 'n', 'i'):
            subScriptBits = kHanFlag;
            break;
        case packScript('H', 'a', 'n', 's'):
            subScriptBits = kHanFlag | kSimplifiedChineseFlag;
            break;
        case packScript('H', 'a', 'n', 't'):
            subScriptBits = kHanFlag | kTraditionalChineseFlag;
            break;
        case packScript('H', 'i', 'r', 'a'):
            subScriptBits = kHiraganaFlag;
            break;
        case packScript('H', 'r', 'k', 't'):
            subScriptBits = kKatakanaFlag | kHiraganaFlag;
            break;
        case packScript('J', 'p', 'a', 'n'):
            subScriptBits = kHanFlag | kKatakanaFlag | kHiraganaFlag;
            break;
        case packScript('K', 'a', 'n', 'a'):
            subScriptBits = kKatakanaFlag;
            break;
        case packScript('K', 'o', 'r', 'e'):
            subScriptBits = kHanFlag | kHangulFlag;
            break;
    }
    return subScriptBits;
}

std::string Locale::getString() const {
    char buf[32] = {};
    size_t i;
    if (mLanguage == NO_LANGUAGE) {
        buf[0] = 'u';
        buf[1] = 'n';
        buf[2] = 'd';
        i = 3;
    } else {
        i = unpackLanguage(mLanguage, buf);
    }
    if (mScript != NO_SCRIPT) {
        uint32_t rawScript = unpackScript(mScript);
        buf[i++] = '-';
        buf[i++] = (rawScript >> 24) & 0xFFu;
        buf[i++] = (rawScript >> 16) & 0xFFu;
        buf[i++] = (rawScript >> 8) & 0xFFu;
        buf[i++] = rawScript & 0xFFu;
    }
    if (mRegion != NO_REGION) {
        buf[i++] = '-';
        i += unpackRegion(mRegion, buf + i);
    }
    if (mVariant != Variant::NO_VARIANT) {
        buf[i++] = '-';
        buf[i++] = '1';
        buf[i++] = '9';
        switch (mVariant) {
            case Variant::GERMAN_1901_ORTHOGRAPHY:
                buf[i++] = '0';
                buf[i++] = '1';
                break;
            case Variant::GERMAN_1996_ORTHOGRAPHY:
                buf[i++] = '9';
                buf[i++] = '6';
                break;
            default:
                MINIKIN_ASSERT(false, "Must not reached.");
        }
    }
    // Add line break unicode extension.
    if (mLBStyle != LineBreakStyle::EMPTY) {
        buf[i++] = '-';
        buf[i++] = 'u';
        buf[i++] = '-';
        buf[i++] = 'l';
        buf[i++] = 'b';
        buf[i++] = '-';
        switch (mLBStyle) {
            case LineBreakStyle::LOOSE:
                buf[i++] = 'l';
                buf[i++] = 'o';
                buf[i++] = 'o';
                buf[i++] = 's';
                buf[i++] = 'e';
                break;
            case LineBreakStyle::NORMAL:
                buf[i++] = 'n';
                buf[i++] = 'o';
                buf[i++] = 'r';
                buf[i++] = 'm';
                buf[i++] = 'a';
                buf[i++] = 'l';
                break;
            case LineBreakStyle::STRICT:
                buf[i++] = 's';
                buf[i++] = 't';
                buf[i++] = 'r';
                buf[i++] = 'i';
                buf[i++] = 'c';
                buf[i++] = 't';
                break;
            default:
                MINIKIN_ASSERT(false, "Must not reached.");
        }
    }
    return std::string(buf, i);
}

Locale Locale::getPartialLocale(SubtagBits bits) const {
    Locale subLocale;
    if ((bits & SubtagBits::LANGUAGE) != SubtagBits::EMPTY) {
        subLocale.mLanguage = mLanguage;
    } else {
        subLocale.mLanguage = packLanguage("und");
    }
    if ((bits & SubtagBits::SCRIPT) != SubtagBits::EMPTY) {
        subLocale.mScript = mScript;
        subLocale.mSubScriptBits = mSubScriptBits;
    }
    if ((bits & SubtagBits::REGION) != SubtagBits::EMPTY) {
        subLocale.mRegion = mRegion;
    }
    if ((bits & SubtagBits::VARIANT) != SubtagBits::EMPTY) {
        subLocale.mVariant = mVariant;
    }
    if ((bits & SubtagBits::EMOJI) != SubtagBits::EMPTY) {
        subLocale.mEmojiStyle = mEmojiStyle;
    }
    return subLocale;
}

bool Locale::isEqualScript(const Locale& other) const {
    return other.mScript == mScript;
}

// static
bool Locale::supportsScript(uint8_t providedBits, uint8_t requestedBits) {
    return requestedBits != 0 && (providedBits & requestedBits) == requestedBits;
}

bool Locale::supportsHbScript(hb_script_t script) const {
    static_assert(unpackScript(packScript('J', 'p', 'a', 'n')) == HB_TAG('J', 'p', 'a', 'n'),
                  "The Minikin script and HarfBuzz hb_script_t have different encodings.");
    uint32_t packedScript = packScript(script);
    if (packedScript == mScript) return true;
    return supportsScript(mSubScriptBits, scriptToSubScriptBits(packedScript));
}

int Locale::calcScoreFor(const LocaleList& supported) const {
    bool languageScriptMatch = false;
    bool subtagMatch = false;
    bool scriptMatch = false;

    for (size_t i = 0; i < supported.size(); ++i) {
        if (mEmojiStyle != EmojiStyle::EMPTY && mEmojiStyle == supported[i].mEmojiStyle) {
            subtagMatch = true;
            if (mLanguage == supported[i].mLanguage) {
                return 4;
            }
        }
        if (isEqualScript(supported[i]) ||
            supportsScript(supported[i].mSubScriptBits, mSubScriptBits)) {
            scriptMatch = true;
            if (mLanguage == supported[i].mLanguage) {
                languageScriptMatch = true;
            }
        }
    }

    if (supportsScript(supported.getUnionOfSubScriptBits(), mSubScriptBits)) {
        scriptMatch = true;
        if (mLanguage == supported[0].mLanguage && supported.isAllTheSameLocale()) {
            return 3;
        }
    }

    if (languageScriptMatch) {
        return 3;
    } else if (subtagMatch) {
        return 2;
    } else if (scriptMatch) {
        return 1;
    }
    return 0;
}

static hb_language_t buildHbLanguage(const Locale& locale) {
    return locale.isSupported() ? hb_language_from_string(locale.getString().c_str(), -1)
                                : HB_LANGUAGE_INVALID;
}

LocaleList::LocaleList(std::vector<Locale>&& locales) : mLocales(std::move(locales)) {
    mIsAllTheSameLocale = true;
    mUnionOfSubScriptBits = 0u;
    mHbLangs.reserve(mLocales.size());
    mEmojiStyle = EmojiStyle::EMPTY;
    const auto firstLanguage = mLocales.empty() ? NO_LANGUAGE : mLocales[0].mLanguage;
    for (const Locale& locale : mLocales) {
        mUnionOfSubScriptBits |= locale.mSubScriptBits;
        if (mIsAllTheSameLocale && firstLanguage != locale.mLanguage) {
            mIsAllTheSameLocale = false;
        }
        mHbLangs.push_back(buildHbLanguage(locale));
        if (mEmojiStyle == EmojiStyle::EMPTY) {
            mEmojiStyle = locale.getEmojiStyle();
        }
    }
}

}  // namespace minikin
