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

#include "slang_backend.h"

#include <string>
#include <vector>
#include <iostream>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/RecordLayout.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"

#include "clang/CodeGen/ModuleBuilder.h"

#include "clang/Frontend/CodeGenOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"

#include "llvm/ADT/Twine.h"
#include "llvm/ADT/StringExtras.h"

#include "llvm/Bitcode/ReaderWriter.h"

#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"

#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/TargetRegistry.h"

#include "llvm/MC/SubtargetFeature.h"

#include "slang_assert.h"
#include "slang.h"
#include "slang_bitcode_gen.h"
#include "slang_rs_context.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_func.h"
#include "slang_rs_export_reduce.h"
#include "slang_rs_export_type.h"
#include "slang_rs_export_var.h"
#include "slang_rs_metadata.h"

#include "rs_cc_options.h"

#include "StripUnkAttr/strip_unknown_attributes_pass.h"

namespace {
class VersionInfoPass : public llvm::ModulePass {
  const clang::CodeGenOptions &mCodeGenOpts;

  const char *getSlangLLVMVersion() const {
    if (mCodeGenOpts.getDebugInfo() != clang::codegenoptions::NoDebugInfo)
      return LLVM_VERSION_STRING;
    return nullptr;
  }

public:
  static char ID;
  VersionInfoPass(const clang::CodeGenOptions &codegenOpts)
      : ModulePass(ID), mCodeGenOpts(codegenOpts) {}
  virtual bool runOnModule(llvm::Module &M) override {
    const char *versionString = getSlangLLVMVersion();
    if (!versionString)
      return false;
    auto &ctx = M.getContext();
    auto md = M.getOrInsertNamedMetadata("slang.llvm.version");
    auto ver = llvm::MDString::get(ctx, versionString);
    md->addOperand(
        llvm::MDNode::get(ctx, llvm::ArrayRef<llvm::Metadata *>(ver)));
    return true;
  }
};

char VersionInfoPass::ID = 0;

llvm::ModulePass *createVersionInfoPass(const clang::CodeGenOptions &cgo) {
  return new VersionInfoPass(cgo);
}
}

