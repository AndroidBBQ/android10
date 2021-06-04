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

// API level when RenderScript was added.
const unsigned int MIN_API_LEVEL = 9;

const NumericalType TYPES[] = {
            {"f16", "FLOAT_16", "half", "short", FLOATING_POINT, 11, 5},
            {"f32", "FLOAT_32", "float", "float", FLOATING_POINT, 24, 8},
            {"f64", "FLOAT_64", "double", "double", FLOATING_POINT, 53, 11},
            {"i8", "SIGNED_8", "char", "byte", SIGNED_INTEGER, 7, 0},
            {"u8", "UNSIGNED_8", "uchar", "byte", UNSIGNED_INTEGER, 8, 0},
            {"i16", "SIGNED_16", "short", "short", SIGNED_INTEGER, 15, 0},
            {"u16", "UNSIGNED_16", "ushort", "short", UNSIGNED_INTEGER, 16, 0},
            {"i32", "SIGNED_32", "int", "int", SIGNED_INTEGER, 31, 0},
            {"u32", "UNSIGNED_32", "uint", "int", UNSIGNED_INTEGER, 32, 0},
            {"i64", "SIGNED_64", "long", "long", SIGNED_INTEGER, 63, 0},
            {"u64", "UNSIGNED_64", "ulong", "long", UNSIGNED_INTEGER, 64, 0},
};

const int NUM_TYPES = sizeof(TYPES) / sizeof(TYPES[0]);

static const char kTagUnreleased[] = "UNRELEASED";

// Patterns that get substituted with C type or RS Data type names in function
// names, arguments, return types, and inlines.
static const string kCTypePatterns[] = {"#1", "#2", "#3", "#4"};
static const string kRSTypePatterns[] = {"#RST_1", "#RST_2", "#RST_3", "#RST_4"};

// The singleton of the collected information of all the spec files.
SystemSpecification systemSpecification;

// Returns the index in TYPES for the provided cType
static int findCType(const string& cType) {
    for (int i = 0; i < NUM_TYPES; i++) {
        if (cType == TYPES[i].cType) {
            return i;
        }
    }
    return -1;
}

/* Converts a string like "u8, u16" to a vector of "ushort", "uint".
 * For non-numerical types, we don't need to convert the abbreviation.
 */
static vector<string> convertToTypeVector(const string& input) {
    // First convert the string to an array of strings.
    vector<string> entries;
    stringstream stream(input);
    string entry;
    while (getline(stream, entry, ',')) {
        trimSpaces(&entry);
        entries.push_back(entry);
    }

    /* Second, we look for present numerical types. We do it this way
     * so the order of numerical types is always the same, no matter
     * how specified in the spec file.
     */
    vector<string> result;
    for (auto t : TYPES) {
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            if (*i == t.specType) {
                result.push_back(t.cType);
                entries.erase(i);
                break;
            }
        }
    }

    // Add the remaining; they are not numerical types.
    for (auto s : entries) {
        result.push_back(s);
    }

    return result;
}

// Returns true if each entry in typeVector is an RS numerical type
static bool isRSTValid(const vector<string> &typeVector) {
    for (auto type: typeVector) {
        if (findCType(type) == -1)
            return false;
    }
    return true;
}

void getVectorSizeAndBaseType(const string& type, string& vectorSize, string& baseType) {
    vectorSize = "1";
    baseType = type;

    /* If it's a vector type, we need to split the base type from the size.
     * We know that's it's a vector type if the last character is a digit and
     * the rest is an actual base type.   We used to only verify the first part,
     * which created a problem with rs_matrix2x2.
     */
    const int last = type.size() - 1;
    const char lastChar = type[last];
    if (lastChar >= '0' && lastChar <= '9') {
        const string trimmed = type.substr(0, last);
        int i = findCType(trimmed);
        if (i >= 0) {
            baseType = trimmed;
            vectorSize = lastChar;
        }
    }
}

