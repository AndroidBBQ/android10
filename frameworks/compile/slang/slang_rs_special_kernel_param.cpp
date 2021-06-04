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

#include "slang_rs_special_kernel_param.h"

#include "clang/AST/ASTContext.h"

#include "bcinfo/MetadataExtractor.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_version.h"

namespace {

enum SpecialParameterKind {
  SPK_LOCATION, // 'int' or 'unsigned int'
  SPK_CONTEXT,  // rs_kernel_context
};

struct SpecialParameter {
  const char *name;
  bcinfo::MetadataSignatureBitval bitval;
  SpecialParameterKind kind;
  SlangTargetAPI minAPI;
};

// Table entries are in the order parameters must occur in a kernel parameter list.
const SpecialParameter specialParameterTable[] = {
  { "context", bcinfo::MD_SIG_Ctxt, SPK_CONTEXT, SLANG_M_TARGET_API },
  { "x", bcinfo::MD_SIG_X, SPK_LOCATION, SLANG_MINIMUM_TARGET_API },
  { "y", bcinfo::MD_SIG_Y, SPK_LOCATION, SLANG_MINIMUM_TARGET_API },
  { "z", bcinfo::MD_SIG_Z, SPK_LOCATION, SLANG_M_TARGET_API },
  { nullptr, bcinfo::MD_SIG_None, SPK_LOCATION, SLANG_MINIMUM_TARGET_API }, // marks end of table
};

// If the specified name matches the name of an entry in
// specialParameterTable, return the corresponding table index.
// Return -1 if not found.
int lookupSpecialKernelParameter(const llvm::StringRef name) {
  for (int i = 0; specialParameterTable[i].name != nullptr; ++i) {
    if (name.equals(specialParameterTable[i].name)) {
      return i;
    }
  }

  return -1;
}

} // end anonymous namespace

