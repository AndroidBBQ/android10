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

#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsCpuExecutable.h"

#ifdef RS_COMPATIBILITY_LIB
    #include <stdio.h>
    #include <sys/stat.h>
    #include <unistd.h>
#else
    #include "rsCppUtils.h"

    #include <bcc/Config.h>
    #include <bcinfo/MetadataExtractor.h>

    #include <zlib.h>
    #include <sys/file.h>
    #include <sys/types.h>
    #include <unistd.h>

    #include <string>
    #include <vector>
#endif

#include <set>
#include <string>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

namespace {

static const bool kDebugGlobalVariables = false;

static bool allocationLODIsNull(const android::renderscript::Allocation *alloc) {
  // Even if alloc != nullptr, mallocPtr could be null if
  // IO_OUTPUT/IO_INPUT with no bound surface.
  return alloc && alloc->mHal.drvState.lod[0].mallocPtr == nullptr;
}

#ifndef RS_COMPATIBILITY_LIB

static void setCompileArguments(std::vector<const char*>* args,
                                const std::string& bcFileName,
                                const char* cacheDir, const char* resName,
                                const char* core_lib, bool useRSDebugContext,
                                const char* bccPluginName, bool emitGlobalInfo,
                                int optLevel, bool emitGlobalInfoSkipConstant) {
    rsAssert(cacheDir && resName && core_lib);
    args->push_back(android::renderscript::RsdCpuScriptImpl::BCC_EXE_PATH);
    args->push_back("-unroll-runtime");
    args->push_back("-scalarize-load-store");
    if (emitGlobalInfo) {
        args->push_back("-rs-global-info");
        if (emitGlobalInfoSkipConstant) {
            args->push_back("-rs-global-info-skip-constant");
        }
    }
    args->push_back("-o");
    args->push_back(resName);
    args->push_back("-output_path");
    args->push_back(cacheDir);
    args->push_back("-bclib");
    args->push_back(core_lib);
    args->push_back("-mtriple");
    args->push_back(DEFAULT_TARGET_TRIPLE_STRING);
    args->push_back("-O");

    switch (optLevel) {
    case 0:
        args->push_back("0");
        break;
    case 3:
        args->push_back("3");
        break;
    default:
        ALOGW("Expected optimization level of 0 or 3. Received %d", optLevel);
        args->push_back("3");
        break;
    }

    // Enable workaround for A53 codegen by default.
#if defined(__aarch64__) && !defined(DISABLE_A53_WORKAROUND)
    args->push_back("-aarch64-fix-cortex-a53-835769");
#endif

    // Execute the bcc compiler.
    if (useRSDebugContext) {
        args->push_back("-rs-debug-ctx");
    } else {
        // Only load additional libraries for compiles that don't use
        // the debug context.
        if (bccPluginName && strlen(bccPluginName) > 0) {
#ifdef __ANDROID__
            // For Android, -plugin option must be used in order to load the
            // vendor plugin from the sphal namespace.
            args->push_back("-plugin");
#else
            args->push_back("-load");
#endif
            args->push_back(bccPluginName);
        }
    }

    args->push_back("-fPIC");
    args->push_back("-embedRSInfo");

    args->push_back(bcFileName.c_str());
    args->push_back(nullptr);
}

static bool compileBitcode(const std::string &bcFileName,
                           const char *bitcode,
                           size_t bitcodeSize,
                           std::vector<const char *> &compileArguments) {
    rsAssert(bitcode && bitcodeSize);

    FILE *bcfile = fopen(bcFileName.c_str(), "we");
    if (!bcfile) {
        ALOGE("Could not write to %s", bcFileName.c_str());
        return false;
    }
    size_t nwritten = fwrite(bitcode, 1, bitcodeSize, bcfile);
    fclose(bcfile);
    if (nwritten != bitcodeSize) {
        ALOGE("Could not write %zu bytes to %s", bitcodeSize,
              bcFileName.c_str());
        return false;
    }

    return android::renderscript::rsuExecuteCommand(
                   android::renderscript::RsdCpuScriptImpl::BCC_EXE_PATH,
                   compileArguments.size()-1, compileArguments.data());
}

// The checksum is unnecessary under a few conditions, since the primary
// use-case for it is debugging. If we are loading something from the
// system partition (read-only), we know that it was precompiled as part of
// application ahead of time (and thus the checksum is completely
// unnecessary). The checksum is also unnecessary on release (non-debug)
// builds, as the only way to get a shared object is to have compiled the
// script once already. On a release build, there is no way to adjust the
// other libraries/dependencies, and so the only reason to recompile would
// be for a source APK change or an OTA. In either case, the APK would be
// reinstalled, which would already clear the code_cache/ directory.
bool isChecksumNeeded(const char *cacheDir) {
    static const std::string sysLibPathVndk = getVndkSysLibPath();
    if ((::strcmp(SYSLIBPATH, cacheDir) == 0) ||
        (::strcmp(sysLibPathVndk.c_str(), cacheDir) == 0) ||
        (::strcmp(SYSLIBPATH_VENDOR, cacheDir) == 0))
        return false;
    char buf[PROP_VALUE_MAX];
    android::renderscript::property_get("ro.debuggable", buf, "");
    return (buf[0] == '1');
}

bool addFileToChecksum(const char *fileName, uint32_t &checksum) {
    int FD = open(fileName, O_RDONLY | O_CLOEXEC);
    if (FD == -1) {
        ALOGE("Cannot open file \'%s\' to compute checksum", fileName);
        return false;
    }

    char buf[256];
    while (true) {
        ssize_t nread = read(FD, buf, sizeof(buf));
        if (nread < 0) { // bail out on failed read
            ALOGE("Error while computing checksum for file \'%s\'", fileName);
            return false;
        }

        checksum = adler32(checksum, (const unsigned char *) buf, nread);
        if (static_cast<size_t>(nread) < sizeof(buf)) // EOF
            break;
    }

    if (close(FD) != 0) {
        ALOGE("Cannot close file \'%s\' after computing checksum", fileName);
        return false;
    }
    return true;
}

#endif  // !defined(RS_COMPATIBILITY_LIB)
}  // namespace

