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

#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

const unsigned int kMinimumApiLevelForTests = 11;
const unsigned int kApiLevelWithFirst64Bit = 21;

// Used to map the built-in types to their mangled representations
struct BuiltInMangling {
    const char* token[3];     // The last two entries can be nullptr
    const char* equivalence;  // The mangled equivalent
};

BuiltInMangling builtInMangling[] = {
            {{"long", "long"}, "x"},
            {{"unsigned", "long", "long"}, "y"},
            {{"long"}, "l"},
            {{"unsigned", "long"}, "m"},
            {{"int"}, "i"},
            {{"unsigned", "int"}, "j"},
            {{"short"}, "s"},
            {{"unsigned", "short"}, "t"},
            {{"char"}, "c"},
            {{"unsigned", "char"}, "h"},
            {{"signed", "char"}, "a"},
            {{"void"}, "v"},
            {{"wchar_t"}, "w"},
            {{"bool"}, "b"},
            {{"__fp16"}, "Dh"},
            {{"float"}, "f"},
            {{"double"}, "d"},
};

/* For the given API level and bitness (e.g. 32 or 64 bit), try to find a
 * substitution for the provided type name, as would be done (mostly) by a
 * preprocessor.  Returns empty string if there's no substitution.
 */
static string findSubstitute(const string& typeName, unsigned int apiLevel, int intSize) {
    const auto& types = systemSpecification.getTypes();
    const auto type = types.find(typeName);
    if (type != types.end()) {
        for (TypeSpecification* spec : type->second->getSpecifications()) {
            // Verify this specification applies
            const VersionInfo info = spec->getVersionInfo();
            if (!info.includesVersion(apiLevel) || (info.intSize != 0 && info.intSize != intSize)) {
                continue;
            }
            switch (spec->getKind()) {
                case SIMPLE: {
                    return spec->getSimpleType();
                }
                case RS_OBJECT: {
                    // Do nothing for RS object types.
                    break;
                }
                case STRUCT: {
                    return spec->getStructName();
                }
                case ENUM:
                    // Do nothing
                    break;
            }
        }
    }
    return "";
}

/* Expand the typedefs found in 'type' into their equivalents and tokenize
 * the resulting list.  'apiLevel' and 'intSize' specifies the API level and bitness
 * we are currently processing.
 */
list<string> expandTypedefs(const string type, unsigned int apiLevel, int intSize, string& vectorSize) {
    // Split the string in tokens.
    istringstream stream(type);
    list<string> tokens{istream_iterator<string>{stream}, istream_iterator<string>{}};
    // Try to substitue each token.
    for (auto i = tokens.begin(); i != tokens.end();) {
        const string substitute = findSubstitute(*i, apiLevel, intSize);
        if (substitute.empty()) {
            // No substitution possible, just go to the next token.
            i++;
        } else {
            // Split the replacement string in tokens.

            /* Get the new vector size. This is for the case of the type being for example
             * rs_quaternion* == float4*, where we need the vector size to be 4 for the
             * purposes of mangling, although the parameter itself is not determined to be
             * a vector. */
            string unused;
            string newVectorSize;
            getVectorSizeAndBaseType(*i, newVectorSize, unused);

            istringstream vectorSizeBuf(vectorSize);
            int vectorSizeVal;
            vectorSizeBuf >> vectorSizeVal;

            istringstream newVectorSizeBuf(newVectorSize);
            int newVectorSizeVal;
            newVectorSizeBuf >> newVectorSizeVal;

            if (newVectorSizeVal > vectorSizeVal)
                vectorSize = newVectorSize;

            istringstream stream(substitute);
            list<string> newTokens{istream_iterator<string>{stream}, istream_iterator<string>{}};
            // Replace the token with the substitution. Don't advance, as the new substitution
            // might itself be replaced.
            // hold previous node
            auto prev = i;
            // insert new nodes after node i
            tokens.splice(++i, std::move(newTokens));
            // remove previous node and set i to beginning of inserted nodes
            i = tokens.erase(prev);
        }
    }
    return tokens;
}

// Remove the first element of the list if it equals 'prefix'.  Return true in that case.
static bool eatFront(list<string>* tokens, const char* prefix) {
    if (tokens->front() == prefix) {
        tokens->pop_front();
        return true;
    }
    return false;
}

/* Search the table of translations for the built-ins for the mangling that
 * corresponds to this list of tokens.  If a match is found, consume these tokens
 * and return a pointer to the string.  If not, return nullptr.
 */
