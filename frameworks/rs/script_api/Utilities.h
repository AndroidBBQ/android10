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

#ifndef ANDROID_RS_API_GENERATOR_UTILITIES_H
#define ANDROID_RS_API_GENERATOR_UTILITIES_H

#include <fstream>
#include <set>
#include <string>

// Capitalizes and removes underscores.  E.g. converts "native_log" to NativeLog.
std::string capitalize(const std::string& source);

// Trim trailing and leading spaces from a string.
void trimSpaces(std::string* s);

// Replaces in string s all occurences of match with rep.
std::string stringReplace(std::string s, std::string match, std::string rep);

// Removes the character from present. Returns true if the string contained the character.
bool charRemoved(char c, std::string* s);

// Removes HTML references from the string.
std::string stripHtml(const std::string& html);

// Returns a string that's an hexadecimal constant of the hash of the string.
std::string hashString(const std::string& s);

// Adds a string to a set.  Return true if it was already in.
bool testAndSet(const std::string& flag, std::set<std::string>* set);

/* Returns a double that should be able to be converted to an integer of size
 * numberOfIntegerBits.
 */
double maxDoubleForInteger(int numberOfIntegerBits, int mantissaSize);

/* Creates an " __attribute__((...))" tag.  If userAttribute starts with '=', we don't
 * use the additionalAttribute.  An empty string will be returned if there are no attributes.
 */
std::string makeAttributeTag(const std::string& userAttribute,
                             const std::string& additionalAttribute, unsigned int deprecatedApiLevel,
                             const std::string& deprecatedMessage);

/* This class is used to generate one source file.  There will be one instance
 * for each generated file.
 */
class GeneratedFile : public std::ofstream {
private:
    std::string mIndent;  // The correct spacing at the beginning of each line.
    const int TAB_SIZE = 4;

public:
    // Opens the stream.  Reports an error if it can't.
    bool start(const std::string& directory, const std::string& name);

    // Write copyright notice & auto-generated warning in Java/C style comments.
    void writeNotices();

    void increaseIndent();               // Increases the new line indentation by 4.
    void decreaseIndent();               // Decreases the new line indentation by 4.
    void comment(const std::string& s);  // Outputs a multiline comment.

    // Starts a control block.  This works both for Java and C++.
    void startBlock() {
        *this << " {\n";
        increaseIndent();
    }

    // Ends a control block.
    void endBlock(bool addSemicolon = false) {
        decreaseIndent();
        indent() << "}" << (addSemicolon ? ";" : "") << "\n";
    }

    /* Indents the line.  By returning *this, we can use like this:
     *  mOut.ident() << "a = b;\n";
     */
    std::ofstream& indent() {
        *this << mIndent;
        return *this;
    }

    std::ofstream& indentPlus() {
        *this << mIndent << std::string(2 * TAB_SIZE, ' ');
        return *this;
    }
};

extern const char AUTO_GENERATED_WARNING[];

#endif  // ANDROID_RS_API_GENERATOR_UTILITIES_H
