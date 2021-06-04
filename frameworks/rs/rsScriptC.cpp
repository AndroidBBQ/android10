/*
 * Copyright (C) 2009-2012 The Android Open Source Project
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

#include "rsContext.h"
#include "rsScriptC.h"

#if !defined(RS_COMPATIBILITY_LIB) && !defined(ANDROID_RS_SERIALIZE)
#include <bcinfo/BitcodeTranslator.h>
#include <bcinfo/BitcodeWrapper.h>
#endif

#include <sys/stat.h>

#include <sstream>
#include <string>

#ifdef _WIN32
/* Define the default path separator for the platform. */
#define OS_PATH_SEPARATOR     '\\'
#define OS_PATH_SEPARATOR_STR "\\"

#else /* not _WIN32 */

/* Define the default path separator for the platform. */
#define OS_PATH_SEPARATOR     '/'
#define OS_PATH_SEPARATOR_STR "/"

#endif

using android::renderscript::ScriptC;

#define GET_TLS()  Context::ScriptTLSStruct * tls = \
    (Context::ScriptTLSStruct *)pthread_getspecific(Context::gThreadTLSKey); \
    Context * rsc = tls->mContext; \
    ScriptC * sc = (ScriptC *) tls->mScript

ScriptC::ScriptC(Context *rsc) : Script(rsc) {
}

ScriptC::~ScriptC() {
    if (mInitialized) {
        mRSC->mHal.funcs.script.invokeFreeChildren(mRSC, this);
        mRSC->mHal.funcs.script.destroy(mRSC, this);
    }
}

#ifndef RS_COMPATIBILITY_LIB
bool ScriptC::createCacheDir(const char *cacheDir) {
    std::string currentDir;
    const std::string cacheDirString(cacheDir);

    struct stat statBuf;
    int statReturn = stat(cacheDir, &statBuf);
    if (!statReturn) {
        return true;
    }

    // Start from the beginning of the cacheDirString.
    int currPos = 0;

    // Reserve space in currentDir for the entire cacheDir path.
    currentDir.reserve(cacheDirString.length());

    while (currPos >= 0) {
        /*
         * The character at currPos should be a path separator.  We need to look
         * for the next one.
         */
        int nextPos = cacheDirString.find(OS_PATH_SEPARATOR, currPos + 1);

        if (nextPos > 0) {
            // A new path separator has been found.
            currentDir += cacheDirString.substr(currPos, nextPos - currPos);
        } else {
            // There are no more path separators.
            currentDir += cacheDirString.substr(currPos);
        }

        currPos = nextPos;

        statReturn = stat(currentDir.c_str(), &statBuf);

        if (statReturn) {
            if (errno == ENOENT) {
                if (mkdir(currentDir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR)) {
                    ALOGE("Couldn't create cache directory: %s",
                          currentDir.c_str());
                    ALOGE("Error: %s", strerror(errno));
                    return false;
                }
            } else {
                ALOGE("Stat error: %s", strerror(errno));
                return false;
            }
        }
    }
    return true;
}
#endif

void ScriptC::setupScript(Context *rsc) {
    mEnviroment.mStartTimeMillis
                = nanoseconds_to_milliseconds(systemTime(SYSTEM_TIME_MONOTONIC));

    for (uint32_t ct=0; ct < mHal.info.exportedVariableCount; ct++) {
        if (mSlots[ct].get() && !mTypes[ct].get()) {
            mTypes[ct].set(mSlots[ct]->getType());
        }

        if (!mTypes[ct].get())
            continue;
        rsc->mHal.funcs.script.setGlobalBind(rsc, this, ct, mSlots[ct].get());
    }
}

void ScriptC::setupGLState(Context *rsc) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (mEnviroment.mFragmentStore.get()) {
        rsc->setProgramStore(mEnviroment.mFragmentStore.get());
    }
    if (mEnviroment.mFragment.get()) {
        rsc->setProgramFragment(mEnviroment.mFragment.get());
    }
    if (mEnviroment.mVertex.get()) {
        rsc->setProgramVertex(mEnviroment.mVertex.get());
    }
    if (mEnviroment.mRaster.get()) {
        rsc->setProgramRaster(mEnviroment.mRaster.get());
    }
#endif
}

uint32_t ScriptC::run(Context *rsc) {
    if (mHal.info.root == nullptr) {
        rsc->setError(RS_ERROR_BAD_SCRIPT, "Attempted to run bad script");
        return 0;
    }

    setupGLState(rsc);
    setupScript(rsc);

    uint32_t ret = 0;

    if (rsc->props.mLogScripts) {
        ALOGV("%p ScriptC::run invoking root,  ptr %p", rsc, mHal.info.root);
    }

    ret = rsc->mHal.funcs.script.invokeRoot(rsc, this);

    if (rsc->props.mLogScripts) {
        ALOGV("%p ScriptC::run invoking complete, ret=%i", rsc, ret);
    }

    return ret;
}


