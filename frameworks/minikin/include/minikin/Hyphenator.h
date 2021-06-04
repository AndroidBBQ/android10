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

/**
 * An implementation of Liang's hyphenation algorithm.
 */

#ifndef MINIKIN_HYPHENATOR_H
#define MINIKIN_HYPHENATOR_H

#include <string>
#include <vector>

#include "minikin/Characters.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

class Hyphenator;

// Registers the hyphenator.
// This doesn't take ownership of the hyphenator but we don't need to care about the ownership.
// In Android, the Hyphenator is allocated in Zygote and never gets released.
void addHyphenator(const std::string& localeStr, const Hyphenator* hyphenator);
void addHyphenatorAlias(const std::string& fromLocaleStr, const std::string& toLocaleStr);

enum class HyphenationType : uint8_t {
    // Note: There are implicit assumptions scattered in the code that DONT_BREAK is 0.

    // Do not break.
    DONT_BREAK = 0,
    // Break the line and insert a normal hyphen.
    BREAK_AND_INSERT_HYPHEN = 1,
    // Break the line and insert an Armenian hyphen (U+058A).
    BREAK_AND_INSERT_ARMENIAN_HYPHEN = 2,
    // Break the line and insert a maqaf (Hebrew hyphen, U+05BE).
    BREAK_AND_INSERT_MAQAF = 3,
    // Break the line and insert a Canadian Syllabics hyphen (U+1400).
    BREAK_AND_INSERT_UCAS_HYPHEN = 4,
    // Break the line, but don't insert a hyphen. Used for cases when there is already a hyphen
    // present or the script does not use a hyphen (e.g. in Malayalam).
    BREAK_AND_DONT_INSERT_HYPHEN = 5,
    // Break and replace the last code unit with hyphen. Used for Catalan "l·l" which hyphenates
    // as "l-/l".
    BREAK_AND_REPLACE_WITH_HYPHEN = 6,
    // Break the line, and repeat the hyphen (which is the last character) at the beginning of the
    // next line. Used in Polish (where "czerwono-niebieska" should hyphenate as
    // "czerwono-/-niebieska") and Slovenian.
    BREAK_AND_INSERT_HYPHEN_AT_NEXT_LINE = 7,
    // Break the line, insert a ZWJ and hyphen at the first line, and a ZWJ at the second line.
    // This is used in Arabic script, mostly for writing systems of Central Asia. It's our default
    // behavior when a soft hyphen is used in Arabic script.
    BREAK_AND_INSERT_HYPHEN_AND_ZWJ = 8
};

// The hyphen edit represents an edit to the string when a word is hyphenated.
// The most common hyphen edit is adding a "-" at the end of a syllable, but nonstandard hyphenation
// allows for more choices.
// One at the beginning of the string/line and one at the end.
enum class EndHyphenEdit : uint8_t {
    // Note that everything inserting characters must have a value greater than or equal to
    // INSERT_HYPHEN.
    NO_EDIT = 0b000,
    REPLACE_WITH_HYPHEN = 0b001,

    INSERT_HYPHEN = 0b010,
    INSERT_ARMENIAN_HYPHEN = 0b011,
    INSERT_MAQAF = 0b100,
    INSERT_UCAS_HYPHEN = 0b101,
    INSERT_ZWJ_AND_HYPHEN = 0b110,
};

enum class StartHyphenEdit : uint8_t {
    NO_EDIT = 0b00,

    INSERT_HYPHEN = 0b01,
    INSERT_ZWJ = 0b10,
};

typedef uint8_t HyphenEdit;
constexpr uint8_t START_BITS_SHIFT = 3;
// The following two masks must keep in sync with the definitions in the Java code at:
// frameworks/base/graphics/java/android/graphics/Paint.java
constexpr uint8_t MASK_END_OF_LINE = 0b00111;
constexpr uint8_t MASK_START_OF_LINE = 0b11000;

inline HyphenEdit packHyphenEdit(StartHyphenEdit start, EndHyphenEdit end) {
    return static_cast<uint8_t>(start) << START_BITS_SHIFT | static_cast<uint8_t>(end);
}

inline EndHyphenEdit endHyphenEdit(HyphenEdit hyphenEdit) {
    return static_cast<EndHyphenEdit>(hyphenEdit & MASK_END_OF_LINE);
}

inline StartHyphenEdit startHyphenEdit(HyphenEdit hyphenEdit) {
    return static_cast<StartHyphenEdit>(hyphenEdit >> START_BITS_SHIFT);
}

inline bool isReplacement(EndHyphenEdit hyph) {
    return hyph == EndHyphenEdit::REPLACE_WITH_HYPHEN;
}

inline bool isInsertion(StartHyphenEdit hyph) {
    return hyph != StartHyphenEdit::NO_EDIT;
}

inline bool isInsertion(EndHyphenEdit hyph) {
    return static_cast<uint8_t>(hyph) >= static_cast<uint8_t>(EndHyphenEdit::INSERT_HYPHEN);
}

template <typename T, size_t size>
constexpr size_t ARRAYSIZE(T const (&)[size]) {
    return size;
}
constexpr uint32_t HYPHEN_STR_ZWJ[] = {CHAR_ZWJ};
constexpr uint32_t HYPHEN_STR_HYPHEN[] = {CHAR_HYPHEN};
constexpr uint32_t HYPHEN_STR_ARMENIAN_HYPHEN[] = {CHAR_ARMENIAN_HYPHEN};
constexpr uint32_t HYPHEN_STR_MAQAF[] = {CHAR_MAQAF};
constexpr uint32_t HYPHEN_STR_UCAS_HYPHEN[] = {CHAR_UCAS_HYPHEN};
constexpr uint32_t HYPHEN_STR_ZWJ_AND_HYPHEN[] = {CHAR_ZWJ, CHAR_HYPHEN};
constexpr std::pair<const uint32_t*, size_t> EMPTY_HYPHEN_STR(nullptr, 0);
#define MAKE_HYPHEN_STR(chars) std::make_pair((chars), ARRAYSIZE(chars))

