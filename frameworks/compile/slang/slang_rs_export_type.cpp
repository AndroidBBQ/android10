/*
 * Copyright 2010-2012, The Android Open Source Project
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

#include "slang_rs_export_type.h"

#include <list>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecordLayout.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_export_element.h"
#include "slang_version.h"

#define CHECK_PARENT_EQUALITY(ParentClass, E) \
  if (!ParentClass::matchODR(E, true))        \
    return false;

namespace slang {

namespace {

// For the data types we support:
//  Category      - data type category
//  SName         - "common name" in script (C99)
//  RsType        - element name in RenderScript
//  RsShortType   - short element name in RenderScript
//  SizeInBits    - size in bits
//  CName         - reflected C name
//  JavaName      - reflected Java name
//  JavaArrayElementName - reflected name in Java arrays
//  CVecName      - prefix for C vector types
//  JavaVecName   - prefix for Java vector type
//  JavaPromotion - unsigned type undergoing Java promotion
//
// IMPORTANT: The data types in this table should be at the same index as
// specified by the corresponding DataType enum.
//
// TODO: Pull this information out into a separate file.
static RSReflectionType gReflectionTypes[] = {
#define _ nullptr
  //      Category     SName              RsType       RsST           CName         JN      JAEN       CVN       JVN     JP
{PrimitiveDataType,   "half",         "FLOAT_16",     "F16", 16,     "half",   "short",  "short",   "Half",  "Short", false},
{PrimitiveDataType,  "float",         "FLOAT_32",     "F32", 32,    "float",   "float",  "float",  "Float",  "Float", false},
{PrimitiveDataType, "double",         "FLOAT_64",     "F64", 64,   "double",  "double", "double", "Double", "Double", false},
{PrimitiveDataType,   "char",         "SIGNED_8",      "I8",  8,   "int8_t",    "byte",   "byte",   "Byte",   "Byte", false},
{PrimitiveDataType,  "short",        "SIGNED_16",     "I16", 16,  "int16_t",   "short",  "short",  "Short",  "Short", false},
{PrimitiveDataType,    "int",        "SIGNED_32",     "I32", 32,  "int32_t",     "int",    "int",    "Int",    "Int", false},
{PrimitiveDataType,   "long",        "SIGNED_64",     "I64", 64,  "int64_t",    "long",   "long",   "Long",   "Long", false},
{PrimitiveDataType,  "uchar",       "UNSIGNED_8",      "U8",  8,  "uint8_t",   "short",   "byte",  "UByte",  "Short",  true},
{PrimitiveDataType, "ushort",      "UNSIGNED_16",     "U16", 16, "uint16_t",     "int",  "short", "UShort",    "Int",  true},
{PrimitiveDataType,   "uint",      "UNSIGNED_32",     "U32", 32, "uint32_t",    "long",    "int",   "UInt",   "Long",  true},
{PrimitiveDataType,  "ulong",      "UNSIGNED_64",     "U64", 64, "uint64_t",    "long",   "long",  "ULong",   "Long", false},
{PrimitiveDataType,   "bool",          "BOOLEAN", "BOOLEAN",  8,     "bool", "boolean",   "byte",        _,        _, false},
{PrimitiveDataType,        _,   "UNSIGNED_5_6_5",         _, 16,          _,         _,        _,        _,        _, false},
{PrimitiveDataType,        _, "UNSIGNED_5_5_5_1",         _, 16,          _,         _,        _,        _,        _, false},
{PrimitiveDataType,        _, "UNSIGNED_4_4_4_4",         _, 16,          _,         _,        _,        _,        _, false},

{MatrixDataType, "rs_matrix2x2", "MATRIX_2X2", _,  4*32, "rs_matrix2x2", "Matrix2f", _, _, _, false},
{MatrixDataType, "rs_matrix3x3", "MATRIX_3X3", _,  9*32, "rs_matrix3x3", "Matrix3f", _, _, _, false},
{MatrixDataType, "rs_matrix4x4", "MATRIX_4X4", _, 16*32, "rs_matrix4x4", "Matrix4f", _, _, _, false},

// RS object types are 32 bits in 32-bit RS, but 256 bits in 64-bit RS.
// This is handled specially by the GetElementSizeInBits() method.
{ObjectDataType,          "rs_element",          "RS_ELEMENT",          "ELEMENT", 32,         "Element",         "Element", _, _, _, false},
{ObjectDataType,             "rs_type",             "RS_TYPE",             "TYPE", 32,            "Type",            "Type", _, _, _, false},
{ObjectDataType,       "rs_allocation",       "RS_ALLOCATION",       "ALLOCATION", 32,      "Allocation",      "Allocation", _, _, _, false},
{ObjectDataType,          "rs_sampler",          "RS_SAMPLER",          "SAMPLER", 32,         "Sampler",         "Sampler", _, _, _, false},
{ObjectDataType,           "rs_script",           "RS_SCRIPT",           "SCRIPT", 32,          "Script",          "Script", _, _, _, false},
{ObjectDataType,             "rs_mesh",             "RS_MESH",             "MESH", 32,            "Mesh",            "Mesh", _, _, _, false},
{ObjectDataType,             "rs_path",             "RS_PATH",             "PATH", 32,            "Path",            "Path", _, _, _, false},
{ObjectDataType, "rs_program_fragment", "RS_PROGRAM_FRAGMENT", "PROGRAM_FRAGMENT", 32, "ProgramFragment", "ProgramFragment", _, _, _, false},
{ObjectDataType,   "rs_program_vertex",   "RS_PROGRAM_VERTEX",   "PROGRAM_VERTEX", 32,   "ProgramVertex",   "ProgramVertex", _, _, _, false},
{ObjectDataType,   "rs_program_raster",   "RS_PROGRAM_RASTER",   "PROGRAM_RASTER", 32,   "ProgramRaster",   "ProgramRaster", _, _, _, false},
{ObjectDataType,    "rs_program_store",    "RS_PROGRAM_STORE",    "PROGRAM_STORE", 32,    "ProgramStore",    "ProgramStore", _, _, _, false},
{ObjectDataType,             "rs_font",             "RS_FONT",             "FONT", 32,            "Font",            "Font", _, _, _, false},
#undef _
};

const int kMaxVectorSize = 4;

struct BuiltinInfo {
  clang::BuiltinType::Kind builtinTypeKind;
  DataType type;
  /* TODO If we return std::string instead of llvm::StringRef, we could build
   * the name instead of duplicating the entries.
   */
  const char *cname[kMaxVectorSize];
};


