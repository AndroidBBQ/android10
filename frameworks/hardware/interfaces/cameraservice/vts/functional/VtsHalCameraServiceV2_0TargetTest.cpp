/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define LOG_TAG "VtsHalCameraServiceV2_0TargetTest"
//#define LOG_NDEBUG 0

#include <android/frameworks/cameraservice/device/2.0/ICameraDeviceUser.h>
#include <android/frameworks/cameraservice/service/2.0/ICameraService.h>
#include <system/camera_metadata.h>

#include <fmq/MessageQueue.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>

#include <gtest/gtest.h>
#include <stdint.h>
#include <unistd.h>

#include <stdio.h>
#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#include <media/NdkImageReader.h>

#include <android/log.h>

#include <CameraMetadata.h>
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetTestEnvBase.h>

namespace android {

using android::Condition;
using android::Mutex;
using android::sp;
using android::frameworks::cameraservice::common::V2_0::Status;
using android::frameworks::cameraservice::device::V2_0::CaptureRequest;
using android::frameworks::cameraservice::device::V2_0::CaptureResultExtras;
using android::frameworks::cameraservice::device::V2_0::ErrorCode;
using android::frameworks::cameraservice::device::V2_0::FmqSizeOrMetadata;
using android::frameworks::cameraservice::device::V2_0::ICameraDeviceCallback;
using android::frameworks::cameraservice::device::V2_0::ICameraDeviceUser;
using android::frameworks::cameraservice::device::V2_0::OutputConfiguration;
using android::frameworks::cameraservice::device::V2_0::PhysicalCaptureResultInfo;
using android::frameworks::cameraservice::device::V2_0::StreamConfigurationMode;
using android::frameworks::cameraservice::device::V2_0::SubmitInfo;
using android::frameworks::cameraservice::device::V2_0::TemplateId;
using android::frameworks::cameraservice::service::V2_0::CameraDeviceStatus;
using android::frameworks::cameraservice::service::V2_0::CameraStatusAndId;
using android::frameworks::cameraservice::service::V2_0::ICameraService;
using android::frameworks::cameraservice::service::V2_0::ICameraServiceListener;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::Return;
using android::hardware::Void;
using android::hardware::camera::common::V1_0::helper::CameraMetadata;
using camera_metadata_enum_android_depth_available_depth_stream_configurations::
    ANDROID_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS_OUTPUT;
using RequestMetadataQueue = hardware::MessageQueue<uint8_t, hardware::kSynchronizedReadWrite>;

static constexpr int kCaptureRequestCount = 10;
static constexpr int kVGAImageWidth = 640;
static constexpr int kVGAImageHeight = 480;
static constexpr int kNumRequests = 4;

#define ASSERT_NOT_NULL(x) ASSERT_TRUE((x) != nullptr)

#define SETUP_TIMEOUT 2000000000  // ns
#define IDLE_TIMEOUT 2000000000   // ns

// Stub listener implementation
class CameraServiceListener : public ICameraServiceListener {
    std::map<hidl_string, CameraDeviceStatus> mCameraStatuses;
    mutable Mutex mLock;

   public:
    virtual ~CameraServiceListener(){};

    virtual Return<void> onStatusChanged(const CameraStatusAndId& statusAndId) override {
        Mutex::Autolock l(mLock);
        mCameraStatuses[statusAndId.cameraId] = statusAndId.deviceStatus;
        return Void();
    };
};

// ICameraDeviceCallback implementation
class CameraDeviceCallbacks : public ICameraDeviceCallback {
   public:
    enum Status {
        IDLE,
        ERROR,
        PREPARED,
        RUNNING,
        RESULT_RECEIVED,
        UNINITIALIZED,
        REPEATING_REQUEST_ERROR,
    };

   protected:
    bool mError = false;
    Status mLastStatus = UNINITIALIZED;
    mutable std::vector<Status> mStatusesHit;
    mutable Mutex mLock;
    mutable Condition mStatusCondition;

   public:
    CameraDeviceCallbacks() {}

    virtual ~CameraDeviceCallbacks() {}

    virtual Return<void> onDeviceError(ErrorCode errorCode,
                                       const CaptureResultExtras& resultExtras) override {
        (void)resultExtras;
        ALOGE("%s: onDeviceError occurred with: %d", __FUNCTION__, static_cast<int>(errorCode));
        Mutex::Autolock l(mLock);
        mError = true;
        mLastStatus = ERROR;
        mStatusesHit.push_back(mLastStatus);
        mStatusCondition.broadcast();
        return Void();
    }

    virtual Return<void> onDeviceIdle() override {
        Mutex::Autolock l(mLock);
        mLastStatus = IDLE;
        mStatusesHit.push_back(mLastStatus);
        mStatusCondition.broadcast();
        return Void();
    }

