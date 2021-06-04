/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "CameraHardwareInterface"
//#define LOG_NDEBUG 0

#include <inttypes.h>
#include <media/hardware/HardwareAPI.h> // For VideoNativeHandleMetadata
#include "CameraHardwareInterface.h"

namespace android {

using namespace hardware::camera::device::V1_0;
using namespace hardware::camera::common::V1_0;
using hardware::hidl_handle;

CameraHardwareInterface::~CameraHardwareInterface()
{
    ALOGI("Destroying camera %s", mName.string());
    if (mHidlDevice != nullptr) {
        mHidlDevice->close();
        mHidlDevice.clear();
        cleanupCirculatingBuffers();
    }
}

status_t CameraHardwareInterface::initialize(sp<CameraProviderManager> manager) {
    ALOGI("Opening camera %s", mName.string());

    status_t ret = manager->openSession(mName.string(), this, &mHidlDevice);
    if (ret != OK) {
        ALOGE("%s: openSession failed! %s (%d)", __FUNCTION__, strerror(-ret), ret);
    }
    return ret;
}

status_t CameraHardwareInterface::setPreviewScalingMode(int scalingMode)
{
    int rc = OK;
    mPreviewScalingMode = scalingMode;
    if (mPreviewWindow != nullptr) {
        rc = native_window_set_scaling_mode(mPreviewWindow.get(),
                scalingMode);
    }
    return rc;
}

status_t CameraHardwareInterface::setPreviewTransform(int transform) {
    int rc = OK;
    mPreviewTransform = transform;
    if (mPreviewWindow != nullptr) {
        rc = native_window_set_buffers_transform(mPreviewWindow.get(),
                mPreviewTransform);
    }
    return rc;
}

/**
 * Implementation of android::hardware::camera::device::V1_0::ICameraDeviceCallback
 */
hardware::Return<void> CameraHardwareInterface::notifyCallback(
        NotifyCallbackMsg msgType, int32_t ext1, int32_t ext2) {
    sNotifyCb((int32_t) msgType, ext1, ext2, (void*) this);
    return hardware::Void();
}

hardware::Return<uint32_t> CameraHardwareInterface::registerMemory(
        const hardware::hidl_handle& descriptor,
        uint32_t bufferSize, uint32_t bufferCount) {
    if (descriptor->numFds != 1) {
        ALOGE("%s: camera memory descriptor has numFds %d (expect 1)",
                __FUNCTION__, descriptor->numFds);
        return 0;
    }
    if (descriptor->data[0] < 0) {
        ALOGE("%s: camera memory descriptor has FD %d (expect >= 0)",
                __FUNCTION__, descriptor->data[0]);
        return 0;
    }

    camera_memory_t* mem = sGetMemory(descriptor->data[0], bufferSize, bufferCount, this);
    sp<CameraHeapMemory> camMem(static_cast<CameraHeapMemory *>(mem->handle));
    int memPoolId = camMem->mHeap->getHeapID();
    if (memPoolId < 0) {
        ALOGE("%s: CameraHeapMemory has FD %d (expect >= 0)", __FUNCTION__, memPoolId);
        return 0;
    }
    std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
    mHidlMemPoolMap.insert(std::make_pair(memPoolId, mem));
    return memPoolId;
}

hardware::Return<void> CameraHardwareInterface::unregisterMemory(uint32_t memId) {
    camera_memory_t* mem = nullptr;
    {
        std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
        if (mHidlMemPoolMap.count(memId) == 0) {
            ALOGE("%s: memory pool ID %d not found", __FUNCTION__, memId);
            return hardware::Void();
        }
        mem = mHidlMemPoolMap.at(memId);
        mHidlMemPoolMap.erase(memId);
    }
    sPutMemory(mem);
    return hardware::Void();
}

hardware::Return<void> CameraHardwareInterface::dataCallback(
        DataCallbackMsg msgType, uint32_t data, uint32_t bufferIndex,
        const hardware::camera::device::V1_0::CameraFrameMetadata& metadata) {
    camera_memory_t* mem = nullptr;
    {
        std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
        if (mHidlMemPoolMap.count(data) == 0) {
            ALOGE("%s: memory pool ID %d not found", __FUNCTION__, data);
            return hardware::Void();
        }
        mem = mHidlMemPoolMap.at(data);
    }
    camera_frame_metadata_t md;
    md.number_of_faces = metadata.faces.size();
    md.faces = (camera_face_t*) metadata.faces.data();
    sDataCb((int32_t) msgType, mem, bufferIndex, &md, this);
    return hardware::Void();
}

hardware::Return<void> CameraHardwareInterface::dataCallbackTimestamp(
        DataCallbackMsg msgType, uint32_t data,
        uint32_t bufferIndex, int64_t timestamp) {
    camera_memory_t* mem = nullptr;
    {
        std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
        if (mHidlMemPoolMap.count(data) == 0) {
            ALOGE("%s: memory pool ID %d not found", __FUNCTION__, data);
            return hardware::Void();
        }
        mem = mHidlMemPoolMap.at(data);
    }
    sDataCbTimestamp(timestamp, (int32_t) msgType, mem, bufferIndex, this);
    return hardware::Void();
}

hardware::Return<void> CameraHardwareInterface::handleCallbackTimestamp(
        DataCallbackMsg msgType, const hidl_handle& frameData, uint32_t data,
        uint32_t bufferIndex, int64_t timestamp) {
    camera_memory_t* mem = nullptr;
    {
        std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
        if (mHidlMemPoolMap.count(data) == 0) {
            ALOGE("%s: memory pool ID %d not found", __FUNCTION__, data);
            return hardware::Void();
        }
        mem = mHidlMemPoolMap.at(data);
    }
    sp<CameraHeapMemory> heapMem(static_cast<CameraHeapMemory *>(mem->handle));
    VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*)
            heapMem->mBuffers[bufferIndex]->pointer();
    md->pHandle = const_cast<native_handle_t*>(frameData.getNativeHandle());
    sDataCbTimestamp(timestamp, (int32_t) msgType, mem, bufferIndex, this);
    return hardware::Void();
}

