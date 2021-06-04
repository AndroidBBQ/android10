/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "ACameraDeviceVendor"

#include <vector>
#include <inttypes.h>
#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <android/frameworks/cameraservice/device/2.0/types.h>
#include <CameraMetadata.h>

#include "ndk_vendor/impl/ACameraDevice.h"
#include "ACameraCaptureSession.h"
#include "ACameraMetadata.h"
#include "ACaptureRequest.h"
#include "utils.h"

#include "ACameraCaptureSession.inc"

#define CHECK_TRANSACTION_AND_RET(remoteRet, status, callName) \
    if (!remoteRet.isOk()) { \
        ALOGE("%s: Transaction error during %s call %s", __FUNCTION__, callName, \
                  remoteRet.description().c_str()); \
        return ACAMERA_ERROR_UNKNOWN; \
    } \
    if (status != Status::NO_ERROR) { \
        ALOGE("%s: %s call failed", __FUNCTION__, callName); \
        return utils::convertFromHidl(status); \
    }

using namespace android;

ACameraDevice::~ACameraDevice() {
    mDevice->stopLooperAndDisconnect();
}

namespace android {
namespace acam {

using HCameraMetadata = frameworks::cameraservice::device::V2_0::CameraMetadata;
using OutputConfiguration = frameworks::cameraservice::device::V2_0::OutputConfiguration;
using SessionConfiguration = frameworks::cameraservice::device::V2_0::SessionConfiguration;
using hardware::Void;

// Static member definitions
const char* CameraDevice::kContextKey        = "Context";
const char* CameraDevice::kDeviceKey         = "Device";
const char* CameraDevice::kErrorCodeKey      = "ErrorCode";
const char* CameraDevice::kCallbackFpKey     = "Callback";
const char* CameraDevice::kSessionSpKey      = "SessionSp";
const char* CameraDevice::kCaptureRequestKey = "CaptureRequest";
const char* CameraDevice::kTimeStampKey      = "TimeStamp";
const char* CameraDevice::kCaptureResultKey  = "CaptureResult";
const char* CameraDevice::kPhysicalCaptureResultKey = "PhysicalCaptureResult";
const char* CameraDevice::kCaptureFailureKey = "CaptureFailure";
const char* CameraDevice::kSequenceIdKey     = "SequenceId";
const char* CameraDevice::kFrameNumberKey    = "FrameNumber";
const char* CameraDevice::kAnwKey            = "Anw";
const char* CameraDevice::kFailingPhysicalCameraId= "FailingPhysicalCameraId";

/**
 * CameraDevice Implementation
 */
CameraDevice::CameraDevice(
        const char* id,
        ACameraDevice_StateCallbacks* cb,
        sp<ACameraMetadata> chars,
        ACameraDevice* wrapper) :
        mCameraId(id),
        mAppCallbacks(*cb),
        mChars(std::move(chars)),
        mServiceCallback(new ServiceCallback(this)),
        mWrapper(wrapper),
        mInError(false),
        mError(ACAMERA_OK),
        mIdle(true),
        mCurrentSession(nullptr) {
    mClosing = false;
    // Setup looper thread to perfrom device callbacks to app
    mCbLooper = new ALooper;
    mCbLooper->setName("C2N-dev-looper");
    status_t err = mCbLooper->start(
            /*runOnCallingThread*/false,
            /*canCallJava*/       true,
            PRIORITY_DEFAULT);
    if (err != OK) {
        ALOGE("%s: Unable to start camera device callback looper: %s (%d)",
                __FUNCTION__, strerror(-err), err);
        setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
    }
    mHandler = new CallbackHandler(id);
    mCbLooper->registerHandler(mHandler);

    const CameraMetadata& metadata = mChars->getInternalData();
    camera_metadata_ro_entry entry = metadata.find(ANDROID_REQUEST_PARTIAL_RESULT_COUNT);
    if (entry.count != 1) {
        ALOGW("%s: bad count %zu for partial result count", __FUNCTION__, entry.count);
        mPartialResultCount = 1;
    } else {
        mPartialResultCount = entry.data.i32[0];
    }

    entry = metadata.find(ANDROID_LENS_INFO_SHADING_MAP_SIZE);
    if (entry.count != 2) {
        ALOGW("%s: bad count %zu for shading map size", __FUNCTION__, entry.count);
        mShadingMapSize[0] = 0;
        mShadingMapSize[1] = 0;
    } else {
        mShadingMapSize[0] = entry.data.i32[0];
        mShadingMapSize[1] = entry.data.i32[1];
    }
}

CameraDevice::~CameraDevice() { }

void
CameraDevice::postSessionMsgAndCleanup(sp<AMessage>& msg) {
    msg->post();
    msg.clear();
    sp<AMessage> cleanupMsg = new AMessage(kWhatCleanUpSessions, mHandler);
    cleanupMsg->post();
}

// TODO: cached created request?
camera_status_t
CameraDevice::createCaptureRequest(
        ACameraDevice_request_template templateId,
        const ACameraIdList* physicalCameraIdList,
        ACaptureRequest** request) const {
    Mutex::Autolock _l(mDeviceLock);
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        return ret;
    }
    if (mRemote == nullptr) {
        return ACAMERA_ERROR_CAMERA_DISCONNECTED;
    }
    CameraMetadata rawRequest;
    Status status = Status::UNKNOWN_ERROR;
    auto remoteRet = mRemote->createDefaultRequest(
        utils::convertToHidl(templateId),
        [&status, &rawRequest](auto s, const hidl_vec<uint8_t> &metadata) {
            status = s;
            if (status == Status::NO_ERROR && utils::convertFromHidlCloned(metadata, &rawRequest)) {
            } else {
                ALOGE("%s: Couldn't create default request", __FUNCTION__);
            }
        });
    CHECK_TRANSACTION_AND_RET(remoteRet, status, "createDefaultRequest()")
    ACaptureRequest* outReq = new ACaptureRequest();
    outReq->settings = new ACameraMetadata(rawRequest.release(), ACameraMetadata::ACM_REQUEST);
    if (physicalCameraIdList != nullptr) {
        for (auto i = 0; i < physicalCameraIdList->numCameras; i++) {
            outReq->physicalSettings.emplace(physicalCameraIdList->cameraIds[i],
                    new ACameraMetadata(*(outReq->settings)));
        }
    }
    outReq->targets  = new ACameraOutputTargets();
    *request = outReq;
    return ACAMERA_OK;
}

camera_status_t
CameraDevice::createCaptureSession(
        const ACaptureSessionOutputContainer*       outputs,
        const ACaptureRequest* sessionParameters,
        const ACameraCaptureSession_stateCallbacks* callbacks,
        /*out*/ACameraCaptureSession** session) {
    sp<ACameraCaptureSession> currentSession = mCurrentSession.promote();
    Mutex::Autolock _l(mDeviceLock);
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        return ret;
    }

    if (currentSession != nullptr) {
        currentSession->closeByDevice();
        stopRepeatingLocked();
    }

    // Create new session
    ret = configureStreamsLocked(outputs, sessionParameters);
    if (ret != ACAMERA_OK) {
        ALOGE("Fail to create new session. cannot configure streams");
        return ret;
    }

    ACameraCaptureSession* newSession = new ACameraCaptureSession(
            mNextSessionId++, outputs, callbacks, this);

    // set new session as current session
    newSession->incStrong((void *) ACameraDevice_createCaptureSession);
    mCurrentSession = newSession;
    mFlushing = false;
    *session = newSession;
    return ACAMERA_OK;
}

