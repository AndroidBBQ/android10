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

#include "slang_rs_foreach_lowering.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "llvm/Support/raw_ostream.h"
#include "slang_rs_context.h"
#include "slang_rs_export_foreach.h"

namespace slang {

namespace {

const char KERNEL_LAUNCH_FUNCTION_NAME[] = "rsForEach";
const char KERNEL_LAUNCH_FUNCTION_NAME_WITH_OPTIONS[] = "rsForEachWithOptions";
const char INTERNAL_LAUNCH_FUNCTION_NAME[] =
    "_Z17rsForEachInternaliP14rs_script_calliiP13rs_allocation";

}  // anonymous namespace

RSForEachLowering::RSForEachLowering(RSContext* ctxt)
    : mCtxt(ctxt), mASTCtxt(ctxt->getASTContext()) {}

// Check if the passed-in expr references a kernel function in the following
// pattern in the AST.
//
// ImplicitCastExpr 'void *' <BitCast>
//  `-ImplicitCastExpr 'int (*)(int)' <FunctionToPointerDecay>
//    `-DeclRefExpr 'int (int)' Function 'foo' 'int (int)'
const clang::FunctionDecl* RSForEachLowering::matchFunctionDesignator(
    clang::Expr* expr) {
  clang::ImplicitCastExpr* ToVoidPtr =
      clang::dyn_cast<clang::ImplicitCastExpr>(expr);
  if (ToVoidPtr == nullptr) {
    return nullptr;
  }

  clang::ImplicitCastExpr* Decay =
      clang::dyn_cast<clang::ImplicitCastExpr>(ToVoidPtr->getSubExpr());

  if (Decay == nullptr) {
    return nullptr;
  }

  clang::DeclRefExpr* DRE =
      clang::dyn_cast<clang::DeclRefExpr>(Decay->getSubExpr());

  if (DRE == nullptr) {
    return nullptr;
  }

  const clang::FunctionDecl* FD =
      clang::dyn_cast<clang::FunctionDecl>(DRE->getDecl());

  if (FD == nullptr) {
    return nullptr;
  }

  return FD;
}

// Checks if the call expression is a legal rsForEach call by looking for the
// following pattern in the AST. On success, returns the first argument that is
// a FunctionDecl of a kernel function.
//
// CallExpr 'void'
// |
// |-ImplicitCastExpr 'void (*)(void *, ...)' <FunctionToPointerDecay>
// | `-DeclRefExpr  'void (void *, ...)'  'rsForEach' 'void (void *, ...)'
// |
// |-ImplicitCastExpr 'void *' <BitCast>
// | `-ImplicitCastExpr 'int (*)(int)' <FunctionToPointerDecay>
// |   `-DeclRefExpr 'int (int)' Function 'foo' 'int (int)'
// |
// |-ImplicitCastExpr 'rs_allocation':'rs_allocation' <LValueToRValue>
// | `-DeclRefExpr 'rs_allocation':'rs_allocation' lvalue ParmVar 'in' 'rs_allocation':'rs_allocation'
// |
// `-ImplicitCastExpr 'rs_allocation':'rs_allocation' <LValueToRValue>
//   `-DeclRefExpr  'rs_allocation':'rs_allocation' lvalue ParmVar 'out' 'rs_allocation':'rs_allocation'
const clang::FunctionDecl* RSForEachLowering::matchKernelLaunchCall(
    clang::CallExpr* CE, int* slot, bool* hasOptions) {
  const clang::Decl* D = CE->getCalleeDecl();
  const clang::FunctionDecl* FD = clang::dyn_cast<clang::FunctionDecl>(D);

  if (FD == nullptr) {
    return nullptr;
  }

  const clang::StringRef& funcName = FD->getName();

  if (funcName.equals(KERNEL_LAUNCH_FUNCTION_NAME)) {
    *hasOptions = false;
  } else if (funcName.equals(KERNEL_LAUNCH_FUNCTION_NAME_WITH_OPTIONS)) {
    *hasOptions = true;
  } else {
    return nullptr;
  }

  if (mInsideKernel) {
    mCtxt->ReportError(CE->getExprLoc(),
        "Invalid kernel launch call made from inside another kernel.");
    return nullptr;
  }

  clang::Expr* arg0 = CE->getArg(0);
  const clang::FunctionDecl* kernel = matchFunctionDesignator(arg0);

  if (kernel == nullptr) {
    mCtxt->ReportError(arg0->getExprLoc(),
                       "Invalid kernel launch call. "
                       "Expects a function designator for the first argument.");
    return nullptr;
  }

  // Verifies that kernel is indeed a "kernel" function.
  *slot = mCtxt->getForEachSlotNumber(kernel);
  if (*slot == -1) {
    mCtxt->ReportError(CE->getExprLoc(),
         "%0 applied to function %1 defined without \"kernel\" attribute")
         << funcName << kernel->getName();
    return nullptr;
  }

  return kernel;
}

// Create an AST node for the declaration of rsForEachInternal
clang::FunctionDecl* RSForEachLowering::CreateForEachInternalFunctionDecl() {
  clang::DeclContext* DC = mASTCtxt.getTranslationUnitDecl();
  clang::SourceLocation Loc;

  llvm::StringRef SR(INTERNAL_LAUNCH_FUNCTION_NAME);
  clang::IdentifierInfo& II = mASTCtxt.Idents.get(SR);
  clang::DeclarationName N(&II);

  clang::FunctionProtoType::ExtProtoInfo EPI;

  const clang::QualType& AllocTy = mCtxt->getAllocationType();
  clang::QualType AllocPtrTy = mASTCtxt.getPointerType(AllocTy);

  clang::QualType ScriptCallTy = mCtxt->getScriptCallType();
  const clang::QualType ScriptCallPtrTy = mASTCtxt.getPointerType(ScriptCallTy);

  clang::QualType ParamTypes[] = {
    mASTCtxt.IntTy,   // int slot
    ScriptCallPtrTy,  // rs_script_call_t* launch_options
    mASTCtxt.IntTy,   // int numOutput
    mASTCtxt.IntTy,   // int numInputs
    AllocPtrTy        // rs_allocation* allocs
  };

  clang::QualType T = mASTCtxt.getFunctionType(
      mASTCtxt.VoidTy,  // Return type
      ParamTypes,       // Parameter types
      EPI);

  clang::FunctionDecl* FD = clang::FunctionDecl::Create(
      mASTCtxt, DC, Loc, Loc, N, T, nullptr, clang::SC_Extern);

  static constexpr unsigned kNumParams = sizeof(ParamTypes) / sizeof(ParamTypes[0]);
  clang::ParmVarDecl *ParamDecls[kNumParams];
  for (unsigned I = 0; I != kNumParams; ++I) {
    ParamDecls[I] = clang::ParmVarDecl::Create(mASTCtxt, FD, Loc,
        Loc, nullptr, ParamTypes[I], nullptr, clang::SC_None, nullptr);
    // Implicit means that this declaration was created by the compiler, and
    // not part of the actual source code.
    ParamDecls[I]->setImplicit();
  }
  FD->setParams(llvm::makeArrayRef(ParamDecls, kNumParams));

  // Implicit means that this declaration was created by the compiler, and
  // not part of the actual source code.
  FD->setImplicit();

  return FD;
}

// Create an expression like the following that references the rsForEachInternal to
// replace the callee in the original call expression that references rsForEach.
//
// ImplicitCastExpr 'void (*)(int, rs_script_call_t*, int, int, rs_allocation*)' <FunctionToPointerDecay>
// `-DeclRefExpr 'void' Function '_Z17rsForEachInternaliP14rs_script_calliiP13rs_allocation' 'void (int, rs_script_call_t*, int, int, rs_allocation*)'
clang::Expr* RSForEachLowering::CreateCalleeExprForInternalForEach() {
  clang::FunctionDecl* FDNew = CreateForEachInternalFunctionDecl();

  const clang::QualType FDNewType = FDNew->getType();

  clang::DeclRefExpr* refExpr = clang::DeclRefExpr::Create(
      mASTCtxt, clang::NestedNameSpecifierLoc(), clang::SourceLocation(), FDNew,
      false, clang::SourceLocation(), FDNewType, clang::VK_RValue);

  clang::Expr* calleeNew = clang::ImplicitCastExpr::Create(
      mASTCtxt, mASTCtxt.getPointerType(FDNewType),
      clang::CK_FunctionToPointerDecay, refExpr, nullptr, clang::VK_RValue);

  return calleeNew;
}

// This visit method checks (via pattern matching) if the call expression is to
// rsForEach, and the arguments satisfy the restrictions on the
// rsForEach API. If so, replace the call with a rsForEachInternal call
// with the first argument replaced by the slot number of the kernel function
// referenced in the original first argument.
//
// See comments to the helper methods defined above for details.
void RSForEachLowering::VisitCallExpr(clang::CallExpr* CE) {
  int slot;
  bool hasOptions;
  const clang::FunctionDecl* kernel = matchKernelLaunchCall(CE, &slot, &hasOptions);
  if (kernel == nullptr) {
    return;
  }

  slangAssert(slot >= 0);

  const unsigned numArgsOrig = CE->getNumArgs();

  clang::QualType resultType = kernel->getReturnType().getCanonicalType();
  const unsigned numOutputsExpected = resultType->isVoidType() ? 0 : 1;

  const unsigned numInputsExpected = RSExportForEach::getNumInputs(mCtxt->getTargetAPI(), kernel);

  // Verifies that rsForEach takes the right number of input and output allocations.
  // TODO: Check input/output allocation types match kernel function expectation.
  const unsigned numAllocations = numArgsOrig - (hasOptions ? 2 : 1);
  if (numInputsExpected + numOutputsExpected != numAllocations) {
    mCtxt->ReportError(
      CE->getExprLoc(),
      "Number of input and output allocations unexpected for kernel function %0")
    << kernel->getName();
    return;
  }

  clang::Expr* calleeNew = CreateCalleeExprForInternalForEach();
  CE->setCallee(calleeNew);

  const clang::CanQualType IntTy = mASTCtxt.IntTy;
  const unsigned IntTySize = mASTCtxt.getTypeSize(IntTy);
  const llvm::APInt APIntSlot(IntTySize, slot);
  const clang::Expr* arg0 = CE->getArg(0);
  const clang::SourceLocation Loc(arg0->getLocStart());
  clang::Expr* IntSlotNum =
      clang::IntegerLiteral::Create(mASTCtxt, APIntSlot, IntTy, Loc);
  CE->setArg(0, IntSlotNum);

  /*
    The last few arguments to rsForEach or rsForEachWithOptions are allocations.
    Creates a new compound literal of an array initialized with those values, and
    passes it to rsForEachInternal as the last (the 5th) argument.

    For example, rsForEach(foo, ain1, ain2, aout) would be translated into
    rsForEachInternal(
        1,                                   // Slot number for kernel
        NULL,                                // Launch options
        2,                                   // Number of input allocations
        1,                                   // Number of output allocations
        (rs_allocation[]){ain1, ain2, aout)  // Input and output allocations
    );

    The AST for the rs_allocation array looks like following:

    ImplicitCastExpr 0x99575670 'struct rs_allocation *' <ArrayToPointerDecay>
    `-CompoundLiteralExpr 0x99575648 'struct rs_allocation [3]' lvalue
      `-InitListExpr 0x99575590 'struct rs_allocation [3]'
      |-ImplicitCastExpr 0x99574b38 'rs_allocation':'struct rs_allocation' <LValueToRValue>
      | `-DeclRefExpr 0x99574a08 'rs_allocation':'struct rs_allocation' lvalue ParmVar 0x9942c408 'ain1' 'rs_allocation':'struct rs_allocation'
      |-ImplicitCastExpr 0x99574b50 'rs_allocation':'struct rs_allocation' <LValueToRValue>
      | `-DeclRefExpr 0x99574a30 'rs_allocation':'struct rs_allocation' lvalue ParmVar 0x9942c478 'ain2' 'rs_allocation':'struct rs_allocation'
      `-ImplicitCastExpr 0x99574b68 'rs_allocation':'struct rs_allocation' <LValueToRValue>
        `-DeclRefExpr 0x99574a58 'rs_allocation':'struct rs_allocation' lvalue ParmVar 0x9942c478 'aout' 'rs_allocation':'struct rs_allocation'
  */

  const clang::QualType& AllocTy = mCtxt->getAllocationType();
  const llvm::APInt APIntNumAllocs(IntTySize, numAllocations);
  clang::QualType AllocArrayTy = mASTCtxt.getConstantArrayType(
      AllocTy,
      APIntNumAllocs,
      clang::ArrayType::ArraySizeModifier::Normal,
      0  // index type qualifiers
  );

  const int allocArgIndexEnd = numArgsOrig - 1;
  int allocArgIndexStart = allocArgIndexEnd;

  clang::Expr** args = CE->getArgs();

  clang::SourceLocation lparenloc;
  clang::SourceLocation rparenloc;

  if (numAllocations > 0) {
    allocArgIndexStart = hasOptions ? 2 : 1;
    lparenloc = args[allocArgIndexStart]->getExprLoc();
    rparenloc = args[allocArgIndexEnd]->getExprLoc();
  }

  clang::InitListExpr* init = new (mASTCtxt) clang::InitListExpr(
      mASTCtxt,
      lparenloc,
      llvm::ArrayRef<clang::Expr*>(args + allocArgIndexStart, numAllocations),
      rparenloc);
  init->setType(AllocArrayTy);

  clang::TypeSourceInfo* ti = mASTCtxt.getTrivialTypeSourceInfo(AllocArrayTy);
  clang::CompoundLiteralExpr* CLE = new (mASTCtxt) clang::CompoundLiteralExpr(
      lparenloc,
      ti,
      AllocArrayTy,
      clang::VK_LValue,  // A compound literal is an l-value in C.
      init,
      false  // Not file scope
  );

  const clang::QualType AllocPtrTy = mASTCtxt.getPointerType(AllocTy);

  clang::ImplicitCastExpr* Decay = clang::ImplicitCastExpr::Create(
      mASTCtxt,
      AllocPtrTy,
      clang::CK_ArrayToPointerDecay,
      CLE,
      nullptr,  // C++ cast path
      clang::VK_RValue
  );

  CE->setNumArgs(mASTCtxt, 5);

  CE->setArg(4, Decay);

  // Sets the new arguments for NULL launch option (if the user does not set one),
  // the number of outputs, and the number of inputs.

  if (!hasOptions) {
    const llvm::APInt APIntZero(IntTySize, 0);
    clang::Expr* IntNull =
        clang::IntegerLiteral::Create(mASTCtxt, APIntZero, IntTy, Loc);
    clang::QualType ScriptCallTy = mCtxt->getScriptCallType();
    const clang::QualType ScriptCallPtrTy = mASTCtxt.getPointerType(ScriptCallTy);
    clang::CStyleCastExpr* Cast =
        clang::CStyleCastExpr::Create(mASTCtxt,
                                      ScriptCallPtrTy,
                                      clang::VK_RValue,
                                      clang::CK_NullToPointer,
                                      IntNull,
                                      nullptr,
                                      mASTCtxt.getTrivialTypeSourceInfo(ScriptCallPtrTy),
                                      clang::SourceLocation(),
                                      clang::SourceLocation());
    CE->setArg(1, Cast);
  }

  const llvm::APInt APIntNumOutput(IntTySize, numOutputsExpected);
  clang::Expr* IntNumOutput =
      clang::IntegerLiteral::Create(mASTCtxt, APIntNumOutput, IntTy, Loc);
  CE->setArg(2, IntNumOutput);

  const llvm::APInt APIntNumInputs(IntTySize, numInputsExpected);
  clang::Expr* IntNumInputs =
      clang::IntegerLiteral::Create(mASTCtxt, APIntNumInputs, IntTy, Loc);
  CE->setArg(3, IntNumInputs);
}

void RSForEachLowering::VisitStmt(clang::Stmt* S) {
  for (clang::Stmt* Child : S->children()) {
    if (Child) {
      Visit(Child);
    }
  }
}

void RSForEachLowering::handleForEachCalls(clang::FunctionDecl* FD,
                                           unsigned int targetAPI) {
  slangAssert(FD && FD->hasBody());

  mInsideKernel = FD->hasAttr<clang::RenderScriptKernelAttr>();
  VisitStmt(FD->getBody());
}

}  // namespace slang
