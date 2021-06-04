/*
 * Copyright (C) 2011-2012 The Android Open Source Project
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

#ifndef RSD_CPU_SCRIPT_H
#define RSD_CPU_SCRIPT_H

#include <rs_hal.h>
#include <rsRuntime.h>

#ifndef RS_COMPATIBILITY_LIB
#include <utility>
#endif

#include "rsCpuCore.h"

#include <string>

namespace bcinfo {
    class MetadataExtractor;
} // namespace bcinfo

namespace android {
namespace renderscript {

class ScriptExecutable;

class RsdCpuScriptImpl : public RsdCpuReferenceImpl::CpuScript {
public:
    bool init(char const *resName, char const *cacheDir,
              uint8_t const *bitcode, size_t bitcodeSize, uint32_t flags,
              char const *bccPluginName = nullptr);
    void populateScript(Script *) override;

    void invokeFunction(uint32_t slot, const void *params, size_t paramLength) override;
    int invokeRoot() override;
    virtual void preLaunch(uint32_t slot, const Allocation ** ains,
                           uint32_t inLen, Allocation * aout, const void * usr,
                           uint32_t usrLen, const RsScriptCall *sc);
    virtual void postLaunch(uint32_t slot, const Allocation ** ains,
                            uint32_t inLen, Allocation * aout,
                            const void * usr, uint32_t usrLen,
                            const RsScriptCall *sc);

    void invokeForEach(uint32_t slot,
                       const Allocation ** ains,
                       uint32_t inLen,
                       Allocation* aout,
                       const void* usr,
                       uint32_t usrLen,
                       const RsScriptCall* sc) override;

    void invokeReduce(uint32_t slot,
                      const Allocation ** ains, uint32_t inLen,
                      Allocation* aout,
                      const RsScriptCall* sc) override;

    void invokeInit() override;
    void invokeFreeChildren() override;

    void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) override;
    void getGlobalVar(uint32_t slot, void *data, size_t dataLength) override;
    void setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                  const Element *e, const uint32_t *dims,
                                  size_t dimLength) override;
    void setGlobalBind(uint32_t slot, Allocation *data) override;
    void setGlobalObj(uint32_t slot, ObjectBase *data) override;

    const char* getFieldName(uint32_t slot) const;

    ~RsdCpuScriptImpl() override;
    RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s);

    const Script * getScript() {return mScript;}

    bool forEachMtlsSetup(const Allocation ** ains, uint32_t inLen,
                          Allocation * aout, const void * usr, uint32_t usrLen,
                          const RsScriptCall *sc, MTLaunchStructForEach *mtls);

    virtual void forEachKernelSetup(uint32_t slot, MTLaunchStructForEach *mtls);

    // Build an MTLaunchStruct suitable for launching a general reduce-style kernel.
    bool reduceMtlsSetup(const Allocation ** ains, uint32_t inLen, const Allocation *aout,
                         const RsScriptCall *sc, MTLaunchStructReduce *mtls);
    // Finalize an MTLaunchStruct for launching a general reduce-style kernel.
    virtual void reduceKernelSetup(uint32_t slot, MTLaunchStructReduce *mtls);

    const RsdCpuReference::CpuSymbol * lookupSymbolMath(const char *sym);
    static void * lookupRuntimeStub(void* pContext, char const* name);

    Allocation * getAllocationForPointer(const void *ptr) const override;
    bool storeRSInfoFromSO();

    int getGlobalEntries() const override;
    const char * getGlobalName(int i) const override;
    const void * getGlobalAddress(int i) const override;
    size_t getGlobalSize(int i) const override;
    uint32_t getGlobalProperties(int i) const override;

protected:
    RsdCpuReferenceImpl *mCtx;
    const Script *mScript;
    void *mScriptSO;

#ifndef RS_COMPATIBILITY_LIB
    // Returns the path to the core library we'll use.
    const char* findCoreLib(const bcinfo::MetadataExtractor& bitCodeMetaData, const char* bitcode,
                            size_t bitcodeSize);
#endif

    RootFunc_t mRoot;
    RootFunc_t mRootExpand;
    InitOrDtorFunc_t mInit;
    InitOrDtorFunc_t mFreeChildren;
    ScriptExecutable* mScriptExec;

    Allocation **mBoundAllocs;
    void * mIntrinsicData;
    bool mIsThreadable;

public:
    static const char* BCC_EXE_PATH;
    const char* getBitcodeFilePath() const { return mBitcodeFilePath.c_str(); }

private:
    bool setUpMtlsDimensions(MTLaunchStructCommon *mtls,
                             const RsLaunchDimensions &baseDim,
                             const RsScriptCall *sc);

    std::string mBitcodeFilePath;
    uint32_t mBuildChecksum;
    bool mChecksumNeeded;
};

Allocation * rsdScriptGetAllocationForPointer(
                        const Context *dc,
                        const Script *script,
                        const void *);

uint32_t constructBuildChecksum(uint8_t const *bitcode, size_t bitcodeSize,
                                const char *commandLine,
                                const char ** bccFiles, size_t numFiles);

} // namespace renderscript

#ifdef __LP64__
#define SYSLIBPATH "/system/lib64"
#define SYSLIBPATH_VNDK "/system/lib64/vndk-sp"
#define SYSLIBPATH_BC "/system/lib64"
#define SYSLIBPATH_VENDOR "/system/vendor/lib64"
#elif defined(BUILD_ARM_FOR_X86) && defined(__arm__)
#define SYSLIBPATH "/system/lib/arm"
#define SYSLIBPATH_VNDK "/system/lib/arm/vndk-sp"
#define SYSLIBPATH_BC "/system/lib"
#define SYSLIBPATH_VENDOR "/system/vendor/lib/arm"
#else
#define SYSLIBPATH "/system/lib"
#define SYSLIBPATH_VNDK "/system/lib/vndk-sp"
#define SYSLIBPATH_BC "/system/lib"
#define SYSLIBPATH_VENDOR "/system/vendor/lib"
#endif

} // namespace android

namespace {

inline bool is_force_recompile() {
  char buf[PROP_VALUE_MAX];

  // Re-compile if floating point precision has been overridden.
  android::renderscript::property_get("debug.rs.precision", buf, "");
  if (buf[0] != '\0') {
    return true;
  }

  // Re-compile if debug.rs.forcerecompile is set.
  android::renderscript::property_get("debug.rs.forcerecompile", buf, "0");
  if ((::strcmp(buf, "1") == 0) || (::strcmp(buf, "true") == 0)) {
    return true;
  } else {
    return false;
  }
}

inline std::string getVndkSysLibPath() {
  char buf[PROP_VALUE_MAX];
  android::renderscript::property_get("ro.vndk.version", buf, "");
  std::string versionStr = buf;
  if (versionStr != "" && versionStr != "current") {
    return SYSLIBPATH_VNDK "-" + versionStr;
  }
  return SYSLIBPATH_VNDK;
}

}  // anonymous namespace

#endif  // RSD_CPU_SCRIPT_H
