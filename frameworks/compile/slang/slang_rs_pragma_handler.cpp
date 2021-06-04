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

#include "slang_rs_pragma_handler.h"

#include <map>
#include <sstream>
#include <string>

#include "clang/AST/ASTContext.h"

#include "clang/Basic/TokenKinds.h"

#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Token.h"

#include "slang_assert.h"
#include "slang_rs_context.h"
#include "slang_rs_export_reduce.h"
#include "slang_version.h"

namespace slang {

namespace {  // Anonymous namespace

class RSExportTypePragmaHandler : public RSPragmaHandler {
 private:
  void handleItem(const std::string &Item) {
    mContext->addPragma(this->getName(), Item);
    mContext->addExportType(Item);
  }

 public:
  RSExportTypePragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) { }

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &FirstToken) {
    this->handleItemListPragma(PP, FirstToken);
  }
};

class RSJavaPackageNamePragmaHandler : public RSPragmaHandler {
 public:
  RSJavaPackageNamePragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) { }

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &FirstToken) {
    // FIXME: Need to validate the extracted package name from pragma.
    // Currently "all chars" specified in pragma will be treated as package
    // name.
    //
    // 18.1 The Grammar of the Java Programming Language
    // (http://java.sun.com/docs/books/jls/third_edition/html/syntax.html#18.1)
    //
    // CompilationUnit:
    //     [[Annotations] package QualifiedIdentifier   ;  ] {ImportDeclaration}
    //     {TypeDeclaration}
    //
    // QualifiedIdentifier:
    //     Identifier { . Identifier }
    //
    // Identifier:
    //     IDENTIFIER
    //
    // 3.8 Identifiers
    // (http://java.sun.com/docs/books/jls/third_edition/html/lexical.html#3.8)
    //
    //

    clang::Token &PragmaToken = FirstToken;
    std::string PackageName;

    // Skip first token, "java_package_name"
    PP.LexUnexpandedToken(PragmaToken);

    // Now, the current token must be clang::tok::lpara
    if (PragmaToken.isNot(clang::tok::l_paren))
      return;

    while (PragmaToken.isNot(clang::tok::eod)) {
      // Lex package name
      PP.LexUnexpandedToken(PragmaToken);

      bool Invalid;
      std::string Spelling = PP.getSpelling(PragmaToken, &Invalid);
      if (!Invalid)
        PackageName.append(Spelling);

      // Pre-mature end (syntax error will be triggered by preprocessor later)
      if (PragmaToken.is(clang::tok::eod) || PragmaToken.is(clang::tok::eof)) {
        break;
      } else {
        // Next token is ')' (end of pragma)
        const clang::Token &NextTok = PP.LookAhead(0);
        if (NextTok.is(clang::tok::r_paren)) {
          mContext->addPragma(this->getName(), PackageName);
          mContext->setReflectJavaPackageName(PackageName);
          // Lex until meets clang::tok::eod
          do {
            PP.LexUnexpandedToken(PragmaToken);
          } while (PragmaToken.isNot(clang::tok::eod));
          break;
        }
      }
    }
  }
};