BuiltinInfo BuiltinInfoTable[] = {
    {clang::BuiltinType::Bool, DataTypeBoolean,
     {"bool", "bool2", "bool3", "bool4"}},
    {clang::BuiltinType::Char_U, DataTypeUnsigned8,
     {"uchar", "uchar2", "uchar3", "uchar4"}},
    {clang::BuiltinType::UChar, DataTypeUnsigned8,
     {"uchar", "uchar2", "uchar3", "uchar4"}},
    {clang::BuiltinType::Char16, DataTypeSigned16,
     {"short", "short2", "short3", "short4"}},
    {clang::BuiltinType::Char32, DataTypeSigned32,
     {"int", "int2", "int3", "int4"}},
    {clang::BuiltinType::UShort, DataTypeUnsigned16,
     {"ushort", "ushort2", "ushort3", "ushort4"}},
    {clang::BuiltinType::UInt, DataTypeUnsigned32,
     {"uint", "uint2", "uint3", "uint4"}},
    {clang::BuiltinType::ULong, DataTypeUnsigned64,
     {"ulong", "ulong2", "ulong3", "ulong4"}},
    {clang::BuiltinType::ULongLong, DataTypeUnsigned64,
     {"ulong", "ulong2", "ulong3", "ulong4"}},

    {clang::BuiltinType::Char_S, DataTypeSigned8,
     {"char", "char2", "char3", "char4"}},
    {clang::BuiltinType::SChar, DataTypeSigned8,
     {"char", "char2", "char3", "char4"}},
    {clang::BuiltinType::Short, DataTypeSigned16,
     {"short", "short2", "short3", "short4"}},
    {clang::BuiltinType::Int, DataTypeSigned32,
     {"int", "int2", "int3", "int4"}},
    {clang::BuiltinType::Long, DataTypeSigned64,
     {"long", "long2", "long3", "long4"}},
    {clang::BuiltinType::LongLong, DataTypeSigned64,
     {"long", "long2", "long3", "long4"}},
    {clang::BuiltinType::Half, DataTypeFloat16,
     {"half", "half2", "half3", "half4"}},
    {clang::BuiltinType::Float, DataTypeFloat32,
     {"float", "float2", "float3", "float4"}},
    {clang::BuiltinType::Double, DataTypeFloat64,
     {"double", "double2", "double3", "double4"}},
};
const int BuiltinInfoTableCount = sizeof(BuiltinInfoTable) / sizeof(BuiltinInfoTable[0]);

struct NameAndPrimitiveType {
  const char *name;
  DataType dataType;
};

static NameAndPrimitiveType MatrixAndObjectDataTypes[] = {
    {"rs_matrix2x2", DataTypeRSMatrix2x2},
    {"rs_matrix3x3", DataTypeRSMatrix3x3},
    {"rs_matrix4x4", DataTypeRSMatrix4x4},
    {"rs_element", DataTypeRSElement},
    {"rs_type", DataTypeRSType},
    {"rs_allocation", DataTypeRSAllocation},
    {"rs_sampler", DataTypeRSSampler},
    {"rs_script", DataTypeRSScript},
    {"rs_mesh", DataTypeRSMesh},
    {"rs_path", DataTypeRSPath},
    {"rs_program_fragment", DataTypeRSProgramFragment},
    {"rs_program_vertex", DataTypeRSProgramVertex},
    {"rs_program_raster", DataTypeRSProgramRaster},
    {"rs_program_store", DataTypeRSProgramStore},
    {"rs_font", DataTypeRSFont},
};

const int MatrixAndObjectDataTypesCount =
    sizeof(MatrixAndObjectDataTypes) / sizeof(MatrixAndObjectDataTypes[0]);

static const clang::Type *TypeExportableHelper(
    const clang::Type *T,
    llvm::SmallPtrSet<const clang::Type*, 8>& SPS,
    slang::RSContext *Context,
    const clang::VarDecl *VD,
    const clang::RecordDecl *TopLevelRecord,
    ExportKind EK);

template <unsigned N>
static void ReportTypeError(slang::RSContext *Context,
                            const clang::NamedDecl *ND,
                            const clang::RecordDecl *TopLevelRecord,
                            const char (&Message)[N],
                            unsigned int TargetAPI = 0) {
  // Attempt to use the type declaration first (if we have one).
  // Fall back to the variable definition, if we are looking at something
  // like an array declaration that can't be exported.
  if (TopLevelRecord) {
    Context->ReportError(TopLevelRecord->getLocation(), Message)
        << TopLevelRecord->getName() << TargetAPI;
  } else if (ND) {
    Context->ReportError(ND->getLocation(), Message) << ND->getName()
                                                     << TargetAPI;
  } else {
    slangAssert(false && "Variables should be validated before exporting");
  }
}

static const clang::Type *ConstantArrayTypeExportableHelper(
    const clang::ConstantArrayType *CAT,
    llvm::SmallPtrSet<const clang::Type*, 8>& SPS,
    slang::RSContext *Context,
    const clang::VarDecl *VD,
    const clang::RecordDecl *TopLevelRecord,
    ExportKind EK) {
  // Check element type
  const clang::Type *ElementType = GetConstantArrayElementType(CAT);
  if (ElementType->isArrayType()) {
    ReportTypeError(Context, VD, TopLevelRecord,
                    "multidimensional arrays cannot be exported: '%0'");
    return nullptr;
  } else if (ElementType->isExtVectorType()) {
    const clang::ExtVectorType *EVT =
        static_cast<const clang::ExtVectorType*>(ElementType);
    unsigned numElements = EVT->getNumElements();

    const clang::Type *BaseElementType = GetExtVectorElementType(EVT);
    if (!RSExportPrimitiveType::IsPrimitiveType(BaseElementType)) {
      ReportTypeError(Context, VD, TopLevelRecord,
        "vectors of non-primitive types cannot be exported: '%0'");
      return nullptr;
    }

    if (numElements == 3 && CAT->getSize() != 1) {
      ReportTypeError(Context, VD, TopLevelRecord,
        "arrays of width 3 vector types cannot be exported: '%0'");
      return nullptr;
    }
  }

  if (TypeExportableHelper(ElementType, SPS, Context, VD,
                           TopLevelRecord, EK) == nullptr) {
    return nullptr;
  } else {
    return CAT;
  }
}

BuiltinInfo *FindBuiltinType(clang::BuiltinType::Kind builtinTypeKind) {
  for (int i = 0; i < BuiltinInfoTableCount; i++) {
    if (builtinTypeKind == BuiltinInfoTable[i].builtinTypeKind) {
      return &BuiltinInfoTable[i];
    }
  }
  return nullptr;
}

