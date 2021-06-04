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

#include "bcc/Script.h"

#include "Assert.h"
#include "Log.h"

#include "bcc/CompilerConfig.h"
#include "bcc/Source.h"

#include "bcinfo/MetadataExtractor.h"

#include <llvm/IR/Module.h>

using namespace bcc;

Script::Script(Source *pSource)
    : mSource(pSource),
      mOptimizationLevel(llvm::CodeGenOpt::Aggressive),
      mLinkRuntimeCallback(nullptr), mEmbedInfo(false), mEmbedGlobalInfo(false),
      mEmbedGlobalInfoSkipConstant(false) {}

bool Script::LinkRuntime(const char *core_lib) {
  bccAssert(core_lib != nullptr);

  // Using the same context with the source.
  BCCContext &context = mSource->getContext();

  Source *libclcore_source = Source::CreateFromFile(context, core_lib);
  if (libclcore_source == nullptr) {
    ALOGE("Failed to load Renderscript library '%s' to link!", core_lib);
    return false;
  }

  if (mLinkRuntimeCallback != nullptr) {
    mLinkRuntimeCallback(this, &mSource->getModule(),
                         &libclcore_source->getModule());
  }

  // For every named metadata node in the source (libclcore_source),
  // the merge process ensures there is a same-named metadata node in
  // the destination (mSource) (creating it if necessary) and appends
  // all of the source node's operands to the end of the destination
  // node's operands.  In the case of the wrapper metadata
  //
  //   kWrapperMetadataName -> (compilerVersion, optimizationLevel)
  //
  // this is not the behavior we want.  Instead, we want to retain the
  // source wrapper metadata:
  // - compiler version in libclcore_source is 0, a nonsensical value.
  //   As documented in slang_version.h, libclcore_source must not
  //   violate any compiler version guarantees, so the right thing to
  //   do is retain the compiler version from source, which specifies
  //   which guarantees source (and hence the merged code) satisfies.
  //   See frameworks/rs/driver/README.txt regarding libclcore_source
  //   obeying compiler version guarantees.
  // - optimization level in source and libclcore_source is meaningful.
  //   We simply define the optimization level in the linked code to
  //   be the optimization level of source.
  // The easiest way to retain the source wrapper metadata is to delete
  // the libclcore_source wrapper metadata.
  llvm::Module &libclcore_module = libclcore_source->getModule();
  llvm::NamedMDNode *const wrapperMDNode =
      libclcore_module.getNamedMetadata(bcinfo::MetadataExtractor::kWrapperMetadataName);
  bccAssert(wrapperMDNode != nullptr);
  libclcore_module.eraseNamedMetadata(wrapperMDNode);
  if (!mSource->merge(*libclcore_source)) {
    ALOGE("Failed to link Renderscript library '%s'!", core_lib);
    delete libclcore_source;
    return false;
  }

  return true;
}

bool Script::mergeSource(Source &pSource) { return mSource->merge(pSource); }
