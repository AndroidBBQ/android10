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

#define LOG_TAG "Camera3-DepthCompositeStream"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include "api1/client2/JpegProcessor.h"
#include "common/CameraProviderManager.h"
#include "dlfcn.h"
#include <gui/Surface.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include "DepthCompositeStream.h"

namespace android {
namespace camera3 {

DepthCompositeStream::DepthCompositeStream(wp<CameraDeviceBase> device,
        wp<hardware::camera2::ICameraDeviceCallbacks> cb) :
        CompositeStream(device, cb),
        mBlobStreamId(-1),
        mBlobSurfaceId(-1),
        mDepthStreamId(-1),
        mDepthSurfaceId(-1),
        mBlobWidth(0),
        mBlobHeight(0),
        mDepthBufferAcquired(false),
        mBlobBufferAcquired(false),
        mProducerListener(new ProducerListener()),
        mMaxJpegSize(-1),
        mIsLogicalCamera(false),
        mDepthPhotoLibHandle(nullptr),
        mDepthPhotoProcess(nullptr) {
    sp<CameraDeviceBase> cameraDevice = device.promote();
    if (cameraDevice.get() != nullptr) {
        CameraMetadata staticInfo = cameraDevice->info();
        auto entry = staticInfo.find(ANDROID_JPEG_MAX_SIZE);
        if (entry.count > 0) {
            mMaxJpegSize = entry.data.i32[0];
        } else {
            ALOGW("%s: Maximum jpeg size absent from camera characteristics", __FUNCTION__);
        }

        entry = staticInfo.find(ANDROID_LENS_INTRINSIC_CALIBRATION);
        if (entry.count == 5) {
            mIntrinsicCalibration.reserve(5);
            mIntrinsicCalibration.insert(mIntrinsicCalibration.end(), entry.data.f,
                    entry.data.f + 5);
        } else {
            ALOGW("%s: Intrinsic calibration absent from camera characteristics!", __FUNCTION__);
        }

        entry = staticInfo.find(ANDROID_LENS_DISTORTION);
        if (entry.count == 5) {
            mLensDistortion.reserve(5);
            mLensDistortion.insert(mLensDistortion.end(), entry.data.f, entry.data.f + 5);
        } else {
            ALOGW("%s: Lens distortion absent from camera characteristics!", __FUNCTION__);
        }

        entry = staticInfo.find(ANDROID_REQUEST_AVAILABLE_CAPABILITIES);
        for (size_t i = 0; i < entry.count; ++i) {
            uint8_t capability = entry.data.u8[i];
            if (capability == ANDROID_REQUEST_AVAILABLE_CAPABILITIES_LOGICAL_MULTI_CAMERA) {
                mIsLogicalCamera = true;
                break;
            }
        }

        getSupportedDepthSizes(staticInfo, &mSupportedDepthSizes);

        mDepthPhotoLibHandle = dlopen(camera3::kDepthPhotoLibrary, RTLD_NOW | RTLD_LOCAL);
        if (mDepthPhotoLibHandle != nullptr) {
            mDepthPhotoProcess = reinterpret_cast<camera3::process_depth_photo_frame> (
                    dlsym(mDepthPhotoLibHandle, camera3::kDepthPhotoProcessFunction));
            if (mDepthPhotoProcess == nullptr) {
                ALOGE("%s: Failed to link to depth photo process function: %s", __FUNCTION__,
                        dlerror());
            }
        } else {
            ALOGE("%s: Failed to link to depth photo library: %s", __FUNCTION__, dlerror());
        }

    }
}

DepthCompositeStream::~DepthCompositeStream() {
    mBlobConsumer.clear(),
    mBlobSurface.clear(),
    mBlobStreamId = -1;
    mBlobSurfaceId = -1;
    mDepthConsumer.clear();
    mDepthSurface.clear();
    mDepthConsumer = nullptr;
    mDepthSurface = nullptr;
    if (mDepthPhotoLibHandle != nullptr) {
        dlclose(mDepthPhotoLibHandle);
        mDepthPhotoLibHandle = nullptr;
    }
    mDepthPhotoProcess = nullptr;
}

void DepthCompositeStream::compilePendingInputLocked() {
    CpuConsumer::LockedBuffer imgBuffer;

    while (!mInputJpegBuffers.empty() && !mBlobBufferAcquired) {
        auto it = mInputJpegBuffers.begin();
        auto res = mBlobConsumer->lockNextBuffer(&imgBuffer);
        if (res == NOT_ENOUGH_DATA) {
            // Can not lock any more buffers.
            break;
        } else if (res != OK) {
            ALOGE("%s: Error locking blob image buffer: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            mPendingInputFrames[*it].error = true;
            mInputJpegBuffers.erase(it);
            continue;
        }

        if (*it != imgBuffer.timestamp) {
            ALOGW("%s: Expecting jpeg buffer with time stamp: %" PRId64 " received buffer with "
                    "time stamp: %" PRId64, __FUNCTION__, *it, imgBuffer.timestamp);
        }

        if ((mPendingInputFrames.find(imgBuffer.timestamp) != mPendingInputFrames.end()) &&
                (mPendingInputFrames[imgBuffer.timestamp].error)) {
            mBlobConsumer->unlockBuffer(imgBuffer);
        } else {
            mPendingInputFrames[imgBuffer.timestamp].jpegBuffer = imgBuffer;
            mBlobBufferAcquired = true;
        }
        mInputJpegBuffers.erase(it);
    }

    while (!mInputDepthBuffers.empty() && !mDepthBufferAcquired) {
        auto it = mInputDepthBuffers.begin();
        auto res = mDepthConsumer->lockNextBuffer(&imgBuffer);
        if (res == NOT_ENOUGH_DATA) {
            // Can not lock any more buffers.
            break;
        } else if (res != OK) {
            ALOGE("%s: Error receiving depth image buffer: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            mPendingInputFrames[*it].error = true;
            mInputDepthBuffers.erase(it);
            continue;
        }

        if (*it != imgBuffer.timestamp) {
            ALOGW("%s: Expecting depth buffer with time stamp: %" PRId64 " received buffer with "
                    "time stamp: %" PRId64, __FUNCTION__, *it, imgBuffer.timestamp);
        }

        if ((mPendingInputFrames.find(imgBuffer.timestamp) != mPendingInputFrames.end()) &&
                (mPendingInputFrames[imgBuffer.timestamp].error)) {
            mDepthConsumer->unlockBuffer(imgBuffer);
        } else {
            mPendingInputFrames[imgBuffer.timestamp].depthBuffer = imgBuffer;
            mDepthBufferAcquired = true;
        }
        mInputDepthBuffers.erase(it);
    }

    while (!mCaptureResults.empty()) {
        auto it = mCaptureResults.begin();
        // Negative timestamp indicates that something went wrong during the capture result
        // collection process.
        if (it->first >= 0) {
            mPendingInputFrames[it->first].frameNumber = std::get<0>(it->second);
            mPendingInputFrames[it->first].result = std::get<1>(it->second);
        }
        mCaptureResults.erase(it);
    }

    while (!mFrameNumberMap.empty()) {
        auto it = mFrameNumberMap.begin();
        mPendingInputFrames[it->second].frameNumber = it->first;
        mFrameNumberMap.erase(it);
    }

    auto it = mErrorFrameNumbers.begin();
    while (it != mErrorFrameNumbers.end()) {
        bool frameFound = false;
        for (auto &inputFrame : mPendingInputFrames) {
            if (inputFrame.second.frameNumber == *it) {
                inputFrame.second.error = true;
                frameFound = true;
                break;
            }
        }

        if (frameFound) {
            it = mErrorFrameNumbers.erase(it);
        } else {
            ALOGW("%s: Not able to find failing input with frame number: %" PRId64, __FUNCTION__,
                    *it);
            it++;
        }
    }
}

bool DepthCompositeStream::getNextReadyInputLocked(int64_t *currentTs /*inout*/) {
    if (currentTs == nullptr) {
        return false;
    }

    bool newInputAvailable = false;
    for (const auto& it : mPendingInputFrames) {
        if ((!it.second.error) && (it.second.depthBuffer.data != nullptr) &&
                (it.second.jpegBuffer.data != nullptr) && (it.first < *currentTs)) {
            *currentTs = it.first;
            newInputAvailable = true;
        }
    }

    return newInputAvailable;
}

int64_t DepthCompositeStream::getNextFailingInputLocked(int64_t *currentTs /*inout*/) {
    int64_t ret = -1;
    if (currentTs == nullptr) {
        return ret;
    }

    for (const auto& it : mPendingInputFrames) {
        if (it.second.error && !it.second.errorNotified && (it.first < *currentTs)) {
            *currentTs = it.first;
            ret = it.second.frameNumber;
        }
    }

    return ret;
}

status_t DepthCompositeStream::processInputFrame(nsecs_t ts, const InputFrame &inputFrame) {
    status_t res;
    sp<ANativeWindow> outputANW = mOutputSurface;
    ANativeWindowBuffer *anb;
    int fenceFd;
    void *dstBuffer;

    auto jpegSize = android::camera2::JpegProcessor::findJpegSize(inputFrame.jpegBuffer.data,
            inputFrame.jpegBuffer.width);
    if (jpegSize == 0) {
        ALOGW("%s: Failed to find input jpeg size, default to using entire buffer!", __FUNCTION__);
        jpegSize = inputFrame.jpegBuffer.width;
    }

    size_t maxDepthJpegSize;
    if (mMaxJpegSize > 0) {
        maxDepthJpegSize = mMaxJpegSize;
    } else {
        maxDepthJpegSize = std::max<size_t> (jpegSize,
                inputFrame.depthBuffer.width * inputFrame.depthBuffer.height * 3 / 2);
    }
    uint8_t jpegQuality = 100;
    auto entry = inputFrame.result.find(ANDROID_JPEG_QUALITY);
    if (entry.count > 0) {
        jpegQuality = entry.data.u8[0];
    }

    // The final depth photo will consist of the main jpeg buffer, the depth map buffer (also in
    // jpeg format) and confidence map (jpeg as well). Assume worst case that all 3 jpeg need
    // max jpeg size.
    size_t finalJpegBufferSize = maxDepthJpegSize * 3;

    if ((res = native_window_set_buffers_dimensions(mOutputSurface.get(), finalJpegBufferSize, 1))
            != OK) {
        ALOGE("%s: Unable to configure stream buffer dimensions"
                " %zux%u for stream %d", __FUNCTION__, finalJpegBufferSize, 1U, mBlobStreamId);
        return res;
    }

    res = outputANW->dequeueBuffer(mOutputSurface.get(), &anb, &fenceFd);
    if (res != OK) {
        ALOGE("%s: Error retrieving output buffer: %s (%d)", __FUNCTION__, strerror(-res),
                res);
        return res;
    }

    sp<GraphicBuffer> gb = GraphicBuffer::from(anb);
    res = gb->lockAsync(GRALLOC_USAGE_SW_WRITE_OFTEN, &dstBuffer, fenceFd);
    if (res != OK) {
        ALOGE("%s: Error trying to lock output buffer fence: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        outputANW->cancelBuffer(mOutputSurface.get(), anb, /*fence*/ -1);
        return res;
    }

    if ((gb->getWidth() < finalJpegBufferSize) || (gb->getHeight() != 1)) {
        ALOGE("%s: Blob buffer size mismatch, expected %dx%d received %zux%u", __FUNCTION__,
                gb->getWidth(), gb->getHeight(), finalJpegBufferSize, 1U);
        outputANW->cancelBuffer(mOutputSurface.get(), anb, /*fence*/ -1);
        return BAD_VALUE;
    }

    DepthPhotoInputFrame depthPhoto;
    depthPhoto.mMainJpegBuffer = reinterpret_cast<const char*> (inputFrame.jpegBuffer.data);
    depthPhoto.mMainJpegWidth = mBlobWidth;
    depthPhoto.mMainJpegHeight = mBlobHeight;
    depthPhoto.mMainJpegSize = jpegSize;
    depthPhoto.mDepthMapBuffer = reinterpret_cast<uint16_t*> (inputFrame.depthBuffer.data);
    depthPhoto.mDepthMapWidth = inputFrame.depthBuffer.width;
    depthPhoto.mDepthMapHeight = inputFrame.depthBuffer.height;
    depthPhoto.mDepthMapStride = inputFrame.depthBuffer.stride;
    depthPhoto.mJpegQuality = jpegQuality;
    depthPhoto.mIsLogical = mIsLogicalCamera;
    depthPhoto.mMaxJpegSize = maxDepthJpegSize;
    // The camera intrinsic calibration layout is as follows:
    // [focalLengthX, focalLengthY, opticalCenterX, opticalCenterY, skew]
    if (mIntrinsicCalibration.size() == 5) {
        memcpy(depthPhoto.mIntrinsicCalibration, mIntrinsicCalibration.data(),
                sizeof(depthPhoto.mIntrinsicCalibration));
        depthPhoto.mIsIntrinsicCalibrationValid = 1;
    } else {
        depthPhoto.mIsIntrinsicCalibrationValid = 0;
    }
    // The camera lens distortion contains the following lens correction coefficients.
    // [kappa_1, kappa_2, kappa_3 kappa_4, kappa_5]
    if (mLensDistortion.size() == 5) {
        memcpy(depthPhoto.mLensDistortion, mLensDistortion.data(),
                sizeof(depthPhoto.mLensDistortion));
        depthPhoto.mIsLensDistortionValid = 1;
    } else {
        depthPhoto.mIsLensDistortionValid = 0;
    }
    entry = inputFrame.result.find(ANDROID_JPEG_ORIENTATION);
    if (entry.count > 0) {
        // The camera jpeg orientation values must be within [0, 90, 180, 270].
        switch (entry.data.i32[0]) {
            case 0:
            case 90:
            case 180:
            case 270:
                depthPhoto.mOrientation = static_cast<DepthPhotoOrientation> (entry.data.i32[0]);
                break;
            default:
                ALOGE("%s: Unexpected jpeg orientation value: %d, default to 0 degrees",
                        __FUNCTION__, entry.data.i32[0]);
        }
    }

    size_t actualJpegSize = 0;
    res = mDepthPhotoProcess(depthPhoto, finalJpegBufferSize, dstBuffer, &actualJpegSize);
    if (res != 0) {
        ALOGE("%s: Depth photo processing failed: %s (%d)", __FUNCTION__, strerror(-res), res);
        outputANW->cancelBuffer(mOutputSurface.get(), anb, /*fence*/ -1);
        return res;
    }

    size_t finalJpegSize = actualJpegSize + sizeof(struct camera3_jpeg_blob);
    if (finalJpegSize > finalJpegBufferSize) {
        ALOGE("%s: Final jpeg buffer not large enough for the jpeg blob header", __FUNCTION__);
        outputANW->cancelBuffer(mOutputSurface.get(), anb, /*fence*/ -1);
        return NO_MEMORY;
    }

    res = native_window_set_buffers_timestamp(mOutputSurface.get(), ts);
    if (res != OK) {
        ALOGE("%s: Stream %d: Error setting timestamp: %s (%d)", __FUNCTION__,
                getStreamId(), strerror(-res), res);
        return res;
    }

    ALOGV("%s: Final jpeg size: %zu", __func__, finalJpegSize);
    uint8_t* header = static_cast<uint8_t *> (dstBuffer) +
        (gb->getWidth() - sizeof(struct camera3_jpeg_blob));
    struct camera3_jpeg_blob *blob = reinterpret_cast<struct camera3_jpeg_blob*> (header);
    blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
    blob->jpeg_size = actualJpegSize;
    outputANW->queueBuffer(mOutputSurface.get(), anb, /*fence*/ -1);

    return res;
}

void DepthCompositeStream::releaseInputFrameLocked(InputFrame *inputFrame /*out*/) {
    if (inputFrame == nullptr) {
        return;
    }

    if (inputFrame->depthBuffer.data != nullptr) {
        mDepthConsumer->unlockBuffer(inputFrame->depthBuffer);
        inputFrame->depthBuffer.data = nullptr;
        mDepthBufferAcquired = false;
    }

    if (inputFrame->jpegBuffer.data != nullptr) {
        mBlobConsumer->unlockBuffer(inputFrame->jpegBuffer);
        inputFrame->jpegBuffer.data = nullptr;
        mBlobBufferAcquired = false;
    }

    if ((inputFrame->error || mErrorState) && !inputFrame->errorNotified) {
        notifyError(inputFrame->frameNumber);
        inputFrame->errorNotified = true;
    }
}

void DepthCompositeStream::releaseInputFramesLocked(int64_t currentTs) {
    auto it = mPendingInputFrames.begin();
    while (it != mPendingInputFrames.end()) {
        if (it->first <= currentTs) {
            releaseInputFrameLocked(&it->second);
            it = mPendingInputFrames.erase(it);
        } else {
            it++;
        }
    }
}

bool DepthCompositeStream::threadLoop() {
    int64_t currentTs = INT64_MAX;
    bool newInputAvailable = false;

    {
        Mutex::Autolock l(mMutex);

        if (mErrorState) {
            // In case we landed in error state, return any pending buffers and
            // halt all further processing.
            compilePendingInputLocked();
            releaseInputFramesLocked(currentTs);
            return false;
        }

        while (!newInputAvailable) {
            compilePendingInputLocked();
            newInputAvailable = getNextReadyInputLocked(&currentTs);
            if (!newInputAvailable) {
                auto failingFrameNumber = getNextFailingInputLocked(&currentTs);
                if (failingFrameNumber >= 0) {
                    // We cannot erase 'mPendingInputFrames[currentTs]' at this point because it is
                    // possible for two internal stream buffers to fail. In such scenario the
                    // composite stream should notify the client about a stream buffer error only
                    // once and this information is kept within 'errorNotified'.
                    // Any present failed input frames will be removed on a subsequent call to
                    // 'releaseInputFramesLocked()'.
                    releaseInputFrameLocked(&mPendingInputFrames[currentTs]);
                    currentTs = INT64_MAX;
                }

                auto ret = mInputReadyCondition.waitRelative(mMutex, kWaitDuration);
                if (ret == TIMED_OUT) {
                    return true;
                } else if (ret != OK) {
                    ALOGE("%s: Timed wait on condition failed: %s (%d)", __FUNCTION__,
                            strerror(-ret), ret);
                    return false;
                }
            }
        }
    }

    auto res = processInputFrame(currentTs, mPendingInputFrames[currentTs]);
    Mutex::Autolock l(mMutex);
    if (res != OK) {
        ALOGE("%s: Failed processing frame with timestamp: %" PRIu64 ": %s (%d)", __FUNCTION__,
                currentTs, strerror(-res), res);
        mPendingInputFrames[currentTs].error = true;
    }

    releaseInputFramesLocked(currentTs);

    return true;
}

bool DepthCompositeStream::isDepthCompositeStream(const sp<Surface> &surface) {
    ANativeWindow *anw = surface.get();
    status_t err;
    int format;
    if ((err = anw->query(anw, NATIVE_WINDOW_FORMAT, &format)) != OK) {
        String8 msg = String8::format("Failed to query Surface format: %s (%d)", strerror(-err),
                err);
        ALOGE("%s: %s", __FUNCTION__, msg.string());
        return false;
    }

    int dataspace;
    if ((err = anw->query(anw, NATIVE_WINDOW_DEFAULT_DATASPACE, &dataspace)) != OK) {
        String8 msg = String8::format("Failed to query Surface dataspace: %s (%d)", strerror(-err),
                err);
        ALOGE("%s: %s", __FUNCTION__, msg.string());
        return false;
    }

    if ((format == HAL_PIXEL_FORMAT_BLOB) && (dataspace == HAL_DATASPACE_DYNAMIC_DEPTH)) {
        return true;
    }

    return false;
}

status_t DepthCompositeStream::createInternalStreams(const std::vector<sp<Surface>>& consumers,
        bool /*hasDeferredConsumer*/, uint32_t width, uint32_t height, int format,
        camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
        std::vector<int> *surfaceIds, int /*streamSetId*/, bool /*isShared*/) {
    if (mSupportedDepthSizes.empty()) {
        ALOGE("%s: This camera device doesn't support any depth map streams!", __FUNCTION__);
        return INVALID_OPERATION;
    }

    size_t depthWidth, depthHeight;
    auto ret = getMatchingDepthSize(width, height, mSupportedDepthSizes, &depthWidth, &depthHeight);
    if (ret != OK) {
        ALOGE("%s: Failed to find an appropriate depth stream size!", __FUNCTION__);
        return ret;
    }

    sp<CameraDeviceBase> device = mDevice.promote();
    if (!device.get()) {
        ALOGE("%s: Invalid camera device!", __FUNCTION__);
        return NO_INIT;
    }

    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    mBlobConsumer = new CpuConsumer(consumer, /*maxLockedBuffers*/1, /*controlledByApp*/ true);
    mBlobConsumer->setFrameAvailableListener(this);
    mBlobConsumer->setName(String8("Camera3-JpegCompositeStream"));
    mBlobSurface = new Surface(producer);

    ret = device->createStream(mBlobSurface, width, height, format, kJpegDataSpace, rotation,
            id, physicalCameraId, surfaceIds);
    if (ret == OK) {
        mBlobStreamId = *id;
        mBlobSurfaceId = (*surfaceIds)[0];
        mOutputSurface = consumers[0];
    } else {
        return ret;
    }

    BufferQueue::createBufferQueue(&producer, &consumer);
    mDepthConsumer = new CpuConsumer(consumer, /*maxLockedBuffers*/ 1, /*controlledByApp*/ true);
    mDepthConsumer->setFrameAvailableListener(this);
    mDepthConsumer->setName(String8("Camera3-DepthCompositeStream"));
    mDepthSurface = new Surface(producer);
    std::vector<int> depthSurfaceId;
    ret = device->createStream(mDepthSurface, depthWidth, depthHeight, kDepthMapPixelFormat,
            kDepthMapDataSpace, rotation, &mDepthStreamId, physicalCameraId, &depthSurfaceId);
    if (ret == OK) {
        mDepthSurfaceId = depthSurfaceId[0];
    } else {
        return ret;
    }

    ret = registerCompositeStreamListener(getStreamId());
    if (ret != OK) {
        ALOGE("%s: Failed to register blob stream listener!", __FUNCTION__);
        return ret;
    }

    ret = registerCompositeStreamListener(mDepthStreamId);
    if (ret != OK) {
        ALOGE("%s: Failed to register depth stream listener!", __FUNCTION__);
        return ret;
    }

    mBlobWidth = width;
    mBlobHeight = height;

    return ret;
}

status_t DepthCompositeStream::configureStream() {
    if (isRunning()) {
        // Processing thread is already running, nothing more to do.
        return NO_ERROR;
    }

    if ((mDepthPhotoLibHandle == nullptr) || (mDepthPhotoProcess == nullptr)) {
        ALOGE("%s: Depth photo library is not present!", __FUNCTION__);
        return NO_INIT;
    }

    if (mOutputSurface.get() == nullptr) {
        ALOGE("%s: No valid output surface set!", __FUNCTION__);
        return NO_INIT;
    }

    auto res = mOutputSurface->connect(NATIVE_WINDOW_API_CAMERA, mProducerListener);
    if (res != OK) {
        ALOGE("%s: Unable to connect to native window for stream %d",
                __FUNCTION__, mBlobStreamId);
        return res;
    }

    if ((res = native_window_set_buffers_format(mOutputSurface.get(), HAL_PIXEL_FORMAT_BLOB))
            != OK) {
        ALOGE("%s: Unable to configure stream buffer format for stream %d", __FUNCTION__,
                mBlobStreamId);
        return res;
    }

    int maxProducerBuffers;
    ANativeWindow *anw = mBlobSurface.get();
    if ((res = anw->query(anw, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &maxProducerBuffers)) != OK) {
        ALOGE("%s: Unable to query consumer undequeued"
                " buffer count for stream %d", __FUNCTION__, mBlobStreamId);
        return res;
    }

    ANativeWindow *anwConsumer = mOutputSurface.get();
    int maxConsumerBuffers;
    if ((res = anwConsumer->query(anwConsumer, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
                    &maxConsumerBuffers)) != OK) {
        ALOGE("%s: Unable to query consumer undequeued"
                " buffer count for stream %d", __FUNCTION__, mBlobStreamId);
        return res;
    }

    if ((res = native_window_set_buffer_count(
                    anwConsumer, maxProducerBuffers + maxConsumerBuffers)) != OK) {
        ALOGE("%s: Unable to set buffer count for stream %d", __FUNCTION__, mBlobStreamId);
        return res;
    }

    run("DepthCompositeStreamProc");

    return NO_ERROR;
}

status_t DepthCompositeStream::deleteInternalStreams() {
    // The 'CameraDeviceClient' parent will delete the blob stream
    requestExit();

    auto ret = join();
    if (ret != OK) {
        ALOGE("%s: Failed to join with the main processing thread: %s (%d)", __FUNCTION__,
                strerror(-ret), ret);
    }

    sp<CameraDeviceBase> device = mDevice.promote();
    if (!device.get()) {
        ALOGE("%s: Invalid camera device!", __FUNCTION__);
        return NO_INIT;
    }

    if (mDepthStreamId >= 0) {
        ret = device->deleteStream(mDepthStreamId);
        mDepthStreamId = -1;
    }

    if (mOutputSurface != nullptr) {
        mOutputSurface->disconnect(NATIVE_WINDOW_API_CAMERA);
        mOutputSurface.clear();
    }

    return ret;
}

void DepthCompositeStream::onFrameAvailable(const BufferItem& item) {
    if (item.mDataSpace == kJpegDataSpace) {
        ALOGV("%s: Jpeg buffer with ts: %" PRIu64 " ms. arrived!",
                __func__, ns2ms(item.mTimestamp));

        Mutex::Autolock l(mMutex);
        if (!mErrorState) {
            mInputJpegBuffers.push_back(item.mTimestamp);
            mInputReadyCondition.signal();
        }
    } else if (item.mDataSpace == kDepthMapDataSpace) {
        ALOGV("%s: Depth buffer with ts: %" PRIu64 " ms. arrived!", __func__,
                ns2ms(item.mTimestamp));

        Mutex::Autolock l(mMutex);
        if (!mErrorState) {
            mInputDepthBuffers.push_back(item.mTimestamp);
            mInputReadyCondition.signal();
        }
    } else {
        ALOGE("%s: Unexpected data space: 0x%x", __FUNCTION__, item.mDataSpace);
    }
}

status_t DepthCompositeStream::insertGbp(SurfaceMap* /*out*/outSurfaceMap,
        Vector<int32_t> * /*out*/outputStreamIds, int32_t* /*out*/currentStreamId) {
    if (outSurfaceMap->find(mDepthStreamId) == outSurfaceMap->end()) {
        (*outSurfaceMap)[mDepthStreamId] = std::vector<size_t>();
        outputStreamIds->push_back(mDepthStreamId);
    }
    (*outSurfaceMap)[mDepthStreamId].push_back(mDepthSurfaceId);

    if (outSurfaceMap->find(mBlobStreamId) == outSurfaceMap->end()) {
        (*outSurfaceMap)[mBlobStreamId] = std::vector<size_t>();
        outputStreamIds->push_back(mBlobStreamId);
    }
    (*outSurfaceMap)[mBlobStreamId].push_back(mBlobSurfaceId);

    if (currentStreamId != nullptr) {
        *currentStreamId = mBlobStreamId;
    }

    return NO_ERROR;
}

void DepthCompositeStream::onResultError(const CaptureResultExtras& resultExtras) {
    // Processing can continue even in case of result errors.
    // At the moment depth composite stream processing relies mainly on static camera
    // characteristics data. The actual result data can be used for the jpeg quality but
    // in case it is absent we can default to maximum.
    eraseResult(resultExtras.frameNumber);
}

bool DepthCompositeStream::onStreamBufferError(const CaptureResultExtras& resultExtras) {
    bool ret = false;
    // Buffer errors concerning internal composite streams should not be directly visible to
    // camera clients. They must only receive a single buffer error with the public composite
    // stream id.
    if ((resultExtras.errorStreamId == mDepthStreamId) ||
            (resultExtras.errorStreamId == mBlobStreamId)) {
        flagAnErrorFrameNumber(resultExtras.frameNumber);
        ret = true;
    }

    return ret;
}

status_t DepthCompositeStream::getMatchingDepthSize(size_t width, size_t height,
        const std::vector<std::tuple<size_t, size_t>>& supporedDepthSizes,
        size_t *depthWidth /*out*/, size_t *depthHeight /*out*/) {
    if ((depthWidth == nullptr) || (depthHeight == nullptr)) {
        return BAD_VALUE;
    }

    float arTol = CameraProviderManager::kDepthARTolerance;
    *depthWidth = *depthHeight = 0;

    float aspectRatio = static_cast<float> (width) / static_cast<float> (height);
    for (const auto& it : supporedDepthSizes) {
        auto currentWidth = std::get<0>(it);
        auto currentHeight = std::get<1>(it);
        if ((currentWidth == width) && (currentHeight == height)) {
            *depthWidth = width;
            *depthHeight = height;
            break;
        } else {
            float currentRatio = static_cast<float> (currentWidth) /
                    static_cast<float> (currentHeight);
            auto currentSize = currentWidth * currentHeight;
            auto oldSize = (*depthWidth) * (*depthHeight);
            if ((fabs(aspectRatio - currentRatio) <= arTol) && (currentSize > oldSize)) {
                *depthWidth = currentWidth;
                *depthHeight = currentHeight;
            }
        }
    }

    return ((*depthWidth > 0) && (*depthHeight > 0)) ? OK : BAD_VALUE;
}

void DepthCompositeStream::getSupportedDepthSizes(const CameraMetadata& ch,
        std::vector<std::tuple<size_t, size_t>>* depthSizes /*out*/) {
    if (depthSizes == nullptr) {
        return;
    }

    auto entry = ch.find(ANDROID_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS);
    if (entry.count > 0) {
        // Depth stream dimensions have four int32_t components
        // (pixelformat, width, height, type)
        size_t entryCount = entry.count / 4;
        depthSizes->reserve(entryCount);
        for (size_t i = 0; i < entry.count; i += 4) {
            if ((entry.data.i32[i] == kDepthMapPixelFormat) &&
                    (entry.data.i32[i+3] ==
                     ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT)) {
                depthSizes->push_back(std::make_tuple(entry.data.i32[i+1],
                            entry.data.i32[i+2]));
            }
        }
    }
}

status_t DepthCompositeStream::getCompositeStreamInfo(const OutputStreamInfo &streamInfo,
            const CameraMetadata& ch, std::vector<OutputStreamInfo>* compositeOutput /*out*/) {
    if (compositeOutput == nullptr) {
        return BAD_VALUE;
    }

    std::vector<std::tuple<size_t, size_t>> depthSizes;
    getSupportedDepthSizes(ch, &depthSizes);
    if (depthSizes.empty()) {
        ALOGE("%s: No depth stream configurations present", __FUNCTION__);
        return BAD_VALUE;
    }

    size_t depthWidth, depthHeight;
    auto ret = getMatchingDepthSize(streamInfo.width, streamInfo.height, depthSizes, &depthWidth,
            &depthHeight);
    if (ret != OK) {
        ALOGE("%s: No matching depth stream size found", __FUNCTION__);
        return ret;
    }

    compositeOutput->clear();
    compositeOutput->insert(compositeOutput->end(), 2, streamInfo);

    // Jpeg/Blob stream info
    (*compositeOutput)[0].dataSpace = kJpegDataSpace;
    (*compositeOutput)[0].consumerUsage = GRALLOC_USAGE_SW_READ_OFTEN;

    // Depth stream info
    (*compositeOutput)[1].width = depthWidth;
    (*compositeOutput)[1].height = depthHeight;
    (*compositeOutput)[1].format = kDepthMapPixelFormat;
    (*compositeOutput)[1].dataSpace = kDepthMapDataSpace;
    (*compositeOutput)[1].consumerUsage = GRALLOC_USAGE_SW_READ_OFTEN;

    return NO_ERROR;
}

}; // namespace camera3
}; // namespace android
