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

#ifndef ANDROID_IMEDIADRMSERVICE_H
#define ANDROID_IMEDIADRMSERVICE_H

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

struct ICrypto;
struct IDrm;

class IMediaDrmService: public IInterface
{
public:
    DECLARE_META_INTERFACE(MediaDrmService);

    virtual sp<ICrypto>         makeCrypto() = 0;
    virtual sp<IDrm>            makeDrm() = 0;
};

// ----------------------------------------------------------------------------

class BnMediaDrmService: public BnInterface<IMediaDrmService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IMEDIADRMSERVICE_H
