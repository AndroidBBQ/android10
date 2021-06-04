/*
 * Copyright (C) 2010 The Android Open Source Project
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
#define LOG_TAG "ACodec"

#ifdef __LP64__
#define OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
#endif

#include <inttypes.h>
#include <utils/Trace.h>

#include <gui/Surface.h>

#include <media/stagefright/ACodec.h>

#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>

#include <media/stagefright/BufferProducerWrapper.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/PersistentSurface.h>
#include <media/stagefright/SurfaceUtils.h>
#include <media/hardware/HardwareAPI.h>
#include <media/MediaBufferHolder.h>
#include <media/OMXBuffer.h>
#include <media/omx/1.0/WOmxNode.h>

#include <hidlmemory/mapping.h>

#include <media/openmax/OMX_AudioExt.h>
#include <media/openmax/OMX_VideoExt.h>
#include <media/openmax/OMX_Component.h>
#include <media/openmax/OMX_IndexExt.h>
#include <media/openmax/OMX_AsString.h>

#include "include/ACodecBufferChannel.h"
#include "include/DataConverter.h"
#include "include/SecureBuffer.h"
#include "include/SharedMemoryBuffer.h"
#include <media/stagefright/omx/OMXUtils.h>

namespace android {

using binder::Status;

enum {
    kMaxIndicesToCheck = 32, // used when enumerating supported formats and profiles
};

// OMX errors are directly mapped into status_t range if
// there is no corresponding MediaError status code.
// Use the statusFromOMXError(int32_t omxError) function.
//
// Currently this is a direct map.
// See frameworks/native/include/media/openmax/OMX_Core.h
//
// Vendor OMX errors     from 0x90000000 - 0x9000FFFF
// Extension OMX errors  from 0x8F000000 - 0x90000000
// Standard OMX errors   from 0x80001000 - 0x80001024 (0x80001024 current)
//

// returns true if err is a recognized OMX error code.
// as OMX error is OMX_S32, this is an int32_t type
static inline bool isOMXError(int32_t err) {
    return (ERROR_CODEC_MIN <= err && err <= ERROR_CODEC_MAX);
}

// converts an OMX error to a status_t
static inline status_t statusFromOMXError(int32_t omxError) {
    switch (omxError) {
    case OMX_ErrorInvalidComponentName:
    case OMX_ErrorComponentNotFound:
        return NAME_NOT_FOUND; // can trigger illegal argument error for provided names.
    default:
        return isOMXError(omxError) ? omxError : 0; // no translation required
    }
}

static inline status_t statusFromBinderStatus(const Status &status) {
    if (status.isOk()) {
        return OK;
    }
    status_t err;
    if ((err = status.serviceSpecificErrorCode()) != OK) {
        return err;
    }
    if ((err = status.transactionError()) != OK) {
        return err;
    }
    // Other exception
    return UNKNOWN_ERROR;
}

// checks and converts status_t to a non-side-effect status_t
static inline status_t makeNoSideEffectStatus(status_t err) {
    switch (err) {
    // the following errors have side effects and may come
    // from other code modules. Remap for safety reasons.
    case INVALID_OPERATION:
    case DEAD_OBJECT:
        return UNKNOWN_ERROR;
    default:
        return err;
    }
}

static OMX_VIDEO_CONTROLRATETYPE getVideoBitrateMode(const sp<AMessage> &msg) {
    int32_t tmp;
    if (msg->findInt32("bitrate-mode", &tmp)) {
        // explicitly translate from MediaCodecInfo.EncoderCapabilities.
        // BITRATE_MODE_* into OMX bitrate mode.
        switch (tmp) {
            //BITRATE_MODE_CQ
            case 0: return OMX_Video_ControlRateConstantQuality;
            //BITRATE_MODE_VBR
            case 1: return OMX_Video_ControlRateVariable;
            //BITRATE_MODE_CBR
            case 2: return OMX_Video_ControlRateConstant;
            default: break;
        }
    }
    return OMX_Video_ControlRateVariable;
}

static bool findVideoBitrateControlInfo(const sp<AMessage> &msg,
        OMX_VIDEO_CONTROLRATETYPE *mode, int32_t *bitrate, int32_t *quality) {
    *mode = getVideoBitrateMode(msg);
    bool isCQ = (*mode == OMX_Video_ControlRateConstantQuality);
    return (!isCQ && msg->findInt32("bitrate", bitrate))
         || (isCQ && msg->findInt32("quality", quality));
}

struct MessageList : public RefBase {
    MessageList() {
    }
    virtual ~MessageList() {
    }
    std::list<sp<AMessage> > &getList() { return mList; }
private:
    std::list<sp<AMessage> > mList;

    DISALLOW_EVIL_CONSTRUCTORS(MessageList);
};

static sp<DataConverter> getCopyConverter() {
    static pthread_once_t once = PTHREAD_ONCE_INIT; // const-inited
    static sp<DataConverter> sCopyConverter;        // zero-inited
    pthread_once(&once, [](){ sCopyConverter = new DataConverter(); });
    return sCopyConverter;
}

struct CodecObserver : public BnOMXObserver {
    explicit CodecObserver(const sp<AMessage> &msg) : mNotify(msg) {}

    // from IOMXObserver
    virtual void onMessages(const std::list<omx_message> &messages) {
        if (messages.empty()) {
            return;
        }

        sp<AMessage> notify = mNotify->dup();
        sp<MessageList> msgList = new MessageList();
        for (std::list<omx_message>::const_iterator it = messages.cbegin();
              it != messages.cend(); ++it) {
            const omx_message &omx_msg = *it;

            sp<AMessage> msg = new AMessage;
            msg->setInt32("type", omx_msg.type);
            switch (omx_msg.type) {
                case omx_message::EVENT:
                {
                    msg->setInt32("event", omx_msg.u.event_data.event);
                    msg->setInt32("data1", omx_msg.u.event_data.data1);
                    msg->setInt32("data2", omx_msg.u.event_data.data2);
                    break;
                }

                case omx_message::EMPTY_BUFFER_DONE:
                {
                    msg->setInt32("buffer", omx_msg.u.buffer_data.buffer);
                    msg->setInt32("fence_fd", omx_msg.fenceFd);
                    break;
                }

                case omx_message::FILL_BUFFER_DONE:
                {
                    msg->setInt32(
                            "buffer", omx_msg.u.extended_buffer_data.buffer);
                    msg->setInt32(
                            "range_offset",
                            omx_msg.u.extended_buffer_data.range_offset);
                    msg->setInt32(
                            "range_length",
                            omx_msg.u.extended_buffer_data.range_length);
                    msg->setInt32(
                            "flags",
                            omx_msg.u.extended_buffer_data.flags);
                    msg->setInt64(
                            "timestamp",
                            omx_msg.u.extended_buffer_data.timestamp);
                    msg->setInt32(
                            "fence_fd", omx_msg.fenceFd);
                    break;
                }

                case omx_message::FRAME_RENDERED:
                {
                    msg->setInt64(
                            "media_time_us", omx_msg.u.render_data.timestamp);
                    msg->setInt64(
                            "system_nano", omx_msg.u.render_data.nanoTime);
                    break;
                }

                default:
                    ALOGE("Unrecognized message type: %d", omx_msg.type);
                    break;
            }
            msgList->getList().push_back(msg);
        }
        notify->setObject("messages", msgList);
        notify->post();
    }

protected:
    virtual ~CodecObserver() {}

private:
    const sp<AMessage> mNotify;

    DISALLOW_EVIL_CONSTRUCTORS(CodecObserver);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::BaseState : public AState {
    explicit BaseState(ACodec *codec, const sp<AState> &parentState = NULL);

protected:
    enum PortMode {
        KEEP_BUFFERS,
        RESUBMIT_BUFFERS,
        FREE_BUFFERS,
    };

    ACodec *mCodec;

    virtual PortMode getPortMode(OMX_U32 portIndex);

    virtual void stateExited();
    virtual bool onMessageReceived(const sp<AMessage> &msg);

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);

    virtual void onOutputBufferDrained(const sp<AMessage> &msg);
    virtual void onInputBufferFilled(const sp<AMessage> &msg);

    void postFillThisBuffer(BufferInfo *info);

private:
    // Handles an OMX message. Returns true iff message was handled.
    bool onOMXMessage(const sp<AMessage> &msg);

    // Handles a list of messages. Returns true iff messages were handled.
    bool onOMXMessageList(const sp<AMessage> &msg);

    // returns true iff this message is for this component and the component is alive
    bool checkOMXMessage(const sp<AMessage> &msg);

    bool onOMXEmptyBufferDone(IOMX::buffer_id bufferID, int fenceFd);

    bool onOMXFillBufferDone(
            IOMX::buffer_id bufferID,
            size_t rangeOffset, size_t rangeLength,
            OMX_U32 flags,
            int64_t timeUs,
            int fenceFd);

    virtual bool onOMXFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano);

    void getMoreInputDataIfPossible();

    DISALLOW_EVIL_CONSTRUCTORS(BaseState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::DeathNotifier :
        public IBinder::DeathRecipient,
        public ::android::hardware::hidl_death_recipient {
    explicit DeathNotifier(const sp<AMessage> &notify)
        : mNotify(notify) {
    }

    virtual void binderDied(const wp<IBinder> &) {
        mNotify->post();
    }

    virtual void serviceDied(
            uint64_t /* cookie */,
            const wp<::android::hidl::base::V1_0::IBase>& /* who */) {
        mNotify->post();
    }

protected:
    virtual ~DeathNotifier() {}

private:
    sp<AMessage> mNotify;

    DISALLOW_EVIL_CONSTRUCTORS(DeathNotifier);
};

struct ACodec::UninitializedState : public ACodec::BaseState {
    explicit UninitializedState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

private:
    void onSetup(const sp<AMessage> &msg);
    bool onAllocateComponent(const sp<AMessage> &msg);

    sp<DeathNotifier> mDeathNotifier;

    DISALLOW_EVIL_CONSTRUCTORS(UninitializedState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::LoadedState : public ACodec::BaseState {
    explicit LoadedState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

private:
    friend struct ACodec::UninitializedState;

    bool onConfigureComponent(const sp<AMessage> &msg);
    void onCreateInputSurface(const sp<AMessage> &msg);
    void onSetInputSurface(const sp<AMessage> &msg);
    void onStart();
    void onShutdown(bool keepComponentAllocated);

    status_t setupInputSurface();

    DISALLOW_EVIL_CONSTRUCTORS(LoadedState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::LoadedToIdleState : public ACodec::BaseState {
    explicit LoadedToIdleState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);
    virtual void stateEntered();

private:
    status_t allocateBuffers();

    DISALLOW_EVIL_CONSTRUCTORS(LoadedToIdleState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::IdleToExecutingState : public ACodec::BaseState {
    explicit IdleToExecutingState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);
    virtual void stateEntered();

private:
    DISALLOW_EVIL_CONSTRUCTORS(IdleToExecutingState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::ExecutingState : public ACodec::BaseState {
    explicit ExecutingState(ACodec *codec);

    void submitRegularOutputBuffers();
    void submitOutputMetaBuffers();
    void submitOutputBuffers();

    // Submit output buffers to the decoder, submit input buffers to client
    // to fill with data.
    void resume();

    // Returns true iff input and output buffers are in play.
    bool active() const { return mActive; }

protected:
    virtual PortMode getPortMode(OMX_U32 portIndex);
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);
    virtual bool onOMXFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano);

private:
    bool mActive;

    DISALLOW_EVIL_CONSTRUCTORS(ExecutingState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::OutputPortSettingsChangedState : public ACodec::BaseState {
    explicit OutputPortSettingsChangedState(ACodec *codec);

protected:
    virtual PortMode getPortMode(OMX_U32 portIndex);
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);
    virtual bool onOMXFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano);

private:
    DISALLOW_EVIL_CONSTRUCTORS(OutputPortSettingsChangedState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::ExecutingToIdleState : public ACodec::BaseState {
    explicit ExecutingToIdleState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);

    virtual void onOutputBufferDrained(const sp<AMessage> &msg);
    virtual void onInputBufferFilled(const sp<AMessage> &msg);

private:
    void changeStateIfWeOwnAllBuffers();

    bool mComponentNowIdle;

    DISALLOW_EVIL_CONSTRUCTORS(ExecutingToIdleState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::IdleToLoadedState : public ACodec::BaseState {
    explicit IdleToLoadedState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);

private:
    DISALLOW_EVIL_CONSTRUCTORS(IdleToLoadedState);
};

////////////////////////////////////////////////////////////////////////////////

struct ACodec::FlushingState : public ACodec::BaseState {
    explicit FlushingState(ACodec *codec);

protected:
    virtual bool onMessageReceived(const sp<AMessage> &msg);
    virtual void stateEntered();

    virtual bool onOMXEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2);

    virtual void onOutputBufferDrained(const sp<AMessage> &msg);
    virtual void onInputBufferFilled(const sp<AMessage> &msg);

private:
    bool mFlushComplete[2];

    void changeStateIfWeOwnAllBuffers();

    DISALLOW_EVIL_CONSTRUCTORS(FlushingState);
};

////////////////////////////////////////////////////////////////////////////////

void ACodec::BufferInfo::setWriteFence(int fenceFd, const char *dbg) {
    if (mFenceFd >= 0) {
        ALOGW("OVERWRITE OF %s fence %d by write fence %d in %s",
                mIsReadFence ? "read" : "write", mFenceFd, fenceFd, dbg);
    }
    mFenceFd = fenceFd;
    mIsReadFence = false;
}

void ACodec::BufferInfo::setReadFence(int fenceFd, const char *dbg) {
    if (mFenceFd >= 0) {
        ALOGW("OVERWRITE OF %s fence %d by read fence %d in %s",
                mIsReadFence ? "read" : "write", mFenceFd, fenceFd, dbg);
    }
    mFenceFd = fenceFd;
    mIsReadFence = true;
}

void ACodec::BufferInfo::checkWriteFence(const char *dbg) {
    if (mFenceFd >= 0 && mIsReadFence) {
        ALOGD("REUSING read fence %d as write fence in %s", mFenceFd, dbg);
    }
}

void ACodec::BufferInfo::checkReadFence(const char *dbg) {
    if (mFenceFd >= 0 && !mIsReadFence) {
        ALOGD("REUSING write fence %d as read fence in %s", mFenceFd, dbg);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::ACodec()
    : mSampleRate(0),
      mNodeGeneration(0),
      mUsingNativeWindow(false),
      mNativeWindowUsageBits(0),
      mLastNativeWindowDataSpace(HAL_DATASPACE_UNKNOWN),
      mIsVideo(false),
      mIsImage(false),
      mIsEncoder(false),
      mFatalError(false),
      mShutdownInProgress(false),
      mExplicitShutdown(false),
      mIsLegacyVP9Decoder(false),
      mEncoderDelay(0),
      mEncoderPadding(0),
      mRotationDegrees(0),
      mChannelMaskPresent(false),
      mChannelMask(0),
      mDequeueCounter(0),
      mMetadataBuffersToSubmit(0),
      mNumUndequeuedBuffers(0),
      mRepeatFrameDelayUs(-1LL),
      mMaxPtsGapUs(0LL),
      mMaxFps(-1),
      mFps(-1.0),
      mCaptureFps(-1.0),
      mCreateInputBuffersSuspended(false),
      mTunneled(false),
      mDescribeColorAspectsIndex((OMX_INDEXTYPE)0),
      mDescribeHDRStaticInfoIndex((OMX_INDEXTYPE)0),
      mDescribeHDR10PlusInfoIndex((OMX_INDEXTYPE)0),
      mStateGeneration(0),
      mVendorExtensionsStatus(kExtensionsUnchecked) {
    memset(&mLastHDRStaticInfo, 0, sizeof(mLastHDRStaticInfo));

    mUninitializedState = new UninitializedState(this);
    mLoadedState = new LoadedState(this);
    mLoadedToIdleState = new LoadedToIdleState(this);
    mIdleToExecutingState = new IdleToExecutingState(this);
    mExecutingState = new ExecutingState(this);

    mOutputPortSettingsChangedState =
        new OutputPortSettingsChangedState(this);

    mExecutingToIdleState = new ExecutingToIdleState(this);
    mIdleToLoadedState = new IdleToLoadedState(this);
    mFlushingState = new FlushingState(this);

    mPortEOS[kPortIndexInput] = mPortEOS[kPortIndexOutput] = false;
    mInputEOSResult = OK;

    mPortMode[kPortIndexInput] = IOMX::kPortModePresetByteBuffer;
    mPortMode[kPortIndexOutput] = IOMX::kPortModePresetByteBuffer;

    memset(&mLastNativeWindowCrop, 0, sizeof(mLastNativeWindowCrop));

    changeState(mUninitializedState);
}

ACodec::~ACodec() {
}

void ACodec::initiateSetup(const sp<AMessage> &msg) {
    msg->setWhat(kWhatSetup);
    msg->setTarget(this);
    msg->post();
}

std::shared_ptr<BufferChannelBase> ACodec::getBufferChannel() {
    if (!mBufferChannel) {
        mBufferChannel = std::make_shared<ACodecBufferChannel>(
                new AMessage(kWhatInputBufferFilled, this),
                new AMessage(kWhatOutputBufferDrained, this));
    }
    return mBufferChannel;
}

void ACodec::signalSetParameters(const sp<AMessage> &params) {
    sp<AMessage> msg = new AMessage(kWhatSetParameters, this);
    msg->setMessage("params", params);
    msg->post();
}

void ACodec::initiateAllocateComponent(const sp<AMessage> &msg) {
    msg->setWhat(kWhatAllocateComponent);
    msg->setTarget(this);
    msg->post();
}

void ACodec::initiateConfigureComponent(const sp<AMessage> &msg) {
    msg->setWhat(kWhatConfigureComponent);
    msg->setTarget(this);
    msg->post();
}

status_t ACodec::setSurface(const sp<Surface> &surface) {
    sp<AMessage> msg = new AMessage(kWhatSetSurface, this);
    msg->setObject("surface", surface);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if (err == OK) {
        (void)response->findInt32("err", &err);
    }
    return err;
}

void ACodec::initiateCreateInputSurface() {
    (new AMessage(kWhatCreateInputSurface, this))->post();
}

void ACodec::initiateSetInputSurface(
        const sp<PersistentSurface> &surface) {
    sp<AMessage> msg = new AMessage(kWhatSetInputSurface, this);
    msg->setObject("input-surface", surface);
    msg->post();
}

void ACodec::signalEndOfInputStream() {
    (new AMessage(kWhatSignalEndOfInputStream, this))->post();
}

void ACodec::initiateStart() {
    (new AMessage(kWhatStart, this))->post();
}

void ACodec::signalFlush() {
    ALOGV("[%s] signalFlush", mComponentName.c_str());
    (new AMessage(kWhatFlush, this))->post();
}

void ACodec::signalResume() {
    (new AMessage(kWhatResume, this))->post();
}

void ACodec::initiateShutdown(bool keepComponentAllocated) {
    sp<AMessage> msg = new AMessage(kWhatShutdown, this);
    msg->setInt32("keepComponentAllocated", keepComponentAllocated);
    msg->post();
    if (!keepComponentAllocated) {
        // ensure shutdown completes in 3 seconds
        (new AMessage(kWhatReleaseCodecInstance, this))->post(3000000);
    }
}

void ACodec::signalRequestIDRFrame() {
    (new AMessage(kWhatRequestIDRFrame, this))->post();
}

// *** NOTE: THE FOLLOWING WORKAROUND WILL BE REMOVED ***
// Some codecs may return input buffers before having them processed.
// This causes a halt if we already signaled an EOS on the input
// port.  For now keep submitting an output buffer if there was an
// EOS on the input port, but not yet on the output port.
void ACodec::signalSubmitOutputMetadataBufferIfEOS_workaround() {
    if (mPortEOS[kPortIndexInput] && !mPortEOS[kPortIndexOutput] &&
            mMetadataBuffersToSubmit > 0) {
        (new AMessage(kWhatSubmitOutputMetadataBufferIfEOS, this))->post();
    }
}

status_t ACodec::handleSetSurface(const sp<Surface> &surface) {
    // allow keeping unset surface
    if (surface == NULL) {
        if (mNativeWindow != NULL) {
            ALOGW("cannot unset a surface");
            return INVALID_OPERATION;
        }
        return OK;
    }

    // cannot switch from bytebuffers to surface
    if (mNativeWindow == NULL) {
        ALOGW("component was not configured with a surface");
        return INVALID_OPERATION;
    }

    ANativeWindow *nativeWindow = surface.get();
    // if we have not yet started the codec, we can simply set the native window
    if (mBuffers[kPortIndexInput].size() == 0) {
        mNativeWindow = surface;
        return OK;
    }

    // we do not support changing a tunneled surface after start
    if (mTunneled) {
        ALOGW("cannot change tunneled surface");
        return INVALID_OPERATION;
    }

    int usageBits = 0;
    // no need to reconnect as we will not dequeue all buffers
    status_t err = setupNativeWindowSizeFormatAndUsage(
            nativeWindow, &usageBits, !storingMetadataInDecodedBuffers());
    if (err != OK) {
        return err;
    }

    int ignoredFlags = kVideoGrallocUsage;
    // New output surface is not allowed to add new usage flag except ignored ones.
    if ((usageBits & ~(mNativeWindowUsageBits | ignoredFlags)) != 0) {
        ALOGW("cannot change usage from %#x to %#x", mNativeWindowUsageBits, usageBits);
        return BAD_VALUE;
    }

    // get min undequeued count. We cannot switch to a surface that has a higher
    // undequeued count than we allocated.
    int minUndequeuedBuffers = 0;
    err = nativeWindow->query(
            nativeWindow, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
            &minUndequeuedBuffers);
    if (err != 0) {
        ALOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query failed: %s (%d)",
                strerror(-err), -err);
        return err;
    }
    if (minUndequeuedBuffers > (int)mNumUndequeuedBuffers) {
        ALOGE("new surface holds onto more buffers (%d) than planned for (%zu)",
                minUndequeuedBuffers, mNumUndequeuedBuffers);
        return BAD_VALUE;
    }

    // we cannot change the number of output buffers while OMX is running
    // set up surface to the same count
    Vector<BufferInfo> &buffers = mBuffers[kPortIndexOutput];
    ALOGV("setting up surface for %zu buffers", buffers.size());

    err = native_window_set_buffer_count(nativeWindow, buffers.size());
    if (err != 0) {
        ALOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err),
                -err);
        return err;
    }

    // need to enable allocation when attaching
    surface->getIGraphicBufferProducer()->allowAllocation(true);

    // dequeueBuffer cannot time out
    surface->setDequeueTimeout(-1);

    // for meta data mode, we move dequeud buffers to the new surface.
    // for non-meta mode, we must move all registered buffers
    for (size_t i = 0; i < buffers.size(); ++i) {
        const BufferInfo &info = buffers[i];
        // skip undequeued buffers for meta data mode
        if (storingMetadataInDecodedBuffers()
                && info.mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
            ALOGV("skipping buffer");
            continue;
        }
        ALOGV("attaching buffer %p", info.mGraphicBuffer->getNativeBuffer());

        err = surface->attachBuffer(info.mGraphicBuffer->getNativeBuffer());
        if (err != OK) {
            ALOGE("failed to attach buffer %p to the new surface: %s (%d)",
                    info.mGraphicBuffer->getNativeBuffer(),
                    strerror(-err), -err);
            return err;
        }
    }

    // cancel undequeued buffers to new surface
    if (!storingMetadataInDecodedBuffers()) {
        for (size_t i = 0; i < buffers.size(); ++i) {
            BufferInfo &info = buffers.editItemAt(i);
            if (info.mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                ALOGV("canceling buffer %p", info.mGraphicBuffer->getNativeBuffer());
                err = nativeWindow->cancelBuffer(
                        nativeWindow, info.mGraphicBuffer->getNativeBuffer(), info.mFenceFd);
                info.mFenceFd = -1;
                if (err != OK) {
                    ALOGE("failed to cancel buffer %p to the new surface: %s (%d)",
                            info.mGraphicBuffer->getNativeBuffer(),
                            strerror(-err), -err);
                    return err;
                }
            }
        }
        // disallow further allocation
        (void)surface->getIGraphicBufferProducer()->allowAllocation(false);
    }

    // push blank buffers to previous window if requested
    if (mFlags & kFlagPushBlankBuffersToNativeWindowOnShutdown) {
        pushBlankBuffersToNativeWindow(mNativeWindow.get());
    }

    mNativeWindow = nativeWindow;
    mNativeWindowUsageBits = usageBits;
    return OK;
}

status_t ACodec::setPortMode(int32_t portIndex, IOMX::PortMode mode) {
    status_t err = mOMXNode->setPortMode(portIndex, mode);
    if (err != OK) {
        ALOGE("[%s] setPortMode on %s to %s failed w/ err %d",
                mComponentName.c_str(),
                portIndex == kPortIndexInput ? "input" : "output",
                asString(mode),
                err);
        return err;
    }

    mPortMode[portIndex] = mode;
    return OK;
}

status_t ACodec::allocateBuffersOnPort(OMX_U32 portIndex) {
    CHECK(portIndex == kPortIndexInput || portIndex == kPortIndexOutput);

    CHECK(mAllocator[portIndex] == NULL);
    CHECK(mBuffers[portIndex].isEmpty());

    status_t err;
    if (mNativeWindow != NULL && portIndex == kPortIndexOutput) {
        if (storingMetadataInDecodedBuffers()) {
            err = allocateOutputMetadataBuffers();
        } else {
            err = allocateOutputBuffersFromNativeWindow();
        }
    } else {
        OMX_PARAM_PORTDEFINITIONTYPE def;
        InitOMXParams(&def);
        def.nPortIndex = portIndex;

        err = mOMXNode->getParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));

        if (err == OK) {
            const IOMX::PortMode &mode = mPortMode[portIndex];
            size_t bufSize = def.nBufferSize;
            // Always allocate VideoNativeMetadata if using ANWBuffer.
            // OMX might use gralloc source internally, but we don't share
            // metadata buffer with OMX, OMX has its own headers.
            if (mode == IOMX::kPortModeDynamicANWBuffer) {
                bufSize = sizeof(VideoNativeMetadata);
            } else if (mode == IOMX::kPortModeDynamicNativeHandle) {
                bufSize = sizeof(VideoNativeHandleMetadata);
            }

            size_t conversionBufferSize = 0;

            sp<DataConverter> converter = mConverter[portIndex];
            if (converter != NULL) {
                // here we assume sane conversions of max 4:1, so result fits in int32
                if (portIndex == kPortIndexInput) {
                    conversionBufferSize = converter->sourceSize(bufSize);
                } else {
                    conversionBufferSize = converter->targetSize(bufSize);
                }
            }

            size_t alignment = 32; // This is the value currently returned by
                                   // MemoryDealer::getAllocationAlignment().
                                   // TODO: Fix this when Treble has
                                   // MemoryHeap/MemoryDealer.

            ALOGV("[%s] Allocating %u buffers of size %zu (from %u using %s) on %s port",
                    mComponentName.c_str(),
                    def.nBufferCountActual, bufSize, def.nBufferSize, asString(mode),
                    portIndex == kPortIndexInput ? "input" : "output");

            // verify buffer sizes to avoid overflow in align()
            if (bufSize == 0 || max(bufSize, conversionBufferSize) > kMaxCodecBufferSize) {
                ALOGE("b/22885421");
                return NO_MEMORY;
            }

            // don't modify bufSize as OMX may not expect it to increase after negotiation
            size_t alignedSize = align(bufSize, alignment);
            size_t alignedConvSize = align(conversionBufferSize, alignment);
            if (def.nBufferCountActual > SIZE_MAX / (alignedSize + alignedConvSize)) {
                ALOGE("b/22885421");
                return NO_MEMORY;
            }

            if (mode != IOMX::kPortModePresetSecureBuffer) {
                mAllocator[portIndex] = TAllocator::getService("ashmem");
                if (mAllocator[portIndex] == nullptr) {
                    ALOGE("hidl allocator on port %d is null",
                            (int)portIndex);
                    return NO_MEMORY;
                }
                // TODO: When Treble has MemoryHeap/MemoryDealer, we should
                // specify the heap size to be
                // def.nBufferCountActual * (alignedSize + alignedConvSize).
            }

            const sp<AMessage> &format =
                    portIndex == kPortIndexInput ? mInputFormat : mOutputFormat;
            for (OMX_U32 i = 0; i < def.nBufferCountActual && err == OK; ++i) {
                hidl_memory hidlMemToken;
                sp<TMemory> hidlMem;
                sp<IMemory> mem;

                BufferInfo info;
                info.mStatus = BufferInfo::OWNED_BY_US;
                info.mFenceFd = -1;
                info.mRenderInfo = NULL;
                info.mGraphicBuffer = NULL;
                info.mNewGraphicBuffer = false;

                if (mode == IOMX::kPortModePresetSecureBuffer) {
                    void *ptr = NULL;
                    sp<NativeHandle> native_handle;
                    err = mOMXNode->allocateSecureBuffer(
                            portIndex, bufSize, &info.mBufferID,
                            &ptr, &native_handle);

                    info.mData = (native_handle == NULL)
                            ? new SecureBuffer(format, ptr, bufSize)
                            : new SecureBuffer(format, native_handle, bufSize);
                    info.mCodecData = info.mData;
                } else {
                    bool success;
                    auto transStatus = mAllocator[portIndex]->allocate(
                            bufSize,
                            [&success, &hidlMemToken](
                                    bool s,
                                    hidl_memory const& m) {
                                success = s;
                                hidlMemToken = m;
                            });

                    if (!transStatus.isOk()) {
                        ALOGE("hidl's AshmemAllocator failed at the "
                                "transport: %s",
                                transStatus.description().c_str());
                        return NO_MEMORY;
                    }
                    if (!success) {
                        return NO_MEMORY;
                    }
                    hidlMem = mapMemory(hidlMemToken);
                    if (hidlMem == nullptr) {
                        return NO_MEMORY;
                    }
                    err = mOMXNode->useBuffer(
                            portIndex, hidlMemToken, &info.mBufferID);

                    if (mode == IOMX::kPortModeDynamicANWBuffer) {
                        VideoNativeMetadata* metaData = (VideoNativeMetadata*)(
                                (void*)hidlMem->getPointer());
                        metaData->nFenceFd = -1;
                    }

                    info.mCodecData = new SharedMemoryBuffer(
                            format, hidlMem);
                    info.mCodecRef = hidlMem;

                    // if we require conversion, allocate conversion buffer for client use;
                    // otherwise, reuse codec buffer
                    if (mConverter[portIndex] != NULL) {
                        CHECK_GT(conversionBufferSize, (size_t)0);
                        bool success;
                        mAllocator[portIndex]->allocate(
                                conversionBufferSize,
                                [&success, &hidlMemToken](
                                        bool s,
                                        hidl_memory const& m) {
                                    success = s;
                                    hidlMemToken = m;
                                });
                        if (!success) {
                            return NO_MEMORY;
                        }
                        hidlMem = mapMemory(hidlMemToken);
                        if (hidlMem == nullptr) {
                            return NO_MEMORY;
                        }
                        info.mData = new SharedMemoryBuffer(format, hidlMem);
                        info.mMemRef = hidlMem;
                    } else {
                        info.mData = info.mCodecData;
                        info.mMemRef = info.mCodecRef;
                    }
                }

                mBuffers[portIndex].push(info);
            }
        }
    }

    if (err != OK) {
        return err;
    }

    std::vector<ACodecBufferChannel::BufferAndId> array(mBuffers[portIndex].size());
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        array[i] = {mBuffers[portIndex][i].mData, mBuffers[portIndex][i].mBufferID};
    }
    if (portIndex == kPortIndexInput) {
        mBufferChannel->setInputBufferArray(array);
    } else if (portIndex == kPortIndexOutput) {
        mBufferChannel->setOutputBufferArray(array);
    } else {
        TRESPASS();
    }

    return OK;
}

status_t ACodec::setupNativeWindowSizeFormatAndUsage(
        ANativeWindow *nativeWindow /* nonnull */, int *finalUsage /* nonnull */,
        bool reconnect) {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    OMX_INDEXTYPE index;
    err = mOMXNode->getExtensionIndex(
            "OMX.google.android.index.AndroidNativeBufferConsumerUsage",
            &index);

    if (err != OK) {
        // allow failure
        err = OK;
    } else {
        int usageBits = 0;
        if (nativeWindow->query(
                nativeWindow,
                NATIVE_WINDOW_CONSUMER_USAGE_BITS,
                &usageBits) == OK) {
            OMX_PARAM_U32TYPE params;
            InitOMXParams(&params);
            params.nPortIndex = kPortIndexOutput;
            params.nU32 = (OMX_U32)usageBits;

            err = mOMXNode->setParameter(index, &params, sizeof(params));

            if (err != OK) {
                ALOGE("Fail to set AndroidNativeBufferConsumerUsage: %d", err);
                return err;
            }
        }
    }

    OMX_U32 usage = 0;
    err = mOMXNode->getGraphicBufferUsage(kPortIndexOutput, &usage);
    if (err != 0) {
        ALOGW("querying usage flags from OMX IL component failed: %d", err);
        // XXX: Currently this error is logged, but not fatal.
        usage = 0;
    }
    int omxUsage = usage;

    if (mFlags & kFlagIsGrallocUsageProtected) {
        usage |= GRALLOC_USAGE_PROTECTED;
    }

    usage |= kVideoGrallocUsage;
    *finalUsage = usage;

    memset(&mLastNativeWindowCrop, 0, sizeof(mLastNativeWindowCrop));
    mLastNativeWindowDataSpace = HAL_DATASPACE_UNKNOWN;

    ALOGV("gralloc usage: %#x(OMX) => %#x(ACodec)", omxUsage, usage);
    return setNativeWindowSizeFormatAndUsage(
            nativeWindow,
            def.format.video.nFrameWidth,
            def.format.video.nFrameHeight,
            def.format.video.eColorFormat,
            mRotationDegrees,
            usage,
            reconnect);
}

status_t ACodec::configureOutputBuffersFromNativeWindow(
        OMX_U32 *bufferCount, OMX_U32 *bufferSize,
        OMX_U32 *minUndequeuedBuffers, bool preregister) {

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err == OK) {
        err = setupNativeWindowSizeFormatAndUsage(
                mNativeWindow.get(), &mNativeWindowUsageBits,
                preregister && !mTunneled /* reconnect */);
    }
    if (err != OK) {
        mNativeWindowUsageBits = 0;
        return err;
    }

    static_cast<Surface *>(mNativeWindow.get())->setDequeueTimeout(-1);

    // Exits here for tunneled video playback codecs -- i.e. skips native window
    // buffer allocation step as this is managed by the tunneled OMX omponent
    // itself and explicitly sets def.nBufferCountActual to 0.
    if (mTunneled) {
        ALOGV("Tunneled Playback: skipping native window buffer allocation.");
        def.nBufferCountActual = 0;
        err = mOMXNode->setParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));

        *minUndequeuedBuffers = 0;
        *bufferCount = 0;
        *bufferSize = 0;
        return err;
    }

    *minUndequeuedBuffers = 0;
    err = mNativeWindow->query(
            mNativeWindow.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
            (int *)minUndequeuedBuffers);

    if (err != 0) {
        ALOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query failed: %s (%d)",
                strerror(-err), -err);
        return err;
    }

    // FIXME: assume that surface is controlled by app (native window
    // returns the number for the case when surface is not controlled by app)
    // FIXME2: This means that minUndeqeueudBufs can be 1 larger than reported
    // For now, try to allocate 1 more buffer, but don't fail if unsuccessful

    // Use conservative allocation while also trying to reduce starvation
    //
    // 1. allocate at least nBufferCountMin + minUndequeuedBuffers - that is the
    //    minimum needed for the consumer to be able to work
    // 2. try to allocate two (2) additional buffers to reduce starvation from
    //    the consumer
    //    plus an extra buffer to account for incorrect minUndequeuedBufs
    for (OMX_U32 extraBuffers = 2 + 1; /* condition inside loop */; extraBuffers--) {
        OMX_U32 newBufferCount =
            def.nBufferCountMin + *minUndequeuedBuffers + extraBuffers;
        def.nBufferCountActual = newBufferCount;
        err = mOMXNode->setParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));

        if (err == OK) {
            *minUndequeuedBuffers += extraBuffers;
            break;
        }

        ALOGW("[%s] setting nBufferCountActual to %u failed: %d",
                mComponentName.c_str(), newBufferCount, err);
        /* exit condition */
        if (extraBuffers == 0) {
            return err;
        }
    }

    err = native_window_set_buffer_count(
            mNativeWindow.get(), def.nBufferCountActual);

    if (err != 0) {
        ALOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err),
                -err);
        return err;
    }

    *bufferCount = def.nBufferCountActual;
    *bufferSize =  def.nBufferSize;
    return err;
}

