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

#ifndef ANDROID_RS_CONTEXT_H
#define ANDROID_RS_CONTEXT_H

#include "rsUtils.h"
#include "rs_hal.h"
#include <string.h>

#include "rsThreadIO.h"
#include "rsScriptC.h"
#include "rsScriptGroup.h"
#include "rsSampler.h"

#define ATRACE_ENABLED(...) false
#define ATRACE_NAME(...)
#define ATRACE_CALL(...)

#ifndef RS_COMPATIBILITY_LIB
#include "rsFont.h"
#include "rsProgramFragment.h"
#include "rsProgramStore.h"
#include "rsProgramRaster.h"
#include "rsProgramVertex.h"
#include "rsFBOCache.h"

#endif

#include <vector>

// ---------------------------------------------------------------------------
namespace android {

namespace renderscript {

class Device;

#if 0
#define CHECK_OBJ(o) { \
    GET_TLS(); \
    if (!ObjectBase::isValid(rsc, (const ObjectBase *)o)) {  \
        ALOGE("Bad object %p at %s, %i", o, __FILE__, __LINE__);  \
    } \
}
#define CHECK_OBJ_OR_NULL(o) { \
    GET_TLS(); \
    if (o && !ObjectBase::isValid(rsc, (const ObjectBase *)o)) {  \
        ALOGE("Bad object %p at %s, %i", o, __FILE__, __LINE__);  \
    } \
}
#else
#define CHECK_OBJ(o)
#define CHECK_OBJ_OR_NULL(o)
#endif



class Context {
public:
    struct Hal {
        void * drv;

        RsdHalFunctions funcs;
        uint32_t flags;
    };
    Hal mHal;

    static Context * createContext(Device *, const RsSurfaceConfig *sc,
            RsContextType ct = RS_CONTEXT_TYPE_NORMAL,
            uint32_t flags = 0,
            const char* vendorDriverName = nullptr);
    static Context * createContextLite();
    ~Context();

    static pthread_mutex_t gMessageMutex;
    static pthread_mutex_t gInitMutex;
    // Library mutex (for providing thread-safe calls from the runtime)
    static pthread_mutex_t gLibMutex;

    class PushState {
    public:
        explicit PushState(Context *);
        ~PushState();

    private:
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        ObjectBaseRef<ProgramFragment> mFragment;
        ObjectBaseRef<ProgramVertex> mVertex;
        ObjectBaseRef<ProgramStore> mStore;
        ObjectBaseRef<ProgramRaster> mRaster;
        ObjectBaseRef<Font> mFont;
#endif
        Context *mRsc;
    };

    RsSurfaceConfig mUserSurfaceConfig;

    ElementState mStateElement;
    TypeState mStateType;
    SamplerState mStateSampler;

    bool isSynchronous() {return mSynchronous;}
    bool setupCheck();

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    FBOCache mFBOCache;
    ProgramFragmentState mStateFragment;
    ProgramStoreState mStateFragmentStore;
    ProgramRasterState mStateRaster;
    ProgramVertexState mStateVertex;
    FontState mStateFont;


    void swapBuffers();
    void setRootScript(Script *);
    void setProgramRaster(ProgramRaster *);
    void setProgramVertex(ProgramVertex *);
    void setProgramFragment(ProgramFragment *);
    void setProgramStore(ProgramStore *);
    void setFont(Font *);

    void updateSurface(void *sur);

    ProgramFragment * getProgramFragment() {return mFragment.get();}
    ProgramStore * getProgramStore() {return mFragmentStore.get();}
    ProgramRaster * getProgramRaster() {return mRaster.get();}
    ProgramVertex * getProgramVertex() {return mVertex.get();}
    Font * getFont() {return mFont.get();}

    void setupProgramStore();

    void pause();
    void resume();
    void setSurface(uint32_t w, uint32_t h, RsNativeWindow sur);
#endif
    void finish();

    void setPriority(int32_t p);
    void destroyWorkerThreadResources();

    void assignName(ObjectBase *obj, const char *name, uint32_t len);
    void removeName(ObjectBase *obj);

    RsMessageToClientType peekMessageToClient(size_t *receiveLen, uint32_t *subID);
    RsMessageToClientType getMessageToClient(void *data, size_t *receiveLen, uint32_t *subID, size_t bufferLen);
    bool sendMessageToClient(const void *data, RsMessageToClientType cmdID, uint32_t subID, size_t len, bool waitForSpace) const;
    uint32_t runScript(Script *s);

    void initToClient();
    void deinitToClient();

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    ProgramFragment * getDefaultProgramFragment() const {
        return mStateFragment.mDefault.get();
    }
    ProgramVertex * getDefaultProgramVertex() const {
        return mStateVertex.mDefault.get();
    }
    ProgramStore * getDefaultProgramStore() const {
        return mStateFragmentStore.mDefault.get();
    }
    ProgramRaster * getDefaultProgramRaster() const {
        return mStateRaster.mDefault.get();
    }
    Font* getDefaultFont() const {
        return mStateFont.mDefault.get();
    }

    uint32_t getWidth() const {return mWidth;}
    uint32_t getHeight() const {return mHeight;}

    uint32_t getCurrentSurfaceWidth() const;
    uint32_t getCurrentSurfaceHeight() const;

    void setWatchdogGL(const char *cmd, uint32_t line, const char *file) const {
        watchdog.command = cmd;
        watchdog.file = file;
        watchdog.line = line;
    }
#endif

    mutable ThreadIO mIO;