void ParameterDefinition::parseParameterDefinition(const string& type, const string& name,
                                                   const string& testOption, int lineNumber,
                                                   bool isReturn, Scanner* scanner) {
    rsType = type;
    specName = name;

    // Determine if this is an output.
    isOutParameter = isReturn || charRemoved('*', &rsType);

    getVectorSizeAndBaseType(rsType, mVectorSize, rsBaseType);
    typeIndex = findCType(rsBaseType);

    if (mVectorSize == "3") {
        vectorWidth = "4";
    } else {
        vectorWidth = mVectorSize;
    }

    /* Create variable names to be used in the java and .rs files.  Because x and
     * y are reserved in .rs files, we prefix variable names with "in" or "out".
     */
    if (isOutParameter) {
        variableName = "out";
        if (!specName.empty()) {
            variableName += capitalize(specName);
        } else if (!isReturn) {
            scanner->error(lineNumber) << "Should have a name.\n";
        }
        doubleVariableName = variableName + "Double";
    } else {
        variableName = "in";
        if (specName.empty()) {
            scanner->error(lineNumber) << "Should have a name.\n";
        }
        variableName += capitalize(specName);
        doubleVariableName = variableName + "Double";
    }
    rsAllocName = "gAlloc" + capitalize(variableName);
    javaAllocName = variableName;
    javaArrayName = "array" + capitalize(javaAllocName);

    // Process the option.
    undefinedIfOutIsNan = false;
    compatibleTypeIndex = -1;
    if (!testOption.empty()) {
        if (testOption.compare(0, 6, "range(") == 0) {
            size_t pComma = testOption.find(',');
            size_t pParen = testOption.find(')');
            if (pComma == string::npos || pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect range " << testOption << "\n";
            } else {
                minValue = testOption.substr(6, pComma - 6);
                maxValue = testOption.substr(pComma + 1, pParen - pComma - 1);
            }
        } else if (testOption.compare(0, 6, "above(") == 0) {
            size_t pParen = testOption.find(')');
            if (pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect testOption " << testOption << "\n";
            } else {
                smallerParameter = testOption.substr(6, pParen - 6);
            }
        } else if (testOption.compare(0, 11, "compatible(") == 0) {
            size_t pParen = testOption.find(')');
            if (pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect testOption " << testOption << "\n";
            } else {
                compatibleTypeIndex = findCType(testOption.substr(11, pParen - 11));
            }
        } else if (testOption.compare(0, 11, "conditional") == 0) {
            undefinedIfOutIsNan = true;
        } else {
            scanner->error(lineNumber) << "Unrecognized testOption " << testOption << "\n";
        }
    }

    isFloatType = false;
    if (typeIndex >= 0) {
        javaBaseType = TYPES[typeIndex].javaType;
        specType = TYPES[typeIndex].specType;
        isFloatType = TYPES[typeIndex].exponentBits > 0;
    }
    if (!minValue.empty()) {
        if (typeIndex < 0 || TYPES[typeIndex].kind != FLOATING_POINT) {
            scanner->error(lineNumber) << "range(,) is only supported for floating point\n";
        }
    }
}

bool VersionInfo::scan(Scanner* scanner, unsigned int maxApiLevel) {
    if (scanner->findOptionalTag("version:")) {
        const string s = scanner->getValue();
        if (s.compare(0, sizeof(kTagUnreleased), kTagUnreleased) == 0) {
            // The API is still under development and does not have
            // an official version number.
            minVersion = maxVersion = kUnreleasedVersion;
        } else {
            sscanf(s.c_str(), "%u %u", &minVersion, &maxVersion);
            if (minVersion && minVersion < MIN_API_LEVEL) {
                scanner->error() << "Minimum version must >= 9\n";
            }
            if (minVersion == MIN_API_LEVEL) {
                minVersion = 0;
            }
            if (maxVersion && maxVersion < MIN_API_LEVEL) {
                scanner->error() << "Maximum version must >= 9\n";
            }
        }
    }
    if (scanner->findOptionalTag("size:")) {
        sscanf(scanner->getValue().c_str(), "%i", &intSize);
    }

    if (maxVersion > maxApiLevel) {
        maxVersion = maxApiLevel;
    }

    return minVersion == 0 || minVersion <= maxApiLevel;
}

