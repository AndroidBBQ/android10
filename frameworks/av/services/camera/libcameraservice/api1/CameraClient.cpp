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

#define LOG_TAG "CameraClient"
//#define LOG_NDEBUG 0

#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <gui/Surface.h>
#include <media/hardware/HardwareAPI.h>

#include "api1/CameraClient.h"
#include "device1/CameraHardwareInterface.h"
#include "CameraService.h"
#include "utils/CameraThreadState.h"

namespace android {

#define LOG1(...) ALOGD_IF(gLogLevel >= 1, __VA_ARGS__);
#define LOG2(...) ALOGD_IF(gLogLevel >= 2, __VA_ARGS__);

CameraClient::CameraClient(const sp<CameraService>& cameraService,
        const sp<hardware::ICameraClient>& cameraClient,
        const String16& clientPackageName,
        int cameraId, int cameraFacing,
        int clientPid, int clientUid,
        int servicePid):
        Client(cameraService, cameraClient, clientPackageName,
                String8::format("%d", cameraId), cameraId, cameraFacing, clientPid,
                clientUid, servicePid)
{
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("CameraClient::CameraClient E (pid %d, id %d)", callingPid, cameraId);

    mHardware = NULL;
    mMsgEnabled = 0;
    mSurface = 0;
    mPreviewWindow = 0;
    mDestructionStarted = false;

    // Callback is disabled by default
    mPreviewCallbackFlag = CAMERA_FRAME_CALLBACK_FLAG_NOOP;
    mOrientation = getOrientation(0, mCameraFacing == CAMERA_FACING_FRONT);
    mPlayShutterSound = true;
    LOG1("CameraClient::CameraClient X (pid %d, id %d)", callingPid, cameraId);
}

status_t CameraClient::initialize(sp<CameraProviderManager> manager,
        const String8& /*monitorTags*/) {
    int callingPid = CameraThreadState::getCallingPid();
    status_t res;

    LOG1("CameraClient::initialize E (pid %d, id %d)", callingPid, mCameraId);

    // Verify ops permissions
    res = startCameraOps();
    if (res != OK) {
        return res;
    }

    char camera_device_name[10];
    snprintf(camera_device_name, sizeof(camera_device_name), "%d", mCameraId);

    mHardware = new CameraHardwareInterface(camera_device_name);
    res = mHardware->initialize(manager);
    if (res != OK) {
        ALOGE("%s: Camera %d: unable to initialize device: %s (%d)",
                __FUNCTION__, mCameraId, strerror(-res), res);
        mHardware.clear();
        return res;
    }

    mHardware->setCallbacks(notifyCallback,
            dataCallback,
            dataCallbackTimestamp,
            handleCallbackTimestampBatch,
            (void *)(uintptr_t)mCameraId);

    // Enable zoom, error, focus, and metadata messages by default
    enableMsgType(CAMERA_MSG_ERROR | CAMERA_MSG_ZOOM | CAMERA_MSG_FOCUS |
                  CAMERA_MSG_PREVIEW_METADATA | CAMERA_MSG_FOCUS_MOVE);

    LOG1("CameraClient::initialize X (pid %d, id %d)", callingPid, mCameraId);
    return OK;
}


// tear down the client
CameraClient::~CameraClient() {
    mDestructionStarted = true;
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("CameraClient::~CameraClient E (pid %d, this %p)", callingPid, this);

    disconnect();
    LOG1("CameraClient::~CameraClient X (pid %d, this %p)", callingPid, this);
}

status_t CameraClient::dump(int fd, const Vector<String16>& args) {
    return BasicClient::dump(fd, args);
}

status_t CameraClient::dumpClient(int fd, const Vector<String16>& args) {
    const size_t SIZE = 256;
    char buffer[SIZE];

    size_t len = snprintf(buffer, SIZE, "Client[%d] (%p) with UID %d\n",
            mCameraId,
            (getRemoteCallback() != NULL ?
                    IInterface::asBinder(getRemoteCallback()).get() : NULL),
            mClientUid);
    len = (len > SIZE - 1) ? SIZE - 1 : len;
    write(fd, buffer, len);

    len = snprintf(buffer, SIZE, "Latest set parameters:\n");
    len = (len > SIZE - 1) ? SIZE - 1 : len;
    write(fd, buffer, len);

    mLatestSetParameters.dump(fd, args);

    const char *enddump = "\n\n";
    write(fd, enddump, strlen(enddump));

    sp<CameraHardwareInterface> hardware = mHardware;
    if (hardware != nullptr) {
        return hardware->dump(fd, args);
    }
    ALOGI("%s: camera device closed already, skip dumping", __FUNCTION__);
    return OK;
}

// ----------------------------------------------------------------------------

status_t CameraClient::checkPid() const {
    int callingPid = CameraThreadState::getCallingPid();
    if (callingPid == mClientPid) return NO_ERROR;

    ALOGW("attempt to use a locked camera from a different process"
         " (old pid %d, new pid %d)", mClientPid, callingPid);
    return EBUSY;
}

status_t CameraClient::checkPidAndHardware() const {
    if (mHardware == 0) {
        ALOGE("attempt to use a camera after disconnect() (pid %d)",
              CameraThreadState::getCallingPid());
        return INVALID_OPERATION;
    }
    status_t result = checkPid();
    if (result != NO_ERROR) return result;
    return NO_ERROR;
}

status_t CameraClient::lock() {
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("lock (pid %d)", callingPid);
    Mutex::Autolock lock(mLock);

    // lock camera to this client if the the camera is unlocked
    if (mClientPid == 0) {
        mClientPid = callingPid;
        return NO_ERROR;
    }

    // returns NO_ERROR if the client already owns the camera, EBUSY otherwise
    return checkPid();
}

status_t CameraClient::unlock() {
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("unlock (pid %d)", callingPid);
    Mutex::Autolock lock(mLock);

    // allow anyone to use camera (after they lock the camera)
    status_t result = checkPid();
    if (result == NO_ERROR) {
        if (mHardware->recordingEnabled()) {
            ALOGE("Not allowed to unlock camera during recording.");
            return INVALID_OPERATION;
        }
        mClientPid = 0;
        LOG1("clear mRemoteCallback (pid %d)", callingPid);
        // we need to remove the reference to ICameraClient so that when the app
        // goes away, the reference count goes to 0.
        mRemoteCallback.clear();
    }
    return result;
}

// connect a new client to the camera
status_t CameraClient::connect(const sp<hardware::ICameraClient>& client) {
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("connect E (pid %d)", callingPid);
    Mutex::Autolock lock(mLock);

    if (mClientPid != 0 && checkPid() != NO_ERROR) {
        ALOGW("Tried to connect to a locked camera (old pid %d, new pid %d)",
                mClientPid, callingPid);
        return EBUSY;
    }

    if (mRemoteCallback != 0 &&
        (IInterface::asBinder(client) == IInterface::asBinder(mRemoteCallback))) {
        LOG1("Connect to the same client");
        return NO_ERROR;
    }

    mPreviewCallbackFlag = CAMERA_FRAME_CALLBACK_FLAG_NOOP;
    mClientPid = callingPid;
    mRemoteCallback = client;

    LOG1("connect X (pid %d)", callingPid);
    return NO_ERROR;
}

static void disconnectWindow(const sp<ANativeWindow>& window) {
    if (window != 0) {
        status_t result = native_window_api_disconnect(window.get(),
                NATIVE_WINDOW_API_CAMERA);
        if (result != NO_ERROR) {
            ALOGW("native_window_api_disconnect failed: %s (%d)", strerror(-result),
                    result);
        }
    }
}

binder::Status CameraClient::disconnect() {
    int callingPid = CameraThreadState::getCallingPid();
    LOG1("disconnect E (pid %d)", callingPid);
    Mutex::Autolock lock(mLock);

    binder::Status res = binder::Status::ok();
    // Allow both client and the cameraserver to disconnect at all times
    if (callingPid != mClientPid && callingPid != mServicePid) {
        ALOGW("different client - don't disconnect");
        return res;
    }

    // Make sure disconnect() is done once and once only, whether it is called
    // from the user directly, or called by the destructor.
    if (mHardware == 0) return res;

    LOG1("hardware teardown");
    // Before destroying mHardware, we must make sure it's in the
    // idle state.
    // Turn off all messages.
    disableMsgType(CAMERA_MSG_ALL_MSGS);
    mHardware->stopPreview();
    sCameraService->updateProxyDeviceState(
            hardware::ICameraServiceProxy::CAMERA_STATE_IDLE,
            mCameraIdStr, mCameraFacing, mClientPackageName,
            hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1);
    mHardware->cancelPicture();
    // Release the hardware resources.
    mHardware->release();

    // Release the held ANativeWindow resources.
    if (mPreviewWindow != 0) {
        disconnectWindow(mPreviewWindow);
        mPreviewWindow = 0;
        mHardware->setPreviewWindow(mPreviewWindow);
    }
    mHardware.clear();

    CameraService::Client::disconnect();

    LOG1("disconnect X (pid %d)", callingPid);

    return res;
}

// ----------------------------------------------------------------------------

status_t CameraClient::setPreviewWindow(const sp<IBinder>& binder,
        const sp<ANativeWindow>& window) {
    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    // return if no change in surface.
    if (binder == mSurface) {
        return NO_ERROR;
    }

    if (window != 0) {
        result = native_window_api_connect(window.get(), NATIVE_WINDOW_API_CAMERA);
        if (result != NO_ERROR) {
            ALOGE("native_window_api_connect failed: %s (%d)", strerror(-result),
                    result);
            return result;
        }
    }

    // If preview has been already started, register preview buffers now.
    if (mHardware->previewEnabled()) {
        if (window != 0) {
            mHardware->setPreviewScalingMode(NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
            mHardware->setPreviewTransform(mOrientation);
            result = mHardware->setPreviewWindow(window);
        }
    }

    if (result == NO_ERROR) {
        // Everything has succeeded.  Disconnect the old window and remember the
        // new window.
        disconnectWindow(mPreviewWindow);
        mSurface = binder;
        mPreviewWindow = window;
    } else {
        // Something went wrong after we connected to the new window, so
        // disconnect here.
        disconnectWindow(window);
    }

    return result;
}

// set the buffer consumer that the preview will use
status_t CameraClient::setPreviewTarget(
        const sp<IGraphicBufferProducer>& bufferProducer) {
    LOG1("setPreviewTarget(%p) (pid %d)", bufferProducer.get(),
            CameraThreadState::getCallingPid());

    sp<IBinder> binder;
    sp<ANativeWindow> window;
    if (bufferProducer != 0) {
        binder = IInterface::asBinder(bufferProducer);
        // Using controlledByApp flag to ensure that the buffer queue remains in
        // async mode for the old camera API, where many applications depend
        // on that behavior.
        window = new Surface(bufferProducer, /*controlledByApp*/ true);
    }
    return setPreviewWindow(binder, window);
}

// set the preview callback flag to affect how the received frames from
// preview are handled.
void CameraClient::setPreviewCallbackFlag(int callback_flag) {
    LOG1("setPreviewCallbackFlag(%d) (pid %d)", callback_flag, CameraThreadState::getCallingPid());
    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) return;

    mPreviewCallbackFlag = callback_flag;
    if (mPreviewCallbackFlag & CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK) {
        enableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    } else {
        disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    }
}

status_t CameraClient::setPreviewCallbackTarget(
        const sp<IGraphicBufferProducer>& callbackProducer) {
    (void)callbackProducer;
    ALOGE("%s: Unimplemented!", __FUNCTION__);
    return INVALID_OPERATION;
}

// start preview mode
status_t CameraClient::startPreview() {
    LOG1("startPreview (pid %d)", CameraThreadState::getCallingPid());
    return startCameraMode(CAMERA_PREVIEW_MODE);
}

// start recording mode
status_t CameraClient::startRecording() {
    LOG1("startRecording (pid %d)", CameraThreadState::getCallingPid());
    return startCameraMode(CAMERA_RECORDING_MODE);
}

// start preview or recording
status_t CameraClient::startCameraMode(camera_mode mode) {
    LOG1("startCameraMode(%d)", mode);
    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    switch(mode) {
        case CAMERA_PREVIEW_MODE:
            if (mSurface == 0 && mPreviewWindow == 0) {
                LOG1("mSurface is not set yet.");
                // still able to start preview in this case.
            }
            return startPreviewMode();
        case CAMERA_RECORDING_MODE:
            if (mSurface == 0 && mPreviewWindow == 0) {
                ALOGE("mSurface or mPreviewWindow must be set before startRecordingMode.");
                return INVALID_OPERATION;
            }
            return startRecordingMode();
        default:
            return UNKNOWN_ERROR;
    }
}

status_t CameraClient::startPreviewMode() {
    LOG1("startPreviewMode");
    status_t result = NO_ERROR;

    // if preview has been enabled, nothing needs to be done
    if (mHardware->previewEnabled()) {
        return NO_ERROR;
    }

    if (mPreviewWindow != 0) {
        mHardware->setPreviewScalingMode(
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        mHardware->setPreviewTransform(mOrientation);
    }
    mHardware->setPreviewWindow(mPreviewWindow);
    result = mHardware->startPreview();
    if (result == NO_ERROR) {
        sCameraService->updateProxyDeviceState(
            hardware::ICameraServiceProxy::CAMERA_STATE_ACTIVE,
            mCameraIdStr, mCameraFacing, mClientPackageName,
            hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1);
    }
    return result;
}

status_t CameraClient::startRecordingMode() {
    LOG1("startRecordingMode");
    status_t result = NO_ERROR;

    // if recording has been enabled, nothing needs to be done
    if (mHardware->recordingEnabled()) {
        return NO_ERROR;
    }

    // if preview has not been started, start preview first
    if (!mHardware->previewEnabled()) {
        result = startPreviewMode();
        if (result != NO_ERROR) {
            return result;
        }
    }

    // start recording mode
    enableMsgType(CAMERA_MSG_VIDEO_FRAME);
    sCameraService->playSound(CameraService::SOUND_RECORDING_START);
    result = mHardware->startRecording();
    if (result != NO_ERROR) {
        ALOGE("mHardware->startRecording() failed with status %d", result);
    }
    return result;
}

// stop preview mode
void CameraClient::stopPreview() {
    LOG1("stopPreview (pid %d)", CameraThreadState::getCallingPid());
    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) return;


    disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    mHardware->stopPreview();
    sCameraService->updateProxyDeviceState(
        hardware::ICameraServiceProxy::CAMERA_STATE_IDLE,
        mCameraIdStr, mCameraFacing, mClientPackageName,
        hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1);
    mPreviewBuffer.clear();
}

// stop recording mode
void CameraClient::stopRecording() {
    LOG1("stopRecording (pid %d)", CameraThreadState::getCallingPid());
    {
        Mutex::Autolock lock(mLock);
        if (checkPidAndHardware() != NO_ERROR) return;

        disableMsgType(CAMERA_MSG_VIDEO_FRAME);
        mHardware->stopRecording();
        sCameraService->playSound(CameraService::SOUND_RECORDING_STOP);

        mPreviewBuffer.clear();
    }

    {
        Mutex::Autolock l(mAvailableCallbackBuffersLock);
        if (!mAvailableCallbackBuffers.empty()) {
            mAvailableCallbackBuffers.clear();
        }
    }
}

// release a recording frame
void CameraClient::releaseRecordingFrame(const sp<IMemory>& mem) {
    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) return;
    if (mem == nullptr) {
        android_errorWriteWithInfoLog(CameraService::SN_EVENT_LOG_ID, "26164272",
                CameraThreadState::getCallingUid(), nullptr, 0);
        return;
    }

