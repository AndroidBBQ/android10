/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "IMediaLogService"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <media/IMediaLogService.h>

namespace android {

enum {
    REGISTER_WRITER = IBinder::FIRST_CALL_TRANSACTION,
    UNREGISTER_WRITER,
    REQUEST_MERGE_WAKEUP,
};

class BpMediaLogService : public BpInterface<IMediaLogService>
{
public:
    explicit BpMediaLogService(const sp<IBinder>& impl)
        : BpInterface<IMediaLogService>(impl)
    {
    }

    virtual void    registerWriter(const sp<IMemory>& shared, size_t size, const char *name) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaLogService::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(shared));
        data.writeInt64((int64_t) size);
        data.writeCString(name);
        status_t status __unused = remote()->transact(REGISTER_WRITER, data, &reply);
        // FIXME ignores status
    }

    virtual void    unregisterWriter(const sp<IMemory>& shared) {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaLogService::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(shared));
        status_t status __unused = remote()->transact(UNREGISTER_WRITER, data, &reply);
        // FIXME ignores status
    }

    virtual void    requestMergeWakeup() {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaLogService::getInterfaceDescriptor());
        status_t status __unused = remote()->transact(REQUEST_MERGE_WAKEUP, data, &reply);
        // FIXME ignores status
    }

};

IMPLEMENT_META_INTERFACE(MediaLogService, "android.media.IMediaLogService");

// ----------------------------------------------------------------------

status_t BnMediaLogService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {

        case REGISTER_WRITER: {
            CHECK_INTERFACE(IMediaLogService, data, reply);
            sp<IMemory> shared = interface_cast<IMemory>(data.readStrongBinder());
            size_t size = (size_t) data.readInt64();
            const char *name = data.readCString();
            registerWriter(shared, size, name);
            return NO_ERROR;
        }

        case UNREGISTER_WRITER: {
            CHECK_INTERFACE(IMediaLogService, data, reply);
            sp<IMemory> shared = interface_cast<IMemory>(data.readStrongBinder());
            unregisterWriter(shared);
            return NO_ERROR;
        }

        case REQUEST_MERGE_WAKEUP: {
            CHECK_INTERFACE(IMediaLogService, data, reply);
            requestMergeWakeup();
            return NO_ERROR;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
