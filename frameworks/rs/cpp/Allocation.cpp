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

#include "RenderScript.h"
#include "rsCppInternal.h"

using android::RSC::Allocation;
using android::RSC::sp;
using android::Surface;

void * Allocation::getIDSafe() const {
    return getID();
}

void Allocation::updateCacheInfo(const sp<const Type>& t) {
    mCurrentDimX = t->getX();
    mCurrentDimY = t->getY();
    mCurrentDimZ = t->getZ();
    mCurrentCount = mCurrentDimX;
    if (mCurrentDimY > 1) {
        mCurrentCount *= mCurrentDimY;
    }
    if (mCurrentDimZ > 1) {
        mCurrentCount *= mCurrentDimZ;
    }
}

Allocation::Allocation(void *id, sp<RS> rs, sp<const Type> t, uint32_t usage) :
    BaseObj(id, rs), mSelectedY(0), mSelectedZ(0), mSelectedLOD(0),
    mSelectedFace(RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X) {

    if ((usage & ~(RS_ALLOCATION_USAGE_SCRIPT |
                   RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE |
                   RS_ALLOCATION_USAGE_GRAPHICS_VERTEX |
                   RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS |
                   RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET |
                   RS_ALLOCATION_USAGE_IO_INPUT |
                   RS_ALLOCATION_USAGE_IO_OUTPUT |
                   RS_ALLOCATION_USAGE_OEM |
                   RS_ALLOCATION_USAGE_SHARED)) != 0) {
        ALOGE("Unknown usage specified.");
    }

    if ((usage & RS_ALLOCATION_USAGE_IO_INPUT) != 0) {
        mWriteAllowed = false;
        if ((usage & ~(RS_ALLOCATION_USAGE_IO_INPUT |
                       RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE |
                       RS_ALLOCATION_USAGE_SCRIPT)) != 0) {
            ALOGE("Invalid usage combination.");
        }
    }

    mType = t;
    mUsage = usage;
    mAutoPadding = false;
    if (t != nullptr) {
        updateCacheInfo(t);
    }

}


void Allocation::validateIsInt64() {
    RsDataType dt = mType->getElement()->getDataType();
    if ((dt == RS_TYPE_SIGNED_64) || (dt == RS_TYPE_UNSIGNED_64)) {
        return;
    }
    ALOGE("64 bit integer source does not match allocation type %i", dt);
}

void Allocation::validateIsInt32() {
    RsDataType dt = mType->getElement()->getDataType();
    if ((dt == RS_TYPE_SIGNED_32) || (dt == RS_TYPE_UNSIGNED_32)) {
        return;
    }
    ALOGE("32 bit integer source does not match allocation type %i", dt);
}

void Allocation::validateIsInt16() {
    RsDataType dt = mType->getElement()->getDataType();
    if ((dt == RS_TYPE_SIGNED_16) || (dt == RS_TYPE_UNSIGNED_16)) {
        return;
    }
    ALOGE("16 bit integer source does not match allocation type %i", dt);
}

void Allocation::validateIsInt8() {
    RsDataType dt = mType->getElement()->getDataType();
    if ((dt == RS_TYPE_SIGNED_8) || (dt == RS_TYPE_UNSIGNED_8)) {
        return;
    }
    ALOGE("8 bit integer source does not match allocation type %i", dt);
}

void Allocation::validateIsFloat32() {
    RsDataType dt = mType->getElement()->getDataType();
    if (dt == RS_TYPE_FLOAT_32) {
        return;
    }
    ALOGE("32 bit float source does not match allocation type %i", dt);
}

void Allocation::validateIsFloat64() {
    RsDataType dt = mType->getElement()->getDataType();
    if (dt == RS_TYPE_FLOAT_64) {
        return;
    }
    ALOGE("64 bit float source does not match allocation type %i", dt);
}