    mHardware->releaseRecordingFrame(mem);
}

void CameraClient::releaseRecordingFrameHandle(native_handle_t *handle) {
    if (handle == nullptr) return;
    Mutex::Autolock lock(mLock);
    sp<IMemory> dataPtr;
    {
        Mutex::Autolock l(mAvailableCallbackBuffersLock);
        if (!mAvailableCallbackBuffers.empty()) {
            dataPtr = mAvailableCallbackBuffers.back();
            mAvailableCallbackBuffers.pop_back();
        }
    }

    if (dataPtr == nullptr) {
        ALOGE("%s: %d: No callback buffer available. Dropping a native handle.", __FUNCTION__,
                __LINE__);
        native_handle_close(handle);
        native_handle_delete(handle);
        return;
    } else if (dataPtr->size() != sizeof(VideoNativeHandleMetadata)) {
        ALOGE("%s: %d: Callback buffer size doesn't match VideoNativeHandleMetadata", __FUNCTION__,
                __LINE__);
        native_handle_close(handle);
        native_handle_delete(handle);
        return;
    }

    if (mHardware != nullptr) {
        VideoNativeHandleMetadata *metadata = (VideoNativeHandleMetadata*)(dataPtr->pointer());
        metadata->eType = kMetadataBufferTypeNativeHandleSource;
        metadata->pHandle = handle;
        mHardware->releaseRecordingFrame(dataPtr);
    }
}

