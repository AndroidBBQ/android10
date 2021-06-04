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

#include "slang_rs_export_reduce.h"

#include <algorithm>
#include <sstream>
#include <string>

#include "clang/AST/ASTContext.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_export_type.h"
#include "slang_rs_object_ref_count.h"
#include "slang_rs_special_kernel_param.h"
#include "slang_version.h"

#include "bcinfo/MetadataExtractor.h"

namespace slang {

const char RSExportReduce::KeyReduce[] = "reduce";
const char RSExportReduce::KeyInitializer[] = "initializer";
const char RSExportReduce::KeyAccumulator[] = "accumulator";
const char RSExportReduce::KeyCombiner[] = "combiner";
const char RSExportReduce::KeyOutConverter[] = "outconverter";
const char RSExportReduce::KeyHalter[] = "halter";

bool RSExportReduce::matchName(const llvm::StringRef &Candidate) const {
  return
      Candidate.equals(mNameInitializer)  ||
      Candidate.equals(mNameAccumulator)  ||
      Candidate.equals(mNameCombiner)     ||
      Candidate.equals(mNameOutConverter) ||
      Candidate.equals(mNameHalter);
}

RSExportReduce *RSExportReduce::Create(RSContext *Context,
                                       const clang::SourceLocation Location,
                                       const llvm::StringRef &NameReduce,
                                       const llvm::StringRef &NameInitializer,
                                       const llvm::StringRef &NameAccumulator,
                                       const llvm::StringRef &NameCombiner,
                                       const llvm::StringRef &NameOutConverter,
                                       const llvm::StringRef &NameHalter) {
  slangAssert(Context);
  RSExportReduce *RNE = new RSExportReduce(Context,
                                           Location,
                                           NameReduce,
                                           NameInitializer,
                                           NameAccumulator,
                                           NameCombiner,
                                           NameOutConverter,
                                           NameHalter);

  return RNE;
}

const char *RSExportReduce::getKey(FnIdent Kind) {
  switch (Kind) {
    default:
      slangAssert(!"Unknown FnIdent");
      // and fall through
    case FN_IDENT_INITIALIZER:
      return KeyInitializer;
    case FN_IDENT_ACCUMULATOR:
      return KeyAccumulator;
    case FN_IDENT_COMBINER:
      return KeyCombiner;
    case FN_IDENT_OUT_CONVERTER:
      return KeyOutConverter;
    case FN_IDENT_HALTER:
      return KeyHalter;
  }
}

// This data is needed during analyzeTranslationUnit() but not afterwards.
// Breaking it out into a struct makes it easy for analyzeTranslationUnit()
// to call a number of helper functions that all need access to this data.
struct RSExportReduce::StateOfAnalyzeTranslationUnit {

  typedef std::function<std::string (const char *Key, const std::string &Name)> DiagnosticDescriptionType;

  StateOfAnalyzeTranslationUnit(
      RSContext &anRSContext,
      clang::Preprocessor &aPP,
      clang::ASTContext &anASTContext,
      const DiagnosticDescriptionType &aDiagnosticDescription) :

      RSC(anRSContext),
      PP(aPP),
      ASTC(anASTContext),
      DiagnosticDescription(aDiagnosticDescription),

      Ok(true),

      FnInitializer(nullptr),
      FnAccumulator(nullptr),
      FnCombiner(nullptr),
      FnOutConverter(nullptr),
      FnHalter(nullptr),

      FnInitializerParam(nullptr),
      FnInitializerParamTy(),

      FnAccumulatorOk(true),
      FnAccumulatorParamFirst(nullptr),
      FnAccumulatorParamFirstTy(),
      FnAccumulatorIndexOfFirstSpecialParameter(0),

      FnOutConverterOk(true),
      FnOutConverterParamFirst(nullptr),
      FnOutConverterParamFirstTy()
  { }

  /*-- Convenience ------------------------------------------*/

  RSContext                       &RSC;
  clang::Preprocessor             &PP;
  clang::ASTContext               &ASTC;
  const DiagnosticDescriptionType  DiagnosticDescription;

  /*-- Actual state -----------------------------------------*/

  bool Ok;