    virtual Return<void> onCaptureStarted(const CaptureResultExtras& resultExtras,
                                          uint64_t timestamp) override {
        (void)resultExtras;
        (void)timestamp;
        Mutex::Autolock l(mLock);
        mLastStatus = RUNNING;
        mStatusesHit.push_back(mLastStatus);
        mStatusCondition.broadcast();
        return Void();
    }

    virtual Return<void> onResultReceived(
        const FmqSizeOrMetadata& sizeOrMetadata, const CaptureResultExtras& resultExtras,
        const hidl_vec<PhysicalCaptureResultInfo>& physicalResultInfos) override {
        (void)sizeOrMetadata;
        (void)resultExtras;
        (void)physicalResultInfos;
        Mutex::Autolock l(mLock);
        mLastStatus = RESULT_RECEIVED;
        mStatusesHit.push_back(mLastStatus);
        mStatusCondition.broadcast();
        return Void();
    }

    virtual Return<void> onRepeatingRequestError(uint64_t lastFrameNumber,
                                                 int32_t stoppedSequenceId) override {
        (void)lastFrameNumber;
        (void)stoppedSequenceId;
        Mutex::Autolock l(mLock);
        mLastStatus = REPEATING_REQUEST_ERROR;
        mStatusesHit.push_back(mLastStatus);
        mStatusCondition.broadcast();
        return Void();
    }

    // Test helper functions:

    bool hadError() const {
        Mutex::Autolock l(mLock);
        return mError;
    }
    bool waitForStatus(Status status) const {
        Mutex::Autolock l(mLock);
        if (mLastStatus == status) {
            return true;
        }

        while (std::find(mStatusesHit.begin(), mStatusesHit.end(), status) == mStatusesHit.end()) {
            if (mStatusCondition.waitRelative(mLock, IDLE_TIMEOUT) != android::OK) {
                mStatusesHit.clear();
                return false;
            }
        }
        mStatusesHit.clear();

        return true;
    }

    void clearStatus() const {
        Mutex::Autolock l(mLock);
        mStatusesHit.clear();
    }

    bool waitForIdle() const { return waitForStatus(IDLE); }
};

class CameraHidlEnvironment : public ::testing::VtsHalHidlTargetTestEnvBase {
   public:
    // get the test environment singleton
    static CameraHidlEnvironment* Instance() {
        static CameraHidlEnvironment* instance = new CameraHidlEnvironment();
        return instance;
    }

    virtual void registerTestServices() override { registerTestService<ICameraService>(); }
};

static bool convertFromHidlCloned(const hidl_vec<uint8_t>& metadata, CameraMetadata* rawMetadata) {
    const camera_metadata* buffer = (camera_metadata_t*)(metadata.data());
    size_t expectedSize = metadata.size();
    int ret = validate_camera_metadata_structure(buffer, &expectedSize);
    if (ret == OK || ret == CAMERA_METADATA_VALIDATION_SHIFTED) {
        *rawMetadata = buffer;
    } else {
        ALOGE("%s: Malformed camera metadata received from caller", __FUNCTION__);
        return false;
    }
    return true;
}

struct StreamConfiguration {
    int32_t width = -1;
    int32_t height = -1;
};

class VtsHalCameraServiceV2_0TargetTest : public ::testing::Test {
   public:
    void SetUp() override {
        cs = ::testing::VtsHalHidlTargetTestBase::getService<ICameraService>(
            CameraHidlEnvironment::Instance()->getServiceName<ICameraService>());
    }

    void TearDown() override {}
    // creates an outputConfiguration with no deferred streams
    OutputConfiguration createOutputConfiguration(const std::vector<native_handle_t*>& nhs) {
        OutputConfiguration output;
        output.rotation = OutputConfiguration::Rotation::R0;
        output.windowGroupId = -1;
        output.windowHandles.resize(nhs.size());
        output.width = 0;
        output.height = 0;
        output.isDeferred = false;
        for (size_t i = 0; i < nhs.size(); i++) {
            output.windowHandles[i] = nhs[i];
        }
        return output;
    }

    void initializeCaptureRequestPartial(CaptureRequest* captureRequest, int32_t streamId,
                                         const hidl_string& cameraId, size_t settingsSize) {
        captureRequest->physicalCameraSettings.resize(1);
        captureRequest->physicalCameraSettings[0].id = cameraId;
        captureRequest->streamAndWindowIds.resize(1);
        captureRequest->streamAndWindowIds[0].streamId = streamId;
        captureRequest->streamAndWindowIds[0].windowId = 0;
        // Write the settings metadata into the fmq.
        captureRequest->physicalCameraSettings[0].settings.fmqMetadataSize(settingsSize);
    }

