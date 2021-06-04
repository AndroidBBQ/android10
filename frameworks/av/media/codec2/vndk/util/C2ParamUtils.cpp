/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define __C2_GENERATE_GLOBAL_VARS__ // to be able to implement the methods defined
#include <C2Enum.h>
#include <util/C2Debug-log.h>
#include <util/C2ParamUtils.h>

#include <utility>
#include <vector>

/** \file
 * Utilities for parameter handling to be used by Codec2 implementations.
 */

/// \cond INTERNAL

/* ---------------------------- UTILITIES FOR ENUMERATION REFLECTION ---------------------------- */

static size_t countLeadingUnderscores(C2StringLiteral a) {
    size_t i = 0;
    while (a[i] == '_') {
        ++i;
    }
    return i;
}

static size_t countMatching(C2StringLiteral a, const C2String &b) {
    for (size_t i = 0; i < b.size(); ++i) {
        if (!a[i] || a[i] != b[i]) {
            return i;
        }
    }
    return b.size();
}

// ABCDef => abc-def
// ABCD2ef => abcd2-ef // 0
// ABCD2Ef => ancd2-ef // -1
// AbcDef => abc-def // -1
// Abc2Def => abc-2def
// Abc2def => abc-2-def
// _Yo => _yo
// _yo => _yo
// C2_yo => c2-yo
// C2__yo => c2-yo

//static
C2String _C2EnumUtils::camelCaseToDashed(C2String name) {
    enum {
        kNone = '.',
        kLower = 'a',
        kUpper = 'A',
        kDigit = '1',
        kDash = '-',
        kUnderscore = '_',
    } type = kNone;
    size_t word_start = 0;
    for (size_t ix = 0; ix < name.size(); ++ix) {
        C2_LOG(VERBOSE) << name.substr(0, word_start) << "|"
                << name.substr(word_start, ix - word_start) << "["
                << name.substr(ix, 1) << "]" << name.substr(ix + 1)
                << ": " << (char)type;
        if (isupper(name[ix])) {
            if (type == kLower) {
                name.insert(ix++, 1, '-');
                word_start = ix;
            }
            name[ix] = tolower(name[ix]);
            type = kUpper;
        } else if (islower(name[ix])) {
            if (type == kDigit && ix > 0) {
                name.insert(ix++, 1, '-');
                word_start = ix;
            } else if (type == kUpper && ix > word_start + 1) {
                name.insert(ix++ - 1, 1, '-');
                word_start = ix - 1;
            }
            type = kLower;
        } else if (isdigit(name[ix])) {
            if (type == kLower) {
                name.insert(ix++, 1, '-');
                word_start = ix;
            }
            type = kDigit;
        } else if (name[ix] == '_') {
            if (type == kDash) {
                name.erase(ix--, 1);
            } else if (type != kNone && type != kUnderscore) {
                name[ix] = '-';
                type = kDash;
                word_start = ix + 1;
            } else {
                type = kUnderscore;
                word_start = ix + 1;
            }
        } else {
            name.resize(ix);
        }
    }
    C2_LOG(VERBOSE) << "=> " << name;
    return name;
}

//static
std::vector<C2String> _C2EnumUtils::sanitizeEnumValueNames(
        const std::vector<C2StringLiteral> names,
        C2StringLiteral _prefix) {
    std::vector<C2String> sanitizedNames;
    C2String prefix;
    size_t extraUnderscores = 0;
    bool first = true;
    if (_prefix) {
        extraUnderscores = countLeadingUnderscores(_prefix);
        prefix = _prefix + extraUnderscores;
        first = false;
        C2_LOG(VERBOSE) << "prefix:" << prefix << ", underscores:" << extraUnderscores;
    }

    // calculate prefix and minimum leading underscores
    for (C2StringLiteral s : names) {
        C2_LOG(VERBOSE) << s;
        size_t underscores = countLeadingUnderscores(s);
        if (first) {
            extraUnderscores = underscores;
            prefix = s + underscores;
            first = false;
        } else {
            size_t matching = countMatching(
                s + underscores,
                prefix);
            prefix.resize(matching);
            extraUnderscores = std::min(underscores, extraUnderscores);
        }
        C2_LOG(VERBOSE) << "prefix:" << prefix << ", underscores:" << extraUnderscores;
        if (prefix.size() == 0 && extraUnderscores == 0) {
            break;
        }
    }

    // we swallow the first underscore after upper case prefixes
    bool upperCasePrefix = true;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (islower(prefix[i])) {
            upperCasePrefix = false;
            break;
        }
    }

    for (C2StringLiteral s : names) {
        size_t underscores = countLeadingUnderscores(s);
        C2String sanitized = C2String(s, underscores - extraUnderscores);
        sanitized.append(s + prefix.size() + underscores +
                    (upperCasePrefix && s[prefix.size() + underscores] == '_'));
        sanitizedNames.push_back(camelCaseToDashed(sanitized));
    }

    for (C2String s : sanitizedNames) {
        C2_LOG(VERBOSE) << s;
    }

    return sanitizedNames;
}

//static
std::vector<C2String> _C2EnumUtils::parseEnumValuesFromString(C2StringLiteral value) {
    std::vector<C2String> foundNames;
    size_t pos = 0, len = strlen(value);
    do {
        size_t endPos = strcspn(value + pos, " ,=") + pos;
        if (endPos > pos) {
            foundNames.emplace_back(value + pos, endPos - pos);
        }
        if (value[endPos] && value[endPos] != ',') {
            endPos += strcspn(value + endPos, ",");
        }
        pos = strspn(value + endPos, " ,") + endPos;
    } while (pos < len);
    return foundNames;
}

/// safe(r) parsing from parameter blob
//static
C2Param *C2ParamUtils::ParseFirst(const uint8_t *blob, size_t size) {
    // _mSize must fit into size, but really C2Param must also to be a valid param
    if (size < sizeof(C2Param)) {
        return nullptr;
    }
    // _mSize must match length
    C2Param *param = (C2Param*)blob;
    if (param->size() > size) {
        return nullptr;
    }
    return param;
}