void CameraClient::releaseRecordingFrameHandleBatch(const std::vector<native_handle_t*>& handles) {
    Mutex::Autolock lock(mLock);
    bool disconnected = (mHardware == nullptr);
    size_t n = handles.size();
    std::vector<sp<IMemory>> frames;
    if (!disconnected) {
        frames.reserve(n);
    }
    bool error = false;
    for (auto& handle : handles) {
        sp<IMemory> dataPtr;
        {
            Mutex::Autolock l(mAvailableCallbackBuffersLock);
            if (!mAvailableCallbackBuffers.empty()) {
                dataPtr = mAvailableCallbackBuffers.back();
                mAvailableCallbackBuffers.pop_back();
            }
        }

        if (dataPtr == nullptr) {
            ALOGE("%s: %d: No callback buffer available. Dropping frames.", __FUNCTION__,
                    __LINE__);
            error = true;
            break;
        } else if (dataPtr->size() != sizeof(VideoNativeHandleMetadata)) {
            ALOGE("%s: %d: Callback buffer must be VideoNativeHandleMetadata", __FUNCTION__,
                    __LINE__);
            error = true;
            break;
        }

        if (!disconnected) {
            VideoNativeHandleMetadata *metadata = (VideoNativeHandleMetadata*)(dataPtr->pointer());
            metadata->eType = kMetadataBufferTypeNativeHandleSource;
            metadata->pHandle = handle;
            frames.push_back(dataPtr);
        }
    }

    if (error) {
        for (auto& handle : handles) {
            native_handle_close(handle);
            native_handle_delete(handle);
        }
    } else if (!disconnected) {
        mHardware->releaseRecordingFrameBatch(frames);
    }
    return;
}