static const clang::Type *TypeExportableHelper(
    clang::Type const *T,
    llvm::SmallPtrSet<clang::Type const *, 8> &SPS,
    slang::RSContext *Context,
    clang::VarDecl const *VD,
    clang::RecordDecl const *TopLevelRecord,
    ExportKind EK) {
  // Normalize first
  if ((T = GetCanonicalType(T)) == nullptr)
    return nullptr;

  if (SPS.count(T))
    return T;

  const clang::Type *CTI = T->getCanonicalTypeInternal().getTypePtr();

  switch (T->getTypeClass()) {
    case clang::Type::Builtin: {
      const clang::BuiltinType *BT = static_cast<const clang::BuiltinType*>(CTI);
      return FindBuiltinType(BT->getKind()) == nullptr ? nullptr : T;
    }
    case clang::Type::Record: {
      if (RSExportPrimitiveType::GetRSSpecificType(T) != DataTypeUnknown) {
        return T;  // RS object type, no further checks are needed
      }

      // Check internal struct
      if (T->isUnionType()) {
        ReportTypeError(Context, VD, T->getAsUnionType()->getDecl(),
                        "unions cannot be exported: '%0'");
        return nullptr;
      } else if (!T->isStructureType()) {
        slangAssert(false && "Unknown type cannot be exported");
        return nullptr;
      }

      clang::RecordDecl *RD = T->getAsStructureType()->getDecl();
      slangAssert(RD);
      RD = RD->getDefinition();
      if (RD == nullptr) {
        ReportTypeError(Context, nullptr, T->getAsStructureType()->getDecl(),
                        "struct is not defined in this module");
        return nullptr;
      }

      if (!TopLevelRecord) {
        TopLevelRecord = RD;
      }
      if (RD->getName().empty()) {
        ReportTypeError(Context, nullptr, RD,
                        "anonymous structures cannot be exported");
        return nullptr;
      }

      // Fast check
      if (RD->hasFlexibleArrayMember() || RD->hasObjectMember())
        return nullptr;

      // Insert myself into checking set
      SPS.insert(T);

      // Check all element
      for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
               FE = RD->field_end();
           FI != FE;
           FI++) {
        const clang::FieldDecl *FD = *FI;
        const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
        FT = GetCanonicalType(FT);

        if (!TypeExportableHelper(FT, SPS, Context, VD, TopLevelRecord,
                                  EK)) {
          return nullptr;
        }

        // We don't support bit fields yet
        //
        // TODO(zonr/srhines): allow bit fields of size 8, 16, 32
        if (FD->isBitField()) {
          Context->ReportError(
              FD->getLocation(),
              "bit fields are not able to be exported: '%0.%1'")
              << RD->getName() << FD->getName();
          return nullptr;
        }
      }

      return T;
    }
    case clang::Type::FunctionProto:
    case clang::Type::FunctionNoProto:
      ReportTypeError(Context, VD, TopLevelRecord,
                      "function types cannot be exported: '%0'");
      return nullptr;
    case clang::Type::Pointer: {
      if (TopLevelRecord) {
        ReportTypeError(Context, VD, TopLevelRecord,
            "structures containing pointers cannot be used as the type of "
            "an exported global variable or the parameter to an exported "
            "function: '%0'");
        return nullptr;
      }

      const clang::PointerType *PT = static_cast<const clang::PointerType*>(CTI);
      const clang::Type *PointeeType = GetPointeeType(PT);

      if (PointeeType->getTypeClass() == clang::Type::Pointer) {
        ReportTypeError(Context, VD, TopLevelRecord,
            "multiple levels of pointers cannot be exported: '%0'");
        return nullptr;
      }

      // Void pointers are forbidden for export, although we must accept
      // void pointers that come in as arguments to a legacy kernel.
      if (PointeeType->isVoidType() && EK != LegacyKernelArgument) {
        ReportTypeError(Context, VD, TopLevelRecord,
            "void pointers cannot be exported: '%0'");
        return nullptr;
      }

      // We don't support pointer with array-type pointee
      if (PointeeType->isArrayType()) {
        ReportTypeError(Context, VD, TopLevelRecord,
            "pointers to arrays cannot be exported: '%0'");
        return nullptr;
      }

      // Check for unsupported pointee type
      if (TypeExportableHelper(PointeeType, SPS, Context, VD,
                                TopLevelRecord, EK) == nullptr)
        return nullptr;
      else
        return T;
    }
    case clang::Type::ExtVector: {
      const clang::ExtVectorType *EVT =
              static_cast<const clang::ExtVectorType*>(CTI);
      // Only vector with size 2, 3 and 4 are supported.
      if (EVT->getNumElements() < 2 || EVT->getNumElements() > 4)
        return nullptr;

      // Check base element type
      const clang::Type *ElementType = GetExtVectorElementType(EVT);

      if ((ElementType->getTypeClass() != clang::Type::Builtin) ||
          (TypeExportableHelper(ElementType, SPS, Context, VD,
                                TopLevelRecord, EK) == nullptr))
        return nullptr;
      else
        return T;
    }
    case clang::Type::ConstantArray: {
      const clang::ConstantArrayType *CAT =
              static_cast<const clang::ConstantArrayType*>(CTI);

      return ConstantArrayTypeExportableHelper(CAT, SPS, Context, VD,
                                               TopLevelRecord, EK);
    }
    case clang::Type::Enum: {
      // FIXME: We currently convert enums to integers, rather than reflecting
      // a more complete (and nicer type-safe Java version).
      return Context->getASTContext().IntTy.getTypePtr();
    }
    default: {
      slangAssert(false && "Unknown type cannot be validated");
      return nullptr;
    }
  }
}

// Return the type that can be used to create RSExportType, will always return
// the canonical type.
//
// If the Type T is not exportable, this function returns nullptr. DiagEngine is
// used to generate proper Clang diagnostic messages when a non-exportable type
// is detected. TopLevelRecord is used to capture the highest struct (in the
// case of a nested hierarchy) for detecting other types that cannot be exported
// (mostly pointers within a struct).
static const clang::Type *TypeExportable(const clang::Type *T,
                                         slang::RSContext *Context,
                                         const clang::VarDecl *VD,
                                         ExportKind EK) {
  llvm::SmallPtrSet<const clang::Type*, 8> SPS =
      llvm::SmallPtrSet<const clang::Type*, 8>();

  return TypeExportableHelper(T, SPS, Context, VD, nullptr, EK);
}

static bool ValidateRSObjectInVarDecl(slang::RSContext *Context,
                                      const clang::VarDecl *VD, bool InCompositeType,
                                      unsigned int TargetAPI) {
  if (TargetAPI < SLANG_JB_TARGET_API) {
    // Only if we are already in a composite type (like an array or structure).
    if (InCompositeType) {
      // Only if we are actually exported (i.e. non-static).
      if (VD->hasLinkage() &&
          (VD->getFormalLinkage() == clang::ExternalLinkage)) {
        // Only if we are not a pointer to an object.
        const clang::Type *T = GetCanonicalType(VD->getType().getTypePtr());
        if (T->getTypeClass() != clang::Type::Pointer) {
          ReportTypeError(Context, VD, nullptr,
                          "arrays/structures containing RS object types "
                          "cannot be exported in target API < %1: '%0'",
                          SLANG_JB_TARGET_API);
          return false;
        }
      }
    }
  }

  return true;
}