Definition::Definition(const std::string& name)
    : mName(name), mDeprecatedApiLevel(0), mHidden(false), mFinalVersion(-1) {
}

void Definition::updateFinalVersion(const VersionInfo& info) {
    /* We set it if:
     * - We have never set mFinalVersion before, or
     * - The max version is 0, which means we have not expired this API, or
     * - We have a max that's later than what we currently have.
     */
    if (mFinalVersion < 0 || info.maxVersion == 0 ||
        (mFinalVersion > 0 &&
         static_cast<int>(info.maxVersion) > mFinalVersion)) {
        mFinalVersion = info.maxVersion;
    }
}

void Definition::scanDocumentationTags(Scanner* scanner, bool firstOccurence,
                                       const SpecFile* specFile) {
    if (scanner->findOptionalTag("hidden:")) {
        scanner->checkNoValue();
        mHidden = true;
    }
    if (scanner->findOptionalTag("deprecated:")) {
        string value = scanner->getValue();
        size_t pComma = value.find(", ");
        if (pComma != string::npos) {
            mDeprecatedMessage = value.substr(pComma + 2);
            value.erase(pComma);
        }
        sscanf(value.c_str(), "%i", &mDeprecatedApiLevel);
        if (mDeprecatedApiLevel <= 0) {
            scanner->error() << "deprecated entries should have a level > 0\n";
        }
    }
    if (firstOccurence) {
        if (scanner->findTag("summary:")) {
            mSummary = scanner->getValue();
        }
        if (scanner->findTag("description:")) {
            scanner->checkNoValue();
            while (scanner->findOptionalTag("")) {
                mDescription.push_back(scanner->getValue());
            }
        }
        mUrl = specFile->getDetailedDocumentationUrl() + "#android_rs:" + mName;
    } else if (scanner->findOptionalTag("summary:")) {
        scanner->error() << "Only the first specification should have a summary.\n";
    }
}

Constant::~Constant() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

Type::~Type() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

Function::Function(const string& name) : Definition(name) {
    mCapitalizedName = capitalize(mName);
}

Function::~Function() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

bool Function::someParametersAreDocumented() const {
    for (auto p : mParameters) {
        if (!p->documentation.empty()) {
            return true;
        }
    }
    return false;
}

void Function::addParameter(ParameterEntry* entry, Scanner* scanner) {
    for (auto i : mParameters) {
        if (i->name == entry->name) {
            // It's a duplicate.
            if (!entry->documentation.empty()) {
                scanner->error(entry->lineNumber)
                            << "Only the first occurence of an arg should have the "
                               "documentation.\n";
            }
            return;
        }
    }
    mParameters.push_back(entry);
}

void Function::addReturn(ParameterEntry* entry, Scanner* scanner) {
    if (entry->documentation.empty()) {
        return;
    }
    if (!mReturnDocumentation.empty()) {
        scanner->error() << "ret: should be documented only for the first variant\n";
    }
    mReturnDocumentation = entry->documentation;
}

void ConstantSpecification::scanConstantSpecification(Scanner* scanner, SpecFile* specFile,
                                                      unsigned int maxApiLevel) {
    string name = scanner->getValue();
    VersionInfo info;
    if (!info.scan(scanner, maxApiLevel)) {
        cout << "Skipping some " << name << " definitions.\n";
        scanner->skipUntilTag("end:");
        return;
    }

    bool created = false;
    Constant* constant = systemSpecification.findOrCreateConstant(name, &created);
    ConstantSpecification* spec = new ConstantSpecification(constant);
    constant->addSpecification(spec);
    constant->updateFinalVersion(info);
    specFile->addConstantSpecification(spec, created);
    spec->mVersionInfo = info;

    if (scanner->findTag("value:")) {
        spec->mValue = scanner->getValue();
    }
    if (scanner->findTag("type:")) {
        spec->mType = scanner->getValue();
    }
    constant->scanDocumentationTags(scanner, created, specFile);

    scanner->findTag("end:");
}

