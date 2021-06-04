/*
 * Copyright (C) 2015 The Android Open Source Project
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
#ifndef _ACAMERA_DEVICE_H
#define _ACAMERA_DEVICE_H

#include <memory>
#include <map>
#include <set>
#include <atomic>
#include <utility>
#include <vector>
#include <utils/StrongPointer.h>
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <utils/List.h>
#include <utils/Vector.h>

#include <android/hardware/camera2/BnCameraDeviceCallbacks.h>
#include <android/hardware/camera2/ICameraDeviceUser.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>
#include <camera/CaptureResult.h>
#include <camera/camera2/OutputConfiguration.h>
#include <camera/camera2/SessionConfiguration.h>
#include <camera/camera2/CaptureRequest.h>

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraCaptureSession.h>

#include "ACameraMetadata.h"

namespace android {
namespace acam {

// Wrap ACameraCaptureFailure so it can be ref-counted
struct CameraCaptureFailure : public RefBase, public ACameraCaptureFailure {};

// Wrap PhysicalCaptureResultInfo so that it can be ref-counted
struct ACameraPhysicalCaptureResultInfo: public RefBase {
    ACameraPhysicalCaptureResultInfo(const std::vector<PhysicalCaptureResultInfo>& info,
            int64_t frameNumber) :
        mPhysicalResultInfo(info), mFrameNumber(frameNumber) {}

    std::vector<PhysicalCaptureResultInfo> mPhysicalResultInfo;
    int64_t mFrameNumber;
};

class CameraDevice final : public RefBase {
  public:
    CameraDevice(const char* id, ACameraDevice_StateCallbacks* cb,
                  sp<ACameraMetadata> chars,
                  ACameraDevice* wrapper);
    ~CameraDevice();

    inline const char* getId() const { return mCameraId.string(); }

    camera_status_t createCaptureRequest(
            ACameraDevice_request_template templateId,
            const ACameraIdList* physicalIdList,
            ACaptureRequest** request) const;

    camera_status_t createCaptureSession(
            const ACaptureSessionOutputContainer*       outputs,
            const ACaptureRequest* sessionParameters,
            const ACameraCaptureSession_stateCallbacks* callbacks,
            /*out*/ACameraCaptureSession** session);

    camera_status_t isSessionConfigurationSupported(
            const ACaptureSessionOutputContainer* sessionOutputContainer) const;

    // Callbacks from camera service
    class ServiceCallback : public hardware::camera2::BnCameraDeviceCallbacks {
      public:
        explicit ServiceCallback(CameraDevice* device) : mDevice(device) {}
        binder::Status onDeviceError(int32_t errorCode,
                           const CaptureResultExtras& resultExtras) override;
        binder::Status onDeviceIdle() override;
        binder::Status onCaptureStarted(const CaptureResultExtras& resultExtras,
                              int64_t timestamp) override;
        binder::Status onResultReceived(const CameraMetadata& metadata,
                              const CaptureResultExtras& resultExtras,
                              const std::vector<PhysicalCaptureResultInfo>& physicalResultInfos) override;
        binder::Status onPrepared(int streamId) override;
        binder::Status onRequestQueueEmpty() override;
        binder::Status onRepeatingRequestError(int64_t lastFrameNumber,
                int32_t stoppedSequenceId) override;
      private:
        const wp<CameraDevice> mDevice;
    };
    inline sp<hardware::camera2::ICameraDeviceCallbacks> getServiceCallback() {
        return mServiceCallback;
    };

    // Camera device is only functional after remote being set
    void setRemoteDevice(sp<hardware::camera2::ICameraDeviceUser> remote);

    inline ACameraDevice* getWrapper() const { return mWrapper; };

    // Stop the looper thread and unregister the handler
    void stopLooperAndDisconnect();

  private:
    friend ACameraCaptureSession;
    camera_status_t checkCameraClosedOrErrorLocked() const;

    // device goes into fatal error state after this
    void setCameraDeviceErrorLocked(camera_status_t error);

    void disconnectLocked(sp<ACameraCaptureSession>& session); // disconnect from camera service

    camera_status_t stopRepeatingLocked();

    camera_status_t flushLocked(ACameraCaptureSession*);

    camera_status_t waitUntilIdleLocked();


    template<class T>
    camera_status_t captureLocked(sp<ACameraCaptureSession> session,
            /*optional*/T* cbs,
            int numRequests, ACaptureRequest** requests,
            /*optional*/int* captureSequenceId);

    template<class T>
    camera_status_t setRepeatingRequestsLocked(sp<ACameraCaptureSession> session,
            /*optional*/T* cbs,
            int numRequests, ACaptureRequest** requests,
            /*optional*/int* captureSequenceId);

    template<class T>
    camera_status_t submitRequestsLocked(
            sp<ACameraCaptureSession> session,
            /*optional*/T* cbs,
            int numRequests, ACaptureRequest** requests,
            /*out*/int* captureSequenceId,
            bool isRepeating);

    camera_status_t updateOutputConfigurationLocked(ACaptureSessionOutput *output);

    camera_status_t allocateCaptureRequest(
            const ACaptureRequest* request, sp<CaptureRequest>& outReq);

    static ACaptureRequest* allocateACaptureRequest(sp<CaptureRequest>& req,
            const std::string& deviceId);
    static void freeACaptureRequest(ACaptureRequest*);

    // only For session to hold device lock
    // Always grab device lock before grabbing session lock
    void lockDeviceForSessionOps() const { mDeviceLock.lock(); };
    void unlockDevice() const { mDeviceLock.unlock(); };

    // For capture session to notify its end of life
    void notifySessionEndOfLifeLocked(ACameraCaptureSession* session);

    camera_status_t configureStreamsLocked(const ACaptureSessionOutputContainer* outputs,
           const ACaptureRequest* sessionParameters);

    // Input message will be posted and cleared after this returns
    void postSessionMsgAndCleanup(sp<AMessage>& msg);

    static camera_status_t getIGBPfromAnw(
            ANativeWindow* anw, sp<IGraphicBufferProducer>& out);

    static camera_status_t getSurfaceFromANativeWindow(
            ANativeWindow* anw, sp<Surface>& out);

    mutable Mutex mDeviceLock;
    const String8 mCameraId;                          // Camera ID
    const ACameraDevice_StateCallbacks mAppCallbacks; // Callback to app
    const sp<ACameraMetadata> mChars;                 // Camera characteristics
    const sp<ServiceCallback> mServiceCallback;
    ACameraDevice* mWrapper;

    // stream id -> pair of (ANW* from application, OutputConfiguration used for camera service)
    std::map<int, std::pair<ANativeWindow*, OutputConfiguration>> mConfiguredOutputs;

    // TODO: maybe a bool will suffice for synchronous implementation?
    std::atomic_bool mClosing;
    inline bool isClosed() { return mClosing; }

    bool mInError = false;
    camera_status_t mError = ACAMERA_OK;
    void onCaptureErrorLocked(
            int32_t errorCode,
            const CaptureResultExtras& resultExtras);

    bool mIdle = true;
    // This will avoid a busy session being deleted before it's back to idle state
    sp<ACameraCaptureSession> mBusySession;

    sp<hardware::camera2::ICameraDeviceUser> mRemote;

    // Looper thread to handle callback to app
    sp<ALooper> mCbLooper;
    // definition of handler and message
    enum {
        // Device state callbacks
        kWhatOnDisconnected,   // onDisconnected
        kWhatOnError,          // onError
        // Session state callbacks
        kWhatSessionStateCb,   // onReady, onActive
        // Capture callbacks
        kWhatCaptureStart,     // onCaptureStarted
        kWhatCaptureResult,    // onCaptureProgressed, onCaptureCompleted
        kWhatLogicalCaptureResult, // onLogicalCameraCaptureCompleted
        kWhatCaptureFail,      // onCaptureFailed
        kWhatLogicalCaptureFail, // onLogicalCameraCaptureFailed
        kWhatCaptureSeqEnd,    // onCaptureSequenceCompleted
        kWhatCaptureSeqAbort,  // onCaptureSequenceAborted
        kWhatCaptureBufferLost,// onCaptureBufferLost
        // Internal cleanup
        kWhatCleanUpSessions   // Cleanup cached sp<ACameraCaptureSession>
    };
    static const char* kContextKey;
    static const char* kDeviceKey;
    static const char* kErrorCodeKey;
    static const char* kCallbackFpKey;
    static const char* kSessionSpKey;
    static const char* kCaptureRequestKey;
    static const char* kTimeStampKey;
    static const char* kCaptureResultKey;
    static const char* kPhysicalCaptureResultKey;
    static const char* kCaptureFailureKey;
    static const char* kSequenceIdKey;
    static const char* kFrameNumberKey;
    static const char* kAnwKey;
    static const char* kFailingPhysicalCameraId;

    class CallbackHandler : public AHandler {
      public:
        explicit CallbackHandler(const char* id);
        void onMessageReceived(const sp<AMessage> &msg) override;

      private:
        std::string mId;
        // This handler will cache all capture session sp until kWhatCleanUpSessions
        // is processed. This is used to guarantee the last session reference is always
        // being removed in callback thread without holding camera device lock
        Vector<sp<ACameraCaptureSession>> mCachedSessions;
    };
    sp<CallbackHandler> mHandler;

    /***********************************
     * Capture session related members *
     ***********************************/
    // The current active session
    wp<ACameraCaptureSession> mCurrentSession;
    bool mFlushing = false;

    int mNextSessionId = 0;
    // TODO: might need another looper/handler to handle callbacks from service

    static const int REQUEST_ID_NONE = -1;
    int mRepeatingSequenceId = REQUEST_ID_NONE;

    // sequence id -> last frame number map
    std::map<int, int64_t> mSequenceLastFrameNumberMap;

    struct CallbackHolder {
        CallbackHolder(sp<ACameraCaptureSession>          session,
                       const Vector<sp<CaptureRequest> >& requests,
                       bool                               isRepeating,
                       ACameraCaptureSession_captureCallbacks* cbs);
        CallbackHolder(sp<ACameraCaptureSession>          session,
                       const Vector<sp<CaptureRequest> >& requests,
                       bool                               isRepeating,
                       ACameraCaptureSession_logicalCamera_captureCallbacks* lcbs);

        template <class T>
        void initCaptureCallbacks(T* cbs) {
            mContext = nullptr;
            mOnCaptureStarted = nullptr;
            mOnCaptureProgressed = nullptr;
            mOnCaptureCompleted = nullptr;
            mOnLogicalCameraCaptureCompleted = nullptr;
            mOnLogicalCameraCaptureFailed = nullptr;
            mOnCaptureFailed = nullptr;
            mOnCaptureSequenceCompleted = nullptr;
            mOnCaptureSequenceAborted = nullptr;
            mOnCaptureBufferLost = nullptr;
            if (cbs != nullptr) {
                mContext = cbs->context;
                mOnCaptureStarted = cbs->onCaptureStarted;
                mOnCaptureProgressed = cbs->onCaptureProgressed;
                mOnCaptureSequenceCompleted = cbs->onCaptureSequenceCompleted;
                mOnCaptureSequenceAborted = cbs->onCaptureSequenceAborted;
                mOnCaptureBufferLost = cbs->onCaptureBufferLost;
            }
        }
        sp<ACameraCaptureSession>   mSession;
        Vector<sp<CaptureRequest> > mRequests;
        const bool                  mIsRepeating;
        const bool                  mIsLogicalCameraCallback;

        void*                       mContext;
        ACameraCaptureSession_captureCallback_start mOnCaptureStarted;
        ACameraCaptureSession_captureCallback_result mOnCaptureProgressed;
        ACameraCaptureSession_captureCallback_result mOnCaptureCompleted;
        ACameraCaptureSession_logicalCamera_captureCallback_result mOnLogicalCameraCaptureCompleted;
        ACameraCaptureSession_logicalCamera_captureCallback_failed mOnLogicalCameraCaptureFailed;
        ACameraCaptureSession_captureCallback_failed mOnCaptureFailed;
        ACameraCaptureSession_captureCallback_sequenceEnd mOnCaptureSequenceCompleted;
        ACameraCaptureSession_captureCallback_sequenceAbort mOnCaptureSequenceAborted;
        ACameraCaptureSession_captureCallback_bufferLost mOnCaptureBufferLost;
    };
    // sequence id -> callbacks map
    std::map<int, CallbackHolder> mSequenceCallbackMap;

    static const int64_t NO_FRAMES_CAPTURED = -1;
    class FrameNumberTracker {
      public:
        // TODO: Called in onResultReceived and onCaptureErrorLocked
        void updateTracker(int64_t frameNumber, bool isError);
        inline int64_t getCompletedFrameNumber() { return mCompletedFrameNumber; }
      private:
        void update();
        void updateCompletedFrameNumber(int64_t frameNumber);

        int64_t mCompletedFrameNumber = NO_FRAMES_CAPTURED;
        List<int64_t> mSkippedFrameNumbers;
        std::set<int64_t> mFutureErrorSet;
    };
    FrameNumberTracker mFrameNumberTracker;

    void checkRepeatingSequenceCompleteLocked(const int sequenceId, const int64_t lastFrameNumber);
    void checkAndFireSequenceCompleteLocked();

    // Misc variables
    int32_t mShadingMapSize[2];   // const after constructor
    int32_t mPartialResultCount;  // const after constructor
    std::vector<std::string> mPhysicalIds; // const after constructor

};

} // namespace acam;
} // namespace android;

