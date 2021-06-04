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

#include "LineBreakerUtil.h"

namespace minikin {

// Very long words trigger O(n^2) behavior in hyphenation, so we disable hyphenation for
// unreasonably long words. This is somewhat of a heuristic because extremely long words are
// possible in some languages. This does mean that very long real words can get broken by
// desperate breaks, with no hyphens.
constexpr size_t LONGEST_HYPHENATED_WORD = 45;

// Hyphenates a string potentially containing non-breaking spaces.
std::vector<HyphenationType> hyphenate(const U16StringPiece& str, const Hyphenator& hyphenator) {
    std::vector<HyphenationType> out;
    const size_t len = str.size();
    out.resize(len);

    // A word here is any consecutive string of non-NBSP characters.
    bool inWord = false;
    size_t wordStart = 0;  // The initial value will never be accessed, but just in case.
    for (size_t i = 0; i <= len; i++) {
        if (i == len || str[i] == CHAR_NBSP) {
            if (inWord) {
                // A word just ended. Hyphenate it.
                const U16StringPiece word = str.substr(Range(wordStart, i));
                if (word.size() <= LONGEST_HYPHENATED_WORD) {
                    hyphenator.hyphenate(word, out.data() + wordStart);
                } else {  // Word is too long. Inefficient to hyphenate.
                    out.insert(out.end(), word.size(), HyphenationType::DONT_BREAK);
                }
                inWord = false;
            }
            if (i < len) {
                // Insert one DONT_BREAK for the NBSP.
                out.push_back(HyphenationType::DONT_BREAK);
            }
        } else if (!inWord) {
            inWord = true;
            wordStart = i;
        }
    }
    return out;
}

}  // namespace minikin
