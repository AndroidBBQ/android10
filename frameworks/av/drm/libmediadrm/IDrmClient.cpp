/*
**
** Copyright 2013, The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "IDrmClient"
#include <utils/Log.h>

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <media/IMediaPlayerClient.h>
#include <mediadrm/IDrmClient.h>

namespace android {

enum {
    NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
};

class BpDrmClient: public BpInterface<IDrmClient>
{
public:
    explicit BpDrmClient(const sp<IBinder>& impl)
        : BpInterface<IDrmClient>(impl)
    {
    }

    virtual void notify(DrmPlugin::EventType eventType, int extra, const Parcel *obj)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDrmClient::getInterfaceDescriptor());
        data.writeInt32((int)eventType);
        data.writeInt32(extra);
        if (obj && obj->dataSize() > 0) {
            data.appendFrom(const_cast<Parcel *>(obj), 0, obj->dataSize());
        }
        remote()->transact(NOTIFY, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(DrmClient, "android.media.IDrmClient");

// ----------------------------------------------------------------------

status_t BnDrmClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case NOTIFY: {
            CHECK_INTERFACE(IDrmClient, data, reply);
            int eventType = data.readInt32();
            int extra = data.readInt32();
            Parcel obj;
            if (data.dataAvail() > 0) {
                obj.appendFrom(const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }

            notify((DrmPlugin::EventType)eventType, extra, &obj);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

} // namespace android
