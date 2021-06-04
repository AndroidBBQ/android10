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

#ifndef ANDROID_RESOURCEMANAGERSERVICE_H
#define ANDROID_RESOURCEMANAGERSERVICE_H

#include <arpa/inet.h>
#include <binder/BinderService.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/Vector.h>

#include <media/IResourceManagerService.h>

namespace android {

class ServiceLog;
struct ProcessInfoInterface;

typedef std::map<std::pair<MediaResource::Type, MediaResource::SubType>, MediaResource> ResourceList;
struct ResourceInfo {
    int64_t clientId;
    uid_t uid;
    sp<IResourceManagerClient> client;
    sp<IBinder::DeathRecipient> deathNotifier;
    ResourceList resources;
};

// TODO: convert these to std::map
typedef KeyedVector<int64_t, ResourceInfo> ResourceInfos;
typedef KeyedVector<int, ResourceInfos> PidResourceInfosMap;

class ResourceManagerService
    : public BinderService<ResourceManagerService>,
      public BnResourceManagerService
{
public:
    struct SystemCallbackInterface : public RefBase {
        virtual void noteStartVideo(int uid) = 0;
        virtual void noteStopVideo(int uid) = 0;
        virtual void noteResetVideo() = 0;
        virtual bool requestCpusetBoost(
                bool enable, const sp<IInterface> &client) = 0;
    };

    static char const *getServiceName() { return "media.resource_manager"; }

    virtual status_t dump(int fd, const Vector<String16>& args);

    ResourceManagerService();
    explicit ResourceManagerService(
            const sp<ProcessInfoInterface> &processInfo,
            const sp<SystemCallbackInterface> &systemResource);

    // IResourceManagerService interface
    virtual void config(const Vector<MediaResourcePolicy> &policies);

    virtual void addResource(
            int pid,
            int uid,
            int64_t clientId,
            const sp<IResourceManagerClient> client,
            const Vector<MediaResource> &resources);

    virtual void removeResource(int pid, int64_t clientId,
            const Vector<MediaResource> &resources);

    virtual void removeClient(int pid, int64_t clientId);

    // Tries to reclaim resource from processes with lower priority than the calling process
    // according to the requested resources.
    // Returns true if any resource has been reclaimed, otherwise returns false.
    virtual bool reclaimResource(int callingPid, const Vector<MediaResource> &resources);

    void removeResource(int pid, int64_t clientId, bool checkValid);

protected:
    virtual ~ResourceManagerService();

private:
    friend class ResourceManagerServiceTest;

    // Gets the list of all the clients who own the specified resource type.
    // Returns false if any client belongs to a process with higher priority than the
    // calling process. The clients will remain unchanged if returns false.
    bool getAllClients_l(int callingPid, MediaResource::Type type,
            Vector<sp<IResourceManagerClient>> *clients);

    // Gets the client who owns specified resource type from lowest possible priority process.
    // Returns false if the calling process priority is not higher than the lowest process
    // priority. The client will remain unchanged if returns false.
    bool getLowestPriorityBiggestClient_l(int callingPid, MediaResource::Type type,
            sp<IResourceManagerClient> *client);

    // Gets lowest priority process that has the specified resource type.
    // Returns false if failed. The output parameters will remain unchanged if failed.
    bool getLowestPriorityPid_l(MediaResource::Type type, int *pid, int *priority);

    // Gets the client who owns biggest piece of specified resource type from pid.
    // Returns false if failed. The client will remain unchanged if failed.
    bool getBiggestClient_l(int pid, MediaResource::Type type, sp<IResourceManagerClient> *client);

    bool isCallingPriorityHigher_l(int callingPid, int pid);

    // A helper function basically calls getLowestPriorityBiggestClient_l and add the result client
    // to the given Vector.
    void getClientForResource_l(
        int callingPid, const MediaResource *res, Vector<sp<IResourceManagerClient>> *clients);

    void onFirstAdded(const MediaResource& res, const ResourceInfo& clientInfo);
    void onLastRemoved(const MediaResource& res, const ResourceInfo& clientInfo);

    mutable Mutex mLock;
    sp<ProcessInfoInterface> mProcessInfo;
    sp<SystemCallbackInterface> mSystemCB;
    sp<ServiceLog> mServiceLog;
    PidResourceInfosMap mMap;
    bool mSupportsMultipleSecureCodecs;
    bool mSupportsSecureWithNonSecureCodec;
    int32_t mCpuBoostCount;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_RESOURCEMANAGERSERVICE_H