status_t CameraClient::setVideoBufferMode(int32_t videoBufferMode) {
    LOG1("setVideoBufferMode: %d", videoBufferMode);
    bool enableMetadataInBuffers = false;

    if (videoBufferMode == VIDEO_BUFFER_MODE_DATA_CALLBACK_METADATA) {
        enableMetadataInBuffers = true;
    } else if (videoBufferMode != VIDEO_BUFFER_MODE_DATA_CALLBACK_YUV) {
        ALOGE("%s: %d: videoBufferMode %d is not supported.", __FUNCTION__, __LINE__,
                videoBufferMode);
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) {
        return UNKNOWN_ERROR;
    }

    return mHardware->storeMetaDataInBuffers(enableMetadataInBuffers);
}

bool CameraClient::previewEnabled() {
    LOG1("previewEnabled (pid %d)", CameraThreadState::getCallingPid());

    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) return false;
    return mHardware->previewEnabled();
}

bool CameraClient::recordingEnabled() {
    LOG1("recordingEnabled (pid %d)", CameraThreadState::getCallingPid());

    Mutex::Autolock lock(mLock);
    if (checkPidAndHardware() != NO_ERROR) return false;
    return mHardware->recordingEnabled();
}

status_t CameraClient::autoFocus() {
    LOG1("autoFocus (pid %d)", CameraThreadState::getCallingPid());

    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    return mHardware->autoFocus();
}

