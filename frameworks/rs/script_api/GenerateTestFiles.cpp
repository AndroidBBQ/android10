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

#include <iomanip>
#include <iostream>
#include <cmath>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

// Converts float2 to FLOAT_32 and 2, etc.
static void convertToRsType(const string& name, string* dataType, char* vectorSize) {
    string s = name;
    int last = s.size() - 1;
    char lastChar = s[last];
    if (lastChar >= '1' && lastChar <= '4') {
        s.erase(last);
        *vectorSize = lastChar;
    } else {
        *vectorSize = '1';
    }
    dataType->clear();
    for (int i = 0; i < NUM_TYPES; i++) {
        if (s == TYPES[i].cType) {
            *dataType = TYPES[i].rsDataType;
            break;
        }
    }
}

// Returns true if any permutation of the function have tests to b
static bool needTestFiles(const Function& function, unsigned int versionOfTestFiles) {
    for (auto spec : function.getSpecifications()) {
        if (spec->hasTests(versionOfTestFiles)) {
            return true;
        }
    }
    return false;
}

/* One instance of this class is generated for each permutation of a function for which
 * we are generating test code.  This instance will generate both the script and the Java
 * section of the test files for this permutation.  The class is mostly used to keep track
 * of the various names shared between script and Java files.
 * WARNING: Because the constructor keeps a reference to the FunctionPermutation, PermutationWriter
 * should not exceed the lifetime of FunctionPermutation.
 */
class PermutationWriter {
private:
    FunctionPermutation& mPermutation;

    string mRsKernelName;
    string mJavaArgumentsClassName;
    string mJavaArgumentsNClassName;
    string mJavaVerifierComputeMethodName;
    string mJavaVerifierVerifyMethodName;
    string mJavaCheckMethodName;
    string mJavaVerifyMethodName;

    // Pointer to the files we are generating.  Handy to avoid always passing them in the calls.
    GeneratedFile* mRs;
    GeneratedFile* mJava;

    /* Shortcuts to the return parameter and the first input parameter of the function
     * specification.
     */
    const ParameterDefinition* mReturnParam;      // Can be nullptr.  NOT OWNED.
    const ParameterDefinition* mFirstInputParam;  // Can be nullptr.  NOT OWNED.

    /* All the parameters plus the return param, if present.  Collecting them together
     * simplifies code generation.  NOT OWNED.
     */
    vector<const ParameterDefinition*> mAllInputsAndOutputs;

    /* We use a class to pass the arguments between the generated code and the CoreVerifier.  This
     * method generates this class.  The set keeps track if we've generated this class already
     * for this test file, as more than one permutation may use the same argument class.
     */
    void writeJavaArgumentClass(bool scalar, set<string>* javaGeneratedArgumentClasses) const;

    // Generate the Check* method that invokes the script and calls the verifier.
    void writeJavaCheckMethod(bool generateCallToVerifier) const;

    // Generate code to define and randomly initialize the input allocation.
    void writeJavaInputAllocationDefinition(const ParameterDefinition& param) const;

    /* Generate code that instantiate an allocation of floats or integers and fills it with
     * random data. This random data must be compatible with the specified type.  This is
     * used for the convert_* tests, as converting values that don't fit yield undefined results.
     */
    void writeJavaRandomCompatibleFloatAllocation(const string& dataType, const string& seed,
                                                  char vectorSize,
                                                  const NumericalType& compatibleType,
                                                  const NumericalType& generatedType) const;
    void writeJavaRandomCompatibleIntegerAllocation(const string& dataType, const string& seed,
                                                    char vectorSize,
                                                    const NumericalType& compatibleType,
                                                    const NumericalType& generatedType) const;

    // Generate code that defines an output allocation.
    void writeJavaOutputAllocationDefinition(const ParameterDefinition& param) const;

    /* Generate the code that verifies the results for RenderScript functions where each entry
     * of a vector is evaluated independently.  If verifierValidates is true, CoreMathVerifier
     * does the actual validation instead of more commonly returning the range of acceptable values.
     */
    void writeJavaVerifyScalarMethod(bool verifierValidates) const;

    /* Generate the code that verify the results for a RenderScript function where a vector
     * is a point in n-dimensional space.
     */
    void writeJavaVerifyVectorMethod() const;

    // Generate the line that creates the Target.
    void writeJavaCreateTarget() const;

    // Generate the method header of the verify function.
    void writeJavaVerifyMethodHeader() const;

    // Generate codes that copies the content of an allocation to an array.
    void writeJavaArrayInitialization(const ParameterDefinition& p) const;

    // Generate code that tests one value returned from the script.
    void writeJavaTestAndSetValid(const ParameterDefinition& p, const string& argsIndex,
                                  const string& actualIndex) const;
    void writeJavaTestOneValue(const ParameterDefinition& p, const string& argsIndex,
                               const string& actualIndex) const;
    // For test:vector cases, generate code that compares returned vector vs. expected value.
    void writeJavaVectorComparison(const ParameterDefinition& p) const;

    // Muliple functions that generates code to build the error message if an error is found.
    void writeJavaAppendOutputToMessage(const ParameterDefinition& p, const string& argsIndex,
                                        const string& actualIndex, bool verifierValidates) const;
    void writeJavaAppendInputToMessage(const ParameterDefinition& p, const string& actual) const;
    void writeJavaAppendNewLineToMessage() const;
    void writeJavaAppendVectorInputToMessage(const ParameterDefinition& p) const;
    void writeJavaAppendVectorOutputToMessage(const ParameterDefinition& p) const;

