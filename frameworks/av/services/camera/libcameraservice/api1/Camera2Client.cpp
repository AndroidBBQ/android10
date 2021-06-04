/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "Camera2Client"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <inttypes.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <cutils/properties.h>
#include <gui/Surface.h>
#include <android/hardware/camera2/ICameraDeviceCallbacks.h>

#include "api1/Camera2Client.h"

#include "api1/client2/StreamingProcessor.h"
#include "api1/client2/JpegProcessor.h"
#include "api1/client2/CaptureSequencer.h"
#include "api1/client2/CallbackProcessor.h"
#include "api1/client2/ZslProcessor.h"
#include "utils/CameraThreadState.h"

#define ALOG1(...) ALOGD_IF(gLogLevel >= 1, __VA_ARGS__);
#define ALOG2(...) ALOGD_IF(gLogLevel >= 2, __VA_ARGS__);

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED [[fallthrough]]
#endif

namespace android {
using namespace camera2;

// Interface used by CameraService

Camera2Client::Camera2Client(const sp<CameraService>& cameraService,
        const sp<hardware::ICameraClient>& cameraClient,
        const String16& clientPackageName,
        const String8& cameraDeviceId,
        int api1CameraId,
        int cameraFacing,
        int clientPid,
        uid_t clientUid,
        int servicePid):
        Camera2ClientBase(cameraService, cameraClient, clientPackageName,
                cameraDeviceId, api1CameraId, cameraFacing,
                clientPid, clientUid, servicePid),
        mParameters(api1CameraId, cameraFacing)
{
    ATRACE_CALL();

    SharedParameters::Lock l(mParameters);
    l.mParameters.state = Parameters::DISCONNECTED;
}

status_t Camera2Client::initialize(sp<CameraProviderManager> manager, const String8& monitorTags) {
    return initializeImpl(manager, monitorTags);
}

bool Camera2Client::isZslEnabledInStillTemplate() {
    bool zslEnabled = false;
    CameraMetadata stillTemplate;
    status_t res = mDevice->createDefaultRequest(CAMERA2_TEMPLATE_STILL_CAPTURE, &stillTemplate);
    if (res == OK) {
        camera_metadata_entry_t enableZsl = stillTemplate.find(ANDROID_CONTROL_ENABLE_ZSL);
        if (enableZsl.count == 1) {
            zslEnabled = (enableZsl.data.u8[0] == ANDROID_CONTROL_ENABLE_ZSL_TRUE);
        }
    }

    return zslEnabled;
}

template<typename TProviderPtr>
status_t Camera2Client::initializeImpl(TProviderPtr providerPtr, const String8& monitorTags)
{
    ATRACE_CALL();
    ALOGV("%s: Initializing client for camera %d", __FUNCTION__, mCameraId);
    status_t res;

    res = Camera2ClientBase::initialize(providerPtr, monitorTags);
    if (res != OK) {
        return res;
    }

    {
        SharedParameters::Lock l(mParameters);

        res = l.mParameters.initialize(mDevice.get(), mDeviceVersion);
        if (res != OK) {
            ALOGE("%s: Camera %d: unable to build defaults: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return NO_INIT;
        }

        l.mParameters.isDeviceZslSupported = isZslEnabledInStillTemplate();
    }

    String8 threadName;

    mStreamingProcessor = new StreamingProcessor(this);
    threadName = String8::format("C2-%d-StreamProc",
            mCameraId);

    mFrameProcessor = new FrameProcessor(mDevice, this);
    threadName = String8::format("C2-%d-FrameProc",
            mCameraId);
    mFrameProcessor->run(threadName.string());

    mCaptureSequencer = new CaptureSequencer(this);
    threadName = String8::format("C2-%d-CaptureSeq",
            mCameraId);
    mCaptureSequencer->run(threadName.string());

    mJpegProcessor = new JpegProcessor(this, mCaptureSequencer);
    threadName = String8::format("C2-%d-JpegProc",
            mCameraId);
    mJpegProcessor->run(threadName.string());

    mZslProcessor = new ZslProcessor(this, mCaptureSequencer);

    threadName = String8::format("C2-%d-ZslProc",
            mCameraId);
    mZslProcessor->run(threadName.string());

    mCallbackProcessor = new CallbackProcessor(this);
    threadName = String8::format("C2-%d-CallbkProc",
            mCameraId);
    mCallbackProcessor->run(threadName.string());

    if (gLogLevel >= 1) {
        SharedParameters::Lock l(mParameters);
        ALOGD("%s: Default parameters converted from camera %d:", __FUNCTION__,
              mCameraId);
        ALOGD("%s", l.mParameters.paramsFlattened.string());
    }

    return OK;
}

Camera2Client::~Camera2Client() {
    ATRACE_CALL();
    ALOGV("~Camera2Client");

    mDestructionStarted = true;

    disconnect();

    ALOGI("Camera %d: Closed", mCameraId);
}

status_t Camera2Client::dump(int fd, const Vector<String16>& args) {
    return BasicClient::dump(fd, args);
}

status_t Camera2Client::dumpClient(int fd, const Vector<String16>& args) {
    String8 result;
    result.appendFormat("Client2[%d] (%p) PID: %d, dump:\n", mCameraId,
            (getRemoteCallback() != NULL ?
                    (IInterface::asBinder(getRemoteCallback()).get()) : NULL),
            mClientPid);
    result.append("  State: ");
#define CASE_APPEND_ENUM(x) case x: result.append(#x "\n"); break;

    const Parameters& p = mParameters.unsafeAccess();

    result.append(Parameters::getStateName(p.state));

    result.append("\n  Current parameters:\n");
    result.appendFormat("    Preview size: %d x %d\n",
            p.previewWidth, p.previewHeight);
    result.appendFormat("    Preview FPS range: %d - %d\n",
            p.previewFpsRange[0], p.previewFpsRange[1]);
    result.appendFormat("    Preview HAL pixel format: 0x%x\n",
            p.previewFormat);
    result.appendFormat("    Preview transform: %x\n",
            p.previewTransform);
    result.appendFormat("    Picture size: %d x %d\n",
            p.pictureWidth, p.pictureHeight);
    result.appendFormat("    Jpeg thumbnail size: %d x %d\n",
            p.jpegThumbSize[0], p.jpegThumbSize[1]);
    result.appendFormat("    Jpeg quality: %d, thumbnail quality: %d\n",
            p.jpegQuality, p.jpegThumbQuality);
    result.appendFormat("    Jpeg rotation: %d\n", p.jpegRotation);
    result.appendFormat("    GPS tags %s\n",
            p.gpsEnabled ? "enabled" : "disabled");
    if (p.gpsEnabled) {
        result.appendFormat("    GPS lat x long x alt: %f x %f x %f\n",
                p.gpsCoordinates[0], p.gpsCoordinates[1],
                p.gpsCoordinates[2]);
        result.appendFormat("    GPS timestamp: %" PRId64 "\n",
                p.gpsTimestamp);
        result.appendFormat("    GPS processing method: %s\n",
                p.gpsProcessingMethod.string());
    }

    result.append("    White balance mode: ");
    switch (p.wbMode) {
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_AUTO)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_INCANDESCENT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_FLUORESCENT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_DAYLIGHT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_TWILIGHT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AWB_MODE_SHADE)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Effect mode: ");
    switch (p.effectMode) {
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_OFF)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_MONO)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_NEGATIVE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_SOLARIZE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_SEPIA)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_POSTERIZE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_WHITEBOARD)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_BLACKBOARD)
        CASE_APPEND_ENUM(ANDROID_CONTROL_EFFECT_MODE_AQUA)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Antibanding mode: ");
    switch (p.antibandingMode) {
        CASE_APPEND_ENUM(ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Scene mode: ");
    switch (p.sceneMode) {
        case ANDROID_CONTROL_SCENE_MODE_DISABLED:
            result.append("AUTO\n"); break;
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_ACTION)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_PORTRAIT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_LANDSCAPE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_NIGHT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_NIGHT_PORTRAIT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_THEATRE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_BEACH)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_SNOW)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_SUNSET)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_STEADYPHOTO)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_FIREWORKS)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_SPORTS)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_PARTY)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_CANDLELIGHT)
        CASE_APPEND_ENUM(ANDROID_CONTROL_SCENE_MODE_BARCODE)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Flash mode: ");
    switch (p.flashMode) {
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_OFF)
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_AUTO)
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_ON)
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_TORCH)
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_RED_EYE)
        CASE_APPEND_ENUM(Parameters::FLASH_MODE_INVALID)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Focus mode: ");
    switch (p.focusMode) {
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_AUTO)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_MACRO)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_CONTINUOUS_VIDEO)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_CONTINUOUS_PICTURE)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_EDOF)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_INFINITY)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_FIXED)
        CASE_APPEND_ENUM(Parameters::FOCUS_MODE_INVALID)
        default: result.append("UNKNOWN\n");
    }

    result.append("   Focus state: ");
    switch (p.focusState) {
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_INACTIVE)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED)
        CASE_APPEND_ENUM(ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED)
        default: result.append("UNKNOWN\n");
    }

    result.append("    Focusing areas:\n");
    for (size_t i = 0; i < p.focusingAreas.size(); i++) {
        result.appendFormat("      [ (%d, %d, %d, %d), weight %d ]\n",
                p.focusingAreas[i].left,
                p.focusingAreas[i].top,
                p.focusingAreas[i].right,
                p.focusingAreas[i].bottom,
                p.focusingAreas[i].weight);
    }

    result.appendFormat("    Exposure compensation index: %d\n",
            p.exposureCompensation);

    result.appendFormat("    AE lock %s, AWB lock %s\n",
            p.autoExposureLock ? "enabled" : "disabled",
            p.autoWhiteBalanceLock ? "enabled" : "disabled" );

    result.appendFormat("    Metering areas:\n");
    for (size_t i = 0; i < p.meteringAreas.size(); i++) {
        result.appendFormat("      [ (%d, %d, %d, %d), weight %d ]\n",
                p.meteringAreas[i].left,
                p.meteringAreas[i].top,
                p.meteringAreas[i].right,
                p.meteringAreas[i].bottom,
                p.meteringAreas[i].weight);
    }

    result.appendFormat("    Zoom index: %d\n", p.zoom);
    result.appendFormat("    Video size: %d x %d\n", p.videoWidth,
            p.videoHeight);

    result.appendFormat("    Recording hint is %s\n",
            p.recordingHint ? "set" : "not set");

    result.appendFormat("    Video stabilization is %s\n",
            p.videoStabilization ? "enabled" : "disabled");

    result.appendFormat("    Selected still capture FPS range: %d - %d\n",
            p.fastInfo.bestStillCaptureFpsRange[0],
            p.fastInfo.bestStillCaptureFpsRange[1]);

    result.appendFormat("    Use zero shutter lag: %s\n",
            p.useZeroShutterLag() ? "yes" : "no");

    result.append("  Current streams:\n");
    result.appendFormat("    Preview stream ID: %d\n",
            getPreviewStreamId());
    result.appendFormat("    Capture stream ID: %d\n",
            getCaptureStreamId());
    result.appendFormat("    Recording stream ID: %d\n",
            getRecordingStreamId());

    result.append("  Quirks for this camera:\n");
    bool haveQuirk = false;
    if (p.quirks.triggerAfWithAuto) {
        result.appendFormat("    triggerAfWithAuto\n");
        haveQuirk = true;
    }
    if (p.quirks.useZslFormat) {
        result.appendFormat("    useZslFormat\n");
        haveQuirk = true;
    }
    if (p.quirks.meteringCropRegion) {
        result.appendFormat("    meteringCropRegion\n");
        haveQuirk = true;
    }
    if (p.quirks.partialResults) {
        result.appendFormat("    usePartialResult\n");
        haveQuirk = true;
    }
    if (!haveQuirk) {
        result.appendFormat("    none\n");
    }

    write(fd, result.string(), result.size());

    mStreamingProcessor->dump(fd, args);

    mCaptureSequencer->dump(fd, args);

    mFrameProcessor->dump(fd, args);

    mZslProcessor->dump(fd, args);

    return dumpDevice(fd, args);
