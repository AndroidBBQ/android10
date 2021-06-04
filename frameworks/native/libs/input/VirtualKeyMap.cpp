/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "VirtualKeyMap"

#include <stdlib.h>
#include <string.h>

#include <input/VirtualKeyMap.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <utils/Tokenizer.h>
#include <utils/Timers.h>

// Enables debug output for the parser.
#define DEBUG_PARSER 0

namespace android {

static const char* WHITESPACE = " \t\r";
static const char* WHITESPACE_OR_FIELD_DELIMITER = " \t\r:";


// --- VirtualKeyMap ---

VirtualKeyMap::VirtualKeyMap() {
}

VirtualKeyMap::~VirtualKeyMap() {
}

std::unique_ptr<VirtualKeyMap> VirtualKeyMap::load(const std::string& filename) {
    Tokenizer* t;
    status_t status = Tokenizer::open(String8(filename.c_str()), &t);
    if (status != OK) {
        ALOGE("Error %d opening virtual key map file %s.", status, filename.c_str());
        return nullptr;
    }
    std::unique_ptr<Tokenizer> tokenizer(t);
    // Using 'new' to access a non-public constructor
    std::unique_ptr<VirtualKeyMap> map(new VirtualKeyMap());
    if (!map) {
        ALOGE("Error allocating virtual key map.");
        return nullptr;
    }

    Parser parser(map.get(), tokenizer.get());
    status = parser.parse();
    if (status != OK) {
        return nullptr;
    }

    return map;
}


// --- VirtualKeyMap::Parser ---

VirtualKeyMap::Parser::Parser(VirtualKeyMap* map, Tokenizer* tokenizer) :
        mMap(map), mTokenizer(tokenizer) {
}

VirtualKeyMap::Parser::~Parser() {
}

status_t VirtualKeyMap::Parser::parse() {
    while (!mTokenizer->isEof()) {
#if DEBUG_PARSER
        ALOGD("Parsing %s: '%s'.", mTokenizer->getLocation().string(),
                mTokenizer->peekRemainderOfLine().string());
#endif

        mTokenizer->skipDelimiters(WHITESPACE);

        if (!mTokenizer->isEol() && mTokenizer->peekChar() != '#') {
            // Multiple keys can appear on one line or they can be broken up across multiple lines.
            do {
                String8 token = mTokenizer->nextToken(WHITESPACE_OR_FIELD_DELIMITER);
                if (token != "0x01") {
                    ALOGE("%s: Unknown virtual key type, expected 0x01.",
                          mTokenizer->getLocation().string());
                    return BAD_VALUE;
                }

                VirtualKeyDefinition defn;
                bool success = parseNextIntField(&defn.scanCode)
                        && parseNextIntField(&defn.centerX)
                        && parseNextIntField(&defn.centerY)
                        && parseNextIntField(&defn.width)
                        && parseNextIntField(&defn.height);
                if (!success) {
                    ALOGE("%s: Expected 5 colon-delimited integers in virtual key definition.",
                          mTokenizer->getLocation().string());
                    return BAD_VALUE;
                }

#if DEBUG_PARSER
                ALOGD("Parsed virtual key: scanCode=%d, centerX=%d, centerY=%d, "
                        "width=%d, height=%d",
                        defn.scanCode, defn.centerX, defn.centerY, defn.width, defn.height);
#endif
                mMap->mVirtualKeys.push_back(defn);
            } while (consumeFieldDelimiterAndSkipWhitespace());

            if (!mTokenizer->isEol()) {
                ALOGE("%s: Expected end of line, got '%s'.",
                        mTokenizer->getLocation().string(),
                        mTokenizer->peekRemainderOfLine().string());
                return BAD_VALUE;
            }
        }

        mTokenizer->nextLine();
    }

    return NO_ERROR;
}

bool VirtualKeyMap::Parser::consumeFieldDelimiterAndSkipWhitespace() {
    mTokenizer->skipDelimiters(WHITESPACE);
    if (mTokenizer->peekChar() == ':') {
        mTokenizer->nextChar();
        mTokenizer->skipDelimiters(WHITESPACE);
        return true;
    }
    return false;
}

bool VirtualKeyMap::Parser::parseNextIntField(int32_t* outValue) {
    if (!consumeFieldDelimiterAndSkipWhitespace()) {
        return false;
    }

    String8 token = mTokenizer->nextToken(WHITESPACE_OR_FIELD_DELIMITER);
    char* end;
    *outValue = strtol(token.string(), &end, 0);
    if (token.isEmpty() || *end != '\0') {
        ALOGE("Expected an integer, got '%s'.", token.string());
        return false;
    }
    return true;
}

} // namespace android