hardware::Return<void> CameraHardwareInterface::handleCallbackTimestampBatch(
        DataCallbackMsg msgType,
        const hardware::hidl_vec<hardware::camera::device::V1_0::HandleTimestampMessage>& messages) {
    std::vector<android::HandleTimestampMessage> msgs;
    msgs.reserve(messages.size());
    {
        std::lock_guard<std::mutex> lock(mHidlMemPoolMapLock);
        for (const auto& hidl_msg : messages) {
            if (mHidlMemPoolMap.count(hidl_msg.data) == 0) {
                ALOGE("%s: memory pool ID %d not found", __FUNCTION__, hidl_msg.data);
                return hardware::Void();
            }
            sp<CameraHeapMemory> mem(
                    static_cast<CameraHeapMemory *>(mHidlMemPoolMap.at(hidl_msg.data)->handle));

            if (hidl_msg.bufferIndex >= mem->mNumBufs) {
                ALOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
                     hidl_msg.bufferIndex, mem->mNumBufs);
                return hardware::Void();
            }
            VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*)
                    mem->mBuffers[hidl_msg.bufferIndex]->pointer();
            md->pHandle = const_cast<native_handle_t*>(hidl_msg.frameData.getNativeHandle());

            msgs.push_back({hidl_msg.timestamp, mem->mBuffers[hidl_msg.bufferIndex]});
        }
    }
    mDataCbTimestampBatch((int32_t) msgType, msgs, mCbUser);
    return hardware::Void();
}

