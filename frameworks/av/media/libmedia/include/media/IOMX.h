/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_IOMX_H_

#define ANDROID_IOMX_H_

#include <binder/IInterface.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <cutils/native_handle.h>

#include <list>

#include <hidl/HybridInterface.h>
#include <media/hardware/MetadataBufferType.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>

#include <media/openmax/OMX_Core.h>
#include <media/openmax/OMX_Video.h>
#include <media/openmax/OMX_VideoExt.h>

namespace android {

class IGraphicBufferProducer;
class IGraphicBufferSource;
class IMemory;
class IOMXBufferSource;
class IOMXNode;
class IOMXObserver;
class NativeHandle;
class OMXBuffer;
struct omx_message;

using hardware::media::omx::V1_0::IOmxNode;

class IOMX : public RefBase {
public:

    typedef uint32_t buffer_id;

    enum {
        kFenceTimeoutMs = 1000
    };

    enum PortMode {
        kPortModePresetStart = 0,
        kPortModePresetByteBuffer,
        kPortModePresetANWBuffer,
        kPortModePresetSecureBuffer,
        kPortModePresetEnd,

        kPortModeDynamicStart = 100,
        kPortModeDynamicANWBuffer,      // uses metadata mode kMetadataBufferTypeANWBuffer
                                        // or kMetadataBufferTypeGrallocSource
        kPortModeDynamicNativeHandle,   // uses metadata mode kMetadataBufferTypeNativeHandleSource
        kPortModeDynamicEnd,
    };

    struct ComponentInfo {
        String8 mName;
        List<String8> mRoles;
    };
    virtual status_t listNodes(List<ComponentInfo> *list) = 0;

    virtual status_t allocateNode(
            const char *name, const sp<IOMXObserver> &observer,
            sp<IOMXNode> *omxNode) = 0;

    virtual status_t createInputSurface(
            sp<IGraphicBufferProducer> *bufferProducer,
            sp<IGraphicBufferSource> *bufferSource) = 0;
};

class IOMXNode : public IInterface {
public:
    DECLARE_HYBRID_META_INTERFACE(OMXNode, IOmxNode);

    typedef IOMX::buffer_id buffer_id;

    virtual status_t freeNode() = 0;

    virtual status_t sendCommand(
            OMX_COMMANDTYPE cmd, OMX_S32 param) = 0;

    virtual status_t getParameter(
            OMX_INDEXTYPE index, void *params, size_t size) = 0;

    virtual status_t setParameter(
            OMX_INDEXTYPE index, const void *params, size_t size) = 0;

    virtual status_t getConfig(
            OMX_INDEXTYPE index, void *params, size_t size) = 0;

    virtual status_t setConfig(
            OMX_INDEXTYPE index, const void *params, size_t size) = 0;

    virtual status_t setPortMode(
            OMX_U32 port_index, IOMX::PortMode mode) = 0;

    virtual status_t prepareForAdaptivePlayback(
            OMX_U32 portIndex, OMX_BOOL enable,
            OMX_U32 maxFrameWidth, OMX_U32 maxFrameHeight) = 0;

    virtual status_t configureVideoTunnelMode(
            OMX_U32 portIndex, OMX_BOOL tunneled,
            OMX_U32 audioHwSync, native_handle_t **sidebandHandle) = 0;

    virtual status_t getGraphicBufferUsage(
            OMX_U32 port_index, OMX_U32* usage) = 0;

    virtual status_t setInputSurface(
            const sp<IOMXBufferSource> &bufferSource) = 0;

    // Allocate an opaque buffer as a native handle. If component supports returning native
    // handles, those are returned in *native_handle. Otherwise, the allocated buffer is
    // returned in *buffer_data. This clearly only makes sense if the caller lives in the
    // same process as the callee, i.e. is the media_server, as the returned "buffer_data"
    // pointer is just that, a pointer into local address space.
    virtual status_t allocateSecureBuffer(
            OMX_U32 port_index, size_t size, buffer_id *buffer,
            void **buffer_data, sp<NativeHandle> *native_handle) = 0;