    bool doesCapabilityExist(const CameraMetadata& characteristics, int capability) {
        camera_metadata_ro_entry rawEntry =
            characteristics.find(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
        EXPECT_TRUE(rawEntry.count > 0);
        for (size_t i = 0; i < rawEntry.count; i++) {
            if (rawEntry.data.u8[i] == capability) {
                return true;
            }
        }
        return false;
    }

    // Return the first advertised available depth stream sizes
    StreamConfiguration getDepthStreamConfiguration(const CameraMetadata& characteristics) {
        camera_metadata_ro_entry rawEntry =
            characteristics.find(ANDROID_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS);
        StreamConfiguration streamConfig;
        const size_t STREAM_FORMAT_OFFSET = 0;
        const size_t STREAM_WIDTH_OFFSET = 1;
        const size_t STREAM_HEIGHT_OFFSET = 2;
        const size_t STREAM_INOUT_OFFSET = 3;
        const size_t STREAM_CONFIG_SIZE = 4;
        if (rawEntry.count < STREAM_CONFIG_SIZE) {
            return streamConfig;
        }
        EXPECT_TRUE((rawEntry.count % STREAM_CONFIG_SIZE) == 0);
        for (size_t i = 0; i < rawEntry.count; i += STREAM_CONFIG_SIZE) {
            int32_t format = rawEntry.data.i32[i + STREAM_FORMAT_OFFSET];
            int32_t use = rawEntry.data.i32[i + STREAM_INOUT_OFFSET];
            if (format == AIMAGE_FORMAT_DEPTH16 &&
                use == ANDROID_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS_OUTPUT) {
                streamConfig.width = rawEntry.data.i32[i + STREAM_WIDTH_OFFSET];
                streamConfig.height = rawEntry.data.i32[i + STREAM_HEIGHT_OFFSET];
                return streamConfig;
            }
        }
        return streamConfig;
    }

    sp<ICameraService> cs = nullptr;
};

// Basic HIDL calls for ICameraService
TEST_F(VtsHalCameraServiceV2_0TargetTest, BasicCameraLifeCycleTest) {
    sp<CameraServiceListener> listener(new CameraServiceListener());
    hidl_vec<CameraStatusAndId> cameraStatuses{};
    Status status = Status::NO_ERROR;
    auto remoteRet =
        cs->addListener(listener, [&status, &cameraStatuses](Status s, auto& retStatuses) {
            status = s;
            cameraStatuses = retStatuses;
        });
    EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
    for (const auto& it : cameraStatuses) {
        CameraMetadata rawMetadata;
        listener->onStatusChanged(it);
        if (it.deviceStatus != CameraDeviceStatus::STATUS_PRESENT) {
            continue;
        }
        remoteRet = cs->getCameraCharacteristics(
            it.cameraId, [&status, &rawMetadata](auto s, const hidl_vec<uint8_t>& metadata) {
                status = s;
                bool cStatus = convertFromHidlCloned(metadata, &rawMetadata);
                EXPECT_TRUE(cStatus);
            });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_FALSE(rawMetadata.isEmpty());
        sp<CameraDeviceCallbacks> callbacks(new CameraDeviceCallbacks());
        sp<ICameraDeviceUser> deviceRemote = nullptr;
        remoteRet = cs->connectDevice(callbacks, it.cameraId,
                                      [&status, &deviceRemote](auto s, auto& device) {
                                          status = s;
                                          deviceRemote = device;
                                      });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_TRUE(deviceRemote != nullptr);

        std::shared_ptr<RequestMetadataQueue> requestMQ = nullptr;
        remoteRet = deviceRemote->getCaptureRequestMetadataQueue([&requestMQ](const auto& mqD) {
            requestMQ = std::make_shared<RequestMetadataQueue>(mqD);
            EXPECT_TRUE(requestMQ->isValid() && (requestMQ->availableToWrite() >= 0));
        });
        EXPECT_TRUE(remoteRet.isOk());
        AImageReader* reader = nullptr;
        bool isDepthOnlyDevice =
            !doesCapabilityExist(rawMetadata,
                                 ANDROID_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE) &&
            doesCapabilityExist(rawMetadata, ANDROID_REQUEST_AVAILABLE_CAPABILITIES_DEPTH_OUTPUT);
        int chosenImageFormat = AIMAGE_FORMAT_YUV_420_888;
        int chosenImageWidth = kVGAImageWidth;
        int chosenImageHeight = kVGAImageHeight;
        if (isDepthOnlyDevice) {
            StreamConfiguration depthStreamConfig = getDepthStreamConfiguration(rawMetadata);
            EXPECT_TRUE(depthStreamConfig.width != -1);
            EXPECT_TRUE(depthStreamConfig.height != -1);
            chosenImageFormat = AIMAGE_FORMAT_DEPTH16;
            chosenImageWidth = depthStreamConfig.width;
            chosenImageHeight = depthStreamConfig.height;
        }

        auto mStatus = AImageReader_new(chosenImageWidth, chosenImageHeight, chosenImageFormat,
                                        kCaptureRequestCount, &reader);
        EXPECT_EQ(mStatus, AMEDIA_OK);
        native_handle_t* wh = nullptr;
        mStatus = AImageReader_getWindowNativeHandle(reader, &wh);
        EXPECT_TRUE(mStatus == AMEDIA_OK && wh != nullptr);
        OutputConfiguration output = createOutputConfiguration({wh});
        Return<Status> ret = deviceRemote->beginConfigure();
        EXPECT_TRUE(ret.isOk() && ret == Status::NO_ERROR);
        int32_t streamId = -1;
        remoteRet = deviceRemote->createStream(output, [&status, &streamId](Status s, auto sId) {
            status = s;
            streamId = sId;
        });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_TRUE(streamId >= 0);
        hidl_vec<uint8_t> hidlParams;
        ret = deviceRemote->endConfigure(StreamConfigurationMode::NORMAL_MODE, hidlParams);
        EXPECT_TRUE(ret.isOk() && ret == Status::NO_ERROR);
        hidl_vec<uint8_t> settingsMetadata;
        remoteRet = deviceRemote->createDefaultRequest(
            TemplateId::PREVIEW, [&status, &settingsMetadata](auto s, const hidl_vec<uint8_t> m) {
                status = s;
                settingsMetadata = m;
            });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_GE(settingsMetadata.size(), 0);
        hidl_vec<CaptureRequest> captureRequests;
        captureRequests.resize(kNumRequests);
        for (int i = 0; i < kNumRequests; i++) {
            CaptureRequest& captureRequest = captureRequests[i];
            initializeCaptureRequestPartial(&captureRequest, streamId, it.cameraId,
                                            settingsMetadata.size());
            // Write the settings metadata into the fmq.
            bool written = requestMQ->write(settingsMetadata.data(), settingsMetadata.size());
            EXPECT_TRUE(written);
        }
        SubmitInfo info;

        // Test a single capture
        remoteRet = deviceRemote->submitRequestList(captureRequests, false,
                                                    [&status, &info](auto s, auto& submitInfo) {
                                                        status = s;
                                                        info = submitInfo;
                                                    });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_GE(info.requestId, 0);
        EXPECT_TRUE(callbacks->waitForStatus(CameraDeviceCallbacks::Status::RESULT_RECEIVED));
        EXPECT_TRUE(callbacks->waitForIdle());

        // Test repeating requests
        CaptureRequest captureRequest;

        initializeCaptureRequestPartial(&captureRequest, streamId, it.cameraId,
                                        settingsMetadata.size());

        bool written = requestMQ->write(settingsMetadata.data(), settingsMetadata.size());
        EXPECT_TRUE(written);

        remoteRet = deviceRemote->submitRequestList({captureRequest}, true,
                                                    [&status, &info](auto s, auto& submitInfo) {
                                                        status = s;
                                                        info = submitInfo;
                                                    });
        EXPECT_TRUE(callbacks->waitForStatus(CameraDeviceCallbacks::Status::RESULT_RECEIVED));
        int64_t lastFrameNumber = -1;
        remoteRet =
            deviceRemote->cancelRepeatingRequest([&status, &lastFrameNumber](auto s, int64_t lf) {
                status = s;
                lastFrameNumber = lf;
            });
        EXPECT_TRUE(remoteRet.isOk() && status == Status::NO_ERROR);
        EXPECT_GE(lastFrameNumber, 0);

        // Test waitUntilIdle()
        auto statusRet = deviceRemote->waitUntilIdle();
        EXPECT_TRUE(statusRet.isOk() && statusRet == Status::NO_ERROR);

        // Test deleteStream()
        statusRet = deviceRemote->deleteStream(streamId);
        EXPECT_TRUE(statusRet.isOk() && statusRet == Status::NO_ERROR);

        remoteRet = deviceRemote->disconnect();
        EXPECT_TRUE(remoteRet.isOk());
    }
    Return<Status> ret = cs->removeListener(listener);
    EXPECT_TRUE(ret.isOk() && ret == Status::NO_ERROR);
}

}  // namespace android

int main(int argc, char** argv) {
    ::testing::AddGlobalTestEnvironment(android::CameraHidlEnvironment::Instance());
    ::testing::InitGoogleTest(&argc, argv);
    android::CameraHidlEnvironment::Instance()->init(&argc, argv);
    int status = RUN_ALL_TESTS();
    return status;
}