void Allocation::validateIsObject() {
    RsDataType dt = mType->getElement()->getDataType();
    if ((dt == RS_TYPE_ELEMENT) ||
        (dt == RS_TYPE_TYPE) ||
        (dt == RS_TYPE_ALLOCATION) ||
        (dt == RS_TYPE_SAMPLER) ||
        (dt == RS_TYPE_SCRIPT) ||
        (dt == RS_TYPE_MESH) ||
        (dt == RS_TYPE_PROGRAM_FRAGMENT) ||
        (dt == RS_TYPE_PROGRAM_VERTEX) ||
        (dt == RS_TYPE_PROGRAM_RASTER) ||
        (dt == RS_TYPE_PROGRAM_STORE)) {
        return;
    }
    ALOGE("Object source does not match allocation type %i", dt);
}

void Allocation::updateFromNative() {
    BaseObj::updateFromNative();

    const void *typeID = RS::dispatch->AllocationGetType(mRS->getContext(), getID());
    if(typeID != nullptr) {
        sp<Type> t = new Type((void *)typeID, mRS);
        t->updateFromNative();
        updateCacheInfo(t);
        mType = t;
    }
}

void Allocation::syncAll(RsAllocationUsageType srcLocation) {
    switch (srcLocation) {
    case RS_ALLOCATION_USAGE_SCRIPT:
    case RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS:
    case RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE:
    case RS_ALLOCATION_USAGE_GRAPHICS_VERTEX:
    case RS_ALLOCATION_USAGE_SHARED:
        break;
    default:
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Source must be exactly one usage type.");
        return;
    }
    tryDispatch(mRS, RS::dispatch->AllocationSyncAll(mRS->getContext(), getIDSafe(), srcLocation));
}

void * Allocation::getPointer(size_t *stride) {
    void *p = nullptr;
    if (!(mUsage & RS_ALLOCATION_USAGE_SHARED)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Allocation does not support USAGE_SHARED.");
        return nullptr;
    }

    // FIXME: decide if lack of getPointer should cause compat mode
    if (RS::dispatch->AllocationGetPointer == nullptr) {
        mRS->throwError(RS_ERROR_RUNTIME_ERROR, "Can't use getPointer on older APIs");
        return nullptr;
    }

    p = RS::dispatch->AllocationGetPointer(mRS->getContext(), getIDSafe(), 0,
                                           RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X, 0, 0, stride, sizeof(size_t));
    if (mRS->getError() != RS_SUCCESS) {
        mRS->throwError(RS_ERROR_RUNTIME_ERROR, "Allocation lock failed");
        p = nullptr;
    }
    return p;
}

// ---------------------------------------------------------------------------
//Functions needed for autopadding & unpadding
static void copyWithPadding(void* ptr, const void* srcPtr, int mSize, int count) {
    int sizeBytesPad = mSize * 4;
    int sizeBytes = mSize * 3;
    uint8_t *dst = static_cast<uint8_t *>(ptr);
    const uint8_t *src = static_cast<const uint8_t *>(srcPtr);
    for (int i = 0; i < count; i++) {
        memcpy(dst, src, sizeBytes);
        dst += sizeBytesPad;
        src += sizeBytes;
    }
}

static void copyWithUnPadding(void* ptr, const void* srcPtr, int mSize, int count) {
    int sizeBytesPad = mSize * 4;
    int sizeBytes = mSize * 3;
    uint8_t *dst = static_cast<uint8_t *>(ptr);
    const uint8_t *src = static_cast<const uint8_t *>(srcPtr);
    for (int i = 0; i < count; i++) {
        memcpy(dst, src, sizeBytes);
        dst += sizeBytes;
        src += sizeBytesPad;
    }
}
// ---------------------------------------------------------------------------

