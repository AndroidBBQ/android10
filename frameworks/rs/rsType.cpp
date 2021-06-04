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

#ifndef RS_COMPATIBILITY_LIB
#include "system/graphics.h"
#else
#include "rsCompatibilityLib.h"
#endif

namespace android {
namespace renderscript {

Type::Type(Context *rsc) : ObjectBase(rsc) {
    memset(&mHal, 0, sizeof(mHal));
    mDimLOD = false;
}

void Type::preDestroy() const {
    auto& types = mRSC->mStateType.mTypes;
    for (uint32_t ct = 0; ct < types.size(); ct++) {
        if (types[ct] == this) {
            types.erase(types.begin() + ct);
            break;
        }
    }
}

Type::~Type() {
    clear();
}

void Type::operator delete(void* ptr) {
    if (ptr) {
        Type *t = (Type*) ptr;
        t->getContext()->mHal.funcs.freeRuntimeMem(ptr);
    }
}

void Type::clear() {
    if (mHal.state.lodCount) {
        delete [] mHal.state.lodDimX;
        delete [] mHal.state.lodDimY;
        delete [] mHal.state.lodDimZ;
    }
    if (mHal.state.arrayCount > 0) {
        delete [] mHal.state.arrays;
    }
    mElement.clear();
    memset(&mHal, 0, sizeof(mHal));
}

TypeState::TypeState() {
}

TypeState::~TypeState() {
    rsAssert(!mTypes.size());
}

void Type::compute() {
    uint32_t oldLODCount = mHal.state.lodCount;
    if (mDimLOD) {
        uint32_t l2x = rsFindHighBit(mHal.state.dimX) + 1;
        uint32_t l2y = rsFindHighBit(mHal.state.dimY) + 1;
        uint32_t l2z = rsFindHighBit(mHal.state.dimZ) + 1;

        mHal.state.lodCount = rsMax(l2x, l2y);
        mHal.state.lodCount = rsMax(mHal.state.lodCount, l2z);
    } else {
        if (mHal.state.dimYuv) {
            mHal.state.lodCount = 3;
        } else {
            mHal.state.lodCount = 1;
        }
    }
    if (mHal.state.lodCount != oldLODCount) {
        if (oldLODCount) {
            delete [] mHal.state.lodDimX;
            delete [] mHal.state.lodDimY;
            delete [] mHal.state.lodDimZ;
        }
        mHal.state.lodDimX = new uint32_t[mHal.state.lodCount];
        mHal.state.lodDimY = new uint32_t[mHal.state.lodCount];
        mHal.state.lodDimZ = new uint32_t[mHal.state.lodCount];
    }

    uint32_t tx = mHal.state.dimX;
    uint32_t ty = mHal.state.dimY;
    uint32_t tz = mHal.state.dimZ;
    mCellCount = 0;
    if (!mHal.state.dimYuv) {
        for (uint32_t lod=0; lod < mHal.state.lodCount; lod++) {
            mHal.state.lodDimX[lod] = tx;
            mHal.state.lodDimY[lod] = ty;
            mHal.state.lodDimZ[lod]  = tz;
            mCellCount += tx * rsMax(ty, 1u) * rsMax(tz, 1u);
            if (tx > 1) tx >>= 1;
            if (ty > 1) ty >>= 1;
            if (tz > 1) tz >>= 1;
        }
    }

    if (mHal.state.faces) {
        mCellCount *= 6;
    }
    // YUV only supports basic 2d
    // so we can stash the plane pointers in the mipmap levels.
    if (mHal.state.dimYuv) {
        mHal.state.lodDimX[0] = tx;
        mHal.state.lodDimY[0] = ty;
        mHal.state.lodDimZ[0] = tz;
        mHal.state.lodDimX[1] = mHal.state.lodDimX[0] / 2;
        mHal.state.lodDimY[1] = mHal.state.lodDimY[0] / 2;
        mHal.state.lodDimX[2] = mHal.state.lodDimX[0] / 2;
        mHal.state.lodDimY[2] = mHal.state.lodDimY[0] / 2;
        mCellCount += mHal.state.lodDimX[0] * mHal.state.lodDimY[0];
        mCellCount += mHal.state.lodDimX[1] * mHal.state.lodDimY[1];
        mCellCount += mHal.state.lodDimX[2] * mHal.state.lodDimY[2];

        switch(mHal.state.dimYuv) {
        case HAL_PIXEL_FORMAT_YV12:
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:  // NV21
            mHal.state.lodDimX[1] = mHal.state.lodDimX[0];
            break;
#ifndef RS_COMPATIBILITY_LIB
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            break;
#endif
        default:
            rsAssert(0);
        }
    }
    mHal.state.element = mElement.get();
}

void Type::dumpLOGV(const char *prefix) const {
    char buf[1024];
    ObjectBase::dumpLOGV(prefix);
    ALOGV("%s   Type: x=%u y=%u z=%u mip=%i face=%i", prefix,
                                                      mHal.state.dimX,
                                                      mHal.state.dimY,
                                                      mHal.state.dimZ,
                                                      mHal.state.lodCount,
                                                      mHal.state.faces);
    snprintf(buf, sizeof(buf), "%s element: ", prefix);
    mElement->dumpLOGV(buf);
}

void Type::serialize(Context *rsc, OStream *stream) const {
    // Need to identify ourselves
    stream->addU32((uint32_t)getClassId());
    stream->addString(getName());

    mElement->serialize(rsc, stream);

    stream->addU32(mHal.state.dimX);
    stream->addU32(mHal.state.dimY);
    stream->addU32(mHal.state.dimZ);

    stream->addU8((uint8_t)(mHal.state.lodCount ? 1 : 0));
    stream->addU8((uint8_t)(mHal.state.faces ? 1 : 0));
}

Type *Type::createFromStream(Context *rsc, IStream *stream) {
    // First make sure we are reading the correct object
    RsA3DClassID classID = (RsA3DClassID)stream->loadU32();
    if (classID != RS_A3D_CLASS_ID_TYPE) {
        ALOGE("type loading skipped due to invalid class id\n");
        return nullptr;
    }

    const char *name = stream->loadString();

    Element *elem = Element::createFromStream(rsc, stream);
    if (!elem) {
        return nullptr;
    }

    RsTypeCreateParams p;
    memset(&p, 0, sizeof(p));
    p.dimX = stream->loadU32();
    p.dimY = stream->loadU32();
    p.dimZ = stream->loadU32();
    p.mipmaps = stream->loadU8();
    p.faces = stream->loadU8();
    Type *type = Type::getType(rsc, elem, &p, sizeof(p));
    elem->decUserRef();

    delete [] name;
    return type;
}

bool Type::getIsNp2() const {
    uint32_t x = getDimX();
    uint32_t y = getDimY();
    uint32_t z = getDimZ();

    if (x && (x & (x-1))) {
        return true;
    }
    if (y && (y & (y-1))) {
        return true;
    }
    if (z && (z & (z-1))) {
        return true;
    }
    return false;
}

ObjectBaseRef<Type> Type::getTypeRef(Context *rsc, const Element *e,
                                     const RsTypeCreateParams *params, size_t len) {
    ObjectBaseRef<Type> returnRef;

    TypeState * stc = &rsc->mStateType;

    ObjectBase::asyncLock();
    for (uint32_t ct=0; ct < stc->mTypes.size(); ct++) {
        Type *t = stc->mTypes[ct];
        if (t->getElement() != e) continue;
        if (t->getDimX() != params->dimX) continue;
        if (t->getDimY() != params->dimY) continue;
        if (t->getDimZ() != params->dimZ) continue;
        if (t->getDimLOD() != params->mipmaps) continue;
        if (t->getDimFaces() != params->faces) continue;
        if (t->getDimYuv() != params->yuv) continue;
        if (t->getArray(0) != params->array0) continue;
        if (t->getArray(1) != params->array1) continue;
        if (t->getArray(2) != params->array2) continue;
        if (t->getArray(3) != params->array3) continue;
        returnRef.set(t);
        ObjectBase::asyncUnlock();
        return returnRef;
    }
    ObjectBase::asyncUnlock();

    // Type objects must use allocator specified by the driver
    void* allocMem = rsc->mHal.funcs.allocRuntimeMem(sizeof(Type), 0);
    if (!allocMem) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Couldn't allocate memory for Type");
        return nullptr;
    }