namespace slang {

void Backend::CreateFunctionPasses() {
  if (!mPerFunctionPasses) {
    mPerFunctionPasses = new llvm::legacy::FunctionPassManager(mpModule);

    llvm::PassManagerBuilder PMBuilder;
    PMBuilder.OptLevel = mCodeGenOpts.OptimizationLevel;
    PMBuilder.populateFunctionPassManager(*mPerFunctionPasses);
  }
}

void Backend::CreateModulePasses() {
  if (!mPerModulePasses) {
    mPerModulePasses = new llvm::legacy::PassManager();

    llvm::PassManagerBuilder PMBuilder;
    PMBuilder.OptLevel = mCodeGenOpts.OptimizationLevel;
    PMBuilder.SizeLevel = mCodeGenOpts.OptimizeSize;
    PMBuilder.DisableUnitAtATime = 0; // TODO Pirama confirm if this is right

    if (mCodeGenOpts.UnrollLoops) {
      PMBuilder.DisableUnrollLoops = 0;
    } else {
      PMBuilder.DisableUnrollLoops = 1;
    }

    PMBuilder.populateModulePassManager(*mPerModulePasses);
    // Add a pass to strip off unknown/unsupported attributes.
    mPerModulePasses->add(createStripUnknownAttributesPass());
    if (!mContext->isCompatLib()) {
      // The version info pass is used to ensure that debugging
      // is matched between slang and bcc.
      mPerModulePasses->add(createVersionInfoPass(mCodeGenOpts));
    }
  }
}

bool Backend::CreateCodeGenPasses() {
  if ((mOT != Slang::OT_Assembly) && (mOT != Slang::OT_Object))
    return true;

  // Now we add passes for code emitting
  if (mCodeGenPasses) {
    return true;
  } else {
    mCodeGenPasses = new llvm::legacy::FunctionPassManager(mpModule);
  }

  // Create the TargetMachine for generating code.
  std::string Triple = mpModule->getTargetTriple();

  std::string Error;
  const llvm::Target* TargetInfo =
      llvm::TargetRegistry::lookupTarget(Triple, Error);
  if (TargetInfo == nullptr) {
    mDiagEngine.Report(clang::diag::err_fe_unable_to_create_target) << Error;
    return false;
  }

  // Target Machine Options
  llvm::TargetOptions Options;

  // Use soft-float ABI for ARM (which is the target used by Slang during code
  // generation).  Codegen still uses hardware FPU by default.  To use software
  // floating point, add 'soft-float' feature to FeaturesStr below.
  Options.FloatABIType = llvm::FloatABI::Soft;

  // BCC needs all unknown symbols resolved at compilation time. So we don't
  // need any relocation model.
  llvm::Reloc::Model RM = llvm::Reloc::Static;

  // This is set for the linker (specify how large of the virtual addresses we
  // can access for all unknown symbols.)
  llvm::CodeModel::Model CM;
  if (mpModule->getDataLayout().getPointerSize() == 4) {
    CM = llvm::CodeModel::Small;
  } else {
    // The target may have pointer size greater than 32 (e.g. x86_64
    // architecture) may need large data address model
    CM = llvm::CodeModel::Medium;
  }

  // Setup feature string
  std::string FeaturesStr;
  if (mTargetOpts.CPU.size() || mTargetOpts.Features.size()) {
    llvm::SubtargetFeatures Features;

    for (std::vector<std::string>::const_iterator
             I = mTargetOpts.Features.begin(), E = mTargetOpts.Features.end();
         I != E;
         I++)
      Features.AddFeature(*I);

    FeaturesStr = Features.getString();
  }

  llvm::TargetMachine *TM =
    TargetInfo->createTargetMachine(Triple, mTargetOpts.CPU, FeaturesStr,
                                    Options, RM, CM);

  // Register allocation policy:
  //  createFastRegisterAllocator: fast but bad quality
  //  createGreedyRegisterAllocator: not so fast but good quality
  llvm::RegisterRegAlloc::setDefault((mCodeGenOpts.OptimizationLevel == 0) ?
                                     llvm::createFastRegisterAllocator :
                                     llvm::createGreedyRegisterAllocator);

  llvm::CodeGenOpt::Level OptLevel = llvm::CodeGenOpt::Default;
  if (mCodeGenOpts.OptimizationLevel == 0) {
    OptLevel = llvm::CodeGenOpt::None;
  } else if (mCodeGenOpts.OptimizationLevel == 3) {
    OptLevel = llvm::CodeGenOpt::Aggressive;
  }

  llvm::TargetMachine::CodeGenFileType CGFT =
      llvm::TargetMachine::CGFT_AssemblyFile;
  if (mOT == Slang::OT_Object) {
    CGFT = llvm::TargetMachine::CGFT_ObjectFile;
  }
  if (TM->addPassesToEmitFile(*mCodeGenPasses, mBufferOutStream,
                              CGFT, OptLevel)) {
    mDiagEngine.Report(clang::diag::err_fe_unable_to_interface_with_target);
    return false;
  }

  return true;
}

Backend::Backend(RSContext *Context, clang::DiagnosticsEngine *DiagEngine,
                 const RSCCOptions &Opts,
                 const clang::HeaderSearchOptions &HeaderSearchOpts,
                 const clang::PreprocessorOptions &PreprocessorOpts,
                 const clang::CodeGenOptions &CodeGenOpts,
                 const clang::TargetOptions &TargetOpts, PragmaList *Pragmas,
                 llvm::raw_ostream *OS, Slang::OutputType OT,
                 clang::SourceManager &SourceMgr, bool AllowRSPrefix,
                 bool IsFilterscript)
    : ASTConsumer(), mTargetOpts(TargetOpts), mpModule(nullptr), mpOS(OS),
      mOT(OT), mGen(nullptr), mPerFunctionPasses(nullptr),
      mPerModulePasses(nullptr), mCodeGenPasses(nullptr),
      mBufferOutStream(*mpOS), mContext(Context),
      mSourceMgr(SourceMgr), mASTPrint(Opts.mASTPrint), mAllowRSPrefix(AllowRSPrefix),
      mIsFilterscript(IsFilterscript), mExportVarMetadata(nullptr),
      mExportFuncMetadata(nullptr), mExportForEachNameMetadata(nullptr),
      mExportForEachSignatureMetadata(nullptr),
      mExportReduceMetadata(nullptr),
      mExportTypeMetadata(nullptr), mRSObjectSlotsMetadata(nullptr),
      mRefCount(mContext->getASTContext()),
      mASTChecker(Context, Context->getTargetAPI(), IsFilterscript),
      mForEachHandler(Context),
      mLLVMContext(slang::getGlobalLLVMContext()), mDiagEngine(*DiagEngine),
      mCodeGenOpts(CodeGenOpts), mPragmas(Pragmas) {
  mGen = CreateLLVMCodeGen(mDiagEngine, "", HeaderSearchOpts, PreprocessorOpts,
      mCodeGenOpts, mLLVMContext);
}

void Backend::Initialize(clang::ASTContext &Ctx) {
  mGen->Initialize(Ctx);

  mpModule = mGen->GetModule();
}

void Backend::HandleTranslationUnit(clang::ASTContext &Ctx) {
  HandleTranslationUnitPre(Ctx);

  if (mASTPrint)
    Ctx.getTranslationUnitDecl()->dump();

  mGen->HandleTranslationUnit(Ctx);

  // Here, we complete a translation unit (whole translation unit is now in LLVM
  // IR). Now, interact with LLVM backend to generate actual machine code (asm
  // or machine code, whatever.)

  // Silently ignore if we weren't initialized for some reason.
  if (!mpModule)
    return;

  llvm::Module *M = mGen->ReleaseModule();
  if (!M) {
    // The module has been released by IR gen on failures, do not double free.
    mpModule = nullptr;
    return;
  }

  slangAssert(mpModule == M &&
              "Unexpected module change during LLVM IR generation");

  // Insert #pragma information into metadata section of module
  if (!mPragmas->empty()) {
    llvm::NamedMDNode *PragmaMetadata =
        mpModule->getOrInsertNamedMetadata(Slang::PragmaMetadataName);
    for (PragmaList::const_iterator I = mPragmas->begin(), E = mPragmas->end();
         I != E;
         I++) {
      llvm::SmallVector<llvm::Metadata*, 2> Pragma;
      // Name goes first
      Pragma.push_back(llvm::MDString::get(mLLVMContext, I->first));
      // And then value
      Pragma.push_back(llvm::MDString::get(mLLVMContext, I->second));

      // Create MDNode and insert into PragmaMetadata
      PragmaMetadata->addOperand(
          llvm::MDNode::get(mLLVMContext, Pragma));
    }
  }

  HandleTranslationUnitPost(mpModule);

  // Create passes for optimization and code emission

  // Create and run per-function passes
  CreateFunctionPasses();
  if (mPerFunctionPasses) {
    mPerFunctionPasses->doInitialization();

    for (llvm::Module::iterator I = mpModule->begin(), E = mpModule->end();
         I != E;
         I++)
      if (!I->isDeclaration())
        mPerFunctionPasses->run(*I);

    mPerFunctionPasses->doFinalization();
  }

  // Create and run module passes
  CreateModulePasses();
  if (mPerModulePasses)
    mPerModulePasses->run(*mpModule);

  switch (mOT) {
    case Slang::OT_Assembly:
    case Slang::OT_Object: {
      if (!CreateCodeGenPasses())
        return;

      mCodeGenPasses->doInitialization();

      for (llvm::Module::iterator I = mpModule->begin(), E = mpModule->end();
          I != E;
          I++)
        if (!I->isDeclaration())
          mCodeGenPasses->run(*I);

      mCodeGenPasses->doFinalization();
      break;
    }
    case Slang::OT_LLVMAssembly: {
      llvm::legacy::PassManager *LLEmitPM = new llvm::legacy::PassManager();
      LLEmitPM->add(llvm::createPrintModulePass(mBufferOutStream));
      LLEmitPM->run(*mpModule);
      break;
    }
    case Slang::OT_Bitcode: {
      writeBitcode(mBufferOutStream, *mpModule, getTargetAPI(),
                   mCodeGenOpts.OptimizationLevel, mCodeGenOpts.getDebugInfo());
      break;
    }
    case Slang::OT_Nothing: {
      return;
    }
    default: {
      slangAssert(false && "Unknown output type");
    }
  }
}

// Insert explicit padding fields into struct to follow the current layout.
//
// A similar algorithm is present in PadHelperFunctionStruct().
void Backend::PadStruct(clang::RecordDecl* RD) {
  // Example of padding:
  //
  //   // ORIGINAL CODE                   // TRANSFORMED CODE
  //   struct foo {                       struct foo {
  //     int a;                             int a;
  //     // 4 bytes of padding              char <RS_PADDING_FIELD_NAME>[4];
  //     long b;                            long b;
  //     int c;                             int c;
  //     // 4 bytes of (tail) padding       char <RS_PADDING_FIELD_NAME>[4];
  //   };                                 };

  // We collect all of RD's fields in a vector FieldsInfo.  We
  // represent tail padding as an entry in the FieldsInfo vector with a
  // null FieldDecl.
  typedef std::pair<size_t, clang::FieldDecl*> FieldInfoType;  // (pre-field padding bytes, field)
  std::vector<FieldInfoType> FieldsInfo;

  // RenderScript is C99-based, so we only expect to see fields.  We
  // could iterate over fields, but instead let's iterate over
  // everything, to verify that there are only fields.
  for (clang::Decl* D : RD->decls()) {
    clang::FieldDecl* FD = clang::dyn_cast<clang::FieldDecl>(D);
    slangAssert(FD && "found a non field declaration within a struct");
    FieldsInfo.push_back(std::make_pair(size_t(0), FD));
  }

  clang::ASTContext& ASTC = mContext->getASTContext();

  // ASTContext caches record layout.  We may transform the record in a way
  // that would render this cached information incorrect.  clang does
  // not provide any way to invalidate this cached information.  We
  // take the following approach:
  //
  // 1. ASSUME that record layout has not yet been computed for RD.
  //
  // 2. Create a temporary clone of RD, and compute its layout.
  //    ASSUME that we know how to clone RD in a way that copies all the
  //    properties that are relevant to its layout.
  //
  // 3. Use the layout information from the temporary clone to
  //    transform RD.
  //
  // NOTE: ASTContext also caches TypeInfo (see
  //       ASTContext::getTypeInfo()).  ASSUME that inserting padding
  //       fields doesn't change the type in any way that affects
  //       TypeInfo.
  //
  // NOTE: A RecordType knows its associated RecordDecl -- so even
  //       while we're manipulating RD, the associated RecordType
  //       still recognizes RD as its RecordDecl.  ASSUME that we
  //       don't do anything during our manipulation that would cause
  //       the RecordType to be followed to RD while RD is in a
  //       partially transformed state.

  // The assumptions above may be brittle, and if they are incorrect,
  // we may get mysterious failures.

  // create a temporary clone
  clang::RecordDecl* RDForLayout =
      clang::RecordDecl::Create(ASTC, clang::TTK_Struct, RD->getDeclContext(),
                                clang::SourceLocation(), clang::SourceLocation(),
                                nullptr /* IdentifierInfo */);
  RDForLayout->startDefinition();
  RDForLayout->setTypeForDecl(RD->getTypeForDecl());
  if (RD->hasAttrs())
    RDForLayout->setAttrs(RD->getAttrs());
  RDForLayout->completeDefinition();

  // move all fields from RD to RDForLayout
  for (const auto &info : FieldsInfo) {
    RD->removeDecl(info.second);
    info.second->setLexicalDeclContext(RDForLayout);
    RDForLayout->addDecl(info.second);
  }

  const clang::ASTRecordLayout& RL = ASTC.getASTRecordLayout(RDForLayout);

  // An exportable type cannot contain a bitfield.  However, it's
  // possible that this current type might have a bitfield and yet
  // share a common initial sequence with an exportable type, so even
  // if the current type has a bitfield, the current type still
  // needs to have explicit padding inserted (in case the two types
  // under discussion are members of a union).  We don't need to
  // insert any padding after the bitfield, however, because that
  // would be beyond the common initial sequence.
  bool foundBitField = false;

  // Is there any padding in this struct?
  bool foundPadding = false;

  unsigned fieldNo = 0;
  uint64_t fieldPrePaddingOffset = 0;  // byte offset of pre-field padding within struct
  for (auto &info : FieldsInfo) {
    const clang::FieldDecl* FD = info.second;

    if ((foundBitField = FD->isBitField()))
      break;

    const uint64_t fieldOffset = RL.getFieldOffset(fieldNo) >> 3;
    const size_t prePadding = fieldOffset - fieldPrePaddingOffset;
    foundPadding |= (prePadding != 0);
    info.first = prePadding;

    // get ready for the next field
    //
    //   assumes that getTypeSize() is the storage size of the Type -- for example,
    //   that it includes a struct's tail padding (if any)
    //
    fieldPrePaddingOffset = fieldOffset + (ASTC.getTypeSize(FD->getType()) >> 3);
    ++fieldNo;
  }

  if (!foundBitField) {
    // In order to ensure that the front end (including reflected
    // code) and back end agree on struct size (not just field
    // offsets) we may need to add explicit tail padding, just as we'e
    // added explicit padding between fields.
    slangAssert(RL.getSize().getQuantity() >= fieldPrePaddingOffset);
    if (const size_t tailPadding = RL.getSize().getQuantity() - fieldPrePaddingOffset) {
      foundPadding = true;
      FieldsInfo.push_back(std::make_pair(tailPadding, nullptr));
    }
  }

  if (false /* change to "true" for extra debugging output */) {
   if (foundPadding) {
     std::cout << "PadStruct(" << RD->getNameAsString() << "):" << std::endl;
     for (const auto &info : FieldsInfo)
       std::cout << "  " << info.first << ", " << (info.second ? info.second->getNameAsString() : "<tail>") << std::endl;
   }
  }

  if (foundPadding && Slang::IsLocInRSHeaderFile(RD->getLocation(), mSourceMgr)) {
    mContext->ReportError(RD->getLocation(), "system structure contains padding: '%0'")
        << RD->getName();
  }

  // now move fields from RDForLayout to RD, and add any necessary
  // padding fields
  const clang::QualType byteType = ASTC.getIntTypeForBitwidth(8, false /* not signed */);
  clang::IdentifierInfo* const paddingIdentifierInfo = &ASTC.Idents.get(RS_PADDING_FIELD_NAME);
  for (const auto &info : FieldsInfo) {
    if (info.first != 0) {
      // Create a padding field: "char <RS_PADDING_FIELD_NAME>[<info.first>];"

      // TODO: Do we need to do anything else to keep this field from being shown in debugger?
      //       There's no source location, and the field is marked as implicit.
      const clang::QualType paddingType =
          ASTC.getConstantArrayType(byteType,
                                    llvm::APInt(sizeof(info.first) << 3, info.first),
                                    clang::ArrayType::Normal, 0 /* IndexTypeQuals */);
      clang::FieldDecl* const FD =
          clang::FieldDecl::Create(ASTC, RD, clang::SourceLocation(), clang::SourceLocation(),
                                   paddingIdentifierInfo,
                                   paddingType,
                                   nullptr,  // TypeSourceInfo*
                                   nullptr,  // BW (bitwidth)
                                   false,    // Mutable = false
                                   clang::ICIS_NoInit);
      FD->setImplicit(true);
      RD->addDecl(FD);
    }
    if (info.second != nullptr) {
      RDForLayout->removeDecl(info.second);
      info.second->setLexicalDeclContext(RD);
      RD->addDecl(info.second);
    }
  }

  // There does not appear to be any safe way to delete a RecordDecl
  // -- for example, there is no RecordDecl destructor to invalidate
  // cached record layout, and if we were to get unlucky, some future
  // RecordDecl could be allocated in the same place as a deleted
  // RDForLayout and "inherit" the cached record layout from
  // RDForLayout.
}

void Backend::HandleTagDeclDefinition(clang::TagDecl *D) {
  // we want to insert explicit padding fields into structs per http://b/29154200 and http://b/28070272
  switch (D->getTagKind()) {
    case clang::TTK_Struct:
      PadStruct(llvm::cast<clang::RecordDecl>(D));
      break;

    case clang::TTK_Union:
      // cannot be part of an exported type
      break;

    case clang::TTK_Enum:
      // a scalar
      break;

    case clang::TTK_Class:
    case clang::TTK_Interface:
    default:
      slangAssert(false && "Unexpected TagTypeKind");
      break;
  }
  mGen->HandleTagDeclDefinition(D);
}

void Backend::CompleteTentativeDefinition(clang::VarDecl *D) {
  mGen->CompleteTentativeDefinition(D);
}

Backend::~Backend() {
  delete mpModule;
  delete mGen;
  delete mPerFunctionPasses;
  delete mPerModulePasses;
  delete mCodeGenPasses;
}

// 1) Add zero initialization of local RS object types
void Backend::AnnotateFunction(clang::FunctionDecl *FD) {
  if (FD &&
      FD->hasBody() &&
      !FD->isImplicit() &&
      !Slang::IsLocInRSHeaderFile(FD->getLocation(), mSourceMgr)) {
    mRefCount.Init();
    mRefCount.SetDeclContext(FD);
    mRefCount.HandleParamsAndLocals(FD);
  }
}

bool Backend::HandleTopLevelDecl(clang::DeclGroupRef D) {
  // Find and remember the types for rs_allocation and rs_script_call_t so
  // they can be used later for translating rsForEach() calls.
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end();
       (mContext->getAllocationType().isNull() ||
        mContext->getScriptCallType().isNull()) &&
       I != E; I++) {
    if (clang::TypeDecl* TD = llvm::dyn_cast<clang::TypeDecl>(*I)) {
      clang::StringRef TypeName = TD->getName();
      if (TypeName.equals("rs_allocation")) {
        mContext->setAllocationType(TD);
      } else if (TypeName.equals("rs_script_call_t")) {
        mContext->setScriptCallType(TD);
      }
    }
  }

