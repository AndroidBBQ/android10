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

#ifndef ANDROID_RS_API_GENERATOR_SPECIFICATION_H
#define ANDROID_RS_API_GENERATOR_SPECIFICATION_H

// See Generator.cpp for documentation of the .spec file format.

#include <climits>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include <vector>

class Constant;
class ConstantSpecification;
class Function;
class FunctionPermutation;
class FunctionSpecification;
class SpecFile;
class Specification;
class Scanner;
class SystemSpecification;
class Type;
class TypeSpecification;

enum NumberKind { SIGNED_INTEGER, UNSIGNED_INTEGER, FLOATING_POINT };

// Table of type equivalences.
struct NumericalType {
    const char* specType;    // Name found in the .spec file
    const char* rsDataType;  // RS data type
    const char* cType;       // Type in a C file
    const char* javaType;    // Type in a Java file
    NumberKind kind;
    /* For integers, number of bits of the number, excluding the sign bit.
     * For floats, number of implied bits of the mantissa.
     */
    int significantBits;
    // For floats, number of bits of the exponent.  0 for integer types.
    int exponentBits;
};

/* Corresponds to one parameter line in a .spec file.  These will be parsed when
 * we instantiate the FunctionPermutation(s) that correspond to one FunctionSpecification.
 */
struct ParameterEntry {
    std::string type;
    std::string name;
    /* Optional information on how to generate test values for this parameter.  Can be:
     * - range(low, high): Generates values between these two limits only.
     * - above(other_parameter): The values must be greater than those of the named parameter.
     *       Used for clamp.
     * - compatible(type): The values must also be fully representable in the specified type.
     * - conditional: Don't verify this value the function return NaN.
     */
    std::string testOption;
    std::string documentation;
    int lineNumber;
};

/* Information about a parameter to a function.  The values of all the fields should only be set by
 * parseParameterDefinition.
 */
struct ParameterDefinition {
    std::string rsType;        // The Renderscript type, e.g. "uint3"
    std::string rsBaseType;    // As above but without the number, e.g. "uint"
    std::string javaBaseType;  // The type we need to declare in Java, e.g. "unsigned int"
    std::string specType;      // The type found in the spec, e.g. "f16"
    bool isFloatType;          // True if it's a floating point value
    /* The number of entries in the vector.  It should be either "1", "2", "3", or "4".  It's also
     * "1" for scalars.
     */
    std::string mVectorSize;
    /* The space the vector takes in an array.  It's the same as the vector size, except for size
     * "3", where the width is "4".
     */
    std::string vectorWidth;

    std::string specName;       // e.g. x, as found in the spec file
    std::string variableName;   // e.g. inX, used both in .rs and .java
    std::string rsAllocName;    // e.g. gAllocInX
    std::string javaAllocName;  // e.g. inX
    std::string javaArrayName;  // e.g. arrayInX
    std::string doubleVariableName; // e.g. inXDouble, used in .java for storing Float16 parameters
                                    // in double.

    // If non empty, the mininum and maximum values to be used when generating the test data.
    std::string minValue;
    std::string maxValue;
    /* If non empty, contains the name of another parameter that should be smaller or equal to this
     * parameter, i.e.  value(smallerParameter) <= value(this).  This is used when testing clamp.
     */
    std::string smallerParameter;

    bool isOutParameter;       // True if this parameter returns data from the script.
    bool undefinedIfOutIsNan;  // If true, we don't validate if 'out' is NaN.

    int typeIndex;            // Index in the TYPES array. Negative if not found in the array.
    int compatibleTypeIndex;  // Index in TYPES for which the test data must also fit.

    /* Fill this object from the type, name, and testOption.
     * isReturn is true if we're processing the "return:"
     */
    void parseParameterDefinition(const std::string& type, const std::string& name,
                                  const std::string& testOption, int lineNumber, bool isReturn,
                                  Scanner* scanner);

    bool isFloat16Parameter() const { return specType.compare("f16") == 0; }
};

