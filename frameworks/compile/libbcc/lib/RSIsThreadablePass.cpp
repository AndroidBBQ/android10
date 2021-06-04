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

#include "Log.h"
#include "RSTransforms.h"

#include <cstdlib>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

namespace { // anonymous namespace

// Create a Module pass that screens all the global functions in the module and
// check if any non-threadable function is callable.  If so, we mark the
// Module as non-threadable by adding a metadata flag '#rs_is_threadable'

class RSIsThreadablePass : public llvm::ModulePass {
private:
  static char ID;

  std::vector<std::string> nonThreadableFns = {
    "_Z22rsgBindProgramFragment19rs_program_fragment",
    "_Z19rsgBindProgramStore16rs_program_store",
    "_Z20rsgBindProgramVertex17rs_program_vertex",
    "_Z20rsgBindProgramRaster17rs_program_raster",
    "_Z14rsgBindSampler19rs_program_fragmentj10rs_sampler",
    "_Z14rsgBindTexture19rs_program_fragmentj13rs_allocation",
    "_Z15rsgBindConstant19rs_program_fragmentj13rs_allocation",
    "_Z15rsgBindConstant17rs_program_vertexj13rs_allocation",
    "_Z36rsgProgramVertexLoadProjectionMatrixPK12rs_matrix4x4",
    "_Z31rsgProgramVertexLoadModelMatrixPK12rs_matrix4x4",
    "_Z33rsgProgramVertexLoadTextureMatrixPK12rs_matrix4x4",
    "_Z35rsgProgramVertexGetProjectionMatrixP12rs_matrix4x4",
    "_Z31rsgProgramFragmentConstantColor19rs_program_fragmentffff",
    "_Z11rsgGetWidthv",
    "_Z12rsgGetHeightv",
    "_Z11rsgDrawRectfffff",
    "_Z11rsgDrawQuadffffffffffff",
    "_Z20rsgDrawQuadTexCoordsffffffffffffffffffff",
    "_Z24rsgDrawSpriteScreenspacefffff",
    "_Z11rsgDrawMesh7rs_mesh",
    "_Z11rsgDrawMesh7rs_meshj",
    "_Z11rsgDrawMesh7rs_meshjjj",
    "_Z25rsgMeshComputeBoundingBox7rs_meshPfS0_S0_S0_S0_S0_",
    "_Z11rsgDrawPath7rs_path",
    "_Z13rsgClearColorffff",
    "_Z13rsgClearDepthf",
    "_Z11rsgDrawTextPKcii",
    "_Z11rsgDrawText13rs_allocationii",
    "_Z14rsgMeasureTextPKcPiS1_S1_S1_",
    "_Z14rsgMeasureText13rs_allocationPiS0_S0_S0_",
    "_Z11rsgBindFont7rs_font",
    "_Z12rsgFontColorffff",
    "_Z18rsgBindColorTarget13rs_allocationj",
    "_Z18rsgBindDepthTarget13rs_allocation",
    "_Z19rsgClearColorTargetj",
    "_Z19rsgClearDepthTargetv",
    "_Z24rsgClearAllRenderTargetsv",
    "_Z7rsGetDtv",
    "_Z5colorffff",
    "_Z9rsgFinishv",
  };

  bool isPresent(std::vector<std::string> &list, const std::string &name) {
    auto lower = std::lower_bound(list.begin(),
                                  list.end(),
                                  name);

    if (lower != list.end() && name.compare(*lower) == 0)
      return true;
    return false;
  }

public:
  RSIsThreadablePass()
    : ModulePass (ID) {
      std::sort(nonThreadableFns.begin(), nonThreadableFns.end());
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &M) override {
    bool threadable = true;

    auto &FunctionList(M.getFunctionList());
    for (auto &F: FunctionList) {
      if (isPresent(nonThreadableFns, F.getName().str())) {
        threadable = false;
        break;
      }
    }

    llvm::LLVMContext &context = M.getContext();
    llvm::MDString *val =
      llvm::MDString::get(context, (threadable) ? "yes" : "no");
    llvm::NamedMDNode *node =
        M.getOrInsertNamedMetadata("#rs_is_threadable");
    node->addOperand(llvm::MDNode::get(context, val));

    return false;
  }

};

}

char RSIsThreadablePass::ID = 0;

namespace bcc {

llvm::ModulePass *
createRSIsThreadablePass () {
  return new RSIsThreadablePass();
}

}