    // Generate the set of instructions to call the script.
    void writeJavaCallToRs(bool relaxed, bool generateCallToVerifier) const;

    // Write an allocation definition if not already emitted in the .rs file.
    void writeRsAllocationDefinition(const ParameterDefinition& param,
                                     set<string>* rsAllocationsGenerated) const;

public:
    /* NOTE: We keep pointers to the permutation and the files.  This object should not
     * outlive the arguments.
     */
    PermutationWriter(FunctionPermutation& permutation, GeneratedFile* rsFile,
                      GeneratedFile* javaFile);
    string getJavaCheckMethodName() const { return mJavaCheckMethodName; }

    // Write the script test function for this permutation.
    void writeRsSection(set<string>* rsAllocationsGenerated) const;
    // Write the section of the Java code that calls the script and validates the results
    void writeJavaSection(set<string>* javaGeneratedArgumentClasses) const;
};

PermutationWriter::PermutationWriter(FunctionPermutation& permutation, GeneratedFile* rsFile,
                                     GeneratedFile* javaFile)
    : mPermutation(permutation),
      mRs(rsFile),
      mJava(javaFile),
      mReturnParam(nullptr),
      mFirstInputParam(nullptr) {
    mRsKernelName = "test" + capitalize(permutation.getName());

    mJavaArgumentsClassName = "Arguments";
    mJavaArgumentsNClassName = "Arguments";
    const string trunk = capitalize(permutation.getNameTrunk());
    mJavaCheckMethodName = "check" + trunk;
    mJavaVerifyMethodName = "verifyResults" + trunk;

    for (auto p : permutation.getParams()) {
        mAllInputsAndOutputs.push_back(p);
        if (mFirstInputParam == nullptr && !p->isOutParameter) {
            mFirstInputParam = p;
        }
    }
    mReturnParam = permutation.getReturn();
    if (mReturnParam) {
        mAllInputsAndOutputs.push_back(mReturnParam);
    }

    for (auto p : mAllInputsAndOutputs) {
        const string capitalizedRsType = capitalize(p->rsType);
        const string capitalizedBaseType = capitalize(p->rsBaseType);
        mRsKernelName += capitalizedRsType;
        mJavaArgumentsClassName += capitalizedBaseType;
        mJavaArgumentsNClassName += capitalizedBaseType;
        if (p->mVectorSize != "1") {
            mJavaArgumentsNClassName += "N";
        }
        mJavaCheckMethodName += capitalizedRsType;
        mJavaVerifyMethodName += capitalizedRsType;
    }
    mJavaVerifierComputeMethodName = "compute" + trunk;
    mJavaVerifierVerifyMethodName = "verify" + trunk;
}

void PermutationWriter::writeJavaSection(set<string>* javaGeneratedArgumentClasses) const {
    // By default, we test the results using item by item comparison.
    const string test = mPermutation.getTest();
    if (test == "scalar" || test == "limited") {
        writeJavaArgumentClass(true, javaGeneratedArgumentClasses);
        writeJavaCheckMethod(true);
        writeJavaVerifyScalarMethod(false);
    } else if (test == "custom") {
        writeJavaArgumentClass(true, javaGeneratedArgumentClasses);
        writeJavaCheckMethod(true);
        writeJavaVerifyScalarMethod(true);
    } else if (test == "vector") {
        writeJavaArgumentClass(false, javaGeneratedArgumentClasses);
        writeJavaCheckMethod(true);
        writeJavaVerifyVectorMethod();
    } else if (test == "noverify") {
        writeJavaCheckMethod(false);
    }
}

void PermutationWriter::writeJavaArgumentClass(bool scalar,
                                               set<string>* javaGeneratedArgumentClasses) const {
    string name;
    if (scalar) {
        name = mJavaArgumentsClassName;
    } else {
        name = mJavaArgumentsNClassName;
    }

    // Make sure we have not generated the argument class already.
    if (!testAndSet(name, javaGeneratedArgumentClasses)) {
        mJava->indent() << "public class " << name;
        mJava->startBlock();

        for (auto p : mAllInputsAndOutputs) {
            bool isFieldArray = !scalar && p->mVectorSize != "1";
            bool isFloatyField = p->isOutParameter && p->isFloatType && mPermutation.getTest() != "custom";

            mJava->indent() << "public ";
            if (isFloatyField) {
                *mJava << "Target.Floaty";
            } else {
                *mJava << p->javaBaseType;
            }
            if (isFieldArray) {
                *mJava << "[]";
            }
            *mJava << " " << p->variableName << ";\n";

            // For Float16 parameters, add an extra 'double' field in the class
            // to hold the Double value converted from the input.
            if (p->isFloat16Parameter() && !isFloatyField) {
                mJava->indent() << "public double";
                if (isFieldArray) {
                    *mJava << "[]";
                }
                *mJava << " " + p->variableName << "Double;\n";
            }
        }
        mJava->endBlock();
        *mJava << "\n";
    }
}