struct VersionInfo {
    /* The range of versions a specification applies to. Zero if there's no restriction,
     * so an API that became available at 12 and is still valid would have min:12 max:0.
     * If non zero, both versions should be at least 9, the API level that introduced
     * RenderScript.
     */
    unsigned int minVersion;
    unsigned int maxVersion;
    // Either 0, 32 or 64.  If 0, this definition is valid for both 32 and 64 bits.
    int intSize;

    VersionInfo() : minVersion(0), maxVersion(0), intSize(0) {}
    /* Scan the version info from the spec file.  maxApiLevel specifies the maximum level
     * we are interested in.  This may alter maxVersion.  This method returns false if the
     * minVersion is greater than the maxApiLevel.
     */
    bool scan(Scanner* scanner, unsigned int maxApiLevel);
    /* Return true if the target can be found whitin the range. */
    bool includesVersion(unsigned int target) const {
        return (minVersion == 0 || target >= minVersion) &&
               (maxVersion == 0 || target <= maxVersion);
    }

    static constexpr unsigned int kUnreleasedVersion = UINT_MAX;
};

// We have three type of definitions
class Definition {
protected:
    std::string mName;
    /* If greater than 0, this definition is deprecated.  It's the API level at which
     * we added the deprecation warning.
     */
    int mDeprecatedApiLevel;
    std::string mDeprecatedMessage;         // Optional specific warning if the API is deprecated
    bool mHidden;                           // True if it should not be documented
    std::string mSummary;                   // A one-line description
    std::vector<std::string> mDescription;  // The comments to be included in the header
    std::string mUrl;                       // The URL of the detailed documentation
    int mFinalVersion;  // API level at which this API was removed, 0 if API is still valid

public:
    Definition(const std::string& name);

    std::string getName() const { return mName; }
    bool deprecated() const { return mDeprecatedApiLevel > 0; }
    int getDeprecatedApiLevel() const { return mDeprecatedApiLevel; }
    std::string getDeprecatedMessage() const { return mDeprecatedMessage; }
    bool hidden() const { return mHidden; }
    std::string getSummary() const { return mSummary; }
    const std::vector<std::string>& getDescription() const { return mDescription; }
    std::string getUrl() const { return mUrl; }
    int getFinalVersion() const { return mFinalVersion; }

    void scanDocumentationTags(Scanner* scanner, bool firstOccurence, const SpecFile* specFile);
    // Keep track of the final version of this API, if any.
    void updateFinalVersion(const VersionInfo& info);
};

/* Represents a constant, like M_PI.  This is a grouping of the version specific specifications.
 * We'll only have one instance of Constant for each name.
 */
class Constant : public Definition {
private:
    std::vector<ConstantSpecification*> mSpecifications;  // Owned

public:
    Constant(const std::string& name) : Definition(name) {}
    ~Constant();

    const std::vector<ConstantSpecification*> getSpecifications() const { return mSpecifications; }
    // This method should only be called by the scanning code.
    void addSpecification(ConstantSpecification* spec) { mSpecifications.push_back(spec); }
};

/* Represents a type, like "float4".  This is a grouping of the version specific specifications.
 * We'll only have one instance of Type for each name.
 */
class Type : public Definition {
private:
    std::vector<TypeSpecification*> mSpecifications;  // Owned

public:
    Type(const std::string& name) : Definition(name) {}
    ~Type();

    const std::vector<TypeSpecification*> getSpecifications() const { return mSpecifications; }
    // This method should only be called by the scanning code.
    void addSpecification(TypeSpecification* spec) { mSpecifications.push_back(spec); }
};

/* Represents a function, like "clamp".  Even though the spec file contains many entries for clamp,
 * we'll only have one clamp instance.
 */
class Function : public Definition {
private:
    // mName in the base class contains the lower case name, e.g. native_log
    std::string mCapitalizedName;  // The capitalized name, e.g. NativeLog

    // The unique parameters between all the specifications.  NOT OWNED.
    std::vector<ParameterEntry*> mParameters;
    std::string mReturnDocumentation;

    std::vector<FunctionSpecification*> mSpecifications;  // Owned

public:
    Function(const std::string& name);
    ~Function();

