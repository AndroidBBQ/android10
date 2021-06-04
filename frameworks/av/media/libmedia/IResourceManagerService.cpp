/*
**
** Copyright 2015, The Android Open Source Project
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
#define LOG_TAG "IResourceManagerService"
#include <utils/Log.h>

#include <media/IResourceManagerService.h>

#include <binder/Parcel.h>

#include <stdint.h>
#include <sys/types.h>

namespace android {

enum {
    CONFIG = IBinder::FIRST_CALL_TRANSACTION,
    ADD_RESOURCE,
    REMOVE_RESOURCE,
    REMOVE_CLIENT,
    RECLAIM_RESOURCE,
};

template <typename T>
static void writeToParcel(Parcel *data, const Vector<T> &items) {
    size_t size = items.size();
    // truncates size, but should be okay for this usecase
    data->writeUint32(static_cast<uint32_t>(size));
    for (size_t i = 0; i < size; i++) {
        items[i].writeToParcel(data);
    }
}

template <typename T>
static void readFromParcel(const Parcel &data, Vector<T> *items) {
    size_t size = (size_t)data.readUint32();
    for (size_t i = 0; i < size && data.dataAvail() > 0; i++) {
        T item;
        item.readFromParcel(data);
        items->add(item);
    }
}

class BpResourceManagerService : public BpInterface<IResourceManagerService>
{
public:
    explicit BpResourceManagerService(const sp<IBinder> &impl)
        : BpInterface<IResourceManagerService>(impl)
    {
    }

    virtual void config(const Vector<MediaResourcePolicy> &policies) {
        Parcel data, reply;
        data.writeInterfaceToken(IResourceManagerService::getInterfaceDescriptor());
        writeToParcel(&data, policies);
        remote()->transact(CONFIG, data, &reply);
    }

    virtual void addResource(
            int pid,
            int uid,
            int64_t clientId,
            const sp<IResourceManagerClient> client,
            const Vector<MediaResource> &resources) {
        Parcel data, reply;
        data.writeInterfaceToken(IResourceManagerService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeInt32(uid);
        data.writeInt64(clientId);
        data.writeStrongBinder(IInterface::asBinder(client));
        writeToParcel(&data, resources);

        remote()->transact(ADD_RESOURCE, data, &reply);
    }

    virtual void removeResource(int pid, int64_t clientId, const Vector<MediaResource> &resources) {
        Parcel data, reply;
        data.writeInterfaceToken(IResourceManagerService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeInt64(clientId);
        writeToParcel(&data, resources);

        remote()->transact(REMOVE_RESOURCE, data, &reply);
    }

    virtual void removeClient(int pid, int64_t clientId) {
        Parcel data, reply;
        data.writeInterfaceToken(IResourceManagerService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeInt64(clientId);

        remote()->transact(REMOVE_CLIENT, data, &reply);
    }

    virtual bool reclaimResource(int callingPid, const Vector<MediaResource> &resources) {
        Parcel data, reply;
        data.writeInterfaceToken(IResourceManagerService::getInterfaceDescriptor());
        data.writeInt32(callingPid);
        writeToParcel(&data, resources);

        bool ret = false;
        status_t status = remote()->transact(RECLAIM_RESOURCE, data, &reply);
        if (status == NO_ERROR) {
            ret = (bool)reply.readInt32();
        }
        return ret;
    }
};

IMPLEMENT_META_INTERFACE(ResourceManagerService, "android.media.IResourceManagerService");

// ----------------------------------------------------------------------


status_t BnResourceManagerService::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    switch (code) {
        case CONFIG: {
            CHECK_INTERFACE(IResourceManagerService, data, reply);
            Vector<MediaResourcePolicy> policies;
            readFromParcel(data, &policies);
            config(policies);
            return NO_ERROR;
        } break;

        case ADD_RESOURCE: {
            CHECK_INTERFACE(IResourceManagerService, data, reply);
            int pid = data.readInt32();
            int uid = data.readInt32();
            int64_t clientId = data.readInt64();
            sp<IResourceManagerClient> client(
                    interface_cast<IResourceManagerClient>(data.readStrongBinder()));
            if (client == NULL) {
                return NO_ERROR;
            }
            Vector<MediaResource> resources;
            readFromParcel(data, &resources);
            addResource(pid, uid, clientId, client, resources);
            return NO_ERROR;
        } break;

        case REMOVE_RESOURCE: {
            CHECK_INTERFACE(IResourceManagerService, data, reply);
            int pid = data.readInt32();
            int64_t clientId = data.readInt64();
            Vector<MediaResource> resources;
            readFromParcel(data, &resources);
            removeResource(pid, clientId, resources);
            return NO_ERROR;
        } break;

        case REMOVE_CLIENT: {
            CHECK_INTERFACE(IResourceManagerService, data, reply);
            int pid = data.readInt32();
            int64_t clientId = data.readInt64();
            removeClient(pid, clientId);
            return NO_ERROR;
        } break;

        case RECLAIM_RESOURCE: {
            CHECK_INTERFACE(IResourceManagerService, data, reply);
            int callingPid = data.readInt32();
            Vector<MediaResource> resources;
            readFromParcel(data, &resources);
            bool ret = reclaimResource(callingPid, resources);
            reply->writeInt32(ret);
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