    Type *nt = new (allocMem) Type(rsc);

#ifdef RS_FIND_OFFSETS
    ALOGE("pointer for type: %p", nt);
    ALOGE("pointer for type.drv: %p", &nt->mHal.drv);
#endif

    nt->mDimLOD = params->mipmaps;
    returnRef.set(nt);
    nt->mElement.set(e);
    nt->mHal.state.dimX = params->dimX;
    nt->mHal.state.dimY = params->dimY;
    nt->mHal.state.dimZ = params->dimZ;
    nt->mHal.state.faces = params->faces;
    nt->mHal.state.dimYuv = params->yuv;

    nt->mHal.state.arrayCount = 0;
    if (params->array0 > 0) nt->mHal.state.arrayCount ++;
    if (params->array1 > 0) nt->mHal.state.arrayCount ++;
    if (params->array2 > 0) nt->mHal.state.arrayCount ++;
    if (params->array3 > 0) nt->mHal.state.arrayCount ++;
    if (nt->mHal.state.arrayCount > 0) {
        nt->mHal.state.arrays = new uint32_t[nt->mHal.state.arrayCount];
        if (params->array0 > 0) nt->mHal.state.arrays[0] = params->array0;
        if (params->array1 > 1) nt->mHal.state.arrays[1] = params->array1;
        if (params->array2 > 2) nt->mHal.state.arrays[2] = params->array2;
        if (params->array3 > 3) nt->mHal.state.arrays[3] = params->array3;
    }

