/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "rsContext.h"
#include "rsAllocation.h"
#include "rs_hal.h"

#ifndef RS_COMPATIBILITY_LIB
#include "rsGrallocConsumer.h"
#endif

namespace android {
namespace renderscript {

Allocation::Allocation(Context *rsc, const Type *type, uint32_t usages,
                       RsAllocationMipmapControl mc, void * ptr)
    : ObjectBase(rsc) {

    memset(&mHal, 0, sizeof(mHal));
    mHal.state.mipmapControl = RS_ALLOCATION_MIPMAP_NONE;
    mHal.state.usageFlags = usages;
    mHal.state.mipmapControl = mc;
    mHal.state.userProvidedPtr = ptr;

    setType(type);
    updateCache();
}

Allocation::Allocation(Context *rsc, const Allocation *alloc, const Type *type)
    : ObjectBase(rsc) {

    memset(&mHal, 0, sizeof(mHal));
    mHal.state.baseAlloc = alloc;
    mHal.state.usageFlags = alloc->mHal.state.usageFlags;
    mHal.state.mipmapControl = RS_ALLOCATION_MIPMAP_NONE;

    setType(type);
    updateCache();
}

void Allocation::operator delete(void* ptr) {
    if (ptr) {
        Allocation *a = (Allocation*) ptr;
        a->getContext()->mHal.funcs.freeRuntimeMem(ptr);
    }
}

Allocation * Allocation::createAllocationStrided(Context *rsc, const Type *type, uint32_t usages,
                                                 RsAllocationMipmapControl mc, void * ptr,
                                                 size_t requiredAlignment) {
    // Allocation objects must use allocator specified by the driver
    void* allocMem = rsc->mHal.funcs.allocRuntimeMem(sizeof(Allocation), 0);

    if (!allocMem) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Couldn't allocate memory for Allocation");
        return nullptr;
    }

    bool success = false;
    Allocation *a = nullptr;
    if (usages & RS_ALLOCATION_USAGE_OEM) {
        if (rsc->mHal.funcs.allocation.initOem != nullptr) {
            a = new (allocMem) Allocation(rsc, type, usages, mc, nullptr);
            success = rsc->mHal.funcs.allocation.initOem(rsc, a, type->getElement()->getHasReferences(), ptr);
        } else {
            rsc->setError(RS_ERROR_FATAL_DRIVER, "Allocation Init called with USAGE_OEM but driver does not support it");
            return nullptr;
        }
#ifdef RS_COMPATIBILITY_LIB
    } else if (usages & RS_ALLOCATION_USAGE_INCREMENTAL_SUPPORT){
        a = new (allocMem) Allocation(rsc, type, usages, mc, ptr);
        success = rsc->mHal.funcs.allocation.initStrided(rsc, a, type->getElement()->getHasReferences(), requiredAlignment);
#endif
    } else {
        a = new (allocMem) Allocation(rsc, type, usages, mc, ptr);
        success = rsc->mHal.funcs.allocation.init(rsc, a, type->getElement()->getHasReferences());
    }

    if (!success) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Allocation::Allocation, alloc failure");
        delete a;
        return nullptr;
    }

    return a;
}

Allocation * Allocation::createAllocation(Context *rsc, const Type *type, uint32_t usages,
                              RsAllocationMipmapControl mc, void * ptr) {
    return Allocation::createAllocationStrided(rsc, type, usages, mc, ptr, kMinimumRSAlignment);
}

Allocation * Allocation::createAdapter(Context *rsc, const Allocation *alloc, const Type *type) {
    // Allocation objects must use allocator specified by the driver
    void* allocMem = rsc->mHal.funcs.allocRuntimeMem(sizeof(Allocation), 0);

    if (!allocMem) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Couldn't allocate memory for Allocation");
        return nullptr;
    }

    Allocation *a = new (allocMem) Allocation(rsc, alloc, type);

    if (!rsc->mHal.funcs.allocation.initAdapter(rsc, a)) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Allocation::Allocation, alloc failure");
        delete a;
        return nullptr;
    }

    return a;
}

void Allocation::adapterOffset(Context *rsc, const uint32_t *offsets, size_t len) {
    if (len >= sizeof(uint32_t) * 9) {
        mHal.state.originX = offsets[0];
        mHal.state.originY = offsets[1];
        mHal.state.originZ = offsets[2];
        mHal.state.originLOD = offsets[3];
        mHal.state.originFace = offsets[4];
        mHal.state.originArray[0] = offsets[5];
        mHal.state.originArray[1] = offsets[6];
        mHal.state.originArray[2] = offsets[7];
        mHal.state.originArray[3] = offsets[8];
    }

    rsc->mHal.funcs.allocation.adapterOffset(rsc, this);
}



