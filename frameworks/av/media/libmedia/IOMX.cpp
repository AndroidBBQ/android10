/*
 * Copyright (c) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "IOMX"
#include <utils/Log.h>

#include <sys/mman.h>

#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <media/IOMX.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/openmax/OMX_IndexExt.h>
#include <media/OMXBuffer.h>
#include <utils/NativeHandle.h>

#include <media/omx/1.0/WOmxNode.h>
#include <android/IGraphicBufferSource.h>
#include <android/IOMXBufferSource.h>

namespace android {

enum {
    CONNECT = IBinder::FIRST_CALL_TRANSACTION,
    LIST_NODES,
    ALLOCATE_NODE,
    CREATE_INPUT_SURFACE,
    FREE_NODE,
    SEND_COMMAND,
    GET_PARAMETER,
    SET_PARAMETER,
    GET_CONFIG,
    SET_CONFIG,
    SET_PORT_MODE,
    SET_INPUT_SURFACE,
    PREPARE_FOR_ADAPTIVE_PLAYBACK,
    ALLOC_SECURE_BUFFER,
    USE_BUFFER,
    FREE_BUFFER,
    FILL_BUFFER,
    EMPTY_BUFFER,
    GET_EXTENSION_INDEX,
    OBSERVER_ON_MSG,
    GET_GRAPHIC_BUFFER_USAGE,
    CONFIGURE_VIDEO_TUNNEL_MODE,
    DISPATCH_MESSAGE,
    SET_QUIRKS,
};

class BpOMXNode : public BpInterface<IOMXNode> {
public:
    explicit BpOMXNode(const sp<IBinder> &impl)
        : BpInterface<IOMXNode>(impl) {
    }

    virtual status_t freeNode() {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        remote()->transact(FREE_NODE, data, &reply);

        return reply.readInt32();
    }

    virtual status_t sendCommand(
            OMX_COMMANDTYPE cmd, OMX_S32 param) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(cmd);
        data.writeInt32(param);
        remote()->transact(SEND_COMMAND, data, &reply);

        return reply.readInt32();
    }

    virtual status_t getParameter(
            OMX_INDEXTYPE index,
            void *params, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt64(size);
        data.write(params, size);
        remote()->transact(GET_PARAMETER, data, &reply);

        status_t err = reply.readInt32();
        if (err != OK) {
            return err;
        }

        reply.read(params, size);

        return OK;
    }

    virtual status_t setParameter(
            OMX_INDEXTYPE index,
            const void *params, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt64(size);
        data.write(params, size);
        remote()->transact(SET_PARAMETER, data, &reply);

        return reply.readInt32();
    }

    virtual status_t getConfig(
            OMX_INDEXTYPE index,
            void *params, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt64(size);
        data.write(params, size);
        remote()->transact(GET_CONFIG, data, &reply);

        status_t err = reply.readInt32();
        if (err != OK) {
            return err;
        }

        reply.read(params, size);

        return OK;
    }

    virtual status_t setConfig(
            OMX_INDEXTYPE index,
            const void *params, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt64(size);
        data.write(params, size);
        remote()->transact(SET_CONFIG, data, &reply);

        return reply.readInt32();
    }

    virtual status_t setPortMode(
            OMX_U32 port_index, IOMX::PortMode mode) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);
        data.writeInt32(mode);
        remote()->transact(SET_PORT_MODE, data, &reply);

        return reply.readInt32();
    }

    virtual status_t getGraphicBufferUsage(
            OMX_U32 port_index, OMX_U32* usage) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);
        remote()->transact(GET_GRAPHIC_BUFFER_USAGE, data, &reply);

        status_t err = reply.readInt32();
        *usage = reply.readInt32();
        return err;
    }

    virtual status_t useBuffer(
            OMX_U32 port_index, const OMXBuffer &omxBuf, buffer_id *buffer) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);

        status_t err = omxBuf.writeToParcel(&data);
        if (err != OK) {
            return err;
        }

        remote()->transact(USE_BUFFER, data, &reply);

        err = reply.readInt32();
        if (err != OK) {
            *buffer = 0;

            return err;
        }

        *buffer = (buffer_id)reply.readInt32();

        return err;
    }

    virtual status_t setInputSurface(
            const sp<IOMXBufferSource> &bufferSource) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());

        data.writeStrongBinder(IInterface::asBinder(bufferSource));

        status_t err = remote()->transact(SET_INPUT_SURFACE, data, &reply);

        if (err != OK) {
            ALOGW("binder transaction failed: %d", err);
            return err;
        }

        err = reply.readInt32();

        return err;
    }

    virtual status_t prepareForAdaptivePlayback(
            OMX_U32 port_index, OMX_BOOL enable,
            OMX_U32 max_width, OMX_U32 max_height) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);
        data.writeInt32((int32_t)enable);
        data.writeInt32(max_width);
        data.writeInt32(max_height);
        remote()->transact(PREPARE_FOR_ADAPTIVE_PLAYBACK, data, &reply);

        status_t err = reply.readInt32();
        return err;
    }

    virtual status_t configureVideoTunnelMode(
            OMX_U32 portIndex, OMX_BOOL tunneled,
            OMX_U32 audioHwSync, native_handle_t **sidebandHandle ) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(portIndex);
        data.writeInt32((int32_t)tunneled);
        data.writeInt32(audioHwSync);
        remote()->transact(CONFIGURE_VIDEO_TUNNEL_MODE, data, &reply);

        status_t err = reply.readInt32();
        if (err == OK && sidebandHandle) {
            *sidebandHandle = (native_handle_t *)reply.readNativeHandle();
        }
        return err;
    }


    virtual status_t allocateSecureBuffer(
            OMX_U32 port_index, size_t size,
            buffer_id *buffer, void **buffer_data, sp<NativeHandle> *native_handle) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);
        data.writeInt64(size);
        remote()->transact(ALLOC_SECURE_BUFFER, data, &reply);

        status_t err = reply.readInt32();
        if (err != OK) {
            *buffer = 0;
            *buffer_data = NULL;
            *native_handle = NULL;
            return err;
        }

        *buffer = (buffer_id)reply.readInt32();
        *buffer_data = (void *)reply.readInt64();
        if (*buffer_data == NULL) {
            *native_handle = NativeHandle::create(
                    reply.readNativeHandle(), true /* ownsHandle */);
        } else {
            *native_handle = NULL;
        }
        return err;
    }

    virtual status_t freeBuffer(
            OMX_U32 port_index, buffer_id buffer) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(port_index);
        data.writeInt32((int32_t)buffer);
        remote()->transact(FREE_BUFFER, data, &reply);

        return reply.readInt32();
    }

    virtual status_t fillBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf, int fenceFd) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32((int32_t)buffer);
        status_t err = omxBuf.writeToParcel(&data);
        if (err != OK) {
            return err;
        }
        data.writeInt32(fenceFd >= 0);
        if (fenceFd >= 0) {
            data.writeFileDescriptor(fenceFd, true /* takeOwnership */);
        }
        remote()->transact(FILL_BUFFER, data, &reply);

        return reply.readInt32();
    }

    virtual status_t emptyBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf,
            OMX_U32 flags, OMX_TICKS timestamp, int fenceFd) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32((int32_t)buffer);
        status_t err = omxBuf.writeToParcel(&data);
        if (err != OK) {
            return err;
        }
        data.writeInt32(flags);
        data.writeInt64(timestamp);
        data.writeInt32(fenceFd >= 0);
        if (fenceFd >= 0) {
            data.writeFileDescriptor(fenceFd, true /* takeOwnership */);
        }
        remote()->transact(EMPTY_BUFFER, data, &reply);

        return reply.readInt32();
    }

    virtual status_t getExtensionIndex(
            const char *parameter_name,
            OMX_INDEXTYPE *index) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeCString(parameter_name);

        remote()->transact(GET_EXTENSION_INDEX, data, &reply);

        status_t err = reply.readInt32();
        if (err == OK) {
            *index = static_cast<OMX_INDEXTYPE>(reply.readInt32());
        } else {
            *index = OMX_IndexComponentStartUnused;
        }

        return err;
    }

    virtual status_t dispatchMessage(const omx_message &msg) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(msg.fenceFd >= 0);
        if (msg.fenceFd >= 0) {
            data.writeFileDescriptor(msg.fenceFd, true /* takeOwnership */);
        }
        data.writeInt32(msg.type);
        data.write(&msg.u, sizeof(msg.u));

        remote()->transact(DISPATCH_MESSAGE, data, &reply);

        return reply.readInt32();
    }

    virtual status_t setQuirks(OMX_U32 quirks) {
        Parcel data, reply;
        data.writeInterfaceToken(IOMXNode::getInterfaceDescriptor());
        data.writeInt32(quirks);

        remote()->transact(SET_QUIRKS, data, &reply);

        return reply.readInt32();
    }
};

