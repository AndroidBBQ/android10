/*
 * Copyright 2010, The Android Open Source Project
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

#include "slang_rs_export_element.h"

#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/IdentifierTable.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_export_type.h"

namespace slang {

bool RSExportElement::Initialized = false;
RSExportElement::ElementInfoMapTy RSExportElement::ElementInfoMap;

struct DataElementInfo {
  const char *name;
  DataType dataType;
  bool normalized;
  int vsize;
};

static DataElementInfo DataElementInfoTable[] = {
    {"rs_pixel_l", DataTypeUnsigned8, true, 1},
    {"rs_pixel_a", DataTypeUnsigned8, true, 1},
    {"rs_pixel_la", DataTypeUnsigned8, true, 2},
    {"rs_pixel_rgb", DataTypeUnsigned8, true, 3},
    {"rs_pixel_rgba", DataTypeUnsigned8, true, 4},
    {"rs_pixel_rgb565", DataTypeUnsigned8, true, 3},
    {"rs_pixel_rgb5551", DataTypeUnsigned8, true, 4},
    {"rs_pixel_rgb4444", DataTypeUnsigned8, true, 4},
};

const int DataElementInfoTableCount = sizeof(DataElementInfoTable) / sizeof(DataElementInfoTable[0]);

// TODO Rename RSExportElement to RSExportDataElement
void RSExportElement::Init() {
  if (!Initialized) {
    // Initialize ElementInfoMap
    for (int i = 0; i < DataElementInfoTableCount; i++) {
      ElementInfo *EI = new ElementInfo;
      EI->type = DataElementInfoTable[i].dataType;
      EI->normalized = DataElementInfoTable[i].normalized;
      EI->vsize = DataElementInfoTable[i].vsize;
      llvm::StringRef Name(DataElementInfoTable[i].name);
      ElementInfoMap.insert(ElementInfoMapTy::value_type::Create(
          Name, ElementInfoMap.getAllocator(), EI));
    }
    Initialized = true;
  }
}

RSExportType *RSExportElement::Create(RSContext *Context,
                                      const clang::Type *T,
                                      const ElementInfo *EI) {
  // Create RSExportType corresponded to the @T first and then verify

  llvm::StringRef TypeName;
  RSExportType *ET = nullptr;

  if (!Initialized)
    Init();

  slangAssert(EI != nullptr && "Element info not found");

  if (!RSExportType::NormalizeType(T, TypeName, Context, nullptr,
                                   NotLegacyKernelArgument))
    return nullptr;

  switch (T->getTypeClass()) {
    case clang::Type::Builtin:
    case clang::Type::Pointer: {
      slangAssert(EI->vsize == 1 && "Element not a primitive class (please "
                                    "check your macro)");
      RSExportPrimitiveType *EPT =
          RSExportPrimitiveType::Create(Context,
                                        T,
                                        TypeName,
                                        EI->normalized);
      // Verify
      slangAssert(EI->type == EPT->getType() && "Element has unexpected type");
      ET = EPT;
      break;
    }
    case clang::Type::ExtVector: {
      slangAssert(EI->vsize > 1 && "Element not a vector class (please check "
                                   "your macro)");
      RSExportVectorType *EVT =
          RSExportVectorType::Create(Context,
                                     static_cast<const clang::ExtVectorType*>(
                                         T->getCanonicalTypeInternal()
                                             .getTypePtr()),
                                     TypeName,
                                     EI->normalized);
      // Verify
      slangAssert(EI->type == EVT->getType() && "Element has unexpected type");
      slangAssert(EI->vsize == EVT->getNumElement() && "Element has unexpected "
                                                       "size of vector");
      ET = EVT;
      break;
    }
    default: {
      // TODO(zonr): warn that type is not exportable
      fprintf(stderr, "RSExportElement::Create : type '%s' is not exportable\n",
              T->getTypeClassName());
      break;
    }
  }

  return ET;
}

RSExportType *RSExportElement::CreateFromDecl(RSContext *Context,
                                              const clang::DeclaratorDecl *DD) {
  const clang::Type* T = RSExportType::GetTypeOfDecl(DD);
  const clang::Type* CT = GetCanonicalType(T);
  const ElementInfo* EI = nullptr;

  // Note: RS element like rs_pixel_rgb elements are either in the type of
  // primitive or vector.
  if ((CT->getTypeClass() != clang::Type::Builtin) &&
      (CT->getTypeClass() != clang::Type::ExtVector)) {
    return RSExportType::Create(Context, T, NotLegacyKernelArgument);
  }

  // Following the typedef chain to see whether it's an element name like
  // rs_pixel_rgb or its alias (via typedef).
  while (T != CT) {
    if (T->getTypeClass() != clang::Type::Typedef) {
      break;
    } else {
      const clang::TypedefType *TT = static_cast<const clang::TypedefType*>(T);
      const clang::TypedefNameDecl *TD = TT->getDecl();
      EI = GetElementInfo(TD->getName());
      if (EI != nullptr)
        break;

      T = TD->getUnderlyingType().getTypePtr();
    }
  }

  if (EI == nullptr) {
    return RSExportType::Create(Context, T, NotLegacyKernelArgument);
  } else {
    return RSExportElement::Create(Context, T, EI);
  }
}

const RSExportElement::ElementInfo *
RSExportElement::GetElementInfo(const llvm::StringRef &Name) {
  if (!Initialized)
    Init();

  ElementInfoMapTy::const_iterator I = ElementInfoMap.find(Name);
  if (I == ElementInfoMap.end())
    return nullptr;
  else
    return I->getValue();
}

}  // namespace slang
