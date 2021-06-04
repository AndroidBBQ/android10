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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_REDUCE_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_REDUCE_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"

#include "slang_rs_context.h"
#include "slang_rs_exportable.h"
#include "slang_rs_export_type.h"

namespace clang {
  class FunctionDecl;
}  // namespace clang

namespace slang {

// Base class for reflecting control-side reduce
class RSExportReduce : public RSExportable {
 public:
  typedef llvm::SmallVectorImpl<const clang::ParmVarDecl*> InVec;
  typedef llvm::SmallVectorImpl<const RSExportType*> InTypeVec;

  typedef InVec::const_iterator InIter;
  typedef InTypeVec::const_iterator InTypeIter;

 private:
  // pragma location (for error reporting)
  clang::SourceLocation mLocation;

  // reduction kernel name
  std::string mNameReduce;

  // constituent function names
  std::string mNameInitializer;
  std::string mNameAccumulator;
  std::string mNameCombiner;
  std::string mNameOutConverter;
  std::string mNameHalter;

  // constituent function identity
  enum FnIdent {
    FN_IDENT_INITIALIZER,
    FN_IDENT_ACCUMULATOR,
    FN_IDENT_COMBINER,
    FN_IDENT_OUT_CONVERTER,
    FN_IDENT_HALTER
  };
  static const char *getKey(FnIdent Kind);

  // signature information for accumulator function
  unsigned int mAccumulatorSignatureMetadata;

  // size of accumulator data type (compType), in bytes
  unsigned int mAccumulatorTypeSize;

  // input information for accumulator function
  static const int kAccumulatorInsSmallSize = 4;
  llvm::SmallVector<const clang::ParmVarDecl*, kAccumulatorInsSmallSize> mAccumulatorIns;
  llvm::SmallVector<const RSExportType*, kAccumulatorInsSmallSize> mAccumulatorInTypes;

  // result information
  RSExportType *mResultType;

  RSExportReduce(RSContext *Context,
                 const clang::SourceLocation Location,
                 const llvm::StringRef &NameReduce,
                 const llvm::StringRef &NameInitializer,
                 const llvm::StringRef &NameAccumulator,
                 const llvm::StringRef &NameCombiner,
                 const llvm::StringRef &NameOutConverter,
                 const llvm::StringRef &NameHalter)
    : RSExportable(Context, RSExportable::EX_REDUCE, Location),
      mLocation(Location),
      mNameReduce(NameReduce),
      mNameInitializer(NameInitializer),
      mNameAccumulator(NameAccumulator),
      mNameCombiner(NameCombiner),
      mNameOutConverter(NameOutConverter),
      mNameHalter(NameHalter),
      mAccumulatorSignatureMetadata(0),
      mAccumulatorTypeSize(0),
      mResultType(nullptr) {
  }

  RSExportReduce(const RSExportReduce &) = delete;
  void operator=(const RSExportReduce &) = delete;

  struct StateOfAnalyzeTranslationUnit;

  static void notOk(StateOfAnalyzeTranslationUnit &S, FnIdent Kind);

  static void checkPointeeConstQualified(StateOfAnalyzeTranslationUnit &S,
                                         FnIdent Kind, const llvm::StringRef &Name,
                                         const clang::ParmVarDecl *Param, bool ExpectedQualification);

  static void checkVoidReturn(StateOfAnalyzeTranslationUnit &S, FnIdent Kind, clang::FunctionDecl *Fn);

  clang::FunctionDecl *lookupFunction(StateOfAnalyzeTranslationUnit &S,
                                      const char *Kind, const llvm::StringRef &Name);

  void analyzeInitializer(StateOfAnalyzeTranslationUnit &S);
  void analyzeAccumulator(StateOfAnalyzeTranslationUnit &S);
  void analyzeCombiner(StateOfAnalyzeTranslationUnit &S);
  void analyzeOutConverter(StateOfAnalyzeTranslationUnit &S);
  void analyzeHalter(StateOfAnalyzeTranslationUnit &S);
  void analyzeResultType(StateOfAnalyzeTranslationUnit &S);

 public:

  static const char KeyReduce[];
  static const char KeyInitializer[];
  static const char KeyAccumulator[];
  static const char KeyCombiner[];
  static const char KeyOutConverter[];
  static const char KeyHalter[];

  static RSExportReduce *Create(RSContext *Context,
                                const clang::SourceLocation Location,
                                const llvm::StringRef &NameReduce,
                                const llvm::StringRef &NameInitializer,
                                const llvm::StringRef &NameAccumulator,
                                const llvm::StringRef &NameCombiner,
                                const llvm::StringRef &NameOutConverter,
                                const llvm::StringRef &NameHalter);

  const clang::SourceLocation &getLocation() const { return mLocation; }

  const std::string &getNameReduce() const { return mNameReduce; }
  const std::string &getNameInitializer() const { return mNameInitializer; }
  const std::string &getNameAccumulator() const { return mNameAccumulator; }
  const std::string &getNameCombiner() const { return mNameCombiner; }
  const std::string &getNameOutConverter() const { return mNameOutConverter; }
  const std::string &getNameHalter() const { return mNameHalter; }

  unsigned int getAccumulatorSignatureMetadata() const { return mAccumulatorSignatureMetadata; }

  unsigned int getAccumulatorTypeSize() const { return mAccumulatorTypeSize; }

  const InVec &getAccumulatorIns() const { return mAccumulatorIns; }
  const InTypeVec &getAccumulatorInTypes() const { return mAccumulatorInTypes; }

  const RSExportType *getResultType() const { return mResultType; }

  // Does one of this reduction's constituent function names match Candidate?
  bool matchName(const llvm::StringRef &Candidate) const;

  bool analyzeTranslationUnit();
};  // RSExportReduce

}  // namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_RS_EXPORT_REDUCE_H_  NOLINT