std::pair<bool, uint64_t> CameraHardwareInterface::getBufferId(
        ANativeWindowBuffer* anb) {
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);

    buffer_handle_t& buf = anb->handle;
    auto it = mBufferIdMap.find(buf);
    if (it == mBufferIdMap.end()) {
        uint64_t bufId = mNextBufferId++;
        mBufferIdMap[buf] = bufId;
        mReversedBufMap[bufId] = anb;
        return std::make_pair(true, bufId);
    } else {
        return std::make_pair(false, it->second);
    }
}

void CameraHardwareInterface::cleanupCirculatingBuffers() {
    std::lock_guard<std::mutex> lock(mBufferIdMapLock);
    mBufferIdMap.clear();
    mReversedBufMap.clear();
}

hardware::Return<void>
CameraHardwareInterface::dequeueBuffer(dequeueBuffer_cb _hidl_cb) {
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return hardware::Void();
    }
    ANativeWindowBuffer* anb;
    int rc = native_window_dequeue_buffer_and_wait(a, &anb);
    Status s = Status::INTERNAL_ERROR;
    uint64_t bufferId = 0;
    uint32_t stride = 0;
    hidl_handle buf = nullptr;
    if (rc == OK) {
        s = Status::OK;
        auto pair = getBufferId(anb);
        buf = (pair.first) ? anb->handle : nullptr;
        bufferId = pair.second;
        stride = anb->stride;
    }

    _hidl_cb(s, bufferId, buf, stride);
    return hardware::Void();
}

hardware::Return<Status>
CameraHardwareInterface::enqueueBuffer(uint64_t bufferId) {
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return Status::INTERNAL_ERROR;
    }
    if (mReversedBufMap.count(bufferId) == 0) {
        ALOGE("%s: bufferId %" PRIu64 " not found", __FUNCTION__, bufferId);
        return Status::ILLEGAL_ARGUMENT;
    }
    int rc = a->queueBuffer(a, mReversedBufMap.at(bufferId), -1);
    if (rc == 0) {
        return Status::OK;
    }
    return Status::INTERNAL_ERROR;
}

hardware::Return<Status>
CameraHardwareInterface::cancelBuffer(uint64_t bufferId) {
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return Status::INTERNAL_ERROR;
    }
    if (mReversedBufMap.count(bufferId) == 0) {
        ALOGE("%s: bufferId %" PRIu64 " not found", __FUNCTION__, bufferId);
        return Status::ILLEGAL_ARGUMENT;
    }
    int rc = a->cancelBuffer(a, mReversedBufMap.at(bufferId), -1);
    if (rc == 0) {
        return Status::OK;
    }
    return Status::INTERNAL_ERROR;
}

hardware::Return<Status>
CameraHardwareInterface::setBufferCount(uint32_t count) {
    ANativeWindow *a = mPreviewWindow.get();
    if (a != nullptr) {
        // Workaround for b/27039775
        // Previously, setting the buffer count would reset the buffer
        // queue's flag that allows for all buffers to be dequeued on the
        // producer side, instead of just the producer's declared max count,
        // if no filled buffers have yet been queued by the producer.  This
        // reset no longer happens, but some HALs depend on this behavior,
        // so it needs to be maintained for HAL backwards compatibility.
        // Simulate the prior behavior by disconnecting/reconnecting to the
        // window and setting the values again.  This has the drawback of
        // actually causing memory reallocation, which may not have happened
        // in the past.
        native_window_api_disconnect(a, NATIVE_WINDOW_API_CAMERA);
        native_window_api_connect(a, NATIVE_WINDOW_API_CAMERA);
        if (mPreviewScalingMode != NOT_SET) {
            native_window_set_scaling_mode(a, mPreviewScalingMode);
        }
        if (mPreviewTransform != NOT_SET) {
            native_window_set_buffers_transform(a, mPreviewTransform);
        }
        if (mPreviewWidth != NOT_SET) {
            native_window_set_buffers_dimensions(a,
                    mPreviewWidth, mPreviewHeight);
            native_window_set_buffers_format(a, mPreviewFormat);
        }
        if (mPreviewUsage != 0) {
            native_window_set_usage(a, mPreviewUsage);
        }
        if (mPreviewSwapInterval != NOT_SET) {
            a->setSwapInterval(a, mPreviewSwapInterval);
        }
        if (mPreviewCrop.left != NOT_SET) {
            native_window_set_crop(a, &(mPreviewCrop));
        }
    }
    int rc = native_window_set_buffer_count(a, count);
    if (rc == OK) {
        cleanupCirculatingBuffers();
        return Status::OK;
    }
    return Status::INTERNAL_ERROR;
}

