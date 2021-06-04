/*
 * Copyright 2010-2014, The Android Open Source Project
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

#include "slang_rs_reflection.h"

#include <sys/stat.h>

#include <cstdarg>
#include <cctype>

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringExtras.h"

#include "os_sep.h"
#include "slang_rs_context.h"
#include "slang_rs_export_var.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_func.h"
#include "slang_rs_export_reduce.h"
#include "slang_rs_reflect_utils.h"
#include "slang_rs_reflection_state.h"
#include "slang_version.h"

#define RS_SCRIPT_CLASS_NAME_PREFIX "ScriptC_"
#define RS_SCRIPT_CLASS_SUPER_CLASS_NAME "ScriptC"

#define RS_TYPE_CLASS_SUPER_CLASS_NAME ".Script.FieldBase"

#define RS_TYPE_ITEM_CLASS_NAME "Item"

#define RS_TYPE_ITEM_SIZEOF_LEGACY "Item.sizeof"
#define RS_TYPE_ITEM_SIZEOF_CURRENT "mElement.getBytesSize()"

#define RS_TYPE_ITEM_BUFFER_NAME "mItemArray"
#define RS_TYPE_ITEM_BUFFER_PACKER_NAME "mIOBuffer"
#define RS_TYPE_ELEMENT_REF_NAME "mElementCache"

#define RS_EXPORT_VAR_INDEX_PREFIX "mExportVarIdx_"
#define RS_EXPORT_VAR_PREFIX "mExportVar_"
#define RS_EXPORT_VAR_ELEM_PREFIX "mExportVarElem_"
#define RS_EXPORT_VAR_DIM_PREFIX "mExportVarDim_"
#define RS_EXPORT_VAR_CONST_PREFIX "const_"

#define RS_ELEM_PREFIX "__"

#define RS_FP_PREFIX "__rs_fp_"

#define RS_RESOURCE_NAME "__rs_resource_name"

#define RS_EXPORT_FUNC_INDEX_PREFIX "mExportFuncIdx_"
#define RS_EXPORT_FOREACH_INDEX_PREFIX "mExportForEachIdx_"
#define RS_EXPORT_REDUCE_INDEX_PREFIX "mExportReduceIdx_"

#define RS_EXPORT_VAR_ALLOCATION_PREFIX "mAlloction_"
#define RS_EXPORT_VAR_DATA_STORAGE_PREFIX "mData_"

#define SAVED_RS_REFERENCE "mRSLocal"

namespace slang {

static void genCheck64BitInternal(const RSContext *Context, ReflectionState *State,
                                  GeneratedFile &Out, bool Parens);

class RSReflectionJavaElementBuilder {
public:
  RSReflectionJavaElementBuilder(const char *ElementBuilderName,
                                 const RSExportRecordType *ERT,
                                 const char *RenderScriptVar,
                                 GeneratedFile *Out, const RSContext *RSContext,
                                 RSReflectionJava *Reflection,
                                 ReflectionState *RState);
  void generate();

private:
  void genAddElement(const RSExportType *ET, const std::string &VarName,
                     unsigned ArraySize);
  void genAddStatementStart();
  void genAddStatementEnd(const std::string &VarName, unsigned ArraySize,
                          unsigned Which = RSReflectionJava::FieldIndex | RSReflectionJava::Field32Index);
  void genAddPadding(int PaddingSize, unsigned Which);  // Which: See RSReflectionJava::incFieldIndex()
  void genAddPadding(int PaddingSize, ReflectionState::Val32 Field32PaddingSize);
  // TODO Will remove later due to field name information is not necessary for
  // C-reflect-to-Java
  std::string createPaddingField() {
    return mPaddingPrefix + llvm::itostr(mPaddingFieldIndex++);
  }

  void genCheck64Bit(bool Parens) {
    genCheck64BitInternal(mRSContext, mState, *mOut, Parens);
  }

  const char *mElementBuilderName;
  const RSExportRecordType *mERT;
  const char *mRenderScriptVar;
  GeneratedFile *mOut;
  std::string mPaddingPrefix;
  int mPaddingFieldIndex;
  const RSContext *mRSContext;
  RSReflectionJava *mReflection;
  ReflectionState *mState;
};

enum MatrixLanguage { ML_Java, ML_Script };
static const char *GetMatrixTypeName(const RSExportMatrixType *EMT, MatrixLanguage lang) {
  static const char *MatrixTypeJavaNameMap[3][2] = {/* 2x2 */ { "Matrix2f", "rs_matrix2x2" },
                                                    /* 3x3 */ { "Matrix3f", "rs_matrix3x3" },
                                                    /* 4x4 */ { "Matrix4f", "rs_matrix4x4" }
  };
  unsigned Dim = EMT->getDim();

  if ((Dim - 2) < (sizeof(MatrixTypeJavaNameMap) / sizeof(const char *)))
    return MatrixTypeJavaNameMap[EMT->getDim() - 2][lang];

  slangAssert(false && "GetMatrixTypeName : Unsupported matrix dimension");
  return nullptr;
}

static const char *GetVectorAccessor(unsigned Index) {
  static const char *VectorAccessorMap[] = {/* 0 */ "x",
                                            /* 1 */ "y",
                                            /* 2 */ "z",
                                            /* 3 */ "w",
  };

  slangAssert((Index < (sizeof(VectorAccessorMap) / sizeof(const char *))) &&
              "Out-of-bound index to access vector member");

  return VectorAccessorMap[Index];
}

static const char *GetPackerAPIName(const RSExportPrimitiveType *EPT) {
  static const char *PrimitiveTypePackerAPINameMap[] = {
      "addI16",     // DataTypeFloat16
      "addF32",     // DataTypeFloat32
      "addF64",     // DataTypeFloat64
      "addI8",      // DataTypeSigned8
      "addI16",     // DataTypeSigned16
      "addI32",     // DataTypeSigned32
      "addI64",     // DataTypeSigned64
      "addU8",      // DataTypeUnsigned8
      "addU16",     // DataTypeUnsigned16
      "addU32",     // DataTypeUnsigned32
      "addU64",     // DataTypeUnsigned64
      "addBoolean", // DataTypeBoolean
      "addU16",     // DataTypeUnsigned565
      "addU16",     // DataTypeUnsigned5551
      "addU16",     // DataTypeUnsigned4444
      "addMatrix",  // DataTypeRSMatrix2x2
      "addMatrix",  // DataTypeRSMatrix3x3
      "addMatrix",  // DataTypeRSMatrix4x4
      "addObj",     // DataTypeRSElement
      "addObj",     // DataTypeRSType
      "addObj",     // DataTypeRSAllocation
      "addObj",     // DataTypeRSSampler
      "addObj",     // DataTypeRSScript
      "addObj",     // DataTypeRSMesh
      "addObj",     // DataTypeRSPath
      "addObj",     // DataTypeRSProgramFragment
      "addObj",     // DataTypeRSProgramVertex
      "addObj",     // DataTypeRSProgramRaster
      "addObj",     // DataTypeRSProgramStore
      "addObj",     // DataTypeRSFont
  };
  unsigned TypeId = EPT->getType();

  if (TypeId < (sizeof(PrimitiveTypePackerAPINameMap) / sizeof(const char *)))
    return PrimitiveTypePackerAPINameMap[EPT->getType()];

  slangAssert(false && "GetPackerAPIName : Unknown primitive data type");
  return nullptr;
}

namespace {

std::string GetReduceResultTypeName(const RSExportType *ET) {
  switch (ET->getClass()) {
    case RSExportType::ExportClassConstantArray: {
      const RSExportConstantArrayType *const CAT = static_cast<const RSExportConstantArrayType *>(ET);
      return "resultArray" + std::to_string(CAT->getNumElement()) + "_" +
          RSReflectionJava::GetTypeName(
              CAT->getElementType(),
              (RSReflectionJava::TypeNameDefault & ~RSReflectionJava::TypeNameWithRecordElementName) |
              RSReflectionJava::TypeNameC);
    }
    case RSExportType::ExportClassRecord:
      return "resultStruct_" +
          RSReflectionJava::GetTypeName(
              ET,
              (RSReflectionJava::TypeNameDefault & ~RSReflectionJava::TypeNameWithRecordElementName) |
              RSReflectionJava::TypeNameC);
    default:
      return "result_" +
          RSReflectionJava::GetTypeName(ET, RSReflectionJava::TypeNameDefault | RSReflectionJava::TypeNameC);
  }
}

std::string GetReduceResultTypeName(const RSExportReduce *ER) {
  return GetReduceResultTypeName(ER->getResultType());
}

} // end anonymous namespace

static const char *GetTypeNullValue(const RSExportType *ET) {
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    const RSExportPrimitiveType *EPT =
        static_cast<const RSExportPrimitiveType *>(ET);
    if (EPT->isRSObjectType())
      return "null";
    else if (EPT->getType() == DataTypeBoolean)
      return "false";
    else
      return "0";
    break;
  }
  case RSExportType::ExportClassPointer:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassMatrix:
  case RSExportType::ExportClassConstantArray:
  case RSExportType::ExportClassRecord: {
    return "null";
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
  return "";
}

static std::string GetBuiltinElementConstruct(const RSExportType *ET) {
  if (ET->getClass() == RSExportType::ExportClassPrimitive) {
    return std::string("Element.") + ET->getElementName();
  } else if (ET->getClass() == RSExportType::ExportClassVector) {
    const RSExportVectorType *EVT = static_cast<const RSExportVectorType *>(ET);
    if (EVT->getType() == DataTypeFloat32) {
      if (EVT->getNumElement() == 2) {
        return "Element.F32_2";
      } else if (EVT->getNumElement() == 3) {
        return "Element.F32_3";
      } else if (EVT->getNumElement() == 4) {
        return "Element.F32_4";
      } else {
        slangAssert(false && "Vectors should be size 2, 3, 4");
      }
    } else if (EVT->getType() == DataTypeUnsigned8) {
      if (EVT->getNumElement() == 4)
        return "Element.U8_4";
    }
  } else if (ET->getClass() == RSExportType::ExportClassMatrix) {
    const RSExportMatrixType *EMT = static_cast<const RSExportMatrixType *>(ET);
    switch (EMT->getDim()) {
    case 2:
      return "Element.MATRIX_2X2";
    case 3:
      return "Element.MATRIX_3X3";
    case 4:
      return "Element.MATRIX_4X4";
    default:
      slangAssert(false && "Unsupported dimension of matrix");
    }
  }
  // RSExportType::ExportClassPointer can't be generated in a struct.

  return "";
}

// If FromIntegerType == DestIntegerType, then Value is returned.
// Otherwise, return a Java expression that zero-extends the value
// Value, assumed to be of type FromIntegerType, to the integer type
// DestIntegerType.
//
// Intended operations:
//  byte  -> {byte,int,short,long}
//  short -> {short,int,long}
//  int   -> {int,long}
//  long  -> long
static std::string ZeroExtendValue(const std::string &Value,
                                   const std::string &FromIntegerType,
                                   const std::string &DestIntegerType) {
#ifndef __DISABLE_ASSERTS
  // Integer types arranged in increasing order by width
  const std::vector<std::string> ValidTypes{"byte", "short", "int", "long"};
  auto FromTypeLoc = std::find(ValidTypes.begin(), ValidTypes.end(), FromIntegerType);
  auto DestTypeLoc = std::find(ValidTypes.begin(), ValidTypes.end(), DestIntegerType);
  // Check that both types are valid.
  slangAssert(FromTypeLoc != ValidTypes.end());
  slangAssert(DestTypeLoc != ValidTypes.end());
  // Check that DestIntegerType is at least as wide as FromIntegerType.
  slangAssert(FromTypeLoc - ValidTypes.begin() <= DestTypeLoc - ValidTypes.begin());
#endif

  if (FromIntegerType == DestIntegerType) {
    return Value;
  }

  std::string Mask, MaskLiteralType;
  if (FromIntegerType == "byte") {
    Mask = "0xff";
    MaskLiteralType = "int";
  } else if (FromIntegerType == "short") {
    Mask = "0xffff";
    MaskLiteralType = "int";
  } else if (FromIntegerType == "int") {
    Mask = "0xffffffffL";
    MaskLiteralType = "long";
  } else {
    // long -> long casts should have already been handled.
    slangAssert(false && "Unknown integer type");
  }

  // Cast the mask to the appropriate type.
  if (MaskLiteralType != DestIntegerType) {
    Mask = "(" + DestIntegerType + ") " + Mask;
  }
  return "((" + DestIntegerType + ") ((" + Value + ") & " + Mask + "))";
}

std::string RSReflectionJava::GetTypeName(const RSExportType *ET, unsigned Style) {
  slangAssert((Style & (TypeNameC|TypeNamePseudoC)) != (TypeNameC|TypeNamePseudoC));
  slangAssert(!(Style & TypeNamePseudoC) || (Style == TypeNamePseudoC));

  const bool CLike = Style & (TypeNameC|TypeNamePseudoC);

  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    const auto ReflectionType =
        RSExportPrimitiveType::getRSReflectionType(static_cast<const RSExportPrimitiveType *>(ET));
    return (CLike ? ReflectionType->s_name : ReflectionType->java_name);
  }
  case RSExportType::ExportClassPointer: {
    slangAssert(!(Style & TypeNameC) &&
                "No need to support C type names for pointer types yet");
    const RSExportType *PointeeType =
        static_cast<const RSExportPointerType *>(ET)->getPointeeType();

    if (Style & TypeNamePseudoC)
      return GetTypeName(PointeeType, Style) + "*";
    else if (PointeeType->getClass() != RSExportType::ExportClassRecord)
      return "Allocation";
    else
      return PointeeType->getElementName();
  }
  case RSExportType::ExportClassVector: {
    const RSExportVectorType *EVT = static_cast<const RSExportVectorType *>(ET);
    const auto ReflectionType = EVT->getRSReflectionType(EVT);
    std::stringstream VecName;
    VecName << (CLike ? ReflectionType->s_name : ReflectionType->rs_java_vector_prefix)
            << EVT->getNumElement();
    return VecName.str();
  }
  case RSExportType::ExportClassMatrix: {
    return GetMatrixTypeName(static_cast<const RSExportMatrixType *>(ET), CLike ? ML_Script : ML_Java);
  }
  case RSExportType::ExportClassConstantArray: {
    const RSExportConstantArrayType *CAT =
        static_cast<const RSExportConstantArrayType *>(ET);
    std::string ElementTypeName = GetTypeName(CAT->getElementType(), Style);
    if (Style & TypeNamePseudoC) {
      std::stringstream ArrayName;
      ArrayName << ElementTypeName << '[' << CAT->getNumElement() << ']';
      return ArrayName.str();
    }
    else if (Style & TypeNameWithConstantArrayBrackets) {
      slangAssert(!(Style & TypeNameC) &&
                  "No need to support C type names for array types with brackets yet");
      ElementTypeName.append("[]");
    }
    return ElementTypeName;
  }
  case RSExportType::ExportClassRecord: {
    slangAssert(!(Style & TypeNameC) &&
                "No need to support C type names for record types yet");
    if (Style & TypeNamePseudoC)
      return "struct " + ET->getName();
    else if (Style & TypeNameWithRecordElementName)
      return ET->getElementName() + "." RS_TYPE_ITEM_CLASS_NAME;
    else
      return ET->getName();
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }

  return "";
}

