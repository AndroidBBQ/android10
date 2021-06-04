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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_ELEMENT_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_ELEMENT_H_

#include <string>

#include "clang/Lex/Token.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

#include "slang_rs_export_type.h"

namespace clang {
  class Type;
  class DeclaratorDecl;
}   // namespace clang

namespace slang {

  class RSContext;
  class RSExportType;

class RSExportElement {
  // This is a utility class for handling the RS_ELEMENT_ADD* marker
  RSExportElement() { }

  typedef struct {
    DataType type;
    bool normalized;
    unsigned vsize;
  } ElementInfo;

  typedef llvm::StringMap<const ElementInfo*> ElementInfoMapTy;

 private:
  // Macro name <-> ElementInfo
  static ElementInfoMapTy ElementInfoMap;

  static bool Initialized;

  static RSExportType *Create(RSContext *Context,
                              const clang::Type *T,
                              const ElementInfo *EI);

  static const ElementInfo *GetElementInfo(const llvm::StringRef &Name);

 public:
  static void Init();

  static RSExportType *CreateFromDecl(RSContext *Context,
                                      const clang::DeclaratorDecl *DD);
};

}   // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_ELEMENT_H_  NOLINT
