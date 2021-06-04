/*
 * Copyright (C) 2009 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXNodeInstance"
#include <android-base/macros.h>
#include <utils/Log.h>

#include <inttypes.h>

#include <media/stagefright/omx/OMXNodeInstance.h>
#include <media/stagefright/omx/OMXMaster.h>
#include <media/stagefright/omx/OMXUtils.h>
#include <android/IOMXBufferSource.h>

#include <media/openmax/OMX_Component.h>
#include <media/openmax/OMX_IndexExt.h>
#include <media/openmax/OMX_VideoExt.h>
#include <media/openmax/OMX_AsString.h>

#include <binder/IMemory.h>
#include <cutils/properties.h>
#include <media/hardware/HardwareAPI.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/MediaErrors.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>
#include <utils/misc.h>
#include <utils/NativeHandle.h>
#include <media/OMXBuffer.h>
#include <media/stagefright/xmlparser/MediaCodecsXmlParser.h>

#include <hidlmemory/mapping.h>

static const OMX_U32 kPortIndexInput = 0;
static const OMX_U32 kPortIndexOutput = 1;

#define CLOGW(fmt, ...) ALOGW("[%p:%s] " fmt, mHandle, mName, ##__VA_ARGS__)

#define CLOG_ERROR_IF(cond, fn, err, fmt, ...) \
    ALOGE_IF(cond, #fn "(%p:%s, " fmt ") ERROR: %s(%#x)", \
    mHandle, mName, ##__VA_ARGS__, asString(err), err)
#define CLOG_ERROR(fn, err, fmt, ...) CLOG_ERROR_IF(true, fn, err, fmt, ##__VA_ARGS__)
#define CLOG_IF_ERROR(fn, err, fmt, ...) \
    CLOG_ERROR_IF((err) != OMX_ErrorNone, fn, err, fmt, ##__VA_ARGS__)

#define CLOGI_(level, fn, fmt, ...) \
    ALOGI_IF(DEBUG >= (level), #fn "(%p:%s, " fmt ")", mHandle, mName, ##__VA_ARGS__)
#define CLOGD_(level, fn, fmt, ...) \
    ALOGD_IF(DEBUG >= (level), #fn "(%p:%s, " fmt ")", mHandle, mName, ##__VA_ARGS__)

#define CLOG_LIFE(fn, fmt, ...)     CLOGI_(ADebug::kDebugLifeCycle,     fn, fmt, ##__VA_ARGS__)
#define CLOG_STATE(fn, fmt, ...)    CLOGI_(ADebug::kDebugState,         fn, fmt, ##__VA_ARGS__)
#define CLOG_CONFIG(fn, fmt, ...)   CLOGI_(ADebug::kDebugConfig,        fn, fmt, ##__VA_ARGS__)
#define CLOG_INTERNAL(fn, fmt, ...) CLOGD_(ADebug::kDebugInternalState, fn, fmt, ##__VA_ARGS__)

#define CLOG_DEBUG_IF(cond, fn, fmt, ...) \
    ALOGD_IF(cond, #fn "(%p, " fmt ")", mHandle, ##__VA_ARGS__)

#define CLOG_BUFFER(fn, fmt, ...) \
    CLOG_DEBUG_IF(DEBUG >= ADebug::kDebugAll, fn, fmt, ##__VA_ARGS__)
#define CLOG_BUMPED_BUFFER(fn, fmt, ...) \
    CLOG_DEBUG_IF(DEBUG_BUMP >= ADebug::kDebugAll, fn, fmt, ##__VA_ARGS__)

/* buffer formatting */
#define BUFFER_FMT(port, fmt, ...) "%s:%u " fmt, portString(port), (port), ##__VA_ARGS__
#define NEW_BUFFER_FMT(buffer_id, port, fmt, ...) \
    BUFFER_FMT(port, fmt ") (#%zu => %#x", ##__VA_ARGS__, mActiveBuffers.size(), (buffer_id))

#define SIMPLE_BUFFER(port, size, data) BUFFER_FMT(port, "%zu@%p", (size), (data))
#define SIMPLE_NEW_BUFFER(buffer_id, port, size, data) \
    NEW_BUFFER_FMT(buffer_id, port, "%zu@%p", (size), (data))

#define EMPTY_BUFFER(addr, header, fenceFd) "%#x [%u@%p fc=%d]", \
    (addr), (header)->nAllocLen, (header)->pBuffer, (fenceFd)
#define FULL_BUFFER(addr, header, fenceFd) "%#" PRIxPTR " [%u@%p (%u..+%u) f=%x ts=%lld fc=%d]", \
    (intptr_t)(addr), (header)->nAllocLen, (header)->pBuffer, \
    (header)->nOffset, (header)->nFilledLen, (header)->nFlags, (header)->nTimeStamp, (fenceFd)

#define WITH_STATS_WRAPPER(fmt, ...) fmt " { IN=%zu/%zu OUT=%zu/%zu }", ##__VA_ARGS__, \
    mInputBuffersWithCodec.size(), mNumPortBuffers[kPortIndexInput], \
    mOutputBuffersWithCodec.size(), mNumPortBuffers[kPortIndexOutput]
// TRICKY: this is needed so formatting macros expand before substitution
#define WITH_STATS(fmt, ...) WITH_STATS_WRAPPER(fmt, ##__VA_ARGS__)

namespace android {

struct BufferMeta {
    explicit BufferMeta(
            const sp<IMemory> &mem, const sp<IHidlMemory> &hidlMemory,
            OMX_U32 portIndex, bool copy, OMX_U8 *backup)
        : mMem(mem),
          mHidlMemory(hidlMemory),
          mCopyFromOmx(portIndex == kPortIndexOutput && copy),
          mCopyToOmx(portIndex == kPortIndexInput && copy),
          mPortIndex(portIndex),
          mBackup(backup) {
    }

    explicit BufferMeta(OMX_U32 portIndex)
        : mCopyFromOmx(false),
          mCopyToOmx(false),
          mPortIndex(portIndex),
          mBackup(NULL) {
    }

    explicit BufferMeta(const sp<GraphicBuffer> &graphicBuffer, OMX_U32 portIndex)
        : mGraphicBuffer(graphicBuffer),
          mCopyFromOmx(false),
          mCopyToOmx(false),
          mPortIndex(portIndex),
          mBackup(NULL) {
    }

    OMX_U8 *getPointer() {
        return mMem.get() ? static_cast<OMX_U8*>(mMem->pointer()) :
                mHidlMemory.get() ? static_cast<OMX_U8*>(
                static_cast<void*>(mHidlMemory->getPointer())) : nullptr;
    }

    void CopyFromOMX(const OMX_BUFFERHEADERTYPE *header) {
        if (!mCopyFromOmx) {
            return;
        }

        // check component returns proper range
        sp<ABuffer> codec = getBuffer(header, true /* limit */);

        memcpy(getPointer() + header->nOffset, codec->data(), codec->size());
    }

    void CopyToOMX(const OMX_BUFFERHEADERTYPE *header) {
        if (!mCopyToOmx) {
            return;
        }

        memcpy(header->pBuffer + header->nOffset,
                getPointer() + header->nOffset,
                header->nFilledLen);
    }

    // return the codec buffer
    sp<ABuffer> getBuffer(const OMX_BUFFERHEADERTYPE *header, bool limit) {
        sp<ABuffer> buf = new ABuffer(header->pBuffer, header->nAllocLen);
        if (limit) {
            if (header->nOffset + header->nFilledLen > header->nOffset
                    && header->nOffset + header->nFilledLen <= header->nAllocLen) {
                buf->setRange(header->nOffset, header->nFilledLen);
            } else {
                buf->setRange(0, 0);
            }
        }
        return buf;
    }

    void setGraphicBuffer(const sp<GraphicBuffer> &graphicBuffer) {
        mGraphicBuffer = graphicBuffer;
    }

    void setNativeHandle(const sp<NativeHandle> &nativeHandle) {
        mNativeHandle = nativeHandle;
    }

    OMX_U32 getPortIndex() {
        return mPortIndex;
    }

    ~BufferMeta() {
        delete[] mBackup;
    }

private:
    sp<GraphicBuffer> mGraphicBuffer;
    sp<NativeHandle> mNativeHandle;
    sp<IMemory> mMem;
    sp<IHidlMemory> mHidlMemory;
    bool mCopyFromOmx;
    bool mCopyToOmx;
    OMX_U32 mPortIndex;
    OMX_U8 *mBackup;

    BufferMeta(const BufferMeta &);
    BufferMeta &operator=(const BufferMeta &);
};

// static
OMX_CALLBACKTYPE OMXNodeInstance::kCallbacks = {
    &OnEvent, &OnEmptyBufferDone, &OnFillBufferDone
};

static inline const char *portString(OMX_U32 portIndex) {
    switch (portIndex) {
        case kPortIndexInput:  return "Input";
        case kPortIndexOutput: return "Output";
        case ~0U:              return "All";
        default:               return "port";
    }
}

////////////////////////////////////////////////////////////////////////////////

// This provides the underlying Thread used by CallbackDispatcher.
// Note that deriving CallbackDispatcher from Thread does not work.

struct OMXNodeInstance::CallbackDispatcherThread : public Thread {
    explicit CallbackDispatcherThread(CallbackDispatcher *dispatcher)
        : mDispatcher(dispatcher) {
    }

private:
    CallbackDispatcher *mDispatcher;

    bool threadLoop();

    CallbackDispatcherThread(const CallbackDispatcherThread &);
    CallbackDispatcherThread &operator=(const CallbackDispatcherThread &);
};

////////////////////////////////////////////////////////////////////////////////

struct OMXNodeInstance::CallbackDispatcher : public RefBase {
    explicit CallbackDispatcher(const sp<OMXNodeInstance> &owner);

    // Posts |msg| to the listener's queue. If |realTime| is true, the listener thread is notified
    // that a new message is available on the queue. Otherwise, the message stays on the queue, but
    // the listener is not notified of it. It will process this message when a subsequent message
    // is posted with |realTime| set to true.
    void post(const omx_message &msg, bool realTime = true);

    bool loop();

protected:
    virtual ~CallbackDispatcher();

private:
    enum {
        // This is used for frame_rendered message batching, which will eventually end up in a
        // single AMessage in MediaCodec when it is signaled to the app. AMessage can contain
        // up-to 64 key-value pairs, and each frame_rendered message uses 2 keys, so the max
        // value for this would be 32. Nonetheless, limit this to 12 to which gives at least 10
        // mseconds of batching at 120Hz.
        kMaxQueueSize = 12,
    };

    Mutex mLock;

    sp<OMXNodeInstance> const mOwner;
    bool mDone;
    Condition mQueueChanged;
    std::list<omx_message> mQueue;

    sp<CallbackDispatcherThread> mThread;

    void dispatch(std::list<omx_message> &messages);

    CallbackDispatcher(const CallbackDispatcher &);
    CallbackDispatcher &operator=(const CallbackDispatcher &);
};

OMXNodeInstance::CallbackDispatcher::CallbackDispatcher(const sp<OMXNodeInstance> &owner)
    : mOwner(owner),
      mDone(false) {
    mThread = new CallbackDispatcherThread(this);
    mThread->run("OMXCallbackDisp", ANDROID_PRIORITY_FOREGROUND);
}

OMXNodeInstance::CallbackDispatcher::~CallbackDispatcher() {
    {
        Mutex::Autolock autoLock(mLock);

        mDone = true;
        mQueueChanged.signal();
    }

    // A join on self can happen if the last ref to CallbackDispatcher
    // is released within the CallbackDispatcherThread loop
    status_t status = mThread->join();
    if (status != WOULD_BLOCK) {
        // Other than join to self, the only other error return codes are
        // whatever readyToRun() returns, and we don't override that
        CHECK_EQ(status, (status_t)NO_ERROR);
    }
}

void OMXNodeInstance::CallbackDispatcher::post(const omx_message &msg, bool realTime) {
    Mutex::Autolock autoLock(mLock);

    mQueue.push_back(msg);
    if (realTime || mQueue.size() >= kMaxQueueSize) {
        mQueueChanged.signal();
    }
}

void OMXNodeInstance::CallbackDispatcher::dispatch(std::list<omx_message> &messages) {
    if (mOwner == NULL) {
        ALOGV("Would have dispatched a message to a node that's already gone.");
        return;
    }
    mOwner->onMessages(messages);
}

bool OMXNodeInstance::CallbackDispatcher::loop() {
    for (;;) {
        std::list<omx_message> messages;

        {
            Mutex::Autolock autoLock(mLock);
            while (!mDone && mQueue.empty()) {
                mQueueChanged.wait(mLock);
            }

            if (mDone) {
                break;
            }

            messages.swap(mQueue);
        }

        dispatch(messages);
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OMXNodeInstance::CallbackDispatcherThread::threadLoop() {
    return mDispatcher->loop();
}

////////////////////////////////////////////////////////////////////////////////

OMXNodeInstance::OMXNodeInstance(
        Omx *owner, const sp<IOMXObserver> &observer, const char *name)
    : mOwner(owner),
      mHandle(NULL),
      mObserver(observer),
      mDying(false),
      mSailed(false),
      mQueriedProhibitedExtensions(false),
      mQuirks(0),
      mBufferIDCount(0),
      mRestorePtsFailed(false),
      mMaxTimestampGapUs(0LL),
      mPrevOriginalTimeUs(-1LL),
      mPrevModifiedTimeUs(-1LL)
{
    mName = ADebug::GetDebugName(name);
    DEBUG = ADebug::GetDebugLevelFromProperty(name, "debug.stagefright.omx-debug");
    ALOGV("debug level for %s is %d", name, DEBUG);
    DEBUG_BUMP = DEBUG;
    mNumPortBuffers[0] = 0;
    mNumPortBuffers[1] = 0;
    mDebugLevelBumpPendingBuffers[0] = 0;
    mDebugLevelBumpPendingBuffers[1] = 0;
    mMetadataType[0] = kMetadataBufferTypeInvalid;
    mMetadataType[1] = kMetadataBufferTypeInvalid;
    mPortMode[0] = IOMX::kPortModePresetByteBuffer;
    mPortMode[1] = IOMX::kPortModePresetByteBuffer;
    mSecureBufferType[0] = kSecureBufferTypeUnknown;
    mSecureBufferType[1] = kSecureBufferTypeUnknown;
    mGraphicBufferEnabled[0] = false;
    mGraphicBufferEnabled[1] = false;
    mIsSecure = AString(name).endsWith(".secure");
    mLegacyAdaptiveExperiment = ADebug::isExperimentEnabled("legacy-adaptive");
}

OMXNodeInstance::~OMXNodeInstance() {
    free(mName);
    CHECK(mHandle == NULL);
}

void OMXNodeInstance::setHandle(OMX_HANDLETYPE handle) {
    CLOG_LIFE(allocateNode, "handle=%p", handle);
    CHECK(mHandle == NULL);
    mHandle = handle;
    if (handle != NULL) {
        mDispatcher = new CallbackDispatcher(this);
    }
}

sp<IOMXBufferSource> OMXNodeInstance::getBufferSource() {
    Mutex::Autolock autoLock(mOMXBufferSourceLock);
    return mOMXBufferSource;
}

void OMXNodeInstance::setBufferSource(const sp<IOMXBufferSource>& bufferSource) {
    Mutex::Autolock autoLock(mOMXBufferSourceLock);
    CLOG_INTERNAL(setBufferSource, "%p", bufferSource.get());
    mOMXBufferSource = bufferSource;
}

OMX_HANDLETYPE OMXNodeInstance::handle() {
    return mHandle;
}

sp<IOMXObserver> OMXNodeInstance::observer() {
    return mObserver;
}

status_t OMXNodeInstance::freeNode() {
    CLOG_LIFE(freeNode, "handle=%p", mHandle);
    static int32_t kMaxNumIterations = 10;

    // Transition the node from its current state all the way down
    // to "Loaded".
    // This ensures that all active buffers are properly freed even
    // for components that don't do this themselves on a call to
    // "FreeHandle".

    // The code below may trigger some more events to be dispatched
    // by the OMX component - we want to ignore them as our client
    // does not expect them.
    bool expected = false;
    if (!mDying.compare_exchange_strong(expected, true)) {
        // exit if we have already freed the node or doing so right now.
        // NOTE: this ensures that the block below executes at most once.
        ALOGV("Already dying");
        return OK;
    }

    OMX_STATETYPE state;
    CHECK_EQ(OMX_GetState(mHandle, &state), OMX_ErrorNone);
    switch (state) {
        case OMX_StateExecuting:
        {
            ALOGV("forcing Executing->Idle");
            sendCommand(OMX_CommandStateSet, OMX_StateIdle);
            OMX_ERRORTYPE err;
            int32_t iteration = 0;
            while ((err = OMX_GetState(mHandle, &state)) == OMX_ErrorNone
                    && state != OMX_StateIdle
                    && state != OMX_StateInvalid) {
                if (++iteration > kMaxNumIterations) {
                    CLOGW("failed to enter Idle state (now %s(%d), aborting.",
                            asString(state), state);
                    state = OMX_StateInvalid;
                    break;
                }

                usleep(100000);
            }
            CHECK_EQ(err, OMX_ErrorNone);

            if (state == OMX_StateInvalid) {
                break;
            }

            FALLTHROUGH_INTENDED;
        }

        case OMX_StateIdle:
        {
            ALOGV("forcing Idle->Loaded");
            sendCommand(OMX_CommandStateSet, OMX_StateLoaded);

            freeActiveBuffers();

            OMX_ERRORTYPE err;
            int32_t iteration = 0;
            while ((err = OMX_GetState(mHandle, &state)) == OMX_ErrorNone
                    && state != OMX_StateLoaded
                    && state != OMX_StateInvalid) {
                if (++iteration > kMaxNumIterations) {
                    CLOGW("failed to enter Loaded state (now %s(%d), aborting.",
                            asString(state), state);
                    state = OMX_StateInvalid;
                    break;
                }

                ALOGV("waiting for Loaded state...");
                usleep(100000);
            }
            CHECK_EQ(err, OMX_ErrorNone);

            FALLTHROUGH_INTENDED;
        }

        case OMX_StateLoaded:
        case OMX_StateInvalid:
            break;

        default:
            LOG_ALWAYS_FATAL("unknown state %s(%#x).", asString(state), state);
            break;
    }

    Mutex::Autolock _l(mLock);

    status_t err = mOwner->freeNode(this);

    mDispatcher.clear();
    mOMXBufferSource.clear();

    mHandle = NULL;
    CLOG_IF_ERROR(freeNode, err, "");
    free(mName);
    mName = NULL;

    ALOGV("OMXNodeInstance going away.");

    return err;
}

status_t OMXNodeInstance::sendCommand(
        OMX_COMMANDTYPE cmd, OMX_S32 param) {
    const sp<IOMXBufferSource> bufferSource(getBufferSource());
    if (bufferSource != NULL && cmd == OMX_CommandStateSet) {
        if (param == OMX_StateIdle) {
            // Initiating transition from Executing -> Idle
            // ACodec is waiting for all buffers to be returned, do NOT
            // submit any more buffers to the codec.
            bufferSource->onOmxIdle();
        } else if (param == OMX_StateLoaded) {
            // Initiating transition from Idle/Executing -> Loaded
            // Buffers are about to be freed.
            bufferSource->onOmxLoaded();
            setBufferSource(NULL);
        }

        // fall through
    }

    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (cmd == OMX_CommandStateSet) {
        // There are no configurations past first StateSet command.
        mSailed = true;
    }

    // bump internal-state debug level for 2 input and output frames past a command
    {
        Mutex::Autolock _l(mDebugLock);
        bumpDebugLevel_l(2 /* numInputBuffers */, 2 /* numOutputBuffers */);
    }

    const char *paramString =
        cmd == OMX_CommandStateSet ? asString((OMX_STATETYPE)param) : portString(param);
    CLOG_STATE(sendCommand, "%s(%d), %s(%d)", asString(cmd), cmd, paramString, param);
    OMX_ERRORTYPE err = OMX_SendCommand(mHandle, cmd, param, NULL);
    CLOG_IF_ERROR(sendCommand, err, "%s(%d), %s(%d)", asString(cmd), cmd, paramString, param);
    return StatusFromOMXError(err);
}

bool OMXNodeInstance::isProhibitedIndex_l(OMX_INDEXTYPE index) {
    // these extensions can only be used from OMXNodeInstance, not by clients directly.
    static const char *restricted_extensions[] = {
        "OMX.google.android.index.storeMetaDataInBuffers",
        "OMX.google.android.index.storeANWBufferInMetadata",
        "OMX.google.android.index.prepareForAdaptivePlayback",
        "OMX.google.android.index.configureVideoTunnelMode",
        "OMX.google.android.index.useAndroidNativeBuffer2",
        "OMX.google.android.index.useAndroidNativeBuffer",
        "OMX.google.android.index.enableAndroidNativeBuffers",
        "OMX.google.android.index.allocateNativeHandle",
        "OMX.google.android.index.getAndroidNativeBufferUsage",
    };

    if ((index > OMX_IndexComponentStartUnused && index < OMX_IndexComponentEndUnused)
            || (index > OMX_IndexPortStartUnused && index < OMX_IndexPortEndUnused)
            || (index > OMX_IndexAudioStartUnused && index < OMX_IndexAudioEndUnused)
            || (index > OMX_IndexVideoStartUnused && index < OMX_IndexVideoEndUnused)
            || (index > OMX_IndexCommonStartUnused && index < OMX_IndexCommonEndUnused)
            || (index > (OMX_INDEXTYPE)OMX_IndexExtAudioStartUnused
                    && index < (OMX_INDEXTYPE)OMX_IndexExtAudioEndUnused)
            || (index > (OMX_INDEXTYPE)OMX_IndexExtVideoStartUnused
                    && index < (OMX_INDEXTYPE)OMX_IndexExtVideoEndUnused)
            || (index > (OMX_INDEXTYPE)OMX_IndexExtOtherStartUnused
                    && index < (OMX_INDEXTYPE)OMX_IndexExtOtherEndUnused)) {
        return false;
    }

    if (!mQueriedProhibitedExtensions) {
        for (size_t i = 0; i < NELEM(restricted_extensions); ++i) {
            OMX_INDEXTYPE ext;
            if (OMX_GetExtensionIndex(mHandle, (OMX_STRING)restricted_extensions[i], &ext) == OMX_ErrorNone) {
                mProhibitedExtensions.add(ext);
            }
        }
        mQueriedProhibitedExtensions = true;
    }

    return mProhibitedExtensions.indexOf(index) >= 0;
}

status_t OMXNodeInstance::getParameter(
        OMX_INDEXTYPE index, void *params, size_t /* size */) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (isProhibitedIndex_l(index)) {
        android_errorWriteLog(0x534e4554, "29422020");
        return BAD_INDEX;
    }

    OMX_ERRORTYPE err = OMX_GetParameter(mHandle, index, params);
    OMX_INDEXEXTTYPE extIndex = (OMX_INDEXEXTTYPE)index;
    // some errors are expected for getParameter
    if (err != OMX_ErrorNoMore) {
        CLOG_IF_ERROR(getParameter, err, "%s(%#x)", asString(extIndex), index);
    }
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::setParameter(
        OMX_INDEXTYPE index, const void *params, size_t size) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    OMX_INDEXEXTTYPE extIndex = (OMX_INDEXEXTTYPE)index;
    CLOG_CONFIG(setParameter, "%s(%#x), %zu@%p)", asString(extIndex), index, size, params);

    if (extIndex == OMX_IndexParamMaxFrameDurationForBitrateControl) {
        return setMaxPtsGapUs(params, size);
    }

    if (isProhibitedIndex_l(index)) {
        android_errorWriteLog(0x534e4554, "29422020");
        return BAD_INDEX;
    }

    OMX_ERRORTYPE err = OMX_SetParameter(
            mHandle, index, const_cast<void *>(params));
    CLOG_IF_ERROR(setParameter, err, "%s(%#x)", asString(extIndex), index);
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::getConfig(
        OMX_INDEXTYPE index, void *params, size_t /* size */) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (isProhibitedIndex_l(index)) {
        android_errorWriteLog(0x534e4554, "29422020");
        return BAD_INDEX;
    }

    OMX_ERRORTYPE err = OMX_GetConfig(mHandle, index, params);
    OMX_INDEXEXTTYPE extIndex = (OMX_INDEXEXTTYPE)index;
    // some errors are expected for getConfig
    if (err != OMX_ErrorNoMore) {
        CLOG_IF_ERROR(getConfig, err, "%s(%#x)", asString(extIndex), index);
    }
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::setConfig(
        OMX_INDEXTYPE index, const void *params, size_t size) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    OMX_INDEXEXTTYPE extIndex = (OMX_INDEXEXTTYPE)index;
    CLOG_CONFIG(setConfig, "%s(%#x), %zu@%p)", asString(extIndex), index, size, params);

    if (isProhibitedIndex_l(index)) {
        android_errorWriteLog(0x534e4554, "29422020");
        return BAD_INDEX;
    }

    OMX_ERRORTYPE err = OMX_SetConfig(
            mHandle, index, const_cast<void *>(params));
    CLOG_IF_ERROR(setConfig, err, "%s(%#x)", asString(extIndex), index);
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::setPortMode(OMX_U32 portIndex, IOMX::PortMode mode) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (portIndex >= NELEM(mPortMode)) {
        ALOGE("b/31385713, portIndex(%u)", portIndex);
        android_errorWriteLog(0x534e4554, "31385713");
        return BAD_VALUE;
    }

    if (mSailed || mNumPortBuffers[portIndex] > 0) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }

    CLOG_CONFIG(setPortMode, "%s(%d), port %d", asString(mode), mode, portIndex);

    status_t err = OK;
    switch (mode) {
    case IOMX::kPortModeDynamicANWBuffer:
    {
        if (portIndex == kPortIndexOutput) {
            if (mLegacyAdaptiveExperiment) {
                CLOG_INTERNAL(setPortMode, "Legacy adaptive experiment: "
                        "not setting port mode to %s(%d) on output",
                        asString(mode), mode);
                err = StatusFromOMXError(OMX_ErrorUnsupportedIndex);
                break;
            }

            err = enableNativeBuffers_l(
                    portIndex, OMX_TRUE /*graphic*/, OMX_TRUE);
            if (err != OK) {
                break;
            }
        }
        (void)enableNativeBuffers_l(portIndex, OMX_FALSE /*graphic*/, OMX_FALSE);
        err = storeMetaDataInBuffers_l(portIndex, OMX_TRUE, NULL);
        break;
    }

    case IOMX::kPortModeDynamicNativeHandle:
    {
        if (portIndex != kPortIndexInput) {
            CLOG_ERROR(setPortMode, BAD_VALUE,
                    "%s(%d) mode is only supported on input port", asString(mode), mode);
            err = BAD_VALUE;
            break;
        }
        (void)enableNativeBuffers_l(portIndex, OMX_TRUE /*graphic*/, OMX_FALSE);
        (void)enableNativeBuffers_l(portIndex, OMX_FALSE /*graphic*/, OMX_FALSE);

        MetadataBufferType metaType = kMetadataBufferTypeNativeHandleSource;
        err = storeMetaDataInBuffers_l(portIndex, OMX_TRUE, &metaType);
        break;
    }

    case IOMX::kPortModePresetSecureBuffer:
    {
        // Allow on both input and output.
        (void)storeMetaDataInBuffers_l(portIndex, OMX_FALSE, NULL);
        (void)enableNativeBuffers_l(portIndex, OMX_TRUE /*graphic*/, OMX_FALSE);
        err = enableNativeBuffers_l(portIndex, OMX_FALSE /*graphic*/, OMX_TRUE);
        break;
    }

    case IOMX::kPortModePresetANWBuffer:
    {
        if (portIndex != kPortIndexOutput) {
            CLOG_ERROR(setPortMode, BAD_VALUE,
                    "%s(%d) mode is only supported on output port", asString(mode), mode);
            err = BAD_VALUE;
            break;
        }

        // Check if we're simulating legacy mode with metadata mode,
        // if so, enable metadata mode.
        if (mLegacyAdaptiveExperiment) {
            if (storeMetaDataInBuffers_l(portIndex, OMX_TRUE, NULL) == OK) {
                CLOG_INTERNAL(setPortMode, "Legacy adaptive experiment: "
                        "metdata mode enabled successfully");
                break;
            }

            CLOG_INTERNAL(setPortMode, "Legacy adaptive experiment: "
                    "unable to enable metadata mode on output");

            mLegacyAdaptiveExperiment = false;
        }

        // Disable secure buffer and enable graphic buffer
        (void)enableNativeBuffers_l(portIndex, OMX_FALSE /*graphic*/, OMX_FALSE);
        err = enableNativeBuffers_l(portIndex, OMX_TRUE /*graphic*/, OMX_TRUE);
        if (err != OK) {
            break;
        }

        // Not running experiment, or metadata is not supported.
        // Disable metadata mode and use legacy mode.
        (void)storeMetaDataInBuffers_l(portIndex, OMX_FALSE, NULL);
        break;
    }

    case IOMX::kPortModePresetByteBuffer:
    {
        // Disable secure buffer, native buffer and metadata.
        (void)enableNativeBuffers_l(portIndex, OMX_TRUE /*graphic*/, OMX_FALSE);
        (void)enableNativeBuffers_l(portIndex, OMX_FALSE /*graphic*/, OMX_FALSE);
        (void)storeMetaDataInBuffers_l(portIndex, OMX_FALSE, NULL);
        break;
    }

    default:
        CLOG_ERROR(setPortMode, BAD_VALUE, "invalid port mode %d", mode);
        err = BAD_VALUE;
        break;
    }

    if (err == OK) {
        mPortMode[portIndex] = mode;
    }
    return err;
}

status_t OMXNodeInstance::enableNativeBuffers_l(
        OMX_U32 portIndex, OMX_BOOL graphic, OMX_BOOL enable) {
    if (portIndex >= NELEM(mSecureBufferType)) {
        ALOGE("b/31385713, portIndex(%u)", portIndex);
        android_errorWriteLog(0x534e4554, "31385713");
        return BAD_VALUE;
    }

    CLOG_CONFIG(enableNativeBuffers, "%s:%u%s, %d", portString(portIndex), portIndex,
                graphic ? ", graphic" : "", enable);
    OMX_STRING name = const_cast<OMX_STRING>(
            graphic ? "OMX.google.android.index.enableAndroidNativeBuffers"
                    : "OMX.google.android.index.allocateNativeHandle");

    OMX_INDEXTYPE index;
    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mHandle, name, &index);

    if (err == OMX_ErrorNone) {
        EnableAndroidNativeBuffersParams params;
        InitOMXParams(&params);
        params.nPortIndex = portIndex;
        params.enable = enable;

        err = OMX_SetParameter(mHandle, index, &params);
        CLOG_IF_ERROR(setParameter, err, "%s(%#x): %s:%u en=%d", name, index,
                      portString(portIndex), portIndex, enable);
        if (!graphic) {
            if (err == OMX_ErrorNone) {
                mSecureBufferType[portIndex] =
                    enable ? kSecureBufferTypeNativeHandle : kSecureBufferTypeOpaque;
            } else if (mSecureBufferType[portIndex] == kSecureBufferTypeUnknown) {
                mSecureBufferType[portIndex] = kSecureBufferTypeOpaque;
            }
        } else {
            if (err == OMX_ErrorNone) {
                mGraphicBufferEnabled[portIndex] = enable;
            } else if (enable) {
                mGraphicBufferEnabled[portIndex] = false;
            }
        }
    } else {
        CLOG_ERROR_IF(enable, getExtensionIndex, err, "%s", name);
        if (!graphic) {
            // Extension not supported, check for manual override with system property
            // This is a temporary workaround until partners support the OMX extension
            if (property_get_bool("media.mediadrmservice.enable", false)) {
                CLOG_CONFIG(enableNativeBuffers, "system property override: using native-handles");
                mSecureBufferType[portIndex] = kSecureBufferTypeNativeHandle;
            } else if (mSecureBufferType[portIndex] == kSecureBufferTypeUnknown) {
                mSecureBufferType[portIndex] = kSecureBufferTypeOpaque;
            }
            err = OMX_ErrorNone;
        }
    }

    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::getGraphicBufferUsage(
        OMX_U32 portIndex, OMX_U32* usage) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    OMX_INDEXTYPE index;
    OMX_STRING name = const_cast<OMX_STRING>(
            "OMX.google.android.index.getAndroidNativeBufferUsage");
    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mHandle, name, &index);

    if (err != OMX_ErrorNone) {
        CLOG_ERROR(getExtensionIndex, err, "%s", name);
        return StatusFromOMXError(err);
    }

    GetAndroidNativeBufferUsageParams params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;

    err = OMX_GetParameter(mHandle, index, &params);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR(getParameter, err, "%s(%#x): %s:%u", name, index,
                portString(portIndex), portIndex);
        return StatusFromOMXError(err);
    }

    *usage = params.nUsage;

    return OK;
}

