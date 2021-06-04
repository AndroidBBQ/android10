/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "IPowerManager"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <powermanager/IPowerManager.h>

namespace android {

class BpPowerManager : public BpInterface<IPowerManager>
{
public:
    explicit BpPowerManager(const sp<IBinder>& impl)
        : BpInterface<IPowerManager>(impl)
    {
    }

    virtual status_t acquireWakeLock(int flags, const sp<IBinder>& lock, const String16& tag,
            const String16& packageName, bool isOneWay)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());

        data.writeStrongBinder(lock);
        data.writeInt32(flags);
        data.writeString16(tag);
        data.writeString16(packageName);
        data.writeInt32(0); // no WorkSource
        data.writeString16(NULL, 0); // no history tag
        return remote()->transact(ACQUIRE_WAKE_LOCK, data, &reply,
                isOneWay ? IBinder::FLAG_ONEWAY : 0);
    }

    virtual status_t acquireWakeLockWithUid(int flags, const sp<IBinder>& lock, const String16& tag,
            const String16& packageName, int uid, bool isOneWay)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());

        data.writeStrongBinder(lock);
        data.writeInt32(flags);
        data.writeString16(tag);
        data.writeString16(packageName);
        data.writeInt32(uid); // uid to blame for the work
        return remote()->transact(ACQUIRE_WAKE_LOCK_UID, data, &reply,
                isOneWay ? IBinder::FLAG_ONEWAY : 0);
    }

    virtual status_t releaseWakeLock(const sp<IBinder>& lock, int flags, bool isOneWay)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeStrongBinder(lock);
        data.writeInt32(flags);
        return remote()->transact(RELEASE_WAKE_LOCK, data, &reply,
                isOneWay ? IBinder::FLAG_ONEWAY : 0);
    }

    virtual status_t updateWakeLockUids(const sp<IBinder>& lock, int len, const int *uids,
            bool isOneWay) {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeStrongBinder(lock);
        data.writeInt32Array(len, uids);
        return remote()->transact(UPDATE_WAKE_LOCK_UIDS, data, &reply,
                isOneWay ? IBinder::FLAG_ONEWAY : 0);
    }

    virtual status_t powerHint(int hintId, int param)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeInt32(hintId);
        data.writeInt32(param);
        // This FLAG_ONEWAY is in the .aidl, so there is no way to disable it
        return remote()->transact(POWER_HINT, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual status_t goToSleep(int64_t event_time_ms, int reason, int flags)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeInt64(event_time_ms);
        data.writeInt32(reason);
        data.writeInt32(flags);
        return remote()->transact(GO_TO_SLEEP, data, &reply, 0);
    }

    virtual status_t reboot(bool confirm, const String16& reason, bool wait)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeInt32(confirm);
        data.writeString16(reason);
        data.writeInt32(wait);
        return remote()->transact(REBOOT, data, &reply, 0);
    }

    virtual status_t shutdown(bool confirm, const String16& reason, bool wait)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeInt32(confirm);
        data.writeString16(reason);
        data.writeInt32(wait);
        return remote()->transact(SHUTDOWN, data, &reply, 0);
    }

    virtual status_t crash(const String16& message)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPowerManager::getInterfaceDescriptor());
        data.writeString16(message);
        return remote()->transact(CRASH, data, &reply, 0);
    }
};

IMPLEMENT_META_INTERFACE(PowerManager, "android.os.IPowerManager");

// ----------------------------------------------------------------------------

}; // namespace android