void PermutationWriter::writeJavaCheckMethod(bool generateCallToVerifier) const {
    mJava->indent() << "private void " << mJavaCheckMethodName << "()";
    mJava->startBlock();

    // Generate the input allocations and initialization.
    for (auto p : mAllInputsAndOutputs) {
        if (!p->isOutParameter) {
            writeJavaInputAllocationDefinition(*p);
        }
    }
    // Generate code to enforce ordering between two allocations if needed.
    for (auto p : mAllInputsAndOutputs) {
        if (!p->isOutParameter && !p->smallerParameter.empty()) {
            string smallerAlloc = "in" + capitalize(p->smallerParameter);
            mJava->indent() << "enforceOrdering(" << smallerAlloc << ", " << p->javaAllocName
                            << ");\n";
        }
    }

    // Generate code to check the full and relaxed scripts.
    writeJavaCallToRs(false, generateCallToVerifier);
    writeJavaCallToRs(true, generateCallToVerifier);

    // Generate code to destroy input Allocations.
    for (auto p : mAllInputsAndOutputs) {
        if (!p->isOutParameter) {
            mJava->indent() << p->javaAllocName << ".destroy();\n";
        }
    }

    mJava->endBlock();
    *mJava << "\n";
}

void PermutationWriter::writeJavaInputAllocationDefinition(const ParameterDefinition& param) const {
    string dataType;
    char vectorSize;
    convertToRsType(param.rsType, &dataType, &vectorSize);

    const string seed = hashString(mJavaCheckMethodName + param.javaAllocName);
    mJava->indent() << "Allocation " << param.javaAllocName << " = ";
    if (param.compatibleTypeIndex >= 0) {
        if (TYPES[param.typeIndex].kind == FLOATING_POINT) {
            writeJavaRandomCompatibleFloatAllocation(dataType, seed, vectorSize,
                                                     TYPES[param.compatibleTypeIndex],
                                                     TYPES[param.typeIndex]);
        } else {
            writeJavaRandomCompatibleIntegerAllocation(dataType, seed, vectorSize,
                                                       TYPES[param.compatibleTypeIndex],
                                                       TYPES[param.typeIndex]);
        }
    } else if (!param.minValue.empty()) {
        *mJava << "createRandomFloatAllocation(mRS, Element.DataType." << dataType << ", "
               << vectorSize << ", " << seed << ", " << param.minValue << ", " << param.maxValue
               << ")";
    } else {
        /* TODO Instead of passing always false, check whether we are doing a limited test.
         * Use instead: (mPermutation.getTest() == "limited" ? "false" : "true")
         */
        *mJava << "createRandomAllocation(mRS, Element.DataType." << dataType << ", " << vectorSize
               << ", " << seed << ", false)";
    }
    *mJava << ";\n";
}

void PermutationWriter::writeJavaRandomCompatibleFloatAllocation(
            const string& dataType, const string& seed, char vectorSize,
            const NumericalType& compatibleType, const NumericalType& generatedType) const {
    *mJava << "createRandomFloatAllocation"
           << "(mRS, Element.DataType." << dataType << ", " << vectorSize << ", " << seed << ", ";
    double minValue = 0.0;
    double maxValue = 0.0;
    switch (compatibleType.kind) {
        case FLOATING_POINT: {
            // We're generating floating point values.  We just worry about the exponent.
            // Subtract 1 for the exponent sign.
            int bits = min(compatibleType.exponentBits, generatedType.exponentBits) - 1;
            maxValue = ldexp(0.95, (1 << bits) - 1);
            minValue = -maxValue;
            break;
        }
        case UNSIGNED_INTEGER:
            maxValue = maxDoubleForInteger(compatibleType.significantBits,
                                           generatedType.significantBits);
            minValue = 0.0;
            break;
        case SIGNED_INTEGER:
            maxValue = maxDoubleForInteger(compatibleType.significantBits,
                                           generatedType.significantBits);
            minValue = -maxValue - 1.0;
            break;
    }
    *mJava << scientific << std::setprecision(19);
    *mJava << minValue << ", " << maxValue << ")";
    mJava->unsetf(ios_base::floatfield);
}

void PermutationWriter::writeJavaRandomCompatibleIntegerAllocation(
            const string& dataType, const string& seed, char vectorSize,
            const NumericalType& compatibleType, const NumericalType& generatedType) const {
    *mJava << "createRandomIntegerAllocation"
           << "(mRS, Element.DataType." << dataType << ", " << vectorSize << ", " << seed << ", ";

    if (compatibleType.kind == FLOATING_POINT) {
        // Currently, all floating points can take any number we generate.
        bool isSigned = generatedType.kind == SIGNED_INTEGER;
        *mJava << (isSigned ? "true" : "false") << ", " << generatedType.significantBits;
    } else {
        bool isSigned =
                    compatibleType.kind == SIGNED_INTEGER && generatedType.kind == SIGNED_INTEGER;
        *mJava << (isSigned ? "true" : "false") << ", "
               << min(compatibleType.significantBits, generatedType.significantBits);
    }
    *mJava << ")";
}

void PermutationWriter::writeJavaOutputAllocationDefinition(
            const ParameterDefinition& param) const {
    string dataType;
    char vectorSize;
    convertToRsType(param.rsType, &dataType, &vectorSize);
    mJava->indent() << "Allocation " << param.javaAllocName << " = Allocation.createSized(mRS, "
                    << "getElement(mRS, Element.DataType." << dataType << ", " << vectorSize
                    << "), INPUTSIZE);\n";
}