static const char* findManglingOfBuiltInType(list<string>* tokens) {
    for (const BuiltInMangling& a : builtInMangling) {
        auto t = tokens->begin();
        auto end = tokens->end();
        bool match = true;
        // We match up to three tokens.
        for (int i = 0; i < 3; i++) {
            if (!a.token[i]) {
                // No more tokens
                break;
            }
            if (t == end || *t++ != a.token[i]) {
                match = false;
            }
        }
        if (match) {
            tokens->erase(tokens->begin(), t);
            return a.equivalence;
        }
    }
    return nullptr;
}

// Mangle a long name by prefixing it with its length, e.g. "13rs_allocation".
static inline string mangleLongName(const string& name) {
    return to_string(name.size()) + name;
}

/* Mangle the type name that's represented by the vector size and list of tokens.
 * The mangling will be returned in full form in 'mangling'.  'compressedMangling'
 * will have the compressed equivalent.  This is built using the 'previousManglings'
 * list.  false is returned if an error is encountered.
 *
 * This function is recursive because compression is possible at each level of the definition.
 * See http://mentorembedded.github.io/cxx-abi/abi.html#mangle.type for a description
 * of the Itanium mangling used by llvm.
 *
 * This function mangles correctly the types currently used by RenderScript.  It does
 * not currently mangle more complicated types like function pointers, namespaces,
 * or other C++ types.  In particular, we don't deal correctly with parenthesis.
 */
static bool mangleType(string vectorSize, list<string>* tokens, vector<string>* previousManglings,
                       string* mangling, string* compressedMangling) {
    string delta;                 // The part of the mangling we're generating for this recursion.
    bool isTerminal = false;      // True if this iteration parses a terminal node in the production.
    bool canBeCompressed = true;  // Will be false for manglings of builtins.

    if (tokens->back() == "*") {
        delta = "P";
        tokens->pop_back();
    } else if (eatFront(tokens, "const")) {
        delta = "K";
    } else if (eatFront(tokens, "volatile")) {
        delta = "V";
    } else if (vectorSize != "1" && vectorSize != "") {
        // For vector, prefix with the abbreviation for a vector, including the size.
        delta = "Dv" + vectorSize + "_";
        vectorSize.clear();  // Reset to mark the size as consumed.
    } else if (eatFront(tokens, "struct")) {
        // For a structure, we just use the structure name
        if (tokens->size() == 0) {
            cerr << "Expected a name after struct\n";
            return false;
        }
        delta = mangleLongName(tokens->front());
        isTerminal = true;
        tokens->pop_front();
    } else if (eatFront(tokens, "...")) {
        delta = "z";
        isTerminal = true;
    } else {
        const char* c = findManglingOfBuiltInType(tokens);
        if (c) {
            // It's a basic type.  We don't use those directly for compression.
            delta = c;
            isTerminal = true;
            canBeCompressed = false;
        } else if (tokens->size() > 0) {
            // It's a complex type name.
            delta = mangleLongName(tokens->front());
            isTerminal = true;
            tokens->pop_front();
        }
    }

    if (isTerminal) {
        // If we're the terminal node, there should be nothing left to mangle.
        if (tokens->size() > 0) {
            cerr << "Expected nothing else but found";
            for (const auto& t : *tokens) {
                cerr << " " << t;
            }
            cerr << "\n";
            return false;
        }
        *mangling = delta;
        *compressedMangling = delta;
    } else {
        // We're not terminal.  Recurse and prefix what we've translated this pass.
        if (tokens->size() == 0) {
            cerr << "Expected a more complete type\n";
            return false;
        }
        string rest, compressedRest;
        if (!mangleType(vectorSize, tokens, previousManglings, &rest, &compressedRest)) {
            return false;
        }
        *mangling = delta + rest;
        *compressedMangling = delta + compressedRest;
    }

    /* If it's a built-in type, we don't look at previously emitted ones and we
     * don't keep track of it.
     */
    if (!canBeCompressed) {
        return true;
    }

    // See if we've encountered this mangling before.
    for (size_t i = 0; i < previousManglings->size(); ++i) {
        if ((*previousManglings)[i] == *mangling) {
            // We have a match, construct an index reference to that previously emitted mangling.
            ostringstream stream2;
            stream2 << 'S';
            if (i > 0) {
                stream2 << (char)('0' + i - 1);
            }
            stream2 << '_';
            *compressedMangling = stream2.str();
            return true;
        }
    }

    // We have not encountered this before.  Add it to the list.
    previousManglings->push_back(*mangling);
    return true;
}