hardware::Return<Status>
CameraHardwareInterface::setBuffersGeometry(
        uint32_t w, uint32_t h, hardware::graphics::common::V1_0::PixelFormat format) {
    Status s = Status::INTERNAL_ERROR;
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return s;
    }
    mPreviewWidth = w;
    mPreviewHeight = h;
    mPreviewFormat = (int) format;
    int rc = native_window_set_buffers_dimensions(a, w, h);
    if (rc == OK) {
        rc = native_window_set_buffers_format(a, mPreviewFormat);
    }
    if (rc == OK) {
        cleanupCirculatingBuffers();
        s = Status::OK;
    }
    return s;
}

hardware::Return<Status>
CameraHardwareInterface::setCrop(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    Status s = Status::INTERNAL_ERROR;
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return s;
    }
    mPreviewCrop.left = left;
    mPreviewCrop.top = top;
    mPreviewCrop.right = right;
    mPreviewCrop.bottom = bottom;
    int rc = native_window_set_crop(a, &mPreviewCrop);
    if (rc == OK) {
        s = Status::OK;
    }
    return s;
}

hardware::Return<Status>
CameraHardwareInterface::setUsage(hardware::graphics::common::V1_0::BufferUsage usage) {
    Status s = Status::INTERNAL_ERROR;
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return s;
    }
    mPreviewUsage = static_cast<uint64_t> (usage);
    int rc = native_window_set_usage(a, mPreviewUsage);
    if (rc == OK) {
        cleanupCirculatingBuffers();
        s = Status::OK;
    }
    return s;
}

hardware::Return<Status>
CameraHardwareInterface::setSwapInterval(int32_t interval) {
    Status s = Status::INTERNAL_ERROR;
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return s;
    }
    mPreviewSwapInterval = interval;
    int rc = a->setSwapInterval(a, interval);
    if (rc == OK) {
        s = Status::OK;
    }
    return s;
}

hardware::Return<void>
CameraHardwareInterface::getMinUndequeuedBufferCount(getMinUndequeuedBufferCount_cb _hidl_cb) {
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return hardware::Void();
    }
    int count = 0;
    int rc = a->query(a, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &count);
    Status s = Status::INTERNAL_ERROR;
    if (rc == OK) {
        s = Status::OK;
    }
    _hidl_cb(s, count);
    return hardware::Void();
}

hardware::Return<Status>
CameraHardwareInterface::setTimestamp(int64_t timestamp) {
    Status s = Status::INTERNAL_ERROR;
    ANativeWindow *a = mPreviewWindow.get();
    if (a == nullptr) {
        ALOGE("%s: preview window is null", __FUNCTION__);
        return s;
    }
    int rc = native_window_set_buffers_timestamp(a, timestamp);
    if (rc == OK) {
        s = Status::OK;
    }
    return s;
}

status_t CameraHardwareInterface::setPreviewWindow(const sp<ANativeWindow>& buf)
{
    ALOGV("%s(%s) buf %p", __FUNCTION__, mName.string(), buf.get());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mPreviewWindow = buf;
        if (buf != nullptr) {
            if (mPreviewScalingMode != NOT_SET) {
                setPreviewScalingMode(mPreviewScalingMode);
            }
            if (mPreviewTransform != NOT_SET) {
                setPreviewTransform(mPreviewTransform);
            }
        }
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->setPreviewWindow(buf.get() ? this : nullptr));
    }
    return INVALID_OPERATION;
}