// Helper function for ValidateType(). We do a recursive descent on the
// type hierarchy to ensure that we can properly export/handle the
// declaration.
// \return true if the variable declaration is valid,
//         false if it is invalid (along with proper diagnostics).
//
// C - ASTContext (for diagnostics + builtin types).
// T - sub-type that we are validating.
// ND - (optional) top-level named declaration that we are validating.
// SPS - set of types we have already seen/validated.
// InCompositeType - true if we are within an outer composite type.
// UnionDecl - set if we are in a sub-type of a union.
// TargetAPI - target SDK API level.
// IsFilterscript - whether or not we are compiling for Filterscript
// IsExtern - is this type externally visible (i.e. extern global or parameter
//                                             to an extern function)
static bool ValidateTypeHelper(
    slang::RSContext *Context,
    clang::ASTContext &C,
    const clang::Type *&T,
    const clang::NamedDecl *ND,
    clang::SourceLocation Loc,
    llvm::SmallPtrSet<const clang::Type*, 8>& SPS,
    bool InCompositeType,
    clang::RecordDecl *UnionDecl,
    unsigned int TargetAPI,
    bool IsFilterscript,
    bool IsExtern) {
  if ((T = GetCanonicalType(T)) == nullptr)
    return true;

  if (SPS.count(T))
    return true;

  const clang::Type *CTI = T->getCanonicalTypeInternal().getTypePtr();

  switch (T->getTypeClass()) {
    case clang::Type::Record: {
      if (RSExportPrimitiveType::IsRSObjectType(T)) {
        const clang::VarDecl *VD = (ND ? llvm::dyn_cast<clang::VarDecl>(ND) : nullptr);
        if (VD && !ValidateRSObjectInVarDecl(Context, VD, InCompositeType,
                                             TargetAPI)) {
          return false;
        }
      }

      if (RSExportPrimitiveType::GetRSSpecificType(T) != DataTypeUnknown) {
        if (!UnionDecl) {
          return true;
        } else if (RSExportPrimitiveType::IsRSObjectType(T)) {
          ReportTypeError(Context, nullptr, UnionDecl,
              "unions containing RS object types are not allowed");
          return false;
        }
      }

      clang::RecordDecl *RD = nullptr;

      // Check internal struct
      if (T->isUnionType()) {
        RD = T->getAsUnionType()->getDecl();
        UnionDecl = RD;
      } else if (T->isStructureType()) {
        RD = T->getAsStructureType()->getDecl();
      } else {
        slangAssert(false && "Unknown type cannot be exported");
        return false;
      }

      slangAssert(RD);
      RD = RD->getDefinition();
      if (RD == nullptr) {
        // FIXME
        return true;
      }

      // Fast check
      if (RD->hasFlexibleArrayMember() || RD->hasObjectMember())
        return false;

      // Insert myself into checking set
      SPS.insert(T);

      // Check all elements
      for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
               FE = RD->field_end();
           FI != FE;
           FI++) {
        const clang::FieldDecl *FD = *FI;
        const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
        FT = GetCanonicalType(FT);

        if (!ValidateTypeHelper(Context, C, FT, ND, Loc, SPS, true, UnionDecl,
                                TargetAPI, IsFilterscript, IsExtern)) {
          return false;
        }
      }

      return true;
    }

    case clang::Type::Builtin: {
      if (IsFilterscript) {
        clang::QualType QT = T->getCanonicalTypeInternal();
        if (QT == C.DoubleTy ||
            QT == C.LongDoubleTy ||
            QT == C.LongTy ||
            QT == C.LongLongTy) {
          if (ND) {
            Context->ReportError(
                Loc,
                "Builtin types > 32 bits in size are forbidden in "
                "Filterscript: '%0'")
                << ND->getName();
          } else {
            Context->ReportError(
                Loc,
                "Builtin types > 32 bits in size are forbidden in "
                "Filterscript");
          }
          return false;
        }
      }
      break;
    }

    case clang::Type::Pointer: {
      if (IsFilterscript) {
        if (ND) {
          Context->ReportError(Loc,
                               "Pointers are forbidden in Filterscript: '%0'")
              << ND->getName();
          return false;
        } else {
          // TODO(srhines): Find a better way to handle expressions (i.e. no
          // NamedDecl) involving pointers in FS that should be allowed.
          // An example would be calls to library functions like
          // rsMatrixMultiply() that take rs_matrixNxN * types.
        }
      }

      // Forbid pointers in structures that are externally visible.
      if (InCompositeType && IsExtern) {
        if (ND) {
          Context->ReportError(Loc,
              "structures containing pointers cannot be used as the type of "
              "an exported global variable or the parameter to an exported "
              "function: '%0'")
            << ND->getName();
        } else {
          Context->ReportError(Loc,
              "structures containing pointers cannot be used as the type of "
              "an exported global variable or the parameter to an exported "
              "function");
        }
        return false;
      }

      const clang::PointerType *PT = static_cast<const clang::PointerType*>(CTI);
      const clang::Type *PointeeType = GetPointeeType(PT);

      return ValidateTypeHelper(Context, C, PointeeType, ND, Loc, SPS,
                                InCompositeType, UnionDecl, TargetAPI,
                                IsFilterscript, IsExtern);
    }

    case clang::Type::ExtVector: {
      const clang::ExtVectorType *EVT =
              static_cast<const clang::ExtVectorType*>(CTI);
      const clang::Type *ElementType = GetExtVectorElementType(EVT);
      if (TargetAPI < SLANG_ICS_TARGET_API &&
          InCompositeType &&
          EVT->getNumElements() == 3 &&
          ND &&
          ND->getFormalLinkage() == clang::ExternalLinkage) {
        ReportTypeError(Context, ND, nullptr,
                        "structs containing vectors of dimension 3 cannot "
                        "be exported at this API level: '%0'");
        return false;
      }
      return ValidateTypeHelper(Context, C, ElementType, ND, Loc, SPS, true,
                                UnionDecl, TargetAPI, IsFilterscript, IsExtern);
    }

    case clang::Type::ConstantArray: {
      const clang::ConstantArrayType *CAT = static_cast<const clang::ConstantArrayType*>(CTI);
      const clang::Type *ElementType = GetConstantArrayElementType(CAT);
      return ValidateTypeHelper(Context, C, ElementType, ND, Loc, SPS, true,
                                UnionDecl, TargetAPI, IsFilterscript, IsExtern);
    }

    default: {
      break;
    }
  }

  return true;
}

}  // namespace

std::string CreateDummyName(const char *type, const std::string &name) {
  std::stringstream S;
  S << "<" << type;
  if (!name.empty()) {
    S << ":" << name;
  }
  S << ">";
  return S.str();
}

/****************************** RSExportType ******************************/
bool RSExportType::NormalizeType(const clang::Type *&T,
                                 llvm::StringRef &TypeName,
                                 RSContext *Context,
                                 const clang::VarDecl *VD,
                                 ExportKind EK) {
  if ((T = TypeExportable(T, Context, VD, EK)) == nullptr) {
    return false;
  }
  // Get type name
  TypeName = RSExportType::GetTypeName(T);
  if (Context && TypeName.empty()) {
    if (VD) {
      Context->ReportError(VD->getLocation(),
                           "anonymous types cannot be exported");
    } else {
      Context->ReportError("anonymous types cannot be exported");
    }
    return false;
  }

  return true;
}

bool RSExportType::ValidateType(slang::RSContext *Context, clang::ASTContext &C,
                                clang::QualType QT, const clang::NamedDecl *ND,
                                clang::SourceLocation Loc,
                                unsigned int TargetAPI, bool IsFilterscript,
                                bool IsExtern) {
  const clang::Type *T = QT.getTypePtr();
  llvm::SmallPtrSet<const clang::Type*, 8> SPS =
      llvm::SmallPtrSet<const clang::Type*, 8>();

  // If this is an externally visible variable declaration, we check if the
  // type is able to be exported first.
  if (auto VD = llvm::dyn_cast_or_null<clang::VarDecl>(ND)) {
    if (VD->getFormalLinkage() == clang::ExternalLinkage) {
      if (!TypeExportable(T, Context, VD, NotLegacyKernelArgument)) {
        return false;
      }
    }
  }
  return ValidateTypeHelper(Context, C, T, ND, Loc, SPS, false, nullptr, TargetAPI,
                            IsFilterscript, IsExtern);
}

bool RSExportType::ValidateVarDecl(slang::RSContext *Context,
                                   clang::VarDecl *VD, unsigned int TargetAPI,
                                   bool IsFilterscript) {
  return ValidateType(Context, VD->getASTContext(), VD->getType(), VD,
                      VD->getLocation(), TargetAPI, IsFilterscript,
                      (VD->getFormalLinkage() == clang::ExternalLinkage));
}

const clang::Type
*RSExportType::GetTypeOfDecl(const clang::DeclaratorDecl *DD) {
  if (DD) {
    clang::QualType T = DD->getType();

    if (T.isNull())
      return nullptr;
    else
      return T.getTypePtr();
  }
  return nullptr;
}