class RSReducePragmaHandler : public RSPragmaHandler {
 public:
  RSReducePragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) { }

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &FirstToken) override {
    // #pragma rs reduce(name)
    //   initializer(initializename)
    //   accumulator(accumulatename)
    //   combiner(combinename)
    //   outconverter(outconvertname)
    //   halter(haltname)

    const clang::SourceLocation PragmaLocation = FirstToken.getLocation();

    clang::Token &PragmaToken = FirstToken;

    // Grab "reduce(name)" ("reduce" is already known to be the first
    // token) and all the "keyword(value)" contributions
    KeywordValueMapType KeywordValueMap({std::make_pair(RSExportReduce::KeyReduce, ""),
                                         std::make_pair(RSExportReduce::KeyInitializer, ""),
                                         std::make_pair(RSExportReduce::KeyAccumulator, ""),
                                         std::make_pair(RSExportReduce::KeyCombiner, ""),
                                         std::make_pair(RSExportReduce::KeyOutConverter, "")});
    if (mContext->getTargetAPI() >= SLANG_FEATURE_GENERAL_REDUCTION_HALTER_API) {
      // Halter functionality has not been released, nor has its
      // specification been finalized with partners.  We do not have a
      // specification that extends through the full RenderScript
      // software stack, either.
      KeywordValueMap.insert(std::make_pair(RSExportReduce::KeyHalter, ""));
    }
    while (PragmaToken.is(clang::tok::identifier)) {
      if (!ProcessKeywordAndValue(PP, PragmaToken, KeywordValueMap))
        return;
    }

    // Make sure there's no end-of-line garbage
    if (PragmaToken.isNot(clang::tok::eod)) {
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "did not expect '%0' here for '#pragma rs %1'"))
          << PP.getSpelling(PragmaToken) << getName();
      return;
    }

    // Make sure we have an accumulator
    if (KeywordValueMap[RSExportReduce::KeyAccumulator].empty()) {
      PP.Diag(PragmaLocation, PP.getDiagnostics().getCustomDiagID(
                                clang::DiagnosticsEngine::Error,
                                "missing '%0' for '#pragma rs %1'"))
          << RSExportReduce::KeyAccumulator << getName();
      return;
    }

    // Make sure the reduction kernel name is unique.  (If we were
    // worried there might be a VERY large number of pragmas, then we
    // could do something more efficient than walking a list to search
    // for duplicates.)
    for (auto I = mContext->export_reduce_begin(),
              E = mContext->export_reduce_end();
         I != E; ++I) {
      if ((*I)->getNameReduce() == KeywordValueMap[RSExportReduce::KeyReduce]) {
        PP.Diag(PragmaLocation, PP.getDiagnostics().getCustomDiagID(
                                  clang::DiagnosticsEngine::Error,
                                  "reduction kernel '%0' declared multiple "
                                  "times (first one is at %1)"))
            << KeywordValueMap[RSExportReduce::KeyReduce]
            << (*I)->getLocation().printToString(PP.getSourceManager());
        return;
      }
    }

    // Check API version.
    if (mContext->getTargetAPI() < SLANG_FEATURE_GENERAL_REDUCTION_API) {
      PP.Diag(PragmaLocation,
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "reduction kernels are not supported in SDK levels %0-%1"))
          << SLANG_MINIMUM_TARGET_API
          << (SLANG_FEATURE_GENERAL_REDUCTION_API - 1);
      return;
    }

    // Handle backward reference from pragma (see Backend::HandleTopLevelDecl for forward reference).
    MarkUsed(PP, KeywordValueMap[RSExportReduce::KeyInitializer]);
    MarkUsed(PP, KeywordValueMap[RSExportReduce::KeyAccumulator]);
    MarkUsed(PP, KeywordValueMap[RSExportReduce::KeyCombiner]);
    MarkUsed(PP, KeywordValueMap[RSExportReduce::KeyOutConverter]);
    MarkUsed(PP, KeywordValueMap[RSExportReduce::KeyHalter]);

    mContext->addExportReduce(RSExportReduce::Create(mContext, PragmaLocation,
                                                     KeywordValueMap[RSExportReduce::KeyReduce],
                                                     KeywordValueMap[RSExportReduce::KeyInitializer],
                                                     KeywordValueMap[RSExportReduce::KeyAccumulator],
                                                     KeywordValueMap[RSExportReduce::KeyCombiner],
                                                     KeywordValueMap[RSExportReduce::KeyOutConverter],
                                                     KeywordValueMap[RSExportReduce::KeyHalter]));
  }

 private:
  typedef std::map<std::string, std::string> KeywordValueMapType;

  void MarkUsed(clang::Preprocessor &PP, const std::string &FunctionName) {
    if (FunctionName.empty())
      return;

    clang::ASTContext &ASTC = mContext->getASTContext();
    clang::TranslationUnitDecl *TUDecl = ASTC.getTranslationUnitDecl();
    slangAssert(TUDecl);
    if (const clang::IdentifierInfo *II = PP.getIdentifierInfo(FunctionName)) {
      for (auto Decl : TUDecl->lookup(II)) {
        clang::FunctionDecl *FDecl = Decl->getAsFunction();
        if (!FDecl || !FDecl->isThisDeclarationADefinition())
          continue;
        // Handle backward reference from pragma (see
        // Backend::HandleTopLevelDecl for forward reference).
        mContext->markUsedByReducePragma(FDecl, RSContext::CheckNameNo);
      }
    }
  }

  // Return comma-separated list of all keys in the map
  static std::string ListKeywords(const KeywordValueMapType &KeywordValueMap) {
    std::string Ret;
    bool First = true;
    for (auto const &entry : KeywordValueMap) {
      if (First)
        First = false;
      else
        Ret += ", ";
      Ret += "'";
      Ret += entry.first;
      Ret += "'";
    }
    return Ret;
  }

  // Parse "keyword(value)" and set KeywordValueMap[keyword] = value.  (Both
  // "keyword" and "value" are identifiers.)
  // Does both syntactic validation and the following semantic validation:
  // - The keyword must be present in the map.
  // - The map entry for the keyword must not contain a value.
  bool ProcessKeywordAndValue(clang::Preprocessor &PP,
                              clang::Token &PragmaToken,
                              KeywordValueMapType &KeywordValueMap) {
    // The current token must be an identifier in KeywordValueMap
    KeywordValueMapType::iterator Entry;
    if (PragmaToken.isNot(clang::tok::identifier) ||
        ((Entry = KeywordValueMap.find(
            PragmaToken.getIdentifierInfo()->getName())) ==
         KeywordValueMap.end())) {
      // Note that we should never get here for the "reduce" token
      // itself, which should already have been recognized.
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "did not recognize '%0' for '#pragma %1'; expected one of "
                "the following keywords: %2"))
          << PragmaToken.getIdentifierInfo()->getName() << getName()
          << ListKeywords(KeywordValueMap);
      return false;
    }
    // ... and there must be no value for this keyword yet
    if (!Entry->second.empty()) {
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "more than one '%0' for '#pragma rs %1'"))
          << Entry->first << getName();
      return false;
    }
    PP.LexUnexpandedToken(PragmaToken);

    // The current token must be clang::tok::l_paren
    if (PragmaToken.isNot(clang::tok::l_paren)) {
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "missing '(' after '%0' for '#pragma rs %1'"))
          << Entry->first << getName();
      return false;
    }
    PP.LexUnexpandedToken(PragmaToken);

    // The current token must be an identifier (a name)
    if (PragmaToken.isNot(clang::tok::identifier)) {
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "missing name after '%0(' for '#pragma rs %1'"))
          << Entry->first << getName();
      return false;
    }
    const std::string Name = PragmaToken.getIdentifierInfo()->getName();
    PP.LexUnexpandedToken(PragmaToken);

    // The current token must be clang::tok::r_paren
    if (PragmaToken.isNot(clang::tok::r_paren)) {
      PP.Diag(PragmaToken.getLocation(),
              PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "missing ')' after '%0(%1' for '#pragma rs %2'"))
          << Entry->first << Name << getName();
      return false;
    }
    PP.LexUnexpandedToken(PragmaToken);

    // Success
    Entry->second = Name;
    return true;
  }
};