void PermutationWriter::writeJavaVerifyScalarMethod(bool verifierValidates) const {
    writeJavaVerifyMethodHeader();
    mJava->startBlock();

    string vectorSize = "1";
    for (auto p : mAllInputsAndOutputs) {
        writeJavaArrayInitialization(*p);
        if (p->mVectorSize != "1" && p->mVectorSize != vectorSize) {
            if (vectorSize == "1") {
                vectorSize = p->mVectorSize;
            } else {
                cerr << "Error.  Had vector " << vectorSize << " and " << p->mVectorSize << "\n";
            }
        }
    }

    mJava->indent() << "StringBuilder message = new StringBuilder();\n";
    mJava->indent() << "boolean errorFound = false;\n";
    mJava->indent() << "for (int i = 0; i < INPUTSIZE; i++)";
    mJava->startBlock();

    mJava->indent() << "for (int j = 0; j < " << vectorSize << " ; j++)";
    mJava->startBlock();

    mJava->indent() << "// Extract the inputs.\n";
    mJava->indent() << mJavaArgumentsClassName << " args = new " << mJavaArgumentsClassName
                    << "();\n";
    for (auto p : mAllInputsAndOutputs) {
        if (!p->isOutParameter) {
            mJava->indent() << "args." << p->variableName << " = " << p->javaArrayName << "[i";
            if (p->vectorWidth != "1") {
                *mJava << " * " << p->vectorWidth << " + j";
            }
            *mJava << "];\n";

            // Convert the Float16 parameter to double and store it in the appropriate field in the
            // Arguments class.
            if (p->isFloat16Parameter()) {
                mJava->indent() << "args." << p->doubleVariableName
                                << " = Float16Utils.convertFloat16ToDouble(args."
                                << p->variableName << ");\n";
            }
        }
    }
    const bool hasFloat = mPermutation.hasFloatAnswers();
    if (verifierValidates) {
        mJava->indent() << "// Extract the outputs.\n";
        for (auto p : mAllInputsAndOutputs) {
            if (p->isOutParameter) {
                mJava->indent() << "args." << p->variableName << " = " << p->javaArrayName
                                << "[i * " << p->vectorWidth << " + j];\n";
                if (p->isFloat16Parameter()) {
                    mJava->indent() << "args." << p->doubleVariableName
                                    << " = Float16Utils.convertFloat16ToDouble(args."
                                    << p->variableName << ");\n";
                }
            }
        }
        mJava->indent() << "// Ask the CoreMathVerifier to validate.\n";
        if (hasFloat) {
            writeJavaCreateTarget();
        }
        mJava->indent() << "String errorMessage = CoreMathVerifier."
                        << mJavaVerifierVerifyMethodName << "(args";
        if (hasFloat) {
            *mJava << ", target";
        }
        *mJava << ");\n";
        mJava->indent() << "boolean valid = errorMessage == null;\n";
    } else {
        mJava->indent() << "// Figure out what the outputs should have been.\n";
        if (hasFloat) {
            writeJavaCreateTarget();
        }
        mJava->indent() << "CoreMathVerifier." << mJavaVerifierComputeMethodName << "(args";
        if (hasFloat) {
            *mJava << ", target";
        }
        *mJava << ");\n";
        mJava->indent() << "// Validate the outputs.\n";
        mJava->indent() << "boolean valid = true;\n";
        for (auto p : mAllInputsAndOutputs) {
            if (p->isOutParameter) {
                writeJavaTestAndSetValid(*p, "", "[i * " + p->vectorWidth + " + j]");
            }
        }
    }

    mJava->indent() << "if (!valid)";
    mJava->startBlock();
    mJava->indent() << "if (!errorFound)";
    mJava->startBlock();
    mJava->indent() << "errorFound = true;\n";

    for (auto p : mAllInputsAndOutputs) {
        if (p->isOutParameter) {
            writeJavaAppendOutputToMessage(*p, "", "[i * " + p->vectorWidth + " + j]",
                                           verifierValidates);
        } else {
            writeJavaAppendInputToMessage(*p, "args." + p->variableName);
        }
    }
    if (verifierValidates) {
        mJava->indent() << "message.append(errorMessage);\n";
    }
    mJava->indent() << "message.append(\"Errors at\");\n";
    mJava->endBlock();

    mJava->indent() << "message.append(\" [\");\n";
    mJava->indent() << "message.append(Integer.toString(i));\n";
    mJava->indent() << "message.append(\", \");\n";
    mJava->indent() << "message.append(Integer.toString(j));\n";
    mJava->indent() << "message.append(\"]\");\n";

    mJava->endBlock();
    mJava->endBlock();
    mJava->endBlock();

    mJava->indent() << "assertFalse(\"Incorrect output for " << mJavaCheckMethodName << "\" +\n";
    mJava->indentPlus()
                << "(relaxed ? \"_relaxed\" : \"\") + \":\\n\" + message.toString(), errorFound);\n";

    mJava->endBlock();
    *mJava << "\n";
}