llvm::StringRef RSExportType::GetTypeName(const clang::Type* T) {
  T = GetCanonicalType(T);
  if (T == nullptr)
    return llvm::StringRef();

  const clang::Type *CTI = T->getCanonicalTypeInternal().getTypePtr();

  switch (T->getTypeClass()) {
    case clang::Type::Builtin: {
      const clang::BuiltinType *BT = static_cast<const clang::BuiltinType*>(CTI);
      BuiltinInfo *info = FindBuiltinType(BT->getKind());
      if (info != nullptr) {
        return info->cname[0];
      }
      slangAssert(false && "Unknown data type of the builtin");
      break;
    }
    case clang::Type::Record: {
      clang::RecordDecl *RD;
      if (T->isStructureType()) {
        RD = T->getAsStructureType()->getDecl();
      } else {
        break;
      }

      llvm::StringRef Name = RD->getName();
      if (Name.empty()) {
        if (RD->getTypedefNameForAnonDecl() != nullptr) {
          Name = RD->getTypedefNameForAnonDecl()->getName();
        }

        if (Name.empty()) {
          // Try to find a name from redeclaration (i.e. typedef)
          for (clang::TagDecl::redecl_iterator RI = RD->redecls_begin(),
                   RE = RD->redecls_end();
               RI != RE;
               RI++) {
            slangAssert(*RI != nullptr && "cannot be NULL object");

            Name = (*RI)->getName();
            if (!Name.empty())
              break;
          }
        }
      }
      return Name;
    }
    case clang::Type::Pointer: {
      // "*" plus pointee name
      const clang::PointerType *P = static_cast<const clang::PointerType*>(CTI);
      const clang::Type *PT = GetPointeeType(P);
      llvm::StringRef PointeeName;
      if (NormalizeType(PT, PointeeName, nullptr, nullptr,
                        NotLegacyKernelArgument)) {
        char *Name = new char[ 1 /* * */ + PointeeName.size() + 1 ];
        Name[0] = '*';
        memcpy(Name + 1, PointeeName.data(), PointeeName.size());
        Name[PointeeName.size() + 1] = '\0';
        return Name;
      }
      break;
    }
    case clang::Type::ExtVector: {
      const clang::ExtVectorType *EVT =
              static_cast<const clang::ExtVectorType*>(CTI);
      return RSExportVectorType::GetTypeName(EVT);
      break;
    }
    case clang::Type::ConstantArray : {
      // Construct name for a constant array is too complicated.
      return "<ConstantArray>";
    }
    default: {
      break;
    }
  }

  return llvm::StringRef();
}


RSExportType *RSExportType::Create(RSContext *Context,
                                   const clang::Type *T,
                                   const llvm::StringRef &TypeName,
                                   ExportKind EK) {
  // Lookup the context to see whether the type was processed before.
  // Newly created RSExportType will insert into context
  // in RSExportType::RSExportType()
  RSContext::export_type_iterator ETI = Context->findExportType(TypeName);

  if (ETI != Context->export_types_end())
    return ETI->second;

  const clang::Type *CTI = T->getCanonicalTypeInternal().getTypePtr();

  RSExportType *ET = nullptr;
  switch (T->getTypeClass()) {
    case clang::Type::Record: {
      DataType dt = RSExportPrimitiveType::GetRSSpecificType(TypeName);
      switch (dt) {
        case DataTypeUnknown: {
          // User-defined types
          ET = RSExportRecordType::Create(Context,
                                          T->getAsStructureType(),
                                          TypeName);
          break;
        }
        case DataTypeRSMatrix2x2: {
          // 2 x 2 Matrix type
          ET = RSExportMatrixType::Create(Context,
                                          T->getAsStructureType(),
                                          TypeName,
                                          2);
          break;
        }
        case DataTypeRSMatrix3x3: {
          // 3 x 3 Matrix type
          ET = RSExportMatrixType::Create(Context,
                                          T->getAsStructureType(),
                                          TypeName,
                                          3);
          break;
        }
        case DataTypeRSMatrix4x4: {
          // 4 x 4 Matrix type
          ET = RSExportMatrixType::Create(Context,
                                          T->getAsStructureType(),
                                          TypeName,
                                          4);
          break;
        }
        default: {
          // Others are primitive types
          ET = RSExportPrimitiveType::Create(Context, T, TypeName);
          break;
        }
      }
      break;
    }
    case clang::Type::Builtin: {
      ET = RSExportPrimitiveType::Create(Context, T, TypeName);
      break;
    }
    case clang::Type::Pointer: {
      ET = RSExportPointerType::Create(Context,
                                       static_cast<const clang::PointerType*>(CTI),
                                       TypeName);
      // FIXME: free the name (allocated in RSExportType::GetTypeName)
      delete [] TypeName.data();
      break;
    }
    case clang::Type::ExtVector: {
      ET = RSExportVectorType::Create(Context,
                                      static_cast<const clang::ExtVectorType*>(CTI),
                                      TypeName);
      break;
    }
    case clang::Type::ConstantArray: {
      ET = RSExportConstantArrayType::Create(
              Context,
              static_cast<const clang::ConstantArrayType*>(CTI));
      break;
    }
    default: {
      Context->ReportError("unknown type cannot be exported: '%0'")
          << T->getTypeClassName();
      break;
    }
  }

  return ET;
}

RSExportType *RSExportType::Create(RSContext *Context, const clang::Type *T,
                                   ExportKind EK, const clang::VarDecl *VD) {
  llvm::StringRef TypeName;
  if (NormalizeType(T, TypeName, Context, VD, EK)) {
    return Create(Context, T, TypeName, EK);
  } else {
    return nullptr;
  }
}

RSExportType *RSExportType::CreateFromDecl(RSContext *Context,
                                           const clang::VarDecl *VD) {
  return RSExportType::Create(Context, GetTypeOfDecl(VD),
                              NotLegacyKernelArgument, VD);
}

size_t RSExportType::getStoreSize() const {
  return getRSContext()->getDataLayout().getTypeStoreSize(getLLVMType());
}

size_t RSExportType::getAllocSize() const {
    return getRSContext()->getDataLayout().getTypeAllocSize(getLLVMType());
}

RSExportType::RSExportType(RSContext *Context,
                           ExportClass Class,
                           const llvm::StringRef &Name, clang::SourceLocation Loc)
    : RSExportable(Context, RSExportable::EX_TYPE, Loc),
      mClass(Class),
      // Make a copy on Name since memory stored @Name is either allocated in
      // ASTContext or allocated in GetTypeName which will be destroyed later.
      mName(Name.data(), Name.size()),
      mLLVMType(nullptr) {
  // Don't cache the type whose name start with '<'. Those type failed to
  // get their name since constructing their name in GetTypeName() requiring
  // complicated work.
  if (!IsDummyName(Name)) {
    // TODO(zonr): Need to check whether the insertion is successful or not.
    Context->insertExportType(llvm::StringRef(Name), this);
  }

}

bool RSExportType::keep() {
  if (!RSExportable::keep())
    return false;
  // Invalidate converted LLVM type.
  mLLVMType = nullptr;
  return true;
}

bool RSExportType::matchODR(const RSExportType *E, bool /* LookInto */) const {
  return (E->getClass() == getClass());
}

RSExportType::~RSExportType() {
}

/************************** RSExportPrimitiveType **************************/
llvm::ManagedStatic<RSExportPrimitiveType::RSSpecificTypeMapTy>
RSExportPrimitiveType::RSSpecificTypeMap;

bool RSExportPrimitiveType::IsPrimitiveType(const clang::Type *T) {
  if ((T != nullptr) && (T->getTypeClass() == clang::Type::Builtin))
    return true;
  else
    return false;
}

DataType
RSExportPrimitiveType::GetRSSpecificType(const llvm::StringRef &TypeName) {
  if (TypeName.empty())
    return DataTypeUnknown;

  if (RSSpecificTypeMap->empty()) {
    for (int i = 0; i < MatrixAndObjectDataTypesCount; i++) {
      (*RSSpecificTypeMap)[MatrixAndObjectDataTypes[i].name] =
          MatrixAndObjectDataTypes[i].dataType;
    }
  }

  RSSpecificTypeMapTy::const_iterator I = RSSpecificTypeMap->find(TypeName);
  if (I == RSSpecificTypeMap->end())
    return DataTypeUnknown;
  else
    return I->getValue();
}

DataType RSExportPrimitiveType::GetRSSpecificType(const clang::Type *T) {
  T = GetCanonicalType(T);
  if ((T == nullptr) || (T->getTypeClass() != clang::Type::Record))
    return DataTypeUnknown;

  return GetRSSpecificType( RSExportType::GetTypeName(T) );
}

bool RSExportPrimitiveType::IsRSMatrixType(DataType DT) {
    if (DT < 0 || DT >= DataTypeMax) {
        return false;
    }
    return gReflectionTypes[DT].category == MatrixDataType;
}

