/*
 * Copyright 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <cstdarg>
#include <cctype>

#include <algorithm>
#include <sstream>
#include <string>

#include "os_sep.h"
#include "slang_rs_context.h"
#include "slang_rs_export_var.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_func.h"
#include "slang_rs_reflect_utils.h"
#include "slang_version.h"

#include "slang_rs_reflection_cpp.h"

using namespace std;

namespace slang {

const char kRsTypeItemClassName[] = "Item";
const char kRsElemPrefix[] = "__rs_elem_";
// The name of the Allocation type that is reflected in C++
const char kAllocationSp[] = "android::RSC::sp<android::RSC::Allocation>";

static const char *GetMatrixTypeName(const RSExportMatrixType *EMT) {
  static const char *MatrixTypeCNameMap[] = {
      "rs_matrix2x2", "rs_matrix3x3", "rs_matrix4x4",
  };
  unsigned Dim = EMT->getDim();

  if ((Dim - 2) < (sizeof(MatrixTypeCNameMap) / sizeof(const char *)))
    return MatrixTypeCNameMap[EMT->getDim() - 2];

  slangAssert(false && "GetMatrixTypeName : Unsupported matrix dimension");
  return nullptr;
}

static std::string GetTypeName(const RSExportType *ET, bool PreIdentifier = true) {
  if((!PreIdentifier) && (ET->getClass() != RSExportType::ExportClassConstantArray)) {
    slangAssert(false && "Non-array type post identifier?");
    return "";
  }
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    const RSExportPrimitiveType *EPT =
        static_cast<const RSExportPrimitiveType *>(ET);
    if (EPT->isRSObjectType()) {
      return std::string("android::RSC::sp<const android::RSC::") +
             RSExportPrimitiveType::getRSReflectionType(EPT)->c_name + ">";
    } else {
      return RSExportPrimitiveType::getRSReflectionType(EPT)->c_name;
    }
  }
  case RSExportType::ExportClassPointer: {
    const RSExportType *PointeeType =
        static_cast<const RSExportPointerType *>(ET)->getPointeeType();

    if (PointeeType->getClass() != RSExportType::ExportClassRecord)
      return kAllocationSp;
    else
      return PointeeType->getElementName();
  }
  case RSExportType::ExportClassVector: {
    const RSExportVectorType *EVT = static_cast<const RSExportVectorType *>(ET);
    std::stringstream VecName;
    VecName << EVT->getRSReflectionType(EVT)->rs_c_vector_prefix
            << EVT->getNumElement();
    return VecName.str();
  }
  case RSExportType::ExportClassMatrix: {
    return GetMatrixTypeName(static_cast<const RSExportMatrixType *>(ET));
  }
  case RSExportType::ExportClassConstantArray: {
    const RSExportConstantArrayType *CAT =
        static_cast<const RSExportConstantArrayType *>(ET);
    if (PreIdentifier) {
      std::string ElementTypeName = GetTypeName(CAT->getElementType());
      return ElementTypeName;
    }
    else {
      std::stringstream ArraySpec;
      ArraySpec << "[" << CAT->getNumElement() << "]";
      return ArraySpec.str();
    }
  }
  case RSExportType::ExportClassRecord: {
    // TODO: Fix for C structs!
    return ET->getElementName() + "." + kRsTypeItemClassName;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }

  return "";
}

RSReflectionCpp::RSReflectionCpp(const RSContext *Context,
                                 const string &OutputDirectory,
                                 const string &RSSourceFileName,
                                 const string &BitCodeFileName)
    : mRSContext(Context), mRSSourceFilePath(RSSourceFileName),
      mBitCodeFilePath(BitCodeFileName), mOutputDirectory(OutputDirectory),
      mNextExportVarSlot(0), mNextExportFuncSlot(0), mNextExportForEachSlot(0) {
  mCleanedRSFileName = RootNameFromRSFileName(mRSSourceFilePath);
  mClassName = "ScriptC_" + mCleanedRSFileName;
}

RSReflectionCpp::~RSReflectionCpp() {}

bool RSReflectionCpp::reflect() {
  writeHeaderFile();
  writeImplementationFile();

  return true;
}

#define RS_TYPE_CLASS_NAME_PREFIX "ScriptField_"

bool RSReflectionCpp::writeHeaderFile() {
  // Create the file and write the license note.
  if (!mOut.startFile(mOutputDirectory, mClassName + ".h", mRSSourceFilePath,
                      mRSContext->getLicenseNote(), false,
                      mRSContext->getVerbose())) {
    return false;
  }

  mOut.indent() << "#include \"RenderScript.h\"\n\n";
  // Add NOLINT to suppress clang-tidy warnings of "using namespace".
  // Keep "using namespace" to compile existing code.
  mOut.indent() << "using namespace android::RSC;  // NOLINT\n\n";

  mOut.comment("This class encapsulates access to the exported elements of the script.  "
               "Typically, you would instantiate this class once, call the set_* methods "
               "for each of the exported global variables you want to change, then call "
               "one of the forEach_ methods to invoke a kernel.");
  mOut.indent() << "class " << mClassName << " : public android::RSC::ScriptC";
  mOut.startBlock();

  mOut.decreaseIndent();
  mOut.indent() << "private:\n";
  mOut.increaseIndent();

  genFieldsToStoreExportVariableValues();
  genTypeInstancesUsedInForEach();
  genFieldsForAllocationTypeVerification();

  mOut.decreaseIndent();
  mOut.indent() << "public:\n";
  mOut.increaseIndent();

  // Generate the constructor and destructor declarations.
  mOut.indent() << mClassName << "(android::RSC::sp<android::RSC::RS> rs);\n";
  mOut.indent() << "virtual ~" << mClassName << "();\n\n";

  genExportVariablesGetterAndSetter();
  genForEachDeclarations();
  genExportFunctionDeclarations();

  mOut.endBlock(true);
  mOut.closeFile();
  return true;
}

void RSReflectionCpp::genTypeInstancesUsedInForEach() {
  for (auto I = mRSContext->export_foreach_begin(),
            E = mRSContext->export_foreach_end();
       I != E; I++) {
    const RSExportForEach *EF = *I;
    const RSExportType *OET = EF->getOutType();

    if (OET) {
      genTypeInstanceFromPointer(OET);
    }

    const RSExportForEach::InTypeVec &InTypes = EF->getInTypes();

    for (RSExportForEach::InTypeIter BI = InTypes.begin(),
         EI = InTypes.end(); BI != EI; BI++) {

      genTypeInstanceFromPointer(*BI);
    }
  }
}

void RSReflectionCpp::genFieldsForAllocationTypeVerification() {
  bool CommentAdded = false;
  for (std::set<std::string>::iterator I = mTypesToCheck.begin(),
                                       E = mTypesToCheck.end();
       I != E; I++) {
    if (!CommentAdded) {
      mOut.comment("The following elements are used to verify the types of "
                   "allocations passed to kernels.");
      CommentAdded = true;
    }
    mOut.indent() << "android::RSC::sp<const android::RSC::Element> "
                  << kRsElemPrefix << *I << ";\n";
  }
}

void RSReflectionCpp::genFieldsToStoreExportVariableValues() {
  bool CommentAdded = false;
  for (RSContext::const_export_var_iterator I = mRSContext->export_vars_begin(),
                                            E = mRSContext->export_vars_end();
       I != E; I++) {
    const RSExportVar *ev = *I;
    if (ev->isConst()) {
      continue;
    }
    if (!CommentAdded) {
      mOut.comment("For each non-const variable exported by the script, we "
                   "have an equivalent field.  This field contains the last "
                   "value this variable was set to using the set_ method.  "
                   "This may not be current value of the variable in the "
                   "script, as the script is free to modify its internal "
                   "variable without changing this field.  If the script "
                   "initializes the exported variable, the constructor will "
                   "initialize this field to the same value.");
      CommentAdded = true;
    }
    mOut.indent() << GetTypeName(ev->getType()) << " " RS_EXPORT_VAR_PREFIX
                  << ev->getName() << ";\n";
  }
}

void RSReflectionCpp::genForEachDeclarations() {
  bool CommentAdded = false;
  for (RSContext::const_export_foreach_iterator
           I = mRSContext->export_foreach_begin(),
           E = mRSContext->export_foreach_end();
       I != E; I++) {
    const RSExportForEach *ForEach = *I;

    if (ForEach->isDummyRoot()) {
      mOut.indent() << "// No forEach_root(...)\n";
      continue;
    }

    if (!CommentAdded) {
      mOut.comment("For each kernel of the script corresponds one method.  "
                   "That method queues the kernel for execution.  The kernel "
                   "may not have completed nor even started by the time this "
                   "function returns.  Calls that extract the data out of the "
                   "output allocation will wait for the kernels to complete.");
      CommentAdded = true;
    }

    std::string FunctionStart = "void forEach_" + ForEach->getName() + "(";
    mOut.indent() << FunctionStart;

    ArgumentList Arguments;
    const RSExportForEach::InVec &Ins = ForEach->getIns();
    for (RSExportForEach::InIter BI = Ins.begin(), EI = Ins.end();
         BI != EI; BI++) {

      Arguments.push_back(Argument(kAllocationSp, (*BI)->getName()));
    }

    if (ForEach->hasOut() || ForEach->hasReturn()) {
      Arguments.push_back(Argument(kAllocationSp, "aout"));
    }

    const RSExportRecordType *ERT = ForEach->getParamPacketType();
    if (ERT) {
      for (RSExportForEach::const_param_iterator i = ForEach->params_begin(),
                                                 e = ForEach->params_end();
           i != e; i++) {
        RSReflectionTypeData rtd;
        (*i)->getType()->convertToRTD(&rtd);
        Arguments.push_back(Argument(rtd.type->c_name, (*i)->getName()));
      }
    }
    genArguments(Arguments, FunctionStart.length());
    mOut << ");\n";
  }
}

void RSReflectionCpp::genExportFunctionDeclarations() {
  for (RSContext::const_export_func_iterator
           I = mRSContext->export_funcs_begin(),
           E = mRSContext->export_funcs_end();
       I != E; I++) {
    const RSExportFunc *ef = *I;

    makeFunctionSignature(false, ef);
  }
}

// forEach_* implementation
void RSReflectionCpp::genExportForEachBodies() {
  uint32_t slot = 0;
  for (auto I = mRSContext->export_foreach_begin(),
            E = mRSContext->export_foreach_end();
       I != E; I++, slot++) {
    const RSExportForEach *ef = *I;
    if (ef->isDummyRoot()) {
      mOut.indent() << "// No forEach_root(...)\n";
      continue;
    }

    ArgumentList Arguments;
    std::string FunctionStart =
        "void " + mClassName + "::forEach_" + ef->getName() + "(";
    mOut.indent() << FunctionStart;

    if (ef->hasIns()) {
      // FIXME: Add support for kernels with multiple inputs.
      slangAssert(ef->getIns().size() == 1);
      Arguments.push_back(Argument(kAllocationSp, "ain"));
    }

    if (ef->hasOut() || ef->hasReturn()) {
      Arguments.push_back(Argument(kAllocationSp, "aout"));
    }

    const RSExportRecordType *ERT = ef->getParamPacketType();
    if (ERT) {
      for (RSExportForEach::const_param_iterator i = ef->params_begin(),
                                                 e = ef->params_end();
           i != e; i++) {
        RSReflectionTypeData rtd;
        (*i)->getType()->convertToRTD(&rtd);
        Arguments.push_back(Argument(rtd.type->c_name, (*i)->getName()));
      }
    }
    genArguments(Arguments, FunctionStart.length());
    mOut << ")";
    mOut.startBlock();

    const RSExportType *OET = ef->getOutType();
    const RSExportForEach::InTypeVec &InTypes = ef->getInTypes();
    if (ef->hasIns()) {
      // FIXME: Add support for kernels with multiple inputs.
      slangAssert(ef->getIns().size() == 1);
      genTypeCheck(InTypes[0], "ain");
    }
    if (OET) {
      genTypeCheck(OET, "aout");
    }

    // TODO Add the appropriate dimension checking code, as seen in
    // slang_rs_reflection.cpp.

    std::string FieldPackerName = ef->getName() + "_fp";
    if (ERT) {
      if (genCreateFieldPacker(ERT, FieldPackerName.c_str())) {
        genPackVarOfType(ERT, nullptr, FieldPackerName.c_str());
      }
    }
    mOut.indent() << "forEach(" << slot << ", ";

    if (ef->hasIns()) {
      // FIXME: Add support for kernels with multiple inputs.
      slangAssert(ef->getIns().size() == 1);
      mOut << "ain, ";
    } else {
      mOut << "NULL, ";
    }

    if (ef->hasOut() || ef->hasReturn()) {
      mOut << "aout, ";
    } else {
      mOut << "NULL, ";
    }

    // FIXME (no support for usrData with C++ kernels)
    mOut << "NULL, 0);\n";
    mOut.endBlock();
  }
}

// invoke_* implementation
void RSReflectionCpp::genExportFunctionBodies() {
  uint32_t slot = 0;
  // Reflect export function
  for (auto I = mRSContext->export_funcs_begin(),
            E = mRSContext->export_funcs_end();
       I != E; I++) {
    const RSExportFunc *ef = *I;

    makeFunctionSignature(true, ef);
    mOut.startBlock();
    const RSExportRecordType *params = ef->getParamPacketType();
    size_t param_len = 0;
    if (params) {
      param_len = params->getAllocSize();
      if (genCreateFieldPacker(params, "__fp")) {
        genPackVarOfType(params, nullptr, "__fp");
      }
    }

    mOut.indent() << "invoke(" << slot;
    if (params) {
      mOut << ", __fp.getData(), " << param_len << ");\n";
    } else {
      mOut << ", NULL, 0);\n";
    }
    mOut.endBlock();

    slot++;
  }
}

bool RSReflectionCpp::genEncodedBitCode() {
  FILE *pfin = fopen(mBitCodeFilePath.c_str(), "rb");
  if (pfin == nullptr) {
    fprintf(stderr, "Error: could not read file %s\n",
            mBitCodeFilePath.c_str());
    return false;
  }

  unsigned char buf[16];
  int read_length;
  mOut.indent() << "static const unsigned char __txt[] =";
  mOut.startBlock();
  while ((read_length = fread(buf, 1, sizeof(buf), pfin)) > 0) {
    mOut.indent();
    for (int i = 0; i < read_length; i++) {
      char buf2[16];
      snprintf(buf2, sizeof(buf2), "0x%02x,", buf[i]);
      mOut << buf2;
    }
    mOut << "\n";
  }
  mOut.endBlock(true);
  mOut << "\n";
  return true;
}

bool RSReflectionCpp::writeImplementationFile() {
  if (!mOut.startFile(mOutputDirectory, mClassName + ".cpp", mRSSourceFilePath,
                      mRSContext->getLicenseNote(), false,
                      mRSContext->getVerbose())) {
    return false;
  }

  // Front matter
  mOut.indent() << "#include \"" << mClassName << ".h\"\n\n";

  genEncodedBitCode();
  mOut.indent() << "\n\n";

  // Constructor
  const std::string &packageName = mRSContext->getReflectJavaPackageName();
  mOut.indent() << mClassName << "::" << mClassName
                << "(android::RSC::sp<android::RSC::RS> rs):\n"
                   "        ScriptC(rs, __txt, sizeof(__txt), \""
                << mCleanedRSFileName << "\", " << mCleanedRSFileName.length()
                << ", \"/data/data/" << packageName << "/app\", sizeof(\""
                << packageName << "\"))";
  mOut.startBlock();
  for (std::set<std::string>::iterator I = mTypesToCheck.begin(),
                                       E = mTypesToCheck.end();
       I != E; I++) {
    mOut.indent() << kRsElemPrefix << *I << " = android::RSC::Element::" << *I
                  << "(mRS);\n";
  }

  for (RSContext::const_export_var_iterator I = mRSContext->export_vars_begin(),
                                            E = mRSContext->export_vars_end();
       I != E; I++) {
    const RSExportVar *EV = *I;
    if (!EV->getInit().isUninit()) {
      genInitExportVariable(EV->getType(), EV->getName(), EV->getInit());
    } else {
      genZeroInitExportVariable(EV->getName());
    }
  }
  mOut.endBlock();

  // Destructor
  mOut.indent() << mClassName << "::~" << mClassName << "()";
  mOut.startBlock();
  mOut.endBlock();

  // Function bodies
  genExportForEachBodies();
  genExportFunctionBodies();

  mOut.closeFile();
  return true;
}

void RSReflectionCpp::genExportVariablesGetterAndSetter() {
  mOut.comment("Methods to set and get the variables exported by the script. "
               "Const variables will not have a setter.\n\n"
               "Note that the value returned by the getter may not be the "
               "current value of the variable in the script.  The getter will "
               "return the initial value of the variable (as defined in the "
               "script) or the the last value set by using the setter method.  "
               "The script is free to modify its value independently.");
  for (RSContext::const_export_var_iterator I = mRSContext->export_vars_begin(),
                                            E = mRSContext->export_vars_end();
       I != E; I++) {
    const RSExportVar *EV = *I;
    const RSExportType *ET = EV->getType();

    switch (ET->getClass()) {
    case RSExportType::ExportClassPrimitive: {
      genGetterAndSetter(static_cast<const RSExportPrimitiveType *>(ET), EV);
      break;
    }
    case RSExportType::ExportClassPointer: {
      // TODO Deprecate this.
      genPointerTypeExportVariable(EV);
      break;
    }
    case RSExportType::ExportClassVector: {
      genGetterAndSetter(static_cast<const RSExportVectorType *>(ET), EV);
      break;
    }
    case RSExportType::ExportClassMatrix: {
      genMatrixTypeExportVariable(EV);
      break;
    }
    case RSExportType::ExportClassConstantArray: {
      genGetterAndSetter(static_cast<const RSExportConstantArrayType *>(ET),
                         EV);
      break;
    }
    case RSExportType::ExportClassRecord: {
      genGetterAndSetter(static_cast<const RSExportRecordType *>(ET), EV);
      break;
    }
    default: { slangAssert(false && "Unknown class of type"); }
    }
  }
}

void RSReflectionCpp::genGetterAndSetter(const RSExportPrimitiveType *EPT,
                                         const RSExportVar *EV) {
  RSReflectionTypeData rtd;
  EPT->convertToRTD(&rtd);
  std::string TypeName = GetTypeName(EPT);

  if (!EV->isConst()) {
    mOut.indent() << "void set_" << EV->getName() << "(" << TypeName << " v)";
    mOut.startBlock();
    mOut.indent() << "setVar(" << getNextExportVarSlot() << ", ";
    if (EPT->isRSObjectType()) {
      mOut << "v";
   } else {
      mOut << "&v, sizeof(v)";
    }
    mOut << ");\n";
    mOut.indent() << RS_EXPORT_VAR_PREFIX << EV->getName() << " = v;\n";
    mOut.endBlock();
  }
  mOut.indent() << TypeName << " get_" << EV->getName() << "() const";
  mOut.startBlock();
  if (EV->isConst()) {
    const clang::APValue &val = EV->getInit();
    bool isBool = !strcmp(TypeName.c_str(), "bool");
    mOut.indent() << "return ";
    genInitValue(val, isBool);
    mOut << ";\n";
  } else {
    mOut.indent() << "return " << RS_EXPORT_VAR_PREFIX << EV->getName()
                  << ";\n";
  }
  mOut.endBlock();
}

void RSReflectionCpp::genPointerTypeExportVariable(const RSExportVar *EV) {
  const RSExportType *ET = EV->getType();

  slangAssert((ET->getClass() == RSExportType::ExportClassPointer) &&
              "Variable should be type of pointer here");

  std::string TypeName = GetTypeName(ET);
  const std::string &VarName = EV->getName();

  RSReflectionTypeData rtd;
  EV->getType()->convertToRTD(&rtd);
  uint32_t slot = getNextExportVarSlot();

  if (!EV->isConst()) {
    mOut.indent() << "void bind_" << VarName << "(" << TypeName << " v)";
    mOut.startBlock();
    mOut.indent() << "bindAllocation(v, " << slot << ");\n";
    mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = v;\n";
    mOut.endBlock();
  }
  mOut.indent() << TypeName << " get_" << VarName << "() const";
  mOut.startBlock();
  if (EV->isConst()) {
    const clang::APValue &val = EV->getInit();
    bool isBool = !strcmp(TypeName.c_str(), "bool");
    mOut.indent() << "return ";
    genInitValue(val, isBool);
    mOut << ";\n";
  } else {
    mOut.indent() << "return " << RS_EXPORT_VAR_PREFIX << VarName << ";\n";
  }
  mOut.endBlock();
}

void RSReflectionCpp::genGetterAndSetter(const RSExportVectorType *EVT,
                                         const RSExportVar *EV) {
  slangAssert(EVT != nullptr);

  RSReflectionTypeData rtd;
  EVT->convertToRTD(&rtd);

  if (!EV->isConst()) {
    mOut.indent() << "void set_" << EV->getName() << "("
                  << rtd.type->rs_c_vector_prefix << EVT->getNumElement()
                  << " v)";
    mOut.startBlock();
    mOut.indent() << "setVar(" << getNextExportVarSlot()
                  << ", &v, sizeof(v));\n";
    mOut.indent() << RS_EXPORT_VAR_PREFIX << EV->getName() << " = v;\n";
    mOut.endBlock();
  }
  mOut.indent() << rtd.type->rs_c_vector_prefix << EVT->getNumElement()
                << " get_" << EV->getName() << "() const";
  mOut.startBlock();
  if (EV->isConst()) {
    const clang::APValue &val = EV->getInit();
    mOut.indent() << "return ";
    genInitValue(val, false);
    mOut << ";\n";
  } else {
    mOut.indent() << "return " << RS_EXPORT_VAR_PREFIX << EV->getName()
                  << ";\n";
  }
  mOut.endBlock();
}

void RSReflectionCpp::genMatrixTypeExportVariable(const RSExportVar *EV) {
  uint32_t slot = getNextExportVarSlot();
  stringstream tmp;
  tmp << slot;

  const RSExportType *ET = EV->getType();
  if (ET->getName() == "rs_matrix4x4") {
    mOut.indent() << "void set_" << EV->getName() << "(float v[16])";
    mOut.startBlock();
    mOut.indent() << "setVar(" << tmp.str() << ", v, sizeof(float)*16);\n";
    mOut.endBlock();
  } else if (ET->getName() == "rs_matrix3x3") {
    mOut.indent() << "void set_" << EV->getName() << "(float v[9])";
    mOut.startBlock();
    mOut.indent() << "setVar(" << tmp.str() << ", v, sizeof(float)*9);";
    mOut.endBlock();
  } else if (ET->getName() == "rs_matrix2x2") {
    mOut.indent() << "void set_" << EV->getName() << "(float v[4])";
    mOut.startBlock();
    mOut.indent() << "setVar(" << tmp.str() << ", v, sizeof(float)*4);";
    mOut.endBlock();
  } else {
    mOut.indent() << "#error: TODO: " << ET->getName();
    slangAssert(false);
  }
}

void RSReflectionCpp::genGetterAndSetter(const RSExportConstantArrayType *AT,
                                         const RSExportVar *EV) {
  std::stringstream ArraySpec;
  const RSExportType *ET = EV->getType();

  const RSExportConstantArrayType *CAT =
      static_cast<const RSExportConstantArrayType *>(ET);

  uint32_t slot = getNextExportVarSlot();
  stringstream tmp;
  tmp << slot;

  ArraySpec << CAT->getNumElement();
  mOut.indent() << "void set_" << EV->getName() << "(" << GetTypeName(EV->getType()) << " v "
      << GetTypeName(EV->getType(), false) << ")";
  mOut.startBlock();
  mOut.indent() << "setVar(" << tmp.str() << ", v, sizeof(" << GetTypeName(EV->getType()) + ") *"
      << ArraySpec.str() << ");";
  mOut.endBlock();
}

void RSReflectionCpp::genGetterAndSetter(const RSExportRecordType *ERT,
                                         const RSExportVar *EV) {
  slangAssert(false);
}

void RSReflectionCpp::makeFunctionSignature(bool isDefinition,
                                            const RSExportFunc *ef) {
  mOut.indent() << "void ";
  if (isDefinition) {
    mOut << mClassName << "::";
  }
  mOut << "invoke_" << ef->getName() << "(";

  if (ef->getParamPacketType()) {
    bool FirstArg = true;
    for (RSExportFunc::const_param_iterator i = ef->params_begin(),
                                            e = ef->params_end();
         i != e; i++) {
      if (!FirstArg) {
        mOut << ", ";
      } else {
        FirstArg = false;
      }
      mOut << GetTypeName((*i)->getType()) << " " << (*i)->getName();
    }
  }

  if (isDefinition) {
    mOut << ")";
  } else {
    mOut << ");\n";
  }
}

void RSReflectionCpp::genArguments(const ArgumentList &Arguments, int Offset) {
  bool FirstArg = true;

  for (ArgumentList::const_iterator I = Arguments.begin(), E = Arguments.end();
       I != E; I++) {
    if (!FirstArg) {
      mOut << ",\n";
      mOut.indent() << string(Offset, ' ');
    } else {
      FirstArg = false;
    }

    mOut << I->Type << " " << I->Name;
    if (!I->DefaultValue.empty()) {
      mOut << " = " << I->DefaultValue;
    }
  }
}

bool RSReflectionCpp::genCreateFieldPacker(const RSExportType *ET,
                                           const char *FieldPackerName) {
  size_t AllocSize = ET->getAllocSize();

  if (AllocSize > 0) {
    mOut.indent() << "android::RSC::FieldPacker " << FieldPackerName << "("
                  << AllocSize << ");\n";
    return true;
  }

  return false;
}

void RSReflectionCpp::genPackVarOfType(const RSExportType *ET,
                                       const char *VarName,
                                       const char *FieldPackerName) {
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassPointer:
  case RSExportType::ExportClassMatrix: {
    mOut.indent() << FieldPackerName << ".add(" << VarName << ");\n";
    break;
  }
  case RSExportType::ExportClassConstantArray: {
    /*const RSExportConstantArrayType *ECAT =
        static_cast<const RSExportConstantArrayType *>(ET);

    // TODO(zonr): more elegant way. Currently, we obtain the unique index
    //             variable (this method involves recursive call which means
    //             we may have more than one level loop, therefore we can't
    //             always use the same index variable name here) name given
    //             in the for-loop from counting the '.' in @VarName.
    unsigned Level = 0;
    size_t LastDotPos = 0;
    std::string ElementVarName(VarName);

  while (LastDotPos != std::string::npos) {
    LastDotPos = ElementVarName.find_first_of('.', LastDotPos + 1);
    Level++;
  }
  std::string IndexVarName("ct");
  IndexVarName.append(llvm::utostr(Level));

  C.indent() << "for (int " << IndexVarName << " = 0; " <<
                      IndexVarName << " < " << ECAT->getSize() << "; " <<
                      IndexVarName << "++)";
  C.startBlock();

  ElementVarName.append("[" + IndexVarName + "]");
  genPackVarOfType(C, ECAT->getElementType(), ElementVarName.c_str(),
                   FieldPackerName);

  C.endBlock();*/
    break;
  }
  case RSExportType::ExportClassRecord: {
    const RSExportRecordType *ERT = static_cast<const RSExportRecordType *>(ET);
    // Relative pos from now on in field packer
    unsigned Pos = 0;

    for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(),
                                                  E = ERT->fields_end();
         I != E; I++) {
      const RSExportRecordType::Field *F = *I;
      std::string FieldName;
      size_t FieldOffset = F->getOffsetInParent();
      const RSExportType *T = F->getType();
      size_t FieldStoreSize = T->getStoreSize();
      size_t FieldAllocSize = T->getAllocSize();

      if (VarName != nullptr)
        FieldName = VarName + ("." + F->getName());
      else
        FieldName = F->getName();

      if (FieldOffset > Pos) {
        mOut.indent() << FieldPackerName << ".skip(" << (FieldOffset - Pos)
                      << ");\n";
      }

      genPackVarOfType(F->getType(), FieldName.c_str(), FieldPackerName);

      // There is padding in the field type
      if (FieldAllocSize > FieldStoreSize) {
        mOut.indent() << FieldPackerName << ".skip("
                      << (FieldAllocSize - FieldStoreSize) << ");\n";
      }

      Pos = FieldOffset + FieldAllocSize;
    }

    // There maybe some padding after the struct
    if (ERT->getAllocSize() > Pos) {
      mOut.indent() << FieldPackerName << ".skip(" << ERT->getAllocSize() - Pos
                    << ");\n";
    }
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
}