void ScriptC::runForEach(Context *rsc,
                         uint32_t slot,
                         const Allocation ** ains,
                         size_t inLen,
                         Allocation * aout,
                         const void * usr,
                         size_t usrBytes,
                         const RsScriptCall *sc) {
    if (slot >= mHal.info.exportedForEachCount) {
        rsc->setError(RS_ERROR_BAD_SCRIPT,
                      "The forEach kernel index is out of bounds");
        return;
    }

    // Trace this function call.
    // To avoid overhead we only build the string if tracing is actually
    // enabled.
    std::stringstream ss;
    if (ATRACE_ENABLED()) {
        ss << "runForEach slot[" << slot << "]";
    }
    std::string msgStr(ss.str());
    ATRACE_NAME(msgStr.c_str());

    if (mRSC->hadFatalError()) return;

    Context::PushState ps(rsc);

    setupGLState(rsc);
    setupScript(rsc);

    if (rsc->props.mLogScripts) {
        ALOGV("%p ScriptC::runForEach invoking slot %i, ptr %p", rsc, slot, this);
    }

    if (rsc->mHal.funcs.script.invokeForEachMulti != nullptr) {
        rsc->mHal.funcs.script.invokeForEachMulti(rsc, this, slot, ains, inLen,
                                                  aout, usr, usrBytes, sc);

    } else if (inLen == 1) {
        rsc->mHal.funcs.script.invokeForEach(rsc, this, slot, ains[0], aout,
                                             usr, usrBytes, sc);

    } else {
        rsc->setError(RS_ERROR_FATAL_DRIVER,
                      "Driver support for multi-input not present");
    }
}

void ScriptC::runReduce(Context *rsc, uint32_t slot,
                        const Allocation ** ains, size_t inLen,
                        Allocation *aout, const RsScriptCall *sc) {
  // TODO: Record the name of the kernel in the tracing information.
  ATRACE_CALL();

  if (slot >= mHal.info.exportedReduceCount) {
      rsc->setError(RS_ERROR_BAD_SCRIPT, "The general reduce kernel index is out of bounds");
      return;
  }
  if (mRSC->hadFatalError()) return;

  setupScript(rsc);

  if (rsc->props.mLogScripts) {
      ALOGV("%p ScriptC::runReduce invoking slot %i, ptr %p", rsc, slot, this);
  }

  rsc->mHal.funcs.script.invokeReduce(rsc, this, slot, ains, inLen, aout, sc);
}

void ScriptC::Invoke(Context *rsc, uint32_t slot, const void *data, size_t len) {
    ATRACE_CALL();

    if (slot >= mHal.info.exportedFunctionCount) {
        rsc->setError(RS_ERROR_BAD_SCRIPT, "The invokable index is out of bounds");
        return;
    }
    if (mRSC->hadFatalError()) return;

    setupScript(rsc);

    if (rsc->props.mLogScripts) {
        ALOGV("%p ScriptC::Invoke invoking slot %i,  ptr %p", rsc, slot, this);
    }
    rsc->mHal.funcs.script.invokeFunction(rsc, this, slot, data, len);
}

static const bool kDebugBitcode = false;

#ifndef RS_COMPATIBILITY_LIB

static bool dumpBitcodeFile(const char *cacheDir, const char *resName,
                            const char *suffix, const uint8_t *bitcode,
                            size_t bitcodeLen) {
    std::string f(cacheDir);
    f.append("/");
    f.append(resName);
    f.append("#");
    f.append(suffix);
    f.append(".bc");

    if (!ScriptC::createCacheDir(cacheDir)) {
        return false;
    }

    FILE *fp = fopen(f.c_str(), "we");
    if (!fp) {
        ALOGE("Could not open %s", f.c_str());
        return false;
    }

    size_t nWritten = fwrite(bitcode, 1, bitcodeLen, fp);
    fclose(fp);
    if (nWritten != bitcodeLen) {
        ALOGE("Could not write %s", f.c_str());
        return false;
    }
    return true;
}

#endif  // !RS_COMPATIBILITY_LIB


