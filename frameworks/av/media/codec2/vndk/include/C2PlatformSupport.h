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

#ifndef STAGEFRIGHT_CODEC2_PLATFORM_SUPPORT_H_
#define STAGEFRIGHT_CODEC2_PLATFORM_SUPPORT_H_

#include <C2Component.h>
#include <C2ComponentFactory.h>

#include <memory>

namespace android {

/**
 * Returns the platform allocator store.
 * \retval nullptr if the platform allocator store could not be obtained
 */
std::shared_ptr<C2AllocatorStore> GetCodec2PlatformAllocatorStore();

/**
 * Platform allocator store IDs
 */
class C2PlatformAllocatorStore : public C2AllocatorStore {
public:
    enum : id_t {
        /**
         * ID of the ion backed platform allocator.
         *
         * C2Handle consists of:
         *   fd  shared ion buffer handle
         *   int size (lo 32 bits)
         *   int size (hi 32 bits)
         *   int magic '\xc2io\x00'
         */
        ION = PLATFORM_START,

        /**
         * ID of the gralloc backed platform allocator.
         *
         * C2Handle layout is not public. Use C2AllocatorGralloc::UnwrapNativeCodec2GrallocHandle
         * to get the underlying gralloc handle from a C2Handle, and WrapNativeCodec2GrallocHandle
         * to create a C2Handle from a gralloc handle - for C2Allocator::priorAllocation.
         */
        GRALLOC,

        /**
         * ID of the bufferqueue backed platform allocator.
         *
         * C2Handle layout is not public. Use C2AllocatorGralloc::UnwrapNativeCodec2GrallocHandle
         * to get the underlying handle from a C2Handle, and WrapNativeCodec2GrallocHandle
         * to create a C2Handle from a handle - for C2Allocator::priorAllocation.
         */
        BUFFERQUEUE,

        /**
         * ID of indicating the end of platform allocator definition.
         *
         * \note always put this macro in the last place.
         *
         * Extended platform store plugin should use this macro as the start ID of its own allocator
         * types.
         */
        PLATFORM_END,
    };
};

/**
 * Retrieves a block pool for a component.
 *
 * \param id        the local ID of the block pool
 * \param component the component using the block pool (must be non-null)
 * \param pool      pointer to where the obtained block pool shall be stored on success. nullptr
 *                  will be stored here on failure
 *
 * \retval C2_OK        the operation was successful
 * \retval C2_BAD_VALUE the component is null
 * \retval C2_NOT_FOUND if the block pool does not exist
 * \retval C2_NO_MEMORY not enough memory to fetch the block pool (this return value is only
 *                      possible for basic pools)
 * \retval C2_TIMED_OUT the operation timed out (this return value is only possible for basic pools)
 * \retval C2_REFUSED   no permission to complete any required allocation (this return value is only
 *                      possible for basic pools)
 * \retval C2_CORRUPTED some unknown, unrecoverable error occured during operation (unexpected,
 *                      this return value is only possible for basic pools)
 */
c2_status_t GetCodec2BlockPool(
        C2BlockPool::local_id_t id, std::shared_ptr<const C2Component> component,
        std::shared_ptr<C2BlockPool> *pool);

/**
 * Creates a block pool.
 * \param allocatorId  the allocator ID which is used to allocate blocks
 * \param component     the component using the block pool (must be non-null)
 * \param pool          pointer to where the created block pool shall be store on success.
 *                      nullptr will be stored here on failure
 *
 * \retval C2_OK        the operation was successful
 * \retval C2_BAD_VALUE the component is null
 * \retval C2_NOT_FOUND if the allocator does not exist
 * \retval C2_NO_MEMORY not enough memory to create a block pool
 */
c2_status_t CreateCodec2BlockPool(
        C2PlatformAllocatorStore::id_t allocatorId,
        std::shared_ptr<const C2Component> component,
        std::shared_ptr<C2BlockPool> *pool);

/**
 * Returns the platform component store.
 * \retval nullptr if the platform component store could not be obtained
 */
std::shared_ptr<C2ComponentStore> GetCodec2PlatformComponentStore();

/**
 * Sets the preferred component store in this process for the sole purpose of accessing its
 * interface. If this is not called, the default IComponentStore HAL (if exists) is the preferred
 * store for this purpose. If the default IComponentStore HAL is not present, the platform
 * component store is used.
 */
void SetPreferredCodec2ComponentStore(std::shared_ptr<C2ComponentStore> store);

} // namespace android

#endif // STAGEFRIGHT_CODEC2_PLATFORM_SUPPORT_H_
