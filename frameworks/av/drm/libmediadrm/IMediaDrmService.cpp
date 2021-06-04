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

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <mediadrm/ICrypto.h>
#include <mediadrm/IDrm.h>
#include <mediadrm/IMediaDrmService.h>

#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>

namespace android {

enum {
    MAKE_CRYPTO = IBinder::FIRST_CALL_TRANSACTION,
    MAKE_DRM,
};

class BpMediaDrmService: public BpInterface<IMediaDrmService>
{
public:
    explicit BpMediaDrmService(const sp<IBinder>& impl)
        : BpInterface<IMediaDrmService>(impl)
    {
    }

    virtual sp<ICrypto> makeCrypto() {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaDrmService::getInterfaceDescriptor());
        remote()->transact(MAKE_CRYPTO, data, &reply);
        return interface_cast<ICrypto>(reply.readStrongBinder());
    }

    virtual sp<IDrm> makeDrm() {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaDrmService::getInterfaceDescriptor());
        remote()->transact(MAKE_DRM, data, &reply);
        return interface_cast<IDrm>(reply.readStrongBinder());
    }

};

IMPLEMENT_META_INTERFACE(MediaDrmService, "android.media.IMediaDrmService");

// ----------------------------------------------------------------------

status_t BnMediaDrmService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case MAKE_CRYPTO: {
            CHECK_INTERFACE(IMediaDrmService, data, reply);
            sp<ICrypto> crypto = makeCrypto();
            reply->writeStrongBinder(IInterface::asBinder(crypto));
            return NO_ERROR;
        } break;
        case MAKE_DRM: {
            CHECK_INTERFACE(IMediaDrmService, data, reply);
            sp<IDrm> drm = makeDrm();
            reply->writeStrongBinder(IInterface::asBinder(drm));
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
