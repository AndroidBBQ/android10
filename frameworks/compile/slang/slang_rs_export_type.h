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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_TYPE_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_TYPE_H_

#include <list>
#include <set>
#include <string>
#include <sstream>

#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/Support/ManagedStatic.h"

#include "slang_rs_exportable.h"

#define RS_PADDING_FIELD_NAME ".rs.padding"

inline const clang::Type* GetCanonicalType(const clang::Type* T) {
  if (T == nullptr) {
    return  nullptr;
  }
  return T->getCanonicalTypeInternal().getTypePtr();
}

inline const clang::Type* GetCanonicalType(clang::QualType QT) {
  return GetCanonicalType(QT.getTypePtr());
}

inline const clang::Type* GetExtVectorElementType(const clang::ExtVectorType *T) {
  if (T == nullptr) {
    return nullptr;
  }
  return GetCanonicalType(T->getElementType());
}

inline const clang::Type* GetPointeeType(const clang::PointerType *T) {
  if (T == nullptr) {
    return nullptr;
  }
  return GetCanonicalType(T->getPointeeType());
}

inline const clang::Type* GetConstantArrayElementType(const clang::ConstantArrayType *T) {
  if (T == nullptr) {
    return nullptr;
  }
  return GetCanonicalType(T->getElementType());
}


namespace llvm {
  class Type;
}   // namespace llvm

namespace slang {

class RSContext;

// Broad grouping of the data types
enum DataTypeCategory {
    PrimitiveDataType,
    MatrixDataType,
    ObjectDataType
};

// Denote whether a particular export is intended for a legacy kernel argument.
// NotLegacyKernelArgument - not a legacy kernel argument (might not even be a
//                           kernel argument).
// LegacyKernelArgument    - legacy pass-by-reference kernel argument using
//                           pointers and no kernel attribute.
enum ExportKind {
   NotLegacyKernelArgument,
   LegacyKernelArgument
 };


// From graphics/java/android/renderscript/Element.java: Element.DataType
/* NOTE: The values of the enums are found compiled in the bit code (i.e. as
 * values, not symbolic.  When adding new types, you must add them to the end.
 * If removing types, you can't re-use the integer value.
 *
 * TODO: but if you do this, you won't be able to keep using First* & Last*
 * for validation.
 *
 * IMPORTANT: This enum should correspond one-for-one to the entries found in the
 * gReflectionsTypes table (except for the two negative numbers).  Don't edit one without
 * the other.
 */
enum DataType {
    DataTypeIsStruct = -2,
    DataTypeUnknown = -1,

    DataTypeFloat16 = 0,
    DataTypeFloat32 = 1,
    DataTypeFloat64 = 2,
    DataTypeSigned8 = 3,
    DataTypeSigned16 = 4,
    DataTypeSigned32 = 5,
    DataTypeSigned64 = 6,
    DataTypeUnsigned8 = 7,
    DataTypeUnsigned16 = 8,
    DataTypeUnsigned32 = 9,
    DataTypeUnsigned64 = 10,
    DataTypeBoolean = 11,
    DataTypeUnsigned565 = 12,
    DataTypeUnsigned5551 = 13,
    DataTypeUnsigned4444 = 14,

    DataTypeRSMatrix2x2 = 15,
    DataTypeRSMatrix3x3 = 16,
    DataTypeRSMatrix4x4 = 17,

    DataTypeRSElement = 18,
    DataTypeRSType = 19,
    DataTypeRSAllocation = 20,
    DataTypeRSSampler = 21,
    DataTypeRSScript = 22,
    DataTypeRSMesh = 23,
    DataTypeRSPath = 24,
    DataTypeRSProgramFragment = 25,
    DataTypeRSProgramVertex = 26,
    DataTypeRSProgramRaster = 27,
    DataTypeRSProgramStore = 28,
    DataTypeRSFont = 29,