camera_status_t CameraDevice::isSessionConfigurationSupported(
        const ACaptureSessionOutputContainer* sessionOutputContainer) const {
    Mutex::Autolock _l(mDeviceLock);
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        return ret;
    }

    SessionConfiguration sessionConfig;
    sessionConfig.inputWidth = 0;
    sessionConfig.inputHeight = 0;
    sessionConfig.inputFormat = -1;
    sessionConfig.operationMode = StreamConfigurationMode::NORMAL_MODE;
    sessionConfig.outputStreams.resize(sessionOutputContainer->mOutputs.size());
    size_t index = 0;
    for (const auto& output : sessionOutputContainer->mOutputs) {
        sessionConfig.outputStreams[index].rotation = utils::convertToHidl(output.mRotation);
        sessionConfig.outputStreams[index].windowGroupId = -1;
        sessionConfig.outputStreams[index].windowHandles.resize(output.mSharedWindows.size() + 1);
        sessionConfig.outputStreams[index].windowHandles[0] = output.mWindow;
        sessionConfig.outputStreams[index].physicalCameraId = output.mPhysicalCameraId;
        index++;
    }

    bool configSupported = false;
    Status status = Status::UNKNOWN_ERROR;
    auto remoteRet = mRemote->isSessionConfigurationSupported(sessionConfig,
        [&status, &configSupported](auto s, auto supported) {
            status = s;
            configSupported = supported;
        });

    CHECK_TRANSACTION_AND_RET(remoteRet, status, "isSessionConfigurationSupported()");
    return configSupported ? ACAMERA_OK : ACAMERA_ERROR_STREAM_CONFIGURE_FAIL;
}

static void addMetadataToPhysicalCameraSettings(const CameraMetadata *metadata,
        const std::string &cameraId, PhysicalCameraSettings *physicalCameraSettings) {
    CameraMetadata metadataCopy = *metadata;
    camera_metadata_t *camera_metadata = metadataCopy.release();
    HCameraMetadata hCameraMetadata;
    utils::convertToHidl(camera_metadata, &hCameraMetadata, /*shouldOwn*/ true);
    physicalCameraSettings->settings.metadata(std::move(hCameraMetadata));
    physicalCameraSettings->id = cameraId;
}

void CameraDevice::addRequestSettingsMetadata(ACaptureRequest *aCaptureRequest,
        sp<CaptureRequest> &req) {
    req->mPhysicalCameraSettings.resize(1 + aCaptureRequest->physicalSettings.size());
    addMetadataToPhysicalCameraSettings(&(aCaptureRequest->settings->getInternalData()), getId(),
                    &(req->mPhysicalCameraSettings[0]));
    size_t i = 1;
    for (auto &physicalSetting : aCaptureRequest->physicalSettings) {
        addMetadataToPhysicalCameraSettings(&(physicalSetting.second->getInternalData()),
                physicalSetting.first, &(req->mPhysicalCameraSettings[i]));
        i++;
    }
}