void CameraHardwareInterface::setCallbacks(notify_callback notify_cb,
        data_callback data_cb,
        data_callback_timestamp data_cb_timestamp,
        data_callback_timestamp_batch data_cb_timestamp_batch,
        void* user)
{
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
    mDataCbTimestamp = data_cb_timestamp;
    mDataCbTimestampBatch = data_cb_timestamp_batch;
    mCbUser = user;

    ALOGV("%s(%s)", __FUNCTION__, mName.string());
}

void CameraHardwareInterface::enableMsgType(int32_t msgType)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mHidlDevice->enableMsgType(msgType);
    }
}

void CameraHardwareInterface::disableMsgType(int32_t msgType)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mHidlDevice->disableMsgType(msgType);
    }
}

int CameraHardwareInterface::msgTypeEnabled(int32_t msgType)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return mHidlDevice->msgTypeEnabled(msgType);
    }
    return false;
}

status_t CameraHardwareInterface::startPreview()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->startPreview());
    }
    return INVALID_OPERATION;
}

void CameraHardwareInterface::stopPreview()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mHidlDevice->stopPreview();
    }
}

int CameraHardwareInterface::previewEnabled()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return mHidlDevice->previewEnabled();
    }
    return false;
}

status_t CameraHardwareInterface::storeMetaDataInBuffers(int enable)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->storeMetaDataInBuffers(enable));
    }
    return enable ? INVALID_OPERATION: OK;
}

status_t CameraHardwareInterface::startRecording()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->startRecording());
    }
    return INVALID_OPERATION;
}

/**
 * Stop a previously started recording.
 */
void CameraHardwareInterface::stopRecording()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mHidlDevice->stopRecording();
    }
}

/**
 * Returns true if recording is enabled.
 */
int CameraHardwareInterface::recordingEnabled()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return mHidlDevice->recordingEnabled();
    }
    return false;
}

void CameraHardwareInterface::releaseRecordingFrame(const sp<IMemory>& mem)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = mem->getMemory(&offset, &size);
    int heapId = heap->getHeapID();
    int bufferIndex = offset / size;
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        if (size == sizeof(VideoNativeHandleMetadata)) {
            VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*) mem->pointer();
            // Caching the handle here because md->pHandle will be subject to HAL's edit
            native_handle_t* nh = md->pHandle;
            hidl_handle frame = nh;
            mHidlDevice->releaseRecordingFrameHandle(heapId, bufferIndex, frame);
            native_handle_close(nh);
            native_handle_delete(nh);
        } else {
            mHidlDevice->releaseRecordingFrame(heapId, bufferIndex);
        }
    }
}

void CameraHardwareInterface::releaseRecordingFrameBatch(const std::vector<sp<IMemory>>& frames)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    size_t n = frames.size();
    std::vector<VideoFrameMessage> msgs;
    msgs.reserve(n);
    for (auto& mem : frames) {
        if (CC_LIKELY(mHidlDevice != nullptr)) {
            ssize_t offset;
            size_t size;
            sp<IMemoryHeap> heap = mem->getMemory(&offset, &size);
            if (size == sizeof(VideoNativeHandleMetadata)) {
                uint32_t heapId = heap->getHeapID();
                uint32_t bufferIndex = offset / size;
                VideoNativeHandleMetadata* md = (VideoNativeHandleMetadata*) mem->pointer();
                // Caching the handle here because md->pHandle will be subject to HAL's edit
                native_handle_t* nh = md->pHandle;
                VideoFrameMessage msg;
                msgs.push_back({nh, heapId, bufferIndex});
            } else {
                ALOGE("%s only supports VideoNativeHandleMetadata mode", __FUNCTION__);
                return;
            }
        }
    }

    mHidlDevice->releaseRecordingFrameHandleBatch(msgs);

    for (auto& msg : msgs) {
        native_handle_t* nh = const_cast<native_handle_t*>(msg.frameData.getNativeHandle());
        native_handle_close(nh);
        native_handle_delete(nh);
    }
}

