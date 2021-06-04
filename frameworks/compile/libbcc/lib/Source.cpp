/*
 * Copyright 2012, The Android Open Source Project
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

#include "bcc/Source.h"

#include "Log.h"
#include "bcc/BCCContext.h"

#include <new>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/MemoryBuffer.h>
#include "llvm/Support/raw_ostream.h"

#include "Assert.h"
#include "bcinfo/BitcodeWrapper.h"
#include "bcinfo/MetadataExtractor.h"

#include "BCCContextImpl.h"

namespace {

// Helper function to load the bitcode. This uses "bitcode lazy load" feature to
// reduce the startup time. On success, return the LLVM module object created
// and take the ownership of input memory buffer (i.e., pInput). On error,
// return nullptr and will NOT take the ownership of pInput.
static inline std::unique_ptr<llvm::Module> helper_load_bitcode(llvm::LLVMContext &pContext,
                                                std::unique_ptr<llvm::MemoryBuffer> &&pInput) {
  llvm::ErrorOr<std::unique_ptr<llvm::Module> > moduleOrError
      = llvm::getLazyBitcodeModule(std::move(pInput), pContext);
  if (std::error_code ec = moduleOrError.getError()) {
    ALOGE("Unable to parse the given bitcode file `%s'! (%s)",
          pInput->getBufferIdentifier(), ec.message().c_str());
  }

  return std::move(moduleOrError.get());
}

static void helper_get_module_metadata_from_bitcode_wrapper(
    uint32_t *compilerVersion, uint32_t *optimizationLevel,
    const bcinfo::BitcodeWrapper &wrapper) {
  *compilerVersion = wrapper.getCompilerVersion();
  *optimizationLevel = wrapper.getOptimizationLevel();
}

static void helper_set_module_metadata_from_bitcode_wrapper(llvm::Module &module,
                                                            const uint32_t compilerVersion,
                                                            const uint32_t optimizationLevel) {
  llvm::LLVMContext &llvmContext = module.getContext();

  llvm::NamedMDNode *const wrapperMDNode =
      module.getOrInsertNamedMetadata(bcinfo::MetadataExtractor::kWrapperMetadataName);
  bccAssert(wrapperMDNode->getNumOperands() == 0);  // expect to have just now created this node

  llvm::SmallVector<llvm::Metadata *, 2> wrapperInfo = {
    llvm::MDString::get(llvmContext, llvm::utostr(compilerVersion)),
    llvm::MDString::get(llvmContext, llvm::utostr(optimizationLevel))
  };

  wrapperMDNode->addOperand(llvm::MDTuple::get(llvmContext, wrapperInfo));
}

} // end anonymous namespace

namespace bcc {

unsigned Source::getCompilerVersion() const {
  return bcinfo::MetadataExtractor(&getModule()).getCompilerVersion();
}

void Source::getWrapperInformation(unsigned *compilerVersion,
                                   unsigned *optimizationLevel) const {
  const bcinfo::MetadataExtractor &me = bcinfo::MetadataExtractor(&getModule());
  *compilerVersion = me.getCompilerVersion();
  *optimizationLevel = me.getOptimizationLevel();
}

void Source::setModule(llvm::Module *pModule) {
  if (!mNoDelete && (mModule != pModule)) delete mModule;
  mModule = pModule;
}

Source *Source::CreateFromBuffer(BCCContext &pContext,
                                 const char *pName,
                                 const char *pBitcode,
                                 size_t pBitcodeSize) {
  llvm::StringRef input_data(pBitcode, pBitcodeSize);
  std::unique_ptr<llvm::MemoryBuffer> input_memory =
      llvm::MemoryBuffer::getMemBuffer(input_data, "", false);

  if (input_memory == nullptr) {
    ALOGE("Unable to load bitcode `%s' from buffer!", pName);
    return nullptr;
  }

  auto managedModule = helper_load_bitcode(pContext.mImpl->mLLVMContext,
                                           std::move(input_memory));

  // Release the managed llvm::Module* since this object gets deleted either in
  // the error check below or in ~Source() (since pNoDelete is false).
  llvm::Module *module = managedModule.release();
  if (module == nullptr) {
    return nullptr;
  }

  uint32_t compilerVersion, optimizationLevel;
  helper_get_module_metadata_from_bitcode_wrapper(&compilerVersion, &optimizationLevel,
                                                  bcinfo::BitcodeWrapper(pBitcode, pBitcodeSize));
  Source *result = CreateFromModule(pContext, pName, *module,
                                    compilerVersion, optimizationLevel,
                                    /* pNoDelete */false);
  if (result == nullptr) {
    delete module;
  }

  return result;
}