status_t ACodec::allocateOutputBuffersFromNativeWindow() {
    // This method only handles the non-metadata mode (or simulating legacy
    // mode with metadata, which is transparent to ACodec).
    CHECK(!storingMetadataInDecodedBuffers());

    OMX_U32 bufferCount, bufferSize, minUndequeuedBuffers;
    status_t err = configureOutputBuffersFromNativeWindow(
            &bufferCount, &bufferSize, &minUndequeuedBuffers, true /* preregister */);
    if (err != 0)
        return err;
    mNumUndequeuedBuffers = minUndequeuedBuffers;

    static_cast<Surface*>(mNativeWindow.get())
            ->getIGraphicBufferProducer()->allowAllocation(true);

    ALOGV("[%s] Allocating %u buffers from a native window of size %u on "
         "output port",
         mComponentName.c_str(), bufferCount, bufferSize);

    // Dequeue buffers and send them to OMX
    for (OMX_U32 i = 0; i < bufferCount; i++) {
        ANativeWindowBuffer *buf;
        int fenceFd;
        err = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &buf, &fenceFd);
        if (err != 0) {
            ALOGE("dequeueBuffer failed: %s (%d)", strerror(-err), -err);
            break;
        }

        sp<GraphicBuffer> graphicBuffer(GraphicBuffer::from(buf));
        BufferInfo info;
        info.mStatus = BufferInfo::OWNED_BY_US;
        info.mFenceFd = fenceFd;
        info.mIsReadFence = false;
        info.mRenderInfo = NULL;
        info.mGraphicBuffer = graphicBuffer;
        info.mNewGraphicBuffer = false;
        info.mDequeuedAt = mDequeueCounter;

        // TODO: We shouln't need to create MediaCodecBuffer. In metadata mode
        //       OMX doesn't use the shared memory buffer, but some code still
        //       access info.mData. Create an ABuffer as a placeholder.
        info.mData = new MediaCodecBuffer(mOutputFormat, new ABuffer(bufferSize));
        info.mCodecData = info.mData;

        mBuffers[kPortIndexOutput].push(info);

        IOMX::buffer_id bufferId;
        err = mOMXNode->useBuffer(kPortIndexOutput, graphicBuffer, &bufferId);
        if (err != 0) {
            ALOGE("registering GraphicBuffer %u with OMX IL component failed: "
                 "%d", i, err);
            break;
        }

        mBuffers[kPortIndexOutput].editItemAt(i).mBufferID = bufferId;

        ALOGV("[%s] Registered graphic buffer with ID %u (pointer = %p)",
             mComponentName.c_str(),
             bufferId, graphicBuffer.get());
    }

    OMX_U32 cancelStart;
    OMX_U32 cancelEnd;

    if (err != OK) {
        // If an error occurred while dequeuing we need to cancel any buffers
        // that were dequeued. Also cancel all if we're in legacy metadata mode.
        cancelStart = 0;
        cancelEnd = mBuffers[kPortIndexOutput].size();
    } else {
        // Return the required minimum undequeued buffers to the native window.
        cancelStart = bufferCount - minUndequeuedBuffers;
        cancelEnd = bufferCount;
    }

    for (OMX_U32 i = cancelStart; i < cancelEnd; i++) {
        BufferInfo *info = &mBuffers[kPortIndexOutput].editItemAt(i);
        if (info->mStatus == BufferInfo::OWNED_BY_US) {
            status_t error = cancelBufferToNativeWindow(info);
            if (err == 0) {
                err = error;
            }
        }
    }

    static_cast<Surface*>(mNativeWindow.get())
            ->getIGraphicBufferProducer()->allowAllocation(false);

    return err;
}

status_t ACodec::allocateOutputMetadataBuffers() {
    CHECK(storingMetadataInDecodedBuffers());

    OMX_U32 bufferCount, bufferSize, minUndequeuedBuffers;
    status_t err = configureOutputBuffersFromNativeWindow(
            &bufferCount, &bufferSize, &minUndequeuedBuffers,
            false /* preregister */);
    if (err != OK)
        return err;
    mNumUndequeuedBuffers = minUndequeuedBuffers;

    ALOGV("[%s] Allocating %u meta buffers on output port",
         mComponentName.c_str(), bufferCount);

    for (OMX_U32 i = 0; i < bufferCount; i++) {
        BufferInfo info;
        info.mStatus = BufferInfo::OWNED_BY_NATIVE_WINDOW;
        info.mFenceFd = -1;
        info.mRenderInfo = NULL;
        info.mGraphicBuffer = NULL;
        info.mNewGraphicBuffer = false;
        info.mDequeuedAt = mDequeueCounter;

        info.mData = new MediaCodecBuffer(mOutputFormat, new ABuffer(bufferSize));

        // Initialize fence fd to -1 to avoid warning in freeBuffer().
        ((VideoNativeMetadata *)info.mData->base())->nFenceFd = -1;

        info.mCodecData = info.mData;

        err = mOMXNode->useBuffer(kPortIndexOutput, OMXBuffer::sPreset, &info.mBufferID);
        mBuffers[kPortIndexOutput].push(info);

        ALOGV("[%s] allocated meta buffer with ID %u",
                mComponentName.c_str(), info.mBufferID);
    }

    mMetadataBuffersToSubmit = bufferCount - minUndequeuedBuffers;
    return err;
}

status_t ACodec::submitOutputMetadataBuffer() {
    CHECK(storingMetadataInDecodedBuffers());
    if (mMetadataBuffersToSubmit == 0)
        return OK;

    BufferInfo *info = dequeueBufferFromNativeWindow();
    if (info == NULL) {
        return ERROR_IO;
    }

    ALOGV("[%s] submitting output meta buffer ID %u for graphic buffer %p",
          mComponentName.c_str(), info->mBufferID, info->mGraphicBuffer->handle);

    --mMetadataBuffersToSubmit;
    info->checkWriteFence("submitOutputMetadataBuffer");
    return fillBuffer(info);
}

status_t ACodec::waitForFence(int fd, const char *dbg ) {
    status_t res = OK;
    if (fd >= 0) {
        sp<Fence> fence = new Fence(fd);
        res = fence->wait(IOMX::kFenceTimeoutMs);
        ALOGW_IF(res != OK, "FENCE TIMEOUT for %d in %s", fd, dbg);
    }
    return res;
}

// static
const char *ACodec::_asString(BufferInfo::Status s) {
    switch (s) {
        case BufferInfo::OWNED_BY_US:            return "OUR";
        case BufferInfo::OWNED_BY_COMPONENT:     return "COMPONENT";
        case BufferInfo::OWNED_BY_UPSTREAM:      return "UPSTREAM";
        case BufferInfo::OWNED_BY_DOWNSTREAM:    return "DOWNSTREAM";
        case BufferInfo::OWNED_BY_NATIVE_WINDOW: return "SURFACE";
        case BufferInfo::UNRECOGNIZED:           return "UNRECOGNIZED";
        default:                                 return "?";
    }
}

void ACodec::dumpBuffers(OMX_U32 portIndex) {
    CHECK(portIndex == kPortIndexInput || portIndex == kPortIndexOutput);
    ALOGI("[%s] %s port has %zu buffers:", mComponentName.c_str(),
            portIndex == kPortIndexInput ? "input" : "output", mBuffers[portIndex].size());
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        const BufferInfo &info = mBuffers[portIndex][i];
        ALOGI("  slot %2zu: #%8u %p/%p %s(%d) dequeued:%u",
                i, info.mBufferID, info.mGraphicBuffer.get(),
                info.mGraphicBuffer == NULL ? NULL : info.mGraphicBuffer->getNativeBuffer(),
                _asString(info.mStatus), info.mStatus, info.mDequeuedAt);
    }
}

status_t ACodec::cancelBufferToNativeWindow(BufferInfo *info) {
    CHECK_EQ((int)info->mStatus, (int)BufferInfo::OWNED_BY_US);

    ALOGV("[%s] Calling cancelBuffer on buffer %u",
         mComponentName.c_str(), info->mBufferID);

    info->checkWriteFence("cancelBufferToNativeWindow");
    int err = mNativeWindow->cancelBuffer(
        mNativeWindow.get(), info->mGraphicBuffer.get(), info->mFenceFd);
    info->mFenceFd = -1;

    ALOGW_IF(err != 0, "[%s] can not return buffer %u to native window",
            mComponentName.c_str(), info->mBufferID);
    // change ownership even if cancelBuffer fails
    info->mStatus = BufferInfo::OWNED_BY_NATIVE_WINDOW;

    return err;
}

void ACodec::updateRenderInfoForDequeuedBuffer(
        ANativeWindowBuffer *buf, int fenceFd, BufferInfo *info) {

    info->mRenderInfo =
        mRenderTracker.updateInfoForDequeuedBuffer(
                buf, fenceFd, info - &mBuffers[kPortIndexOutput][0]);

    // check for any fences already signaled
    notifyOfRenderedFrames(false /* dropIncomplete */, info->mRenderInfo);
}

void ACodec::onFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano) {
    if (mRenderTracker.onFrameRendered(mediaTimeUs, systemNano) != OK) {
        mRenderTracker.dumpRenderQueue();
    }
}

void ACodec::notifyOfRenderedFrames(bool dropIncomplete, FrameRenderTracker::Info *until) {
    std::list<FrameRenderTracker::Info> done =
        mRenderTracker.checkFencesAndGetRenderedFrames(until, dropIncomplete);

    // unlink untracked frames
    for (std::list<FrameRenderTracker::Info>::const_iterator it = done.cbegin();
            it != done.cend(); ++it) {
        ssize_t index = it->getIndex();
        if (index >= 0 && (size_t)index < mBuffers[kPortIndexOutput].size()) {
            mBuffers[kPortIndexOutput].editItemAt(index).mRenderInfo = NULL;
        } else if (index >= 0) {
            // THIS SHOULD NEVER HAPPEN
            ALOGE("invalid index %zd in %zu", index, mBuffers[kPortIndexOutput].size());
        }
    }

    mCallback->onOutputFramesRendered(done);
}

ACodec::BufferInfo *ACodec::dequeueBufferFromNativeWindow() {
    ANativeWindowBuffer *buf;
    CHECK(mNativeWindow.get() != NULL);

    if (mTunneled) {
        ALOGW("dequeueBufferFromNativeWindow() should not be called in tunnel"
              " video playback mode mode!");
        return NULL;
    }

    if (mFatalError) {
        ALOGW("not dequeuing from native window due to fatal error");
        return NULL;
    }

    int fenceFd = -1;
    do {
        status_t err = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &buf, &fenceFd);
        if (err != 0) {
            ALOGE("dequeueBuffer failed: %s(%d).", asString(err), err);
            return NULL;
        }

        bool stale = false;
        for (size_t i = mBuffers[kPortIndexOutput].size(); i > 0;) {
            i--;
            BufferInfo *info = &mBuffers[kPortIndexOutput].editItemAt(i);

            if (info->mGraphicBuffer != NULL &&
                    info->mGraphicBuffer->handle == buf->handle) {
                // Since consumers can attach buffers to BufferQueues, it is possible
                // that a known yet stale buffer can return from a surface that we
                // once used.  We can simply ignore this as we have already dequeued
                // this buffer properly.  NOTE: this does not eliminate all cases,
                // e.g. it is possible that we have queued the valid buffer to the
                // NW, and a stale copy of the same buffer gets dequeued - which will
                // be treated as the valid buffer by ACodec.
                if (info->mStatus != BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                    ALOGI("dequeued stale buffer %p. discarding", buf);
                    stale = true;
                    break;
                }

                ALOGV("dequeued buffer #%u with age %u, graphicBuffer %p",
                        (unsigned)(info - &mBuffers[kPortIndexOutput][0]),
                        mDequeueCounter - info->mDequeuedAt,
                        info->mGraphicBuffer->handle);

                info->mStatus = BufferInfo::OWNED_BY_US;
                info->setWriteFence(fenceFd, "dequeueBufferFromNativeWindow");
                updateRenderInfoForDequeuedBuffer(buf, fenceFd, info);
                return info;
            }
        }

        // It is also possible to receive a previously unregistered buffer
        // in non-meta mode. These should be treated as stale buffers. The
        // same is possible in meta mode, in which case, it will be treated
        // as a normal buffer, which is not desirable.
        // TODO: fix this.
        if (!stale && !storingMetadataInDecodedBuffers()) {
            ALOGI("dequeued unrecognized (stale) buffer %p. discarding", buf);
            stale = true;
        }
        if (stale) {
            // TODO: detach stale buffer, but there is no API yet to do it.
            buf = NULL;
        }
    } while (buf == NULL);

    // get oldest undequeued buffer
    BufferInfo *oldest = NULL;
    for (size_t i = mBuffers[kPortIndexOutput].size(); i > 0;) {
        i--;
        BufferInfo *info =
            &mBuffers[kPortIndexOutput].editItemAt(i);
        if (info->mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW &&
            (oldest == NULL ||
             // avoid potential issues from counter rolling over
             mDequeueCounter - info->mDequeuedAt >
                    mDequeueCounter - oldest->mDequeuedAt)) {
            oldest = info;
        }
    }

    // it is impossible dequeue a buffer when there are no buffers with ANW
    CHECK(oldest != NULL);
    // it is impossible to dequeue an unknown buffer in non-meta mode, as the
    // while loop above does not complete
    CHECK(storingMetadataInDecodedBuffers());

    // discard buffer in LRU info and replace with new buffer
    oldest->mGraphicBuffer = GraphicBuffer::from(buf);
    oldest->mNewGraphicBuffer = true;
    oldest->mStatus = BufferInfo::OWNED_BY_US;
    oldest->setWriteFence(fenceFd, "dequeueBufferFromNativeWindow for oldest");
    mRenderTracker.untrackFrame(oldest->mRenderInfo);
    oldest->mRenderInfo = NULL;

    ALOGV("replaced oldest buffer #%u with age %u, graphicBuffer %p",
            (unsigned)(oldest - &mBuffers[kPortIndexOutput][0]),
            mDequeueCounter - oldest->mDequeuedAt,
            oldest->mGraphicBuffer->handle);

    updateRenderInfoForDequeuedBuffer(buf, fenceFd, oldest);
    return oldest;
}

status_t ACodec::freeBuffersOnPort(OMX_U32 portIndex) {
    if (portIndex == kPortIndexInput) {
        mBufferChannel->setInputBufferArray({});
    } else {
        mBufferChannel->setOutputBufferArray({});
    }

    status_t err = OK;
    for (size_t i = mBuffers[portIndex].size(); i > 0;) {
        i--;
        status_t err2 = freeBuffer(portIndex, i);
        if (err == OK) {
            err = err2;
        }
    }

    mAllocator[portIndex].clear();
    return err;
}

status_t ACodec::freeOutputBuffersNotOwnedByComponent() {
    status_t err = OK;
    for (size_t i = mBuffers[kPortIndexOutput].size(); i > 0;) {
        i--;
        BufferInfo *info =
            &mBuffers[kPortIndexOutput].editItemAt(i);

        // At this time some buffers may still be with the component
        // or being drained.
        if (info->mStatus != BufferInfo::OWNED_BY_COMPONENT &&
            info->mStatus != BufferInfo::OWNED_BY_DOWNSTREAM) {
            status_t err2 = freeBuffer(kPortIndexOutput, i);
            if (err == OK) {
                err = err2;
            }
        }
    }

    return err;
}

status_t ACodec::freeBuffer(OMX_U32 portIndex, size_t i) {
    BufferInfo *info = &mBuffers[portIndex].editItemAt(i);
    status_t err = OK;

    // there should not be any fences in the metadata
    if (mPortMode[portIndex] == IOMX::kPortModeDynamicANWBuffer && info->mCodecData != NULL
            && info->mCodecData->size() >= sizeof(VideoNativeMetadata)) {
        int fenceFd = ((VideoNativeMetadata *)info->mCodecData->base())->nFenceFd;
        if (fenceFd >= 0) {
            ALOGW("unreleased fence (%d) in %s metadata buffer %zu",
                    fenceFd, portIndex == kPortIndexInput ? "input" : "output", i);
        }
    }

    switch (info->mStatus) {
        case BufferInfo::OWNED_BY_US:
            if (portIndex == kPortIndexOutput && mNativeWindow != NULL) {
                (void)cancelBufferToNativeWindow(info);
            }
            FALLTHROUGH_INTENDED;

        case BufferInfo::OWNED_BY_NATIVE_WINDOW:
            err = mOMXNode->freeBuffer(portIndex, info->mBufferID);
            break;

        default:
            ALOGE("trying to free buffer not owned by us or ANW (%d)", info->mStatus);
            err = FAILED_TRANSACTION;
            break;
    }

    if (info->mFenceFd >= 0) {
        ::close(info->mFenceFd);
    }

    if (portIndex == kPortIndexOutput) {
        mRenderTracker.untrackFrame(info->mRenderInfo, i);
        info->mRenderInfo = NULL;
    }

    // remove buffer even if mOMXNode->freeBuffer fails
    mBuffers[portIndex].removeAt(i);
    return err;
}

ACodec::BufferInfo *ACodec::findBufferByID(
        uint32_t portIndex, IOMX::buffer_id bufferID, ssize_t *index) {
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        BufferInfo *info = &mBuffers[portIndex].editItemAt(i);

        if (info->mBufferID == bufferID) {
            if (index != NULL) {
                *index = i;
            }
            return info;
        }
    }

    ALOGE("Could not find buffer with ID %u", bufferID);
    return NULL;
}

status_t ACodec::fillBuffer(BufferInfo *info) {
    status_t err;
    // Even in dynamic ANW buffer mode, if the graphic buffer is not changing,
    // send sPreset instead of the same graphic buffer, so that OMX server
    // side doesn't update the meta. In theory it should make no difference,
    // however when the same buffer is parcelled again, a new handle could be
    // created on server side, and some decoder doesn't recognize the handle
    // even if it's the same buffer.
    if (!storingMetadataInDecodedBuffers() || !info->mNewGraphicBuffer) {
        err = mOMXNode->fillBuffer(
            info->mBufferID, OMXBuffer::sPreset, info->mFenceFd);
    } else {
        err = mOMXNode->fillBuffer(
            info->mBufferID, info->mGraphicBuffer, info->mFenceFd);
    }

    info->mNewGraphicBuffer = false;
    info->mFenceFd = -1;
    if (err == OK) {
        info->mStatus = BufferInfo::OWNED_BY_COMPONENT;
    }
    return err;
}

status_t ACodec::setComponentRole(
        bool isEncoder, const char *mime) {
    const char *role = GetComponentRole(isEncoder, mime);
    if (role == NULL) {
        return BAD_VALUE;
    }
    status_t err = SetComponentRole(mOMXNode, role);
    if (err != OK) {
        ALOGW("[%s] Failed to set standard component role '%s'.",
             mComponentName.c_str(), role);
    }
    return err;
}

status_t ACodec::configureCodec(
        const char *mime, const sp<AMessage> &msg) {
    int32_t encoder;
    if (!msg->findInt32("encoder", &encoder)) {
        encoder = false;
    }

    sp<AMessage> inputFormat = new AMessage;
    sp<AMessage> outputFormat = new AMessage;
    mConfigFormat = msg;

    mIsEncoder = encoder;
    mIsVideo = !strncasecmp(mime, "video/", 6);
    mIsImage = !strncasecmp(mime, "image/", 6);

    mPortMode[kPortIndexInput] = IOMX::kPortModePresetByteBuffer;
    mPortMode[kPortIndexOutput] = IOMX::kPortModePresetByteBuffer;

    status_t err = setComponentRole(encoder /* isEncoder */, mime);

    if (err != OK) {
        return err;
    }

    OMX_VIDEO_CONTROLRATETYPE bitrateMode;
    int32_t bitrate = 0, quality;
    // FLAC encoder or video encoder in constant quality mode doesn't need a
    // bitrate, other encoders do.
    if (encoder) {
        if (mIsVideo || mIsImage) {
            if (!findVideoBitrateControlInfo(msg, &bitrateMode, &bitrate, &quality)) {
                return INVALID_OPERATION;
            }
        } else if (strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC)
            && !msg->findInt32("bitrate", &bitrate)) {
            return INVALID_OPERATION;
        }
    }

    // propagate bitrate to the output so that the muxer has it
    if (encoder && msg->findInt32("bitrate", &bitrate)) {
        // Technically ISO spec says that 'bitrate' should be 0 for VBR even though it is the
        // average bitrate. We've been setting both bitrate and max-bitrate to this same value.
        outputFormat->setInt32("bitrate", bitrate);
        outputFormat->setInt32("max-bitrate", bitrate);
    }

    int32_t storeMeta;
    if (encoder) {
        IOMX::PortMode mode = IOMX::kPortModePresetByteBuffer;
        if (msg->findInt32("android._input-metadata-buffer-type", &storeMeta)
                && storeMeta != kMetadataBufferTypeInvalid) {
            if (storeMeta == kMetadataBufferTypeNativeHandleSource) {
                mode = IOMX::kPortModeDynamicNativeHandle;
            } else if (storeMeta == kMetadataBufferTypeANWBuffer ||
                    storeMeta == kMetadataBufferTypeGrallocSource) {
                mode = IOMX::kPortModeDynamicANWBuffer;
            } else {
                return BAD_VALUE;
            }
        }
        err = setPortMode(kPortIndexInput, mode);
        if (err != OK) {
            return err;
        }

        if (mode != IOMX::kPortModePresetByteBuffer) {
            uint32_t usageBits;
            if (mOMXNode->getParameter(
                    (OMX_INDEXTYPE)OMX_IndexParamConsumerUsageBits,
                    &usageBits, sizeof(usageBits)) == OK) {
                inputFormat->setInt32(
                        "using-sw-read-often", !!(usageBits & GRALLOC_USAGE_SW_READ_OFTEN));
            }
        }
    }

    int32_t prependSPSPPS = 0;
    if (encoder && mIsVideo
            && msg->findInt32("prepend-sps-pps-to-idr-frames", &prependSPSPPS)
            && prependSPSPPS != 0) {
        OMX_INDEXTYPE index;
        err = mOMXNode->getExtensionIndex(
                "OMX.google.android.index.prependSPSPPSToIDRFrames", &index);

        if (err == OK) {
            PrependSPSPPSToIDRFramesParams params;
            InitOMXParams(&params);
            params.bEnable = OMX_TRUE;

            err = mOMXNode->setParameter(index, &params, sizeof(params));
        }

        if (err != OK) {
            ALOGE("Encoder could not be configured to emit SPS/PPS before "
                  "IDR frames. (err %d)", err);

            return err;
        }
    }

    // Only enable metadata mode on encoder output if encoder can prepend
    // sps/pps to idr frames, since in metadata mode the bitstream is in an
    // opaque handle, to which we don't have access.
    if (encoder && mIsVideo) {
        OMX_BOOL enable = (OMX_BOOL) (prependSPSPPS
            && msg->findInt32("android._store-metadata-in-buffers-output", &storeMeta)
            && storeMeta != 0);
        if (mFlags & kFlagIsSecure) {
            enable = OMX_TRUE;
        }

        err = setPortMode(kPortIndexOutput, enable ?
                IOMX::kPortModePresetSecureBuffer : IOMX::kPortModePresetByteBuffer);
        if (err != OK) {
            return err;
        }

        if (!msg->findInt64(
                KEY_REPEAT_PREVIOUS_FRAME_AFTER, &mRepeatFrameDelayUs)) {
            mRepeatFrameDelayUs = -1LL;
        }

        if (!msg->findDouble("time-lapse-fps", &mCaptureFps)) {
            float captureRate;
            if (msg->findAsFloat(KEY_CAPTURE_RATE, &captureRate)) {
                mCaptureFps = captureRate;
            } else {
                mCaptureFps = -1.0;
            }
        }

        if (!msg->findInt32(
                KEY_CREATE_INPUT_SURFACE_SUSPENDED,
                (int32_t*)&mCreateInputBuffersSuspended)) {
            mCreateInputBuffersSuspended = false;
        }
    }

    if (encoder && (mIsVideo || mIsImage)) {
        // only allow 32-bit value, since we pass it as U32 to OMX.
        if (!msg->findInt64(KEY_MAX_PTS_GAP_TO_ENCODER, &mMaxPtsGapUs)) {
            mMaxPtsGapUs = 0LL;
        } else if (mMaxPtsGapUs > INT32_MAX || mMaxPtsGapUs < INT32_MIN) {
            ALOGW("Unsupported value for max pts gap %lld", (long long) mMaxPtsGapUs);
            mMaxPtsGapUs = 0LL;
        }

        if (!msg->findFloat(KEY_MAX_FPS_TO_ENCODER, &mMaxFps)) {
            mMaxFps = -1;
        }

        // notify GraphicBufferSource to allow backward frames
        if (mMaxPtsGapUs < 0LL) {
            mMaxFps = -1;
        }
    }

    // NOTE: we only use native window for video decoders
    sp<RefBase> obj;
    bool haveNativeWindow = msg->findObject("native-window", &obj)
            && obj != NULL && mIsVideo && !encoder;
    mUsingNativeWindow = haveNativeWindow;
    if (mIsVideo && !encoder) {
        inputFormat->setInt32("adaptive-playback", false);

        int32_t usageProtected;
        if (msg->findInt32("protected", &usageProtected) && usageProtected) {
            if (!haveNativeWindow) {
                ALOGE("protected output buffers must be sent to an ANativeWindow");
                return PERMISSION_DENIED;
            }
            mFlags |= kFlagIsGrallocUsageProtected;
            mFlags |= kFlagPushBlankBuffersToNativeWindowOnShutdown;
        }
    }
    if (mFlags & kFlagIsSecure) {
        // use native_handles for secure input buffers
        err = setPortMode(kPortIndexInput, IOMX::kPortModePresetSecureBuffer);

        if (err != OK) {
            ALOGI("falling back to non-native_handles");
            setPortMode(kPortIndexInput, IOMX::kPortModePresetByteBuffer);
            err = OK; // ignore error for now
        }
    }
    if (haveNativeWindow) {
        sp<ANativeWindow> nativeWindow =
            static_cast<ANativeWindow *>(static_cast<Surface *>(obj.get()));

        // START of temporary support for automatic FRC - THIS WILL BE REMOVED
        int32_t autoFrc;
        if (msg->findInt32("auto-frc", &autoFrc)) {
            bool enabled = autoFrc;
            OMX_CONFIG_BOOLEANTYPE config;
            InitOMXParams(&config);
            config.bEnabled = (OMX_BOOL)enabled;
            status_t temp = mOMXNode->setConfig(
                    (OMX_INDEXTYPE)OMX_IndexConfigAutoFramerateConversion,
                    &config, sizeof(config));
            if (temp == OK) {
                outputFormat->setInt32("auto-frc", enabled);
            } else if (enabled) {
                ALOGI("codec does not support requested auto-frc (err %d)", temp);
            }
        }
        // END of temporary support for automatic FRC

        int32_t tunneled;
        if (msg->findInt32("feature-tunneled-playback", &tunneled) &&
            tunneled != 0) {
            ALOGI("Configuring TUNNELED video playback.");
            mTunneled = true;

            int32_t audioHwSync = 0;
            if (!msg->findInt32("audio-hw-sync", &audioHwSync)) {
                ALOGW("No Audio HW Sync provided for video tunnel");
            }
            err = configureTunneledVideoPlayback(audioHwSync, nativeWindow);
            if (err != OK) {
                ALOGE("configureTunneledVideoPlayback(%d,%p) failed!",
                        audioHwSync, nativeWindow.get());
                return err;
            }

            int32_t maxWidth = 0, maxHeight = 0;
            if (msg->findInt32("max-width", &maxWidth) &&
                    msg->findInt32("max-height", &maxHeight)) {

                err = mOMXNode->prepareForAdaptivePlayback(
                        kPortIndexOutput, OMX_TRUE, maxWidth, maxHeight);
                if (err != OK) {
                    ALOGW("[%s] prepareForAdaptivePlayback failed w/ err %d",
                            mComponentName.c_str(), err);
                    // allow failure
                    err = OK;
                } else {
                    inputFormat->setInt32("max-width", maxWidth);
                    inputFormat->setInt32("max-height", maxHeight);
                    inputFormat->setInt32("adaptive-playback", true);
                }
            }
        } else {
            ALOGV("Configuring CPU controlled video playback.");
            mTunneled = false;

            // Explicity reset the sideband handle of the window for
            // non-tunneled video in case the window was previously used
            // for a tunneled video playback.
            err = native_window_set_sideband_stream(nativeWindow.get(), NULL);
            if (err != OK) {
                ALOGE("set_sideband_stream(NULL) failed! (err %d).", err);
                return err;
            }

            err = setPortMode(kPortIndexOutput, IOMX::kPortModeDynamicANWBuffer);
            if (err != OK) {
                // if adaptive playback has been requested, try JB fallback
                // NOTE: THIS FALLBACK MECHANISM WILL BE REMOVED DUE TO ITS
                // LARGE MEMORY REQUIREMENT

                // we will not do adaptive playback on software accessed
                // surfaces as they never had to respond to changes in the
                // crop window, and we don't trust that they will be able to.
                int usageBits = 0;
                bool canDoAdaptivePlayback;

                if (nativeWindow->query(
                        nativeWindow.get(),
                        NATIVE_WINDOW_CONSUMER_USAGE_BITS,
                        &usageBits) != OK) {
                    canDoAdaptivePlayback = false;
                } else {
                    canDoAdaptivePlayback =
                        (usageBits &
                                (GRALLOC_USAGE_SW_READ_MASK |
                                 GRALLOC_USAGE_SW_WRITE_MASK)) == 0;
                }

                int32_t maxWidth = 0, maxHeight = 0;
                if (canDoAdaptivePlayback &&
                        msg->findInt32("max-width", &maxWidth) &&
                        msg->findInt32("max-height", &maxHeight)) {
                    ALOGV("[%s] prepareForAdaptivePlayback(%dx%d)",
                            mComponentName.c_str(), maxWidth, maxHeight);

                    err = mOMXNode->prepareForAdaptivePlayback(
                            kPortIndexOutput, OMX_TRUE, maxWidth, maxHeight);
                    ALOGW_IF(err != OK,
                            "[%s] prepareForAdaptivePlayback failed w/ err %d",
                            mComponentName.c_str(), err);

                    if (err == OK) {
                        inputFormat->setInt32("max-width", maxWidth);
                        inputFormat->setInt32("max-height", maxHeight);
                        inputFormat->setInt32("adaptive-playback", true);
                    }
                }
                // allow failure
                err = OK;
            } else {
                ALOGV("[%s] setPortMode on output to %s succeeded",
                        mComponentName.c_str(), asString(IOMX::kPortModeDynamicANWBuffer));
                CHECK(storingMetadataInDecodedBuffers());
                inputFormat->setInt32("adaptive-playback", true);
            }

            int32_t push;
            if (msg->findInt32("push-blank-buffers-on-shutdown", &push)
                    && push != 0) {
                mFlags |= kFlagPushBlankBuffersToNativeWindowOnShutdown;
            }
        }

        int32_t rotationDegrees;
        if (msg->findInt32("rotation-degrees", &rotationDegrees)) {
            mRotationDegrees = rotationDegrees;
        } else {
            mRotationDegrees = 0;
        }
    }

    AudioEncoding pcmEncoding = kAudioEncodingPcm16bit;
    (void)msg->findInt32("pcm-encoding", (int32_t*)&pcmEncoding);
    // invalid encodings will default to PCM-16bit in setupRawAudioFormat.

    if (mIsVideo || mIsImage) {
        // determine need for software renderer
        bool usingSwRenderer = false;
        if (haveNativeWindow && mComponentName.startsWith("OMX.google.")) {
            usingSwRenderer = true;
            haveNativeWindow = false;
            (void)setPortMode(kPortIndexOutput, IOMX::kPortModePresetByteBuffer);
        } else if (haveNativeWindow && !storingMetadataInDecodedBuffers()) {
            err = setPortMode(kPortIndexOutput, IOMX::kPortModePresetANWBuffer);
            if (err != OK) {
                return err;
            }
        }

        if (encoder) {
            err = setupVideoEncoder(mime, msg, outputFormat, inputFormat);
        } else {
            err = setupVideoDecoder(mime, msg, haveNativeWindow, usingSwRenderer, outputFormat);
        }

        if (err != OK) {
            return err;
        }

        if (haveNativeWindow) {
            mNativeWindow = static_cast<Surface *>(obj.get());

            // fallback for devices that do not handle flex-YUV for native buffers
            int32_t requestedColorFormat = OMX_COLOR_FormatUnused;
            if (msg->findInt32("color-format", &requestedColorFormat) &&
                    requestedColorFormat == OMX_COLOR_FormatYUV420Flexible) {
                status_t err = getPortFormat(kPortIndexOutput, outputFormat);
                if (err != OK) {
                    return err;
                }
                int32_t colorFormat = OMX_COLOR_FormatUnused;
                OMX_U32 flexibleEquivalent = OMX_COLOR_FormatUnused;
                if (!outputFormat->findInt32("color-format", &colorFormat)) {
                    ALOGE("ouptut port did not have a color format (wrong domain?)");
                    return BAD_VALUE;
                }
                ALOGD("[%s] Requested output format %#x and got %#x.",
                        mComponentName.c_str(), requestedColorFormat, colorFormat);
                if (!IsFlexibleColorFormat(
                        mOMXNode, colorFormat, haveNativeWindow, &flexibleEquivalent)
                        || flexibleEquivalent != (OMX_U32)requestedColorFormat) {
                    // device did not handle flex-YUV request for native window, fall back
                    // to SW renderer
                    ALOGI("[%s] Falling back to software renderer", mComponentName.c_str());
                    mNativeWindow.clear();
                    mNativeWindowUsageBits = 0;
                    haveNativeWindow = false;
                    usingSwRenderer = true;
                    // TODO: implement adaptive-playback support for bytebuffer mode.
                    // This is done by SW codecs, but most HW codecs don't support it.
                    err = setPortMode(kPortIndexOutput, IOMX::kPortModePresetByteBuffer);
                    inputFormat->setInt32("adaptive-playback", false);
                    if (mFlags & kFlagIsGrallocUsageProtected) {
                        // fallback is not supported for protected playback
                        err = PERMISSION_DENIED;
                    } else if (err == OK) {
                        err = setupVideoDecoder(
                                mime, msg, haveNativeWindow, usingSwRenderer, outputFormat);
                    }
                }
            }
        }

        if (usingSwRenderer) {
            outputFormat->setInt32("using-sw-renderer", 1);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II)) {
        int32_t numChannels, sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            // Since we did not always check for these, leave them optional
            // and have the decoder figure it all out.
            err = OK;
        } else {
            err = setupRawAudioFormat(
                    encoder ? kPortIndexInput : kPortIndexOutput,
                    sampleRate,
                    numChannels);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        int32_t numChannels, sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            int32_t isADTS, aacProfile;
            int32_t sbrMode;
            int32_t maxOutputChannelCount;
            int32_t pcmLimiterEnable;
            drcParams_t drc;
            if (!msg->findInt32("is-adts", &isADTS)) {
                isADTS = 0;
            }
            if (!msg->findInt32("aac-profile", &aacProfile)) {
                aacProfile = OMX_AUDIO_AACObjectNull;
            }
            if (!msg->findInt32("aac-sbr-mode", &sbrMode)) {
                sbrMode = -1;
            }

            if (!msg->findInt32("aac-max-output-channel_count", &maxOutputChannelCount)) {
                maxOutputChannelCount = -1;
            }
            if (!msg->findInt32("aac-pcm-limiter-enable", &pcmLimiterEnable)) {
                // value is unknown
                pcmLimiterEnable = -1;
            }
            if (!msg->findInt32("aac-encoded-target-level", &drc.encodedTargetLevel)) {
                // value is unknown
                drc.encodedTargetLevel = -1;
            }
            if (!msg->findInt32("aac-drc-cut-level", &drc.drcCut)) {
                // value is unknown
                drc.drcCut = -1;
            }
            if (!msg->findInt32("aac-drc-boost-level", &drc.drcBoost)) {
                // value is unknown
                drc.drcBoost = -1;
            }
            if (!msg->findInt32("aac-drc-heavy-compression", &drc.heavyCompression)) {
                // value is unknown
                drc.heavyCompression = -1;
            }
            if (!msg->findInt32("aac-target-ref-level", &drc.targetRefLevel)) {
                // value is unknown
                drc.targetRefLevel = -1;
            }
            if (!msg->findInt32("aac-drc-effect-type", &drc.effectType)) {
                // value is unknown
                drc.effectType = -2; // valid values are -1 and over
            }

            err = setupAACCodec(
                    encoder, numChannels, sampleRate, bitrate, aacProfile,
                    isADTS != 0, sbrMode, maxOutputChannelCount, drc,
                    pcmLimiterEnable);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_NB)) {
        err = setupAMRCodec(encoder, false /* isWAMR */, bitrate);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
        err = setupAMRCodec(encoder, true /* isWAMR */, bitrate);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_G711_ALAW)
            || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_G711_MLAW)) {
        // These are PCM-like formats with a fixed sample rate but
        // a variable number of channels.

        int32_t numChannels;
        if (!msg->findInt32("channel-count", &numChannels)) {
            err = INVALID_OPERATION;
        } else {
            int32_t sampleRate;
            if (!msg->findInt32("sample-rate", &sampleRate)) {
                sampleRate = 8000;
            }
            err = setupG711Codec(encoder, sampleRate, numChannels);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC)) {
        // numChannels needs to be set to properly communicate PCM values.
        int32_t numChannels = 2, sampleRate = 44100, compressionLevel = -1;
        if (encoder &&
                (!msg->findInt32("channel-count", &numChannels)
                        || !msg->findInt32("sample-rate", &sampleRate))) {
            ALOGE("missing channel count or sample rate for FLAC encoder");
            err = INVALID_OPERATION;
        } else {
            if (encoder) {
                if (!msg->findInt32(
                            "complexity", &compressionLevel) &&
                    !msg->findInt32(
                            "flac-compression-level", &compressionLevel)) {
                    compressionLevel = 5; // default FLAC compression level
                } else if (compressionLevel < 0) {
                    ALOGW("compression level %d outside [0..8] range, "
                          "using 0",
                          compressionLevel);
                    compressionLevel = 0;
                } else if (compressionLevel > 8) {
                    ALOGW("compression level %d outside [0..8] range, "
                          "using 8",
                          compressionLevel);
                    compressionLevel = 8;
                }
            }
            err = setupFlacCodec(
                    encoder, numChannels, sampleRate, compressionLevel, pcmEncoding);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW)) {
        int32_t numChannels, sampleRate;
        if (encoder
                || !msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupRawAudioFormat(kPortIndexInput, sampleRate, numChannels, pcmEncoding);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupAC3Codec(encoder, numChannels, sampleRate);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_EAC3)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupEAC3Codec(encoder, numChannels, sampleRate);
        }
     } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC4)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupAC4Codec(encoder, numChannels, sampleRate);
        }
    }

    if (err != OK) {
        return err;
    }

    if (!msg->findInt32("encoder-delay", &mEncoderDelay)) {
        mEncoderDelay = 0;
    }

    if (!msg->findInt32("encoder-padding", &mEncoderPadding)) {
        mEncoderPadding = 0;
    }

    if (msg->findInt32("channel-mask", &mChannelMask)) {
        mChannelMaskPresent = true;
    } else {
        mChannelMaskPresent = false;
    }

    int32_t maxInputSize;
    if (msg->findInt32("max-input-size", &maxInputSize)) {
        err = setMinBufferSize(kPortIndexInput, (size_t)maxInputSize);
        err = OK; // ignore error
    } else if (!strcmp("OMX.Nvidia.aac.decoder", mComponentName.c_str())) {
        err = setMinBufferSize(kPortIndexInput, 8192);  // XXX
        err = OK; // ignore error
    }

    int32_t priority;
    if (msg->findInt32("priority", &priority)) {
        err = setPriority(priority);
        err = OK; // ignore error
    }

    int32_t rateInt = -1;
    float rateFloat = -1;
    if (!msg->findFloat("operating-rate", &rateFloat)) {
        msg->findInt32("operating-rate", &rateInt);
        rateFloat = (float)rateInt;  // 16MHz (FLINTMAX) is OK for upper bound.
    }
    if (rateFloat > 0) {
        err = setOperatingRate(rateFloat, mIsVideo);
        err = OK; // ignore errors
    }

    if (err == OK) {
        err = setVendorParameters(msg);
        if (err != OK) {
            return err;
        }
    }

    // NOTE: both mBaseOutputFormat and mOutputFormat are outputFormat to signal first frame.
    mBaseOutputFormat = outputFormat;
    mLastOutputFormat.clear();

    err = getPortFormat(kPortIndexInput, inputFormat);
    if (err == OK) {
        err = getPortFormat(kPortIndexOutput, outputFormat);
        if (err == OK) {
            mInputFormat = inputFormat;
            mOutputFormat = outputFormat;
        }
    }

    // create data converters if needed
    if (!mIsVideo && !mIsImage && err == OK) {
        AudioEncoding codecPcmEncoding = kAudioEncodingPcm16bit;
        if (encoder) {
            (void)mInputFormat->findInt32("pcm-encoding", (int32_t*)&codecPcmEncoding);
            mConverter[kPortIndexInput] = AudioConverter::Create(pcmEncoding, codecPcmEncoding);
            if (mConverter[kPortIndexInput] != NULL) {
                ALOGD("%s: encoder %s input format pcm encoding converter from %d to %d",
                        __func__, mComponentName.c_str(), pcmEncoding, codecPcmEncoding);
                mInputFormat->setInt32("pcm-encoding", pcmEncoding);
            }
        } else {
            (void)mOutputFormat->findInt32("pcm-encoding", (int32_t*)&codecPcmEncoding);
            mConverter[kPortIndexOutput] = AudioConverter::Create(codecPcmEncoding, pcmEncoding);
            if (mConverter[kPortIndexOutput] != NULL) {
                ALOGD("%s: decoder %s output format pcm encoding converter from %d to %d",
                        __func__, mComponentName.c_str(), codecPcmEncoding, pcmEncoding);
                mOutputFormat->setInt32("pcm-encoding", pcmEncoding);
            }
        }
    }

    return err;
}