  clang::FunctionDecl *FnInitializer;
  clang::FunctionDecl *FnAccumulator;
  clang::FunctionDecl *FnCombiner;
  clang::FunctionDecl *FnOutConverter;
  clang::FunctionDecl *FnHalter;

  clang::ParmVarDecl  *FnInitializerParam;
  clang::QualType      FnInitializerParamTy;

  bool                 FnAccumulatorOk;
  clang::ParmVarDecl  *FnAccumulatorParamFirst;
  clang::QualType      FnAccumulatorParamFirstTy;
  size_t               FnAccumulatorIndexOfFirstSpecialParameter;

  bool                 FnOutConverterOk;  // also true if no outconverter
  clang::ParmVarDecl  *FnOutConverterParamFirst;
  clang::QualType      FnOutConverterParamFirstTy;
};

// does update S.Ok
clang::FunctionDecl *RSExportReduce::lookupFunction(StateOfAnalyzeTranslationUnit &S,
                                                    const char *Kind, const llvm::StringRef &Name) {
  if (Name.empty())
    return nullptr;

  clang::TranslationUnitDecl *TUDecl = getRSContext()->getASTContext().getTranslationUnitDecl();
  slangAssert(TUDecl);

  clang::FunctionDecl *Ret = nullptr;
  const clang::IdentifierInfo *II = S.PP.getIdentifierInfo(Name);
  if (II) {
    for (auto Decl : TUDecl->lookup(II)) {
      clang::FunctionDecl *FDecl = Decl->getAsFunction();
      if (!FDecl || !FDecl->isThisDeclarationADefinition())
        continue;
      if (Ret) {
        S.RSC.ReportError(mLocation,
                          "duplicate function definition for '%0(%1)' for '#pragma rs %2(%3)' (%4, %5)")
            << Kind << Name << KeyReduce << mNameReduce
            << Ret->getLocation().printToString(S.PP.getSourceManager())
            << FDecl->getLocation().printToString(S.PP.getSourceManager());
        S.Ok = false;
        return nullptr;
      }
      Ret = FDecl;
    }
  }
  if (!Ret) {
    // Either the identifier lookup failed, or we never found the function definition.
    S.RSC.ReportError(mLocation,
                      "could not find function definition for '%0(%1)' for '#pragma rs %2(%3)'")
        << Kind << Name << KeyReduce << mNameReduce;
    S.Ok = false;
    return nullptr;
  }
  if (Ret) {
    // Must have internal linkage
    if (Ret->getFormalLinkage() != clang::InternalLinkage) {
      S.RSC.ReportError(Ret->getLocation(), "%0 must be static")
          << S.DiagnosticDescription(Kind, Name);
      S.Ok = false;
    }
  }
  if (Ret == nullptr)
    S.Ok = false;
  return Ret;
}

// updates S.Ok; and, depending on Kind, possibly S.FnAccumulatorOk or S.FnOutConverterOk
void RSExportReduce::notOk(StateOfAnalyzeTranslationUnit &S, FnIdent Kind) {
    S.Ok = false;
    if (Kind == FN_IDENT_ACCUMULATOR) {
      S.FnAccumulatorOk = false;
    } else if (Kind == FN_IDENT_OUT_CONVERTER) {
      S.FnOutConverterOk = false;
    }
}

// updates S.Ok; and, depending on Kind, possibly S.FnAccumulatorOk or S.FnOutConverterOk
void RSExportReduce::checkVoidReturn(StateOfAnalyzeTranslationUnit &S,
                                     FnIdent Kind, clang::FunctionDecl *Fn) {
  slangAssert(Fn);
  const clang::QualType ReturnTy = Fn->getReturnType().getCanonicalType();
  if (!ReturnTy->isVoidType()) {
    S.RSC.ReportError(Fn->getLocation(),
                      "%0 must return void not '%1'")
        << S.DiagnosticDescription(getKey(Kind), Fn->getName()) << ReturnTy.getAsString();
    notOk(S, Kind);
  }
}

// updates S.Ok; and, depending on Kind, possibly S.FnAccumulatorOk or S.FnOutConverterOk
void RSExportReduce::checkPointeeConstQualified(StateOfAnalyzeTranslationUnit &S,
                                                FnIdent Kind, const llvm::StringRef &Name,
                                                const clang::ParmVarDecl *Param, bool ExpectedQualification) {
  const clang::QualType ParamQType = Param->getType();
  slangAssert(ParamQType->isPointerType());
  const clang::QualType PointeeQType = ParamQType->getPointeeType();
  if (PointeeQType.isConstQualified() != ExpectedQualification) {
    S.RSC.ReportError(Param->getLocation(),
                      "%0 parameter '%1' (type '%2') must%3 point to const-qualified type")
        << S.DiagnosticDescription(getKey(Kind), Name)
        << Param->getName() << ParamQType.getAsString()
        << (ExpectedQualification ? "" : " not");
    notOk(S, Kind);
  }
}

// Process "void mNameInitializer(compType *accum)"
void RSExportReduce::analyzeInitializer(StateOfAnalyzeTranslationUnit &S) {
  if (!S.FnInitializer) // initializer is always optional
    return;

  // Must return void
  checkVoidReturn(S, FN_IDENT_INITIALIZER, S.FnInitializer);

  // Must have exactly one parameter
  if (S.FnInitializer->getNumParams() != 1) {
    S.RSC.ReportError(S.FnInitializer->getLocation(),
                      "%0 must take exactly 1 parameter (found %1)")
        << S.DiagnosticDescription(KeyInitializer, mNameInitializer)
        << S.FnInitializer->getNumParams();
    S.Ok = false;
    return;
  }

  // Parameter must not be a special parameter
  S.FnInitializerParam = S.FnInitializer->getParamDecl(0);
  if (isSpecialKernelParameter(S.FnInitializerParam->getName())) {
    S.RSC.ReportError(S.FnInitializer->getLocation(),
                      "%0 cannot take special parameter '%1'")
        << S.DiagnosticDescription(KeyInitializer, mNameInitializer)
        << S.FnInitializerParam->getName();
    S.Ok = false;
    return;
  }

  // Parameter must be of pointer type
  S.FnInitializerParamTy = S.FnInitializerParam->getType().getCanonicalType();
  if (!S.FnInitializerParamTy->isPointerType()) {
    S.RSC.ReportError(S.FnInitializer->getLocation(),
                      "%0 parameter '%1' must be of pointer type not '%2'")
        << S.DiagnosticDescription(KeyInitializer, mNameInitializer)
        << S.FnInitializerParam->getName() << S.FnInitializerParamTy.getAsString();
    S.Ok = false;
    return;
  }

  // Parameter must not point to const-qualified
  checkPointeeConstQualified(S, FN_IDENT_INITIALIZER, mNameInitializer, S.FnInitializerParam, false);
}

// Process "void mNameAccumulator(compType *accum, in1Type in1, …, inNType inN[, specialarguments])"
void RSExportReduce::analyzeAccumulator(StateOfAnalyzeTranslationUnit &S) {
  slangAssert(S.FnAccumulator);

  // Must return void
  checkVoidReturn(S, FN_IDENT_ACCUMULATOR, S.FnAccumulator);

  // Must have initial parameter of same type as initializer parameter
  // (if there is an initializer), followed by at least 1 input

  if (S.FnAccumulator->getNumParams() < 2) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 must take at least 2 parameters")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator);
    S.Ok = S.FnAccumulatorOk = false;
    return;
  }

