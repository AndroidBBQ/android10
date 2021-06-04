/*
 * Copyright 2015, The Android Open Source Project
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

#ifndef BCC_RS_UTILS_H
#define BCC_RS_UTILS_H

#include "rsDefines.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ADT/StringRef.h>

#include <string>

namespace {

static inline llvm::StringRef getUnsuffixedStructName(const llvm::StructType *T) {
#ifdef _DEBUG
  // Bug: 22926131
  // When building with assertions enabled, LLVM cannot read the name of a
  // literal (anonymous) structure, because they shouldn't actually ever have
  // a name. Unfortunately, due to past definitions of RenderScript object
  // types as anonymous structures typedef-ed to their proper typename,
  // we had been relying on accessing this information. LLVM bitcode retains
  // the typedef-ed name for such anonymous structures. There is a
  // corresponding (safe) fix to the RenderScript headers to actually name
  // these types the same as their typedef name to simplify things. That
  // fixes this issue going forward, but it won't allow us to compile legacy
  // code properly. In that case, we just have non-assert builds ignore the
  // fact that anonymous structures shouldn't have their name read, and do it
  // anyway. Note that RSCompilerDriver.cpp checks the compiler version
  // number (from llvm-rs-cc) to ensure that we are only ever building modern
  // code when we have assertions enabled. Legacy code can only be compiled
  // correctly with a non-asserting compiler.
  //
  // Note that the whole reason for looking at the "unsuffixed" name of the
  // type is because LLVM suffixes duplicate typedefs of the same anonymous
  // structure. In the new case, where all of the RS object types have a
  // proper name, they won't have a dotted suffix at all. We still need
  // to look at the old unsuffixed version to handle legacy code properly.
  if (T->isLiteral()) {
    return "";
  }
#endif

  // Get just the object type name with no suffix.
  size_t LastDot = T->getName().rfind('.');
  if (LastDot == strlen("struct")) {
    // If we get back to just the "struct" part, we know that we had a
    // raw typename (i.e. struct.rs_element with no ".[0-9]+" suffix on it.
    // In that case, we will want to create our slice such that it contains
    // the entire name.
    LastDot = T->getName().size();
  }
  return T->getStructName().slice(0, LastDot);
}

const char kAllocationTypeName[] = "struct.rs_allocation";
const char kElementTypeName[]    = "struct.rs_element";
const char kSamplerTypeName[]    = "struct.rs_sampler";
const char kScriptTypeName[]     = "struct.rs_script";
const char kTypeTypeName[]       = "struct.rs_type";

// Returns the RsDataType for a given input LLVM type.
// This is only used to distinguish the associated RS object types (i.e.
// rs_allocation, rs_element, rs_sampler, rs_script, and rs_type).
// All other types are reported back as RS_TYPE_NONE, since no special
// handling would be necessary.
static inline enum RsDataType getRsDataTypeForType(const llvm::Type *T) {
  if (T->isStructTy()) {
    const llvm::StringRef StructName = getUnsuffixedStructName(llvm::dyn_cast<const llvm::StructType>(T));
    if (StructName.equals(kAllocationTypeName)) {
      return RS_TYPE_ALLOCATION;
    } else if (StructName.equals(kElementTypeName)) {
      return RS_TYPE_ELEMENT;
    } else if (StructName.equals(kSamplerTypeName)) {
      return RS_TYPE_SAMPLER;
    } else if (StructName.equals(kScriptTypeName)) {
      return RS_TYPE_SCRIPT;
    } else if (StructName.equals(kTypeTypeName)) {
      return RS_TYPE_TYPE;
    }
  }
  return RS_TYPE_NONE;
}

// Returns true if the input type is one of our RenderScript object types
// (allocation, element, sampler, script, type) and false if it is not.
static inline bool isRsObjectType(const llvm::Type *T) {
  return getRsDataTypeForType(T) != RS_TYPE_NONE;
}

}  // end namespace

// When we have a general reduction kernel with no combiner function,
// we will synthesize a combiner function from the accumulator
// function.  Given the accumulator function name, what should be the
// name of the combiner function?
static inline std::string nameReduceCombinerFromAccumulator(llvm::StringRef accumName) {
  return std::string(accumName) + ".combiner";
}

#endif // BCC_RS_UTILS_H