#undef CASE_APPEND_ENUM
}

// ICamera interface

binder::Status Camera2Client::disconnect() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);

    binder::Status res = binder::Status::ok();
    // Allow both client and the cameraserver to disconnect at all times
    int callingPid = CameraThreadState::getCallingPid();
    if (callingPid != mClientPid && callingPid != mServicePid) return res;

    if (mDevice == 0) return res;

    ALOGV("Camera %d: Shutting down", mCameraId);

    /**
     * disconnect() cannot call any methods that might need to promote a
     * wp<Camera2Client>, since disconnect can be called from the destructor, at
     * which point all such promotions will fail.
     */

    stopPreviewL();

    {
        SharedParameters::Lock l(mParameters);
        if (l.mParameters.state == Parameters::DISCONNECTED) return res;
        l.mParameters.state = Parameters::DISCONNECTED;
    }

    mFrameProcessor->requestExit();
    mCaptureSequencer->requestExit();
    mJpegProcessor->requestExit();
    mZslProcessor->requestExit();
    mCallbackProcessor->requestExit();

    ALOGV("Camera %d: Waiting for threads", mCameraId);

    {
        // Don't wait with lock held, in case the other threads need to
        // complete callbacks that re-enter Camera2Client
        mBinderSerializationLock.unlock();

        mFrameProcessor->join();
        mCaptureSequencer->join();
        mJpegProcessor->join();
        mZslProcessor->join();
        mCallbackProcessor->join();

        mBinderSerializationLock.lock();
    }

    ALOGV("Camera %d: Deleting streams", mCameraId);

    mStreamingProcessor->deletePreviewStream();
    mStreamingProcessor->deleteRecordingStream();
    mJpegProcessor->deleteStream();
    mCallbackProcessor->deleteStream();
    mZslProcessor->deleteStream();

    ALOGV("Camera %d: Disconnecting device", mCameraId);

    mDevice->disconnect();

    CameraService::Client::disconnect();

    return res;
}

status_t Camera2Client::connect(const sp<hardware::ICameraClient>& client) {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);

    if (mClientPid != 0 && CameraThreadState::getCallingPid() != mClientPid) {
        ALOGE("%s: Camera %d: Connection attempt from pid %d; "
                "current locked to pid %d", __FUNCTION__,
                mCameraId, CameraThreadState::getCallingPid(), mClientPid);
        return BAD_VALUE;
    }

    mClientPid = CameraThreadState::getCallingPid();

    mRemoteCallback = client;
    mSharedCameraCallbacks = client;

    return OK;
}

status_t Camera2Client::lock() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    ALOGV("%s: Camera %d: Lock call from pid %d; current client pid %d",
            __FUNCTION__, mCameraId, CameraThreadState::getCallingPid(), mClientPid);

    if (mClientPid == 0) {
        mClientPid = CameraThreadState::getCallingPid();
        return OK;
    }

    if (mClientPid != CameraThreadState::getCallingPid()) {
        ALOGE("%s: Camera %d: Lock call from pid %d; currently locked to pid %d",
                __FUNCTION__, mCameraId, CameraThreadState::getCallingPid(), mClientPid);
        return EBUSY;
    }

    return OK;
}

status_t Camera2Client::unlock() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    ALOGV("%s: Camera %d: Unlock call from pid %d; current client pid %d",
            __FUNCTION__, mCameraId, CameraThreadState::getCallingPid(), mClientPid);

    if (mClientPid == CameraThreadState::getCallingPid()) {
        SharedParameters::Lock l(mParameters);
        if (l.mParameters.state == Parameters::RECORD ||
                l.mParameters.state == Parameters::VIDEO_SNAPSHOT) {
            ALOGD("Not allowed to unlock camera during recording.");
            return INVALID_OPERATION;
        }
        mClientPid = 0;
        mRemoteCallback.clear();
        mSharedCameraCallbacks.clear();
        return OK;
    }

    ALOGE("%s: Camera %d: Unlock call from pid %d; currently locked to pid %d",
            __FUNCTION__, mCameraId, CameraThreadState::getCallingPid(), mClientPid);
    return EBUSY;
}

status_t Camera2Client::setPreviewTarget(
        const sp<IGraphicBufferProducer>& bufferProducer) {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    sp<IBinder> binder;
    sp<Surface> window;
    if (bufferProducer != 0) {
        binder = IInterface::asBinder(bufferProducer);
        // Using controlledByApp flag to ensure that the buffer queue remains in
        // async mode for the old camera API, where many applications depend
        // on that behavior.
        window = new Surface(bufferProducer, /*controlledByApp*/ true);
    }
    return setPreviewWindowL(binder, window);
}