using ::android::hardware::media::omx::V1_0::utils::LWOmxNode;
class HpOMXNode : public HpInterface<BpOMXNode, LWOmxNode> {
public:
    HpOMXNode(const sp<IBinder>& base) : PBase(base) {}

    virtual status_t freeNode() {
        return mBase->freeNode();
    }

    virtual status_t sendCommand(
            OMX_COMMANDTYPE cmd, OMX_S32 param) {
        return mBase->sendCommand(cmd, param);
    }

    virtual status_t getParameter(
            OMX_INDEXTYPE index, void *params, size_t size) {
        return mBase->getParameter(index, params, size);
    }

    virtual status_t setParameter(
            OMX_INDEXTYPE index, const void *params, size_t size) {
        return mBase->setParameter(index, params, size);
    }

    virtual status_t getConfig(
            OMX_INDEXTYPE index, void *params, size_t size) {
        return mBase->getConfig(index, params, size);
    }

    virtual status_t setConfig(
            OMX_INDEXTYPE index, const void *params, size_t size) {
        return mBase->setConfig(index, params, size);
    }

    virtual status_t setPortMode(
            OMX_U32 port_index, IOMX::PortMode mode) {
        return mBase->setPortMode(port_index, mode);
    }

    virtual status_t prepareForAdaptivePlayback(
            OMX_U32 portIndex, OMX_BOOL enable,
            OMX_U32 maxFrameWidth, OMX_U32 maxFrameHeight) {
        return mBase->prepareForAdaptivePlayback(
                portIndex, enable, maxFrameWidth, maxFrameHeight);
    }