camera_status_t CameraDevice::updateOutputConfigurationLocked(ACaptureSessionOutput *output) {
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        return ret;
    }

    if (output == nullptr) {
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (!output->mIsShared) {
        ALOGE("Error output configuration is not shared");
        return ACAMERA_ERROR_INVALID_OPERATION;
    }

    int32_t streamId = -1;
    for (auto& kvPair : mConfiguredOutputs) {
        if (utils::isWindowNativeHandleEqual(kvPair.second.first, output->mWindow)) {
            streamId = kvPair.first;
            break;
        }
    }
    if (streamId < 0) {
        ALOGE("Error: Invalid output configuration");
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    OutputConfigurationWrapper outConfigW;
    OutputConfiguration &outConfig = outConfigW.mOutputConfiguration;
    outConfig.rotation = utils::convertToHidl(output->mRotation);
    outConfig.windowHandles.resize(output->mSharedWindows.size() + 1);
    outConfig.windowHandles[0] = output->mWindow;
    outConfig.physicalCameraId = output->mPhysicalCameraId;
    int i = 1;
    for (auto& anw : output->mSharedWindows) {
        outConfig.windowHandles[i++] = anw;
    }

    auto remoteRet = mRemote->updateOutputConfiguration(streamId, outConfig);
    if (!remoteRet.isOk()) {
        ALOGE("%s: Transaction error in updating OutputConfiguration: %s", __FUNCTION__,
              remoteRet.description().c_str());
        return ACAMERA_ERROR_UNKNOWN;
    }

    switch (remoteRet) {
            case Status::NO_ERROR:
                break;
            case Status::INVALID_OPERATION:
                ALOGE("Camera device %s invalid operation", getId());
                return ACAMERA_ERROR_INVALID_OPERATION;
            case Status::ALREADY_EXISTS:
                ALOGE("Camera device %s output surface already exists", getId());
                return ACAMERA_ERROR_INVALID_PARAMETER;
            case Status::ILLEGAL_ARGUMENT:
                ALOGE("Camera device %s invalid input argument", getId());
                return ACAMERA_ERROR_INVALID_PARAMETER;
            default:
                ALOGE("Camera device %s failed to add shared output", getId());
                return ACAMERA_ERROR_UNKNOWN;
    }

    mConfiguredOutputs[streamId] = std::make_pair(output->mWindow, outConfigW);

    return ACAMERA_OK;
}

camera_status_t
CameraDevice::allocateCaptureRequestLocked(
        const ACaptureRequest* request, /*out*/sp<CaptureRequest> &outReq) {
    sp<CaptureRequest> req(new CaptureRequest());
    req->mCaptureRequest.physicalCameraSettings.resize(1 + request->physicalSettings.size());

    size_t index = 0;
    allocateOneCaptureRequestMetadata(
            req->mCaptureRequest.physicalCameraSettings[index++], mCameraId, request->settings);

    for (auto& physicalEntry : request->physicalSettings) {
        allocateOneCaptureRequestMetadata(
                req->mCaptureRequest.physicalCameraSettings[index++],
                physicalEntry.first, physicalEntry.second);
    }

    std::vector<int32_t> requestStreamIdxList;
    std::vector<int32_t> requestSurfaceIdxList;
    for (auto outputTarget : request->targets->mOutputs) {
        native_handle_t* anw = outputTarget.mWindow;
        bool found = false;
        req->mSurfaceList.push_back(anw);
        // lookup stream/surface ID
        for (const auto& kvPair : mConfiguredOutputs) {
            int streamId = kvPair.first;
            const OutputConfigurationWrapper& outConfig = kvPair.second.second;
            const auto& windowHandles = outConfig.mOutputConfiguration.windowHandles;
            for (int surfaceId = 0; surfaceId < (int) windowHandles.size(); surfaceId++) {
                // If two native handles are equivalent, so are their surfaces.
                if (utils::isWindowNativeHandleEqual(windowHandles[surfaceId].getNativeHandle(),
                                                      anw)) {
                    found = true;
                    requestStreamIdxList.push_back(streamId);
                    requestSurfaceIdxList.push_back(surfaceId);
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        if (!found) {
            ALOGE("Unconfigured output target %p in capture request!", anw);
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
    }
    req->mCaptureRequest.streamAndWindowIds.resize(requestStreamIdxList.size());
    for (int i = 0; i < requestStreamIdxList.size(); i++) {
        req->mCaptureRequest.streamAndWindowIds[i].streamId = requestStreamIdxList[i];
        req->mCaptureRequest.streamAndWindowIds[i].windowId = requestSurfaceIdxList[i];
    }
    outReq = req;
    return ACAMERA_OK;
}

void CameraDevice::allocateOneCaptureRequestMetadata(
        PhysicalCameraSettings& cameraSettings,
        const std::string& id, const sp<ACameraMetadata>& metadata) {
    cameraSettings.id = id;
    // TODO: Do we really need to copy the metadata here ?
    CameraMetadata metadataCopy = metadata->getInternalData();
    camera_metadata_t *cameraMetadata = metadataCopy.release();
    HCameraMetadata hCameraMetadata;
    utils::convertToHidl(cameraMetadata, &hCameraMetadata, true);
    if (metadata != nullptr) {
        if (hCameraMetadata.data() != nullptr &&
            mCaptureRequestMetadataQueue != nullptr &&
            mCaptureRequestMetadataQueue->write(
                reinterpret_cast<const uint8_t *>(hCameraMetadata.data()),
                hCameraMetadata.size())) {
            // The metadata field of the union would've been destructued, so no need
            // to re-size it.
            cameraSettings.settings.fmqMetadataSize(hCameraMetadata.size());
        } else {
            ALOGE("Fmq write capture result failed, falling back to hwbinder");
            cameraSettings.settings.metadata(std::move(hCameraMetadata));
        }
    }
}


ACaptureRequest*
CameraDevice::allocateACaptureRequest(sp<CaptureRequest>& req, const char* deviceId) {
    ACaptureRequest* pRequest = new ACaptureRequest();
    for (size_t i = 0; i < req->mPhysicalCameraSettings.size(); i++) {
        const std::string& id = req->mPhysicalCameraSettings[i].id;
        CameraMetadata clone;
        utils::convertFromHidlCloned(req->mPhysicalCameraSettings[i].settings.metadata(), &clone);
        camera_metadata_t *clonep = clone.release();
        if (id == deviceId) {
            pRequest->settings = new ACameraMetadata(clonep, ACameraMetadata::ACM_REQUEST);
        } else {
            pRequest->physicalSettings[req->mPhysicalCameraSettings[i].id] =
                    new ACameraMetadata(clonep, ACameraMetadata::ACM_REQUEST);
        }
    }
    pRequest->targets = new ACameraOutputTargets();
    for (size_t i = 0; i < req->mSurfaceList.size(); i++) {
        native_handle_t* anw = req->mSurfaceList[i];
        ACameraOutputTarget outputTarget(anw);
        pRequest->targets->mOutputs.insert(outputTarget);
    }
    return pRequest;
}

void
CameraDevice::freeACaptureRequest(ACaptureRequest* req) {
    if (req == nullptr) {
        return;
    }
    req->settings.clear();
    delete req->targets;
    delete req;
}

void
CameraDevice::notifySessionEndOfLifeLocked(ACameraCaptureSession* session) {
    if (isClosed()) {
        // Device is closing already. do nothing
        return;
    }

    if (mCurrentSession != session) {
        // Session has been replaced by other seesion or device is closed
        return;
    }
    mCurrentSession = nullptr;

    // Should not happen
    if (!session->mIsClosed) {
        ALOGE("Error: unclosed session %p reaches end of life!", session);
        setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
        return;
    }

    // No new session, unconfigure now
    camera_status_t ret = configureStreamsLocked(nullptr, nullptr);
    if (ret != ACAMERA_OK) {
        ALOGE("Unconfigure stream failed. Device might still be configured! ret %d", ret);
    }
}

void
CameraDevice::disconnectLocked(sp<ACameraCaptureSession>& session) {
    if (mClosing.exchange(true)) {
        // Already closing, just return
        ALOGW("Camera device %s is already closing.", getId());
        return;
    }

    if (mRemote != nullptr) {
        auto ret = mRemote->disconnect();
        if (!ret.isOk()) {
            ALOGE("%s: Transaction error while disconnecting device %s", __FUNCTION__,
                  ret.description().c_str());
        }
    }
    mRemote = nullptr;

    if (session != nullptr) {
        session->closeByDevice();
    }
}

camera_status_t
CameraDevice::stopRepeatingLocked() {
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Camera %s stop repeating failed! ret %d", getId(), ret);
        return ret;
    }
    if (mRepeatingSequenceId != REQUEST_ID_NONE) {
        int repeatingSequenceId = mRepeatingSequenceId;
        mRepeatingSequenceId = REQUEST_ID_NONE;

        int64_t lastFrameNumber;
        Status status = Status::UNKNOWN_ERROR;
        auto remoteRet = mRemote->cancelRepeatingRequest(
                [&status, &lastFrameNumber](Status s, auto frameNumber) {
                    status = s;
                    lastFrameNumber = frameNumber;
                });
        CHECK_TRANSACTION_AND_RET(remoteRet, status, "cancelRepeatingRequest()");
        checkRepeatingSequenceCompleteLocked(repeatingSequenceId, lastFrameNumber);
    }
    return ACAMERA_OK;
}

camera_status_t
CameraDevice::flushLocked(ACameraCaptureSession* session) {
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Camera %s abort captures failed! ret %d", getId(), ret);
        return ret;
    }

    // This should never happen because creating a new session will close
    // previous one and thus reject any API call from previous session.
    // But still good to check here in case something unexpected happen.
    if (mCurrentSession != session) {
        ALOGE("Camera %s session %p is not current active session!", getId(), session);
        return ACAMERA_ERROR_INVALID_OPERATION;
    }

    if (mFlushing) {
        ALOGW("Camera %s is already aborting captures", getId());
        return ACAMERA_OK;
    }

    mFlushing = true;

    // Send onActive callback to guarantee there is always active->ready transition
    sp<AMessage> msg = new AMessage(kWhatSessionStateCb, mHandler);
    msg->setPointer(kContextKey, session->mUserSessionCallback.context);
    msg->setObject(kSessionSpKey, session);
    msg->setPointer(kCallbackFpKey, (void*) session->mUserSessionCallback.onActive);
    postSessionMsgAndCleanup(msg);

    // If device is already idling, send callback and exit early
    if (mIdle) {
        sp<AMessage> msg = new AMessage(kWhatSessionStateCb, mHandler);
        msg->setPointer(kContextKey, session->mUserSessionCallback.context);
        msg->setObject(kSessionSpKey, session);
        msg->setPointer(kCallbackFpKey, (void*) session->mUserSessionCallback.onReady);
        postSessionMsgAndCleanup(msg);
        mFlushing = false;
        return ACAMERA_OK;
    }

    int64_t lastFrameNumber;
    Status status = Status::UNKNOWN_ERROR;
    auto remoteRet = mRemote->flush([&status, &lastFrameNumber](auto s, auto frameNumber) {
                                        status = s;
                                        lastFrameNumber = frameNumber;
                                    });
    CHECK_TRANSACTION_AND_RET(remoteRet, status, "flush()")
    if (mRepeatingSequenceId != REQUEST_ID_NONE) {
        checkRepeatingSequenceCompleteLocked(mRepeatingSequenceId, lastFrameNumber);
    }
    return ACAMERA_OK;
}

camera_status_t
CameraDevice::waitUntilIdleLocked() {
    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Wait until camera %s idle failed! ret %d", getId(), ret);
        return ret;
    }

    if (mRepeatingSequenceId != REQUEST_ID_NONE) {
        ALOGE("Camera device %s won't go to idle when there is repeating request!", getId());
        return ACAMERA_ERROR_INVALID_OPERATION;
    }

    auto remoteRet = mRemote->waitUntilIdle();
    CHECK_TRANSACTION_AND_RET(remoteRet, remoteRet, "waitUntilIdle()")
    return ACAMERA_OK;
}

camera_status_t
CameraDevice::configureStreamsLocked(const ACaptureSessionOutputContainer* outputs,
        const ACaptureRequest* sessionParameters) {
    ACaptureSessionOutputContainer emptyOutput;
    if (outputs == nullptr) {
        outputs = &emptyOutput;
    }

    camera_status_t ret = checkCameraClosedOrErrorLocked();
    if (ret != ACAMERA_OK) {
        return ret;
    }

    std::set<std::pair<native_handle_ptr_wrapper, OutputConfigurationWrapper>> outputSet;
    for (auto outConfig : outputs->mOutputs) {
        native_handle_t* anw = outConfig.mWindow;
        OutputConfigurationWrapper outConfigInsertW;
        OutputConfiguration &outConfigInsert = outConfigInsertW.mOutputConfiguration;
        outConfigInsert.rotation = utils::convertToHidl(outConfig.mRotation);
        outConfigInsert.windowGroupId = -1;
        outConfigInsert.windowHandles.resize(outConfig.mSharedWindows.size() + 1);
        outConfigInsert.windowHandles[0] = anw;
        outConfigInsert.physicalCameraId = outConfig.mPhysicalCameraId;
        native_handle_ptr_wrapper wrap(anw);
        outputSet.insert(std::make_pair(anw, outConfigInsertW));
    }
    std::set<std::pair<native_handle_ptr_wrapper, OutputConfigurationWrapper>> addSet = outputSet;
    std::vector<int32_t> deleteList;

    // Determine which streams need to be created, which to be deleted
    for (auto& kvPair : mConfiguredOutputs) {
        int32_t streamId = kvPair.first;
        auto& outputPair = kvPair.second;
        if (outputSet.count(outputPair)) {
            deleteList.push_back(streamId); // Need to delete a no longer needed stream
        } else {
            addSet.erase(outputPair);        // No need to add already existing stream
        }
    }

    ret = stopRepeatingLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Camera device %s stop repeating failed, ret %d", getId(), ret);
        return ret;
    }

    ret = waitUntilIdleLocked();
    if (ret != ACAMERA_OK) {
        ALOGE("Camera device %s wait until idle failed, ret %d", getId(), ret);
        return ret;
    }

    // Send onReady to previous session
    // CurrentSession will be updated after configureStreamLocked, so here
    // mCurrentSession is the session to be replaced by a new session
    if (!mIdle && mCurrentSession != nullptr) {
        if (mBusySession != mCurrentSession) {
            ALOGE("Current session != busy session");
            setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
            return ACAMERA_ERROR_CAMERA_DEVICE;
        }
        sp<AMessage> msg = new AMessage(kWhatSessionStateCb, mHandler);
        msg->setPointer(kContextKey, mBusySession->mUserSessionCallback.context);
        msg->setObject(kSessionSpKey, mBusySession);
        msg->setPointer(kCallbackFpKey, (void*) mBusySession->mUserSessionCallback.onReady);
        mBusySession.clear();
        postSessionMsgAndCleanup(msg);
    }
    mIdle = true;

    auto remoteRet = mRemote->beginConfigure();
    CHECK_TRANSACTION_AND_RET(remoteRet, remoteRet, "beginConfigure()")

    // delete to-be-deleted streams
    for (auto streamId : deleteList) {
        remoteRet = mRemote->deleteStream(streamId);
        CHECK_TRANSACTION_AND_RET(remoteRet, remoteRet, "deleteStream()")
        mConfiguredOutputs.erase(streamId);
    }

    // add new streams
    for (auto outputPair : addSet) {
        int streamId;
        Status status = Status::UNKNOWN_ERROR;
        auto ret = mRemote->createStream(outputPair.second,
                                         [&status, &streamId](Status s, auto stream_id) {
                                             status = s;
                                             streamId = stream_id;
                                         });
        CHECK_TRANSACTION_AND_RET(ret, status, "createStream()")
        mConfiguredOutputs.insert(std::make_pair(streamId, outputPair));
    }

    CameraMetadata params;
    HCameraMetadata hidlParams;
    if ((sessionParameters != nullptr) && (sessionParameters->settings != nullptr)) {
        params.append(sessionParameters->settings->getInternalData());
        const camera_metadata_t *params_metadata = params.getAndLock();
        utils::convertToHidl(params_metadata, &hidlParams);
        params.unlock(params_metadata);
    }
    remoteRet = mRemote->endConfigure(StreamConfigurationMode::NORMAL_MODE, hidlParams);
    CHECK_TRANSACTION_AND_RET(remoteRet, remoteRet, "endConfigure()")
    return ACAMERA_OK;
}

void
CameraDevice::setRemoteDevice(sp<ICameraDeviceUser> remote) {
    Mutex::Autolock _l(mDeviceLock);
    mRemote = remote;
}

bool
CameraDevice::setDeviceMetadataQueues() {
        if (mRemote == nullptr) {
          ALOGE("mRemote must not be null while trying to fetch metadata queues");
          return false;
        }
        std::shared_ptr<RequestMetadataQueue> &reqQueue = mCaptureRequestMetadataQueue;
        auto ret =
            mRemote->getCaptureRequestMetadataQueue(
                [&reqQueue](const auto &mqDescriptor) {
                    reqQueue = std::make_shared<RequestMetadataQueue>(mqDescriptor);
                    if (!reqQueue->isValid() || reqQueue->availableToWrite() <=0) {
                        ALOGE("Empty fmq from cameraserver");
                        reqQueue = nullptr;
                    }
                });
        if (!ret.isOk()) {
            ALOGE("Transaction error trying to get capture request metadata queue");
            return false;
        }
        std::shared_ptr<ResultMetadataQueue> &resQueue = mCaptureResultMetadataQueue;
        ret =
                mRemote->getCaptureResultMetadataQueue(
                        [&resQueue](const auto &mqDescriptor) {
                            resQueue = std::make_shared<ResultMetadataQueue>(mqDescriptor);
                            if (!resQueue->isValid() || resQueue->availableToWrite() <=0) {
                                ALOGE("Empty fmq from cameraserver");
                            }
                        });
        if (!ret.isOk()) {
            ALOGE("Transaction error trying to get capture result metadata queue");
            return false;
        }
        return true;
}

camera_status_t
CameraDevice::checkCameraClosedOrErrorLocked() const {
    if (mRemote == nullptr) {
        ALOGE("%s: camera device already closed", __FUNCTION__);
        return ACAMERA_ERROR_CAMERA_DISCONNECTED;
    }
    if (mInError) {// triggered by onDeviceError
        ALOGE("%s: camera device has encountered a serious error", __FUNCTION__);
        return mError;
    }
    return ACAMERA_OK;
}

void
CameraDevice::setCameraDeviceErrorLocked(camera_status_t error) {
    mInError = true;
    mError = error;
    return;
}

void
CameraDevice::FrameNumberTracker::updateTracker(int64_t frameNumber, bool isError) {
    ALOGV("updateTracker frame %" PRId64 " isError %d", frameNumber, isError);
    if (isError) {
        mFutureErrorSet.insert(frameNumber);
    } else if (frameNumber <= mCompletedFrameNumber) {
        ALOGE("Frame number %" PRId64 " decreased! current fn %" PRId64,
                frameNumber, mCompletedFrameNumber);
        return;
    } else {
        if (frameNumber != mCompletedFrameNumber + 1) {
            ALOGE("Frame number out of order. Expect %" PRId64 " but get %" PRId64,
                    mCompletedFrameNumber + 1, frameNumber);
            // Do not assert as in java implementation
        }
        mCompletedFrameNumber = frameNumber;
    }
    update();
}

void
CameraDevice::FrameNumberTracker::update() {
    for (auto it = mFutureErrorSet.begin(); it != mFutureErrorSet.end();) {
        int64_t errorFrameNumber = *it;
        if (errorFrameNumber == mCompletedFrameNumber + 1) {
            mCompletedFrameNumber++;
            it = mFutureErrorSet.erase(it);
        } else if (errorFrameNumber <= mCompletedFrameNumber) {
            // This should not happen, but deal with it anyway
            ALOGE("Completd frame number passed through current frame number!");
            // erase the old error since it's no longer useful
            it = mFutureErrorSet.erase(it);
        } else {
            // Normal requests hasn't catched up error frames, just break
            break;
        }
    }
    ALOGV("Update complete frame %" PRId64, mCompletedFrameNumber);
}

void
CameraDevice::onCaptureErrorLocked(
        ErrorCode errorCode,
        const CaptureResultExtras& resultExtras) {
    int sequenceId = resultExtras.requestId;
    int64_t frameNumber = resultExtras.frameNumber;
    int32_t burstId = resultExtras.burstId;
    auto it = mSequenceCallbackMap.find(sequenceId);
    if (it == mSequenceCallbackMap.end()) {
        ALOGE("%s: Error: capture sequence index %d not found!",
                __FUNCTION__, sequenceId);
        setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
        return;
    }

    CallbackHolder cbh = (*it).second;
    sp<ACameraCaptureSession> session = cbh.mSession;
    if ((size_t) burstId >= cbh.mRequests.size()) {
        ALOGE("%s: Error: request index %d out of bound (size %zu)",
                __FUNCTION__, burstId, cbh.mRequests.size());
        setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
        return;
    }
    sp<CaptureRequest> request = cbh.mRequests[burstId];

    // Handle buffer error
    if (errorCode == ErrorCode::CAMERA_BUFFER) {
        int32_t streamId = resultExtras.errorStreamId;
        ACameraCaptureSession_captureCallback_bufferLost onBufferLost =
                cbh.mOnCaptureBufferLost;
        auto outputPairIt = mConfiguredOutputs.find(streamId);
        if (outputPairIt == mConfiguredOutputs.end()) {
            ALOGE("%s: Error: stream id %d does not exist", __FUNCTION__, streamId);
            setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
            return;
        }

        const auto& windowHandles = outputPairIt->second.second.mOutputConfiguration.windowHandles;
        for (const auto& outHandle : windowHandles) {
            for (auto streamAndWindowId : request->mCaptureRequest.streamAndWindowIds) {
                int32_t windowId = streamAndWindowId.windowId;
                if (utils::isWindowNativeHandleEqual(windowHandles[windowId],outHandle)) {
                    native_handle_t* anw =
                        const_cast<native_handle_t *>(windowHandles[windowId].getNativeHandle());
                    ALOGV("Camera %s Lost output buffer for ANW %p frame %" PRId64,
                            getId(), anw, frameNumber);

                    sp<AMessage> msg = new AMessage(kWhatCaptureBufferLost, mHandler);
                    msg->setPointer(kContextKey, cbh.mContext);
                    msg->setObject(kSessionSpKey, session);
                    msg->setPointer(kCallbackFpKey, (void*) onBufferLost);
                    msg->setObject(kCaptureRequestKey, request);
                    msg->setPointer(kAnwKey, (void*) anw);
                    msg->setInt64(kFrameNumberKey, frameNumber);
                    postSessionMsgAndCleanup(msg);
                }
            }
        }
    } else { // Handle other capture failures
        // Fire capture failure callback if there is one registered
        ACameraCaptureSession_captureCallback_failed onError = cbh.mOnCaptureFailed;
        sp<CameraCaptureFailure> failure(new CameraCaptureFailure());
        failure->frameNumber = frameNumber;
        // TODO: refine this when implementing flush
        failure->reason      = CAPTURE_FAILURE_REASON_ERROR;
        failure->sequenceId  = sequenceId;
        failure->wasImageCaptured = (errorCode == ErrorCode::CAMERA_RESULT);

        sp<AMessage> msg = new AMessage(cbh.mIsLogicalCameraCallback ? kWhatLogicalCaptureFail :
                kWhatCaptureFail, mHandler);
        msg->setPointer(kContextKey, cbh.mContext);
        msg->setObject(kSessionSpKey, session);
        if (cbh.mIsLogicalCameraCallback) {
            if (resultExtras.errorPhysicalCameraId.size() > 0) {
                msg->setString(kFailingPhysicalCameraId, resultExtras.errorPhysicalCameraId.c_str(),
                        resultExtras.errorPhysicalCameraId.size());
            }
            msg->setPointer(kCallbackFpKey, (void*) cbh.mOnLogicalCameraCaptureFailed);
        } else {
            msg->setPointer(kCallbackFpKey, (void*) onError);
        }
        msg->setObject(kCaptureRequestKey, request);
        msg->setObject(kCaptureFailureKey, failure);
        postSessionMsgAndCleanup(msg);

        // Update tracker
        mFrameNumberTracker.updateTracker(frameNumber, /*isError*/true);
        checkAndFireSequenceCompleteLocked();
    }
    return;
}

CameraDevice::CallbackHandler::CallbackHandler(const char *id) : mId(id) { }

void CameraDevice::CallbackHandler::onMessageReceived(
        const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatOnDisconnected:
        case kWhatOnError:
        case kWhatSessionStateCb:
        case kWhatCaptureStart:
        case kWhatCaptureResult:
        case kWhatLogicalCaptureResult:
        case kWhatCaptureFail:
        case kWhatLogicalCaptureFail:
        case kWhatCaptureSeqEnd:
        case kWhatCaptureSeqAbort:
        case kWhatCaptureBufferLost:
            ALOGV("%s: Received msg %d", __FUNCTION__, msg->what());
            break;
        case kWhatCleanUpSessions:
            mCachedSessions.clear();
            return;
        default:
            ALOGE("%s:Error: unknown device callback %d", __FUNCTION__, msg->what());
            return;
    }
    // Check the common part of all message
    void* context;
    bool found = msg->findPointer(kContextKey, &context);
    if (!found) {
        ALOGE("%s: Cannot find callback context!", __FUNCTION__);
        return;
    }
    switch (msg->what()) {
        case kWhatOnDisconnected:
        {
            ACameraDevice* dev;
            found = msg->findPointer(kDeviceKey, (void**) &dev);
            if (!found || dev == nullptr) {
                ALOGE("%s: Cannot find device pointer!", __FUNCTION__);
                return;
            }
            ACameraDevice_StateCallback onDisconnected;
            found = msg->findPointer(kCallbackFpKey, (void**) &onDisconnected);
            if (!found) {
                ALOGE("%s: Cannot find onDisconnected!", __FUNCTION__);
                return;
            }
            if (onDisconnected == nullptr) {
                return;
            }
            (*onDisconnected)(context, dev);
            break;
        }
        case kWhatOnError:
        {
            ACameraDevice* dev;
            found = msg->findPointer(kDeviceKey, (void**) &dev);
            if (!found || dev == nullptr) {
                ALOGE("%s: Cannot find device pointer!", __FUNCTION__);
                return;
            }
            ACameraDevice_ErrorStateCallback onError;
            found = msg->findPointer(kCallbackFpKey, (void**) &onError);
            if (!found) {
                ALOGE("%s: Cannot find onError!", __FUNCTION__);
                return;
            }
            int errorCode;
            found = msg->findInt32(kErrorCodeKey, &errorCode);
            if (!found) {
                ALOGE("%s: Cannot find error code!", __FUNCTION__);
                return;
            }
            if (onError == nullptr) {
                return;
            }
            (*onError)(context, dev, errorCode);
            break;
        }
        case kWhatSessionStateCb:
        case kWhatCaptureStart:
        case kWhatCaptureResult:
        case kWhatLogicalCaptureResult:
        case kWhatCaptureFail:
        case kWhatLogicalCaptureFail:
        case kWhatCaptureSeqEnd:
        case kWhatCaptureSeqAbort:
        case kWhatCaptureBufferLost:
        {
            sp<RefBase> obj;
            found = msg->findObject(kSessionSpKey, &obj);
            if (!found || obj == nullptr) {
                ALOGE("%s: Cannot find session pointer!", __FUNCTION__);
                return;
            }
            sp<ACameraCaptureSession> session(static_cast<ACameraCaptureSession*>(obj.get()));
            mCachedSessions.push(session);
            sp<CaptureRequest> requestSp = nullptr;
            const char *id_cstr = mId.c_str();
            switch (msg->what()) {
                case kWhatCaptureStart:
                case kWhatCaptureResult:
                case kWhatLogicalCaptureResult:
                case kWhatCaptureFail:
                case kWhatLogicalCaptureFail:
                case kWhatCaptureBufferLost:
                    found = msg->findObject(kCaptureRequestKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find capture request!", __FUNCTION__);
                        return;
                    }
                    requestSp = static_cast<CaptureRequest*>(obj.get());
                    break;
            }

            switch (msg->what()) {
                case kWhatSessionStateCb:
                {
                    ACameraCaptureSession_stateCallback onState;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onState);
                    if (!found) {
                        ALOGE("%s: Cannot find state callback!", __FUNCTION__);
                        return;
                    }
                    if (onState == nullptr) {
                        return;
                    }
                    (*onState)(context, session.get());
                    break;
                }
                case kWhatCaptureStart:
                {
                    ACameraCaptureSession_captureCallback_start onStart;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onStart);
                    if (!found) {
                        ALOGE("%s: Cannot find capture start callback!", __FUNCTION__);
                        return;
                    }
                    if (onStart == nullptr) {
                        return;
                    }
                    int64_t timestamp;
                    found = msg->findInt64(kTimeStampKey, &timestamp);
                    if (!found) {
                        ALOGE("%s: Cannot find timestamp!", __FUNCTION__);
                        return;
                    }
                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onStart)(context, session.get(), request, timestamp);
                    freeACaptureRequest(request);
                    break;
                }
                case kWhatCaptureResult:
                {
                    ACameraCaptureSession_captureCallback_result onResult;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onResult);
                    if (!found) {
                        ALOGE("%s: Cannot find capture result callback!", __FUNCTION__);
                        return;
                    }
                    if (onResult == nullptr) {
                        return;
                    }

                    found = msg->findObject(kCaptureResultKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find capture result!", __FUNCTION__);
                        return;
                    }
                    sp<ACameraMetadata> result(static_cast<ACameraMetadata*>(obj.get()));
                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onResult)(context, session.get(), request, result.get());
                    freeACaptureRequest(request);
                    break;
                }
                case kWhatLogicalCaptureResult:
                {
                    ACameraCaptureSession_logicalCamera_captureCallback_result onResult;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onResult);
                    if (!found) {
                        ALOGE("%s: Cannot find capture result callback!", __FUNCTION__);
                        return;
                    }
                    if (onResult == nullptr) {
                        return;
                    }

                    found = msg->findObject(kCaptureResultKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find capture result!", __FUNCTION__);
                        return;
                    }
                    sp<ACameraMetadata> result(static_cast<ACameraMetadata*>(obj.get()));

                    found = msg->findObject(kPhysicalCaptureResultKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find physical capture result!", __FUNCTION__);
                        return;
                    }
                    sp<ACameraPhysicalCaptureResultInfo> physicalResult(
                            static_cast<ACameraPhysicalCaptureResultInfo*>(obj.get()));
                    std::vector<PhysicalCaptureResultInfoLocal>& physicalResultInfo =
                            physicalResult->mPhysicalResultInfo;

                    std::vector<std::string> physicalCameraIds;
                    std::vector<sp<ACameraMetadata>> physicalMetadataCopy;
                    for (size_t i = 0; i < physicalResultInfo.size(); i++) {
                        physicalCameraIds.push_back(physicalResultInfo[i].physicalCameraId);

                        CameraMetadata clone = physicalResultInfo[i].physicalMetadata;
                        clone.update(ANDROID_SYNC_FRAME_NUMBER,
                                &physicalResult->mFrameNumber, /*data_count*/1);
                        sp<ACameraMetadata> metadata =
                                new ACameraMetadata(clone.release(), ACameraMetadata::ACM_RESULT);
                        physicalMetadataCopy.push_back(metadata);
                    }
                    std::vector<const char*> physicalCameraIdPtrs;
                    std::vector<const ACameraMetadata*> physicalMetadataCopyPtrs;
                    for (size_t i = 0; i < physicalResultInfo.size(); i++) {
                        physicalCameraIdPtrs.push_back(physicalCameraIds[i].c_str());
                        physicalMetadataCopyPtrs.push_back(physicalMetadataCopy[i].get());
                    }

                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onResult)(context, session.get(), request, result.get(),
                            physicalResultInfo.size(), physicalCameraIdPtrs.data(),
                            physicalMetadataCopyPtrs.data());
                    freeACaptureRequest(request);
                    break;
                }

                case kWhatCaptureFail:
                {
                    ACameraCaptureSession_captureCallback_failed onFail;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onFail);
                    if (!found) {
                        ALOGE("%s: Cannot find capture fail callback!", __FUNCTION__);
                        return;
                    }
                    if (onFail == nullptr) {
                        return;
                    }

                    found = msg->findObject(kCaptureFailureKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find capture failure!", __FUNCTION__);
                        return;
                    }
                    sp<CameraCaptureFailure> failureSp(
                            static_cast<CameraCaptureFailure*>(obj.get()));
                    ACameraCaptureFailure* failure =
                            static_cast<ACameraCaptureFailure*>(failureSp.get());
                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onFail)(context, session.get(), request, failure);
                    freeACaptureRequest(request);
                    break;
                }
                case kWhatLogicalCaptureFail:
                {
                    ACameraCaptureSession_logicalCamera_captureCallback_failed onFail;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onFail);
                    if (!found) {
                        ALOGE("%s: Cannot find capture fail callback!", __FUNCTION__);
                        return;
                    }
                    if (onFail == nullptr) {
                        return;
                    }

                    found = msg->findObject(kCaptureFailureKey, &obj);
                    if (!found) {
                        ALOGE("%s: Cannot find capture failure!", __FUNCTION__);
                        return;
                    }
                    sp<CameraCaptureFailure> failureSp(
                            static_cast<CameraCaptureFailure*>(obj.get()));
                    ALogicalCameraCaptureFailure failure;
                    AString physicalCameraId;
                    found = msg->findString(kFailingPhysicalCameraId, &physicalCameraId);
                    if (found && !physicalCameraId.empty()) {
                        failure.physicalCameraId = physicalCameraId.c_str();
                    } else {
                        failure.physicalCameraId = nullptr;
                    }
                    failure.captureFailure = *failureSp;
                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onFail)(context, session.get(), request, &failure);
                    freeACaptureRequest(request);
                    break;
                }
                case kWhatCaptureSeqEnd:
                {
                    ACameraCaptureSession_captureCallback_sequenceEnd onSeqEnd;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onSeqEnd);
                    if (!found) {
                        ALOGE("%s: Cannot find sequence end callback!", __FUNCTION__);
                        return;
                    }
                    if (onSeqEnd == nullptr) {
                        return;
                    }
                    int seqId;
                    found = msg->findInt32(kSequenceIdKey, &seqId);
                    if (!found) {
                        ALOGE("%s: Cannot find frame number!", __FUNCTION__);
                        return;
                    }
                    int64_t frameNumber;
                    found = msg->findInt64(kFrameNumberKey, &frameNumber);
                    if (!found) {
                        ALOGE("%s: Cannot find frame number!", __FUNCTION__);
                        return;
                    }
                    (*onSeqEnd)(context, session.get(), seqId, frameNumber);
                    break;
                }
                case kWhatCaptureSeqAbort:
                {
                    ACameraCaptureSession_captureCallback_sequenceAbort onSeqAbort;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onSeqAbort);
                    if (!found) {
                        ALOGE("%s: Cannot find sequence end callback!", __FUNCTION__);
                        return;
                    }
                    if (onSeqAbort == nullptr) {
                        return;
                    }
                    int seqId;
                    found = msg->findInt32(kSequenceIdKey, &seqId);
                    if (!found) {
                        ALOGE("%s: Cannot find frame number!", __FUNCTION__);
                        return;
                    }
                    (*onSeqAbort)(context, session.get(), seqId);
                    break;
                }
                case kWhatCaptureBufferLost:
                {
                    ACameraCaptureSession_captureCallback_bufferLost onBufferLost;
                    found = msg->findPointer(kCallbackFpKey, (void**) &onBufferLost);
                    if (!found) {
                        ALOGE("%s: Cannot find buffer lost callback!", __FUNCTION__);
                        return;
                    }
                    if (onBufferLost == nullptr) {
                        return;
                    }

                    native_handle_t* anw;
                    found = msg->findPointer(kAnwKey, (void**) &anw);
                    if (!found) {
                        ALOGE("%s: Cannot find native_handle_t!", __FUNCTION__);
                        return;
                    }

                    int64_t frameNumber;
                    found = msg->findInt64(kFrameNumberKey, &frameNumber);
                    if (!found) {
                        ALOGE("%s: Cannot find frame number!", __FUNCTION__);
                        return;
                    }

                    ACaptureRequest* request = allocateACaptureRequest(requestSp, id_cstr);
                    (*onBufferLost)(context, session.get(), request, anw, frameNumber);
                    freeACaptureRequest(request);
                    break;
                }
            }
            break;
        }
    }
}