/**
 * ACameraDevice opaque struct definition
 * Leave outside of android namespace because it's NDK struct
 */
struct ACameraDevice {
    ACameraDevice(const char* id, ACameraDevice_StateCallbacks* cb,
                  sp<ACameraMetadata> chars) :
            mDevice(new android::acam::CameraDevice(id, cb, chars, this)) {}

    ~ACameraDevice();

    /*******************
     * NDK public APIs *
     *******************/
    inline const char* getId() const { return mDevice->getId(); }

    camera_status_t createCaptureRequest(
            ACameraDevice_request_template templateId,
            const ACameraIdList* physicalCameraIdList,
            ACaptureRequest** request) const {
        return mDevice->createCaptureRequest(templateId, physicalCameraIdList, request);
    }

    camera_status_t createCaptureSession(
            const ACaptureSessionOutputContainer*       outputs,
            const ACaptureRequest* sessionParameters,
            const ACameraCaptureSession_stateCallbacks* callbacks,
            /*out*/ACameraCaptureSession** session) {
        return mDevice->createCaptureSession(outputs, sessionParameters, callbacks, session);
    }

    camera_status_t isSessionConfigurationSupported(
            const ACaptureSessionOutputContainer* sessionOutputContainer) const {
        return mDevice->isSessionConfigurationSupported(sessionOutputContainer);
    }

    /***********************
     * Device interal APIs *
     ***********************/
    inline android::sp<android::hardware::camera2::ICameraDeviceCallbacks> getServiceCallback() {
        return mDevice->getServiceCallback();
    };

    // Camera device is only functional after remote being set
    inline void setRemoteDevice(android::sp<android::hardware::camera2::ICameraDeviceUser> remote) {
        mDevice->setRemoteDevice(remote);
    }

  private:
    android::sp<android::acam::CameraDevice> mDevice;
};

#endif // _ACAMERA_DEVICE_H