void PermutationWriter::writeJavaVerifyVectorMethod() const {
    writeJavaVerifyMethodHeader();
    mJava->startBlock();

    for (auto p : mAllInputsAndOutputs) {
        writeJavaArrayInitialization(*p);
    }
    mJava->indent() << "StringBuilder message = new StringBuilder();\n";
    mJava->indent() << "boolean errorFound = false;\n";
    mJava->indent() << "for (int i = 0; i < INPUTSIZE; i++)";
    mJava->startBlock();

    mJava->indent() << mJavaArgumentsNClassName << " args = new " << mJavaArgumentsNClassName
                    << "();\n";

    mJava->indent() << "// Create the appropriate sized arrays in args\n";
    for (auto p : mAllInputsAndOutputs) {
        if (p->mVectorSize != "1") {
            string type = p->javaBaseType;
            if (p->isOutParameter && p->isFloatType) {
                type = "Target.Floaty";
            }
            mJava->indent() << "args." << p->variableName << " = new " << type << "["
                            << p->mVectorSize << "];\n";
            if (p->isFloat16Parameter() && !p->isOutParameter) {
                mJava->indent() << "args." << p->variableName << "Double = new double["
                                << p->mVectorSize << "];\n";
            }
        }
    }

    mJava->indent() << "// Fill args with the input values\n";
    for (auto p : mAllInputsAndOutputs) {
        if (!p->isOutParameter) {
            if (p->mVectorSize == "1") {
                mJava->indent() << "args." << p->variableName << " = " << p->javaArrayName << "[i]"
                                << ";\n";
                // Convert the Float16 parameter to double and store it in the appropriate field in
                // the Arguments class.
                if (p->isFloat16Parameter()) {
                    mJava->indent() << "args." << p->doubleVariableName << " = "
                                    << "Float16Utils.convertFloat16ToDouble(args."
                                    << p->variableName << ");\n";
                }
            } else {
                mJava->indent() << "for (int j = 0; j < " << p->mVectorSize << " ; j++)";
                mJava->startBlock();
                mJava->indent() << "args." << p->variableName << "[j] = "
                                << p->javaArrayName << "[i * " << p->vectorWidth << " + j]"
                                << ";\n";

                // Convert the Float16 parameter to double and store it in the appropriate field in
                // the Arguments class.
                if (p->isFloat16Parameter()) {
                    mJava->indent() << "args." << p->doubleVariableName << "[j] = "
                                    << "Float16Utils.convertFloat16ToDouble(args."
                                    << p->variableName << "[j]);\n";
                }
                mJava->endBlock();
            }
        }
    }
    writeJavaCreateTarget();
    mJava->indent() << "CoreMathVerifier." << mJavaVerifierComputeMethodName
                    << "(args, target);\n\n";

    mJava->indent() << "// Compare the expected outputs to the actual values returned by RS.\n";
    mJava->indent() << "boolean valid = true;\n";
    for (auto p : mAllInputsAndOutputs) {
        if (p->isOutParameter) {
            writeJavaVectorComparison(*p);
        }
    }

    mJava->indent() << "if (!valid)";
    mJava->startBlock();
    mJava->indent() << "if (!errorFound)";
    mJava->startBlock();
    mJava->indent() << "errorFound = true;\n";

    for (auto p : mAllInputsAndOutputs) {
        if (p->isOutParameter) {
            writeJavaAppendVectorOutputToMessage(*p);
        } else {
            writeJavaAppendVectorInputToMessage(*p);
        }
    }
    mJava->indent() << "message.append(\"Errors at\");\n";
    mJava->endBlock();

    mJava->indent() << "message.append(\" [\");\n";
    mJava->indent() << "message.append(Integer.toString(i));\n";
    mJava->indent() << "message.append(\"]\");\n";

    mJava->endBlock();
    mJava->endBlock();

    mJava->indent() << "assertFalse(\"Incorrect output for " << mJavaCheckMethodName << "\" +\n";
    mJava->indentPlus()
                << "(relaxed ? \"_relaxed\" : \"\") + \":\\n\" + message.toString(), errorFound);\n";

    mJava->endBlock();
    *mJava << "\n";
}


void PermutationWriter::writeJavaCreateTarget() const {
    string name = mPermutation.getName();

    const char* functionType = "NORMAL";
    size_t end = name.find('_');
    if (end != string::npos) {
        if (name.compare(0, end, "native") == 0) {
            functionType = "NATIVE";
        } else if (name.compare(0, end, "half") == 0) {
            functionType = "HALF";
        } else if (name.compare(0, end, "fast") == 0) {
            functionType = "FAST";
        }
    }

    string floatType = mReturnParam->specType;
    const char* precisionStr = "";
    if (floatType.compare("f16") == 0) {
        precisionStr = "HALF";
    } else if (floatType.compare("f32") == 0) {
        precisionStr = "FLOAT";
    } else if (floatType.compare("f64") == 0) {
        precisionStr = "DOUBLE";
    } else {
        cerr << "Error. Unreachable.  Return type is not floating point\n";
    }

    mJava->indent() << "Target target = new Target(Target.FunctionType." <<
                    functionType << ", Target.ReturnType." << precisionStr <<
                    ", relaxed);\n";
}

void PermutationWriter::writeJavaVerifyMethodHeader() const {
    mJava->indent() << "private void " << mJavaVerifyMethodName << "(";
    for (auto p : mAllInputsAndOutputs) {
        *mJava << "Allocation " << p->javaAllocName << ", ";
    }
    *mJava << "boolean relaxed)";
}

void PermutationWriter::writeJavaArrayInitialization(const ParameterDefinition& p) const {
    mJava->indent() << p.javaBaseType << "[] " << p.javaArrayName << " = new " << p.javaBaseType
                    << "[INPUTSIZE * " << p.vectorWidth << "];\n";

    /* For basic types, populate the array with values, to help understand failures.  We have had
     * bugs where the output buffer was all 0.  We were not sure if there was a failed copy or
     * the GPU driver was copying zeroes.
     */
    if (p.typeIndex >= 0) {
        mJava->indent() << "Arrays.fill(" << p.javaArrayName << ", (" << TYPES[p.typeIndex].javaType
                        << ") 42);\n";
    }

    mJava->indent() << p.javaAllocName << ".copyTo(" << p.javaArrayName << ");\n";
}