    std::string getCapitalizedName() const { return mCapitalizedName; }
    const std::vector<ParameterEntry*>& getParameters() const { return mParameters; }
    std::string getReturnDocumentation() const { return mReturnDocumentation; }
    const std::vector<FunctionSpecification*> getSpecifications() const { return mSpecifications; }

    bool someParametersAreDocumented() const;

    // The following methods should only be called by the scanning code.
    void addParameter(ParameterEntry* entry, Scanner* scanner);
    void addReturn(ParameterEntry* entry, Scanner* scanner);
    void addSpecification(FunctionSpecification* spec) { mSpecifications.push_back(spec); }
};

/* Base class for TypeSpecification, ConstantSpecification, and FunctionSpecification.
 * A specification can be specific to a range of RenderScript version or 32bits vs 64 bits.
 * This base class contains code to parse and store this version information.
 */
class Specification {
protected:
    VersionInfo mVersionInfo;
    void scanVersionInfo(Scanner* scanner);

public:
    VersionInfo getVersionInfo() const { return mVersionInfo; }
};

/* Defines one of the many variations of a constant.  There's a one to one correspondence between
 * ConstantSpecification objects and entries in the spec file.
 */
class ConstantSpecification : public Specification {
private:
    Constant* mConstant;  // Not owned

    std::string mValue;  // E.g. "3.1415"
    std::string mType;   // E.g. "float"
public:
    ConstantSpecification(Constant* constant) : mConstant(constant) {}

    Constant* getConstant() const { return mConstant; }
    std::string getValue() const { return mValue; }
    std::string getType() const { return mType; }

    // Parse a constant specification and add it to specFile.
    static void scanConstantSpecification(Scanner* scanner, SpecFile* specFile, unsigned int maxApiLevel);
};

enum TypeKind {
    SIMPLE,
    RS_OBJECT,
    STRUCT,
    ENUM,
};

/* Defines one of the many variations of a type.  There's a one to one correspondence between
 * TypeSpecification objects and entries in the spec file.
 */
class TypeSpecification : public Specification {
private:
    Type* mType;  // Not owned

    TypeKind mKind;  // The kind of type specification

    // If mKind is SIMPLE:
    std::string mSimpleType;  // The definition of the type

    // If mKind is STRUCT:
    std::string mStructName;                  // The name found after the struct keyword
    std::vector<std::string> mFields;         // One entry per struct field
    std::vector<std::string> mFieldComments;  // One entry per struct field
    std::string mAttribute;                   // Some structures may have attributes

    // If mKind is ENUM:
    std::string mEnumName;                    // The name found after the enum keyword
    std::vector<std::string> mValues;         // One entry per enum value
    std::vector<std::string> mValueComments;  // One entry per enum value
public:
    TypeSpecification(Type* type) : mType(type) {}

    Type* getType() const { return mType; }
    TypeKind getKind() const { return mKind; }
    std::string getSimpleType() const { return mSimpleType; }
    std::string getStructName() const { return mStructName; }
    const std::vector<std::string>& getFields() const { return mFields; }
    const std::vector<std::string>& getFieldComments() const { return mFieldComments; }
    std::string getAttribute() const { return mAttribute; }
    std::string getEnumName() const { return mEnumName; }
    const std::vector<std::string>& getValues() const { return mValues; }
    const std::vector<std::string>& getValueComments() const { return mValueComments; }

    // Parse a type specification and add it to specFile.
    static void scanTypeSpecification(Scanner* scanner, SpecFile* specFile, unsigned int maxApiLevel);
};

// Maximum number of placeholders (like #1, #2) in function specifications.
const int MAX_REPLACEABLES = 4;

/* Defines one of the many variations of the function.  There's a one to one correspondence between
 * FunctionSpecification objects and entries in the spec file.  Some of the strings that are parts
 * of a FunctionSpecification can include placeholders, which are "#1", "#2", "#3", and "#4".  We'll
 * replace these by values before generating the files.
 */
class FunctionSpecification : public Specification {
private:
    Function* mFunction;  // Not owned