void TypeSpecification::scanTypeSpecification(Scanner* scanner, SpecFile* specFile,
                                              unsigned int maxApiLevel) {
    string name = scanner->getValue();
    VersionInfo info;
    if (!info.scan(scanner, maxApiLevel)) {
        cout << "Skipping some " << name << " definitions.\n";
        scanner->skipUntilTag("end:");
        return;
    }

    bool created = false;
    Type* type = systemSpecification.findOrCreateType(name, &created);
    TypeSpecification* spec = new TypeSpecification(type);
    type->addSpecification(spec);
    type->updateFinalVersion(info);
    specFile->addTypeSpecification(spec, created);
    spec->mVersionInfo = info;

    if (scanner->findOptionalTag("simple:")) {
        spec->mKind = SIMPLE;
        spec->mSimpleType = scanner->getValue();
    }
    if (scanner->findOptionalTag("rs_object:")) {
        spec->mKind = RS_OBJECT;
    }
    if (scanner->findOptionalTag("struct:")) {
        spec->mKind = STRUCT;
        spec->mStructName = scanner->getValue();
        while (scanner->findOptionalTag("field:")) {
            string s = scanner->getValue();
            string comment;
            scanner->parseDocumentation(&s, &comment);
            spec->mFields.push_back(s);
            spec->mFieldComments.push_back(comment);
        }
    }
    if (scanner->findOptionalTag("enum:")) {
        spec->mKind = ENUM;
        spec->mEnumName = scanner->getValue();
        while (scanner->findOptionalTag("value:")) {
            string s = scanner->getValue();
            string comment;
            scanner->parseDocumentation(&s, &comment);
            spec->mValues.push_back(s);
            spec->mValueComments.push_back(comment);
        }
    }
    if (scanner->findOptionalTag("attrib:")) {
        spec->mAttribute = scanner->getValue();
    }
    type->scanDocumentationTags(scanner, created, specFile);

    scanner->findTag("end:");
}

FunctionSpecification::~FunctionSpecification() {
    for (auto i : mParameters) {
        delete i;
    }
    delete mReturn;
    for (auto i : mPermutations) {
        delete i;
    }
}

string FunctionSpecification::expandRSTypeInString(const string &s,
                                                   const string &pattern,
                                                   const string &cTypeStr) const {
    // Find index of numerical type corresponding to cTypeStr.  The case where
    // pattern is found in s but cTypeStr is not a numerical type is checked in
    // checkRSTPatternValidity.
    int typeIdx = findCType(cTypeStr);
    if (typeIdx == -1) {
        return s;
    }
    // If index exists, perform replacement.
    return stringReplace(s, pattern, TYPES[typeIdx].rsDataType);
}

string FunctionSpecification::expandString(string s,
                                           int replacementIndexes[MAX_REPLACEABLES]) const {


    for (unsigned idx = 0; idx < mReplaceables.size(); idx ++) {
        string toString = mReplaceables[idx][replacementIndexes[idx]];

        // replace #RST_i patterns with RS datatype corresponding to toString
        s = expandRSTypeInString(s, kRSTypePatterns[idx], toString);

        // replace #i patterns with C type from mReplaceables
        s = stringReplace(s, kCTypePatterns[idx], toString);
    }

    return s;
}

void FunctionSpecification::expandStringVector(const vector<string>& in,
                                               int replacementIndexes[MAX_REPLACEABLES],
                                               vector<string>* out) const {
    out->clear();
    for (vector<string>::const_iterator iter = in.begin(); iter != in.end(); iter++) {
        out->push_back(expandString(*iter, replacementIndexes));
    }
}

