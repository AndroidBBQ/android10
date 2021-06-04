/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <input/IInputFlinger.h>

namespace android {

class BpInputFlinger : public BpInterface<IInputFlinger> {
public:
    explicit BpInputFlinger(const sp<IBinder>& impl) :
            BpInterface<IInputFlinger>(impl) { }

    virtual void setInputWindows(const std::vector<InputWindowInfo>& inputInfo,
            const sp<ISetInputWindowsListener>& setInputWindowsListener) {
        Parcel data, reply;
        data.writeInterfaceToken(IInputFlinger::getInterfaceDescriptor());

        data.writeUint32(static_cast<uint32_t>(inputInfo.size()));
        for (const auto& info : inputInfo) {
            info.write(data);
        }
        data.writeStrongBinder(IInterface::asBinder(setInputWindowsListener));

        remote()->transact(BnInputFlinger::SET_INPUT_WINDOWS_TRANSACTION, data, &reply,
                IBinder::FLAG_ONEWAY);
    }

    virtual void registerInputChannel(const sp<InputChannel>& channel) {
        Parcel data, reply;
        data.writeInterfaceToken(IInputFlinger::getInterfaceDescriptor());
        channel->write(data);
        remote()->transact(BnInputFlinger::REGISTER_INPUT_CHANNEL_TRANSACTION, data, &reply);
    }

    virtual void unregisterInputChannel(const sp<InputChannel>& channel) {
        Parcel data, reply;
        data.writeInterfaceToken(IInputFlinger::getInterfaceDescriptor());
        channel->write(data);
        remote()->transact(BnInputFlinger::UNREGISTER_INPUT_CHANNEL_TRANSACTION, data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(InputFlinger, "android.input.IInputFlinger");

status_t BnInputFlinger::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    switch(code) {
    case SET_INPUT_WINDOWS_TRANSACTION: {
        CHECK_INTERFACE(IInputFlinger, data, reply);
        size_t count = data.readUint32();
        if (count > data.dataSize()) {
            return BAD_VALUE;
        }
        std::vector<InputWindowInfo> handles;
        for (size_t i = 0; i < count; i++) {
            handles.push_back(InputWindowInfo::read(data));
        }
        const sp<ISetInputWindowsListener> setInputWindowsListener =
                ISetInputWindowsListener::asInterface(data.readStrongBinder());
        setInputWindows(handles, setInputWindowsListener);
        break;
    }
    case REGISTER_INPUT_CHANNEL_TRANSACTION: {
        CHECK_INTERFACE(IInputFlinger, data, reply);
        sp<InputChannel> channel = new InputChannel();
        channel->read(data);
        registerInputChannel(channel);
        break;
    }
    case UNREGISTER_INPUT_CHANNEL_TRANSACTION: {
        CHECK_INTERFACE(IInputFlinger, data, reply);
        sp<InputChannel> channel = new InputChannel();
        channel->read(data);
        unregisterInputChannel(channel);
        break;
    }
    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
    return NO_ERROR;
}

};
