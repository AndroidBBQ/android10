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

#include <binder/IProcessInfoService.h>
#include <binder/Parcel.h>
#include <utils/Errors.h>
#include <sys/types.h>

namespace android {

// ----------------------------------------------------------------------

class BpProcessInfoService : public BpInterface<IProcessInfoService> {
public:
    explicit BpProcessInfoService(const sp<IBinder>& impl)
        : BpInterface<IProcessInfoService>(impl) {}

    virtual status_t getProcessStatesFromPids(size_t length, /*in*/ int32_t* pids,
            /*out*/ int32_t* states)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IProcessInfoService::getInterfaceDescriptor());
        data.writeInt32Array(length, pids);
        data.writeInt32(length); // write length of output array, used by java AIDL stubs
        status_t err = remote()->transact(GET_PROCESS_STATES_FROM_PIDS, data, &reply);
        if (err != NO_ERROR || ((err = reply.readExceptionCode()) != NO_ERROR)) {
            return err;
        }
        int32_t replyLen = reply.readInt32();
        if (static_cast<size_t>(replyLen) != length) {
            return NOT_ENOUGH_DATA;
        }
        if (replyLen > 0 && (err = reply.read(states, length * sizeof(*states))) != NO_ERROR) {
            return err;
        }
        return reply.readInt32();
    }

    virtual status_t getProcessStatesAndOomScoresFromPids(size_t length,
            /*in*/ int32_t* pids, /*out*/ int32_t* states, /*out*/ int32_t* scores)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IProcessInfoService::getInterfaceDescriptor());
        data.writeInt32Array(length, pids);
        // write length of output arrays, used by java AIDL stubs
        data.writeInt32(length);
        data.writeInt32(length);
        status_t err = remote()->transact(
                GET_PROCESS_STATES_AND_OOM_SCORES_FROM_PIDS, data, &reply);
        if (err != NO_ERROR
                || ((err = reply.readExceptionCode()) != NO_ERROR)) {
            return err;
        }
        int32_t replyLen = reply.readInt32();
        if (static_cast<size_t>(replyLen) != length) {
            return NOT_ENOUGH_DATA;
        }
        if (replyLen > 0 && (err = reply.read(
                states, length * sizeof(*states))) != NO_ERROR) {
            return err;
        }
        replyLen = reply.readInt32();
        if (static_cast<size_t>(replyLen) != length) {
            return NOT_ENOUGH_DATA;
        }
        if (replyLen > 0 && (err = reply.read(
                scores, length * sizeof(*scores))) != NO_ERROR) {
            return err;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(ProcessInfoService, "android.os.IProcessInfoService");

// ----------------------------------------------------------------------

}; // namespace android