  S.FnAccumulatorParamFirst = S.FnAccumulator->getParamDecl(0);
  S.FnAccumulatorParamFirstTy = S.FnAccumulatorParamFirst->getType().getCanonicalType();

  // First parameter must be of pointer type
  if (!S.FnAccumulatorParamFirstTy->isPointerType()) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' must be of pointer type not '%2'")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
    S.Ok = S.FnAccumulatorOk = false;
    return;
  }

  // If there is an initializer with a pointer-typed parameter (as
  // opposed to an initializer with a bad parameter list), then
  // accumulator first parameter must be of same type as initializer
  // parameter
  if (S.FnInitializer &&
      !S.FnInitializerParamTy.isNull() &&
      S.FnInitializerParamTy->isPointerType() &&
      !S.FnAccumulator->getASTContext().hasSameUnqualifiedType(
          S.FnInitializerParamTy->getPointeeType().getCanonicalType(),
          S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType())) {
    // <accumulator> parameter '<baz>' (type '<tbaz>') and initializer <goo>() parameter '<gaz>' (type '<tgaz>')
    //   must be pointers to the same type
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' (type '%2') and %3 %4() parameter '%5' (type '%6')"
                      " must be pointers to the same type")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString()
        << KeyInitializer << mNameInitializer
        << S.FnInitializerParam->getName() << S.FnInitializerParamTy.getAsString();
    S.Ok = S.FnAccumulatorOk = false;
  }

  if (S.FnAccumulatorOk && S.FnAccumulatorParamFirstTy->getPointeeType()->isFunctionType()) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' (type '%2') must not be pointer to function type")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
    S.Ok = S.FnAccumulatorOk = false;
  }

  if (S.FnAccumulatorOk && S.FnAccumulatorParamFirstTy->getPointeeType()->isIncompleteType()) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' (type '%2') must not be pointer to incomplete type")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
    S.Ok = S.FnAccumulatorOk = false;
  }

  if (S.FnAccumulatorOk &&
      HasRSObjectType(S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType().getTypePtr())) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' (type '%2') must not be pointer to data containing an object type")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
    S.Ok = S.FnAccumulatorOk = false;
  }

  // Parameter must not point to const-qualified
  checkPointeeConstQualified(S, FN_IDENT_ACCUMULATOR, mNameAccumulator, S.FnAccumulatorParamFirst, false);

  // Analyze special parameters
  S.Ok &= (S.FnAccumulatorOk &= processSpecialKernelParameters(
                                  &S.RSC,
                                  std::bind(S.DiagnosticDescription, KeyAccumulator, mNameAccumulator),
                                  S.FnAccumulator,
                                  &S.FnAccumulatorIndexOfFirstSpecialParameter,
                                  &mAccumulatorSignatureMetadata));

  // Must have at least an accumulator and an input.
  // If we get here we know there are at least 2 arguments; so the only problem case is
  // where we have an accumulator followed immediately by a special parameter.
  if (S.FnAccumulatorIndexOfFirstSpecialParameter < 2) {
    slangAssert(S.FnAccumulatorIndexOfFirstSpecialParameter < S.FnAccumulator->getNumParams());
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 must have at least 1 input ('%1' is a special parameter)")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulator->getParamDecl(S.FnAccumulatorIndexOfFirstSpecialParameter)->getName();
    S.Ok = S.FnAccumulatorOk = false;
    return;
  }

  if (S.FnAccumulatorOk) {
    mAccumulatorSignatureMetadata |= bcinfo::MD_SIG_In;
    mAccumulatorTypeSize = S.ASTC.getTypeSizeInChars(S.FnAccumulatorParamFirstTy->getPointeeType()).getQuantity();
    for (size_t ParamIdx = 1; ParamIdx < S.FnAccumulatorIndexOfFirstSpecialParameter; ++ParamIdx) {
      const clang::ParmVarDecl *const Param = S.FnAccumulator->getParamDecl(ParamIdx);
      mAccumulatorIns.push_back(Param);
      const clang::QualType ParamQType = Param->getType().getCanonicalType();
      const clang::Type *ParamType = ParamQType.getTypePtr();

      RSExportType *ParamEType = nullptr;
      if (ParamQType->isPointerType()) {
        S.RSC.ReportError(Param->getLocation(),
                          "%0 parameter '%1' (type '%2') must not be a pointer")
            << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
            << Param->getName() << ParamQType.getAsString();
        S.Ok = false;
      } else if (HasRSObjectType(ParamType)) {
        S.RSC.ReportError(Param->getLocation(),
                          "%0 parameter '%1' (type '%2') must not contain an object type")
            << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
            << Param->getName() << ParamQType.getAsString();
        S.Ok = false;
      } else if (RSExportType::ValidateType(&S.RSC, S.ASTC, ParamQType, Param, Param->getLocStart(),
                                            S.RSC.getTargetAPI(),
                                            false /* IsFilterscript */,
                                            true /* IsExtern */)) {
        // TODO: Better diagnostics on validation or creation failure?
        ParamEType = RSExportType::Create(&S.RSC, ParamType, NotLegacyKernelArgument);
        S.Ok &= (ParamEType != nullptr);
      } else {
        S.Ok = false;
      }
      mAccumulatorInTypes.push_back(ParamEType); // possibly nullptr
    }
  }
}

