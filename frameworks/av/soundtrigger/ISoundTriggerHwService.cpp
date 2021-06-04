/*
**
** Copyright 2014, The Android Open Source Project
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

#define LOG_TAG "BpSoundTriggerHwService"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/Errors.h>

#include <stdint.h>
#include <sys/types.h>
#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <soundtrigger/ISoundTriggerHwService.h>
#include <soundtrigger/ISoundTrigger.h>
#include <soundtrigger/ISoundTriggerClient.h>

namespace android {

enum {
    LIST_MODULES = IBinder::FIRST_CALL_TRANSACTION,
    ATTACH,
    SET_CAPTURE_STATE,
};

#define MAX_ITEMS_PER_LIST 1024

class BpSoundTriggerHwService: public BpInterface<ISoundTriggerHwService>
{
public:
    explicit BpSoundTriggerHwService(const sp<IBinder>& impl)
        : BpInterface<ISoundTriggerHwService>(impl)
    {
    }

    virtual status_t listModules(const String16& opPackageName,
                                 struct sound_trigger_module_descriptor *modules,
                                 uint32_t *numModules)
    {
        if (numModules == NULL || (*numModules != 0 && modules == NULL)) {
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(ISoundTriggerHwService::getInterfaceDescriptor());
        data.writeString16(opPackageName);
        unsigned int numModulesReq = (modules == NULL) ? 0 : *numModules;
        data.writeInt32(numModulesReq);
        status_t status = remote()->transact(LIST_MODULES, data, &reply);
        if (status == NO_ERROR) {
            status = (status_t)reply.readInt32();
            *numModules = (unsigned int)reply.readInt32();
        }
        ALOGV("listModules() status %d got *numModules %d", status, *numModules);
        if (status == NO_ERROR) {
            if (numModulesReq > *numModules) {
                numModulesReq = *numModules;
            }
            if (numModulesReq > 0) {
                reply.read(modules, numModulesReq * sizeof(struct sound_trigger_module_descriptor));
            }
        }
        return status;
    }

    virtual status_t attach(const String16& opPackageName,
                            const sound_trigger_module_handle_t handle,
                            const sp<ISoundTriggerClient>& client,
                            sp<ISoundTrigger>& module)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISoundTriggerHwService::getInterfaceDescriptor());
        data.writeString16(opPackageName);
        data.write(&handle, sizeof(sound_trigger_module_handle_t));
        data.writeStrongBinder(IInterface::asBinder(client));
        status_t status = remote()->transact(ATTACH, data, &reply);
        if (status != NO_ERROR) {
            return status;
        }
        status = reply.readInt32();
        if (reply.readInt32() != 0) {
            module = interface_cast<ISoundTrigger>(reply.readStrongBinder());
        }
        return status;
    }

    virtual status_t setCaptureState(bool active)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISoundTriggerHwService::getInterfaceDescriptor());
        data.writeInt32(active);
        status_t status = remote()->transact(SET_CAPTURE_STATE, data, &reply);
        if (status == NO_ERROR) {
            status = reply.readInt32();
        }
        return status;
    }

};

IMPLEMENT_META_INTERFACE(SoundTriggerHwService, "android.hardware.ISoundTriggerHwService");

// ----------------------------------------------------------------------

status_t BnSoundTriggerHwService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case LIST_MODULES: {
            CHECK_INTERFACE(ISoundTriggerHwService, data, reply);
            String16 opPackageName;
            status_t status = data.readString16(&opPackageName);
            if (status != NO_ERROR) {
                return status;
            }
            unsigned int numModulesReq = data.readInt32();
            if (numModulesReq > MAX_ITEMS_PER_LIST) {
                numModulesReq = MAX_ITEMS_PER_LIST;
            }
            unsigned int numModules = numModulesReq;
            struct sound_trigger_module_descriptor *modules =
                    (struct sound_trigger_module_descriptor *)calloc(numModulesReq,
                                                   sizeof(struct sound_trigger_module_descriptor));
            if (modules == NULL) {
                reply->writeInt32(NO_MEMORY);
                reply->writeInt32(0);
                return NO_ERROR;
            }
            status = listModules(opPackageName, modules, &numModules);
            reply->writeInt32(status);
            reply->writeInt32(numModules);
            ALOGV("LIST_MODULES status %d got numModules %d", status, numModules);

            if (status == NO_ERROR) {
                if (numModulesReq > numModules) {
                    numModulesReq = numModules;
                }
                reply->write(modules,
                             numModulesReq * sizeof(struct sound_trigger_module_descriptor));
            }
            free(modules);
            return NO_ERROR;
        }

        case ATTACH: {
            CHECK_INTERFACE(ISoundTriggerHwService, data, reply);
            String16 opPackageName;
            status_t status = data.readString16(&opPackageName);
            if (status != NO_ERROR) {
                return status;
            }
            sound_trigger_module_handle_t handle;
            data.read(&handle, sizeof(sound_trigger_module_handle_t));
            sp<ISoundTriggerClient> client =
                    interface_cast<ISoundTriggerClient>(data.readStrongBinder());
            sp<ISoundTrigger> module;
            status = attach(opPackageName, handle, client, module);
            reply->writeInt32(status);
            if (module != 0) {
                reply->writeInt32(1);
                reply->writeStrongBinder(IInterface::asBinder(module));
            } else {
                reply->writeInt32(0);
            }
            return NO_ERROR;
        } break;

        case SET_CAPTURE_STATE: {
            CHECK_INTERFACE(ISoundTriggerHwService, data, reply);
            reply->writeInt32(setCaptureState((bool)data.readInt32()));
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
