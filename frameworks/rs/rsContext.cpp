/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "rs.h"
#include "rsDevice.h"
#include "rsContext.h"
#include "rsThreadIO.h"

#include "rsgApiStructs.h"

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
#include "rsMesh.h"
#endif

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/syscall.h>
#include <string.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#endif

namespace android {
namespace renderscript {

pthread_mutex_t Context::gInitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Context::gMessageMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Context::gLibMutex = PTHREAD_MUTEX_INITIALIZER;

bool Context::initGLThread() {
    pthread_mutex_lock(&gInitMutex);

    int32_t ret = mHal.funcs.initGraphics(this);
    if (ret < 0) {
        pthread_mutex_unlock(&gInitMutex);
        ALOGE("%p initGraphics failed", this);
        return false;
    }

    mSyncFd = ret;
    pthread_mutex_unlock(&gInitMutex);
    return true;
}

void Context::deinitEGL() {
#ifndef RS_COMPATIBILITY_LIB
    mHal.funcs.shutdownGraphics(this);
#endif
}

Context::PushState::PushState(Context *con) {
    mRsc = con;
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (con->mIsGraphicsContext) {
        mFragment.set(con->getProgramFragment());
        mVertex.set(con->getProgramVertex());
        mStore.set(con->getProgramStore());
        mRaster.set(con->getProgramRaster());
        mFont.set(con->getFont());
    }
#endif
}

Context::PushState::~PushState() {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (mRsc->mIsGraphicsContext) {
        mRsc->setProgramFragment(mFragment.get());
        mRsc->setProgramVertex(mVertex.get());
        mRsc->setProgramStore(mStore.get());
        mRsc->setProgramRaster(mRaster.get());
        mRsc->setFont(mFont.get());
    }
#endif
}


uint32_t Context::runScript(Script *s) {
    PushState ps(this);

    uint32_t ret = s->run(this);
    return ret;
}

uint32_t Context::runRootScript() {
    timerSet(RS_TIMER_SCRIPT);
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    mStateFragmentStore.mLast.clear();
#endif
    watchdog.inRoot = true;
    uint32_t ret = runScript(mRootScript.get());
    watchdog.inRoot = false;

    return ret;
}

uint64_t Context::getTime() const {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_nsec + ((uint64_t)t.tv_sec * 1000 * 1000 * 1000);
}

void Context::timerReset() {
    for (int ct=0; ct < _RS_TIMER_TOTAL; ct++) {
        mTimers[ct] = 0;
    }
}

void Context::timerInit() {
    mTimeLast = getTime();
    mTimeFrame = mTimeLast;
    mTimeLastFrame = mTimeLast;
    mTimerActive = RS_TIMER_INTERNAL;
    mAverageFPSFrameCount = 0;
    mAverageFPSStartTime = mTimeLast;
    mAverageFPS = 0;
    timerReset();
}

void Context::timerFrame() {
    mTimeLastFrame = mTimeFrame;
    mTimeFrame = getTime();
    // Update average fps
    const uint64_t averageFramerateInterval = 1000 * 1000000;
    mAverageFPSFrameCount ++;
    uint64_t inverval = mTimeFrame - mAverageFPSStartTime;
    if (inverval >= averageFramerateInterval) {
        inverval = inverval / 1000000;
        mAverageFPS = (mAverageFPSFrameCount * 1000) / inverval;
        mAverageFPSFrameCount = 0;
        mAverageFPSStartTime = mTimeFrame;
    }
}

void Context::timerSet(Timers tm) {
    uint64_t last = mTimeLast;
    mTimeLast = getTime();
    mTimers[mTimerActive] += mTimeLast - last;
    mTimerActive = tm;
}

void Context::timerPrint() {
    double total = 0;
    for (int ct = 0; ct < _RS_TIMER_TOTAL; ct++) {
        total += mTimers[ct];
    }
    uint64_t frame = mTimeFrame - mTimeLastFrame;
    mTimeMSLastFrame = frame / 1000000;
    mTimeMSLastScript = mTimers[RS_TIMER_SCRIPT] / 1000000;
    mTimeMSLastSwap = mTimers[RS_TIMER_CLEAR_SWAP] / 1000000;


    if (props.mLogTimes) {
        ALOGV("RS: Frame (%i),   Script %2.1f%% (%i),  Swap %2.1f%% (%i),  Idle %2.1f%% (%" PRIi64 "),  "
              "Internal %2.1f%% (%" PRIi64 "), Avg fps: %u",
             mTimeMSLastFrame,
             100.0 * mTimers[RS_TIMER_SCRIPT] / total, mTimeMSLastScript,
             100.0 * mTimers[RS_TIMER_CLEAR_SWAP] / total, mTimeMSLastSwap,
             100.0 * mTimers[RS_TIMER_IDLE] / total, mTimers[RS_TIMER_IDLE] / 1000000,
             100.0 * mTimers[RS_TIMER_INTERNAL] / total, mTimers[RS_TIMER_INTERNAL] / 1000000,
             mAverageFPS);
    }
}

bool Context::setupCheck() {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    mFragmentStore->setup(this, &mStateFragmentStore);
    mFragment->setup(this, &mStateFragment);
    mRaster->setup(this, &mStateRaster);
    mVertex->setup(this, &mStateVertex);
    mFBOCache.setup(this);
#endif
    return true;
}

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
void Context::setupProgramStore() {
    mFragmentStore->setup(this, &mStateFragmentStore);
}
#endif

static uint32_t getProp(const char *str) {
#ifdef __ANDROID__
    char buf[PROP_VALUE_MAX];
    property_get(str, buf, "0");
    return atoi(buf);
#else
    return 0;
#endif
}

void Context::displayDebugStats() {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Avg fps %u, Frame %i ms, Script %i ms",
             mAverageFPS, mTimeMSLastFrame, mTimeMSLastScript);
    float oldR, oldG, oldB, oldA;
    mStateFont.getFontColor(&oldR, &oldG, &oldB, &oldA);
    uint32_t bufferLen = strlen(buffer);