// Process "void combinename(compType *accum, const compType *val)"
void RSExportReduce::analyzeCombiner(StateOfAnalyzeTranslationUnit &S) {
  if (S.FnCombiner) {
    // Must return void
    checkVoidReturn(S, FN_IDENT_COMBINER, S.FnCombiner);

    // Must have exactly two parameters, of same type as first accumulator parameter

    if (S.FnCombiner->getNumParams() != 2) {
      S.RSC.ReportError(S.FnCombiner->getLocation(),
                        "%0 must take exactly 2 parameters (found %1)")
          << S.DiagnosticDescription(KeyCombiner, mNameCombiner)
          << S.FnCombiner->getNumParams();
      S.Ok = false;
      return;
    }

    if (S.FnAccumulatorParamFirstTy.isNull() || !S.FnAccumulatorParamFirstTy->isPointerType()) {
      // We're already in an error situation.  We could compare
      // against the initializer parameter type instead of the first
      // accumulator parameter type (we'd have to check for the
      // availability of a parameter type there, too), but it does not
      // seem worth the effort.
      //
      // Likewise, we could compare the two combiner parameter types
      // against each other.
      slangAssert(!S.Ok);
      return;
    }

    for (int ParamIdx = 0; ParamIdx < 2; ++ParamIdx) {
      const clang::ParmVarDecl *const FnCombinerParam = S.FnCombiner->getParamDecl(ParamIdx);
      const clang::QualType FnCombinerParamTy = FnCombinerParam->getType().getCanonicalType();
      if (!FnCombinerParamTy->isPointerType() ||
          !S.FnCombiner->getASTContext().hasSameUnqualifiedType(
              S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType(),
              FnCombinerParamTy->getPointeeType().getCanonicalType())) {
        // <combiner> parameter '<baz>' (type '<tbaz>')
        //   and accumulator <goo>() parameter '<gaz>' (type '<tgaz>') must be pointers to the same type
        S.RSC.ReportError(S.FnCombiner->getLocation(),
                          "%0 parameter '%1' (type '%2') and %3 %4() parameter '%5' (type '%6')"
                          " must be pointers to the same type")
            << S.DiagnosticDescription(KeyCombiner, mNameCombiner)
            << FnCombinerParam->getName() << FnCombinerParamTy.getAsString()
            << KeyAccumulator << mNameAccumulator
            << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
        S.Ok = false;
      } else {
        // Check const-qualification
        checkPointeeConstQualified(S, FN_IDENT_COMBINER, mNameCombiner, FnCombinerParam, ParamIdx==1);
      }
    }

    return;
  }

  // Ensure accumulator properties permit omission of combiner.

  if (!S.FnAccumulatorOk) {
    // Couldn't fully analyze accumulator, so cannot see whether it permits omission of combiner.
    return;
  }

  if (mAccumulatorIns.size() != 1 ||
      S.FnAccumulatorIndexOfFirstSpecialParameter != S.FnAccumulator->getNumParams())
  {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 must have exactly 1 input"
                      " and no special parameters in order for the %1 to be omitted")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << KeyCombiner;
    S.Ok = false;
    return;
  }

  const clang::ParmVarDecl *const FnAccumulatorParamInput = S.FnAccumulator->getParamDecl(1);
  const clang::QualType FnAccumulatorParamInputTy = FnAccumulatorParamInput->getType().getCanonicalType();
  if (!S.FnAccumulator->getASTContext().hasSameUnqualifiedType(
          S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType(),
          FnAccumulatorParamInputTy.getCanonicalType())) {
    S.RSC.ReportError(S.FnAccumulator->getLocation(),
                      "%0 parameter '%1' (type '%2')"
                      " must be pointer to the type of parameter '%3' (type '%4')"
                      " in order for the %5 to be omitted")
        << S.DiagnosticDescription(KeyAccumulator, mNameAccumulator)
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString()
        << FnAccumulatorParamInput->getName() << FnAccumulatorParamInputTy.getAsString()
        << KeyCombiner;
    S.Ok = false;
  }
}

