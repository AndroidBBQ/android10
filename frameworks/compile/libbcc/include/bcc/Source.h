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

#ifndef BCC_SOURCE_H
#define BCC_SOURCE_H

#include <string>

namespace llvm {
  class Module;
}

namespace bcinfo {
  class MetadataExtractor;
}

namespace bcc {

class BCCContext;

class Source {
private:
  const std::string mName; // A unique name
  BCCContext &mContext;
  llvm::Module *mModule;

  bcinfo::MetadataExtractor *mMetadata;

  // If true, destructor won't destroy the mModule.
  bool mNoDelete;

  // Keep track of whether mModule is destroyed (possibly as a consequence of
  // getting linked with a different llvm::Module).
  bool mIsModuleDestroyed;

private:
  Source(const char* name, BCCContext &pContext, llvm::Module &pModule,
         bool pNoDelete = false);

public:
  static Source *CreateFromBuffer(BCCContext &pContext,
                                  const char *pName,
                                  const char *pBitcode,
                                  size_t pBitcodeSize);

  static Source *CreateFromFile(BCCContext &pContext,
                                const std::string &pPath);

  // Create a Source object from an existing module. If pNoDelete
  // is true, destructor won't call delete on the given module.
  static Source *CreateFromModule(BCCContext &pContext,
                                  const char* name,
                                  llvm::Module &pModule,
                                  uint32_t compilerVersion,
                                  uint32_t optimizationLevel,
                                  bool pNoDelete = false);

  const std::string& getName() const { return mName; }

  // Merge the current source with pSource. pSource
  // will be destroyed after successfully merged. Return false on error.
  bool merge(Source &pSource);

  unsigned getCompilerVersion() const;

  void getWrapperInformation(unsigned *compilerVersion,
                             unsigned *optimizationLevel) const;

  inline BCCContext &getContext()
  { return mContext; }
  inline const BCCContext &getContext() const
  { return mContext; }

  void setModule(llvm::Module *pModule);

  inline llvm::Module &getModule()
  { return *mModule;  }
  inline const llvm::Module &getModule() const
  { return *mModule;  }

  // Get the "identifier" of the bitcode. This will return the value of pName
  // when it's created using CreateFromBuffer and pPath if CreateFromFile().
  const std::string &getIdentifier() const;

  void addBuildChecksumMetadata(const char *) const;

  // Get whether debugging has been enabled for this module by checking
  // for presence of debug info in the module.
  bool getDebugInfoEnabled() const;

  // Extract metadata from mModule using MetadataExtractor.
  bool extractMetadata();
  bcinfo::MetadataExtractor* getMetadata() const { return mMetadata; }

  // Mark mModule was destroyed in the process of linking with a different
  // llvm::Module
  void markModuleDestroyed() { mIsModuleDestroyed = true; }

  ~Source();
};

} // namespace bcc

#endif // BCC_SOURCE_H