status_t OMXNodeInstance::storeMetaDataInBuffers_l(
        OMX_U32 portIndex, OMX_BOOL enable, MetadataBufferType *type) {
    if (mSailed) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }
    if (portIndex != kPortIndexInput && portIndex != kPortIndexOutput) {
        android_errorWriteLog(0x534e4554, "26324358");
        if (type != NULL) {
            *type = kMetadataBufferTypeInvalid;
        }
        return BAD_VALUE;
    }

    OMX_INDEXTYPE index;
    OMX_STRING name = const_cast<OMX_STRING>(
            "OMX.google.android.index.storeMetaDataInBuffers");

    OMX_STRING nativeBufferName = const_cast<OMX_STRING>(
            "OMX.google.android.index.storeANWBufferInMetadata");
    MetadataBufferType negotiatedType;
    MetadataBufferType requestedType = type != NULL ? *type : kMetadataBufferTypeANWBuffer;

    StoreMetaDataInBuffersParams params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;
    params.bStoreMetaData = enable;

    OMX_ERRORTYPE err =
        requestedType == kMetadataBufferTypeANWBuffer
                ? OMX_GetExtensionIndex(mHandle, nativeBufferName, &index)
                : OMX_ErrorUnsupportedIndex;
    OMX_ERRORTYPE xerr = err;
    if (err == OMX_ErrorNone) {
        err = OMX_SetParameter(mHandle, index, &params);
        if (err == OMX_ErrorNone) {
            name = nativeBufferName; // set name for debugging
            negotiatedType = requestedType;
        }
    }
    if (err != OMX_ErrorNone) {
        err = OMX_GetExtensionIndex(mHandle, name, &index);
        xerr = err;
        if (err == OMX_ErrorNone) {
            negotiatedType =
                requestedType == kMetadataBufferTypeANWBuffer
                        ? kMetadataBufferTypeGrallocSource : requestedType;
            err = OMX_SetParameter(mHandle, index, &params);
        }
        if (err == OMX_ErrorBadParameter) {
            err = OMX_ErrorUnsupportedIndex;
        }
    }

    // don't log loud error if component does not support metadata mode on the output
    if (err != OMX_ErrorNone) {
        if (err == OMX_ErrorUnsupportedIndex && portIndex == kPortIndexOutput) {
            CLOGW("component does not support metadata mode; using fallback");
        } else if (xerr != OMX_ErrorNone) {
            CLOG_ERROR(getExtensionIndex, xerr, "%s", name);
        } else {
            CLOG_ERROR(setParameter, err, "%s(%#x): %s:%u en=%d type=%d", name, index,
                    portString(portIndex), portIndex, enable, negotiatedType);
        }
        negotiatedType = mMetadataType[portIndex];
    } else {
        if (!enable) {
            negotiatedType = kMetadataBufferTypeInvalid;
        }
        mMetadataType[portIndex] = negotiatedType;
    }
    CLOG_CONFIG(storeMetaDataInBuffers, "%s:%u %srequested %s:%d negotiated %s:%d",
            portString(portIndex), portIndex, enable ? "" : "UN",
            asString(requestedType), requestedType, asString(negotiatedType), negotiatedType);

    if (type != NULL) {
        *type = negotiatedType;
    }

    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::prepareForAdaptivePlayback(
        OMX_U32 portIndex, OMX_BOOL enable, OMX_U32 maxFrameWidth,
        OMX_U32 maxFrameHeight) {
    Mutex::Autolock autolock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (mSailed) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }
    CLOG_CONFIG(prepareForAdaptivePlayback, "%s:%u en=%d max=%ux%u",
            portString(portIndex), portIndex, enable, maxFrameWidth, maxFrameHeight);

    if (mLegacyAdaptiveExperiment) {
        CLOG_INTERNAL(prepareForAdaptivePlayback,
                "Legacy adaptive experiment: reporting success");
        return OK;
    }

    OMX_INDEXTYPE index;
    OMX_STRING name = const_cast<OMX_STRING>(
            "OMX.google.android.index.prepareForAdaptivePlayback");

    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mHandle, name, &index);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR_IF(enable, getExtensionIndex, err, "%s", name);
        return StatusFromOMXError(err);
    }

    PrepareForAdaptivePlaybackParams params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;
    params.bEnable = enable;
    params.nMaxFrameWidth = maxFrameWidth;
    params.nMaxFrameHeight = maxFrameHeight;

    err = OMX_SetParameter(mHandle, index, &params);
    CLOG_IF_ERROR(setParameter, err, "%s(%#x): %s:%u en=%d max=%ux%u", name, index,
            portString(portIndex), portIndex, enable, maxFrameWidth, maxFrameHeight);
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::configureVideoTunnelMode(
        OMX_U32 portIndex, OMX_BOOL tunneled, OMX_U32 audioHwSync,
        native_handle_t **sidebandHandle) {
    Mutex::Autolock autolock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (mSailed) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }
    CLOG_CONFIG(configureVideoTunnelMode, "%s:%u tun=%d sync=%u",
            portString(portIndex), portIndex, tunneled, audioHwSync);

    OMX_INDEXTYPE index;
    OMX_STRING name = const_cast<OMX_STRING>(
            "OMX.google.android.index.configureVideoTunnelMode");

    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mHandle, name, &index);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR_IF(tunneled, getExtensionIndex, err, "%s", name);
        return StatusFromOMXError(err);
    }

    ConfigureVideoTunnelModeParams tunnelParams;
    InitOMXParams(&tunnelParams);
    tunnelParams.nPortIndex = portIndex;
    tunnelParams.bTunneled = tunneled;
    tunnelParams.nAudioHwSync = audioHwSync;
    err = OMX_SetParameter(mHandle, index, &tunnelParams);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR(setParameter, err, "%s(%#x): %s:%u tun=%d sync=%u", name, index,
                portString(portIndex), portIndex, tunneled, audioHwSync);
        return StatusFromOMXError(err);
    }

    err = OMX_GetParameter(mHandle, index, &tunnelParams);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR(getParameter, err, "%s(%#x): %s:%u tun=%d sync=%u", name, index,
                portString(portIndex), portIndex, tunneled, audioHwSync);
        return StatusFromOMXError(err);
    }
    if (sidebandHandle) {
        *sidebandHandle = (native_handle_t*)tunnelParams.pSidebandWindow;
    }

    return OK;
}

