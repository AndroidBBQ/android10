/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <binder/IUidObserver.h>

#include <binder/Parcel.h>

namespace android {

// ------------------------------------------------------------------------------------

class BpUidObserver : public BpInterface<IUidObserver>
{
public:
    explicit BpUidObserver(const sp<IBinder>& impl)
        : BpInterface<IUidObserver>(impl)
    {
    }

    virtual void onUidGone(uid_t uid, bool disabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IUidObserver::getInterfaceDescriptor());
        data.writeInt32((int32_t) uid);
        data.writeInt32(disabled ? 1 : 0);
        remote()->transact(ON_UID_GONE_TRANSACTION, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual void onUidActive(uid_t uid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IUidObserver::getInterfaceDescriptor());
        data.writeInt32((int32_t) uid);
        remote()->transact(ON_UID_ACTIVE_TRANSACTION, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual void onUidIdle(uid_t uid, bool disabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IUidObserver::getInterfaceDescriptor());
        data.writeInt32((int32_t) uid);
        data.writeInt32(disabled ? 1 : 0);
        remote()->transact(ON_UID_IDLE_TRANSACTION, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual void onUidStateChanged(uid_t uid, int32_t procState, int64_t procStateSeq)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IUidObserver::getInterfaceDescriptor());
        data.writeInt32((int32_t) uid);
        data.writeInt32(procState);
        data.writeInt64(procStateSeq);
        remote()->transact(ON_UID_STATE_CHANGED_TRANSACTION, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

// ----------------------------------------------------------------------

IMPLEMENT_META_INTERFACE(UidObserver, "android.app.IUidObserver");

// ----------------------------------------------------------------------

status_t BnUidObserver::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ON_UID_GONE_TRANSACTION: {
            CHECK_INTERFACE(IUidObserver, data, reply);
            uid_t uid = data.readInt32();
            bool disabled = data.readInt32() == 1;
            onUidGone(uid, disabled);
            return NO_ERROR;
        } break;

        case ON_UID_ACTIVE_TRANSACTION: {
            CHECK_INTERFACE(IUidObserver, data, reply);
            uid_t uid = data.readInt32();
            onUidActive(uid);
            return NO_ERROR;
        } break;

        case ON_UID_IDLE_TRANSACTION: {
            CHECK_INTERFACE(IUidObserver, data, reply);
            uid_t uid = data.readInt32();
            bool disabled = data.readInt32() == 1;
            onUidIdle(uid, disabled);
            return NO_ERROR;
        } break;
        case ON_UID_STATE_CHANGED_TRANSACTION: {
            CHECK_INTERFACE(IUidObserver, data, reply);
            uid_t uid = data.readInt32();
            int32_t procState = data.readInt32();
            int64_t procStateSeq = data.readInt64();
            onUidStateChanged(uid, procState, procStateSeq);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
