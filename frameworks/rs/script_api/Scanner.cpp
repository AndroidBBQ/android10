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

#include <iostream>
#include <sstream>

#include "Scanner.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

// Maximum of errors we'll report before bailing out.
const int MAX_ERRORS = 10;

Scanner::Scanner(const string& fileName, FILE* file)
    : mFileName(fileName), mFile(file), mLineNumber(0), mTagConsumed(true), mErrorCount(0) {
}

bool Scanner::atEnd() {
    return (mTagConsumed && feof(mFile)) || mErrorCount > MAX_ERRORS;
}

int Scanner::getChar() {
    int c = fgetc(mFile);
    if (c == '\n') {
        mLineNumber++;
    }
    return c;
}

void Scanner::readUpTo(char delimiter, string* segment) {
    for (;;) {
        int c = getChar();
        if (c == EOF || c == '\n') {
            break;
        }
        segment->push_back((char)c);
        if (c == delimiter) {
            break;
        }
    }
}

void Scanner::readRestOfLine(string* segment) {
    for (;;) {
        int c = getChar();
        if (c == EOF || c == '\n') {
            return;
        }
        segment->push_back((char)c);
    }
}

bool Scanner::getNextEntry() {
    mTag.clear();
    mValue.clear();
    for (;;) {
        int c = getChar();
        if (c == EOF) {
            return false;
        }
        if (c == '#') {
            // Skip the comment
            string comment;
            readRestOfLine(&comment);
            continue;
        }
        if (c == ' ') {
            readRestOfLine(&mValue);
            break;
        } else if (c == '\n') {
            break;
        } else {
            mTag = c;
            readUpTo(':', &mTag);
            readRestOfLine(&mValue);
            trimSpaces(&mValue);
            break;
        }
    }
    return true;
}

ostream& Scanner::error() {
    return error(mLineNumber);
}

ostream& Scanner::error(int lineNumber) {
    if (++mErrorCount <= MAX_ERRORS) {
        cerr << mFileName << ":" << lineNumber << ": error: ";
    }
    return cerr;
}

void Scanner::skipBlankEntries() {
    while (findOptionalTag("")) {
        if (!mValue.empty()) {
            error() << "Unexpected: \" " << mValue << "\".\n";
        }
    }
}

bool Scanner::findTag(const char* tag) {
    bool found = findOptionalTag(tag);
    if (!found) {
        error() << "Found \"" << mTag << "\" while looking for \"" << tag << "\".\n";
    }
    mTagConsumed = true;
    return found;
}

bool Scanner::findOptionalTag(const char* tag) {
    if (mTagConsumed) {
        if (!getNextEntry()) {
            return false;
        }
    }
    mTagConsumed = (mTag == tag);
    return mTagConsumed;
}

void Scanner::skipUntilTag(const char* tag) {
    while(!findOptionalTag(tag)) {
        mTagConsumed = true;
    }
}

void Scanner::checkNoValue() {
    if (!mValue.empty()) {
        error() << "Did not expect \"" << mValue << "\" after \"" << mTag << "\".\n";
    }
}

void Scanner::parseDocumentation(string* s, string* documentation) {
    size_t docStart = s->find(", \"");
    if (docStart == string::npos) {
        documentation->erase();
    } else {
        size_t first = docStart + 3;
        size_t last = s->find('\"', first);
        if (last == string::npos) {
            error() << "Missing closing double quote\n";
        }
        *documentation = s->substr(first, last - first);
        s->erase(docStart);
    }
}

ParameterEntry* Scanner::parseArgString(bool isReturn) {
    string s = mValue;
    ParameterEntry* p = new ParameterEntry();
    parseDocumentation(&s, &p->documentation);

    size_t optionStart = s.find(", ");
    if (optionStart != string::npos) {
        p->testOption = s.substr(optionStart + 2);
        s.erase(optionStart);
    }

    trimSpaces(&s);
    if (!isReturn) {
        size_t nameStart = s.rfind(' ');
        if (nameStart == string::npos) {
            if (s == "...") {
                p->name = s;
                p->type = s;
                p->lineNumber = mLineNumber;
                return p;
            } else {
                error() << "Missing variable name\n";
            }
        } else {
            p->name = s.substr(nameStart + 1);
            s.erase(nameStart);
            if (p->name.find('*') != string::npos) {
                error() << "The '*' should be attached to the type\n";
            }
        }
    }

    if (s == "void" && !isReturn) {
        error() << "void is only allowed for ret:\n";
    }
    p->type = s;
    p->lineNumber = mLineNumber;
    return p;
}