bool RSExportPrimitiveType::IsRSObjectType(DataType DT) {
    if (DT < 0 || DT >= DataTypeMax) {
        return false;
    }
    return gReflectionTypes[DT].category == ObjectDataType;
}

bool RSExportPrimitiveType::IsStructureTypeWithRSObject(const clang::Type *T) {
  bool RSObjectTypeSeen = false;
  slangAssert(T);
  while (T->isArrayType()) {
    T = T->getArrayElementTypeNoTypeQual();
    slangAssert(T);
  }

  const clang::RecordType *RT = T->getAsStructureType();
  if (!RT) {
    return false;
  }

  const clang::RecordDecl *RD = RT->getDecl();
  if (RD) {
    RD = RD->getDefinition();
  }
  if (!RD) {
    return false;
  }

  for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
         FE = RD->field_end();
       FI != FE;
       FI++) {
    // We just look through all field declarations to see if we find a
    // declaration for an RS object type (or an array of one).
    const clang::FieldDecl *FD = *FI;
    const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
    slangAssert(FT);
    while (FT->isArrayType()) {
      FT = FT->getArrayElementTypeNoTypeQual();
      slangAssert(FT);
    }

    DataType DT = GetRSSpecificType(FT);
    if (IsRSObjectType(DT)) {
      // RS object types definitely need to be zero-initialized
      RSObjectTypeSeen = true;
    } else {
      switch (DT) {
        case DataTypeRSMatrix2x2:
        case DataTypeRSMatrix3x3:
        case DataTypeRSMatrix4x4:
          // Matrix types should get zero-initialized as well
          RSObjectTypeSeen = true;
          break;
        default:
          // Ignore all other primitive types
          break;
      }
      if (FT->isStructureType()) {
        // Recursively handle structs of structs (even though these can't
        // be exported, it is possible for a user to have them internally).
        RSObjectTypeSeen |= IsStructureTypeWithRSObject(FT);
      }
    }
  }

  return RSObjectTypeSeen;
}

size_t RSExportPrimitiveType::GetElementSizeInBits(const RSExportPrimitiveType *EPT) {
  int type = EPT->getType();
  slangAssert((type > DataTypeUnknown && type < DataTypeMax) &&
              "RSExportPrimitiveType::GetElementSizeInBits : unknown data type");
  // All RS object types are 256 bits in 64-bit RS.
  if (EPT->isRSObjectType() && EPT->getRSContext()->is64Bit()) {
    return 256;
  }
  return gReflectionTypes[type].size_in_bits;
}

DataType
RSExportPrimitiveType::GetDataType(RSContext *Context, const clang::Type *T) {
  if (T == nullptr)
    return DataTypeUnknown;

  switch (T->getTypeClass()) {
    case clang::Type::Builtin: {
      const clang::BuiltinType *BT =
              static_cast<const clang::BuiltinType*>(T->getCanonicalTypeInternal().getTypePtr());
      BuiltinInfo *info = FindBuiltinType(BT->getKind());
      if (info != nullptr) {
        return info->type;
      }
      // The size of type WChar depend on platform so we abandon the support
      // to them.
      Context->ReportError("built-in type cannot be exported: '%0'")
          << T->getTypeClassName();
      break;
    }
    case clang::Type::Record: {
      // must be RS object type
      return RSExportPrimitiveType::GetRSSpecificType(T);
    }
    default: {
      Context->ReportError("primitive type cannot be exported: '%0'")
          << T->getTypeClassName();
      break;
    }
  }

  return DataTypeUnknown;
}

RSExportPrimitiveType
*RSExportPrimitiveType::Create(RSContext *Context,
                               const clang::Type *T,
                               const llvm::StringRef &TypeName,
                               bool Normalized) {
  DataType DT = GetDataType(Context, T);

  if ((DT == DataTypeUnknown) || TypeName.empty())
    return nullptr;
  else
    return new RSExportPrimitiveType(Context, ExportClassPrimitive, TypeName,
                                     DT, Normalized);
}

RSExportPrimitiveType *RSExportPrimitiveType::Create(RSContext *Context,
                                                     const clang::Type *T) {
  llvm::StringRef TypeName;
  if (RSExportType::NormalizeType(T, TypeName, Context, nullptr,
                                  NotLegacyKernelArgument) &&
      IsPrimitiveType(T)) {
    return Create(Context, T, TypeName);
  } else {
    return nullptr;
  }
}

llvm::Type *RSExportPrimitiveType::convertToLLVMType() const {
  llvm::LLVMContext &C = getRSContext()->getLLVMContext();

  if (isRSObjectType()) {
    // struct {
    //   int *p;
    // } __attribute__((packed, aligned(pointer_size)))
    //
    // which is
    //
    // <{ [1 x i32] }> in LLVM
    //
    std::vector<llvm::Type *> Elements;
    if (getRSContext()->is64Bit()) {
      // 64-bit path
      Elements.push_back(llvm::ArrayType::get(llvm::Type::getInt64Ty(C), 4));
      return llvm::StructType::get(C, Elements, true);
    } else {
      // 32-bit legacy path
      Elements.push_back(llvm::ArrayType::get(llvm::Type::getInt32Ty(C), 1));
      return llvm::StructType::get(C, Elements, true);
    }
  }

  switch (mType) {
    case DataTypeFloat16: {
      return llvm::Type::getHalfTy(C);
      break;
    }
    case DataTypeFloat32: {
      return llvm::Type::getFloatTy(C);
      break;
    }
    case DataTypeFloat64: {
      return llvm::Type::getDoubleTy(C);
      break;
    }
    case DataTypeBoolean: {
      return llvm::Type::getInt1Ty(C);
      break;
    }
    case DataTypeSigned8:
    case DataTypeUnsigned8: {
      return llvm::Type::getInt8Ty(C);
      break;
    }
    case DataTypeSigned16:
    case DataTypeUnsigned16:
    case DataTypeUnsigned565:
    case DataTypeUnsigned5551:
    case DataTypeUnsigned4444: {
      return llvm::Type::getInt16Ty(C);
      break;
    }
    case DataTypeSigned32:
    case DataTypeUnsigned32: {
      return llvm::Type::getInt32Ty(C);
      break;
    }
    case DataTypeSigned64:
    case DataTypeUnsigned64: {
      return llvm::Type::getInt64Ty(C);
      break;
    }
    default: {
      slangAssert(false && "Unknown data type");
    }
  }

  return nullptr;
}

bool RSExportPrimitiveType::matchODR(const RSExportType *E,
                                     bool /* LookInto */) const {
  CHECK_PARENT_EQUALITY(RSExportType, E);
  return (static_cast<const RSExportPrimitiveType*>(E)->getType() == getType());
}

RSReflectionType *RSExportPrimitiveType::getRSReflectionType(DataType DT) {
  if (DT > DataTypeUnknown && DT < DataTypeMax) {
    return &gReflectionTypes[DT];
  } else {
    return nullptr;
  }
}

/**************************** RSExportPointerType ****************************/

RSExportPointerType
*RSExportPointerType::Create(RSContext *Context,
                             const clang::PointerType *PT,
                             const llvm::StringRef &TypeName) {
  const clang::Type *PointeeType = GetPointeeType(PT);
  const RSExportType *PointeeET;

  if (PointeeType->getTypeClass() != clang::Type::Pointer) {
    PointeeET = RSExportType::Create(Context, PointeeType,
                                     NotLegacyKernelArgument);
  } else {
    // Double or higher dimension of pointer, export as int*
    PointeeET = RSExportPrimitiveType::Create(Context,
                    Context->getASTContext().IntTy.getTypePtr());
  }

  if (PointeeET == nullptr) {
    // Error diagnostic is emitted for corresponding pointee type
    return nullptr;
  }

  return new RSExportPointerType(Context, TypeName, PointeeET);
}