    /* How to test.  One of:
     * "scalar": Generate test code that checks entries of each vector indepently.  E.g. for
     *           sin(float3), the test code will call the CoreMathVerfier.computeSin 3 times.
     * "limited": Like "scalar" but we don't generate extreme values.  This is not currently
     *            enabled as we were generating to many errors.
     * "custom": Like "scalar" but instead of calling CoreMathVerifier.computeXXX() to compute
     *           the expected value, we call instead CoreMathVerifier.verifyXXX().  This method
     *           returns a string that contains the error message, null if there's no error.
     * "vector": Generate test code that calls the CoreMathVerifier only once for each vector.
     *           This is useful for APIs like dot() or length().
     * "noverify": Generate test code that calls the API but don't verify the returned value.
     *             This can discover unresolved references.
     * "": Don't test.  This is the default.
     */
    std::string mTest;
    bool mInternal;               // Internal. Not visible to users. (Default: false)
    bool mIntrinsic;              // Compiler intrinsic that is lowered to an internal API.
                                  // (Default: false)
    std::string mAttribute;       // Function attributes.
    std::string mPrecisionLimit;  // Maximum precision required when checking output of this
                                  // function.

    // The vectors of values with which we'll replace #1, #2, ...
    std::vector<std::vector<std::string> > mReplaceables;

    // i-th entry is true if each entry in mReplaceables[i] has an equivalent
    // RS numerical type (i.e. present in TYPES global)
    std::vector<bool> mIsRSTAllowed;

    /* The collection of permutations for this specification, i.e. this class instantianted
     * for specific values of #1, #2, etc.  Owned.
     */
    std::vector<FunctionPermutation*> mPermutations;

    // The following fields may contain placeholders that will be replaced using the mReplaceables.

    /* As of this writing, convert_... is the only function with #1 in its name.
     * The related Function object contains the name of the function without #n, e.g. convert.
     * This is the name with the #, e.g. convert_#1_#2
     */
    std::string mUnexpandedName;
    ParameterEntry* mReturn;                   // The return type. The name should be empty.  Owned.
    std::vector<ParameterEntry*> mParameters;  // The parameters.  Owned.
    std::vector<std::string> mInline;          // The inline code to be included in the header

    /* Substitute the placeholders in the strings (e.g. #1, #2, ...) by the
     * corresponding entries in mReplaceables.  Substitute placeholders for RS
     * types (#RST_1, #RST_2, ...) by the RS Data type strings (UNSIGNED_8,
     * FLOAT_32 etc.) of the corresponding types in mReplaceables.
     * indexOfReplaceable1 selects with value to use for #1, same for 2, 3, and
     * 4.
     */
    std::string expandString(std::string s, int indexOfReplaceable[MAX_REPLACEABLES]) const;
    void expandStringVector(const std::vector<std::string>& in,
                            int replacementIndexes[MAX_REPLACEABLES],
                            std::vector<std::string>* out) const;

    // Helper function used by expandString to perform #RST_* substitution
    std::string expandRSTypeInString(const std::string &s,
                                     const std::string &pattern,
                                     const std::string &cTypeStr) const;

    // Fill the mPermutations field.
    void createPermutations(Function* function, Scanner* scanner);

public:
    FunctionSpecification(Function* function) : mFunction(function), mInternal(false),
        mIntrinsic(false), mReturn(nullptr) {}
    ~FunctionSpecification();

    Function* getFunction() const { return mFunction; }
    bool isInternal() const { return mInternal; }
    bool isIntrinsic() const { return mIntrinsic; }
    std::string getAttribute() const { return mAttribute; }
    std::string getTest() const { return mTest; }
    std::string getPrecisionLimit() const { return mPrecisionLimit; }

    const std::vector<FunctionPermutation*>& getPermutations() const { return mPermutations; }

    std::string getName(int replacementIndexes[MAX_REPLACEABLES]) const;
    void getReturn(int replacementIndexes[MAX_REPLACEABLES], std::string* retType,
                   int* lineNumber) const;
    size_t getNumberOfParams() const { return mParameters.size(); }
    void getParam(size_t index, int replacementIndexes[MAX_REPLACEABLES], std::string* type,
                  std::string* name, std::string* testOption, int* lineNumber) const;
    void getInlines(int replacementIndexes[MAX_REPLACEABLES],
                    std::vector<std::string>* inlines) const;