    // This should always be last and correspond to the size of the gReflectionTypes table.
    DataTypeMax
};

typedef struct {
    // The data type category
    DataTypeCategory category;
    // "Common name" in script (C99)
    const char * s_name;
    // The element name in RenderScript
    const char * rs_type;
    // The short element name in RenderScript
    const char * rs_short_type;
    // The size of the type in bits
    uint32_t size_in_bits;
    // The reflected name in C code
    const char * c_name;
    // The reflected name in Java code
    const char * java_name;
    // The array type that is compatible with Allocations of our type,
    // for use with copyTo(), copyFrom()
    const char * java_array_element_name;
    // The prefix for C vector types
    const char * rs_c_vector_prefix;
    // The prefix for Java vector types
    const char * rs_java_vector_prefix;
    // Indicates an unsigned type undergoing Java promotion
    bool java_promotion;
} RSReflectionType;


typedef struct RSReflectionTypeData_rec {
    const RSReflectionType *type;
    uint32_t vecSize;   // number of elements; one if not a vector
    bool isPointer;
    uint32_t arraySize; // number of elements; zero if not an array

    // Subelements
    //std::vector<const struct RSReflectionTypeData_rec *> fields;
    //std::vector< std::string > fieldNames;
    //std::vector< uint32_t> fieldOffsetBytes;
} RSReflectionTypeData;

// Make a name for types that are too complicated to create the real names.
std::string CreateDummyName(const char *type, const std::string &name);

inline bool IsDummyName(const llvm::StringRef &Name) {
  return Name.startswith("<");
}

class RSExportType : public RSExportable {
  friend class RSExportElement;
 public:
  typedef enum {
    ExportClassPrimitive,
    ExportClassPointer,
    ExportClassVector,
    ExportClassMatrix,
    ExportClassConstantArray,
    ExportClassRecord
  } ExportClass;

  void convertToRTD(RSReflectionTypeData *rtd) const;

 private:
  ExportClass mClass;
  std::string mName;

  // Cache the result after calling convertToLLVMType() at the first time
  mutable llvm::Type *mLLVMType;

 protected:
  RSExportType(RSContext *Context,
               ExportClass Class,
               const llvm::StringRef &Name,
               clang::SourceLocation Loc = clang::SourceLocation());

  // Let's make it private since there're some prerequisites to call this
  // function.
  //
  // @T was normalized by calling RSExportType::NormalizeType().
  // @TypeName was retrieved from RSExportType::GetTypeName() before calling
  //           this.
  // @EK denotes whether this @T is being used for a legacy kernel argument or
  //     something else.
  //
  static RSExportType *Create(RSContext *Context,
                              const clang::Type *T,
                              const llvm::StringRef &TypeName,
                              ExportKind EK);

  static llvm::StringRef GetTypeName(const clang::Type *T);

  // This function convert the RSExportType to LLVM type. Actually, it should be
  // "convert Clang type to LLVM type." However, clang doesn't make this API
  // (lib/CodeGen/CodeGenTypes.h) public, we need to do by ourselves.
  //
  // Once we can get LLVM type, we can use LLVM to get alignment information,
  // allocation size of a given type and structure layout that LLVM used
  // (all of these information are target dependent) without dealing with these
  // by ourselves.
  virtual llvm::Type *convertToLLVMType() const = 0;
  // Record type may recursively reference its type definition. We need a
  // temporary type setup before the type construction gets done.
  inline void setAbstractLLVMType(llvm::Type *LLVMType) const {
    mLLVMType = LLVMType;
  }

  virtual ~RSExportType();

 public:
  // This function additionally verifies that the Type T is exportable.
  // If it is not, this function returns false. Otherwise it returns true.
  static bool NormalizeType(const clang::Type *&T,
                            llvm::StringRef &TypeName,
                            RSContext *Context,
                            const clang::VarDecl *VD,
                            ExportKind EK);

  // This function checks whether the specified type can be handled by RS/FS.
  // If it cannot, this function returns false. Otherwise it returns true.
  // Filterscript has additional restrictions on supported types.
  static bool ValidateType(slang::RSContext *Context, clang::ASTContext &C,
                           clang::QualType QT, const clang::NamedDecl *ND,
                           clang::SourceLocation Loc, unsigned int TargetAPI,
                           bool IsFilterscript, bool IsExtern);

  // This function ensures that the VarDecl can be properly handled by RS.
  // If it cannot, this function returns false. Otherwise it returns true.
  // Filterscript has additional restrictions on supported types.
  static bool ValidateVarDecl(slang::RSContext *Context, clang::VarDecl *VD,
                              unsigned int TargetAPI, bool IsFilterscript);

  // @T may not be normalized
  static RSExportType *Create(RSContext *Context, const clang::Type *T,
                              ExportKind EK,
                              // T is type of VD or of subobject within VD
                              const clang::VarDecl *VD = nullptr);
  static RSExportType *CreateFromDecl(RSContext *Context,
                                      const clang::VarDecl *VD);