// Process "void outconvertname(resultType *result, const compType *accum)"
void RSExportReduce::analyzeOutConverter(StateOfAnalyzeTranslationUnit &S) {
  if (!S.FnOutConverter) // outconverter is always optional
    return;

  // Must return void
  checkVoidReturn(S, FN_IDENT_OUT_CONVERTER, S.FnOutConverter);

  // Must have exactly two parameters
  if (S.FnOutConverter->getNumParams() != 2) {
    S.RSC.ReportError(S.FnOutConverter->getLocation(),
                      "%0 must take exactly 2 parameters (found %1)")
        << S.DiagnosticDescription(KeyOutConverter, mNameOutConverter)
        << S.FnOutConverter->getNumParams();
    S.Ok = S.FnOutConverterOk = false;
    return;
  }

  // Parameters must not be special and must be of pointer type;
  // and second parameter must match first accumulator parameter
  for (int ParamIdx = 0; ParamIdx < 2; ++ParamIdx) {
    clang::ParmVarDecl *const FnOutConverterParam = S.FnOutConverter->getParamDecl(ParamIdx);

    if (isSpecialKernelParameter(FnOutConverterParam->getName())) {
      S.RSC.ReportError(S.FnOutConverter->getLocation(),
                        "%0 cannot take special parameter '%1'")
          << S.DiagnosticDescription(KeyOutConverter, mNameOutConverter)
          << FnOutConverterParam->getName();
      S.Ok = S.FnOutConverterOk = false;
      continue;
    }

    const clang::QualType FnOutConverterParamTy = FnOutConverterParam->getType().getCanonicalType();

    if (!FnOutConverterParamTy->isPointerType()) {
      S.RSC.ReportError(S.FnOutConverter->getLocation(),
                        "%0 parameter '%1' must be of pointer type not '%2'")
          << S.DiagnosticDescription(KeyOutConverter, mNameOutConverter)
          << FnOutConverterParam->getName() << FnOutConverterParamTy.getAsString();
      S.Ok = S.FnOutConverterOk = false;
      continue;
    }

    // Check const-qualification
    checkPointeeConstQualified(S, FN_IDENT_OUT_CONVERTER, mNameOutConverter, FnOutConverterParam, ParamIdx==1);

    if (ParamIdx == 0) {
      S.FnOutConverterParamFirst = FnOutConverterParam;
      S.FnOutConverterParamFirstTy = FnOutConverterParamTy;
      continue;
    }

    if (S.FnAccumulatorParamFirstTy.isNull() || !S.FnAccumulatorParamFirstTy->isPointerType()) {
      // We're already in an error situation.  We could compare
      // against the initializer parameter type instead of the first
      // accumulator parameter type (we'd have to check for the
      // availability of a parameter type there, too), but it does not
      // seem worth the effort.
      slangAssert(!S.Ok);
      continue;
    }

    if (!S.FnOutConverter->getASTContext().hasSameUnqualifiedType(
            S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType(),
            FnOutConverterParamTy->getPointeeType().getCanonicalType())) {
      // <outconverter> parameter '<baz>' (type '<tbaz>')
      //   and accumulator <goo>() parameter '<gaz>' (type '<tgaz>') must be pointers to the same type
      S.RSC.ReportError(S.FnOutConverter->getLocation(),
                        "%0 parameter '%1' (type '%2') and %3 %4() parameter '%5' (type '%6')"
                        " must be pointers to the same type")
          << S.DiagnosticDescription(KeyOutConverter, mNameOutConverter)
          << FnOutConverterParam->getName() << FnOutConverterParamTy.getAsString()
          << KeyAccumulator << mNameAccumulator
          << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
      S.Ok = S.FnOutConverterOk = false;
    }
  }
}