  // Disallow user-defined functions with prefix "rs"
  if (!mAllowRSPrefix) {
    // Iterate all function declarations in the program.
    for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end();
         I != E; I++) {
      clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(*I);
      if (FD == nullptr)
        continue;
      if (!FD->getName().startswith("rs"))  // Check prefix
        continue;
      if (!Slang::IsLocInRSHeaderFile(FD->getLocation(), mSourceMgr))
        mContext->ReportError(FD->getLocation(),
                              "invalid function name prefix, "
                              "\"rs\" is reserved: '%0'")
            << FD->getName();
    }
  }

  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; I++) {
    clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(*I);
    if (FD) {
      // Handle forward reference from pragma (see
      // RSReducePragmaHandler::HandlePragma for backward reference).
      mContext->markUsedByReducePragma(FD, RSContext::CheckNameYes);
      if (FD->isGlobal()) {
        // Check that we don't have any array parameters being misinterpreted as
        // kernel pointers due to the C type system's array to pointer decay.
        size_t numParams = FD->getNumParams();
        for (size_t i = 0; i < numParams; i++) {
          const clang::ParmVarDecl *PVD = FD->getParamDecl(i);
          clang::QualType QT = PVD->getOriginalType();
          if (QT->isArrayType()) {
            mContext->ReportError(
                PVD->getTypeSpecStartLoc(),
                "exported function parameters may not have array type: %0")
                << QT;
          }
        }
        AnnotateFunction(FD);
      }
    }

    if (getTargetAPI() >= SLANG_FEATURE_SINGLE_SOURCE_API) {
      if (FD && FD->hasBody() && !FD->isImplicit() &&
          !Slang::IsLocInRSHeaderFile(FD->getLocation(), mSourceMgr)) {
        if (FD->hasAttr<clang::RenderScriptKernelAttr>()) {
          // Log functions with attribute "kernel" by their names, and assign
          // them slot numbers. Any other function cannot be used in a
          // rsForEach() or rsForEachWithOptions() call, including old-style
          // kernel functions which are defined without the "kernel" attribute.
          mContext->addForEach(FD);
        }
        // Look for any kernel launch calls and translate them into using the
        // internal API.
        // Report a compiler error on kernel launches inside a kernel.
        mForEachHandler.handleForEachCalls(FD, getTargetAPI());
      }
    }
  }

  return mGen->HandleTopLevelDecl(D);
}