CameraDevice::CallbackHolder::CallbackHolder(
        sp<ACameraCaptureSession>          session,
        const Vector<sp<CaptureRequest> >& requests,
        bool                               isRepeating,
        ACameraCaptureSession_captureCallbacks* cbs) :
        mSession(session), mRequests(requests),
        mIsRepeating(isRepeating),
        mIsLogicalCameraCallback(false) {
    initCaptureCallbacks(cbs);

    if (cbs != nullptr) {
        mOnCaptureCompleted = cbs->onCaptureCompleted;
        mOnCaptureFailed = cbs->onCaptureFailed;
    }
}

CameraDevice::CallbackHolder::CallbackHolder(
        sp<ACameraCaptureSession>          session,
        const Vector<sp<CaptureRequest> >& requests,
        bool                               isRepeating,
        ACameraCaptureSession_logicalCamera_captureCallbacks* lcbs) :
        mSession(session), mRequests(requests),
        mIsRepeating(isRepeating),
        mIsLogicalCameraCallback(true) {
    initCaptureCallbacks(lcbs);

    if (lcbs != nullptr) {
        mOnLogicalCameraCaptureCompleted = lcbs->onLogicalCameraCaptureCompleted;
        mOnLogicalCameraCaptureFailed = lcbs->onLogicalCameraCaptureFailed;
    }
}