namespace android {
namespace renderscript {

#ifndef RS_COMPATIBILITY_LIB

uint32_t constructBuildChecksum(uint8_t const *bitcode, size_t bitcodeSize,
                                const char *commandLine,
                                const char** bccFiles, size_t numFiles) {
    uint32_t checksum = adler32(0L, Z_NULL, 0);

    // include checksum of bitcode
    if (bitcode != nullptr && bitcodeSize > 0) {
        checksum = adler32(checksum, bitcode, bitcodeSize);
    }

    // include checksum of command line arguments
    checksum = adler32(checksum, (const unsigned char *) commandLine,
                       strlen(commandLine));

    // include checksum of bccFiles
    for (size_t i = 0; i < numFiles; i++) {
        const char* bccFile = bccFiles[i];
        if (bccFile[0] != 0 && !addFileToChecksum(bccFile, checksum)) {
            // return empty checksum instead of something partial/corrupt
            return 0;
        }
    }

    return checksum;
}

#endif  // !RS_COMPATIBILITY_LIB

RsdCpuScriptImpl::RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s) {
    mCtx = ctx;
    mScript = s;

    mScriptSO = nullptr;

    mRoot = nullptr;
    mRootExpand = nullptr;
    mInit = nullptr;
    mFreeChildren = nullptr;
    mScriptExec = nullptr;

    mBoundAllocs = nullptr;
    mIntrinsicData = nullptr;
    mIsThreadable = true;

    mBuildChecksum = 0;
    mChecksumNeeded = false;
}

bool RsdCpuScriptImpl::storeRSInfoFromSO() {
    // The shared object may have an invalid build checksum.
    // Validate and fail early.
    mScriptExec = ScriptExecutable::createFromSharedObject(
            mScriptSO, mChecksumNeeded ? mBuildChecksum : 0);

    if (mScriptExec == nullptr) {
        return false;
    }

    mRoot = (RootFunc_t) dlsym(mScriptSO, "root");
    if (mRoot) {
        //ALOGE("Found root(): %p", mRoot);
    }
    mRootExpand = (RootFunc_t) dlsym(mScriptSO, "root.expand");
    if (mRootExpand) {
        //ALOGE("Found root.expand(): %p", mRootExpand);
    }
    mInit = (InitOrDtorFunc_t) dlsym(mScriptSO, "init");
    if (mInit) {
        //ALOGE("Found init(): %p", mInit);
    }
    mFreeChildren = (InitOrDtorFunc_t) dlsym(mScriptSO, ".rs.dtor");
    if (mFreeChildren) {
        //ALOGE("Found .rs.dtor(): %p", mFreeChildren);
    }

    size_t varCount = mScriptExec->getExportedVariableCount();
    if (varCount > 0) {
        mBoundAllocs = new Allocation *[varCount];
        memset(mBoundAllocs, 0, varCount * sizeof(*mBoundAllocs));
    }

    mIsThreadable = mScriptExec->getThreadable();
    //ALOGE("Script isThreadable? %d", mIsThreadable);

    if (kDebugGlobalVariables) {
        mScriptExec->dumpGlobalInfo();
    }

    return true;
}

bool RsdCpuScriptImpl::init(char const *resName, char const *cacheDir,
                            uint8_t const *bitcode, size_t bitcodeSize,
                            uint32_t flags, char const *bccPluginName) {
    //ALOGE("rsdScriptCreate %p %p %p %p %i %i %p", rsc, resName, cacheDir,
    // bitcode, bitcodeSize, flags, lookupFunc);
    //ALOGE("rsdScriptInit %p %p", rsc, script);

    mCtx->lockMutex();
#ifndef RS_COMPATIBILITY_LIB
    bool useRSDebugContext = false;

    bcinfo::MetadataExtractor bitcodeMetadata((const char *) bitcode, bitcodeSize);
    if (!bitcodeMetadata.extract()) {
        ALOGE("Could not extract metadata from bitcode");
        mCtx->unlockMutex();
        return false;
    }

    const char* core_lib = findCoreLib(bitcodeMetadata, (const char*)bitcode, bitcodeSize);

    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        useRSDebugContext = true;
    }

