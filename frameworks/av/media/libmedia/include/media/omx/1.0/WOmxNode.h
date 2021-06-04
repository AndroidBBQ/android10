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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXNODE_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXNODE_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <utils/Errors.h>
#include <media/IOMX.h>
#include <hidl/HybridInterface.h>

#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <android/hardware/media/omx/1.0/IOmxObserver.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::media::omx::V1_0::CodecBuffer;
using ::android::hardware::media::omx::V1_0::IOmxBufferSource;
using ::android::hardware::media::omx::V1_0::IOmxNode;
using ::android::hardware::media::omx::V1_0::IOmxObserver;
using ::android::hardware::media::omx::V1_0::Message;
using ::android::hardware::media::omx::V1_0::PortMode;
using ::android::hardware::media::omx::V1_0::Status;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

/**
 * Wrapper classes for conversion
 * ==============================
 *
 * Naming convention:
 * - LW = Legacy Wrapper --- It wraps a Treble object inside a legacy object.
 * - TW = Treble Wrapper --- It wraps a legacy object inside a Treble object.
 */

struct LWOmxNode : public H2BConverter<IOmxNode, BnOMXNode> {
    LWOmxNode(sp<IOmxNode> const& base) : CBase(base) {}
    status_t freeNode() override;
    status_t sendCommand(
            OMX_COMMANDTYPE cmd, OMX_S32 param) override;
    status_t getParameter(
            OMX_INDEXTYPE index, void *params, size_t size) override;
    status_t setParameter(
            OMX_INDEXTYPE index, const void *params, size_t size) override;
    status_t getConfig(
            OMX_INDEXTYPE index, void *params, size_t size) override;
    status_t setConfig(
            OMX_INDEXTYPE index, const void *params, size_t size) override;
    status_t setPortMode(
            OMX_U32 port_index, IOMX::PortMode mode) override;
    status_t prepareForAdaptivePlayback(
            OMX_U32 portIndex, OMX_BOOL enable,
            OMX_U32 maxFrameWidth, OMX_U32 maxFrameHeight) override;
    status_t configureVideoTunnelMode(
            OMX_U32 portIndex, OMX_BOOL tunneled,
            OMX_U32 audioHwSync, native_handle_t **sidebandHandle) override;
    status_t getGraphicBufferUsage(
            OMX_U32 port_index, OMX_U32* usage) override;
    status_t setInputSurface(
            const sp<IOMXBufferSource> &bufferSource) override;
    status_t allocateSecureBuffer(
            OMX_U32 port_index, size_t size, buffer_id *buffer,
            void **buffer_data, sp<NativeHandle> *native_handle) override;
    status_t useBuffer(
            OMX_U32 port_index, const OMXBuffer &omxBuf,
            buffer_id *buffer) override;
    status_t freeBuffer(
            OMX_U32 port_index, buffer_id buffer) override;
    status_t fillBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf,
            int fenceFd = -1) override;
    status_t emptyBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf,
            OMX_U32 flags, OMX_TICKS timestamp, int fenceFd = -1) override;
    status_t getExtensionIndex(
            const char *parameter_name,
            OMX_INDEXTYPE *index) override;
    status_t dispatchMessage(const omx_message &msg) override;
};

struct TWOmxNode : public IOmxNode {
    sp<IOMXNode> mBase;
    TWOmxNode(sp<IOMXNode> const& base);

    Return<Status> freeNode() override;
    Return<Status> sendCommand(uint32_t cmd, int32_t param) override;
    Return<void> getParameter(
            uint32_t index, hidl_vec<uint8_t> const& inParams,
            getParameter_cb _hidl_cb) override;
    Return<Status> setParameter(
            uint32_t index, hidl_vec<uint8_t> const& params) override;
    Return<void> getConfig(
            uint32_t index, hidl_vec<uint8_t> const& inConfig,
            getConfig_cb _hidl_cb) override;
    Return<Status> setConfig(
            uint32_t index, hidl_vec<uint8_t> const& config) override;
    Return<Status> setPortMode(uint32_t portIndex, PortMode mode) override;
    Return<Status> prepareForAdaptivePlayback(
            uint32_t portIndex, bool enable,
            uint32_t maxFrameWidth, uint32_t maxFrameHeight) override;
    Return<void> configureVideoTunnelMode(
            uint32_t portIndex, bool tunneled, uint32_t audioHwSync,
            configureVideoTunnelMode_cb _hidl_cb) override;
    Return<void> getGraphicBufferUsage(
            uint32_t portIndex,
            getGraphicBufferUsage_cb _hidl_cb) override;
    Return<Status> setInputSurface(
            sp<IOmxBufferSource> const& bufferSource) override;
    Return<void> allocateSecureBuffer(
            uint32_t portIndex, uint64_t size,
            allocateSecureBuffer_cb _hidl_cb) override;
    Return<void> useBuffer(
            uint32_t portIndex, CodecBuffer const& codecBuffer,
            useBuffer_cb _hidl_cb) override;
    Return<Status> freeBuffer(uint32_t portIndex, uint32_t buffer) override;
    Return<Status> fillBuffer(
            uint32_t buffer, CodecBuffer const& codecBuffer,
            const hidl_handle& fence) override;
    Return<Status> emptyBuffer(
            uint32_t buffer, CodecBuffer const& codecBuffer,
            uint32_t flags, uint64_t timestampUs,
            hidl_handle const& fence) override;
    Return<void> getExtensionIndex(
            hidl_string const& parameterName,
            getExtensionIndex_cb _hidl_cb) override;
    Return<Status> dispatchMessage(Message const& msg) override;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXNODE_H