void Allocation::updateCache() {
    const Type *type = mHal.state.type;
    mHal.state.yuv = type->getDimYuv();
    mHal.state.hasFaces = type->getDimFaces();
    mHal.state.hasMipmaps = type->getDimLOD();
    mHal.state.elementSizeBytes = type->getElementSizeBytes();
    mHal.state.hasReferences = mHal.state.type->getElement()->getHasReferences();
}

Allocation::~Allocation() {
#ifndef RS_COMPATIBILITY_LIB
    if (mGrallocConsumer) {
        mGrallocConsumer->releaseIdx(mCurrentIdx);
        if (!mGrallocConsumer->isActive()) {
            delete mGrallocConsumer;
        }
        mGrallocConsumer = nullptr;
    }
#endif

    freeChildrenUnlocked();
    mRSC->mHal.funcs.allocation.destroy(mRSC, this);
}

void Allocation::syncAll(Context *rsc, RsAllocationUsageType src) {
    rsc->mHal.funcs.allocation.syncAll(rsc, this, src);
}

void * Allocation::getPointer(const Context *rsc, uint32_t lod, RsAllocationCubemapFace face,
                          uint32_t z, uint32_t array, size_t *stride) {

    if ((lod >= mHal.drvState.lodCount) ||
        (z && (z >= mHal.drvState.lod[lod].dimZ)) ||
        ((face != RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X) && !mHal.state.hasFaces) ||
        (array != 0)) {
        return nullptr;
    }

    if (mRSC->mHal.funcs.allocation.getPointer != nullptr) {
        // Notify the driver, if present that the user is mapping the buffer
        mRSC->mHal.funcs.allocation.getPointer(rsc, this, lod, face, z, array);
    }

    size_t s = 0;
    if ((stride != nullptr) && mHal.drvState.lod[0].dimY) {
        *stride = mHal.drvState.lod[lod].stride;
    }
    return mHal.drvState.lod[lod].mallocPtr;
}

void Allocation::data(Context *rsc, uint32_t xoff, uint32_t lod,
                         uint32_t count, const void *data, size_t sizeBytes) {
    const size_t eSize = mHal.state.type->getElementSizeBytes();

    if ((count * eSize) != sizeBytes) {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "Allocation::subData called with mismatched size expected %zu, got %zu",
                 (count * eSize), sizeBytes);
        rsc->setError(RS_ERROR_BAD_VALUE, buf);
        mHal.state.type->dumpLOGV("type info");
        return;
    }

    rsc->mHal.funcs.allocation.data1D(rsc, this, xoff, lod, count, data, sizeBytes);
    sendDirty(rsc);
}

void Allocation::data(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                      uint32_t w, uint32_t h, const void *data, size_t sizeBytes, size_t stride) {
    rsc->mHal.funcs.allocation.data2D(rsc, this, xoff, yoff, lod, face, w, h, data, sizeBytes, stride);
    sendDirty(rsc);
}

void Allocation::data(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t zoff,
                      uint32_t lod,
                      uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes, size_t stride) {
    rsc->mHal.funcs.allocation.data3D(rsc, this, xoff, yoff, zoff, lod, w, h, d, data, sizeBytes, stride);
    sendDirty(rsc);
}

void Allocation::read(Context *rsc, uint32_t xoff, uint32_t lod,
                      uint32_t count, void *data, size_t sizeBytes) {
    const size_t eSize = mHal.state.type->getElementSizeBytes();

    if ((count * eSize) != sizeBytes) {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "Allocation::read called with mismatched size expected %zu, got %zu",
                 (count * eSize), sizeBytes);
        rsc->setError(RS_ERROR_BAD_VALUE, buf);
        mHal.state.type->dumpLOGV("type info");
        return;
    }

    rsc->mHal.funcs.allocation.read1D(rsc, this, xoff, lod, count, data, sizeBytes);
}

void Allocation::read(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                      uint32_t w, uint32_t h, void *data, size_t sizeBytes, size_t stride) {
    const size_t eSize = mHal.state.elementSizeBytes;
    const size_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    } else {
        if ((lineSize * h) != sizeBytes) {
            char buf[1024];
            snprintf(buf, sizeof(buf), "Allocation size mismatch, expected %zu, got %zu",
                     (lineSize * h), sizeBytes);
            rsc->setError(RS_ERROR_BAD_VALUE, buf);
            return;
        }
    }

    rsc->mHal.funcs.allocation.read2D(rsc, this, xoff, yoff, lod, face, w, h, data, sizeBytes, stride);
}

