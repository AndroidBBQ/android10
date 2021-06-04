/*
 * Copyright 2019, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CCodecBuffers"
#include <utils/Log.h>

#include <C2PlatformSupport.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaCodecConstants.h>

#include "CCodecBuffers.h"

namespace android {

namespace {

sp<GraphicBlockBuffer> AllocateGraphicBuffer(
        const std::shared_ptr<C2BlockPool> &pool,
        const sp<AMessage> &format,
        uint32_t pixelFormat,
        const C2MemoryUsage &usage,
        const std::shared_ptr<LocalBufferPool> &localBufferPool) {
    int32_t width, height;
    if (!format->findInt32("width", &width) || !format->findInt32("height", &height)) {
        ALOGD("format lacks width or height");
        return nullptr;
    }

    std::shared_ptr<C2GraphicBlock> block;
    c2_status_t err = pool->fetchGraphicBlock(
            width, height, pixelFormat, usage, &block);
    if (err != C2_OK) {
        ALOGD("fetch graphic block failed: %d", err);
        return nullptr;
    }

    return GraphicBlockBuffer::Allocate(
            format,
            block,
            [localBufferPool](size_t capacity) {
                return localBufferPool->newBuffer(capacity);
            });
}

}  // namespace

// CCodecBuffers

void CCodecBuffers::setFormat(const sp<AMessage> &format) {
    CHECK(format != nullptr);
    mFormat = format;
}

sp<AMessage> CCodecBuffers::dupFormat() {
    return mFormat != nullptr ? mFormat->dup() : nullptr;
}

void CCodecBuffers::handleImageData(const sp<Codec2Buffer> &buffer) {
    sp<ABuffer> imageDataCandidate = buffer->getImageData();
    if (imageDataCandidate == nullptr) {
        return;
    }
    sp<ABuffer> imageData;
    if (!mFormat->findBuffer("image-data", &imageData)
            || imageDataCandidate->size() != imageData->size()
            || memcmp(imageDataCandidate->data(), imageData->data(), imageData->size()) != 0) {
        ALOGD("[%s] updating image-data", mName);
        sp<AMessage> newFormat = dupFormat();
        newFormat->setBuffer("image-data", imageDataCandidate);
        MediaImage2 *img = (MediaImage2*)imageDataCandidate->data();
        if (img->mNumPlanes > 0 && img->mType != img->MEDIA_IMAGE_TYPE_UNKNOWN) {
            int32_t stride = img->mPlane[0].mRowInc;
            newFormat->setInt32(KEY_STRIDE, stride);
            ALOGD("[%s] updating stride = %d", mName, stride);
            if (img->mNumPlanes > 1 && stride > 0) {
                int32_t vstride = (img->mPlane[1].mOffset - img->mPlane[0].mOffset) / stride;
                newFormat->setInt32(KEY_SLICE_HEIGHT, vstride);
                ALOGD("[%s] updating vstride = %d", mName, vstride);
            }
        }
        setFormat(newFormat);
        buffer->setFormat(newFormat);
    }
}

// InputBuffers

sp<Codec2Buffer> InputBuffers::cloneAndReleaseBuffer(const sp<MediaCodecBuffer> &buffer) {
    sp<Codec2Buffer> copy = createNewBuffer();
    if (copy == nullptr) {
        return nullptr;
    }
    std::shared_ptr<C2Buffer> c2buffer;
    if (!releaseBuffer(buffer, &c2buffer, true)) {
        return nullptr;
    }
    if (!copy->canCopy(c2buffer)) {
        return nullptr;
    }
    if (!copy->copy(c2buffer)) {
        return nullptr;
    }
    return copy;
}

// OutputBuffers

void OutputBuffers::initSkipCutBuffer(
        int32_t delay, int32_t padding, int32_t sampleRate, int32_t channelCount) {
    CHECK(mSkipCutBuffer == nullptr);
    mDelay = delay;
    mPadding = padding;
    mSampleRate = sampleRate;
    setSkipCutBuffer(delay, padding, channelCount);
}

void OutputBuffers::updateSkipCutBuffer(int32_t sampleRate, int32_t channelCount) {
    if (mSkipCutBuffer == nullptr) {
        return;
    }
    int32_t delay = mDelay;
    int32_t padding = mPadding;
    if (sampleRate != mSampleRate) {
        delay = ((int64_t)delay * sampleRate) / mSampleRate;
        padding = ((int64_t)padding * sampleRate) / mSampleRate;
    }
    setSkipCutBuffer(delay, padding, channelCount);
}

void OutputBuffers::submit(const sp<MediaCodecBuffer> &buffer) {
    if (mSkipCutBuffer != nullptr) {
        mSkipCutBuffer->submit(buffer);
    }
}

void OutputBuffers::transferSkipCutBuffer(const sp<SkipCutBuffer> &scb) {
    mSkipCutBuffer = scb;
}

void OutputBuffers::setSkipCutBuffer(int32_t skip, int32_t cut, int32_t channelCount) {
    if (mSkipCutBuffer != nullptr) {
        size_t prevSize = mSkipCutBuffer->size();
        if (prevSize != 0u) {
            ALOGD("[%s] Replacing SkipCutBuffer holding %zu bytes", mName, prevSize);
        }
    }
    mSkipCutBuffer = new SkipCutBuffer(skip, cut, channelCount);
}

// LocalBufferPool

std::shared_ptr<LocalBufferPool> LocalBufferPool::Create(size_t poolCapacity) {
    return std::shared_ptr<LocalBufferPool>(new LocalBufferPool(poolCapacity));
}

sp<ABuffer> LocalBufferPool::newBuffer(size_t capacity) {
    Mutex::Autolock lock(mMutex);
    auto it = std::find_if(
            mPool.begin(), mPool.end(),
            [capacity](const std::vector<uint8_t> &vec) {
                return vec.capacity() >= capacity;
            });
    if (it != mPool.end()) {
        sp<ABuffer> buffer = new VectorBuffer(std::move(*it), shared_from_this());
        mPool.erase(it);
        return buffer;
    }
    if (mUsedSize + capacity > mPoolCapacity) {
        while (!mPool.empty()) {
            mUsedSize -= mPool.back().capacity();
            mPool.pop_back();
        }
        if (mUsedSize + capacity > mPoolCapacity) {
            ALOGD("mUsedSize = %zu, capacity = %zu, mPoolCapacity = %zu",
                    mUsedSize, capacity, mPoolCapacity);
            return nullptr;
        }
    }
    std::vector<uint8_t> vec(capacity);
    mUsedSize += vec.capacity();
    return new VectorBuffer(std::move(vec), shared_from_this());
}

LocalBufferPool::VectorBuffer::VectorBuffer(
        std::vector<uint8_t> &&vec, const std::shared_ptr<LocalBufferPool> &pool)
    : ABuffer(vec.data(), vec.capacity()),
      mVec(std::move(vec)),
      mPool(pool) {
}

LocalBufferPool::VectorBuffer::~VectorBuffer() {
    std::shared_ptr<LocalBufferPool> pool = mPool.lock();
    if (pool) {
        // If pool is alive, return the vector back to the pool so that
        // it can be recycled.
        pool->returnVector(std::move(mVec));
    }
}

void LocalBufferPool::returnVector(std::vector<uint8_t> &&vec) {
    Mutex::Autolock lock(mMutex);
    mPool.push_front(std::move(vec));
}

// FlexBuffersImpl

size_t FlexBuffersImpl::assignSlot(const sp<Codec2Buffer> &buffer) {
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        if (mBuffers[i].clientBuffer == nullptr
                && mBuffers[i].compBuffer.expired()) {
            mBuffers[i].clientBuffer = buffer;
            return i;
        }
    }
    mBuffers.push_back({ buffer, std::weak_ptr<C2Buffer>() });
    return mBuffers.size() - 1;
}

bool FlexBuffersImpl::releaseSlot(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    sp<Codec2Buffer> clientBuffer;
    size_t index = mBuffers.size();
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        if (mBuffers[i].clientBuffer == buffer) {
            clientBuffer = mBuffers[i].clientBuffer;
            if (release) {
                mBuffers[i].clientBuffer.clear();
            }
            index = i;
            break;
        }
    }
    if (clientBuffer == nullptr) {
        ALOGV("[%s] %s: No matching buffer found", mName, __func__);
        return false;
    }
    std::shared_ptr<C2Buffer> result = mBuffers[index].compBuffer.lock();
    if (!result) {
        result = clientBuffer->asC2Buffer();
        mBuffers[index].compBuffer = result;
    }
    if (c2buffer) {
        *c2buffer = result;
    }
    return true;
}

bool FlexBuffersImpl::expireComponentBuffer(const std::shared_ptr<C2Buffer> &c2buffer) {
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        std::shared_ptr<C2Buffer> compBuffer =
                mBuffers[i].compBuffer.lock();
        if (!compBuffer || compBuffer != c2buffer) {
            continue;
        }
        mBuffers[i].compBuffer.reset();
        ALOGV("[%s] codec released buffer #%zu", mName, i);
        return true;
    }
    ALOGV("[%s] codec released an unknown buffer", mName);
    return false;
}

void FlexBuffersImpl::flush() {
    ALOGV("[%s] buffers are flushed %zu", mName, mBuffers.size());
    mBuffers.clear();
}

size_t FlexBuffersImpl::numClientBuffers() const {
    return std::count_if(
            mBuffers.begin(), mBuffers.end(),
            [](const Entry &entry) {
                return (entry.clientBuffer != nullptr);
            });
}

size_t FlexBuffersImpl::numComponentBuffers() const {
    return std::count_if(
            mBuffers.begin(), mBuffers.end(),
            [](const Entry &entry) {
                return !entry.compBuffer.expired();
            });
}

// BuffersArrayImpl

void BuffersArrayImpl::initialize(
        const FlexBuffersImpl &impl,
        size_t minSize,
        std::function<sp<Codec2Buffer>()> allocate) {
    mImplName = impl.mImplName + "[N]";
    mName = mImplName.c_str();
    for (size_t i = 0; i < impl.mBuffers.size(); ++i) {
        sp<Codec2Buffer> clientBuffer = impl.mBuffers[i].clientBuffer;
        bool ownedByClient = (clientBuffer != nullptr);
        if (!ownedByClient) {
            clientBuffer = allocate();
        }
        mBuffers.push_back({ clientBuffer, impl.mBuffers[i].compBuffer, ownedByClient });
    }
    ALOGV("[%s] converted %zu buffers to array mode of %zu", mName, mBuffers.size(), minSize);
    for (size_t i = impl.mBuffers.size(); i < minSize; ++i) {
        mBuffers.push_back({ allocate(), std::weak_ptr<C2Buffer>(), false });
    }
}

status_t BuffersArrayImpl::grabBuffer(
        size_t *index,
        sp<Codec2Buffer> *buffer,
        std::function<bool(const sp<Codec2Buffer> &)> match) {
    // allBuffersDontMatch remains true if all buffers are available but
    // match() returns false for every buffer.
    bool allBuffersDontMatch = true;
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        if (!mBuffers[i].ownedByClient && mBuffers[i].compBuffer.expired()) {
            if (match(mBuffers[i].clientBuffer)) {
                mBuffers[i].ownedByClient = true;
                *buffer = mBuffers[i].clientBuffer;
                (*buffer)->meta()->clear();
                (*buffer)->setRange(0, (*buffer)->capacity());
                *index = i;
                return OK;
            }
        } else {
            allBuffersDontMatch = false;
        }
    }
    return allBuffersDontMatch ? NO_MEMORY : WOULD_BLOCK;
}

bool BuffersArrayImpl::returnBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    sp<Codec2Buffer> clientBuffer;
    size_t index = mBuffers.size();
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        if (mBuffers[i].clientBuffer == buffer) {
            if (!mBuffers[i].ownedByClient) {
                ALOGD("[%s] Client returned a buffer it does not own according to our record: %zu",
                      mName, i);
            }
            clientBuffer = mBuffers[i].clientBuffer;
            if (release) {
                mBuffers[i].ownedByClient = false;
            }
            index = i;
            break;
        }
    }
    if (clientBuffer == nullptr) {
        ALOGV("[%s] %s: No matching buffer found", mName, __func__);
        return false;
    }
    ALOGV("[%s] %s: matching buffer found (index=%zu)", mName, __func__, index);
    std::shared_ptr<C2Buffer> result = mBuffers[index].compBuffer.lock();
    if (!result) {
        result = clientBuffer->asC2Buffer();
        mBuffers[index].compBuffer = result;
    }
    if (c2buffer) {
        *c2buffer = result;
    }
    return true;
}

bool BuffersArrayImpl::expireComponentBuffer(const std::shared_ptr<C2Buffer> &c2buffer) {
    for (size_t i = 0; i < mBuffers.size(); ++i) {
        std::shared_ptr<C2Buffer> compBuffer =
                mBuffers[i].compBuffer.lock();
        if (!compBuffer) {
            continue;
        }
        if (c2buffer == compBuffer) {
            if (mBuffers[i].ownedByClient) {
                // This should not happen.
                ALOGD("[%s] codec released a buffer owned by client "
                      "(index %zu)", mName, i);
            }
            mBuffers[i].compBuffer.reset();
            ALOGV("[%s] codec released buffer #%zu(array mode)", mName, i);
            return true;
        }
    }
    ALOGV("[%s] codec released an unknown buffer (array mode)", mName);
    return false;
}

void BuffersArrayImpl::getArray(Vector<sp<MediaCodecBuffer>> *array) const {
    array->clear();
    for (const Entry &entry : mBuffers) {
        array->push(entry.clientBuffer);
    }
}

void BuffersArrayImpl::flush() {
    for (Entry &entry : mBuffers) {
        entry.ownedByClient = false;
    }
}

void BuffersArrayImpl::realloc(std::function<sp<Codec2Buffer>()> alloc) {
    size_t size = mBuffers.size();
    mBuffers.clear();
    for (size_t i = 0; i < size; ++i) {
        mBuffers.push_back({ alloc(), std::weak_ptr<C2Buffer>(), false });
    }
}

void BuffersArrayImpl::grow(
        size_t newSize, std::function<sp<Codec2Buffer>()> alloc) {
    CHECK_LT(mBuffers.size(), newSize);
    while (mBuffers.size() < newSize) {
        mBuffers.push_back({ alloc(), std::weak_ptr<C2Buffer>(), false });
    }
}

size_t BuffersArrayImpl::numClientBuffers() const {
    return std::count_if(
            mBuffers.begin(), mBuffers.end(),
            [](const Entry &entry) {
                return entry.ownedByClient;
            });
}

size_t BuffersArrayImpl::arraySize() const {
    return mBuffers.size();
}

// InputBuffersArray

void InputBuffersArray::initialize(
        const FlexBuffersImpl &impl,
        size_t minSize,
        std::function<sp<Codec2Buffer>()> allocate) {
    mAllocate = allocate;
    mImpl.initialize(impl, minSize, allocate);
}

void InputBuffersArray::getArray(Vector<sp<MediaCodecBuffer>> *array) const {
    mImpl.getArray(array);
}

bool InputBuffersArray::requestNewBuffer(size_t *index, sp<MediaCodecBuffer> *buffer) {
    sp<Codec2Buffer> c2Buffer;
    status_t err = mImpl.grabBuffer(index, &c2Buffer);
    if (err == OK) {
        c2Buffer->setFormat(mFormat);
        handleImageData(c2Buffer);
        *buffer = c2Buffer;
        return true;
    }
    return false;
}

bool InputBuffersArray::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    return mImpl.returnBuffer(buffer, c2buffer, release);
}

bool InputBuffersArray::expireComponentBuffer(
        const std::shared_ptr<C2Buffer> &c2buffer) {
    return mImpl.expireComponentBuffer(c2buffer);
}

void InputBuffersArray::flush() {
    mImpl.flush();
}

size_t InputBuffersArray::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

sp<Codec2Buffer> InputBuffersArray::createNewBuffer() {
    return mAllocate();
}

// LinearInputBuffers

bool LinearInputBuffers::requestNewBuffer(size_t *index, sp<MediaCodecBuffer> *buffer) {
    sp<Codec2Buffer> newBuffer = createNewBuffer();
    if (newBuffer == nullptr) {
        return false;
    }
    *index = mImpl.assignSlot(newBuffer);
    *buffer = newBuffer;
    return true;
}

bool LinearInputBuffers::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    return mImpl.releaseSlot(buffer, c2buffer, release);
}

bool LinearInputBuffers::expireComponentBuffer(
        const std::shared_ptr<C2Buffer> &c2buffer) {
    return mImpl.expireComponentBuffer(c2buffer);
}

void LinearInputBuffers::flush() {
    // This is no-op by default unless we're in array mode where we need to keep
    // track of the flushed work.
    mImpl.flush();
}

std::unique_ptr<InputBuffers> LinearInputBuffers::toArrayMode(size_t size) {
    std::unique_ptr<InputBuffersArray> array(
            new InputBuffersArray(mComponentName.c_str(), "1D-Input[N]"));
    array->setPool(mPool);
    array->setFormat(mFormat);
    array->initialize(
            mImpl,
            size,
            [pool = mPool, format = mFormat] () -> sp<Codec2Buffer> {
                return Alloc(pool, format);
            });
    return std::move(array);
}

size_t LinearInputBuffers::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

// static
sp<Codec2Buffer> LinearInputBuffers::Alloc(
        const std::shared_ptr<C2BlockPool> &pool, const sp<AMessage> &format) {
    int32_t capacity = kLinearBufferSize;
    (void)format->findInt32(KEY_MAX_INPUT_SIZE, &capacity);
    if ((size_t)capacity > kMaxLinearBufferSize) {
        ALOGD("client requested %d, capped to %zu", capacity, kMaxLinearBufferSize);
        capacity = kMaxLinearBufferSize;
    }

    // TODO: read usage from intf
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    std::shared_ptr<C2LinearBlock> block;

    c2_status_t err = pool->fetchLinearBlock(capacity, usage, &block);
    if (err != C2_OK) {
        return nullptr;
    }

    return LinearBlockBuffer::Allocate(format, block);
}

sp<Codec2Buffer> LinearInputBuffers::createNewBuffer() {
    return Alloc(mPool, mFormat);
}

// EncryptedLinearInputBuffers

EncryptedLinearInputBuffers::EncryptedLinearInputBuffers(
        bool secure,
        const sp<MemoryDealer> &dealer,
        const sp<ICrypto> &crypto,
        int32_t heapSeqNum,
        size_t capacity,
        size_t numInputSlots,
        const char *componentName, const char *name)
    : LinearInputBuffers(componentName, name),
      mUsage({0, 0}),
      mDealer(dealer),
      mCrypto(crypto),
      mMemoryVector(new std::vector<Entry>){
    if (secure) {
        mUsage = { C2MemoryUsage::READ_PROTECTED, 0 };
    } else {
        mUsage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    }
    for (size_t i = 0; i < numInputSlots; ++i) {
        sp<IMemory> memory = mDealer->allocate(capacity);
        if (memory == nullptr) {
            ALOGD("[%s] Failed to allocate memory from dealer: only %zu slots allocated",
                  mName, i);
            break;
        }
        mMemoryVector->push_back({std::weak_ptr<C2LinearBlock>(), memory, heapSeqNum});
    }
}

std::unique_ptr<InputBuffers> EncryptedLinearInputBuffers::toArrayMode(size_t size) {
    std::unique_ptr<InputBuffersArray> array(
            new InputBuffersArray(mComponentName.c_str(), "1D-EncryptedInput[N]"));
    array->setPool(mPool);
    array->setFormat(mFormat);
    array->initialize(
            mImpl,
            size,
            [pool = mPool,
             format = mFormat,
             usage = mUsage,
             memoryVector = mMemoryVector] () -> sp<Codec2Buffer> {
                return Alloc(pool, format, usage, memoryVector);
            });
    return std::move(array);
}


// static
sp<Codec2Buffer> EncryptedLinearInputBuffers::Alloc(
        const std::shared_ptr<C2BlockPool> &pool,
        const sp<AMessage> &format,
        C2MemoryUsage usage,
        const std::shared_ptr<std::vector<EncryptedLinearInputBuffers::Entry>> &memoryVector) {
    int32_t capacity = kLinearBufferSize;
    (void)format->findInt32(KEY_MAX_INPUT_SIZE, &capacity);
    if ((size_t)capacity > kMaxLinearBufferSize) {
        ALOGD("client requested %d, capped to %zu", capacity, kMaxLinearBufferSize);
        capacity = kMaxLinearBufferSize;
    }

    sp<IMemory> memory;
    size_t slot = 0;
    int32_t heapSeqNum = -1;
    for (; slot < memoryVector->size(); ++slot) {
        if (memoryVector->at(slot).block.expired()) {
            memory = memoryVector->at(slot).memory;
            heapSeqNum = memoryVector->at(slot).heapSeqNum;
            break;
        }
    }
    if (memory == nullptr) {
        return nullptr;
    }

    std::shared_ptr<C2LinearBlock> block;
    c2_status_t err = pool->fetchLinearBlock(capacity, usage, &block);
    if (err != C2_OK || block == nullptr) {
        return nullptr;
    }

    memoryVector->at(slot).block = block;
    return new EncryptedLinearBlockBuffer(format, block, memory, heapSeqNum);
}

sp<Codec2Buffer> EncryptedLinearInputBuffers::createNewBuffer() {
    // TODO: android_2020
    return nullptr;
}

// GraphicMetadataInputBuffers

GraphicMetadataInputBuffers::GraphicMetadataInputBuffers(
        const char *componentName, const char *name)
    : InputBuffers(componentName, name),
      mImpl(mName),
      mStore(GetCodec2PlatformAllocatorStore()) { }

bool GraphicMetadataInputBuffers::requestNewBuffer(
        size_t *index, sp<MediaCodecBuffer> *buffer) {
    sp<Codec2Buffer> newBuffer = createNewBuffer();
    if (newBuffer == nullptr) {
        return false;
    }
    *index = mImpl.assignSlot(newBuffer);
    *buffer = newBuffer;
    return true;
}

bool GraphicMetadataInputBuffers::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    return mImpl.releaseSlot(buffer, c2buffer, release);
}

bool GraphicMetadataInputBuffers::expireComponentBuffer(
        const std::shared_ptr<C2Buffer> &c2buffer) {
    return mImpl.expireComponentBuffer(c2buffer);
}

void GraphicMetadataInputBuffers::flush() {
    // This is no-op by default unless we're in array mode where we need to keep
    // track of the flushed work.
}

std::unique_ptr<InputBuffers> GraphicMetadataInputBuffers::toArrayMode(
        size_t size) {
    std::shared_ptr<C2Allocator> alloc;
    c2_status_t err = mStore->fetchAllocator(mPool->getAllocatorId(), &alloc);
    if (err != C2_OK) {
        return nullptr;
    }
    std::unique_ptr<InputBuffersArray> array(
            new InputBuffersArray(mComponentName.c_str(), "2D-MetaInput[N]"));
    array->setPool(mPool);
    array->setFormat(mFormat);
    array->initialize(
            mImpl,
            size,
            [format = mFormat, alloc]() -> sp<Codec2Buffer> {
                return new GraphicMetadataBuffer(format, alloc);
            });
    return std::move(array);
}

size_t GraphicMetadataInputBuffers::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

sp<Codec2Buffer> GraphicMetadataInputBuffers::createNewBuffer() {
    std::shared_ptr<C2Allocator> alloc;
    c2_status_t err = mStore->fetchAllocator(mPool->getAllocatorId(), &alloc);
    if (err != C2_OK) {
        return nullptr;
    }
    return new GraphicMetadataBuffer(mFormat, alloc);
}

// GraphicInputBuffers

GraphicInputBuffers::GraphicInputBuffers(
        size_t numInputSlots, const char *componentName, const char *name)
    : InputBuffers(componentName, name),
      mImpl(mName),
      mLocalBufferPool(LocalBufferPool::Create(
              kMaxLinearBufferSize * numInputSlots)) { }

bool GraphicInputBuffers::requestNewBuffer(size_t *index, sp<MediaCodecBuffer> *buffer) {
    sp<Codec2Buffer> newBuffer = createNewBuffer();
    if (newBuffer == nullptr) {
        return false;
    }
    *index = mImpl.assignSlot(newBuffer);
    handleImageData(newBuffer);
    *buffer = newBuffer;
    return true;
}

bool GraphicInputBuffers::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer,
        bool release) {
    return mImpl.releaseSlot(buffer, c2buffer, release);
}

bool GraphicInputBuffers::expireComponentBuffer(
        const std::shared_ptr<C2Buffer> &c2buffer) {
    return mImpl.expireComponentBuffer(c2buffer);
}

void GraphicInputBuffers::flush() {
    // This is no-op by default unless we're in array mode where we need to keep
    // track of the flushed work.
}

std::unique_ptr<InputBuffers> GraphicInputBuffers::toArrayMode(size_t size) {
    std::unique_ptr<InputBuffersArray> array(
            new InputBuffersArray(mComponentName.c_str(), "2D-BB-Input[N]"));
    array->setPool(mPool);
    array->setFormat(mFormat);
    array->initialize(
            mImpl,
            size,
            [pool = mPool, format = mFormat, lbp = mLocalBufferPool]() -> sp<Codec2Buffer> {
                C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
                return AllocateGraphicBuffer(
                        pool, format, HAL_PIXEL_FORMAT_YV12, usage, lbp);
            });
    return std::move(array);
}

size_t GraphicInputBuffers::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

sp<Codec2Buffer> GraphicInputBuffers::createNewBuffer() {
    // TODO: read usage from intf
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    return AllocateGraphicBuffer(
            mPool, mFormat, HAL_PIXEL_FORMAT_YV12, usage, mLocalBufferPool);
}

// OutputBuffersArray

void OutputBuffersArray::initialize(
        const FlexBuffersImpl &impl,
        size_t minSize,
        std::function<sp<Codec2Buffer>()> allocate) {
    mAlloc = allocate;
    mImpl.initialize(impl, minSize, allocate);
}

status_t OutputBuffersArray::registerBuffer(
        const std::shared_ptr<C2Buffer> &buffer,
        size_t *index,
        sp<MediaCodecBuffer> *clientBuffer) {
    sp<Codec2Buffer> c2Buffer;
    status_t err = mImpl.grabBuffer(
            index,
            &c2Buffer,
            [buffer](const sp<Codec2Buffer> &clientBuffer) {
                return clientBuffer->canCopy(buffer);
            });
    if (err == WOULD_BLOCK) {
        ALOGV("[%s] buffers temporarily not available", mName);
        return err;
    } else if (err != OK) {
        ALOGD("[%s] grabBuffer failed: %d", mName, err);
        return err;
    }
    c2Buffer->setFormat(mFormat);
    if (!c2Buffer->copy(buffer)) {
        ALOGD("[%s] copy buffer failed", mName);
        return WOULD_BLOCK;
    }
    submit(c2Buffer);
    handleImageData(c2Buffer);
    *clientBuffer = c2Buffer;
    ALOGV("[%s] grabbed buffer %zu", mName, *index);
    return OK;
}

status_t OutputBuffersArray::registerCsd(
        const C2StreamInitDataInfo::output *csd,
        size_t *index,
        sp<MediaCodecBuffer> *clientBuffer) {
    sp<Codec2Buffer> c2Buffer;
    status_t err = mImpl.grabBuffer(
            index,
            &c2Buffer,
            [csd](const sp<Codec2Buffer> &clientBuffer) {
                return clientBuffer->base() != nullptr
                        && clientBuffer->capacity() >= csd->flexCount();
            });
    if (err != OK) {
        return err;
    }
    memcpy(c2Buffer->base(), csd->m.value, csd->flexCount());
    c2Buffer->setRange(0, csd->flexCount());
    c2Buffer->setFormat(mFormat);
    *clientBuffer = c2Buffer;
    return OK;
}

bool OutputBuffersArray::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer, std::shared_ptr<C2Buffer> *c2buffer) {
    return mImpl.returnBuffer(buffer, c2buffer, true);
}

void OutputBuffersArray::flush(const std::list<std::unique_ptr<C2Work>> &flushedWork) {
    (void)flushedWork;
    mImpl.flush();
    if (mSkipCutBuffer != nullptr) {
        mSkipCutBuffer->clear();
    }
}

void OutputBuffersArray::getArray(Vector<sp<MediaCodecBuffer>> *array) const {
    mImpl.getArray(array);
}

size_t OutputBuffersArray::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

void OutputBuffersArray::realloc(const std::shared_ptr<C2Buffer> &c2buffer) {
    switch (c2buffer->data().type()) {
        case C2BufferData::LINEAR: {
            uint32_t size = kLinearBufferSize;
            const std::vector<C2ConstLinearBlock> &linear_blocks = c2buffer->data().linearBlocks();
            const uint32_t block_size = linear_blocks.front().size();
            if (block_size < kMaxLinearBufferSize / 2) {
                size = block_size * 2;
            } else {
                size = kMaxLinearBufferSize;
            }
            mAlloc = [format = mFormat, size] {
                return new LocalLinearBuffer(format, new ABuffer(size));
            };
            ALOGD("[%s] reallocating with linear buffer of size %u", mName, size);
            break;
        }

        case C2BufferData::GRAPHIC: {
            // This is only called for RawGraphicOutputBuffers.
            mAlloc = [format = mFormat,
                      lbp = LocalBufferPool::Create(kMaxLinearBufferSize * mImpl.arraySize())] {
                return ConstGraphicBlockBuffer::AllocateEmpty(
                        format,
                        [lbp](size_t capacity) {
                            return lbp->newBuffer(capacity);
                        });
            };
            ALOGD("[%s] reallocating with graphic buffer: format = %s",
                  mName, mFormat->debugString().c_str());
            break;
        }

        case C2BufferData::INVALID:         [[fallthrough]];
        case C2BufferData::LINEAR_CHUNKS:   [[fallthrough]];
        case C2BufferData::GRAPHIC_CHUNKS:  [[fallthrough]];
        default:
            ALOGD("Unsupported type: %d", (int)c2buffer->data().type());
            return;
    }
    mImpl.realloc(mAlloc);
}

void OutputBuffersArray::grow(size_t newSize) {
    mImpl.grow(newSize, mAlloc);
}

// FlexOutputBuffers

status_t FlexOutputBuffers::registerBuffer(
        const std::shared_ptr<C2Buffer> &buffer,
        size_t *index,
        sp<MediaCodecBuffer> *clientBuffer) {
    sp<Codec2Buffer> newBuffer = wrap(buffer);
    if (newBuffer == nullptr) {
        return NO_MEMORY;
    }
    newBuffer->setFormat(mFormat);
    *index = mImpl.assignSlot(newBuffer);
    handleImageData(newBuffer);
    *clientBuffer = newBuffer;
    ALOGV("[%s] registered buffer %zu", mName, *index);
    return OK;
}

status_t FlexOutputBuffers::registerCsd(
        const C2StreamInitDataInfo::output *csd,
        size_t *index,
        sp<MediaCodecBuffer> *clientBuffer) {
    sp<Codec2Buffer> newBuffer = new LocalLinearBuffer(
            mFormat, ABuffer::CreateAsCopy(csd->m.value, csd->flexCount()));
    *index = mImpl.assignSlot(newBuffer);
    *clientBuffer = newBuffer;
    return OK;
}

bool FlexOutputBuffers::releaseBuffer(
        const sp<MediaCodecBuffer> &buffer,
        std::shared_ptr<C2Buffer> *c2buffer) {
    return mImpl.releaseSlot(buffer, c2buffer, true);
}

void FlexOutputBuffers::flush(
        const std::list<std::unique_ptr<C2Work>> &flushedWork) {
    (void) flushedWork;
    // This is no-op by default unless we're in array mode where we need to keep
    // track of the flushed work.
}

std::unique_ptr<OutputBuffers> FlexOutputBuffers::toArrayMode(size_t size) {
    std::unique_ptr<OutputBuffersArray> array(new OutputBuffersArray(mComponentName.c_str()));
    array->setFormat(mFormat);
    array->transferSkipCutBuffer(mSkipCutBuffer);
    std::function<sp<Codec2Buffer>()> alloc = getAlloc();
    array->initialize(mImpl, size, alloc);
    return std::move(array);
}

size_t FlexOutputBuffers::numClientBuffers() const {
    return mImpl.numClientBuffers();
}

// LinearOutputBuffers

void LinearOutputBuffers::flush(
        const std::list<std::unique_ptr<C2Work>> &flushedWork) {
    if (mSkipCutBuffer != nullptr) {
        mSkipCutBuffer->clear();
    }
    FlexOutputBuffers::flush(flushedWork);
}

sp<Codec2Buffer> LinearOutputBuffers::wrap(const std::shared_ptr<C2Buffer> &buffer) {
    if (buffer == nullptr) {
        ALOGV("[%s] using a dummy buffer", mName);
        return new LocalLinearBuffer(mFormat, new ABuffer(0));
    }
    if (buffer->data().type() != C2BufferData::LINEAR) {
        ALOGV("[%s] non-linear buffer %d", mName, buffer->data().type());
        // We expect linear output buffers from the component.
        return nullptr;
    }
    if (buffer->data().linearBlocks().size() != 1u) {
        ALOGV("[%s] no linear buffers", mName);
        // We expect one and only one linear block from the component.
        return nullptr;
    }
    sp<Codec2Buffer> clientBuffer = ConstLinearBlockBuffer::Allocate(mFormat, buffer);
    if (clientBuffer == nullptr) {
        ALOGD("[%s] ConstLinearBlockBuffer::Allocate failed", mName);
        return nullptr;
    }
    submit(clientBuffer);
    return clientBuffer;
}

std::function<sp<Codec2Buffer>()> LinearOutputBuffers::getAlloc() {
    return [format = mFormat]{
        // TODO: proper max output size
        return new LocalLinearBuffer(format, new ABuffer(kLinearBufferSize));
    };
}

// GraphicOutputBuffers

sp<Codec2Buffer> GraphicOutputBuffers::wrap(const std::shared_ptr<C2Buffer> &buffer) {
    return new DummyContainerBuffer(mFormat, buffer);
}

std::function<sp<Codec2Buffer>()> GraphicOutputBuffers::getAlloc() {
    return [format = mFormat]{
        return new DummyContainerBuffer(format);
    };
}

// RawGraphicOutputBuffers

RawGraphicOutputBuffers::RawGraphicOutputBuffers(
        size_t numOutputSlots, const char *componentName, const char *name)
    : FlexOutputBuffers(componentName, name),
      mLocalBufferPool(LocalBufferPool::Create(
              kMaxLinearBufferSize * numOutputSlots)) { }

sp<Codec2Buffer> RawGraphicOutputBuffers::wrap(const std::shared_ptr<C2Buffer> &buffer) {
    if (buffer == nullptr) {
        sp<Codec2Buffer> c2buffer = ConstGraphicBlockBuffer::AllocateEmpty(
                mFormat,
                [lbp = mLocalBufferPool](size_t capacity) {
                    return lbp->newBuffer(capacity);
                });
        if (c2buffer == nullptr) {
            ALOGD("[%s] ConstGraphicBlockBuffer::AllocateEmpty failed", mName);
            return nullptr;
        }
        c2buffer->setRange(0, 0);
        return c2buffer;
    } else {
        return ConstGraphicBlockBuffer::Allocate(
                mFormat,
                buffer,
                [lbp = mLocalBufferPool](size_t capacity) {
                    return lbp->newBuffer(capacity);
                });
    }
}

std::function<sp<Codec2Buffer>()> RawGraphicOutputBuffers::getAlloc() {
    return [format = mFormat, lbp = mLocalBufferPool]{
        return ConstGraphicBlockBuffer::AllocateEmpty(
                format,
                [lbp](size_t capacity) {
                    return lbp->newBuffer(capacity);
                });
    };
}

}  // namespace android