status_t ACodec::setLatency(uint32_t latency) {
    OMX_PARAM_U32TYPE config;
    InitOMXParams(&config);
    config.nPortIndex = kPortIndexInput;
    config.nU32 = (OMX_U32)latency;
    status_t err = mOMXNode->setConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigLatency,
            &config, sizeof(config));
    return err;
}

status_t ACodec::getLatency(uint32_t *latency) {
    OMX_PARAM_U32TYPE config;
    InitOMXParams(&config);
    config.nPortIndex = kPortIndexInput;
    status_t err = mOMXNode->getConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigLatency,
            &config, sizeof(config));
    if (err == OK) {
        *latency = config.nU32;
    }
    return err;
}

status_t ACodec::setAudioPresentation(int32_t presentationId, int32_t programId) {
    OMX_AUDIO_CONFIG_ANDROID_AUDIOPRESENTATION config;
    InitOMXParams(&config);
    config.nPresentationId = (OMX_S32)presentationId;
    config.nProgramId = (OMX_S32)programId;
    status_t err = mOMXNode->setConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigAudioPresentation,
            &config, sizeof(config));
    return err;
}

status_t ACodec::setPriority(int32_t priority) {
    if (priority < 0) {
        return BAD_VALUE;
    }
    OMX_PARAM_U32TYPE config;
    InitOMXParams(&config);
    config.nU32 = (OMX_U32)priority;
    status_t temp = mOMXNode->setConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigPriority,
            &config, sizeof(config));
    if (temp != OK) {
        ALOGI("codec does not support config priority (err %d)", temp);
    }
    return OK;
}

status_t ACodec::setOperatingRate(float rateFloat, bool isVideo) {
    if (rateFloat < 0) {
        return BAD_VALUE;
    }
    OMX_U32 rate;
    if (isVideo) {
        if (rateFloat > 65535) {
            return BAD_VALUE;
        }
        rate = (OMX_U32)(rateFloat * 65536.0f + 0.5f);
    } else {
        if (rateFloat > UINT_MAX) {
            return BAD_VALUE;
        }
        rate = (OMX_U32)(rateFloat);
    }
    OMX_PARAM_U32TYPE config;
    InitOMXParams(&config);
    config.nU32 = rate;
    status_t err = mOMXNode->setConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigOperatingRate,
            &config, sizeof(config));
    if (err != OK) {
        ALOGI("codec does not support config operating rate (err %d)", err);
    }
    return OK;
}

status_t ACodec::getIntraRefreshPeriod(uint32_t *intraRefreshPeriod) {
    OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexOutput;
    status_t err = mOMXNode->getConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigAndroidIntraRefresh, &params, sizeof(params));
    if (err == OK) {
        *intraRefreshPeriod = params.nRefreshPeriod;
        return OK;
    }

    // Fallback to query through standard OMX index.
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE refreshParams;
    InitOMXParams(&refreshParams);
    refreshParams.nPortIndex = kPortIndexOutput;
    refreshParams.eRefreshMode = OMX_VIDEO_IntraRefreshCyclic;
    err = mOMXNode->getParameter(
            OMX_IndexParamVideoIntraRefresh, &refreshParams, sizeof(refreshParams));
    if (err != OK || refreshParams.nCirMBs == 0) {
        *intraRefreshPeriod = 0;
        return OK;
    }

    // Calculate period based on width and height
    uint32_t width, height;
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;
    def.nPortIndex = kPortIndexOutput;
    err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        *intraRefreshPeriod = 0;
        return err;
    }
    width = video_def->nFrameWidth;
    height = video_def->nFrameHeight;
    // Use H.264/AVC MacroBlock size 16x16
    *intraRefreshPeriod = divUp((divUp(width, 16u) * divUp(height, 16u)), refreshParams.nCirMBs);

    return OK;
}

status_t ACodec::setIntraRefreshPeriod(uint32_t intraRefreshPeriod, bool inConfigure) {
    OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexOutput;
    params.nRefreshPeriod = intraRefreshPeriod;
    status_t err = mOMXNode->setConfig(
            (OMX_INDEXTYPE)OMX_IndexConfigAndroidIntraRefresh, &params, sizeof(params));
    if (err == OK) {
        return OK;
    }

    // Only in configure state, a component could invoke setParameter.
    if (!inConfigure) {
        return INVALID_OPERATION;
    } else {
        ALOGI("[%s] try falling back to Cyclic", mComponentName.c_str());
    }

    OMX_VIDEO_PARAM_INTRAREFRESHTYPE refreshParams;
    InitOMXParams(&refreshParams);
    refreshParams.nPortIndex = kPortIndexOutput;
    refreshParams.eRefreshMode = OMX_VIDEO_IntraRefreshCyclic;

    if (intraRefreshPeriod == 0) {
        // 0 means disable intra refresh.
        refreshParams.nCirMBs = 0;
    } else {
        // Calculate macroblocks that need to be intra coded base on width and height
        uint32_t width, height;
        OMX_PARAM_PORTDEFINITIONTYPE def;
        InitOMXParams(&def);
        OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;
        def.nPortIndex = kPortIndexOutput;
        err = mOMXNode->getParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));
        if (err != OK) {
            return err;
        }
        width = video_def->nFrameWidth;
        height = video_def->nFrameHeight;
        // Use H.264/AVC MacroBlock size 16x16
        refreshParams.nCirMBs = divUp((divUp(width, 16u) * divUp(height, 16u)), intraRefreshPeriod);
    }

    err = mOMXNode->setParameter(
            OMX_IndexParamVideoIntraRefresh,
            &refreshParams, sizeof(refreshParams));
    if (err != OK) {
        return err;
    }

    return OK;
}

status_t ACodec::configureTemporalLayers(
        const sp<AMessage> &msg, bool inConfigure, sp<AMessage> &outputFormat) {
    if (!mIsVideo || !mIsEncoder) {
        return INVALID_OPERATION;
    }

    AString tsSchema;
    if (!msg->findString("ts-schema", &tsSchema)) {
        return OK;
    }

    unsigned int numLayers = 0;
    unsigned int numBLayers = 0;
    int tags;
    char dummy;
    OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE pattern =
        OMX_VIDEO_AndroidTemporalLayeringPatternNone;
    if (sscanf(tsSchema.c_str(), "webrtc.vp8.%u-layer%c", &numLayers, &dummy) == 1
            && numLayers > 0) {
        pattern = OMX_VIDEO_AndroidTemporalLayeringPatternWebRTC;
    } else if ((tags = sscanf(tsSchema.c_str(), "android.generic.%u%c%u%c",
                    &numLayers, &dummy, &numBLayers, &dummy))
            && (tags == 1 || (tags == 3 && dummy == '+'))
            && numLayers > 0 && numLayers < UINT32_MAX - numBLayers) {
        numLayers += numBLayers;
        pattern = OMX_VIDEO_AndroidTemporalLayeringPatternAndroid;
    } else {
        ALOGI("Ignoring unsupported ts-schema [%s]", tsSchema.c_str());
        return BAD_VALUE;
    }

    OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE layerParams;
    InitOMXParams(&layerParams);
    layerParams.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
            &layerParams, sizeof(layerParams));

    if (err != OK) {
        return err;
    } else if (!(layerParams.eSupportedPatterns & pattern)) {
        return BAD_VALUE;
    }

    numLayers = min(numLayers, layerParams.nLayerCountMax);
    numBLayers = min(numBLayers, layerParams.nBLayerCountMax);

    if (!inConfigure) {
        OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE layerConfig;
        InitOMXParams(&layerConfig);
        layerConfig.nPortIndex = kPortIndexOutput;
        layerConfig.ePattern = pattern;
        layerConfig.nPLayerCountActual = numLayers - numBLayers;
        layerConfig.nBLayerCountActual = numBLayers;
        layerConfig.bBitrateRatiosSpecified = OMX_FALSE;

        err = mOMXNode->setConfig(
                (OMX_INDEXTYPE)OMX_IndexConfigAndroidVideoTemporalLayering,
                &layerConfig, sizeof(layerConfig));
    } else {
        layerParams.ePattern = pattern;
        layerParams.nPLayerCountActual = numLayers - numBLayers;
        layerParams.nBLayerCountActual = numBLayers;
        layerParams.bBitrateRatiosSpecified = OMX_FALSE;
        layerParams.nLayerCountMax = numLayers;
        layerParams.nBLayerCountMax = numBLayers;

        err = mOMXNode->setParameter(
                (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
                &layerParams, sizeof(layerParams));
    }

    AString configSchema;
    if (pattern == OMX_VIDEO_AndroidTemporalLayeringPatternAndroid) {
        configSchema = AStringPrintf("android.generic.%u+%u", numLayers - numBLayers, numBLayers);
    } else if (pattern == OMX_VIDEO_AndroidTemporalLayeringPatternWebRTC) {
        configSchema = AStringPrintf("webrtc.vp8.%u", numLayers);
    }

    if (err != OK) {
        ALOGW("Failed to set temporal layers to %s (requested %s)",
                configSchema.c_str(), tsSchema.c_str());
        return err;
    }

    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
            &layerParams, sizeof(layerParams));

    if (err == OK) {
        ALOGD("Temporal layers requested:%s configured:%s got:%s(%u: P=%u, B=%u)",
                tsSchema.c_str(), configSchema.c_str(),
                asString(layerParams.ePattern), layerParams.ePattern,
                layerParams.nPLayerCountActual, layerParams.nBLayerCountActual);

        if (outputFormat.get() == mOutputFormat.get()) {
            mOutputFormat = mOutputFormat->dup(); // trigger an output format change event
        }
        // assume we got what we configured
        outputFormat->setString("ts-schema", configSchema);
    }
    return err;
}

status_t ACodec::setMinBufferSize(OMX_U32 portIndex, size_t size) {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    if (def.nBufferSize >= size) {
        return OK;
    }

    def.nBufferSize = size;

    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    if (def.nBufferSize < size) {
        ALOGE("failed to set min buffer size to %zu (is still %u)", size, def.nBufferSize);
        return FAILED_TRANSACTION;
    }

    return OK;
}

status_t ACodec::selectAudioPortFormat(
        OMX_U32 portIndex, OMX_AUDIO_CODINGTYPE desiredFormat) {
    OMX_AUDIO_PARAM_PORTFORMATTYPE format;
    InitOMXParams(&format);

    format.nPortIndex = portIndex;
    for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
        format.nIndex = index;
        status_t err = mOMXNode->getParameter(
                OMX_IndexParamAudioPortFormat, &format, sizeof(format));

        if (err != OK) {
            return err;
        }

        if (format.eEncoding == desiredFormat) {
            break;
        }

        if (index == kMaxIndicesToCheck) {
            ALOGW("[%s] stopping checking formats after %u: %s(%x)",
                    mComponentName.c_str(), index,
                    asString(format.eEncoding), format.eEncoding);
            return ERROR_UNSUPPORTED;
        }
    }

    return mOMXNode->setParameter(
            OMX_IndexParamAudioPortFormat, &format, sizeof(format));
}

status_t ACodec::setupAACCodec(
        bool encoder, int32_t numChannels, int32_t sampleRate,
        int32_t bitRate, int32_t aacProfile, bool isADTS, int32_t sbrMode,
        int32_t maxOutputChannelCount, const drcParams_t& drc,
        int32_t pcmLimiterEnable) {
    if (encoder && isADTS) {
        return -EINVAL;
    }

    status_t err = setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput,
            sampleRate,
            numChannels);

    if (err != OK) {
        return err;
    }

    if (encoder) {
        err = selectAudioPortFormat(kPortIndexOutput, OMX_AUDIO_CodingAAC);

        if (err != OK) {
            return err;
        }

        OMX_PARAM_PORTDEFINITIONTYPE def;
        InitOMXParams(&def);
        def.nPortIndex = kPortIndexOutput;

        err = mOMXNode->getParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));

        if (err != OK) {
            return err;
        }

        def.format.audio.bFlagErrorConcealment = OMX_TRUE;
        def.format.audio.eEncoding = OMX_AUDIO_CodingAAC;

        err = mOMXNode->setParameter(
                OMX_IndexParamPortDefinition, &def, sizeof(def));

        if (err != OK) {
            return err;
        }

        OMX_AUDIO_PARAM_AACPROFILETYPE profile;
        InitOMXParams(&profile);
        profile.nPortIndex = kPortIndexOutput;

        err = mOMXNode->getParameter(
                OMX_IndexParamAudioAac, &profile, sizeof(profile));

        if (err != OK) {
            return err;
        }

        profile.nChannels = numChannels;

        profile.eChannelMode =
            (numChannels == 1)
                ? OMX_AUDIO_ChannelModeMono: OMX_AUDIO_ChannelModeStereo;

        profile.nSampleRate = sampleRate;
        profile.nBitRate = bitRate;
        profile.nAudioBandWidth = 0;
        profile.nFrameLength = 0;
        profile.nAACtools = OMX_AUDIO_AACToolAll;
        profile.nAACERtools = OMX_AUDIO_AACERNone;
        profile.eAACProfile = (OMX_AUDIO_AACPROFILETYPE) aacProfile;
        profile.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4FF;
        switch (sbrMode) {
        case 0:
            // disable sbr
            profile.nAACtools &= ~OMX_AUDIO_AACToolAndroidSSBR;
            profile.nAACtools &= ~OMX_AUDIO_AACToolAndroidDSBR;
            break;
        case 1:
            // enable single-rate sbr
            profile.nAACtools |= OMX_AUDIO_AACToolAndroidSSBR;
            profile.nAACtools &= ~OMX_AUDIO_AACToolAndroidDSBR;
            break;
        case 2:
            // enable dual-rate sbr
            profile.nAACtools &= ~OMX_AUDIO_AACToolAndroidSSBR;
            profile.nAACtools |= OMX_AUDIO_AACToolAndroidDSBR;
            break;
        case -1:
            // enable both modes -> the codec will decide which mode should be used
            profile.nAACtools |= OMX_AUDIO_AACToolAndroidSSBR;
            profile.nAACtools |= OMX_AUDIO_AACToolAndroidDSBR;
            break;
        default:
            // unsupported sbr mode
            return BAD_VALUE;
        }


        err = mOMXNode->setParameter(
                OMX_IndexParamAudioAac, &profile, sizeof(profile));

        if (err != OK) {
            return err;
        }

        return err;
    }

    OMX_AUDIO_PARAM_AACPROFILETYPE profile;
    InitOMXParams(&profile);
    profile.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            OMX_IndexParamAudioAac, &profile, sizeof(profile));

    if (err != OK) {
        return err;
    }

    profile.nChannels = numChannels;
    profile.nSampleRate = sampleRate;

    profile.eAACStreamFormat =
        isADTS
            ? OMX_AUDIO_AACStreamFormatMP4ADTS
            : OMX_AUDIO_AACStreamFormatMP4FF;

    OMX_AUDIO_PARAM_ANDROID_AACDRCPRESENTATIONTYPE presentation;
    InitOMXParams(&presentation);
    presentation.nMaxOutputChannels = maxOutputChannelCount;
    presentation.nDrcCut = drc.drcCut;
    presentation.nDrcBoost = drc.drcBoost;
    presentation.nHeavyCompression = drc.heavyCompression;
    presentation.nTargetReferenceLevel = drc.targetRefLevel;
    presentation.nEncodedTargetLevel = drc.encodedTargetLevel;
    presentation.nPCMLimiterEnable = pcmLimiterEnable;
    presentation.nDrcEffectType = drc.effectType;

    status_t res = mOMXNode->setParameter(
            OMX_IndexParamAudioAac, &profile, sizeof(profile));
    if (res == OK) {
        // optional parameters, will not cause configuration failure
        if (mOMXNode->setParameter(
                (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAacDrcPresentation,
                &presentation, sizeof(presentation)) == ERROR_UNSUPPORTED) {
            // prior to 9.0 we used a different config structure and index
            OMX_AUDIO_PARAM_ANDROID_AACPRESENTATIONTYPE presentation8;
            InitOMXParams(&presentation8);
            presentation8.nMaxOutputChannels = presentation.nMaxOutputChannels;
            presentation8.nDrcCut = presentation.nDrcCut;
            presentation8.nDrcBoost = presentation.nDrcBoost;
            presentation8.nHeavyCompression = presentation.nHeavyCompression;
            presentation8.nTargetReferenceLevel = presentation.nTargetReferenceLevel;
            presentation8.nEncodedTargetLevel = presentation.nEncodedTargetLevel;
            presentation8.nPCMLimiterEnable = presentation.nPCMLimiterEnable;
            (void)mOMXNode->setParameter(
                (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAacPresentation,
                &presentation8, sizeof(presentation8));
        }
    } else {
        ALOGW("did not set AudioAndroidAacPresentation due to error %d when setting AudioAac", res);
    }
    mSampleRate = sampleRate;
    return res;
}

status_t ACodec::setupAC3Codec(
        bool encoder, int32_t numChannels, int32_t sampleRate) {
    status_t err = setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput, sampleRate, numChannels);

    if (err != OK) {
        return err;
    }

    if (encoder) {
        ALOGW("AC3 encoding is not supported.");
        return INVALID_OPERATION;
    }

    OMX_AUDIO_PARAM_ANDROID_AC3TYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc3, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    def.nChannels = numChannels;
    def.nSampleRate = sampleRate;

    return mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc3, &def, sizeof(def));
}

status_t ACodec::setupEAC3Codec(
        bool encoder, int32_t numChannels, int32_t sampleRate) {
    status_t err = setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput, sampleRate, numChannels);

    if (err != OK) {
        return err;
    }

    if (encoder) {
        ALOGW("EAC3 encoding is not supported.");
        return INVALID_OPERATION;
    }

    OMX_AUDIO_PARAM_ANDROID_EAC3TYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidEac3, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    def.nChannels = numChannels;
    def.nSampleRate = sampleRate;

    return mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidEac3, &def, sizeof(def));
}

status_t ACodec::setupAC4Codec(
        bool encoder, int32_t numChannels, int32_t sampleRate) {
    status_t err = setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput, sampleRate, numChannels);

    if (err != OK) {
        return err;
    }

    if (encoder) {
        ALOGW("AC4 encoding is not supported.");
        return INVALID_OPERATION;
    }

    OMX_AUDIO_PARAM_ANDROID_AC4TYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc4, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    def.nChannels = numChannels;
    def.nSampleRate = sampleRate;

    return mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc4, &def, sizeof(def));
}

static OMX_AUDIO_AMRBANDMODETYPE pickModeFromBitRate(
        bool isAMRWB, int32_t bps) {
    if (isAMRWB) {
        if (bps <= 6600) {
            return OMX_AUDIO_AMRBandModeWB0;
        } else if (bps <= 8850) {
            return OMX_AUDIO_AMRBandModeWB1;
        } else if (bps <= 12650) {
            return OMX_AUDIO_AMRBandModeWB2;
        } else if (bps <= 14250) {
            return OMX_AUDIO_AMRBandModeWB3;
        } else if (bps <= 15850) {
            return OMX_AUDIO_AMRBandModeWB4;
        } else if (bps <= 18250) {
            return OMX_AUDIO_AMRBandModeWB5;
        } else if (bps <= 19850) {
            return OMX_AUDIO_AMRBandModeWB6;
        } else if (bps <= 23050) {
            return OMX_AUDIO_AMRBandModeWB7;
        }

        // 23850 bps
        return OMX_AUDIO_AMRBandModeWB8;
    } else {  // AMRNB
        if (bps <= 4750) {
            return OMX_AUDIO_AMRBandModeNB0;
        } else if (bps <= 5150) {
            return OMX_AUDIO_AMRBandModeNB1;
        } else if (bps <= 5900) {
            return OMX_AUDIO_AMRBandModeNB2;
        } else if (bps <= 6700) {
            return OMX_AUDIO_AMRBandModeNB3;
        } else if (bps <= 7400) {
            return OMX_AUDIO_AMRBandModeNB4;
        } else if (bps <= 7950) {
            return OMX_AUDIO_AMRBandModeNB5;
        } else if (bps <= 10200) {
            return OMX_AUDIO_AMRBandModeNB6;
        }

        // 12200 bps
        return OMX_AUDIO_AMRBandModeNB7;
    }
}

status_t ACodec::setupAMRCodec(bool encoder, bool isWAMR, int32_t bitrate) {
    OMX_AUDIO_PARAM_AMRTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = encoder ? kPortIndexOutput : kPortIndexInput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamAudioAmr, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    def.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;
    def.eAMRBandMode = pickModeFromBitRate(isWAMR, bitrate);

    err = mOMXNode->setParameter(
            OMX_IndexParamAudioAmr, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    return setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput,
            isWAMR ? 16000 : 8000 /* sampleRate */,
            1 /* numChannels */);
}

status_t ACodec::setupG711Codec(bool encoder, int32_t sampleRate, int32_t numChannels) {
    if (encoder) {
        return INVALID_OPERATION;
    }

    return setupRawAudioFormat(
            kPortIndexInput, sampleRate, numChannels);
}

status_t ACodec::setupFlacCodec(
        bool encoder, int32_t numChannels, int32_t sampleRate, int32_t compressionLevel,
        AudioEncoding encoding) {
    if (encoder) {
        OMX_AUDIO_PARAM_FLACTYPE def;
        InitOMXParams(&def);
        def.nPortIndex = kPortIndexOutput;

        // configure compression level
        status_t err = mOMXNode->getParameter(OMX_IndexParamAudioFlac, &def, sizeof(def));
        if (err != OK) {
            ALOGE("setupFlacCodec(): Error %d getting OMX_IndexParamAudioFlac parameter", err);
            return err;
        }
        def.nCompressionLevel = compressionLevel;
        err = mOMXNode->setParameter(OMX_IndexParamAudioFlac, &def, sizeof(def));
        if (err != OK) {
            ALOGE("setupFlacCodec(): Error %d setting OMX_IndexParamAudioFlac parameter", err);
            return err;
        }
    }

    return setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput,
            sampleRate,
            numChannels,
            encoding);
}

status_t ACodec::setupRawAudioFormat(
        OMX_U32 portIndex, int32_t sampleRate, int32_t numChannels, AudioEncoding encoding) {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    def.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    OMX_AUDIO_PARAM_PCMMODETYPE pcmParams;
    InitOMXParams(&pcmParams);
    pcmParams.nPortIndex = portIndex;

    err = mOMXNode->getParameter(
            OMX_IndexParamAudioPcm, &pcmParams, sizeof(pcmParams));

    if (err != OK) {
        return err;
    }

    pcmParams.nChannels = numChannels;
    switch (encoding) {
        case kAudioEncodingPcm8bit:
            pcmParams.eNumData = OMX_NumericalDataUnsigned;
            pcmParams.nBitPerSample = 8;
            break;
        case kAudioEncodingPcmFloat:
            pcmParams.eNumData = OMX_NumericalDataFloat;
            pcmParams.nBitPerSample = 32;
            break;
        case kAudioEncodingPcm16bit:
            pcmParams.eNumData = OMX_NumericalDataSigned;
            pcmParams.nBitPerSample = 16;
            break;
        default:
            return BAD_VALUE;
    }
    pcmParams.bInterleaved = OMX_TRUE;
    pcmParams.nSamplingRate = sampleRate;
    pcmParams.ePCMMode = OMX_AUDIO_PCMModeLinear;

    if (getOMXChannelMapping(numChannels, pcmParams.eChannelMapping) != OK) {
        ALOGE("%s: incorrect numChannels: %d", __func__, numChannels);
        return OMX_ErrorNone;
    }

    err = mOMXNode->setParameter(
            OMX_IndexParamAudioPcm, &pcmParams, sizeof(pcmParams));
    // if we could not set up raw format to non-16-bit, try with 16-bit
    // NOTE: we will also verify this via readback, in case codec ignores these fields
    if (err != OK && encoding != kAudioEncodingPcm16bit) {
        pcmParams.eNumData = OMX_NumericalDataSigned;
        pcmParams.nBitPerSample = 16;
        err = mOMXNode->setParameter(
                OMX_IndexParamAudioPcm, &pcmParams, sizeof(pcmParams));
    }
    return err;
}

status_t ACodec::configureTunneledVideoPlayback(
        int32_t audioHwSync, const sp<ANativeWindow> &nativeWindow) {
    native_handle_t* sidebandHandle;

    status_t err = mOMXNode->configureVideoTunnelMode(
            kPortIndexOutput, OMX_TRUE, audioHwSync, &sidebandHandle);
    if (err != OK) {
        ALOGE("configureVideoTunnelMode failed! (err %d).", err);
        return err;
    }

    err = native_window_set_sideband_stream(nativeWindow.get(), sidebandHandle);
    if (err != OK) {
        ALOGE("native_window_set_sideband_stream(%p) failed! (err %d).",
                sidebandHandle, err);
        return err;
    }

    return OK;
}

status_t ACodec::setVideoPortFormatType(
        OMX_U32 portIndex,
        OMX_VIDEO_CODINGTYPE compressionFormat,
        OMX_COLOR_FORMATTYPE colorFormat,
        bool usingNativeBuffers) {
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    InitOMXParams(&format);
    format.nPortIndex = portIndex;
    format.nIndex = 0;
    bool found = false;

    for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
        format.nIndex = index;
        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoPortFormat,
                &format, sizeof(format));

        if (err != OK) {
            return err;
        }

        // substitute back flexible color format to codec supported format
        OMX_U32 flexibleEquivalent;
        if (compressionFormat == OMX_VIDEO_CodingUnused
                && IsFlexibleColorFormat(
                        mOMXNode, format.eColorFormat, usingNativeBuffers, &flexibleEquivalent)
                && colorFormat == flexibleEquivalent) {
            ALOGI("[%s] using color format %#x in place of %#x",
                    mComponentName.c_str(), format.eColorFormat, colorFormat);
            colorFormat = format.eColorFormat;
        }

        // The following assertion is violated by TI's video decoder.
        // CHECK_EQ(format.nIndex, index);

        if (!strcmp("OMX.TI.Video.encoder", mComponentName.c_str())) {
            if (portIndex == kPortIndexInput
                    && colorFormat == format.eColorFormat) {
                // eCompressionFormat does not seem right.
                found = true;
                break;
            }
            if (portIndex == kPortIndexOutput
                    && compressionFormat == format.eCompressionFormat) {
                // eColorFormat does not seem right.
                found = true;
                break;
            }
        }

        if (format.eCompressionFormat == compressionFormat
            && format.eColorFormat == colorFormat) {
            found = true;
            break;
        }

        if (index == kMaxIndicesToCheck) {
            ALOGW("[%s] stopping checking formats after %u: %s(%x)/%s(%x)",
                    mComponentName.c_str(), index,
                    asString(format.eCompressionFormat), format.eCompressionFormat,
                    asString(format.eColorFormat), format.eColorFormat);
        }
    }

    if (!found) {
        return UNKNOWN_ERROR;
    }

    status_t err = mOMXNode->setParameter(
            OMX_IndexParamVideoPortFormat, &format, sizeof(format));

    return err;
}

// Set optimal output format. OMX component lists output formats in the order
// of preference, but this got more complicated since the introduction of flexible
// YUV formats. We support a legacy behavior for applications that do not use
// surface output, do not specify an output format, but expect a "usable" standard
// OMX format. SW readable and standard formats must be flex-YUV.
//
// Suggested preference order:
// - optimal format for texture rendering (mediaplayer behavior)
// - optimal SW readable & texture renderable format (flex-YUV support)
// - optimal SW readable non-renderable format (flex-YUV bytebuffer support)
// - legacy "usable" standard formats
//
// For legacy support, we prefer a standard format, but will settle for a SW readable
// flex-YUV format.
status_t ACodec::setSupportedOutputFormat(bool getLegacyFlexibleFormat) {
    OMX_VIDEO_PARAM_PORTFORMATTYPE format, legacyFormat;
    InitOMXParams(&format);
    format.nPortIndex = kPortIndexOutput;

    InitOMXParams(&legacyFormat);
    // this field will change when we find a suitable legacy format
    legacyFormat.eColorFormat = OMX_COLOR_FormatUnused;

    for (OMX_U32 index = 0; ; ++index) {
        format.nIndex = index;
        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoPortFormat, &format, sizeof(format));
        if (err != OK) {
            // no more formats, pick legacy format if found
            if (legacyFormat.eColorFormat != OMX_COLOR_FormatUnused) {
                 memcpy(&format, &legacyFormat, sizeof(format));
                 break;
            }
            return err;
        }
        if (format.eCompressionFormat != OMX_VIDEO_CodingUnused) {
            return OMX_ErrorBadParameter;
        }
        if (!getLegacyFlexibleFormat) {
            break;
        }
        // standard formats that were exposed to users before
        if (format.eColorFormat == OMX_COLOR_FormatYUV420Planar
                || format.eColorFormat == OMX_COLOR_FormatYUV420PackedPlanar
                || format.eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar
                || format.eColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar
                || format.eColorFormat == OMX_TI_COLOR_FormatYUV420PackedSemiPlanar) {
            break;
        }
        // find best legacy non-standard format
        OMX_U32 flexibleEquivalent;
        if (legacyFormat.eColorFormat == OMX_COLOR_FormatUnused
                && IsFlexibleColorFormat(
                        mOMXNode, format.eColorFormat, false /* usingNativeBuffers */,
                        &flexibleEquivalent)
                && flexibleEquivalent == OMX_COLOR_FormatYUV420Flexible) {
            memcpy(&legacyFormat, &format, sizeof(format));
        }
    }
    return mOMXNode->setParameter(
            OMX_IndexParamVideoPortFormat, &format, sizeof(format));
}

