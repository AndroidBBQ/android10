/*
 * Copyright (C) 2012 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "ICrypto"
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <cutils/log.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <mediadrm/ICrypto.h>
#include <utils/Log.h>

namespace android {

enum {
    INIT_CHECK = IBinder::FIRST_CALL_TRANSACTION,
    IS_CRYPTO_SUPPORTED,
    CREATE_PLUGIN,
    DESTROY_PLUGIN,
    REQUIRES_SECURE_COMPONENT,
    DECRYPT,
    NOTIFY_RESOLUTION,
    SET_MEDIADRM_SESSION,
    SET_HEAP,
    UNSET_HEAP,
};

struct BpCrypto : public BpInterface<ICrypto> {
    explicit BpCrypto(const sp<IBinder> &impl)
        : BpInterface<ICrypto>(impl) {
    }

    virtual status_t initCheck() const {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        remote()->transact(INIT_CHECK, data, &reply);

        return reply.readInt32();
    }

    virtual bool isCryptoSchemeSupported(const uint8_t uuid[16]) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.write(uuid, 16);
        remote()->transact(IS_CRYPTO_SUPPORTED, data, &reply);

        return reply.readInt32() != 0;
    }

    virtual status_t createPlugin(
            const uint8_t uuid[16], const void *opaqueData, size_t opaqueSize) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.write(uuid, 16);
        data.writeInt32(opaqueSize);

        if (opaqueSize > 0) {
            data.write(opaqueData, opaqueSize);
        }

        remote()->transact(CREATE_PLUGIN, data, &reply);

        return reply.readInt32();
    }

    virtual status_t destroyPlugin() {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        remote()->transact(DESTROY_PLUGIN, data, &reply);

        return reply.readInt32();
    }

    virtual bool requiresSecureDecoderComponent(
            const char *mime) const {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.writeCString(mime);
        remote()->transact(REQUIRES_SECURE_COMPONENT, data, &reply);

        return reply.readInt32() != 0;
    }

    virtual ssize_t decrypt(const uint8_t key[16], const uint8_t iv[16],
            CryptoPlugin::Mode mode, const CryptoPlugin::Pattern &pattern,
            const SourceBuffer &source, size_t offset,
            const CryptoPlugin::SubSample *subSamples, size_t numSubSamples,
            const DestinationBuffer &destination, AString *errorDetailMsg) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.writeInt32(mode);
        data.writeInt32(pattern.mEncryptBlocks);
        data.writeInt32(pattern.mSkipBlocks);

        static const uint8_t kDummy[16] = { 0 };

        if (key == NULL) {
            key = kDummy;
        }

        if (iv == NULL) {
            iv = kDummy;
        }

        data.write(key, 16);
        data.write(iv, 16);

        size_t totalSize = 0;
        for (size_t i = 0; i < numSubSamples; ++i) {
            totalSize += subSamples[i].mNumBytesOfEncryptedData;
            totalSize += subSamples[i].mNumBytesOfClearData;
        }

        data.writeInt32(totalSize);
        data.writeStrongBinder(IInterface::asBinder(source.mSharedMemory));
        data.writeInt32(source.mHeapSeqNum);
        data.writeInt32(offset);

        data.writeInt32(numSubSamples);
        data.write(subSamples, sizeof(CryptoPlugin::SubSample) * numSubSamples);

        data.writeInt32((int32_t)destination.mType);
        if (destination.mType == kDestinationTypeNativeHandle) {
            if (destination.mHandle == NULL) {
                return BAD_VALUE;
            }
            data.writeNativeHandle(destination.mHandle);
        } else {
            if (destination.mSharedMemory == NULL) {
                return BAD_VALUE;
            }
            data.writeStrongBinder(IInterface::asBinder(destination.mSharedMemory));
        }

        remote()->transact(DECRYPT, data, &reply);

        ssize_t result = reply.readInt32();

        if (isCryptoError(result)) {
            AString msg = reply.readCString();
            if (errorDetailMsg) {
                *errorDetailMsg = msg;
            }
        }

        return result;
    }

    virtual void notifyResolution(
        uint32_t width, uint32_t height) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.writeInt32(width);
        data.writeInt32(height);
        remote()->transact(NOTIFY_RESOLUTION, data, &reply);
    }

    virtual status_t setMediaDrmSession(const Vector<uint8_t> &sessionId) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());

        writeVector(data, sessionId);
        remote()->transact(SET_MEDIADRM_SESSION, data, &reply);

        return reply.readInt32();
    }

    virtual int32_t setHeap(const sp<IMemoryHeap> &heap) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(heap));
        status_t err = remote()->transact(SET_HEAP, data, &reply);
        if (err != NO_ERROR) {
            return -1;
        }
        int32_t seqNum;
        if (reply.readInt32(&seqNum) != NO_ERROR) {
            return -1;
        }
        return seqNum;
    }

    virtual void unsetHeap(int32_t seqNum) {
        Parcel data, reply;
        data.writeInterfaceToken(ICrypto::getInterfaceDescriptor());
        data.writeInt32(seqNum);
        remote()->transact(UNSET_HEAP, data, &reply);
        return;
    }


private:
    void readVector(Parcel &reply, Vector<uint8_t> &vector) const {
        uint32_t size = reply.readInt32();
        vector.insertAt((size_t)0, size);
        reply.read(vector.editArray(), size);
    }

    void writeVector(Parcel &data, Vector<uint8_t> const &vector) const {
        data.writeInt32(vector.size());
        data.write(vector.array(), vector.size());
    }

    DISALLOW_EVIL_CONSTRUCTORS(BpCrypto);
};

IMPLEMENT_META_INTERFACE(Crypto, "android.hardware.ICrypto");

////////////////////////////////////////////////////////////////////////////////

void BnCrypto::readVector(const Parcel &data, Vector<uint8_t> &vector) const {
    uint32_t size = data.readInt32();
    if (vector.insertAt((size_t)0, size) < 0) {
        vector.clear();
    }
    if (data.read(vector.editArray(), size) != NO_ERROR) {
        vector.clear();
        android_errorWriteWithInfoLog(0x534e4554, "62872384", -1, NULL, 0);
    }
}

void BnCrypto::writeVector(Parcel *reply, Vector<uint8_t> const &vector) const {
    reply->writeInt32(vector.size());
    reply->write(vector.array(), vector.size());
}

status_t BnCrypto::onTransact(
    uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case INIT_CHECK:
        {
            CHECK_INTERFACE(ICrypto, data, reply);
            reply->writeInt32(initCheck());

            return OK;
        }

        case IS_CRYPTO_SUPPORTED:
        {
            CHECK_INTERFACE(ICrypto, data, reply);
            uint8_t uuid[16];
            data.read(uuid, sizeof(uuid));
            reply->writeInt32(isCryptoSchemeSupported(uuid));

            return OK;
        }

        case CREATE_PLUGIN:
        {
            CHECK_INTERFACE(ICrypto, data, reply);

            uint8_t uuid[16] = {0};
            if (data.read(uuid, sizeof(uuid)) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "144767096");
                reply->writeInt32(BAD_VALUE);
                return OK;
            }

            size_t opaqueSize = data.readInt32();
            void *opaqueData = NULL;

            const size_t kMaxOpaqueSize = 100 * 1024;
            if (opaqueSize > kMaxOpaqueSize) {
                return BAD_VALUE;
            }

            opaqueData = malloc(opaqueSize);
            if (NULL == opaqueData) {
                return NO_MEMORY;
            }

            if (data.read(opaqueData, opaqueSize) != NO_ERROR) {
                android_errorWriteLog(0x534e4554, "144767096");
                reply->writeInt32(BAD_VALUE);
                return OK;
            }
            reply->writeInt32(createPlugin(uuid, opaqueData, opaqueSize));

            free(opaqueData);
            opaqueData = NULL;

            return OK;
        }

        case DESTROY_PLUGIN:
        {
            CHECK_INTERFACE(ICrypto, data, reply);
            reply->writeInt32(destroyPlugin());

            return OK;
        }

        case REQUIRES_SECURE_COMPONENT:
        {
            CHECK_INTERFACE(ICrypto, data, reply);

            const char *mime = data.readCString();
            if (mime == NULL) {
                reply->writeInt32(BAD_VALUE);
            } else {
                reply->writeInt32(requiresSecureDecoderComponent(mime));
            }

            return OK;
        }

        case DECRYPT:
        {
            CHECK_INTERFACE(ICrypto, data, reply);

            CryptoPlugin::Mode mode = (CryptoPlugin::Mode)data.readInt32();
            CryptoPlugin::Pattern pattern;
            pattern.mEncryptBlocks = data.readInt32();
            pattern.mSkipBlocks = data.readInt32();

            uint8_t key[16];
            data.read(key, sizeof(key));

            uint8_t iv[16];
            data.read(iv, sizeof(iv));

            size_t totalSize = data.readInt32();

            SourceBuffer source;

            source.mSharedMemory =
                interface_cast<IMemory>(data.readStrongBinder());
            if (source.mSharedMemory == NULL) {
                reply->writeInt32(BAD_VALUE);
                return OK;
            }
            source.mHeapSeqNum = data.readInt32();

            int32_t offset = data.readInt32();

            int32_t numSubSamples = data.readInt32();
            if (numSubSamples < 0 || numSubSamples > 0xffff) {
                reply->writeInt32(BAD_VALUE);
                return OK;
            }

            std::unique_ptr<CryptoPlugin::SubSample[]> subSamples =
                    std::make_unique<CryptoPlugin::SubSample[]>(numSubSamples);

            data.read(subSamples.get(),
                    sizeof(CryptoPlugin::SubSample) * numSubSamples);

            DestinationBuffer destination;
            destination.mType = (DestinationType)data.readInt32();
            if (destination.mType == kDestinationTypeNativeHandle) {
                destination.mHandle = data.readNativeHandle();
                if (destination.mHandle == NULL) {
                    reply->writeInt32(BAD_VALUE);
                    return OK;
                }
            } else if (destination.mType == kDestinationTypeSharedMemory) {
                destination.mSharedMemory =
                        interface_cast<IMemory>(data.readStrongBinder());
                if (destination.mSharedMemory == NULL) {
                    reply->writeInt32(BAD_VALUE);
                    return OK;
                }
                sp<IMemory> dest = destination.mSharedMemory;
                if (totalSize > dest->size() ||
                        (size_t)dest->offset() > dest->size() - totalSize) {
                    reply->writeInt32(BAD_VALUE);
                    android_errorWriteLog(0x534e4554, "71389378");
                    return OK;
                }
            } else {
                reply->writeInt32(BAD_VALUE);
                android_errorWriteLog(0x534e4554, "70526702");
                return OK;
            }

            AString errorDetailMsg;
            ssize_t result;

            size_t sumSubsampleSizes = 0;
            bool overflow = false;
            for (int32_t i = 0; i < numSubSamples; ++i) {
                CryptoPlugin::SubSample &ss = subSamples[i];
                if (sumSubsampleSizes <= SIZE_MAX - ss.mNumBytesOfEncryptedData) {
                    sumSubsampleSizes += ss.mNumBytesOfEncryptedData;
                } else {
                    overflow = true;
                }
                if (sumSubsampleSizes <= SIZE_MAX - ss.mNumBytesOfClearData) {
                    sumSubsampleSizes += ss.mNumBytesOfClearData;
                } else {
                    overflow = true;
                }
            }

            if (overflow || sumSubsampleSizes != totalSize) {
                result = -EINVAL;
            } else if (totalSize > source.mSharedMemory->size()) {
                result = -EINVAL;
            } else if ((size_t)offset > source.mSharedMemory->size() - totalSize) {
                result = -EINVAL;
            } else {
                result = decrypt(key, iv, mode, pattern, source, offset,
                        subSamples.get(), numSubSamples, destination, &errorDetailMsg);
            }

            reply->writeInt32(result);

            if (isCryptoError(result)) {
                reply->writeCString(errorDetailMsg.c_str());
            }

            if (destination.mType == kDestinationTypeNativeHandle) {
                int err;
                if ((err = native_handle_close(destination.mHandle)) < 0) {
                    ALOGW("secure buffer native_handle_close failed: %d", err);
                }
                if ((err = native_handle_delete(destination.mHandle)) < 0) {
                    ALOGW("secure buffer native_handle_delete failed: %d", err);
                }
            }

            subSamples.reset();
            return OK;
        }

        case NOTIFY_RESOLUTION:
        {
            CHECK_INTERFACE(ICrypto, data, reply);

            int32_t width = data.readInt32();
            int32_t height = data.readInt32();
            notifyResolution(width, height);

            return OK;
        }

        case SET_MEDIADRM_SESSION:
        {
            CHECK_INTERFACE(IDrm, data, reply);
            Vector<uint8_t> sessionId;
            readVector(data, sessionId);
            reply->writeInt32(setMediaDrmSession(sessionId));
            return OK;
        }

        case SET_HEAP:
        {
            CHECK_INTERFACE(ICrypto, data, reply);
            sp<IMemoryHeap> heap =
                interface_cast<IMemoryHeap>(data.readStrongBinder());
            reply->writeInt32(setHeap(heap));
            return OK;
        }

        case UNSET_HEAP:
        {
            CHECK_INTERFACE(ICrypto, data, reply);
            int32_t seqNum = data.readInt32();
            unsetHeap(seqNum);
            return OK;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}  // namespace android
