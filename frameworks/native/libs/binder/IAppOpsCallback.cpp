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

#define LOG_TAG "AppOpsCallback"

#include <binder/IAppOpsCallback.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include <private/binder/Static.h>

namespace android {

// ----------------------------------------------------------------------

class BpAppOpsCallback : public BpInterface<IAppOpsCallback>
{
public:
    explicit BpAppOpsCallback(const sp<IBinder>& impl)
        : BpInterface<IAppOpsCallback>(impl)
    {
    }

    virtual void opChanged(int32_t op, const String16& packageName) {
        Parcel data, reply;
        data.writeInterfaceToken(IAppOpsCallback::getInterfaceDescriptor());
        data.writeInt32(op);
        data.writeString16(packageName);
        remote()->transact(OP_CHANGED_TRANSACTION, data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(AppOpsCallback, "com.android.internal.app.IAppOpsCallback");

// ----------------------------------------------------------------------

// NOLINTNEXTLINE(google-default-arguments)
status_t BnAppOpsCallback::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case OP_CHANGED_TRANSACTION: {
            CHECK_INTERFACE(IAppOpsCallback, data, reply);
            int32_t op = data.readInt32();
            String16 packageName;
            (void)data.readString16(&packageName);
            opChanged(op, packageName);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
