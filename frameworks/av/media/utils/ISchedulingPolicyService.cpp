/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "ISchedulingPolicyService"
//#define LOG_NDEBUG 0

#include <binder/Parcel.h>
#include "ISchedulingPolicyService.h"

namespace android {

// Keep in sync with frameworks/base/core/java/android/os/ISchedulingPolicyService.aidl
enum {
    REQUEST_PRIORITY_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION,
    REQUEST_CPUSET_BOOST,
};

// ----------------------------------------------------------------------

class BpSchedulingPolicyService : public BpInterface<ISchedulingPolicyService>
{
public:
    explicit BpSchedulingPolicyService(const sp<IBinder>& impl)
        : BpInterface<ISchedulingPolicyService>(impl)
    {
    }

    virtual int requestPriority(int32_t pid, int32_t tid,
                                int32_t prio, bool isForApp, bool asynchronous)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISchedulingPolicyService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeInt32(tid);
        data.writeInt32(prio);
        data.writeBool(isForApp);
        uint32_t flags = asynchronous ? IBinder::FLAG_ONEWAY : 0;
        status_t status = remote()->transact(REQUEST_PRIORITY_TRANSACTION, data, &reply, flags);
        if (status != NO_ERROR) {
            return status;
        }
        if (asynchronous) {
            return NO_ERROR;
        }
        // fail on exception: force binder reconnection
        if (reply.readExceptionCode() != 0) {
            return DEAD_OBJECT;
        }
        return reply.readInt32();
    }

    virtual int requestCpusetBoost(bool enable, const sp<IInterface>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISchedulingPolicyService::getInterfaceDescriptor());
        data.writeInt32(enable);
        data.writeStrongBinder(IInterface::asBinder(client));
        status_t status = remote()->transact(REQUEST_CPUSET_BOOST, data, &reply, 0);
        if (status != NO_ERROR) {
            return status;
        }
        // fail on exception: force binder reconnection
        if (reply.readExceptionCode() != 0) {
            return DEAD_OBJECT;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(SchedulingPolicyService, "android.os.ISchedulingPolicyService");

// ----------------------------------------------------------------------

status_t BnSchedulingPolicyService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
    case REQUEST_PRIORITY_TRANSACTION:
    case REQUEST_CPUSET_BOOST:
        // Not reached
        return NO_ERROR;
        break;
    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}

}   // namespace android