status_t CameraClient::cancelAutoFocus() {
    LOG1("cancelAutoFocus (pid %d)", CameraThreadState::getCallingPid());

    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    return mHardware->cancelAutoFocus();
}

// take a picture - image is returned in callback
status_t CameraClient::takePicture(int msgType) {
    LOG1("takePicture (pid %d): 0x%x", CameraThreadState::getCallingPid(), msgType);

    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    if ((msgType & CAMERA_MSG_RAW_IMAGE) &&
        (msgType & CAMERA_MSG_RAW_IMAGE_NOTIFY)) {
        ALOGE("CAMERA_MSG_RAW_IMAGE and CAMERA_MSG_RAW_IMAGE_NOTIFY"
                " cannot be both enabled");
        return BAD_VALUE;
    }

    // We only accept picture related message types
    // and ignore other types of messages for takePicture().
    int picMsgType = msgType
                        & (CAMERA_MSG_SHUTTER |
                           CAMERA_MSG_POSTVIEW_FRAME |
                           CAMERA_MSG_RAW_IMAGE |
                           CAMERA_MSG_RAW_IMAGE_NOTIFY |
                           CAMERA_MSG_COMPRESSED_IMAGE);

    enableMsgType(picMsgType);

    return mHardware->takePicture();
}

// set preview/capture parameters - key/value pairs
status_t CameraClient::setParameters(const String8& params) {
    LOG1("setParameters (pid %d) (%s)", CameraThreadState::getCallingPid(), params.string());

    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    mLatestSetParameters = CameraParameters(params);
    CameraParameters p(params);
    return mHardware->setParameters(p);
}

// get preview/capture parameters - key/value pairs
String8 CameraClient::getParameters() const {
    Mutex::Autolock lock(mLock);
    // The camera service can unconditionally get the parameters at all times
    if (CameraThreadState::getCallingPid() != mServicePid && checkPidAndHardware() != NO_ERROR) {
        return String8();
    }

    String8 params(mHardware->getParameters().flatten());
    LOG1("getParameters (pid %d) (%s)", CameraThreadState::getCallingPid(), params.string());
    return params;
}

// enable shutter sound
status_t CameraClient::enableShutterSound(bool enable) {
    LOG1("enableShutterSound (pid %d)", CameraThreadState::getCallingPid());

    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    if (enable) {
        mPlayShutterSound = true;
        return OK;
    }

    mPlayShutterSound = false;
    return OK;
}

status_t CameraClient::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) {
    LOG1("sendCommand (pid %d)", CameraThreadState::getCallingPid());
    int orientation;
    Mutex::Autolock lock(mLock);
    status_t result = checkPidAndHardware();
    if (result != NO_ERROR) return result;

    if (cmd == CAMERA_CMD_SET_DISPLAY_ORIENTATION) {
        // Mirror the preview if the camera is front-facing.
        orientation = getOrientation(arg1, mCameraFacing == CAMERA_FACING_FRONT);
        if (orientation == -1) return BAD_VALUE;

        if (mOrientation != orientation) {
            mOrientation = orientation;
            if (mPreviewWindow != 0) {
                mHardware->setPreviewTransform(mOrientation);
            }
        }
        return OK;
    } else if (cmd == CAMERA_CMD_ENABLE_SHUTTER_SOUND) {
        switch (arg1) {
            case 0:
                return enableShutterSound(false);
            case 1:
                return enableShutterSound(true);
            default:
                return BAD_VALUE;
        }
        return OK;
    } else if (cmd == CAMERA_CMD_PLAY_RECORDING_SOUND) {
        sCameraService->playSound(CameraService::SOUND_RECORDING_START);
    } else if (cmd == CAMERA_CMD_SET_VIDEO_BUFFER_COUNT) {
        // Silently ignore this command
        return INVALID_OPERATION;
    } else if (cmd == CAMERA_CMD_PING) {
        // If mHardware is 0, checkPidAndHardware will return error.
        return OK;
    }

    return mHardware->sendCommand(cmd, arg1, arg2);
}

// ----------------------------------------------------------------------------

void CameraClient::enableMsgType(int32_t msgType) {
    android_atomic_or(msgType, &mMsgEnabled);
    mHardware->enableMsgType(msgType);
}

void CameraClient::disableMsgType(int32_t msgType) {
    android_atomic_and(~msgType, &mMsgEnabled);
    mHardware->disableMsgType(msgType);
}