void RSReflectionCpp::genTypeCheck(const RSExportType *ET,
                                   const char *VarName) {
  mOut.indent() << "// Type check for " << VarName << "\n";

  if (ET->getClass() == RSExportType::ExportClassPointer) {
    const RSExportPointerType *EPT =
        static_cast<const RSExportPointerType *>(ET);
    ET = EPT->getPointeeType();
  }

  std::string TypeName;
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassRecord: {
    TypeName = ET->getElementName();
    break;
  }

  default:
    break;
  }

  if (!TypeName.empty()) {
    mOut.indent() << "if (!" << VarName
                  << "->getType()->getElement()->isCompatible("
                  << kRsElemPrefix << TypeName << "))";
    mOut.startBlock();
    mOut.indent() << "mRS->throwError(RS_ERROR_RUNTIME_ERROR, "
                     "\"Incompatible type\");\n";
    mOut.indent() << "return;\n";
    mOut.endBlock();
  }
}

void RSReflectionCpp::genTypeInstanceFromPointer(const RSExportType *ET) {
  if (ET->getClass() == RSExportType::ExportClassPointer) {
    // For pointer parameters to original forEach kernels.
    const RSExportPointerType *EPT =
        static_cast<const RSExportPointerType *>(ET);
    genTypeInstance(EPT->getPointeeType());
  } else {
    // For handling pass-by-value kernel parameters.
    genTypeInstance(ET);
  }
}

