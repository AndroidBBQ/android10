/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "ShellCallback"

#include <unistd.h>
#include <fcntl.h>

#include <binder/IShellCallback.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include <private/binder/Static.h>

namespace android {

// ----------------------------------------------------------------------

class BpShellCallback : public BpInterface<IShellCallback>
{
public:
    explicit BpShellCallback(const sp<IBinder>& impl)
        : BpInterface<IShellCallback>(impl)
    {
    }

    virtual int openFile(const String16& path, const String16& seLinuxContext,
            const String16& mode) {
        Parcel data, reply;
        data.writeInterfaceToken(IShellCallback::getInterfaceDescriptor());
        data.writeString16(path);
        data.writeString16(seLinuxContext);
        data.writeString16(mode);
        remote()->transact(OP_OPEN_OUTPUT_FILE, data, &reply, 0);
        reply.readExceptionCode();
        int fd = reply.readParcelFileDescriptor();
        return fd >= 0 ? fcntl(fd, F_DUPFD_CLOEXEC, 0) : fd;

    }
};

IMPLEMENT_META_INTERFACE(ShellCallback, "com.android.internal.os.IShellCallback");

// ----------------------------------------------------------------------

// NOLINTNEXTLINE(google-default-arguments)
status_t BnShellCallback::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case OP_OPEN_OUTPUT_FILE: {
            CHECK_INTERFACE(IShellCallback, data, reply);
            String16 path(data.readString16());
            String16 seLinuxContext(data.readString16());
            String16 mode(data.readString16());
            int fd = openFile(path, seLinuxContext, mode);
            if (reply != nullptr) {
                reply->writeNoException();
                if (fd >= 0) {
                    reply->writeInt32(1);
                    reply->writeParcelFileDescriptor(fd, true);
                } else {
                    reply->writeInt32(0);
                }
            } else if (fd >= 0) {
                close(fd);
            }
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