status_t CameraHardwareInterface::autoFocus()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->autoFocus());
    }
    return INVALID_OPERATION;
}

status_t CameraHardwareInterface::cancelAutoFocus()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->cancelAutoFocus());
    }
    return INVALID_OPERATION;
}

status_t CameraHardwareInterface::takePicture()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->takePicture());
    }
    return INVALID_OPERATION;
}

status_t CameraHardwareInterface::cancelPicture()
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->cancelPicture());
    }
    return INVALID_OPERATION;
}

status_t CameraHardwareInterface::setParameters(const CameraParameters &params)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->setParameters(params.flatten().string()));
    }
    return INVALID_OPERATION;
}

CameraParameters CameraHardwareInterface::getParameters() const
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    CameraParameters parms;
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        hardware::hidl_string outParam;
        mHidlDevice->getParameters(
                [&outParam](const auto& outStr) {
                    outParam = outStr;
                });
        String8 tmp(outParam.c_str());
        parms.unflatten(tmp);
    }
    return parms;
}

status_t CameraHardwareInterface::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        return CameraProviderManager::mapToStatusT(
                mHidlDevice->sendCommand((CommandType) cmd, arg1, arg2));
    }
    return INVALID_OPERATION;
}

/**
 * Release the hardware resources owned by this object.  Note that this is
 * *not* done in the destructor.
 */
void CameraHardwareInterface::release() {
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        mHidlDevice->close();
        mHidlDevice.clear();
    }
}

/**
 * Dump state of the camera hardware
 */
status_t CameraHardwareInterface::dump(int fd, const Vector<String16>& /*args*/) const
{
    ALOGV("%s(%s)", __FUNCTION__, mName.string());
    if (CC_LIKELY(mHidlDevice != nullptr)) {
        native_handle_t* handle = native_handle_create(1,0);
        handle->data[0] = fd;
        Status s = mHidlDevice->dumpState(handle);
        native_handle_delete(handle);
        return CameraProviderManager::mapToStatusT(s);
    }
    return OK; // It's fine if the HAL doesn't implement dump()
}

void CameraHardwareInterface::sNotifyCb(int32_t msg_type, int32_t ext1,
                        int32_t ext2, void *user)
{
    ALOGV("%s", __FUNCTION__);
    CameraHardwareInterface *object =
            static_cast<CameraHardwareInterface *>(user);
    object->mNotifyCb(msg_type, ext1, ext2, object->mCbUser);
}

void CameraHardwareInterface::sDataCb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user)
{
    ALOGV("%s", __FUNCTION__);
    CameraHardwareInterface *object =
            static_cast<CameraHardwareInterface *>(user);
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        ALOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }
    object->mDataCb(msg_type, mem->mBuffers[index], metadata, object->mCbUser);
}

void CameraHardwareInterface::sDataCbTimestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user)
{
    ALOGV("%s", __FUNCTION__);
    CameraHardwareInterface *object =
            static_cast<CameraHardwareInterface *>(user);
    // Start refcounting the heap object from here on.  When the clients
    // drop all references, it will be destroyed (as well as the enclosed
    // MemoryHeapBase.
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        ALOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }
    object->mDataCbTimestamp(timestamp, msg_type, mem->mBuffers[index], object->mCbUser);
}

camera_memory_t* CameraHardwareInterface::sGetMemory(
        int fd, size_t buf_size, uint_t num_bufs,
        void *user __attribute__((unused)))
{
    CameraHeapMemory *mem;
    if (fd < 0) {
        mem = new CameraHeapMemory(buf_size, num_bufs);
    } else {
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    }
    mem->incStrong(mem);
    return &mem->handle;
}

void CameraHardwareInterface::sPutMemory(camera_memory_t *data)
{
    if (!data) {
        return;
    }

    CameraHeapMemory *mem = static_cast<CameraHeapMemory *>(data->handle);
    mem->decStrong(mem);
}

}; // namespace android