    // Parse the "test:" line.
    void parseTest(Scanner* scanner);

    // Return true if we need to generate tests for this function.
    bool hasTests(unsigned int versionOfTestFiles) const;

    bool hasInline() const { return mInline.size() > 0; }

    /* Return true if this function can be overloaded.  This is added by default to all
     * specifications, so except for the very few exceptions that start the attributes
     * with an '=' to avoid this, we'll return true.
     */
    bool isOverloadable() const {
        return mAttribute.empty() || mAttribute[0] != '=';
    }

    /* Check if RST_i is present in 's' and report an error if 'allow' is false
     * or the i-th replacement list is not a valid candidate for RST_i
     * replacement
     */
    void checkRSTPatternValidity(const std::string &s, bool allow, Scanner *scanner);

    // Parse a function specification and add it to specFile.
    static void scanFunctionSpecification(Scanner* scanner, SpecFile* specFile, unsigned int maxApiLevel);
};

/* A concrete version of a function specification, where all placeholders have been replaced by
 * actual values.
 */
class FunctionPermutation {
private:
    // These are the expanded version of those found on FunctionSpecification
    std::string mName;
    std::string mNameTrunk;  // The name without any expansion, e.g. convert
    std::string mTest;       // How to test.  One of "scalar", "vector", "noverify", "limited", and
                             // "none".
    std::string mPrecisionLimit;  // Maximum precision required when checking output of this
                                  // function.

    // The parameters of the function.  This does not include the return type.  Owned.
    std::vector<ParameterDefinition*> mParams;
    // The return type.  nullptr if a void function.  Owned.
    ParameterDefinition* mReturn;

    // The number of input and output parameters.  mOutputCount counts the return type.
    int mInputCount;
    int mOutputCount;

    // Whether one of the output parameters is a float.
    bool mHasFloatAnswers;

    // The inline code that implements this function.  Will be empty if not an inline.
    std::vector<std::string> mInline;

public:
    FunctionPermutation(Function* function, FunctionSpecification* specification,
                        int replacementIndexes[MAX_REPLACEABLES], Scanner* scanner);
    ~FunctionPermutation();

    std::string getName() const { return mName; }
    std::string getNameTrunk() const { return mNameTrunk; }
    std::string getTest() const { return mTest; }
    std::string getPrecisionLimit() const { return mPrecisionLimit; }

    const std::vector<std::string>& getInline() const { return mInline; }
    const ParameterDefinition* getReturn() const { return mReturn; }
    int getInputCount() const { return mInputCount; }
    int getOutputCount() const { return mOutputCount; }
    bool hasFloatAnswers() const { return mHasFloatAnswers; }

    const std::vector<ParameterDefinition*> getParams() const { return mParams; }
};

// An entire spec file and the methods to process it.
class SpecFile {
private:
    std::string mSpecFileName;
    std::string mHeaderFileName;
    std::string mDetailedDocumentationUrl;
    std::string mBriefDescription;
    std::vector<std::string> mFullDescription;
    // Text to insert as-is in the generated header.
    std::vector<std::string> mVerbatimInclude;

    /* The constants, types, and functions specifications declared in this
     *  file, in the order they are found in the file.  This matters for
     * header generation, as some types and inline functions depend
     * on each other.  Pointers not owned.
     */
    std::list<ConstantSpecification*> mConstantSpecificationsList;
    std::list<TypeSpecification*> mTypeSpecificationsList;
    std::list<FunctionSpecification*> mFunctionSpecificationsList;

    /* The constants, types, and functions that are documented in this file.
     * In very rare cases, specifications for an API are split across multiple
     * files, e.g. currently for ClearObject().  The documentation for
     * that function must be found in the first spec file encountered, so the
     * order of the files on the command line matters.
     */
    std::map<std::string, Constant*> mDocumentedConstants;
    std::map<std::string, Type*> mDocumentedTypes;
    std::map<std::string, Function*> mDocumentedFunctions;

public:
    explicit SpecFile(const std::string& specFileName);

