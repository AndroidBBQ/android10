/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RSOV_SCRIPT_H
#define RSOV_SCRIPT_H

#include <vulkan/vulkan.h>

#include <map>
#include <vector>

#include "bcinfo/MetadataExtractor.h"
#include "rsDefines.h"
#include "rs_hal.h"
#include "rsd_cpu.h"

namespace android {
namespace renderscript {

class Allocation;
class Context;
class Element;
class Script;
class ScriptC;

namespace rsov {

class RSoVAllocation;
class RSoVBuffer;
class RSoVContext;

// TODO: CpuScript is a bad name for the base class. Fix with a refactoring.
class RSoVScript : RsdCpuReference::CpuScript {
 public:
  RSoVScript(RSoVContext *context, std::vector<uint32_t> &&spvWords,
             bcinfo::MetadataExtractor *ME,
             std::map<std::string, int> *GAMapping);
  RSoVScript(RSoVContext *context, const std::vector<uint32_t> &spvWords,
             bcinfo::MetadataExtractor *ME,
             std::map<std::string, int> *GAMapping) = delete;

  virtual ~RSoVScript();

  static bool isScriptCpuBacked(const Script *s);
  static void initScriptOnCpu(Script *s, RsdCpuReference::CpuScript *cs);
  static void initScriptOnRSoV(Script *s, RSoVScript *rsovScript);

  void populateScript(Script *) override;
  void invokeFunction(uint32_t slot, const void *params,
                      size_t paramLength) override;
  int invokeRoot() override;

  void invokeForEach(uint32_t slot, const Allocation **ains, uint32_t inLen,
                     Allocation *aout, const void *usr, uint32_t usrLen,
                     const RsScriptCall *sc) override;

  void invokeReduce(uint32_t slot, const Allocation **ains, uint32_t inLen,
                    Allocation *aout, const RsScriptCall *sc) override;

  void invokeInit() override;
  void invokeFreeChildren() override;

  void setGlobalVar(uint32_t slot, const void *data,
                    size_t dataLength) override;
  void getGlobalVar(uint32_t slot, void *data, size_t dataLength) override;
  void setGlobalVarWithElemDims(uint32_t slot, const void *data,
                                size_t dataLength, const Element *e,
                                const uint32_t *dims,
                                size_t dimLength) override;

  void setGlobalBind(uint32_t slot, Allocation *data) override;
  void setGlobalObj(uint32_t slot, ObjectBase *obj) override;

  Allocation *getAllocationForPointer(const void *ptr) const override;

  // Returns number of global variables in this Script (may be 0 if
  // compiler is not configured to emit this information).
  int getGlobalEntries() const override;
  // Returns the name of the global variable at index i.
  const char *getGlobalName(int i) const override;
  // Returns the CPU address of the global variable at index i.
  const void *getGlobalAddress(int i) const override;
  // Returns the size (in bytes) of the global variable at index i.
  size_t getGlobalSize(int i) const override;
  // Returns the properties of the global variable at index i.
  uint32_t getGlobalProperties(int i) const override;

  void setCpuScript(RsdCpuReference::CpuScript *cs) { mCpuScript = cs; }

  RsdCpuReference::CpuScript *getCpuScript() const { return mCpuScript; }

 private:
  void InitDescriptorAndPipelineLayouts(uint32_t inLen);
  void InitShader(uint32_t slot);
  void InitDescriptorPool(uint32_t inLen);
  void InitDescriptorSet(const std::vector<RSoVAllocation *> &inputAllocations,
                         RSoVAllocation *outputAllocation);
  void InitPipelineCache();
  void InitPipeline();
  void MarshalTypeInfo();
  void runForEach(uint32_t slot, uint32_t inLen,
                  const std::vector<RSoVAllocation *> &input,
                  RSoVAllocation *output);

  // Gets the offset for the global variable with the given slot number in
  // the global buffer
  uint32_t GetExportedVarOffset(uint32_t slot) const {
    // High-level Java or C++ API has verified that slot is in range
    return mExportedVarOffsets[slot];
  }

  static constexpr int CPU_SCRIPT_MAGIC_NUMBER = 0x60000;

  RSoVContext *mRSoV;
  VkDevice mDevice;
  std::vector<uint32_t> mSPIRVWords;
  RsdCpuReference::CpuScript *mCpuScript;