llvm::Type *RSExportPointerType::convertToLLVMType() const {
  llvm::Type *PointeeType = mPointeeType->getLLVMType();
  return llvm::PointerType::getUnqual(PointeeType);
}

bool RSExportPointerType::keep() {
  if (!RSExportType::keep())
    return false;
  const_cast<RSExportType*>(mPointeeType)->keep();
  return true;
}

bool RSExportPointerType::matchODR(const RSExportType *E,
                                   bool /* LookInto */) const {
  // Exported types cannot contain pointers
  slangAssert(false && "Not supposed to perform ODR check on pointers");
  return false;
}

/***************************** RSExportVectorType *****************************/
llvm::StringRef
RSExportVectorType::GetTypeName(const clang::ExtVectorType *EVT) {
  const clang::Type *ElementType = GetExtVectorElementType(EVT);
  llvm::StringRef name;

  if ((ElementType->getTypeClass() != clang::Type::Builtin))
    return name;

  const clang::BuiltinType *BT =
          static_cast<const clang::BuiltinType*>(
              ElementType->getCanonicalTypeInternal().getTypePtr());

  if ((EVT->getNumElements() < 1) ||
      (EVT->getNumElements() > 4))
    return name;

  BuiltinInfo *info = FindBuiltinType(BT->getKind());
  if (info != nullptr) {
    int I = EVT->getNumElements() - 1;
    if (I < kMaxVectorSize) {
      name = info->cname[I];
    } else {
      slangAssert(false && "Max vector is 4");
    }
  }
  return name;
}

RSExportVectorType *RSExportVectorType::Create(RSContext *Context,
                                               const clang::ExtVectorType *EVT,
                                               const llvm::StringRef &TypeName,
                                               bool Normalized) {
  slangAssert(EVT != nullptr && EVT->getTypeClass() == clang::Type::ExtVector);

  const clang::Type *ElementType = GetExtVectorElementType(EVT);
  DataType DT = RSExportPrimitiveType::GetDataType(Context, ElementType);

  if (DT != DataTypeUnknown)
    return new RSExportVectorType(Context,
                                  TypeName,
                                  DT,
                                  Normalized,
                                  EVT->getNumElements());
  else
    return nullptr;
}

llvm::Type *RSExportVectorType::convertToLLVMType() const {
  llvm::Type *ElementType = RSExportPrimitiveType::convertToLLVMType();
  return llvm::VectorType::get(ElementType, getNumElement());
}

bool RSExportVectorType::matchODR(const RSExportType *E,
                                  bool /* LookInto*/) const {
  CHECK_PARENT_EQUALITY(RSExportPrimitiveType, E);
  return (static_cast<const RSExportVectorType*>(E)->getNumElement()
              == getNumElement());
}

/***************************** RSExportMatrixType *****************************/
RSExportMatrixType *RSExportMatrixType::Create(RSContext *Context,
                                               const clang::RecordType *RT,
                                               const llvm::StringRef &TypeName,
                                               unsigned Dim) {
  slangAssert((RT != nullptr) && (RT->getTypeClass() == clang::Type::Record));
  slangAssert((Dim > 1) && "Invalid dimension of matrix");

  // Check whether the struct rs_matrix is in our expected form (but assume it's
  // correct if we're not sure whether it's correct or not)
  const clang::RecordDecl* RD = RT->getDecl();
  RD = RD->getDefinition();
  if (RD != nullptr) {
    // Find definition, perform further examination
    if (RD->field_empty()) {
      Context->ReportError(
          RD->getLocation(),
          "invalid matrix struct: must have 1 field for saving values: '%0'")
          << RD->getName();
      return nullptr;
    }

    clang::RecordDecl::field_iterator FIT = RD->field_begin();
    const clang::FieldDecl *FD = *FIT;
    const clang::Type *FT = RSExportType::GetTypeOfDecl(FD);
    if ((FT == nullptr) || (FT->getTypeClass() != clang::Type::ConstantArray)) {
      Context->ReportError(RD->getLocation(),
                           "invalid matrix struct: first field should"
                           " be an array with constant size: '%0'")
          << RD->getName();
      return nullptr;
    }
    const clang::ConstantArrayType *CAT =
      static_cast<const clang::ConstantArrayType *>(FT);
    const clang::Type *ElementType = GetConstantArrayElementType(CAT);
    if ((ElementType == nullptr) ||
        (ElementType->getTypeClass() != clang::Type::Builtin) ||
        (static_cast<const clang::BuiltinType *>(ElementType)->getKind() !=
         clang::BuiltinType::Float)) {
      Context->ReportError(RD->getLocation(),
                           "invalid matrix struct: first field "
                           "should be a float array: '%0'")
          << RD->getName();
      return nullptr;
    }

    if (CAT->getSize() != Dim * Dim) {
      Context->ReportError(RD->getLocation(),
                           "invalid matrix struct: first field "
                           "should be an array with size %0: '%1'")
          << (Dim * Dim) << (RD->getName());
      return nullptr;
    }

    FIT++;
    if (FIT != RD->field_end()) {
      Context->ReportError(RD->getLocation(),
                           "invalid matrix struct: must have "
                           "exactly 1 field: '%0'")
          << RD->getName();
      return nullptr;
    }
  }

  return new RSExportMatrixType(Context, TypeName, Dim);
}

llvm::Type *RSExportMatrixType::convertToLLVMType() const {
  // Construct LLVM type:
  // struct {
  //  float X[mDim * mDim];
  // }

  llvm::LLVMContext &C = getRSContext()->getLLVMContext();
  llvm::ArrayType *X = llvm::ArrayType::get(llvm::Type::getFloatTy(C),
                                            mDim * mDim);
  return llvm::StructType::get(C, X, false);
}

bool RSExportMatrixType::matchODR(const RSExportType *E,
                                  bool /* LookInto */) const {
  CHECK_PARENT_EQUALITY(RSExportType, E);
  return (static_cast<const RSExportMatrixType*>(E)->getDim() == getDim());
}

/************************* RSExportConstantArrayType *************************/
RSExportConstantArrayType
*RSExportConstantArrayType::Create(RSContext *Context,
                                   const clang::ConstantArrayType *CAT) {
  slangAssert(CAT != nullptr && CAT->getTypeClass() == clang::Type::ConstantArray);

  slangAssert((CAT->getSize().getActiveBits() < 32) && "array too large");

  unsigned Size = static_cast<unsigned>(CAT->getSize().getZExtValue());
  slangAssert((Size > 0) && "Constant array should have size greater than 0");

  const clang::Type *ElementType = GetConstantArrayElementType(CAT);
  RSExportType *ElementET = RSExportType::Create(Context, ElementType,
                                                 NotLegacyKernelArgument);

  if (ElementET == nullptr) {
    return nullptr;
  }

  return new RSExportConstantArrayType(Context,
                                       ElementET,
                                       Size);
}

llvm::Type *RSExportConstantArrayType::convertToLLVMType() const {
  return llvm::ArrayType::get(mElementType->getLLVMType(), getNumElement());
}

bool RSExportConstantArrayType::keep() {
  if (!RSExportType::keep())
    return false;
  const_cast<RSExportType*>(mElementType)->keep();
  return true;
}

bool RSExportConstantArrayType::matchODR(const RSExportType *E,
                                         bool LookInto) const {
  CHECK_PARENT_EQUALITY(RSExportType, E);
  const RSExportConstantArrayType *RHS =
      static_cast<const RSExportConstantArrayType*>(E);
  return ((getNumElement() == RHS->getNumElement()) &&
          (getElementType()->matchODR(RHS->getElementType(), LookInto)));
}