class RSReflectLicensePragmaHandler : public RSPragmaHandler {
 private:
  void handleItem(const std::string &Item) {
    mContext->addPragma(this->getName(), Item);
    mContext->setLicenseNote(Item);
  }

 public:
  RSReflectLicensePragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) { }

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &FirstToken) {
    this->handleOptionalStringLiteralParamPragma(PP, FirstToken);
  }
};

class RSVersionPragmaHandler : public RSPragmaHandler {
 private:
  void handleInt(clang::Preprocessor &PP,
                 clang::Token &Tok,
                 const int v) {
    if (v != 1) {
      PP.Diag(Tok,
              PP.getDiagnostics().getCustomDiagID(
                  clang::DiagnosticsEngine::Error,
                  "pragma for version in source file must be set to 1"));
      mContext->setVersion(1);
      return;
    }
    std::stringstream ss;
    ss << v;
    mContext->addPragma(this->getName(), ss.str());
    mContext->setVersion(v);
  }

 public:
  RSVersionPragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) { }

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &FirstToken) {
    this->handleIntegerParamPragma(PP, FirstToken);
  }
};

// Handles the pragmas rs_fp_full, rs_fp_relaxed, and rs_fp_imprecise.
// There's one instance of this handler for each of the above values.
// Only getName() differs between the instances.
class RSPrecisionPragmaHandler : public RSPragmaHandler {
public:
  RSPrecisionPragmaHandler(llvm::StringRef Name, RSContext *Context)
      : RSPragmaHandler(Name, Context) {}