void RSReflectionCpp::genTypeInstance(const RSExportType *ET) {
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassConstantArray:
  case RSExportType::ExportClassRecord: {
    std::string TypeName = ET->getElementName();
    mTypesToCheck.insert(TypeName);
    break;
  }

  default:
    break;
  }
}

void RSReflectionCpp::genInitExportVariable(const RSExportType *ET,
                                            const std::string &VarName,
                                            const clang::APValue &Val) {
  slangAssert(!Val.isUninit() && "Not a valid initializer");

  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    const RSExportPrimitiveType *EPT =
        static_cast<const RSExportPrimitiveType *>(ET);
    if (EPT->getType() == DataTypeBoolean) {
      genInitBoolExportVariable(VarName, Val);
    } else {
      genInitPrimitiveExportVariable(VarName, Val);
    }
    break;
  }
  case RSExportType::ExportClassPointer: {
    if (!Val.isInt() || Val.getInt().getSExtValue() != 0)
      std::cerr << "Initializer which is non-NULL to pointer type variable "
                   "will be ignored" << std::endl;
    break;
  }
  case RSExportType::ExportClassVector: {
    const RSExportVectorType *EVT = static_cast<const RSExportVectorType *>(ET);
    switch (Val.getKind()) {
    case clang::APValue::Int:
    case clang::APValue::Float: {
      for (unsigned i = 0; i < EVT->getNumElement(); i++) {
        std::string Name = VarName + "." + getVectorAccessor(i);
        genInitPrimitiveExportVariable(Name, Val);
      }
      break;
    }
    case clang::APValue::Vector: {
      unsigned NumElements = std::min(
          static_cast<unsigned>(EVT->getNumElement()), Val.getVectorLength());
      for (unsigned i = 0; i < NumElements; i++) {
        const clang::APValue &ElementVal = Val.getVectorElt(i);
        std::string Name = VarName + "." + getVectorAccessor(i);
        genInitPrimitiveExportVariable(Name, ElementVal);
      }
      break;
    }
    case clang::APValue::MemberPointer:
    case clang::APValue::Uninitialized:
    case clang::APValue::ComplexInt:
    case clang::APValue::ComplexFloat:
    case clang::APValue::LValue:
    case clang::APValue::Array:
    case clang::APValue::Struct:
    case clang::APValue::Union:
    case clang::APValue::AddrLabelDiff: {
      slangAssert(false && "Unexpected type of value of initializer.");
    }
    }
    break;
  }
  case RSExportType::ExportClassMatrix:
  case RSExportType::ExportClassConstantArray:
  case RSExportType::ExportClassRecord: {
    slangAssert(false && "Unsupported initializer for record/matrix/constant "
                         "array type variable currently");
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
}