void PermutationWriter::writeJavaTestAndSetValid(const ParameterDefinition& p,
                                                 const string& argsIndex,
                                                 const string& actualIndex) const {
    writeJavaTestOneValue(p, argsIndex, actualIndex);
    mJava->startBlock();
    mJava->indent() << "valid = false;\n";
    mJava->endBlock();
}

void PermutationWriter::writeJavaTestOneValue(const ParameterDefinition& p, const string& argsIndex,
                                              const string& actualIndex) const {
    string actualOut;
    if (p.isFloat16Parameter()) {
        // For Float16 values, the output needs to be converted to Double.
        actualOut = "Float16Utils.convertFloat16ToDouble(" + p.javaArrayName + actualIndex + ")";
    } else {
        actualOut = p.javaArrayName + actualIndex;
    }

    mJava->indent() << "if (";
    if (p.isFloatType) {
        *mJava << "!args." << p.variableName << argsIndex << ".couldBe(" << actualOut;
        const string s = mPermutation.getPrecisionLimit();
        if (!s.empty()) {
            *mJava << ", " << s;
        }
        *mJava << ")";
    } else {
        *mJava << "args." << p.variableName << argsIndex << " != " << p.javaArrayName
               << actualIndex;
    }

    if (p.undefinedIfOutIsNan && mReturnParam) {
        *mJava << " && !args." << mReturnParam->variableName << argsIndex << ".isNaN()";
    }
    *mJava << ")";
}

void PermutationWriter::writeJavaVectorComparison(const ParameterDefinition& p) const {
    if (p.mVectorSize == "1") {
        writeJavaTestAndSetValid(p, "", "[i]");
    } else {
        mJava->indent() << "for (int j = 0; j < " << p.mVectorSize << " ; j++)";
        mJava->startBlock();
        writeJavaTestAndSetValid(p, "[j]", "[i * " + p.vectorWidth + " + j]");
        mJava->endBlock();
    }
}

void PermutationWriter::writeJavaAppendOutputToMessage(const ParameterDefinition& p,
                                                       const string& argsIndex,
                                                       const string& actualIndex,
                                                       bool verifierValidates) const {
    if (verifierValidates) {
        mJava->indent() << "message.append(\"Output " << p.variableName << ": \");\n";
        mJava->indent() << "appendVariableToMessage(message, args." << p.variableName << argsIndex
                        << ");\n";
        writeJavaAppendNewLineToMessage();
        if (p.isFloat16Parameter()) {
            writeJavaAppendNewLineToMessage();
            mJava->indent() << "message.append(\"Output " << p.variableName
                            << " (in double): \");\n";
            mJava->indent() << "appendVariableToMessage(message, args." << p.doubleVariableName
                            << ");\n";
            writeJavaAppendNewLineToMessage();
        }
    } else {
        mJava->indent() << "message.append(\"Expected output " << p.variableName << ": \");\n";
        mJava->indent() << "appendVariableToMessage(message, args." << p.variableName << argsIndex
                        << ");\n";
        writeJavaAppendNewLineToMessage();

        mJava->indent() << "message.append(\"Actual   output " << p.variableName << ": \");\n";
        mJava->indent() << "appendVariableToMessage(message, " << p.javaArrayName << actualIndex
                        << ");\n";

        if (p.isFloat16Parameter()) {
            writeJavaAppendNewLineToMessage();
            mJava->indent() << "message.append(\"Actual   output " << p.variableName
                            << " (in double): \");\n";
            mJava->indent() << "appendVariableToMessage(message, Float16Utils.convertFloat16ToDouble("
                            << p.javaArrayName << actualIndex << "));\n";
        }

        writeJavaTestOneValue(p, argsIndex, actualIndex);
        mJava->startBlock();
        mJava->indent() << "message.append(\" FAIL\");\n";
        mJava->endBlock();
        writeJavaAppendNewLineToMessage();
    }
}

void PermutationWriter::writeJavaAppendInputToMessage(const ParameterDefinition& p,
                                                      const string& actual) const {
    mJava->indent() << "message.append(\"Input " << p.variableName << ": \");\n";
    mJava->indent() << "appendVariableToMessage(message, " << actual << ");\n";
    writeJavaAppendNewLineToMessage();
}

void PermutationWriter::writeJavaAppendNewLineToMessage() const {
    mJava->indent() << "message.append(\"\\n\");\n";
}

void PermutationWriter::writeJavaAppendVectorInputToMessage(const ParameterDefinition& p) const {
    if (p.mVectorSize == "1") {
        writeJavaAppendInputToMessage(p, p.javaArrayName + "[i]");
    } else {
        mJava->indent() << "for (int j = 0; j < " << p.mVectorSize << " ; j++)";
        mJava->startBlock();
        writeJavaAppendInputToMessage(p, p.javaArrayName + "[i * " + p.vectorWidth + " + j]");
        mJava->endBlock();
    }
}

void PermutationWriter::writeJavaAppendVectorOutputToMessage(const ParameterDefinition& p) const {
    if (p.mVectorSize == "1") {
        writeJavaAppendOutputToMessage(p, "", "[i]", false);
    } else {
        mJava->indent() << "for (int j = 0; j < " << p.mVectorSize << " ; j++)";
        mJava->startBlock();
        writeJavaAppendOutputToMessage(p, "[j]", "[i * " + p.vectorWidth + " + j]", false);
        mJava->endBlock();
    }
}