void Allocation::read(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                      uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes, size_t stride) {
    const size_t eSize = mHal.state.elementSizeBytes;
    const size_t lineSize = eSize * w;
    if (!stride) {
        stride = lineSize;
    }

    rsc->mHal.funcs.allocation.read3D(rsc, this, xoff, yoff, zoff, lod, w, h, d, data, sizeBytes, stride);

}

void Allocation::elementData(Context *rsc, uint32_t x, uint32_t y, uint32_t z,
                             const void *data, uint32_t cIdx, size_t sizeBytes) {
    if (x >= mHal.drvState.lod[0].dimX) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData X offset out of range.");
        return;
    }

    if (y > 0 && y >= mHal.drvState.lod[0].dimY) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData Y offset out of range.");
        return;
    }

    if (z > 0 && z >= mHal.drvState.lod[0].dimZ) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData Z offset out of range.");
        return;
    }

    if (cIdx >= mHal.state.type->getElement()->getFieldCount()) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData component out of range.");
        return;
    }

    const Element * e = mHal.state.type->getElement()->getField(cIdx);
    uint32_t elemArraySize = mHal.state.type->getElement()->getFieldArraySize(cIdx);
    if (sizeBytes != e->getSizeBytes() * elemArraySize) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData bad size.");
        return;
    }

    rsc->mHal.funcs.allocation.elementData(rsc, this, x, y, z, data, cIdx, sizeBytes);
    sendDirty(rsc);
}

void Allocation::elementRead(Context *rsc, uint32_t x, uint32_t y, uint32_t z,
                             void *data, uint32_t cIdx, size_t sizeBytes) {
    if (x >= mHal.drvState.lod[0].dimX) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData X offset out of range.");
        return;
    }

    if (y > 0 && y >= mHal.drvState.lod[0].dimY) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData Y offset out of range.");
        return;
    }

    if (z > 0 && z >= mHal.drvState.lod[0].dimZ) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData Z offset out of range.");
        return;
    }

    if (cIdx >= mHal.state.type->getElement()->getFieldCount()) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData component out of range.");
        return;
    }

    const Element * e = mHal.state.type->getElement()->getField(cIdx);
    uint32_t elemArraySize = mHal.state.type->getElement()->getFieldArraySize(cIdx);
    if (sizeBytes != e->getSizeBytes() * elemArraySize) {
        rsc->setError(RS_ERROR_BAD_VALUE, "subElementData bad size.");
        return;
    }

    rsc->mHal.funcs.allocation.elementRead(rsc, this, x, y, z, data, cIdx, sizeBytes);
}

void Allocation::addProgramToDirty(const Program *p) {
    mToDirtyList.push_back(p);
}

void Allocation::removeProgramToDirty(const Program *p) {
    for (size_t ct=0; ct < mToDirtyList.size(); ct++) {
        if (mToDirtyList[ct] == p) {
            mToDirtyList.erase(mToDirtyList.begin() + ct);
            return;
        }
    }
    rsAssert(0);
}

void Allocation::dumpLOGV(const char *prefix) const {
    ObjectBase::dumpLOGV(prefix);
    char buf[1024];

    if ((strlen(prefix) + 10) < sizeof(buf)) {
        snprintf(buf, sizeof(buf), "%s type ", prefix);
        if (mHal.state.type) {
            mHal.state.type->dumpLOGV(buf);
        }
    }
    ALOGV("%s allocation ptr=%p  mUsageFlags=0x04%x, mMipmapControl=0x%04x",
         prefix, mHal.drvState.lod[0].mallocPtr, mHal.state.usageFlags, mHal.state.mipmapControl);
}

uint32_t Allocation::getPackedSize() const {
    uint32_t numItems = mHal.state.type->getCellCount();
    return numItems * mHal.state.type->getElement()->getSizeBytesUnpadded();
}

