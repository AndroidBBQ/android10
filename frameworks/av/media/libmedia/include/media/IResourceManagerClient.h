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

#ifndef ANDROID_IRESOURCEMANAGERCLIENT_H
#define ANDROID_IRESOURCEMANAGERCLIENT_H

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class IResourceManagerClient: public IInterface
{
public:
    DECLARE_META_INTERFACE(ResourceManagerClient);

    virtual bool reclaimResource() = 0;
    virtual String8 getName() = 0;
};

// ----------------------------------------------------------------------------

class BnResourceManagerClient: public BnInterface<IResourceManagerClient>
{
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel &data,
                                Parcel *reply,
                                uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IRESOURCEMANAGERCLIENT_H