void PermutationWriter::writeJavaCallToRs(bool relaxed, bool generateCallToVerifier) const {
    string script = "script";
    if (relaxed) {
        script += "Relaxed";
    }

    mJava->indent() << "try";
    mJava->startBlock();

    for (auto p : mAllInputsAndOutputs) {
        if (p->isOutParameter) {
            writeJavaOutputAllocationDefinition(*p);
        }
    }

    for (auto p : mPermutation.getParams()) {
        if (p != mFirstInputParam) {
            mJava->indent() << script << ".set_" << p->rsAllocName << "(" << p->javaAllocName
                            << ");\n";
        }
    }

    mJava->indent() << script << ".forEach_" << mRsKernelName << "(";
    bool needComma = false;
    if (mFirstInputParam) {
        *mJava << mFirstInputParam->javaAllocName;
        needComma = true;
    }
    if (mReturnParam) {
        if (needComma) {
            *mJava << ", ";
        }
        *mJava << mReturnParam->variableName << ");\n";
    }

    if (generateCallToVerifier) {
        mJava->indent() << mJavaVerifyMethodName << "(";
        for (auto p : mAllInputsAndOutputs) {
            *mJava << p->variableName << ", ";
        }

        if (relaxed) {
            *mJava << "true";
        } else {
            *mJava << "false";
        }
        *mJava << ");\n";
    }

    // Generate code to destroy output Allocations.
    for (auto p : mAllInputsAndOutputs) {
        if (p->isOutParameter) {
            mJava->indent() << p->javaAllocName << ".destroy();\n";
        }
    }

    mJava->decreaseIndent();
    mJava->indent() << "} catch (Exception e) {\n";
    mJava->increaseIndent();
    mJava->indent() << "throw new RSRuntimeException(\"RenderScript. Can't invoke forEach_"
                    << mRsKernelName << ": \" + e.toString());\n";
    mJava->endBlock();
}

/* Write the section of the .rs file for this permutation.
 *
 * We communicate the extra input and output parameters via global allocations.
 * For example, if we have a function that takes three arguments, two for input
 * and one for output:
 *
 * start:
 * name: gamn
 * ret: float3
 * arg: float3 a
 * arg: int b
 * arg: float3 *c
 * end:
 *
 * We'll produce:
 *
 * rs_allocation gAllocInB;
 * rs_allocation gAllocOutC;
 *
 * float3 __attribute__((kernel)) test_gamn_float3_int_float3(float3 inA, unsigned int x) {
 *    int inB;
 *    float3 outC;
 *    float2 out;
 *    inB = rsGetElementAt_int(gAllocInB, x);
 *    out = gamn(a, in_b, &outC);
 *    rsSetElementAt_float4(gAllocOutC, &outC, x);
 *    return out;
 * }
 *
 * We avoid re-using x and y from the definition because these have reserved
 * meanings in a .rs file.
 */
void PermutationWriter::writeRsSection(set<string>* rsAllocationsGenerated) const {
    // Write the allocation declarations we'll need.
    for (auto p : mPermutation.getParams()) {
        // Don't need allocation for one input and one return value.
        if (p != mFirstInputParam) {
            writeRsAllocationDefinition(*p, rsAllocationsGenerated);
        }
    }
    *mRs << "\n";

    // Write the function header.
    if (mReturnParam) {
        *mRs << mReturnParam->rsType;
    } else {
        *mRs << "void";
    }
    *mRs << " __attribute__((kernel)) " << mRsKernelName;
    *mRs << "(";
    bool needComma = false;
    if (mFirstInputParam) {
        *mRs << mFirstInputParam->rsType << " " << mFirstInputParam->variableName;
        needComma = true;
    }
    if (mPermutation.getOutputCount() > 1 || mPermutation.getInputCount() > 1) {
        if (needComma) {
            *mRs << ", ";
        }
        *mRs << "unsigned int x";
    }
    *mRs << ")";
    mRs->startBlock();

    // Write the local variable declarations and initializations.
    for (auto p : mPermutation.getParams()) {
        if (p == mFirstInputParam) {
            continue;
        }
        mRs->indent() << p->rsType << " " << p->variableName;
        if (p->isOutParameter) {
            *mRs << " = 0;\n";
        } else {
            *mRs << " = rsGetElementAt_" << p->rsType << "(" << p->rsAllocName << ", x);\n";
        }
    }

    // Write the function call.
    if (mReturnParam) {
        if (mPermutation.getOutputCount() > 1) {
            mRs->indent() << mReturnParam->rsType << " " << mReturnParam->variableName << " = ";
        } else {
            mRs->indent() << "return ";
        }
    }
    *mRs << mPermutation.getName() << "(";
    needComma = false;
    for (auto p : mPermutation.getParams()) {
        if (needComma) {
            *mRs << ", ";
        }
        if (p->isOutParameter) {
            *mRs << "&";
        }
        *mRs << p->variableName;
        needComma = true;
    }
    *mRs << ");\n";

    if (mPermutation.getOutputCount() > 1) {
        // Write setting the extra out parameters into the allocations.
        for (auto p : mPermutation.getParams()) {
            if (p->isOutParameter) {
                mRs->indent() << "rsSetElementAt_" << p->rsType << "(" << p->rsAllocName << ", ";
                // Check if we need to use '&' for this type of argument.
                char lastChar = p->variableName.back();
                if (lastChar >= '0' && lastChar <= '9') {
                    *mRs << "&";
                }
                *mRs << p->variableName << ", x);\n";
            }
        }
        if (mReturnParam) {
            mRs->indent() << "return " << mReturnParam->variableName << ";\n";
        }
    }
    mRs->endBlock();
}