void Allocation::writePackedData(Context *rsc, const Type *type,
                                 uint8_t *dst, const uint8_t *src, bool dstPadded) {
    const Element *elem = type->getElement();
    uint32_t unpaddedBytes = elem->getSizeBytesUnpadded();
    uint32_t paddedBytes = elem->getSizeBytes();
    uint32_t numItems = type->getPackedSizeBytes() / paddedBytes;

    uint32_t srcInc = !dstPadded ? paddedBytes : unpaddedBytes;
    uint32_t dstInc =  dstPadded ? paddedBytes : unpaddedBytes;

    // no sub-elements
    uint32_t fieldCount = elem->getFieldCount();
    if (fieldCount == 0) {
        for (uint32_t i = 0; i < numItems; i ++) {
            memcpy(dst, src, unpaddedBytes);
            src += srcInc;
            dst += dstInc;
        }
        return;
    }

    // Cache offsets
    uint32_t *offsetsPadded = new uint32_t[fieldCount];
    uint32_t *offsetsUnpadded = new uint32_t[fieldCount];
    uint32_t *sizeUnpadded = new uint32_t[fieldCount];

    for (uint32_t i = 0; i < fieldCount; i++) {
        offsetsPadded[i] = elem->getFieldOffsetBytes(i);
        offsetsUnpadded[i] = elem->getFieldOffsetBytesUnpadded(i);
        sizeUnpadded[i] = elem->getField(i)->getSizeBytesUnpadded();
    }

    uint32_t *srcOffsets = !dstPadded ? offsetsPadded : offsetsUnpadded;
    uint32_t *dstOffsets =  dstPadded ? offsetsPadded : offsetsUnpadded;

    // complex elements, need to copy subelem after subelem
    for (uint32_t i = 0; i < numItems; i ++) {
        for (uint32_t fI = 0; fI < fieldCount; fI++) {
            memcpy(dst + dstOffsets[fI], src + srcOffsets[fI], sizeUnpadded[fI]);
        }
        src += srcInc;
        dst += dstInc;
    }

    delete[] offsetsPadded;
    delete[] offsetsUnpadded;
    delete[] sizeUnpadded;
}

void Allocation::unpackVec3Allocation(Context *rsc, const void *data, size_t dataSize) {
    const uint8_t *src = (const uint8_t*)data;
    uint8_t *dst = (uint8_t *)rsc->mHal.funcs.allocation.lock1D(rsc, this);

    writePackedData(rsc, getType(), dst, src, true);
    rsc->mHal.funcs.allocation.unlock1D(rsc, this);
}

void Allocation::packVec3Allocation(Context *rsc, OStream *stream) const {
    uint32_t unpaddedBytes = getType()->getElement()->getSizeBytesUnpadded();
    uint32_t numItems = mHal.state.type->getCellCount();

    const uint8_t *src = (const uint8_t*)rsc->mHal.funcs.allocation.lock1D(rsc, this);
    uint8_t *dst = new uint8_t[numItems * unpaddedBytes];

    writePackedData(rsc, getType(), dst, src, false);
    stream->addByteArray(dst, getPackedSize());

    delete[] dst;
    rsc->mHal.funcs.allocation.unlock1D(rsc, this);
}

void Allocation::serialize(Context *rsc, OStream *stream) const {
    // Need to identify ourselves
    stream->addU32((uint32_t)getClassId());
    stream->addString(getName());

    // First thing we need to serialize is the type object since it will be needed
    // to initialize the class
    mHal.state.type->serialize(rsc, stream);

    uint32_t dataSize = mHal.state.type->getPackedSizeBytes();
    // 3 element vectors are padded to 4 in memory, but padding isn't serialized
    uint32_t packedSize = getPackedSize();
    // Write how much data we are storing
    stream->addU32(packedSize);
    if (dataSize == packedSize) {
        // Now write the data
        stream->addByteArray(rsc->mHal.funcs.allocation.lock1D(rsc, this), dataSize);
        rsc->mHal.funcs.allocation.unlock1D(rsc, this);
    } else {
        // Now write the data
        packVec3Allocation(rsc, stream);
    }
}

Allocation *Allocation::createFromStream(Context *rsc, IStream *stream) {
    // First make sure we are reading the correct object
    RsA3DClassID classID = (RsA3DClassID)stream->loadU32();
    if (classID != RS_A3D_CLASS_ID_ALLOCATION) {
        rsc->setError(RS_ERROR_FATAL_DRIVER,
                      "allocation loading failed due to corrupt file. (invalid id)\n");
        return nullptr;
    }

    const char *name = stream->loadString();

    Type *type = Type::createFromStream(rsc, stream);
    if (!type) {
        return nullptr;
    }
    type->compute();

    Allocation *alloc = Allocation::createAllocation(rsc, type, RS_ALLOCATION_USAGE_SCRIPT);
    type->decUserRef();

    // Number of bytes we wrote out for this allocation
    uint32_t dataSize = stream->loadU32();
    // 3 element vectors are padded to 4 in memory, but padding isn't serialized
    uint32_t packedSize = alloc->getPackedSize();
    if (dataSize != type->getPackedSizeBytes() &&
        dataSize != packedSize) {
        rsc->setError(RS_ERROR_FATAL_DRIVER,
                      "allocation loading failed due to corrupt file. (invalid size)\n");
        ObjectBase::checkDelete(alloc);
        ObjectBase::checkDelete(type);
        return nullptr;
    }

    alloc->assignName(name);
    if (dataSize == type->getPackedSizeBytes()) {
        uint32_t count = dataSize / type->getElementSizeBytes();
        // Read in all of our allocation data
        alloc->data(rsc, 0, 0, count, stream->getPtr() + stream->getPos(), dataSize);
    } else {
        alloc->unpackVec3Allocation(rsc, stream->getPtr() + stream->getPos(), dataSize);
    }
    stream->reset(stream->getPos() + dataSize);

    return alloc;
}