void RSReflectionJava::genConditionalVal(const std::string &Prefix, bool Parens,
                                         size_t Val, ReflectionState::Val32 Val32) {
  if (Prefix.empty() || (Val != 0) || (Val32.first && (Val32.second != 0 ))) {
    mOut << Prefix;

    if (!Val32.first || (Val == Val32.second)) {
      // Either we're ignoring the 32-bit case, or 32-bit and 64-bit
      // values are the same.
      mOut << Val;
    } else {
      // We cannot ignore the 32-bit case, and 32-bit and 64-bit
      // values differ.
      if (Parens)
        mOut << '(';
      genCheck64Bit(true);
      mOut << " ? " << Val << " : " << Val32.second;
      if (Parens)
        mOut << ')';
    }
  }
}

static void genCheck64BitInternal(const RSContext *Context, ReflectionState *State,
                                  GeneratedFile &Out, bool Parens) {
  State->setOutputClassDivergent();
  if (Context->isCompatLib()) {
    if (Parens)
      Out << '(';
    Out << "RenderScript.getPointerSize() == 8";
    if (Parens)
      Out << ')';
  }
  else
    Out << "sIs64Bit";
}

void RSReflectionJava::genCheck64Bit(bool Parens) {
  genCheck64BitInternal(mRSContext, mState, mOut, Parens);
}

void RSReflectionJava::genCompute64Bit() {
  if (mRSContext->isCompatLib()) {
    // We can rely on RenderScript class in lockstep with llvm-rs-cc
    // and hence in lockstep with the generated code, so we don't need
    // any complicated logic to determine pointer size.
    return;
  }

  // Note that Android L is the first release to support 64-bit
  // targets.  When RenderScript is compiled with "-target-api $v"
  // with "$v < 21" (L is API level 21), we only compile for 32-bit,
  // and we reflect during that compile, so there are no divergent
  // structs, and we will not get here.

  slangAssert(mRSContext->getTargetAPI() >= SLANG_L_TARGET_API);

  mOut.indent() << "private static boolean sIs64Bit;\n\n";
  mOut.indent() << "static";
  mOut.startBlock();
  mOut.indent() << "if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)";
  mOut.startBlock();
  mOut.indent() << "sIs64Bit = Process.is64Bit();\n";
  mOut.endBlock();
  mOut.indent() << "else";
  mOut.startBlock();
  mOut.indent() << "try";
  mOut.startBlock();
  mOut.indent() << "Field f = RenderScript.class.getDeclaredField(\"sPointerSize\");\n";
  mOut.indent() << "f.setAccessible(true);\n";
  mOut.indent() << "sIs64Bit = (f.getInt(null) == 8);\n";
  mOut.endBlock();

  // If reflection fails, assume we're on a 32-bit-only device
  // (64-bit-only is not allowed).  This should only happen if the
  // device is L-or-later but has been customized in some way so that
  // the field "sPointerSize" -- introduced in L -- is not present.
  //
  // Alternatively, we could treat this as 64-bit (reverting to the
  // behavior prior to the fix for http://b/32780232) or we could
  // decide we have no idea what's going on and throw an exception.
  mOut.indent() << "catch (Throwable e)";
  mOut.startBlock();
  mOut.indent() << "sIs64Bit = false;\n";
  mOut.endBlock();

  mOut.endBlock();
  mOut.endBlock();
}

/********************** Methods to generate script class **********************/
RSReflectionJava::RSReflectionJava(const RSContext *Context,
                                   std::vector<std::string> *GeneratedFileNames,
                                   const std::string &OutputBaseDirectory,
                                   const std::string &RSSourceFileName,
                                   const std::string &BitCodeFileName,
                                   bool EmbedBitcodeInJava,
                                   ReflectionState *RState)
    : mRSContext(Context), mState(RState), mCollecting(RState->isCollecting()),
      mPackageName(Context->getReflectJavaPackageName()),
      mRSPackageName(Context->getRSPackageName()),
      mOutputBaseDirectory(OutputBaseDirectory),
      mRSSourceFileName(RSSourceFileName), mBitCodeFileName(BitCodeFileName),
      mResourceId(RSSlangReflectUtils::JavaClassNameFromRSFileName(
          mBitCodeFileName.c_str())),
      mScriptClassName(RS_SCRIPT_CLASS_NAME_PREFIX +
                       RSSlangReflectUtils::JavaClassNameFromRSFileName(
                           mRSSourceFileName.c_str())),
      mEmbedBitcodeInJava(EmbedBitcodeInJava), mNextExportVarSlot(0),
      mNextExportFuncSlot(0), mNextExportForEachSlot(0),
      mNextExportReduceSlot(0), mLastError(""),
  mGeneratedFileNames(GeneratedFileNames), mFieldIndex(0), mField32Index(0) {
  slangAssert(mGeneratedFileNames && "Must supply GeneratedFileNames");
  slangAssert(!mPackageName.empty() && mPackageName != "-");

  mOutputDirectory = RSSlangReflectUtils::ComputePackagedPath(
                         OutputBaseDirectory.c_str(), mPackageName.c_str()) +
                     OS_PATH_SEPARATOR_STR;

  // mElement.getBytesSize only exists on JB+
  if (mRSContext->getTargetAPI() >= SLANG_JB_TARGET_API) {
      mItemSizeof = RS_TYPE_ITEM_SIZEOF_CURRENT;
  } else {
      mItemSizeof = RS_TYPE_ITEM_SIZEOF_LEGACY;
  }

  mState->nextFile(mRSContext, mPackageName, mRSSourceFileName);
}

bool RSReflectionJava::genScriptClass(const std::string &ClassName,
                                      std::string &ErrorMsg) {
  if (!mCollecting) {
    if (!startClass(AM_Public, false, ClassName, RS_SCRIPT_CLASS_SUPER_CLASS_NAME,
                    ErrorMsg))
      return false;

    mState->beginOutputClass();
    genScriptClassConstructor();
  }

  // Reflect exported variables
  mState->beginVariables(mRSContext->export_vars_size());
  for (auto I = mRSContext->export_vars_begin(),
            E = mRSContext->export_vars_end();
       I != E; I++)
    genExportVariable(*I);
  mState->endVariables();

  // Reflect exported forEach functions (only available on ICS+)
  if (mRSContext->getTargetAPI() >= SLANG_ICS_TARGET_API) {
    mState->beginForEaches(mRSContext->getNumAssignedForEachOrdinals());
    for (auto I = mRSContext->export_foreach_begin(),
              E = mRSContext->export_foreach_end();
         I != E; I++) {
      genExportForEach(*I);
    }
    mState->endForEaches();
  }

  // Reflect exported reduce functions
  if (!mCollecting) {
    for (const RSExportType *ResultType : mRSContext->getReduceResultTypes(
             // FilterIn
             exportableReduce,

             // Compare
             [](const RSExportType *A, const RSExportType *B)
             { return GetReduceResultTypeName(A) < GetReduceResultTypeName(B); }))
      genExportReduceResultType(ResultType);
  }
  mState->beginReduces(mRSContext->export_reduce_size());
  for (auto I = mRSContext->export_reduce_begin(),
            E = mRSContext->export_reduce_end();
       I != E; ++I)
    genExportReduce(*I);
  mState->endReduces();

  // Reflect exported functions (invokable)
  mState->beginInvokables(mRSContext->export_funcs_size());
  for (auto I = mRSContext->export_funcs_begin(),
            E = mRSContext->export_funcs_end();
       I != E; ++I)
    genExportFunction(*I);
  mState->endInvokables();

  if (!mCollecting) {
    if (mState->endOutputClass())
      genCompute64Bit();

    endClass();

    mGeneratedFileNames->push_back(mScriptClassName);
  }

  return true;
}

void RSReflectionJava::genScriptClassConstructor() {
  std::string className(RSSlangReflectUtils::JavaBitcodeClassNameFromRSFileName(
      mRSSourceFileName.c_str()));
  // Provide a simple way to reference this object.
  mOut.indent() << "private static final String " RS_RESOURCE_NAME " = \""
                << getResourceId() << "\";\n";

  // Generate a simple constructor with only a single parameter (the rest
  // can be inferred from information we already have).
  mOut.indent() << "// Constructor\n";
  startFunction(AM_Public, false, nullptr, getClassName(), 1, "RenderScript",
                "rs");

  const bool haveReduceExportables =
    mRSContext->export_reduce_begin() != mRSContext->export_reduce_end();

  if (getEmbedBitcodeInJava()) {
    // Call new single argument Java-only constructor
    mOut.indent() << "super(rs,\n";
    mOut.indent() << "      " << RS_RESOURCE_NAME ",\n";
    mOut.indent() << "      " << className << ".getBitCode32(),\n";
    mOut.indent() << "      " << className << ".getBitCode64());\n";
  } else {
    // Call alternate constructor with required parameters.
    // Look up the proper raw bitcode resource id via the context.
    mOut.indent() << "this(rs,\n";
    mOut.indent() << "     rs.getApplicationContext().getResources(),\n";
    mOut.indent() << "     rs.getApplicationContext().getResources()."
                     "getIdentifier(\n";
    mOut.indent() << "         " RS_RESOURCE_NAME ", \"raw\",\n";
    mOut.indent()
        << "         rs.getApplicationContext().getPackageName()));\n";
    endFunction();

    // Alternate constructor (legacy) with 3 original parameters.
    startFunction(AM_Public, false, nullptr, getClassName(), 3, "RenderScript",
                  "rs", "Resources", "resources", "int", "id");
    // Call constructor of super class
    mOut.indent() << "super(rs, resources, id);\n";
  }

  // If an exported variable has initial value, reflect it.
  // Keep this in sync with initialization handling in ReflectionState::declareVariable().

  for (auto I = mRSContext->export_vars_begin(),
            E = mRSContext->export_vars_end();
       I != E; I++) {
    const RSExportVar *EV = *I;
    if (!EV->getInit().isUninit()) {
      genInitExportVariable(EV->getType(), EV->getName(), EV->getInit());
    } else if (EV->getArraySize()) {
      // Always create an initial zero-init array object.
      mOut.indent() << RS_EXPORT_VAR_PREFIX << EV->getName() << " = new "
                    << GetTypeName(EV->getType(), TypeNameDefault & ~TypeNameWithConstantArrayBrackets) << "["
                    << EV->getArraySize() << "];\n";
      size_t NumInits = EV->getNumInits();
      const RSExportConstantArrayType *ECAT =
          static_cast<const RSExportConstantArrayType *>(EV->getType());
      const RSExportType *ET = ECAT->getElementType();
      for (size_t i = 0; i < NumInits; i++) {
        std::stringstream Name;
        Name << EV->getName() << "[" << i << "]";
        genInitExportVariable(ET, Name.str(), EV->getInitArray(i));
      }
    }
    if (mRSContext->getTargetAPI() >= SLANG_JB_TARGET_API) {
      genTypeInstance(EV->getType());
    }
    genFieldPackerInstance(EV->getType());
  }

  if (haveReduceExportables) {
    mOut.indent() << SAVED_RS_REFERENCE << " = rs;\n";
  }

  // Reflect argument / return types in kernels

  for (auto I = mRSContext->export_foreach_begin(),
            E = mRSContext->export_foreach_end();
       I != E; I++) {
    const RSExportForEach *EF = *I;

    const RSExportForEach::InTypeVec &InTypes = EF->getInTypes();
    for (RSExportForEach::InTypeIter BI = InTypes.begin(), EI = InTypes.end();
         BI != EI; BI++) {
      if (*BI != nullptr) {
        genTypeInstanceFromPointer(*BI);
      }
    }

    const RSExportType *OET = EF->getOutType();
    if (OET) {
      genTypeInstanceFromPointer(OET);
    }
  }

  for (auto I = mRSContext->export_reduce_begin(),
            E = mRSContext->export_reduce_end();
       I != E; I++) {
    const RSExportReduce *ER = *I;

    const RSExportType *RT = ER->getResultType();
    slangAssert(RT != nullptr);
    if (!exportableReduce(RT))
      continue;

    genTypeInstance(RT);

    const RSExportReduce::InTypeVec &InTypes = ER->getAccumulatorInTypes();
    for (RSExportReduce::InTypeIter BI = InTypes.begin(), EI = InTypes.end();
         BI != EI; BI++) {
      slangAssert(*BI != nullptr);
      genTypeInstance(*BI);
    }
  }

  endFunction();

  for (std::set<std::string>::iterator I = mTypesToCheck.begin(),
                                       E = mTypesToCheck.end();
       I != E; I++) {
    mOut.indent() << "private Element " RS_ELEM_PREFIX << *I << ";\n";
  }

  for (std::set<std::string>::iterator I = mFieldPackerTypes.begin(),
                                       E = mFieldPackerTypes.end();
       I != E; I++) {
    mOut.indent() << "private FieldPacker " RS_FP_PREFIX << *I << ";\n";
  }

  if (haveReduceExportables) {
    // We save a private copy of rs in order to create temporary
    // allocations in the reduce_* entry points.
    mOut.indent() << "private RenderScript " << SAVED_RS_REFERENCE << ";\n";
  }
}

void RSReflectionJava::genInitBoolExportVariable(const std::string &VarName,
                                                 const clang::APValue &Val) {
  slangAssert(!Val.isUninit() && "Not a valid initializer");
  slangAssert((Val.getKind() == clang::APValue::Int) &&
              "Bool type has wrong initial APValue");

  mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = ";

  mOut << ((Val.getInt().getSExtValue() == 0) ? "false" : "true") << ";\n";
}

void
RSReflectionJava::genInitPrimitiveExportVariable(const std::string &VarName,
                                                 const clang::APValue &Val) {
  slangAssert(!Val.isUninit() && "Not a valid initializer");

  mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = ";
  genInitValue(Val, false);
  mOut << ";\n";
}