status_t Camera2Client::setPreviewWindowL(const sp<IBinder>& binder,
        const sp<Surface>& window) {
    ATRACE_CALL();
    status_t res;

    if (binder == mPreviewSurface) {
        ALOGV("%s: Camera %d: New window is same as old window",
                __FUNCTION__, mCameraId);
        return NO_ERROR;
    }

    Parameters::State state;
    {
        SharedParameters::Lock l(mParameters);
        state = l.mParameters.state;
    }
    switch (state) {
        case Parameters::DISCONNECTED:
        case Parameters::RECORD:
        case Parameters::STILL_CAPTURE:
        case Parameters::VIDEO_SNAPSHOT:
            ALOGE("%s: Camera %d: Cannot set preview display while in state %s",
                    __FUNCTION__, mCameraId,
                    Parameters::getStateName(state));
            return INVALID_OPERATION;
        case Parameters::STOPPED:
        case Parameters::WAITING_FOR_PREVIEW_WINDOW:
            // OK
            break;
        case Parameters::PREVIEW:
            // Already running preview - need to stop and create a new stream
            res = stopStream();
            if (res != OK) {
                ALOGE("%s: Unable to stop preview to swap windows: %s (%d)",
                        __FUNCTION__, strerror(-res), res);
                return res;
            }
            state = Parameters::WAITING_FOR_PREVIEW_WINDOW;
            break;
    }

    mPreviewSurface = binder;
    res = mStreamingProcessor->setPreviewWindow(window);
    if (res != OK) {
        ALOGE("%s: Unable to set new preview window: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    if (state == Parameters::WAITING_FOR_PREVIEW_WINDOW) {
        SharedParameters::Lock l(mParameters);
        l.mParameters.state = state;
        return startPreviewL(l.mParameters, false);
    }

    return OK;
}

void Camera2Client::setPreviewCallbackFlag(int flag) {
    ATRACE_CALL();
    ALOGV("%s: Camera %d: Flag 0x%x", __FUNCTION__, mCameraId, flag);
    Mutex::Autolock icl(mBinderSerializationLock);

    if ( checkPid(__FUNCTION__) != OK) return;

    SharedParameters::Lock l(mParameters);
    setPreviewCallbackFlagL(l.mParameters, flag);
}

void Camera2Client::setPreviewCallbackFlagL(Parameters &params, int flag) {
    status_t res = OK;

    switch(params.state) {
        case Parameters::STOPPED:
        case Parameters::WAITING_FOR_PREVIEW_WINDOW:
        case Parameters::PREVIEW:
        case Parameters::STILL_CAPTURE:
            // OK
            break;
        default:
            if (flag & CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK) {
                ALOGE("%s: Camera %d: Can't use preview callbacks "
                        "in state %d", __FUNCTION__, mCameraId, params.state);
                return;
            }
    }

    if (flag & CAMERA_FRAME_CALLBACK_FLAG_ONE_SHOT_MASK) {
        ALOGV("%s: setting oneshot", __FUNCTION__);
        params.previewCallbackOneShot = true;
    }
    if (params.previewCallbackFlags != (uint32_t)flag) {

        if (params.previewCallbackSurface && flag != CAMERA_FRAME_CALLBACK_FLAG_NOOP) {
            // Disable any existing preview callback window when enabling
            // preview callback flags
            res = mCallbackProcessor->setCallbackWindow(NULL);
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to clear preview callback surface:"
                        " %s (%d)", __FUNCTION__, mCameraId, strerror(-res), res);
                return;
            }
            params.previewCallbackSurface = false;
        }

        params.previewCallbackFlags = flag;

        if (params.state == Parameters::PREVIEW) {
            res = startPreviewL(params, true);
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to refresh request in state %s",
                        __FUNCTION__, mCameraId,
                        Parameters::getStateName(params.state));
            }
        }
    }
}

status_t Camera2Client::setPreviewCallbackTarget(
        const sp<IGraphicBufferProducer>& callbackProducer) {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    sp<Surface> window;
    if (callbackProducer != 0) {
        window = new Surface(callbackProducer);
    }

    res = mCallbackProcessor->setCallbackWindow(window);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to set preview callback surface: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    SharedParameters::Lock l(mParameters);

    if (window != NULL) {
        // Disable traditional callbacks when a valid callback target is given
        l.mParameters.previewCallbackFlags = CAMERA_FRAME_CALLBACK_FLAG_NOOP;
        l.mParameters.previewCallbackOneShot = false;
        l.mParameters.previewCallbackSurface = true;
    } else {
        // Disable callback target if given a NULL interface.
        l.mParameters.previewCallbackSurface = false;
    }

    switch(l.mParameters.state) {
        case Parameters::PREVIEW:
            res = startPreviewL(l.mParameters, true);
            break;
        case Parameters::RECORD:
        case Parameters::VIDEO_SNAPSHOT:
            res = startRecordingL(l.mParameters, true);
            break;
        default:
            break;
    }
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to refresh request in state %s",
                __FUNCTION__, mCameraId,
                Parameters::getStateName(l.mParameters.state));
    }

    return OK;
}


status_t Camera2Client::startPreview() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;
    SharedParameters::Lock l(mParameters);
    return startPreviewL(l.mParameters, false);
}

status_t Camera2Client::startPreviewL(Parameters &params, bool restart) {
    ATRACE_CALL();
    status_t res;

    ALOGV("%s: state == %d, restart = %d", __FUNCTION__, params.state, restart);

    if ( (params.state == Parameters::PREVIEW ||
                    params.state == Parameters::RECORD ||
                    params.state == Parameters::VIDEO_SNAPSHOT)
            && !restart) {
        // Succeed attempt to re-enter a streaming state
        ALOGI("%s: Camera %d: Preview already active, ignoring restart",
                __FUNCTION__, mCameraId);
        return OK;
    }
    if (params.state > Parameters::PREVIEW && !restart) {
        ALOGE("%s: Can't start preview in state %s",
                __FUNCTION__,
                Parameters::getStateName(params.state));
        return INVALID_OPERATION;
    }

    if (!mStreamingProcessor->haveValidPreviewWindow()) {
        params.state = Parameters::WAITING_FOR_PREVIEW_WINDOW;
        return OK;
    }
    params.state = Parameters::STOPPED;
    int lastPreviewStreamId = mStreamingProcessor->getPreviewStreamId();

    res = mStreamingProcessor->updatePreviewStream(params);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update preview stream: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    bool previewStreamChanged = mStreamingProcessor->getPreviewStreamId() != lastPreviewStreamId;

    // We could wait to create the JPEG output stream until first actual use
    // (first takePicture call). However, this would substantially increase the
    // first capture latency on HAL3 devices.
    // So create it unconditionally at preview start. As a drawback,
    // this increases gralloc memory consumption for applications that don't
    // ever take a picture. Do not enter this mode when jpeg stream will slow
    // down preview.
    // TODO: Find a better compromise, though this likely would involve HAL
    // changes.
    int lastJpegStreamId = mJpegProcessor->getStreamId();
    // If jpeg stream will slow down preview, make sure we remove it before starting preview
    if (params.slowJpegMode) {
        if (lastJpegStreamId != NO_STREAM) {
            // Pause preview if we are streaming
            int32_t activeRequestId = mStreamingProcessor->getActiveRequestId();
            if (activeRequestId != 0) {
                res = mStreamingProcessor->togglePauseStream(/*pause*/true);
                if (res != OK) {
                    ALOGE("%s: Camera %d: Can't pause streaming: %s (%d)",
                            __FUNCTION__, mCameraId, strerror(-res), res);
                }
                res = mDevice->waitUntilDrained();
                if (res != OK) {
                    ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                            __FUNCTION__, mCameraId, strerror(-res), res);
                }
            }

            res = mJpegProcessor->deleteStream();

            if (res != OK) {
                ALOGE("%s: Camera %d: delete Jpeg stream failed: %s (%d)",
                        __FUNCTION__, mCameraId,  strerror(-res), res);
            }

            if (activeRequestId != 0) {
                res = mStreamingProcessor->togglePauseStream(/*pause*/false);
                if (res != OK) {
                    ALOGE("%s: Camera %d: Can't unpause streaming: %s (%d)",
                            __FUNCTION__, mCameraId, strerror(-res), res);
                }
            }
        }
    } else {
        res = updateProcessorStream(mJpegProcessor, params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't pre-configure still image "
                    "stream: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
    }
    bool jpegStreamChanged = mJpegProcessor->getStreamId() != lastJpegStreamId;

    Vector<int32_t> outputStreams;
    bool callbacksEnabled = (params.previewCallbackFlags &
            CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK) ||
            params.previewCallbackSurface;

    if (callbacksEnabled) {
        // Can't have recording stream hanging around when enabling callbacks,
        // since it exceeds the max stream count on some devices.
        if (mStreamingProcessor->getRecordingStreamId() != NO_STREAM) {
            ALOGV("%s: Camera %d: Clearing out recording stream before "
                    "creating callback stream", __FUNCTION__, mCameraId);
            res = mStreamingProcessor->stopStream();
            if (res != OK) {
                ALOGE("%s: Camera %d: Can't stop streaming to delete "
                        "recording stream", __FUNCTION__, mCameraId);
                return res;
            }
            res = mStreamingProcessor->deleteRecordingStream();
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to delete recording stream before "
                        "enabling callbacks: %s (%d)", __FUNCTION__, mCameraId,
                        strerror(-res), res);
                return res;
            }
        }

        res = mCallbackProcessor->updateStream(params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to update callback stream: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
        outputStreams.push(getCallbackStreamId());
    } else if (previewStreamChanged && mCallbackProcessor->getStreamId() != NO_STREAM) {
        /**
         * Delete the unused callback stream when preview stream is changed and
         * preview is not enabled. Don't need stop preview stream as preview is in
         * STOPPED state now.
         */
        ALOGV("%s: Camera %d: Delete unused preview callback stream.",  __FUNCTION__, mCameraId);
        res = mCallbackProcessor->deleteStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to delete callback stream %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
    }

    if (params.useZeroShutterLag() &&
            getRecordingStreamId() == NO_STREAM) {
        res = updateProcessorStream(mZslProcessor, params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to update ZSL stream: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }

        if (jpegStreamChanged) {
            ALOGV("%s: Camera %d: Clear ZSL buffer queue when Jpeg size is changed",
                    __FUNCTION__, mCameraId);
            mZslProcessor->clearZslQueue();
        }
        outputStreams.push(getZslStreamId());
    } else {
        mZslProcessor->deleteStream();
    }

    outputStreams.push(getPreviewStreamId());

    if (params.isDeviceZslSupported) {
        // If device ZSL is supported, resume preview buffers that may be paused
        // during last takePicture().
        mDevice->dropStreamBuffers(false, getPreviewStreamId());
    }

    if (!params.recordingHint) {
        if (!restart) {
            res = mStreamingProcessor->updatePreviewRequest(params);
            if (res != OK) {
                ALOGE("%s: Camera %d: Can't set up preview request: "
                        "%s (%d)", __FUNCTION__, mCameraId,
                        strerror(-res), res);
                return res;
            }
        }
        res = mStreamingProcessor->startStream(StreamingProcessor::PREVIEW,
                outputStreams);
    } else {
        if (!restart) {
            res = mStreamingProcessor->updateRecordingRequest(params);
            if (res != OK) {
                ALOGE("%s: Camera %d: Can't set up preview request with "
                        "record hint: %s (%d)", __FUNCTION__, mCameraId,
                        strerror(-res), res);
                return res;
            }
        }
        res = mStreamingProcessor->startStream(StreamingProcessor::RECORD,
                outputStreams);
    }
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to start streaming preview: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    params.state = Parameters::PREVIEW;
    return OK;
}