void Allocation::sendDirty(const Context *rsc) const {
#ifndef RS_COMPATIBILITY_LIB
    for (size_t ct=0; ct < mToDirtyList.size(); ct++) {
        mToDirtyList[ct]->forceDirty();
    }
#endif
    mRSC->mHal.funcs.allocation.markDirty(rsc, this);
}

void Allocation::incRefs(const void *ptr, size_t ct, size_t startOff) const {
    mHal.state.type->incRefs(ptr, ct, startOff);
}

void Allocation::decRefs(const void *ptr, size_t ct, size_t startOff) const {
    if (!mHal.state.hasReferences || !getIsScript()) {
        return;
    }
    mHal.state.type->decRefs(ptr, ct, startOff);
}

void Allocation::callUpdateCacheObject(const Context *rsc, void *dstObj) const {
    if (rsc->mHal.funcs.allocation.updateCachedObject != nullptr) {
        rsc->mHal.funcs.allocation.updateCachedObject(rsc, this, (rs_allocation *)dstObj);
    } else {
        *((const void **)dstObj) = this;
    }
}


void Allocation::freeChildrenUnlocked () {
    void *ptr = mRSC->mHal.funcs.allocation.lock1D(mRSC, this);
    decRefs(ptr, mHal.state.type->getCellCount(), 0);
    mRSC->mHal.funcs.allocation.unlock1D(mRSC, this);
}

bool Allocation::freeChildren() {
    if (mHal.state.hasReferences) {
        incSysRef();
        freeChildrenUnlocked();
        return decSysRef();
    }
    return false;
}

void Allocation::copyRange1D(Context *rsc, const Allocation *src, int32_t srcOff, int32_t destOff, int32_t len) {
}

void Allocation::resize1D(Context *rsc, uint32_t dimX) {
    uint32_t oldDimX = mHal.drvState.lod[0].dimX;
    if (dimX == oldDimX) {
        return;
    }

    ObjectBaseRef<Type> t = mHal.state.type->cloneAndResize1D(rsc, dimX);
    if (dimX < oldDimX) {
        decRefs(rsc->mHal.funcs.allocation.lock1D(rsc, this), oldDimX - dimX, dimX);
        rsc->mHal.funcs.allocation.unlock1D(rsc, this);
    }
    rsc->mHal.funcs.allocation.resize(rsc, this, t.get(), mHal.state.hasReferences);
    setType(t.get());
    updateCache();
}

void Allocation::resize2D(Context *rsc, uint32_t dimX, uint32_t dimY) {
    rsc->setError(RS_ERROR_FATAL_DRIVER, "resize2d not implemented");
}

void Allocation::setupGrallocConsumer(const Context *rsc, uint32_t numAlloc) {
#ifndef RS_COMPATIBILITY_LIB
    // Configure GrallocConsumer to be in asynchronous mode
    if (numAlloc > MAX_NUM_ALLOC || numAlloc <= 0) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "resize2d not implemented");
        return;
    }
    mGrallocConsumer = new GrallocConsumer(rsc, this, numAlloc);
    mCurrentIdx = 0;
    mBufferQueueInited = true;
#endif
}

void * Allocation::getSurface(const Context *rsc) {
#ifndef RS_COMPATIBILITY_LIB
    // Configure GrallocConsumer to be in asynchronous mode
    if (!mBufferQueueInited) {
        // This case is only used for single frame processing,
        // since we will always call setupGrallocConsumer first in
        // multi-frame case.
        setupGrallocConsumer(rsc, 1);
    }
    return mGrallocConsumer->getNativeWindow();
#else
    return nullptr;
#endif
}

void Allocation::shareBufferQueue(const Context *rsc, const Allocation *alloc) {
#ifndef RS_COMPATIBILITY_LIB
    mGrallocConsumer = alloc->mGrallocConsumer;
    mCurrentIdx = mGrallocConsumer->getNextAvailableIdx(this);
    if (mCurrentIdx >= mGrallocConsumer->mNumAlloc) {
        rsc->setError(RS_ERROR_DRIVER, "Maximum allocations attached to a BufferQueue");
        return;
    }
    mBufferQueueInited = true;
#endif
}