static const struct VideoCodingMapEntry {
    const char *mMime;
    OMX_VIDEO_CODINGTYPE mVideoCodingType;
} kVideoCodingMapEntry[] = {
    { MEDIA_MIMETYPE_VIDEO_AVC, OMX_VIDEO_CodingAVC },
    { MEDIA_MIMETYPE_VIDEO_HEVC, OMX_VIDEO_CodingHEVC },
    { MEDIA_MIMETYPE_VIDEO_MPEG4, OMX_VIDEO_CodingMPEG4 },
    { MEDIA_MIMETYPE_VIDEO_H263, OMX_VIDEO_CodingH263 },
    { MEDIA_MIMETYPE_VIDEO_MPEG2, OMX_VIDEO_CodingMPEG2 },
    { MEDIA_MIMETYPE_VIDEO_VP8, OMX_VIDEO_CodingVP8 },
    { MEDIA_MIMETYPE_VIDEO_VP9, OMX_VIDEO_CodingVP9 },
    { MEDIA_MIMETYPE_VIDEO_DOLBY_VISION, OMX_VIDEO_CodingDolbyVision },
    { MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC, OMX_VIDEO_CodingImageHEIC },
};

static status_t GetVideoCodingTypeFromMime(
        const char *mime, OMX_VIDEO_CODINGTYPE *codingType) {
    for (size_t i = 0;
         i < sizeof(kVideoCodingMapEntry) / sizeof(kVideoCodingMapEntry[0]);
         ++i) {
        if (!strcasecmp(mime, kVideoCodingMapEntry[i].mMime)) {
            *codingType = kVideoCodingMapEntry[i].mVideoCodingType;
            return OK;
        }
    }

    *codingType = OMX_VIDEO_CodingUnused;

    return ERROR_UNSUPPORTED;
}

static status_t GetMimeTypeForVideoCoding(
        OMX_VIDEO_CODINGTYPE codingType, AString *mime) {
    for (size_t i = 0;
         i < sizeof(kVideoCodingMapEntry) / sizeof(kVideoCodingMapEntry[0]);
         ++i) {
        if (codingType == kVideoCodingMapEntry[i].mVideoCodingType) {
            *mime = kVideoCodingMapEntry[i].mMime;
            return OK;
        }
    }

    mime->clear();

    return ERROR_UNSUPPORTED;
}

status_t ACodec::setPortBufferNum(OMX_U32 portIndex, int bufferNum) {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;
    status_t err;
    ALOGD("Setting [%s] %s port buffer number: %d", mComponentName.c_str(),
            portIndex == kPortIndexInput ? "input" : "output", bufferNum);
    err = mOMXNode->getParameter(
        OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        return err;
    }
    def.nBufferCountActual = bufferNum;
    err = mOMXNode->setParameter(
        OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        // Component could reject this request.
        ALOGW("Fail to set [%s] %s port buffer number: %d", mComponentName.c_str(),
            portIndex == kPortIndexInput ? "input" : "output", bufferNum);
    }
    return OK;
}

status_t ACodec::setupVideoDecoder(
        const char *mime, const sp<AMessage> &msg, bool haveNativeWindow,
        bool usingSwRenderer, sp<AMessage> &outputFormat) {
    int32_t width, height;
    if (!msg->findInt32("width", &width)
            || !msg->findInt32("height", &height)) {
        return INVALID_OPERATION;
    }

    OMX_VIDEO_CODINGTYPE compressionFormat;
    status_t err = GetVideoCodingTypeFromMime(mime, &compressionFormat);

    if (err != OK) {
        return err;
    }

    if (compressionFormat == OMX_VIDEO_CodingHEVC) {
        int32_t profile;
        if (msg->findInt32("profile", &profile)) {
            // verify if Main10 profile is supported at all, and fail
            // immediately if it's not supported.
            if (profile == OMX_VIDEO_HEVCProfileMain10 ||
                profile == OMX_VIDEO_HEVCProfileMain10HDR10) {
                err = verifySupportForProfileAndLevel(
                        kPortIndexInput, profile, 0);
                if (err != OK) {
                    return err;
                }
            }
        }
    }

    if (compressionFormat == OMX_VIDEO_CodingVP9) {
        OMX_VIDEO_PARAM_PROFILELEVELTYPE params;
        InitOMXParams(&params);
        params.nPortIndex = kPortIndexInput;
        // Check if VP9 decoder advertises supported profiles.
        params.nProfileIndex = 0;
        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoProfileLevelQuerySupported,
                &params, sizeof(params));
        mIsLegacyVP9Decoder = err != OK;
    }

    err = setVideoPortFormatType(
            kPortIndexInput, compressionFormat, OMX_COLOR_FormatUnused);

    if (err != OK) {
        return err;
    }

    int32_t tmp;
    if (msg->findInt32("color-format", &tmp)) {
        OMX_COLOR_FORMATTYPE colorFormat =
            static_cast<OMX_COLOR_FORMATTYPE>(tmp);
        err = setVideoPortFormatType(
                kPortIndexOutput, OMX_VIDEO_CodingUnused, colorFormat, haveNativeWindow);
        if (err != OK) {
            ALOGW("[%s] does not support color format %d",
                  mComponentName.c_str(), colorFormat);
            err = setSupportedOutputFormat(!haveNativeWindow /* getLegacyFlexibleFormat */);
        }
    } else {
        err = setSupportedOutputFormat(!haveNativeWindow /* getLegacyFlexibleFormat */);
    }

    if (err != OK) {
        return err;
    }

    // Set the component input buffer number to be |tmp|. If succeed,
    // component will set input port buffer number to be |tmp|. If fail,
    // component will keep the same buffer number as before.
    if (msg->findInt32("android._num-input-buffers", &tmp)) {
        err = setPortBufferNum(kPortIndexInput, tmp);
        if (err != OK)
            return err;
    }

    // Set the component output buffer number to be |tmp|. If succeed,
    // component will set output port buffer number to be |tmp|. If fail,
    // component will keep the same buffer number as before.
    if (msg->findInt32("android._num-output-buffers", &tmp)) {
        err = setPortBufferNum(kPortIndexOutput, tmp);
        if (err != OK)
            return err;
    }

    int32_t frameRateInt;
    float frameRateFloat;
    if (!msg->findFloat("frame-rate", &frameRateFloat)) {
        if (!msg->findInt32("frame-rate", &frameRateInt)) {
            frameRateInt = -1;
        }
        frameRateFloat = (float)frameRateInt;
    }

    err = setVideoFormatOnPort(
            kPortIndexInput, width, height, compressionFormat, frameRateFloat);

    if (err != OK) {
        return err;
    }

    err = setVideoFormatOnPort(
            kPortIndexOutput, width, height, OMX_VIDEO_CodingUnused);

    if (err != OK) {
        return err;
    }

    err = setColorAspectsForVideoDecoder(
            width, height, haveNativeWindow | usingSwRenderer, msg, outputFormat);
    if (err == ERROR_UNSUPPORTED) { // support is optional
        err = OK;
    }

    if (err != OK) {
        return err;
    }

    err = setHDRStaticInfoForVideoCodec(kPortIndexOutput, msg, outputFormat);
    if (err == ERROR_UNSUPPORTED) { // support is optional
        err = OK;
    }
    return err;
}

status_t ACodec::initDescribeColorAspectsIndex() {
    status_t err = mOMXNode->getExtensionIndex(
            "OMX.google.android.index.describeColorAspects", &mDescribeColorAspectsIndex);
    if (err != OK) {
        mDescribeColorAspectsIndex = (OMX_INDEXTYPE)0;
    }
    return err;
}

status_t ACodec::setCodecColorAspects(DescribeColorAspectsParams &params, bool verify) {
    status_t err = ERROR_UNSUPPORTED;
    if (mDescribeColorAspectsIndex) {
        err = mOMXNode->setConfig(mDescribeColorAspectsIndex, &params, sizeof(params));
    }
    ALOGV("[%s] setting color aspects (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) err=%d(%s)",
            mComponentName.c_str(),
            params.sAspects.mRange, asString(params.sAspects.mRange),
            params.sAspects.mPrimaries, asString(params.sAspects.mPrimaries),
            params.sAspects.mMatrixCoeffs, asString(params.sAspects.mMatrixCoeffs),
            params.sAspects.mTransfer, asString(params.sAspects.mTransfer),
            err, asString(err));

    if (verify && err == OK) {
        err = getCodecColorAspects(params);
    }

    ALOGW_IF(err == ERROR_UNSUPPORTED && mDescribeColorAspectsIndex,
            "[%s] setting color aspects failed even though codec advertises support",
            mComponentName.c_str());
    return err;
}

status_t ACodec::setColorAspectsForVideoDecoder(
        int32_t width, int32_t height, bool usingNativeWindow,
        const sp<AMessage> &configFormat, sp<AMessage> &outputFormat) {
    DescribeColorAspectsParams params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexOutput;

    getColorAspectsFromFormat(configFormat, params.sAspects);
    if (usingNativeWindow) {
        setDefaultCodecColorAspectsIfNeeded(params.sAspects, width, height);
        // The default aspects will be set back to the output format during the
        // getFormat phase of configure(). Set non-Unspecified values back into the
        // format, in case component does not support this enumeration.
        setColorAspectsIntoFormat(params.sAspects, outputFormat);
    }

    (void)initDescribeColorAspectsIndex();

    // communicate color aspects to codec
    return setCodecColorAspects(params);
}

status_t ACodec::getCodecColorAspects(DescribeColorAspectsParams &params) {
    status_t err = ERROR_UNSUPPORTED;
    if (mDescribeColorAspectsIndex) {
        err = mOMXNode->getConfig(mDescribeColorAspectsIndex, &params, sizeof(params));
    }
    ALOGV("[%s] got color aspects (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) err=%d(%s)",
            mComponentName.c_str(),
            params.sAspects.mRange, asString(params.sAspects.mRange),
            params.sAspects.mPrimaries, asString(params.sAspects.mPrimaries),
            params.sAspects.mMatrixCoeffs, asString(params.sAspects.mMatrixCoeffs),
            params.sAspects.mTransfer, asString(params.sAspects.mTransfer),
            err, asString(err));
    if (params.bRequestingDataSpace) {
        ALOGV("for dataspace %#x", params.nDataSpace);
    }
    if (err == ERROR_UNSUPPORTED && mDescribeColorAspectsIndex
            && !params.bRequestingDataSpace && !params.bDataSpaceChanged) {
        ALOGW("[%s] getting color aspects failed even though codec advertises support",
                mComponentName.c_str());
    }
    return err;
}

status_t ACodec::getInputColorAspectsForVideoEncoder(sp<AMessage> &format) {
    DescribeColorAspectsParams params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexInput;
    status_t err = getCodecColorAspects(params);
    if (err == OK) {
        // we only set encoder input aspects if codec supports them
        setColorAspectsIntoFormat(params.sAspects, format, true /* force */);
    }
    return err;
}

status_t ACodec::getDataSpace(
        DescribeColorAspectsParams &params, android_dataspace *dataSpace /* nonnull */,
        bool tryCodec) {
    status_t err = OK;
    if (tryCodec) {
        // request dataspace guidance from codec.
        params.bRequestingDataSpace = OMX_TRUE;
        err = getCodecColorAspects(params);
        params.bRequestingDataSpace = OMX_FALSE;
        if (err == OK && params.nDataSpace != HAL_DATASPACE_UNKNOWN) {
            *dataSpace = (android_dataspace)params.nDataSpace;
            return err;
        } else if (err == ERROR_UNSUPPORTED) {
            // ignore not-implemented error for dataspace requests
            err = OK;
        }
    }

    // this returns legacy versions if available
    *dataSpace = getDataSpaceForColorAspects(params.sAspects, true /* mayexpand */);
    ALOGV("[%s] using color aspects (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) "
          "and dataspace %#x",
            mComponentName.c_str(),
            params.sAspects.mRange, asString(params.sAspects.mRange),
            params.sAspects.mPrimaries, asString(params.sAspects.mPrimaries),
            params.sAspects.mMatrixCoeffs, asString(params.sAspects.mMatrixCoeffs),
            params.sAspects.mTransfer, asString(params.sAspects.mTransfer),
            *dataSpace);
    return err;
}


status_t ACodec::getColorAspectsAndDataSpaceForVideoDecoder(
        int32_t width, int32_t height, const sp<AMessage> &configFormat, sp<AMessage> &outputFormat,
        android_dataspace *dataSpace) {
    DescribeColorAspectsParams params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexOutput;

    // reset default format and get resulting format
    getColorAspectsFromFormat(configFormat, params.sAspects);
    if (dataSpace != NULL) {
        setDefaultCodecColorAspectsIfNeeded(params.sAspects, width, height);
    }
    status_t err = setCodecColorAspects(params, true /* readBack */);

    // we always set specified aspects for decoders
    setColorAspectsIntoFormat(params.sAspects, outputFormat);

    if (dataSpace != NULL) {
        status_t res = getDataSpace(params, dataSpace, err == OK /* tryCodec */);
        if (err == OK) {
            err = res;
        }
    }

    return err;
}

// initial video encoder setup for bytebuffer mode
status_t ACodec::setColorAspectsForVideoEncoder(
        const sp<AMessage> &configFormat, sp<AMessage> &outputFormat, sp<AMessage> &inputFormat) {
    // copy config to output format as this is not exposed via getFormat
    copyColorConfig(configFormat, outputFormat);

    DescribeColorAspectsParams params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexInput;
    getColorAspectsFromFormat(configFormat, params.sAspects);

    (void)initDescribeColorAspectsIndex();

    int32_t usingRecorder;
    if (configFormat->findInt32("android._using-recorder", &usingRecorder) && usingRecorder) {
        android_dataspace dataSpace = HAL_DATASPACE_BT709;
        int32_t width, height;
        if (configFormat->findInt32("width", &width)
                && configFormat->findInt32("height", &height)) {
            setDefaultCodecColorAspectsIfNeeded(params.sAspects, width, height);
            status_t err = getDataSpace(
                    params, &dataSpace, mDescribeColorAspectsIndex /* tryCodec */);
            if (err != OK) {
                return err;
            }
            setColorAspectsIntoFormat(params.sAspects, outputFormat);
        }
        inputFormat->setInt32("android._dataspace", (int32_t)dataSpace);
    }

    // communicate color aspects to codec, but do not allow change of the platform aspects
    ColorAspects origAspects = params.sAspects;
    for (int triesLeft = 2; --triesLeft >= 0; ) {
        status_t err = setCodecColorAspects(params, true /* readBack */);
        if (err != OK
                || !ColorUtils::checkIfAspectsChangedAndUnspecifyThem(
                        params.sAspects, origAspects, true /* usePlatformAspects */)) {
            return err;
        }
        ALOGW_IF(triesLeft == 0, "[%s] Codec repeatedly changed requested ColorAspects.",
                mComponentName.c_str());
    }
    return OK;
}

status_t ACodec::setHDRStaticInfoForVideoCodec(
        OMX_U32 portIndex, const sp<AMessage> &configFormat, sp<AMessage> &outputFormat) {
    CHECK(portIndex == kPortIndexInput || portIndex == kPortIndexOutput);

    DescribeHDRStaticInfoParams params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;

    HDRStaticInfo *info = &params.sInfo;
    if (getHDRStaticInfoFromFormat(configFormat, info)) {
        setHDRStaticInfoIntoFormat(params.sInfo, outputFormat);
    }

    (void)initDescribeHDRStaticInfoIndex();

    // communicate HDR static Info to codec
    return setHDRStaticInfo(params);
}

// subsequent initial video encoder setup for surface mode
status_t ACodec::setInitialColorAspectsForVideoEncoderSurfaceAndGetDataSpace(
        android_dataspace *dataSpace /* nonnull */) {
    DescribeColorAspectsParams params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexInput;
    ColorAspects &aspects = params.sAspects;

    // reset default format and store resulting format into both input and output formats
    getColorAspectsFromFormat(mConfigFormat, aspects);
    int32_t width, height;
    if (mInputFormat->findInt32("width", &width) && mInputFormat->findInt32("height", &height)) {
        setDefaultCodecColorAspectsIfNeeded(aspects, width, height);
    }
    setColorAspectsIntoFormat(aspects, mInputFormat);
    setColorAspectsIntoFormat(aspects, mOutputFormat);

    // communicate color aspects to codec, but do not allow any change
    ColorAspects origAspects = aspects;
    status_t err = OK;
    for (int triesLeft = 2; mDescribeColorAspectsIndex && --triesLeft >= 0; ) {
        status_t err = setCodecColorAspects(params, true /* readBack */);
        if (err != OK || !ColorUtils::checkIfAspectsChangedAndUnspecifyThem(aspects, origAspects)) {
            break;
        }
        ALOGW_IF(triesLeft == 0, "[%s] Codec repeatedly changed requested ColorAspects.",
                mComponentName.c_str());
    }

    *dataSpace = HAL_DATASPACE_BT709;
    aspects = origAspects; // restore desired color aspects
    status_t res = getDataSpace(
            params, dataSpace, err == OK && mDescribeColorAspectsIndex /* tryCodec */);
    if (err == OK) {
        err = res;
    }
    mInputFormat->setInt32("android._dataspace", (int32_t)*dataSpace);
    mInputFormat->setBuffer(
            "android._color-aspects", ABuffer::CreateAsCopy(&aspects, sizeof(aspects)));

    // update input format with codec supported color aspects (basically set unsupported
    // aspects to Unspecified)
    if (err == OK) {
        (void)getInputColorAspectsForVideoEncoder(mInputFormat);
    }

    ALOGV("set default color aspects, updated input format to %s, output format to %s",
            mInputFormat->debugString(4).c_str(), mOutputFormat->debugString(4).c_str());

    return err;
}

status_t ACodec::getHDRStaticInfoForVideoCodec(OMX_U32 portIndex, sp<AMessage> &format) {
    CHECK(portIndex == kPortIndexInput || portIndex == kPortIndexOutput);
    DescribeHDRStaticInfoParams params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;

    status_t err = getHDRStaticInfo(params);
    if (err == OK) {
        // we only set decodec output HDRStaticInfo if codec supports them
        setHDRStaticInfoIntoFormat(params.sInfo, format);
    }
    return err;
}

status_t ACodec::initDescribeHDRStaticInfoIndex() {
    status_t err = mOMXNode->getExtensionIndex(
            "OMX.google.android.index.describeHDRStaticInfo", &mDescribeHDRStaticInfoIndex);
    if (err != OK) {
        mDescribeHDRStaticInfoIndex = (OMX_INDEXTYPE)0;
        return err;
    }

    err = mOMXNode->getExtensionIndex(
                "OMX.google.android.index.describeHDR10PlusInfo", &mDescribeHDR10PlusInfoIndex);
    if (err != OK) {
        mDescribeHDR10PlusInfoIndex = (OMX_INDEXTYPE)0;
        return err;
    }

    return OK;
}

status_t ACodec::setHDRStaticInfo(const DescribeHDRStaticInfoParams &params) {
    status_t err = ERROR_UNSUPPORTED;
    if (mDescribeHDRStaticInfoIndex) {
        err = mOMXNode->setConfig(mDescribeHDRStaticInfoIndex, &params, sizeof(params));
    }

    const HDRStaticInfo *info = &params.sInfo;
    ALOGV("[%s] setting  HDRStaticInfo (R: %u %u, G: %u %u, B: %u, %u, W: %u, %u, "
            "MaxDispL: %u, MinDispL: %u, MaxContentL: %u, MaxFrameAvgL: %u)",
            mComponentName.c_str(),
            info->sType1.mR.x, info->sType1.mR.y, info->sType1.mG.x, info->sType1.mG.y,
            info->sType1.mB.x, info->sType1.mB.y, info->sType1.mW.x, info->sType1.mW.y,
            info->sType1.mMaxDisplayLuminance, info->sType1.mMinDisplayLuminance,
            info->sType1.mMaxContentLightLevel, info->sType1.mMaxFrameAverageLightLevel);

    ALOGW_IF(err == ERROR_UNSUPPORTED && mDescribeHDRStaticInfoIndex,
            "[%s] setting HDRStaticInfo failed even though codec advertises support",
            mComponentName.c_str());
    return err;
}

status_t ACodec::getHDRStaticInfo(DescribeHDRStaticInfoParams &params) {
    status_t err = ERROR_UNSUPPORTED;
    if (mDescribeHDRStaticInfoIndex) {
        err = mOMXNode->getConfig(mDescribeHDRStaticInfoIndex, &params, sizeof(params));
    }

    ALOGW_IF(err == ERROR_UNSUPPORTED && mDescribeHDRStaticInfoIndex,
            "[%s] getting HDRStaticInfo failed even though codec advertises support",
            mComponentName.c_str());
    return err;
}

status_t ACodec::setupVideoEncoder(
        const char *mime, const sp<AMessage> &msg,
        sp<AMessage> &outputFormat, sp<AMessage> &inputFormat) {
    int32_t tmp;
    if (!msg->findInt32("color-format", &tmp)) {
        return INVALID_OPERATION;
    }

    OMX_COLOR_FORMATTYPE colorFormat =
        static_cast<OMX_COLOR_FORMATTYPE>(tmp);

    status_t err = setVideoPortFormatType(
            kPortIndexInput, OMX_VIDEO_CodingUnused, colorFormat);

    if (err != OK) {
        ALOGE("[%s] does not support color format %d",
              mComponentName.c_str(), colorFormat);

        return err;
    }

    /* Input port configuration */

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);

    OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;

    def.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    OMX_VIDEO_CONTROLRATETYPE bitrateMode;
    int32_t width, height, bitrate = 0, quality;
    if (!msg->findInt32("width", &width)
            || !msg->findInt32("height", &height)
            || !findVideoBitrateControlInfo(
                    msg, &bitrateMode, &bitrate, &quality)) {
        return INVALID_OPERATION;
    }

    video_def->nFrameWidth = width;
    video_def->nFrameHeight = height;

    int32_t stride;
    if (!msg->findInt32("stride", &stride)) {
        stride = width;
    }

    video_def->nStride = stride;

    int32_t sliceHeight;
    if (!msg->findInt32("slice-height", &sliceHeight)) {
        sliceHeight = height;
    }

    video_def->nSliceHeight = sliceHeight;

    def.nBufferSize = (video_def->nStride * video_def->nSliceHeight * 3) / 2;

    float framerate;
    if (!msg->findFloat("frame-rate", &framerate)) {
        int32_t tmp;
        if (!msg->findInt32("frame-rate", &tmp)) {
            return INVALID_OPERATION;
        }
        mFps = (double)tmp;
    } else {
        mFps = (double)framerate;
    }
    // propagate framerate to the output so that the muxer has it
    outputFormat->setInt32("frame-rate", (int32_t)mFps);

    video_def->xFramerate = (OMX_U32)(mFps * 65536);
    video_def->eCompressionFormat = OMX_VIDEO_CodingUnused;
    // this is redundant as it was already set up in setVideoPortFormatType
    // FIXME for now skip this only for flexible YUV formats
    if (colorFormat != OMX_COLOR_FormatYUV420Flexible) {
        video_def->eColorFormat = colorFormat;
    }

    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        ALOGE("[%s] failed to set input port definition parameters.",
              mComponentName.c_str());

        return err;
    }

    /* Output port configuration */

    OMX_VIDEO_CODINGTYPE compressionFormat;
    err = GetVideoCodingTypeFromMime(mime, &compressionFormat);

    if (err != OK) {
        return err;
    }

    err = setVideoPortFormatType(
            kPortIndexOutput, compressionFormat, OMX_COLOR_FormatUnused);

    if (err != OK) {
        ALOGE("[%s] does not support compression format %d",
             mComponentName.c_str(), compressionFormat);

        return err;
    }

    def.nPortIndex = kPortIndexOutput;

    err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        return err;
    }

    video_def->nFrameWidth = width;
    video_def->nFrameHeight = height;
    video_def->xFramerate = 0;
    video_def->nBitrate = bitrate;
    video_def->eCompressionFormat = compressionFormat;
    video_def->eColorFormat = OMX_COLOR_FormatUnused;

    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if (err != OK) {
        ALOGE("[%s] failed to set output port definition parameters.",
              mComponentName.c_str());

        return err;
    }

    int32_t intraRefreshPeriod = 0;
    if (msg->findInt32("intra-refresh-period", &intraRefreshPeriod)
            && intraRefreshPeriod >= 0) {
        err = setIntraRefreshPeriod((uint32_t)intraRefreshPeriod, true);
        if (err != OK) {
            ALOGI("[%s] failed setIntraRefreshPeriod. Failure is fine since this key is optional",
                    mComponentName.c_str());
            err = OK;
        }
    }

    configureEncoderLatency(msg);

    switch (compressionFormat) {
        case OMX_VIDEO_CodingMPEG4:
            err = setupMPEG4EncoderParameters(msg);
            break;

        case OMX_VIDEO_CodingH263:
            err = setupH263EncoderParameters(msg);
            break;

        case OMX_VIDEO_CodingAVC:
            err = setupAVCEncoderParameters(msg);
            break;

        case OMX_VIDEO_CodingHEVC:
        case OMX_VIDEO_CodingImageHEIC:
            err = setupHEVCEncoderParameters(msg, outputFormat);
            break;

        case OMX_VIDEO_CodingVP8:
        case OMX_VIDEO_CodingVP9:
            err = setupVPXEncoderParameters(msg, outputFormat);
            break;

        default:
            break;
    }

    if (err != OK) {
        return err;
    }

    // Set up color aspects on input, but propagate them to the output format, as they will
    // not be read back from encoder.
    err = setColorAspectsForVideoEncoder(msg, outputFormat, inputFormat);
    if (err == ERROR_UNSUPPORTED) {
        ALOGI("[%s] cannot encode color aspects. Ignoring.", mComponentName.c_str());
        err = OK;
    }

    if (err != OK) {
        return err;
    }

    err = setHDRStaticInfoForVideoCodec(kPortIndexInput, msg, outputFormat);
    if (err == ERROR_UNSUPPORTED) { // support is optional
        ALOGI("[%s] cannot encode HDR static metadata. Ignoring.", mComponentName.c_str());
        err = OK;
    }

    if (err != OK) {
        return err;
    }

    switch (compressionFormat) {
        case OMX_VIDEO_CodingAVC:
        case OMX_VIDEO_CodingHEVC:
            err = configureTemporalLayers(msg, true /* inConfigure */, outputFormat);
            if (err != OK) {
                err = OK; // ignore failure
            }
            break;

        case OMX_VIDEO_CodingVP8:
        case OMX_VIDEO_CodingVP9:
            // TODO: do we need to support android.generic layering? webrtc layering is
            // already set up in setupVPXEncoderParameters.
            break;

        default:
            break;
    }

    if (err == OK) {
        ALOGI("setupVideoEncoder succeeded");
    }

    return err;
}

status_t ACodec::setCyclicIntraMacroblockRefresh(const sp<AMessage> &msg, int32_t mode) {
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE params;
    InitOMXParams(&params);
    params.nPortIndex = kPortIndexOutput;

    params.eRefreshMode = static_cast<OMX_VIDEO_INTRAREFRESHTYPE>(mode);

    if (params.eRefreshMode == OMX_VIDEO_IntraRefreshCyclic ||
            params.eRefreshMode == OMX_VIDEO_IntraRefreshBoth) {
        int32_t mbs;
        if (!msg->findInt32("intra-refresh-CIR-mbs", &mbs)) {
            return INVALID_OPERATION;
        }
        params.nCirMBs = mbs;
    }

    if (params.eRefreshMode == OMX_VIDEO_IntraRefreshAdaptive ||
            params.eRefreshMode == OMX_VIDEO_IntraRefreshBoth) {
        int32_t mbs;
        if (!msg->findInt32("intra-refresh-AIR-mbs", &mbs)) {
            return INVALID_OPERATION;
        }
        params.nAirMBs = mbs;

        int32_t ref;
        if (!msg->findInt32("intra-refresh-AIR-ref", &ref)) {
            return INVALID_OPERATION;
        }
        params.nAirRef = ref;
    }

    status_t err = mOMXNode->setParameter(
            OMX_IndexParamVideoIntraRefresh, &params, sizeof(params));
    return err;
}

static OMX_U32 setPFramesSpacing(
        float iFramesInterval /* seconds */, int32_t frameRate, uint32_t BFramesSpacing = 0) {
    // BFramesSpacing is the number of B frames between I/P frames
    // PFramesSpacing (the value to be returned) is the number of P frames between I frames
    //
    // keyFrameInterval = ((PFramesSpacing + 1) * BFramesSpacing) + PFramesSpacing + 1
    //                                     ^^^                            ^^^        ^^^
    //                              number of B frames                number of P    I frame
    //
    //                  = (PFramesSpacing + 1) * (BFramesSpacing + 1)
    //
    // E.g.
    //      I   P   I  : I-interval: 8, nPFrames 1, nBFrames 3
    //       BBB BBB

    if (iFramesInterval < 0) { // just 1 key frame
        return 0xFFFFFFFE; // don't use maxint as key-frame-interval calculation will add 1
    } else if (iFramesInterval == 0) { // just key frames
        return 0;
    }

    // round down as key-frame-interval is an upper limit
    uint32_t keyFrameInterval = uint32_t(frameRate * iFramesInterval);
    OMX_U32 ret = keyFrameInterval / (BFramesSpacing + 1);
    return ret > 0 ? ret - 1 : 0;
}

status_t ACodec::setupMPEG4EncoderParameters(const sp<AMessage> &msg) {
    int32_t bitrate;
    float iFrameInterval;
    if (!msg->findInt32("bitrate", &bitrate)
            || !msg->findAsFloat("i-frame-interval", &iFrameInterval)) {
        return INVALID_OPERATION;
    }

    OMX_VIDEO_CONTROLRATETYPE bitrateMode = getVideoBitrateMode(msg);

    float frameRate;
    if (!msg->findFloat("frame-rate", &frameRate)) {
        int32_t tmp;
        if (!msg->findInt32("frame-rate", &tmp)) {
            return INVALID_OPERATION;
        }
        frameRate = (float)tmp;
    }

    OMX_VIDEO_PARAM_MPEG4TYPE mpeg4type;
    InitOMXParams(&mpeg4type);
    mpeg4type.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamVideoMpeg4, &mpeg4type, sizeof(mpeg4type));

    if (err != OK) {
        return err;
    }

    mpeg4type.nSliceHeaderSpacing = 0;
    mpeg4type.bSVH = OMX_FALSE;
    mpeg4type.bGov = OMX_FALSE;

    mpeg4type.nAllowedPictureTypes =
        OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;

    mpeg4type.nBFrames = 0;
    mpeg4type.nPFrames = setPFramesSpacing(iFrameInterval, frameRate, mpeg4type.nBFrames);
    if (mpeg4type.nPFrames == 0) {
        mpeg4type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI;
    }
    mpeg4type.nIDCVLCThreshold = 0;
    mpeg4type.bACPred = OMX_TRUE;
    mpeg4type.nMaxPacketSize = 256;
    mpeg4type.nTimeIncRes = 1000;
    mpeg4type.nHeaderExtension = 0;
    mpeg4type.bReversibleVLC = OMX_FALSE;

    int32_t profile;
    if (msg->findInt32("profile", &profile)) {
        int32_t level;
        if (!msg->findInt32("level", &level)) {
            return INVALID_OPERATION;
        }

        err = verifySupportForProfileAndLevel(kPortIndexOutput, profile, level);

        if (err != OK) {
            return err;
        }

        mpeg4type.eProfile = static_cast<OMX_VIDEO_MPEG4PROFILETYPE>(profile);
        mpeg4type.eLevel = static_cast<OMX_VIDEO_MPEG4LEVELTYPE>(level);
    }

    err = mOMXNode->setParameter(
            OMX_IndexParamVideoMpeg4, &mpeg4type, sizeof(mpeg4type));

    if (err != OK) {
        return err;
    }

    err = configureBitrate(bitrateMode, bitrate);

    if (err != OK) {
        return err;
    }

    return setupErrorCorrectionParameters();
}

status_t ACodec::setupH263EncoderParameters(const sp<AMessage> &msg) {
    int32_t bitrate;
    float iFrameInterval;
    if (!msg->findInt32("bitrate", &bitrate)
            || !msg->findAsFloat("i-frame-interval", &iFrameInterval)) {
        return INVALID_OPERATION;
    }

    OMX_VIDEO_CONTROLRATETYPE bitrateMode = getVideoBitrateMode(msg);

    float frameRate;
    if (!msg->findFloat("frame-rate", &frameRate)) {
        int32_t tmp;
        if (!msg->findInt32("frame-rate", &tmp)) {
            return INVALID_OPERATION;
        }
        frameRate = (float)tmp;
    }

    OMX_VIDEO_PARAM_H263TYPE h263type;
    InitOMXParams(&h263type);
    h263type.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamVideoH263, &h263type, sizeof(h263type));

    if (err != OK) {
        return err;
    }

    h263type.nAllowedPictureTypes =
        OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;

    h263type.nBFrames = 0;
    h263type.nPFrames = setPFramesSpacing(iFrameInterval, frameRate, h263type.nBFrames);
    if (h263type.nPFrames == 0) {
        h263type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI;
    }

    int32_t profile;
    if (msg->findInt32("profile", &profile)) {
        int32_t level;
        if (!msg->findInt32("level", &level)) {
            return INVALID_OPERATION;
        }

        err = verifySupportForProfileAndLevel(kPortIndexOutput, profile, level);

        if (err != OK) {
            return err;
        }

        h263type.eProfile = static_cast<OMX_VIDEO_H263PROFILETYPE>(profile);
        h263type.eLevel = static_cast<OMX_VIDEO_H263LEVELTYPE>(level);
    }

    h263type.bPLUSPTYPEAllowed = OMX_FALSE;
    h263type.bForceRoundingTypeToZero = OMX_FALSE;
    h263type.nPictureHeaderRepetition = 0;
    h263type.nGOBHeaderInterval = 0;

    err = mOMXNode->setParameter(
            OMX_IndexParamVideoH263, &h263type, sizeof(h263type));

    if (err != OK) {
        return err;
    }

    err = configureBitrate(bitrateMode, bitrate);

    if (err != OK) {
        return err;
    }

    return setupErrorCorrectionParameters();
}

// static
int /* OMX_VIDEO_AVCLEVELTYPE */ ACodec::getAVCLevelFor(
        int width, int height, int rate, int bitrate,
        OMX_VIDEO_AVCPROFILEEXTTYPE profile) {
    // convert bitrate to main/baseline profile kbps equivalent
    switch ((uint32_t)profile) {
        case OMX_VIDEO_AVCProfileHigh10:
            bitrate = divUp(bitrate, 3000); break;
        case OMX_VIDEO_AVCProfileConstrainedHigh:
        case OMX_VIDEO_AVCProfileHigh:
            bitrate = divUp(bitrate, 1250); break;
        default:
            bitrate = divUp(bitrate, 1000); break;
    }

    // convert size and rate to MBs
    width = divUp(width, 16);
    height = divUp(height, 16);
    int mbs = width * height;
    rate *= mbs;
    int maxDimension = max(width, height);

    static const int limits[][5] = {
        /*    MBps      MB   dim  bitrate        level */
        {     1485,     99,   28,     64, OMX_VIDEO_AVCLevel1  },
        {     1485,     99,   28,    128, OMX_VIDEO_AVCLevel1b },
        {     3000,    396,   56,    192, OMX_VIDEO_AVCLevel11 },
        {     6000,    396,   56,    384, OMX_VIDEO_AVCLevel12 },
        {    11880,    396,   56,    768, OMX_VIDEO_AVCLevel13 },
        {    11880,    396,   56,   2000, OMX_VIDEO_AVCLevel2  },
        {    19800,    792,   79,   4000, OMX_VIDEO_AVCLevel21 },
        {    20250,   1620,  113,   4000, OMX_VIDEO_AVCLevel22 },
        {    40500,   1620,  113,  10000, OMX_VIDEO_AVCLevel3  },
        {   108000,   3600,  169,  14000, OMX_VIDEO_AVCLevel31 },
        {   216000,   5120,  202,  20000, OMX_VIDEO_AVCLevel32 },
        {   245760,   8192,  256,  20000, OMX_VIDEO_AVCLevel4  },
        {   245760,   8192,  256,  50000, OMX_VIDEO_AVCLevel41 },
        {   522240,   8704,  263,  50000, OMX_VIDEO_AVCLevel42 },
        {   589824,  22080,  420, 135000, OMX_VIDEO_AVCLevel5  },
        {   983040,  36864,  543, 240000, OMX_VIDEO_AVCLevel51 },
        {  2073600,  36864,  543, 240000, OMX_VIDEO_AVCLevel52 },
        {  4177920, 139264, 1055, 240000, OMX_VIDEO_AVCLevel6  },
        {  8355840, 139264, 1055, 480000, OMX_VIDEO_AVCLevel61 },
        { 16711680, 139264, 1055, 800000, OMX_VIDEO_AVCLevel62 },
    };

    for (size_t i = 0; i < ARRAY_SIZE(limits); i++) {
        const int (&limit)[5] = limits[i];
        if (rate <= limit[0] && mbs <= limit[1] && maxDimension <= limit[2]
                && bitrate <= limit[3]) {
            return limit[4];
        }
    }
    return 0;
}