void Allocation::copy1DRangeFrom(uint32_t off, size_t count, const void *data) {

    if(count < 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Count must be >= 1.");
        return;
    }
    if((off + count) > mCurrentCount) {
        ALOGE("Overflow, Available count %u, got %zu at offset %u.", mCurrentCount, count, off);
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid copy specified");
        return;
    }
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * count);
        copyWithPadding(ptr, data, eSize / 4, count);
        tryDispatch(mRS, RS::dispatch->Allocation1DData(mRS->getContext(), getIDSafe(), off, mSelectedLOD,
                                                        count, ptr, count * mType->getElement()->getSizeBytes()));
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation1DData(mRS->getContext(), getIDSafe(), off, mSelectedLOD,
                                                        count, data, count * mType->getElement()->getSizeBytes()));
    }
}

void Allocation::copy1DRangeTo(uint32_t off, size_t count, void *data) {
    if(count < 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Count must be >= 1.");
        return;
    }
    if((off + count) > mCurrentCount) {
        ALOGE("Overflow, Available count %u, got %zu at offset %u.", mCurrentCount, count, off);
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid copy specified");
        return;
    }
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * count);
        tryDispatch(mRS, RS::dispatch->Allocation1DRead(mRS->getContext(), getIDSafe(), off, mSelectedLOD,
                                                        count, ptr, count * mType->getElement()->getSizeBytes()));
        copyWithUnPadding(data, ptr, eSize / 4, count);
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation1DRead(mRS->getContext(), getIDSafe(), off, mSelectedLOD,
                                                        count, data, count * mType->getElement()->getSizeBytes()));
    }
}

void Allocation::copy1DRangeFrom(uint32_t off, size_t count, const sp<const Allocation>& data,
                                 uint32_t dataOff) {

    tryDispatch(mRS, RS::dispatch->AllocationCopy2DRange(mRS->getContext(), getIDSafe(), off, 0,
                                                         mSelectedLOD, mSelectedFace,
                                                         count, 1, data->getIDSafe(), dataOff, 0,
                                                         data->mSelectedLOD, data->mSelectedFace));
}

void Allocation::copy1DFrom(const void* data) {
    copy1DRangeFrom(0, mCurrentCount, data);
}

void Allocation::copy1DTo(void* data) {
    copy1DRangeTo(0, mCurrentCount, data);
}


void Allocation::validate2DRange(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h) {
    if (mAdaptedAllocation != nullptr) {

    } else {
        if (((xoff + w) > mCurrentDimX) || ((yoff + h) > mCurrentDimY)) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Updated region larger than allocation.");
        }
    }
}