void PermutationWriter::writeRsAllocationDefinition(const ParameterDefinition& param,
                                                    set<string>* rsAllocationsGenerated) const {
    if (!testAndSet(param.rsAllocName, rsAllocationsGenerated)) {
        *mRs << "rs_allocation " << param.rsAllocName << ";\n";
    }
}

// Open the mJavaFile and writes the header.
static bool startJavaFile(GeneratedFile* file, const string& directory,
                          const string& testName,
                          const string& relaxedTestName) {
    const string fileName = testName + ".java";
    if (!file->start(directory, fileName)) {
        return false;
    }
    file->writeNotices();

    *file << "package android.renderscript.cts;\n\n";

    *file << "import android.renderscript.Allocation;\n";
    *file << "import android.renderscript.RSRuntimeException;\n";
    *file << "import android.renderscript.Element;\n";
    *file << "import android.renderscript.cts.Target;\n\n";
    *file << "import java.util.Arrays;\n\n";

    *file << "public class " << testName << " extends RSBaseCompute";
    file->startBlock();  // The corresponding endBlock() is in finishJavaFile()
    *file << "\n";

    file->indent() << "private ScriptC_" << testName << " script;\n";
    file->indent() << "private ScriptC_" << relaxedTestName << " scriptRelaxed;\n\n";

    file->indent() << "@Override\n";
    file->indent() << "protected void setUp() throws Exception";
    file->startBlock();

    file->indent() << "super.setUp();\n";
    file->indent() << "script = new ScriptC_" << testName << "(mRS);\n";
    file->indent() << "scriptRelaxed = new ScriptC_" << relaxedTestName << "(mRS);\n";

    file->endBlock();
    *file << "\n";

    file->indent() << "@Override\n";
    file->indent() << "protected void tearDown() throws Exception";
    file->startBlock();

    file->indent() << "script.destroy();\n";
    file->indent() << "scriptRelaxed.destroy();\n";
    file->indent() << "super.tearDown();\n";

    file->endBlock();
    *file << "\n";

    return true;
}

// Write the test method that calls all the generated Check methods.
static void finishJavaFile(GeneratedFile* file, const Function& function,
                           const vector<string>& javaCheckMethods) {
    file->indent() << "public void test" << function.getCapitalizedName() << "()";
    file->startBlock();
    for (auto m : javaCheckMethods) {
        file->indent() << m << "();\n";
    }
    file->endBlock();

    file->endBlock();
}

// Open the script file and write its header.
static bool startRsFile(GeneratedFile* file, const string& directory,
                        const string& testName) {
    string fileName = testName + ".rs";
    if (!file->start(directory, fileName)) {
        return false;
    }
    file->writeNotices();

    *file << "#pragma version(1)\n";
    *file << "#pragma rs java_package_name(android.renderscript.cts)\n\n";
    return true;
}

// Write the entire *Relaxed.rs test file, as it only depends on the name.
static bool writeRelaxedRsFile(const string& directory, const string& testName,
                               const string& relaxedTestName) {
    string name = relaxedTestName + ".rs";

    GeneratedFile file;
    if (!file.start(directory, name)) {
        return false;
    }
    file.writeNotices();

    file << "#include \"" << testName << ".rs\"\n";
    file << "#pragma rs_fp_relaxed\n";
    file.close();
    return true;
}

/* Write the .java and the two .rs test files.  versionOfTestFiles is used to restrict which API
 * to test.
 */
static bool writeTestFilesForFunction(const Function& function, const string& directory,
                                      unsigned int versionOfTestFiles) {
    // Avoid creating empty files if we're not testing this function.
    if (!needTestFiles(function, versionOfTestFiles)) {
        return true;
    }

    const string testName = "Test" + function.getCapitalizedName();
    const string relaxedTestName = testName + "Relaxed";

    if (!writeRelaxedRsFile(directory, testName, relaxedTestName)) {
        return false;
    }

    GeneratedFile rsFile;    // The Renderscript test file we're generating.
    GeneratedFile javaFile;  // The Jave test file we're generating.
    if (!startRsFile(&rsFile, directory, testName)) {
        return false;
    }

    if (!startJavaFile(&javaFile, directory, testName, relaxedTestName)) {
        return false;
    }

    /* We keep track of the allocations generated in the .rs file and the argument classes defined
     * in the Java file, as we share these between the functions created for each specification.
     */
    set<string> rsAllocationsGenerated;
    set<string> javaGeneratedArgumentClasses;
    // Lines of Java code to invoke the check methods.
    vector<string> javaCheckMethods;

    for (auto spec : function.getSpecifications()) {
        if (spec->hasTests(versionOfTestFiles)) {
            for (auto permutation : spec->getPermutations()) {
                PermutationWriter w(*permutation, &rsFile, &javaFile);
                w.writeRsSection(&rsAllocationsGenerated);
                w.writeJavaSection(&javaGeneratedArgumentClasses);

                // Store the check method to be called.
                javaCheckMethods.push_back(w.getJavaCheckMethodName());
            }
        }
    }

    finishJavaFile(&javaFile, function, javaCheckMethods);
    // There's no work to wrap-up in the .rs file.

    rsFile.close();
    javaFile.close();
    return true;
}

bool generateTestFiles(const string& directory, unsigned int versionOfTestFiles) {
    bool success = true;
    for (auto f : systemSpecification.getFunctions()) {
        if (!writeTestFilesForFunction(*f.second, directory, versionOfTestFiles)) {
            success = false;
        }
    }
    return success;
}