void Camera2Client::stopPreview() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return;
    stopPreviewL();
}

void Camera2Client::stopPreviewL() {
    ATRACE_CALL();
    status_t res;
    const nsecs_t kStopCaptureTimeout = 3000000000LL; // 3 seconds
    Parameters::State state;
    {
        SharedParameters::Lock l(mParameters);
        state = l.mParameters.state;
    }

    switch (state) {
        case Parameters::DISCONNECTED:
            // Nothing to do.
            break;
        case Parameters::STOPPED:
        case Parameters::VIDEO_SNAPSHOT:
        case Parameters::STILL_CAPTURE:
            mCaptureSequencer->waitUntilIdle(kStopCaptureTimeout);
            FALLTHROUGH_INTENDED;
        case Parameters::RECORD:
        case Parameters::PREVIEW:
            syncWithDevice();
            res = stopStream();
            if (res != OK) {
                ALOGE("%s: Camera %d: Can't stop streaming: %s (%d)",
                        __FUNCTION__, mCameraId, strerror(-res), res);
            }

            // Flush all in-process captures and buffer in order to stop
            // preview faster.
            res = mDevice->flush();
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to flush pending requests: %s (%d)",
                        __FUNCTION__, mCameraId, strerror(-res), res);
            }

            res = mDevice->waitUntilDrained();
            if (res != OK) {
                ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                        __FUNCTION__, mCameraId, strerror(-res), res);
            }
            // Clean up recording stream
            res = mStreamingProcessor->deleteRecordingStream();
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to delete recording stream before "
                        "stop preview: %s (%d)",
                        __FUNCTION__, mCameraId, strerror(-res), res);
            }
            FALLTHROUGH_INTENDED;
        case Parameters::WAITING_FOR_PREVIEW_WINDOW: {
            SharedParameters::Lock l(mParameters);
            l.mParameters.state = Parameters::STOPPED;
            commandStopFaceDetectionL(l.mParameters);
            break;
        }
        default:
            ALOGE("%s: Camera %d: Unknown state %d", __FUNCTION__, mCameraId,
                    state);
    }
}

bool Camera2Client::previewEnabled() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return false;

    SharedParameters::Lock l(mParameters);
    return l.mParameters.state == Parameters::PREVIEW;
}

status_t Camera2Client::setVideoBufferMode(int32_t videoBufferMode) {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    SharedParameters::Lock l(mParameters);
    switch (l.mParameters.state) {
        case Parameters::RECORD:
        case Parameters::VIDEO_SNAPSHOT:
            ALOGE("%s: Camera %d: Can't be called in state %s",
                    __FUNCTION__, mCameraId,
                    Parameters::getStateName(l.mParameters.state));
            return INVALID_OPERATION;
        default:
            // OK
            break;
    }

    if (videoBufferMode != VIDEO_BUFFER_MODE_BUFFER_QUEUE) {
        ALOGE("%s: %d: Only video buffer queue is supported", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    l.mParameters.videoBufferMode = videoBufferMode;

    return OK;
}

status_t Camera2Client::startRecording() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;
    SharedParameters::Lock l(mParameters);

    return startRecordingL(l.mParameters, false);
}

status_t Camera2Client::startRecordingL(Parameters &params, bool restart) {
    status_t res = OK;

    ALOGV("%s: state == %d, restart = %d", __FUNCTION__, params.state, restart);

    switch (params.state) {
        case Parameters::STOPPED:
            res = startPreviewL(params, false);
            if (res != OK) return res;
            // Make sure first preview request is submitted to the HAL device to avoid
            // two consecutive set of configure_streams being called into the HAL.
            // TODO: Refactor this to avoid initial preview configuration.
            syncWithDevice();
            break;
        case Parameters::PREVIEW:
            // Ready to go
            break;
        case Parameters::RECORD:
        case Parameters::VIDEO_SNAPSHOT:
            // OK to call this when recording is already on, just skip unless
            // we're looking to restart
            if (!restart) return OK;
            break;
        default:
            ALOGE("%s: Camera %d: Can't start recording in state %s",
                    __FUNCTION__, mCameraId,
                    Parameters::getStateName(params.state));
            return INVALID_OPERATION;
    };

    if (params.videoBufferMode != VIDEO_BUFFER_MODE_BUFFER_QUEUE) {
        ALOGE("%s: Camera %d: Recording only supported buffer queue mode, but "
                "mode %d is requested!", __FUNCTION__, mCameraId, params.videoBufferMode);
        return INVALID_OPERATION;
    }

    if (!mStreamingProcessor->haveValidRecordingWindow()) {
        ALOGE("%s: No valid recording window", __FUNCTION__);
        return INVALID_OPERATION;
    }

    if (!restart) {
        sCameraService->playSound(CameraService::SOUND_RECORDING_START);
        mStreamingProcessor->updateRecordingRequest(params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to update recording request: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
    }

    // Not all devices can support a preview callback stream and a recording
    // stream at the same time, so assume none of them can.
    if (mCallbackProcessor->getStreamId() != NO_STREAM) {
        ALOGV("%s: Camera %d: Clearing out callback stream before "
                "creating recording stream", __FUNCTION__, mCameraId);
        res = mStreamingProcessor->stopStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't stop streaming to delete callback stream",
                    __FUNCTION__, mCameraId);
            return res;
        }
        res = mCallbackProcessor->deleteStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to delete callback stream before "
                    "record: %s (%d)", __FUNCTION__, mCameraId,
                    strerror(-res), res);
            return res;
        }
    }

    // Clean up ZSL before transitioning into recording
    if (mZslProcessor->getStreamId() != NO_STREAM) {
        ALOGV("%s: Camera %d: Clearing out zsl stream before "
                "creating recording stream", __FUNCTION__, mCameraId);
        res = mStreamingProcessor->stopStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't stop streaming to delete callback stream",
                    __FUNCTION__, mCameraId);
            return res;
        }
        res = mDevice->waitUntilDrained();
        if (res != OK) {
            ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
        }
        res = mZslProcessor->clearZslQueue();
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't clear zsl queue",
                    __FUNCTION__, mCameraId);
            return res;
        }
        res = mZslProcessor->deleteStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to delete zsl stream before "
                    "record: %s (%d)", __FUNCTION__, mCameraId,
                    strerror(-res), res);
            return res;
        }
    }

    // Disable callbacks if they're enabled; can't record and use callbacks,
    // and we can't fail record start without stagefright asserting.
    params.previewCallbackFlags = 0;

    // May need to reconfigure video snapshot JPEG sizes
    // during recording startup, so need a more complex sequence here to
    // ensure an early stream reconfiguration doesn't happen
    bool recordingStreamNeedsUpdate;
    res = mStreamingProcessor->recordingStreamNeedsUpdate(params, &recordingStreamNeedsUpdate);
    if (res != OK) {
        ALOGE("%s: Camera %d: Can't query recording stream",
                __FUNCTION__, mCameraId);
        return res;
    }

    if (recordingStreamNeedsUpdate) {
        // Need to stop stream here so updateProcessorStream won't trigger configureStream
        // Right now camera device cannot handle configureStream failure gracefully
        // when device is streaming
        res = mStreamingProcessor->stopStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't stop streaming to update record "
                    "stream", __FUNCTION__, mCameraId);
            return res;
        }
        res = mDevice->waitUntilDrained();
        if (res != OK) {
            ALOGE("%s: Camera %d: Waiting to stop streaming failed: "
                    "%s (%d)", __FUNCTION__, mCameraId,
                    strerror(-res), res);
        }

        res = updateProcessorStream<
            StreamingProcessor,
            &StreamingProcessor::updateRecordingStream>(
                                                        mStreamingProcessor,
                                                        params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to update recording stream: "
                    "%s (%d)", __FUNCTION__, mCameraId,
                    strerror(-res), res);
            return res;
        }
    }

    Vector<int32_t> outputStreams;
    outputStreams.push(getPreviewStreamId());
    outputStreams.push(getRecordingStreamId());

    res = mStreamingProcessor->startStream(StreamingProcessor::RECORD,
            outputStreams);

    // startStream might trigger a configureStream call and device might fail
    // configureStream due to jpeg size > video size. Try again with jpeg size overridden
    // to video size.
    if (res == BAD_VALUE) {
        overrideVideoSnapshotSize(params);
        res = mStreamingProcessor->startStream(StreamingProcessor::RECORD,
                outputStreams);
    }

    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to start recording stream: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    if (params.state < Parameters::RECORD) {
        params.state = Parameters::RECORD;
    }

    return OK;
}