/**************************** RSExportRecordType ****************************/
RSExportRecordType *RSExportRecordType::Create(RSContext *Context,
                                               const clang::RecordType *RT,
                                               const llvm::StringRef &TypeName,
                                               bool mIsArtificial) {
  slangAssert(RT != nullptr && RT->getTypeClass() == clang::Type::Record);

  const clang::RecordDecl *RD = RT->getDecl();
  slangAssert(RD->isStruct());

  RD = RD->getDefinition();
  if (RD == nullptr) {
    slangAssert(false && "struct is not defined in this module");
    return nullptr;
  }

  // Struct layout construct by clang. We rely on this for obtaining the
  // alloc size of a struct and offset of every field in that struct.
  const clang::ASTRecordLayout *RL =
      &Context->getASTContext().getASTRecordLayout(RD);
  slangAssert((RL != nullptr) &&
      "Failed to retrieve the struct layout from Clang.");

  RSExportRecordType *ERT =
      new RSExportRecordType(Context,
                             TypeName,
                             RD->getLocation(),
                             RD->hasAttr<clang::PackedAttr>(),
                             mIsArtificial,
                             RL->getDataSize().getQuantity(),
                             RL->getSize().getQuantity());
  unsigned int Index = 0;

  for (clang::RecordDecl::field_iterator FI = RD->field_begin(),
           FE = RD->field_end();
       FI != FE;
       FI++, Index++) {

    // FIXME: All fields should be primitive type
    slangAssert(FI->getKind() == clang::Decl::Field);
    clang::FieldDecl *FD = *FI;

    if (FD->isBitField()) {
      return nullptr;
    }

    if (FD->isImplicit() && (FD->getName() == RS_PADDING_FIELD_NAME))
      continue;

    // Type
    RSExportType *ET = RSExportElement::CreateFromDecl(Context, FD);

    if (ET != nullptr) {
      ERT->mFields.push_back(
          new Field(ET, FD->getName(), ERT,
                    static_cast<size_t>(RL->getFieldOffset(Index) >> 3)));
    } else {
      // clang static analysis complains about a potential memory leak
      // for the memory pointed by ERT at the end of this basic
      // block. This is a false warning because the compiler does not
      // see that the pointer to this memory is saved away in the
      // constructor for RSExportRecordType by calling
      // RSContext::newExportable(this). So, we disable this
      // particular instance of the warning.
      Context->ReportError(RD->getLocation(),
                           "field type cannot be exported: '%0.%1'")
          << RD->getName() << FD->getName(); // NOLINT
      return nullptr;
    }
  }

  return ERT;
}

llvm::Type *RSExportRecordType::convertToLLVMType() const {
  // Create an opaque type since struct may reference itself recursively.

  // TODO(sliao): LLVM took out the OpaqueType. Any other to migrate to?
  std::vector<llvm::Type*> FieldTypes;

  for (const_field_iterator FI = fields_begin(), FE = fields_end();
       FI != FE;
       FI++) {
    const Field *F = *FI;
    const RSExportType *FET = F->getType();

    FieldTypes.push_back(FET->getLLVMType());
  }

  llvm::StructType *ST = llvm::StructType::get(getRSContext()->getLLVMContext(),
                                               FieldTypes,
                                               mIsPacked);
  if (ST != nullptr) {
    return ST;
  } else {
    return nullptr;
  }
}

bool RSExportRecordType::keep() {
  if (!RSExportType::keep())
    return false;
  for (std::list<const Field*>::iterator I = mFields.begin(),
          E = mFields.end();
       I != E;
       I++) {
    const_cast<RSExportType*>((*I)->getType())->keep();
  }
  return true;
}

bool RSExportRecordType::matchODR(const RSExportType *E, bool LookInto) const {
  CHECK_PARENT_EQUALITY(RSExportType, E);
  // Enforce ODR checking - the type E represents must hold
  // *exactly* the same "definition" as the one defined previously. We
  // say two record types A and B have the same definition iff:
  //
  //  struct A {              struct B {
  //    Type(a1) a1,            Type(b1) b1,
  //    Type(a2) a2,            Type(b1) b2,
  //    ...                     ...
  //    Type(aN) aN             Type(bM) bM,
  //  };                      }
  //  Cond. #0. A = B;
  //  Cond. #1. They have same number of fields, i.e., N = M;
  //  Cond. #2. for (i := 1 to N)
  //              Type(ai).matchODR(Type(bi)) must hold;
  //  Cond. #3. for (i := 1 to N)
  //              Name(ai) = Name(bi) must hold;
  //
  // where,
  //  Type(F) = the type of field F and
  //  Name(F) = the field name.


  const RSExportRecordType *ERT = static_cast<const RSExportRecordType*>(E);
  // Cond. #0.
  if (getName() != ERT->getName())
    return false;

  // Examine fields - types and names
  if (LookInto) {
    // Cond. #1
    if (ERT->getFields().size() != getFields().size())
      return false;

    for (RSExportRecordType::const_field_iterator AI = fields_begin(),
         BI = ERT->fields_begin(), AE = fields_end(); AI != AE; ++AI, ++BI) {
      const RSExportType *AITy = (*AI)->getType();
      const RSExportType *BITy = (*BI)->getType();
      // Cond. #3; field names must agree
      if ((*AI)->getName() != (*BI)->getName())
        return false;

      // Cond. #2; field types must agree recursively until we see another
      // next level of RSExportRecordType - such field types will be
      // examined and reported later when checkODR() encounters them.
      if (!AITy->matchODR(BITy, false))
        return false;
    }
  }
  return true;
}

void RSExportType::convertToRTD(RSReflectionTypeData *rtd) const {
    memset(rtd, 0, sizeof(*rtd));
    rtd->vecSize = 1;

    switch(getClass()) {
    case RSExportType::ExportClassPrimitive: {
            const RSExportPrimitiveType *EPT = static_cast<const RSExportPrimitiveType*>(this);
            rtd->type = RSExportPrimitiveType::getRSReflectionType(EPT);
            return;
        }
    case RSExportType::ExportClassPointer: {
            const RSExportPointerType *EPT = static_cast<const RSExportPointerType*>(this);
            const RSExportType *PointeeType = EPT->getPointeeType();
            PointeeType->convertToRTD(rtd);
            rtd->isPointer = true;
            return;
        }
    case RSExportType::ExportClassVector: {
            const RSExportVectorType *EVT = static_cast<const RSExportVectorType*>(this);
            rtd->type = EVT->getRSReflectionType(EVT);
            rtd->vecSize = EVT->getNumElement();
            return;
        }
    case RSExportType::ExportClassMatrix: {
            const RSExportMatrixType *EMT = static_cast<const RSExportMatrixType*>(this);
            unsigned Dim = EMT->getDim();
            slangAssert((Dim >= 2) && (Dim <= 4));
            rtd->type = &gReflectionTypes[15 + Dim-2];
            return;
        }
    case RSExportType::ExportClassConstantArray: {
            const RSExportConstantArrayType* CAT =
              static_cast<const RSExportConstantArrayType*>(this);
            CAT->getElementType()->convertToRTD(rtd);
            rtd->arraySize = CAT->getNumElement();
            return;
        }
    case RSExportType::ExportClassRecord: {
            slangAssert(!"RSExportType::ExportClassRecord not implemented");
            return;// RS_TYPE_CLASS_NAME_PREFIX + ET->getName() + ".Item";
        }
    default: {
            slangAssert(false && "Unknown class of type");
        }
    }
}


}  // namespace slang
