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

#include <binder/IInterface.h>
#include <cutils/native_handle.h>
#include <media/hardware/CryptoAPI.h>
#include <media/stagefright/foundation/ABase.h>

#ifndef ANDROID_ICRYPTO_H_

#define ANDROID_ICRYPTO_H_

namespace android {

struct AString;
class IMemory;
class IMemoryHeap;

struct ICrypto : public IInterface {
    DECLARE_META_INTERFACE(Crypto);

    virtual status_t initCheck() const = 0;

    virtual bool isCryptoSchemeSupported(const uint8_t uuid[16]) = 0;

    virtual status_t createPlugin(
            const uint8_t uuid[16], const void *data, size_t size) = 0;

    virtual status_t destroyPlugin() = 0;

    virtual bool requiresSecureDecoderComponent(
            const char *mime) const = 0;

    virtual void notifyResolution(uint32_t width, uint32_t height) = 0;

    virtual status_t setMediaDrmSession(const Vector<uint8_t> &sessionId) = 0;

    struct SourceBuffer {
        sp<IMemory> mSharedMemory;
        int32_t mHeapSeqNum;
    };

    enum DestinationType {
        kDestinationTypeSharedMemory, // non-secure
        kDestinationTypeNativeHandle  // secure
    };

    struct DestinationBuffer {
        DestinationType mType;
        native_handle_t *mHandle;
        sp<IMemory> mSharedMemory;
    };

    virtual ssize_t decrypt(const uint8_t key[16], const uint8_t iv[16],
            CryptoPlugin::Mode mode, const CryptoPlugin::Pattern &pattern,
            const SourceBuffer &source, size_t offset,
            const CryptoPlugin::SubSample *subSamples, size_t numSubSamples,
            const DestinationBuffer &destination, AString *errorDetailMsg) = 0;

    /**
     * Declare the heap that the shared memory source buffers passed
     * to decrypt will be allocated from. Returns a sequence number
     * that subsequent decrypt calls can use to refer to the heap,
     * with -1 indicating failure.
     */
    virtual int32_t setHeap(const sp<IMemoryHeap>& heap) = 0;
    virtual void unsetHeap(int32_t seqNum) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(ICrypto);
};

struct BnCrypto : public BnInterface<ICrypto> {
    virtual status_t onTransact(
            uint32_t code, const Parcel &data, Parcel *reply,
            uint32_t flags = 0);
private:
    void readVector(const Parcel &data, Vector<uint8_t> &vector) const;
    void writeVector(Parcel *reply, Vector<uint8_t> const &vector) const;
};

}  // namespace android

#endif // ANDROID_ICRYPTO_H_
