/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "IDataSource"
#include <utils/Log.h>
#include <utils/Timers.h>

#include <media/IDataSource.h>

#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <drm/drm_framework_common.h>
#include <media/stagefright/foundation/ADebug.h>

namespace android {

enum {
    GET_IMEMORY = IBinder::FIRST_CALL_TRANSACTION,
    READ_AT,
    GET_SIZE,
    CLOSE,
    GET_FLAGS,
    TO_STRING,
    DRM_INITIALIZATION,
};

struct BpDataSource : public BpInterface<IDataSource> {
    explicit BpDataSource(const sp<IBinder>& impl)
        : BpInterface<IDataSource>(impl) {}

    virtual sp<IMemory> getIMemory() {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        remote()->transact(GET_IMEMORY, data, &reply);
        sp<IBinder> binder = reply.readStrongBinder();
        return interface_cast<IMemory>(binder);
    }

    virtual ssize_t readAt(off64_t offset, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        data.writeInt64(offset);
        data.writeInt64(size);
        status_t err = remote()->transact(READ_AT, data, &reply);
        if (err != OK) {
            return err;
        }
        int64_t value = 0;
        err = reply.readInt64(&value);
        if (err != OK) {
            return err;
        }
        return (ssize_t)value;
    }

    virtual status_t getSize(off64_t* size) {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        remote()->transact(GET_SIZE, data, &reply);
        status_t err = reply.readInt32();
        *size = reply.readInt64();
        return err;
    }

    virtual void close() {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        remote()->transact(CLOSE, data, &reply);
    }

    virtual uint32_t getFlags() {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        remote()->transact(GET_FLAGS, data, &reply);
        return reply.readUint32();
    }

    virtual String8 toString() {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        remote()->transact(TO_STRING, data, &reply);
        return reply.readString8();
    }

    virtual sp<DecryptHandle> DrmInitialization(const char *mime) {
        Parcel data, reply;
        data.writeInterfaceToken(IDataSource::getInterfaceDescriptor());
        if (mime == NULL) {
            data.writeInt32(0);
        } else {
            data.writeInt32(1);
            data.writeCString(mime);
        }
        remote()->transact(DRM_INITIALIZATION, data, &reply);
        sp<DecryptHandle> handle;
        if (reply.dataAvail() != 0) {
            handle = new DecryptHandle();
            handle->decryptId = reply.readInt32();
            handle->mimeType = reply.readString8();
            handle->decryptApiType = reply.readInt32();
            handle->status = reply.readInt32();

            const int bufferLength = data.readInt32();
            if (bufferLength != -1) {
                handle->decryptInfo = new DecryptInfo();
                handle->decryptInfo->decryptBufferLength = bufferLength;
            }

            size_t size = data.readInt32();
            for (size_t i = 0; i < size; ++i) {
                DrmCopyControl key = (DrmCopyControl)data.readInt32();
                int value = data.readInt32();
                handle->copyControlVector.add(key, value);
            }

            size = data.readInt32();
            for (size_t i = 0; i < size; ++i) {
                String8 key = data.readString8();
                String8 value = data.readString8();
                handle->extendedData.add(key, value);
            }
        }
        return handle;
    }
};

IMPLEMENT_META_INTERFACE(DataSource, "android.media.IDataSource");

status_t BnDataSource::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    switch (code) {
        case GET_IMEMORY: {
            CHECK_INTERFACE(IDataSource, data, reply);
            reply->writeStrongBinder(IInterface::asBinder(getIMemory()));
            return NO_ERROR;
        } break;
        case READ_AT: {
            CHECK_INTERFACE(IDataSource, data, reply);
            off64_t offset = (off64_t) data.readInt64();
            size_t size = (size_t) data.readInt64();
            reply->writeInt64(readAt(offset, size));
            return NO_ERROR;
        } break;
        case GET_SIZE: {
            CHECK_INTERFACE(IDataSource, data, reply);
            off64_t size;
            status_t err = getSize(&size);
            reply->writeInt32(err);
            reply->writeInt64(size);
            return NO_ERROR;
        } break;
        case CLOSE: {
            CHECK_INTERFACE(IDataSource, data, reply);
            close();
            return NO_ERROR;
        } break;
        case GET_FLAGS: {
            CHECK_INTERFACE(IDataSource, data, reply);
            reply->writeUint32(getFlags());
            return NO_ERROR;
        } break;
        case TO_STRING: {
            CHECK_INTERFACE(IDataSource, data, reply);
            reply->writeString8(toString());
            return NO_ERROR;
        } break;
        case DRM_INITIALIZATION: {
            CHECK_INTERFACE(IDataSource, data, reply);
            const char *mime = NULL;
            const int32_t flag = data.readInt32();
            if (flag != 0) {
                mime = data.readCString();
            }
            sp<DecryptHandle> handle = DrmInitialization(mime);
            if (handle != NULL) {
                reply->writeInt32(handle->decryptId);
                reply->writeString8(handle->mimeType);
                reply->writeInt32(handle->decryptApiType);
                reply->writeInt32(handle->status);

                if (handle->decryptInfo != NULL) {
                    reply->writeInt32(handle->decryptInfo->decryptBufferLength);
                } else {
                    reply->writeInt32(-1);
                }

                size_t size = handle->copyControlVector.size();
                reply->writeInt32(size);
                for (size_t i = 0; i < size; ++i) {
                    reply->writeInt32(handle->copyControlVector.keyAt(i));
                    reply->writeInt32(handle->copyControlVector.valueAt(i));
                }

                size = handle->extendedData.size();
                reply->writeInt32(size);
                for (size_t i = 0; i < size; ++i) {
                    reply->writeString8(handle->extendedData.keyAt(i));
                    reply->writeString8(handle->extendedData.valueAt(i));
                }
            }
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}  // namespace android