  void HandlePragma(clang::Preprocessor &PP,
                    clang::PragmaIntroducerKind Introducer,
                    clang::Token &Token) {
    std::string Precision = getName();
    // We are deprecating rs_fp_imprecise.
    if (Precision == "rs_fp_imprecise") {
      PP.Diag(Token, PP.getDiagnostics().getCustomDiagID(
                         clang::DiagnosticsEngine::Warning,
                         "rs_fp_imprecise is deprecated.  Assuming "
                         "rs_fp_relaxed instead."));
      Precision = "rs_fp_relaxed";
    }
    // Check if we have already encountered a precision pragma already.
    std::string PreviousPrecision = mContext->getPrecision();
    if (!PreviousPrecision.empty()) {
      // If the previous specified a different value, it's an error.
      if (PreviousPrecision != Precision) {
        PP.Diag(Token, PP.getDiagnostics().getCustomDiagID(
                           clang::DiagnosticsEngine::Error,
                           "Multiple float precisions specified.  Encountered "
                           "%0 previously."))
            << PreviousPrecision;
      }
      // Otherwise we ignore redundant entries.
      return;
    }

    mContext->addPragma(Precision, "");
    mContext->setPrecision(Precision);
  }
};

}  // namespace

void RSPragmaHandler::handleItemListPragma(clang::Preprocessor &PP,
                                           clang::Token &FirstToken) {
  clang::Token &PragmaToken = FirstToken;

  // Skip first token, like "export_var"
  PP.LexUnexpandedToken(PragmaToken);

  // Now, the current token must be clang::tok::lpara
  if (PragmaToken.isNot(clang::tok::l_paren))
    return;

  while (PragmaToken.isNot(clang::tok::eod)) {
    // Lex variable name
    PP.LexUnexpandedToken(PragmaToken);
    if (PragmaToken.is(clang::tok::identifier))
      this->handleItem(PP.getSpelling(PragmaToken));
    else
      break;

    slangAssert(PragmaToken.isNot(clang::tok::eod));

    PP.LexUnexpandedToken(PragmaToken);

    if (PragmaToken.isNot(clang::tok::comma))
      break;
  }
}

void RSPragmaHandler::handleNonParamPragma(clang::Preprocessor &PP,
                                           clang::Token &FirstToken) {
  clang::Token &PragmaToken = FirstToken;

  // Skip first token, like "export_var_all"
  PP.LexUnexpandedToken(PragmaToken);

  // Should be end immediately
  if (PragmaToken.isNot(clang::tok::eod))
    if (PragmaToken.isNot(clang::tok::r_paren)) {
      PP.Diag(PragmaToken,
              PP.getDiagnostics().getCustomDiagID(
                  clang::DiagnosticsEngine::Error,
                  "expected a ')'"));
      return;
    }
}