status_t ACodec::setupAVCEncoderParameters(const sp<AMessage> &msg) {
    int32_t bitrate;
    float iFrameInterval;
    if (!msg->findInt32("bitrate", &bitrate)
            || !msg->findAsFloat("i-frame-interval", &iFrameInterval)) {
        return INVALID_OPERATION;
    }

    OMX_VIDEO_CONTROLRATETYPE bitrateMode = getVideoBitrateMode(msg);

    float frameRate;
    if (!msg->findFloat("frame-rate", &frameRate)) {
        int32_t tmp;
        if (!msg->findInt32("frame-rate", &tmp)) {
            return INVALID_OPERATION;
        }
        frameRate = (float)tmp;
    }

    status_t err = OK;
    int32_t intraRefreshMode = 0;
    if (msg->findInt32("intra-refresh-mode", &intraRefreshMode)) {
        err = setCyclicIntraMacroblockRefresh(msg, intraRefreshMode);
        if (err != OK) {
            ALOGE("Setting intra macroblock refresh mode (%d) failed: 0x%x",
                    err, intraRefreshMode);
            return err;
        }
    }

    OMX_VIDEO_PARAM_AVCTYPE h264type;
    InitOMXParams(&h264type);
    h264type.nPortIndex = kPortIndexOutput;

    err = mOMXNode->getParameter(
            OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));

    if (err != OK) {
        return err;
    }

    h264type.nAllowedPictureTypes =
        OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;

    int32_t profile;
    if (msg->findInt32("profile", &profile)) {
        int32_t level;
        if (!msg->findInt32("level", &level)) {
            return INVALID_OPERATION;
        }

        err = verifySupportForProfileAndLevel(kPortIndexOutput, profile, level);

        if (err != OK) {
            return err;
        }

        h264type.eProfile = static_cast<OMX_VIDEO_AVCPROFILETYPE>(profile);
        h264type.eLevel = static_cast<OMX_VIDEO_AVCLEVELTYPE>(level);
    } else {
        h264type.eProfile = OMX_VIDEO_AVCProfileBaseline;
#if 0   /* DON'T YET DEFAULT TO HIGHEST PROFILE */
        // Use largest supported profile for AVC recording if profile is not specified.
        for (OMX_VIDEO_AVCPROFILETYPE profile : {
                OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCProfileMain }) {
            if (verifySupportForProfileAndLevel(kPortIndexOutput, profile, 0) == OK) {
                h264type.eProfile = profile;
                break;
            }
        }
#endif
    }

    ALOGI("setupAVCEncoderParameters with [profile: %s] [level: %s]",
            asString(h264type.eProfile), asString(h264type.eLevel));

    if (h264type.eProfile == OMX_VIDEO_AVCProfileBaseline) {
        h264type.nSliceHeaderSpacing = 0;
        h264type.bUseHadamard = OMX_TRUE;
        h264type.nRefFrames = 1;
        h264type.nBFrames = 0;
        h264type.nPFrames = setPFramesSpacing(iFrameInterval, frameRate, h264type.nBFrames);
        if (h264type.nPFrames == 0) {
            h264type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI;
        }
        h264type.nRefIdx10ActiveMinus1 = 0;
        h264type.nRefIdx11ActiveMinus1 = 0;
        h264type.bEntropyCodingCABAC = OMX_FALSE;
        h264type.bWeightedPPrediction = OMX_FALSE;
        h264type.bconstIpred = OMX_FALSE;
        h264type.bDirect8x8Inference = OMX_FALSE;
        h264type.bDirectSpatialTemporal = OMX_FALSE;
        h264type.nCabacInitIdc = 0;
    } else if (h264type.eProfile == OMX_VIDEO_AVCProfileMain ||
            h264type.eProfile == OMX_VIDEO_AVCProfileHigh) {
        h264type.nSliceHeaderSpacing = 0;
        h264type.bUseHadamard = OMX_TRUE;
        int32_t maxBframes = 0;
        (void)msg->findInt32(KEY_MAX_B_FRAMES, &maxBframes);
        h264type.nBFrames = uint32_t(maxBframes);
        if (mLatency && h264type.nBFrames > *mLatency) {
            h264type.nBFrames = *mLatency;
        }
        h264type.nRefFrames = h264type.nBFrames == 0 ? 1 : 2;

        h264type.nPFrames = setPFramesSpacing(iFrameInterval, frameRate, h264type.nBFrames);
        h264type.nAllowedPictureTypes =
            OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
        h264type.nRefIdx10ActiveMinus1 = 0;
        h264type.nRefIdx11ActiveMinus1 = 0;
        h264type.bEntropyCodingCABAC = OMX_TRUE;
        h264type.bWeightedPPrediction = OMX_TRUE;
        h264type.bconstIpred = OMX_TRUE;
        h264type.bDirect8x8Inference = OMX_TRUE;
        h264type.bDirectSpatialTemporal = OMX_TRUE;
        h264type.nCabacInitIdc = 1;
    }

    if (h264type.nBFrames != 0) {
        h264type.nAllowedPictureTypes |= OMX_VIDEO_PictureTypeB;
    }

    h264type.bEnableUEP = OMX_FALSE;
    h264type.bEnableFMO = OMX_FALSE;
    h264type.bEnableASO = OMX_FALSE;
    h264type.bEnableRS = OMX_FALSE;
    h264type.bFrameMBsOnly = OMX_TRUE;
    h264type.bMBAFF = OMX_FALSE;
    h264type.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;

    err = mOMXNode->setParameter(
            OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));

    if (err != OK) {
        return err;
    }

    // TRICKY: if we are enabling temporal layering as well, some codecs may not support layering
    // when B-frames are enabled. Detect this now so we can disable B frames if temporal layering
    // is preferred.
    AString tsSchema;
    int32_t preferBFrames = (int32_t)false;
    if (msg->findString("ts-schema", &tsSchema)
            && (!msg->findInt32("android._prefer-b-frames", &preferBFrames) || !preferBFrames)) {
        OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE layering;
        InitOMXParams(&layering);
        layering.nPortIndex = kPortIndexOutput;
        if (mOMXNode->getParameter(
                        (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
                        &layering, sizeof(layering)) == OK
                && layering.eSupportedPatterns
                && layering.nBLayerCountMax == 0) {
            h264type.nBFrames = 0;
            h264type.nPFrames = setPFramesSpacing(iFrameInterval, frameRate, h264type.nBFrames);
            h264type.nAllowedPictureTypes &= ~OMX_VIDEO_PictureTypeB;
            ALOGI("disabling B-frames");
            err = mOMXNode->setParameter(
                    OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));

            if (err != OK) {
                return err;
            }
        }
    }

    return configureBitrate(bitrateMode, bitrate);
}

status_t ACodec::configureImageGrid(
        const sp<AMessage> &msg, sp<AMessage> &outputFormat) {
    int32_t tileWidth, tileHeight, gridRows, gridCols;
    OMX_BOOL useGrid = OMX_FALSE;
    if (msg->findInt32("tile-width", &tileWidth) &&
        msg->findInt32("tile-height", &tileHeight) &&
        msg->findInt32("grid-rows", &gridRows) &&
        msg->findInt32("grid-cols", &gridCols)) {
        useGrid = OMX_TRUE;
    } else {
        // when bEnabled is false, the tile info is not used,
        // but clear out these too.
        tileWidth = tileHeight = gridRows = gridCols = 0;
    }

    if (!mIsImage && !useGrid) {
        return OK;
    }

    OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE gridType;
    InitOMXParams(&gridType);
    gridType.nPortIndex = kPortIndexOutput;
    gridType.bEnabled = useGrid;
    gridType.nTileWidth = tileWidth;
    gridType.nTileHeight = tileHeight;
    gridType.nGridRows = gridRows;
    gridType.nGridCols = gridCols;

    ALOGV("sending image grid info to component: bEnabled %d, tile %dx%d, grid %dx%d",
            gridType.bEnabled,
            gridType.nTileWidth,
            gridType.nTileHeight,
            gridType.nGridRows,
            gridType.nGridCols);

    status_t err = mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamVideoAndroidImageGrid,
            &gridType, sizeof(gridType));

    // for video encoders, grid config is only a hint.
    if (!mIsImage) {
        return OK;
    }

    // image encoders must support grid config.
    if (err != OK) {
        return err;
    }

    // query to get the image encoder's real grid config as it might be
    // different from the requested, and transfer that to the output.
    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamVideoAndroidImageGrid,
            &gridType, sizeof(gridType));

    ALOGV("received image grid info from component: bEnabled %d, tile %dx%d, grid %dx%d",
            gridType.bEnabled,
            gridType.nTileWidth,
            gridType.nTileHeight,
            gridType.nGridRows,
            gridType.nGridCols);

    if (err == OK && gridType.bEnabled) {
        outputFormat->setInt32("tile-width", gridType.nTileWidth);
        outputFormat->setInt32("tile-height", gridType.nTileHeight);
        outputFormat->setInt32("grid-rows", gridType.nGridRows);
        outputFormat->setInt32("grid-cols", gridType.nGridCols);
    }

    return err;
}

status_t ACodec::setupHEVCEncoderParameters(
        const sp<AMessage> &msg, sp<AMessage> &outputFormat) {
    OMX_VIDEO_CONTROLRATETYPE bitrateMode;
    int32_t bitrate, quality;
    if (!findVideoBitrateControlInfo(msg, &bitrateMode, &bitrate, &quality)) {
        return INVALID_OPERATION;
    }

    OMX_VIDEO_PARAM_HEVCTYPE hevcType;
    InitOMXParams(&hevcType);
    hevcType.nPortIndex = kPortIndexOutput;

    status_t err = OK;
    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamVideoHevc, &hevcType, sizeof(hevcType));
    if (err != OK) {
        return err;
    }

    int32_t profile;
    if (msg->findInt32("profile", &profile)) {
        int32_t level;
        if (!msg->findInt32("level", &level)) {
            return INVALID_OPERATION;
        }

        err = verifySupportForProfileAndLevel(kPortIndexOutput, profile, level);
        if (err != OK) {
            return err;
        }

        hevcType.eProfile = static_cast<OMX_VIDEO_HEVCPROFILETYPE>(profile);
        hevcType.eLevel = static_cast<OMX_VIDEO_HEVCLEVELTYPE>(level);
    }
    // TODO: finer control?
    if (mIsImage) {
        hevcType.nKeyFrameInterval = 1;
    } else {
        float iFrameInterval;
        if (!msg->findAsFloat("i-frame-interval", &iFrameInterval)) {
            return INVALID_OPERATION;
        }

        float frameRate;
        if (!msg->findFloat("frame-rate", &frameRate)) {
            int32_t tmp;
            if (!msg->findInt32("frame-rate", &tmp)) {
                return INVALID_OPERATION;
            }
            frameRate = (float)tmp;
        }

        hevcType.nKeyFrameInterval =
                setPFramesSpacing(iFrameInterval, frameRate) + 1;
    }


    err = mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamVideoHevc, &hevcType, sizeof(hevcType));
    if (err != OK) {
        return err;
    }

    err = configureImageGrid(msg, outputFormat);

    if (err != OK) {
        return err;
    }

    return configureBitrate(bitrateMode, bitrate, quality);
}

status_t ACodec::setupVPXEncoderParameters(const sp<AMessage> &msg, sp<AMessage> &outputFormat) {
    int32_t bitrate;
    float iFrameInterval = 0;
    size_t tsLayers = 0;
    OMX_VIDEO_ANDROID_VPXTEMPORALLAYERPATTERNTYPE pattern =
        OMX_VIDEO_VPXTemporalLayerPatternNone;
    static const uint32_t kVp8LayerRateAlloction
        [OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS]
        [OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS] = {
        {100, 100, 100},  // 1 layer
        { 60, 100, 100},  // 2 layers {60%, 40%}
        { 40,  60, 100},  // 3 layers {40%, 20%, 40%}
    };
    if (!msg->findInt32("bitrate", &bitrate)) {
        return INVALID_OPERATION;
    }
    msg->findAsFloat("i-frame-interval", &iFrameInterval);

    OMX_VIDEO_CONTROLRATETYPE bitrateMode = getVideoBitrateMode(msg);

    float frameRate;
    if (!msg->findFloat("frame-rate", &frameRate)) {
        int32_t tmp;
        if (!msg->findInt32("frame-rate", &tmp)) {
            return INVALID_OPERATION;
        }
        frameRate = (float)tmp;
    }

    AString tsSchema;
    OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE tsType =
        OMX_VIDEO_AndroidTemporalLayeringPatternNone;

    if (msg->findString("ts-schema", &tsSchema)) {
        unsigned int numLayers = 0;
        unsigned int numBLayers = 0;
        int tags;
        char dummy;
        if (sscanf(tsSchema.c_str(), "webrtc.vp8.%u-layer%c", &numLayers, &dummy) == 1
                && numLayers > 0) {
            pattern = OMX_VIDEO_VPXTemporalLayerPatternWebRTC;
            tsType = OMX_VIDEO_AndroidTemporalLayeringPatternWebRTC;
            tsLayers = numLayers;
        } else if ((tags = sscanf(tsSchema.c_str(), "android.generic.%u%c%u%c",
                        &numLayers, &dummy, &numBLayers, &dummy))
                && (tags == 1 || (tags == 3 && dummy == '+'))
                && numLayers > 0 && numLayers < UINT32_MAX - numBLayers) {
            pattern = OMX_VIDEO_VPXTemporalLayerPatternWebRTC;
            // VPX does not have a concept of B-frames, so just count all layers
            tsType = OMX_VIDEO_AndroidTemporalLayeringPatternAndroid;
            tsLayers = numLayers + numBLayers;
        } else {
            ALOGW("Ignoring unsupported ts-schema [%s]", tsSchema.c_str());
        }
        tsLayers = min(tsLayers, (size_t)OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS);
    }

    OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE vp8type;
    InitOMXParams(&vp8type);
    vp8type.nPortIndex = kPortIndexOutput;
    status_t err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamVideoAndroidVp8Encoder,
            &vp8type, sizeof(vp8type));

    if (err == OK) {
        if (iFrameInterval > 0) {
            vp8type.nKeyFrameInterval = setPFramesSpacing(iFrameInterval, frameRate) + 1;
        }
        vp8type.eTemporalPattern = pattern;
        vp8type.nTemporalLayerCount = tsLayers;
        if (tsLayers > 0) {
            for (size_t i = 0; i < OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS; i++) {
                vp8type.nTemporalLayerBitrateRatio[i] =
                    kVp8LayerRateAlloction[tsLayers - 1][i];
            }
        }
        if (bitrateMode == OMX_Video_ControlRateConstant) {
            vp8type.nMinQuantizer = 2;
            vp8type.nMaxQuantizer = 63;
        }

        err = mOMXNode->setParameter(
                (OMX_INDEXTYPE)OMX_IndexParamVideoAndroidVp8Encoder,
                &vp8type, sizeof(vp8type));
        if (err != OK) {
            ALOGW("Extended VP8 parameters set failed: %d", err);
        } else if (tsType == OMX_VIDEO_AndroidTemporalLayeringPatternWebRTC) {
            // advertise even single layer WebRTC layering, as it is defined
            outputFormat->setString("ts-schema", AStringPrintf("webrtc.vp8.%u-layer", tsLayers));
        } else if (tsLayers > 0) {
            // tsType == OMX_VIDEO_AndroidTemporalLayeringPatternAndroid
            outputFormat->setString("ts-schema", AStringPrintf("android.generic.%u", tsLayers));
        }
    }

    return configureBitrate(bitrateMode, bitrate);
}

status_t ACodec::verifySupportForProfileAndLevel(
        OMX_U32 portIndex, int32_t profile, int32_t level) {
    OMX_VIDEO_PARAM_PROFILELEVELTYPE params;
    InitOMXParams(&params);
    params.nPortIndex = portIndex;

    for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
        params.nProfileIndex = index;
        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoProfileLevelQuerySupported,
                &params, sizeof(params));

        if (err != OK) {
            return err;
        }

        int32_t supportedProfile = static_cast<int32_t>(params.eProfile);
        int32_t supportedLevel = static_cast<int32_t>(params.eLevel);

        if (profile == supportedProfile && level <= supportedLevel) {
            return OK;
        }

        if (index == kMaxIndicesToCheck) {
            ALOGW("[%s] stopping checking profiles after %u: %x/%x",
                    mComponentName.c_str(), index,
                    params.eProfile, params.eLevel);
        }
    }
    return ERROR_UNSUPPORTED;
}

status_t ACodec::configureBitrate(
        OMX_VIDEO_CONTROLRATETYPE bitrateMode, int32_t bitrate, int32_t quality) {
    OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
    InitOMXParams(&bitrateType);
    bitrateType.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));

    if (err != OK) {
        return err;
    }

    bitrateType.eControlRate = bitrateMode;

    // write it out explicitly even if it's a union
    if (bitrateMode == OMX_Video_ControlRateConstantQuality) {
        bitrateType.nQualityFactor = quality;
    } else {
        bitrateType.nTargetBitrate = bitrate;
    }

    return mOMXNode->setParameter(
            OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));
}

void ACodec::configureEncoderLatency(const sp<AMessage> &msg) {
    if (!mIsEncoder || !mIsVideo) {
        return;
    }

    int32_t latency = 0, bitrateMode;
    if (msg->findInt32("latency", &latency) && latency > 0) {
        status_t err = setLatency(latency);
        if (err != OK) {
            ALOGW("[%s] failed setLatency. Failure is fine since this key is optional",
                    mComponentName.c_str());
            err = OK;
        } else {
            mLatency = latency;
        }
    } else if ((!msg->findInt32("bitrate-mode", &bitrateMode) &&
            bitrateMode == OMX_Video_ControlRateConstant)) {
        // default the latency to be 1 if latency key is not specified or unsupported and bitrateMode
        // is CBR.
        mLatency = 1;
    }
}

status_t ACodec::setupErrorCorrectionParameters() {
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE errorCorrectionType;
    InitOMXParams(&errorCorrectionType);
    errorCorrectionType.nPortIndex = kPortIndexOutput;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamVideoErrorCorrection,
            &errorCorrectionType, sizeof(errorCorrectionType));

    if (err != OK) {
        return OK;  // Optional feature. Ignore this failure
    }

    errorCorrectionType.bEnableHEC = OMX_FALSE;
    errorCorrectionType.bEnableResync = OMX_TRUE;
    errorCorrectionType.nResynchMarkerSpacing = 256;
    errorCorrectionType.bEnableDataPartitioning = OMX_FALSE;
    errorCorrectionType.bEnableRVLC = OMX_FALSE;

    return mOMXNode->setParameter(
            OMX_IndexParamVideoErrorCorrection,
            &errorCorrectionType, sizeof(errorCorrectionType));
}

status_t ACodec::setVideoFormatOnPort(
        OMX_U32 portIndex,
        int32_t width, int32_t height, OMX_VIDEO_CODINGTYPE compressionFormat,
        float frameRate) {
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;

    OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;

    status_t err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        return err;
    }

    if (portIndex == kPortIndexInput) {
        // XXX Need a (much) better heuristic to compute input buffer sizes.
        const size_t X = 64 * 1024;
        if (def.nBufferSize < X) {
            def.nBufferSize = X;
        }
    }

    if (def.eDomain != OMX_PortDomainVideo) {
        ALOGE("expected video port, got %s(%d)", asString(def.eDomain), def.eDomain);
        return FAILED_TRANSACTION;
    }

    video_def->nFrameWidth = width;
    video_def->nFrameHeight = height;

    if (portIndex == kPortIndexInput) {
        video_def->eCompressionFormat = compressionFormat;
        video_def->eColorFormat = OMX_COLOR_FormatUnused;
        if (frameRate >= 0) {
            video_def->xFramerate = (OMX_U32)(frameRate * 65536.0f);
        }
    }

    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    return err;
}

size_t ACodec::countBuffersOwnedByComponent(OMX_U32 portIndex) const {
    size_t n = 0;

    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        const BufferInfo &info = mBuffers[portIndex].itemAt(i);

        if (info.mStatus == BufferInfo::OWNED_BY_COMPONENT) {
            ++n;
        }
    }

    return n;
}

size_t ACodec::countBuffersOwnedByNativeWindow() const {
    size_t n = 0;

    for (size_t i = 0; i < mBuffers[kPortIndexOutput].size(); ++i) {
        const BufferInfo &info = mBuffers[kPortIndexOutput].itemAt(i);

        if (info.mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
            ++n;
        }
    }

    return n;
}

void ACodec::waitUntilAllPossibleNativeWindowBuffersAreReturnedToUs() {
    if (mNativeWindow == NULL) {
        return;
    }

    while (countBuffersOwnedByNativeWindow() > mNumUndequeuedBuffers
            && dequeueBufferFromNativeWindow() != NULL) {
        // these buffers will be submitted as regular buffers; account for this
        if (storingMetadataInDecodedBuffers() && mMetadataBuffersToSubmit > 0) {
            --mMetadataBuffersToSubmit;
        }
    }
}

bool ACodec::allYourBuffersAreBelongToUs(
        OMX_U32 portIndex) {
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        BufferInfo *info = &mBuffers[portIndex].editItemAt(i);

        if (info->mStatus != BufferInfo::OWNED_BY_US
                && info->mStatus != BufferInfo::OWNED_BY_NATIVE_WINDOW) {
            ALOGV("[%s] Buffer %u on port %u still has status %d",
                    mComponentName.c_str(),
                    info->mBufferID, portIndex, info->mStatus);
            return false;
        }
    }

    return true;
}

bool ACodec::allYourBuffersAreBelongToUs() {
    return allYourBuffersAreBelongToUs(kPortIndexInput)
        && allYourBuffersAreBelongToUs(kPortIndexOutput);
}

void ACodec::deferMessage(const sp<AMessage> &msg) {
    mDeferredQueue.push_back(msg);
}

void ACodec::processDeferredMessages() {
    List<sp<AMessage> > queue = mDeferredQueue;
    mDeferredQueue.clear();

    List<sp<AMessage> >::iterator it = queue.begin();
    while (it != queue.end()) {
        onMessageReceived(*it++);
    }
}

status_t ACodec::getPortFormat(OMX_U32 portIndex, sp<AMessage> &notify) {
    const char *niceIndex = portIndex == kPortIndexInput ? "input" : "output";
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;

    status_t err = mOMXNode->getParameter(OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        return err;
    }

    if (def.eDir != (portIndex == kPortIndexOutput ? OMX_DirOutput : OMX_DirInput)) {
        ALOGE("unexpected dir: %s(%d) on %s port", asString(def.eDir), def.eDir, niceIndex);
        return BAD_VALUE;
    }

    switch (def.eDomain) {
        case OMX_PortDomainVideo:
        {
            OMX_VIDEO_PORTDEFINITIONTYPE *videoDef = &def.format.video;
            switch ((int)videoDef->eCompressionFormat) {
                case OMX_VIDEO_CodingUnused:
                {
                    CHECK(mIsEncoder ^ (portIndex == kPortIndexOutput));
                    notify->setString("mime", MEDIA_MIMETYPE_VIDEO_RAW);

                    notify->setInt32("stride", videoDef->nStride);
                    notify->setInt32("slice-height", videoDef->nSliceHeight);
                    notify->setInt32("color-format", videoDef->eColorFormat);

                    if (mNativeWindow == NULL) {
                        DescribeColorFormat2Params describeParams;
                        InitOMXParams(&describeParams);
                        describeParams.eColorFormat = videoDef->eColorFormat;
                        describeParams.nFrameWidth = videoDef->nFrameWidth;
                        describeParams.nFrameHeight = videoDef->nFrameHeight;
                        describeParams.nStride = videoDef->nStride;
                        describeParams.nSliceHeight = videoDef->nSliceHeight;
                        describeParams.bUsingNativeBuffers = OMX_FALSE;

                        if (DescribeColorFormat(mOMXNode, describeParams)) {
                            notify->setBuffer(
                                    "image-data",
                                    ABuffer::CreateAsCopy(
                                            &describeParams.sMediaImage,
                                            sizeof(describeParams.sMediaImage)));

                            MediaImage2 &img = describeParams.sMediaImage;
                            MediaImage2::PlaneInfo *plane = img.mPlane;
                            ALOGV("[%s] MediaImage { F(%ux%u) @%u+%d+%d @%u+%d+%d @%u+%d+%d }",
                                    mComponentName.c_str(), img.mWidth, img.mHeight,
                                    plane[0].mOffset, plane[0].mColInc, plane[0].mRowInc,
                                    plane[1].mOffset, plane[1].mColInc, plane[1].mRowInc,
                                    plane[2].mOffset, plane[2].mColInc, plane[2].mRowInc);
                        }
                    }

                    int32_t width = (int32_t)videoDef->nFrameWidth;
                    int32_t height = (int32_t)videoDef->nFrameHeight;

                    if (portIndex == kPortIndexOutput) {
                        OMX_CONFIG_RECTTYPE rect;
                        InitOMXParams(&rect);
                        rect.nPortIndex = portIndex;

                        if (mOMXNode->getConfig(
                                    (portIndex == kPortIndexOutput ?
                                            OMX_IndexConfigCommonOutputCrop :
                                            OMX_IndexConfigCommonInputCrop),
                                    &rect, sizeof(rect)) != OK) {
                            rect.nLeft = 0;
                            rect.nTop = 0;
                            rect.nWidth = videoDef->nFrameWidth;
                            rect.nHeight = videoDef->nFrameHeight;
                        }

                        if (rect.nLeft < 0 || rect.nTop < 0 ||
                            rect.nWidth == 0 || rect.nHeight == 0 ||
                            rect.nLeft + rect.nWidth > videoDef->nFrameWidth ||
                            rect.nTop + rect.nHeight > videoDef->nFrameHeight) {
                            ALOGE("Wrong cropped rect (%d, %d, %u, %u) vs. frame (%u, %u)",
                                    rect.nLeft, rect.nTop,
                                    rect.nWidth, rect.nHeight,
                                    videoDef->nFrameWidth, videoDef->nFrameHeight);
                            return BAD_VALUE;
                        }

                        notify->setRect(
                                "crop",
                                rect.nLeft,
                                rect.nTop,
                                rect.nLeft + rect.nWidth - 1,
                                rect.nTop + rect.nHeight - 1);

                        width = rect.nWidth;
                        height = rect.nHeight;

                        android_dataspace dataSpace = HAL_DATASPACE_UNKNOWN;
                        (void)getColorAspectsAndDataSpaceForVideoDecoder(
                                width, height, mConfigFormat, notify,
                                mUsingNativeWindow ? &dataSpace : NULL);
                        if (mUsingNativeWindow) {
                            notify->setInt32("android._dataspace", dataSpace);
                        }
                        (void)getHDRStaticInfoForVideoCodec(kPortIndexOutput, notify);
                    } else {
                        (void)getInputColorAspectsForVideoEncoder(notify);
                        if (mConfigFormat->contains("hdr-static-info")) {
                            (void)getHDRStaticInfoForVideoCodec(kPortIndexInput, notify);
                        }
                        uint32_t latency = 0;
                        if (mIsEncoder && !mIsImage &&
                                getLatency(&latency) == OK && latency > 0) {
                            notify->setInt32("latency", latency);
                        }
                    }

                    break;
                }

                case OMX_VIDEO_CodingVP8:
                case OMX_VIDEO_CodingVP9:
                {
                    OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE vp8type;
                    InitOMXParams(&vp8type);
                    vp8type.nPortIndex = kPortIndexOutput;
                    status_t err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamVideoAndroidVp8Encoder,
                            &vp8type,
                            sizeof(vp8type));

                    if (err == OK) {
                        if (vp8type.eTemporalPattern == OMX_VIDEO_VPXTemporalLayerPatternWebRTC
                                && vp8type.nTemporalLayerCount > 0
                                && vp8type.nTemporalLayerCount
                                        <= OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS) {
                            // advertise as android.generic if we configured for android.generic
                            AString origSchema;
                            if (notify->findString("ts-schema", &origSchema)
                                    && origSchema.startsWith("android.generic")) {
                                notify->setString("ts-schema", AStringPrintf(
                                        "android.generic.%u", vp8type.nTemporalLayerCount));
                            } else {
                                notify->setString("ts-schema", AStringPrintf(
                                        "webrtc.vp8.%u-layer", vp8type.nTemporalLayerCount));
                            }
                        }
                    }
                    // Fall through to set up mime.
                    FALLTHROUGH_INTENDED;
                }

                default:
                {
                    if (mIsEncoder ^ (portIndex == kPortIndexOutput)) {
                        // should be CodingUnused
                        ALOGE("Raw port video compression format is %s(%d)",
                                asString(videoDef->eCompressionFormat),
                                videoDef->eCompressionFormat);
                        return BAD_VALUE;
                    }
                    AString mime;
                    if (GetMimeTypeForVideoCoding(
                        videoDef->eCompressionFormat, &mime) != OK) {
                        notify->setString("mime", "application/octet-stream");
                    } else {
                        notify->setString("mime", mime.c_str());
                    }
                    uint32_t intraRefreshPeriod = 0;
                    if (mIsEncoder && !mIsImage &&
                            getIntraRefreshPeriod(&intraRefreshPeriod) == OK
                            && intraRefreshPeriod > 0) {
                        notify->setInt32("intra-refresh-period", intraRefreshPeriod);
                    }
                    break;
                }
            }
            notify->setInt32("width", videoDef->nFrameWidth);
            notify->setInt32("height", videoDef->nFrameHeight);
            ALOGV("[%s] %s format is %s", mComponentName.c_str(),
                    portIndex == kPortIndexInput ? "input" : "output",
                    notify->debugString().c_str());

            break;
        }

        case OMX_PortDomainAudio:
        {
            OMX_AUDIO_PORTDEFINITIONTYPE *audioDef = &def.format.audio;

            switch ((int)audioDef->eEncoding) {
                case OMX_AUDIO_CodingPCM:
                {
                    OMX_AUDIO_PARAM_PCMMODETYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioPcm, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    if (params.nChannels <= 0
                            || (params.nChannels != 1 && !params.bInterleaved)
                            || params.ePCMMode != OMX_AUDIO_PCMModeLinear) {
                        ALOGE("unsupported PCM port: %u channels%s, %u-bit",
                                params.nChannels,
                                params.bInterleaved ? " interleaved" : "",
                                params.nBitPerSample);
                        return FAILED_TRANSACTION;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_RAW);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSamplingRate);

                    AudioEncoding encoding = kAudioEncodingPcm16bit;
                    if (params.eNumData == OMX_NumericalDataUnsigned
                            && params.nBitPerSample == 8u) {
                        encoding = kAudioEncodingPcm8bit;
                    } else if (params.eNumData == OMX_NumericalDataFloat
                            && params.nBitPerSample == 32u) {
                        encoding = kAudioEncodingPcmFloat;
                    } else if (params.nBitPerSample != 16u
                            || params.eNumData != OMX_NumericalDataSigned) {
                        ALOGE("unsupported PCM port: %s(%d), %s(%d) mode ",
                                asString(params.eNumData), params.eNumData,
                                asString(params.ePCMMode), params.ePCMMode);
                        return FAILED_TRANSACTION;
                    }
                    notify->setInt32("pcm-encoding", encoding);

                    if (mChannelMaskPresent) {
                        notify->setInt32("channel-mask", mChannelMask);
                    }
                    break;
                }

                case OMX_AUDIO_CodingAAC:
                {
                    OMX_AUDIO_PARAM_AACPROFILETYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioAac, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_AAC);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    notify->setInt32("bitrate", params.nBitRate);
                    break;
                }

                case OMX_AUDIO_CodingAMR:
                {
                    OMX_AUDIO_PARAM_AMRTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioAmr, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setInt32("channel-count", 1);
                    if (params.eAMRBandMode >= OMX_AUDIO_AMRBandModeWB0) {
                        notify->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_WB);
                        notify->setInt32("sample-rate", 16000);
                    } else {
                        notify->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_NB);
                        notify->setInt32("sample-rate", 8000);
                    }
                    break;
                }

                case OMX_AUDIO_CodingFLAC:
                {
                    OMX_AUDIO_PARAM_FLACTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioFlac, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_FLAC);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingMP3:
                {
                    OMX_AUDIO_PARAM_MP3TYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioMp3, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_MPEG);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingVORBIS:
                {
                    OMX_AUDIO_PARAM_VORBISTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            OMX_IndexParamAudioVorbis, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_VORBIS);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingAndroidAC3:
                {
                    OMX_AUDIO_PARAM_ANDROID_AC3TYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc3,
                            &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_AC3);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingAndroidEAC3:
                {
                    OMX_AUDIO_PARAM_ANDROID_EAC3TYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidEac3,
                            &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_EAC3);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingAndroidAC4:
                {
                    OMX_AUDIO_PARAM_ANDROID_AC4TYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidAc4,
                            &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_AC4);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingAndroidOPUS:
                {
                    OMX_AUDIO_PARAM_ANDROID_OPUSTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamAudioAndroidOpus,
                            &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_OPUS);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }

                case OMX_AUDIO_CodingG711:
                {
                    OMX_AUDIO_PARAM_PCMMODETYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                            (OMX_INDEXTYPE)OMX_IndexParamAudioPcm, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    const char *mime = NULL;
                    if (params.ePCMMode == OMX_AUDIO_PCMModeMULaw) {
                        mime = MEDIA_MIMETYPE_AUDIO_G711_MLAW;
                    } else if (params.ePCMMode == OMX_AUDIO_PCMModeALaw) {
                        mime = MEDIA_MIMETYPE_AUDIO_G711_ALAW;
                    } else { // params.ePCMMode == OMX_AUDIO_PCMModeLinear
                        mime = MEDIA_MIMETYPE_AUDIO_RAW;
                    }
                    notify->setString("mime", mime);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSamplingRate);
                    notify->setInt32("pcm-encoding", kAudioEncodingPcm16bit);
                    break;
                }

                case OMX_AUDIO_CodingGSMFR:
                {
                    OMX_AUDIO_PARAM_PCMMODETYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    err = mOMXNode->getParameter(
                                OMX_IndexParamAudioPcm, &params, sizeof(params));
                    if (err != OK) {
                        return err;
                    }

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_MSGSM);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSamplingRate);
                    break;
                }

                default:
                    ALOGE("Unsupported audio coding: %s(%d)\n",
                            asString(audioDef->eEncoding), audioDef->eEncoding);
                    return BAD_TYPE;
            }
            break;
        }

        default:
            ALOGE("Unsupported domain: %s(%d)", asString(def.eDomain), def.eDomain);
            return BAD_TYPE;
    }

    return getVendorParameters(portIndex, notify);
}

DescribeHDR10PlusInfoParams* ACodec::getHDR10PlusInfo(size_t paramSizeUsed) {
    if (mDescribeHDR10PlusInfoIndex == 0) {
        ALOGE("getHDR10PlusInfo: does not support DescribeHDR10PlusInfoParams");
        return nullptr;
    }

    size_t newSize = sizeof(DescribeHDR10PlusInfoParams) - 1 +
            ((paramSizeUsed > 0) ? paramSizeUsed : 512);
    if (mHdr10PlusScratchBuffer == nullptr
            || newSize > mHdr10PlusScratchBuffer->size()) {
        mHdr10PlusScratchBuffer = new ABuffer(newSize);
    }
    DescribeHDR10PlusInfoParams *config =
            (DescribeHDR10PlusInfoParams *)mHdr10PlusScratchBuffer->data();
    InitOMXParams(config);
    config->nSize = mHdr10PlusScratchBuffer->size();
    config->nPortIndex = 1;
    size_t paramSize = config->nSize - sizeof(DescribeHDR10PlusInfoParams) + 1;
    config->nParamSize = paramSize;
    config->nParamSizeUsed = 0;
    status_t err = mOMXNode->getConfig(
            (OMX_INDEXTYPE)mDescribeHDR10PlusInfoIndex,
            config, config->nSize);
    if (err != OK) {
        ALOGE("failed to get DescribeHDR10PlusInfoParams (err %d)", err);
        return nullptr;
    }
    if (config->nParamSize != paramSize) {
        ALOGE("DescribeHDR10PlusInfoParams alters nParamSize: %u vs %zu",
                config->nParamSize, paramSize);
        return nullptr;
    }
    if (paramSizeUsed > 0 && config->nParamSizeUsed != paramSizeUsed) {
        ALOGE("DescribeHDR10PlusInfoParams returns wrong nParamSizeUsed: %u vs %zu",
                config->nParamSizeUsed, paramSizeUsed);
        return nullptr;
    }
    return config;
}