// Write to the stream the mangled representation of each parameter.
static bool writeParameters(ostringstream* stream, const std::vector<ParameterDefinition*>& params,
                            unsigned int apiLevel, int intSize) {
    if (params.empty()) {
        *stream << "v";
        return true;
    }
    /* We keep track of the previously generated parameter types, as type mangling
     * is compressed by reusing previous manglings.
     */
    vector<string> previousManglings;
    for (ParameterDefinition* p : params) {
        // Expand the typedefs and create a tokenized list.
        string vectorSize = p->mVectorSize;
        list<string> tokens = expandTypedefs(p->rsType, apiLevel, intSize, vectorSize);
        if (p->isOutParameter) {
            tokens.push_back("*");
        }
        string mangling, compressedMangling;

        if (!mangleType(vectorSize, &tokens, &previousManglings, &mangling,
                        &compressedMangling)) {
            return false;
        }
        *stream << compressedMangling;
    }
    return true;
}

/* Add the mangling for this permutation of the function.  apiLevel and intSize is used
 * to select the correct type when expanding complex type.
 */
static bool addFunctionManglingToSet(const FunctionPermutation& permutation,
                                     bool overloadable, unsigned int apiLevel,
                                     int intSize, set<string>* allManglings) {
    const string& functionName = permutation.getName();
    string mangling;
    if (overloadable) {
        ostringstream stream;
        stream << "_Z" << mangleLongName(functionName);
        if (!writeParameters(&stream, permutation.getParams(), apiLevel, intSize)) {
            cerr << "Error mangling " << functionName << ".  See above message.\n";
            return false;
        }
        mangling = stream.str();
    } else {
        mangling = functionName;
    }
    allManglings->insert(mangling);
    return true;
}

/* Add to the set the mangling of each function prototype that can be generated from this
 * specification, i.e. for all the versions covered and for 32/64 bits.  We call this
 * for each API level because the implementation of a type may have changed in the range
 * of API levels covered.
 */
static bool addManglingsForSpecification(const FunctionSpecification& spec,
                                         unsigned int lastApiLevel,
                                         set<string>* allManglings) {
    // If the function is inlined, we won't generate an unresolved external for that.
    if (spec.hasInline()) {
        return true;
    }
    const VersionInfo info = spec.getVersionInfo();
    unsigned int minApiLevel, maxApiLevel;
    minApiLevel = info.minVersion ? info.minVersion : kMinimumApiLevelForTests;
    maxApiLevel = info.maxVersion ? info.maxVersion : lastApiLevel;
    const bool overloadable = spec.isOverloadable();

    /* We track success rather than aborting early in case of failure so that we
     * generate all the error messages.
     */
    bool success = true;
    // Use 64-bit integer here for the loop count to avoid overflow
    // (minApiLevel == maxApiLevel == UINT_MAX for unreleased API)
    for (int64_t apiLevel = minApiLevel; apiLevel <= maxApiLevel; ++apiLevel) {
        for (auto permutation : spec.getPermutations()) {
            if (info.intSize == 0 || info.intSize == 32) {
                if (!addFunctionManglingToSet(*permutation, overloadable,
                                              apiLevel, 32, allManglings)) {
                    success = false;
                }
            }
            if (apiLevel >= kApiLevelWithFirst64Bit && (info.intSize == 0 || info.intSize == 64)) {
                if (!addFunctionManglingToSet(*permutation, overloadable,
                                              apiLevel, 64, allManglings)) {
                    success = false;
                }
            }
        }
    }
    return success;
}

/* Generate the white list file of the mangled function prototypes.  This generated list is used
 * to validate unresolved external references.  'lastApiLevel' is the largest api level found in
 * all spec files.
 */
static bool generateWhiteListFile(unsigned int lastApiLevel) {
    bool success = true;
    // We generate all the manglings in a set to remove duplicates and to order them.
    set<string> allManglings;
    for (auto f : systemSpecification.getFunctions()) {
        const Function* function = f.second;
        for (auto spec : function->getSpecifications()) {
            // Compiler intrinsics are not runtime APIs. Do not include them in the whitelist.
            if (spec->isIntrinsic()) {
                continue;
            }
            if (!addManglingsForSpecification(*spec, lastApiLevel,
                                              &allManglings)) {
                success = false;  // We continue so we can generate all errors.
            }
        }
    }

    if (success) {
        GeneratedFile file;
        if (!file.start(".", "RSStubsWhiteList.cpp")) {
            return false;
        }

        file.writeNotices();
        file << "#include \"RSStubsWhiteList.h\"\n\n";
        file << "std::vector<std::string> stubList = {\n";
        for (const auto& e : allManglings) {
            file << "\"" << e << "\",\n";
        }
        file << "};\n";
    }
    return success;
}

// Add a uniquely named variable definition to the file and return its name.
static const string addVariable(GeneratedFile* file, unsigned int* variableNumber) {
    const string name = "buf" + to_string((*variableNumber)++);
    /* Some data structures like rs_tm can't be exported.  We'll just use a dumb buffer
     * and cast its address later on.
     */
    *file << "char " << name << "[200];\n";
    return name;
}