void Camera2Client::stopRecording() {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    SharedParameters::Lock l(mParameters);

    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return;

    switch (l.mParameters.state) {
        case Parameters::RECORD:
            // OK to stop
            break;
        case Parameters::STOPPED:
        case Parameters::PREVIEW:
        case Parameters::STILL_CAPTURE:
        case Parameters::VIDEO_SNAPSHOT:
        default:
            ALOGE("%s: Camera %d: Can't stop recording in state %s",
                    __FUNCTION__, mCameraId,
                    Parameters::getStateName(l.mParameters.state));
            return;
    };

    sCameraService->playSound(CameraService::SOUND_RECORDING_STOP);

    // Remove recording stream because the video target may be abandoned soon.
    res = stopStream();
    if (res != OK) {
        ALOGE("%s: Camera %d: Can't stop streaming: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }

    res = mDevice->waitUntilDrained();
    if (res != OK) {
        ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }
    // Clean up recording stream
    res = mStreamingProcessor->deleteRecordingStream();
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to delete recording stream before "
                "stop preview: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }
    l.mParameters.recoverOverriddenJpegSize();

    // Restart preview
    res = startPreviewL(l.mParameters, true);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to return to preview",
                __FUNCTION__, mCameraId);
    }
}

bool Camera2Client::recordingEnabled() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);

    if ( checkPid(__FUNCTION__) != OK) return false;

    return recordingEnabledL();
}

bool Camera2Client::recordingEnabledL() {
    ATRACE_CALL();
    SharedParameters::Lock l(mParameters);

    return (l.mParameters.state == Parameters::RECORD
            || l.mParameters.state == Parameters::VIDEO_SNAPSHOT);
}

void Camera2Client::releaseRecordingFrame(const sp<IMemory>& mem) {
    (void)mem;
    ATRACE_CALL();
    ALOGW("%s: Not supported in buffer queue mode.", __FUNCTION__);
}

void Camera2Client::releaseRecordingFrameHandle(native_handle_t *handle) {
    (void)handle;
    ATRACE_CALL();
    ALOGW("%s: Not supported in buffer queue mode.", __FUNCTION__);
}

void Camera2Client::releaseRecordingFrameHandleBatch(
        const std::vector<native_handle_t*>& handles) {
    (void)handles;
    ATRACE_CALL();
    ALOGW("%s: Not supported in buffer queue mode.", __FUNCTION__);
}

status_t Camera2Client::autoFocus() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    ALOGV("%s: Camera %d", __FUNCTION__, mCameraId);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    int triggerId;
    bool notifyImmediately = false;
    bool notifySuccess = false;
    {
        SharedParameters::Lock l(mParameters);
        if (l.mParameters.state < Parameters::PREVIEW) {
            ALOGE("%s: Camera %d: Call autoFocus when preview is inactive (state = %d).",
                    __FUNCTION__, mCameraId, l.mParameters.state);
            return INVALID_OPERATION;
        }

        /**
          * If the camera does not support auto-focus, it is a no-op and
          * onAutoFocus(boolean, Camera) callback will be called immediately
          * with a fake value of success set to true.
          *
          * Similarly, if focus mode is set to INFINITY, there's no reason to
          * bother the HAL.
          */
        if (l.mParameters.focusMode == Parameters::FOCUS_MODE_FIXED ||
                l.mParameters.focusMode == Parameters::FOCUS_MODE_INFINITY) {
            notifyImmediately = true;
            notifySuccess = true;
        }
        /**
         * If we're in CAF mode, and AF has already been locked, just fire back
         * the callback right away; the HAL would not send a notification since
         * no state change would happen on a AF trigger.
         */
        if ( (l.mParameters.focusMode == Parameters::FOCUS_MODE_CONTINUOUS_PICTURE ||
                l.mParameters.focusMode == Parameters::FOCUS_MODE_CONTINUOUS_VIDEO) &&
                l.mParameters.focusState == ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED ) {
            notifyImmediately = true;
            notifySuccess = true;
        }
        /**
         * Send immediate notification back to client
         */
        if (notifyImmediately) {
            SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);
            if (l.mRemoteCallback != 0) {
                l.mRemoteCallback->notifyCallback(CAMERA_MSG_FOCUS,
                        notifySuccess ? 1 : 0, 0);
            }
            return OK;
        }
        /**
         * Handle quirk mode for AF in scene modes
         */
        if (l.mParameters.quirks.triggerAfWithAuto &&
                l.mParameters.sceneMode != ANDROID_CONTROL_SCENE_MODE_DISABLED &&
                l.mParameters.focusMode != Parameters::FOCUS_MODE_AUTO &&
                !l.mParameters.focusingAreas[0].isEmpty()) {
            ALOGV("%s: Quirk: Switching from focusMode %d to AUTO",
                    __FUNCTION__, l.mParameters.focusMode);
            l.mParameters.shadowFocusMode = l.mParameters.focusMode;
            l.mParameters.focusMode = Parameters::FOCUS_MODE_AUTO;
            updateRequests(l.mParameters);
        }

        l.mParameters.currentAfTriggerId = ++l.mParameters.afTriggerCounter;
        triggerId = l.mParameters.currentAfTriggerId;
    }
    ATRACE_ASYNC_BEGIN(kAutofocusLabel, triggerId);

    syncWithDevice();

    mDevice->triggerAutofocus(triggerId);

    return OK;
}

status_t Camera2Client::cancelAutoFocus() {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    ALOGV("%s: Camera %d", __FUNCTION__, mCameraId);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    int triggerId;
    {
        SharedParameters::Lock l(mParameters);
        // Canceling does nothing in FIXED or INFINITY modes
        if (l.mParameters.focusMode == Parameters::FOCUS_MODE_FIXED ||
                l.mParameters.focusMode == Parameters::FOCUS_MODE_INFINITY) {
            return OK;
        }

        // An active AF trigger is canceled
        if (l.mParameters.afTriggerCounter == l.mParameters.currentAfTriggerId) {
            ATRACE_ASYNC_END(kAutofocusLabel, l.mParameters.currentAfTriggerId);
        }

        triggerId = ++l.mParameters.afTriggerCounter;

        // When using triggerAfWithAuto quirk, may need to reset focus mode to
        // the real state at this point. No need to cancel explicitly if
        // changing the AF mode.
        if (l.mParameters.shadowFocusMode != Parameters::FOCUS_MODE_INVALID) {
            ALOGV("%s: Quirk: Restoring focus mode to %d", __FUNCTION__,
                    l.mParameters.shadowFocusMode);
            l.mParameters.focusMode = l.mParameters.shadowFocusMode;
            l.mParameters.shadowFocusMode = Parameters::FOCUS_MODE_INVALID;
            updateRequests(l.mParameters);

            return OK;
        }
        if (l.mParameters.allowZslMode) {
            mZslProcessor->clearZslQueue();
        }
    }
    syncWithDevice();

    mDevice->triggerCancelAutofocus(triggerId);

    return OK;
}