    // Timers
    enum Timers {
        RS_TIMER_IDLE,
        RS_TIMER_INTERNAL,
        RS_TIMER_SCRIPT,
        RS_TIMER_CLEAR_SWAP,
        _RS_TIMER_TOTAL
    };
    uint64_t getTime() const;
    void timerInit();
    void timerReset();
    void timerSet(Timers);
    void timerPrint();
    void timerFrame();

    struct {
        bool mLogTimes;
        bool mLogScripts;
        bool mLogShaders;
        bool mLogShadersAttr;
        bool mLogShadersUniforms;
        bool mLogVisual;
        uint32_t mLogReduce;
        bool mDebugReduceSplitAccum;
        uint32_t mDebugMaxThreads;
    } props;

    mutable struct {
        bool inRoot;
        const char *command;
        const char *file;
        uint32_t line;
    } watchdog;
    static void printWatchdogInfo(void *ctx);

    void dumpDebug() const;
    void setError(RsError e, const char *msg = nullptr) const;

    mutable const ObjectBase * mObjHead;

    uint32_t getDPI() const {return mDPI;}
    void setDPI(uint32_t dpi) {mDPI = dpi;}

    uint32_t getTargetSdkVersion() const {return mTargetSdkVersion;}
    void setTargetSdkVersion(uint32_t sdkVer) {mTargetSdkVersion = sdkVer;}

    RsContextType getContextType() const { return mContextType; }
    void setContextType(RsContextType ct) { mContextType = ct; }

    // Check for Fatal errors
    // Should be used to prevent work from being launched
    // which could take the process down.  Maximizes the chance
    // the process lives long enough to get the error to the developer
    bool hadFatalError() {return mFatalErrorOccured;}

    uint32_t getOptLevel() const { return mOptLevel; }
    void setOptLevel(uint32_t optLevel) { mOptLevel = optLevel; }

    Device *mDev;

#ifdef RS_COMPATIBILITY_LIB
    void setNativeLibDir(const char * libDir, uint32_t length) {
        if (!hasSetNativeLibDir) {
            if (length <= PATH_MAX) {
                memcpy(nativeLibDir, libDir, length);
                nativeLibDir[length] = 0;
                hasSetNativeLibDir = true;
            } else {
                setError(RS_ERROR_BAD_VALUE, "Invalid path");
            }
        }
    }
    const char * getNativeLibDir() {
        return nativeLibDir;
    }
#endif

    void setCacheDir(const char * cacheDir_arg, uint32_t length);
    const char * getCacheDir() {
        if (hasSetCacheDir) {
            return mCacheDir;
        }
        return nullptr;
    }

    // Returns the actual loaded driver's name (like "libRSDriver.so").
    const char * getDriverName() {
        return mDriverName;
    }

    // Set a new driver name, should be called from within
    // rsdHalInit in order to alter default behaviour.
    void setDriverName(const char * name) {
        if (!mDriverName) {
            mDriverName = name;
        }
    }

protected:

    uint32_t mTargetSdkVersion;
    uint32_t mDPI;
    uint32_t mWidth;
    uint32_t mHeight;
    int32_t mThreadPriority;
    bool mIsGraphicsContext;

    bool mForceRSoV;
    bool mForceCpu;

    RsContextType mContextType;
    uint32_t mOptLevel;

    bool mRunning;
    bool mExit;
    bool mPaused;
    mutable bool mFatalErrorOccured;
    mutable RsError mError;


    pthread_t mThreadId;
    pid_t mNativeThreadId;

    ObjectBaseRef<Script> mRootScript;
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    ObjectBaseRef<ProgramFragment> mFragment;
    ObjectBaseRef<ProgramVertex> mVertex;
    ObjectBaseRef<ProgramStore> mFragmentStore;
    ObjectBaseRef<ProgramRaster> mRaster;
    ObjectBaseRef<Font> mFont;
#endif

    void displayDebugStats();

private:
    Context();
    bool initContext(Device *, const RsSurfaceConfig *sc);
    bool mSynchronous;
    bool initGLThread();
    void deinitEGL();

    uint32_t runRootScript();

    bool loadRuntime(const char* filename);
    // Loads the driver.
    // forceDefault: If true, loads the default CPU driver.
    // forceRSoV:  If true, overrides forceDefault and loads the RSoV driver.
    bool loadDriver(bool forceDefault, bool forceRSoV);
    static void * threadProc(void *);
    static void * helperThreadProc(void *);

    bool mHasSurface;
    bool mIsContextLite;

    // This holds the name of the driver (like "libRSDriver.so").
    // Since this is always just a static string, we don't have to
    // allocate, copy, or free any memory here.
    const char* mDriverName;
    const char* mVendorDriverName;

    std::vector<ObjectBase *> mNames;

    // Sync fence id for Graphic API, default value -1.
    int32_t mSyncFd = -1;
    uint64_t mTimers[_RS_TIMER_TOTAL];
    Timers mTimerActive;
    uint64_t mTimeLast;
    uint64_t mTimeFrame;
    uint64_t mTimeLastFrame;
    uint32_t mTimeMSLastFrame;
    uint32_t mTimeMSLastScript;
    uint32_t mTimeMSLastSwap;
    uint32_t mAverageFPSFrameCount;
    uint64_t mAverageFPSStartTime;
    uint32_t mAverageFPS;
#ifdef RS_COMPATIBILITY_LIB
    bool hasSetNativeLibDir = false;
    char nativeLibDir[PATH_MAX+1];
#endif
    bool hasSetCacheDir = false;
    char mCacheDir[PATH_MAX+1];
};

void LF_ObjDestroy_handcode(const Context *rsc, RsAsyncVoidPtr objPtr);

} // namespace renderscript
} // namespace android
#endif