    virtual status_t configureVideoTunnelMode(
            OMX_U32 portIndex, OMX_BOOL tunneled,
            OMX_U32 audioHwSync, native_handle_t **sidebandHandle) {
        return mBase->configureVideoTunnelMode(
                portIndex, tunneled, audioHwSync, sidebandHandle);
    }

    virtual status_t getGraphicBufferUsage(
            OMX_U32 port_index, OMX_U32* usage) {
        return mBase->getGraphicBufferUsage(port_index, usage);
    }

    virtual status_t setInputSurface(
            const sp<IOMXBufferSource> &bufferSource) {
        return mBase->setInputSurface(bufferSource);
    }

    virtual status_t allocateSecureBuffer(
            OMX_U32 port_index, size_t size, buffer_id *buffer,
            void **buffer_data, sp<NativeHandle> *native_handle) {
        return mBase->allocateSecureBuffer(
                port_index, size, buffer, buffer_data, native_handle);
    }

    virtual status_t useBuffer(
            OMX_U32 port_index, const OMXBuffer &omxBuf, buffer_id *buffer) {
        return mBase->useBuffer(port_index, omxBuf, buffer);
    }

    virtual status_t freeBuffer(
            OMX_U32 port_index, buffer_id buffer) {
        return mBase->freeBuffer(port_index, buffer);
    }