/* Write to the file the globals needed to make the call for this permutation.  The actual
 * call is stored in 'calls', as we'll need to generate all the global variable declarations
 * before the function definition.
 */
static void generateTestCall(GeneratedFile* file, ostringstream* calls,
                             unsigned int* variableNumber,
                             const FunctionPermutation& permutation) {
    *calls << "    ";

    // Handle the return type.
    const auto ret = permutation.getReturn();
    if (ret && ret->rsType != "void" && ret->rsType != "const void") {
        *calls << "*(" << ret->rsType << "*)" << addVariable(file, variableNumber) << " = ";
    }

    *calls << permutation.getName() << "(";

    // Generate the arguments.
    const char* separator = "";
    for (auto p : permutation.getParams()) {
        *calls << separator;
        if (p->rsType == "rs_kernel_context") {
            // Special case for the kernel context, as it has a special existence.
            *calls << "context";
        } else if (p->rsType == "...") {
            // Special case for varargs. No need for casting.
            *calls << addVariable(file, variableNumber);
        } else if (p->isOutParameter) {
            *calls << "(" << p->rsType << "*) " << addVariable(file, variableNumber);
        } else {
            *calls << "*(" << p->rsType << "*)" << addVariable(file, variableNumber);
        }
        separator = ", ";
    }
    *calls << ");\n";
}

/* Generate a test file that will be used in the frameworks/compile/slang/tests unit tests.
 * This file tests that all RenderScript APIs can be called for the specified API level.
 * To avoid the compiler agressively pruning out our calls, we use globals as inputs and outputs.
 *
 * Since some structures can't be defined at the global level, we use casts of simple byte
 * buffers to get around that restriction.
 *
 * This file can be used to verify the white list that's also generated in this file.  To do so,
 * run "llvm-nm -undefined-only -just-symbol-name" on the resulting bit code.
 */
static bool generateApiTesterFile(const string& slangTestDirectory, unsigned int apiLevel) {
    GeneratedFile file;
    if (!file.start(slangTestDirectory, "all" + to_string(apiLevel) + ".rs")) {
        return false;
    }

    /* This unusual comment is used by slang/tests/test.py to know which parameter to pass
     * to llvm-rs-cc when compiling the test.
     */
    file << "// -target-api " << apiLevel << " -Wno-deprecated-declarations\n";

    file.writeNotices();
    file << "#pragma version(1)\n";
    file << "#pragma rs java_package_name(com.example.renderscript.testallapi)\n\n";
    if (apiLevel < 23) {  // All rs_graphics APIs were deprecated in api level 23.
        file << "#include \"rs_graphics.rsh\"\n\n";
    }

    /* The code below emits globals and calls to functions in parallel.  We store
     * the calls in a stream so that we can emit them in the file in the proper order.
     */
    ostringstream calls;
    unsigned int variableNumber = 0;  // Used to generate unique names.
    for (auto f : systemSpecification.getFunctions()) {
        const Function* function = f.second;
        for (auto spec : function->getSpecifications()) {
            // Do not include internal APIs in the API tests.
            if (spec->isInternal()) {
                continue;
            }
            VersionInfo info = spec->getVersionInfo();
            if (!info.includesVersion(apiLevel)) {
                continue;
            }
            if (info.intSize == 32) {
                calls << "#ifndef __LP64__\n";
            } else if (info.intSize == 64) {
                calls << "#ifdef __LP64__\n";
            }
            for (auto permutation : spec->getPermutations()) {
                // http://b/27358969 Do not test rsForEach in the all-api test.
                if (apiLevel >= 24 && permutation->getName().compare(0, 9, "rsForEach") == 0)
                  continue;
                generateTestCall(&file, &calls, &variableNumber, *permutation);
            }
            if (info.intSize != 0) {
                calls << "#endif\n";
            }
        }
    }
    file << "\n";

    // Modify the style of kernel as required by the API level.
    if (apiLevel >= 23) {
        file << "void RS_KERNEL test(int in, rs_kernel_context context) {\n";
    } else if (apiLevel >= 17) {
        file << "void RS_KERNEL test(int in) {\n";
    } else {
        file << "void root(const int* in) {\n";
    }
    file << calls.str();
    file << "}\n";

    return true;
}

bool generateStubsWhiteList(const string& slangTestDirectory, unsigned int maxApiLevel) {
    unsigned int lastApiLevel = min(systemSpecification.getMaximumApiLevel(), maxApiLevel);
    if (!generateWhiteListFile(lastApiLevel)) {
        return false;
    }
    // Generate a test file for each apiLevel.
    for (unsigned int i = kMinimumApiLevelForTests; i <= lastApiLevel; ++i) {
        if (!generateApiTesterFile(slangTestDirectory, i)) {
            return false;
        }
    }
    return true;
}
