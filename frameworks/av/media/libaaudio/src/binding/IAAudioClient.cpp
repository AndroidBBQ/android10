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

#define LOG_TAG "AAudio"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <aaudio/AAudio.h>

#include "binding/AAudioBinderClient.h"
#include "binding/AAudioServiceDefinitions.h"
#include "binding/IAAudioClient.h"
#include "utility/AAudioUtilities.h"

namespace android {

using aaudio::aaudio_handle_t;

/**
 * This is used by the AAudio Service to talk to an AAudio Client.
 *
 * The order of parameters in the Parcels must match with code in AAudioClient.cpp.
 */
class BpAAudioClient : public BpInterface<IAAudioClient>
{
public:
    explicit BpAAudioClient(const sp<IBinder>& impl)
        : BpInterface<IAAudioClient>(impl)
    {
    }

    void onStreamChange(aaudio_handle_t handle, int32_t opcode, int32_t value) override {
        Parcel data, reply;
        data.writeInterfaceToken(IAAudioClient::getInterfaceDescriptor());
        data.writeInt32(handle);
        data.writeInt32(opcode);
        data.writeInt32(value);
        remote()->transact(ON_STREAM_CHANGE, data,  &reply, IBinder::FLAG_ONEWAY);
    }

};

// Implement an interface to the service.
IMPLEMENT_META_INTERFACE(AAudioClient, "IAAudioClient");

// The order of parameters in the Parcels must match with code in BpAAudioClient

status_t BnAAudioClient::onTransact(uint32_t code, const Parcel& data,
                                        Parcel* reply, uint32_t flags) {
    aaudio_handle_t streamHandle;
    int32_t opcode = 0;
    int32_t value = 0;
    ALOGV("BnAAudioClient::onTransact(%u) %u", code, flags);

    switch(code) {
        case ON_STREAM_CHANGE: {
            CHECK_INTERFACE(IAAudioClient, data, reply);
            data.readInt32(&streamHandle);
            data.readInt32(&opcode);
            data.readInt32(&value);
            onStreamChange(streamHandle, opcode, value);
            ALOGD("BnAAudioClient onStreamChange(%x, %d, %d)", streamHandle, opcode, value);
            return NO_ERROR;
        } break;

        default:
            // ALOGW("BnAAudioClient::onTransact not handled %u", code);
            return BBinder::onTransact(code, data, reply, flags);
    }
}

} /* namespace android */