// Process "bool haltername(const compType *accum)"
void RSExportReduce::analyzeHalter(StateOfAnalyzeTranslationUnit &S) {
  if (!S.FnHalter) // halter is always optional
    return;

  // Must return bool
  const clang::QualType ReturnTy = S.FnHalter->getReturnType().getCanonicalType();
  if (!ReturnTy->isBooleanType()) {
    S.RSC.ReportError(S.FnHalter->getLocation(),
                    "%0 must return bool not '%1'")
        << S.DiagnosticDescription(KeyHalter, mNameHalter) << ReturnTy.getAsString();
    S.Ok = false;
  }

  // Must have exactly one parameter
  if (S.FnHalter->getNumParams() != 1) {
    S.RSC.ReportError(S.FnHalter->getLocation(),
                      "%0 must take exactly 1 parameter (found %1)")
        << S.DiagnosticDescription(KeyHalter, mNameHalter)
        << S.FnHalter->getNumParams();
    S.Ok = false;
    return;
  }

  // Parameter must not be a special parameter
  const clang::ParmVarDecl *const FnHalterParam = S.FnHalter->getParamDecl(0);
  if (isSpecialKernelParameter(FnHalterParam->getName())) {
    S.RSC.ReportError(S.FnHalter->getLocation(),
                      "%0 cannot take special parameter '%1'")
        << S.DiagnosticDescription(KeyHalter, mNameHalter)
        << FnHalterParam->getName();
    S.Ok = false;
    return;
  }

  // Parameter must be same type as first accumulator parameter

  if (S.FnAccumulatorParamFirstTy.isNull() || !S.FnAccumulatorParamFirstTy->isPointerType()) {
    // We're already in an error situation.  We could compare against
    // the initializer parameter type or the first combiner parameter
    // type instead of the first accumulator parameter type (we'd have
    // to check for the availability of a parameter type there, too),
    // but it does not seem worth the effort.
    slangAssert(!S.Ok);
    return;
  }

  const clang::QualType FnHalterParamTy = FnHalterParam->getType().getCanonicalType();
  if (!FnHalterParamTy->isPointerType() ||
      !S.FnHalter->getASTContext().hasSameUnqualifiedType(
          S.FnAccumulatorParamFirstTy->getPointeeType().getCanonicalType(),
          FnHalterParamTy->getPointeeType().getCanonicalType())) {
    // <halter> parameter '<baz>' (type '<tbaz>')
    //   and accumulator <goo>() parameter '<gaz>' (type '<tgaz>') must be pointers to the same type
    S.RSC.ReportError(S.FnHalter->getLocation(),
                      "%0 parameter '%1' (type '%2') and %3 %4() parameter '%5' (type '%6')"
                      " must be pointers to the same type")
        << S.DiagnosticDescription(KeyHalter, mNameHalter)
        << FnHalterParam->getName() << FnHalterParamTy.getAsString()
        << KeyAccumulator << mNameAccumulator
        << S.FnAccumulatorParamFirst->getName() << S.FnAccumulatorParamFirstTy.getAsString();
    S.Ok = false;
    return;
  }

  // Parameter must point to const-qualified
  checkPointeeConstQualified(S, FN_IDENT_HALTER, mNameHalter, FnHalterParam, true);
}