void
CameraDevice::checkRepeatingSequenceCompleteLocked(
    const int sequenceId, const int64_t lastFrameNumber) {
    ALOGV("Repeating seqId %d lastFrameNumer %" PRId64, sequenceId, lastFrameNumber);
    if (lastFrameNumber == NO_FRAMES_CAPTURED) {
        if (mSequenceCallbackMap.count(sequenceId) == 0) {
            ALOGW("No callback found for sequenceId %d", sequenceId);
            return;
        }
        // remove callback holder from callback map
        auto cbIt = mSequenceCallbackMap.find(sequenceId);
        CallbackHolder cbh = cbIt->second;
        mSequenceCallbackMap.erase(cbIt);
        // send seq aborted callback
        sp<AMessage> msg = new AMessage(kWhatCaptureSeqAbort, mHandler);
        msg->setPointer(kContextKey, cbh.mContext);
        msg->setObject(kSessionSpKey, cbh.mSession);
        msg->setPointer(kCallbackFpKey, (void*) cbh.mOnCaptureSequenceAborted);
        msg->setInt32(kSequenceIdKey, sequenceId);
        postSessionMsgAndCleanup(msg);
    } else {
        // Use mSequenceLastFrameNumberMap to track
        mSequenceLastFrameNumberMap.insert(std::make_pair(sequenceId, lastFrameNumber));

        // Last frame might have arrived. Check now
        checkAndFireSequenceCompleteLocked();
    }
}

