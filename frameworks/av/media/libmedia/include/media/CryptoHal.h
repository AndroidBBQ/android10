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

#ifndef CRYPTO_HAL_H_

#define CRYPTO_HAL_H_

#include <android/hardware/drm/1.0/ICryptoFactory.h>
#include <android/hardware/drm/1.0/ICryptoPlugin.h>
#include <android/hardware/drm/1.1/ICryptoFactory.h>
#include <android/hardware/drm/1.2/ICryptoPlugin.h>

#include <mediadrm/ICrypto.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>

namespace drm = ::android::hardware::drm;
using drm::V1_0::ICryptoFactory;
using drm::V1_0::ICryptoPlugin;
using drm::V1_0::SharedBuffer;

class IMemoryHeap;

namespace android {

struct CryptoHal : public BnCrypto {
    CryptoHal();
    virtual ~CryptoHal();

    virtual status_t initCheck() const;

    virtual bool isCryptoSchemeSupported(const uint8_t uuid[16]);

    virtual status_t createPlugin(
            const uint8_t uuid[16], const void *data, size_t size);

    virtual status_t destroyPlugin();

    virtual bool requiresSecureDecoderComponent(
            const char *mime) const;

    virtual void notifyResolution(uint32_t width, uint32_t height);

    virtual status_t setMediaDrmSession(const Vector<uint8_t> &sessionId);

    virtual ssize_t decrypt(const uint8_t key[16], const uint8_t iv[16],
            CryptoPlugin::Mode mode, const CryptoPlugin::Pattern &pattern,
            const ICrypto::SourceBuffer &source, size_t offset,
            const CryptoPlugin::SubSample *subSamples, size_t numSubSamples,
            const ICrypto::DestinationBuffer &destination,
            AString *errorDetailMsg);

    virtual int32_t setHeap(const sp<IMemoryHeap>& heap) {
        return setHeapBase(heap);
    }
    virtual void unsetHeap(int32_t seqNum) { clearHeapBase(seqNum); }

private:
    mutable Mutex mLock;

    const Vector<sp<ICryptoFactory>> mFactories;
    sp<ICryptoPlugin> mPlugin;
    sp<drm::V1_2::ICryptoPlugin> mPluginV1_2;

    /**
     * mInitCheck is:
     *   NO_INIT if a plugin hasn't been created yet
     *   ERROR_UNSUPPORTED if a plugin can't be created for the uuid
     *   OK after a plugin has been created and mPlugin is valid
     */
    status_t mInitCheck;

    struct HeapBase {
        HeapBase() : mBufferId(0), mSize(0) {}
        HeapBase(uint32_t bufferId, size_t size) :
            mBufferId(bufferId), mSize(size) {}

        uint32_t getBufferId() const {return mBufferId;}
        size_t getSize() const {return mSize;}

    private:
        uint32_t mBufferId;
        size_t mSize;
    };

    KeyedVector<int32_t, HeapBase> mHeapBases;
    uint32_t mNextBufferId;
    int32_t mHeapSeqNum;

    Vector<sp<ICryptoFactory>> makeCryptoFactories();
    sp<ICryptoPlugin> makeCryptoPlugin(const sp<ICryptoFactory>& factory,
            const uint8_t uuid[16], const void *initData, size_t size);

    int32_t setHeapBase(const sp<IMemoryHeap>& heap);
    void clearHeapBase(int32_t seqNum);

    status_t toSharedBuffer(const sp<IMemory>& memory, int32_t seqNum, ::SharedBuffer* buffer);

    DISALLOW_EVIL_CONSTRUCTORS(CryptoHal);
};

}  // namespace android

#endif  // CRYPTO_HAL_H_
