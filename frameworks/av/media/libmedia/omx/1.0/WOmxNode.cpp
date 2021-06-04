/*
 * Copyright 2016, The Android Open Source Project
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

#include <algorithm>

#include <media/omx/1.0/WOmxNode.h>
#include <media/omx/1.0/WOmxBufferSource.h>
#include <media/omx/1.0/Conversion.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::Void;

// LWOmxNode
status_t LWOmxNode::freeNode() {
    return toStatusT(mBase->freeNode());
}

status_t LWOmxNode::sendCommand(
        OMX_COMMANDTYPE cmd, OMX_S32 param) {
    return toStatusT(mBase->sendCommand(
            toRawCommandType(cmd), param));
}

status_t LWOmxNode::getParameter(
        OMX_INDEXTYPE index, void *params, size_t size) {
    hidl_vec<uint8_t> tParams = inHidlBytes(params, size);
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getParameter(
            toRawIndexType(index),
            tParams,
            [&fnStatus, params](
                    Status status, hidl_vec<uint8_t> const& outParams) {
                fnStatus = toStatusT(status);
                std::copy(
                        outParams.data(),
                        outParams.data() + outParams.size(),
                        static_cast<uint8_t*>(params));
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::setParameter(
        OMX_INDEXTYPE index, const void *params, size_t size) {
    hidl_vec<uint8_t> tParams = inHidlBytes(params, size);
    return toStatusT(mBase->setParameter(
            toRawIndexType(index), tParams));
}

status_t LWOmxNode::getConfig(
        OMX_INDEXTYPE index, void *params, size_t size) {
    hidl_vec<uint8_t> tParams = inHidlBytes(params, size);
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getConfig(
            toRawIndexType(index),
            tParams,
            [&fnStatus, params, size](
                    Status status, hidl_vec<uint8_t> const& outParams) {
                fnStatus = toStatusT(status);
                std::copy(
                        outParams.data(),
                        outParams.data() + size,
                        static_cast<uint8_t*>(params));
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::setConfig(
        OMX_INDEXTYPE index, const void *params, size_t size) {
    hidl_vec<uint8_t> tParams = inHidlBytes(params, size);
    return toStatusT(mBase->setConfig(toRawIndexType(index), tParams));
}

status_t LWOmxNode::setPortMode(
        OMX_U32 port_index, IOMX::PortMode mode) {
    return toStatusT(mBase->setPortMode(port_index, toHardwarePortMode(mode)));
}

status_t LWOmxNode::prepareForAdaptivePlayback(
        OMX_U32 portIndex, OMX_BOOL enable,
        OMX_U32 maxFrameWidth, OMX_U32 maxFrameHeight) {
    return toStatusT(mBase->prepareForAdaptivePlayback(
            portIndex, toRawBool(enable), maxFrameWidth, maxFrameHeight));
}

status_t LWOmxNode::configureVideoTunnelMode(
        OMX_U32 portIndex, OMX_BOOL tunneled,
        OMX_U32 audioHwSync, native_handle_t **sidebandHandle) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->configureVideoTunnelMode(
            portIndex,
            toRawBool(tunneled),
            audioHwSync,
            [&fnStatus, sidebandHandle](
                    Status status, hidl_handle const& outSidebandHandle) {
                fnStatus = toStatusT(status);
                *sidebandHandle = outSidebandHandle == nullptr ?
                        nullptr : native_handle_clone(outSidebandHandle);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::getGraphicBufferUsage(
        OMX_U32 portIndex, OMX_U32* usage) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getGraphicBufferUsage(
            portIndex,
            [&fnStatus, usage](
                    Status status, uint32_t outUsage) {
                fnStatus = toStatusT(status);
                *usage = outUsage;
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::setInputSurface(
        const sp<IOMXBufferSource> &bufferSource) {
    return toStatusT(mBase->setInputSurface(
            new TWOmxBufferSource(bufferSource)));
}

status_t LWOmxNode::allocateSecureBuffer(
        OMX_U32 portIndex, size_t size, buffer_id *buffer,
        void **buffer_data, sp<NativeHandle> *native_handle) {
    *buffer_data = nullptr;
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->allocateSecureBuffer(
            portIndex,
            static_cast<uint64_t>(size),
            [&fnStatus, buffer, native_handle](
                    Status status,
                    uint32_t outBuffer,
                    hidl_handle const& outNativeHandle) {
                fnStatus = toStatusT(status);
                *buffer = outBuffer;
                *native_handle = outNativeHandle.getNativeHandle() == nullptr ?
                        nullptr : NativeHandle::create(
                        native_handle_clone(outNativeHandle), true);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::useBuffer(
        OMX_U32 portIndex, const OMXBuffer &omxBuffer, buffer_id *buffer) {
    CodecBuffer codecBuffer;
    if (!wrapAs(&codecBuffer, omxBuffer)) {
        return BAD_VALUE;
    }
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->useBuffer(
            portIndex,
            codecBuffer,
            [&fnStatus, buffer](Status status, uint32_t outBuffer) {
                fnStatus = toStatusT(status);
                *buffer = outBuffer;
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::freeBuffer(
        OMX_U32 portIndex, buffer_id buffer) {
    return toStatusT(mBase->freeBuffer(portIndex, buffer));
}

status_t LWOmxNode::fillBuffer(
        buffer_id buffer, const OMXBuffer &omxBuffer, int fenceFd) {
    CodecBuffer codecBuffer;
    if (!wrapAs(&codecBuffer, omxBuffer)) {
        return BAD_VALUE;
    }
    native_handle_t* fenceNh = native_handle_create_from_fd(fenceFd);
    if (!fenceNh) {
        return NO_MEMORY;
    }
    status_t status = toStatusT(mBase->fillBuffer(
            buffer, codecBuffer, fenceNh));
    native_handle_close(fenceNh);
    native_handle_delete(fenceNh);
    return status;
}

status_t LWOmxNode::emptyBuffer(
        buffer_id buffer, const OMXBuffer &omxBuffer,
        OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {
    CodecBuffer codecBuffer;
    if (!wrapAs(&codecBuffer, omxBuffer)) {
        return BAD_VALUE;
    }
    native_handle_t* fenceNh = native_handle_create_from_fd(fenceFd);
    if (!fenceNh) {
        return NO_MEMORY;
    }
    status_t status = toStatusT(mBase->emptyBuffer(
            buffer,
            codecBuffer,
            flags,
            toRawTicks(timestamp),
            fenceNh));
    native_handle_close(fenceNh);
    native_handle_delete(fenceNh);
    return status;
}
status_t LWOmxNode::getExtensionIndex(
        const char *parameter_name,
        OMX_INDEXTYPE *index) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getExtensionIndex(
            hidl_string(parameter_name),
            [&fnStatus, index](Status status, uint32_t outIndex) {
                fnStatus = toStatusT(status);
                *index = toEnumIndexType(outIndex);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmxNode::dispatchMessage(const omx_message &lMsg) {
    Message tMsg;
    native_handle_t* nh;
    if (!wrapAs(&tMsg, &nh, lMsg)) {
        return NO_MEMORY;
    }
    status_t status = toStatusT(mBase->dispatchMessage(tMsg));
    native_handle_close(nh);
    native_handle_delete(nh);
    return status;
}

// TWOmxNode
TWOmxNode::TWOmxNode(sp<IOMXNode> const& base) : mBase(base) {
}

Return<Status> TWOmxNode::freeNode() {
    return toStatus(mBase->freeNode());
}

Return<Status> TWOmxNode::sendCommand(uint32_t cmd, int32_t param) {
    return toStatus(mBase->sendCommand(toEnumCommandType(cmd), param));
}

Return<void> TWOmxNode::getParameter(
        uint32_t index, hidl_vec<uint8_t> const& inParams,
        getParameter_cb _hidl_cb) {
    hidl_vec<uint8_t> params(inParams);
    Status status = toStatus(mBase->getParameter(
            toEnumIndexType(index),
            static_cast<void*>(params.data()),
            params.size()));
    _hidl_cb(status, params);
    return Void();
}

Return<Status> TWOmxNode::setParameter(
        uint32_t index, hidl_vec<uint8_t> const& inParams) {
    hidl_vec<uint8_t> params(inParams);
    return toStatus(mBase->setParameter(
            toEnumIndexType(index),
            static_cast<void const*>(params.data()),
            params.size()));
}

Return<void> TWOmxNode::getConfig(
        uint32_t index, const hidl_vec<uint8_t>& inConfig,
        getConfig_cb _hidl_cb) {
    hidl_vec<uint8_t> config(inConfig);
    Status status = toStatus(mBase->getConfig(
            toEnumIndexType(index),
            static_cast<void*>(config.data()),
            config.size()));
    _hidl_cb(status, config);
    return Void();
}

Return<Status> TWOmxNode::setConfig(
        uint32_t index, const hidl_vec<uint8_t>& inConfig) {
    hidl_vec<uint8_t> config(inConfig);
    return toStatus(mBase->setConfig(
            toEnumIndexType(index),
            static_cast<void const*>(config.data()),
            config.size()));
}

Return<Status> TWOmxNode::setPortMode(uint32_t portIndex, PortMode mode) {
    return toStatus(mBase->setPortMode(portIndex, toIOMXPortMode(mode)));
}

Return<Status> TWOmxNode::prepareForAdaptivePlayback(
        uint32_t portIndex, bool enable,
        uint32_t maxFrameWidth, uint32_t maxFrameHeight) {
    return toStatus(mBase->prepareForAdaptivePlayback(
            portIndex,
            toEnumBool(enable),
            maxFrameWidth,
            maxFrameHeight));
}

Return<void> TWOmxNode::configureVideoTunnelMode(
        uint32_t portIndex, bool tunneled, uint32_t audioHwSync,
        configureVideoTunnelMode_cb _hidl_cb) {
    native_handle_t* sidebandHandle = nullptr;
    Status status = toStatus(mBase->configureVideoTunnelMode(
            portIndex,
            toEnumBool(tunneled),
            audioHwSync,
            &sidebandHandle));
    _hidl_cb(status, hidl_handle(sidebandHandle));
    return Void();
}

Return<void> TWOmxNode::getGraphicBufferUsage(
        uint32_t portIndex, getGraphicBufferUsage_cb _hidl_cb) {
    OMX_U32 usage;
    Status status = toStatus(mBase->getGraphicBufferUsage(
            portIndex, &usage));
    _hidl_cb(status, usage);
    return Void();
}

Return<Status> TWOmxNode::setInputSurface(
        const sp<IOmxBufferSource>& bufferSource) {
    return toStatus(mBase->setInputSurface(new LWOmxBufferSource(
            bufferSource)));
}

Return<void> TWOmxNode::allocateSecureBuffer(
        uint32_t portIndex, uint64_t size,
        allocateSecureBuffer_cb _hidl_cb) {
    IOMX::buffer_id buffer;
    void* bufferData;
    sp<NativeHandle> nativeHandle;
    Status status = toStatus(mBase->allocateSecureBuffer(
            portIndex,
            static_cast<size_t>(size),
            &buffer,
            &bufferData,
            &nativeHandle));
    _hidl_cb(status, buffer, nativeHandle == nullptr ?
            nullptr : nativeHandle->handle());
    return Void();
}

Return<void> TWOmxNode::useBuffer(
        uint32_t portIndex, const CodecBuffer& codecBuffer,
        useBuffer_cb _hidl_cb) {
    IOMX::buffer_id buffer;
    OMXBuffer omxBuffer;
    if (!convertTo(&omxBuffer, codecBuffer)) {
        _hidl_cb(Status::BAD_VALUE, 0);
        return Void();
    }
    Status status = toStatus(mBase->useBuffer(
            portIndex, omxBuffer, &buffer));
    _hidl_cb(status, buffer);
    return Void();
}

Return<Status> TWOmxNode::freeBuffer(uint32_t portIndex, uint32_t buffer) {
    return toStatus(mBase->freeBuffer(portIndex, buffer));
}

Return<Status> TWOmxNode::fillBuffer(
        uint32_t buffer, const CodecBuffer& codecBuffer,
        const hidl_handle& fence) {
    OMXBuffer omxBuffer;
    if (!convertTo(&omxBuffer, codecBuffer)) {
        return Status::BAD_VALUE;
    }
    return toStatus(mBase->fillBuffer(
            buffer,
            omxBuffer,
            dup(native_handle_read_fd(fence))));
}

Return<Status> TWOmxNode::emptyBuffer(
        uint32_t buffer, const CodecBuffer& codecBuffer, uint32_t flags,
        uint64_t timestampUs, const hidl_handle& fence) {
    OMXBuffer omxBuffer;
    if (!convertTo(&omxBuffer, codecBuffer)) {
        return Status::BAD_VALUE;
    }
    return toStatus(mBase->emptyBuffer(
            buffer,
            omxBuffer,
            flags,
            toOMXTicks(timestampUs),
            dup(native_handle_read_fd(fence))));
}

Return<void> TWOmxNode::getExtensionIndex(
        const hidl_string& parameterName,
        getExtensionIndex_cb _hidl_cb) {
    OMX_INDEXTYPE index;
    Status status = toStatus(mBase->getExtensionIndex(
            parameterName.c_str(), &index));
    _hidl_cb(status, toRawIndexType(index));
    return Void();
}

Return<Status> TWOmxNode::dispatchMessage(const Message& tMsg) {
    omx_message lMsg;
    if (!convertTo(&lMsg, tMsg)) {
        return Status::BAD_VALUE;
    }
    return toStatus(mBase->dispatchMessage(lMsg));
}

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android