  static const clang::Type *GetTypeOfDecl(const clang::DeclaratorDecl *DD);

  inline ExportClass getClass() const { return mClass; }

  inline llvm::Type *getLLVMType() const {
    if (mLLVMType == nullptr)
      mLLVMType = convertToLLVMType();
    return mLLVMType;
  }

  // Return the maximum number of bytes that may be written when this type is stored.
  virtual size_t getStoreSize() const;

  // Return the distance in bytes between successive elements of this type; it includes padding.
  virtual size_t getAllocSize() const;

  inline const std::string &getName() const { return mName; }

  virtual std::string getElementName() const {
    // Base case is actually an invalid C/Java identifier.
    return "@@INVALID@@";
  }

  virtual bool keep();
  // matchODR(): a helper function for Slang::checkODR() on ODR validation
  //
  // The LookInto parameter dictates whether to recursively validate member
  // types of given compound types. This currently only affects struct
  // (RSExportRecordType); it has no effect on primitive types, vector types,
  // or matrix types.
  //
  // Consider the following pseudo code of nested struct types:
  //
  // Translation unit #1:     Translation unit #2:
  //
  // struct Name(AA) {        struct Name(BB) {
  //   Type(aa) aa;             Type(bb) bb;
  // };                       };
  //
  // struct Name(A)  {        struct Name(B) {
  //   struct Name(AA) a;       struct Name(BB) b;
  // };                       };
  //
  // Case 1:
  // Assuming aa and bb do not match (say mismatching just in field name), but
  // the rest does, then the desirable behavior is to report an ODR violation
  // on struct BB (vs. struct AA) but not on struct B (vs. struct A), because
  // BB is the tightest enclosing declaration of the violation, not B.
  //
  // For this case, RSExportRecordType::matchODR() has the following behavior:
  //
  // A.matchODR(B, true) should NOT report an ODR violation;
  // AA.matchODR(BB, true) should report an ODR violation w.r.t. struct BB;
  // A.matchODR(B, false) should not report an error;
  // AA.matchODR(BB, false) should not report an error.
  //
  // Slang::checkODR() acts as a driver for this validation case. It calls
  // A.matchODR() and AA.matchODR() comparing against B and BB respectively.
  //
  // By setting LookInto true when Slang::checkODR() calls matchODR() with
  // the outermost compound type, and false with any recursively discovered
  // types, we can ensure the desirable ODR violation reporting behavior.
  //
  // Case 2:
  // Assuming Name(AA) != Name(BB), but the rest of the declarations match,
  // then the desirable behavior is to report an ODR violation on struct B
  // (vs. struct A).
  //
  // In this case, RSExportRecordType::matchODR() has the following behavior:
  //
  // A.matchODR(B, true) should report an ODR violation w.r.t. struct B;
  // because AA and BB are two different types, AA.matchODR(BB) won't be
  // called.
  //
  // A.matchODR(B, false) should not report an error; this happens, should
  // there be any more additional enclosing types for A subject to ODR check.
  virtual bool matchODR(const RSExportType *E, bool LookInto) const;
};  // RSExportType

// Primitive types
class RSExportPrimitiveType : public RSExportType {
  friend class RSExportType;
  friend class RSExportElement;
 private:
  DataType mType;
  bool mNormalized;

  typedef llvm::StringMap<DataType> RSSpecificTypeMapTy;
  static llvm::ManagedStatic<RSSpecificTypeMapTy> RSSpecificTypeMap;

  static const size_t SizeOfDataTypeInBits[];
  // @T was normalized by calling RSExportType::NormalizeType() before calling
  // this.
  // @TypeName was retrieved from RSExportType::GetTypeName() before calling
  // this
  static RSExportPrimitiveType *Create(RSContext *Context,
                                       const clang::Type *T,
                                       const llvm::StringRef &TypeName,
                                       bool Normalized = false);

 protected:
  RSExportPrimitiveType(RSContext *Context,
                        // for derived class to set their type class
                        ExportClass Class,
                        const llvm::StringRef &Name,
                        DataType DT,
                        bool Normalized)
      : RSExportType(Context, Class, Name),
        mType(DT),
        mNormalized(Normalized) {
  }

  virtual llvm::Type *convertToLLVMType() const;

  static DataType GetDataType(RSContext *Context, const clang::Type *T);

 public:
  // T is normalized by calling RSExportType::NormalizeType() before
  // calling this
  static bool IsPrimitiveType(const clang::Type *T);