    std::string getSpecFileName() const { return mSpecFileName; }
    std::string getHeaderFileName() const { return mHeaderFileName; }
    std::string getDetailedDocumentationUrl() const { return mDetailedDocumentationUrl; }
    const std::string getBriefDescription() const { return mBriefDescription; }
    const std::vector<std::string>& getFullDescription() const { return mFullDescription; }
    const std::vector<std::string>& getVerbatimInclude() const { return mVerbatimInclude; }

    const std::list<ConstantSpecification*>& getConstantSpecifications() const {
        return mConstantSpecificationsList;
    }
    const std::list<TypeSpecification*>& getTypeSpecifications() const {
        return mTypeSpecificationsList;
    }
    const std::list<FunctionSpecification*>& getFunctionSpecifications() const {
        return mFunctionSpecificationsList;
    }
    const std::map<std::string, Constant*>& getDocumentedConstants() const {
        return mDocumentedConstants;
    }
    const std::map<std::string, Type*>& getDocumentedTypes() const { return mDocumentedTypes; }
    const std::map<std::string, Function*>& getDocumentedFunctions() const {
        return mDocumentedFunctions;
    }

    bool hasSpecifications() const {
        return !mDocumentedConstants.empty() || !mDocumentedTypes.empty() ||
               !mDocumentedFunctions.empty();
    }

    bool readSpecFile(unsigned int maxApiLevel);

    /* These are called by the parser to keep track of the specifications defined in this file.
     * hasDocumentation is true if this specification containes the documentation.
     */
    void addConstantSpecification(ConstantSpecification* spec, bool hasDocumentation);
    void addTypeSpecification(TypeSpecification* spec, bool hasDocumentation);
    void addFunctionSpecification(FunctionSpecification* spec, bool hasDocumentation);
};

// The collection of all the spec files.
class SystemSpecification {
private:
    std::vector<SpecFile*> mSpecFiles;

    /* Entries in the table of contents.  We accumulate them in a map to sort them.
     * Pointers are owned.
     */
    std::map<std::string, Constant*> mConstants;
    std::map<std::string, Type*> mTypes;
    std::map<std::string, Function*> mFunctions;

public:
    ~SystemSpecification();

    /* These are called the parser to create unique instances per name.  Set *created to true
     * if the named specification did not already exist.
     */
    Constant* findOrCreateConstant(const std::string& name, bool* created);
    Type* findOrCreateType(const std::string& name, bool* created);
    Function* findOrCreateFunction(const std::string& name, bool* created);

    /* Parse the spec file and create the object hierarchy, adding a pointer to mSpecFiles.
     * We won't include information passed the specified level.
     */
    bool readSpecFile(const std::string& fileName, unsigned int maxApiLevel);
    // Generate all the files.
    bool generateFiles(unsigned int maxApiLevel) const;

    const std::vector<SpecFile*>& getSpecFiles() const { return mSpecFiles; }
    const std::map<std::string, Constant*>& getConstants() const { return mConstants; }
    const std::map<std::string, Type*>& getTypes() const { return mTypes; }
    const std::map<std::string, Function*>& getFunctions() const { return mFunctions; }

    // Returns "<a href='...'> for the named specification, or empty if not found.
    std::string getHtmlAnchor(const std::string& name) const;

    // Returns the maximum API level specified in any spec file.
    unsigned int getMaximumApiLevel();
};

// Singleton that represents the collection of all the specs we're processing.
extern SystemSpecification systemSpecification;

// Table of equivalences of numerical types.
extern const NumericalType TYPES[];
extern const int NUM_TYPES;

/* Given a renderscript type (string) calculate the vector size and base type. If the type
 * is not a vector the vector size is 1 and baseType is just the type itself.
 */
void getVectorSizeAndBaseType(const std::string& type, std::string& vectorSize,
                              std::string& baseType);

#endif  // ANDROID_RS_API_GENERATOR_SPECIFICATION_H