void FunctionSpecification::createPermutations(Function* function, Scanner* scanner) {
    int start[MAX_REPLACEABLES];
    int end[MAX_REPLACEABLES];
    for (int i = 0; i < MAX_REPLACEABLES; i++) {
        if (i < (int)mReplaceables.size()) {
            start[i] = 0;
            end[i] = mReplaceables[i].size();
        } else {
            start[i] = -1;
            end[i] = 0;
        }
    }
    int replacementIndexes[MAX_REPLACEABLES];
    // TODO: These loops assume that MAX_REPLACEABLES is 4.
    for (replacementIndexes[3] = start[3]; replacementIndexes[3] < end[3];
         replacementIndexes[3]++) {
        for (replacementIndexes[2] = start[2]; replacementIndexes[2] < end[2];
             replacementIndexes[2]++) {
            for (replacementIndexes[1] = start[1]; replacementIndexes[1] < end[1];
                 replacementIndexes[1]++) {
                for (replacementIndexes[0] = start[0]; replacementIndexes[0] < end[0];
                     replacementIndexes[0]++) {
                    auto p = new FunctionPermutation(function, this, replacementIndexes, scanner);
                    mPermutations.push_back(p);
                }
            }
        }
    }
}

string FunctionSpecification::getName(int replacementIndexes[MAX_REPLACEABLES]) const {
    return expandString(mUnexpandedName, replacementIndexes);
}

void FunctionSpecification::getReturn(int replacementIndexes[MAX_REPLACEABLES],
                                      std::string* retType, int* lineNumber) const {
    *retType = expandString(mReturn->type, replacementIndexes);
    *lineNumber = mReturn->lineNumber;
}

void FunctionSpecification::getParam(size_t index, int replacementIndexes[MAX_REPLACEABLES],
                                     std::string* type, std::string* name, std::string* testOption,
                                     int* lineNumber) const {
    ParameterEntry* p = mParameters[index];
    *type = expandString(p->type, replacementIndexes);
    *name = p->name;
    *testOption = expandString(p->testOption, replacementIndexes);
    *lineNumber = p->lineNumber;
}

void FunctionSpecification::getInlines(int replacementIndexes[MAX_REPLACEABLES],
                                       std::vector<std::string>* inlines) const {
    expandStringVector(mInline, replacementIndexes, inlines);
}

void FunctionSpecification::parseTest(Scanner* scanner) {
    const string value = scanner->getValue();
    if (value == "scalar" || value == "vector" || value == "noverify" || value == "custom" ||
        value == "none") {
        mTest = value;
    } else if (value.compare(0, 7, "limited") == 0) {
        mTest = "limited";
        if (value.compare(7, 1, "(") == 0) {
            size_t pParen = value.find(')');
            if (pParen == string::npos) {
                scanner->error() << "Incorrect test: \"" << value << "\"\n";
            } else {
                mPrecisionLimit = value.substr(8, pParen - 8);
            }
        }
    } else {
        scanner->error() << "Unrecognized test option: \"" << value << "\"\n";
    }
}

bool FunctionSpecification::hasTests(unsigned int versionOfTestFiles) const {
    if (mVersionInfo.maxVersion != 0 && mVersionInfo.maxVersion < versionOfTestFiles) {
        return false;
    }
    if (mTest == "none") {
        return false;
    }
    return true;
}

void FunctionSpecification::checkRSTPatternValidity(const string &inlineStr,  bool allow,
                                                    Scanner *scanner) {
    for (int i = 0; i < MAX_REPLACEABLES; i ++) {
        bool patternFound = inlineStr.find(kRSTypePatterns[i]) != string::npos;

        if (patternFound) {
            if (!allow) {
                scanner->error() << "RST_i pattern not allowed here\n";
            }
            else if (mIsRSTAllowed[i] == false) {
                scanner->error() << "Found pattern \"" << kRSTypePatterns[i]
                    << "\" in spec.  But some entry in the corresponding"
                    << " parameter list cannot be translated to an RS type\n";
            }
        }
    }
}

