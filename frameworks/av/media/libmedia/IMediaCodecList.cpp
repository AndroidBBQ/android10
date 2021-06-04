/*
 * Copyright (C) 2014, The Android Open Source Project
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
#include <media/stagefright/MediaCodecList.h>
#include <media/IMediaCodecList.h>
#include <media/MediaCodecInfo.h>

#include <utils/Errors.h>  // for status_t

namespace android {

enum {
    CREATE = IBinder::FIRST_CALL_TRANSACTION,
    COUNT_CODECS,
    GET_CODEC_INFO,
    GET_GLOBAL_SETTINGS,
    FIND_CODEC_BY_TYPE,
    FIND_CODEC_BY_NAME,
};

class BpMediaCodecList: public BpInterface<IMediaCodecList>
{
public:
    explicit BpMediaCodecList(const sp<IBinder>& impl)
        : BpInterface<IMediaCodecList>(impl)
    {
    }

    virtual size_t countCodecs() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaCodecList::getInterfaceDescriptor());
        remote()->transact(COUNT_CODECS, data, &reply);
        return static_cast<size_t>(reply.readInt32());
    }

    virtual sp<MediaCodecInfo> getCodecInfo(size_t index) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaCodecList::getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(GET_CODEC_INFO, data, &reply);
        status_t err = reply.readInt32();
        if (err == OK) {
            return MediaCodecInfo::FromParcel(reply);
        } else {
            return NULL;
        }
    }

    virtual const sp<AMessage> getGlobalSettings() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaCodecList::getInterfaceDescriptor());
        remote()->transact(GET_GLOBAL_SETTINGS, data, &reply);
        status_t err = reply.readInt32();
        if (err == OK) {
            return AMessage::FromParcel(reply);
        } else {
            return NULL;
        }
    }

    virtual ssize_t findCodecByType(
            const char *type, bool encoder, size_t startIndex = 0) const
    {
        if (startIndex > INT32_MAX) {
            return NAME_NOT_FOUND;
        }

        Parcel data, reply;
        data.writeInterfaceToken(IMediaCodecList::getInterfaceDescriptor());
        data.writeCString(type);
        data.writeInt32(encoder);
        data.writeInt32(startIndex);
        remote()->transact(FIND_CODEC_BY_TYPE, data, &reply);
        return static_cast<ssize_t>(reply.readInt32());
    }

    virtual ssize_t findCodecByName(const char *name) const
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaCodecList::getInterfaceDescriptor());
        data.writeCString(name);
        remote()->transact(FIND_CODEC_BY_NAME, data, &reply);
        return static_cast<ssize_t>(reply.readInt32());
    }
};

IMPLEMENT_META_INTERFACE(MediaCodecList, "android.media.IMediaCodecList");

// ----------------------------------------------------------------------

status_t BnMediaCodecList::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case COUNT_CODECS:
        {
            CHECK_INTERFACE(IMediaCodecList, data, reply);
            size_t count = countCodecs();
            if (count > INT32_MAX) {
                count = INT32_MAX;
            }
            reply->writeInt32(count);
            return NO_ERROR;
        }
        break;

        case GET_CODEC_INFO:
        {
            CHECK_INTERFACE(IMediaCodecList, data, reply);
            size_t index = static_cast<size_t>(data.readInt32());
            const sp<MediaCodecInfo> info = getCodecInfo(index);
            if (info != NULL) {
                reply->writeInt32(OK);
                info->writeToParcel(reply);
            } else {
                reply->writeInt32(-ERANGE);
            }
            return NO_ERROR;
        }
        break;

        case GET_GLOBAL_SETTINGS:
        {
            CHECK_INTERFACE(IMediaCodecList, data, reply);
            const sp<AMessage> info = getGlobalSettings();
            if (info != NULL) {
                reply->writeInt32(OK);
                info->writeToParcel(reply);
            } else {
                reply->writeInt32(-ERANGE);
            }
            return NO_ERROR;
        }
        break;

        case FIND_CODEC_BY_TYPE:
        {
            CHECK_INTERFACE(IMediaCodecList, data, reply);
            const char *type = data.readCString();
            if (type == NULL) {
                reply->writeInt32(NAME_NOT_FOUND);
                return NO_ERROR;
            }
            bool isEncoder = static_cast<bool>(data.readInt32());
            size_t startIndex = static_cast<size_t>(data.readInt32());
            ssize_t index = findCodecByType(type, isEncoder, startIndex);
            if (index > INT32_MAX || index < 0) {
                index = NAME_NOT_FOUND;
            }
            reply->writeInt32(index);
            return NO_ERROR;
        }
        break;

        case FIND_CODEC_BY_NAME:
        {
            CHECK_INTERFACE(IMediaCodecList, data, reply);
            const char *name = data.readCString();
            if (name == NULL) {
                reply->writeInt32(NAME_NOT_FOUND);
                return NO_ERROR;
            }
            ssize_t index = findCodecByName(name);
            if (index > INT32_MAX || index < 0) {
                index = NAME_NOT_FOUND;
            }
            reply->writeInt32(index);
            return NO_ERROR;
        }
        break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