status_t Camera2Client::takePicture(int /*msgType*/) {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    int takePictureCounter;
    bool shouldSyncWithDevice = true;
    {
        SharedParameters::Lock l(mParameters);
        switch (l.mParameters.state) {
            case Parameters::DISCONNECTED:
            case Parameters::STOPPED:
            case Parameters::WAITING_FOR_PREVIEW_WINDOW:
                ALOGE("%s: Camera %d: Cannot take picture without preview enabled",
                        __FUNCTION__, mCameraId);
                return INVALID_OPERATION;
            case Parameters::PREVIEW:
                // Good to go for takePicture
                res = commandStopFaceDetectionL(l.mParameters);
                if (res != OK) {
                    ALOGE("%s: Camera %d: Unable to stop face detection for still capture",
                            __FUNCTION__, mCameraId);
                    return res;
                }
                l.mParameters.state = Parameters::STILL_CAPTURE;

                // Remove recording stream to prevent video snapshot jpeg logic kicking in
                if (l.mParameters.isJpegSizeOverridden() &&
                        mStreamingProcessor->getRecordingStreamId() != NO_STREAM) {
                    res = mStreamingProcessor->togglePauseStream(/*pause*/true);
                    if (res != OK) {
                        ALOGE("%s: Camera %d: Can't pause streaming: %s (%d)",
                                __FUNCTION__, mCameraId, strerror(-res), res);
                    }
                    res = mDevice->waitUntilDrained();
                    if (res != OK) {
                        ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                                __FUNCTION__, mCameraId, strerror(-res), res);
                    }
                    // Clean up recording stream
                    res = mStreamingProcessor->deleteRecordingStream();
                    if (res != OK) {
                        ALOGE("%s: Camera %d: Unable to delete recording stream before "
                                "stop preview: %s (%d)",
                                __FUNCTION__, mCameraId, strerror(-res), res);
                    }
                    res = mStreamingProcessor->togglePauseStream(/*pause*/false);
                    if (res != OK) {
                        ALOGE("%s: Camera %d: Can't unpause streaming: %s (%d)",
                                __FUNCTION__, mCameraId, strerror(-res), res);
                    }
                    l.mParameters.recoverOverriddenJpegSize();
                }
                break;
            case Parameters::RECORD:
                // Good to go for video snapshot
                l.mParameters.state = Parameters::VIDEO_SNAPSHOT;
                break;
            case Parameters::STILL_CAPTURE:
            case Parameters::VIDEO_SNAPSHOT:
                ALOGE("%s: Camera %d: Already taking a picture",
                        __FUNCTION__, mCameraId);
                return INVALID_OPERATION;
        }

        ALOGV("%s: Camera %d: Starting picture capture", __FUNCTION__, mCameraId);
        int lastJpegStreamId = mJpegProcessor->getStreamId();
        // slowJpegMode will create jpeg stream in CaptureSequencer before capturing
        if (!l.mParameters.slowJpegMode) {
            res = updateProcessorStream(mJpegProcessor, l.mParameters);
        }

        // If video snapshot fail to configureStream, try override video snapshot size to
        // video size
        if (res == BAD_VALUE && l.mParameters.state == Parameters::VIDEO_SNAPSHOT) {
            overrideVideoSnapshotSize(l.mParameters);
            res = updateProcessorStream(mJpegProcessor, l.mParameters);
        }
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't set up still image stream: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
        takePictureCounter = ++l.mParameters.takePictureCounter;

        // Clear ZSL buffer queue when Jpeg size is changed.
        bool jpegStreamChanged = mJpegProcessor->getStreamId() != lastJpegStreamId;
        if (l.mParameters.allowZslMode && jpegStreamChanged) {
            ALOGV("%s: Camera %d: Clear ZSL buffer queue when Jpeg size is changed",
                    __FUNCTION__, mCameraId);
            mZslProcessor->clearZslQueue();
        }

        // We should always sync with the device in case flash is turned on,
        // the camera device suggests that flash is needed (AE state FLASH_REQUIRED)
        // or we are in some other AE state different from CONVERGED that may need
        // precapture trigger.
        if (l.mParameters.flashMode != Parameters::FLASH_MODE_ON &&
                (l.mParameters.aeState == ANDROID_CONTROL_AE_STATE_CONVERGED)) {
            shouldSyncWithDevice  = false;
        }
    }

    ATRACE_ASYNC_BEGIN(kTakepictureLabel, takePictureCounter);

    // Make sure HAL has correct settings in case precapture trigger is needed.
    if (shouldSyncWithDevice) {
        syncWithDevice();
    }

    res = mCaptureSequencer->startCapture();
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to start capture: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }

    return res;
}

status_t Camera2Client::setParameters(const String8& params) {
    ATRACE_CALL();
    ALOGV("%s: Camera %d", __FUNCTION__, mCameraId);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    SharedParameters::Lock l(mParameters);

    Parameters::focusMode_t focusModeBefore = l.mParameters.focusMode;
    res = l.mParameters.set(params);
    if (res != OK) return res;
    Parameters::focusMode_t focusModeAfter = l.mParameters.focusMode;

    if (l.mParameters.allowZslMode && focusModeAfter != focusModeBefore) {
        mZslProcessor->clearZslQueue();
    }

    res = updateRequests(l.mParameters);

    return res;
}

String8 Camera2Client::getParameters() const {
    ATRACE_CALL();
    ALOGV("%s: Camera %d", __FUNCTION__, mCameraId);
    Mutex::Autolock icl(mBinderSerializationLock);
    // The camera service can unconditionally get the parameters at all times
    if (CameraThreadState::getCallingPid() != mServicePid && checkPid(__FUNCTION__) != OK) return String8();

    SharedParameters::ReadLock l(mParameters);

    return l.mParameters.get();
}

status_t Camera2Client::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) {
    ATRACE_CALL();
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    ALOGV("%s: Camera %d: Command %d (%d, %d)", __FUNCTION__, mCameraId,
            cmd, arg1, arg2);

    switch (cmd) {
        case CAMERA_CMD_START_SMOOTH_ZOOM:
            return commandStartSmoothZoomL();
        case CAMERA_CMD_STOP_SMOOTH_ZOOM:
            return commandStopSmoothZoomL();
        case CAMERA_CMD_SET_DISPLAY_ORIENTATION:
            return commandSetDisplayOrientationL(arg1);
        case CAMERA_CMD_ENABLE_SHUTTER_SOUND:
            return commandEnableShutterSoundL(arg1 == 1);
        case CAMERA_CMD_PLAY_RECORDING_SOUND:
            return commandPlayRecordingSoundL();
        case CAMERA_CMD_START_FACE_DETECTION:
            return commandStartFaceDetectionL(arg1);
        case CAMERA_CMD_STOP_FACE_DETECTION: {
            SharedParameters::Lock l(mParameters);
            return commandStopFaceDetectionL(l.mParameters);
        }
        case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
            return commandEnableFocusMoveMsgL(arg1 == 1);
        case CAMERA_CMD_PING:
            return commandPingL();
        case CAMERA_CMD_SET_VIDEO_BUFFER_COUNT:
        case CAMERA_CMD_SET_VIDEO_FORMAT:
            ALOGE("%s: command %d (arguments %d, %d) is not supported.",
                    __FUNCTION__, cmd, arg1, arg2);
            return BAD_VALUE;
        default:
            ALOGE("%s: Unknown command %d (arguments %d, %d)",
                    __FUNCTION__, cmd, arg1, arg2);
            return BAD_VALUE;
    }
}

status_t Camera2Client::commandStartSmoothZoomL() {
    ALOGE("%s: Unimplemented!", __FUNCTION__);
    return OK;
}

status_t Camera2Client::commandStopSmoothZoomL() {
    ALOGE("%s: Unimplemented!", __FUNCTION__);
    return OK;
}

status_t Camera2Client::commandSetDisplayOrientationL(int degrees) {
    int transform = Parameters::degToTransform(degrees,
            mCameraFacing == CAMERA_FACING_FRONT);
    if (transform == -1) {
        ALOGE("%s: Camera %d: Error setting %d as display orientation value",
                __FUNCTION__, mCameraId, degrees);
        return BAD_VALUE;
    }
    SharedParameters::Lock l(mParameters);
    if (transform != l.mParameters.previewTransform &&
            getPreviewStreamId() != NO_STREAM) {
        mDevice->setStreamTransform(getPreviewStreamId(), transform);
    }
    l.mParameters.previewTransform = transform;
    return OK;
}

status_t Camera2Client::commandEnableShutterSoundL(bool enable) {
    SharedParameters::Lock l(mParameters);
    if (enable) {
        l.mParameters.playShutterSound = true;
        return OK;
    }

    l.mParameters.playShutterSound = false;
    return OK;
}

status_t Camera2Client::commandPlayRecordingSoundL() {
    sCameraService->playSound(CameraService::SOUND_RECORDING_START);
    return OK;
}

status_t Camera2Client::commandStartFaceDetectionL(int /*type*/) {
    ALOGV("%s: Camera %d: Starting face detection",
          __FUNCTION__, mCameraId);
    status_t res;
    SharedParameters::Lock l(mParameters);
    switch (l.mParameters.state) {
        case Parameters::DISCONNECTED:
        case Parameters::STOPPED:
        case Parameters::WAITING_FOR_PREVIEW_WINDOW:
        case Parameters::STILL_CAPTURE:
            ALOGE("%s: Camera %d: Cannot start face detection without preview active",
                    __FUNCTION__, mCameraId);
            return INVALID_OPERATION;
        case Parameters::PREVIEW:
        case Parameters::RECORD:
        case Parameters::VIDEO_SNAPSHOT:
            // Good to go for starting face detect
            break;
    }
    // Ignoring type
    if (l.mParameters.fastInfo.bestFaceDetectMode ==
            ANDROID_STATISTICS_FACE_DETECT_MODE_OFF) {
        ALOGE("%s: Camera %d: Face detection not supported",
                __FUNCTION__, mCameraId);
        return BAD_VALUE;
    }
    if (l.mParameters.enableFaceDetect) return OK;

    l.mParameters.enableFaceDetect = true;

    res = updateRequests(l.mParameters);

    return res;
}