void RSExportReduce::analyzeResultType(StateOfAnalyzeTranslationUnit &S) {
  if (!(S.FnAccumulatorOk && S.FnOutConverterOk)) {
    // No idea what the result type is
    slangAssert(!S.Ok);
    return;
  }

  struct ResultInfoType {
    const clang::QualType QType;
    clang::VarDecl *const Decl;
    const char *FnKey;
    const std::string &FnName;
    std::function<std::string ()> UnlessOutConverter;
  } ResultInfo =
        S.FnOutConverter
        ? ResultInfoType({ S.FnOutConverterParamFirstTy, S.FnOutConverterParamFirst,
                           KeyOutConverter, mNameOutConverter,
                           []() { return std::string(""); }})
        : ResultInfoType({ S.FnAccumulatorParamFirstTy,  S.FnAccumulatorParamFirst,
                           KeyAccumulator,  mNameAccumulator,
                           []() { return std::string(" unless ") + KeyOutConverter + " is provided"; }});
  const clang::QualType PointeeQType = ResultInfo.QType->getPointeeType();

  if (PointeeQType->isPointerType()) {
    S.RSC.ReportError(ResultInfo.Decl->getLocation(),
                      "%0 parameter '%1' (type '%2') must not point to a pointer%3")
        << S.DiagnosticDescription(ResultInfo.FnKey, ResultInfo.FnName)
        << ResultInfo.Decl->getName() << ResultInfo.QType.getAsString()
        << ResultInfo.UnlessOutConverter();
  } else if (PointeeQType->isIncompleteType()) {
    S.RSC.ReportError(ResultInfo.Decl->getLocation(),
                      "%0 parameter '%1' (type '%2') must not point to an incomplete type%3")
        << S.DiagnosticDescription(ResultInfo.FnKey, ResultInfo.FnName)
        << ResultInfo.Decl->getName() << ResultInfo.QType.getAsString()
        << ResultInfo.UnlessOutConverter();
  } else if (HasRSObjectType(PointeeQType.getTypePtr())) {
    S.RSC.ReportError(ResultInfo.Decl->getLocation(),
                      "%0 parameter '%1' (type '%2') must not point to data containing an object type%3")
        << S.DiagnosticDescription(ResultInfo.FnKey, ResultInfo.FnName)
        << ResultInfo.Decl->getName() << ResultInfo.QType.getAsString()
        << ResultInfo.UnlessOutConverter();
  } else if (RSExportType::ValidateType(&S.RSC, S.ASTC, PointeeQType,
                                        ResultInfo.Decl, ResultInfo.Decl->getLocStart(),
                                        S.RSC.getTargetAPI(),
                                        false /* IsFilterscript */,
                                        true /* IsExtern */)) {
    // TODO: Better diagnostics on validation or creation failure?
    if ((mResultType = RSExportType::Create(&S.RSC, PointeeQType.getTypePtr(),
                                            NotLegacyKernelArgument, ResultInfo.Decl)) != nullptr) {
      const RSExportType *CheckType = mResultType;
      const char *ArrayErrorPhrase = "";
      if (mResultType->getClass() == RSExportType::ExportClassConstantArray) {
        CheckType = static_cast<const RSExportConstantArrayType *>(mResultType)->getElementType();
        ArrayErrorPhrase = "n array of";
      }
      switch (CheckType->getClass()) {
        case RSExportType::ExportClassMatrix:
          // Not supported for now -- what does a matrix result type mean?
          S.RSC.ReportError(ResultInfo.Decl->getLocation(),
                            "%0 parameter '%1' (type '%2') must not point to a%3 matrix type%4")
              << S.DiagnosticDescription(ResultInfo.FnKey, ResultInfo.FnName)
              << ResultInfo.Decl->getName() << ResultInfo.QType.getAsString()
              << ArrayErrorPhrase
              << ResultInfo.UnlessOutConverter();
          mResultType = nullptr;
          break;
        default:
          // All's well
          break;
      }
    }
  }

  if (mResultType)
    S.RSC.insertExportReduceResultType(mResultType);
  else
    S.Ok = false;
}