void Allocation::setSurface(const Context *rsc, RsNativeWindow sur) {
    ANativeWindow *nw = (ANativeWindow *)sur;
    rsc->mHal.funcs.allocation.setSurface(rsc, this, nw);
}

void Allocation::ioSend(const Context *rsc) {
    rsc->mHal.funcs.allocation.ioSend(rsc, this);
}

void Allocation::ioReceive(const Context *rsc) {
    void *ptr = nullptr;
    size_t stride = 0;
#ifndef RS_COMPATIBILITY_LIB
    if (mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT) {
        media_status_t ret = mGrallocConsumer->lockNextBuffer(mCurrentIdx);

        if (ret == AMEDIA_OK) {
            rsc->mHal.funcs.allocation.ioReceive(rsc, this);
        } else if (ret == AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE) {
            // No new frame, don't do anything
        } else {
            rsc->setError(RS_ERROR_DRIVER, "Error receiving IO input buffer.");
        }

    }
#endif
}

bool Allocation::hasSameDims(const Allocation *other) const {
    const Type *type0 = this->getType(),
               *type1 = other->getType();

    return (type0->getCellCount() == type1->getCellCount()) &&
           (type0->getDimLOD()    == type1->getDimLOD())    &&
           (type0->getDimFaces()  == type1->getDimFaces())  &&
           (type0->getDimYuv()    == type1->getDimYuv())    &&
           (type0->getDimX()      == type1->getDimX())      &&
           (type0->getDimY()      == type1->getDimY())      &&
           (type0->getDimZ()      == type1->getDimZ());
}


/////////////////
//

void rsi_AllocationSyncAll(Context *rsc, RsAllocation va, RsAllocationUsageType src) {
    Allocation *a = static_cast<Allocation *>(va);
    a->sendDirty(rsc);
    a->syncAll(rsc, src);
}

void rsi_AllocationGenerateMipmaps(Context *rsc, RsAllocation va) {
    Allocation *alloc = static_cast<Allocation *>(va);
    rsc->mHal.funcs.allocation.generateMipmaps(rsc, alloc);
}

void rsi_AllocationCopyToBitmap(Context *rsc, RsAllocation va, void *data, size_t sizeBytes) {
    Allocation *a = static_cast<Allocation *>(va);
    const Type * t = a->getType();
    a->read(rsc, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X,
            t->getDimX(), t->getDimY(), data, sizeBytes, 0);
}

void rsi_Allocation1DData(Context *rsc, RsAllocation va, uint32_t xoff, uint32_t lod,
                          uint32_t count, const void *data, size_t sizeBytes) {
    Allocation *a = static_cast<Allocation *>(va);
    a->data(rsc, xoff, lod, count, data, sizeBytes);
}

void rsi_Allocation1DElementData(Context *rsc, RsAllocation va, uint32_t x,
                                 uint32_t lod, const void *data, size_t sizeBytes, size_t eoff) {
    Allocation *a = static_cast<Allocation *>(va);
    a->elementData(rsc, x, 0, 0, data, eoff, sizeBytes);
}

void rsi_AllocationElementData(Context *rsc, RsAllocation va, uint32_t x, uint32_t y, uint32_t z,
                               uint32_t lod, const void *data, size_t sizeBytes, size_t eoff) {
    Allocation *a = static_cast<Allocation *>(va);
    a->elementData(rsc, x, y, z, data, eoff, sizeBytes);
}

void rsi_Allocation2DData(Context *rsc, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
                          uint32_t w, uint32_t h, const void *data, size_t sizeBytes, size_t stride) {
    Allocation *a = static_cast<Allocation *>(va);
    a->data(rsc, xoff, yoff, lod, face, w, h, data, sizeBytes, stride);
}

void rsi_Allocation3DData(Context *rsc, RsAllocation va, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                          uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes, size_t stride) {
    Allocation *a = static_cast<Allocation *>(va);
    a->data(rsc, xoff, yoff, zoff, lod, w, h, d, data, sizeBytes, stride);
}


void rsi_AllocationRead(Context *rsc, RsAllocation va, void *data, size_t sizeBytes) {
    Allocation *a = static_cast<Allocation *>(va);
    const Type * t = a->getType();
    if(t->getDimZ()) {
        a->read(rsc, 0, 0, 0, 0, t->getDimX(), t->getDimY(), t->getDimZ(),
                data, sizeBytes, 0);
    } else if(t->getDimY()) {
        a->read(rsc, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X,
                t->getDimX(), t->getDimY(), data, sizeBytes, 0);
    } else {
        a->read(rsc, 0, 0, t->getDimX(), data, sizeBytes);
    }

}