void
CameraDevice::checkAndFireSequenceCompleteLocked() {
    int64_t completedFrameNumber = mFrameNumberTracker.getCompletedFrameNumber();
    auto it = mSequenceLastFrameNumberMap.begin();
    while (it != mSequenceLastFrameNumberMap.end()) {
        int sequenceId = it->first;
        int64_t lastFrameNumber = it->second;
        bool seqCompleted = false;
        bool hasCallback  = true;

        if (mRemote == nullptr) {
            ALOGW("Camera %s closed while checking sequence complete", getId());
            return;
        }

        // Check if there is callback for this sequence
        // This should not happen because we always register callback (with nullptr inside)
        if (mSequenceCallbackMap.count(sequenceId) == 0) {
            ALOGW("No callback found for sequenceId %d", sequenceId);
            hasCallback = false;
        }

        if (lastFrameNumber <= completedFrameNumber) {
            ALOGV("seq %d reached last frame %" PRId64 ", completed %" PRId64,
                  sequenceId, lastFrameNumber, completedFrameNumber);
            seqCompleted = true;
        }

        if (seqCompleted && hasCallback) {
            // remove callback holder from callback map
            auto cbIt = mSequenceCallbackMap.find(sequenceId);
            CallbackHolder cbh = cbIt->second;
            mSequenceCallbackMap.erase(cbIt);
            // send seq complete callback
            sp<AMessage> msg = new AMessage(kWhatCaptureSeqEnd, mHandler);
            msg->setPointer(kContextKey, cbh.mContext);
            msg->setObject(kSessionSpKey, cbh.mSession);
            msg->setPointer(kCallbackFpKey, (void*) cbh.mOnCaptureSequenceCompleted);
            msg->setInt32(kSequenceIdKey, sequenceId);
            msg->setInt64(kFrameNumberKey, lastFrameNumber);

            // Clear the session sp before we send out the message
            // This will guarantee the rare case where the message is processed
            // before cbh goes out of scope and causing we call the session
            // destructor while holding device lock
            cbh.mSession.clear();

            postSessionMsgAndCleanup(msg);
        }

        // No need to track sequence complete if there is no callback registered
        if (seqCompleted || !hasCallback) {
            it = mSequenceLastFrameNumberMap.erase(it);
        } else {
            ++it;
        }
    }
}