    virtual status_t fillBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf, int fenceFd = -1) {
        return mBase->fillBuffer(buffer, omxBuf, fenceFd);
    }

    virtual status_t emptyBuffer(
            buffer_id buffer, const OMXBuffer &omxBuf,
            OMX_U32 flags, OMX_TICKS timestamp, int fenceFd = -1) {
        return mBase->emptyBuffer(buffer, omxBuf, flags, timestamp, fenceFd);
    }

    virtual status_t getExtensionIndex(
            const char *parameter_name,
            OMX_INDEXTYPE *index) {
        return mBase->getExtensionIndex(parameter_name, index);
    }

    virtual status_t dispatchMessage(const omx_message &msg) {
        return mBase->dispatchMessage(msg);
    }
};

IMPLEMENT_HYBRID_META_INTERFACE(OMXNode, "android.hardware.IOMXNode");

////////////////////////////////////////////////////////////////////////////////

#define CHECK_OMX_INTERFACE(interface, data, reply) \
        do { if (!(data).enforceInterface(interface::getInterfaceDescriptor())) { \
            ALOGW("Call incorrectly routed to " #interface); \
            return PERMISSION_DENIED; \
        } } while (0)

status_t BnOMXNode::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case FREE_NODE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            reply->writeInt32(freeNode());

            return NO_ERROR;
        }

        case SEND_COMMAND:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_COMMANDTYPE cmd =
                static_cast<OMX_COMMANDTYPE>(data.readInt32());

            OMX_S32 param = data.readInt32();
            reply->writeInt32(sendCommand(cmd, param));

            return NO_ERROR;
        }

        case GET_PARAMETER:
        case SET_PARAMETER:
        case GET_CONFIG:
        case SET_CONFIG:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_INDEXTYPE index = static_cast<OMX_INDEXTYPE>(data.readInt32());

            size_t size = data.readInt64();

            status_t err = NOT_ENOUGH_DATA;
            void *params = NULL;
            size_t pageSize = 0;
            size_t allocSize = 0;
            bool isUsageBits = (index == (OMX_INDEXTYPE) OMX_IndexParamConsumerUsageBits);
            if ((isUsageBits && size < 4) || (!isUsageBits && size < 8)) {
                // we expect the structure to contain at least the size and
                // version, 8 bytes total
                ALOGE("b/27207275 (%zu) (%d/%d)", size, int(index), int(code));
                android_errorWriteLog(0x534e4554, "27207275");
            } else {
                err = NO_MEMORY;
                pageSize = (size_t) sysconf(_SC_PAGE_SIZE);
                if (size > SIZE_MAX - (pageSize * 2)) {
                    ALOGE("requested param size too big");
                } else {
                    allocSize = (size + pageSize * 2) & ~(pageSize - 1);
                    params = mmap(NULL, allocSize, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1 /* fd */, 0 /* offset */);
                }
                if (params != MAP_FAILED && params != NULL) {
                    err = data.read(params, size);
                    if (err != OK) {
                        android_errorWriteLog(0x534e4554, "26914474");
                    } else {
                        err = NOT_ENOUGH_DATA;
                        OMX_U32 declaredSize = *(OMX_U32*)params;
                        if (index != (OMX_INDEXTYPE) OMX_IndexParamConsumerUsageBits &&
                                declaredSize > size) {
                            // the buffer says it's bigger than it actually is
                            ALOGE("b/27207275 (%u/%zu)", declaredSize, size);
                            android_errorWriteLog(0x534e4554, "27207275");
                        } else {
                            // mark the last page as inaccessible, to avoid exploitation
                            // of codecs that access past the end of the allocation because
                            // they didn't check the size
                            if (mprotect((char*)params + allocSize - pageSize, pageSize,
                                    PROT_NONE) != 0) {
                                ALOGE("mprotect failed: %s", strerror(errno));
                            } else {
                                switch (code) {
                                    case GET_PARAMETER:
                                        err = getParameter(index, params, size);
                                        break;
                                    case SET_PARAMETER:
                                        err = setParameter(index, params, size);
                                        break;
                                    case GET_CONFIG:
                                        err = getConfig(index, params, size);
                                        break;
                                    case SET_CONFIG:
                                        err = setConfig(index, params, size);
                                        break;
                                    default:
                                        TRESPASS();
                                }
                            }
                        }
                    }
                } else {
                    ALOGE("couldn't map: %s", strerror(errno));
                }
            }

            reply->writeInt32(err);

            if ((code == GET_PARAMETER || code == GET_CONFIG) && err == OK) {
                reply->write(params, size);
            }

            if (params) {
                munmap(params, allocSize);
            }
            params = NULL;

            return NO_ERROR;
        }

        case SET_PORT_MODE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);
            OMX_U32 port_index = data.readInt32();
            IOMX::PortMode mode = (IOMX::PortMode) data.readInt32();
            reply->writeInt32(setPortMode(port_index, mode));

            return NO_ERROR;
        }

        case GET_GRAPHIC_BUFFER_USAGE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();

            OMX_U32 usage = 0;
            status_t err = getGraphicBufferUsage(port_index, &usage);
            reply->writeInt32(err);
            reply->writeInt32(usage);

            return NO_ERROR;
        }

        case USE_BUFFER:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();

            OMXBuffer omxBuf;
            status_t err = omxBuf.readFromParcel(&data);
            if (err != OK) {
                return err;
            }

            buffer_id buffer;
            err = useBuffer(port_index, omxBuf, &buffer);
            reply->writeInt32(err);

            if (err == OK) {
                reply->writeInt32((int32_t)buffer);
            }

            return NO_ERROR;
        }

        case SET_INPUT_SURFACE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            sp<IOMXBufferSource> bufferSource =
                    interface_cast<IOMXBufferSource>(data.readStrongBinder());

            status_t err = setInputSurface(bufferSource);
            reply->writeInt32(err);

            return NO_ERROR;
        }

        case PREPARE_FOR_ADAPTIVE_PLAYBACK:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();
            OMX_BOOL enable = (OMX_BOOL)data.readInt32();
            OMX_U32 max_width = data.readInt32();
            OMX_U32 max_height = data.readInt32();

            status_t err = prepareForAdaptivePlayback(
                    port_index, enable, max_width, max_height);
            reply->writeInt32(err);

            return NO_ERROR;
        }

        case CONFIGURE_VIDEO_TUNNEL_MODE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();
            OMX_BOOL tunneled = (OMX_BOOL)data.readInt32();
            OMX_U32 audio_hw_sync = data.readInt32();

            native_handle_t *sideband_handle = NULL;
            status_t err = configureVideoTunnelMode(
                    port_index, tunneled, audio_hw_sync, &sideband_handle);
            reply->writeInt32(err);
            if(err == OK){
                reply->writeNativeHandle(sideband_handle);
            }

            return NO_ERROR;
        }

        case ALLOC_SECURE_BUFFER:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();
            if (!isSecure() || port_index != 0 /* kPortIndexInput */) {
                ALOGE("b/24310423");
                reply->writeInt32(INVALID_OPERATION);
                return NO_ERROR;
            }

            size_t size = data.readInt64();

            buffer_id buffer;
            void *buffer_data = NULL;
            sp<NativeHandle> native_handle;
            status_t err = allocateSecureBuffer(
                    port_index, size, &buffer, &buffer_data, &native_handle);
            reply->writeInt32(err);

            if (err == OK) {
                reply->writeInt32((int32_t)buffer);
                reply->writeInt64((uintptr_t)buffer_data);
                if (buffer_data == NULL) {
                    reply->writeNativeHandle(native_handle == NULL ? NULL : native_handle->handle());
                }
            }

            return NO_ERROR;
        }

        case FREE_BUFFER:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            OMX_U32 port_index = data.readInt32();
            buffer_id buffer = (buffer_id)data.readInt32();
            reply->writeInt32(freeBuffer(port_index, buffer));

            return NO_ERROR;
        }

        case FILL_BUFFER:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            buffer_id buffer = (buffer_id)data.readInt32();

            OMXBuffer omxBuf;
            status_t err = omxBuf.readFromParcel(&data);
            if (err != OK) {
                return err;
            }

            bool haveFence = data.readInt32();
            int fenceFd = haveFence ? ::dup(data.readFileDescriptor()) : -1;

            reply->writeInt32(fillBuffer(buffer, omxBuf, fenceFd));

            return NO_ERROR;
        }

        case EMPTY_BUFFER:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            buffer_id buffer = (buffer_id)data.readInt32();
            OMXBuffer omxBuf;
            status_t err = omxBuf.readFromParcel(&data);
            if (err != OK) {
                return err;
            }
            OMX_U32 flags = data.readInt32();
            OMX_TICKS timestamp = data.readInt64();
            bool haveFence = data.readInt32();
            int fenceFd = haveFence ? ::dup(data.readFileDescriptor()) : -1;
            reply->writeInt32(emptyBuffer(
                    buffer, omxBuf, flags, timestamp, fenceFd));

            return NO_ERROR;
        }

        case GET_EXTENSION_INDEX:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);

            const char *parameter_name = data.readCString();

            if (parameter_name == NULL) {
                ALOGE("b/26392700");
                reply->writeInt32(INVALID_OPERATION);
                return NO_ERROR;
            }

            OMX_INDEXTYPE index;
            status_t err = getExtensionIndex(parameter_name, &index);

            reply->writeInt32(err);

            if (err == OK) {
                reply->writeInt32(index);
            }

            return OK;
        }

        case DISPATCH_MESSAGE:
        {
            CHECK_OMX_INTERFACE(IOMXNode, data, reply);
            omx_message msg;
            int haveFence = data.readInt32();
            msg.fenceFd = haveFence ? ::dup(data.readFileDescriptor()) : -1;
            msg.type = (typeof(msg.type))data.readInt32();
            status_t err = data.read(&msg.u, sizeof(msg.u));

            if (err == OK) {
                err = dispatchMessage(msg);
            }
            reply->writeInt32(err);

            return NO_ERROR;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

////////////////////////////////////////////////////////////////////////////////

class BpOMXObserver : public BpInterface<IOMXObserver> {
public:
    explicit BpOMXObserver(const sp<IBinder> &impl)
        : BpInterface<IOMXObserver>(impl) {
    }

    virtual void onMessages(const std::list<omx_message> &messages) {
        Parcel data, reply;
        std::list<omx_message>::const_iterator it = messages.cbegin();
        if (messages.empty()) {
            return;
        }
        data.writeInterfaceToken(IOMXObserver::getInterfaceDescriptor());
        while (it != messages.cend()) {
            const omx_message &msg = *it++;
            data.writeInt32(msg.fenceFd >= 0);
            if (msg.fenceFd >= 0) {
                data.writeFileDescriptor(msg.fenceFd, true /* takeOwnership */);
            }
            data.writeInt32(msg.type);
            data.write(&msg.u, sizeof(msg.u));
            ALOGV("onMessage writing message %d, size %zu", msg.type, sizeof(msg));
        }
        data.writeInt32(-1); // mark end
        remote()->transact(OBSERVER_ON_MSG, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(OMXObserver, "android.hardware.IOMXObserver");

status_t BnOMXObserver::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case OBSERVER_ON_MSG:
        {
            CHECK_OMX_INTERFACE(IOMXObserver, data, reply);
            std::list<omx_message> messages;
            status_t err = FAILED_TRANSACTION; // must receive at least one message
            do {
                int haveFence = data.readInt32();
                if (haveFence < 0) { // we use -1 to mark end of messages
                    break;
                }
                omx_message msg;
                msg.fenceFd = haveFence ? ::dup(data.readFileDescriptor()) : -1;
                msg.type = (typeof(msg.type))data.readInt32();
                err = data.read(&msg.u, sizeof(msg.u));
                ALOGV("onTransact reading message %d, size %zu", msg.type, sizeof(msg));
                messages.push_back(msg);
            } while (err == OK);

            if (err == OK) {
                onMessages(messages);
            }

            return err;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}  // namespace android