status_t Camera2Client::commandStopFaceDetectionL(Parameters &params) {
    status_t res = OK;
    ALOGV("%s: Camera %d: Stopping face detection",
          __FUNCTION__, mCameraId);

    if (!params.enableFaceDetect) return OK;

    params.enableFaceDetect = false;

    if (params.state == Parameters::PREVIEW
            || params.state == Parameters::RECORD
            || params.state == Parameters::VIDEO_SNAPSHOT) {
        res = updateRequests(params);
    }

    return res;
}

status_t Camera2Client::commandEnableFocusMoveMsgL(bool enable) {
    SharedParameters::Lock l(mParameters);
    l.mParameters.enableFocusMoveMessages = enable;

    return OK;
}

status_t Camera2Client::commandPingL() {
    // Always ping back if access is proper and device is alive
    SharedParameters::Lock l(mParameters);
    if (l.mParameters.state != Parameters::DISCONNECTED) {
        return OK;
    } else {
        return NO_INIT;
    }
}

void Camera2Client::notifyError(int32_t errorCode,
        const CaptureResultExtras& resultExtras) {
    int32_t err = CAMERA_ERROR_UNKNOWN;
    switch(errorCode) {
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_DISCONNECTED:
            err = CAMERA_ERROR_RELEASED;
            break;
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_DEVICE:
            err = CAMERA_ERROR_UNKNOWN;
            break;
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_SERVICE:
            err = CAMERA_ERROR_SERVER_DIED;
            break;
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_REQUEST:
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_RESULT:
        case hardware::camera2::ICameraDeviceCallbacks::ERROR_CAMERA_BUFFER:
            ALOGW("%s: Received recoverable error %d from HAL - ignoring, requestId %" PRId32,
                    __FUNCTION__, errorCode, resultExtras.requestId);
            mCaptureSequencer->notifyError(errorCode, resultExtras);
            return;
        default:
            err = CAMERA_ERROR_UNKNOWN;
            break;
    }

    ALOGE("%s: Error condition %d reported by HAL, requestId %" PRId32, __FUNCTION__, errorCode,
              resultExtras.requestId);

    SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);
    if (l.mRemoteCallback != nullptr) {
        l.mRemoteCallback->notifyCallback(CAMERA_MSG_ERROR, err, 0);
    }
}


/** Device-related methods */
void Camera2Client::notifyAutoFocus(uint8_t newState, int triggerId) {
    ALOGV("%s: Autofocus state now %d, last trigger %d",
            __FUNCTION__, newState, triggerId);
    bool sendCompletedMessage = false;
    bool sendMovingMessage = false;

    bool success = false;
    bool afInMotion = false;
    {
        SharedParameters::Lock l(mParameters);
        // Trace end of AF state
        char tmp[32];
        if (l.mParameters.afStateCounter > 0) {
            camera_metadata_enum_snprint(
                ANDROID_CONTROL_AF_STATE, l.mParameters.focusState, tmp, sizeof(tmp));
            ATRACE_ASYNC_END(tmp, l.mParameters.afStateCounter);
        }

        // Update state
        l.mParameters.focusState = newState;
        l.mParameters.afStateCounter++;

        // Trace start of AF state

        camera_metadata_enum_snprint(
            ANDROID_CONTROL_AF_STATE, l.mParameters.focusState, tmp, sizeof(tmp));
        ATRACE_ASYNC_BEGIN(tmp, l.mParameters.afStateCounter);

        switch (l.mParameters.focusMode) {
            case Parameters::FOCUS_MODE_AUTO:
            case Parameters::FOCUS_MODE_MACRO:
                // Don't send notifications upstream if they're not for the current AF
                // trigger. For example, if cancel was called in between, or if we
                // already sent a notification about this AF call.
                if (triggerId != l.mParameters.currentAfTriggerId) break;
                switch (newState) {
                    case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
                        success = true;
                        FALLTHROUGH_INTENDED;
                    case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                        sendCompletedMessage = true;
                        l.mParameters.currentAfTriggerId = -1;
                        break;
                    case ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN:
                        // Just starting focusing, ignore
                        break;
                    case ANDROID_CONTROL_AF_STATE_INACTIVE:
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED:
                    default:
                        // Unexpected in AUTO/MACRO mode
                        ALOGE("%s: Unexpected AF state transition in AUTO/MACRO mode: %d",
                                __FUNCTION__, newState);
                        break;
                }
                break;
            case Parameters::FOCUS_MODE_CONTINUOUS_VIDEO:
            case Parameters::FOCUS_MODE_CONTINUOUS_PICTURE:
                switch (newState) {
                    case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
                        success = true;
                        FALLTHROUGH_INTENDED;
                    case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                        // Don't send notifications upstream if they're not for
                        // the current AF trigger. For example, if cancel was
                        // called in between, or if we already sent a
                        // notification about this AF call.
                        // Send both a 'AF done' callback and a 'AF move' callback
                        if (triggerId != l.mParameters.currentAfTriggerId) break;
                        sendCompletedMessage = true;
                        afInMotion = false;
                        if (l.mParameters.enableFocusMoveMessages &&
                                l.mParameters.afInMotion) {
                            sendMovingMessage = true;
                        }
                        l.mParameters.currentAfTriggerId = -1;
                        break;
                    case ANDROID_CONTROL_AF_STATE_INACTIVE:
                        // Cancel was called, or we switched state; care if
                        // currently moving
                        afInMotion = false;
                        if (l.mParameters.enableFocusMoveMessages &&
                                l.mParameters.afInMotion) {
                            sendMovingMessage = true;
                        }
                        break;
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
                        // Start passive scan, inform upstream
                        afInMotion = true;
                        FALLTHROUGH_INTENDED;
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
                    case ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED:
                        // Stop passive scan, inform upstream
                        if (l.mParameters.enableFocusMoveMessages) {
                            sendMovingMessage = true;
                        }
                        break;
                }
                l.mParameters.afInMotion = afInMotion;
                break;
            case Parameters::FOCUS_MODE_EDOF:
            case Parameters::FOCUS_MODE_INFINITY:
            case Parameters::FOCUS_MODE_FIXED:
            default:
                if (newState != ANDROID_CONTROL_AF_STATE_INACTIVE) {
                    ALOGE("%s: Unexpected AF state change %d "
                            "(ID %d) in focus mode %d",
                          __FUNCTION__, newState, triggerId,
                            l.mParameters.focusMode);
                }
        }
    }
    if (sendMovingMessage) {
        SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);
        if (l.mRemoteCallback != 0) {
            l.mRemoteCallback->notifyCallback(CAMERA_MSG_FOCUS_MOVE,
                    afInMotion ? 1 : 0, 0);
        }
    }
    if (sendCompletedMessage) {
        ATRACE_ASYNC_END(kAutofocusLabel, triggerId);
        SharedCameraCallbacks::Lock l(mSharedCameraCallbacks);
        if (l.mRemoteCallback != 0) {
            l.mRemoteCallback->notifyCallback(CAMERA_MSG_FOCUS,
                    success ? 1 : 0, 0);
        }
    }
}

void Camera2Client::notifyAutoExposure(uint8_t newState, int triggerId) {
    ALOGV("%s: Autoexposure state now %d, last trigger %d",
            __FUNCTION__, newState, triggerId);
    {
        SharedParameters::Lock l(mParameters);
        // Update state
        l.mParameters.aeState = newState;
    }
    mCaptureSequencer->notifyAutoExposure(newState, triggerId);
}

void Camera2Client::notifyShutter(const CaptureResultExtras& resultExtras,
                                  nsecs_t timestamp) {
    ALOGV("%s: Shutter notification for request id %" PRId32 " at time %" PRId64,
            __FUNCTION__, resultExtras.requestId, timestamp);
    mCaptureSequencer->notifyShutter(resultExtras, timestamp);

    Camera2ClientBase::notifyShutter(resultExtras, timestamp);
}

camera2::SharedParameters& Camera2Client::getParameters() {
    return mParameters;
}

int Camera2Client::getPreviewStreamId() const {
    return mStreamingProcessor->getPreviewStreamId();
}

int Camera2Client::getCaptureStreamId() const {
    return mJpegProcessor->getStreamId();
}

int Camera2Client::getCallbackStreamId() const {
    return mCallbackProcessor->getStreamId();
}

int Camera2Client::getRecordingStreamId() const {
    return mStreamingProcessor->getRecordingStreamId();
}

int Camera2Client::getZslStreamId() const {
    return mZslProcessor->getStreamId();
}

status_t Camera2Client::registerFrameListener(int32_t minId, int32_t maxId,
        const wp<camera2::FrameProcessor::FilteredListener>& listener, bool sendPartials) {
    return mFrameProcessor->registerListener(minId, maxId, listener, sendPartials);
}