void CameraDevice::stopLooperAndDisconnect() {
    Mutex::Autolock _l(mDeviceLock);
    sp<ACameraCaptureSession> session = mCurrentSession.promote();
    if (!isClosed()) {
        disconnectLocked(session);
    }
    mCurrentSession = nullptr;
    if (mCbLooper != nullptr) {
      mCbLooper->unregisterHandler(mHandler->id());
      mCbLooper->stop();
    }
    mCbLooper.clear();
    mHandler.clear();
}

/**
  * Camera service callback implementation
  */
android::hardware::Return<void>
CameraDevice::ServiceCallback::onDeviceError(
        ErrorCode errorCode,
        const CaptureResultExtras& resultExtras) {
    ALOGD("Device error received, code %d, frame number %" PRId64 ", request ID %d, subseq ID %d"
            " physical camera ID %s", errorCode, resultExtras.frameNumber, resultExtras.requestId,
            resultExtras.burstId, resultExtras.errorPhysicalCameraId.c_str());
    auto ret = Void();
    sp<CameraDevice> dev = mDevice.promote();
    if (dev == nullptr) {
        return ret; // device has been closed
    }

    sp<ACameraCaptureSession> session = dev->mCurrentSession.promote();
    Mutex::Autolock _l(dev->mDeviceLock);
    if (dev->mRemote == nullptr) {
        return ret; // device has been closed
    }
    switch (errorCode) {
        case ErrorCode::CAMERA_DISCONNECTED:
        {
            // Camera is disconnected, close the session and expect no more callbacks
            if (session != nullptr) {
                session->closeByDevice();
            }
            dev->mCurrentSession = nullptr;
            sp<AMessage> msg = new AMessage(kWhatOnDisconnected, dev->mHandler);
            msg->setPointer(kContextKey, dev->mAppCallbacks.context);
            msg->setPointer(kDeviceKey, (void*) dev->getWrapper());
            msg->setPointer(kCallbackFpKey, (void*) dev->mAppCallbacks.onDisconnected);
            msg->post();
            break;
        }
        default:
            ALOGE("Unknown error from camera device: %d", errorCode);
            [[fallthrough]];
        case ErrorCode::CAMERA_DEVICE:
        case ErrorCode::CAMERA_SERVICE:
        {
            int32_t errorVal = ::ERROR_CAMERA_DEVICE;
            // We keep this switch since this block might be encountered with
            // more than just 2 states. The default fallthrough could have us
            // handling more unmatched error cases.
            switch (errorCode) {
                case ErrorCode::CAMERA_DEVICE:
                    dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
                    break;
                case ErrorCode::CAMERA_SERVICE:
                    dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
                    errorVal = ::ERROR_CAMERA_SERVICE;
                    break;
                default:
                    dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_UNKNOWN);
                    break;
            }
            sp<AMessage> msg = new AMessage(kWhatOnError, dev->mHandler);
            msg->setPointer(kContextKey, dev->mAppCallbacks.context);
            msg->setPointer(kDeviceKey, (void*) dev->getWrapper());
            msg->setPointer(kCallbackFpKey, (void*) dev->mAppCallbacks.onError);
            msg->setInt32(kErrorCodeKey, errorVal);
            msg->post();
            break;
        }
        case ErrorCode::CAMERA_REQUEST:
        case ErrorCode::CAMERA_RESULT:
        case ErrorCode::CAMERA_BUFFER:
            dev->onCaptureErrorLocked(errorCode, resultExtras);
            break;
    }
    return ret;
}

android::hardware::Return<void>
CameraDevice::ServiceCallback::onDeviceIdle() {
    ALOGV("Camera is now idle");
    auto ret = Void();
    sp<CameraDevice> dev = mDevice.promote();
    if (dev == nullptr) {
        return ret; // device has been closed
    }

    Mutex::Autolock _l(dev->mDeviceLock);
    if (dev->isClosed() || dev->mRemote == nullptr) {
        return ret;
    }

    if (dev->mIdle) {
        // Already in idle state. Possibly other thread did waitUntilIdle
        return ret;
    }

    if (dev->mCurrentSession != nullptr) {
        ALOGE("onDeviceIdle sending state cb");
        if (dev->mBusySession != dev->mCurrentSession) {
            ALOGE("Current session != busy session");
            dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_DEVICE);
            return ret;
        }

        sp<AMessage> msg = new AMessage(kWhatSessionStateCb, dev->mHandler);
        msg->setPointer(kContextKey, dev->mBusySession->mUserSessionCallback.context);
        msg->setObject(kSessionSpKey, dev->mBusySession);
        msg->setPointer(kCallbackFpKey, (void*) dev->mBusySession->mUserSessionCallback.onReady);
        // Make sure we clear the sp first so the session destructor can
        // only happen on handler thread (where we don't hold device/session lock)
        dev->mBusySession.clear();
        dev->postSessionMsgAndCleanup(msg);
    }
    dev->mIdle = true;
    dev->mFlushing = false;
    return ret;
}