    int optLevel = mCtx->getContext()->getOptLevel();

    std::string bcFileName(cacheDir);
    bcFileName.append("/");
    bcFileName.append(resName);
    bcFileName.append(".bc");

    std::vector<const char*> compileArguments;
    bool emitGlobalInfo = mCtx->getEmbedGlobalInfo();
    bool emitGlobalInfoSkipConstant = mCtx->getEmbedGlobalInfoSkipConstant();
    setCompileArguments(&compileArguments, bcFileName, cacheDir, resName, core_lib,
                        useRSDebugContext, bccPluginName, emitGlobalInfo,
                        optLevel, emitGlobalInfoSkipConstant);

    mChecksumNeeded = isChecksumNeeded(cacheDir);
    if (mChecksumNeeded) {
        std::vector<const char *> bccFiles = { BCC_EXE_PATH,
                                               core_lib,
                                             };

        // The last argument of compileArguments is a nullptr, so remove 1 from
        // the size.
        std::unique_ptr<const char> compileCommandLine(
            rsuJoinStrings(compileArguments.size()-1, compileArguments.data()));

        mBuildChecksum = constructBuildChecksum(bitcode, bitcodeSize,
                                                compileCommandLine.get(),
                                                bccFiles.data(), bccFiles.size());

        if (mBuildChecksum == 0) {
            // cannot compute checksum but verification is enabled
            mCtx->unlockMutex();
            return false;
        }
    }
    else {
        // add a dummy/constant as a checksum if verification is disabled
        mBuildChecksum = 0xabadcafe;
    }

    // Append build checksum to commandline
    // Handle the terminal nullptr in compileArguments
    compileArguments.pop_back();
    compileArguments.push_back("-build-checksum");
    std::stringstream ss;
    ss << std::hex << mBuildChecksum;
    std::string checksumStr(ss.str());
    compileArguments.push_back(checksumStr.c_str());
    compileArguments.push_back(nullptr);