  // @T may not be normalized
  static RSExportPrimitiveType *Create(RSContext *Context,
                                       const clang::Type *T);

  static DataType GetRSSpecificType(const llvm::StringRef &TypeName);
  static DataType GetRSSpecificType(const clang::Type *T);

  static bool IsRSMatrixType(DataType DT);
  static bool IsRSObjectType(DataType DT);
  static bool IsRSObjectType(const clang::Type *T) {
    return IsRSObjectType(GetRSSpecificType(T));
  }

  // Determines whether T is [an array of] struct that contains at least one
  // RS object type within it.
  static bool IsStructureTypeWithRSObject(const clang::Type *T);

  // For a primitive type, this is the size of the type.
  // For a vector type (RSExportVectorType is derived from RSExportPrimitiveType),
  // this is the size of a single vector element (component).
  static size_t GetElementSizeInBits(const RSExportPrimitiveType *EPT);

  inline DataType getType() const { return mType; }
  inline bool isRSObjectType() const {
      return IsRSObjectType(mType);
  }

  bool matchODR(const RSExportType *E, bool LookInto) const override;

  static RSReflectionType *getRSReflectionType(DataType DT);
  static RSReflectionType *getRSReflectionType(
      const RSExportPrimitiveType *EPT) {
    return getRSReflectionType(EPT->getType());
  }

  // For a vector type, this is the size of a single element.
  unsigned getElementSizeInBytes() const { return (GetElementSizeInBits(this) >> 3); }

  std::string getElementName() const {
    return getRSReflectionType(this)->rs_short_type;
  }
};  // RSExportPrimitiveType


class RSExportPointerType : public RSExportType {
  friend class RSExportType;
  friend class RSExportFunc;
 private:
  const RSExportType *mPointeeType;

  RSExportPointerType(RSContext *Context,
                      const llvm::StringRef &Name,
                      const RSExportType *PointeeType)
      : RSExportType(Context, ExportClassPointer, Name),
        mPointeeType(PointeeType) {
  }

  // @PT was normalized by calling RSExportType::NormalizeType() before calling
  // this.
  static RSExportPointerType *Create(RSContext *Context,
                                     const clang::PointerType *PT,
                                     const llvm::StringRef &TypeName);

  virtual llvm::Type *convertToLLVMType() const;

 public:
  virtual bool keep();

  inline const RSExportType *getPointeeType() const { return mPointeeType; }

  bool matchODR(const RSExportType *E, bool LookInto) const override;
};  // RSExportPointerType


class RSExportVectorType : public RSExportPrimitiveType {
  friend class RSExportType;
  friend class RSExportElement;
 private:
  unsigned mNumElement;   // number of elements (components)

  RSExportVectorType(RSContext *Context,
                     const llvm::StringRef &Name,
                     DataType DT,
                     bool Normalized,
                     unsigned NumElement)
      : RSExportPrimitiveType(Context, ExportClassVector, Name,
                              DT, Normalized),
        mNumElement(NumElement) {
  }

  // @EVT was normalized by calling RSExportType::NormalizeType() before
  // calling this.
  static RSExportVectorType *Create(RSContext *Context,
                                    const clang::ExtVectorType *EVT,
                                    const llvm::StringRef &TypeName,
                                    bool Normalized = false);

  virtual llvm::Type *convertToLLVMType() const;

 public:
  static llvm::StringRef GetTypeName(const clang::ExtVectorType *EVT);

  inline unsigned getNumElement() const { return mNumElement; }

  std::string getElementName() const {
    std::stringstream Name;
    Name << RSExportPrimitiveType::getRSReflectionType(this)->rs_short_type
         << "_" << getNumElement();
    return Name.str();
  }

  bool matchODR(const RSExportType *E, bool LookInto) const override;
};

// Only *square* *float* matrix is supported by now.
//
// struct rs_matrix{2x2,3x3,4x4, ..., NxN} should be defined as the following
// form *exactly*:
//  typedef struct {
//    float m[{NxN}];
//  } rs_matrixNxN;
//
//  where mDim will be N.
class RSExportMatrixType : public RSExportType {
  friend class RSExportType;
 private:
  unsigned mDim;  // dimension

  RSExportMatrixType(RSContext *Context,
                     const llvm::StringRef &Name,
                     unsigned Dim)
    : RSExportType(Context, ExportClassMatrix, Name),
      mDim(Dim) {
  }

  virtual llvm::Type *convertToLLVMType() const;

