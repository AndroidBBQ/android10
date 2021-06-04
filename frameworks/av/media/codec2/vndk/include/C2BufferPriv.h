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

#ifndef STAGEFRIGHT_CODEC2_BUFFER_PRIV_H_
#define STAGEFRIGHT_CODEC2_BUFFER_PRIV_H_

#include <functional>

#include <C2Buffer.h>
#include <android/hardware/media/bufferpool/2.0/IAccessor.h>

class C2BasicLinearBlockPool : public C2BlockPool {
public:
    explicit C2BasicLinearBlockPool(const std::shared_ptr<C2Allocator> &allocator);

    virtual ~C2BasicLinearBlockPool() override = default;

    virtual C2Allocator::id_t getAllocatorId() const override {
        return mAllocator->getId();
    }

    virtual local_id_t getLocalId() const override {
        return BASIC_LINEAR;
    }

    virtual c2_status_t fetchLinearBlock(
            uint32_t capacity,
            C2MemoryUsage usage,
            std::shared_ptr<C2LinearBlock> *block /* nonnull */) override;

    // TODO: fetchCircularBlock

private:
    const std::shared_ptr<C2Allocator> mAllocator;
};

class C2BasicGraphicBlockPool : public C2BlockPool {
public:
    explicit C2BasicGraphicBlockPool(const std::shared_ptr<C2Allocator> &allocator);

    virtual ~C2BasicGraphicBlockPool() override = default;

    virtual C2Allocator::id_t getAllocatorId() const override {
        return mAllocator->getId();
    }

    virtual local_id_t getLocalId() const override {
        return BASIC_GRAPHIC;
    }

    virtual c2_status_t fetchGraphicBlock(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            C2MemoryUsage usage,
            std::shared_ptr<C2GraphicBlock> *block /* nonnull */) override;

private:
    const std::shared_ptr<C2Allocator> mAllocator;
};

class C2PooledBlockPool : public C2BlockPool {
public:
    C2PooledBlockPool(const std::shared_ptr<C2Allocator> &allocator, const local_id_t localId);

    virtual ~C2PooledBlockPool() override;

    virtual C2Allocator::id_t getAllocatorId() const override {
        return mAllocator->getId();
    }

    virtual local_id_t getLocalId() const override {
        return mLocalId;
    }

    virtual c2_status_t fetchLinearBlock(
            uint32_t capacity,
            C2MemoryUsage usage,
            std::shared_ptr<C2LinearBlock> *block /* nonnull */) override;

    virtual c2_status_t fetchGraphicBlock(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            C2MemoryUsage usage,
            std::shared_ptr<C2GraphicBlock> *block) override;

    /**
     * Retrieves the connection Id for underlying bufferpool
     */
    int64_t getConnectionId();

    /**
     * Retrieves the accessor which is used by underlying bufferpool. (It can be
     * passed to receiving process.)
     *
     * \param accessor          IAccessor will be written to this out parameter.
     *
     * \return true             IAcessor is writen successfully.
     * \return false            IAccessor is not written.
     */
    bool getAccessor(android::sp<android::hardware::media::bufferpool::V2_0::IAccessor> *accessor);

private:
    const std::shared_ptr<C2Allocator> mAllocator;
    const local_id_t mLocalId;

    class Impl;
    std::unique_ptr<Impl> mImpl;
};

#endif // STAGEFRIGHT_CODEC2_BUFFER_PRIV_H_