void FunctionSpecification::scanFunctionSpecification(Scanner* scanner, SpecFile* specFile,
                                                      unsigned int maxApiLevel) {
    // Some functions like convert have # part of the name.  Truncate at that point.
    const string& unexpandedName = scanner->getValue();
    string name = unexpandedName;
    size_t p = name.find('#');
    if (p != string::npos) {
        if (p > 0 && name[p - 1] == '_') {
            p--;
        }
        name.erase(p);
    }
    VersionInfo info;
    if (!info.scan(scanner, maxApiLevel)) {
        cout << "Skipping some " << name << " definitions.\n";
        scanner->skipUntilTag("end:");
        return;
    }

    bool created = false;
    Function* function = systemSpecification.findOrCreateFunction(name, &created);
    FunctionSpecification* spec = new FunctionSpecification(function);
    function->addSpecification(spec);
    function->updateFinalVersion(info);
    specFile->addFunctionSpecification(spec, created);

    spec->mUnexpandedName = unexpandedName;
    spec->mTest = "scalar";  // default
    spec->mVersionInfo = info;

    if (scanner->findOptionalTag("internal:")) {
        spec->mInternal = (scanner->getValue() == "true");
    }
    if (scanner->findOptionalTag("intrinsic:")) {
        spec->mIntrinsic = (scanner->getValue() == "true");
    }
    if (scanner->findOptionalTag("attrib:")) {
        spec->mAttribute = scanner->getValue();
    }
    if (scanner->findOptionalTag("w:")) {
        vector<string> t;
        if (scanner->getValue().find("1") != string::npos) {
            t.push_back("");
        }
        if (scanner->getValue().find("2") != string::npos) {
            t.push_back("2");
        }
        if (scanner->getValue().find("3") != string::npos) {
            t.push_back("3");
        }
        if (scanner->getValue().find("4") != string::npos) {
            t.push_back("4");
        }
        spec->mReplaceables.push_back(t);
        // RST_i pattern not applicable for width.
        spec->mIsRSTAllowed.push_back(false);
    }

    while (scanner->findOptionalTag("t:")) {
        spec->mReplaceables.push_back(convertToTypeVector(scanner->getValue()));
        spec->mIsRSTAllowed.push_back(isRSTValid(spec->mReplaceables.back()));
    }

    // Disallow RST_* pattern in function name
    // FIXME the line number for this error would be wrong
    spec->checkRSTPatternValidity(unexpandedName, false, scanner);

    if (scanner->findTag("ret:")) {
        ParameterEntry* p = scanner->parseArgString(true);
        function->addReturn(p, scanner);
        spec->mReturn = p;

        // Disallow RST_* pattern in return type
        spec->checkRSTPatternValidity(p->type, false, scanner);
    }
    while (scanner->findOptionalTag("arg:")) {
        ParameterEntry* p = scanner->parseArgString(false);
        function->addParameter(p, scanner);
        spec->mParameters.push_back(p);

        // Disallow RST_* pattern in parameter type or testOption
        spec->checkRSTPatternValidity(p->type, false, scanner);
        spec->checkRSTPatternValidity(p->testOption, false, scanner);
    }

    function->scanDocumentationTags(scanner, created, specFile);

    if (scanner->findOptionalTag("inline:")) {
        scanner->checkNoValue();
        while (scanner->findOptionalTag("")) {
            spec->mInline.push_back(scanner->getValue());

            // Allow RST_* pattern in inline definitions
            spec->checkRSTPatternValidity(spec->mInline.back(), true, scanner);
        }
    }
    if (scanner->findOptionalTag("test:")) {
        spec->parseTest(scanner);
    }

    scanner->findTag("end:");

    spec->createPermutations(function, scanner);
}