void RSReflectionJava::genInitExportVariable(const RSExportType *ET,
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
      std::cout << "Initializer which is non-NULL to pointer type variable "
                   "will be ignored\n";
    break;
  }
  case RSExportType::ExportClassVector: {
    const RSExportVectorType *EVT = static_cast<const RSExportVectorType *>(ET);
    switch (Val.getKind()) {
    case clang::APValue::Int:
    case clang::APValue::Float: {
      for (unsigned i = 0; i < EVT->getNumElement(); i++) {
        std::string Name = VarName + "." + GetVectorAccessor(i);
        genInitPrimitiveExportVariable(Name, Val);
      }
      break;
    }
    case clang::APValue::Vector: {
      std::stringstream VecName;
      VecName << EVT->getRSReflectionType(EVT)->rs_java_vector_prefix
              << EVT->getNumElement();
      mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = new "
                    << VecName.str() << "();\n";

      unsigned NumElements = std::min(
          static_cast<unsigned>(EVT->getNumElement()), Val.getVectorLength());
      for (unsigned i = 0; i < NumElements; i++) {
        const clang::APValue &ElementVal = Val.getVectorElt(i);
        std::string Name = VarName + "." + GetVectorAccessor(i);
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
  // TODO(zonr): Resolving initializer of a record (and matrix) type variable
  // is complex. It cannot obtain by just simply evaluating the initializer
  // expression.
  case RSExportType::ExportClassMatrix:
  case RSExportType::ExportClassConstantArray:
  case RSExportType::ExportClassRecord: {
#if 0
      unsigned InitIndex = 0;
      const RSExportRecordType *ERT =
          static_cast<const RSExportRecordType*>(ET);

      slangAssert((Val.getKind() == clang::APValue::Vector) &&
          "Unexpected type of initializer for record type variable");

      mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName
                 << " = new " << ERT->getElementName()
                 <<  "." RS_TYPE_ITEM_CLASS_NAME"();\n";

      for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(),
               E = ERT->fields_end();
           I != E;
           I++) {
        const RSExportRecordType::Field *F = *I;
        std::string FieldName = VarName + "." + F->getName();

        if (InitIndex > Val.getVectorLength())
          break;

        genInitPrimitiveExportVariable(FieldName,
                                       Val.getVectorElt(InitIndex++));
      }
#endif
    slangAssert(false && "Unsupported initializer for record/matrix/constant "
                         "array type variable currently");
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
}

void RSReflectionJava::genExportVariable(const RSExportVar *EV) {
  const RSExportType *ET = EV->getType();

  const ReflectionState::Val32
      AllocSize32 = mState->declareVariable(EV);

  if (mCollecting)
    return;

  mOut.indent() << "private final static int " << RS_EXPORT_VAR_INDEX_PREFIX
                << EV->getName() << " = " << getNextExportVarSlot() << ";\n";

  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    genPrimitiveTypeExportVariable(EV);
    break;
  }
  case RSExportType::ExportClassPointer: {
    genPointerTypeExportVariable(EV);
    break;
  }
  case RSExportType::ExportClassVector: {
    genVectorTypeExportVariable(EV);
    break;
  }
  case RSExportType::ExportClassMatrix: {
    genMatrixTypeExportVariable(EV);
    break;
  }
  case RSExportType::ExportClassConstantArray: {
    genConstantArrayTypeExportVariable(EV, AllocSize32);
    break;
  }
  case RSExportType::ExportClassRecord: {
    genRecordTypeExportVariable(EV, AllocSize32);
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
}

// Keep this in sync with Invokable analysis in ReflectionState::declareInvokable().
void RSReflectionJava::genExportFunction(const RSExportFunc *EF) {
  mState->declareInvokable(EF);

  if (!mCollecting) {
    mOut.indent() << "private final static int " << RS_EXPORT_FUNC_INDEX_PREFIX
                  << EF->getName() << " = " << getNextExportFuncSlot() << ";\n";
  }

  // invoke_*()
  ArgTy Args;

  if (!mCollecting) {
    if (EF->hasParam()) {
      for (RSExportFunc::const_param_iterator I = EF->params_begin(),
                                              E = EF->params_end();
           I != E; I++) {
        Args.push_back(
            std::make_pair(GetTypeName((*I)->getType()), (*I)->getName()));
      }
    }

    if (mRSContext->getTargetAPI() >= SLANG_M_TARGET_API) {
      startFunction(AM_Public, false, "Script.InvokeID",
                    "getInvokeID_" + EF->getName(), 0);

      mOut.indent() << "return createInvokeID(" << RS_EXPORT_FUNC_INDEX_PREFIX
                    << EF->getName() << ");\n";

      endFunction();
    }

    startFunction(AM_Public, false, "void",
                  "invoke_" + EF->getName(/*Mangle=*/false),
                  // We are using un-mangled name since Java
                  // supports method overloading.
                  Args);
  }

  if (!EF->hasParam()) {
    if (!mCollecting)
      mOut.indent() << "invoke(" << RS_EXPORT_FUNC_INDEX_PREFIX << EF->getName()
                    << ");\n";
  } else {
    const RSExportRecordType *ERT = EF->getParamPacketType();

    // NOTE: This type isn't on the RSContext::export_types* list.
    mState->declareRecord(ERT, false);

    std::string FieldPackerName = EF->getName() + "_fp";

    if (genCreateFieldPacker(ERT, FieldPackerName.c_str(),
                             mState->getRecord32(ERT).getRecordAllocSize()))
      genPackVarOfType(ERT, nullptr, FieldPackerName.c_str());

    if (!mCollecting)
      mOut.indent() << "invoke(" << RS_EXPORT_FUNC_INDEX_PREFIX << EF->getName()
                    << ", " << FieldPackerName << ");\n";
  }

  if (!mCollecting)
    endFunction();
}

void RSReflectionJava::genPairwiseDimCheck(const std::string &name0,
                                           const std::string &name1) {
  mOut.indent() << "// Verify dimensions\n";
  mOut.indent() << "t0 = " << name0 << ".getType();\n";
  mOut.indent() << "t1 = " << name1 << ".getType();\n";
  mOut.indent() << "if ((t0.getCount() != t1.getCount()) ||\n";
  mOut.indent() << "    (t0.getX() != t1.getX()) ||\n";
  mOut.indent() << "    (t0.getY() != t1.getY()) ||\n";
  mOut.indent() << "    (t0.getZ() != t1.getZ()) ||\n";
  mOut.indent() << "    (t0.hasFaces()   != t1.hasFaces()) ||\n";
  mOut.indent() << "    (t0.hasMipmaps() != t1.hasMipmaps())) {\n";
  mOut.indent() << "    throw new RSRuntimeException(\"Dimension mismatch "
                << "between parameters " << name0 << " and " << name1
                << "!\");\n";
  mOut.indent() << "}\n\n";
}

void RSReflectionJava::genNullArrayCheck(const std::string &ArrayName) {
  mOut.indent() << "// Verify that \"" << ArrayName << "\" is non-null.\n";
  mOut.indent() << "if (" << ArrayName << " == null) {\n";
  mOut.indent() << "    throw new RSIllegalArgumentException(\"Array \\\""
                << ArrayName << "\\\" is null!\");\n";
  mOut.indent() << "}\n";
}

void RSReflectionJava::genVectorLengthCompatibilityCheck(const std::string &ArrayName,
                                                         unsigned VecSize) {
  mOut.indent() << "// Verify that the array length is a multiple of the vector size.\n";
  mOut.indent() << "if (" << ArrayName << ".length % " << std::to_string(VecSize)
                << " != 0) {\n";
  mOut.indent() << "    throw new RSIllegalArgumentException(\"Array \\\"" << ArrayName
                << "\\\" is not a multiple of " << std::to_string(VecSize)
                << " in length!\");\n";
  mOut.indent() << "}\n";
}

// Keep this in sync with ForEach analysis in ReflectionState::beginForEach()
// and other ReflectionState::*ForEach*() methods.
void RSReflectionJava::genExportForEach(const RSExportForEach *EF) {
  if (EF->isDummyRoot()) {
    mState->declareForEachDummyRoot(EF);

    if (!mCollecting) {
      // Skip reflection for dummy root() kernels. Note that we have to
      // advance the next slot number for ForEach, however.
      mOut.indent() << "//private final static int "
                    << RS_EXPORT_FOREACH_INDEX_PREFIX << EF->getName() << " = "
                    << getNextExportForEachSlot() << ";\n";
    }

    return;
  }

  if (!mCollecting) {
    mOut.indent() << "private final static int " << RS_EXPORT_FOREACH_INDEX_PREFIX
                  << EF->getName() << " = " << getNextExportForEachSlot()
                  << ";\n";
  }

  // forEach_*()
  ArgTy Args;
  bool HasAllocation = false; // at least one in/out allocation?

  const RSExportForEach::InVec     &Ins     = EF->getIns();
  const RSExportForEach::InTypeVec &InTypes = EF->getInTypes();
  const RSExportType               *OET     = EF->getOutType();
  const RSExportRecordType         *ERT     = EF->getParamPacketType();

  mState->beginForEach(EF);

  for (RSExportForEach::InTypeIter BI = InTypes.begin(), EI = InTypes.end();
       BI != EI; BI++) {
    mState->addForEachIn(EF, *BI);
  }

  if (Ins.size() == 1) {
    HasAllocation = true;
    if (!mCollecting)
      Args.push_back(std::make_pair("Allocation", "ain"));
  } else if (Ins.size() > 1) {
    HasAllocation = true;
    if (!mCollecting) {
      for (RSExportForEach::InIter BI = Ins.begin(), EI = Ins.end(); BI != EI;
           BI++) {
        Args.push_back(std::make_pair("Allocation",
                                      "ain_" + (*BI)->getName().str()));
      }
    }
  }

  if (EF->hasOut() || EF->hasReturn()) {
    HasAllocation = true;
    if (!mCollecting)
      Args.push_back(std::make_pair("Allocation", "aout"));
  }

  if (ERT) {
    for (RSExportForEach::const_param_iterator I = EF->params_begin(),
                                               E = EF->params_end();
         I != E; I++) {
      mState->addForEachParam(EF, (*I)->getType());
      if (!mCollecting)
        Args.push_back(
            std::make_pair(GetTypeName((*I)->getType()), (*I)->getName()));
    }
  }

  if (mRSContext->getTargetAPI() >= SLANG_JB_MR1_TARGET_API) {
    mState->addForEachSignatureMetadata(EF, EF->getSignatureMetadata());

    if (!mCollecting) {
      startFunction(AM_Public, false, "Script.KernelID",
                    "getKernelID_" + EF->getName(), 0);

      // TODO: add element checking
      mOut.indent() << "return createKernelID(" << RS_EXPORT_FOREACH_INDEX_PREFIX
                    << EF->getName() << ", " << EF->getSignatureMetadata()
                    << ", null, null);\n";

      endFunction();
    }
  }

  if (!mCollecting) {
    if (mRSContext->getTargetAPI() >= SLANG_JB_MR2_TARGET_API) {
      if (HasAllocation) {
        startFunction(AM_Public, false, "void", "forEach_" + EF->getName(), Args);

        mOut.indent() << "forEach_" << EF->getName();
        mOut << "(";

        if (Ins.size() == 1) {
          mOut << "ain, ";

        } else if (Ins.size() > 1) {
          for (RSExportForEach::InIter BI = Ins.begin(), EI = Ins.end(); BI != EI;
               BI++) {

            mOut << "ain_" << (*BI)->getName().str() << ", ";
          }
        }

        if (EF->hasOut() || EF->hasReturn()) {
          mOut << "aout, ";
        }

        if (EF->hasUsrData()) {
          mOut << Args.back().second << ", ";
        }

        // No clipped bounds to pass in.
        mOut << "null);\n";

        endFunction();
      }

      // Add the clipped kernel parameters to the Args list.
      Args.push_back(std::make_pair("Script.LaunchOptions", "sc"));
    }
  }

  if (!mCollecting) {
    startFunction(AM_Public, false, "void", "forEach_" + EF->getName(), Args);

    if (InTypes.size() == 1) {
      if (InTypes.front() != nullptr) {
        genTypeCheck(InTypes.front(), "ain");
      }

    } else if (InTypes.size() > 1) {
      size_t Index = 0;
      for (RSExportForEach::InTypeIter BI = InTypes.begin(), EI = InTypes.end();
           BI != EI; BI++, ++Index) {

        if (*BI != nullptr) {
          genTypeCheck(*BI, ("ain_" + Ins[Index]->getName()).str().c_str());
        }
      }
    }

    if (OET) {
      genTypeCheck(OET, "aout");
    }

    if (Ins.size() == 1 && (EF->hasOut() || EF->hasReturn())) {
      mOut.indent() << "Type t0, t1;";
      genPairwiseDimCheck("ain", "aout");

    } else if (Ins.size() > 1) {
      mOut.indent() << "Type t0, t1;";

      std::string In0Name = "ain_" + Ins[0]->getName().str();

      for (size_t index = 1; index < Ins.size(); ++index) {
        genPairwiseDimCheck(In0Name, "ain_" + Ins[index]->getName().str());
      }

      if (EF->hasOut() || EF->hasReturn()) {
        genPairwiseDimCheck(In0Name, "aout");
      }
    }
  }

  std::string FieldPackerName = EF->getName() + "_fp";
  if (ERT) {
    // NOTE: This type isn't on the RSContext::export_types* list.
    mState->declareRecord(ERT, false);

    if (genCreateFieldPacker(ERT, FieldPackerName.c_str(),
                             mState->getRecord32(ERT).getRecordAllocSize())) {
      genPackVarOfType(ERT, nullptr, FieldPackerName.c_str());
    }
  }

  mState->endForEach();

  if (mCollecting)
    return;

  mOut.indent() << "forEach(" << RS_EXPORT_FOREACH_INDEX_PREFIX
                << EF->getName();

  if (Ins.size() == 1) {
    mOut << ", ain";
  } else if (Ins.size() > 1) {
    mOut << ", new Allocation[]{ain_" << Ins[0]->getName().str();

    for (size_t index = 1; index < Ins.size(); ++index) {
      mOut << ", ain_" << Ins[index]->getName().str();
    }

    mOut << "}";

  } else {
    mOut << ", (Allocation) null";
  }

  if (EF->hasOut() || EF->hasReturn())
    mOut << ", aout";
  else
    mOut << ", null";

  if (EF->hasUsrData())
    mOut << ", " << FieldPackerName;
  else
    mOut << ", null";

  if (mRSContext->getTargetAPI() >= SLANG_JB_MR2_TARGET_API) {
    mOut << ", sc);\n";
  } else {
    mOut << ");\n";
  }

  endFunction();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// Reductions with certain legal result types can only be reflected for NDK, not for Java.
bool RSReflectionJava::exportableReduce(const RSExportType *ResultType) {
  const RSExportType *CheckType = ResultType;
  if (ResultType->getClass() == RSExportType::ExportClassConstantArray)
    CheckType = static_cast<const RSExportConstantArrayType *>(ResultType)->getElementType();
  if (CheckType->getClass() == RSExportType::ExportClassRecord) {
    // No Java reflection for struct until http://b/22236498 is resolved.
    return false;
  }

  return true;
}

namespace {
enum MappingComment { MappingCommentWithoutType, MappingCommentWithCType };

// OUTPUTS
//   InputParamName      = name to use for input parameter
//   InputMappingComment = text showing the mapping from InputParamName to the corresponding
//                           accumulator function parameter name (and possibly type)
// INPUTS
//   NamePrefix          = beginning of parameter name (e.g., "in")
//   MappingComment      = whether or not InputMappingComment should contain type
//   ER                  = description of the reduction
//   InIdx               = which input (numbered from zero)
void getReduceInputStrings(std::string &InputParamName, std::string &InputMappingComment,
                           const std::string &NamePrefix, MappingComment Mapping,
                           const RSExportReduce *ER, size_t InIdx) {
  InputParamName = NamePrefix + std::to_string(InIdx+1);
  std::string TypeString;
  if (Mapping == MappingCommentWithCType) {
    const RSExportType *InType = ER->getAccumulatorInTypes()[InIdx];
    if (InType->getClass() == RSExportType::ExportClassRecord) {
      // convertToRTD doesn't understand this type
      TypeString = "/* struct <> */ ";
    } else {
      RSReflectionTypeData InTypeData;
      ER->getAccumulatorInTypes()[InIdx]->convertToRTD(&InTypeData);
      slangAssert(InTypeData.type->s_name != nullptr);
      if (InTypeData.vecSize > 1) {
        TypeString = InTypeData.type->s_name + std::to_string(InTypeData.vecSize) + " ";
      } else {
        TypeString = InTypeData.type->s_name + std::string(" ");
      }
    }
  }
  InputMappingComment = InputParamName + " = \"" + TypeString + std::string(ER->getAccumulatorIns()[InIdx]->getName()) + "\"";
}

} // end anonymous namespace

// Keep this in sync with Reduce analysis in ReflectionState::declareReduce().
void RSReflectionJava::genExportReduce(const RSExportReduce *ER) {
  const bool IsExportable = exportableReduce(ER->getResultType());

  // Need to track even a non-exportable reduce, both so that we get
  // the count of reduction kernels correct, and so that we can
  // intelligently diagnose cases where 32-bit and 64-bit compiles
  // disagree as to whether a reduction kernel is exportable.
  mState->declareReduce(ER, IsExportable);

  if (!IsExportable || mCollecting)
    return;

  // Generate the reflected function index.
  mOut.indent() << "private final static int " << RS_EXPORT_REDUCE_INDEX_PREFIX
                << ER->getNameReduce() << " = " << getNextExportReduceSlot()
                << ";\n";

  /****** remember resultSvType generation **********************************************************/

  // Two variants of reduce_* entry points get generated.
  // Array variant:
  //   result_<resultSvType> reduce_<name>(<devecSiIn1Type>[] in1, ..., <devecSiInNType>[] inN)
  // Allocation variant:
  //   result_<resultSvType> reduce_<name>(Allocation in1, ..., Allocation inN)
  //   result_<resultSvType> reduce_<name>(Allocation in1, ..., Allocation inN, Script.LaunchOptions sc)

  genExportReduceArrayVariant(ER);
  genExportReduceAllocationVariant(ER);
}

void RSReflectionJava::genExportReduceArrayVariant(const RSExportReduce *ER) {
  // Analysis of result type.  Returns early if result type is not
  // suitable for array method reflection.
  const RSExportType *const ResultType = ER->getResultType();
  auto ResultTypeClass = ResultType->getClass();
  switch (ResultTypeClass) {
      case RSExportType::ExportClassConstantArray:
      case RSExportType::ExportClassMatrix:
      case RSExportType::ExportClassPrimitive:
      case RSExportType::ExportClassVector:
        // Ok
        break;

      case RSExportType::ExportClassPointer:
        slangAssert(!"Should not get here with pointer type");
        return;

      case RSExportType::ExportClassRecord:
        // TODO: convertToRTD() cannot handle this.  Why not?
        return;

      default:
        slangAssert(!"Unknown export class");
        return;
  }
  RSReflectionTypeData ResultTypeData;
  ResultType->convertToRTD(&ResultTypeData);
  if (!ResultTypeData.type->java_name || !ResultTypeData.type->java_array_element_name ||
      (ResultTypeData.vecSize > 1 && !ResultTypeData.type->rs_java_vector_prefix)) {
    slangAssert(false);
    return;
  }
  const std::string ResultTypeName = GetReduceResultTypeName(ER);

  // Analysis of inputs.  Returns early if some input type is not
  // suitable for array method reflection.
  llvm::SmallVector<RSReflectionTypeData, 1> InsTypeData;
  ArgTy Args;
  const auto &Ins = ER->getAccumulatorIns();
  const auto &InTypes = ER->getAccumulatorInTypes();
  slangAssert(Ins.size() == InTypes.size());
  InsTypeData.resize(Ins.size());
  llvm::SmallVector<std::string, 1> InComments;
  for (size_t InIdx = 0, InEnd = Ins.size(); InIdx < InEnd; ++InIdx) {
    const RSExportType *const InType = InTypes[InIdx];
    switch (InType->getClass()) {
      case RSExportType::ExportClassMatrix:
      case RSExportType::ExportClassPrimitive:
      case RSExportType::ExportClassVector:
        // Ok
        break;

      case RSExportType::ExportClassConstantArray:
        // No
        return;

      case RSExportType::ExportClassPointer:
        slangAssert(!"Should not get here with pointer type");
        return;

      case RSExportType::ExportClassRecord:
        // TODO: convertToRTD() cannot handle this.  Why not?
        return;

      default:
        slangAssert(!"Unknown export class");
        return;
    }

    RSReflectionTypeData &InTypeData = InsTypeData[InIdx];
    InType->convertToRTD(&InTypeData);
    if (!InTypeData.type->java_name || !InTypeData.type->java_array_element_name ||
        (InTypeData.vecSize > 1 && !InTypeData.type->rs_java_vector_prefix)) {
      return;
    }

    std::string InputParamName, InputComment;
    getReduceInputStrings(InputParamName, InputComment, "in", MappingCommentWithoutType, ER, InIdx);
    if (InTypeData.vecSize > 1)
      InputComment += (", flattened " + std::to_string(InTypeData.vecSize) + "-vectors");
    InComments.push_back(InputComment);

    const std::string InputTypeName = std::string(InTypeData.type->java_array_element_name) + "[]";
    Args.push_back(std::make_pair(InputTypeName, InputParamName));
  }

  const std::string MethodName = "reduce_" + ER->getNameReduce();

  // result_<resultSvType> reduce_<name>(<devecSiIn1Type>[] in1, ..., <devecSiInNType>[] inN)

  for (const std::string &InComment : InComments)
    mOut.indent() << "// " << InComment << "\n";
  startFunction(AM_Public, false, ResultTypeName.c_str(), MethodName, Args);
  slangAssert(Ins.size() == InTypes.size());
  slangAssert(Ins.size() == InsTypeData.size());
  slangAssert(Ins.size() == Args.size());
  std::string In1Length;
  std::string InputAllocationOutgoingArgumentList;
  std::vector<std::string> InputAllocationNames;
  for (size_t InIdx = 0, InEnd = Ins.size(); InIdx < InEnd; ++InIdx) {
    const std::string &ArgName = Args[InIdx].second;
    genNullArrayCheck(ArgName);
    std::string InLength = ArgName + ".length";
    const uint32_t VecSize = InsTypeData[InIdx].vecSize;
    if (VecSize > 1) {
      InLength += " / " + std::to_string(VecSize);
      genVectorLengthCompatibilityCheck(ArgName, VecSize);
    }
    if (InIdx == 0) {
      In1Length = InLength;
    } else {
      mOut.indent() << "// Verify that input array lengths are the same.\n";
      mOut.indent() << "if (" << In1Length << " != " << InLength << ") {\n";
      mOut.indent() << "    throw new RSRuntimeException(\"Array length mismatch "
                    << "between parameters \\\"" << Args[0].second << "\\\" and \\\"" << ArgName
                    << "\\\"!\");\n";
      mOut.indent() << "}\n";
    }
    // Create a temporary input allocation
    const std::string TempName = "a" + ArgName;
    mOut.indent() << "Allocation " << TempName << " = Allocation.createSized("
                  << SAVED_RS_REFERENCE << ", "
                  << RS_ELEM_PREFIX << InTypes[InIdx]->getElementName() << ", "
                  << InLength << ");\n";
    mOut.indent() << TempName << ".setAutoPadding(true);\n";
    mOut.indent() << TempName << ".copyFrom(" << ArgName << ");\n";
    // ... and put that input allocation on the outgoing argument list
    if (!InputAllocationOutgoingArgumentList.empty())
      InputAllocationOutgoingArgumentList += ", ";
    InputAllocationOutgoingArgumentList += TempName;
    // ... and keep track of it for setting result.mTempIns
    InputAllocationNames.push_back(TempName);
  }

  mOut << "\n";
  mOut.indent() << ResultTypeName << " result = " << MethodName << "(" << InputAllocationOutgoingArgumentList << ", null);\n";
  if (!InputAllocationNames.empty()) {
    mOut.indent() << "result.mTempIns = new Allocation[]{";
    bool EmittedFirst = false;
    for (const std::string &InputAllocationName : InputAllocationNames) {
      if (!EmittedFirst) {
        EmittedFirst = true;
      } else {
        mOut << ", ";
      }
      mOut << InputAllocationName;
    }
    mOut << "};\n";
  }
  mOut.indent() << "return result;\n";
  endFunction();
}

void RSReflectionJava::genExportReduceAllocationVariant(const RSExportReduce *ER) {
  const auto &Ins = ER->getAccumulatorIns();
  const auto &InTypes = ER->getAccumulatorInTypes();
  const RSExportType *ResultType = ER->getResultType();

  llvm::SmallVector<std::string, 1> InComments;
  ArgTy Args;
  for (size_t InIdx = 0, InEnd = Ins.size(); InIdx < InEnd; ++InIdx) {
    std::string InputParamName, InputComment;
    getReduceInputStrings(InputParamName, InputComment, "ain", MappingCommentWithCType, ER, InIdx);
    InComments.push_back(InputComment);
    Args.push_back(std::make_pair("Allocation", InputParamName));
  }

  const std::string MethodName = "reduce_" + ER->getNameReduce();
  const std::string ResultTypeName = GetReduceResultTypeName(ER);

  // result_<resultSvType> reduce_<name>(Allocation in1, ..., Allocation inN)

  for (const std::string &InComment : InComments)
    mOut.indent() << "// " << InComment << "\n";
  startFunction(AM_Public, false, ResultTypeName.c_str(), MethodName, Args);
  mOut.indent() << "return " << MethodName << "(";
  bool EmittedFirstArg = false;
  for (const auto &Arg : Args) {
    if (!EmittedFirstArg) {
      EmittedFirstArg = true;
    } else {
      mOut << ", ";
    }
    mOut << Arg.second;
  }
  mOut << ", null);\n";
  endFunction();

  // result_<resultSvType> reduce_<name>(Allocation in1, ..., Allocation inN, Script.LaunchOptions sc)

  static const char FormalOptionsName[] = "sc";
  Args.push_back(std::make_pair("Script.LaunchOptions", FormalOptionsName));
  for (const std::string &InComment : InComments)
    mOut.indent() << "// " << InComment << "\n";
  startFunction(AM_Public, false, ResultTypeName.c_str(), MethodName, Args);
  const std::string &In0Name = Args[0].second;
  // Sanity-check inputs
  if (Ins.size() > 1)
    mOut.indent() << "Type t0, t1;\n";
  for (size_t InIdx = 0, InEnd = Ins.size(); InIdx < InEnd; ++InIdx) {
    const std::string &InName = Args[InIdx].second;
    genTypeCheck(InTypes[InIdx], InName.c_str());
    if (InIdx > 0)
      genPairwiseDimCheck(In0Name, InName);
  }
  // Create a temporary output allocation
  const char OutputAllocName[] = "aout";
  const size_t OutputAllocLength =
      ResultType->getClass() == RSExportType::ExportClassConstantArray
      ? static_cast<const RSExportConstantArrayType *>(ResultType)->getNumElement()
      : 1;
  mOut.indent() << "Allocation " << OutputAllocName << " = Allocation.createSized("
                << SAVED_RS_REFERENCE << ", "
                << RS_ELEM_PREFIX << ResultType->getElementName() << ", "
                << OutputAllocLength << ");\n";
  mOut.indent() << OutputAllocName << ".setAutoPadding(true);\n";
  // Call the underlying reduce entry point
  mOut.indent() << "reduce(" << RS_EXPORT_REDUCE_INDEX_PREFIX << ER->getNameReduce()
                << ", new Allocation[]{" << In0Name;
  for (size_t InIdx = 1, InEnd = Ins.size(); InIdx < InEnd; ++InIdx)
    mOut << ", " << Args[InIdx].second;
  mOut << "}, " << OutputAllocName << ", " << FormalOptionsName << ");\n";
  mOut.indent() << "return new " << ResultTypeName << "(" << OutputAllocName << ");\n";
  endFunction();
}

namespace {

// When we've copied the Allocation to a Java array, how do we
// further process the elements of that array?
enum MapFromAllocation {
  MapFromAllocationTrivial,  // no further processing
  MapFromAllocationPositive, // need to ensure elements are positive (range check)
  MapFromAllocationBoolean,  // need to convert elements from byte to boolean
  MapFromAllocationPromote   // need to zero extend elements
};

// Return Java expression that maps from an Allocation element to a Java non-vector result.
//
// MFA                     = mapping kind
// ArrayElementTypeName    = type of InVal (having been copied out of Allocation to Java array)
// ReflectedScalarTypeName = type of mapped value
// InVal                   = input value that must be mapped
//
std::string genReduceResultMapping(MapFromAllocation MFA,
                                   const std::string &ArrayElementTypeName,
                                   const std::string &ReflectedScalarTypeName,
                                   const char *InVal) {
  switch (MFA) {
    default:
      slangAssert(!"Unknown MapFromAllocation");
      // and fall through
    case MapFromAllocationPositive: // range checking must be done separately
    case MapFromAllocationTrivial:
      return InVal;
    case MapFromAllocationBoolean:
      return std::string(InVal) + std::string(" != 0");
    case MapFromAllocationPromote:
      return ZeroExtendValue(InVal,
                             ArrayElementTypeName,
                             ReflectedScalarTypeName);
  }
}

// Return Java expression that maps from an Allocation element to a Java vector result.
//
// MFA                     = mapping kind
// ArrayElementTypeName    = type of InVal (having been copied out of Allocation to Java array)
// ReflectedScalarTypeName = type of mapped value
// VectorTypeName          = type of vector
// VectorElementCount      = number of elements in the vector
// InArray                 = input array containing vector elements
// InIdx                   = index of first vector element within InArray (or nullptr, if 0)
//
std::string genReduceResultVectorMapping(MapFromAllocation MFA,
                                         const std::string &ArrayElementTypeName,
                                         const std::string &ReflectedScalarTypeName,
                                         const std::string &VectorTypeName,
                                         unsigned VectorElementCount,
                                         const char *InArray, const char *InIdx = nullptr) {
  std::string result = "new " + VectorTypeName + "(";
  for (unsigned VectorElementIdx = 0; VectorElementIdx < VectorElementCount; ++VectorElementIdx) {
    if (VectorElementIdx)
     result += ", ";

    std::string ArrayElementName = std::string(InArray) + "[";
    if (InIdx)
      ArrayElementName += std::string(InIdx) + "+";
    ArrayElementName += std::to_string(VectorElementIdx) + "]";

    result += genReduceResultMapping(MFA, ArrayElementTypeName, ReflectedScalarTypeName,
                                     ArrayElementName.c_str());
  }
  result += ")";
  return result;
}

void genReduceResultRangeCheck(GeneratedFile &Out, const char *InVal) {
  Out.indent() << "if (" << InVal << " < 0)\n";
  Out.indent() << "    throw new RSRuntimeException(\"Result is not representible in Java\");\n";
}

} // end anonymous namespace

void RSReflectionJava::genExportReduceResultType(const RSExportType *ResultType) {
  if (!exportableReduce(ResultType))
    return;

  const std::string ClassName = GetReduceResultTypeName(ResultType);
  const std::string GetMethodReturnTypeName = GetTypeName(ResultType);
  mOut.indent() << "// To obtain the result, invoke get(), which blocks\n";
  mOut.indent() << "// until the asynchronously-launched operation has completed.\n";
  mOut.indent() << "public static class " << ClassName;
  mOut.startBlock();
  startFunction(AM_Public, false, GetMethodReturnTypeName.c_str(), "get", 0);

  RSReflectionTypeData TypeData;
  ResultType->convertToRTD(&TypeData);

  const std::string UnbracketedResultTypeName =
      GetTypeName(ResultType, TypeNameDefault & ~TypeNameWithConstantArrayBrackets);
  const std::string ReflectedScalarTypeName = TypeData.type->java_name;
  // Note: MATRIX* types do not have a java_array_element_name
  const std::string ArrayElementTypeName =
      TypeData.type->java_array_element_name
      ? std::string(TypeData.type->java_array_element_name)
      : ReflectedScalarTypeName;

  MapFromAllocation MFA = MapFromAllocationTrivial;
  if (std::string(TypeData.type->rs_type) == "UNSIGNED_64")
    MFA = MapFromAllocationPositive;
  else if (ReflectedScalarTypeName == "boolean")
    MFA = MapFromAllocationBoolean;
  else if (ReflectedScalarTypeName != ArrayElementTypeName)
    MFA = MapFromAllocationPromote;

  mOut.indent() << "if (!mGotResult)";
  mOut.startBlock();

  if (TypeData.vecSize == 1) { // result type is non-vector
    // <ArrayElementType>[] outArray = new <ArrayElementType>[1];
    // mOut.copyTo(outArray);
    mOut.indent() << ArrayElementTypeName << "[] outArray = new " << ArrayElementTypeName
                  << "[" << std::max(TypeData.arraySize, 1U) << "];\n";
    mOut.indent() << "mOut.copyTo(outArray);\n";
    if (TypeData.arraySize == 0) { // result type is non-array non-vector
      // mResult = outArray[0]; // but there are several special cases
      if (MFA == MapFromAllocationPositive)
        genReduceResultRangeCheck(mOut, "outArray[0]");
      mOut.indent() << "mResult = "
                    << genReduceResultMapping(MFA, ArrayElementTypeName, ReflectedScalarTypeName,
                                              "outArray[0]")
                    << ";\n";
    } else { // result type is array of non-vector
      if (MFA == MapFromAllocationTrivial) {
        // mResult = outArray;
        mOut.indent() << "mResult = outArray;\n";
      } else {
        // <ResultType> result = new <UnbracketedResultType>[<ArrayElementCount>];
        // for (unsigned Idx = 0; Idx < <ArrayElementCount>; ++Idx)
        //   result[Idx] = <Transform>(outArray[Idx]);
        // mResult = result; // but there are several special cases
        if (MFA != MapFromAllocationPositive) {
          mOut.indent() << GetTypeName(ResultType) << " result = new "
                        << UnbracketedResultTypeName
                        << "[" << TypeData.arraySize << "];\n";
        }
        mOut.indent() << "for (int Idx = 0; Idx < " << TypeData.arraySize << "; ++Idx)";
        mOut.startBlock();
        if (MFA == MapFromAllocationPositive) {
          genReduceResultRangeCheck(mOut, "outArray[Idx]");
        } else {
          mOut.indent() << "result[Idx] = "
                        << genReduceResultMapping(MFA, ArrayElementTypeName, ReflectedScalarTypeName,
                                                     "outArray[Idx]")
                        << ";\n";
        }
        mOut.endBlock();
        mOut.indent() << "mResult = " << (MFA == MapFromAllocationPositive ? "outArray" : "result") << ";\n";
      }
    }
  } else { // result type is vector or array of vector
    // <ArrayElementType>[] outArray = new <ArrayElementType>[<VectorElementCount> * <ArrayElementCount>];
    // mOut.copyTo(outArray);
    const unsigned VectorElementCount = TypeData.vecSize;
    const unsigned OutArrayElementCount = VectorElementCount * std::max(TypeData.arraySize, 1U);
    mOut.indent() << ArrayElementTypeName << "[] outArray = new " << ArrayElementTypeName
                  << "[" << OutArrayElementCount << "];\n";
    mOut.indent() << "mOut.copyTo(outArray);\n";
    if (MFA == MapFromAllocationPositive) {
      mOut.indent() << "for (int Idx = 0; Idx < " << OutArrayElementCount << "; ++Idx)";
      mOut.startBlock();
      genReduceResultRangeCheck(mOut, "outArray[Idx]");
      mOut.endBlock();
    }
    if (TypeData.arraySize == 0) { // result type is vector
      // mResult = new <ResultType>(outArray[0], outArray[1] ...); // but there are several special cases
      mOut.indent() << "mResult = "
                    << genReduceResultVectorMapping(MFA,
                                                    ArrayElementTypeName, ReflectedScalarTypeName,
                                                    GetTypeName(ResultType), VectorElementCount,
                                                    "outArray")
                    << ";\n";
    } else { // result type is array of vector
      // <ResultType> result = new <UnbracketedResultType>[<ArrayElementCount>];
      // for (unsigned Idx = 0; Idx < <ArrayElementCount>; ++Idx)
      //   result[Idx] = new <UnbracketedResultType>(outArray[<ArrayElementCount>*Idx+0],
      //                                             outArray[<ArrayElementCount>*Idx+1]...);
      // mResult = result; // but there are several special cases
      mOut.indent() << GetTypeName(ResultType) << " result = new "
                    << UnbracketedResultTypeName
                    << "[" << TypeData.arraySize << "];\n";
      mOut.indent() << "for (int Idx = 0; Idx < " << TypeData.arraySize << "; ++Idx)";
      mOut.startBlock();
      mOut.indent() << "result[Idx] = "
                    << genReduceResultVectorMapping(MFA,
                                                    ArrayElementTypeName, ReflectedScalarTypeName,
                                                    UnbracketedResultTypeName, VectorElementCount,
                                                    "outArray", (std::to_string(VectorElementCount) + "*Idx").c_str())
                    << ";\n";
      mOut.endBlock();
      mOut.indent() << "mResult = result;\n";
    }
  }

  mOut.indent() << "mOut.destroy();\n";
  mOut.indent() << "mOut = null;  // make Java object eligible for garbage collection\n";
  mOut.indent() << "if (mTempIns != null)";
  mOut.startBlock();
  mOut.indent() << "for (Allocation tempIn : mTempIns)";
  mOut.startBlock();
  mOut.indent() << "tempIn.destroy();\n";
  mOut.endBlock();
  mOut.indent() << "mTempIns = null;  // make Java objects eligible for garbage collection\n";
  mOut.endBlock();
  mOut.indent() << "mGotResult = true;\n";
  mOut.endBlock();

  mOut.indent() << "return mResult;\n";
  endFunction();

  startFunction(AM_Private, false, nullptr, ClassName, 1, "Allocation", "out");
  // TODO: Generate allocation type check and size check?  Or move
  // responsibility for instantiating the Allocation here, instead of
  // the reduce_* method?
  mOut.indent() << "mTempIns = null;\n";
  mOut.indent() << "mOut = out;\n";
  mOut.indent() << "mGotResult = false;\n";
  endFunction();
  mOut.indent() << "private Allocation[] mTempIns;\n";
  mOut.indent() << "private Allocation mOut;\n";
  // TODO: If result is reference type rather than primitive type, we
  // could omit mGotResult and use mResult==null to indicate that we
  // haven't obtained the result yet.
  mOut.indent() << "private boolean mGotResult;\n";
  mOut.indent() << "private " << GetMethodReturnTypeName << " mResult;\n";
  mOut.endBlock();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void RSReflectionJava::genTypeInstanceFromPointer(const RSExportType *ET) {
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

void RSReflectionJava::genTypeInstance(const RSExportType *ET) {
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassConstantArray: {
    std::string TypeName = ET->getElementName();
    if (addTypeNameForElement(TypeName)) {
      mOut.indent() << RS_ELEM_PREFIX << TypeName << " = Element." << TypeName
                    << "(rs);\n";
    }
    break;
  }

  case RSExportType::ExportClassRecord: {
    std::string ClassName = ET->getElementName();
    if (addTypeNameForElement(ClassName)) {
      mOut.indent() << RS_ELEM_PREFIX << ClassName << " = " << ClassName
                    << ".createElement(rs);\n";
    }
    break;
  }

  default:
    break;
  }
}

void RSReflectionJava::genFieldPackerInstance(const RSExportType *ET) {
  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassConstantArray:
  case RSExportType::ExportClassRecord: {
    std::string TypeName = ET->getElementName();
    addTypeNameForFieldPacker(TypeName);
    break;
  }

  default:
    break;
  }
}

void RSReflectionJava::genTypeCheck(const RSExportType *ET,
                                    const char *VarName) {
  mOut.indent() << "// check " << VarName << "\n";

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
                  << ".getType().getElement().isCompatible(" RS_ELEM_PREFIX
                  << TypeName << ")) {\n";
    mOut.indent() << "    throw new RSRuntimeException(\"Type mismatch with "
                  << TypeName << "!\");\n";
    mOut.indent() << "}\n";
  }
}

void RSReflectionJava::genPrimitiveTypeExportVariable(const RSExportVar *EV) {
  slangAssert(
      (EV->getType()->getClass() == RSExportType::ExportClassPrimitive) &&
      "Variable should be type of primitive here");

  const RSExportPrimitiveType *EPT =
      static_cast<const RSExportPrimitiveType *>(EV->getType());
  std::string TypeName = GetTypeName(EPT);
  const std::string &VarName = EV->getName();

  genPrivateExportVariable(TypeName, EV->getName());

  if (EV->isConst()) {
    mOut.indent() << "public final static " << TypeName
                  << " " RS_EXPORT_VAR_CONST_PREFIX << VarName << " = ";
    const clang::APValue &Val = EV->getInit();
    genInitValue(Val, EPT->getType() == DataTypeBoolean);
    mOut << ";\n";
  } else {
    // set_*()
    // This must remain synchronized, since multiple Dalvik threads may
    // be calling setters.
    startFunction(AM_PublicSynchronized, false, "void", "set_" + VarName, 1,
                  TypeName.c_str(), "v");
    if ((EPT->getElementSizeInBytes() < 4) || EV->isUnsigned()) {
      // We create/cache a per-type FieldPacker. This allows us to reuse the
      // validation logic (for catching negative inputs from Dalvik, as well
      // as inputs that are too large to be represented in the unsigned type).
      // Sub-integer types are also handled specially here, so that we don't
      // overwrite bytes accidentally.
      std::string ElemName = EPT->getElementName();
      std::string FPName;
      FPName = RS_FP_PREFIX + ElemName;
      mOut.indent() << "if (" << FPName << "!= null) {\n";
      mOut.increaseIndent();
      mOut.indent() << FPName << ".reset();\n";
      mOut.decreaseIndent();
      mOut.indent() << "} else {\n";
      mOut.increaseIndent();
      mOut.indent() << FPName << " = new FieldPacker(" << EPT->getElementSizeInBytes()
                    << ");\n";
      mOut.decreaseIndent();
      mOut.indent() << "}\n";

      genPackVarOfType(EPT, "v", FPName.c_str());
      mOut.indent() << "setVar(" << RS_EXPORT_VAR_INDEX_PREFIX << VarName
                    << ", " << FPName << ");\n";
    } else {
      mOut.indent() << "setVar(" << RS_EXPORT_VAR_INDEX_PREFIX << VarName
                    << ", v);\n";
    }

    // Dalvik update comes last, since the input may be invalid (and hence
    // throw an exception).
    mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = v;\n";

    endFunction();
  }

  genGetExportVariable(TypeName, VarName);
  genGetFieldID(VarName);
}

void RSReflectionJava::genInitValue(const clang::APValue &Val, bool asBool) {
  switch (Val.getKind()) {
  case clang::APValue::Int: {
    const llvm::APInt &api = Val.getInt();
    if (asBool) {
      mOut << ((api.getSExtValue() == 0) ? "false" : "true");
    } else {
      // TODO: Handle unsigned correctly
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

void RSReflectionJava::genPointerTypeExportVariable(const RSExportVar *EV) {
  const RSExportType *ET = EV->getType();
  const RSExportType *PointeeType;

  slangAssert((ET->getClass() == RSExportType::ExportClassPointer) &&
              "Variable should be type of pointer here");

  PointeeType = static_cast<const RSExportPointerType *>(ET)->getPointeeType();
  std::string TypeName = GetTypeName(ET);
  const std::string &VarName = EV->getName();

  genPrivateExportVariable(TypeName, VarName);

  // bind_*()
  startFunction(AM_Public, false, "void", "bind_" + VarName, 1,
                TypeName.c_str(), "v");

  mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = v;\n";
  mOut.indent() << "if (v == null) bindAllocation(null, "
                << RS_EXPORT_VAR_INDEX_PREFIX << VarName << ");\n";

  if (PointeeType->getClass() == RSExportType::ExportClassRecord) {
    mOut.indent() << "else bindAllocation(v.getAllocation(), "
                  << RS_EXPORT_VAR_INDEX_PREFIX << VarName << ");\n";
  } else {
    mOut.indent() << "else bindAllocation(v, " << RS_EXPORT_VAR_INDEX_PREFIX
                  << VarName << ");\n";
  }

  endFunction();

  genGetExportVariable(TypeName, VarName);
}

void RSReflectionJava::genVectorTypeExportVariable(const RSExportVar *EV) {
  slangAssert((EV->getType()->getClass() == RSExportType::ExportClassVector) &&
              "Variable should be type of vector here");

  std::string TypeName = GetTypeName(EV->getType());
  std::string VarName = EV->getName();

  genPrivateExportVariable(TypeName, VarName);
  genSetExportVariable(TypeName, EV, 1);
  genGetExportVariable(TypeName, VarName);
  genGetFieldID(VarName);
}

void RSReflectionJava::genMatrixTypeExportVariable(const RSExportVar *EV) {
  slangAssert((EV->getType()->getClass() == RSExportType::ExportClassMatrix) &&
              "Variable should be type of matrix here");

  const RSExportType *ET = EV->getType();
  std::string TypeName = GetTypeName(ET);
  const std::string &VarName = EV->getName();

  genPrivateExportVariable(TypeName, VarName);

  // set_*()
  if (!EV->isConst()) {
    const char *FieldPackerName = "fp";
    startFunction(AM_PublicSynchronized, false, "void", "set_" + VarName, 1,
                  TypeName.c_str(), "v");
    mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = v;\n";

    if (genCreateFieldPacker(ET, FieldPackerName, ReflectionState::NoVal32()))
      genPackVarOfType(ET, "v", FieldPackerName);
    mOut.indent() << "setVar(" RS_EXPORT_VAR_INDEX_PREFIX << VarName << ", "
                  << FieldPackerName << ");\n";

    endFunction();
  }

  genGetExportVariable(TypeName, VarName);
  genGetFieldID(VarName);
}

void
RSReflectionJava::genConstantArrayTypeExportVariable(const RSExportVar *EV,
                                                     ReflectionState::Val32 AllocSize32) {
  const RSExportType *const ET = EV->getType();
  slangAssert(
      (ET->getClass() == RSExportType::ExportClassConstantArray) &&
      "Variable should be type of constant array here");

  std::string TypeName = GetTypeName(EV->getType());
  std::string VarName = EV->getName();

  genPrivateExportVariable(TypeName, VarName);
  genSetExportVariable(TypeName, EV,
                       static_cast<const RSExportConstantArrayType *>(ET)->getNumElement(),
                       AllocSize32);
  genGetExportVariable(TypeName, VarName);
  genGetFieldID(VarName);
}

void RSReflectionJava::genRecordTypeExportVariable(const RSExportVar *EV,
                                                   ReflectionState::Val32 AllocSize32) {
  slangAssert((EV->getType()->getClass() == RSExportType::ExportClassRecord) &&
              "Variable should be type of struct here");

  std::string TypeName = GetTypeName(EV->getType());
  std::string VarName = EV->getName();

  genPrivateExportVariable(TypeName, VarName);
  genSetExportVariable(TypeName, EV, 1, AllocSize32);
  genGetExportVariable(TypeName, VarName);
  genGetFieldID(VarName);
}

void RSReflectionJava::genPrivateExportVariable(const std::string &TypeName,
                                                const std::string &VarName) {
  mOut.indent() << "private " << TypeName << " " << RS_EXPORT_VAR_PREFIX
                << VarName << ";\n";
}

// Dimension = array element count; otherwise, 1.
void RSReflectionJava::genSetExportVariable(const std::string &TypeName,
                                            const RSExportVar *EV,
                                            unsigned Dimension,
                                            ReflectionState::Val32 AllocSize32) {
  if (!EV->isConst()) {
    const char *FieldPackerName = "fp";
    const std::string &VarName = EV->getName();
    const RSExportType *ET = EV->getType();
    startFunction(AM_PublicSynchronized, false, "void", "set_" + VarName, 1,
                  TypeName.c_str(), "v");
    mOut.indent() << RS_EXPORT_VAR_PREFIX << VarName << " = v;\n";

    if (genCreateFieldPacker(ET, FieldPackerName, AllocSize32))
      genPackVarOfType(ET, "v", FieldPackerName);

    if (mRSContext->getTargetAPI() < SLANG_JB_TARGET_API) {
      // Legacy apps must use the old setVar() without Element/dim components.
      mOut.indent() << "setVar(" << RS_EXPORT_VAR_INDEX_PREFIX << VarName
                    << ", " << FieldPackerName << ");\n";
    } else {
      // We only have support for one-dimensional array reflection today,
      // but the entry point (i.e. setVar()) takes an array of dimensions.
      mOut.indent() << "int []__dimArr = new int[1];\n";
      mOut.indent() << "__dimArr[0] = " << Dimension << ";\n";
      mOut.indent() << "setVar(" << RS_EXPORT_VAR_INDEX_PREFIX << VarName
                    << ", " << FieldPackerName << ", " << RS_ELEM_PREFIX
                    << ET->getElementName() << ", __dimArr);\n";
    }

    endFunction();
  }
}

void RSReflectionJava::genGetExportVariable(const std::string &TypeName,
                                            const std::string &VarName) {
  startFunction(AM_Public, false, TypeName.c_str(), "get_" + VarName, 0);

  mOut.indent() << "return " << RS_EXPORT_VAR_PREFIX << VarName << ";\n";

  endFunction();
}

void RSReflectionJava::genGetFieldID(const std::string &VarName) {
  // We only generate getFieldID_*() for non-Pointer (bind) types.
  if (mRSContext->getTargetAPI() >= SLANG_JB_MR1_TARGET_API) {
    startFunction(AM_Public, false, "Script.FieldID", "getFieldID_" + VarName,
                  0);

    mOut.indent() << "return createFieldID(" << RS_EXPORT_VAR_INDEX_PREFIX
                  << VarName << ", null);\n";

    endFunction();
  }
}

/******************* Methods to generate script class /end *******************/

bool RSReflectionJava::genCreateFieldPacker(const RSExportType *ET,
                                            const char *FieldPackerName,
                                            ReflectionState::Val32 AllocSize32) {
  size_t AllocSize = ET->getAllocSize();
  slangAssert(!AllocSize32.first || ((AllocSize == 0) == (AllocSize32.second == 0)));
  if (AllocSize > 0) {
    if (!mCollecting) {
      mOut.indent() << "FieldPacker " << FieldPackerName << " = new FieldPacker(";
      genConditionalVal("", false, AllocSize, AllocSize32);
      mOut << ");\n";
    }
  }
  else
    return false;
  return true;
}

void RSReflectionJava::genPackVarOfType(const RSExportType *ET,
                                        const char *VarName,
                                        const char *FieldPackerName) {
  if (mCollecting)
    return;

  switch (ET->getClass()) {
  case RSExportType::ExportClassPrimitive:
  case RSExportType::ExportClassVector: {
    mOut.indent() << FieldPackerName << "."
                  << GetPackerAPIName(
                         static_cast<const RSExportPrimitiveType *>(ET)) << "("
                  << VarName << ");\n";
    break;
  }
  case RSExportType::ExportClassPointer: {
    // Must reflect as type Allocation in Java
    const RSExportType *PointeeType =
        static_cast<const RSExportPointerType *>(ET)->getPointeeType();

    if (PointeeType->getClass() != RSExportType::ExportClassRecord) {
      mOut.indent() << FieldPackerName << ".addI32(" << VarName
                    << ".getPtr());\n";
    } else {
      mOut.indent() << FieldPackerName << ".addI32(" << VarName
                    << ".getAllocation().getPtr());\n";
    }
    break;
  }
  case RSExportType::ExportClassMatrix: {
    mOut.indent() << FieldPackerName << ".addMatrix(" << VarName << ");\n";
    break;
  }
  case RSExportType::ExportClassConstantArray: {
    const RSExportConstantArrayType *ECAT =
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

    mOut.indent() << "for (int " << IndexVarName << " = 0; " << IndexVarName
                  << " < " << ECAT->getNumElement() << "; " << IndexVarName << "++)";
    mOut.startBlock();

    ElementVarName.append("[" + IndexVarName + "]");
    genPackVarOfType(ECAT->getElementType(), ElementVarName.c_str(),
                     FieldPackerName);

    mOut.endBlock();
    break;
  }
  case RSExportType::ExportClassRecord: {
    // Keep struct/field layout in sync with ReflectionState::declareRecord()

    const RSExportRecordType *ERT = static_cast<const RSExportRecordType *>(ET);
    const ReflectionState::Record32 Record32 = mState->getRecord32(ERT);

    auto emitSkip = [this, &FieldPackerName](size_t At, size_t Need,
                                             ReflectionState::Val32 Padding32) {
      if ((Need > At) || (Padding32.first && (Padding32.second != 0))) {
        size_t Padding = Need - At;
        mOut.indent() << FieldPackerName << ".skip(";
        if (!Padding32.first || (Padding == Padding32.second))
          mOut << Padding;
        else {
          genCheck64Bit(true);
          mOut << " ? " << Padding << " : " << Padding32.second;
        }
        mOut << ");\n";
      }
    };

    // Relative pos from now on in field packer
    unsigned Pos = 0;

    unsigned FieldNum = 0;
    for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(),
                                                  E = ERT->fields_end();
         I != E; I++, FieldNum++) {
      const RSExportRecordType::Field *F = *I;
      std::string FieldName;
      size_t FieldOffset = F->getOffsetInParent();
      const RSExportType *T = F->getType();
      size_t FieldStoreSize = T->getStoreSize();
      size_t FieldAllocSize = T->getAllocSize();

      const auto Field32PreAndPostPadding = Record32.getFieldPreAndPostPadding(FieldNum);

      if (VarName != nullptr)
        FieldName = VarName + ("." + F->getName());
      else
        FieldName = F->getName();

      emitSkip(Pos, FieldOffset, Field32PreAndPostPadding.first /* pre */);

      genPackVarOfType(F->getType(), FieldName.c_str(), FieldPackerName);

      // There is padding in the field type?
      emitSkip(FieldStoreSize, FieldAllocSize, Field32PreAndPostPadding.second /* post */);

      Pos = FieldOffset + FieldAllocSize;
    }

    // There maybe some padding after the struct
    emitSkip(Pos, ERT->getAllocSize(), Record32.getRecordPostPadding());
    break;
  }
  default: { slangAssert(false && "Unknown class of type"); }
  }
}

void RSReflectionJava::genAllocateVarOfType(const RSExportType *T,
                                            const std::string &VarName) {
  switch (T->getClass()) {
  case RSExportType::ExportClassPrimitive: {
    // Primitive type like int in Java has its own storage once it's declared.
    //
    // FIXME: Should we allocate storage for RS object?
    // if (static_cast<const RSExportPrimitiveType *>(T)->isRSObjectType())
    //  mOut.indent() << VarName << " = new " << GetTypeName(T) << "();\n";
    break;
  }
  case RSExportType::ExportClassPointer: {
    // Pointer type is an instance of Allocation or a TypeClass whose value is
    // expected to be assigned by programmer later in Java program. Therefore
    // we don't reflect things like [VarName] = new Allocation();
    mOut.indent() << VarName << " = null;\n";
    break;
  }
  case RSExportType::ExportClassConstantArray: {
    const RSExportConstantArrayType *ECAT =
        static_cast<const RSExportConstantArrayType *>(T);
    const RSExportType *ElementType = ECAT->getElementType();

    mOut.indent() << VarName << " = new " << GetTypeName(ElementType) << "["
                  << ECAT->getNumElement() << "];\n";

    // Primitive type element doesn't need allocation code.
    if (ElementType->getClass() != RSExportType::ExportClassPrimitive) {
      mOut.indent() << "for (int $ct = 0; $ct < " << ECAT->getNumElement()
                    << "; $ct++)";
      mOut.startBlock();

      std::string ElementVarName(VarName);
      ElementVarName.append("[$ct]");
      genAllocateVarOfType(ElementType, ElementVarName);

      mOut.endBlock();
    }
    break;
  }
  case RSExportType::ExportClassVector:
  case RSExportType::ExportClassMatrix:
  case RSExportType::ExportClassRecord: {
    mOut.indent() << VarName << " = new " << GetTypeName(T) << "();\n";
    break;
  }
  }
}

void RSReflectionJava::genNewItemBufferIfNull(const char *Index) {
  mOut.indent() << "if (" << RS_TYPE_ITEM_BUFFER_NAME " == null) ";
  mOut << RS_TYPE_ITEM_BUFFER_NAME << " = new " << RS_TYPE_ITEM_CLASS_NAME
       << "[getType().getX() /* count */];\n";
  if (Index != nullptr) {
    mOut.indent() << "if (" << RS_TYPE_ITEM_BUFFER_NAME << "[" << Index
                  << "] == null) ";
    mOut << RS_TYPE_ITEM_BUFFER_NAME << "[" << Index << "] = new "
         << RS_TYPE_ITEM_CLASS_NAME << "();\n";
  }
}

void RSReflectionJava::genNewItemBufferPackerIfNull() {
  mOut.indent() << "if (" << RS_TYPE_ITEM_BUFFER_PACKER_NAME << " == null) ";
  mOut << RS_TYPE_ITEM_BUFFER_PACKER_NAME " = new FieldPacker("
       <<  mItemSizeof << " * getType().getX()/* count */);\n";
}

/********************** Methods to generate type class  **********************/
bool RSReflectionJava::genTypeClass(const RSExportRecordType *ERT,
                                    std::string &ErrorMsg) {
  mState->declareRecord(ERT);
  if (mCollecting)
    return true;

  std::string ClassName = ERT->getElementName();
  std::string superClassName = getRSPackageName();
  superClassName += RS_TYPE_CLASS_SUPER_CLASS_NAME;

  if (!startClass(AM_Public, false, ClassName, superClassName.c_str(),
                  ErrorMsg))
    return false;

  mGeneratedFileNames->push_back(ClassName);

  mState->beginOutputClass();

  genTypeItemClass(ERT);

  // Declare item buffer and item buffer packer
  mOut.indent() << "private " << RS_TYPE_ITEM_CLASS_NAME << " "
                << RS_TYPE_ITEM_BUFFER_NAME << "[];\n";
  mOut.indent() << "private FieldPacker " << RS_TYPE_ITEM_BUFFER_PACKER_NAME
                << ";\n";
  mOut.indent() << "private static java.lang.ref.WeakReference<Element> "
                << RS_TYPE_ELEMENT_REF_NAME
                << " = new java.lang.ref.WeakReference<Element>(null);\n";

  genTypeClassConstructor(ERT);
  genTypeClassCopyToArrayLocal(ERT);
  genTypeClassCopyToArray(ERT);
  genTypeClassItemSetter(ERT);
  genTypeClassItemGetter(ERT);
  genTypeClassComponentSetter(ERT);
  genTypeClassComponentGetter(ERT);
  genTypeClassCopyAll(ERT);
  if (!mRSContext->isCompatLib()) {
    // Skip the resize method if we are targeting a compatibility library.
    genTypeClassResize();
  }

  if (mState->endOutputClass())
    genCompute64Bit();

  endClass();

  resetFieldIndex();
  clearFieldIndexMap();

  return true;
}

void RSReflectionJava::genTypeItemClass(const RSExportRecordType *ERT) {
  mOut.indent() << "static public class " RS_TYPE_ITEM_CLASS_NAME;
  mOut.startBlock();

  // Sizeof should not be exposed for 64-bit; it is not accurate
  if (mRSContext->getTargetAPI() < 21) {
      mOut.indent() << "public static final int sizeof = " << ERT->getAllocSize()
                    << ";\n";
  }

  // Member elements
  mOut << "\n";
  for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
                                                FE = ERT->fields_end();
       FI != FE; FI++) {
    mOut.indent() << GetTypeName((*FI)->getType()) << " " << (*FI)->getName()
                  << ";\n";
  }

  // Constructor
  mOut << "\n";
  mOut.indent() << RS_TYPE_ITEM_CLASS_NAME << "()";
  mOut.startBlock();

  for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
                                                FE = ERT->fields_end();
       FI != FE; FI++) {
    const RSExportRecordType::Field *F = *FI;
    genAllocateVarOfType(F->getType(), F->getName());
  }

  // end Constructor
  mOut.endBlock();

  // end Item class
  mOut.endBlock();
}

void RSReflectionJava::genTypeClassConstructor(const RSExportRecordType *ERT) {
  const char *RenderScriptVar = "rs";

  startFunction(AM_Public, true, "Element", "createElement", 1, "RenderScript",
                RenderScriptVar);

  // TODO(all): Fix weak-refs + multi-context issue.
  // mOut.indent() << "Element e = " << RS_TYPE_ELEMENT_REF_NAME
  //            << ".get();\n";
  // mOut.indent() << "if (e != null) return e;\n";
  RSReflectionJavaElementBuilder builder("eb", ERT, RenderScriptVar, &mOut,
                                         mRSContext, this, mState);
  builder.generate();

  mOut.indent() << "return eb.create();\n";
  // mOut.indent() << "e = eb.create();\n";
  // mOut.indent() << RS_TYPE_ELEMENT_REF_NAME
  //            << " = new java.lang.ref.WeakReference<Element>(e);\n";
  // mOut.indent() << "return e;\n";
  endFunction();

  // private with element
  startFunction(AM_Private, false, nullptr, getClassName(), 1, "RenderScript",
                RenderScriptVar);
  mOut.indent() << RS_TYPE_ITEM_BUFFER_NAME << " = null;\n";
  mOut.indent() << RS_TYPE_ITEM_BUFFER_PACKER_NAME << " = null;\n";
  mOut.indent() << "mElement = createElement(" << RenderScriptVar << ");\n";
  endFunction();

  // 1D without usage
  startFunction(AM_Public, false, nullptr, getClassName(), 2, "RenderScript",
                RenderScriptVar, "int", "count");

  mOut.indent() << RS_TYPE_ITEM_BUFFER_NAME << " = null;\n";
  mOut.indent() << RS_TYPE_ITEM_BUFFER_PACKER_NAME << " = null;\n";
  mOut.indent() << "mElement = createElement(" << RenderScriptVar << ");\n";
  // Call init() in super class
  mOut.indent() << "init(" << RenderScriptVar << ", count);\n";
  endFunction();

  // 1D with usage
  startFunction(AM_Public, false, nullptr, getClassName(), 3, "RenderScript",
                RenderScriptVar, "int", "count", "int", "usages");

  mOut.indent() << RS_TYPE_ITEM_BUFFER_NAME << " = null;\n";
  mOut.indent() << RS_TYPE_ITEM_BUFFER_PACKER_NAME << " = null;\n";
  mOut.indent() << "mElement = createElement(" << RenderScriptVar << ");\n";
  // Call init() in super class
  mOut.indent() << "init(" << RenderScriptVar << ", count, usages);\n";
  endFunction();

  // create1D with usage
  startFunction(AM_Public, true, getClassName().c_str(), "create1D", 3,
                "RenderScript", RenderScriptVar, "int", "dimX", "int",
                "usages");
  mOut.indent() << getClassName() << " obj = new " << getClassName() << "("
                << RenderScriptVar << ");\n";
  mOut.indent() << "obj.mAllocation = Allocation.createSized("
                   "rs, obj.mElement, dimX, usages);\n";
  mOut.indent() << "return obj;\n";
  endFunction();

  // create1D without usage
  startFunction(AM_Public, true, getClassName().c_str(), "create1D", 2,
                "RenderScript", RenderScriptVar, "int", "dimX");
  mOut.indent() << "return create1D(" << RenderScriptVar
                << ", dimX, Allocation.USAGE_SCRIPT);\n";
  endFunction();

  // create2D without usage
  startFunction(AM_Public, true, getClassName().c_str(), "create2D", 3,
                "RenderScript", RenderScriptVar, "int", "dimX", "int", "dimY");
  mOut.indent() << "return create2D(" << RenderScriptVar
                << ", dimX, dimY, Allocation.USAGE_SCRIPT);\n";
  endFunction();

  // create2D with usage
  startFunction(AM_Public, true, getClassName().c_str(), "create2D", 4,
                "RenderScript", RenderScriptVar, "int", "dimX", "int", "dimY",
                "int", "usages");

  mOut.indent() << getClassName() << " obj = new " << getClassName() << "("
                << RenderScriptVar << ");\n";
  mOut.indent() << "Type.Builder b = new Type.Builder(rs, obj.mElement);\n";
  mOut.indent() << "b.setX(dimX);\n";
  mOut.indent() << "b.setY(dimY);\n";
  mOut.indent() << "Type t = b.create();\n";
  mOut.indent() << "obj.mAllocation = Allocation.createTyped(rs, t, usages);\n";
  mOut.indent() << "return obj;\n";
  endFunction();

  // createTypeBuilder
  startFunction(AM_Public, true, "Type.Builder", "createTypeBuilder", 1,
                "RenderScript", RenderScriptVar);
  mOut.indent() << "Element e = createElement(" << RenderScriptVar << ");\n";
  mOut.indent() << "return new Type.Builder(rs, e);\n";
  endFunction();

  // createCustom with usage
  startFunction(AM_Public, true, getClassName().c_str(), "createCustom", 3,
                "RenderScript", RenderScriptVar, "Type.Builder", "tb", "int",
                "usages");
  mOut.indent() << getClassName() << " obj = new " << getClassName() << "("
                << RenderScriptVar << ");\n";
  mOut.indent() << "Type t = tb.create();\n";
  mOut.indent() << "if (t.getElement() != obj.mElement) {\n";
  mOut.indent() << "    throw new RSIllegalArgumentException("
                   "\"Type.Builder did not match expected element type.\");\n";
  mOut.indent() << "}\n";
  mOut.indent() << "obj.mAllocation = Allocation.createTyped(rs, t, usages);\n";
  mOut.indent() << "return obj;\n";
  endFunction();
}

void RSReflectionJava::genTypeClassCopyToArray(const RSExportRecordType *ERT) {
  startFunction(AM_Private, false, "void", "copyToArray", 2,
                RS_TYPE_ITEM_CLASS_NAME, "i", "int", "index");

  genNewItemBufferPackerIfNull();
  mOut.indent() << RS_TYPE_ITEM_BUFFER_PACKER_NAME << ".reset(index * "
                << mItemSizeof << ");\n";

  mOut.indent() << "copyToArrayLocal(i, " RS_TYPE_ITEM_BUFFER_PACKER_NAME
                   ");\n";

  endFunction();
}

void
RSReflectionJava::genTypeClassCopyToArrayLocal(const RSExportRecordType *ERT) {
  startFunction(AM_Private, false, "void", "copyToArrayLocal", 2,
                RS_TYPE_ITEM_CLASS_NAME, "i", "FieldPacker", "fp");

  genPackVarOfType(ERT, "i", "fp");

  endFunction();
}

void RSReflectionJava::genTypeClassItemSetter(const RSExportRecordType *ERT) {
  startFunction(AM_PublicSynchronized, false, "void", "set", 3,
                RS_TYPE_ITEM_CLASS_NAME, "i", "int", "index", "boolean",
                "copyNow");
  genNewItemBufferIfNull(nullptr);
  mOut.indent() << RS_TYPE_ITEM_BUFFER_NAME << "[index] = i;\n";

  mOut.indent() << "if (copyNow) ";
  mOut.startBlock();

  mOut.indent() << "copyToArray(i, index);\n";
  mOut.indent() << "FieldPacker fp = new FieldPacker(" << mItemSizeof << ");\n";
  mOut.indent() << "copyToArrayLocal(i, fp);\n";
  mOut.indent() << "mAllocation.setFromFieldPacker(index, fp);\n";

  // End of if (copyNow)
  mOut.endBlock();

  endFunction();
}

void RSReflectionJava::genTypeClassItemGetter(const RSExportRecordType *ERT) {
  startFunction(AM_PublicSynchronized, false, RS_TYPE_ITEM_CLASS_NAME, "get", 1,
                "int", "index");
  mOut.indent() << "if (" << RS_TYPE_ITEM_BUFFER_NAME
                << " == null) return null;\n";
  mOut.indent() << "return " << RS_TYPE_ITEM_BUFFER_NAME << "[index];\n";
  endFunction();
}

void
RSReflectionJava::genTypeClassComponentSetter(const RSExportRecordType *ERT) {
  const ReflectionState::Record32 Record32 = mState->getRecord32(ERT);

  unsigned FieldNum = 0;
  for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
                                                FE = ERT->fields_end();
       FI != FE; FI++, FieldNum++) {
    const RSExportRecordType::Field *F = *FI;
    size_t FieldOffset = F->getOffsetInParent();
    size_t FieldStoreSize = F->getType()->getStoreSize();
    std::pair<unsigned, unsigned> FieldIndex = getFieldIndex(F);

    const auto Field32OffsetAndStoreSize = Record32.getFieldOffsetAndStoreSize(FieldNum);

    startFunction(AM_PublicSynchronized, false, "void", "set_" + F->getName(),
                  3, "int", "index", GetTypeName(F->getType()).c_str(), "v",
                  "boolean", "copyNow");
    genNewItemBufferPackerIfNull();
    genNewItemBufferIfNull("index");
    mOut.indent() << RS_TYPE_ITEM_BUFFER_NAME << "[index]." << F->getName()
                  << " = v;\n";

    mOut.indent() << "if (copyNow) ";
    mOut.startBlock();

    mOut.indent() << RS_TYPE_ITEM_BUFFER_PACKER_NAME << ".reset(index * "
                  << mItemSizeof;
    genConditionalVal(" + ", true, FieldOffset, Field32OffsetAndStoreSize.first /* offset */);
    mOut << ");\n";

    genPackVarOfType(F->getType(), "v", RS_TYPE_ITEM_BUFFER_PACKER_NAME);

    mOut.indent() << "FieldPacker fp = new FieldPacker(";
    genConditionalVal("", false, FieldStoreSize, Field32OffsetAndStoreSize.second /* size */);
    mOut << ");\n";

    genPackVarOfType(F->getType(), "v", "fp");
    mOut.indent() << "mAllocation.setFromFieldPacker(index, ";
    genConditionalVal("", false, FieldIndex.first,
                      ReflectionState::Val32(Field32OffsetAndStoreSize.first.first, FieldIndex.second));
    mOut << ", fp);\n";

    // End of if (copyNow)
    mOut.endBlock();

    endFunction();
  }
}

void
RSReflectionJava::genTypeClassComponentGetter(const RSExportRecordType *ERT) {
  for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
                                                FE = ERT->fields_end();
       FI != FE; FI++) {
    const RSExportRecordType::Field *F = *FI;
    startFunction(AM_PublicSynchronized, false,
                  GetTypeName(F->getType()).c_str(), "get_" + F->getName(), 1,
                  "int", "index");
    mOut.indent() << "if (" RS_TYPE_ITEM_BUFFER_NAME << " == null) return "
                  << GetTypeNullValue(F->getType()) << ";\n";
    mOut.indent() << "return " RS_TYPE_ITEM_BUFFER_NAME << "[index]."
                  << F->getName() << ";\n";
    endFunction();
  }
}

void RSReflectionJava::genTypeClassCopyAll(const RSExportRecordType *ERT) {
  startFunction(AM_PublicSynchronized, false, "void", "copyAll", 0);

  mOut.indent() << "for (int ct = 0; ct < " << RS_TYPE_ITEM_BUFFER_NAME
                << ".length; ct++)"
                << " copyToArray(" << RS_TYPE_ITEM_BUFFER_NAME
                << "[ct], ct);\n";
  mOut.indent() << "mAllocation.setFromFieldPacker(0, "
                << RS_TYPE_ITEM_BUFFER_PACKER_NAME ");\n";

  endFunction();
}

void RSReflectionJava::genTypeClassResize() {
  startFunction(AM_PublicSynchronized, false, "void", "resize", 1, "int",
                "newSize");

  mOut.indent() << "if (mItemArray != null) ";
  mOut.startBlock();
  mOut.indent() << "int oldSize = mItemArray.length;\n";
  mOut.indent() << "int copySize = Math.min(oldSize, newSize);\n";
  mOut.indent() << "if (newSize == oldSize) return;\n";
  mOut.indent() << "Item ni[] = new Item[newSize];\n";
  mOut.indent() << "System.arraycopy(mItemArray, 0, ni, 0, copySize);\n";
  mOut.indent() << "mItemArray = ni;\n";
  mOut.endBlock();
  mOut.indent() << "mAllocation.resize(newSize);\n";

  mOut.indent() << "if (" RS_TYPE_ITEM_BUFFER_PACKER_NAME
                   " != null) " RS_TYPE_ITEM_BUFFER_PACKER_NAME " = "
                   "new FieldPacker(" << mItemSizeof << " * getType().getX()/* count */);\n";

  endFunction();
}

/******************** Methods to generate type class /end ********************/

/********** Methods to create Element in Java of given record type ***********/

RSReflectionJavaElementBuilder::RSReflectionJavaElementBuilder(
    const char *ElementBuilderName, const RSExportRecordType *ERT,
    const char *RenderScriptVar, GeneratedFile *Out, const RSContext *RSContext,
    RSReflectionJava *Reflection, ReflectionState *RState)
    : mElementBuilderName(ElementBuilderName), mERT(ERT),
      mRenderScriptVar(RenderScriptVar), mOut(Out), mPaddingFieldIndex(1),
      mRSContext(RSContext), mReflection(Reflection), mState(RState) {
  if (mRSContext->getTargetAPI() < SLANG_ICS_TARGET_API) {
    mPaddingPrefix = "#padding_";
  } else {
    mPaddingPrefix = "#rs_padding_";
  }
}

void RSReflectionJavaElementBuilder::generate() {
  mOut->indent() << "Element.Builder " << mElementBuilderName
                 << " = new Element.Builder(" << mRenderScriptVar << ");\n";
  genAddElement(mERT, "", /* ArraySize = */ 0);
}

void RSReflectionJavaElementBuilder::genAddElement(const RSExportType *ET,
                                                   const std::string &VarName,
                                                   unsigned ArraySize) {
  std::string ElementConstruct = GetBuiltinElementConstruct(ET);

  if (ElementConstruct != "") {
    genAddStatementStart();
    *mOut << ElementConstruct << "(" << mRenderScriptVar << ")";
    genAddStatementEnd(VarName, ArraySize);
  } else {

    switch (ET->getClass()) {
    case RSExportType::ExportClassPrimitive: {
      const RSExportPrimitiveType *EPT =
          static_cast<const RSExportPrimitiveType *>(ET);
      const char *DataTypeName =
          RSExportPrimitiveType::getRSReflectionType(EPT)->rs_type;
      genAddStatementStart();
      *mOut << "Element.createUser(" << mRenderScriptVar
            << ", Element.DataType." << DataTypeName << ")";
      genAddStatementEnd(VarName, ArraySize);
      break;
    }
    case RSExportType::ExportClassVector: {
      const RSExportVectorType *EVT =
          static_cast<const RSExportVectorType *>(ET);
      const char *DataTypeName =
          RSExportPrimitiveType::getRSReflectionType(EVT)->rs_type;
      genAddStatementStart();
      *mOut << "Element.createVector(" << mRenderScriptVar
            << ", Element.DataType." << DataTypeName << ", "
            << EVT->getNumElement() << ")";
      genAddStatementEnd(VarName, ArraySize);
      break;
    }
    case RSExportType::ExportClassPointer:
      // Pointer type variable should be resolved in
      // GetBuiltinElementConstruct()
      slangAssert(false && "??");
      break;
    case RSExportType::ExportClassMatrix:
      // Matrix type variable should be resolved
      // in GetBuiltinElementConstruct()
      slangAssert(false && "??");
      break;
    case RSExportType::ExportClassConstantArray: {
      const RSExportConstantArrayType *ECAT =
          static_cast<const RSExportConstantArrayType *>(ET);

      const RSExportType *ElementType = ECAT->getElementType();
      if (ElementType->getClass() != RSExportType::ExportClassRecord) {
        genAddElement(ECAT->getElementType(), VarName, ECAT->getNumElement());
      } else {
        slangAssert((ArraySize == 0) && "Cannot reflect multidimensional array types");
        ArraySize = ECAT->getNumElement();
        genAddStatementStart();
        *mOut << ElementType->getElementName() << ".createElement(" << mRenderScriptVar << ")";
        genAddStatementEnd(VarName, ArraySize);
      }
      break;
    }
    case RSExportType::ExportClassRecord: {
      // Simalar to case of RSExportType::ExportClassRecord in genPackVarOfType.
      //
      // TODO(zonr): Generalize these two function such that there's no
      //             duplicated codes.

      // Keep struct/field layout in sync with ReflectionState::declareRecord()

      const RSExportRecordType *ERT =
          static_cast<const RSExportRecordType *>(ET);
      const ReflectionState::Record32 Record32 = mState->getRecord32(ERT);

      int Pos = 0; // relative pos from now on

      unsigned FieldNum = 0;
      for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(),
                                                    E = ERT->fields_end();
           I != E; I++, FieldNum++) {
        const RSExportRecordType::Field *F = *I;
        size_t FieldOffset = F->getOffsetInParent();
        const RSExportType *T = F->getType();
        size_t FieldStoreSize = T->getStoreSize();
        size_t FieldAllocSize = T->getAllocSize();

        const auto Field32PreAndPostPadding = Record32.getFieldPreAndPostPadding(FieldNum);

        std::string FieldName;
        if (!VarName.empty())
          FieldName = VarName + "." + F->getName();
        else
          FieldName = F->getName();

        // Alignment
        genAddPadding(FieldOffset - Pos, Field32PreAndPostPadding.first /* pre */);

        // eb.add(...)
        mReflection->addFieldIndexMapping(F);
        if (F->getType()->getClass() != RSExportType::ExportClassRecord) {
          genAddElement(F->getType(), FieldName, 0);
        } else {
          genAddStatementStart();
          *mOut << F->getType()->getElementName() << ".createElement(" << mRenderScriptVar << ")";
          genAddStatementEnd(FieldName, ArraySize);
        }

        if (mRSContext->getTargetAPI() < SLANG_ICS_TARGET_API) {
          // There is padding within the field type. This is only necessary
          // for HC-targeted APIs.
          genAddPadding(FieldAllocSize - FieldStoreSize, Field32PreAndPostPadding.second /* post */);
        }

        Pos = FieldOffset + FieldAllocSize;
      }

      // There maybe some padding after the struct
      size_t RecordAllocSize = ERT->getAllocSize();
      genAddPadding(RecordAllocSize - Pos, Record32.getRecordPostPadding());
      break;
    }
    default:
      slangAssert(false && "Unknown class of type");
      break;
    }
  }
}

void RSReflectionJavaElementBuilder::genAddPadding(int PaddingSize, unsigned Which) {
  while (PaddingSize > 0) {
    const std::string &VarName = createPaddingField();
    genAddStatementStart();
    if (PaddingSize >= 4) {
      *mOut << "Element.U32(" << mRenderScriptVar << ")";
      PaddingSize -= 4;
    } else if (PaddingSize >= 2) {
      *mOut << "Element.U16(" << mRenderScriptVar << ")";
      PaddingSize -= 2;
    } else if (PaddingSize >= 1) {
      *mOut << "Element.U8(" << mRenderScriptVar << ")";
      PaddingSize -= 1;
    }
    genAddStatementEnd(VarName, 0, Which);
  }
}

void RSReflectionJavaElementBuilder::genAddPadding(int PaddingSize,
                                                   ReflectionState::Val32 Field32PaddingSize) {
  if (!Field32PaddingSize.first || (PaddingSize == Field32PaddingSize.second)) {
    // Either we're ignoring the 32-bit case, or 32-bit and 64-bit
    // padding are the same.
    genAddPadding(PaddingSize, RSReflectionJava::FieldIndex | RSReflectionJava::Field32Index);
    return;
  }

  // We cannot ignore the 32-bit case, and 32-bit and 64-bit padding differ.

  if ((PaddingSize == 0) != (Field32PaddingSize.second == 0)) {
    // Only pad one of the two cases.

    mOut->indent() << "if (";
    if (PaddingSize == 0)
      *mOut << '!';
    genCheck64Bit(PaddingSize == 0);
    *mOut << ')';

    mOut->startBlock();
    if (PaddingSize != 0)
      genAddPadding(PaddingSize, RSReflectionJava::FieldIndex);
    else
      genAddPadding(Field32PaddingSize.second, RSReflectionJava::Field32Index);
    mOut->endBlock();
    return;
  }

  // Pad both of the two cases, differently.
  mOut->indent() << "if (";
  genCheck64Bit(false);
  *mOut << ')';
  mOut->startBlock();
  genAddPadding(PaddingSize, RSReflectionJava::FieldIndex);
  mOut->endBlock();
  mOut->indent() << "else";
  mOut->startBlock();
  genAddPadding(Field32PaddingSize.second, RSReflectionJava::Field32Index);
  mOut->endBlock();
}

void RSReflectionJavaElementBuilder::genAddStatementStart() {
  mOut->indent() << mElementBuilderName << ".add(";
}

void
RSReflectionJavaElementBuilder::genAddStatementEnd(const std::string &VarName,
                                                   unsigned ArraySize,
                                                   unsigned Which) {
  *mOut << ", \"" << VarName << "\"";
  if (ArraySize > 0) {
    *mOut << ", " << ArraySize;
  }
  *mOut << ");\n";
  // TODO Review incFieldIndex.  It's probably better to assign the numbers at
  // the start rather
  // than as we're generating the code.
  mReflection->incFieldIndex(Which);
}

/******** Methods to create Element in Java of given record type /end ********/

bool RSReflectionJava::reflect() {
  // Note that we may issue declareRecord() in many places during the
  // reflection process.
  mState->beginRecords();

  std::string ErrorMsg;
  if (!genScriptClass(mScriptClassName, ErrorMsg)) {
    std::cerr << "Failed to generate class " << mScriptClassName << " ("
              << ErrorMsg << ")\n";
    return false;
  }

  // class ScriptField_<TypeName>
  for (RSContext::const_export_type_iterator
           TI = mRSContext->export_types_begin(),
           TE = mRSContext->export_types_end();
       TI != TE; TI++) {
    const RSExportType *ET = TI->getValue();

    if (ET->getClass() == RSExportType::ExportClassRecord) {
      const RSExportRecordType *ERT =
          static_cast<const RSExportRecordType *>(ET);

      if (!ERT->isArtificial() && !genTypeClass(ERT, ErrorMsg)) {
        std::cerr << "Failed to generate type class for struct '"
                  << ERT->getName() << "' (" << ErrorMsg << ")\n";
        return false;
      }
    }
  }

  mState->endRecords();

  return true;
}

const char *RSReflectionJava::AccessModifierStr(AccessModifier AM) {
  switch (AM) {
  case AM_Public:
    return "public";
    break;
  case AM_Protected:
    return "protected";
    break;
  case AM_Private:
    return "private";
    break;
  case AM_PublicSynchronized:
    return "public synchronized";
    break;
  default:
    return "";
    break;
  }
}

bool RSReflectionJava::startClass(AccessModifier AM, bool IsStatic,
                                  const std::string &ClassName,
                                  const char *SuperClassName,
                                  std::string &ErrorMsg) {
  // Open file for class
  std::string FileName = ClassName + ".java";
  if (!mOut.startFile(mOutputDirectory, FileName, mRSSourceFileName,
                      mRSContext->getLicenseNote(), true,
                      mRSContext->getVerbose())) {
    return false;
  }

  // Package
  if (!mPackageName.empty()) {
    mOut << "package " << mPackageName << ";\n";
  }
  mOut << "\n";

  // Imports
  //
  // The first few imports are only needed for divergent classes, but
  // at this point we don't know whether we are emitting a divergent
  // class.
  //
  if (!mRSContext->isCompatLib()) {
    mOut << "import android.os.Build;\n";
    mOut << "import android.os.Process;\n";
    mOut << "import java.lang.reflect.Field;\n";
  }
  // (End of imports needed for divergent classes.)
  mOut << "import " << mRSPackageName << ".*;\n";
  if (getEmbedBitcodeInJava()) {
    mOut << "import " << mPackageName << "."
          << RSSlangReflectUtils::JavaBitcodeClassNameFromRSFileName(
                 mRSSourceFileName.c_str()) << ";\n";
  } else {
    mOut << "import android.content.res.Resources;\n";
  }
  mOut << "\n";

  // All reflected classes should be annotated as hidden, so that they won't
  // be exposed in SDK.
  mOut << "/**\n";
  mOut << " * @hide\n";
  mOut << " */\n";

  mOut << AccessModifierStr(AM) << ((IsStatic) ? " static" : "") << " class "
       << ClassName;
  if (SuperClassName != nullptr)
    mOut << " extends " << SuperClassName;

  mOut.startBlock();

  mClassName = ClassName;

  return true;
}

void RSReflectionJava::endClass() {
  mOut.endBlock();
  mOut.closeFile();
  clear();
}

void RSReflectionJava::startTypeClass(const std::string &ClassName) {
  mOut.indent() << "public static class " << ClassName;
  mOut.startBlock();
}

void RSReflectionJava::endTypeClass() { mOut.endBlock(); }

void RSReflectionJava::startFunction(AccessModifier AM, bool IsStatic,
                                     const char *ReturnType,
                                     const std::string &FunctionName, int Argc,
                                     ...) {
  ArgTy Args;
  va_list vl;
  va_start(vl, Argc);

  for (int i = 0; i < Argc; i++) {
    const char *ArgType = va_arg(vl, const char *);
    const char *ArgName = va_arg(vl, const char *);

    Args.push_back(std::make_pair(ArgType, ArgName));
  }
  va_end(vl);

  startFunction(AM, IsStatic, ReturnType, FunctionName, Args);
}

void RSReflectionJava::startFunction(AccessModifier AM, bool IsStatic,
                                     const char *ReturnType,
                                     const std::string &FunctionName,
                                     const ArgTy &Args) {
  mOut.indent() << AccessModifierStr(AM) << ((IsStatic) ? " static " : " ")
                << ((ReturnType) ? ReturnType : "") << " " << FunctionName
                << "(";

  bool FirstArg = true;
  for (ArgTy::const_iterator I = Args.begin(), E = Args.end(); I != E; I++) {
    if (!FirstArg)
      mOut << ", ";
    else
      FirstArg = false;

    mOut << I->first << " " << I->second;
  }

  mOut << ")";
  mOut.startBlock();
}

void RSReflectionJava::endFunction() { mOut.endBlock(); }

bool RSReflectionJava::addTypeNameForElement(const std::string &TypeName) {
  if (mTypesToCheck.find(TypeName) == mTypesToCheck.end()) {
    mTypesToCheck.insert(TypeName);
    return true;
  } else {
    return false;
  }
}

bool RSReflectionJava::addTypeNameForFieldPacker(const std::string &TypeName) {
  if (mFieldPackerTypes.find(TypeName) == mFieldPackerTypes.end()) {
    mFieldPackerTypes.insert(TypeName);
    return true;
  } else {
    return false;
  }
}

} // namespace slang