bool ScriptC::runCompiler(Context *rsc,
                          const char *resName,
                          const char *cacheDir,
                          const uint8_t *bitcode,
                          size_t bitcodeLen) {
    ATRACE_CALL();
    //ALOGE("runCompiler %p %p %p %p %p %i", rsc, this, resName, cacheDir, bitcode, bitcodeLen);
#ifndef RS_COMPATIBILITY_LIB
    uint32_t sdkVersion = 0;
    bcinfo::BitcodeWrapper bcWrapper((const char *)bitcode, bitcodeLen);
    if (!bcWrapper.unwrap()) {
        ALOGE("Bitcode is not in proper container format (raw or wrapper)");
        return false;
    }

    if (bcWrapper.getBCFileType() == bcinfo::BC_WRAPPER) {
        sdkVersion = bcWrapper.getTargetAPI();
    }

    if (sdkVersion == 0) {
        // This signals that we didn't have a wrapper containing information
        // about the bitcode.
        sdkVersion = rsc->getTargetSdkVersion();
    }

    // Save off the sdkVersion, so that we can handle broken cases later.
    // Bug 19734267
    mApiLevel = sdkVersion;

    bcinfo::BitcodeTranslator BT((const char *)bitcode, bitcodeLen,
                                 sdkVersion);
    if (!BT.translate()) {
        ALOGE("Failed to translate bitcode from version: %u", sdkVersion);
        return false;
    }
    bitcode = (const uint8_t *) BT.getTranslatedBitcode();
    bitcodeLen = BT.getTranslatedBitcodeSize();

    if (kDebugBitcode) {
        if (!dumpBitcodeFile(cacheDir, resName, "after", bitcode, bitcodeLen)) {
            return false;
        }
    }


    // Set the optimization level of bcc to be the same as the
    // optimization level used to compile the bitcode.
    rsc->setOptLevel(bcWrapper.getOptimizationLevel());

    if (!cacheDir) {
        // MUST BE FIXED BEFORE ANYTHING USING C++ API IS RELEASED
        cacheDir = getenv("EXTERNAL_STORAGE");
        ALOGV("Cache dir changed to %s", cacheDir);
    }

    // ensure that cache dir exists
    if (cacheDir && !createCacheDir(cacheDir)) {
      return false;
    }
#endif

    if (!rsc->mHal.funcs.script.init(rsc, this, resName, cacheDir, bitcode, bitcodeLen, 0)) {
        return false;
    }

    mInitialized = true;
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    mEnviroment.mFragment.set(rsc->getDefaultProgramFragment());
    mEnviroment.mVertex.set(rsc->getDefaultProgramVertex());
    mEnviroment.mFragmentStore.set(rsc->getDefaultProgramStore());
    mEnviroment.mRaster.set(rsc->getDefaultProgramRaster());
#endif

    rsc->mHal.funcs.script.invokeInit(rsc, this);

    for (size_t i=0; i < mHal.info.exportedPragmaCount; ++i) {
        const char * key = mHal.info.exportedPragmaKeyList[i];
        const char * value = mHal.info.exportedPragmaValueList[i];
        //ALOGE("pragma %s %s", keys[i], values[i]);
        if (!strcmp(key, "version")) {
            if (!strcmp(value, "1")) {
                continue;
            }
            ALOGE("Invalid version pragma value: %s\n", value);
            return false;
        }

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        if (!strcmp(key, "stateVertex")) {
            if (!strcmp(value, "default")) {
                continue;
            }
            if (!strcmp(value, "parent")) {
                mEnviroment.mVertex.clear();
                continue;
            }
            ALOGE("Unrecognized value %s passed to stateVertex", value);
            return false;
        }

        if (!strcmp(key, "stateRaster")) {
            if (!strcmp(value, "default")) {
                continue;
            }
            if (!strcmp(value, "parent")) {
                mEnviroment.mRaster.clear();
                continue;
            }
            ALOGE("Unrecognized value %s passed to stateRaster", value);
            return false;
        }

        if (!strcmp(key, "stateFragment")) {
            if (!strcmp(value, "default")) {
                continue;
            }
            if (!strcmp(value, "parent")) {
                mEnviroment.mFragment.clear();
                continue;
            }
            ALOGE("Unrecognized value %s passed to stateFragment", value);
            return false;
        }

        if (!strcmp(key, "stateStore")) {
            if (!strcmp(value, "default")) {
                continue;
            }
            if (!strcmp(value, "parent")) {
                mEnviroment.mFragmentStore.clear();
                continue;
            }
            ALOGE("Unrecognized value %s passed to stateStore", value);
            return false;
        }
#endif

    }

    mSlots = new ObjectBaseRef<Allocation>[mHal.info.exportedVariableCount];
    mTypes = new ObjectBaseRef<const Type>[mHal.info.exportedVariableCount];

    return true;
}

namespace android {
namespace renderscript {

RsScript rsi_ScriptCCreate(Context *rsc,
                           const char *resName, size_t resName_length,
                           const char *cacheDir, size_t cacheDir_length,
                           const char *text, size_t text_length)
{
    ScriptC *s = new ScriptC(rsc);

    if (!s->runCompiler(rsc, resName, cacheDir, (uint8_t *)text, text_length)) {
        // Error during compile, destroy s and return null.
        ObjectBase::checkDelete(s);
        return nullptr;
    }

    s->incUserRef();
    return s;
}

} // namespace renderscript
} // namespace android
