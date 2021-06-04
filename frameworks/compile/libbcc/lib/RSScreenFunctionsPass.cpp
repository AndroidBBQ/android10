/*
 * Copyright 2014, The Android Open Source Project
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

#include "Log.h"
#include "RSTransforms.h"
#include "RSStubsWhiteList.h"

#include <cstdlib>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

namespace { // anonymous namespace

// Create a Module pass that screens all the global functions in the
// module and check if any disallowed external function is accessible
// and potentially callable.
class RSScreenFunctionsPass : public llvm::ModulePass {
private:
  static char ID;

  std::vector<std::string> &whiteList = stubList;

  bool isPresent(std::vector<std::string> &list, const std::string &name) {
    auto lower = std::lower_bound(list.begin(),
                                  list.end(),
                                  name);

    if (lower != list.end() && name.compare(*lower) == 0)
      return true;
    return false;
  }

  bool isLegal(llvm::Function &F) {
    // A global function symbol is legal if
    // a. it has a body, i.e. is not empty or
    // b. its name starts with "llvm." or
    // c. it is present in the whitelist

    if (!F.empty())
      return true;

    llvm::StringRef FName = F.getName();
    if (FName.startswith("llvm."))
      return true;

    if (isPresent(whiteList, FName.str()))
      return true;

    return false;
  }

public:
  RSScreenFunctionsPass()
    : ModulePass (ID) {
      std::sort(whiteList.begin(), whiteList.end());
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &M) override {
    bool failed = false;

    auto &FunctionList(M.getFunctionList());
    for(auto &F: FunctionList) {
      if (!isLegal(F)) {
        ALOGE("Call to function %s from RenderScript is disallowed\n",
              F.getName().str().c_str());
        failed = true;
      }
    }

    if (failed) {
      llvm::report_fatal_error("Use of undefined external function");
    }

    return false;
  }

};

}

char RSScreenFunctionsPass::ID = 0;

namespace bcc {

llvm::ModulePass *
createRSScreenFunctionsPass() {
  return new RSScreenFunctionsPass();
}

}