    const bool reuse = !is_force_recompile() && !useRSDebugContext;
    if (reuse) {
        mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);

        // Read RS info from the shared object to detect checksum mismatch
        if (mScriptSO != nullptr && !storeRSInfoFromSO()) {
            dlclose(mScriptSO);
            mScriptSO = nullptr;
        }
    }

    // If reuse is desired and we can't, it's either not there or out of date.
    // We compile the bit code and try loading again.
    if (mScriptSO == nullptr) {
        if (!compileBitcode(bcFileName, (const char*)bitcode, bitcodeSize,
                            compileArguments))
        {
            ALOGE("bcc: FAILS to compile '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        std::string SOPath;

        if (!SharedLibraryUtils::createSharedLibrary(
                mCtx->getContext()->getDriverName(), cacheDir, resName, reuse,
                &SOPath)) {
            ALOGE("Linker: Failed to link object file '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        if (reuse) {
            mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);
        } else {
            mScriptSO = SharedLibraryUtils::loadAndDeleteSharedLibrary(SOPath.c_str());
        }
        if (mScriptSO == nullptr) {
            ALOGE("Unable to load '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        // Read RS symbol information from the .so.
        if (!storeRSInfoFromSO()) {
            goto error;
        }
    }

    mBitcodeFilePath.assign(bcFileName.c_str());

#else  // RS_COMPATIBILITY_LIB is defined
    const char *nativeLibDir = mCtx->getContext()->getNativeLibDir();
    mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName, nativeLibDir);

    if (!mScriptSO) {
        goto error;
    }

    if (!storeRSInfoFromSO()) {
        goto error;
    }
#endif
    mCtx->unlockMutex();
    return true;

error:

    mCtx->unlockMutex();
    if (mScriptSO) {
        dlclose(mScriptSO);
        mScriptSO = nullptr;
    }
    return false;
}

#ifndef RS_COMPATIBILITY_LIB

const char* RsdCpuScriptImpl::findCoreLib(const bcinfo::MetadataExtractor& ME, const char* bitcode,
                                          size_t bitcodeSize) {
    const char* defaultLib = SYSLIBPATH_BC"/libclcore.bc";

    // If we're debugging, use the debug library.
    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        if (ME.hasDebugInfo()) {
            return SYSLIBPATH_BC"/libclcore_debug_g.bc";
        }
        return SYSLIBPATH_BC"/libclcore_debug.bc";
    }

    if (ME.hasDebugInfo()) {
        return SYSLIBPATH_BC"/libclcore_g.bc";
    }

    // If a callback has been registered to specify a library, use that.
    RSSelectRTCallback selectRTCallback = mCtx->getSelectRTCallback();
    if (selectRTCallback != nullptr) {
        return selectRTCallback((const char*)bitcode, bitcodeSize);
    }

    // Check for a platform specific library
#if defined(ARCH_ARM_HAVE_NEON) && !defined(DISABLE_CLCORE_NEON)
    enum bcinfo::RSFloatPrecision prec = ME.getRSFloatPrecision();
    if (prec == bcinfo::RS_FP_Relaxed) {
        // NEON-capable ARMv7a devices can use an accelerated math library
        // for all reduced precision scripts.
        // ARMv8 does not use NEON, as ASIMD can be used with all precision
        // levels.
        return SYSLIBPATH_BC"/libclcore_neon.bc";
    } else {
        return defaultLib;
    }
#elif defined(__i386__) || defined(__x86_64__)
    // x86 devices will use an optimized library.
    return SYSLIBPATH_BC"/libclcore_x86.bc";
#else
    return defaultLib;
#endif
}

#endif

void RsdCpuScriptImpl::populateScript(Script *script) {
    // Copy info over to runtime
    script->mHal.info.exportedFunctionCount = mScriptExec->getExportedFunctionCount();
    script->mHal.info.exportedReduceCount = mScriptExec->getExportedReduceCount();
    script->mHal.info.exportedForEachCount = mScriptExec->getExportedForEachCount();
    script->mHal.info.exportedVariableCount = mScriptExec->getExportedVariableCount();
    script->mHal.info.exportedPragmaCount = mScriptExec->getPragmaCount();;
    script->mHal.info.exportedPragmaKeyList = mScriptExec->getPragmaKeys();
    script->mHal.info.exportedPragmaValueList = mScriptExec->getPragmaValues();

    // Bug, need to stash in metadata
    if (mRootExpand) {
        script->mHal.info.root = mRootExpand;
    } else {
        script->mHal.info.root = mRoot;
    }
}

// Set up the launch dimensions, and write the values of the launch
// dimensions into the mtls start/end fields.
//
// Inputs:
//    baseDim - base shape of the input
//         sc - used to constrain the launch dimensions
//
// Returns:
//   True on success, false on failure to set up
bool RsdCpuScriptImpl::setUpMtlsDimensions(MTLaunchStructCommon *mtls,
                                           const RsLaunchDimensions &baseDim,
                                           const RsScriptCall *sc) {
    rsAssert(mtls);

#define SET_UP_DIMENSION(DIM_FIELD, SC_FIELD) do {            \
    if (!sc || (sc->SC_FIELD##End == 0)) {                    \
        mtls->end.DIM_FIELD = baseDim.DIM_FIELD;              \
    } else {                                                  \
        mtls->start.DIM_FIELD =                               \
            rsMin(baseDim.DIM_FIELD, sc->SC_FIELD##Start);    \
        mtls->end.DIM_FIELD =                                 \
            rsMin(baseDim.DIM_FIELD, sc->SC_FIELD##End);      \
        if (mtls->start.DIM_FIELD >= mtls->end.DIM_FIELD) {   \
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT, \
                "Failed to launch kernel; Invalid "           \
                #SC_FIELD "Start or " #SC_FIELD "End.");      \
            return false;                                     \
        }                                                     \
    }} while(0)

    SET_UP_DIMENSION(x, x);
    SET_UP_DIMENSION(y, y);
    SET_UP_DIMENSION(z, z);
    // Checks and setup of fields other than x, y, z are ignored, since those
    // fields are not used in the runtime and are not visible in the Java API.
#undef SET_UP_DIMENSION

    return true;
}

// Preliminary work to prepare a general reduce-style kernel for launch.
bool RsdCpuScriptImpl::reduceMtlsSetup(const Allocation ** ains,
                                       uint32_t inLen,
                                       const Allocation * aout,
                                       const RsScriptCall *sc,
                                       MTLaunchStructReduce *mtls) {
    rsAssert(ains && (inLen >= 1) && aout);
    memset(mtls, 0, sizeof(MTLaunchStructReduce));
    mtls->dimPtr = &mtls->redp.dim;

    for (int index = inLen; --index >= 0;) {
        if (allocationLODIsNull(ains[index])) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                         "reduce called with null in allocations");
            return false;
        }
    }

    if (allocationLODIsNull(aout)) {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "reduce called with null out allocation");
        return false;
    }

    const Allocation *ain0   = ains[0];
    const Type       *inType = ain0->getType();

    mtls->redp.dim.x = inType->getDimX();
    mtls->redp.dim.y = inType->getDimY();
    mtls->redp.dim.z = inType->getDimZ();

    for (int Index = inLen; --Index >= 1;) {
        if (!ain0->hasSameDims(ains[Index])) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                         "Failed to launch reduction kernel;"
                                         "dimensions of input allocations do not match.");
            return false;
        }
    }

    if (!setUpMtlsDimensions(mtls, mtls->redp.dim, sc)) {
        return false;
    }

    // The X & Y walkers always want 0-1 min even if dim is not present
    mtls->end.x = rsMax((uint32_t)1, mtls->end.x);
    mtls->end.y = rsMax((uint32_t)1, mtls->end.y);

    mtls->rs = mCtx;

    mtls->mSliceNum    = 0;
    mtls->mSliceSize   = 1;
    mtls->isThreadable = mIsThreadable;

    // Set up output,
    mtls->redp.outLen = 1;
    mtls->redp.outPtr[0] = (uint8_t *)aout->mHal.drvState.lod[0].mallocPtr;
    mtls->redp.outStride[0] = aout->getType()->getElementSizeBytes();

    // Set up input.
    memcpy(mtls->ains, ains, inLen * sizeof(ains[0]));
    mtls->redp.inLen = inLen;
    for (int index = inLen; --index >= 0;) {
        mtls->redp.inPtr[index] = (const uint8_t*)ains[index]->mHal.drvState.lod[0].mallocPtr;
        mtls->redp.inStride[index] = ains[index]->getType()->getElementSizeBytes();
    }

    // All validation passed, ok to launch threads
    return true;
}

