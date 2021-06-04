/*
 * Copyright (C) 2013 The Android Open Source Project
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

/* This program processes Renderscript function definitions described in spec files.
 * For each spec file provided on the command line, it generates a corresponding
 * Renderscript header (*.rsh) which is meant for inclusion in client scripts.
 *
 * This program also generates Junit test files to automatically test each of the
 * functions using randomly generated data.  We create two files for each function:
 * - a Renderscript file named Test{Function}.rs,
 * - a Junit file named Test{function}.java, which calls the above RS file.
 *
 * Finally, this program generates HTML documentation files.
 *
 * This program takes an optional -v parameter, the API level to target.  The generated
 * files will not contain APIs passed that API level.  Note that this does not affect
 * generic comments found in headers.
 *
 * This program contains five main classes:
 * - SpecFile: Represents on spec file.
 * - Function: Each instance represents a function, like clamp.  Even though the
 *      spec file contains many entries for clamp, we'll only have one clamp instance.
 * - FunctionSpecification: Defines one of the many variations of the function.  There's
 *      a one to one correspondance between FunctionSpecification objects and entries in the
 *      spec file.  Strings that are parts of a FunctionSpecification can include placeholders,
 *      which are "#1", "#2", "#3", and "#4".  We'll replace these by values before
 *      generating the files.
 * - Permutation: A concrete version of a specification, where all placeholders have
 *      been replaced by actual values.
 * - ParameterDefinition: A definition of a parameter of a concrete function.
 *
 * The format of the .spec files is described below.  Line that starts with # are comments.
 * Replace the {} sections with your own contents.  [] indicates optional parts.
 *
 * It should start with a header as follows:
 *
 * header:
 * summary:  {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * [include:
 *     { Multiline code lines to be included as-is in the generated header file.}]
 * end:
 *
 * Constants are defined as follows:
 *
 * constant:  {The name of the constant.}
 * [version: ({Starting API level} [ {Last API level that supports this.}] | UNRELEASED)
 * [size: {32 or 64.  Used if this is available only for 32 or 64 bit code.}]
 * value: {The value of the constant.}
 * type: {The type of the constant.}
 * [hidden:]   ...If present, don't document the constant.  Omit the following two fields.
 * [deprecated: [{Deprecation message.}]   ... This is deprecated.  Compiler will issue a wrning.
 * summary: {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * end:
 *
 * Types can either be simple types, structs, or enums.  They have the format:
 *
 * type:  {The typedef name of the type.}
 * [version: ({Starting API level} [ {Last API level that supports this.}] | UNRELEASED)
 * [size: {32 or 64.  Used if this is available only for 32 or 64 bit code.}]
 * simple: {The C declaration that this type is the typedef equivalent.}
 * [hidden:]   ...If present, don't document the type.  Omit the following two fields.
 * [deprecated: [{Deprecation message.}]   ... This is deprecated.  Compiler will issue a wrning.
 * summary: {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * end:
 *
 * type:  {The typedef name of the type.}
 * [version: ({Starting API level} [ {Last API level that supports this.}] | UNRELEASED)
 * [size: {32 or 64.  Used if this is available only for 32 or 64 bit code.}]
 * struct: [{The name that will appear right after the struct keyword}]
 * field: {Type and name of the field}[, "{One line documentation of the field}"]
 * field:   ... Same for all the other fields of the struct.
 * [attrib: {Attributes of the struct.}]
 * [hidden:]   ...If present, don't document the type.  Omit the following two fields.
 * summary: {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * end:
 *
 * type:  {The typedef name of the type.}
 * [version: ({Starting API level} [ {Last API level that supports this.}] | UNRELEASED)
 * [size: {32 or 64.  Used if this is available only for 32 or 64 bit code.}]
 * enum: [{The name that will appear right after the enum keyword}]
 * value: {Type and name of the field}[, "{One line documentation of the field}"]
 * value:   ... Same for all the other values of the enum.
 * [hidden:]   ...If present, don't document the type.  Omit the following two fields.
 * summary: {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * end:

 * Functions have the following format:
 *
 * function:  {The name of the function.}
 * [version: ({Starting API level} [ {Last API level that supports this.}] | UNRELEASED)
 * [size: {32 or 64.  Used if this is available only for 32 or 64 bit code.}]
 * [attrib: {Attributes of the function.}]
 * [w: {A comma separated list of width supported.  Only 1, 2, 3, 4 are supported.
 * [t: {A comma separated list of the types supported.}]]
 * ... Up to four w: or t: can be defined.  The order matter.  These will be replace
 * ... the #1, #2, #3, #4 that can be found in the rest of the specification.
 * ret: [{The return type} [, "{One line documentation of the return}"]]
 * [arg:(({Type}[ {Name})]|{Elipsis})[, {ParameterEntry.testOption}][, "{One line documentation of the field}"]]
 * [arg:   ... Same for all the other arguments of the function.]
 * [hidden:]   ... If present, don't include in the HTML documentation.
 * [deprecated: [{Deprecation message.}]   ... This is deprecated.  Compiler will issue a wrning.
 * summary: {A one line string describing this section.}
 * description:
 *     {Multiline description.  Can include HTML.  References to constants, types,
 *      and functions can be created by prefixing with a '@'.}
 * [inline:
 *     {Multiline code that implements this function inline.}]
 * [test: {How to test this function.  See FunctionSpecification::mTest.}]
 * end:
 */

#include <stdio.h>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <strings.h>

#include "Generator.h"
#include "Scanner.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

static bool parseCommandLine(int argc, char* argv[], unsigned int* maxApiLevel,
                             vector<string>* specFileNames) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'v') {
                i++;
                if (i < argc) {
                    char* end;
                    *maxApiLevel = strtol(argv[i], &end, 10);
                    if (*end != '\0') {
                        cerr << "Error. Can't parse the version number" << argv[i] << "\n";
                        return false;
                    }
                } else {
                    cerr << "Missing version number after -v\n";
                    return false;
                }
            } else {
                cerr << "Unrecognized flag %s\n" << argv[i] << "\n";
                return false;
            }
        } else {
            specFileNames->push_back(argv[i]);
        }
    }
    if (specFileNames->size() == 0) {
        cerr << "No spec file specified\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    // If there's no restriction, generated test files for the very highest version.
    unsigned int maxApiLevel = VersionInfo::kUnreleasedVersion;
    vector<string> specFileNames;
    if (!parseCommandLine(argc, argv, &maxApiLevel, &specFileNames)) {
        cout << "Usage: gen_runtime spec_file [spec_file...] [-v "
                "version_of_test_files]\n";
        return -1;
    }
    bool success = true;
    for (auto i : specFileNames) {
        if (!systemSpecification.readSpecFile(i, maxApiLevel)) {
            success = false;
        }
    }
    if (success) {
        success = systemSpecification.generateFiles(maxApiLevel);
    }
    return success ? 0 : -2;
}