inline std::pair<const uint32_t*, size_t> getHyphenString(StartHyphenEdit hyph) {
    if (hyph == StartHyphenEdit::INSERT_ZWJ) {
        return MAKE_HYPHEN_STR(HYPHEN_STR_ZWJ);
    } else if (hyph == StartHyphenEdit::INSERT_HYPHEN) {
        return MAKE_HYPHEN_STR(HYPHEN_STR_HYPHEN);
    } else {
        return EMPTY_HYPHEN_STR;
    }
}

inline std::pair<const uint32_t*, size_t> getHyphenString(EndHyphenEdit hyph) {
    switch (hyph) {
        case EndHyphenEdit::REPLACE_WITH_HYPHEN:  // fall through
        case EndHyphenEdit::INSERT_HYPHEN:
            return MAKE_HYPHEN_STR(HYPHEN_STR_HYPHEN);
        case EndHyphenEdit::INSERT_ARMENIAN_HYPHEN:
            return MAKE_HYPHEN_STR(HYPHEN_STR_ARMENIAN_HYPHEN);
        case EndHyphenEdit::INSERT_MAQAF:
            return MAKE_HYPHEN_STR(HYPHEN_STR_MAQAF);
        case EndHyphenEdit::INSERT_UCAS_HYPHEN:
            return MAKE_HYPHEN_STR(HYPHEN_STR_UCAS_HYPHEN);
        case EndHyphenEdit::INSERT_ZWJ_AND_HYPHEN:
            return MAKE_HYPHEN_STR(HYPHEN_STR_ZWJ_AND_HYPHEN);
        case EndHyphenEdit::NO_EDIT:
        default:
            return EMPTY_HYPHEN_STR;
    }
}
#undef MAKE_HYPHEN_STR

EndHyphenEdit editForThisLine(HyphenationType type);
StartHyphenEdit editForNextLine(HyphenationType type);

// hyb file header; implementation details are in the .cpp file
struct Header;

class Hyphenator {
public:
    // Compute the hyphenation of a word, storing the hyphenation in result vector. Each entry in
    // the vector is a "hyphenation type" for a potential hyphenation that can be applied at the
    // corresponding code unit offset in the word.
    //
    // out must have at least the length of the word capacity.
    //
    // Example: word is "hyphen", result is the following, corresponding to "hy-phen":
    // [DONT_BREAK, DONT_BREAK, BREAK_AND_INSERT_HYPHEN, DONT_BREAK, DONT_BREAK, DONT_BREAK]
    void hyphenate(const U16StringPiece& word, HyphenationType* out) const;

    // Compute the hyphenation of a word.
    //
    // out will be resized to word length.
    void hyphenate(const U16StringPiece& word, std::vector<HyphenationType>* out) const {
        out->resize(word.size());
        return hyphenate(word, out->data());
    }

    // Returns true if the codepoint is like U+2010 HYPHEN in line breaking and usage: a character
    // immediately after which line breaks are allowed, but words containing it should not be
    // automatically hyphenated.
    static bool isLineBreakingHyphen(uint32_t cp);

    // pattern data is in binary format, as described in doc/hyb_file_format.md. Note:
    // the caller is responsible for ensuring that the lifetime of the pattern data is
    // at least as long as the Hyphenator object.

    // This class doesn't copy or take ownership of patternData. Caller must keep the data valid
    // until this instance is deleted.
    // Note: nullptr is valid input, in which case the hyphenator only processes soft hyphens.
    static Hyphenator* loadBinary(const uint8_t* patternData, size_t minPrefix, size_t minSuffix,
                                  const std::string& locale);

private:
    enum class HyphenationLocale : uint8_t {
        OTHER = 0,
        CATALAN = 1,
        POLISH = 2,
        SLOVENIAN = 3,
    };

    // Use Hyphenator::loadBinary instead.
    Hyphenator(const uint8_t* patternData, size_t minPrefix, size_t minSuffix,
               HyphenationLocale hyphenLocale);

    // apply various hyphenation rules including hard and soft hyphens, ignoring patterns
    void hyphenateWithNoPatterns(const U16StringPiece& word, HyphenationType* out) const;

    // Try looking up word in alphabet table, return DONT_BREAK if any code units fail to map.
    // Otherwise, returns BREAK_AND_INSERT_HYPHEN, BREAK_AND_INSERT_ARMENIAN_HYPHEN, or
    // BREAK_AND_DONT_INSERT_HYPHEN based on the the script of the characters seen.
    // Note that this method writes len+2 entries into alpha_codes (including start and stop)
    HyphenationType alphabetLookup(uint16_t* alpha_codes, const U16StringPiece& word) const;

    // calculate hyphenation from patterns, assuming alphabet lookup has already been done
    void hyphenateFromCodes(const uint16_t* codes, size_t len, HyphenationType hyphenValue,
                            HyphenationType* out) const;

    // See also LONGEST_HYPHENATED_WORD in LineBreaker.cpp. Here the constant is used so
    // that temporary buffers can be stack-allocated without waste, which is a slightly
    // different use case. It measures UTF-16 code units.
    static const size_t MAX_HYPHENATED_SIZE = 64;

    const uint8_t* mPatternData;
    const size_t mMinPrefix, mMinSuffix;
    const HyphenationLocale mHyphenationLocale;

    // accessors for binary data
    const Header* getHeader() const { return reinterpret_cast<const Header*>(mPatternData); }
};

}  // namespace minikin

#endif  // MINIKIN_HYPHENATOR_H