bool RSExportReduce::analyzeTranslationUnit() {

  RSContext &RSC = *getRSContext();
  clang::Preprocessor &PP = RSC.getPreprocessor();

  StateOfAnalyzeTranslationUnit S(
      RSC, PP, RSC.getASTContext(),
      [&PP, this] (const char *Key, const std::string &Name) {
        std::ostringstream Description;
        Description
            << Key << " " << Name << "()"
            << " for '#pragma rs " << KeyReduce << "(" << mNameReduce << ")'"
            << " (" << mLocation.printToString(PP.getSourceManager()) << ")";
        return Description.str();
      });

  S.FnInitializer  = lookupFunction(S, KeyInitializer,  mNameInitializer);
  S.FnAccumulator  = lookupFunction(S, KeyAccumulator,  mNameAccumulator);
  S.FnCombiner     = lookupFunction(S, KeyCombiner,     mNameCombiner);
  S.FnOutConverter = lookupFunction(S, KeyOutConverter, mNameOutConverter);
  S.FnHalter       = lookupFunction(S, KeyHalter,       mNameHalter);

  if (!S.Ok)
    return false;

  analyzeInitializer(S);
  analyzeAccumulator(S);
  analyzeCombiner(S);
  analyzeOutConverter(S);
  analyzeHalter(S);
  analyzeResultType(S);

  return S.Ok;
}

}  // namespace slang