void ACodec::onConfigUpdate(OMX_INDEXTYPE configIndex) {
    if (mDescribeHDR10PlusInfoIndex == 0
            || configIndex != mDescribeHDR10PlusInfoIndex) {
        // mDescribeHDR10PlusInfoIndex is the only update we recognize now
        return;
    }

    DescribeHDR10PlusInfoParams *config = getHDR10PlusInfo();
    if (config == nullptr) {
        return;
    }
    if (config->nParamSizeUsed > config->nParamSize) {
        // try again with the size specified
        config = getHDR10PlusInfo(config->nParamSizeUsed);
        if (config == nullptr) {
            return;
        }
    }

    mOutputFormat = mOutputFormat->dup(); // trigger an output format changed event
    mOutputFormat->setBuffer("hdr10-plus-info",
            ABuffer::CreateAsCopy(config->nValue, config->nParamSizeUsed));
}

void ACodec::onDataSpaceChanged(android_dataspace dataSpace, const ColorAspects &aspects) {
    // aspects are normally communicated in ColorAspects
    int32_t range, standard, transfer;
    convertCodecColorAspectsToPlatformAspects(aspects, &range, &standard, &transfer);

    // if some aspects are unspecified, use dataspace fields
    if (range == 0) {
        range = (dataSpace & HAL_DATASPACE_RANGE_MASK) >> HAL_DATASPACE_RANGE_SHIFT;
    }
    if (standard == 0) {
        standard = (dataSpace & HAL_DATASPACE_STANDARD_MASK) >> HAL_DATASPACE_STANDARD_SHIFT;
    }
    if (transfer == 0) {
        transfer = (dataSpace & HAL_DATASPACE_TRANSFER_MASK) >> HAL_DATASPACE_TRANSFER_SHIFT;
    }

    mOutputFormat = mOutputFormat->dup(); // trigger an output format changed event
    if (range != 0) {
        mOutputFormat->setInt32("color-range", range);
    }
    if (standard != 0) {
        mOutputFormat->setInt32("color-standard", standard);
    }
    if (transfer != 0) {
        mOutputFormat->setInt32("color-transfer", transfer);
    }

    ALOGD("dataspace changed to %#x (R:%d(%s), P:%d(%s), M:%d(%s), T:%d(%s)) "
          "(R:%d(%s), S:%d(%s), T:%d(%s))",
            dataSpace,
            aspects.mRange, asString(aspects.mRange),
            aspects.mPrimaries, asString(aspects.mPrimaries),
            aspects.mMatrixCoeffs, asString(aspects.mMatrixCoeffs),
            aspects.mTransfer, asString(aspects.mTransfer),
            range, asString((ColorRange)range),
            standard, asString((ColorStandard)standard),
            transfer, asString((ColorTransfer)transfer));
}

void ACodec::onOutputFormatChanged(sp<const AMessage> expectedFormat) {
    // store new output format, at the same time mark that this is no longer the first frame
    mOutputFormat = mBaseOutputFormat->dup();

    if (getPortFormat(kPortIndexOutput, mOutputFormat) != OK) {
        ALOGE("[%s] Failed to get port format to send format change", mComponentName.c_str());
        return;
    }

    if (expectedFormat != NULL) {
        sp<const AMessage> changes = expectedFormat->changesFrom(mOutputFormat);
        sp<const AMessage> to = mOutputFormat->changesFrom(expectedFormat);
        if (changes->countEntries() != 0 || to->countEntries() != 0) {
            ALOGW("[%s] BAD CODEC: Output format changed unexpectedly from (diff) %s to (diff) %s",
                    mComponentName.c_str(),
                    changes->debugString(4).c_str(), to->debugString(4).c_str());
        }
    }

    if (!mIsVideo && !mIsEncoder) {
        AudioEncoding pcmEncoding = kAudioEncodingPcm16bit;
        (void)mConfigFormat->findInt32("pcm-encoding", (int32_t*)&pcmEncoding);
        AudioEncoding codecPcmEncoding = kAudioEncodingPcm16bit;
        (void)mOutputFormat->findInt32("pcm-encoding", (int32_t*)&codecPcmEncoding);

        mConverter[kPortIndexOutput] = AudioConverter::Create(codecPcmEncoding, pcmEncoding);
        if (mConverter[kPortIndexOutput] != NULL) {
            mOutputFormat->setInt32("pcm-encoding", pcmEncoding);
        }
    }

    if (mTunneled) {
        sendFormatChange();
    }
}

void ACodec::sendFormatChange() {
    AString mime;
    CHECK(mOutputFormat->findString("mime", &mime));

    if (mime == MEDIA_MIMETYPE_AUDIO_RAW && (mEncoderDelay || mEncoderPadding)) {
        int32_t channelCount, sampleRate;
        CHECK(mOutputFormat->findInt32("channel-count", &channelCount));
        CHECK(mOutputFormat->findInt32("sample-rate", &sampleRate));
        if (mSampleRate != 0 && sampleRate != 0) {
            // avoiding 32-bit overflows in intermediate values
            mEncoderDelay = (int32_t)((((int64_t)mEncoderDelay) * sampleRate) / mSampleRate);
            mEncoderPadding = (int32_t)((((int64_t)mEncoderPadding) * sampleRate) / mSampleRate);
            mSampleRate = sampleRate;
        }
        if (mSkipCutBuffer != NULL) {
            size_t prevbufsize = mSkipCutBuffer->size();
            if (prevbufsize != 0) {
                ALOGW("Replacing SkipCutBuffer holding %zu bytes", prevbufsize);
            }
        }
        mSkipCutBuffer = new SkipCutBuffer(mEncoderDelay, mEncoderPadding, channelCount);
    }

    // mLastOutputFormat is not used when tunneled; doing this just to stay consistent
    mLastOutputFormat = mOutputFormat;
}

void ACodec::signalError(OMX_ERRORTYPE error, status_t internalError) {
    ALOGE("signalError(omxError %#x, internalError %d)", error, internalError);

    if (internalError == UNKNOWN_ERROR) { // find better error code
        const status_t omxStatus = statusFromOMXError(error);
        if (omxStatus != 0) {
            internalError = omxStatus;
        } else {
            ALOGW("Invalid OMX error %#x", error);
        }
    }

    mFatalError = true;
    mCallback->onError(internalError, ACTION_CODE_FATAL);
}

status_t ACodec::requestIDRFrame() {
    if (!mIsEncoder) {
        return ERROR_UNSUPPORTED;
    }

    OMX_CONFIG_INTRAREFRESHVOPTYPE params;
    InitOMXParams(&params);

    params.nPortIndex = kPortIndexOutput;
    params.IntraRefreshVOP = OMX_TRUE;

    return mOMXNode->setConfig(
            OMX_IndexConfigVideoIntraVOPRefresh,
            &params,
            sizeof(params));
}

////////////////////////////////////////////////////////////////////////////////

ACodec::BaseState::BaseState(ACodec *codec, const sp<AState> &parentState)
    : AState(parentState),
      mCodec(codec) {
}

ACodec::BaseState::PortMode ACodec::BaseState::getPortMode(
        OMX_U32 /* portIndex */) {
    return KEEP_BUFFERS;
}

void ACodec::BaseState::stateExited() {
    ++mCodec->mStateGeneration;
}

bool ACodec::BaseState::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatInputBufferFilled:
        {
            onInputBufferFilled(msg);
            break;
        }

        case kWhatOutputBufferDrained:
        {
            onOutputBufferDrained(msg);
            break;
        }

        case ACodec::kWhatOMXMessageList:
        {
            return checkOMXMessage(msg) ? onOMXMessageList(msg) : true;
        }

        case ACodec::kWhatOMXMessageItem:
        {
            // no need to check as we already did it for kWhatOMXMessageList
            return onOMXMessage(msg);
        }

        case ACodec::kWhatOMXMessage:
        {
            return checkOMXMessage(msg) ? onOMXMessage(msg) : true;
        }

        case ACodec::kWhatSetSurface:
        {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));

            sp<RefBase> obj;
            CHECK(msg->findObject("surface", &obj));

            status_t err = mCodec->handleSetSurface(static_cast<Surface *>(obj.get()));

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case ACodec::kWhatCreateInputSurface:
        case ACodec::kWhatSetInputSurface:
        case ACodec::kWhatSignalEndOfInputStream:
        {
            // This may result in an app illegal state exception.
            ALOGE("Message 0x%x was not handled", msg->what());
            mCodec->signalError(OMX_ErrorUndefined, INVALID_OPERATION);
            return true;
        }

        case ACodec::kWhatOMXDied:
        {
            // This will result in kFlagSawMediaServerDie handling in MediaCodec.
            ALOGE("OMX/mediaserver died, signalling error!");
            mCodec->mGraphicBufferSource.clear();
            mCodec->signalError(OMX_ErrorResourcesLost, DEAD_OBJECT);
            break;
        }

        case ACodec::kWhatReleaseCodecInstance:
        {
            ALOGI("[%s] forcing the release of codec",
                    mCodec->mComponentName.c_str());
            status_t err = mCodec->mOMXNode->freeNode();
            ALOGE_IF("[%s] failed to release codec instance: err=%d",
                       mCodec->mComponentName.c_str(), err);
            mCodec->mCallback->onReleaseCompleted();

            mCodec->changeState(mCodec->mUninitializedState);
            break;
        }

        case ACodec::kWhatForceStateTransition:
        {
            ALOGV("Already transitioned --- ignore");
            break;
        }

        case kWhatCheckIfStuck: {
            ALOGV("No-op by default");
            break;
        }

        default:
            return false;
    }

    return true;
}

bool ACodec::BaseState::checkOMXMessage(const sp<AMessage> &msg) {
    // there is a possibility that this is an outstanding message for a
    // codec that we have already destroyed
    if (mCodec->mOMXNode == NULL) {
        ALOGI("ignoring message as already freed component: %s",
                msg->debugString().c_str());
        return false;
    }

    int32_t generation;
    CHECK(msg->findInt32("generation", (int32_t*)&generation));
    if (generation != mCodec->mNodeGeneration) {
        ALOGW("Unexpected message for component: %s, gen %u, cur %u",
                msg->debugString().c_str(), generation, mCodec->mNodeGeneration);
        return false;
    }
    return true;
}

bool ACodec::BaseState::onOMXMessageList(const sp<AMessage> &msg) {
    sp<RefBase> obj;
    CHECK(msg->findObject("messages", &obj));
    sp<MessageList> msgList = static_cast<MessageList *>(obj.get());

    bool receivedRenderedEvents = false;
    for (std::list<sp<AMessage>>::const_iterator it = msgList->getList().cbegin();
          it != msgList->getList().cend(); ++it) {
        (*it)->setWhat(ACodec::kWhatOMXMessageItem);
        mCodec->handleMessage(*it);
        int32_t type;
        CHECK((*it)->findInt32("type", &type));
        if (type == omx_message::FRAME_RENDERED) {
            receivedRenderedEvents = true;
        }
    }

    if (receivedRenderedEvents) {
        // NOTE: all buffers are rendered in this case
        mCodec->notifyOfRenderedFrames();
    }
    return true;
}

bool ACodec::BaseState::onOMXMessage(const sp<AMessage> &msg) {
    int32_t type;
    CHECK(msg->findInt32("type", &type));

    switch (type) {
        case omx_message::EVENT:
        {
            int32_t event, data1, data2;
            CHECK(msg->findInt32("event", &event));
            CHECK(msg->findInt32("data1", &data1));
            CHECK(msg->findInt32("data2", &data2));

            if (event == OMX_EventCmdComplete
                    && data1 == OMX_CommandFlush
                    && data2 == (int32_t)OMX_ALL) {
                // Use of this notification is not consistent across
                // implementations. We'll drop this notification and rely
                // on flush-complete notifications on the individual port
                // indices instead.

                return true;
            }

            return onOMXEvent(
                    static_cast<OMX_EVENTTYPE>(event),
                    static_cast<OMX_U32>(data1),
                    static_cast<OMX_U32>(data2));
        }

        case omx_message::EMPTY_BUFFER_DONE:
        {
            IOMX::buffer_id bufferID;
            int32_t fenceFd;

            CHECK(msg->findInt32("buffer", (int32_t*)&bufferID));
            CHECK(msg->findInt32("fence_fd", &fenceFd));

            return onOMXEmptyBufferDone(bufferID, fenceFd);
        }

        case omx_message::FILL_BUFFER_DONE:
        {
            IOMX::buffer_id bufferID;
            CHECK(msg->findInt32("buffer", (int32_t*)&bufferID));

            int32_t rangeOffset, rangeLength, flags, fenceFd;
            int64_t timeUs;

            CHECK(msg->findInt32("range_offset", &rangeOffset));
            CHECK(msg->findInt32("range_length", &rangeLength));
            CHECK(msg->findInt32("flags", &flags));
            CHECK(msg->findInt64("timestamp", &timeUs));
            CHECK(msg->findInt32("fence_fd", &fenceFd));

            return onOMXFillBufferDone(
                    bufferID,
                    (size_t)rangeOffset, (size_t)rangeLength,
                    (OMX_U32)flags,
                    timeUs,
                    fenceFd);
        }

        case omx_message::FRAME_RENDERED:
        {
            int64_t mediaTimeUs, systemNano;

            CHECK(msg->findInt64("media_time_us", &mediaTimeUs));
            CHECK(msg->findInt64("system_nano", &systemNano));

            return onOMXFrameRendered(
                    mediaTimeUs, systemNano);
        }

        default:
            ALOGE("Unexpected message type: %d", type);
            return false;
    }
}

bool ACodec::BaseState::onOMXFrameRendered(
        int64_t mediaTimeUs __unused, nsecs_t systemNano __unused) {
    // ignore outside of Executing and PortSettingsChanged states
    return true;
}

bool ACodec::BaseState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    if (event == OMX_EventDataSpaceChanged) {
        ColorAspects aspects = ColorUtils::unpackToColorAspects(data2);

        mCodec->onDataSpaceChanged((android_dataspace)data1, aspects);
        return true;
    }

    if (event != OMX_EventError) {
        ALOGV("[%s] EVENT(%d, 0x%08x, 0x%08x)",
             mCodec->mComponentName.c_str(), event, data1, data2);

        return false;
    }

    ALOGE("[%s] ERROR(0x%08x)", mCodec->mComponentName.c_str(), data1);

    // verify OMX component sends back an error we expect.
    OMX_ERRORTYPE omxError = (OMX_ERRORTYPE)data1;
    if (!isOMXError(omxError)) {
        ALOGW("Invalid OMX error %#x", omxError);
        omxError = OMX_ErrorUndefined;
    }
    mCodec->signalError(omxError);

    return true;
}

bool ACodec::BaseState::onOMXEmptyBufferDone(IOMX::buffer_id bufferID, int fenceFd) {
    ALOGV("[%s] onOMXEmptyBufferDone %u",
         mCodec->mComponentName.c_str(), bufferID);

    BufferInfo *info = mCodec->findBufferByID(kPortIndexInput, bufferID);
    BufferInfo::Status status = BufferInfo::getSafeStatus(info);
    if (status != BufferInfo::OWNED_BY_COMPONENT) {
        ALOGE("Wrong ownership in EBD: %s(%d) buffer #%u", _asString(status), status, bufferID);
        mCodec->dumpBuffers(kPortIndexInput);
        if (fenceFd >= 0) {
            ::close(fenceFd);
        }
        return false;
    }
    info->mStatus = BufferInfo::OWNED_BY_US;

    // input buffers cannot take fences, so wait for any fence now
    (void)mCodec->waitForFence(fenceFd, "onOMXEmptyBufferDone");
    fenceFd = -1;

    // still save fence for completeness
    info->setWriteFence(fenceFd, "onOMXEmptyBufferDone");

    // We're in "store-metadata-in-buffers" mode, the underlying
    // OMX component had access to data that's implicitly refcounted
    // by this "MediaBuffer" object. Now that the OMX component has
    // told us that it's done with the input buffer, we can decrement
    // the mediaBuffer's reference count.
    info->mData->meta()->setObject("mediaBufferHolder", sp<MediaBufferHolder>(nullptr));

    PortMode mode = getPortMode(kPortIndexInput);

    switch (mode) {
        case KEEP_BUFFERS:
            break;

        case RESUBMIT_BUFFERS:
            postFillThisBuffer(info);
            break;

        case FREE_BUFFERS:
        default:
            ALOGE("SHOULD NOT REACH HERE: cannot free empty output buffers");
            return false;
    }

    return true;
}

void ACodec::BaseState::postFillThisBuffer(BufferInfo *info) {
    if (mCodec->mPortEOS[kPortIndexInput]) {
        return;
    }

    CHECK_EQ((int)info->mStatus, (int)BufferInfo::OWNED_BY_US);

    info->mData->setFormat(mCodec->mInputFormat);
    mCodec->mBufferChannel->fillThisBuffer(info->mBufferID);
    info->mData.clear();
    info->mStatus = BufferInfo::OWNED_BY_UPSTREAM;
}

void ACodec::BaseState::onInputBufferFilled(const sp<AMessage> &msg) {
    IOMX::buffer_id bufferID;
    CHECK(msg->findInt32("buffer-id", (int32_t*)&bufferID));
    sp<MediaCodecBuffer> buffer;
    int32_t err = OK;
    bool eos = false;
    PortMode mode = getPortMode(kPortIndexInput);
    int32_t discarded = 0;
    if (msg->findInt32("discarded", &discarded) && discarded) {
        // these are unfilled buffers returned by client
        // buffers are returned on MediaCodec.flush
        mode = KEEP_BUFFERS;
    }
    sp<RefBase> obj;
    CHECK(msg->findObject("buffer", &obj));
    buffer = static_cast<MediaCodecBuffer *>(obj.get());

    int32_t tmp;
    if (buffer != NULL && buffer->meta()->findInt32("eos", &tmp) && tmp) {
        eos = true;
        err = ERROR_END_OF_STREAM;
    }

    BufferInfo *info = mCodec->findBufferByID(kPortIndexInput, bufferID);
    BufferInfo::Status status = BufferInfo::getSafeStatus(info);
    if (status != BufferInfo::OWNED_BY_UPSTREAM) {
        ALOGE("Wrong ownership in IBF: %s(%d) buffer #%u", _asString(status), status, bufferID);
        mCodec->dumpBuffers(kPortIndexInput);
        mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
        return;
    }

    info->mStatus = BufferInfo::OWNED_BY_US;
    info->mData = buffer;

    switch (mode) {
        case KEEP_BUFFERS:
        {
            if (eos) {
                if (!mCodec->mPortEOS[kPortIndexInput]) {
                    mCodec->mPortEOS[kPortIndexInput] = true;
                    mCodec->mInputEOSResult = err;
                }
            }
            break;
        }

        case RESUBMIT_BUFFERS:
        {
            if (buffer != NULL && !mCodec->mPortEOS[kPortIndexInput]) {
                // Do not send empty input buffer w/o EOS to the component.
                if (buffer->size() == 0 && !eos) {
                    postFillThisBuffer(info);
                    break;
                }

                int64_t timeUs;
                CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

                OMX_U32 flags = OMX_BUFFERFLAG_ENDOFFRAME;

                int32_t isCSD = 0;
                if (buffer->meta()->findInt32("csd", &isCSD) && isCSD != 0) {
                    if (mCodec->mIsLegacyVP9Decoder) {
                        ALOGV("[%s] is legacy VP9 decoder. Ignore %u codec specific data",
                            mCodec->mComponentName.c_str(), bufferID);
                        postFillThisBuffer(info);
                        break;
                    }
                    flags |= OMX_BUFFERFLAG_CODECCONFIG;
                }

                if (eos) {
                    flags |= OMX_BUFFERFLAG_EOS;
                }

                size_t size = buffer->size();
                size_t offset = buffer->offset();
                if (buffer->base() != info->mCodecData->base()) {
                    ALOGV("[%s] Needs to copy input data for buffer %u. (%p != %p)",
                         mCodec->mComponentName.c_str(),
                         bufferID,
                         buffer->base(), info->mCodecData->base());

                    sp<DataConverter> converter = mCodec->mConverter[kPortIndexInput];
                    if (converter == NULL || isCSD) {
                        converter = getCopyConverter();
                    }
                    status_t err = converter->convert(buffer, info->mCodecData);
                    if (err != OK) {
                        mCodec->signalError(OMX_ErrorUndefined, err);
                        return;
                    }
                    size = info->mCodecData->size();
                } else {
                    info->mCodecData->setRange(offset, size);
                }

                if (flags & OMX_BUFFERFLAG_CODECCONFIG) {
                    ALOGV("[%s] calling emptyBuffer %u w/ codec specific data",
                         mCodec->mComponentName.c_str(), bufferID);
                } else if (flags & OMX_BUFFERFLAG_EOS) {
                    ALOGV("[%s] calling emptyBuffer %u w/ EOS",
                         mCodec->mComponentName.c_str(), bufferID);
                } else {
#if TRACK_BUFFER_TIMING
                    ALOGI("[%s] calling emptyBuffer %u w/ time %lld us",
                         mCodec->mComponentName.c_str(), bufferID, (long long)timeUs);
#else
                    ALOGV("[%s] calling emptyBuffer %u w/ time %lld us",
                         mCodec->mComponentName.c_str(), bufferID, (long long)timeUs);
#endif
                }

#if TRACK_BUFFER_TIMING
                ACodec::BufferStats stats;
                stats.mEmptyBufferTimeUs = ALooper::GetNowUs();
                stats.mFillBufferDoneTimeUs = -1ll;
                mCodec->mBufferStats.add(timeUs, stats);
#endif

                if (mCodec->storingMetadataInDecodedBuffers()) {
                    // try to submit an output buffer for each input buffer
                    PortMode outputMode = getPortMode(kPortIndexOutput);

                    ALOGV("MetadataBuffersToSubmit=%u portMode=%s",
                            mCodec->mMetadataBuffersToSubmit,
                            (outputMode == FREE_BUFFERS ? "FREE" :
                             outputMode == KEEP_BUFFERS ? "KEEP" : "RESUBMIT"));
                    if (outputMode == RESUBMIT_BUFFERS) {
                        mCodec->submitOutputMetadataBuffer();
                    }
                }
                info->checkReadFence("onInputBufferFilled");

                status_t err2 = OK;
                switch (mCodec->mPortMode[kPortIndexInput]) {
                case IOMX::kPortModePresetByteBuffer:
                case IOMX::kPortModePresetANWBuffer:
                case IOMX::kPortModePresetSecureBuffer:
                    {
                        err2 = mCodec->mOMXNode->emptyBuffer(
                            bufferID, info->mCodecData, flags, timeUs, info->mFenceFd);
                    }
                    break;
#ifndef OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
                case IOMX::kPortModeDynamicNativeHandle:
                    if (info->mCodecData->size() >= sizeof(VideoNativeHandleMetadata)) {
                        VideoNativeHandleMetadata *vnhmd =
                            (VideoNativeHandleMetadata*)info->mCodecData->base();
                        sp<NativeHandle> handle = NativeHandle::create(
                                vnhmd->pHandle, false /* ownsHandle */);
                        err2 = mCodec->mOMXNode->emptyBuffer(
                            bufferID, handle, flags, timeUs, info->mFenceFd);
                    }
                    break;
                case IOMX::kPortModeDynamicANWBuffer:
                    if (info->mCodecData->size() >= sizeof(VideoNativeMetadata)) {
                        VideoNativeMetadata *vnmd = (VideoNativeMetadata*)info->mCodecData->base();
                        sp<GraphicBuffer> graphicBuffer = GraphicBuffer::from(vnmd->pBuffer);
                        err2 = mCodec->mOMXNode->emptyBuffer(
                            bufferID, graphicBuffer, flags, timeUs, info->mFenceFd);
                    }
                    break;
#endif
                default:
                    ALOGW("Can't marshall %s data in %zu sized buffers in %zu-bit mode",
                            asString(mCodec->mPortMode[kPortIndexInput]),
                            info->mCodecData->size(),
                            sizeof(buffer_handle_t) * 8);
                    err2 = ERROR_UNSUPPORTED;
                    break;
                }

                info->mFenceFd = -1;
                if (err2 != OK) {
                    mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err2));
                    return;
                }
                info->mStatus = BufferInfo::OWNED_BY_COMPONENT;
                // Hold the reference while component is using the buffer.
                info->mData = buffer;

                if (!eos && err == OK) {
                    getMoreInputDataIfPossible();
                } else {
                    ALOGV("[%s] Signalled EOS (%d) on the input port",
                         mCodec->mComponentName.c_str(), err);

                    mCodec->mPortEOS[kPortIndexInput] = true;
                    mCodec->mInputEOSResult = err;
                }
            } else if (!mCodec->mPortEOS[kPortIndexInput]) {
                if (err != OK && err != ERROR_END_OF_STREAM) {
                    ALOGV("[%s] Signalling EOS on the input port due to error %d",
                         mCodec->mComponentName.c_str(), err);
                } else {
                    ALOGV("[%s] Signalling EOS on the input port",
                         mCodec->mComponentName.c_str());
                }

                ALOGV("[%s] calling emptyBuffer %u signalling EOS",
                     mCodec->mComponentName.c_str(), bufferID);

                info->checkReadFence("onInputBufferFilled");
                status_t err2 = mCodec->mOMXNode->emptyBuffer(
                        bufferID, OMXBuffer::sPreset, OMX_BUFFERFLAG_EOS, 0, info->mFenceFd);
                info->mFenceFd = -1;
                if (err2 != OK) {
                    mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err2));
                    return;
                }
                info->mStatus = BufferInfo::OWNED_BY_COMPONENT;

                mCodec->mPortEOS[kPortIndexInput] = true;
                mCodec->mInputEOSResult = err;
            }
            break;
        }

        case FREE_BUFFERS:
            break;

        default:
            ALOGE("invalid port mode: %d", mode);
            break;
    }
}

void ACodec::BaseState::getMoreInputDataIfPossible() {
    if (mCodec->mPortEOS[kPortIndexInput]) {
        return;
    }

    BufferInfo *eligible = NULL;

    for (size_t i = 0; i < mCodec->mBuffers[kPortIndexInput].size(); ++i) {
        BufferInfo *info = &mCodec->mBuffers[kPortIndexInput].editItemAt(i);

#if 0
        if (info->mStatus == BufferInfo::OWNED_BY_UPSTREAM) {
            // There's already a "read" pending.
            return;
        }
#endif

        if (info->mStatus == BufferInfo::OWNED_BY_US) {
            eligible = info;
        }
    }

    if (eligible == NULL) {
        return;
    }

    postFillThisBuffer(eligible);
}

bool ACodec::BaseState::onOMXFillBufferDone(
        IOMX::buffer_id bufferID,
        size_t rangeOffset, size_t rangeLength,
        OMX_U32 flags,
        int64_t timeUs,
        int fenceFd) {
    ALOGV("[%s] onOMXFillBufferDone %u time %" PRId64 " us, flags = 0x%08x",
         mCodec->mComponentName.c_str(), bufferID, timeUs, flags);

    ssize_t index;
    status_t err= OK;

#if TRACK_BUFFER_TIMING
    index = mCodec->mBufferStats.indexOfKey(timeUs);
    if (index >= 0) {
        ACodec::BufferStats *stats = &mCodec->mBufferStats.editValueAt(index);
        stats->mFillBufferDoneTimeUs = ALooper::GetNowUs();

        ALOGI("frame PTS %lld: %lld",
                timeUs,
                stats->mFillBufferDoneTimeUs - stats->mEmptyBufferTimeUs);

        mCodec->mBufferStats.removeItemsAt(index);
        stats = NULL;
    }
#endif

    BufferInfo *info =
        mCodec->findBufferByID(kPortIndexOutput, bufferID, &index);
    BufferInfo::Status status = BufferInfo::getSafeStatus(info);
    if (status != BufferInfo::OWNED_BY_COMPONENT) {
        ALOGE("Wrong ownership in FBD: %s(%d) buffer #%u", _asString(status), status, bufferID);
        mCodec->dumpBuffers(kPortIndexOutput);
        mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
        if (fenceFd >= 0) {
            ::close(fenceFd);
        }
        return true;
    }

    info->mDequeuedAt = ++mCodec->mDequeueCounter;
    info->mStatus = BufferInfo::OWNED_BY_US;

    if (info->mRenderInfo != NULL) {
        // The fence for an emptied buffer must have signaled, but there still could be queued
        // or out-of-order dequeued buffers in the render queue prior to this buffer. Drop these,
        // as we will soon requeue this buffer to the surface. While in theory we could still keep
        // track of buffers that are requeued to the surface, it is better to add support to the
        // buffer-queue to notify us of released buffers and their fences (in the future).
        mCodec->notifyOfRenderedFrames(true /* dropIncomplete */);
    }

    // byte buffers cannot take fences, so wait for any fence now
    if (mCodec->mNativeWindow == NULL) {
        (void)mCodec->waitForFence(fenceFd, "onOMXFillBufferDone");
        fenceFd = -1;
    }
    info->setReadFence(fenceFd, "onOMXFillBufferDone");

    PortMode mode = getPortMode(kPortIndexOutput);

    switch (mode) {
        case KEEP_BUFFERS:
            break;

        case RESUBMIT_BUFFERS:
        {
            if (rangeLength == 0 && (!(flags & OMX_BUFFERFLAG_EOS)
                    || mCodec->mPortEOS[kPortIndexOutput])) {
                ALOGV("[%s] calling fillBuffer %u",
                     mCodec->mComponentName.c_str(), info->mBufferID);

                err = mCodec->fillBuffer(info);
                if (err != OK) {
                    mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
                    return true;
                }
                break;
            }

            sp<MediaCodecBuffer> buffer = info->mData;

            if (mCodec->mOutputFormat != mCodec->mLastOutputFormat && rangeLength > 0) {
                // pretend that output format has changed on the first frame (we used to do this)
                if (mCodec->mBaseOutputFormat == mCodec->mOutputFormat) {
                    mCodec->onOutputFormatChanged(mCodec->mOutputFormat);
                }
                mCodec->sendFormatChange();
            }
            buffer->setFormat(mCodec->mOutputFormat);

            if (mCodec->usingSecureBufferOnEncoderOutput()) {
                native_handle_t *handle = NULL;
                sp<SecureBuffer> secureBuffer = static_cast<SecureBuffer *>(buffer.get());
                if (secureBuffer != NULL) {
#ifdef OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
                    // handle is only valid on 32-bit/mediaserver process
                    handle = NULL;
#else
                    handle = (native_handle_t *)secureBuffer->getDestinationPointer();
#endif
                }
                buffer->meta()->setPointer("handle", handle);
                buffer->meta()->setInt32("rangeOffset", rangeOffset);
                buffer->meta()->setInt32("rangeLength", rangeLength);
            } else if (buffer->base() == info->mCodecData->base()) {
                buffer->setRange(rangeOffset, rangeLength);
            } else {
                info->mCodecData->setRange(rangeOffset, rangeLength);
                // in this case we know that mConverter is not null
                status_t err = mCodec->mConverter[kPortIndexOutput]->convert(
                        info->mCodecData, buffer);
                if (err != OK) {
                    mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
                    return true;
                }
            }
#if 0
            if (mCodec->mNativeWindow == NULL) {
                if (IsIDR(info->mData->data(), info->mData->size())) {
                    ALOGI("IDR frame");
                }
            }
#endif

            if (mCodec->mSkipCutBuffer != NULL) {
                mCodec->mSkipCutBuffer->submit(buffer);
            }
            buffer->meta()->setInt64("timeUs", timeUs);

            info->mData.clear();

            mCodec->mBufferChannel->drainThisBuffer(info->mBufferID, flags);

            info->mStatus = BufferInfo::OWNED_BY_DOWNSTREAM;

            if (flags & OMX_BUFFERFLAG_EOS) {
                ALOGV("[%s] saw output EOS", mCodec->mComponentName.c_str());

                mCodec->mCallback->onEos(mCodec->mInputEOSResult);
                mCodec->mPortEOS[kPortIndexOutput] = true;
            }
            break;
        }

        case FREE_BUFFERS:
            err = mCodec->freeBuffer(kPortIndexOutput, index);
            if (err != OK) {
                mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
                return true;
            }
            break;

        default:
            ALOGE("Invalid port mode: %d", mode);
            return false;
    }

    return true;
}