void Backend::HandleTranslationUnitPre(clang::ASTContext &C) {
  clang::TranslationUnitDecl *TUDecl = C.getTranslationUnitDecl();

  if (!mContext->processReducePragmas(this))
    return;

  // If we have an invalid RS/FS AST, don't check further.
  if (!mASTChecker.Validate()) {
    return;
  }

  if (mIsFilterscript) {
    mContext->addPragma("rs_fp_relaxed", "");
  }

  int version = mContext->getVersion();
  if (version == 0) {
    // Not setting a version is an error
    mDiagEngine.Report(
        mSourceMgr.getLocForEndOfFile(mSourceMgr.getMainFileID()),
        mDiagEngine.getCustomDiagID(
            clang::DiagnosticsEngine::Error,
            "missing pragma for version in source file"));
  } else {
    slangAssert(version == 1);
  }

  if (mContext->getReflectJavaPackageName().empty()) {
    mDiagEngine.Report(
        mSourceMgr.getLocForEndOfFile(mSourceMgr.getMainFileID()),
        mDiagEngine.getCustomDiagID(clang::DiagnosticsEngine::Error,
                                    "missing \"#pragma rs "
                                    "java_package_name(com.foo.bar)\" "
                                    "in source file"));
    return;
  }

  // Create a static global destructor if necessary (to handle RS object
  // runtime cleanup).
  clang::FunctionDecl *FD = mRefCount.CreateStaticGlobalDtor();
  if (FD) {
    HandleTopLevelDecl(clang::DeclGroupRef(FD));
  }

  // Process any static function declarations
  for (clang::DeclContext::decl_iterator I = TUDecl->decls_begin(),
          E = TUDecl->decls_end(); I != E; I++) {
    if ((I->getKind() >= clang::Decl::firstFunction) &&
        (I->getKind() <= clang::Decl::lastFunction)) {
      clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(*I);
      if (FD && !FD->isGlobal()) {
        AnnotateFunction(FD);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void Backend::dumpExportVarInfo(llvm::Module *M) {
  int slotCount = 0;
  if (mExportVarMetadata == nullptr)
    mExportVarMetadata = M->getOrInsertNamedMetadata(RS_EXPORT_VAR_MN);

  llvm::SmallVector<llvm::Metadata *, 2> ExportVarInfo;

  // We emit slot information (#rs_object_slots) for any reference counted
  // RS type or pointer (which can also be bound).

  for (RSContext::const_export_var_iterator I = mContext->export_vars_begin(),
          E = mContext->export_vars_end();
       I != E;
       I++) {
    const RSExportVar *EV = *I;
    const RSExportType *ET = EV->getType();
    bool countsAsRSObject = false;

    // Variable name
    ExportVarInfo.push_back(
        llvm::MDString::get(mLLVMContext, EV->getName().c_str()));

    // Type name
    switch (ET->getClass()) {
      case RSExportType::ExportClassPrimitive: {
        const RSExportPrimitiveType *PT =
            static_cast<const RSExportPrimitiveType*>(ET);
        ExportVarInfo.push_back(
            llvm::MDString::get(
              mLLVMContext, llvm::utostr(PT->getType())));
        if (PT->isRSObjectType()) {
          countsAsRSObject = true;
        }
        break;
      }
      case RSExportType::ExportClassPointer: {
        ExportVarInfo.push_back(
            llvm::MDString::get(
              mLLVMContext, ("*" + static_cast<const RSExportPointerType*>(ET)
                ->getPointeeType()->getName()).c_str()));
        break;
      }
      case RSExportType::ExportClassMatrix: {
        ExportVarInfo.push_back(
            llvm::MDString::get(
              mLLVMContext, llvm::utostr(
                  /* TODO Strange value.  This pushes just a number, quite
                   * different than the other cases.  What is this used for?
                   * These are the metadata values that some partner drivers
                   * want to reference (for TBAA, etc.). We may want to look
                   * at whether these provide any reasonable value (or have
                   * distinct enough values to actually depend on).
                   */
                DataTypeRSMatrix2x2 +
                static_cast<const RSExportMatrixType*>(ET)->getDim() - 2)));
        break;
      }
      case RSExportType::ExportClassVector:
      case RSExportType::ExportClassConstantArray:
      case RSExportType::ExportClassRecord: {
        ExportVarInfo.push_back(
            llvm::MDString::get(mLLVMContext,
              EV->getType()->getName().c_str()));
        break;
      }
    }

    mExportVarMetadata->addOperand(
        llvm::MDNode::get(mLLVMContext, ExportVarInfo));
    ExportVarInfo.clear();

    if (mRSObjectSlotsMetadata == nullptr) {
      mRSObjectSlotsMetadata =
          M->getOrInsertNamedMetadata(RS_OBJECT_SLOTS_MN);
    }

    if (countsAsRSObject) {
      mRSObjectSlotsMetadata->addOperand(llvm::MDNode::get(mLLVMContext,
          llvm::MDString::get(mLLVMContext, llvm::utostr(slotCount))));
    }

    slotCount++;
  }
}

// A similar algorithm is present in Backend::PadStruct().
static void PadHelperFunctionStruct(llvm::Module *M,
                                    llvm::StructType **paddedStructType,
                                    std::vector<unsigned> *origFieldNumToPaddedFieldNum,
                                    llvm::StructType *origStructType) {
  slangAssert(origFieldNumToPaddedFieldNum->empty());
  origFieldNumToPaddedFieldNum->resize(2 * origStructType->getNumElements());

  llvm::LLVMContext &llvmContext = M->getContext();

  const llvm::DataLayout *DL = &M->getDataLayout();
  const llvm::StructLayout *SL = DL->getStructLayout(origStructType);

  // Field types -- including any padding fields we need to insert.
  std::vector<llvm::Type *> paddedFieldTypes;
  paddedFieldTypes.reserve(2 * origStructType->getNumElements());

  // Is there any padding in this struct?
  bool foundPadding = false;

  llvm::Type *const byteType = llvm::Type::getInt8Ty(llvmContext);
  unsigned origFieldNum = 0, paddedFieldNum = 0;
  uint64_t fieldPrePaddingOffset = 0;  // byte offset of pre-field padding within struct
  for (llvm::Type *fieldType : origStructType->elements()) {
    const uint64_t fieldOffset = SL->getElementOffset(origFieldNum);
    const size_t prePadding = fieldOffset - fieldPrePaddingOffset;
    if (prePadding != 0) {
      foundPadding = true;
      paddedFieldTypes.push_back(llvm::ArrayType::get(byteType, prePadding));
      ++paddedFieldNum;
    }
    paddedFieldTypes.push_back(fieldType);
    (*origFieldNumToPaddedFieldNum)[origFieldNum] = paddedFieldNum;

    // get ready for the next field
    fieldPrePaddingOffset = fieldOffset + DL->getTypeAllocSize(fieldType);
    ++origFieldNum;
    ++paddedFieldNum;
  }

  // In order to ensure that the front end (including reflected code)
  // and back end agree on struct size (not just field offsets) we may
  // need to add explicit tail padding, just as we'e added explicit
  // padding between fields.
  slangAssert(SL->getSizeInBytes() >= fieldPrePaddingOffset);
  if (const size_t tailPadding = SL->getSizeInBytes() - fieldPrePaddingOffset) {
    foundPadding = true;
    paddedFieldTypes.push_back(llvm::ArrayType::get(byteType, tailPadding));
  }

  *paddedStructType = (foundPadding
                       ? llvm::StructType::get(llvmContext, paddedFieldTypes)
                       : origStructType);
}

void Backend::dumpExportFunctionInfo(llvm::Module *M) {
  if (mExportFuncMetadata == nullptr)
    mExportFuncMetadata =
        M->getOrInsertNamedMetadata(RS_EXPORT_FUNC_MN);

  llvm::SmallVector<llvm::Metadata *, 1> ExportFuncInfo;

  for (RSContext::const_export_func_iterator
          I = mContext->export_funcs_begin(),
          E = mContext->export_funcs_end();
       I != E;
       I++) {
    const RSExportFunc *EF = *I;

    // Function name
    if (!EF->hasParam()) {
      ExportFuncInfo.push_back(llvm::MDString::get(mLLVMContext,
                                                   EF->getName().c_str()));
    } else {
      llvm::Function *F = M->getFunction(EF->getName());
      llvm::Function *HelperFunction;
      const std::string HelperFunctionName(".helper_" + EF->getName());

      slangAssert(F && "Function marked as exported disappeared in Bitcode");

      // Create helper function
      {
        llvm::StructType *OrigHelperFunctionParameterTy = nullptr;
        llvm::StructType *PaddedHelperFunctionParameterTy = nullptr;

        std::vector<unsigned> OrigFieldNumToPaddedFieldNum;
        std::vector<bool> isPassedViaPtr;

        if (!F->getArgumentList().empty()) {
          std::vector<llvm::Type*> HelperFunctionParameterTys;
          for (llvm::Function::arg_iterator AI = F->arg_begin(),
                                            AE = F->arg_end();
               AI != AE; AI++) {
            if (AI->getType()->isPointerTy() &&
                AI->getType()->getPointerElementType()->isStructTy()) {
              HelperFunctionParameterTys.push_back(
                  AI->getType()->getPointerElementType());
              isPassedViaPtr.push_back(true);
            } else {
              // on 64-bit architecture(s), a vector type could be too big
              // to be passed in a register and instead passed
              // via a pointer to a temporary copy
              llvm::Type *Ty = AI->getType();
              bool viaPtr = false;
              if (Ty->isPointerTy() && Ty->getPointerElementType()) {
                Ty = Ty->getPointerElementType();
                viaPtr = true;
              }
              HelperFunctionParameterTys.push_back(Ty);
              isPassedViaPtr.push_back(viaPtr);
            }
          }
          OrigHelperFunctionParameterTy =
              llvm::StructType::get(mLLVMContext, HelperFunctionParameterTys);
          PadHelperFunctionStruct(M,
                                  &PaddedHelperFunctionParameterTy, &OrigFieldNumToPaddedFieldNum,
                                  OrigHelperFunctionParameterTy);
        }

        if (!EF->checkParameterPacketType(OrigHelperFunctionParameterTy)) {
          fprintf(stderr, "Failed to export function %s: parameter type "
                          "mismatch during creation of helper function.\n",
                  EF->getName().c_str());

          const RSExportRecordType *Expected = EF->getParamPacketType();
          if (Expected) {
            fprintf(stderr, "Expected:\n");
            Expected->getLLVMType()->dump();
          }
          if (OrigHelperFunctionParameterTy) {
            fprintf(stderr, "Got:\n");
            OrigHelperFunctionParameterTy->dump();
          }
          abort();
        }

        std::vector<llvm::Type*> Params;
        if (PaddedHelperFunctionParameterTy) {
          llvm::PointerType *HelperFunctionParameterTyP =
              llvm::PointerType::getUnqual(PaddedHelperFunctionParameterTy);
          Params.push_back(HelperFunctionParameterTyP);
        }

        llvm::FunctionType * HelperFunctionType =
            llvm::FunctionType::get(F->getReturnType(),
                                    Params,
                                    /* IsVarArgs = */false);

        HelperFunction =
            llvm::Function::Create(HelperFunctionType,
                                   llvm::GlobalValue::ExternalLinkage,
                                   HelperFunctionName,
                                   M);

        HelperFunction->addFnAttr(llvm::Attribute::NoInline);
        HelperFunction->setCallingConv(F->getCallingConv());

        // Create helper function body
        {
          llvm::Argument *HelperFunctionParameter =
              &(*HelperFunction->arg_begin());
          llvm::BasicBlock *BB =
              llvm::BasicBlock::Create(mLLVMContext, "entry", HelperFunction);
          llvm::IRBuilder<> *IB = new llvm::IRBuilder<>(BB);
          llvm::SmallVector<llvm::Value*, 6> Params;
          llvm::Value *Idx[2];

          Idx[0] =
              llvm::ConstantInt::get(llvm::Type::getInt32Ty(mLLVMContext), 0);

          // getelementptr and load instruction for all elements in
          // parameter .p
          for (size_t origFieldNum = 0; origFieldNum < EF->getNumParameters(); origFieldNum++) {
            // getelementptr
            Idx[1] = llvm::ConstantInt::get(
              llvm::Type::getInt32Ty(mLLVMContext), OrigFieldNumToPaddedFieldNum[origFieldNum]);

            llvm::Value *Ptr = NULL;

            Ptr = IB->CreateInBoundsGEP(HelperFunctionParameter, Idx);

            // Load is only required for non-struct ptrs
            if (isPassedViaPtr[origFieldNum]) {
                Params.push_back(Ptr);
            } else {
                llvm::Value *V = IB->CreateLoad(Ptr);
                Params.push_back(V);
            }
          }

          // Call and pass the all elements as parameter to F
          llvm::CallInst *CI = IB->CreateCall(F, Params);

          CI->setCallingConv(F->getCallingConv());

          if (F->getReturnType() == llvm::Type::getVoidTy(mLLVMContext)) {
            IB->CreateRetVoid();
          } else {
            IB->CreateRet(CI);
          }

          delete IB;
        }
      }

      ExportFuncInfo.push_back(
          llvm::MDString::get(mLLVMContext, HelperFunctionName.c_str()));
    }

    mExportFuncMetadata->addOperand(
        llvm::MDNode::get(mLLVMContext, ExportFuncInfo));
    ExportFuncInfo.clear();
  }
}

void Backend::dumpExportForEachInfo(llvm::Module *M) {
  if (mExportForEachNameMetadata == nullptr) {
    mExportForEachNameMetadata =
        M->getOrInsertNamedMetadata(RS_EXPORT_FOREACH_NAME_MN);
  }
  if (mExportForEachSignatureMetadata == nullptr) {
    mExportForEachSignatureMetadata =
        M->getOrInsertNamedMetadata(RS_EXPORT_FOREACH_MN);
  }

  llvm::SmallVector<llvm::Metadata *, 1> ExportForEachName;
  llvm::SmallVector<llvm::Metadata *, 1> ExportForEachInfo;

  for (RSContext::const_export_foreach_iterator
          I = mContext->export_foreach_begin(),
          E = mContext->export_foreach_end();
       I != E;
       I++) {
    const RSExportForEach *EFE = *I;

    ExportForEachName.push_back(
        llvm::MDString::get(mLLVMContext, EFE->getName().c_str()));

    mExportForEachNameMetadata->addOperand(
        llvm::MDNode::get(mLLVMContext, ExportForEachName));
    ExportForEachName.clear();

    ExportForEachInfo.push_back(
        llvm::MDString::get(mLLVMContext,
                            llvm::utostr(EFE->getSignatureMetadata())));

    mExportForEachSignatureMetadata->addOperand(
        llvm::MDNode::get(mLLVMContext, ExportForEachInfo));
    ExportForEachInfo.clear();
  }
}

void Backend::dumpExportReduceInfo(llvm::Module *M) {
  if (!mExportReduceMetadata) {
    mExportReduceMetadata =
      M->getOrInsertNamedMetadata(RS_EXPORT_REDUCE_MN);
  }

  llvm::SmallVector<llvm::Metadata *, 6> ExportReduceInfo;
  // Add operand to ExportReduceInfo, padding out missing operands with
  // nullptr.
  auto addOperand = [&ExportReduceInfo](uint32_t Idx, llvm::Metadata *N) {
    while (Idx > ExportReduceInfo.size())
      ExportReduceInfo.push_back(nullptr);
    ExportReduceInfo.push_back(N);
  };
  // Add string operand to ExportReduceInfo, padding out missing operands
  // with nullptr.
  // If string is empty, then do not add it unless Always is true.
  auto addString = [&addOperand, this](uint32_t Idx, const std::string &S,
                                       bool Always = true) {
    if (Always || !S.empty())
      addOperand(Idx, llvm::MDString::get(mLLVMContext, S));
  };

  // Add the description of the reduction kernels to the metadata node.
  for (auto I = mContext->export_reduce_begin(),
            E = mContext->export_reduce_end();
       I != E; ++I) {
    ExportReduceInfo.clear();

    int Idx = 0;

    addString(Idx++, (*I)->getNameReduce());

    addOperand(Idx++, llvm::MDString::get(mLLVMContext, llvm::utostr((*I)->getAccumulatorTypeSize())));

    llvm::SmallVector<llvm::Metadata *, 2> Accumulator;
    Accumulator.push_back(
      llvm::MDString::get(mLLVMContext, (*I)->getNameAccumulator()));
    Accumulator.push_back(llvm::MDString::get(
      mLLVMContext,
      llvm::utostr((*I)->getAccumulatorSignatureMetadata())));
    addOperand(Idx++, llvm::MDTuple::get(mLLVMContext, Accumulator));

    addString(Idx++, (*I)->getNameInitializer(), false);
    addString(Idx++, (*I)->getNameCombiner(), false);
    addString(Idx++, (*I)->getNameOutConverter(), false);
    addString(Idx++, (*I)->getNameHalter(), false);

    mExportReduceMetadata->addOperand(
      llvm::MDTuple::get(mLLVMContext, ExportReduceInfo));
  }
}

void Backend::dumpExportTypeInfo(llvm::Module *M) {
  llvm::SmallVector<llvm::Metadata *, 1> ExportTypeInfo;

  for (RSContext::const_export_type_iterator
          I = mContext->export_types_begin(),
          E = mContext->export_types_end();
       I != E;
       I++) {
    // First, dump type name list to export
    const RSExportType *ET = I->getValue();

    ExportTypeInfo.clear();
    // Type name
    ExportTypeInfo.push_back(
        llvm::MDString::get(mLLVMContext, ET->getName().c_str()));

    if (ET->getClass() == RSExportType::ExportClassRecord) {
      const RSExportRecordType *ERT =
          static_cast<const RSExportRecordType*>(ET);

      if (mExportTypeMetadata == nullptr)
        mExportTypeMetadata =
            M->getOrInsertNamedMetadata(RS_EXPORT_TYPE_MN);

      mExportTypeMetadata->addOperand(
          llvm::MDNode::get(mLLVMContext, ExportTypeInfo));

      // Now, export struct field information to %[struct name]
      std::string StructInfoMetadataName("%");
      StructInfoMetadataName.append(ET->getName());
      llvm::NamedMDNode *StructInfoMetadata =
          M->getOrInsertNamedMetadata(StructInfoMetadataName);
      llvm::SmallVector<llvm::Metadata *, 3> FieldInfo;

      slangAssert(StructInfoMetadata->getNumOperands() == 0 &&
                  "Metadata with same name was created before");
      for (RSExportRecordType::const_field_iterator FI = ERT->fields_begin(),
              FE = ERT->fields_end();
           FI != FE;
           FI++) {
        const RSExportRecordType::Field *F = *FI;

        // 1. field name
        FieldInfo.push_back(llvm::MDString::get(mLLVMContext,
                                                F->getName().c_str()));

        // 2. field type name
        FieldInfo.push_back(
            llvm::MDString::get(mLLVMContext,
                                F->getType()->getName().c_str()));

        StructInfoMetadata->addOperand(
            llvm::MDNode::get(mLLVMContext, FieldInfo));
        FieldInfo.clear();
      }
    }   // ET->getClass() == RSExportType::ExportClassRecord
  }
}

void Backend::HandleTranslationUnitPost(llvm::Module *M) {

  if (!mContext->is64Bit()) {
    M->setDataLayout("e-p:32:32-i64:64-v128:64:128-n32-S64");
  }

  if (!mContext->processExports())
    return;

  if (mContext->hasExportVar())
    dumpExportVarInfo(M);

  if (mContext->hasExportFunc())
    dumpExportFunctionInfo(M);

  if (mContext->hasExportForEach())
    dumpExportForEachInfo(M);

  if (mContext->hasExportReduce())
    dumpExportReduceInfo(M);

  if (mContext->hasExportType())
    dumpExportTypeInfo(M);
}

}  // namespace slang
