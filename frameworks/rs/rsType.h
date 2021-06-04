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

#ifndef ANDROID_STRUCTURED_TYPE_H
#define ANDROID_STRUCTURED_TYPE_H

#include "rsElement.h"

#include <vector>

// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {
/*****************************************************************************
 * CAUTION
 *
 * Any layout changes for this class may require a corresponding change to be
 * made to frameworks/rs/driver/runtime/rs_structs.h, which contains
 * a partial copy of the information below.
 *
 *****************************************************************************/

class Type : public ObjectBase {
public:
    const static uint32_t mMaxArrays = 4;

    struct Hal {
        mutable void *drv;

        struct State {
            const Element * element;

            // Size of the structure in the various dimensions.  A missing Dimension is
            // specified as a 0 and not a 1.
            uint32_t dimX;
            uint32_t dimY;
            uint32_t dimZ;
            uint32_t *lodDimX;
            uint32_t *lodDimY;
            uint32_t *lodDimZ;
            uint32_t *arrays;
            uint32_t lodCount;
            uint32_t dimYuv;
            uint32_t arrayCount;
            bool faces;
        };
        State state;
    };
    Hal mHal;

    void operator delete(void* ptr);

    Type * createTex2D(const Element *, size_t w, size_t h, bool mip);

    size_t getCellCount() const {return mCellCount;}
    size_t getElementSizeBytes() const {return mElement->getSizeBytes();}
    size_t getPackedSizeBytes() const {return mCellCount * mElement->getSizeBytes();}
    const Element * getElement() const {return mElement.get();}

    uint32_t getDimX() const {return mHal.state.dimX;}
    uint32_t getDimY() const {return mHal.state.dimY;}
    uint32_t getDimZ() const {return mHal.state.dimZ;}
    bool getDimLOD() const {return mDimLOD;}
    bool getDimFaces() const {return mHal.state.faces;}
    uint32_t getDimYuv() const {return mHal.state.dimYuv;}
    uint32_t getArray(uint32_t idx) const {
        if (idx < mHal.state.arrayCount) {
            return mHal.state.arrays[idx];
        }
        return 0;
    }

    uint32_t getLODDimX(uint32_t lod) const {
        rsAssert(lod < mHal.state.lodCount);
        return mHal.state.lodDimX[lod];
    }
    uint32_t getLODDimY(uint32_t lod) const {
        rsAssert(lod < mHal.state.lodCount);
        return mHal.state.lodDimY[lod];
    }
    uint32_t getLODDimZ(uint32_t lod) const {
        rsAssert(lod < mHal.state.lodCount);
        return mHal.state.lodDimZ[lod];
    }

    uint32_t getLODCount() const {return mHal.state.lodCount;}
    bool getIsNp2() const;

    void clear();
    void compute();

    void dumpLOGV(const char *prefix) const;
    virtual void serialize(Context *rsc, OStream *stream) const;
    virtual RsA3DClassID getClassId() const { return RS_A3D_CLASS_ID_TYPE; }
    static Type *createFromStream(Context *rsc, IStream *stream);

    ObjectBaseRef<Type> cloneAndResize1D(Context *rsc, uint32_t dimX) const;
    ObjectBaseRef<Type> cloneAndResize2D(Context *rsc, uint32_t dimX, uint32_t dimY) const;

    static ObjectBaseRef<Type> getTypeRef(Context *rsc, const Element *e,
                                          const RsTypeCreateParams *params, size_t len);

    static Type* getType(Context *rsc, const Element *e,
                         const RsTypeCreateParams *params, size_t len) {
        ObjectBaseRef<Type> type = getTypeRef(rsc, e, params, len);
        type->incUserRef();
        return type.get();
    }

    static ObjectBaseRef<Type> getTypeRef(Context *rsc, const Element *e, uint32_t dimX, uint32_t dimY = 0) {
        RsTypeCreateParams p;
        memset(&p, 0, sizeof(p));
        p.dimX = dimX;
        p.dimY = dimY;
        return getTypeRef(rsc, e, &p, sizeof(p));
    }

    void incRefs(const void *ptr, size_t ct, size_t startOff = 0) const;
    void decRefs(const void *ptr, size_t ct, size_t startOff = 0) const;
    virtual void callUpdateCacheObject(const Context *rsc, void *dstObj) const;

protected:
    void makeLODTable();
    bool mDimLOD;

    // Internal structure from most to least significant.
    // * Array dimensions
    // * Faces
    // * Mipmaps
    // * xyz

    ObjectBaseRef<const Element> mElement;

    // count of mipmap levels, 0 indicates no mipmapping

    size_t mCellCount;
protected:
    virtual void preDestroy() const;
    virtual ~Type();

private:
    explicit Type(Context *);
    Type(const Type &);
};


class TypeState {
public:
    TypeState();
    ~TypeState();

    // Cache of all existing types.
    std::vector<Type *> mTypes;
};


} // namespace renderscript
} // namespace android
#endif //ANDROID_STRUCTURED_TYPE