// Preliminary work to prepare a forEach-style kernel for launch.
bool RsdCpuScriptImpl::forEachMtlsSetup(const Allocation ** ains,
                                        uint32_t inLen,
                                        Allocation * aout,
                                        const void * usr, uint32_t usrLen,
                                        const RsScriptCall *sc,
                                        MTLaunchStructForEach *mtls) {
    if (ains == nullptr && inLen != 0) {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
          "rsForEach called with none-zero inLen with null in allocations");
        return false;
    }

    memset(mtls, 0, sizeof(MTLaunchStructForEach));
    mtls->dimPtr = &mtls->fep.dim;

    for (int index = inLen; --index >= 0;) {
        if (allocationLODIsNull(ains[index])) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                         "rsForEach called with null in allocations");
            return false;
        }
    }

    if (allocationLODIsNull(aout)) {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null out allocations");
        return false;
    }

    // The only situation where ains[j] is null is when inLen==1 and j==0;
    // and that can only happen for an old-style kernel in API level 11~13,
    // where the input allocation cannot be skipped if the output allocation is specified.
    if (inLen != 0)
        rsAssert((inLen == 1) || (ains[0] != nullptr));

    if (inLen > 0 && ains[0]) {
        const Allocation *ain0   = ains[0];
        const Type       *inType = ain0->getType();

        mtls->fep.dim.x = inType->getDimX();
        mtls->fep.dim.y = inType->getDimY();
        mtls->fep.dim.z = inType->getDimZ();

        for (int Index = inLen; --Index >= 1;) {
            if (!ain0->hasSameDims(ains[Index])) {
                mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                  "Failed to launch kernel; dimensions of input "
                  "allocations do not match.");
                return false;
            }
        }
    } else if (aout != nullptr) {
        const Type *outType = aout->getType();

        mtls->fep.dim.x = outType->getDimX();
        mtls->fep.dim.y = outType->getDimY();
        mtls->fep.dim.z = outType->getDimZ();

    } else if (sc != nullptr) {
        mtls->fep.dim.x = sc->xEnd;
        mtls->fep.dim.y = sc->yEnd;
        mtls->fep.dim.z = 0;
    } else {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null allocations");
        return false;
    }

    if (inLen > 0 && aout != nullptr) {
        if (ains[0] && !ains[0]->hasSameDims(aout)) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
              "Failed to launch kernel; dimensions of input and output allocations do not match.");

            return false;
        }
    }

    if (!setUpMtlsDimensions(mtls, mtls->fep.dim, sc)) {
        return false;
    }

    // The X & Y walkers always want 0-1 min even if dim is not present
    mtls->end.x    = rsMax((uint32_t)1, mtls->end.x);
    mtls->end.y    = rsMax((uint32_t)1, mtls->end.y);
    mtls->rs       = mCtx;
    if (ains) {
        memcpy(mtls->ains, ains, inLen * sizeof(ains[0]));
    }
    mtls->aout[0]    = aout;
    mtls->fep.usr    = usr;
    mtls->fep.usrLen = usrLen;
    mtls->mSliceSize = 1;
    mtls->mSliceNum  = 0;

    mtls->isThreadable  = mIsThreadable;

    if (inLen > 0) {
        mtls->fep.inLen = inLen;
        for (int index = inLen; --index >= 0;) {
            if (ains[index] == nullptr) {
                // In old style kernels, the first and only input allocation could be null.
                // Not allowed in newer styles.
                rsAssert(inLen == 1 && index == 0);
                continue;
            }
            mtls->fep.inPtr[index] = (const uint8_t*)ains[index]->mHal.drvState.lod[0].mallocPtr;
            mtls->fep.inStride[index] = ains[index]->getType()->getElementSizeBytes();
        }
    }

    if (aout != nullptr) {
        mtls->fep.outPtr[0] = (uint8_t *)aout->mHal.drvState.lod[0].mallocPtr;
        mtls->fep.outStride[0] = aout->getType()->getElementSizeBytes();
    }

    // All validation passed, ok to launch threads
    return true;
}


