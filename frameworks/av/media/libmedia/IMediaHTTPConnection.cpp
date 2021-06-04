/*
 * Copyright (C) 2013 The Android Open Source Project
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
#define LOG_TAG "IMediaHTTPConnection"
#include <utils/Log.h>

#include <media/IMediaHTTPConnection.h>

#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <utils/String8.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

enum {
    CONNECT = IBinder::FIRST_CALL_TRANSACTION,
    DISCONNECT,
    READ_AT,
    GET_SIZE,
    GET_MIME_TYPE,
    GET_URI
};

struct BpMediaHTTPConnection : public BpInterface<IMediaHTTPConnection> {
    explicit BpMediaHTTPConnection(const sp<IBinder> &impl)
        : BpInterface<IMediaHTTPConnection>(impl) {
    }

    virtual bool connect(
            const char *uri, const KeyedVector<String8, String8> *headers) {
        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        String16 tmp(uri);
        data.writeString16(tmp);

        tmp = String16("");
        if (headers != NULL) {
            for (size_t i = 0; i < headers->size(); ++i) {
                String16 key(headers->keyAt(i).string());
                String16 val(headers->valueAt(i).string());

                tmp.append(key);
                tmp.append(String16(": "));
                tmp.append(val);
                tmp.append(String16("\r\n"));
            }
        }
        data.writeString16(tmp);

        remote()->transact(CONNECT, data, &reply);

        int32_t exceptionCode = reply.readExceptionCode();

        if (exceptionCode) {
            return false;
        }

        sp<IBinder> binder = reply.readStrongBinder();
        mMemory = interface_cast<IMemory>(binder);

        return mMemory != NULL;
    }

    virtual void disconnect() {
        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        remote()->transact(DISCONNECT, data, &reply);
    }

    virtual ssize_t readAt(off64_t offset, void *buffer, size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        data.writeInt64(offset);
        data.writeInt32(size);

        status_t err = remote()->transact(READ_AT, data, &reply);
        if (err != OK) {
            ALOGE("remote readAt failed");
            return UNKNOWN_ERROR;
        }

        int32_t exceptionCode = reply.readExceptionCode();

        if (exceptionCode) {
            return UNKNOWN_ERROR;
        }

        int32_t lenOrErrorCode = reply.readInt32();

        // Negative values are error codes
        if (lenOrErrorCode < 0) {
            return lenOrErrorCode;
        }

        size_t len = lenOrErrorCode;

        if (len > size) {
            ALOGE("requested %zu, got %zu", size, len);
            return ERROR_OUT_OF_RANGE;
        }
        if (len > mMemory->size()) {
            ALOGE("got %zu, but memory has %zu", len, mMemory->size());
            return ERROR_OUT_OF_RANGE;
        }
        if(buffer == NULL) {
           ALOGE("readAt got a NULL buffer");
           return UNKNOWN_ERROR;
        }
        if (mMemory->pointer() == NULL) {
           ALOGE("readAt got a NULL mMemory->pointer()");
           return UNKNOWN_ERROR;
        }

        memcpy(buffer, mMemory->pointer(), len);

        return len;
    }

    virtual off64_t getSize() {
        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        remote()->transact(GET_SIZE, data, &reply);

        int32_t exceptionCode = reply.readExceptionCode();

        if (exceptionCode) {
            return UNKNOWN_ERROR;
        }

        return reply.readInt64();
    }

    virtual status_t getMIMEType(String8 *mimeType) {
        *mimeType = String8("");

        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        remote()->transact(GET_MIME_TYPE, data, &reply);

        int32_t exceptionCode = reply.readExceptionCode();

        if (exceptionCode) {
            return UNKNOWN_ERROR;
        }

        *mimeType = String8(reply.readString16());

        return OK;
    }

    virtual status_t getUri(String8 *uri) {
        *uri = String8("");

        Parcel data, reply;
        data.writeInterfaceToken(
                IMediaHTTPConnection::getInterfaceDescriptor());

        remote()->transact(GET_URI, data, &reply);

        int32_t exceptionCode = reply.readExceptionCode();

        if (exceptionCode) {
            return UNKNOWN_ERROR;
        }

        *uri = String8(reply.readString16());

        return OK;
    }

private:
    sp<IMemory> mMemory;
};

IMPLEMENT_META_INTERFACE(
        MediaHTTPConnection, "android.media.IMediaHTTPConnection");

} // namespace android