    nt->compute();

    ObjectBase::asyncLock();
    stc->mTypes.push_back(nt);
    ObjectBase::asyncUnlock();

    return returnRef;
}

ObjectBaseRef<Type> Type::cloneAndResize1D(Context *rsc, uint32_t dimX) const {
    RsTypeCreateParams p;
    memset(&p, 0, sizeof(p));
    p.dimX = dimX;
    p.dimY = getDimY();
    p.dimZ = getDimZ();
    p.mipmaps = getDimLOD();
    return getTypeRef(rsc, mElement.get(), &p, sizeof(p));
}

ObjectBaseRef<Type> Type::cloneAndResize2D(Context *rsc,
                              uint32_t dimX,
                              uint32_t dimY) const {
    RsTypeCreateParams p;
    memset(&p, 0, sizeof(p));
    p.dimX = dimX;
    p.dimY = dimY;
    p.dimZ = getDimZ();
    p.mipmaps = getDimLOD();
    p.faces = getDimFaces();
    p.yuv = getDimYuv();
    return getTypeRef(rsc, mElement.get(), &p, sizeof(p));
}


void Type::incRefs(const void *ptr, size_t ct, size_t startOff) const {
    const uint8_t *p = static_cast<const uint8_t *>(ptr);
    const Element *e = mHal.state.element;
    uint32_t stride = e->getSizeBytes();

    p += stride * startOff;
    while (ct > 0) {
        e->incRefs(p);
        ct--;
        p += stride;
    }
}


void Type::decRefs(const void *ptr, size_t ct, size_t startOff) const {
    if (!mHal.state.element->getHasReferences()) {
        return;
    }
    const uint8_t *p = static_cast<const uint8_t *>(ptr);
    const Element *e = mHal.state.element;
    uint32_t stride = e->getSizeBytes();

    p += stride * startOff;
    while (ct > 0) {
        e->decRefs(p);
        ct--;
        p += stride;
    }
}

void Type::callUpdateCacheObject(const Context *rsc, void *dstObj) const {
    if (rsc->mHal.funcs.type.updateCachedObject != nullptr) {
        rsc->mHal.funcs.type.updateCachedObject(rsc, this, (rs_type *)dstObj);
    } else {
        *((const void **)dstObj) = this;
    }
}

//////////////////////////////////////////////////
//

RsType rsi_TypeCreate(Context *rsc, RsElement _e, uint32_t dimX,
                     uint32_t dimY, uint32_t dimZ, bool mipmaps, bool faces, uint32_t yuv) {
    Element *e = static_cast<Element *>(_e);

    RsTypeCreateParams p;
    memset(&p, 0, sizeof(p));
    p.dimX = dimX;
    p.dimY = dimY;
    p.dimZ = dimZ;
    p.mipmaps = mipmaps;
    p.faces = faces;
    p.yuv = yuv;
    return Type::getType(rsc, e, &p, sizeof(p));
}

RsType rsi_TypeCreate2(Context *rsc, const RsTypeCreateParams *p, size_t len) {
    Element *e = static_cast<Element *>(p->e);
    return Type::getType(rsc, e, p, len);
}

} // namespace renderscript
} // namespace android