void RsdCpuScriptImpl::invokeForEach(uint32_t slot,
                                     const Allocation ** ains,
                                     uint32_t inLen,
                                     Allocation * aout,
                                     const void * usr,
                                     uint32_t usrLen,
                                     const RsScriptCall *sc) {

    MTLaunchStructForEach mtls;

    if (forEachMtlsSetup(ains, inLen, aout, usr, usrLen, sc, &mtls)) {
        forEachKernelSetup(slot, &mtls);

        RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
        mCtx->launchForEach(ains, inLen, aout, sc, &mtls);
        mCtx->setTLS(oldTLS);
    }
}

void RsdCpuScriptImpl::invokeReduce(uint32_t slot,
                                    const Allocation ** ains, uint32_t inLen,
                                    Allocation *aout,
                                    const RsScriptCall *sc) {
  MTLaunchStructReduce mtls;

  if (reduceMtlsSetup(ains, inLen, aout, sc, &mtls)) {
    reduceKernelSetup(slot, &mtls);
    RsdCpuScriptImpl *oldTLS = mCtx->setTLS(this);
    mCtx->launchReduce(ains, inLen, aout, &mtls);
    mCtx->setTLS(oldTLS);
  }
}

void RsdCpuScriptImpl::forEachKernelSetup(uint32_t slot, MTLaunchStructForEach *mtls) {
    mtls->script = this;
    mtls->fep.slot = slot;
    mtls->kernel = mScriptExec->getForEachFunction(slot);
    rsAssert(mtls->kernel != nullptr);
}