status_t OMXNodeInstance::useBuffer(
        OMX_U32 portIndex, const OMXBuffer &omxBuffer, IOMX::buffer_id *buffer) {
    if (buffer == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    if (portIndex >= NELEM(mNumPortBuffers)) {
        return BAD_VALUE;
    }

    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (!mSailed) {
        ALOGE("b/35467458");
        android_errorWriteLog(0x534e4554, "35467458");
        return BAD_VALUE;
    }

    switch (omxBuffer.mBufferType) {
        case OMXBuffer::kBufferTypePreset: {
            if (mPortMode[portIndex] != IOMX::kPortModeDynamicANWBuffer
                    && mPortMode[portIndex] != IOMX::kPortModeDynamicNativeHandle) {
                break;
            }
            return useBuffer_l(portIndex, NULL, NULL, buffer);
        }

        case OMXBuffer::kBufferTypeSharedMem: {
            if (mPortMode[portIndex] != IOMX::kPortModePresetByteBuffer
                    && mPortMode[portIndex] != IOMX::kPortModeDynamicANWBuffer) {
                break;
            }
            return useBuffer_l(portIndex, omxBuffer.mMem, NULL, buffer);
        }

        case OMXBuffer::kBufferTypeANWBuffer: {
            if (mPortMode[portIndex] != IOMX::kPortModePresetANWBuffer
                    && mPortMode[portIndex] != IOMX::kPortModeDynamicANWBuffer) {
                break;
            }
            return useGraphicBuffer_l(portIndex, omxBuffer.mGraphicBuffer, buffer);
        }

        case OMXBuffer::kBufferTypeHidlMemory: {
                if (mPortMode[portIndex] != IOMX::kPortModePresetByteBuffer
                        && mPortMode[portIndex] != IOMX::kPortModeDynamicANWBuffer
                        && mPortMode[portIndex] != IOMX::kPortModeDynamicNativeHandle) {
                    break;
                }
                sp<IHidlMemory> hidlMemory = mapMemory(omxBuffer.mHidlMemory);
                if (hidlMemory == nullptr) {
                    ALOGE("OMXNodeInstance useBuffer() failed to map memory");
                    return NO_MEMORY;
                }
                return useBuffer_l(portIndex, NULL, hidlMemory, buffer);
        }
        default:
            return BAD_VALUE;
            break;
    }

    ALOGE("b/77486542 : bufferType = %d vs. portMode = %d",
          omxBuffer.mBufferType, mPortMode[portIndex]);
    android_errorWriteLog(0x534e4554, "77486542");
    return INVALID_OPERATION;
}

status_t OMXNodeInstance::useBuffer_l(
        OMX_U32 portIndex, const sp<IMemory> &params,
        const sp<IHidlMemory> &hParams, IOMX::buffer_id *buffer) {
    BufferMeta *buffer_meta;
    OMX_BUFFERHEADERTYPE *header;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    bool isMetadata = mMetadataType[portIndex] != kMetadataBufferTypeInvalid;

    if (!isMetadata && mGraphicBufferEnabled[portIndex]) {
        ALOGE("b/62948670");
        android_errorWriteLog(0x534e4554, "62948670");
        return INVALID_OPERATION;
    }

    size_t paramsSize;
    void* paramsPointer;
    if (params != NULL && hParams != NULL) {
        return BAD_VALUE;
    }
    if (params != NULL) {
        paramsPointer = params->pointer();
        paramsSize = params->size();
    } else if (hParams != NULL) {
        paramsPointer = hParams->getPointer();
        paramsSize = hParams->getSize();
    } else {
        paramsPointer = nullptr;
    }

    OMX_U32 allottedSize;
    if (isMetadata) {
        if (mMetadataType[portIndex] == kMetadataBufferTypeGrallocSource) {
            allottedSize = sizeof(VideoGrallocMetadata);
        } else if (mMetadataType[portIndex] == kMetadataBufferTypeANWBuffer) {
            allottedSize = sizeof(VideoNativeMetadata);
        } else if (mMetadataType[portIndex] == kMetadataBufferTypeNativeHandleSource) {
            allottedSize = sizeof(VideoNativeHandleMetadata);
        } else {
            return BAD_VALUE;
        }
    } else {
        // NULL params is allowed only in metadata mode.
        if (paramsPointer == nullptr) {
            ALOGE("b/25884056");
            return BAD_VALUE;
        }
        allottedSize = paramsSize;
    }

    bool isOutputGraphicMetadata = (portIndex == kPortIndexOutput) &&
            (mMetadataType[portIndex] == kMetadataBufferTypeGrallocSource ||
                    mMetadataType[portIndex] == kMetadataBufferTypeANWBuffer);

    uint32_t requiresAllocateBufferBit =
        (portIndex == kPortIndexInput)
            ? kRequiresAllocateBufferOnInputPorts
            : kRequiresAllocateBufferOnOutputPorts;

    // we use useBuffer for output metadata regardless of quirks
    if (!isOutputGraphicMetadata && (mQuirks & requiresAllocateBufferBit)) {
        // metadata buffers are not connected cross process; only copy if not meta.
        buffer_meta = new BufferMeta(
                    params, hParams, portIndex, !isMetadata /* copy */, NULL /* data */);

        err = OMX_AllocateBuffer(
                mHandle, &header, portIndex, buffer_meta, allottedSize);

        if (err != OMX_ErrorNone) {
            CLOG_ERROR(allocateBuffer, err,
                    SIMPLE_BUFFER(portIndex, (size_t)allottedSize,
                            paramsPointer));
        }
    } else {
        OMX_U8 *data = NULL;

        // metadata buffers are not connected cross process
        // use a backup buffer instead of the actual buffer
        if (isMetadata) {
            data = new (std::nothrow) OMX_U8[allottedSize];
            if (data == NULL) {
                return NO_MEMORY;
            }
            memset(data, 0, allottedSize);

            buffer_meta = new BufferMeta(
                    params, hParams, portIndex, false /* copy */, data);
        } else {
            data = static_cast<OMX_U8 *>(paramsPointer);

            buffer_meta = new BufferMeta(
                    params, hParams, portIndex, false /* copy */, NULL);
        }

        err = OMX_UseBuffer(
                mHandle, &header, portIndex, buffer_meta,
                allottedSize, data);

        if (err != OMX_ErrorNone) {
            CLOG_ERROR(useBuffer, err, SIMPLE_BUFFER(
                    portIndex, (size_t)allottedSize, data));
        }
    }

    if (err != OMX_ErrorNone) {
        delete buffer_meta;
        buffer_meta = NULL;

        *buffer = 0;

        return StatusFromOMXError(err);
    }

    CHECK_EQ(header->pAppPrivate, buffer_meta);

    *buffer = makeBufferID(header);

    addActiveBuffer(portIndex, *buffer);

    sp<IOMXBufferSource> bufferSource(getBufferSource());
    if (bufferSource != NULL && portIndex == kPortIndexInput) {
        bufferSource->onInputBufferAdded(*buffer);
    }

    CLOG_BUFFER(useBuffer, NEW_BUFFER_FMT(
            *buffer, portIndex, "%u(%zu)@%p", allottedSize, paramsSize, paramsPointer));
    return OK;
}

status_t OMXNodeInstance::useGraphicBuffer2_l(
        OMX_U32 portIndex, const sp<GraphicBuffer>& graphicBuffer,
        IOMX::buffer_id *buffer) {
    if (graphicBuffer == NULL || buffer == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    // port definition
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;
    OMX_ERRORTYPE err = OMX_GetParameter(mHandle, OMX_IndexParamPortDefinition, &def);
    if (err != OMX_ErrorNone) {
        OMX_INDEXTYPE index = OMX_IndexParamPortDefinition;
        CLOG_ERROR(getParameter, err, "%s(%#x): %s:%u",
                asString(index), index, portString(portIndex), portIndex);
        return UNKNOWN_ERROR;
    }

    BufferMeta *bufferMeta = new BufferMeta(graphicBuffer, portIndex);

    OMX_BUFFERHEADERTYPE *header = NULL;
    OMX_U8* bufferHandle = const_cast<OMX_U8*>(
            reinterpret_cast<const OMX_U8*>(graphicBuffer->handle));

    err = OMX_UseBuffer(
            mHandle,
            &header,
            portIndex,
            bufferMeta,
            def.nBufferSize,
            bufferHandle);

    if (err != OMX_ErrorNone) {
        CLOG_ERROR(useBuffer, err, BUFFER_FMT(portIndex, "%u@%p", def.nBufferSize, bufferHandle));
        delete bufferMeta;
        bufferMeta = NULL;
        *buffer = 0;
        return StatusFromOMXError(err);
    }

    CHECK_EQ(header->pBuffer, bufferHandle);
    CHECK_EQ(header->pAppPrivate, bufferMeta);

    *buffer = makeBufferID(header);

    addActiveBuffer(portIndex, *buffer);
    CLOG_BUFFER(useGraphicBuffer2, NEW_BUFFER_FMT(
            *buffer, portIndex, "%u@%p", def.nBufferSize, bufferHandle));
    return OK;
}

// XXX: This function is here for backwards compatibility.  Once the OMX
// implementations have been updated this can be removed and useGraphicBuffer2
// can be renamed to useGraphicBuffer.
status_t OMXNodeInstance::useGraphicBuffer_l(
        OMX_U32 portIndex, const sp<GraphicBuffer>& graphicBuffer,
        IOMX::buffer_id *buffer) {
    if (graphicBuffer == NULL || buffer == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    // First, see if we're in metadata mode. We could be running an experiment to simulate
    // legacy behavior (preallocated buffers) on devices that supports meta.
    if (mMetadataType[portIndex] != kMetadataBufferTypeInvalid) {
        return useGraphicBufferWithMetadata_l(
                portIndex, graphicBuffer, buffer);
    }

    if (!mGraphicBufferEnabled[portIndex]) {
        // Report error if this is not in graphic buffer mode.
        ALOGE("b/62948670");
        android_errorWriteLog(0x534e4554, "62948670");
        return INVALID_OPERATION;
    }

    // See if the newer version of the extension is present.
    OMX_INDEXTYPE index;
    if (OMX_GetExtensionIndex(
            mHandle,
            const_cast<OMX_STRING>("OMX.google.android.index.useAndroidNativeBuffer2"),
            &index) == OMX_ErrorNone) {
        return useGraphicBuffer2_l(portIndex, graphicBuffer, buffer);
    }

    OMX_STRING name = const_cast<OMX_STRING>(
        "OMX.google.android.index.useAndroidNativeBuffer");
    OMX_ERRORTYPE err = OMX_GetExtensionIndex(mHandle, name, &index);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR(getExtensionIndex, err, "%s", name);
        return StatusFromOMXError(err);
    }

    BufferMeta *bufferMeta = new BufferMeta(graphicBuffer, portIndex);

    OMX_BUFFERHEADERTYPE *header;

    OMX_VERSIONTYPE ver;
    ver.s.nVersionMajor = 1;
    ver.s.nVersionMinor = 0;
    ver.s.nRevision = 0;
    ver.s.nStep = 0;
    UseAndroidNativeBufferParams params = {
        sizeof(UseAndroidNativeBufferParams), ver, portIndex, bufferMeta,
        &header, graphicBuffer,
    };

    err = OMX_SetParameter(mHandle, index, &params);

    if (err != OMX_ErrorNone) {
        CLOG_ERROR(setParameter, err, "%s(%#x): %s:%u meta=%p GB=%p", name, index,
                portString(portIndex), portIndex, bufferMeta, graphicBuffer->handle);

        delete bufferMeta;
        bufferMeta = NULL;

        *buffer = 0;

        return StatusFromOMXError(err);
    }

    CHECK_EQ(header->pAppPrivate, bufferMeta);

    *buffer = makeBufferID(header);

    addActiveBuffer(portIndex, *buffer);
    CLOG_BUFFER(useGraphicBuffer, NEW_BUFFER_FMT(
            *buffer, portIndex, "GB=%p", graphicBuffer->handle));
    return OK;
}

status_t OMXNodeInstance::useGraphicBufferWithMetadata_l(
        OMX_U32 portIndex, const sp<GraphicBuffer> &graphicBuffer,
        IOMX::buffer_id *buffer) {
    if (portIndex != kPortIndexOutput) {
        return BAD_VALUE;
    }

    if (mMetadataType[portIndex] != kMetadataBufferTypeGrallocSource &&
            mMetadataType[portIndex] != kMetadataBufferTypeANWBuffer) {
        return BAD_VALUE;
    }

    status_t err = useBuffer_l(portIndex, NULL, NULL, buffer);
    if (err != OK) {
        return err;
    }

    OMX_BUFFERHEADERTYPE *header = findBufferHeader(*buffer, portIndex);

    return updateGraphicBufferInMeta_l(portIndex, graphicBuffer, *buffer, header);

}

status_t OMXNodeInstance::updateGraphicBufferInMeta_l(
        OMX_U32 portIndex, const sp<GraphicBuffer>& graphicBuffer,
        IOMX::buffer_id buffer, OMX_BUFFERHEADERTYPE *header) {
    // No need to check |graphicBuffer| since NULL is valid for it as below.
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    if (portIndex != kPortIndexInput && portIndex != kPortIndexOutput) {
        return BAD_VALUE;
    }

    BufferMeta *bufferMeta = (BufferMeta *)(header->pAppPrivate);
    sp<ABuffer> data = bufferMeta->getBuffer(header, false /* limit */);
    bufferMeta->setGraphicBuffer(graphicBuffer);
    MetadataBufferType metaType = mMetadataType[portIndex];
    if (metaType == kMetadataBufferTypeGrallocSource
            && data->capacity() >= sizeof(VideoGrallocMetadata)) {
        VideoGrallocMetadata &metadata = *(VideoGrallocMetadata *)(data->data());
        metadata.eType = kMetadataBufferTypeGrallocSource;
        metadata.pHandle = graphicBuffer == NULL ? NULL : graphicBuffer->handle;
    } else if (metaType == kMetadataBufferTypeANWBuffer
            && data->capacity() >= sizeof(VideoNativeMetadata)) {
        VideoNativeMetadata &metadata = *(VideoNativeMetadata *)(data->data());
        metadata.eType = kMetadataBufferTypeANWBuffer;
        metadata.pBuffer = graphicBuffer == NULL ? NULL : graphicBuffer->getNativeBuffer();
        metadata.nFenceFd = -1;
    } else {
        CLOG_ERROR(updateGraphicBufferInMeta, BAD_VALUE, "%s:%u, %#x bad type (%d) or size (%u)",
            portString(portIndex), portIndex, buffer, mMetadataType[portIndex], header->nAllocLen);
        return BAD_VALUE;
    }

    CLOG_BUFFER(updateGraphicBufferInMeta, "%s:%u, %#x := %p",
            portString(portIndex), portIndex, buffer,
            graphicBuffer == NULL ? NULL : graphicBuffer->handle);
    return OK;
}

status_t OMXNodeInstance::updateNativeHandleInMeta_l(
        OMX_U32 portIndex, const sp<NativeHandle>& nativeHandle,
        IOMX::buffer_id buffer, OMX_BUFFERHEADERTYPE *header) {
    // No need to check |nativeHandle| since NULL is valid for it as below.
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    if (portIndex != kPortIndexInput && portIndex != kPortIndexOutput) {
        return BAD_VALUE;
    }

    BufferMeta *bufferMeta = (BufferMeta *)(header->pAppPrivate);
    sp<ABuffer> data = bufferMeta->getBuffer(header, false /* limit */);
    bufferMeta->setNativeHandle(nativeHandle);
    if (mMetadataType[portIndex] == kMetadataBufferTypeNativeHandleSource
            && data->capacity() >= sizeof(VideoNativeHandleMetadata)) {
        VideoNativeHandleMetadata &metadata = *(VideoNativeHandleMetadata *)(data->data());
        metadata.eType = mMetadataType[portIndex];
        metadata.pHandle =
            nativeHandle == NULL ? NULL : const_cast<native_handle*>(nativeHandle->handle());
    } else {
        CLOG_ERROR(updateNativeHandleInMeta, BAD_VALUE, "%s:%u, %#x bad type (%d) or size (%zu)",
            portString(portIndex), portIndex, buffer, mMetadataType[portIndex], data->capacity());
        return BAD_VALUE;
    }

    CLOG_BUFFER(updateNativeHandleInMeta, "%s:%u, %#x := %p",
            portString(portIndex), portIndex, buffer,
            nativeHandle == NULL ? NULL : nativeHandle->handle());
    return OK;
}

status_t OMXNodeInstance::setInputSurface(
        const sp<IOMXBufferSource> &bufferSource) {
    Mutex::Autolock autolock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    status_t err;

    // only allow graphic source on input port, when there are no allocated buffers yet
    if (mNumPortBuffers[kPortIndexInput] > 0) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }

    if (getBufferSource() != NULL) {
        return ALREADY_EXISTS;
    }

    err = storeMetaDataInBuffers_l(kPortIndexInput, OMX_TRUE, NULL);
    if (err != OK) {
        return err;
    }

    // Retrieve the width and height of the graphic buffer, set when the
    // codec was configured.
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;
    OMX_ERRORTYPE oerr = OMX_GetParameter(
            mHandle, OMX_IndexParamPortDefinition, &def);
    if (oerr != OMX_ErrorNone) {
        OMX_INDEXTYPE index = OMX_IndexParamPortDefinition;
        CLOG_ERROR(getParameter, oerr, "%s(%#x): %s:%u", asString(index),
                index, portString(kPortIndexInput), kPortIndexInput);
        return UNKNOWN_ERROR;
    }

    if (def.format.video.eColorFormat != OMX_COLOR_FormatAndroidOpaque) {
        CLOGW("createInputSurface requires COLOR_FormatSurface "
                "(AndroidOpaque) color format instead of %s(%#x)",
                asString(def.format.video.eColorFormat), def.format.video.eColorFormat);
        return INVALID_OPERATION;
    }

    if (def.format.video.nFrameWidth == 0
            || def.format.video.nFrameHeight == 0) {
        ALOGE("Invalid video dimension %ux%u",
                def.format.video.nFrameWidth,
                def.format.video.nFrameHeight);
        return BAD_VALUE;
    }

    setBufferSource(bufferSource);
    return OK;
}

status_t OMXNodeInstance::allocateSecureBuffer(
        OMX_U32 portIndex, size_t size, IOMX::buffer_id *buffer,
        void **buffer_data, sp<NativeHandle> *native_handle) {
    if (buffer == NULL || buffer_data == NULL || native_handle == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    if (portIndex >= NELEM(mSecureBufferType)) {
        ALOGE("b/31385713, portIndex(%u)", portIndex);
        android_errorWriteLog(0x534e4554, "31385713");
        return BAD_VALUE;
    }

    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    if (!mSailed) {
        ALOGE("b/35467458");
        android_errorWriteLog(0x534e4554, "35467458");
        return BAD_VALUE;
    }
    if (mPortMode[portIndex] != IOMX::kPortModePresetSecureBuffer) {
        ALOGE("b/77486542");
        android_errorWriteLog(0x534e4554, "77486542");
        return INVALID_OPERATION;
    }
    BufferMeta *buffer_meta = new BufferMeta(portIndex);

    OMX_BUFFERHEADERTYPE *header;

    OMX_ERRORTYPE err = OMX_AllocateBuffer(
            mHandle, &header, portIndex, buffer_meta, size);

    if (err != OMX_ErrorNone) {
        CLOG_ERROR(allocateBuffer, err, BUFFER_FMT(portIndex, "%zu@", size));
        delete buffer_meta;
        buffer_meta = NULL;

        *buffer = 0;

        return StatusFromOMXError(err);
    }

    CHECK_EQ(header->pAppPrivate, buffer_meta);

    *buffer = makeBufferID(header);
    if (mSecureBufferType[portIndex] == kSecureBufferTypeNativeHandle) {
        *buffer_data = NULL;
        *native_handle = NativeHandle::create(
                (native_handle_t *)header->pBuffer, false /* ownsHandle */);
    } else {
        *buffer_data = header->pBuffer;
        *native_handle = NULL;
    }

    addActiveBuffer(portIndex, *buffer);

    sp<IOMXBufferSource> bufferSource(getBufferSource());
    if (bufferSource != NULL && portIndex == kPortIndexInput) {
        bufferSource->onInputBufferAdded(*buffer);
    }
    CLOG_BUFFER(allocateSecureBuffer, NEW_BUFFER_FMT(
            *buffer, portIndex, "%zu@%p:%p", size, *buffer_data,
            *native_handle == NULL ? NULL : (*native_handle)->handle()));

    return OK;
}

status_t OMXNodeInstance::freeBuffer(
        OMX_U32 portIndex, IOMX::buffer_id buffer) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    CLOG_BUFFER(freeBuffer, "%s:%u %#x", portString(portIndex), portIndex, buffer);

    removeActiveBuffer(portIndex, buffer);

    OMX_BUFFERHEADERTYPE *header = findBufferHeader(buffer, portIndex);
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }
    BufferMeta *buffer_meta = static_cast<BufferMeta *>(header->pAppPrivate);

    // Invalidate buffers in the client side first before calling OMX_FreeBuffer.
    // If not, pending events in the client side might access the buffers after free.
    invalidateBufferID(buffer);

    OMX_ERRORTYPE err = OMX_FreeBuffer(mHandle, portIndex, header);
    CLOG_IF_ERROR(freeBuffer, err, "%s:%u %#x", portString(portIndex), portIndex, buffer);

    delete buffer_meta;
    buffer_meta = NULL;

    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::fillBuffer(
        IOMX::buffer_id buffer, const OMXBuffer &omxBuffer, int fenceFd) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    OMX_BUFFERHEADERTYPE *header = findBufferHeader(buffer, kPortIndexOutput);
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    if (omxBuffer.mBufferType == OMXBuffer::kBufferTypeANWBuffer) {
        status_t err = updateGraphicBufferInMeta_l(
                kPortIndexOutput, omxBuffer.mGraphicBuffer, buffer, header);

        if (err != OK) {
            CLOG_ERROR(fillBuffer, err, FULL_BUFFER(
                    (intptr_t)header->pBuffer, header, fenceFd));
            return err;
        }
    } else if (omxBuffer.mBufferType != OMXBuffer::kBufferTypePreset) {
        return BAD_VALUE;
    }

    header->nFilledLen = 0;
    header->nOffset = 0;
    header->nFlags = 0;

    // meta now owns fenceFd
    status_t res = storeFenceInMeta_l(header, fenceFd, kPortIndexOutput);
    if (res != OK) {
        CLOG_ERROR(fillBuffer::storeFenceInMeta, res, EMPTY_BUFFER(buffer, header, fenceFd));
        return res;
    }

    {
        Mutex::Autolock _l(mDebugLock);
        mOutputBuffersWithCodec.add(header);
        CLOG_BUMPED_BUFFER(fillBuffer, WITH_STATS(EMPTY_BUFFER(buffer, header, fenceFd)));
    }

    OMX_ERRORTYPE err = OMX_FillThisBuffer(mHandle, header);
    if (err != OMX_ErrorNone) {
        CLOG_ERROR(fillBuffer, err, EMPTY_BUFFER(buffer, header, fenceFd));
        Mutex::Autolock _l(mDebugLock);
        mOutputBuffersWithCodec.remove(header);
    }
    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::emptyBuffer(
        buffer_id buffer, const OMXBuffer &omxBuffer,
        OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    switch (omxBuffer.mBufferType) {
    case OMXBuffer::kBufferTypePreset:
        return emptyBuffer_l(
                buffer, omxBuffer.mRangeOffset, omxBuffer.mRangeLength,
                flags, timestamp, fenceFd);

    case OMXBuffer::kBufferTypeANWBuffer:
        return emptyGraphicBuffer_l(
                buffer, omxBuffer.mGraphicBuffer, flags, timestamp, fenceFd);

    case OMXBuffer::kBufferTypeNativeHandle:
        return emptyNativeHandleBuffer_l(
                buffer, omxBuffer.mNativeHandle, flags, timestamp, fenceFd);

    default:
        break;
    }

    return BAD_VALUE;
}

status_t OMXNodeInstance::emptyBuffer_l(
        IOMX::buffer_id buffer,
        OMX_U32 rangeOffset, OMX_U32 rangeLength,
        OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {

    // no emptybuffer if using input surface
    if (getBufferSource() != NULL) {
        android_errorWriteLog(0x534e4554, "29422020");
        return INVALID_OPERATION;
    }

    OMX_BUFFERHEADERTYPE *header = findBufferHeader(buffer, kPortIndexInput);
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }
    BufferMeta *buffer_meta =
        static_cast<BufferMeta *>(header->pAppPrivate);

    // set up proper filled length if component is configured for gralloc metadata mode
    // ignore rangeOffset in this case (as client may be assuming ANW meta buffers).
    if (mMetadataType[kPortIndexInput] == kMetadataBufferTypeGrallocSource) {
        header->nFilledLen = rangeLength ? sizeof(VideoGrallocMetadata) : 0;
        header->nOffset = 0;
    } else {
        // rangeLength and rangeOffset must be a subset of the allocated data in the buffer.
        // corner case: we permit rangeOffset == end-of-buffer with rangeLength == 0.
        if (rangeOffset > header->nAllocLen
                || rangeLength > header->nAllocLen - rangeOffset) {
            CLOG_ERROR(emptyBuffer, OMX_ErrorBadParameter, FULL_BUFFER(NULL, header, fenceFd));
            if (fenceFd >= 0) {
                ::close(fenceFd);
            }
            return BAD_VALUE;
        }
        header->nFilledLen = rangeLength;
        header->nOffset = rangeOffset;

        buffer_meta->CopyToOMX(header);
    }

    return emptyBuffer_l(header, flags, timestamp, (intptr_t)buffer, fenceFd);
}

// log queued buffer activity for the next few input and/or output frames
// if logging at internal state level
void OMXNodeInstance::bumpDebugLevel_l(size_t numInputBuffers, size_t numOutputBuffers) {
    if (DEBUG == ADebug::kDebugInternalState) {
        DEBUG_BUMP = ADebug::kDebugAll;
        if (numInputBuffers > 0) {
            mDebugLevelBumpPendingBuffers[kPortIndexInput] = numInputBuffers;
        }
        if (numOutputBuffers > 0) {
            mDebugLevelBumpPendingBuffers[kPortIndexOutput] = numOutputBuffers;
        }
    }
}

void OMXNodeInstance::unbumpDebugLevel_l(size_t portIndex) {
    if (mDebugLevelBumpPendingBuffers[portIndex]) {
        --mDebugLevelBumpPendingBuffers[portIndex];
    }
    if (!mDebugLevelBumpPendingBuffers[0]
            && !mDebugLevelBumpPendingBuffers[1]) {
        DEBUG_BUMP = DEBUG;
    }
}

status_t OMXNodeInstance::storeFenceInMeta_l(
        OMX_BUFFERHEADERTYPE *header, int fenceFd, OMX_U32 portIndex) {
    // propagate fence if component supports it; wait for it otherwise
    OMX_U32 metaSize = portIndex == kPortIndexInput ? header->nFilledLen : header->nAllocLen;
    if (mMetadataType[portIndex] == kMetadataBufferTypeANWBuffer
            && metaSize >= sizeof(VideoNativeMetadata)) {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(header->pBuffer);
        if (nativeMeta.nFenceFd >= 0) {
            ALOGE("fence (%d) already exists in meta", nativeMeta.nFenceFd);
            if (fenceFd >= 0) {
                ::close(fenceFd);
            }
            return ALREADY_EXISTS;
        }
        nativeMeta.nFenceFd = fenceFd;
    } else if (fenceFd >= 0) {
        CLOG_BUFFER(storeFenceInMeta, "waiting for fence %d", fenceFd);
        sp<Fence> fence = new Fence(fenceFd);
        return fence->wait(IOMX::kFenceTimeoutMs);
    }
    return OK;
}

int OMXNodeInstance::retrieveFenceFromMeta_l(
        OMX_BUFFERHEADERTYPE *header, OMX_U32 portIndex) {
    OMX_U32 metaSize = portIndex == kPortIndexInput ? header->nAllocLen : header->nFilledLen;
    int fenceFd = -1;
    if (mMetadataType[portIndex] == kMetadataBufferTypeANWBuffer
            && header->nAllocLen >= sizeof(VideoNativeMetadata)) {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(header->pBuffer);
        if (nativeMeta.eType == kMetadataBufferTypeANWBuffer) {
            fenceFd = nativeMeta.nFenceFd;
            nativeMeta.nFenceFd = -1;
        }
        if (metaSize < sizeof(nativeMeta) && fenceFd >= 0) {
            CLOG_ERROR(foundFenceInEmptyMeta, BAD_VALUE, FULL_BUFFER(
                    NULL, header, nativeMeta.nFenceFd));
            fenceFd = -1;
        }
    }
    return fenceFd;
}

status_t OMXNodeInstance::emptyBuffer_l(
        OMX_BUFFERHEADERTYPE *header, OMX_U32 flags, OMX_TICKS timestamp,
        intptr_t debugAddr, int fenceFd) {
    header->nFlags = flags;
    header->nTimeStamp = timestamp;

    status_t res = storeFenceInMeta_l(header, fenceFd, kPortIndexInput);
    if (res != OK) {
        CLOG_ERROR(emptyBuffer::storeFenceInMeta, res, WITH_STATS(
                FULL_BUFFER(debugAddr, header, fenceFd)));
        return res;
    }

    {
        Mutex::Autolock _l(mDebugLock);
        mInputBuffersWithCodec.add(header);

        // bump internal-state debug level for 2 input frames past a buffer with CSD
        if ((flags & OMX_BUFFERFLAG_CODECCONFIG) != 0) {
            bumpDebugLevel_l(2 /* numInputBuffers */, 0 /* numOutputBuffers */);
        }

        CLOG_BUMPED_BUFFER(emptyBuffer, WITH_STATS(FULL_BUFFER(debugAddr, header, fenceFd)));
    }

    OMX_ERRORTYPE err = OMX_EmptyThisBuffer(mHandle, header);
    CLOG_IF_ERROR(emptyBuffer, err, FULL_BUFFER(debugAddr, header, fenceFd));

    {
        Mutex::Autolock _l(mDebugLock);
        if (err != OMX_ErrorNone) {
            mInputBuffersWithCodec.remove(header);
        } else if (!(flags & OMX_BUFFERFLAG_CODECCONFIG)) {
            unbumpDebugLevel_l(kPortIndexInput);
        }
    }

    return StatusFromOMXError(err);
}

// like emptyBuffer, but the data is already in header->pBuffer
status_t OMXNodeInstance::emptyGraphicBuffer_l(
        IOMX::buffer_id buffer, const sp<GraphicBuffer> &graphicBuffer,
        OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {
    OMX_BUFFERHEADERTYPE *header = findBufferHeader(buffer, kPortIndexInput);
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    status_t err = updateGraphicBufferInMeta_l(
            kPortIndexInput, graphicBuffer, buffer, header);
    if (err != OK) {
        CLOG_ERROR(emptyGraphicBuffer, err, FULL_BUFFER(
                (intptr_t)header->pBuffer, header, fenceFd));
        return err;
    }

    int64_t codecTimeUs = getCodecTimestamp(timestamp);

    header->nOffset = 0;
    if (graphicBuffer == NULL) {
        header->nFilledLen = 0;
    } else if (mMetadataType[kPortIndexInput] == kMetadataBufferTypeGrallocSource) {
        header->nFilledLen = sizeof(VideoGrallocMetadata);
    } else {
        header->nFilledLen = sizeof(VideoNativeMetadata);
    }
    return emptyBuffer_l(header, flags, codecTimeUs, (intptr_t)header->pBuffer, fenceFd);
}

status_t OMXNodeInstance::setMaxPtsGapUs(const void *params, size_t size) {
    if (params == NULL || size != sizeof(OMX_PARAM_U32TYPE)) {
        CLOG_ERROR(setMaxPtsGapUs, BAD_VALUE, "invalid params (%p,%zu)", params, size);
        return BAD_VALUE;
    }

    // The incoming number is an int32_t contained in OMX_U32.
    // Cast to int32_t first then int64_t.
    mMaxTimestampGapUs = (int32_t)((OMX_PARAM_U32TYPE*)params)->nU32;

    return OK;
}

int64_t OMXNodeInstance::getCodecTimestamp(OMX_TICKS timestamp) {
    int64_t originalTimeUs = timestamp;

    if (mMaxTimestampGapUs > 0LL) {
        /* Cap timestamp gap between adjacent frames to specified max
         *
         * In the scenario of cast mirroring, encoding could be suspended for
         * prolonged periods. Limiting the pts gap to workaround the problem
         * where encoder's rate control logic produces huge frames after a
         * long period of suspension.
         */
        if (mPrevOriginalTimeUs >= 0LL) {
            int64_t timestampGapUs = originalTimeUs - mPrevOriginalTimeUs;
            timestamp = (timestampGapUs < mMaxTimestampGapUs ?
                timestampGapUs : mMaxTimestampGapUs) + mPrevModifiedTimeUs;
        }
        ALOGV("IN  timestamp: %lld -> %lld",
            static_cast<long long>(originalTimeUs),
            static_cast<long long>(timestamp));
    } else if (mMaxTimestampGapUs < 0LL) {
        /*
         * Apply a fixed timestamp gap between adjacent frames.
         *
         * This is used by scenarios like still image capture where timestamps
         * on frames could go forward or backward. Some encoders may silently
         * drop frames when it goes backward (or even stay unchanged).
         */
        if (mPrevOriginalTimeUs >= 0LL) {
            timestamp = mPrevModifiedTimeUs - mMaxTimestampGapUs;
        }
        ALOGV("IN  timestamp: %lld -> %lld",
            static_cast<long long>(originalTimeUs),
            static_cast<long long>(timestamp));
    }

    mPrevOriginalTimeUs = originalTimeUs;
    mPrevModifiedTimeUs = timestamp;

    if (mMaxTimestampGapUs != 0LL && !mRestorePtsFailed) {
        mOriginalTimeUs.add(timestamp, originalTimeUs);
    }

    return timestamp;
}

status_t OMXNodeInstance::emptyNativeHandleBuffer_l(
        IOMX::buffer_id buffer, const sp<NativeHandle> &nativeHandle,
        OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {
    OMX_BUFFERHEADERTYPE *header = findBufferHeader(buffer, kPortIndexInput);
    if (header == NULL) {
        ALOGE("b/25884056");
        return BAD_VALUE;
    }

    status_t err = updateNativeHandleInMeta_l(
            kPortIndexInput, nativeHandle, buffer, header);
    if (err != OK) {
        CLOG_ERROR(emptyNativeHandleBuffer_l, err, FULL_BUFFER(
                (intptr_t)header->pBuffer, header, fenceFd));
        return err;
    }

    header->nOffset = 0;
    header->nFilledLen = (nativeHandle == NULL) ? 0 : sizeof(VideoNativeMetadata);

    return emptyBuffer_l(header, flags, timestamp, (intptr_t)header->pBuffer, fenceFd);
}

void OMXNodeInstance::codecBufferFilled(omx_message &msg) {
    Mutex::Autolock autoLock(mLock);

    if (mMaxTimestampGapUs == 0LL || mRestorePtsFailed) {
        return;
    }

    OMX_U32 &flags = msg.u.extended_buffer_data.flags;
    OMX_TICKS &timestamp = msg.u.extended_buffer_data.timestamp;

    if (!(flags & OMX_BUFFERFLAG_CODECCONFIG)) {
        ssize_t index = mOriginalTimeUs.indexOfKey(timestamp);
        if (index >= 0) {
            ALOGV("OUT timestamp: %lld -> %lld",
                    static_cast<long long>(timestamp),
                    static_cast<long long>(mOriginalTimeUs[index]));
            timestamp = mOriginalTimeUs[index];
            mOriginalTimeUs.removeItemsAt(index);
        } else {
            // giving up the effort as encoder doesn't appear to preserve pts
            ALOGW("giving up limiting timestamp gap (pts = %lld)", timestamp);
            mRestorePtsFailed = true;
        }
    }
}

status_t OMXNodeInstance::getExtensionIndex(
        const char *parameterName, OMX_INDEXTYPE *index) {
    Mutex::Autolock autoLock(mLock);
    if (mHandle == NULL) {
        return DEAD_OBJECT;
    }

    OMX_ERRORTYPE err = OMX_GetExtensionIndex(
            mHandle, const_cast<char *>(parameterName), index);

    return StatusFromOMXError(err);
}

status_t OMXNodeInstance::dispatchMessage(const omx_message &msg) {
    mDispatcher->post(msg, true /*realTime*/);
    return OK;
}

status_t OMXNodeInstance::setQuirks(OMX_U32 quirks) {
    if (quirks & ~kQuirksMask) {
        return BAD_VALUE;
    }

    mQuirks = quirks;

    return OK;
}

bool OMXNodeInstance::handleMessage(omx_message &msg) {
    if (msg.type == omx_message::FILL_BUFFER_DONE) {
        OMX_BUFFERHEADERTYPE *buffer =
            findBufferHeader(msg.u.extended_buffer_data.buffer, kPortIndexOutput);
        if (buffer == NULL) {
            ALOGE("b/25884056");
            return false;
        }

        {
            Mutex::Autolock _l(mDebugLock);
            mOutputBuffersWithCodec.remove(buffer);

            CLOG_BUMPED_BUFFER(
                    FBD, WITH_STATS(FULL_BUFFER(
                            msg.u.extended_buffer_data.buffer, buffer, msg.fenceFd)));

            unbumpDebugLevel_l(kPortIndexOutput);
        }

        BufferMeta *buffer_meta =
            static_cast<BufferMeta *>(buffer->pAppPrivate);

        if (buffer->nOffset + buffer->nFilledLen < buffer->nOffset
                || buffer->nOffset + buffer->nFilledLen > buffer->nAllocLen) {
            CLOG_ERROR(onFillBufferDone, OMX_ErrorBadParameter,
                    FULL_BUFFER(NULL, buffer, msg.fenceFd));
        }
        buffer_meta->CopyFromOMX(buffer);

        // fix up the buffer info (especially timestamp) if needed
        codecBufferFilled(msg);
    } else if (msg.type == omx_message::EMPTY_BUFFER_DONE) {
        OMX_BUFFERHEADERTYPE *buffer =
            findBufferHeader(msg.u.buffer_data.buffer, kPortIndexInput);
        if (buffer == NULL) {
            return false;
        }

        {
            Mutex::Autolock _l(mDebugLock);
            mInputBuffersWithCodec.remove(buffer);

            CLOG_BUMPED_BUFFER(
                    EBD, WITH_STATS(EMPTY_BUFFER(msg.u.buffer_data.buffer, buffer, msg.fenceFd)));
        }

        const sp<IOMXBufferSource> bufferSource(getBufferSource());

        if (bufferSource != NULL) {
            // This is one of the buffers used exclusively by IOMXBufferSource.
            // Don't dispatch a message back to ACodec, since it doesn't
            // know that anyone asked to have the buffer emptied and will
            // be very confused.
            bufferSource->onInputBufferEmptied(
                    msg.u.buffer_data.buffer, OMXFenceParcelable(msg.fenceFd));
            return true;
        }
    } else if (msg.type == omx_message::EVENT &&
            msg.u.event_data.event == OMX_EventDataSpaceChanged) {
        handleDataSpaceChanged(msg);
    }

    return false;
}

bool OMXNodeInstance::handleDataSpaceChanged(omx_message &msg) {
    android_dataspace dataSpace = (android_dataspace) msg.u.event_data.data1;
    android_dataspace origDataSpace = dataSpace;

    if (!ColorUtils::convertDataSpaceToV0(dataSpace)) {
        // Do not process the data space change, don't notify client either
        return true;
    }

    android_pixel_format pixelFormat = (android_pixel_format)msg.u.event_data.data3;

    ColorAspects requestedAspects = ColorUtils::unpackToColorAspects(msg.u.event_data.data2);
    ColorAspects aspects = requestedAspects; // initially requested aspects

    // request color aspects to encode
    OMX_INDEXTYPE index;
    status_t err = getExtensionIndex(
            "OMX.google.android.index.describeColorAspects", &index);
    if (err == OK) {
        // V0 dataspace
        DescribeColorAspectsParams params;
        InitOMXParams(&params);
        params.nPortIndex = kPortIndexInput;
        params.nDataSpace = origDataSpace;
        params.nPixelFormat = pixelFormat;
        params.bDataSpaceChanged = OMX_TRUE;
        params.sAspects = requestedAspects;

        err = getConfig(index, &params, sizeof(params));
        if (err == OK) {
            aspects = params.sAspects;
            ALOGD("Codec resolved it to (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) err=%d(%s)",
                    params.sAspects.mRange, asString(params.sAspects.mRange),
                    params.sAspects.mPrimaries, asString(params.sAspects.mPrimaries),
                    params.sAspects.mMatrixCoeffs, asString(params.sAspects.mMatrixCoeffs),
                    params.sAspects.mTransfer, asString(params.sAspects.mTransfer),
                    err, asString(err));
        } else {
            params.sAspects = aspects;
            err = OK;
        }
        params.bDataSpaceChanged = OMX_FALSE;
        for (int triesLeft = 2; --triesLeft >= 0; ) {
            status_t err = setConfig(index, &params, sizeof(params));
            if (err == OK) {
                err = getConfig(index, &params, sizeof(params));
            }
            if (err != OK || !ColorUtils::checkIfAspectsChangedAndUnspecifyThem(
                    params.sAspects, aspects)) {
                // if we can't set or get color aspects, still communicate dataspace to client
                break;
            }

            ALOGW_IF(triesLeft == 0, "Codec repeatedly changed requested ColorAspects.");
        }
    }

    ALOGV("Set color aspects to (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) err=%d(%s)",
            aspects.mRange, asString(aspects.mRange),
            aspects.mPrimaries, asString(aspects.mPrimaries),
            aspects.mMatrixCoeffs, asString(aspects.mMatrixCoeffs),
            aspects.mTransfer, asString(aspects.mTransfer),
            err, asString(err));

    // signal client that the dataspace has changed; this will update the output format
    // TODO: we should tie this to an output buffer somehow, and signal the change
    // just before the output buffer is returned to the client, but there are many
    // ways this could fail (e.g. flushing), and we are not yet supporting this scenario.

    msg.u.event_data.data1 = (OMX_U32) dataSpace;
    msg.u.event_data.data2 = (OMX_U32) ColorUtils::packToU32(aspects);

    return false;
}

void OMXNodeInstance::onMessages(std::list<omx_message> &messages) {
    for (std::list<omx_message>::iterator it = messages.begin(); it != messages.end(); ) {
        if (handleMessage(*it)) {
            messages.erase(it++);
        } else {
            ++it;
        }
    }

    if (!messages.empty()) {
        mObserver->onMessages(messages);
    }
}

void OMXNodeInstance::onObserverDied() {
    ALOGE("!!! Observer died. Quickly, do something, ... anything...");

    // Try to force shutdown of the node and hope for the best.
    freeNode();
}

// OMXNodeInstance::OnEvent calls OMX::OnEvent, which then calls here.
// Don't try to acquire mLock here -- in rare circumstances this will hang.
void OMXNodeInstance::onEvent(
        OMX_EVENTTYPE event, OMX_U32 arg1, OMX_U32 arg2) {
    const char *arg1String = "??";
    const char *arg2String = "??";
    ADebug::Level level = ADebug::kDebugInternalState;

    switch (event) {
        case OMX_EventCmdComplete:
            arg1String = asString((OMX_COMMANDTYPE)arg1);
            switch (arg1) {
                case OMX_CommandStateSet:
                    arg2String = asString((OMX_STATETYPE)arg2);
                    level = ADebug::kDebugState;
                    break;
                case OMX_CommandFlush:
                case OMX_CommandPortEnable:
                {
                    // bump internal-state debug level for 2 input and output frames
                    Mutex::Autolock _l(mDebugLock);
                    bumpDebugLevel_l(2 /* numInputBuffers */, 2 /* numOutputBuffers */);
                    FALLTHROUGH_INTENDED;
                }
                default:
                    arg2String = portString(arg2);
            }
            break;
        case OMX_EventError:
            arg1String = asString((OMX_ERRORTYPE)arg1);
            level = ADebug::kDebugLifeCycle;
            break;
        case OMX_EventPortSettingsChanged:
            arg2String = asString((OMX_INDEXEXTTYPE)arg2);
            FALLTHROUGH_INTENDED;
        default:
            arg1String = portString(arg1);
    }

    CLOGI_(level, onEvent, "%s(%x), %s(%x), %s(%x)",
            asString(event), event, arg1String, arg1, arg2String, arg2);
    const sp<IOMXBufferSource> bufferSource(getBufferSource());

    if (bufferSource != NULL
            && event == OMX_EventCmdComplete
            && arg1 == OMX_CommandStateSet
            && arg2 == OMX_StateExecuting) {
        bufferSource->onOmxExecuting();
    }

    // allow configuration if we return to the loaded state
    if (event == OMX_EventCmdComplete
            && arg1 == OMX_CommandStateSet
            && arg2 == OMX_StateLoaded) {
        mSailed = false;
    }
}

// static
OMX_ERRORTYPE OMXNodeInstance::OnEvent(
        OMX_IN OMX_HANDLETYPE /* hComponent */,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData) {
    if (pAppData == NULL) {
        ALOGE("b/25884056");
        return OMX_ErrorBadParameter;
    }
    OMXNodeInstance *instance = static_cast<OMXNodeInstance *>(pAppData);
    if (instance->mDying) {
        return OMX_ErrorNone;
    }

    instance->onEvent(eEvent, nData1, nData2);

    // output rendered events are not processed as regular events until they hit the observer
    if (eEvent == OMX_EventOutputRendered) {
        if (pEventData == NULL) {
            return OMX_ErrorBadParameter;
        }

        // process data from array
        OMX_VIDEO_RENDEREVENTTYPE *renderData = (OMX_VIDEO_RENDEREVENTTYPE *)pEventData;
        for (size_t i = 0; i < nData1; ++i) {
            omx_message msg;
            msg.type = omx_message::FRAME_RENDERED;
            msg.fenceFd = -1;
            msg.u.render_data.timestamp = renderData[i].nMediaTimeUs;
            msg.u.render_data.nanoTime = renderData[i].nSystemTimeNs;
            bool realTime = msg.u.render_data.timestamp == INT64_MAX;
            instance->mDispatcher->post(msg, realTime);
        }
        return OMX_ErrorNone;
    }

    omx_message msg;
    msg.type = omx_message::EVENT;
    msg.fenceFd = -1;
    msg.u.event_data.event = eEvent;
    msg.u.event_data.data1 = nData1;
    msg.u.event_data.data2 = nData2;

    instance->mDispatcher->post(msg, true /* realTime */);

    return OMX_ErrorNone;
}

// static
OMX_ERRORTYPE OMXNodeInstance::OnEmptyBufferDone(
        OMX_IN OMX_HANDLETYPE /* hComponent */,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer) {
    if (pAppData == NULL) {
        ALOGE("b/25884056");
        return OMX_ErrorBadParameter;
    }
    OMXNodeInstance *instance = static_cast<OMXNodeInstance *>(pAppData);
    if (instance->mDying) {
        return OMX_ErrorNone;
    }
    int fenceFd = instance->retrieveFenceFromMeta_l(pBuffer, kPortIndexOutput);

    omx_message msg;
    msg.type = omx_message::EMPTY_BUFFER_DONE;
    msg.fenceFd = fenceFd;
    msg.u.buffer_data.buffer = instance->findBufferID(pBuffer);
    instance->mDispatcher->post(msg);

    return OMX_ErrorNone;
}

// static
OMX_ERRORTYPE OMXNodeInstance::OnFillBufferDone(
        OMX_IN OMX_HANDLETYPE /* hComponent */,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer) {
    if (pAppData == NULL) {
        ALOGE("b/25884056");
        return OMX_ErrorBadParameter;
    }
    OMXNodeInstance *instance = static_cast<OMXNodeInstance *>(pAppData);
    if (instance->mDying) {
        return OMX_ErrorNone;
    }
    int fenceFd = instance->retrieveFenceFromMeta_l(pBuffer, kPortIndexOutput);

    omx_message msg;
    msg.type = omx_message::FILL_BUFFER_DONE;
    msg.fenceFd = fenceFd;
    msg.u.extended_buffer_data.buffer = instance->findBufferID(pBuffer);
    msg.u.extended_buffer_data.range_offset = pBuffer->nOffset;
    msg.u.extended_buffer_data.range_length = pBuffer->nFilledLen;
    msg.u.extended_buffer_data.flags = pBuffer->nFlags;
    msg.u.extended_buffer_data.timestamp = pBuffer->nTimeStamp;
    instance->mDispatcher->post(msg);

    return OMX_ErrorNone;
}

void OMXNodeInstance::addActiveBuffer(OMX_U32 portIndex, IOMX::buffer_id id) {
    ActiveBuffer active;
    active.mPortIndex = portIndex;
    active.mID = id;
    mActiveBuffers.push(active);

    if (portIndex < NELEM(mNumPortBuffers)) {
        ++mNumPortBuffers[portIndex];
    }
}

void OMXNodeInstance::removeActiveBuffer(
        OMX_U32 portIndex, IOMX::buffer_id id) {
    for (size_t i = 0; i < mActiveBuffers.size(); ++i) {
        if (mActiveBuffers[i].mPortIndex == portIndex
                && mActiveBuffers[i].mID == id) {
            mActiveBuffers.removeItemsAt(i);

            if (portIndex < NELEM(mNumPortBuffers)) {
                --mNumPortBuffers[portIndex];
            }
            return;
        }
    }

     CLOGW("Attempt to remove an active buffer [%#x] we know nothing about...", id);
}

void OMXNodeInstance::freeActiveBuffers() {
    // Make sure to count down here, as freeBuffer will in turn remove
    // the active buffer from the vector...
    for (size_t i = mActiveBuffers.size(); i > 0;) {
        i--;
        freeBuffer(mActiveBuffers[i].mPortIndex, mActiveBuffers[i].mID);
    }
}

IOMX::buffer_id OMXNodeInstance::makeBufferID(OMX_BUFFERHEADERTYPE *bufferHeader) {
    if (bufferHeader == NULL) {
        return 0;
    }
    Mutex::Autolock autoLock(mBufferIDLock);
    IOMX::buffer_id buffer;
    do { // handle the very unlikely case of ID overflow
        if (++mBufferIDCount == 0) {
            ++mBufferIDCount;
        }
        buffer = (IOMX::buffer_id)mBufferIDCount;
    } while (mBufferIDToBufferHeader.indexOfKey(buffer) >= 0);
    mBufferIDToBufferHeader.add(buffer, bufferHeader);
    mBufferHeaderToBufferID.add(bufferHeader, buffer);
    return buffer;
}

OMX_BUFFERHEADERTYPE *OMXNodeInstance::findBufferHeader(
        IOMX::buffer_id buffer, OMX_U32 portIndex) {
    if (buffer == 0) {
        return NULL;
    }
    Mutex::Autolock autoLock(mBufferIDLock);
    ssize_t index = mBufferIDToBufferHeader.indexOfKey(buffer);
    if (index < 0) {
        CLOGW("findBufferHeader: buffer %u not found", buffer);
        return NULL;
    }
    OMX_BUFFERHEADERTYPE *header = mBufferIDToBufferHeader.valueAt(index);
    BufferMeta *buffer_meta =
        static_cast<BufferMeta *>(header->pAppPrivate);
    if (buffer_meta->getPortIndex() != portIndex) {
        CLOGW("findBufferHeader: buffer %u found but with incorrect port index.", buffer);
        android_errorWriteLog(0x534e4554, "28816827");
        return NULL;
    }
    return header;
}

IOMX::buffer_id OMXNodeInstance::findBufferID(OMX_BUFFERHEADERTYPE *bufferHeader) {
    if (bufferHeader == NULL) {
        return 0;
    }
    Mutex::Autolock autoLock(mBufferIDLock);
    ssize_t index = mBufferHeaderToBufferID.indexOfKey(bufferHeader);
    if (index < 0) {
        CLOGW("findBufferID: bufferHeader %p not found", bufferHeader);
        return 0;
    }
    return mBufferHeaderToBufferID.valueAt(index);
}

void OMXNodeInstance::invalidateBufferID(IOMX::buffer_id buffer) {
    if (buffer == 0) {
        return;
    }
    Mutex::Autolock autoLock(mBufferIDLock);
    ssize_t index = mBufferIDToBufferHeader.indexOfKey(buffer);
    if (index < 0) {
        CLOGW("invalidateBufferID: buffer %u not found", buffer);
        return;
    }
    mBufferHeaderToBufferID.removeItem(mBufferIDToBufferHeader.valueAt(index));
    mBufferIDToBufferHeader.removeItemsAt(index);
}

}  // namespace android
