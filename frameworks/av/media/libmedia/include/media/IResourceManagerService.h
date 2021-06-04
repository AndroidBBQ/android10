/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_IRESOURCEMANAGERSERVICE_H
#define ANDROID_IRESOURCEMANAGERSERVICE_H

#include <utils/Errors.h>  // for status_t
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <media/IResourceManagerClient.h>
#include <media/MediaResource.h>
#include <media/MediaResourcePolicy.h>

namespace android {

class IResourceManagerService: public IInterface
{
public:
    DECLARE_META_INTERFACE(ResourceManagerService);

    virtual void config(const Vector<MediaResourcePolicy> &policies) = 0;

    virtual void addResource(
            int pid,
            int uid,
            int64_t clientId,
            const sp<IResourceManagerClient> client,
            const Vector<MediaResource> &resources) = 0;

    virtual void removeResource(int pid, int64_t clientId,
            const Vector<MediaResource> &resources) = 0;

    virtual void removeClient(int pid, int64_t clientId) = 0;

    virtual bool reclaimResource(
            int callingPid,
            const Vector<MediaResource> &resources) = 0;
};

// ----------------------------------------------------------------------------

class BnResourceManagerService: public BnInterface<IResourceManagerService>
{
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel &data,
                                Parcel *reply,
                                uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IRESOURCEMANAGERSERVICE_H