#define CHECK_MESSAGE_INTERVAL 10 // 10ms
bool CameraClient::lockIfMessageWanted(int32_t msgType) {
    int sleepCount = 0;
    while (mMsgEnabled & msgType) {
        if (mLock.tryLock() == NO_ERROR) {
            if (sleepCount > 0) {
                LOG1("lockIfMessageWanted(%d): waited for %d ms",
                    msgType, sleepCount * CHECK_MESSAGE_INTERVAL);
            }

            // If messages are no longer enabled after acquiring lock, release and drop message
            if ((mMsgEnabled & msgType) == 0) {
                mLock.unlock();
                break;
            }

            return true;
        }
        if (sleepCount++ == 0) {
            LOG1("lockIfMessageWanted(%d): enter sleep", msgType);
        }
        usleep(CHECK_MESSAGE_INTERVAL * 1000);
    }
    ALOGW("lockIfMessageWanted(%d): dropped unwanted message", msgType);
    return false;
}

sp<CameraClient> CameraClient::getClientFromCookie(void* user) {
    String8 cameraId = String8::format("%d", (int)(intptr_t) user);
    auto clientDescriptor = sCameraService->mActiveClientManager.get(cameraId);
    if (clientDescriptor != nullptr) {
        return sp<CameraClient>{
                static_cast<CameraClient*>(clientDescriptor->getValue().get())};
    }
    return sp<CameraClient>{nullptr};
}

// Callback messages can be dispatched to internal handlers or pass to our
// client's callback functions, depending on the message type.
//
// notifyCallback:
//      CAMERA_MSG_SHUTTER              handleShutter
//      (others)                        c->notifyCallback
// dataCallback:
//      CAMERA_MSG_PREVIEW_FRAME        handlePreviewData
//      CAMERA_MSG_POSTVIEW_FRAME       handlePostview
//      CAMERA_MSG_RAW_IMAGE            handleRawPicture
//      CAMERA_MSG_COMPRESSED_IMAGE     handleCompressedPicture
//      (others)                        c->dataCallback
// dataCallbackTimestamp
//      (others)                        c->dataCallbackTimestamp

void CameraClient::notifyCallback(int32_t msgType, int32_t ext1,
        int32_t ext2, void* user) {
    LOG2("notifyCallback(%d)", msgType);

    sp<CameraClient> client = getClientFromCookie(user);
    if (client.get() == nullptr) return;

    if (!client->lockIfMessageWanted(msgType)) return;

    switch (msgType) {
        case CAMERA_MSG_SHUTTER:
            // ext1 is the dimension of the yuv picture.
            client->handleShutter();
            break;
        default:
            client->handleGenericNotify(msgType, ext1, ext2);
            break;
    }
}

void CameraClient::dataCallback(int32_t msgType,
        const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata, void* user) {
    LOG2("dataCallback(%d)", msgType);

    sp<CameraClient> client = getClientFromCookie(user);
    if (client.get() == nullptr) return;

    if (!client->lockIfMessageWanted(msgType)) return;
    if (dataPtr == 0 && metadata == NULL) {
        ALOGE("Null data returned in data callback");
        client->handleGenericNotify(CAMERA_MSG_ERROR, UNKNOWN_ERROR, 0);
        return;
    }

    switch (msgType & ~CAMERA_MSG_PREVIEW_METADATA) {
        case CAMERA_MSG_PREVIEW_FRAME:
            client->handlePreviewData(msgType, dataPtr, metadata);
            break;
        case CAMERA_MSG_POSTVIEW_FRAME:
            client->handlePostview(dataPtr);
            break;
        case CAMERA_MSG_RAW_IMAGE:
            client->handleRawPicture(dataPtr);
            break;
        case CAMERA_MSG_COMPRESSED_IMAGE:
            client->handleCompressedPicture(dataPtr);
            break;
        default:
            client->handleGenericData(msgType, dataPtr, metadata);
            break;
    }
}

void CameraClient::dataCallbackTimestamp(nsecs_t timestamp,
        int32_t msgType, const sp<IMemory>& dataPtr, void* user) {
    LOG2("dataCallbackTimestamp(%d)", msgType);

    sp<CameraClient> client = getClientFromCookie(user);
    if (client.get() == nullptr) return;

    if (!client->lockIfMessageWanted(msgType)) return;

    if (dataPtr == 0) {
        ALOGE("Null data returned in data with timestamp callback");
        client->handleGenericNotify(CAMERA_MSG_ERROR, UNKNOWN_ERROR, 0);
        return;
    }

    client->handleGenericDataTimestamp(timestamp, msgType, dataPtr);
}