void rsi_AllocationResize1D(Context *rsc, RsAllocation va, uint32_t dimX) {
    Allocation *a = static_cast<Allocation *>(va);
    a->resize1D(rsc, dimX);
}

void rsi_AllocationResize2D(Context *rsc, RsAllocation va, uint32_t dimX, uint32_t dimY) {
    Allocation *a = static_cast<Allocation *>(va);
    a->resize2D(rsc, dimX, dimY);
}

RsAllocation rsi_AllocationCreateTyped(Context *rsc, RsType vtype,
                                       RsAllocationMipmapControl mipmaps,
                                       uint32_t usages, uintptr_t ptr) {
    Allocation * alloc = Allocation::createAllocation(rsc, static_cast<Type *>(vtype), usages, mipmaps, (void*)ptr);
    if (!alloc) {
        return nullptr;
    }
    alloc->incUserRef();
    return alloc;
}

RsAllocation rsi_AllocationCreateStrided(Context *rsc, RsType vtype,
                                         RsAllocationMipmapControl mipmaps,
                                         uint32_t usages, uintptr_t ptr,
                                         size_t requiredAlignment) {
    Allocation * alloc = Allocation::createAllocationStrided(rsc, static_cast<Type *>(vtype), usages, mipmaps,
                                                             (void*)ptr, requiredAlignment);
    if (!alloc) {
        return nullptr;
    }
    alloc->incUserRef();
    return alloc;
}

RsAllocation rsi_AllocationCreateFromBitmap(Context *rsc, RsType vtype,
                                            RsAllocationMipmapControl mipmaps,
                                            const void *data, size_t sizeBytes, uint32_t usages) {
    Type *t = static_cast<Type *>(vtype);

    RsAllocation vTexAlloc = rsi_AllocationCreateTyped(rsc, vtype, mipmaps, usages, 0);
    Allocation *texAlloc = static_cast<Allocation *>(vTexAlloc);
    if (texAlloc == nullptr) {
        ALOGE("Memory allocation failure");
        return nullptr;
    }

    texAlloc->data(rsc, 0, 0, 0, RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X,
                   t->getDimX(), t->getDimY(), data, sizeBytes, 0);
    if (mipmaps == RS_ALLOCATION_MIPMAP_FULL) {
        rsc->mHal.funcs.allocation.generateMipmaps(rsc, texAlloc);
    }

    texAlloc->sendDirty(rsc);
    return texAlloc;
}

RsAllocation rsi_AllocationCubeCreateFromBitmap(Context *rsc, RsType vtype,
                                                RsAllocationMipmapControl mipmaps,
                                                const void *data, size_t sizeBytes, uint32_t usages) {
    Type *t = static_cast<Type *>(vtype);

    // Cubemap allocation's faces should be Width by Width each.
    // Source data should have 6 * Width by Width pixels
    // Error checking is done in the java layer
    RsAllocation vTexAlloc = rsi_AllocationCreateTyped(rsc, vtype, mipmaps, usages, 0);
    Allocation *texAlloc = static_cast<Allocation *>(vTexAlloc);
    if (texAlloc == nullptr) {
        ALOGE("Memory allocation failure");
        return nullptr;
    }

    uint32_t faceSize = t->getDimX();
    uint32_t strideBytes = faceSize * 6 * t->getElementSizeBytes();
    uint32_t copySize = faceSize * t->getElementSizeBytes();

    uint8_t *sourcePtr = (uint8_t*)data;
    for (uint32_t face = 0; face < 6; face ++) {
        for (uint32_t dI = 0; dI < faceSize; dI ++) {
            texAlloc->data(rsc, 0, dI, 0, (RsAllocationCubemapFace)face,
                           t->getDimX(), 1, sourcePtr + strideBytes * dI, copySize, 0);
        }

        // Move the data pointer to the next cube face
        sourcePtr += copySize;
    }

    if (mipmaps == RS_ALLOCATION_MIPMAP_FULL) {
        rsc->mHal.funcs.allocation.generateMipmaps(rsc, texAlloc);
    }

    texAlloc->sendDirty(rsc);
    return texAlloc;
}

void rsi_AllocationCopy2DRange(Context *rsc,
                               RsAllocation dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff,
                               uint32_t dstMip, uint32_t dstFace,
                               uint32_t width, uint32_t height,
                               RsAllocation srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff,
                               uint32_t srcMip, uint32_t srcFace) {
    Allocation *dst = static_cast<Allocation *>(dstAlloc);
    Allocation *src= static_cast<Allocation *>(srcAlloc);
    rsc->mHal.funcs.allocation.allocData2D(rsc, dst, dstXoff, dstYoff, dstMip,
                                           (RsAllocationCubemapFace)dstFace,
                                           width, height,
                                           src, srcXoff, srcYoff,srcMip,
                                           (RsAllocationCubemapFace)srcFace);
}

