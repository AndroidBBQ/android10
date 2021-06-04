/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <input/ISetInputWindowsListener.h>

namespace android {

class BpSetInputWindowsListener : public BpInterface<ISetInputWindowsListener> {
public:
    explicit BpSetInputWindowsListener(const sp<IBinder>& impl)
        : BpInterface<ISetInputWindowsListener>(impl) {
    }

    virtual ~BpSetInputWindowsListener() = default;

    virtual void onSetInputWindowsFinished() {
        Parcel data, reply;
        data.writeInterfaceToken(ISetInputWindowsListener::getInterfaceDescriptor());
        remote()->transact(BnSetInputWindowsListener::ON_SET_INPUT_WINDOWS_FINISHED, data, &reply,
                IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(SetInputWindowsListener, "android.input.ISetInputWindowsListener");

status_t BnSetInputWindowsListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
        uint32_t flags) {
    switch(code) {
        case ON_SET_INPUT_WINDOWS_FINISHED: {
            CHECK_INTERFACE(ISetInputWindowsListener, data, reply);
            onSetInputWindowsFinished();
            return NO_ERROR;
        }
        default: {
            return BBinder::onTransact(code, data, reply, flags);
        }
    }
}

} // namespace android