void CameraClient::handleCallbackTimestampBatch(
        int32_t msgType, const std::vector<HandleTimestampMessage>& msgs, void* user) {
    LOG2("dataCallbackTimestampBatch");
    sp<CameraClient> client = getClientFromCookie(user);
    if (client.get() == nullptr) return;
    if (!client->lockIfMessageWanted(msgType)) return;

    sp<hardware::ICameraClient> c = client->mRemoteCallback;
    client->mLock.unlock();
    if (c != 0 && msgs.size() > 0) {
        size_t n = msgs.size();
        std::vector<nsecs_t> timestamps;
        std::vector<native_handle_t*> handles;
        timestamps.reserve(n);
        handles.reserve(n);
        for (auto& msg : msgs) {
            native_handle_t* handle = nullptr;
            if (msg.dataPtr->size() != sizeof(VideoNativeHandleMetadata)) {
                ALOGE("%s: dataPtr does not contain VideoNativeHandleMetadata!", __FUNCTION__);
                return;
            }
            VideoNativeHandleMetadata *metadata =
                (VideoNativeHandleMetadata*)(msg.dataPtr->pointer());
            if (metadata->eType == kMetadataBufferTypeNativeHandleSource) {
                handle = metadata->pHandle;
            }

            if (handle == nullptr) {
                ALOGE("%s: VideoNativeHandleMetadata type mismatch or null handle passed!",
                        __FUNCTION__);
                return;
            }
            {
                Mutex::Autolock l(client->mAvailableCallbackBuffersLock);
                client->mAvailableCallbackBuffers.push_back(msg.dataPtr);
            }
            timestamps.push_back(msg.timestamp);
            handles.push_back(handle);
        }
        c->recordingFrameHandleCallbackTimestampBatch(timestamps, handles);
    }
}

// snapshot taken callback
void CameraClient::handleShutter(void) {
    if (mPlayShutterSound) {
        sCameraService->playSound(CameraService::SOUND_SHUTTER);
    }

    sp<hardware::ICameraClient> c = mRemoteCallback;
    if (c != 0) {
        mLock.unlock();
        c->notifyCallback(CAMERA_MSG_SHUTTER, 0, 0);
        if (!lockIfMessageWanted(CAMERA_MSG_SHUTTER)) return;
    }
    disableMsgType(CAMERA_MSG_SHUTTER);

    // Shutters only happen in response to takePicture, so mark device as
    // idle now, until preview is restarted
    sCameraService->updateProxyDeviceState(
        hardware::ICameraServiceProxy::CAMERA_STATE_IDLE,
        mCameraIdStr, mCameraFacing, mClientPackageName,
        hardware::ICameraServiceProxy::CAMERA_API_LEVEL_1);

    mLock.unlock();
}

// preview callback - frame buffer update
void CameraClient::handlePreviewData(int32_t msgType,
                                              const sp<IMemory>& mem,
                                              camera_frame_metadata_t *metadata) {
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = mem->getMemory(&offset, &size);

    // local copy of the callback flags
    int flags = mPreviewCallbackFlag;

    // is callback enabled?
    if (!(flags & CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK)) {
        // If the enable bit is off, the copy-out and one-shot bits are ignored
        LOG2("frame callback is disabled");
        mLock.unlock();
        return;
    }

    // hold a strong pointer to the client
    sp<hardware::ICameraClient> c = mRemoteCallback;

    // clear callback flags if no client or one-shot mode
    if (c == 0 || (mPreviewCallbackFlag & CAMERA_FRAME_CALLBACK_FLAG_ONE_SHOT_MASK)) {
        LOG2("Disable preview callback");
        mPreviewCallbackFlag &= ~(CAMERA_FRAME_CALLBACK_FLAG_ONE_SHOT_MASK |
                                  CAMERA_FRAME_CALLBACK_FLAG_COPY_OUT_MASK |
                                  CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);
        disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    }

    if (c != 0) {
        // Is the received frame copied out or not?
        if (flags & CAMERA_FRAME_CALLBACK_FLAG_COPY_OUT_MASK) {
            LOG2("frame is copied");
            copyFrameAndPostCopiedFrame(msgType, c, heap, offset, size, metadata);
        } else {
            LOG2("frame is forwarded");
            mLock.unlock();
            c->dataCallback(msgType, mem, metadata);
        }
    } else {
        mLock.unlock();
    }
}

// picture callback - postview image ready
void CameraClient::handlePostview(const sp<IMemory>& mem) {
    disableMsgType(CAMERA_MSG_POSTVIEW_FRAME);

    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0) {
        c->dataCallback(CAMERA_MSG_POSTVIEW_FRAME, mem, NULL);
    }
}

// picture callback - raw image ready
void CameraClient::handleRawPicture(const sp<IMemory>& mem) {
    disableMsgType(CAMERA_MSG_RAW_IMAGE);

    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = mem->getMemory(&offset, &size);

    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0) {
        c->dataCallback(CAMERA_MSG_RAW_IMAGE, mem, NULL);
    }
}

// picture callback - compressed picture ready
void CameraClient::handleCompressedPicture(const sp<IMemory>& mem) {
    disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);

    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0) {
        c->dataCallback(CAMERA_MSG_COMPRESSED_IMAGE, mem, NULL);
    }
}


