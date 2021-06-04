/*
 * Copyright 2012, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_REFLECTION_CPP_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_REFLECTION_CPP_H_

#include "slang_rs_reflect_utils.h"

#include <set>
#include <string>

#define RS_EXPORT_VAR_PREFIX "mExportVar_"

namespace slang {

class RSReflectionCpp {
 public:
  RSReflectionCpp(const RSContext *Context, const std::string &OutputDirectory,
                  const std::string &RSSourceFileName,
                  const std::string &BitCodeFileName);
  virtual ~RSReflectionCpp();

  bool reflect();

 private:
  struct Argument {
    std::string Type;
    std::string Name;
    std::string DefaultValue;
    Argument(std::string Type, std::string Name, std::string DefaultValue = "")
      : Type(Type), Name(Name), DefaultValue(DefaultValue) {}
  };
  typedef std::vector<Argument> ArgumentList;

  // Information coming from the compiler about the code we're reflecting.
  const RSContext *mRSContext;

  // Path to the *.rs file for which we're generating C++ code.
  std::string mRSSourceFilePath;
  // Path to the file that contains the byte code generated from the *.rs file.
  std::string mBitCodeFilePath;
  // The directory where we'll generate the C++ files.
  std::string mOutputDirectory;
  // A cleaned up version of the *.rs file name that can be used in generating
  // C++ identifiers.
  std::string mCleanedRSFileName;
  // The name of the generated C++ class.
  std::string mClassName;

  // TODO document
  unsigned int mNextExportVarSlot;
  unsigned int mNextExportFuncSlot;
  unsigned int mNextExportForEachSlot;

  // Generated RS Elements for type-checking code.
  std::set<std::string> mTypesToCheck;

  inline void clear() {
    mNextExportVarSlot = 0;
    mNextExportFuncSlot = 0;
    mNextExportForEachSlot = 0;
    mTypesToCheck.clear();
  }

  // The file we are currently generating, either the header or the
  // implementation file.
  GeneratedFile mOut;

  void genInitValue(const clang::APValue &Val, bool asBool = false);
  static const char *getVectorAccessor(unsigned index);

  inline unsigned int getNextExportVarSlot() { return mNextExportVarSlot++; }

  inline unsigned int getNextExportFuncSlot() { return mNextExportFuncSlot++; }

  inline unsigned int getNextExportForEachSlot() {
    return mNextExportForEachSlot++;
  }

  bool writeHeaderFile();
  bool writeImplementationFile();

  // Write out signatures both in the header and implementation.
  void makeFunctionSignature(bool isDefinition, const RSExportFunc *ef);

  bool genEncodedBitCode();
  void genFieldsToStoreExportVariableValues();
  void genTypeInstancesUsedInForEach();
  void genFieldsForAllocationTypeVerification();

  // Write out the code for the getters and setters.
  void genExportVariablesGetterAndSetter();

  // Write out the code for the declaration of the kernel entry points.
  void genForEachDeclarations();
  void genExportFunctionDeclarations();

  // Write out code for the definitions of the kernel entry points.
  void genExportForEachBodies();
  void genExportFunctionBodies();

  bool startScriptHeader();

  // Write out code for an export variable initialization.
  void genInitExportVariable(const RSExportType *ET, const std::string &VarName,
                             const clang::APValue &Val);
  void genZeroInitExportVariable(const std::string &VarName);
  void genInitBoolExportVariable(const std::string &VarName,
                                 const clang::APValue &Val);
  void genInitPrimitiveExportVariable(const std::string &VarName,
                                      const clang::APValue &Val);

  // Produce an argument string of the form "T1 t, T2 u, T3 v".
  void genArguments(const ArgumentList &Args, int Offset);

  void genPointerTypeExportVariable(const RSExportVar *EV);
  void genMatrixTypeExportVariable(const RSExportVar *EV);
  void genRecordTypeExportVariable(const RSExportVar *EV);

  void genGetterAndSetter(const RSExportPrimitiveType *EPT, const RSExportVar* EV);
  void genGetterAndSetter(const RSExportVectorType *EVT, const RSExportVar* EV);
  void genGetterAndSetter(const RSExportConstantArrayType *AT, const RSExportVar* EV);
  void genGetterAndSetter(const RSExportRecordType *ERT, const RSExportVar *EV);

  // Write out a local FieldPacker (if necessary).
  bool genCreateFieldPacker(const RSExportType *T, const char *FieldPackerName);

  // Populate (write) the FieldPacker with add() operations.
  void genPackVarOfType(const RSExportType *ET, const char *VarName,
                        const char *FieldPackerName);

  // Generate a runtime type check for VarName.
  void genTypeCheck(const RSExportType *ET, const char *VarName);

  // Generate a type instance for a given type.
  void genTypeInstanceFromPointer(const RSExportType *ET);
  void genTypeInstance(const RSExportType *ET);

}; // class RSReflectionCpp

} // namespace slang

#endif // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_REFLECTION_CPP_H_  NOLINT