void ACodec::BaseState::onOutputBufferDrained(const sp<AMessage> &msg) {
    IOMX::buffer_id bufferID;
    CHECK(msg->findInt32("buffer-id", (int32_t*)&bufferID));
    sp<RefBase> obj;
    CHECK(msg->findObject("buffer", &obj));
    sp<MediaCodecBuffer> buffer = static_cast<MediaCodecBuffer *>(obj.get());
    int32_t discarded = 0;
    msg->findInt32("discarded", &discarded);

    ssize_t index;
    BufferInfo *info = mCodec->findBufferByID(kPortIndexOutput, bufferID, &index);
    BufferInfo::Status status = BufferInfo::getSafeStatus(info);
    if (status != BufferInfo::OWNED_BY_DOWNSTREAM) {
        ALOGE("Wrong ownership in OBD: %s(%d) buffer #%u", _asString(status), status, bufferID);
        mCodec->dumpBuffers(kPortIndexOutput);
        mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
        return;
    }
    info->mData = buffer;
    int32_t render;
    if (mCodec->mNativeWindow != NULL
            && msg->findInt32("render", &render) && render != 0
            && !discarded && buffer->size() != 0) {
        ATRACE_NAME("render");
        // The client wants this buffer to be rendered.

        android_native_rect_t crop;
        if (buffer->format()->findRect("crop", &crop.left, &crop.top, &crop.right, &crop.bottom)) {
            // NOTE: native window uses extended right-bottom coordinate
            ++crop.right;
            ++crop.bottom;
            if (memcmp(&crop, &mCodec->mLastNativeWindowCrop, sizeof(crop)) != 0) {
                mCodec->mLastNativeWindowCrop = crop;
                status_t err = native_window_set_crop(mCodec->mNativeWindow.get(), &crop);
                ALOGW_IF(err != NO_ERROR, "failed to set crop: %d", err);
            }
        }

        int32_t dataSpace;
        if (buffer->format()->findInt32("android._dataspace", &dataSpace)
                && dataSpace != mCodec->mLastNativeWindowDataSpace) {
            status_t err = native_window_set_buffers_data_space(
                    mCodec->mNativeWindow.get(), (android_dataspace)dataSpace);
            mCodec->mLastNativeWindowDataSpace = dataSpace;
            ALOGW_IF(err != NO_ERROR, "failed to set dataspace: %d", err);
        }
        if (buffer->format()->contains("hdr-static-info")) {
            HDRStaticInfo info;
            if (ColorUtils::getHDRStaticInfoFromFormat(buffer->format(), &info)
                && memcmp(&mCodec->mLastHDRStaticInfo, &info, sizeof(info))) {
                setNativeWindowHdrMetadata(mCodec->mNativeWindow.get(), &info);
                mCodec->mLastHDRStaticInfo = info;
            }
        }

        sp<ABuffer> hdr10PlusInfo;
        if (buffer->format()->findBuffer("hdr10-plus-info", &hdr10PlusInfo)
                && hdr10PlusInfo != nullptr && hdr10PlusInfo->size() > 0
                && hdr10PlusInfo != mCodec->mLastHdr10PlusBuffer) {
            native_window_set_buffers_hdr10_plus_metadata(mCodec->mNativeWindow.get(),
                    hdr10PlusInfo->size(), hdr10PlusInfo->data());
            mCodec->mLastHdr10PlusBuffer = hdr10PlusInfo;
        }

        // save buffers sent to the surface so we can get render time when they return
        int64_t mediaTimeUs = -1;
        buffer->meta()->findInt64("timeUs", &mediaTimeUs);
        if (mediaTimeUs >= 0) {
            mCodec->mRenderTracker.onFrameQueued(
                    mediaTimeUs, info->mGraphicBuffer, new Fence(::dup(info->mFenceFd)));
        }

        int64_t timestampNs = 0;
        if (!msg->findInt64("timestampNs", &timestampNs)) {
            // use media timestamp if client did not request a specific render timestamp
            if (buffer->meta()->findInt64("timeUs", &timestampNs)) {
                ALOGV("using buffer PTS of %lld", (long long)timestampNs);
                timestampNs *= 1000;
            }
        }

        status_t err;
        err = native_window_set_buffers_timestamp(mCodec->mNativeWindow.get(), timestampNs);
        ALOGW_IF(err != NO_ERROR, "failed to set buffer timestamp: %d", err);

        info->checkReadFence("onOutputBufferDrained before queueBuffer");
        err = mCodec->mNativeWindow->queueBuffer(
                    mCodec->mNativeWindow.get(), info->mGraphicBuffer.get(), info->mFenceFd);
        info->mFenceFd = -1;
        if (err == OK) {
            info->mStatus = BufferInfo::OWNED_BY_NATIVE_WINDOW;
        } else {
            ALOGE("queueBuffer failed in onOutputBufferDrained: %d", err);
            mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
            info->mStatus = BufferInfo::OWNED_BY_US;
            // keeping read fence as write fence to avoid clobbering
            info->mIsReadFence = false;
        }
    } else {
        if (mCodec->mNativeWindow != NULL && (discarded || buffer->size() != 0)) {
            // move read fence into write fence to avoid clobbering
            info->mIsReadFence = false;
            ATRACE_NAME("frame-drop");
        }
        info->mStatus = BufferInfo::OWNED_BY_US;
    }

    PortMode mode = getPortMode(kPortIndexOutput);

    switch (mode) {
        case KEEP_BUFFERS:
        {
            // XXX fishy, revisit!!! What about the FREE_BUFFERS case below?

            if (info->mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                // We cannot resubmit the buffer we just rendered, dequeue
                // the spare instead.

                info = mCodec->dequeueBufferFromNativeWindow();
            }
            break;
        }

        case RESUBMIT_BUFFERS:
        {
            if (!mCodec->mPortEOS[kPortIndexOutput]) {
                if (info->mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                    // We cannot resubmit the buffer we just rendered, dequeue
                    // the spare instead.

                    info = mCodec->dequeueBufferFromNativeWindow();
                }

                if (info != NULL) {
                    ALOGV("[%s] calling fillBuffer %u",
                         mCodec->mComponentName.c_str(), info->mBufferID);
                    info->checkWriteFence("onOutputBufferDrained::RESUBMIT_BUFFERS");
                    status_t err = mCodec->fillBuffer(info);
                    if (err != OK) {
                        mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
                    }
                }
            }
            break;
        }

        case FREE_BUFFERS:
        {
            status_t err = mCodec->freeBuffer(kPortIndexOutput, index);
            if (err != OK) {
                mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
            }
            break;
        }

        default:
            ALOGE("Invalid port mode: %d", mode);
            return;
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::UninitializedState::UninitializedState(ACodec *codec)
    : BaseState(codec) {
}

void ACodec::UninitializedState::stateEntered() {
    ALOGV("Now uninitialized");

    if (mDeathNotifier != NULL) {
        if (mCodec->mOMXNode != NULL) {
            auto tOmxNode = mCodec->mOMXNode->getHalInterface<IOmxNode>();
            if (tOmxNode) {
                tOmxNode->unlinkToDeath(mDeathNotifier);
            }
        }
        mDeathNotifier.clear();
    }

    mCodec->mUsingNativeWindow = false;
    mCodec->mNativeWindow.clear();
    mCodec->mNativeWindowUsageBits = 0;
    mCodec->mOMX.clear();
    mCodec->mOMXNode.clear();
    mCodec->mFlags = 0;
    mCodec->mPortMode[kPortIndexInput] = IOMX::kPortModePresetByteBuffer;
    mCodec->mPortMode[kPortIndexOutput] = IOMX::kPortModePresetByteBuffer;
    mCodec->mConverter[0].clear();
    mCodec->mConverter[1].clear();
    mCodec->mComponentName.clear();
}

bool ACodec::UninitializedState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case ACodec::kWhatSetup:
        {
            onSetup(msg);

            handled = true;
            break;
        }

        case ACodec::kWhatAllocateComponent:
        {
            onAllocateComponent(msg);
            handled = true;
            break;
        }

        case ACodec::kWhatShutdown:
        {
            int32_t keepComponentAllocated;
            CHECK(msg->findInt32(
                        "keepComponentAllocated", &keepComponentAllocated));
            ALOGW_IF(keepComponentAllocated,
                     "cannot keep component allocated on shutdown in Uninitialized state");
            if (keepComponentAllocated) {
                mCodec->mCallback->onStopCompleted();
            } else {
                mCodec->mCallback->onReleaseCompleted();
            }
            handled = true;
            break;
        }

        case ACodec::kWhatFlush:
        {
            mCodec->mCallback->onFlushCompleted();
            handled = true;
            break;
        }

        case ACodec::kWhatReleaseCodecInstance:
        {
            // nothing to do, as we have already signaled shutdown
            handled = true;
            break;
        }

        default:
            return BaseState::onMessageReceived(msg);
    }

    return handled;
}

void ACodec::UninitializedState::onSetup(
        const sp<AMessage> &msg) {
    if (onAllocateComponent(msg)
            && mCodec->mLoadedState->onConfigureComponent(msg)) {
        mCodec->mLoadedState->onStart();
    }
}

bool ACodec::UninitializedState::onAllocateComponent(const sp<AMessage> &msg) {
    ALOGV("onAllocateComponent");

    CHECK(mCodec->mOMXNode == NULL);

    sp<AMessage> notify = new AMessage(kWhatOMXMessageList, mCodec);
    notify->setInt32("generation", mCodec->mNodeGeneration + 1);

    sp<RefBase> obj;
    CHECK(msg->findObject("codecInfo", &obj));
    sp<MediaCodecInfo> info = (MediaCodecInfo *)obj.get();
    if (info == nullptr) {
        ALOGE("Unexpected nullptr for codec information");
        mCodec->signalError(OMX_ErrorUndefined, UNKNOWN_ERROR);
        return false;
    }
    AString owner = (info->getOwnerName() == nullptr) ? "default" : info->getOwnerName();

    AString componentName;
    CHECK(msg->findString("componentName", &componentName));

    sp<CodecObserver> observer = new CodecObserver(notify);
    sp<IOMX> omx;
    sp<IOMXNode> omxNode;

    status_t err = NAME_NOT_FOUND;
    OMXClient client;
    if (client.connect(owner.c_str()) != OK) {
        mCodec->signalError(OMX_ErrorUndefined, NO_INIT);
        return false;
    }
    omx = client.interface();

    pid_t tid = gettid();
    int prevPriority = androidGetThreadPriority(tid);
    androidSetThreadPriority(tid, ANDROID_PRIORITY_FOREGROUND);
    err = omx->allocateNode(componentName.c_str(), observer, &omxNode);
    androidSetThreadPriority(tid, prevPriority);

    if (err != OK) {
        ALOGE("Unable to instantiate codec '%s' with err %#x.", componentName.c_str(), err);

        mCodec->signalError((OMX_ERRORTYPE)err, makeNoSideEffectStatus(err));
        return false;
    }

    mDeathNotifier = new DeathNotifier(new AMessage(kWhatOMXDied, mCodec));
    auto tOmxNode = omxNode->getHalInterface<IOmxNode>();
    if (tOmxNode && !tOmxNode->linkToDeath(mDeathNotifier, 0)) {
        mDeathNotifier.clear();
    }

    ++mCodec->mNodeGeneration;

    mCodec->mComponentName = componentName;
    mCodec->mRenderTracker.setComponentName(componentName);
    mCodec->mFlags = 0;

    if (componentName.endsWith(".secure")) {
        mCodec->mFlags |= kFlagIsSecure;
        mCodec->mFlags |= kFlagIsGrallocUsageProtected;
        mCodec->mFlags |= kFlagPushBlankBuffersToNativeWindowOnShutdown;
    }

    mCodec->mOMX = omx;
    mCodec->mOMXNode = omxNode;
    mCodec->mCallback->onComponentAllocated(mCodec->mComponentName.c_str());
    mCodec->changeState(mCodec->mLoadedState);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

ACodec::LoadedState::LoadedState(ACodec *codec)
    : BaseState(codec) {
}

void ACodec::LoadedState::stateEntered() {
    ALOGV("[%s] Now Loaded", mCodec->mComponentName.c_str());

    mCodec->mPortEOS[kPortIndexInput] =
        mCodec->mPortEOS[kPortIndexOutput] = false;

    mCodec->mInputEOSResult = OK;

    mCodec->mDequeueCounter = 0;
    mCodec->mMetadataBuffersToSubmit = 0;
    mCodec->mRepeatFrameDelayUs = -1LL;
    mCodec->mInputFormat.clear();
    mCodec->mOutputFormat.clear();
    mCodec->mBaseOutputFormat.clear();
    mCodec->mGraphicBufferSource.clear();

    if (mCodec->mShutdownInProgress) {
        bool keepComponentAllocated = mCodec->mKeepComponentAllocated;

        mCodec->mShutdownInProgress = false;
        mCodec->mKeepComponentAllocated = false;

        onShutdown(keepComponentAllocated);
    }
    mCodec->mExplicitShutdown = false;

    mCodec->processDeferredMessages();
}

void ACodec::LoadedState::onShutdown(bool keepComponentAllocated) {
    if (!keepComponentAllocated) {
        (void)mCodec->mOMXNode->freeNode();

        mCodec->changeState(mCodec->mUninitializedState);
    }

    if (mCodec->mExplicitShutdown) {
        if (keepComponentAllocated) {
            mCodec->mCallback->onStopCompleted();
        } else {
            mCodec->mCallback->onReleaseCompleted();
        }
        mCodec->mExplicitShutdown = false;
    }
}

bool ACodec::LoadedState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case ACodec::kWhatConfigureComponent:
        {
            onConfigureComponent(msg);
            handled = true;
            break;
        }

        case ACodec::kWhatCreateInputSurface:
        {
            onCreateInputSurface(msg);
            handled = true;
            break;
        }

        case ACodec::kWhatSetInputSurface:
        {
            onSetInputSurface(msg);
            handled = true;
            break;
        }

        case ACodec::kWhatStart:
        {
            onStart();
            handled = true;
            break;
        }

        case ACodec::kWhatShutdown:
        {
            int32_t keepComponentAllocated;
            CHECK(msg->findInt32(
                        "keepComponentAllocated", &keepComponentAllocated));

            mCodec->mExplicitShutdown = true;
            onShutdown(keepComponentAllocated);

            handled = true;
            break;
        }

        case ACodec::kWhatFlush:
        {
            mCodec->mCallback->onFlushCompleted();
            handled = true;
            break;
        }

        default:
            return BaseState::onMessageReceived(msg);
    }

    return handled;
}

bool ACodec::LoadedState::onConfigureComponent(
        const sp<AMessage> &msg) {
    ALOGV("onConfigureComponent");

    CHECK(mCodec->mOMXNode != NULL);

    status_t err = OK;
    AString mime;
    if (!msg->findString("mime", &mime)) {
        err = BAD_VALUE;
    } else {
        err = mCodec->configureCodec(mime.c_str(), msg);
    }
    if (err != OK) {
        ALOGE("[%s] configureCodec returning error %d",
              mCodec->mComponentName.c_str(), err);

        mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
        return false;
    }

    mCodec->mCallback->onComponentConfigured(mCodec->mInputFormat, mCodec->mOutputFormat);

    return true;
}

status_t ACodec::LoadedState::setupInputSurface() {
    if (mCodec->mGraphicBufferSource == NULL) {
        return BAD_VALUE;
    }

    android_dataspace dataSpace;
    status_t err =
        mCodec->setInitialColorAspectsForVideoEncoderSurfaceAndGetDataSpace(&dataSpace);
    if (err != OK) {
        ALOGE("Failed to get default data space");
        return err;
    }

    err = statusFromBinderStatus(
            mCodec->mGraphicBufferSource->configure(mCodec->mOMXNode, dataSpace));
    if (err != OK) {
        ALOGE("[%s] Unable to configure for node (err %d)",
              mCodec->mComponentName.c_str(), err);
        return err;
    }

    if (mCodec->mRepeatFrameDelayUs > 0LL) {
        err = statusFromBinderStatus(
                mCodec->mGraphicBufferSource->setRepeatPreviousFrameDelayUs(
                        mCodec->mRepeatFrameDelayUs));

        if (err != OK) {
            ALOGE("[%s] Unable to configure option to repeat previous "
                  "frames (err %d)",
                  mCodec->mComponentName.c_str(), err);
            return err;
        }
    }

    if (mCodec->mIsVideo && mCodec->mMaxPtsGapUs != 0LL) {
        OMX_PARAM_U32TYPE maxPtsGapParams;
        InitOMXParams(&maxPtsGapParams);
        maxPtsGapParams.nPortIndex = kPortIndexInput;
        maxPtsGapParams.nU32 = (uint32_t)mCodec->mMaxPtsGapUs;

        err = mCodec->mOMXNode->setParameter(
                (OMX_INDEXTYPE)OMX_IndexParamMaxFrameDurationForBitrateControl,
                &maxPtsGapParams, sizeof(maxPtsGapParams));

        if (err != OK) {
            ALOGE("[%s] Unable to configure max timestamp gap (err %d)",
                    mCodec->mComponentName.c_str(), err);
            return err;
        }
    }

    if (mCodec->mMaxFps > 0 || mCodec->mMaxPtsGapUs < 0) {
        err = statusFromBinderStatus(
                mCodec->mGraphicBufferSource->setMaxFps(mCodec->mMaxFps));

        if (err != OK) {
            ALOGE("[%s] Unable to configure max fps (err %d)",
                    mCodec->mComponentName.c_str(), err);
            return err;
        }
    }

    if (mCodec->mCaptureFps > 0. && mCodec->mFps > 0.) {
        err = statusFromBinderStatus(
                mCodec->mGraphicBufferSource->setTimeLapseConfig(
                        mCodec->mFps, mCodec->mCaptureFps));

        if (err != OK) {
            ALOGE("[%s] Unable to configure time lapse (err %d)",
                    mCodec->mComponentName.c_str(), err);
            return err;
        }
    }

    if (mCodec->mCreateInputBuffersSuspended) {
        err = statusFromBinderStatus(
                mCodec->mGraphicBufferSource->setSuspend(true, -1));

        if (err != OK) {
            ALOGE("[%s] Unable to configure option to suspend (err %d)",
                  mCodec->mComponentName.c_str(), err);
            return err;
        }
    }

    uint32_t usageBits;
    if (mCodec->mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamConsumerUsageBits,
            &usageBits, sizeof(usageBits)) == OK) {
        mCodec->mInputFormat->setInt32(
                "using-sw-read-often", !!(usageBits & GRALLOC_USAGE_SW_READ_OFTEN));
    }

    sp<ABuffer> colorAspectsBuffer;
    if (mCodec->mInputFormat->findBuffer("android._color-aspects", &colorAspectsBuffer)) {
        if (colorAspectsBuffer->size() != sizeof(ColorAspects)) {
            return INVALID_OPERATION;
        }

        err = statusFromBinderStatus(
                mCodec->mGraphicBufferSource->setColorAspects(ColorUtils::packToU32(
                        *(ColorAspects *)colorAspectsBuffer->base())));

        if (err != OK) {
            ALOGE("[%s] Unable to configure color aspects (err %d)",
                  mCodec->mComponentName.c_str(), err);
            return err;
        }
    }
    return OK;
}

void ACodec::LoadedState::onCreateInputSurface(
        const sp<AMessage> & /* msg */) {
    ALOGV("onCreateInputSurface");

    sp<IGraphicBufferProducer> bufferProducer;
    status_t err = mCodec->mOMX->createInputSurface(
            &bufferProducer, &mCodec->mGraphicBufferSource);

    if (err == OK) {
        err = setupInputSurface();
    }

    if (err == OK) {
        mCodec->mCallback->onInputSurfaceCreated(
                mCodec->mInputFormat,
                mCodec->mOutputFormat,
                new BufferProducerWrapper(bufferProducer));
    } else {
        // Can't use mCodec->signalError() here -- MediaCodec won't forward
        // the error through because it's in the "configured" state.  We
        // send a kWhatInputSurfaceCreated with an error value instead.
        ALOGE("[%s] onCreateInputSurface returning error %d",
                mCodec->mComponentName.c_str(), err);
        mCodec->mCallback->onInputSurfaceCreationFailed(err);
    }
}

void ACodec::LoadedState::onSetInputSurface(const sp<AMessage> &msg) {
    ALOGV("onSetInputSurface");

    sp<RefBase> obj;
    CHECK(msg->findObject("input-surface", &obj));
    if (obj == NULL) {
        ALOGE("[%s] NULL input surface", mCodec->mComponentName.c_str());
        mCodec->mCallback->onInputSurfaceDeclined(BAD_VALUE);
        return;
    }

    sp<PersistentSurface> surface = static_cast<PersistentSurface *>(obj.get());
    mCodec->mGraphicBufferSource = surface->getBufferSource();
    status_t err = setupInputSurface();

    if (err == OK) {
        mCodec->mCallback->onInputSurfaceAccepted(
                mCodec->mInputFormat, mCodec->mOutputFormat);
    } else {
        // Can't use mCodec->signalError() here -- MediaCodec won't forward
        // the error through because it's in the "configured" state.  We
        // send a kWhatInputSurfaceAccepted with an error value instead.
        ALOGE("[%s] onSetInputSurface returning error %d",
                mCodec->mComponentName.c_str(), err);
        mCodec->mCallback->onInputSurfaceDeclined(err);
    }
}

void ACodec::LoadedState::onStart() {
    ALOGV("onStart");

    status_t err = mCodec->mOMXNode->sendCommand(OMX_CommandStateSet, OMX_StateIdle);
    if (err != OK) {
        mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
    } else {
        mCodec->changeState(mCodec->mLoadedToIdleState);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::LoadedToIdleState::LoadedToIdleState(ACodec *codec)
    : BaseState(codec) {
}

void ACodec::LoadedToIdleState::stateEntered() {
    ALOGV("[%s] Now Loaded->Idle", mCodec->mComponentName.c_str());

    status_t err;
    if ((err = allocateBuffers()) != OK) {
        ALOGE("Failed to allocate buffers after transitioning to IDLE state "
             "(error 0x%08x)",
             err);

        mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));

        mCodec->mOMXNode->sendCommand(
                OMX_CommandStateSet, OMX_StateLoaded);
        if (mCodec->allYourBuffersAreBelongToUs(kPortIndexInput)) {
            mCodec->freeBuffersOnPort(kPortIndexInput);
        }
        if (mCodec->allYourBuffersAreBelongToUs(kPortIndexOutput)) {
            mCodec->freeBuffersOnPort(kPortIndexOutput);
        }

        mCodec->changeState(mCodec->mLoadedState);
    }
}

status_t ACodec::LoadedToIdleState::allocateBuffers() {
    status_t err = mCodec->allocateBuffersOnPort(kPortIndexInput);
    if (err != OK) {
        return err;
    }

    err = mCodec->allocateBuffersOnPort(kPortIndexOutput);
    if (err != OK) {
        return err;
    }

    mCodec->mCallback->onStartCompleted();

    return OK;
}

bool ACodec::LoadedToIdleState::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSetParameters:
        case kWhatShutdown:
        {
            mCodec->deferMessage(msg);
            return true;
        }

        case kWhatSignalEndOfInputStream:
        {
            mCodec->onSignalEndOfInputStream();
            return true;
        }

        case kWhatResume:
        {
            // We'll be active soon enough.
            return true;
        }

        case kWhatFlush:
        {
            // We haven't even started yet, so we're flushed alright...
            mCodec->mCallback->onFlushCompleted();
            return true;
        }

        default:
            return BaseState::onMessageReceived(msg);
    }
}

bool ACodec::LoadedToIdleState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventCmdComplete:
        {
            status_t err = OK;
            if (data1 != (OMX_U32)OMX_CommandStateSet
                    || data2 != (OMX_U32)OMX_StateIdle) {
                ALOGE("Unexpected command completion in LoadedToIdleState: %s(%u) %s(%u)",
                        asString((OMX_COMMANDTYPE)data1), data1,
                        asString((OMX_STATETYPE)data2), data2);
                err = FAILED_TRANSACTION;
            }

            if (err == OK) {
                err = mCodec->mOMXNode->sendCommand(
                    OMX_CommandStateSet, OMX_StateExecuting);
            }

            if (err != OK) {
                mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
            } else {
                mCodec->changeState(mCodec->mIdleToExecutingState);
            }

            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::IdleToExecutingState::IdleToExecutingState(ACodec *codec)
    : BaseState(codec) {
}

void ACodec::IdleToExecutingState::stateEntered() {
    ALOGV("[%s] Now Idle->Executing", mCodec->mComponentName.c_str());
}

bool ACodec::IdleToExecutingState::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatSetParameters:
        case kWhatShutdown:
        {
            mCodec->deferMessage(msg);
            return true;
        }

        case kWhatResume:
        {
            // We'll be active soon enough.
            return true;
        }

        case kWhatFlush:
        {
            // We haven't even started yet, so we're flushed alright...
            mCodec->mCallback->onFlushCompleted();
            return true;
        }

        case kWhatSignalEndOfInputStream:
        {
            mCodec->onSignalEndOfInputStream();
            return true;
        }

        default:
            return BaseState::onMessageReceived(msg);
    }
}

bool ACodec::IdleToExecutingState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventCmdComplete:
        {
            if (data1 != (OMX_U32)OMX_CommandStateSet
                    || data2 != (OMX_U32)OMX_StateExecuting) {
                ALOGE("Unexpected command completion in IdleToExecutingState: %s(%u) %s(%u)",
                        asString((OMX_COMMANDTYPE)data1), data1,
                        asString((OMX_STATETYPE)data2), data2);
                mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
                return true;
            }

            mCodec->mExecutingState->resume();
            mCodec->changeState(mCodec->mExecutingState);

            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::ExecutingState::ExecutingState(ACodec *codec)
    : BaseState(codec),
      mActive(false) {
}

ACodec::BaseState::PortMode ACodec::ExecutingState::getPortMode(
        OMX_U32 /* portIndex */) {
    return RESUBMIT_BUFFERS;
}

void ACodec::ExecutingState::submitOutputMetaBuffers() {
    // submit as many buffers as there are input buffers with the codec
    // in case we are in port reconfiguring
    for (size_t i = 0; i < mCodec->mBuffers[kPortIndexInput].size(); ++i) {
        BufferInfo *info = &mCodec->mBuffers[kPortIndexInput].editItemAt(i);

        if (info->mStatus == BufferInfo::OWNED_BY_COMPONENT) {
            if (mCodec->submitOutputMetadataBuffer() != OK)
                break;
        }
    }

    // *** NOTE: THE FOLLOWING WORKAROUND WILL BE REMOVED ***
    mCodec->signalSubmitOutputMetadataBufferIfEOS_workaround();
}

void ACodec::ExecutingState::submitRegularOutputBuffers() {
    bool failed = false;
    for (size_t i = 0; i < mCodec->mBuffers[kPortIndexOutput].size(); ++i) {
        BufferInfo *info = &mCodec->mBuffers[kPortIndexOutput].editItemAt(i);

        if (mCodec->mNativeWindow != NULL) {
            if (info->mStatus != BufferInfo::OWNED_BY_US
                    && info->mStatus != BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                ALOGE("buffers should be owned by us or the surface");
                failed = true;
                break;
            }

            if (info->mStatus == BufferInfo::OWNED_BY_NATIVE_WINDOW) {
                continue;
            }
        } else {
            if (info->mStatus != BufferInfo::OWNED_BY_US) {
                ALOGE("buffers should be owned by us");
                failed = true;
                break;
            }
        }

        ALOGV("[%s] calling fillBuffer %u", mCodec->mComponentName.c_str(), info->mBufferID);

        info->checkWriteFence("submitRegularOutputBuffers");
        status_t err = mCodec->fillBuffer(info);
        if (err != OK) {
            failed = true;
            break;
        }
    }

    if (failed) {
        mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
    }
}

void ACodec::ExecutingState::submitOutputBuffers() {
    submitRegularOutputBuffers();
    if (mCodec->storingMetadataInDecodedBuffers()) {
        submitOutputMetaBuffers();
    }
}

void ACodec::ExecutingState::resume() {
    if (mActive) {
        ALOGV("[%s] We're already active, no need to resume.", mCodec->mComponentName.c_str());
        return;
    }

    submitOutputBuffers();

    // Post all available input buffers
    if (mCodec->mBuffers[kPortIndexInput].size() == 0u) {
        ALOGW("[%s] we don't have any input buffers to resume", mCodec->mComponentName.c_str());
    }

    for (size_t i = 0; i < mCodec->mBuffers[kPortIndexInput].size(); i++) {
        BufferInfo *info = &mCodec->mBuffers[kPortIndexInput].editItemAt(i);
        if (info->mStatus == BufferInfo::OWNED_BY_US) {
            postFillThisBuffer(info);
        }
    }

    mActive = true;
}

void ACodec::ExecutingState::stateEntered() {
    ALOGV("[%s] Now Executing", mCodec->mComponentName.c_str());
    mCodec->mRenderTracker.clear(systemTime(CLOCK_MONOTONIC));
    mCodec->processDeferredMessages();
}

bool ACodec::ExecutingState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case kWhatShutdown:
        {
            int32_t keepComponentAllocated;
            CHECK(msg->findInt32(
                        "keepComponentAllocated", &keepComponentAllocated));

            mCodec->mShutdownInProgress = true;
            mCodec->mExplicitShutdown = true;
            mCodec->mKeepComponentAllocated = keepComponentAllocated;

            mActive = false;

            status_t err = mCodec->mOMXNode->sendCommand(
                    OMX_CommandStateSet, OMX_StateIdle);
            if (err != OK) {
                if (keepComponentAllocated) {
                    mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
                }
                // TODO: do some recovery here.
            } else {
                mCodec->changeState(mCodec->mExecutingToIdleState);
            }

            handled = true;
            break;
        }

        case kWhatFlush:
        {
            ALOGV("[%s] ExecutingState flushing now "
                 "(codec owns %zu/%zu input, %zu/%zu output).",
                    mCodec->mComponentName.c_str(),
                    mCodec->countBuffersOwnedByComponent(kPortIndexInput),
                    mCodec->mBuffers[kPortIndexInput].size(),
                    mCodec->countBuffersOwnedByComponent(kPortIndexOutput),
                    mCodec->mBuffers[kPortIndexOutput].size());

            mActive = false;

            status_t err = mCodec->mOMXNode->sendCommand(OMX_CommandFlush, OMX_ALL);
            if (err != OK) {
                mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
            } else {
                mCodec->changeState(mCodec->mFlushingState);
            }

            handled = true;
            break;
        }

        case kWhatResume:
        {
            resume();

            handled = true;
            break;
        }

        case kWhatRequestIDRFrame:
        {
            status_t err = mCodec->requestIDRFrame();
            if (err != OK) {
                ALOGW("Requesting an IDR frame failed.");
            }

            handled = true;
            break;
        }

        case kWhatSetParameters:
        {
            sp<AMessage> params;
            CHECK(msg->findMessage("params", &params));

            status_t err = mCodec->setParameters(params);

            sp<AMessage> reply;
            if (msg->findMessage("reply", &reply)) {
                reply->setInt32("err", err);
                reply->post();
            }

            handled = true;
            break;
        }

        case ACodec::kWhatSignalEndOfInputStream:
        {
            mCodec->onSignalEndOfInputStream();
            handled = true;
            break;
        }

        // *** NOTE: THE FOLLOWING WORKAROUND WILL BE REMOVED ***
        case kWhatSubmitOutputMetadataBufferIfEOS:
        {
            if (mCodec->mPortEOS[kPortIndexInput] &&
                    !mCodec->mPortEOS[kPortIndexOutput]) {
                status_t err = mCodec->submitOutputMetadataBuffer();
                if (err == OK) {
                    mCodec->signalSubmitOutputMetadataBufferIfEOS_workaround();
                }
            }
            return true;
        }

        default:
            handled = BaseState::onMessageReceived(msg);
            break;
    }

    return handled;
}

status_t ACodec::setParameters(const sp<AMessage> &params) {
    int32_t videoBitrate;
    if (params->findInt32("video-bitrate", &videoBitrate)) {
        OMX_VIDEO_CONFIG_BITRATETYPE configParams;
        InitOMXParams(&configParams);
        configParams.nPortIndex = kPortIndexOutput;
        configParams.nEncodeBitrate = videoBitrate;

        status_t err = mOMXNode->setConfig(
                OMX_IndexConfigVideoBitrate,
                &configParams,
                sizeof(configParams));

        if (err != OK) {
            ALOGE("setConfig(OMX_IndexConfigVideoBitrate, %d) failed w/ err %d",
                   videoBitrate, err);

            return err;
        }
    }

    int64_t timeOffsetUs;
    if (params->findInt64(PARAMETER_KEY_OFFSET_TIME, &timeOffsetUs)) {
        if (mGraphicBufferSource == NULL) {
            ALOGE("[%s] Invalid to set input buffer time offset without surface",
                    mComponentName.c_str());
            return INVALID_OPERATION;
        }

        status_t err = statusFromBinderStatus(
                mGraphicBufferSource->setTimeOffsetUs(timeOffsetUs));

        if (err != OK) {
            ALOGE("[%s] Unable to set input buffer time offset (err %d)",
                mComponentName.c_str(),
                err);
            return err;
        }
    }

    int64_t skipFramesBeforeUs;
    if (params->findInt64("skip-frames-before", &skipFramesBeforeUs)) {
        if (mGraphicBufferSource == NULL) {
            ALOGE("[%s] Invalid to set start time without surface",
                    mComponentName.c_str());
            return INVALID_OPERATION;
        }

        status_t err = statusFromBinderStatus(
                mGraphicBufferSource->setStartTimeUs(skipFramesBeforeUs));

        if (err != OK) {
            ALOGE("Failed to set parameter 'skip-frames-before' (err %d)", err);
            return err;
        }
    }

    int32_t dropInputFrames;
    if (params->findInt32(PARAMETER_KEY_SUSPEND, &dropInputFrames)) {
        if (mGraphicBufferSource == NULL) {
            ALOGE("[%s] Invalid to set suspend without surface",
                    mComponentName.c_str());
            return INVALID_OPERATION;
        }

        int64_t suspendStartTimeUs = -1;
        (void) params->findInt64(PARAMETER_KEY_SUSPEND_TIME, &suspendStartTimeUs);
        status_t err = statusFromBinderStatus(
                mGraphicBufferSource->setSuspend(dropInputFrames != 0, suspendStartTimeUs));

        if (err != OK) {
            ALOGE("Failed to set parameter 'drop-input-frames' (err %d)", err);
            return err;
        }
    }

    int64_t stopTimeUs;
    if (params->findInt64("stop-time-us", &stopTimeUs)) {
        if (mGraphicBufferSource == NULL) {
            ALOGE("[%s] Invalid to set stop time without surface",
                    mComponentName.c_str());
            return INVALID_OPERATION;
        }
        status_t err = statusFromBinderStatus(
                mGraphicBufferSource->setStopTimeUs(stopTimeUs));

        if (err != OK) {
            ALOGE("Failed to set parameter 'stop-time-us' (err %d)", err);
            return err;
        }

        int64_t stopTimeOffsetUs;
        err = statusFromBinderStatus(
                mGraphicBufferSource->getStopTimeOffsetUs(&stopTimeOffsetUs));

        if (err != OK) {
            ALOGE("Failed to get stop time offset (err %d)", err);
            return err;
        }
        mInputFormat->setInt64("android._stop-time-offset-us", stopTimeOffsetUs);
    }

    int32_t dummy;
    if (params->findInt32("request-sync", &dummy)) {
        status_t err = requestIDRFrame();

        if (err != OK) {
            ALOGE("Requesting a sync frame failed w/ err %d", err);
            return err;
        }
    }

    int32_t rateInt = -1;
    float rateFloat = -1;
    if (!params->findFloat("operating-rate", &rateFloat)) {
        params->findInt32("operating-rate", &rateInt);
        rateFloat = (float) rateInt; // 16MHz (FLINTMAX) is OK for upper bound.
    }
    if (rateFloat > 0) {
        status_t err = setOperatingRate(rateFloat, mIsVideo);
        if (err != OK) {
            ALOGI("Failed to set parameter 'operating-rate' (err %d)", err);
        }
    }

    int32_t intraRefreshPeriod = 0;
    if (params->findInt32("intra-refresh-period", &intraRefreshPeriod)
            && intraRefreshPeriod > 0) {
        status_t err = setIntraRefreshPeriod(intraRefreshPeriod, false);
        if (err != OK) {
            ALOGI("[%s] failed setIntraRefreshPeriod. Failure is fine since this key is optional",
                    mComponentName.c_str());
            err = OK;
        }
    }

    int32_t latency = 0;
    if (params->findInt32("latency", &latency) && latency > 0) {
        status_t err = setLatency(latency);
        if (err != OK) {
            ALOGI("[%s] failed setLatency. Failure is fine since this key is optional",
                    mComponentName.c_str());
            err = OK;
        }
    }

    int32_t presentationId = -1;
    if (params->findInt32("audio-presentation-presentation-id", &presentationId)) {
        int32_t programId = -1;
        params->findInt32("audio-presentation-program-id", &programId);
        status_t err = setAudioPresentation(presentationId, programId);
        if (err != OK) {
            ALOGI("[%s] failed setAudioPresentation. Failure is fine since this key is optional",
                    mComponentName.c_str());
            err = OK;
        }
    }

    sp<ABuffer> hdr10PlusInfo;
    if (params->findBuffer("hdr10-plus-info", &hdr10PlusInfo)
            && hdr10PlusInfo != nullptr && hdr10PlusInfo->size() > 0) {
        (void)setHdr10PlusInfo(hdr10PlusInfo);
    }

    // Ignore errors as failure is expected for codecs that aren't video encoders.
    (void)configureTemporalLayers(params, false /* inConfigure */, mOutputFormat);

    return setVendorParameters(params);
}

status_t ACodec::setHdr10PlusInfo(const sp<ABuffer> &hdr10PlusInfo) {
    if (mDescribeHDR10PlusInfoIndex == 0) {
        ALOGE("setHdr10PlusInfo: does not support DescribeHDR10PlusInfoParams");
        return ERROR_UNSUPPORTED;
    }
    size_t newSize = sizeof(DescribeHDR10PlusInfoParams) + hdr10PlusInfo->size() - 1;
    if (mHdr10PlusScratchBuffer == nullptr ||
            newSize > mHdr10PlusScratchBuffer->size()) {
        mHdr10PlusScratchBuffer = new ABuffer(newSize);
    }
    DescribeHDR10PlusInfoParams *config =
            (DescribeHDR10PlusInfoParams *)mHdr10PlusScratchBuffer->data();
    InitOMXParams(config);
    config->nPortIndex = 0;
    config->nSize = newSize;
    config->nParamSize = hdr10PlusInfo->size();
    config->nParamSizeUsed = hdr10PlusInfo->size();
    memcpy(config->nValue, hdr10PlusInfo->data(), hdr10PlusInfo->size());
    status_t err = mOMXNode->setConfig(
            (OMX_INDEXTYPE)mDescribeHDR10PlusInfoIndex,
            config, config->nSize);
    if (err != OK) {
        ALOGE("failed to set DescribeHDR10PlusInfoParams (err %d)", err);
    }
    return OK;
}

// Removes trailing tags matching |tag| from |key| (e.g. a settings name). |minLength| specifies
// the minimum number of characters to keep in |key| (even if it has trailing tags).
// (Used to remove trailing 'value' tags in settings names, e.g. to normalize
// 'vendor.settingsX.value' to 'vendor.settingsX')
static void removeTrailingTags(char *key, size_t minLength, const char *tag) {
    size_t length = strlen(key);
    size_t tagLength = strlen(tag);
    while (length > minLength + tagLength
            && !strcmp(key + length - tagLength, tag)
            && key[length - tagLength - 1] == '.') {
        length -= tagLength + 1;
        key[length] = '\0';
    }
}