void Allocation::copy2DRangeFrom(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h,
                                 const void *data) {
    validate2DRange(xoff, yoff, w, h);
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * w * h);
        copyWithPadding(ptr, data, eSize / 4, w * h);
        tryDispatch(mRS, RS::dispatch->Allocation2DData(mRS->getContext(), getIDSafe(), xoff,
                                                        yoff, mSelectedLOD, mSelectedFace,
                                                        w, h, ptr, w * h * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation2DData(mRS->getContext(), getIDSafe(), xoff,
                                                        yoff, mSelectedLOD, mSelectedFace,
                                                        w, h, data, w * h * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
    }
}

void Allocation::copy2DRangeFrom(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h,
                                 const sp<const Allocation>& data, uint32_t dataXoff, uint32_t dataYoff) {
    validate2DRange(xoff, yoff, w, h);
    tryDispatch(mRS, RS::dispatch->AllocationCopy2DRange(mRS->getContext(), getIDSafe(), xoff, yoff,
                                                         mSelectedLOD, mSelectedFace,
                                                         w, h, data->getIDSafe(), dataXoff, dataYoff,
                                                         data->mSelectedLOD, data->mSelectedFace));
}

void Allocation::copy2DRangeTo(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h,
                               void* data) {
    validate2DRange(xoff, yoff, w, h);
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * w * h);
        tryDispatch(mRS, RS::dispatch->Allocation2DRead(mRS->getContext(), getIDSafe(), xoff, yoff,
                                                        mSelectedLOD, mSelectedFace, w, h, ptr,
                                                        w * h * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
        copyWithUnPadding(data, ptr, eSize / 4, w * h);
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation2DRead(mRS->getContext(), getIDSafe(), xoff, yoff,
                                                        mSelectedLOD, mSelectedFace, w, h, data,
                                                        w * h * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
    }
}

void Allocation::copy2DStridedFrom(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h,
                                   const void *data, size_t stride) {
    validate2DRange(xoff, yoff, w, h);
    tryDispatch(mRS, RS::dispatch->Allocation2DData(mRS->getContext(), getIDSafe(), xoff, yoff,
                                                    mSelectedLOD, mSelectedFace, w, h, data,
                                                    w * h * mType->getElement()->getSizeBytes(), stride));
}

void Allocation::copy2DStridedFrom(const void* data, size_t stride) {
    copy2DStridedFrom(0, 0, mCurrentDimX, mCurrentDimY, data, stride);
}

void Allocation::copy2DStridedTo(uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h,
                                 void *data, size_t stride) {
    validate2DRange(xoff, yoff, w, h);
    tryDispatch(mRS, RS::dispatch->Allocation2DRead(mRS->getContext(), getIDSafe(), xoff, yoff,
                                                    mSelectedLOD, mSelectedFace, w, h, data,
                                                    w * h * mType->getElement()->getSizeBytes(), stride));
}

void Allocation::copy2DStridedTo(void* data, size_t stride) {
    copy2DStridedTo(0, 0, mCurrentDimX, mCurrentDimY, data, stride);
}

void Allocation::validate3DRange(uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t w,
                                 uint32_t h, uint32_t d) {
    if (mAdaptedAllocation != nullptr) {

    } else {
        if (((xoff + w) > mCurrentDimX) || ((yoff + h) > mCurrentDimY) || ((zoff + d) > mCurrentDimZ)) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Updated region larger than allocation.");
        }
    }
}

void Allocation::copy3DRangeFrom(uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t w,
                                 uint32_t h, uint32_t d, const void* data) {
    validate3DRange(xoff, yoff, zoff, w, h, d);
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * w * h * d);
        copyWithPadding(ptr, data, eSize / 4, w * h * d);
        tryDispatch(mRS, RS::dispatch->Allocation3DData(mRS->getContext(), getIDSafe(), xoff, yoff, zoff,
                                                        mSelectedLOD, w, h, d, ptr,
                                                        w * h * d * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation3DData(mRS->getContext(), getIDSafe(), xoff, yoff, zoff,
                                                        mSelectedLOD, w, h, d, data,
                                                        w * h * d * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
    }
}

void Allocation::copy3DRangeFrom(uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t w, uint32_t h, uint32_t d,
                                 const sp<const Allocation>& data, uint32_t dataXoff, uint32_t dataYoff, uint32_t dataZoff) {
    validate3DRange(xoff, yoff, zoff, w, h, d);
    tryDispatch(mRS, RS::dispatch->AllocationCopy3DRange(mRS->getContext(), getIDSafe(), xoff, yoff, zoff,
                                                         mSelectedLOD, w, h, d, data->getIDSafe(),
                                                         dataXoff, dataYoff, dataZoff, data->mSelectedLOD));
}

void Allocation::copy3DRangeTo(uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t w,
                                 uint32_t h, uint32_t d, void* data) {
    validate3DRange(xoff, yoff, zoff, w, h, d);
    if (mAutoPadding && (mType->getElement()->getVectorSize() == 3)) {
        size_t eSize = mType->getElement()->getSizeBytes();
        void *ptr = malloc(eSize * w * h * d);
        tryDispatch(mRS, RS::dispatch->Allocation3DRead(mRS->getContext(), getIDSafe(), xoff, yoff, zoff,
                                                        mSelectedLOD, w, h, d, ptr,
                                                        w * h * d * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
        copyWithUnPadding(data, ptr, eSize / 4, w * h * d);
        free(ptr);
    } else {
        tryDispatch(mRS, RS::dispatch->Allocation3DRead(mRS->getContext(), getIDSafe(), xoff, yoff, zoff,
                                                        mSelectedLOD, w, h, d, data,
                                                        w * h * d * mType->getElement()->getSizeBytes(),
                                                        w * mType->getElement()->getSizeBytes()));
    }
}

sp<Allocation> Allocation::createTyped(const sp<RS>& rs, const sp<const Type>& type,
                                    RsAllocationMipmapControl mipmaps, uint32_t usage) {
    void *id = 0;
    if (rs->getError() == RS_SUCCESS) {
        id = RS::dispatch->AllocationCreateTyped(rs->getContext(), type->getID(), mipmaps, usage, 0);
    }
    if (id == 0) {
        rs->throwError(RS_ERROR_RUNTIME_ERROR, "Allocation creation failed");
        return nullptr;
    }
    return new Allocation(id, rs, type, usage);
}

sp<Allocation> Allocation::createTyped(const sp<RS>& rs, const sp<const Type>& type,
                                    RsAllocationMipmapControl mipmaps, uint32_t usage,
                                    void *pointer) {
    void *id = 0;
    if (rs->getError() == RS_SUCCESS) {
        id = RS::dispatch->AllocationCreateTyped(rs->getContext(), type->getID(), mipmaps, usage,
                                                 (uintptr_t)pointer);
    }
    if (id == 0) {
        rs->throwError(RS_ERROR_RUNTIME_ERROR, "Allocation creation failed");
        return nullptr;
    }
    return new Allocation(id, rs, type, usage);
}

sp<Allocation> Allocation::createTyped(const sp<RS>& rs, const sp<const Type>& type,
                                    uint32_t usage) {
    return createTyped(rs, type, RS_ALLOCATION_MIPMAP_NONE, usage);
}

sp<Allocation> Allocation::createSized(const sp<RS>& rs, const sp<const Element>& e,
                                    size_t count, uint32_t usage) {
    Type::Builder b(rs, e);
    b.setX(count);
    sp<const Type> t = b.create();

    return createTyped(rs, t, usage);
}

sp<Allocation> Allocation::createSized2D(const sp<RS>& rs, const sp<const Element>& e,
                                      size_t x, size_t y, uint32_t usage) {
    Type::Builder b(rs, e);
    b.setX(x);
    b.setY(y);
    sp<const Type> t = b.create();

    return createTyped(rs, t, usage);
}

void Allocation::ioSendOutput() {
#ifndef RS_COMPATIBILITY_LIB
    if ((mUsage & RS_ALLOCATION_USAGE_IO_OUTPUT) == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Can only send buffer if IO_OUTPUT usage specified.");
        return;
    }
    tryDispatch(mRS, RS::dispatch->AllocationIoSend(mRS->getContext(), getID()));
#endif
}

void Allocation::ioGetInput() {
#ifndef RS_COMPATIBILITY_LIB
    if ((mUsage & RS_ALLOCATION_USAGE_IO_INPUT) == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Can only get buffer if IO_INPUT usage specified.");
        return;
    }
    tryDispatch(mRS, RS::dispatch->AllocationIoReceive(mRS->getContext(), getID()));
#endif
}

#ifndef RS_COMPATIBILITY_LIB
#include <gui/Surface.h>

sp<Surface> Allocation::getSurface() {
    if ((mUsage & RS_ALLOCATION_USAGE_IO_INPUT) == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Can only get Surface if IO_INPUT usage specified.");
        return nullptr;
    }
    ANativeWindow *anw = (ANativeWindow *)RS::dispatch->AllocationGetSurface(mRS->getContext(),
                                                                             getID());
    sp<Surface> surface(static_cast<Surface*>(anw));
    return surface;
}

void Allocation::setSurface(const sp<Surface>& s) {
    if ((mUsage & RS_ALLOCATION_USAGE_IO_OUTPUT) == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Can only set Surface if IO_OUTPUT usage specified.");
        return;
    }
    tryDispatch(mRS, RS::dispatch->AllocationSetSurface(mRS->getContext(), getID(),
                                                        static_cast<ANativeWindow *>(s.get())));
}

#endif