Source *Source::CreateFromFile(BCCContext &pContext, const std::string &pPath) {

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> mb_or_error =
      llvm::MemoryBuffer::getFile(pPath);
  if (mb_or_error.getError()) {
    ALOGE("Failed to load bitcode from path %s! (%s)", pPath.c_str(),
          mb_or_error.getError().message().c_str());
    return nullptr;
  }
  std::unique_ptr<llvm::MemoryBuffer> input_data = std::move(mb_or_error.get());

  uint32_t compilerVersion, optimizationLevel;
  helper_get_module_metadata_from_bitcode_wrapper(&compilerVersion, &optimizationLevel,
                                                  bcinfo::BitcodeWrapper(input_data->getBufferStart(),
                                                                         input_data->getBufferSize()));

  std::unique_ptr<llvm::MemoryBuffer> input_memory(input_data.release());
  auto managedModule = helper_load_bitcode(pContext.mImpl->mLLVMContext,
                                           std::move(input_memory));

  // Release the managed llvm::Module* since this object gets deleted either in
  // the error check below or in ~Source() (since pNoDelete is false).
  llvm::Module *module = managedModule.release();
  if (module == nullptr) {
    return nullptr;
  }

  Source *result = CreateFromModule(pContext, pPath.c_str(), *module,
                                    compilerVersion, optimizationLevel,
                                    /* pNoDelete */false);
  if (result == nullptr) {
    delete module;
  }

  return result;
}

Source *Source::CreateFromModule(BCCContext &pContext, const char* name, llvm::Module &pModule,
                                 const uint32_t compilerVersion,
                                 const uint32_t optimizationLevel,
                                 bool pNoDelete) {
  std::string ErrorInfo;
  llvm::raw_string_ostream ErrorStream(ErrorInfo);
  pModule.materializeAll();
  if (llvm::verifyModule(pModule, &ErrorStream)) {
    ALOGE("Bitcode of RenderScript module does not pass verification: `%s'!",
          ErrorStream.str().c_str());
    return nullptr;
  }

  Source *result = new (std::nothrow) Source(name, pContext, pModule, pNoDelete);
  if (result == nullptr) {
    ALOGE("Out of memory during Source object allocation for `%s'!",
          pModule.getModuleIdentifier().c_str());
  }
  helper_set_module_metadata_from_bitcode_wrapper(pModule, compilerVersion, optimizationLevel);
  return result;
}

Source::Source(const char* name, BCCContext &pContext, llvm::Module &pModule,
               bool pNoDelete)
    : mName(name), mContext(pContext), mModule(&pModule), mMetadata(nullptr),
      mNoDelete(pNoDelete), mIsModuleDestroyed(false) {
    pContext.addSource(*this);
}

Source::~Source() {
  mContext.removeSource(*this);
  if (!mNoDelete && !mIsModuleDestroyed)
    delete mModule;
  delete mMetadata;
}

bool Source::merge(Source &pSource) {
  // TODO(srhines): Add back logging of actual diagnostics from linking.
  if (llvm::Linker::linkModules(*mModule, std::unique_ptr<llvm::Module>(&pSource.getModule())) != 0) {
    ALOGE("Failed to link source `%s' with `%s'!",
          getIdentifier().c_str(), pSource.getIdentifier().c_str());
    return false;
  }
  // pSource.getModule() is destroyed after linking.
  pSource.markModuleDestroyed();

  return true;
}

const std::string &Source::getIdentifier() const {
  return mModule->getModuleIdentifier();
}

void Source::addBuildChecksumMetadata(const char *buildChecksum) const {
    llvm::LLVMContext &context = mContext.mImpl->mLLVMContext;
    llvm::MDString *val = llvm::MDString::get(context, buildChecksum);
    llvm::NamedMDNode *node =
        mModule->getOrInsertNamedMetadata("#rs_build_checksum");
    node->addOperand(llvm::MDNode::get(context, val));
}

bool Source::getDebugInfoEnabled() const {
  return mModule->getNamedMetadata("llvm.dbg.cu") != nullptr;
}

bool Source::extractMetadata() {
  mMetadata = new bcinfo::MetadataExtractor(mModule);
  return mMetadata->extract();
}

} // namespace bcc