void rsi_AllocationCopy3DRange(Context *rsc,
                               RsAllocation dstAlloc,
                               uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                               uint32_t dstMip,
                               uint32_t width, uint32_t height, uint32_t depth,
                               RsAllocation srcAlloc,
                               uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                               uint32_t srcMip) {
    Allocation *dst = static_cast<Allocation *>(dstAlloc);
    Allocation *src= static_cast<Allocation *>(srcAlloc);
    rsc->mHal.funcs.allocation.allocData3D(rsc, dst, dstXoff, dstYoff, dstZoff, dstMip,
                                           width, height, depth,
                                           src, srcXoff, srcYoff, srcZoff, srcMip);
}

void rsi_AllocationSetupBufferQueue(Context *rsc, RsAllocation valloc, uint32_t numAlloc) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    alloc->setupGrallocConsumer(rsc, numAlloc);
}

void * rsi_AllocationGetSurface(Context *rsc, RsAllocation valloc) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    void *s = alloc->getSurface(rsc);
    return s;
}

void rsi_AllocationShareBufferQueue(Context *rsc, RsAllocation valloc1, RsAllocation valloc2) {
    Allocation *alloc1 = static_cast<Allocation *>(valloc1);
    Allocation *alloc2 = static_cast<Allocation *>(valloc2);
    alloc1->shareBufferQueue(rsc, alloc2);
}

void rsi_AllocationSetSurface(Context *rsc, RsAllocation valloc, RsNativeWindow sur) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    alloc->setSurface(rsc, sur);
}

void rsi_AllocationIoSend(Context *rsc, RsAllocation valloc) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    alloc->ioSend(rsc);
}

int64_t rsi_AllocationIoReceive(Context *rsc, RsAllocation valloc) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    alloc->ioReceive(rsc);
    return alloc->getTimeStamp();
}

void *rsi_AllocationGetPointer(Context *rsc, RsAllocation valloc,
                          uint32_t lod, RsAllocationCubemapFace face,
                          uint32_t z, uint32_t array, size_t *stride, size_t strideLen) {
    Allocation *alloc = static_cast<Allocation *>(valloc);
    rsAssert(strideLen == sizeof(size_t));

    return alloc->getPointer(rsc, lod, face, z, array, stride);
}

void rsi_Allocation1DRead(Context *rsc, RsAllocation va, uint32_t xoff, uint32_t lod,
                          uint32_t count, void *data, size_t sizeBytes) {
    Allocation *a = static_cast<Allocation *>(va);
    rsc->mHal.funcs.allocation.read1D(rsc, a, xoff, lod, count, data, sizeBytes);
}

void rsi_AllocationElementRead(Context *rsc, RsAllocation va, uint32_t x, uint32_t y, uint32_t z,
                                 uint32_t lod, void *data, size_t sizeBytes, size_t eoff) {
    Allocation *a = static_cast<Allocation *>(va);
    a->elementRead(rsc, x, y, z, data, eoff, sizeBytes);
}

void rsi_Allocation2DRead(Context *rsc, RsAllocation va, uint32_t xoff, uint32_t yoff,
                          uint32_t lod, RsAllocationCubemapFace face, uint32_t w,
                          uint32_t h, void *data, size_t sizeBytes, size_t stride) {
    Allocation *a = static_cast<Allocation *>(va);
    a->read(rsc, xoff, yoff, lod, face, w, h, data, sizeBytes, stride);
}

void rsi_Allocation3DRead(Context *rsc, RsAllocation va,
                          uint32_t xoff, uint32_t yoff, uint32_t zoff,
                          uint32_t lod, uint32_t w, uint32_t h, uint32_t d,
                          void *data, size_t sizeBytes, size_t stride) {
    Allocation *a = static_cast<Allocation *>(va);
    a->read(rsc, xoff, yoff, zoff, lod, w, h, d, data, sizeBytes, stride);
}

RsAllocation rsi_AllocationAdapterCreate(Context *rsc, RsType vwindow, RsAllocation vbase) {


    Allocation * alloc = Allocation::createAdapter(rsc,
            static_cast<Allocation *>(vbase), static_cast<Type *>(vwindow));
    if (!alloc) {
        return nullptr;
    }
    alloc->incUserRef();
    return alloc;
}

void rsi_AllocationAdapterOffset(Context *rsc, RsAllocation va, const uint32_t *offsets, size_t len) {
    Allocation *a = static_cast<Allocation *>(va);
    a->adapterOffset(rsc, offsets, len);
}


}  // namespace renderscript
}  // namespace android