FunctionPermutation::FunctionPermutation(Function* func, FunctionSpecification* spec,
                                         int replacementIndexes[MAX_REPLACEABLES], Scanner* scanner)
    : mReturn(nullptr), mInputCount(0), mOutputCount(0) {
    // We expand the strings now to make capitalization easier.  The previous code preserved
    // the #n
    // markers just before emitting, which made capitalization difficult.
    mName = spec->getName(replacementIndexes);
    mNameTrunk = func->getName();
    mTest = spec->getTest();
    mPrecisionLimit = spec->getPrecisionLimit();
    spec->getInlines(replacementIndexes, &mInline);

    mHasFloatAnswers = false;
    for (size_t i = 0; i < spec->getNumberOfParams(); i++) {
        string type, name, testOption;
        int lineNumber = 0;
        spec->getParam(i, replacementIndexes, &type, &name, &testOption, &lineNumber);
        ParameterDefinition* def = new ParameterDefinition();
        def->parseParameterDefinition(type, name, testOption, lineNumber, false, scanner);
        if (def->isOutParameter) {
            mOutputCount++;
        } else {
            mInputCount++;
        }

        if (def->typeIndex < 0 && mTest != "none") {
            scanner->error(lineNumber)
                        << "Could not find " << def->rsBaseType
                        << " while generating automated tests.  Use test: none if not needed.\n";
        }
        if (def->isOutParameter && def->isFloatType) {
            mHasFloatAnswers = true;
        }
        mParams.push_back(def);
    }

    string retType;
    int lineNumber = 0;
    spec->getReturn(replacementIndexes, &retType, &lineNumber);
    if (!retType.empty()) {
        mReturn = new ParameterDefinition();
        mReturn->parseParameterDefinition(retType, "", "", lineNumber, true, scanner);
        if (mReturn->isFloatType) {
            mHasFloatAnswers = true;
        }
        mOutputCount++;
    }
}

FunctionPermutation::~FunctionPermutation() {
    for (auto i : mParams) {
        delete i;
    }
    delete mReturn;
}

SpecFile::SpecFile(const string& specFileName) : mSpecFileName(specFileName) {
    string core = mSpecFileName;
    // Remove .spec
    size_t l = core.length();
    const char SPEC[] = ".spec";
    const int SPEC_SIZE = sizeof(SPEC) - 1;
    const int start = l - SPEC_SIZE;
    if (start >= 0 && core.compare(start, SPEC_SIZE, SPEC) == 0) {
        core.erase(start);
    }

    // The header file name should have the same base but with a ".rsh" extension.
    mHeaderFileName = core + ".rsh";
    mDetailedDocumentationUrl = core + ".html";
}

void SpecFile::addConstantSpecification(ConstantSpecification* spec, bool hasDocumentation) {
    mConstantSpecificationsList.push_back(spec);
    if (hasDocumentation) {
        Constant* constant = spec->getConstant();
        mDocumentedConstants.insert(pair<string, Constant*>(constant->getName(), constant));
    }
}

void SpecFile::addTypeSpecification(TypeSpecification* spec, bool hasDocumentation) {
    mTypeSpecificationsList.push_back(spec);
    if (hasDocumentation) {
        Type* type = spec->getType();
        mDocumentedTypes.insert(pair<string, Type*>(type->getName(), type));
    }
}

void SpecFile::addFunctionSpecification(FunctionSpecification* spec, bool hasDocumentation) {
    mFunctionSpecificationsList.push_back(spec);
    if (hasDocumentation) {
        Function* function = spec->getFunction();
        mDocumentedFunctions.insert(pair<string, Function*>(function->getName(), function));
    }
}

// Read the specification, adding the definitions to the global functions map.
bool SpecFile::readSpecFile(unsigned int maxApiLevel) {
    FILE* specFile = fopen(mSpecFileName.c_str(), "rte");
    if (!specFile) {
        cerr << "Error opening input file: " << mSpecFileName << "\n";
        return false;
    }

    Scanner scanner(mSpecFileName, specFile);

    // Scan the header that should start the file.
    scanner.skipBlankEntries();
    if (scanner.findTag("header:")) {
        if (scanner.findTag("summary:")) {
            mBriefDescription = scanner.getValue();
        }
        if (scanner.findTag("description:")) {
            scanner.checkNoValue();
            while (scanner.findOptionalTag("")) {
                mFullDescription.push_back(scanner.getValue());
            }
        }
        if (scanner.findOptionalTag("include:")) {
            scanner.checkNoValue();
            while (scanner.findOptionalTag("")) {
                mVerbatimInclude.push_back(scanner.getValue());
            }
        }
        scanner.findTag("end:");
    }

    while (1) {
        scanner.skipBlankEntries();
        if (scanner.atEnd()) {
            break;
        }
        const string tag = scanner.getNextTag();
        if (tag == "function:") {
            FunctionSpecification::scanFunctionSpecification(&scanner, this, maxApiLevel);
        } else if (tag == "type:") {
            TypeSpecification::scanTypeSpecification(&scanner, this, maxApiLevel);
        } else if (tag == "constant:") {
            ConstantSpecification::scanConstantSpecification(&scanner, this, maxApiLevel);
        } else {
            scanner.error() << "Expected function:, type:, or constant:.  Found: " << tag << "\n";
            return false;
        }
    }

    fclose(specFile);
    return scanner.getErrorCount() == 0;
}

