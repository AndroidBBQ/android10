/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "ResultReceiver"

#include <binder/IResultReceiver.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include <private/binder/Static.h>

namespace android {

// ----------------------------------------------------------------------

class BpResultReceiver : public BpInterface<IResultReceiver>
{
public:
    explicit BpResultReceiver(const sp<IBinder>& impl)
        : BpInterface<IResultReceiver>(impl)
    {
    }

    virtual void send(int32_t resultCode) {
        Parcel data;
        data.writeInterfaceToken(IResultReceiver::getInterfaceDescriptor());
        data.writeInt32(resultCode);
        remote()->transact(OP_SEND, data, nullptr, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(ResultReceiver, "com.android.internal.os.IResultReceiver");

// ----------------------------------------------------------------------

// NOLINTNEXTLINE(google-default-arguments)
status_t BnResultReceiver::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case OP_SEND: {
            CHECK_INTERFACE(IResultReceiver, data, reply);
            int32_t resultCode = data.readInt32();
            send(resultCode);
            if (reply != nullptr) {
                reply->writeNoException();
            }
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