    // Instructs the component to use the buffer passed in via |omxBuf| on the
    // specified port. Returns in |*buffer| the buffer id that the component
    // assigns to this buffer. |omxBuf| must be one of:
    // 1) OMXBuffer::sPreset for meta-mode,
    // 2) type kBufferTypeANWBuffer for non-meta-graphic buffer mode,
    // 3) type kBufferTypeSharedMem for bytebuffer mode.
    virtual status_t useBuffer(
            OMX_U32 port_index, const OMXBuffer &omxBuf, buffer_id *buffer) = 0;

    // Frees the buffer on the specified port with buffer id |buffer|.
    virtual status_t freeBuffer(
            OMX_U32 port_index, buffer_id buffer) = 0;

    // Calls OMX_FillBuffer on buffer. Passes |fenceFd| to component if it
    // supports fences. Otherwise, it waits on |fenceFd| before calling
    // OMX_FillBuffer. Takes ownership of |fenceFd| even if this call fails.
    // If the port is in metadata mode, the buffer will be updated to point
    // to the new buffer passed in via |omxBuf| before OMX_FillBuffer is called.
    // Otherwise info in the |omxBuf| is not used.
    virtual status_t fillBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf, int fenceFd = -1) = 0;

    // Calls OMX_EmptyBuffer on buffer. Passes |fenceFd| to component if it
    // supports fences. Otherwise, it waits on |fenceFd| before calling
    // OMX_EmptyBuffer. Takes ownership of |fenceFd| even if this call fails.
    // If the port is in metadata mode, the buffer will be updated to point
    // to the new buffer passed in via |omxBuf| before OMX_EmptyBuffer is called.
    virtual status_t emptyBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf,
            OMX_U32 flags, OMX_TICKS timestamp, int fenceFd = -1) = 0;

    virtual status_t getExtensionIndex(
            const char *parameter_name,
            OMX_INDEXTYPE *index) = 0;

    virtual status_t dispatchMessage(const omx_message &msg) = 0;
};

struct omx_message {
    enum {
        EVENT,
        EMPTY_BUFFER_DONE,
        FILL_BUFFER_DONE,
        FRAME_RENDERED,
    } type;

    int fenceFd; // used for EMPTY_BUFFER_DONE and FILL_BUFFER_DONE; client must close this

    union {
        // if type == EVENT
        struct {
            OMX_EVENTTYPE event;
            OMX_U32 data1;
            OMX_U32 data2;
            OMX_U32 data3;
            OMX_U32 data4;
        } event_data;

        // if type == EMPTY_BUFFER_DONE
        struct {
            IOMX::buffer_id buffer;
        } buffer_data;

        // if type == FILL_BUFFER_DONE
        struct {
            IOMX::buffer_id buffer;
            OMX_U32 range_offset;
            OMX_U32 range_length;
            OMX_U32 flags;
            OMX_TICKS timestamp;
        } extended_buffer_data;

        // if type == FRAME_RENDERED
        struct {
            OMX_TICKS timestamp;
            OMX_S64 nanoTime;
        } render_data;
    } u;
};

class IOMXObserver : public IInterface {
public:
    DECLARE_META_INTERFACE(OMXObserver);

    // Handle (list of) messages.
    virtual void onMessages(const std::list<omx_message> &messages) = 0;
};

////////////////////////////////////////////////////////////////////////////////
class BnOMXNode : public BnInterface<IOMXNode> {
public:
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);

protected:
    // check if the codec is secure.
    virtual bool isSecure() const {
        return false;
    }
};

class BnOMXObserver : public BnInterface<IOMXObserver> {
public:
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);
};

}  // namespace android

#endif  // ANDROID_IOMX_H_