void RsdCpuScriptImpl::reduceKernelSetup(uint32_t slot, MTLaunchStructReduce *mtls) {
    mtls->script = this;
    mtls->redp.slot = slot;

    const ReduceDescription *desc = mScriptExec->getReduceDescription(slot);
    mtls->accumFunc = desc->accumFunc;
    mtls->initFunc  = desc->initFunc;   // might legally be nullptr
    mtls->combFunc  = desc->combFunc;   // might legally be nullptr
    mtls->outFunc   = desc->outFunc;    // might legally be nullptr
    mtls->accumSize = desc->accumSize;

    rsAssert(mtls->accumFunc != nullptr);
}

int RsdCpuScriptImpl::invokeRoot() {
    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    int ret = mRoot();
    mCtx->setTLS(oldTLS);
    return ret;
}

void RsdCpuScriptImpl::invokeInit() {
    if (mInit) {
        mInit();
    }
}

void RsdCpuScriptImpl::invokeFreeChildren() {
    if (mFreeChildren) {
        mFreeChildren();
    }
}

void RsdCpuScriptImpl::invokeFunction(uint32_t slot, const void *params,
                                      size_t paramLength) {
    //ALOGE("invoke %i %p %zu", slot, params, paramLength);
    void * ap = nullptr;

#if defined(__x86_64__)
    // The invoked function could have input parameter of vector type for example float4 which
    // requires void* params to be 16 bytes aligned when using SSE instructions for x86_64 platform.
    // So try to align void* params before passing them into RS exported function.

    if ((uint8_t)(uint64_t)params & 0x0F) {
        if ((ap = (void*)memalign(16, paramLength)) != nullptr) {
            memcpy(ap, params, paramLength);
        } else {
            ALOGE("x86_64: invokeFunction memalign error, still use params which"
                  " is not 16 bytes aligned.");
        }
    }
#endif

    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    reinterpret_cast<void (*)(const void *, uint32_t)>(
        mScriptExec->getInvokeFunction(slot))(ap? (const void *) ap: params, paramLength);

#if defined(__x86_64__)
    free(ap);
#endif

    mCtx->setTLS(oldTLS);
}