namespace slang {

// Is the specified name the name of an entry in the specialParameterTable?
bool isSpecialKernelParameter(const llvm::StringRef Name) {
  return lookupSpecialKernelParameter(Name) >= 0;
}

// Return a comma-separated list of names in specialParameterTable
// that are available at the specified API level.
std::string listSpecialKernelParameters(unsigned int api) {
  std::string ret;
  bool first = true;
  for (int i = 0; specialParameterTable[i].name != nullptr; ++i) {
    if (specialParameterTable[i].minAPI > api)
      continue;
    if (first)
      first = false;
    else
      ret += ", ";
    ret += "'";
    ret += specialParameterTable[i].name;
    ret += "'";
  }
  return ret;
}

// Process the optional special parameters:
// - Sets *IndexOfFirstSpecialParameter to the index of the first special parameter, or
//     FD->getNumParams() if none are found.
// - Add bits to *SignatureMetadata for the found special parameters.
// Returns true if no errors.
bool processSpecialKernelParameters(
  slang::RSContext *Context,
  const std::function<std::string ()> &DiagnosticDescription,
  const clang::FunctionDecl *FD,
  size_t *IndexOfFirstSpecialParameter,
  unsigned int *SignatureMetadata) {
  slangAssert(IndexOfFirstSpecialParameter != nullptr);
  slangAssert(SignatureMetadata != nullptr);
  slangAssert(*SignatureMetadata == 0);
  clang::ASTContext &C = Context->getASTContext();

  // Find all special parameters if present.
  int LastSpecialParameterIdx = -1; // index into specialParameterTable
  int FirstLocationSpecialParameterIdx = -1; // index into specialParameterTable
  clang::QualType FirstLocationSpecialParameterType;
  size_t NumParams = FD->getNumParams();
  *IndexOfFirstSpecialParameter = NumParams;
  bool valid = true;
  for (size_t i = 0; i < NumParams; i++) {
    const clang::ParmVarDecl *PVD = FD->getParamDecl(i);
    const llvm::StringRef ParamName = PVD->getName();
    const clang::QualType Type = PVD->getType();
    const clang::QualType QT = Type.getCanonicalType();
    const clang::QualType UT = QT.getUnqualifiedType();
    int SpecialParameterIdx = lookupSpecialKernelParameter(ParamName);

    static const char KernelContextUnqualifiedTypeName[] =
        "const struct rs_kernel_context_t *";
    static const char KernelContextTypeName[] = "rs_kernel_context";

    // If the type is rs_context, it should have been named "context" and classified
    // as a special parameter.
    if (SpecialParameterIdx < 0 && UT.getAsString() == KernelContextUnqualifiedTypeName) {
      Context->ReportError(
          PVD->getLocation(),
          "The special parameter of type '%0' must be called "
          "'context' instead of '%1'.")
          << KernelContextTypeName << ParamName;
      SpecialParameterIdx = lookupSpecialKernelParameter("context");
    }

    // If it's not a special parameter, check that it appears before any special
    // parameter.
    if (SpecialParameterIdx < 0) {
      if (*IndexOfFirstSpecialParameter < NumParams) {
        Context->ReportError(PVD->getLocation(),
                             "In %0, parameter '%1' cannot "
                             "appear after any of the special parameters (%2).")
            << DiagnosticDescription()
            << ParamName
            << listSpecialKernelParameters(Context->getTargetAPI());
        valid = false;
      }
      continue;
    }

    const SpecialParameter &SP = specialParameterTable[SpecialParameterIdx];

    // Verify that this special parameter is OK for the current API level.
    if (Context->getTargetAPI() < SP.minAPI) {
      Context->ReportError(PVD->getLocation(),
                           "%0 targeting SDK levels "
                           "%1-%2 may not use special parameter '%3'.")
          << DiagnosticDescription()
          << SLANG_MINIMUM_TARGET_API << (SP.minAPI - 1)
          << SP.name;
      valid = false;
    }

    // Check that the order of the special parameters is correct.
    if (SpecialParameterIdx < LastSpecialParameterIdx) {
      Context->ReportError(
          PVD->getLocation(),
          "In %0, special parameter '%1' must "
          "be defined before special parameter '%2'.")
          << DiagnosticDescription()
          << SP.name
          << specialParameterTable[LastSpecialParameterIdx].name;
      valid = false;
    }

    // Validate the data type of the special parameter.
    switch (SP.kind) {
    case SPK_LOCATION: {
      // Location special parameters can only be int or uint.
      if (UT != C.UnsignedIntTy && UT != C.IntTy) {
        Context->ReportError(PVD->getLocation(),
                             "Special parameter '%0' must be of type 'int' or "
                             "'unsigned int'. It is of type '%1'.")
            << ParamName << Type.getAsString();
        valid = false;
      }

      // Ensure that all location special parameters have the same type.
      if (FirstLocationSpecialParameterIdx >= 0) {
        if (Type != FirstLocationSpecialParameterType) {
          Context->ReportError(
              PVD->getLocation(),
              "Special parameters '%0' and '%1' must be of the same type. "
              "'%0' is of type '%2' while '%1' is of type '%3'.")
              << specialParameterTable[FirstLocationSpecialParameterIdx].name
              << SP.name << FirstLocationSpecialParameterType.getAsString()
              << Type.getAsString();
          valid = false;
        }
      } else {
        FirstLocationSpecialParameterIdx = SpecialParameterIdx;
        FirstLocationSpecialParameterType = Type;
      }
    } break;
    case SPK_CONTEXT: {
      // Check that variables named "context" are of type rs_context.
      if (UT.getAsString() != KernelContextUnqualifiedTypeName) {
        Context->ReportError(PVD->getLocation(),
                             "Special parameter '%0' must be of type '%1'. "
                             "It is of type '%2'.")
            << ParamName << KernelContextTypeName
            << Type.getAsString();
        valid = false;
      }
    } break;
    default:
      slangAssert(!"Unexpected special parameter type");
    }

    // We should not be invoked if two parameters of the same name are present.
    slangAssert(!(*SignatureMetadata & SP.bitval));
    *SignatureMetadata |= SP.bitval;

    LastSpecialParameterIdx = SpecialParameterIdx;
    // If this is the first time we find a special parameter, save it.
    if (*IndexOfFirstSpecialParameter >= NumParams) {
      *IndexOfFirstSpecialParameter = i;
    }
  }
  return valid;
}

} // namespace slang
