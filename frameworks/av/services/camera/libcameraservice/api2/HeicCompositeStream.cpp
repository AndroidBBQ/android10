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

#define LOG_TAG "Camera3-HeicCompositeStream"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include <linux/memfd.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <android/hardware/camera/device/3.5/types.h>
#include <libyuv.h>
#include <gui/Surface.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <media/ICrypto.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/MediaCodecConstants.h>

#include "common/CameraDeviceBase.h"
#include "utils/ExifUtils.h"
#include "HeicEncoderInfoManager.h"
#include "HeicCompositeStream.h"

using android::hardware::camera::device::V3_5::CameraBlob;
using android::hardware::camera::device::V3_5::CameraBlobId;

namespace android {
namespace camera3 {

HeicCompositeStream::HeicCompositeStream(wp<CameraDeviceBase> device,
        wp<hardware::camera2::ICameraDeviceCallbacks> cb) :
        CompositeStream(device, cb),
        mUseHeic(false),
        mNumOutputTiles(1),
        mOutputWidth(0),
        mOutputHeight(0),
        mMaxHeicBufferSize(0),
        mGridWidth(HeicEncoderInfoManager::kGridWidth),
        mGridHeight(HeicEncoderInfoManager::kGridHeight),
        mGridRows(1),
        mGridCols(1),
        mUseGrid(false),
        mAppSegmentStreamId(-1),
        mAppSegmentSurfaceId(-1),
        mMainImageStreamId(-1),
        mMainImageSurfaceId(-1),
        mYuvBufferAcquired(false),
        mProducerListener(new ProducerListener()),
        mDequeuedOutputBufferCnt(0),
        mLockedAppSegmentBufferCnt(0),
        mCodecOutputCounter(0),
        mGridTimestampUs(0) {
}

HeicCompositeStream::~HeicCompositeStream() {
    // Call deinitCodec in case stream hasn't been deleted yet to avoid any
    // memory/resource leak.
    deinitCodec();

    mInputAppSegmentBuffers.clear();
    mCodecOutputBuffers.clear();

    mAppSegmentStreamId = -1;
    mAppSegmentSurfaceId = -1;
    mAppSegmentConsumer.clear();
    mAppSegmentSurface.clear();

    mMainImageStreamId = -1;
    mMainImageSurfaceId = -1;
    mMainImageConsumer.clear();
    mMainImageSurface.clear();
}

bool HeicCompositeStream::isHeicCompositeStream(const sp<Surface> &surface) {
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

    return ((format == HAL_PIXEL_FORMAT_BLOB) && (dataspace == HAL_DATASPACE_HEIF));
}

status_t HeicCompositeStream::createInternalStreams(const std::vector<sp<Surface>>& consumers,
        bool /*hasDeferredConsumer*/, uint32_t width, uint32_t height, int format,
        camera3_stream_rotation_t rotation, int *id, const String8& physicalCameraId,
        std::vector<int> *surfaceIds, int /*streamSetId*/, bool /*isShared*/) {

    sp<CameraDeviceBase> device = mDevice.promote();
    if (!device.get()) {
        ALOGE("%s: Invalid camera device!", __FUNCTION__);
        return NO_INIT;
    }

    status_t res = initializeCodec(width, height, device);
    if (res != OK) {
        ALOGE("%s: Failed to initialize HEIC/HEVC codec: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return NO_INIT;
    }

    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    mAppSegmentConsumer = new CpuConsumer(consumer, kMaxAcquiredAppSegment);
    mAppSegmentConsumer->setFrameAvailableListener(this);
    mAppSegmentConsumer->setName(String8("Camera3-HeicComposite-AppSegmentStream"));
    mAppSegmentSurface = new Surface(producer);

    mStaticInfo = device->info();

    res = device->createStream(mAppSegmentSurface, mAppSegmentMaxSize, 1, format,
            kAppSegmentDataSpace, rotation, &mAppSegmentStreamId, physicalCameraId, surfaceIds);
    if (res == OK) {
        mAppSegmentSurfaceId = (*surfaceIds)[0];
    } else {
        ALOGE("%s: Failed to create JPEG App segment stream: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    if (!mUseGrid) {
        res = mCodec->createInputSurface(&producer);
        if (res != OK) {
            ALOGE("%s: Failed to create input surface for Heic codec: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
    } else {
        BufferQueue::createBufferQueue(&producer, &consumer);
        mMainImageConsumer = new CpuConsumer(consumer, 1);
        mMainImageConsumer->setFrameAvailableListener(this);
        mMainImageConsumer->setName(String8("Camera3-HeicComposite-HevcInputYUVStream"));
    }
    mMainImageSurface = new Surface(producer);

    res = mCodec->start();
    if (res != OK) {
        ALOGE("%s: Failed to start codec: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    std::vector<int> sourceSurfaceId;
    //Use YUV_888 format if framework tiling is needed.
    int srcStreamFmt = mUseGrid ? HAL_PIXEL_FORMAT_YCbCr_420_888 :
            HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    res = device->createStream(mMainImageSurface, width, height, srcStreamFmt, kHeifDataSpace,
            rotation, id, physicalCameraId, &sourceSurfaceId);
    if (res == OK) {
        mMainImageSurfaceId = sourceSurfaceId[0];
        mMainImageStreamId = *id;
    } else {
        ALOGE("%s: Failed to create main image stream: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    mOutputSurface = consumers[0];
    res = registerCompositeStreamListener(getStreamId());
    if (res != OK) {
        ALOGE("%s: Failed to register HAL main image stream", __FUNCTION__);
        return res;
    }

    initCopyRowFunction(width);
    return res;
}

status_t HeicCompositeStream::deleteInternalStreams() {
    requestExit();
    auto res = join();
    if (res != OK) {
        ALOGE("%s: Failed to join with the main processing thread: %s (%d)", __FUNCTION__,
                strerror(-res), res);
    }

    deinitCodec();

    if (mAppSegmentStreamId >= 0) {
        sp<CameraDeviceBase> device = mDevice.promote();
        if (!device.get()) {
            ALOGE("%s: Invalid camera device!", __FUNCTION__);
            return NO_INIT;
        }

        res = device->deleteStream(mAppSegmentStreamId);
        mAppSegmentStreamId = -1;
    }

    if (mOutputSurface != nullptr) {
        mOutputSurface->disconnect(NATIVE_WINDOW_API_CAMERA);
        mOutputSurface.clear();
    }
    return res;
}

void HeicCompositeStream::onBufferReleased(const BufferInfo& bufferInfo) {
    Mutex::Autolock l(mMutex);

    if (bufferInfo.mError) return;

    mCodecOutputBufferTimestamps.push(bufferInfo.mTimestamp);
    ALOGV("%s: [%" PRId64 "]: Adding codecOutputBufferTimestamp (%zu timestamps in total)",
            __FUNCTION__, bufferInfo.mTimestamp, mCodecOutputBufferTimestamps.size());
}

// We need to get the settings early to handle the case where the codec output
// arrives earlier than result metadata.
void HeicCompositeStream::onBufferRequestForFrameNumber(uint64_t frameNumber, int streamId,
        const CameraMetadata& settings) {
    ATRACE_ASYNC_BEGIN("HEIC capture", frameNumber);

    Mutex::Autolock l(mMutex);
    if (mErrorState || (streamId != getStreamId())) {
        return;
    }

    mPendingCaptureResults.emplace(frameNumber, CameraMetadata());

    camera_metadata_ro_entry entry;

    int32_t orientation = 0;
    entry = settings.find(ANDROID_JPEG_ORIENTATION);
    if (entry.count == 1) {
        orientation = entry.data.i32[0];
    }

    int32_t quality = kDefaultJpegQuality;
    entry = settings.find(ANDROID_JPEG_QUALITY);
    if (entry.count == 1) {
        quality = entry.data.i32[0];
    }

    mSettingsByFrameNumber[frameNumber] = std::make_pair(orientation, quality);
}

void HeicCompositeStream::onFrameAvailable(const BufferItem& item) {
    if (item.mDataSpace == static_cast<android_dataspace>(kAppSegmentDataSpace)) {
        ALOGV("%s: JPEG APP segments buffer with ts: %" PRIu64 " ms. arrived!",
                __func__, ns2ms(item.mTimestamp));

        Mutex::Autolock l(mMutex);
        if (!mErrorState) {
            mInputAppSegmentBuffers.push_back(item.mTimestamp);
            mInputReadyCondition.signal();
        }
    } else if (item.mDataSpace == kHeifDataSpace) {
        ALOGV("%s: YUV_888 buffer with ts: %" PRIu64 " ms. arrived!",
                __func__, ns2ms(item.mTimestamp));

        Mutex::Autolock l(mMutex);
        if (!mUseGrid) {
            ALOGE("%s: YUV_888 internal stream is only supported for HEVC tiling",
                    __FUNCTION__);
            return;
        }
        if (!mErrorState) {
            mInputYuvBuffers.push_back(item.mTimestamp);
            mInputReadyCondition.signal();
        }
    } else {
        ALOGE("%s: Unexpected data space: 0x%x", __FUNCTION__, item.mDataSpace);
    }
}

status_t HeicCompositeStream::getCompositeStreamInfo(const OutputStreamInfo &streamInfo,
            const CameraMetadata& ch, std::vector<OutputStreamInfo>* compositeOutput /*out*/) {
    if (compositeOutput == nullptr) {
        return BAD_VALUE;
    }

    compositeOutput->clear();

    bool useGrid, useHeic;
    bool isSizeSupported = isSizeSupportedByHeifEncoder(
            streamInfo.width, streamInfo.height, &useHeic, &useGrid, nullptr);
    if (!isSizeSupported) {
        // Size is not supported by either encoder.
        return OK;
    }

    compositeOutput->insert(compositeOutput->end(), 2, streamInfo);

    // JPEG APPS segments Blob stream info
    (*compositeOutput)[0].width = calcAppSegmentMaxSize(ch);
    (*compositeOutput)[0].height = 1;
    (*compositeOutput)[0].format = HAL_PIXEL_FORMAT_BLOB;
    (*compositeOutput)[0].dataSpace = kAppSegmentDataSpace;
    (*compositeOutput)[0].consumerUsage = GRALLOC_USAGE_SW_READ_OFTEN;

    // YUV/IMPLEMENTATION_DEFINED stream info
    (*compositeOutput)[1].width = streamInfo.width;
    (*compositeOutput)[1].height = streamInfo.height;
    (*compositeOutput)[1].format = useGrid ? HAL_PIXEL_FORMAT_YCbCr_420_888 :
            HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    (*compositeOutput)[1].dataSpace = kHeifDataSpace;
    (*compositeOutput)[1].consumerUsage = useHeic ? GRALLOC_USAGE_HW_IMAGE_ENCODER :
            useGrid ? GRALLOC_USAGE_SW_READ_OFTEN : GRALLOC_USAGE_HW_VIDEO_ENCODER;

    return NO_ERROR;
}

bool HeicCompositeStream::isSizeSupportedByHeifEncoder(int32_t width, int32_t height,
        bool* useHeic, bool* useGrid, int64_t* stall, AString* hevcName) {
    static HeicEncoderInfoManager& heicManager = HeicEncoderInfoManager::getInstance();
    return heicManager.isSizeSupported(width, height, useHeic, useGrid, stall, hevcName);
}

bool HeicCompositeStream::isInMemoryTempFileSupported() {
    int memfd = syscall(__NR_memfd_create, "HEIF-try-memfd", MFD_CLOEXEC);
    if (memfd == -1) {
        if (errno != ENOSYS) {
            ALOGE("%s: Failed to create tmpfs file. errno %d", __FUNCTION__, errno);
        }
        return false;
    }
    close(memfd);
    return true;
}

void HeicCompositeStream::onHeicOutputFrameAvailable(
        const CodecOutputBufferInfo& outputBufferInfo) {
    Mutex::Autolock l(mMutex);

    ALOGV("%s: index %d, offset %d, size %d, time %" PRId64 ", flags 0x%x",
            __FUNCTION__, outputBufferInfo.index, outputBufferInfo.offset,
            outputBufferInfo.size, outputBufferInfo.timeUs, outputBufferInfo.flags);

    if (!mErrorState) {
        if ((outputBufferInfo.size > 0) &&
                ((outputBufferInfo.flags & MediaCodec::BUFFER_FLAG_CODECCONFIG) == 0)) {
            mCodecOutputBuffers.push_back(outputBufferInfo);
            mInputReadyCondition.signal();
        } else {
            ALOGV("%s: Releasing output buffer: size %d flags: 0x%x ", __FUNCTION__,
                outputBufferInfo.size, outputBufferInfo.flags);
            mCodec->releaseOutputBuffer(outputBufferInfo.index);
        }
    } else {
        mCodec->releaseOutputBuffer(outputBufferInfo.index);
    }
}

void HeicCompositeStream::onHeicInputFrameAvailable(int32_t index) {
    Mutex::Autolock l(mMutex);

    if (!mUseGrid) {
        ALOGE("%s: Codec YUV input mode must only be used for Hevc tiling mode", __FUNCTION__);
        return;
    }

    mCodecInputBuffers.push_back(index);
    mInputReadyCondition.signal();
}

void HeicCompositeStream::onHeicFormatChanged(sp<AMessage>& newFormat) {
    if (newFormat == nullptr) {
        ALOGE("%s: newFormat must not be null!", __FUNCTION__);
        return;
    }

    Mutex::Autolock l(mMutex);

    AString mime;
    AString mimeHeic(MIMETYPE_IMAGE_ANDROID_HEIC);
    newFormat->findString(KEY_MIME, &mime);
    if (mime != mimeHeic) {
        // For HEVC codec, below keys need to be filled out or overwritten so that the
        // muxer can handle them as HEIC output image.
        newFormat->setString(KEY_MIME, mimeHeic);
        newFormat->setInt32(KEY_WIDTH, mOutputWidth);
        newFormat->setInt32(KEY_HEIGHT, mOutputHeight);
        if (mUseGrid) {
            newFormat->setInt32(KEY_TILE_WIDTH, mGridWidth);
            newFormat->setInt32(KEY_TILE_HEIGHT, mGridHeight);
            newFormat->setInt32(KEY_GRID_ROWS, mGridRows);
            newFormat->setInt32(KEY_GRID_COLUMNS, mGridCols);
        }
    }
    newFormat->setInt32(KEY_IS_DEFAULT, 1 /*isPrimary*/);

    int32_t gridRows, gridCols;
    if (newFormat->findInt32(KEY_GRID_ROWS, &gridRows) &&
            newFormat->findInt32(KEY_GRID_COLUMNS, &gridCols)) {
        mNumOutputTiles = gridRows * gridCols;
    } else {
        mNumOutputTiles = 1;
    }

    mFormat = newFormat;

    ALOGV("%s: mNumOutputTiles is %zu", __FUNCTION__, mNumOutputTiles);
    mInputReadyCondition.signal();
}

void HeicCompositeStream::onHeicCodecError() {
    Mutex::Autolock l(mMutex);
    mErrorState = true;
}

status_t HeicCompositeStream::configureStream() {
    if (isRunning()) {
        // Processing thread is already running, nothing more to do.
        return NO_ERROR;
    }

    if (mOutputSurface.get() == nullptr) {
        ALOGE("%s: No valid output surface set!", __FUNCTION__);
        return NO_INIT;
    }

    auto res = mOutputSurface->connect(NATIVE_WINDOW_API_CAMERA, mProducerListener);
    if (res != OK) {
        ALOGE("%s: Unable to connect to native window for stream %d",
                __FUNCTION__, mMainImageStreamId);
        return res;
    }

    if ((res = native_window_set_buffers_format(mOutputSurface.get(), HAL_PIXEL_FORMAT_BLOB))
            != OK) {
        ALOGE("%s: Unable to configure stream buffer format for stream %d", __FUNCTION__,
                mMainImageStreamId);
        return res;
    }

    ANativeWindow *anwConsumer = mOutputSurface.get();
    int maxConsumerBuffers;
    if ((res = anwConsumer->query(anwConsumer, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
                    &maxConsumerBuffers)) != OK) {
        ALOGE("%s: Unable to query consumer undequeued"
                " buffer count for stream %d", __FUNCTION__, mMainImageStreamId);
        return res;
    }

    // Cannot use SourceSurface buffer count since it could be codec's 512*512 tile
    // buffer count.
    if ((res = native_window_set_buffer_count(
                    anwConsumer, kMaxOutputSurfaceProducerCount + maxConsumerBuffers)) != OK) {
        ALOGE("%s: Unable to set buffer count for stream %d", __FUNCTION__, mMainImageStreamId);
        return res;
    }

    if ((res = native_window_set_buffers_dimensions(anwConsumer, mMaxHeicBufferSize, 1)) != OK) {
        ALOGE("%s: Unable to set buffer dimension %zu x 1 for stream %d: %s (%d)",
                __FUNCTION__, mMaxHeicBufferSize, mMainImageStreamId, strerror(-res), res);
        return res;
    }

    run("HeicCompositeStreamProc");

    return NO_ERROR;
}

status_t HeicCompositeStream::insertGbp(SurfaceMap* /*out*/outSurfaceMap,
        Vector<int32_t>* /*out*/outputStreamIds, int32_t* /*out*/currentStreamId) {
    if (outSurfaceMap->find(mAppSegmentStreamId) == outSurfaceMap->end()) {
        (*outSurfaceMap)[mAppSegmentStreamId] = std::vector<size_t>();
        outputStreamIds->push_back(mAppSegmentStreamId);
    }
    (*outSurfaceMap)[mAppSegmentStreamId].push_back(mAppSegmentSurfaceId);

    if (outSurfaceMap->find(mMainImageStreamId) == outSurfaceMap->end()) {
        (*outSurfaceMap)[mMainImageStreamId] = std::vector<size_t>();
        outputStreamIds->push_back(mMainImageStreamId);
    }
    (*outSurfaceMap)[mMainImageStreamId].push_back(mMainImageSurfaceId);

    if (currentStreamId != nullptr) {
        *currentStreamId = mMainImageStreamId;
    }

    return NO_ERROR;
}

void HeicCompositeStream::onShutter(const CaptureResultExtras& resultExtras, nsecs_t timestamp) {
    Mutex::Autolock l(mMutex);
    if (mErrorState) {
        return;
    }

    if (mSettingsByFrameNumber.find(resultExtras.frameNumber) != mSettingsByFrameNumber.end()) {
        ALOGV("%s: [%" PRId64 "]: frameNumber %" PRId64, __FUNCTION__,
                timestamp, resultExtras.frameNumber);
        mFrameNumberMap.emplace(resultExtras.frameNumber, timestamp);
        mSettingsByTimestamp[timestamp] = mSettingsByFrameNumber[resultExtras.frameNumber];
        mSettingsByFrameNumber.erase(resultExtras.frameNumber);
        mInputReadyCondition.signal();
    }
}

void HeicCompositeStream::compilePendingInputLocked() {
    while (!mSettingsByTimestamp.empty()) {
        auto it = mSettingsByTimestamp.begin();
        mPendingInputFrames[it->first].orientation = it->second.first;
        mPendingInputFrames[it->first].quality = it->second.second;
        mSettingsByTimestamp.erase(it);
    }

    while (!mInputAppSegmentBuffers.empty()) {
        CpuConsumer::LockedBuffer imgBuffer;
        auto it = mInputAppSegmentBuffers.begin();
        auto res = mAppSegmentConsumer->lockNextBuffer(&imgBuffer);
        if (res == NOT_ENOUGH_DATA) {
            // Can not lock any more buffers.
            break;
        } else if ((res != OK) || (*it != imgBuffer.timestamp)) {
            if (res != OK) {
                ALOGE("%s: Error locking JPEG_APP_SEGMENTS image buffer: %s (%d)", __FUNCTION__,
                        strerror(-res), res);
            } else {
                ALOGE("%s: Expecting JPEG_APP_SEGMENTS buffer with time stamp: %" PRId64
                        " received buffer with time stamp: %" PRId64, __FUNCTION__,
                        *it, imgBuffer.timestamp);
                mAppSegmentConsumer->unlockBuffer(imgBuffer);
            }
            mPendingInputFrames[*it].error = true;
            mInputAppSegmentBuffers.erase(it);
            continue;
        }

        if ((mPendingInputFrames.find(imgBuffer.timestamp) != mPendingInputFrames.end()) &&
                (mPendingInputFrames[imgBuffer.timestamp].error)) {
            mAppSegmentConsumer->unlockBuffer(imgBuffer);
        } else {
            mPendingInputFrames[imgBuffer.timestamp].appSegmentBuffer = imgBuffer;
            mLockedAppSegmentBufferCnt++;
        }
        mInputAppSegmentBuffers.erase(it);
    }

    while (!mInputYuvBuffers.empty() && !mYuvBufferAcquired) {
        CpuConsumer::LockedBuffer imgBuffer;
        auto it = mInputYuvBuffers.begin();
        auto res = mMainImageConsumer->lockNextBuffer(&imgBuffer);
        if (res == NOT_ENOUGH_DATA) {
            // Can not lock any more buffers.
            break;
        } else if (res != OK) {
            ALOGE("%s: Error locking YUV_888 image buffer: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            mPendingInputFrames[*it].error = true;
            mInputYuvBuffers.erase(it);
            continue;
        } else if (*it != imgBuffer.timestamp) {
            ALOGW("%s: Expecting YUV_888 buffer with time stamp: %" PRId64 " received buffer with "
                    "time stamp: %" PRId64, __FUNCTION__, *it, imgBuffer.timestamp);
            mPendingInputFrames[*it].error = true;
            mInputYuvBuffers.erase(it);
            continue;
        }

        if ((mPendingInputFrames.find(imgBuffer.timestamp) != mPendingInputFrames.end()) &&
                (mPendingInputFrames[imgBuffer.timestamp].error)) {
            mMainImageConsumer->unlockBuffer(imgBuffer);
        } else {
            mPendingInputFrames[imgBuffer.timestamp].yuvBuffer = imgBuffer;
            mYuvBufferAcquired = true;
        }
        mInputYuvBuffers.erase(it);
    }

    while (!mCodecOutputBuffers.empty()) {
        auto it = mCodecOutputBuffers.begin();
        // Bitstream buffer timestamp doesn't necessarily directly correlate with input
        // buffer timestamp. Assume encoder input to output is FIFO, use a queue
        // to look up timestamp.
        int64_t bufferTime = -1;
        if (mCodecOutputBufferTimestamps.empty()) {
            ALOGV("%s: Failed to find buffer timestamp for codec output buffer!", __FUNCTION__);
            break;
        } else {
            // Direct mapping between camera timestamp (in ns) and codec timestamp (in us).
            bufferTime = mCodecOutputBufferTimestamps.front();
            mCodecOutputCounter++;
            if (mCodecOutputCounter == mNumOutputTiles) {
                mCodecOutputBufferTimestamps.pop();
                mCodecOutputCounter = 0;
            }

            mPendingInputFrames[bufferTime].codecOutputBuffers.push_back(*it);
            ALOGV("%s: [%" PRId64 "]: Pushing codecOutputBuffers (time %" PRId64 " us)",
                    __FUNCTION__, bufferTime, it->timeUs);
        }
        mCodecOutputBuffers.erase(it);
    }

    while (!mFrameNumberMap.empty()) {
        auto it = mFrameNumberMap.begin();
        mPendingInputFrames[it->second].frameNumber = it->first;
        ALOGV("%s: [%" PRId64 "]: frameNumber is %" PRId64, __FUNCTION__, it->second, it->first);
        mFrameNumberMap.erase(it);
    }

    while (!mCaptureResults.empty()) {
        auto it = mCaptureResults.begin();
        // Negative timestamp indicates that something went wrong during the capture result
        // collection process.
        if (it->first >= 0) {
            if (mPendingInputFrames[it->first].frameNumber == std::get<0>(it->second)) {
                mPendingInputFrames[it->first].result =
                        std::make_unique<CameraMetadata>(std::get<1>(it->second));
            } else {
                ALOGE("%s: Capture result frameNumber/timestamp mapping changed between "
                        "shutter and capture result!", __FUNCTION__);
            }
        }
        mCaptureResults.erase(it);
    }

    // mErrorFrameNumbers stores frame number of dropped buffers.
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

    // Distribute codec input buffers to be filled out from YUV output
    for (auto it = mPendingInputFrames.begin();
            it != mPendingInputFrames.end() && mCodecInputBuffers.size() > 0; it++) {
        InputFrame& inputFrame(it->second);
        if (inputFrame.codecInputCounter < mGridRows * mGridCols) {
            // Available input tiles that are required for the current input
            // image.
            size_t newInputTiles = std::min(mCodecInputBuffers.size(),
                    mGridRows * mGridCols - inputFrame.codecInputCounter);
            for (size_t i = 0; i < newInputTiles; i++) {
                CodecInputBufferInfo inputInfo =
                        { mCodecInputBuffers[0], mGridTimestampUs++, inputFrame.codecInputCounter };
                inputFrame.codecInputBuffers.push_back(inputInfo);

                mCodecInputBuffers.erase(mCodecInputBuffers.begin());
                inputFrame.codecInputCounter++;
            }
            break;
        }
    }
}

bool HeicCompositeStream::getNextReadyInputLocked(int64_t *currentTs /*out*/) {
    if (currentTs == nullptr) {
        return false;
    }

    bool newInputAvailable = false;
    for (auto& it : mPendingInputFrames) {
        // New input is considered to be available only if:
        // 1. input buffers are ready, or
        // 2. App segment and muxer is created, or
        // 3. A codec output tile is ready, and an output buffer is available.
        // This makes sure that muxer gets created only when an output tile is
        // generated, because right now we only handle 1 HEIC output buffer at a
        // time (max dequeued buffer count is 1).
        bool appSegmentReady = (it.second.appSegmentBuffer.data != nullptr) &&
                !it.second.appSegmentWritten && it.second.result != nullptr &&
                it.second.muxer != nullptr;
        bool codecOutputReady = !it.second.codecOutputBuffers.empty();
        bool codecInputReady = (it.second.yuvBuffer.data != nullptr) &&
                (!it.second.codecInputBuffers.empty());
        bool hasOutputBuffer = it.second.muxer != nullptr ||
                (mDequeuedOutputBufferCnt < kMaxOutputSurfaceProducerCount);
        if ((!it.second.error) &&
                (it.first < *currentTs) &&
                (appSegmentReady || (codecOutputReady && hasOutputBuffer) || codecInputReady)) {
            *currentTs = it.first;
            if (it.second.format == nullptr && mFormat != nullptr) {
                it.second.format = mFormat->dup();
            }
            newInputAvailable = true;
            break;
        }
    }

    return newInputAvailable;
}

int64_t HeicCompositeStream::getNextFailingInputLocked(int64_t *currentTs /*out*/) {
    int64_t res = -1;
    if (currentTs == nullptr) {
        return res;
    }

    for (const auto& it : mPendingInputFrames) {
        if (it.second.error && !it.second.errorNotified && (it.first < *currentTs)) {
            *currentTs = it.first;
            res = it.second.frameNumber;
            break;
        }
    }

    return res;
}

status_t HeicCompositeStream::processInputFrame(nsecs_t timestamp,
        InputFrame &inputFrame) {
    ATRACE_CALL();
    status_t res = OK;

    bool appSegmentReady = inputFrame.appSegmentBuffer.data != nullptr &&
            !inputFrame.appSegmentWritten && inputFrame.result != nullptr &&
            inputFrame.muxer != nullptr;
    bool codecOutputReady = inputFrame.codecOutputBuffers.size() > 0;
    bool codecInputReady = inputFrame.yuvBuffer.data != nullptr &&
            !inputFrame.codecInputBuffers.empty();
    bool hasOutputBuffer = inputFrame.muxer != nullptr ||
            (mDequeuedOutputBufferCnt < kMaxOutputSurfaceProducerCount);

    ALOGV("%s: [%" PRId64 "]: appSegmentReady %d, codecOutputReady %d, codecInputReady %d,"
            " dequeuedOutputBuffer %d", __FUNCTION__, timestamp, appSegmentReady,
            codecOutputReady, codecInputReady, mDequeuedOutputBufferCnt);

    // Handle inputs for Hevc tiling
    if (codecInputReady) {
        res = processCodecInputFrame(inputFrame);
        if (res != OK) {
            ALOGE("%s: Failed to process codec input frame: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }
    }

    if (!(codecOutputReady && hasOutputBuffer) && !appSegmentReady) {
        return OK;
    }

    // Initialize and start muxer if not yet done so. In this case,
    // codecOutputReady must be true. Otherwise, appSegmentReady is guaranteed
    // to be false, and the function must have returned early.
    if (inputFrame.muxer == nullptr) {
        res = startMuxerForInputFrame(timestamp, inputFrame);
        if (res != OK) {
            ALOGE("%s: Failed to create and start muxer: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }
    }

    // Write JPEG APP segments data to the muxer.
    if (appSegmentReady) {
        res = processAppSegment(timestamp, inputFrame);
        if (res != OK) {
            ALOGE("%s: Failed to process JPEG APP segments: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }
    }

    // Write media codec bitstream buffers to muxer.
    while (!inputFrame.codecOutputBuffers.empty()) {
        res = processOneCodecOutputFrame(timestamp, inputFrame);
        if (res != OK) {
            ALOGE("%s: Failed to process codec output frame: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }
    }

    if (inputFrame.pendingOutputTiles == 0) {
        if (inputFrame.appSegmentWritten) {
            res = processCompletedInputFrame(timestamp, inputFrame);
            if (res != OK) {
                ALOGE("%s: Failed to process completed input frame: %s (%d)", __FUNCTION__,
                        strerror(-res), res);
                return res;
            }
        } else if (mLockedAppSegmentBufferCnt == kMaxAcquiredAppSegment) {
            ALOGE("%s: Out-of-order app segment buffers reaches limit %u", __FUNCTION__,
                    kMaxAcquiredAppSegment);
            return INVALID_OPERATION;
        }
    }

    return res;
}

status_t HeicCompositeStream::startMuxerForInputFrame(nsecs_t timestamp, InputFrame &inputFrame) {
    sp<ANativeWindow> outputANW = mOutputSurface;

    auto res = outputANW->dequeueBuffer(mOutputSurface.get(), &inputFrame.anb, &inputFrame.fenceFd);
    if (res != OK) {
        ALOGE("%s: Error retrieving output buffer: %s (%d)", __FUNCTION__, strerror(-res),
                res);
        return res;
    }
    mDequeuedOutputBufferCnt++;

    // Combine current thread id, stream id and timestamp to uniquely identify image.
    std::ostringstream tempOutputFile;
    tempOutputFile << "HEIF-" << pthread_self() << "-"
            << getStreamId() << "-" << timestamp;
    inputFrame.fileFd = syscall(__NR_memfd_create, tempOutputFile.str().c_str(), MFD_CLOEXEC);
    if (inputFrame.fileFd < 0) {
        ALOGE("%s: Failed to create file %s. Error no is %d", __FUNCTION__,
                tempOutputFile.str().c_str(), errno);
        return NO_INIT;
    }
    inputFrame.muxer = new MediaMuxer(inputFrame.fileFd, MediaMuxer::OUTPUT_FORMAT_HEIF);
    if (inputFrame.muxer == nullptr) {
        ALOGE("%s: Failed to create MediaMuxer for file fd %d",
                __FUNCTION__, inputFrame.fileFd);
        return NO_INIT;
    }

    res = inputFrame.muxer->setOrientationHint(inputFrame.orientation);
    if (res != OK) {
        ALOGE("%s: Failed to setOrientationHint: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }
    // Set encoder quality
    {
        sp<AMessage> qualityParams = new AMessage;
        qualityParams->setInt32(PARAMETER_KEY_VIDEO_BITRATE, inputFrame.quality);
        res = mCodec->setParameters(qualityParams);
        if (res != OK) {
            ALOGE("%s: Failed to set codec quality: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
    }

    ssize_t trackId = inputFrame.muxer->addTrack(inputFrame.format);
    if (trackId < 0) {
        ALOGE("%s: Failed to addTrack to the muxer: %zd", __FUNCTION__, trackId);
        return NO_INIT;
    }

    inputFrame.trackIndex = trackId;
    inputFrame.pendingOutputTiles = mNumOutputTiles;

    res = inputFrame.muxer->start();
    if (res != OK) {
        ALOGE("%s: Failed to start MediaMuxer: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    ALOGV("%s: [%" PRId64 "]: Muxer started for inputFrame", __FUNCTION__,
            timestamp);
    return OK;
}

status_t HeicCompositeStream::processAppSegment(nsecs_t timestamp, InputFrame &inputFrame) {
    size_t app1Size = 0;
    auto appSegmentSize = findAppSegmentsSize(inputFrame.appSegmentBuffer.data,
            inputFrame.appSegmentBuffer.width * inputFrame.appSegmentBuffer.height,
            &app1Size);
    if (appSegmentSize == 0) {
        ALOGE("%s: Failed to find JPEG APP segment size", __FUNCTION__);
        return NO_INIT;
    }

    std::unique_ptr<ExifUtils> exifUtils(ExifUtils::create());
    auto exifRes = exifUtils->initialize(inputFrame.appSegmentBuffer.data, app1Size);
    if (!exifRes) {
        ALOGE("%s: Failed to initialize ExifUtils object!", __FUNCTION__);
        return BAD_VALUE;
    }
    exifRes = exifUtils->setFromMetadata(*inputFrame.result, mStaticInfo,
            mOutputWidth, mOutputHeight);
    if (!exifRes) {
        ALOGE("%s: Failed to set Exif tags using metadata and main image sizes", __FUNCTION__);
        return BAD_VALUE;
    }
    exifRes = exifUtils->setOrientation(inputFrame.orientation);
    if (!exifRes) {
        ALOGE("%s: ExifUtils failed to set orientation", __FUNCTION__);
        return BAD_VALUE;
    }
    exifRes = exifUtils->generateApp1();
    if (!exifRes) {
        ALOGE("%s: ExifUtils failed to generate APP1 segment", __FUNCTION__);
        return BAD_VALUE;
    }

    unsigned int newApp1Length = exifUtils->getApp1Length();
    const uint8_t *newApp1Segment = exifUtils->getApp1Buffer();

    //Assemble the APP1 marker buffer required by MediaCodec
    uint8_t kExifApp1Marker[] = {'E', 'x', 'i', 'f', 0xFF, 0xE1, 0x00, 0x00};
    kExifApp1Marker[6] = static_cast<uint8_t>(newApp1Length >> 8);
    kExifApp1Marker[7] = static_cast<uint8_t>(newApp1Length & 0xFF);
    size_t appSegmentBufferSize = sizeof(kExifApp1Marker) +
            appSegmentSize - app1Size + newApp1Length;
    uint8_t* appSegmentBuffer = new uint8_t[appSegmentBufferSize];
    memcpy(appSegmentBuffer, kExifApp1Marker, sizeof(kExifApp1Marker));
    memcpy(appSegmentBuffer + sizeof(kExifApp1Marker), newApp1Segment, newApp1Length);
    if (appSegmentSize - app1Size > 0) {
        memcpy(appSegmentBuffer + sizeof(kExifApp1Marker) + newApp1Length,
                inputFrame.appSegmentBuffer.data + app1Size, appSegmentSize - app1Size);
    }

    sp<ABuffer> aBuffer = new ABuffer(appSegmentBuffer, appSegmentBufferSize);
    auto res = inputFrame.muxer->writeSampleData(aBuffer, inputFrame.trackIndex,
            timestamp, MediaCodec::BUFFER_FLAG_MUXER_DATA);
    delete[] appSegmentBuffer;

    if (res != OK) {
        ALOGE("%s: Failed to write JPEG APP segments to muxer: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    ALOGV("%s: [%" PRId64 "]: appSegmentSize is %zu, width %d, height %d, app1Size %zu",
          __FUNCTION__, timestamp, appSegmentSize, inputFrame.appSegmentBuffer.width,
          inputFrame.appSegmentBuffer.height, app1Size);

    inputFrame.appSegmentWritten = true;
    // Release the buffer now so any pending input app segments can be processed
    mAppSegmentConsumer->unlockBuffer(inputFrame.appSegmentBuffer);
    inputFrame.appSegmentBuffer.data = nullptr;
    mLockedAppSegmentBufferCnt--;

    return OK;
}

status_t HeicCompositeStream::processCodecInputFrame(InputFrame &inputFrame) {
    for (auto& inputBuffer : inputFrame.codecInputBuffers) {
        sp<MediaCodecBuffer> buffer;
        auto res = mCodec->getInputBuffer(inputBuffer.index, &buffer);
        if (res != OK) {
            ALOGE("%s: Error getting codec input buffer: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }

        // Copy one tile from source to destination.
        size_t tileX = inputBuffer.tileIndex % mGridCols;
        size_t tileY = inputBuffer.tileIndex / mGridCols;
        size_t top = mGridHeight * tileY;
        size_t left = mGridWidth * tileX;
        size_t width = (tileX == static_cast<size_t>(mGridCols) - 1) ?
                mOutputWidth - tileX * mGridWidth : mGridWidth;
        size_t height = (tileY == static_cast<size_t>(mGridRows) - 1) ?
                mOutputHeight - tileY * mGridHeight : mGridHeight;
        ALOGV("%s: inputBuffer tileIndex [%zu, %zu], top %zu, left %zu, width %zu, height %zu,"
                " timeUs %" PRId64, __FUNCTION__, tileX, tileY, top, left, width, height,
                inputBuffer.timeUs);

        res = copyOneYuvTile(buffer, inputFrame.yuvBuffer, top, left, width, height);
        if (res != OK) {
            ALOGE("%s: Failed to copy YUV tile %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return res;
        }

        res = mCodec->queueInputBuffer(inputBuffer.index, 0, buffer->capacity(),
                inputBuffer.timeUs, 0, nullptr /*errorDetailMsg*/);
        if (res != OK) {
            ALOGE("%s: Failed to queueInputBuffer to Codec: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            return res;
        }
    }

    inputFrame.codecInputBuffers.clear();
    return OK;
}

status_t HeicCompositeStream::processOneCodecOutputFrame(nsecs_t timestamp,
        InputFrame &inputFrame) {
    auto it = inputFrame.codecOutputBuffers.begin();
    sp<MediaCodecBuffer> buffer;
    status_t res = mCodec->getOutputBuffer(it->index, &buffer);
    if (res != OK) {
        ALOGE("%s: Error getting Heic codec output buffer at index %d: %s (%d)",
                __FUNCTION__, it->index, strerror(-res), res);
        return res;
    }
    if (buffer == nullptr) {
        ALOGE("%s: Invalid Heic codec output buffer at index %d",
                __FUNCTION__, it->index);
        return BAD_VALUE;
    }

    sp<ABuffer> aBuffer = new ABuffer(buffer->data(), buffer->size());
    res = inputFrame.muxer->writeSampleData(
            aBuffer, inputFrame.trackIndex, timestamp, 0 /*flags*/);
    if (res != OK) {
        ALOGE("%s: Failed to write buffer index %d to muxer: %s (%d)",
                __FUNCTION__, it->index, strerror(-res), res);
        return res;
    }

    mCodec->releaseOutputBuffer(it->index);
    if (inputFrame.pendingOutputTiles == 0) {
        ALOGW("%s: Codec generated more tiles than expected!", __FUNCTION__);
    } else {
        inputFrame.pendingOutputTiles--;
    }

    inputFrame.codecOutputBuffers.erase(inputFrame.codecOutputBuffers.begin());

    ALOGV("%s: [%" PRId64 "]: Output buffer index %d",
        __FUNCTION__, timestamp, it->index);
    return OK;
}

status_t HeicCompositeStream::processCompletedInputFrame(nsecs_t timestamp,
        InputFrame &inputFrame) {
    sp<ANativeWindow> outputANW = mOutputSurface;
    inputFrame.muxer->stop();

    // Copy the content of the file to memory.
    sp<GraphicBuffer> gb = GraphicBuffer::from(inputFrame.anb);
    void* dstBuffer;
    auto res = gb->lockAsync(GRALLOC_USAGE_SW_WRITE_OFTEN, &dstBuffer, inputFrame.fenceFd);
    if (res != OK) {
        ALOGE("%s: Error trying to lock output buffer fence: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    off_t fSize = lseek(inputFrame.fileFd, 0, SEEK_END);
    if (static_cast<size_t>(fSize) > mMaxHeicBufferSize - sizeof(CameraBlob)) {
        ALOGE("%s: Error: MediaMuxer output size %ld is larger than buffer sizer %zu",
                __FUNCTION__, fSize, mMaxHeicBufferSize - sizeof(CameraBlob));
        return BAD_VALUE;
    }

    lseek(inputFrame.fileFd, 0, SEEK_SET);
    ssize_t bytesRead = read(inputFrame.fileFd, dstBuffer, fSize);
    if (bytesRead < fSize) {
        ALOGE("%s: Only %zd of %ld bytes read", __FUNCTION__, bytesRead, fSize);
        return BAD_VALUE;
    }

    close(inputFrame.fileFd);
    inputFrame.fileFd = -1;

    // Fill in HEIC header
    uint8_t *header = static_cast<uint8_t*>(dstBuffer) + mMaxHeicBufferSize - sizeof(CameraBlob);
    struct CameraBlob *blobHeader = (struct CameraBlob *)header;
    // Must be in sync with CAMERA3_HEIC_BLOB_ID in android_media_Utils.cpp
    blobHeader->blobId = static_cast<CameraBlobId>(0x00FE);
    blobHeader->blobSize = fSize;

    res = native_window_set_buffers_timestamp(mOutputSurface.get(), timestamp);
    if (res != OK) {
        ALOGE("%s: Stream %d: Error setting timestamp: %s (%d)",
               __FUNCTION__, getStreamId(), strerror(-res), res);
        return res;
    }

    res = outputANW->queueBuffer(mOutputSurface.get(), inputFrame.anb, /*fence*/ -1);
    if (res != OK) {
        ALOGE("%s: Failed to queueBuffer to Heic stream: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }
    inputFrame.anb = nullptr;
    mDequeuedOutputBufferCnt--;

    ALOGV("%s: [%" PRId64 "]", __FUNCTION__, timestamp);
    ATRACE_ASYNC_END("HEIC capture", inputFrame.frameNumber);
    return OK;
}


void HeicCompositeStream::releaseInputFrameLocked(InputFrame *inputFrame /*out*/) {
    if (inputFrame == nullptr) {
        return;
    }

    if (inputFrame->appSegmentBuffer.data != nullptr) {
        mAppSegmentConsumer->unlockBuffer(inputFrame->appSegmentBuffer);
        inputFrame->appSegmentBuffer.data = nullptr;
    }

    while (!inputFrame->codecOutputBuffers.empty()) {
        auto it = inputFrame->codecOutputBuffers.begin();
        ALOGV("%s: releaseOutputBuffer index %d", __FUNCTION__, it->index);
        mCodec->releaseOutputBuffer(it->index);
        inputFrame->codecOutputBuffers.erase(it);
    }

    if (inputFrame->yuvBuffer.data != nullptr) {
        mMainImageConsumer->unlockBuffer(inputFrame->yuvBuffer);
        inputFrame->yuvBuffer.data = nullptr;
        mYuvBufferAcquired = false;
    }

    while (!inputFrame->codecInputBuffers.empty()) {
        auto it = inputFrame->codecInputBuffers.begin();
        inputFrame->codecInputBuffers.erase(it);
    }

    if ((inputFrame->error || mErrorState) && !inputFrame->errorNotified) {
        notifyError(inputFrame->frameNumber);
        inputFrame->errorNotified = true;
    }

    if (inputFrame->fileFd >= 0) {
        close(inputFrame->fileFd);
        inputFrame->fileFd = -1;
    }

    if (inputFrame->anb != nullptr) {
        sp<ANativeWindow> outputANW = mOutputSurface;
        outputANW->cancelBuffer(mOutputSurface.get(), inputFrame->anb, /*fence*/ -1);
        inputFrame->anb = nullptr;
    }
}

void HeicCompositeStream::releaseInputFramesLocked() {
    auto it = mPendingInputFrames.begin();
    while (it != mPendingInputFrames.end()) {
        auto& inputFrame = it->second;
        if (inputFrame.error ||
            (inputFrame.appSegmentWritten && inputFrame.pendingOutputTiles == 0)) {
            releaseInputFrameLocked(&inputFrame);
            it = mPendingInputFrames.erase(it);
        } else {
            it++;
        }
    }
}

status_t HeicCompositeStream::initializeCodec(uint32_t width, uint32_t height,
        const sp<CameraDeviceBase>& cameraDevice) {
    ALOGV("%s", __FUNCTION__);

    bool useGrid = false;
    AString hevcName;
    bool isSizeSupported = isSizeSupportedByHeifEncoder(width, height,
            &mUseHeic, &useGrid, nullptr, &hevcName);
    if (!isSizeSupported) {
        ALOGE("%s: Encoder doesnt' support size %u x %u!",
                __FUNCTION__, width, height);
        return BAD_VALUE;
    }

    // Create Looper for MediaCodec.
    auto desiredMime = mUseHeic ? MIMETYPE_IMAGE_ANDROID_HEIC : MIMETYPE_VIDEO_HEVC;
    mCodecLooper = new ALooper;
    mCodecLooper->setName("Camera3-HeicComposite-MediaCodecLooper");
    status_t res = mCodecLooper->start(
            false,   // runOnCallingThread
            false,    // canCallJava
            PRIORITY_AUDIO);
    if (res != OK) {
        ALOGE("%s: Failed to start codec looper: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return NO_INIT;
    }

    // Create HEIC/HEVC codec.
    if (mUseHeic) {
        mCodec = MediaCodec::CreateByType(mCodecLooper, desiredMime, true /*encoder*/);
    } else {
        mCodec = MediaCodec::CreateByComponentName(mCodecLooper, hevcName);
    }
    if (mCodec == nullptr) {
        ALOGE("%s: Failed to create codec for %s", __FUNCTION__, desiredMime);
        return NO_INIT;
    }

    // Create Looper and handler for Codec callback.
    mCodecCallbackHandler = new CodecCallbackHandler(this);
    if (mCodecCallbackHandler == nullptr) {
        ALOGE("%s: Failed to create codec callback handler", __FUNCTION__);
        return NO_MEMORY;
    }
    mCallbackLooper = new ALooper;
    mCallbackLooper->setName("Camera3-HeicComposite-MediaCodecCallbackLooper");
    res = mCallbackLooper->start(
            false,   // runOnCallingThread
            false,    // canCallJava
            PRIORITY_AUDIO);
    if (res != OK) {
        ALOGE("%s: Failed to start media callback looper: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return NO_INIT;
    }
    mCallbackLooper->registerHandler(mCodecCallbackHandler);

    mAsyncNotify = new AMessage(kWhatCallbackNotify, mCodecCallbackHandler);
    res = mCodec->setCallback(mAsyncNotify);
    if (res != OK) {
        ALOGE("%s: Failed to set MediaCodec callback: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    // Create output format and configure the Codec.
    sp<AMessage> outputFormat = new AMessage();
    outputFormat->setString(KEY_MIME, desiredMime);
    outputFormat->setInt32(KEY_BITRATE_MODE, BITRATE_MODE_CQ);
    outputFormat->setInt32(KEY_QUALITY, kDefaultJpegQuality);
    // Ask codec to skip timestamp check and encode all frames.
    outputFormat->setInt64(KEY_MAX_PTS_GAP_TO_ENCODER, kNoFrameDropMaxPtsGap);

    int32_t gridWidth, gridHeight, gridRows, gridCols;
    if (useGrid || mUseHeic) {
        gridWidth = HeicEncoderInfoManager::kGridWidth;
        gridHeight = HeicEncoderInfoManager::kGridHeight;
        gridRows = (height + gridHeight - 1)/gridHeight;
        gridCols = (width + gridWidth - 1)/gridWidth;

        if (mUseHeic) {
            outputFormat->setInt32(KEY_TILE_WIDTH, gridWidth);
            outputFormat->setInt32(KEY_TILE_HEIGHT, gridHeight);
            outputFormat->setInt32(KEY_GRID_COLUMNS, gridCols);
            outputFormat->setInt32(KEY_GRID_ROWS, gridRows);
        }

    } else {
        gridWidth = width;
        gridHeight = height;
        gridRows = 1;
        gridCols = 1;
    }

    outputFormat->setInt32(KEY_WIDTH, !useGrid ? width : gridWidth);
    outputFormat->setInt32(KEY_HEIGHT, !useGrid ? height : gridHeight);
    outputFormat->setInt32(KEY_I_FRAME_INTERVAL, 0);
    outputFormat->setInt32(KEY_COLOR_FORMAT,
            useGrid ? COLOR_FormatYUV420Flexible : COLOR_FormatSurface);
    outputFormat->setInt32(KEY_FRAME_RATE, gridRows * gridCols);
    // This only serves as a hint to encoder when encoding is not real-time.
    outputFormat->setInt32(KEY_OPERATING_RATE, useGrid ? kGridOpRate : kNoGridOpRate);

    res = mCodec->configure(outputFormat, nullptr /*nativeWindow*/,
            nullptr /*crypto*/, CONFIGURE_FLAG_ENCODE);
    if (res != OK) {
        ALOGE("%s: Failed to configure codec: %s (%d)", __FUNCTION__,
                strerror(-res), res);
        return res;
    }

    mGridWidth = gridWidth;
    mGridHeight = gridHeight;
    mGridRows = gridRows;
    mGridCols = gridCols;
    mUseGrid = useGrid;
    mOutputWidth = width;
    mOutputHeight = height;
    mAppSegmentMaxSize = calcAppSegmentMaxSize(cameraDevice->info());
    mMaxHeicBufferSize = mOutputWidth * mOutputHeight * 3 / 2 + mAppSegmentMaxSize;

    return OK;
}

void HeicCompositeStream::deinitCodec() {
    ALOGV("%s", __FUNCTION__);
    if (mCodec != nullptr) {
        mCodec->stop();
        mCodec->release();
        mCodec.clear();
    }

    if (mCodecLooper != nullptr) {
        mCodecLooper->stop();
        mCodecLooper.clear();
    }

    if (mCallbackLooper != nullptr) {
        mCallbackLooper->stop();
        mCallbackLooper.clear();
    }

    mAsyncNotify.clear();
    mFormat.clear();
}

// Return the size of the complete list of app segment, 0 indicates failure
size_t HeicCompositeStream::findAppSegmentsSize(const uint8_t* appSegmentBuffer,
        size_t maxSize, size_t *app1SegmentSize) {
    if (appSegmentBuffer == nullptr || app1SegmentSize == nullptr) {
        ALOGE("%s: Invalid input appSegmentBuffer %p, app1SegmentSize %p",
                __FUNCTION__, appSegmentBuffer, app1SegmentSize);
        return 0;
    }

    size_t expectedSize = 0;
    // First check for EXIF transport header at the end of the buffer
    const uint8_t *header = appSegmentBuffer + (maxSize - sizeof(struct CameraBlob));
    const struct CameraBlob *blob = (const struct CameraBlob*)(header);
    if (blob->blobId != CameraBlobId::JPEG_APP_SEGMENTS) {
        ALOGE("%s: Invalid EXIF blobId %hu", __FUNCTION__, blob->blobId);
        return 0;
    }

    expectedSize = blob->blobSize;
    if (expectedSize == 0 || expectedSize > maxSize - sizeof(struct CameraBlob)) {
        ALOGE("%s: Invalid blobSize %zu.", __FUNCTION__, expectedSize);
        return 0;
    }

    uint32_t totalSize = 0;

    // Verify APP1 marker (mandatory)
    uint8_t app1Marker[] = {0xFF, 0xE1};
    if (memcmp(appSegmentBuffer, app1Marker, sizeof(app1Marker))) {
        ALOGE("%s: Invalid APP1 marker: %x, %x", __FUNCTION__,
                appSegmentBuffer[0], appSegmentBuffer[1]);
        return 0;
    }
    totalSize += sizeof(app1Marker);

    uint16_t app1Size = (static_cast<uint16_t>(appSegmentBuffer[totalSize]) << 8) +
            appSegmentBuffer[totalSize+1];
    totalSize += app1Size;

    ALOGV("%s: Expected APP segments size %zu, APP1 segment size %u",
            __FUNCTION__, expectedSize, app1Size);
    while (totalSize < expectedSize) {
        if (appSegmentBuffer[totalSize] != 0xFF ||
                appSegmentBuffer[totalSize+1] <= 0xE1 ||
                appSegmentBuffer[totalSize+1] > 0xEF) {
            // Invalid APPn marker
            ALOGE("%s: Invalid APPn marker: %x, %x", __FUNCTION__,
                    appSegmentBuffer[totalSize], appSegmentBuffer[totalSize+1]);
            return 0;
        }
        totalSize += 2;

        uint16_t appnSize = (static_cast<uint16_t>(appSegmentBuffer[totalSize]) << 8) +
                appSegmentBuffer[totalSize+1];
        totalSize += appnSize;
    }

    if (totalSize != expectedSize) {
        ALOGE("%s: Invalid JPEG APP segments: totalSize %u vs expected size %zu",
                __FUNCTION__, totalSize, expectedSize);
        return 0;
    }

    *app1SegmentSize = app1Size + sizeof(app1Marker);
    return expectedSize;
}

int64_t HeicCompositeStream::findTimestampInNsLocked(int64_t timeInUs) {
    for (const auto& fn : mFrameNumberMap) {
        if (timeInUs == ns2us(fn.second)) {
            return fn.second;
        }
    }
    for (const auto& inputFrame : mPendingInputFrames) {
        if (timeInUs == ns2us(inputFrame.first)) {
            return inputFrame.first;
        }
    }
    return -1;
}

status_t HeicCompositeStream::copyOneYuvTile(sp<MediaCodecBuffer>& codecBuffer,
        const CpuConsumer::LockedBuffer& yuvBuffer,
        size_t top, size_t left, size_t width, size_t height) {
    ATRACE_CALL();

    // Get stride information for codecBuffer
    sp<ABuffer> imageData;
    if (!codecBuffer->meta()->findBuffer("image-data", &imageData)) {
        ALOGE("%s: Codec input buffer is not for image data!", __FUNCTION__);
        return BAD_VALUE;
    }
    if (imageData->size() != sizeof(MediaImage2)) {
        ALOGE("%s: Invalid codec input image size %zu, expected %zu",
                __FUNCTION__, imageData->size(), sizeof(MediaImage2));
        return BAD_VALUE;
    }
    MediaImage2* imageInfo = reinterpret_cast<MediaImage2*>(imageData->data());
    if (imageInfo->mType != MediaImage2::MEDIA_IMAGE_TYPE_YUV ||
            imageInfo->mBitDepth != 8 ||
            imageInfo->mBitDepthAllocated != 8 ||
            imageInfo->mNumPlanes != 3) {
        ALOGE("%s: Invalid codec input image info: mType %d, mBitDepth %d, "
                "mBitDepthAllocated %d, mNumPlanes %d!", __FUNCTION__,
                imageInfo->mType, imageInfo->mBitDepth,
                imageInfo->mBitDepthAllocated, imageInfo->mNumPlanes);
        return BAD_VALUE;
    }

    ALOGV("%s: yuvBuffer chromaStep %d, chromaStride %d",
            __FUNCTION__, yuvBuffer.chromaStep, yuvBuffer.chromaStride);
    ALOGV("%s: U offset %u, V offset %u, U rowInc %d, V rowInc %d, U colInc %d, V colInc %d",
            __FUNCTION__, imageInfo->mPlane[MediaImage2::U].mOffset,
            imageInfo->mPlane[MediaImage2::V].mOffset,
            imageInfo->mPlane[MediaImage2::U].mRowInc,
            imageInfo->mPlane[MediaImage2::V].mRowInc,
            imageInfo->mPlane[MediaImage2::U].mColInc,
            imageInfo->mPlane[MediaImage2::V].mColInc);

    // Y
    for (auto row = top; row < top+height; row++) {
        uint8_t *dst = codecBuffer->data() + imageInfo->mPlane[MediaImage2::Y].mOffset +
                imageInfo->mPlane[MediaImage2::Y].mRowInc * (row - top);
        mFnCopyRow(yuvBuffer.data+row*yuvBuffer.stride+left, dst, width);
    }

    // U is Cb, V is Cr
    bool codecUPlaneFirst = imageInfo->mPlane[MediaImage2::V].mOffset >
            imageInfo->mPlane[MediaImage2::U].mOffset;
    uint32_t codecUvOffsetDiff = codecUPlaneFirst ?
            imageInfo->mPlane[MediaImage2::V].mOffset - imageInfo->mPlane[MediaImage2::U].mOffset :
            imageInfo->mPlane[MediaImage2::U].mOffset - imageInfo->mPlane[MediaImage2::V].mOffset;
    bool isCodecUvSemiplannar = (codecUvOffsetDiff == 1) &&
            (imageInfo->mPlane[MediaImage2::U].mRowInc ==
            imageInfo->mPlane[MediaImage2::V].mRowInc) &&
            (imageInfo->mPlane[MediaImage2::U].mColInc == 2) &&
            (imageInfo->mPlane[MediaImage2::V].mColInc == 2);
    bool isCodecUvPlannar =
            ((codecUPlaneFirst && codecUvOffsetDiff >=
                    imageInfo->mPlane[MediaImage2::U].mRowInc * imageInfo->mHeight/2) ||
            ((!codecUPlaneFirst && codecUvOffsetDiff >=
                    imageInfo->mPlane[MediaImage2::V].mRowInc * imageInfo->mHeight/2))) &&
            imageInfo->mPlane[MediaImage2::U].mColInc == 1 &&
            imageInfo->mPlane[MediaImage2::V].mColInc == 1;
    bool cameraUPlaneFirst = yuvBuffer.dataCr > yuvBuffer.dataCb;

    if (isCodecUvSemiplannar && yuvBuffer.chromaStep == 2 &&
            (codecUPlaneFirst == cameraUPlaneFirst)) {
        // UV semiplannar
        // The chrome plane could be either Cb first, or Cr first. Take the
        // smaller address.
        uint8_t *src = std::min(yuvBuffer.dataCb, yuvBuffer.dataCr);
        MediaImage2::PlaneIndex dstPlane = codecUvOffsetDiff > 0 ? MediaImage2::U : MediaImage2::V;
        for (auto row = top/2; row < (top+height)/2; row++) {
            uint8_t *dst = codecBuffer->data() + imageInfo->mPlane[dstPlane].mOffset +
                    imageInfo->mPlane[dstPlane].mRowInc * (row - top/2);
            mFnCopyRow(src+row*yuvBuffer.chromaStride+left, dst, width);
        }
    } else if (isCodecUvPlannar && yuvBuffer.chromaStep == 1) {
        // U plane
        for (auto row = top/2; row < (top+height)/2; row++) {
            uint8_t *dst = codecBuffer->data() + imageInfo->mPlane[MediaImage2::U].mOffset +
                    imageInfo->mPlane[MediaImage2::U].mRowInc * (row - top/2);
            mFnCopyRow(yuvBuffer.dataCb+row*yuvBuffer.chromaStride+left/2, dst, width/2);
        }

        // V plane
        for (auto row = top/2; row < (top+height)/2; row++) {
            uint8_t *dst = codecBuffer->data() + imageInfo->mPlane[MediaImage2::V].mOffset +
                    imageInfo->mPlane[MediaImage2::V].mRowInc * (row - top/2);
            mFnCopyRow(yuvBuffer.dataCr+row*yuvBuffer.chromaStride+left/2, dst, width/2);
        }
    } else {
        // Convert between semiplannar and plannar, or when UV orders are
        // different.
        uint8_t *dst = codecBuffer->data();
        for (auto row = top/2; row < (top+height)/2; row++) {
            for (auto col = left/2; col < (left+width)/2; col++) {
                // U/Cb
                int32_t dstIndex = imageInfo->mPlane[MediaImage2::U].mOffset +
                        imageInfo->mPlane[MediaImage2::U].mRowInc * (row - top/2) +
                        imageInfo->mPlane[MediaImage2::U].mColInc * (col - left/2);
                int32_t srcIndex = row * yuvBuffer.chromaStride + yuvBuffer.chromaStep * col;
                dst[dstIndex] = yuvBuffer.dataCb[srcIndex];

                // V/Cr
                dstIndex = imageInfo->mPlane[MediaImage2::V].mOffset +
                        imageInfo->mPlane[MediaImage2::V].mRowInc * (row - top/2) +
                        imageInfo->mPlane[MediaImage2::V].mColInc * (col - left/2);
                srcIndex = row * yuvBuffer.chromaStride + yuvBuffer.chromaStep * col;
                dst[dstIndex] = yuvBuffer.dataCr[srcIndex];
            }
        }
    }
    return OK;
}

void HeicCompositeStream::initCopyRowFunction(int32_t width)
{
    using namespace libyuv;

    mFnCopyRow = CopyRow_C;
#if defined(HAS_COPYROW_SSE2)
    if (TestCpuFlag(kCpuHasSSE2)) {
        mFnCopyRow = IS_ALIGNED(width, 32) ? CopyRow_SSE2 : CopyRow_Any_SSE2;
    }
#endif
#if defined(HAS_COPYROW_AVX)
    if (TestCpuFlag(kCpuHasAVX)) {
        mFnCopyRow = IS_ALIGNED(width, 64) ? CopyRow_AVX : CopyRow_Any_AVX;
    }
#endif
#if defined(HAS_COPYROW_ERMS)
    if (TestCpuFlag(kCpuHasERMS)) {
        mFnCopyRow = CopyRow_ERMS;
    }
#endif
#if defined(HAS_COPYROW_NEON)
    if (TestCpuFlag(kCpuHasNEON)) {
        mFnCopyRow = IS_ALIGNED(width, 32) ? CopyRow_NEON : CopyRow_Any_NEON;
    }
#endif
#if defined(HAS_COPYROW_MIPS)
    if (TestCpuFlag(kCpuHasMIPS)) {
        mFnCopyRow = CopyRow_MIPS;
    }
#endif
}

size_t HeicCompositeStream::calcAppSegmentMaxSize(const CameraMetadata& info) {
    camera_metadata_ro_entry_t entry = info.find(ANDROID_HEIC_INFO_MAX_JPEG_APP_SEGMENTS_COUNT);
    size_t maxAppsSegment = 1;
    if (entry.count > 0) {
        maxAppsSegment = entry.data.u8[0] < 1 ? 1 :
                entry.data.u8[0] > 16 ? 16 : entry.data.u8[0];
    }
    return maxAppsSegment * (2 + 0xFFFF) + sizeof(struct CameraBlob);
}

bool HeicCompositeStream::threadLoop() {
    int64_t currentTs = INT64_MAX;
    bool newInputAvailable = false;

    {
        Mutex::Autolock l(mMutex);
        if (mErrorState) {
            // In case we landed in error state, return any pending buffers and
            // halt all further processing.
            compilePendingInputLocked();
            releaseInputFramesLocked();
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
        ALOGE("%s: Failed processing frame with timestamp: %" PRIu64 ": %s (%d)",
                __FUNCTION__, currentTs, strerror(-res), res);
        mPendingInputFrames[currentTs].error = true;
    }

    releaseInputFramesLocked();

    return true;
}

bool HeicCompositeStream::onStreamBufferError(const CaptureResultExtras& resultExtras) {
    bool res = false;
    // Buffer errors concerning internal composite streams should not be directly visible to
    // camera clients. They must only receive a single buffer error with the public composite
    // stream id.
    if ((resultExtras.errorStreamId == mAppSegmentStreamId) ||
            (resultExtras.errorStreamId == mMainImageStreamId)) {
        flagAnErrorFrameNumber(resultExtras.frameNumber);
        res = true;
    }

    return res;
}

void HeicCompositeStream::onResultError(const CaptureResultExtras& resultExtras) {
    // For result error, since the APPS_SEGMENT buffer already contains EXIF,
    // simply skip using the capture result metadata to override EXIF.
    Mutex::Autolock l(mMutex);

    int64_t timestamp = -1;
    for (const auto& fn : mFrameNumberMap) {
        if (fn.first == resultExtras.frameNumber) {
            timestamp = fn.second;
            break;
        }
    }
    if (timestamp == -1) {
        for (const auto& inputFrame : mPendingInputFrames) {
            if (inputFrame.second.frameNumber == resultExtras.frameNumber) {
                timestamp = inputFrame.first;
                break;
            }
        }
    }

    if (timestamp == -1) {
        ALOGE("%s: Failed to find shutter timestamp for result error!", __FUNCTION__);
        return;
    }

    mCaptureResults.emplace(timestamp, std::make_tuple(resultExtras.frameNumber, CameraMetadata()));
    mInputReadyCondition.signal();
}

void HeicCompositeStream::CodecCallbackHandler::onMessageReceived(const sp<AMessage> &msg) {
    sp<HeicCompositeStream> parent = mParent.promote();
    if (parent == nullptr) return;

    switch (msg->what()) {
        case kWhatCallbackNotify: {
             int32_t cbID;
             if (!msg->findInt32("callbackID", &cbID)) {
                 ALOGE("kWhatCallbackNotify: callbackID is expected.");
                 break;
             }

             ALOGV("kWhatCallbackNotify: cbID = %d", cbID);

             switch (cbID) {
                 case MediaCodec::CB_INPUT_AVAILABLE: {
                     int32_t index;
                     if (!msg->findInt32("index", &index)) {
                         ALOGE("CB_INPUT_AVAILABLE: index is expected.");
                         break;
                     }
                     parent->onHeicInputFrameAvailable(index);
                     break;
                 }

                 case MediaCodec::CB_OUTPUT_AVAILABLE: {
                     int32_t index;
                     size_t offset;
                     size_t size;
                     int64_t timeUs;
                     int32_t flags;

                     if (!msg->findInt32("index", &index)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: index is expected.");
                         break;
                     }
                     if (!msg->findSize("offset", &offset)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: offset is expected.");
                         break;
                     }
                     if (!msg->findSize("size", &size)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: size is expected.");
                         break;
                     }
                     if (!msg->findInt64("timeUs", &timeUs)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: timeUs is expected.");
                         break;
                     }
                     if (!msg->findInt32("flags", &flags)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: flags is expected.");
                         break;
                     }

                     CodecOutputBufferInfo bufferInfo = {
                         index,
                         (int32_t)offset,
                         (int32_t)size,
                         timeUs,
                         (uint32_t)flags};

                     parent->onHeicOutputFrameAvailable(bufferInfo);
                     break;
                 }

                 case MediaCodec::CB_OUTPUT_FORMAT_CHANGED: {
                     sp<AMessage> format;
                     if (!msg->findMessage("format", &format)) {
                         ALOGE("CB_OUTPUT_FORMAT_CHANGED: format is expected.");
                         break;
                     }
                     // Here format is MediaCodec's internal copy of output format.
                     // Make a copy since onHeicFormatChanged() might modify it.
                     sp<AMessage> formatCopy;
                     if (format != nullptr) {
                         formatCopy = format->dup();
                     }
                     parent->onHeicFormatChanged(formatCopy);
                     break;
                 }

                 case MediaCodec::CB_ERROR: {
                     status_t err;
                     int32_t actionCode;
                     AString detail;
                     if (!msg->findInt32("err", &err)) {
                         ALOGE("CB_ERROR: err is expected.");
                         break;
                     }
                     if (!msg->findInt32("action", &actionCode)) {
                         ALOGE("CB_ERROR: action is expected.");
                         break;
                     }
                     msg->findString("detail", &detail);
                     ALOGE("Codec reported error(0x%x), actionCode(%d), detail(%s)",
                             err, actionCode, detail.c_str());

                     parent->onHeicCodecError();
                     break;
                 }

                 default: {
                     ALOGE("kWhatCallbackNotify: callbackID(%d) is unexpected.", cbID);
                     break;
                 }
             }
             break;
        }

        default:
            ALOGE("shouldn't be here");
            break;
    }
}

}; // namespace camera3
}; // namespace android