status_t Camera2Client::removeFrameListener(int32_t minId, int32_t maxId,
        const wp<camera2::FrameProcessor::FilteredListener>& listener) {
    return mFrameProcessor->removeListener(minId, maxId, listener);
}

status_t Camera2Client::stopStream() {
    return mStreamingProcessor->stopStream();
}

status_t Camera2Client::createJpegStreamL(Parameters &params) {
    status_t res = OK;
    int lastJpegStreamId = mJpegProcessor->getStreamId();
    if (lastJpegStreamId != NO_STREAM) {
        return INVALID_OPERATION;
    }

    res = mStreamingProcessor->togglePauseStream(/*pause*/true);
    if (res != OK) {
        ALOGE("%s: Camera %d: Can't pause streaming: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    res = mDevice->flush();
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable flush device: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    // Ideally we don't need this, but current camera device
    // status tracking mechanism demands it.
    res = mDevice->waitUntilDrained();
    if (res != OK) {
        ALOGE("%s: Camera %d: Waiting device drain failed: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }

    res = updateProcessorStream(mJpegProcessor, params);
    return res;
}

const int32_t Camera2Client::kPreviewRequestIdStart;
const int32_t Camera2Client::kPreviewRequestIdEnd;
const int32_t Camera2Client::kRecordingRequestIdStart;
const int32_t Camera2Client::kRecordingRequestIdEnd;
const int32_t Camera2Client::kCaptureRequestIdStart;
const int32_t Camera2Client::kCaptureRequestIdEnd;

/** Utility methods */

status_t Camera2Client::updateRequests(Parameters &params) {
    status_t res;

    ALOGV("%s: Camera %d: state = %d", __FUNCTION__, getCameraId(), params.state);

    res = mStreamingProcessor->incrementStreamingIds();
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to increment request IDs: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    res = mStreamingProcessor->updatePreviewRequest(params);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update preview request: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }
    res = mStreamingProcessor->updateRecordingRequest(params);
    if (res != OK) {
        ALOGE("%s: Camera %d: Unable to update recording request: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        return res;
    }

    if (params.state == Parameters::PREVIEW) {
        res = startPreviewL(params, true);
        if (res != OK) {
            ALOGE("%s: Camera %d: Error streaming new preview request: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
    } else if (params.state == Parameters::RECORD ||
            params.state == Parameters::VIDEO_SNAPSHOT) {
        res = startRecordingL(params, true);
        if (res != OK) {
            ALOGE("%s: Camera %d: Error streaming new record request: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
            return res;
        }
    }
    return res;
}


size_t Camera2Client::calculateBufferSize(int width, int height,
        int format, int stride) {
    switch (format) {
        case HAL_PIXEL_FORMAT_YCbCr_422_SP: // NV16
            return width * height * 2;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP: // NV21
            return width * height * 3 / 2;
        case HAL_PIXEL_FORMAT_YCbCr_422_I: // YUY2
            return width * height * 2;
        case HAL_PIXEL_FORMAT_YV12: {      // YV12
            size_t ySize = stride * height;
            size_t uvStride = (stride / 2 + 0xF) & ~0xF;
            size_t uvSize = uvStride * height / 2;
            return ySize + uvSize * 2;
        }
        case HAL_PIXEL_FORMAT_RGB_565:
            return width * height * 2;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return width * height * 4;
        case HAL_PIXEL_FORMAT_RAW16:
            return width * height * 2;
        default:
            ALOGE("%s: Unknown preview format: %x",
                    __FUNCTION__,  format);
            return 0;
    }
}

status_t Camera2Client::syncWithDevice() {
    ATRACE_CALL();
    const nsecs_t kMaxSyncTimeout = 500000000; // 500 ms
    status_t res;

    int32_t activeRequestId = mStreamingProcessor->getActiveRequestId();
    if (activeRequestId == 0) return OK;

    res = mDevice->waitUntilRequestReceived(activeRequestId, kMaxSyncTimeout);
    if (res == TIMED_OUT) {
        ALOGE("%s: Camera %d: Timed out waiting sync with HAL",
                __FUNCTION__, mCameraId);
    } else if (res != OK) {
        ALOGE("%s: Camera %d: Error while waiting to sync with HAL",
                __FUNCTION__, mCameraId);
    }
    return res;
}

template <typename ProcessorT>
status_t Camera2Client::updateProcessorStream(sp<ProcessorT> processor,
                                              camera2::Parameters params) {
    // No default template arguments until C++11, so we need this overload
    return updateProcessorStream<ProcessorT, &ProcessorT::updateStream>(
            processor, params);
}

template <typename ProcessorT,
          status_t (ProcessorT::*updateStreamF)(const Parameters &)>
status_t Camera2Client::updateProcessorStream(sp<ProcessorT> processor,
                                              Parameters params) {
    status_t res;

    // Get raw pointer since sp<T> doesn't have operator->*
    ProcessorT *processorPtr = processor.get();
    res = (processorPtr->*updateStreamF)(params);

    /**
     * Can't update the stream if it's busy?
     *
     * Then we need to stop the device (by temporarily clearing the request
     * queue) and then try again. Resume streaming once we're done.
     */
    if (res == -EBUSY) {
        ALOGV("%s: Camera %d: Pausing to update stream", __FUNCTION__,
                mCameraId);
        res = mStreamingProcessor->togglePauseStream(/*pause*/true);
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't pause streaming: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
        }

        res = mDevice->waitUntilDrained();
        if (res != OK) {
            ALOGE("%s: Camera %d: Waiting to stop streaming failed: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
        }

        res = (processorPtr->*updateStreamF)(params);
        if (res != OK) {
            ALOGE("%s: Camera %d: Failed to update processing stream "
                  " despite having halted streaming first: %s (%d)",
                  __FUNCTION__, mCameraId, strerror(-res), res);
        }

        res = mStreamingProcessor->togglePauseStream(/*pause*/false);
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't unpause streaming: %s (%d)",
                    __FUNCTION__, mCameraId, strerror(-res), res);
        }
    }

    return res;
}

status_t Camera2Client::overrideVideoSnapshotSize(Parameters &params) {
    ALOGV("%s: Camera %d: configure still size to video size before recording"
            , __FUNCTION__, mCameraId);
    params.overrideJpegSizeByVideoSize();
    status_t res = updateProcessorStream(mJpegProcessor, params);
    if (res != OK) {
        ALOGE("%s: Camera %d: Can't override video snapshot size to video size: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
    }
    return res;
}

status_t Camera2Client::setVideoTarget(const sp<IGraphicBufferProducer>& bufferProducer) {
    ATRACE_CALL();
    ALOGV("%s: E", __FUNCTION__);
    Mutex::Autolock icl(mBinderSerializationLock);
    status_t res;
    if ( (res = checkPid(__FUNCTION__) ) != OK) return res;

    sp<IBinder> binder = IInterface::asBinder(bufferProducer);
    if (binder == mVideoSurface) {
        ALOGV("%s: Camera %d: New video window is same as old video window",
                __FUNCTION__, mCameraId);
        return NO_ERROR;
    }

    sp<Surface> window;
    int format;
    android_dataspace dataSpace;

    if (bufferProducer != nullptr) {
        // Using controlledByApp flag to ensure that the buffer queue remains in
        // async mode for the old camera API, where many applications depend
        // on that behavior.
        window = new Surface(bufferProducer, /*controlledByApp*/ true);

        ANativeWindow *anw = window.get();

        if ((res = anw->query(anw, NATIVE_WINDOW_FORMAT, &format)) != OK) {
            ALOGE("%s: Failed to query Surface format", __FUNCTION__);
            return res;
        }

        if ((res = anw->query(anw, NATIVE_WINDOW_DEFAULT_DATASPACE,
                                reinterpret_cast<int*>(&dataSpace))) != OK) {
            ALOGE("%s: Failed to query Surface dataSpace", __FUNCTION__);
            return res;
        }
    }

    Parameters::State state;
    {
        SharedParameters::Lock l(mParameters);
        state = l.mParameters.state;
    }

    switch (state) {
        case Parameters::STOPPED:
        case Parameters::WAITING_FOR_PREVIEW_WINDOW:
        case Parameters::PREVIEW:
            // OK
            break;
        case Parameters::DISCONNECTED:
        case Parameters::RECORD:
        case Parameters::STILL_CAPTURE:
        case Parameters::VIDEO_SNAPSHOT:
        default:
            ALOGE("%s: Camera %d: Cannot set video target while in state %s",
                    __FUNCTION__, mCameraId,
                    Parameters::getStateName(state));
            return INVALID_OPERATION;
    }

    mVideoSurface = binder;
    res = mStreamingProcessor->setRecordingWindow(window);
    if (res != OK) {
        ALOGE("%s: Unable to set new recording window: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    {
        SharedParameters::Lock l(mParameters);
        l.mParameters.videoFormat = format;
        l.mParameters.videoDataSpace = dataSpace;
    }

    return OK;
}

const char* Camera2Client::kAutofocusLabel = "autofocus";
const char* Camera2Client::kTakepictureLabel = "take_picture";

} // namespace android