 public:
  // @RT was normalized by calling RSExportType::NormalizeType() before
  // calling this.
  static RSExportMatrixType *Create(RSContext *Context,
                                    const clang::RecordType *RT,
                                    const llvm::StringRef &TypeName,
                                    unsigned Dim);

  inline unsigned getDim() const { return mDim; }

  bool matchODR(const RSExportType *E, bool LookInto) const override;

};

class RSExportConstantArrayType : public RSExportType {
  friend class RSExportType;
 private:
  const RSExportType *mElementType;  // Array element type
  unsigned mNumElement;              // Array element count

  RSExportConstantArrayType(RSContext *Context,
                            const RSExportType *ElementType,
                            unsigned NumElement)
    : RSExportType(Context, ExportClassConstantArray, "<ConstantArray>"),
      mElementType(ElementType),
      mNumElement(NumElement) {
  }

  // @CAT was normalized by calling RSExportType::NormalizeType() before
  // calling this.
  static RSExportConstantArrayType *Create(RSContext *Context,
                                           const clang::ConstantArrayType *CAT);

  virtual llvm::Type *convertToLLVMType() const;

 public:
  unsigned getNumElement() const { return mNumElement; }
  const RSExportType *getElementType() const { return mElementType; }

  std::string getElementName() const {
    return mElementType->getElementName();
  }

  virtual bool keep();
  bool matchODR(const RSExportType *E, bool LookInto) const override;
};

class RSExportRecordType : public RSExportType {
  friend class RSExportType;
 public:
  class Field {
   private:
    const RSExportType *mType;
    // Field name
    std::string mName;
    // Link to the struct that contain this field
    const RSExportRecordType *mParent;
    // Offset in the container
    size_t mOffset;

   public:
    Field(const RSExportType *T,
          const llvm::StringRef &Name,
          const RSExportRecordType *Parent,
          size_t Offset)
        : mType(T),
          mName(Name.data(), Name.size()),
          mParent(Parent),
          mOffset(Offset) {
    }

    inline const RSExportRecordType *getParent() const { return mParent; }
    inline const RSExportType *getType() const { return mType; }
    inline const std::string &getName() const { return mName; }
    inline size_t getOffsetInParent() const { return mOffset; }
  };

  typedef std::list<const Field*>::const_iterator const_field_iterator;

  inline const_field_iterator fields_begin() const {
    return this->mFields.begin();
  }
  inline const_field_iterator fields_end() const {
    return this->mFields.end();
  }
  inline size_t fields_size() const {
    return this->mFields.size();
  }

 private:
  std::list<const Field*> mFields;
  bool mIsPacked;
  // Artificial export struct type is not exported by user (and thus it won't
  // get reflected)
  bool mIsArtificial;
  size_t mStoreSize;
  size_t mAllocSize;

  RSExportRecordType(RSContext *Context,
                     const llvm::StringRef &Name,
                     clang::SourceLocation Loc,
                     bool IsPacked,
                     bool IsArtificial,
                     size_t StoreSize,
                     size_t AllocSize)
      : RSExportType(Context, ExportClassRecord, Name, Loc),
        mIsPacked(IsPacked),
        mIsArtificial(IsArtificial),
        mStoreSize(StoreSize),
        mAllocSize(AllocSize) {
  }

  // @RT was normalized by calling RSExportType::NormalizeType() before calling
  // this.
  // @TypeName was retrieved from RSExportType::GetTypeName() before calling
  // this.
  static RSExportRecordType *Create(RSContext *Context,
                                    const clang::RecordType *RT,
                                    const llvm::StringRef &TypeName,
                                    bool mIsArtificial = false);

  virtual llvm::Type *convertToLLVMType() const;

 public:
  inline const std::list<const Field*>& getFields() const { return mFields; }
  inline bool isPacked() const { return mIsPacked; }
  inline bool isArtificial() const { return mIsArtificial; }
  virtual size_t getStoreSize() const { return mStoreSize; }
  virtual size_t getAllocSize() const { return mAllocSize; }

  virtual std::string getElementName() const {
    return "ScriptField_" + getName();
  }

  virtual bool keep();
  bool matchODR(const RSExportType *E, bool LookInto) const override;

  ~RSExportRecordType() {
    for (std::list<const Field*>::iterator I = mFields.begin(),
             E = mFields.end();
         I != E;
         I++)
      if (*I != nullptr)
        delete *I;
  }
};  // RSExportRecordType

}   // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_TYPE_H_  NOLINT