SystemSpecification::~SystemSpecification() {
    for (auto i : mConstants) {
        delete i.second;
    }
    for (auto i : mTypes) {
        delete i.second;
    }
    for (auto i : mFunctions) {
        delete i.second;
    }
    for (auto i : mSpecFiles) {
        delete i;
    }
}

// Returns the named entry in the map.  Creates it if it's not there.
template <class T>
T* findOrCreate(const string& name, map<string, T*>* map, bool* created) {
    auto iter = map->find(name);
    if (iter != map->end()) {
        *created = false;
        return iter->second;
    }
    *created = true;
    T* f = new T(name);
    map->insert(pair<string, T*>(name, f));
    return f;
}

Constant* SystemSpecification::findOrCreateConstant(const string& name, bool* created) {
    return findOrCreate<Constant>(name, &mConstants, created);
}

Type* SystemSpecification::findOrCreateType(const string& name, bool* created) {
    return findOrCreate<Type>(name, &mTypes, created);
}

Function* SystemSpecification::findOrCreateFunction(const string& name, bool* created) {
    return findOrCreate<Function>(name, &mFunctions, created);
}

bool SystemSpecification::readSpecFile(const string& fileName, unsigned int maxApiLevel) {
    SpecFile* spec = new SpecFile(fileName);
    if (!spec->readSpecFile(maxApiLevel)) {
        cerr << fileName << ": Failed to parse.\n";
        return false;
    }
    mSpecFiles.push_back(spec);
    return true;
}


static void updateMaxApiLevel(const VersionInfo& info, unsigned int* maxApiLevel) {
    if (info.minVersion == VersionInfo::kUnreleasedVersion) {
        // Ignore development API level in consideration of max API level.
        return;
    }
    *maxApiLevel = max(*maxApiLevel, max(info.minVersion, info.maxVersion));
}

unsigned int SystemSpecification::getMaximumApiLevel() {
    unsigned int maxApiLevel = 0;
    for (auto i : mConstants) {
        for (auto j: i.second->getSpecifications()) {
            updateMaxApiLevel(j->getVersionInfo(), &maxApiLevel);
        }
    }
    for (auto i : mTypes) {
        for (auto j: i.second->getSpecifications()) {
            updateMaxApiLevel(j->getVersionInfo(), &maxApiLevel);
        }
    }
    for (auto i : mFunctions) {
        for (auto j: i.second->getSpecifications()) {
            updateMaxApiLevel(j->getVersionInfo(), &maxApiLevel);
        }
    }
    return maxApiLevel;
}

bool SystemSpecification::generateFiles(unsigned int maxApiLevel) const {
    bool success = generateHeaderFiles("include") &&
                   generateDocumentation("docs") &&
                   generateTestFiles("test", maxApiLevel) &&
                   generateStubsWhiteList("slangtest", maxApiLevel);
    if (success) {
        cout << "Successfully processed " << mTypes.size() << " types, " << mConstants.size()
             << " constants, and " << mFunctions.size() << " functions.\n";
    }
    return success;
}

string SystemSpecification::getHtmlAnchor(const string& name) const {
    Definition* d = nullptr;
    auto c = mConstants.find(name);
    if (c != mConstants.end()) {
        d = c->second;
    } else {
        auto t = mTypes.find(name);
        if (t != mTypes.end()) {
            d = t->second;
        } else {
            auto f = mFunctions.find(name);
            if (f != mFunctions.end()) {
                d = f->second;
            } else {
                return string();
            }
        }
    }
    ostringstream stream;
    stream << "<a href='" << d->getUrl() << "'>" << name << "</a>";
    return stream.str();
}