void CameraClient::handleGenericNotify(int32_t msgType,
    int32_t ext1, int32_t ext2) {
    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0) {
        c->notifyCallback(msgType, ext1, ext2);
    }
}

void CameraClient::handleGenericData(int32_t msgType,
    const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {
    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0) {
        c->dataCallback(msgType, dataPtr, metadata);
    }
}

void CameraClient::handleGenericDataTimestamp(nsecs_t timestamp,
    int32_t msgType, const sp<IMemory>& dataPtr) {
    sp<hardware::ICameraClient> c = mRemoteCallback;
    mLock.unlock();
    if (c != 0 && dataPtr != nullptr) {
        native_handle_t* handle = nullptr;

        // Check if dataPtr contains a VideoNativeHandleMetadata.
        if (dataPtr->size() == sizeof(VideoNativeHandleMetadata)) {
            VideoNativeHandleMetadata *metadata =
                (VideoNativeHandleMetadata*)(dataPtr->pointer());
            if (metadata->eType == kMetadataBufferTypeNativeHandleSource) {
                handle = metadata->pHandle;
            }
        }

        // If dataPtr contains a native handle, send it via recordingFrameHandleCallbackTimestamp.
        if (handle != nullptr) {
            {
                Mutex::Autolock l(mAvailableCallbackBuffersLock);
                mAvailableCallbackBuffers.push_back(dataPtr);
            }
            c->recordingFrameHandleCallbackTimestamp(timestamp, handle);
        } else {
            c->dataCallbackTimestamp(timestamp, msgType, dataPtr);
        }
    }
}

void CameraClient::copyFrameAndPostCopiedFrame(
        int32_t msgType, const sp<hardware::ICameraClient>& client,
        const sp<IMemoryHeap>& heap, size_t offset, size_t size,
        camera_frame_metadata_t *metadata) {
    LOG2("copyFrameAndPostCopiedFrame");
    // It is necessary to copy out of pmem before sending this to
    // the callback. For efficiency, reuse the same MemoryHeapBase
    // provided it's big enough. Don't allocate the memory or
    // perform the copy if there's no callback.
    // hold the preview lock while we grab a reference to the preview buffer
    sp<MemoryHeapBase> previewBuffer;

    if (mPreviewBuffer == 0) {
        mPreviewBuffer = new MemoryHeapBase(size, 0, NULL);
    } else if (size > mPreviewBuffer->virtualSize()) {
        mPreviewBuffer.clear();
        mPreviewBuffer = new MemoryHeapBase(size, 0, NULL);
    }
    if (mPreviewBuffer == 0) {
        ALOGE("failed to allocate space for preview buffer");
        mLock.unlock();
        return;
    }
    previewBuffer = mPreviewBuffer;

    void* previewBufferBase = previewBuffer->base();
    void* heapBase = heap->base();

    if (heapBase == MAP_FAILED) {
        ALOGE("%s: Failed to mmap heap for preview frame.", __FUNCTION__);
        mLock.unlock();
        return;
    } else if (previewBufferBase == MAP_FAILED) {
        ALOGE("%s: Failed to mmap preview buffer for preview frame.", __FUNCTION__);
        mLock.unlock();
        return;
    }

    memcpy(previewBufferBase, (uint8_t *) heapBase + offset, size);

    sp<MemoryBase> frame = new MemoryBase(previewBuffer, 0, size);
    if (frame == 0) {
        ALOGE("failed to allocate space for frame callback");
        mLock.unlock();
        return;
    }

    mLock.unlock();
    client->dataCallback(msgType, frame, metadata);
}

int CameraClient::getOrientation(int degrees, bool mirror) {
    if (!mirror) {
        if (degrees == 0) return 0;
        else if (degrees == 90) return HAL_TRANSFORM_ROT_90;
        else if (degrees == 180) return HAL_TRANSFORM_ROT_180;
        else if (degrees == 270) return HAL_TRANSFORM_ROT_270;
    } else {  // Do mirror (horizontal flip)
        if (degrees == 0) {           // FLIP_H and ROT_0
            return HAL_TRANSFORM_FLIP_H;
        } else if (degrees == 90) {   // FLIP_H and ROT_90
            return HAL_TRANSFORM_FLIP_H | HAL_TRANSFORM_ROT_90;
        } else if (degrees == 180) {  // FLIP_H and ROT_180
            return HAL_TRANSFORM_FLIP_V;
        } else if (degrees == 270) {  // FLIP_H and ROT_270
            return HAL_TRANSFORM_FLIP_V | HAL_TRANSFORM_ROT_90;
        }
    }
    ALOGE("Invalid setDisplayOrientation degrees=%d", degrees);
    return -1;
}

status_t CameraClient::setVideoTarget(const sp<IGraphicBufferProducer>& bufferProducer) {
    (void)bufferProducer;
    ALOGE("%s: %d: CameraClient doesn't support setting a video target.", __FUNCTION__, __LINE__);
    return INVALID_OPERATION;
}

}; // namespace android