/**
 * Struct encompassing a vendor extension config structure and a potential error status (in case
 * the structure is null). Used to iterate through vendor extensions.
 */
struct VendorExtension {
    OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *config;  // structure does not own config
    status_t status;

    // create based on an error status
    VendorExtension(status_t s_ = NO_INIT) : config(nullptr), status(s_) { }

    // create based on a successfully retrieved config structure
    VendorExtension(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *c_) : config(c_), status(OK) { }
};

// class VendorExtensions;
/**
 * Forward iterator to enumerate vendor extensions supported by an OMX component.
 */
class VendorExtensionIterator {
//private:
    static constexpr size_t kLastIndex = ~(size_t)0; // last index marker

    sp<IOMXNode> mNode;                   // component
    size_t mIndex;                        // current android extension index
    std::unique_ptr<uint8_t[]> mBacking;  // current extension's backing
    VendorExtension mCurrent;             // current extension

    VendorExtensionIterator(const sp<IOMXNode> &node, size_t index)
        : mNode(node),
          mIndex(index) {
        mCurrent = retrieve();
    }

    friend class VendorExtensions;

public:
    // copy constructor
    VendorExtensionIterator(const VendorExtensionIterator &it)
        : VendorExtensionIterator(it.mNode, it.mIndex) { }

    // retrieves the current extension pointed to by this iterator
    VendorExtension retrieve() {
        if (mIndex == kLastIndex) {
            return NO_INIT;
        }

        // try with one param first, then retry if extension needs more than 1 param
        for (size_t paramSizeUsed = 1;; ) {
            if (paramSizeUsed > OMX_MAX_ANDROID_VENDOR_PARAMCOUNT) {
                return BAD_VALUE; // this prevents overflow in the following formula
            }

            size_t size = sizeof(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE) +
                (paramSizeUsed - 1) * sizeof(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE::param);
            mBacking.reset(new uint8_t[size]);
            if (!mBacking) {
                return NO_MEMORY;
            }

            OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *config =
                reinterpret_cast<OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *>(mBacking.get());

            InitOMXParams(config);
            config->nSize = size;
            config->nIndex = mIndex;
            config->nParamSizeUsed = paramSizeUsed;
            status_t err = mNode->getConfig(
                    (OMX_INDEXTYPE)OMX_IndexConfigAndroidVendorExtension, config, size);
            if (err == OK && config->nParamCount > paramSizeUsed && paramSizeUsed == 1) {
                // reallocate if we need a bigger config
                paramSizeUsed = config->nParamCount;
                continue;
            } else if (err == NOT_ENOUGH_DATA
                   || (err != OK && mIndex == 0)) {
                // stop iterator on no-more signal, or if index is not at all supported
                mIndex = kLastIndex;
                return NO_INIT;
            } else if (err != OK) {
                return err;
            } else if (paramSizeUsed != config->nParamSizeUsed) {
                return BAD_VALUE; // component shall not modify size of nParam
            }

            return config;
        }
    }

    // returns extension pointed to by this iterator
    VendorExtension operator*() {
        return mCurrent;
    }

    // prefix increment: move to next extension
    VendorExtensionIterator &operator++() { // prefix
        if (mIndex != kLastIndex) {
            ++mIndex;
            mCurrent = retrieve();
        }
        return *this;
    }

    // iterator equality operators
    bool operator==(const VendorExtensionIterator &o) {
        return mNode == o.mNode && mIndex == o.mIndex;
    }

    bool operator!=(const VendorExtensionIterator &o) {
        return !(*this == o);
    }
};

/**
 * Iterable container for vendor extensions provided by a component
 */
class VendorExtensions {
//private:
    sp<IOMXNode> mNode;

public:
    VendorExtensions(const sp<IOMXNode> &node)
        : mNode(node) {
    }

    VendorExtensionIterator begin() {
        return VendorExtensionIterator(mNode, 0);
    }

    VendorExtensionIterator end() {
        return VendorExtensionIterator(mNode, VendorExtensionIterator::kLastIndex);
    }
};

status_t ACodec::setVendorParameters(const sp<AMessage> &params) {
    std::map<std::string, std::string> vendorKeys; // maps reduced name to actual name
    constexpr char prefix[] = "vendor.";
    constexpr size_t prefixLength = sizeof(prefix) - 1;
    // longest possible vendor param name
    char reducedKey[OMX_MAX_STRINGNAME_SIZE + OMX_MAX_STRINGVALUE_SIZE];

    // identify all vendor keys to speed up search later and to detect vendor keys
    for (size_t i = params->countEntries(); i; --i) {
        AMessage::Type keyType;
        const char* key = params->getEntryNameAt(i - 1, &keyType);
        if (key != nullptr && !strncmp(key, prefix, prefixLength)
                // it is safe to limit format keys to the max vendor param size as we only
                // shorten parameter names by removing any trailing 'value' tags, and we
                // already remove the vendor prefix.
                && strlen(key + prefixLength) < sizeof(reducedKey)
                && (keyType == AMessage::kTypeInt32
                        || keyType == AMessage::kTypeInt64
                        || keyType == AMessage::kTypeString)) {
            strcpy(reducedKey, key + prefixLength);
            removeTrailingTags(reducedKey, 0, "value");
            auto existingKey = vendorKeys.find(reducedKey);
            if (existingKey != vendorKeys.end()) {
                ALOGW("[%s] vendor parameter '%s' aliases parameter '%s'",
                        mComponentName.c_str(), key, existingKey->second.c_str());
                // ignore for now
            }
            vendorKeys.emplace(reducedKey, key);
        }
    }

    // don't bother component if we don't have vendor extensions as they may not have implemented
    // the android vendor extension support, which will lead to unnecessary OMX failure logs.
    if (vendorKeys.empty()) {
        return OK;
    }

    char key[sizeof(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE::cName) +
            sizeof(OMX_CONFIG_ANDROID_VENDOR_PARAMTYPE::cKey)];

    status_t finalError = OK;

    // don't try again if component does not have vendor extensions
    if (mVendorExtensionsStatus == kExtensionsNone) {
        return OK;
    }

    for (VendorExtension ext : VendorExtensions(mOMXNode)) {
        OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *config = ext.config;
        if (config == nullptr) {
            return ext.status;
        }

        mVendorExtensionsStatus = kExtensionsExist;

        config->cName[sizeof(config->cName) - 1] = '\0'; // null-terminate name
        strcpy(key, (const char *)config->cName);
        size_t nameLength = strlen(key);
        key[nameLength] = '.';

        // don't set vendor extension if client has not provided any of its parameters
        // or if client simply unsets parameters that are already unset
        bool needToSet = false;
        for (size_t paramIndex = 0; paramIndex < config->nParamCount; ++paramIndex) {
            // null-terminate param key
            config->param[paramIndex].cKey[sizeof(config->param[0].cKey) - 1] = '\0';
            strcpy(key + nameLength + 1, (const char *)config->param[paramIndex].cKey);
            removeTrailingTags(key, nameLength, "value");
            auto existingKey = vendorKeys.find(key);

            // don't touch (e.g. change) parameters that are not specified by client
            if (existingKey == vendorKeys.end()) {
                continue;
            }

            bool wasSet = config->param[paramIndex].bSet;
            switch (config->param[paramIndex].eValueType) {
            case OMX_AndroidVendorValueInt32:
            {
                int32_t value;
                config->param[paramIndex].bSet =
                    (OMX_BOOL)params->findInt32(existingKey->second.c_str(), &value);
                if (config->param[paramIndex].bSet) {
                    config->param[paramIndex].nInt32 = value;
                }
                break;
            }
            case OMX_AndroidVendorValueInt64:
            {
                int64_t value;
                config->param[paramIndex].bSet =
                    (OMX_BOOL)params->findAsInt64(existingKey->second.c_str(), &value);
                if (config->param[paramIndex].bSet) {
                    config->param[paramIndex].nInt64 = value;
                }
                break;
            }
            case OMX_AndroidVendorValueString:
            {
                AString value;
                config->param[paramIndex].bSet =
                    (OMX_BOOL)params->findString(existingKey->second.c_str(), &value);
                if (config->param[paramIndex].bSet) {
                    size_t dstSize = sizeof(config->param[paramIndex].cString);
                    strncpy((char *)config->param[paramIndex].cString, value.c_str(), dstSize - 1);
                    // null terminate value
                    config->param[paramIndex].cString[dstSize - 1] = '\0';
                }
                break;
            }
            default:
                ALOGW("[%s] vendor parameter '%s' is not a supported value",
                        mComponentName.c_str(), key);
                continue;
            }
            if (config->param[paramIndex].bSet || wasSet) {
                needToSet = true;
            }
        }

        if (needToSet) {
            status_t err = mOMXNode->setConfig(
                    (OMX_INDEXTYPE)OMX_IndexConfigAndroidVendorExtension,
                    config, config->nSize);
            if (err != OK) {
                key[nameLength] = '\0';
                ALOGW("[%s] failed to set vendor extension '%s'", mComponentName.c_str(), key);
                // try to set each extension, and return first failure
                if (finalError == OK) {
                    finalError = err;
                }
            }
        }
    }

    if (mVendorExtensionsStatus == kExtensionsUnchecked) {
        mVendorExtensionsStatus = kExtensionsNone;
    }

    return finalError;
}

status_t ACodec::getVendorParameters(OMX_U32 portIndex, sp<AMessage> &format) {
    constexpr char prefix[] = "vendor.";
    constexpr size_t prefixLength = sizeof(prefix) - 1;
    char key[sizeof(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE::cName) +
            sizeof(OMX_CONFIG_ANDROID_VENDOR_PARAMTYPE::cKey) + prefixLength];
    strcpy(key, prefix);

    // don't try again if component does not have vendor extensions
    if (mVendorExtensionsStatus == kExtensionsNone) {
        return OK;
    }

    for (VendorExtension ext : VendorExtensions(mOMXNode)) {
        OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *config = ext.config;
        if (config == nullptr) {
            return ext.status;
        }

        mVendorExtensionsStatus = kExtensionsExist;

        if (config->eDir != (portIndex == kPortIndexInput ? OMX_DirInput : OMX_DirOutput)) {
            continue;
        }

        config->cName[sizeof(config->cName) - 1] = '\0'; // null-terminate name
        strcpy(key + prefixLength, (const char *)config->cName);
        size_t nameLength = strlen(key);
        key[nameLength] = '.';

        for (size_t paramIndex = 0; paramIndex < config->nParamCount; ++paramIndex) {
            // null-terminate param key
            config->param[paramIndex].cKey[sizeof(config->param[0].cKey) - 1] = '\0';
            strcpy(key + nameLength + 1, (const char *)config->param[paramIndex].cKey);
            removeTrailingTags(key, nameLength, "value");
            if (config->param[paramIndex].bSet) {
                switch (config->param[paramIndex].eValueType) {
                case OMX_AndroidVendorValueInt32:
                {
                    format->setInt32(key, config->param[paramIndex].nInt32);
                    break;
                }
                case OMX_AndroidVendorValueInt64:
                {
                    format->setInt64(key, config->param[paramIndex].nInt64);
                    break;
                }
                case OMX_AndroidVendorValueString:
                {
                    config->param[paramIndex].cString[OMX_MAX_STRINGVALUE_SIZE - 1] = '\0';
                    format->setString(key, (const char *)config->param[paramIndex].cString);
                    break;
                }
                default:
                    ALOGW("vendor parameter %s is not a supported value", key);
                    continue;
                }
            }
        }
    }

    if (mVendorExtensionsStatus == kExtensionsUnchecked) {
        mVendorExtensionsStatus = kExtensionsNone;
    }

    return OK;
}

void ACodec::onSignalEndOfInputStream() {
    status_t err = INVALID_OPERATION;
    if (mGraphicBufferSource != NULL) {
        err = statusFromBinderStatus(mGraphicBufferSource->signalEndOfInputStream());
    }
    mCallback->onSignaledInputEOS(err);
}

void ACodec::forceStateTransition(int generation) {
    if (generation != mStateGeneration) {
        ALOGV("Ignoring stale force state transition message: #%d (now #%d)",
                generation, mStateGeneration);
        return;
    }
    ALOGE("State machine stuck");
    // Error must have already been signalled to the client.

    // Deferred messages will be handled at LoadedState at the end of the
    // transition.
    mShutdownInProgress = true;
    // No shutdown complete callback at the end of the transition.
    mExplicitShutdown = false;
    mKeepComponentAllocated = true;

    status_t err = mOMXNode->sendCommand(OMX_CommandStateSet, OMX_StateIdle);
    if (err != OK) {
        // TODO: do some recovery here.
    } else {
        changeState(mExecutingToIdleState);
    }
}

bool ACodec::ExecutingState::onOMXFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano) {
    mCodec->onFrameRendered(mediaTimeUs, systemNano);
    return true;
}

bool ACodec::ExecutingState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventPortSettingsChanged:
        {
            CHECK_EQ(data1, (OMX_U32)kPortIndexOutput);

            mCodec->onOutputFormatChanged();

            if (data2 == 0 || data2 == OMX_IndexParamPortDefinition) {
                mCodec->mMetadataBuffersToSubmit = 0;
                CHECK_EQ(mCodec->mOMXNode->sendCommand(
                            OMX_CommandPortDisable, kPortIndexOutput),
                         (status_t)OK);

                mCodec->freeOutputBuffersNotOwnedByComponent();

                mCodec->changeState(mCodec->mOutputPortSettingsChangedState);
            } else if (data2 != OMX_IndexConfigCommonOutputCrop
                    && data2 != OMX_IndexConfigAndroidIntraRefresh) {
                ALOGV("[%s] OMX_EventPortSettingsChanged 0x%08x",
                     mCodec->mComponentName.c_str(), data2);
            }

            return true;
        }

        case OMX_EventConfigUpdate:
        {
            CHECK_EQ(data1, (OMX_U32)kPortIndexOutput);

            mCodec->onConfigUpdate((OMX_INDEXTYPE)data2);

            return true;
        }

        case OMX_EventBufferFlag:
        {
            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::OutputPortSettingsChangedState::OutputPortSettingsChangedState(
        ACodec *codec)
    : BaseState(codec) {
}

ACodec::BaseState::PortMode ACodec::OutputPortSettingsChangedState::getPortMode(
        OMX_U32 portIndex) {
    if (portIndex == kPortIndexOutput) {
        return FREE_BUFFERS;
    }

    CHECK_EQ(portIndex, (OMX_U32)kPortIndexInput);

    return RESUBMIT_BUFFERS;
}

bool ACodec::OutputPortSettingsChangedState::onMessageReceived(
        const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case kWhatFlush:
        case kWhatShutdown: {
            if (mCodec->mFatalError) {
                sp<AMessage> msg = new AMessage(ACodec::kWhatForceStateTransition, mCodec);
                msg->setInt32("generation", mCodec->mStateGeneration);
                msg->post(3000000);
            }
            FALLTHROUGH_INTENDED;
        }
        case kWhatResume:
        case kWhatSetParameters:
        {
            if (msg->what() == kWhatResume) {
                ALOGV("[%s] Deferring resume", mCodec->mComponentName.c_str());
            }

            mCodec->deferMessage(msg);
            handled = true;
            break;
        }

        case kWhatForceStateTransition:
        {
            int32_t generation = 0;
            CHECK(msg->findInt32("generation", &generation));
            mCodec->forceStateTransition(generation);

            handled = true;
            break;
        }

        case kWhatCheckIfStuck:
        {
            int32_t generation = 0;
            CHECK(msg->findInt32("generation", &generation));
            if (generation == mCodec->mStateGeneration) {
                mCodec->signalError(OMX_ErrorUndefined, TIMED_OUT);
            }

            handled = true;
            break;
        }

        default:
            handled = BaseState::onMessageReceived(msg);
            break;
    }

    return handled;
}

void ACodec::OutputPortSettingsChangedState::stateEntered() {
    ALOGV("[%s] Now handling output port settings change",
         mCodec->mComponentName.c_str());

    // If we haven't transitioned after 3 seconds, we're probably stuck.
    sp<AMessage> msg = new AMessage(ACodec::kWhatCheckIfStuck, mCodec);
    msg->setInt32("generation", mCodec->mStateGeneration);
    msg->post(3000000);
}

bool ACodec::OutputPortSettingsChangedState::onOMXFrameRendered(
        int64_t mediaTimeUs, nsecs_t systemNano) {
    mCodec->onFrameRendered(mediaTimeUs, systemNano);
    return true;
}

bool ACodec::OutputPortSettingsChangedState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventCmdComplete:
        {
            if (data1 == (OMX_U32)OMX_CommandPortDisable) {
                if (data2 != (OMX_U32)kPortIndexOutput) {
                    ALOGW("ignoring EventCmdComplete CommandPortDisable for port %u", data2);
                    return false;
                }

                ALOGV("[%s] Output port now disabled.", mCodec->mComponentName.c_str());

                status_t err = OK;
                if (!mCodec->mBuffers[kPortIndexOutput].isEmpty()) {
                    ALOGE("disabled port should be empty, but has %zu buffers",
                            mCodec->mBuffers[kPortIndexOutput].size());
                    err = FAILED_TRANSACTION;
                } else {
                    mCodec->mAllocator[kPortIndexOutput].clear();
                }

                if (err == OK) {
                    err = mCodec->mOMXNode->sendCommand(
                            OMX_CommandPortEnable, kPortIndexOutput);
                }

                // Clear the RenderQueue in which queued GraphicBuffers hold the
                // actual buffer references in order to free them early.
                mCodec->mRenderTracker.clear(systemTime(CLOCK_MONOTONIC));

                if (err == OK) {
                    err = mCodec->allocateBuffersOnPort(kPortIndexOutput);
                    ALOGE_IF(err != OK, "Failed to allocate output port buffers after port "
                            "reconfiguration: (%d)", err);
                    mCodec->mCallback->onOutputBuffersChanged();
                }

                if (err != OK) {
                    mCodec->signalError(OMX_ErrorUndefined, makeNoSideEffectStatus(err));
                    ALOGE("Error occurred while disabling the output port");
                }

                return true;
            } else if (data1 == (OMX_U32)OMX_CommandPortEnable) {
                if (data2 != (OMX_U32)kPortIndexOutput) {
                    ALOGW("ignoring EventCmdComplete OMX_CommandPortEnable for port %u", data2);
                    return false;
                }

                ALOGV("[%s] Output port now reenabled.", mCodec->mComponentName.c_str());

                if (mCodec->mExecutingState->active()) {
                    mCodec->mExecutingState->submitOutputBuffers();
                }

                mCodec->changeState(mCodec->mExecutingState);

                return true;
            }

            return false;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::ExecutingToIdleState::ExecutingToIdleState(ACodec *codec)
    : BaseState(codec),
      mComponentNowIdle(false) {
}

bool ACodec::ExecutingToIdleState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case kWhatFlush:
        {
            // Don't send me a flush request if you previously wanted me
            // to shutdown.
            ALOGW("Ignoring flush request in ExecutingToIdleState");
            break;
        }

        case kWhatShutdown:
        {
            mCodec->deferMessage(msg);
            handled = true;
            break;
        }

        default:
            handled = BaseState::onMessageReceived(msg);
            break;
    }

    return handled;
}

void ACodec::ExecutingToIdleState::stateEntered() {
    ALOGV("[%s] Now Executing->Idle", mCodec->mComponentName.c_str());

    mComponentNowIdle = false;
    mCodec->mLastOutputFormat.clear();
}

bool ACodec::ExecutingToIdleState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventCmdComplete:
        {
            if (data1 != (OMX_U32)OMX_CommandStateSet
                    || data2 != (OMX_U32)OMX_StateIdle) {
                ALOGE("Unexpected command completion in ExecutingToIdleState: %s(%u) %s(%u)",
                        asString((OMX_COMMANDTYPE)data1), data1,
                        asString((OMX_STATETYPE)data2), data2);
                mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
                return true;
            }

            mComponentNowIdle = true;

            changeStateIfWeOwnAllBuffers();

            return true;
        }

        case OMX_EventPortSettingsChanged:
        case OMX_EventBufferFlag:
        {
            // We're shutting down and don't care about this anymore.
            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

void ACodec::ExecutingToIdleState::changeStateIfWeOwnAllBuffers() {
    if (mComponentNowIdle && mCodec->allYourBuffersAreBelongToUs()) {
        status_t err = mCodec->mOMXNode->sendCommand(
                OMX_CommandStateSet, OMX_StateLoaded);
        if (err == OK) {
            err = mCodec->freeBuffersOnPort(kPortIndexInput);
            status_t err2 = mCodec->freeBuffersOnPort(kPortIndexOutput);
            if (err == OK) {
                err = err2;
            }
        }

        if ((mCodec->mFlags & kFlagPushBlankBuffersToNativeWindowOnShutdown)
                && mCodec->mNativeWindow != NULL) {
            // We push enough 1x1 blank buffers to ensure that one of
            // them has made it to the display.  This allows the OMX
            // component teardown to zero out any protected buffers
            // without the risk of scanning out one of those buffers.
            pushBlankBuffersToNativeWindow(mCodec->mNativeWindow.get());
        }

        if (err != OK) {
            mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
            return;
        }

        mCodec->changeState(mCodec->mIdleToLoadedState);
    }
}

void ACodec::ExecutingToIdleState::onInputBufferFilled(
        const sp<AMessage> &msg) {
    BaseState::onInputBufferFilled(msg);

    changeStateIfWeOwnAllBuffers();
}

void ACodec::ExecutingToIdleState::onOutputBufferDrained(
        const sp<AMessage> &msg) {
    BaseState::onOutputBufferDrained(msg);

    changeStateIfWeOwnAllBuffers();
}

////////////////////////////////////////////////////////////////////////////////

ACodec::IdleToLoadedState::IdleToLoadedState(ACodec *codec)
    : BaseState(codec) {
}

bool ACodec::IdleToLoadedState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case kWhatShutdown:
        {
            mCodec->deferMessage(msg);
            handled = true;
            break;
        }

        case kWhatFlush:
        {
            // Don't send me a flush request if you previously wanted me
            // to shutdown.
            ALOGE("Got flush request in IdleToLoadedState");
            break;
        }

        default:
            handled = BaseState::onMessageReceived(msg);
            break;
    }

    return handled;
}

void ACodec::IdleToLoadedState::stateEntered() {
    ALOGV("[%s] Now Idle->Loaded", mCodec->mComponentName.c_str());
}

bool ACodec::IdleToLoadedState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    switch (event) {
        case OMX_EventCmdComplete:
        {
            if (data1 != (OMX_U32)OMX_CommandStateSet
                    || data2 != (OMX_U32)OMX_StateLoaded) {
                ALOGE("Unexpected command completion in IdleToLoadedState: %s(%u) %s(%u)",
                        asString((OMX_COMMANDTYPE)data1), data1,
                        asString((OMX_STATETYPE)data2), data2);
                mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
                return true;
            }

            mCodec->changeState(mCodec->mLoadedState);

            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }
}

////////////////////////////////////////////////////////////////////////////////

ACodec::FlushingState::FlushingState(ACodec *codec)
    : BaseState(codec) {
}

void ACodec::FlushingState::stateEntered() {
    ALOGV("[%s] Now Flushing", mCodec->mComponentName.c_str());

    mFlushComplete[kPortIndexInput] = mFlushComplete[kPortIndexOutput] = false;

    // If we haven't transitioned after 3 seconds, we're probably stuck.
    sp<AMessage> msg = new AMessage(ACodec::kWhatCheckIfStuck, mCodec);
    msg->setInt32("generation", mCodec->mStateGeneration);
    msg->post(3000000);
}

bool ACodec::FlushingState::onMessageReceived(const sp<AMessage> &msg) {
    bool handled = false;

    switch (msg->what()) {
        case kWhatShutdown:
        {
            mCodec->deferMessage(msg);
            if (mCodec->mFatalError) {
                sp<AMessage> msg = new AMessage(ACodec::kWhatForceStateTransition, mCodec);
                msg->setInt32("generation", mCodec->mStateGeneration);
                msg->post(3000000);
            }
            handled = true;
            break;
        }

        case kWhatFlush:
        {
            // We're already doing this right now.
            handled = true;
            break;
        }

        case kWhatForceStateTransition:
        {
            int32_t generation = 0;
            CHECK(msg->findInt32("generation", &generation));
            mCodec->forceStateTransition(generation);

            handled = true;
            break;
        }

        case kWhatCheckIfStuck:
        {
            int32_t generation = 0;
            CHECK(msg->findInt32("generation", &generation));
            if (generation == mCodec->mStateGeneration) {
                mCodec->signalError(OMX_ErrorUndefined, TIMED_OUT);
            }

            handled = true;
            break;
        }

        default:
            handled = BaseState::onMessageReceived(msg);
            break;
    }

    return handled;
}

bool ACodec::FlushingState::onOMXEvent(
        OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
    ALOGV("[%s] FlushingState onOMXEvent(%u,%d)",
            mCodec->mComponentName.c_str(), event, (OMX_S32)data1);

    switch (event) {
        case OMX_EventCmdComplete:
        {
            if (data1 != (OMX_U32)OMX_CommandFlush) {
                ALOGE("unexpected EventCmdComplete %s(%d) data2:%d in FlushingState",
                        asString((OMX_COMMANDTYPE)data1), data1, data2);
                mCodec->signalError(OMX_ErrorUndefined, FAILED_TRANSACTION);
                return true;
            }

            if (data2 == kPortIndexInput || data2 == kPortIndexOutput) {
                if (mFlushComplete[data2]) {
                    ALOGW("Flush already completed for %s port",
                            data2 == kPortIndexInput ? "input" : "output");
                    return true;
                }
                mFlushComplete[data2] = true;

                if (mFlushComplete[kPortIndexInput] && mFlushComplete[kPortIndexOutput]) {
                    changeStateIfWeOwnAllBuffers();
                }
            } else if (data2 == OMX_ALL) {
                if (!mFlushComplete[kPortIndexInput] || !mFlushComplete[kPortIndexOutput]) {
                    ALOGW("received flush complete event for OMX_ALL before ports have been"
                            "flushed (%d/%d)",
                            mFlushComplete[kPortIndexInput], mFlushComplete[kPortIndexOutput]);
                    return false;
                }

                changeStateIfWeOwnAllBuffers();
            } else {
                ALOGW("data2 not OMX_ALL but %u in EventCmdComplete CommandFlush", data2);
            }

            return true;
        }

        case OMX_EventPortSettingsChanged:
        {
            sp<AMessage> msg = new AMessage(kWhatOMXMessage, mCodec);
            msg->setInt32("type", omx_message::EVENT);
            msg->setInt32("generation", mCodec->mNodeGeneration);
            msg->setInt32("event", event);
            msg->setInt32("data1", data1);
            msg->setInt32("data2", data2);

            ALOGV("[%s] Deferring OMX_EventPortSettingsChanged",
                 mCodec->mComponentName.c_str());

            mCodec->deferMessage(msg);

            return true;
        }

        default:
            return BaseState::onOMXEvent(event, data1, data2);
    }

    return true;
}

void ACodec::FlushingState::onOutputBufferDrained(const sp<AMessage> &msg) {
    BaseState::onOutputBufferDrained(msg);

    changeStateIfWeOwnAllBuffers();
}

void ACodec::FlushingState::onInputBufferFilled(const sp<AMessage> &msg) {
    BaseState::onInputBufferFilled(msg);

    changeStateIfWeOwnAllBuffers();
}

void ACodec::FlushingState::changeStateIfWeOwnAllBuffers() {
    if (mFlushComplete[kPortIndexInput]
            && mFlushComplete[kPortIndexOutput]
            && mCodec->allYourBuffersAreBelongToUs()) {
        // We now own all buffers except possibly those still queued with
        // the native window for rendering. Let's get those back as well.
        mCodec->waitUntilAllPossibleNativeWindowBuffersAreReturnedToUs();

        mCodec->mRenderTracker.clear(systemTime(CLOCK_MONOTONIC));

        mCodec->mCallback->onFlushCompleted();

        mCodec->mPortEOS[kPortIndexInput] =
            mCodec->mPortEOS[kPortIndexOutput] = false;

        mCodec->mInputEOSResult = OK;

        if (mCodec->mSkipCutBuffer != NULL) {
            mCodec->mSkipCutBuffer->clear();
        }

        mCodec->changeState(mCodec->mExecutingState);
    }
}

status_t ACodec::queryCapabilities(
        const char* owner, const char* name, const char* mime, bool isEncoder,
        MediaCodecInfo::CapabilitiesWriter* caps) {
    const char *role = GetComponentRole(isEncoder, mime);
    if (role == NULL) {
        return BAD_VALUE;
    }

    OMXClient client;
    status_t err = client.connect(owner);
    if (err != OK) {
        return err;
    }

    sp<IOMX> omx = client.interface();
    sp<CodecObserver> observer = new CodecObserver(new AMessage);
    sp<IOMXNode> omxNode;

    err = omx->allocateNode(name, observer, &omxNode);
    if (err != OK) {
        client.disconnect();
        return err;
    }

    err = SetComponentRole(omxNode, role);
    if (err != OK) {
        omxNode->freeNode();
        client.disconnect();
        return err;
    }

    bool isVideo = strncasecmp(mime, "video/", 6) == 0;
    bool isImage = strncasecmp(mime, "image/", 6) == 0;

    if (isVideo || isImage) {
        OMX_VIDEO_PARAM_PROFILELEVELTYPE param;
        InitOMXParams(&param);
        param.nPortIndex = isEncoder ? kPortIndexOutput : kPortIndexInput;

        for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
            param.nProfileIndex = index;
            status_t err = omxNode->getParameter(
                    OMX_IndexParamVideoProfileLevelQuerySupported,
                    &param, sizeof(param));
            if (err != OK) {
                break;
            }
            caps->addProfileLevel(param.eProfile, param.eLevel);

            // AVC components may not list the constrained profiles explicitly, but
            // decoders that support a profile also support its constrained version.
            // Encoders must explicitly support constrained profiles.
            if (!isEncoder && strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC) == 0) {
                if (param.eProfile == OMX_VIDEO_AVCProfileHigh) {
                    caps->addProfileLevel(OMX_VIDEO_AVCProfileConstrainedHigh, param.eLevel);
                } else if (param.eProfile == OMX_VIDEO_AVCProfileBaseline) {
                    caps->addProfileLevel(OMX_VIDEO_AVCProfileConstrainedBaseline, param.eLevel);
                }
            }

            if (index == kMaxIndicesToCheck) {
                ALOGW("[%s] stopping checking profiles after %u: %x/%x",
                        name, index,
                        param.eProfile, param.eLevel);
            }
        }

        // Color format query
        // return colors in the order reported by the OMX component
        // prefix "flexible" standard ones with the flexible equivalent
        OMX_VIDEO_PARAM_PORTFORMATTYPE portFormat;
        InitOMXParams(&portFormat);
        portFormat.nPortIndex = isEncoder ? kPortIndexInput : kPortIndexOutput;
        for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
            portFormat.nIndex = index;
            status_t err = omxNode->getParameter(
                    OMX_IndexParamVideoPortFormat,
                    &portFormat, sizeof(portFormat));
            if (err != OK) {
                break;
            }

            OMX_U32 flexibleEquivalent;
            if (IsFlexibleColorFormat(
                    omxNode, portFormat.eColorFormat, false /* usingNativeWindow */,
                    &flexibleEquivalent)) {
                caps->addColorFormat(flexibleEquivalent);
            }
            caps->addColorFormat(portFormat.eColorFormat);

            if (index == kMaxIndicesToCheck) {
                ALOGW("[%s] stopping checking formats after %u: %s(%x)",
                        name, index,
                        asString(portFormat.eColorFormat), portFormat.eColorFormat);
            }
        }
    } else if (strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC) == 0) {
        // More audio codecs if they have profiles.
        OMX_AUDIO_PARAM_ANDROID_PROFILETYPE param;
        InitOMXParams(&param);
        param.nPortIndex = isEncoder ? kPortIndexOutput : kPortIndexInput;
        for (OMX_U32 index = 0; index <= kMaxIndicesToCheck; ++index) {
            param.nProfileIndex = index;
            status_t err = omxNode->getParameter(
                    (OMX_INDEXTYPE)OMX_IndexParamAudioProfileQuerySupported,
                    &param, sizeof(param));
            if (err != OK) {
                break;
            }
            // For audio, level is ignored.
            caps->addProfileLevel(param.eProfile, 0 /* level */);

            if (index == kMaxIndicesToCheck) {
                ALOGW("[%s] stopping checking profiles after %u: %x",
                        name, index,
                        param.eProfile);
            }
        }

        // NOTE: Without Android extensions, OMX does not provide a way to query
        // AAC profile support
        if (param.nProfileIndex == 0) {
            ALOGW("component %s doesn't support profile query.", name);
        }
    }

    if (isVideo && !isEncoder) {
        native_handle_t *sidebandHandle = NULL;
        if (omxNode->configureVideoTunnelMode(
                kPortIndexOutput, OMX_TRUE, 0, &sidebandHandle) == OK) {
            // tunneled playback includes adaptive playback
        } else {
            // tunneled playback is not supported
            caps->removeDetail(MediaCodecInfo::Capabilities::FEATURE_TUNNELED_PLAYBACK);
            if (omxNode->setPortMode(
                    kPortIndexOutput, IOMX::kPortModeDynamicANWBuffer) != OK &&
                    omxNode->prepareForAdaptivePlayback(
                        kPortIndexOutput, OMX_TRUE,
                        1280 /* width */, 720 /* height */) != OK) {
                // adaptive playback is not supported
                caps->removeDetail(MediaCodecInfo::Capabilities::FEATURE_ADAPTIVE_PLAYBACK);
            }
        }
    }

    if (isVideo && isEncoder) {
        OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE params;
        InitOMXParams(&params);
        params.nPortIndex = kPortIndexOutput;

        OMX_VIDEO_PARAM_INTRAREFRESHTYPE fallbackParams;
        InitOMXParams(&fallbackParams);
        fallbackParams.nPortIndex = kPortIndexOutput;
        fallbackParams.eRefreshMode = OMX_VIDEO_IntraRefreshCyclic;

        if (omxNode->getConfig(
                (OMX_INDEXTYPE)OMX_IndexConfigAndroidIntraRefresh,
                &params, sizeof(params)) != OK &&
                omxNode->getParameter(
                    OMX_IndexParamVideoIntraRefresh, &fallbackParams,
                    sizeof(fallbackParams)) != OK) {
            // intra refresh is not supported
            caps->removeDetail(MediaCodecInfo::Capabilities::FEATURE_INTRA_REFRESH);
        }
    }

    omxNode->freeNode();
    client.disconnect();
    return OK;
}

// These are supposed be equivalent to the logic in
// "audio_channel_out_mask_from_count".
//static
status_t ACodec::getOMXChannelMapping(size_t numChannels, OMX_AUDIO_CHANNELTYPE map[]) {
    switch (numChannels) {
        case 1:
            map[0] = OMX_AUDIO_ChannelCF;
            break;
        case 2:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            break;
        case 3:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelCF;
            break;
        case 4:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelLR;
            map[3] = OMX_AUDIO_ChannelRR;
            break;
        case 5:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelCF;
            map[3] = OMX_AUDIO_ChannelLR;
            map[4] = OMX_AUDIO_ChannelRR;
            break;
        case 6:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelCF;
            map[3] = OMX_AUDIO_ChannelLFE;
            map[4] = OMX_AUDIO_ChannelLR;
            map[5] = OMX_AUDIO_ChannelRR;
            break;
        case 7:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelCF;
            map[3] = OMX_AUDIO_ChannelLFE;
            map[4] = OMX_AUDIO_ChannelLR;
            map[5] = OMX_AUDIO_ChannelRR;
            map[6] = OMX_AUDIO_ChannelCS;
            break;
        case 8:
            map[0] = OMX_AUDIO_ChannelLF;
            map[1] = OMX_AUDIO_ChannelRF;
            map[2] = OMX_AUDIO_ChannelCF;
            map[3] = OMX_AUDIO_ChannelLFE;
            map[4] = OMX_AUDIO_ChannelLR;
            map[5] = OMX_AUDIO_ChannelRR;
            map[6] = OMX_AUDIO_ChannelLS;
            map[7] = OMX_AUDIO_ChannelRS;
            break;
        default:
            return -EINVAL;
    }

    return OK;
}

}  // namespace android