android::hardware::Return<void>
CameraDevice::ServiceCallback::onCaptureStarted(
        const CaptureResultExtras& resultExtras,
        uint64_t timestamp) {
    auto ret = Void();

    sp<CameraDevice> dev = mDevice.promote();
    if (dev == nullptr) {
        return ret; // device has been closed
    }
    Mutex::Autolock _l(dev->mDeviceLock);
    if (dev->isClosed() || dev->mRemote == nullptr) {
        return ret;
    }

    int32_t sequenceId = resultExtras.requestId;
    int32_t burstId = resultExtras.burstId;

    auto it = dev->mSequenceCallbackMap.find(sequenceId);
    if (it != dev->mSequenceCallbackMap.end()) {
        CallbackHolder cbh = (*it).second;
        ACameraCaptureSession_captureCallback_start onStart = cbh.mOnCaptureStarted;
        sp<ACameraCaptureSession> session = cbh.mSession;
        if ((size_t) burstId >= cbh.mRequests.size()) {
            ALOGE("%s: Error: request index %d out of bound (size %zu)",
                    __FUNCTION__, burstId, cbh.mRequests.size());
            dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
        }
        sp<CaptureRequest> request = cbh.mRequests[burstId];
        sp<AMessage> msg = new AMessage(kWhatCaptureStart, dev->mHandler);
        msg->setPointer(kContextKey, cbh.mContext);
        msg->setObject(kSessionSpKey, session);
        msg->setPointer(kCallbackFpKey, (void*) onStart);
        msg->setObject(kCaptureRequestKey, request);
        msg->setInt64(kTimeStampKey, timestamp);
        dev->postSessionMsgAndCleanup(msg);
    }
    return ret;
}

android::hardware::Return<void>
CameraDevice::ServiceCallback::onResultReceived(
        const FmqSizeOrMetadata& resultMetadata,
        const CaptureResultExtras& resultExtras,
        const hidl_vec<PhysicalCaptureResultInfo>& physicalResultInfos) {
    auto ret = Void();

    sp<CameraDevice> dev = mDevice.promote();
    if (dev == nullptr) {
        return ret; // device has been closed
    }
    int32_t sequenceId = resultExtras.requestId;
    int64_t frameNumber = resultExtras.frameNumber;
    int32_t burstId = resultExtras.burstId;
    bool    isPartialResult = (resultExtras.partialResultCount < dev->mPartialResultCount);

    if (!isPartialResult) {
        ALOGV("SeqId %d frame %" PRId64 " result arrive.", sequenceId, frameNumber);
    }

    Mutex::Autolock _l(dev->mDeviceLock);
    if (dev->mRemote == nullptr) {
        return ret; // device has been disconnected
    }

    if (dev->isClosed()) {
        if (!isPartialResult) {
            dev->mFrameNumberTracker.updateTracker(frameNumber, /*isError*/false);
        }
        // early return to avoid callback sent to closed devices
        return ret;
    }

    CameraMetadata metadataCopy;
    camera_status_t status = readOneResultMetadata(resultMetadata,
            dev->mCaptureResultMetadataQueue.get(), &metadataCopy);
    if (status != ACAMERA_OK) {
        ALOGE("%s: result metadata couldn't be converted", __FUNCTION__);
        return ret;
    }

    metadataCopy.update(ANDROID_LENS_INFO_SHADING_MAP_SIZE, dev->mShadingMapSize, /*data_count*/2);
    metadataCopy.update(ANDROID_SYNC_FRAME_NUMBER, &frameNumber, /*data_count*/1);

    auto it = dev->mSequenceCallbackMap.find(sequenceId);
    if (it != dev->mSequenceCallbackMap.end()) {
        CallbackHolder cbh = (*it).second;
        sp<ACameraCaptureSession> session = cbh.mSession;
        if ((size_t) burstId >= cbh.mRequests.size()) {
            ALOGE("%s: Error: request index %d out of bound (size %zu)",
                    __FUNCTION__, burstId, cbh.mRequests.size());
            dev->setCameraDeviceErrorLocked(ACAMERA_ERROR_CAMERA_SERVICE);
        }
        sp<CaptureRequest> request = cbh.mRequests[burstId];
        sp<ACameraMetadata> result(new ACameraMetadata(
                metadataCopy.release(), ACameraMetadata::ACM_RESULT));

        std::vector<PhysicalCaptureResultInfoLocal> localPhysicalResult;
        localPhysicalResult.resize(physicalResultInfos.size());
        for (size_t i = 0; i < physicalResultInfos.size(); i++) {
            localPhysicalResult[i].physicalCameraId = physicalResultInfos[i].physicalCameraId;
            status = readOneResultMetadata(physicalResultInfos[i].physicalCameraMetadata,
                    dev->mCaptureResultMetadataQueue.get(),
                    &localPhysicalResult[i].physicalMetadata);
            if (status != ACAMERA_OK) {
                ALOGE("%s: physical camera result metadata couldn't be converted", __FUNCTION__);
                return ret;
            }
        }
        sp<ACameraPhysicalCaptureResultInfo> physicalResult(
                new ACameraPhysicalCaptureResultInfo(localPhysicalResult, frameNumber));

        sp<AMessage> msg = new AMessage(
                cbh.mIsLogicalCameraCallback ? kWhatLogicalCaptureResult : kWhatCaptureResult,
                dev->mHandler);
        msg->setPointer(kContextKey, cbh.mContext);
        msg->setObject(kSessionSpKey, session);
        msg->setObject(kCaptureRequestKey, request);
        msg->setObject(kCaptureResultKey, result);
        if (isPartialResult) {
            msg->setPointer(kCallbackFpKey,
                    (void *)cbh.mOnCaptureProgressed);
        } else if (cbh.mIsLogicalCameraCallback) {
            msg->setPointer(kCallbackFpKey,
                    (void *)cbh.mOnLogicalCameraCaptureCompleted);
            msg->setObject(kPhysicalCaptureResultKey, physicalResult);
        } else {
            msg->setPointer(kCallbackFpKey,
                    (void *)cbh.mOnCaptureCompleted);
        }
        dev->postSessionMsgAndCleanup(msg);
    }

    if (!isPartialResult) {
        dev->mFrameNumberTracker.updateTracker(frameNumber, /*isError*/false);
        dev->checkAndFireSequenceCompleteLocked();
    }

    return ret;
}

android::hardware::Return<void>
CameraDevice::ServiceCallback::onRepeatingRequestError(
        uint64_t lastFrameNumber, int32_t stoppedSequenceId) {
    auto ret = Void();

    sp<CameraDevice> dev = mDevice.promote();
    if (dev == nullptr) {
        return ret; // device has been closed
    }

    Mutex::Autolock _l(dev->mDeviceLock);

    int repeatingSequenceId = dev->mRepeatingSequenceId;
    if (stoppedSequenceId == repeatingSequenceId) {
        dev->mRepeatingSequenceId = REQUEST_ID_NONE;
    }

    dev->checkRepeatingSequenceCompleteLocked(repeatingSequenceId, lastFrameNumber);

    return ret;
}

camera_status_t CameraDevice::ServiceCallback::readOneResultMetadata(
        const FmqSizeOrMetadata& fmqSizeOrMetadata, ResultMetadataQueue* metadataQueue,
        CameraMetadata* metadata) {
    if (metadataQueue == nullptr || metadata == nullptr) {
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    bool converted;
    HCameraMetadata hCameraMetadata;
    if (fmqSizeOrMetadata.getDiscriminator() ==
            FmqSizeOrMetadata::hidl_discriminator::fmqMetadataSize) {
        hCameraMetadata.resize(fmqSizeOrMetadata.fmqMetadataSize());
        bool read = metadataQueue->read(
                hCameraMetadata.data(), fmqSizeOrMetadata.fmqMetadataSize());
        if (!read) {
            ALOGE("%s capture request settings could't be read from fmq", __FUNCTION__);
            return ACAMERA_ERROR_UNKNOWN;
        }
        // TODO: Do we actually need to clone here ?
        converted = utils::convertFromHidlCloned(hCameraMetadata, metadata);
    } else {
        converted = utils::convertFromHidlCloned(fmqSizeOrMetadata.metadata(), metadata);
    }

    return converted ? ACAMERA_OK : ACAMERA_ERROR_UNKNOWN;
}

} // namespace acam
} // namespace android