void RSPragmaHandler::handleOptionalStringLiteralParamPragma(
    clang::Preprocessor &PP, clang::Token &FirstToken) {
  clang::Token &PragmaToken = FirstToken;

  // Skip first token, like "set_reflect_license"
  PP.LexUnexpandedToken(PragmaToken);

  // Now, the current token must be clang::tok::lpara
  if (PragmaToken.isNot(clang::tok::l_paren))
    return;

  // If not ')', eat the following string literal as the license
  PP.LexUnexpandedToken(PragmaToken);
  if (PragmaToken.isNot(clang::tok::r_paren)) {
    // Eat the whole string literal
    clang::StringLiteralParser StringLiteral(PragmaToken, PP);
    if (StringLiteral.hadError) {
      // Diagnostics will be generated automatically
      return;
    } else {
      this->handleItem(std::string(StringLiteral.GetString()));
    }

    // The current token should be clang::tok::r_para
    PP.LexUnexpandedToken(PragmaToken);
    if (PragmaToken.isNot(clang::tok::r_paren)) {
      PP.Diag(PragmaToken,
              PP.getDiagnostics().getCustomDiagID(
                  clang::DiagnosticsEngine::Error,
                  "expected a ')'"));
      return;
    }
  } else {
    // If no argument, remove the license
    this->handleItem("");
  }
}

void RSPragmaHandler::handleIntegerParamPragma(
    clang::Preprocessor &PP, clang::Token &FirstToken) {
  clang::Token &PragmaToken = FirstToken;

  // Skip first token, like "version"
  PP.LexUnexpandedToken(PragmaToken);

  // Now, the current token must be clang::tok::lpara
  if (PragmaToken.isNot(clang::tok::l_paren)) {
    // If no argument, set the version to 0
    this->handleInt(PP, PragmaToken, 0);
    return;
  }
  PP.LexUnexpandedToken(PragmaToken);

  if (PragmaToken.is(clang::tok::numeric_constant)) {
    llvm::SmallString<128> SpellingBuffer;
    SpellingBuffer.resize(PragmaToken.getLength() + 1);
    llvm::StringRef TokSpelling = PP.getSpelling(PragmaToken, SpellingBuffer);
    clang::NumericLiteralParser NumericLiteral(TokSpelling,
        PragmaToken.getLocation(), PP);
    if (NumericLiteral.hadError) {
      // Diagnostics will be generated automatically
      return;
    } else {
      llvm::APInt Val(32, 0);
      NumericLiteral.GetIntegerValue(Val);
      this->handleInt(PP, PragmaToken, static_cast<int>(Val.getSExtValue()));
    }
    PP.LexUnexpandedToken(PragmaToken);
  } else {
    // If no argument, set the version to 0
    this->handleInt(PP, PragmaToken, 0);
  }

  if (PragmaToken.isNot(clang::tok::r_paren)) {
    PP.Diag(PragmaToken,
            PP.getDiagnostics().getCustomDiagID(
                clang::DiagnosticsEngine::Error,
                "expected a ')'"));
    return;
  }

  do {
    PP.LexUnexpandedToken(PragmaToken);
  } while (PragmaToken.isNot(clang::tok::eod));
}

void AddPragmaHandlers(clang::Preprocessor &PP, RSContext *RsContext) {
  // For #pragma rs export_type
  PP.AddPragmaHandler("rs",
                      new RSExportTypePragmaHandler("export_type", RsContext));

  // For #pragma rs java_package_name
  PP.AddPragmaHandler(
      "rs", new RSJavaPackageNamePragmaHandler("java_package_name", RsContext));

  // For #pragma rs reduce
  PP.AddPragmaHandler(
      "rs", new RSReducePragmaHandler(RSExportReduce::KeyReduce, RsContext));

  // For #pragma rs set_reflect_license
  PP.AddPragmaHandler(
      "rs", new RSReflectLicensePragmaHandler("set_reflect_license", RsContext));

  // For #pragma version
  PP.AddPragmaHandler(new RSVersionPragmaHandler("version", RsContext));

  // For #pragma rs_fp*
  PP.AddPragmaHandler(new RSPrecisionPragmaHandler("rs_fp_full", RsContext));
  PP.AddPragmaHandler(new RSPrecisionPragmaHandler("rs_fp_relaxed", RsContext));
  PP.AddPragmaHandler(new RSPrecisionPragmaHandler("rs_fp_imprecise", RsContext));
}


}  // namespace slang