    ObjectBaseRef<Font> lastFont(getFont());
    setFont(nullptr);
    float shadowCol = 0.1f;
    mStateFont.setFontColor(shadowCol, shadowCol, shadowCol, 1.0f);
    mStateFont.renderText(buffer, bufferLen, 5, getHeight() - 6);

    mStateFont.setFontColor(1.0f, 0.7f, 0.0f, 1.0f);
    mStateFont.renderText(buffer, bufferLen, 4, getHeight() - 7);

    setFont(lastFont.get());
    mStateFont.setFontColor(oldR, oldG, oldB, oldA);
#endif
}

void * Context::threadProc(void *vrsc) {
    Context *rsc = static_cast<Context *>(vrsc);

    rsc->mNativeThreadId = gettid();
    rsc->props.mLogTimes = getProp("debug.rs.profile") != 0;
    rsc->props.mLogScripts = getProp("debug.rs.script") != 0;
    rsc->props.mLogShaders = getProp("debug.rs.shader") != 0;
    rsc->props.mLogShadersAttr = getProp("debug.rs.shader.attributes") != 0;
    rsc->props.mLogShadersUniforms = getProp("debug.rs.shader.uniforms") != 0;
    rsc->props.mLogVisual = getProp("debug.rs.visual") != 0;
    rsc->props.mLogReduce = getProp("debug.rs.reduce");
    rsc->props.mDebugReduceSplitAccum = getProp("debug.rs.reduce-split-accum") != 0;
    rsc->props.mDebugMaxThreads = getProp("debug.rs.max-threads");

    if (getProp("debug.rs.debug") != 0) {
        ALOGD("Forcing debug context due to debug.rs.debug.");
        rsc->mContextType = RS_CONTEXT_TYPE_DEBUG;
        rsc->mForceCpu = true;
    }

    bool forceRSoV = getProp("debug.rs.rsov") != 0;
    if (forceRSoV) {
        ALOGD("Force the use of RSoV driver");
        rsc->mForceRSoV = true;
    }

    bool forceCpu = getProp("debug.rs.default-CPU-driver") != 0;
    if (forceCpu) {
        ALOGD("Skipping hardware driver and loading default CPU driver");
        rsc->mForceCpu = true;
    }

    rsc->mForceCpu |= rsc->mIsGraphicsContext;
    if (!rsc->loadDriver(rsc->mForceCpu, rsc->mForceRSoV)) {
      rsc->setError(RS_ERROR_DRIVER, "Failed loading driver");
      return nullptr;
    }

    if (!rsc->isSynchronous()) {
        // Due to legacy we default to normal_graphics
        // setPriority will make the adjustments as needed.
        rsc->setPriority(RS_THREAD_PRIORITY_NORMAL_GRAPHICS);
    }

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (rsc->mIsGraphicsContext) {
        if (!rsc->initGLThread()) {
            rsc->setError(RS_ERROR_OUT_OF_MEMORY, "Failed initializing GL");
            return nullptr;
        }

        rsc->mStateRaster.init(rsc);
        rsc->setProgramRaster(nullptr);
        rsc->mStateVertex.init(rsc);
        rsc->setProgramVertex(nullptr);
        rsc->mStateFragment.init(rsc);
        rsc->setProgramFragment(nullptr);
        rsc->mStateFragmentStore.init(rsc);
        rsc->setProgramStore(nullptr);
        rsc->mStateFont.init(rsc);
        rsc->setFont(nullptr);
        rsc->mStateSampler.init(rsc);
        rsc->mFBOCache.init(rsc);
    }
#endif

    rsc->mRunning = true;

    if (rsc->isSynchronous()) {
        return nullptr;
    }

    if (!rsc->mIsGraphicsContext) {
        while (!rsc->mExit) {
            rsc->mIO.playCoreCommands(rsc, -1);
        }
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    } else {
        // The number of millisecond to wait between successive calls to the
        // root function.  The special value 0 means that root should not be
        // called again until something external changes.
        // See compile/slang/README.rst and search for "The function **root**"
        // for more details.
        int whenToCallAgain = 0;

        while (!rsc->mExit) {
            rsc->timerSet(RS_TIMER_IDLE);
            // While it's tempting to simply have if(whenToCallAgain > 0)
            // usleep(whentoCallAgain * 1000), doing it this way emulates
            // more closely what the original code did.
            if (whenToCallAgain > 16) {
                usleep((whenToCallAgain - 16) * 1000);
            }

            if (!rsc->mRootScript.get() || !rsc->mHasSurface || rsc->mPaused || whenToCallAgain == 0) {
                rsc->mIO.playCoreCommands(rsc, -1);
            } else {
                rsc->mIO.playCoreCommands(rsc, rsc->mSyncFd);
            }

            if (rsc->mRootScript.get() && rsc->mHasSurface && !rsc->mPaused) {
                whenToCallAgain = rsc->runRootScript();

                if (rsc->props.mLogVisual) {
                    rsc->displayDebugStats();
                }
                rsc->timerSet(RS_TIMER_CLEAR_SWAP);
                rsc->mHal.funcs.swap(rsc);
                rsc->timerFrame();
                rsc->timerSet(RS_TIMER_INTERNAL);
                rsc->timerPrint();
                rsc->timerReset();
            }
        }
#endif
    }

    //ALOGV("%p RS Thread exiting", rsc);

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (rsc->mIsGraphicsContext) {
        pthread_mutex_lock(&gInitMutex);
        rsc->deinitEGL();
        pthread_mutex_unlock(&gInitMutex);
    }
#endif

    //ALOGV("%p RS Thread exited", rsc);
    return nullptr;
}

void Context::destroyWorkerThreadResources() {
    //ALOGV("destroyWorkerThreadResources 1");
    ObjectBase::zeroAllUserRef(this);
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    if (mIsGraphicsContext) {
         mRaster.clear();
         mFragment.clear();
         mVertex.clear();
         mFragmentStore.clear();
         mFont.clear();
         mRootScript.clear();
         mStateRaster.deinit(this);
         mStateVertex.deinit(this);
         mStateFragment.deinit(this);
         mStateFragmentStore.deinit(this);
         mStateFont.deinit(this);
         mStateSampler.deinit(this);
         mFBOCache.deinit(this);
    }
#endif
    ObjectBase::freeAllChildren(this);
    mExit = true;
    //ALOGV("destroyWorkerThreadResources 2");
}

void Context::printWatchdogInfo(void *ctx) {
    Context *rsc = (Context *)ctx;
    if (rsc->watchdog.command && rsc->watchdog.file) {
        ALOGE("RS watchdog timeout: %i  %s  line %i %s", rsc->watchdog.inRoot,
             rsc->watchdog.command, rsc->watchdog.line, rsc->watchdog.file);
    } else {
        ALOGE("RS watchdog timeout: %i", rsc->watchdog.inRoot);
    }
}


void Context::setPriority(int32_t p) {
    switch (p) {
    // The public API will always send NORMAL_GRAPHICS
    // for normal, we adjust here
    case RS_THREAD_PRIORITY_NORMAL_GRAPHICS:
        if (!mIsGraphicsContext) {
            if (mHal.flags & RS_CONTEXT_LOW_LATENCY) {
                p = RS_THREAD_PRIORITY_LOW_LATENCY;
            } else {
                p = RS_THREAD_PRIORITY_NORMAL;
            }
        }
        break;
    case RS_THREAD_PRIORITY_LOW:
        break;
    }

    // Note: If we put this in the proper "background" policy
    // the wallpapers can become completly unresponsive at times.
    // This is probably not what we want for something the user is actively
    // looking at.
    mThreadPriority = p;
    setpriority(PRIO_PROCESS, mNativeThreadId, p);
    mHal.funcs.setPriority(this, mThreadPriority);
}

Context::Context() {
    mDev = nullptr;
    mRunning = false;
    mExit = false;
    mPaused = false;
    mObjHead = nullptr;
    mError = RS_ERROR_NONE;
    mTargetSdkVersion = 14;
    mDPI = 96;
    mIsContextLite = false;
    memset(&watchdog, 0, sizeof(watchdog));
    memset(&mHal, 0, sizeof(mHal));
    mForceCpu = false;
    mForceRSoV = false;
    mContextType = RS_CONTEXT_TYPE_NORMAL;
    mOptLevel = 3;
    mSynchronous = false;
    mFatalErrorOccured = false;

    memset(mCacheDir, 0, sizeof(mCacheDir));
#ifdef RS_COMPATIBILITY_LIB
    memset(nativeLibDir, 0, sizeof(nativeLibDir));
#endif
}

void Context::setCacheDir(const char * cacheDir_arg, uint32_t length) {
    if (length <= PATH_MAX) {
        memcpy(mCacheDir, cacheDir_arg, length);
        mCacheDir[length] = 0;
        hasSetCacheDir = true;
    } else {
        setError(RS_ERROR_BAD_VALUE, "Invalid path");
    }
}

Context * Context::createContext(Device *dev, const RsSurfaceConfig *sc,
                                 RsContextType ct, uint32_t flags,
                                 const char* vendorDriverName) {
    Context * rsc = new Context();

    if (flags & RS_CONTEXT_LOW_LATENCY) {
        rsc->mForceCpu = true;
    }
    if (flags & RS_CONTEXT_SYNCHRONOUS) {
        rsc->mSynchronous = true;
    }
    rsc->mContextType = ct;
    rsc->mHal.flags = flags;
    rsc->mVendorDriverName = vendorDriverName;

    if (!rsc->initContext(dev, sc)) {
        delete rsc;
        return nullptr;
    }

    return rsc;
}

Context * Context::createContextLite() {
    Context * rsc = new Context();
    rsc->mIsContextLite = true;
    return rsc;
}

bool Context::initContext(Device *dev, const RsSurfaceConfig *sc) {
    pthread_mutex_lock(&gInitMutex);

    if (!mIO.init()) {
        ALOGE("Failed initializing IO Fifo");
        pthread_mutex_unlock(&gInitMutex);
        return false;
    }
    mIO.setTimeoutCallback(printWatchdogInfo, this, 2e9);

    if (sc) {
        mUserSurfaceConfig = *sc;
    } else {
        memset(&mUserSurfaceConfig, 0, sizeof(mUserSurfaceConfig));
    }

    mIsGraphicsContext = sc != nullptr;

    int status;
    pthread_attr_t threadAttr;

    pthread_mutex_unlock(&gInitMutex);

    // Global init done at this point.

    status = pthread_attr_init(&threadAttr);
    if (status) {
        ALOGE("Failed to init thread attribute.");
        return false;
    }

    mHasSurface = false;
    mDriverName = NULL;

    timerInit();
    timerSet(RS_TIMER_INTERNAL);
    if (mSynchronous) {
        threadProc(this);

        if (mError != RS_ERROR_NONE) {
            ALOGE("Errors during thread init (sync mode)");
            return false;
        }
    } else {
        status = pthread_create(&mThreadId, &threadAttr, threadProc, this);
        if (status) {
            ALOGE("Failed to start rs context thread.");
            return false;
        }
        while (!mRunning && (mError == RS_ERROR_NONE)) {
            usleep(100);
        }

        if (mError != RS_ERROR_NONE) {
            ALOGE("Errors during thread init");
            return false;
        }

        pthread_attr_destroy(&threadAttr);
    }
    return true;
}

Context::~Context() {
    if (!mIsContextLite) {
        mPaused = false;
        void *res;

        mIO.shutdown();
        if (!mSynchronous && mRunning) {
            // Only try to join a pthread when:
            // 1. The Context is asynchronous.
            // 2. pthread successfully created and running.
            pthread_join(mThreadId, &res);
        }
        rsAssert(mExit);

        if (mHal.funcs.shutdownDriver && mHal.drv) {
            mHal.funcs.shutdownDriver(this);
        }
    }
}

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
void Context::setSurface(uint32_t w, uint32_t h, RsNativeWindow sur) {
    rsAssert(mIsGraphicsContext);
    mHal.funcs.setSurface(this, w, h, sur);

    mHasSurface = sur != nullptr;
    mWidth = w;
    mHeight = h;

    if (mWidth && mHeight) {
        mStateVertex.updateSize(this);
        mFBOCache.updateSize();
    }
}

uint32_t Context::getCurrentSurfaceWidth() const {
    for (uint32_t i = 0; i < mFBOCache.mHal.state.colorTargetsCount; i ++) {
        if (mFBOCache.mHal.state.colorTargets[i] != nullptr) {
            return mFBOCache.mHal.state.colorTargets[i]->getType()->getDimX();
        }
    }
    if (mFBOCache.mHal.state.depthTarget != nullptr) {
        return mFBOCache.mHal.state.depthTarget->getType()->getDimX();
    }
    return mWidth;
}

uint32_t Context::getCurrentSurfaceHeight() const {
    for (uint32_t i = 0; i < mFBOCache.mHal.state.colorTargetsCount; i ++) {
        if (mFBOCache.mHal.state.colorTargets[i] != nullptr) {
            return mFBOCache.mHal.state.colorTargets[i]->getType()->getDimY();
        }
    }
    if (mFBOCache.mHal.state.depthTarget != nullptr) {
        return mFBOCache.mHal.state.depthTarget->getType()->getDimY();
    }
    return mHeight;
}

void Context::pause() {
    rsAssert(mIsGraphicsContext);
    mPaused = true;
}

void Context::resume() {
    rsAssert(mIsGraphicsContext);
    mPaused = false;
}

void Context::setRootScript(Script *s) {
    rsAssert(mIsGraphicsContext);
    mRootScript.set(s);
}

void Context::setProgramStore(ProgramStore *pfs) {
    rsAssert(mIsGraphicsContext);
    if (pfs == nullptr) {
        mFragmentStore.set(mStateFragmentStore.mDefault);
    } else {
        mFragmentStore.set(pfs);
    }
}

void Context::setProgramFragment(ProgramFragment *pf) {
    rsAssert(mIsGraphicsContext);
    if (pf == nullptr) {
        mFragment.set(mStateFragment.mDefault);
    } else {
        mFragment.set(pf);
    }
}

void Context::setProgramRaster(ProgramRaster *pr) {
    rsAssert(mIsGraphicsContext);
    if (pr == nullptr) {
        mRaster.set(mStateRaster.mDefault);
    } else {
        mRaster.set(pr);
    }
}

void Context::setProgramVertex(ProgramVertex *pv) {
    rsAssert(mIsGraphicsContext);
    if (pv == nullptr) {
        mVertex.set(mStateVertex.mDefault);
    } else {
        mVertex.set(pv);
    }
}

void Context::setFont(Font *f) {
    rsAssert(mIsGraphicsContext);
    if (f == nullptr) {
        mFont.set(mStateFont.mDefault);
    } else {
        mFont.set(f);
    }
}
#endif

void Context::finish() {
    if (mHal.funcs.finish) {
        mHal.funcs.finish(this);
    }
}

void Context::assignName(ObjectBase *obj, const char *name, uint32_t len) {
    rsAssert(!obj->getName());
    obj->setName(name, len);
    mNames.push_back(obj);
}

void Context::removeName(ObjectBase *obj) {
    for (size_t ct=0; ct < mNames.size(); ct++) {
        if (obj == mNames[ct]) {
            mNames.erase(mNames.begin() + ct);
            return;
        }
    }
}

RsMessageToClientType Context::peekMessageToClient(size_t *receiveLen, uint32_t *subID) {
    return (RsMessageToClientType)mIO.getClientHeader(receiveLen, subID);
}

RsMessageToClientType Context::getMessageToClient(void *data, size_t *receiveLen, uint32_t *subID, size_t bufferLen) {
    return (RsMessageToClientType)mIO.getClientPayload(data, receiveLen, subID, bufferLen);
}

bool Context::sendMessageToClient(const void *data, RsMessageToClientType cmdID,
                                  uint32_t subID, size_t len, bool waitForSpace) const {

    pthread_mutex_lock(&gMessageMutex);
    bool ret = mIO.sendToClient(cmdID, subID, data, len, waitForSpace);
    pthread_mutex_unlock(&gMessageMutex);
    return ret;
}

void Context::initToClient() {
    while (!mRunning) {
        usleep(100);
    }
}

void Context::deinitToClient() {
    mIO.clientShutdown();
}

void Context::setError(RsError e, const char *msg) const {
    mError = e;

    if (mError >= RS_ERROR_FATAL_DEBUG) {
        // If a FATAL error occurred, set the flag to indicate the process
        // will be goign down
        mFatalErrorOccured = true;
    }

    sendMessageToClient(msg, RS_MESSAGE_TO_CLIENT_ERROR, e, strlen(msg) + 1, true);
}


void Context::dumpDebug() const {
    ALOGE("RS Context debug %p", this);
    ALOGE("RS Context debug");

    ALOGE(" RS width %i, height %i", mWidth, mHeight);
    ALOGE(" RS running %i, exit %i, paused %i", mRunning, mExit, mPaused);
    ALOGE(" RS pThreadID %li, nativeThreadID %i", (long int)mThreadId, mNativeThreadId);
}

///////////////////////////////////////////////////////////////////////////////////////////
//

void rsi_ContextFinish(Context *rsc) {
    rsc->finish();
}

void rsi_ContextBindRootScript(Context *rsc, RsScript vs) {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    Script *s = static_cast<Script *>(vs);
    rsc->setRootScript(s);
#endif
}

void rsi_ContextSetCacheDir(Context *rsc, const char *cacheDir, size_t cacheDir_length) {
    rsc->setCacheDir(cacheDir, cacheDir_length);
}

void rsi_ContextBindSampler(Context *rsc, uint32_t slot, RsSampler vs) {
    Sampler *s = static_cast<Sampler *>(vs);

    if (slot > RS_MAX_SAMPLER_SLOT) {
        ALOGE("Invalid sampler slot");
        return;
    }

    s->bindToContext(&rsc->mStateSampler, slot);
}

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
void rsi_ContextBindProgramStore(Context *rsc, RsProgramStore vpfs) {
    ProgramStore *pfs = static_cast<ProgramStore *>(vpfs);
    rsc->setProgramStore(pfs);
}

void rsi_ContextBindProgramFragment(Context *rsc, RsProgramFragment vpf) {
    ProgramFragment *pf = static_cast<ProgramFragment *>(vpf);
    rsc->setProgramFragment(pf);
}

void rsi_ContextBindProgramRaster(Context *rsc, RsProgramRaster vpr) {
    ProgramRaster *pr = static_cast<ProgramRaster *>(vpr);
    rsc->setProgramRaster(pr);
}

void rsi_ContextBindProgramVertex(Context *rsc, RsProgramVertex vpv) {
    ProgramVertex *pv = static_cast<ProgramVertex *>(vpv);
    rsc->setProgramVertex(pv);
}

void rsi_ContextBindFont(Context *rsc, RsFont vfont) {
    Font *font = static_cast<Font *>(vfont);
    rsc->setFont(font);
}
#endif

void rsi_AssignName(Context *rsc, RsObjectBase obj, const char *name, size_t name_length) {
    ObjectBase *ob = static_cast<ObjectBase *>(obj);
    rsc->assignName(ob, name, name_length);
}

void rsi_ObjDestroy(Context *rsc, void *optr) {
    ObjectBase *ob = static_cast<ObjectBase *>(optr);
    rsc->removeName(ob);
    ob->decUserRef();
}

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
void rsi_ContextPause(Context *rsc) {
    rsc->pause();
}

void rsi_ContextResume(Context *rsc) {
    rsc->resume();
}

void rsi_ContextSetSurface(Context *rsc, uint32_t w, uint32_t h, RsNativeWindow sur) {
    rsc->setSurface(w, h, sur);
}
#endif

void rsi_ContextSetPriority(Context *rsc, int32_t p) {
    rsc->setPriority(p);
}

void rsi_ContextDump(Context *rsc, int32_t bits) {
    ObjectBase::dumpAll(rsc);
}

void rsi_ContextDestroyWorker(Context *rsc) {
    rsc->destroyWorkerThreadResources();
}

void rsi_ContextDestroy(Context *rsc) {
    //ALOGE("%p rsContextDestroy", rsc);
    rsc->destroyWorkerThreadResources();
    delete rsc;
    //ALOGV("%p rsContextDestroy done", rsc);
}

RsMessageToClientType rsi_ContextPeekMessage(Context *rsc,
                                           size_t * receiveLen, size_t receiveLen_length,
                                           uint32_t * subID, size_t subID_length) {
    return rsc->peekMessageToClient(receiveLen, subID);
}

RsMessageToClientType rsi_ContextGetMessage(Context *rsc, void * data, size_t data_length,
                                          size_t * receiveLen, size_t receiveLen_length,
                                          uint32_t * subID, size_t subID_length) {
    rsAssert(subID_length == sizeof(uint32_t));
    rsAssert(receiveLen_length == sizeof(size_t));
    return rsc->getMessageToClient(data, receiveLen, subID, data_length);
}

void rsi_ContextInitToClient(Context *rsc) {
    rsc->initToClient();
}

void rsi_ContextDeinitToClient(Context *rsc) {
    rsc->deinitToClient();
}

void rsi_ContextSendMessage(Context *rsc, uint32_t id, const uint8_t *data, size_t len) {
    rsc->sendMessageToClient(data, RS_MESSAGE_TO_CLIENT_USER, id, len, true);
}

// implementation of handcode LF_ObjDestroy
// required so nObjDestroy can be run from finalizer without blocking
void LF_ObjDestroy_handcode(const Context *rsc, RsAsyncVoidPtr objPtr) {
    if (((Context *)rsc)->isSynchronous()) {
        rsi_ObjDestroy((Context *)rsc, objPtr);
        return;
    }

    // struct has two parts:
    // RsPlaybackRemoteHeader (cmdID and bytes)
    // RS_CMD_ObjDestroy (ptr)
    struct destroyCmd {
        uint32_t cmdID;
        uint32_t bytes;
        RsAsyncVoidPtr ptr;
     };

    destroyCmd cmd;
    cmd.cmdID = RS_CMD_ID_ObjDestroy;
    cmd.bytes = sizeof(RsAsyncVoidPtr);
    cmd.ptr = objPtr;
    ThreadIO *io = &((Context *)rsc)->mIO;
    io->coreWrite((void*)&cmd, sizeof(destroyCmd));

}

} // namespace renderscript
} // namespace android
