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

#define LOG_TAG "AppOpsService"

#include <binder/IAppOpsService.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include <private/binder/Static.h>

namespace android {

// ----------------------------------------------------------------------

class BpAppOpsService : public BpInterface<IAppOpsService>
{
public:
    explicit BpAppOpsService(const sp<IBinder>& impl)
        : BpInterface<IAppOpsService>(impl)
    {
    }

    virtual int32_t checkOperation(int32_t code, int32_t uid, const String16& packageName) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeInt32(code);
        data.writeInt32(uid);
        data.writeString16(packageName);
        remote()->transact(CHECK_OPERATION_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) return MODE_ERRORED;
        return reply.readInt32();
    }

    virtual int32_t noteOperation(int32_t code, int32_t uid, const String16& packageName) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeInt32(code);
        data.writeInt32(uid);
        data.writeString16(packageName);
        remote()->transact(NOTE_OPERATION_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) return MODE_ERRORED;
        return reply.readInt32();
    }

    virtual int32_t startOperation(const sp<IBinder>& token, int32_t code, int32_t uid,
                const String16& packageName, bool startIfModeDefault) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeStrongBinder(token);
        data.writeInt32(code);
        data.writeInt32(uid);
        data.writeString16(packageName);
        data.writeInt32(startIfModeDefault ? 1 : 0);
        remote()->transact(START_OPERATION_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) return MODE_ERRORED;
        return reply.readInt32();
    }

    virtual void finishOperation(const sp<IBinder>& token, int32_t code, int32_t uid,
            const String16& packageName) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeStrongBinder(token);
        data.writeInt32(code);
        data.writeInt32(uid);
        data.writeString16(packageName);
        remote()->transact(FINISH_OPERATION_TRANSACTION, data, &reply);
    }

    virtual void startWatchingMode(int32_t op, const String16& packageName,
            const sp<IAppOpsCallback>& callback) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeInt32(op);
        data.writeString16(packageName);
        data.writeStrongBinder(IInterface::asBinder(callback));
        remote()->transact(START_WATCHING_MODE_TRANSACTION, data, &reply);
    }

    virtual void stopWatchingMode(const sp<IAppOpsCallback>& callback) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(callback));
        remote()->transact(STOP_WATCHING_MODE_TRANSACTION, data, &reply);
    }

    virtual sp<IBinder> getToken(const sp<IBinder>& clientToken) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeStrongBinder(clientToken);
        remote()->transact(GET_TOKEN_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) return nullptr;
        return reply.readStrongBinder();
    }


    virtual int32_t permissionToOpCode(const String16& permission) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeString16(permission);
        remote()->transact(PERMISSION_TO_OP_CODE_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) return -1;
        return reply.readInt32();
    }

    virtual int32_t checkAudioOperation(int32_t code, int32_t usage,
            int32_t uid, const String16& packageName) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
        data.writeInt32(code);
        data.writeInt32(usage);
        data.writeInt32(uid);
        data.writeString16(packageName);
        remote()->transact(CHECK_AUDIO_OPERATION_TRANSACTION, data, &reply);
        // fail on exception
        if (reply.readExceptionCode() != 0) {
            return MODE_ERRORED;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(AppOpsService, "com.android.internal.app.IAppOpsService");

// ----------------------------------------------------------------------

// NOLINTNEXTLINE(google-default-arguments)
status_t BnAppOpsService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    //printf("AppOpsService received: "); data.print();
    switch(code) {
        case CHECK_OPERATION_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            int32_t code = data.readInt32();
            int32_t uid = data.readInt32();
            String16 packageName = data.readString16();
            int32_t res = checkOperation(code, uid, packageName);
            reply->writeNoException();
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case NOTE_OPERATION_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            int32_t code = data.readInt32();
            int32_t uid = data.readInt32();
            String16 packageName = data.readString16();
            int32_t res = noteOperation(code, uid, packageName);
            reply->writeNoException();
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case START_OPERATION_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            sp<IBinder> token = data.readStrongBinder();
            int32_t code = data.readInt32();
            int32_t uid = data.readInt32();
            String16 packageName = data.readString16();
            bool startIfModeDefault = data.readInt32() == 1;
            int32_t res = startOperation(token, code, uid, packageName, startIfModeDefault);
            reply->writeNoException();
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case FINISH_OPERATION_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            sp<IBinder> token = data.readStrongBinder();
            int32_t code = data.readInt32();
            int32_t uid = data.readInt32();
            String16 packageName = data.readString16();
            finishOperation(token, code, uid, packageName);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case START_WATCHING_MODE_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            int32_t op = data.readInt32();
            String16 packageName = data.readString16();
            sp<IAppOpsCallback> callback = interface_cast<IAppOpsCallback>(data.readStrongBinder());
            startWatchingMode(op, packageName, callback);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case STOP_WATCHING_MODE_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            sp<IAppOpsCallback> callback = interface_cast<IAppOpsCallback>(data.readStrongBinder());
            stopWatchingMode(callback);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case GET_TOKEN_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            sp<IBinder> clientToken = data.readStrongBinder();
            sp<IBinder> token = getToken(clientToken);
            reply->writeNoException();
            reply->writeStrongBinder(token);
            return NO_ERROR;
        } break;
        case PERMISSION_TO_OP_CODE_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            String16 permission = data.readString16();
            const int32_t opCode = permissionToOpCode(permission);
            reply->writeNoException();
            reply->writeInt32(opCode);
            return NO_ERROR;
        } break;
        case CHECK_AUDIO_OPERATION_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsService, data, reply);
            const int32_t code = data.readInt32();
            const int32_t usage = data.readInt32();
            const int32_t uid = data.readInt32();
            const String16 packageName = data.readString16();
            const int32_t res = checkAudioOperation(code, usage, uid, packageName);
            reply->writeNoException();
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