const char *RSReflectionCpp::getVectorAccessor(unsigned Index) {
  static const char *VectorAccessorMap[] = {/* 0 */ "x",
                                            /* 1 */ "y",
                                            /* 2 */ "z",
                                            /* 3 */ "w",
  };

  slangAssert((Index < (sizeof(VectorAccessorMap) / sizeof(const char *))) &&
              "Out-of-bound index to access vector member");

  return VectorAccessorMap[Index];
}

void RSReflectionCpp::genZeroInitExportVariable(const std::string &VarName) {
  mOut.indent() << "memset(&" << RS_EXPORT_VAR_PREFIX << VarName
                << ", 0, sizeof(" << RS_EXPORT_VAR_PREFIX << VarName << "));\n";
}

void
RSReflectionCpp::genInitPrimitiveExportVariable(const std::string &VarName,
                                                const clang::APValue &Val) {
  slangAssert(!Val.isUninit() && "Not a valid initializer");

  mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = ";
  genInitValue(Val);
  mOut << ";\n";
}

void RSReflectionCpp::genInitValue(const clang::APValue &Val, bool asBool) {
  switch (Val.getKind()) {
  case clang::APValue::Int: {
    const llvm::APInt &api = Val.getInt();
    if (asBool) {
      mOut << ((api.getSExtValue() == 0) ? "false" : "true");
    } else {
      // TODO: Handle unsigned correctly for C++
      mOut << api.getSExtValue();
      if (api.getBitWidth() > 32) {
        mOut << "L";
      }
    }
    break;
  }

  case clang::APValue::Float: {
    const llvm::APFloat &apf = Val.getFloat();
    llvm::SmallString<30> s;
    apf.toString(s);
    mOut << s.c_str();
    if (&apf.getSemantics() == &llvm::APFloat::IEEEsingle) {
      if (s.count('.') == 0) {
        mOut << ".f";
      } else {
        mOut << "f";
      }
    }
    break;
  }

  case clang::APValue::ComplexInt:
  case clang::APValue::ComplexFloat:
  case clang::APValue::LValue:
  case clang::APValue::Vector: {
    slangAssert(false && "Primitive type cannot have such kind of initializer");
    break;
  }

  default: { slangAssert(false && "Unknown kind of initializer"); }
  }
}

void RSReflectionCpp::genInitBoolExportVariable(const std::string &VarName,
                                                const clang::APValue &Val) {
  slangAssert(!Val.isUninit() && "Not a valid initializer");
  slangAssert((Val.getKind() == clang::APValue::Int) &&
              "Bool type has wrong initial APValue");

  mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = "
                << ((Val.getInt().getSExtValue() == 0) ? "false" : "true")
                << ";";
}

} // namespace slang
