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

#ifndef ANDROID_RS_API_GENERATOR_SCANNER_H
#define ANDROID_RS_API_GENERATOR_SCANNER_H

#include <fstream>
#include <string>

struct ParameterEntry;

class Scanner {
private:
    std::string mFileName;
    // The file being parsed
    FILE* mFile;
    // The line number of the current entry.
    int mLineNumber;
    // The tag of the current entry to be processed.  See FindTag().
    std::string mTag;
    // The value of this entry.  See FindTag().
    std::string mValue;
    // Was the current tag processed?
    bool mTagConsumed;
    // Number of errors encountered.
    int mErrorCount;

    /* Returns the next character from the file, incrementing the line count
     * if \n is found.
     */
    int getChar();
    /* Reads from the file, adding the characters to "segment" until
     * the delimiter is found, a new line, or the eof.  The delimiter is added.
     */
    void readUpTo(char delimiter, std::string* segment);
    /* Reads from the file, adding the characters to "segment" until
     * the end of the line.
     */
    void readRestOfLine(std::string* segment);

    /* Finds the next line that's not a comment (a line that starts with #).
     * This line is parsed into a tag and a value.
     * A line that starts with a space (or is empty) is considered to have
     * a null tag and all but the first character are the value.
     * Lines that start with a non-space charcter should have a ": " to
     * separate the tag from the value.
     * Returns false if no more entries.
     */
    bool getNextEntry();

public:
    Scanner(const std::string& fileName, FILE* file);
    bool atEnd();
    std::string getValue() { return mValue; }
    std::string getNextTag() {
        mTagConsumed = true;
        return mTag;
    }

    // Skips over blank entries, reporting errors that start with a space.
    void skipBlankEntries();
    /* Finds the next unprocessed tag.  This entry should start with the specified tag.
     * Returns false if the tag is not found and prints an error.
     */
    bool findTag(const char* tag);
    // Same as findTag but does not print an error if the tag is not found.
    bool findOptionalTag(const char* tag);
    // Keep reading from the stream until the tag is found.
    void skipUntilTag(const char* tag);
    // Verifies there's no value.
    void checkNoValue();

    std::ostream& error();
    std::ostream& error(int lineNumber);

    /* Removes an optional double quoted "documentation" found at the end of a line.
     * Erases that documention from the input string.
     */
    void parseDocumentation(std::string* s, std::string* documentation);
    /* Parse an arg: definition.  It's of the form:
     *    type[*] name [, test_option] [, "documentation"]
     * The type and name are required.  The * indicates it's an output parameter.
     * The test_option specifiies restrictions on values used when generating the test cases.
     * It's one of range(), compatible(), conditional(), above().
     * The documentation is enclosed in double quotes.
     */
    ParameterEntry* parseArgString(bool isReturn);
    bool getErrorCount() const { return mErrorCount; }
};

#endif  // ANDROID_RS_API_GENERATOR_SCANNER_H