  static constexpr int NUM_DESCRIPTOR_SETS = 1;
  std::vector<VkDescriptorSetLayout> mDescLayout;
  VkPipelineLayout mPipelineLayout;
  VkPipeline mComputePipeline;
  // TODO: Multiple stages for multiple kernels
  VkPipelineShaderStageCreateInfo mShaderStage;
  VkDescriptorPool mDescPool;
  std::vector<VkDescriptorSet> mDescSet;
  // For kernel names
  const bcinfo::MetadataExtractor *mME;
  std::unique_ptr<RSoVBuffer> mGlobals;
  std::vector<uint32_t> mExportedVarOffsets;
  // Metadata of global allocations
  std::unique_ptr<RSoVBuffer> mGlobalAllocationMetadata;
  // Mapping of global allocation to rsov-assigned ID
  std::unique_ptr<std::map<std::string, int> > mGAMapping;
};

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

extern bool rsovScriptInit(const android::renderscript::Context *rsc,
                           android::renderscript::ScriptC *script,
                           char const *resName, char const *cacheDir,
                           uint8_t const *bitcode, size_t bitcodeSize,
                           uint32_t flags);

extern bool rsovInitIntrinsic(const android::renderscript::Context *rsc,
                              android::renderscript::Script *s,
                              RsScriptIntrinsicID iid,
                              android::renderscript::Element *e);

extern void rsovScriptInvokeFunction(const android::renderscript::Context *dc,
                                     android::renderscript::Script *script,
                                     uint32_t slot, const void *params,
                                     size_t paramLength);

extern void rsovScriptInvokeForEach(
    const android::renderscript::Context *rsc, android::renderscript::Script *s,
    uint32_t slot, const android::renderscript::Allocation *ain,
    android::renderscript::Allocation *aout, const void *usr, size_t usrLen,
    const RsScriptCall *sc);

extern void rsovScriptInvokeReduce(
    const android::renderscript::Context *rsc, android::renderscript::Script *s,
    uint32_t slot, const android::renderscript::Allocation **ains, size_t inLen,
    android::renderscript::Allocation *aout, const RsScriptCall *sc);

extern void rsovScriptInvokeForEachMulti(
    const android::renderscript::Context *rsc, android::renderscript::Script *s,
    uint32_t slot, const android::renderscript::Allocation **ains, size_t inLen,
    android::renderscript::Allocation *aout, const void *usr, size_t usrLen,
    const RsScriptCall *sc);

extern int rsovScriptInvokeRoot(const android::renderscript::Context *dc,
                                android::renderscript::Script *script);

extern void rsovScriptInvokeInit(const android::renderscript::Context *dc,
                                 android::renderscript::Script *script);

extern void rsovScriptInvokeFreeChildren(
    const android::renderscript::Context *dc,
    android::renderscript::Script *script);

extern void rsovScriptSetGlobalVar(const android::renderscript::Context *,
                                   const android::renderscript::Script *,
                                   uint32_t slot, void *data, size_t dataLen);

extern void rsovScriptGetGlobalVar(const android::renderscript::Context *,
                                   const android::renderscript::Script *,
                                   uint32_t slot, void *data, size_t dataLen);

extern void rsovScriptSetGlobalVarWithElemDims(
    const android::renderscript::Context *,
    const android::renderscript::Script *, uint32_t slot, void *data,
    size_t dataLength, const android::renderscript::Element *,
    const uint32_t *dims, size_t dimLength);
extern void rsovScriptSetGlobalBind(const android::renderscript::Context *,
                                    const android::renderscript::Script *,
                                    uint32_t slot,
                                    android::renderscript::Allocation *data);

extern void rsovScriptSetGlobalObj(const android::renderscript::Context *,
                                   const android::renderscript::Script *,
                                   uint32_t slot,
                                   android::renderscript::ObjectBase *data);

extern void rsovScriptSetGlobal(const android::renderscript::Context *dc,
                                const android::renderscript::Script *script,
                                uint32_t slot, void *data, size_t dataLength);
extern void rsovScriptGetGlobal(const android::renderscript::Context *dc,
                                const android::renderscript::Script *script,
                                uint32_t slot, void *data, size_t dataLength);
extern void rsovScriptDestroy(const android::renderscript::Context *dc,
                              android::renderscript::Script *script);

extern android::renderscript::Allocation *rsovScriptGetAllocationForPointer(
    const android::renderscript::Context *dc,
    const android::renderscript::Script *script, const void *);

extern void rsovScriptUpdateCachedObject(
    const android::renderscript::Context *rsc,
    const android::renderscript::Script *script,
    android::renderscript::rs_script *obj);

#endif  // RSOV_SCRIPT_H