void RsdCpuScriptImpl::setGlobalVar(uint32_t slot, const void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalVar %i %p %zu", slot, data, dataLength);

    //if (mIntrinsicID) {
        //mIntrinsicFuncs.setVar(dc, script, drv->mIntrinsicData, slot, data, dataLength);
        //return;
    //}

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::getGlobalVar(uint32_t slot, void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("getGlobalVar %i %p %zu", slot, data, dataLength);

    int32_t *srcPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!srcPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }
    memcpy(data, srcPtr, dataLength);
}


void RsdCpuScriptImpl::setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                                const Element *elem,
                                                const uint32_t *dims, size_t dimLength) {
    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    // We want to look at dimension in terms of integer components,
    // but dimLength is given in terms of bytes.
    dimLength /= sizeof(int);

    // Only a single dimension is currently supported.
    rsAssert(dimLength == 1);
    if (dimLength == 1) {
        // First do the increment loop.
        size_t stride = elem->getSizeBytes();
        const char *cVal = reinterpret_cast<const char *>(data);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->incRefs(cVal);
            cVal += stride;
        }

        // Decrement loop comes after (to prevent race conditions).
        char *oldVal = reinterpret_cast<char *>(destPtr);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->decRefs(oldVal);
            oldVal += stride;
        }
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::setGlobalBind(uint32_t slot, Allocation *data) {

    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalBind %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    void *ptr = nullptr;
    mBoundAllocs[slot] = data;
    if (data) {
        ptr = data->mHal.drvState.lod[0].mallocPtr;
    }
    memcpy(destPtr, &ptr, sizeof(void *));
}

void RsdCpuScriptImpl::setGlobalObj(uint32_t slot, ObjectBase *data) {

    //rsAssert(script->mFieldIsObject[slot]);
    //ALOGE("setGlobalObj %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    rsrSetObject(mCtx->getContext(), (rs_object_base *)destPtr, data);
}

const char* RsdCpuScriptImpl::getFieldName(uint32_t slot) const {
    return mScriptExec->getFieldName(slot);
}

RsdCpuScriptImpl::~RsdCpuScriptImpl() {
    delete mScriptExec;
    delete[] mBoundAllocs;
    if (mScriptSO) {
        dlclose(mScriptSO);
    }
}

Allocation * RsdCpuScriptImpl::getAllocationForPointer(const void *ptr) const {
    if (!ptr) {
        return nullptr;
    }

    for (uint32_t ct=0; ct < mScript->mHal.info.exportedVariableCount; ct++) {
        Allocation *a = mBoundAllocs[ct];
        if (!a) continue;
        if (a->mHal.drvState.lod[0].mallocPtr == ptr) {
            return a;
        }
    }
    ALOGE("rsGetAllocation, failed to find %p", ptr);
    return nullptr;
}

int RsdCpuScriptImpl::getGlobalEntries() const {
    return mScriptExec->getGlobalEntries();
}

const char * RsdCpuScriptImpl::getGlobalName(int i) const {
    return mScriptExec->getGlobalName(i);
}

const void * RsdCpuScriptImpl::getGlobalAddress(int i) const {
    return mScriptExec->getGlobalAddress(i);
}

size_t RsdCpuScriptImpl::getGlobalSize(int i) const {
    return mScriptExec->getGlobalSize(i);
}

uint32_t RsdCpuScriptImpl::getGlobalProperties(int i) const {
    return mScriptExec->getGlobalProperties(i);
}

void RsdCpuScriptImpl::preLaunch(uint32_t slot, const Allocation ** ains,
                                 uint32_t inLen, Allocation * aout,
                                 const void * usr, uint32_t usrLen,
                                 const RsScriptCall *sc) {}

void RsdCpuScriptImpl::postLaunch(uint32_t slot, const Allocation ** ains,
                                  uint32_t inLen, Allocation * aout,
                                  const void * usr, uint32_t usrLen,
                                  const RsScriptCall *sc) {}


} // namespace renderscript
} // namespace android
