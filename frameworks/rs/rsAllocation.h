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

#ifndef ANDROID_STRUCTURED_ALLOCATION_H
#define ANDROID_STRUCTURED_ALLOCATION_H

#include "rsType.h"

#include <vector>

struct AHardwareBuffer;

// ---------------------------------------------------------------------------
namespace android {

namespace renderscript {

class Program;
class GrallocConsumer;

/*****************************************************************************
 * CAUTION
 *
 * Any layout changes for this class may require a corresponding change to be
 * made to frameworks/compile/libbcc/lib/ScriptCRT/rs_core.c, which contains
 * a partial copy of the information below.
 *
 *****************************************************************************/
class Allocation : public ObjectBase {
    // The graphics equivalent of malloc.  The allocation contains a structure of elements.

public:
    const static int MAX_LOD = 16;
    // The mininum alignment requirement for RenderScript. Must be power of 2 and larger than 0.
    const static size_t kMinimumRSAlignment = 16;
    // The maximun number of Allocations that can share a single BufferQueue;
    const static uint32_t MAX_NUM_ALLOC = 16;

    struct Hal {
        void * drv;

        struct State {
            const Type * type;

            uint32_t usageFlags;
            RsAllocationMipmapControl mipmapControl;

            // Cached fields from the Type and Element
            // to prevent pointer chasing in critical loops.
            uint32_t yuv;
            uint32_t elementSizeBytes;
            bool hasMipmaps;
            bool hasFaces;
            bool hasReferences;
            void * userProvidedPtr;
            int32_t surfaceTextureID;
            AHardwareBuffer *nativeBuffer;
            int64_t timestamp;

            // Allocation adapter state
            const Allocation *baseAlloc;
            uint32_t originX;
            uint32_t originY;
            uint32_t originZ;
            uint32_t originLOD;
            uint32_t originFace;
            uint32_t originArray[Type::mMaxArrays];
        };
        State state;

        struct DrvState {
            struct LodState {
                void * mallocPtr;
                size_t stride;
                uint32_t dimX;
                uint32_t dimY;
                uint32_t dimZ;
            } lod[android::renderscript::Allocation::MAX_LOD];
            size_t faceOffset;
            uint32_t lodCount;
            uint32_t faceCount;

            struct YuvState {
                uint32_t shift;
                uint32_t step;
            } yuv;

            int grallocFlags;
            uint32_t dimArray[Type::mMaxArrays];
        };
        mutable DrvState drvState;

    };
    Hal mHal;

    void operator delete(void* ptr);

    static Allocation * createAllocation(Context *rsc, const Type *, uint32_t usages,
                                         RsAllocationMipmapControl mc = RS_ALLOCATION_MIPMAP_NONE,
                                         void *ptr = 0);
    static Allocation * createAllocationStrided(Context *rsc, const Type *, uint32_t usages,
                                                RsAllocationMipmapControl mc = RS_ALLOCATION_MIPMAP_NONE,
                                                void *ptr = 0, size_t byteAligned = 16);
    static Allocation * createAdapter(Context *rsc, const Allocation *alloc, const Type *type);


    virtual ~Allocation();
    void updateCache();

    const Type * getType() const {return mHal.state.type;}

    void syncAll(Context *rsc, RsAllocationUsageType src);

    void copyRange1D(Context *rsc, const Allocation *src, int32_t srcOff, int32_t destOff, int32_t len);

    void resize1D(Context *rsc, uint32_t dimX);
    void resize2D(Context *rsc, uint32_t dimX, uint32_t dimY);

    void data(Context *rsc, uint32_t xoff, uint32_t lod, uint32_t count, const void *data, size_t sizeBytes);
    void data(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
              uint32_t w, uint32_t h, const void *data, size_t sizeBytes, size_t stride);
    void data(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
              uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes, size_t stride);

    void read(Context *rsc, uint32_t xoff, uint32_t lod, uint32_t count, void *data, size_t sizeBytes);
    void read(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t lod, RsAllocationCubemapFace face,
              uint32_t w, uint32_t h, void *data, size_t sizeBytes, size_t stride);
    void read(Context *rsc, uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
              uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes, size_t stride);

    void elementData(Context *rsc, uint32_t x, uint32_t y, uint32_t z,
                     const void *data, uint32_t elementOff, size_t sizeBytes);

    void elementRead(Context *rsc, uint32_t x, uint32_t y, uint32_t z,
                     void *data, uint32_t elementOff, size_t sizeBytes);

    void addProgramToDirty(const Program *);
    void removeProgramToDirty(const Program *);

    virtual void dumpLOGV(const char *prefix) const;
    virtual void serialize(Context *rsc, OStream *stream) const;
    virtual RsA3DClassID getClassId() const { return RS_A3D_CLASS_ID_ALLOCATION; }
    static Allocation *createFromStream(Context *rsc, IStream *stream);

    bool getIsScript() const {
        return (mHal.state.usageFlags & RS_ALLOCATION_USAGE_SCRIPT) != 0;
    }
    bool getIsTexture() const {
        return (mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE) != 0;
    }
    bool getIsRenderTarget() const {
        return (mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET) != 0;
    }
    bool getIsBufferObject() const {
        return (mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_VERTEX) != 0;
    }

    void incRefs(const void *ptr, size_t ct, size_t startOff = 0) const;
    void decRefs(const void *ptr, size_t ct, size_t startOff = 0) const;
    virtual void callUpdateCacheObject(const Context *rsc, void *dstObj) const;
    virtual bool freeChildren();

    void sendDirty(const Context *rsc) const;
    bool getHasGraphicsMipmaps() const {
        return mHal.state.mipmapControl != RS_ALLOCATION_MIPMAP_NONE;
    }

    void setupGrallocConsumer(const Context *rsc, uint32_t numAlloc);
    void shareBufferQueue(const Context *rsc, const Allocation *alloc);
    void * getSurface(const Context *rsc);
    void setSurface(const Context *rsc, RsNativeWindow sur);
    void ioSend(const Context *rsc);
    void ioReceive(const Context *rsc);
    int64_t getTimeStamp() {return mHal.state.timestamp;}

    void adapterOffset(Context *rsc, const uint32_t *offsets, size_t len);

    void * getPointer(const Context *rsc, uint32_t lod, RsAllocationCubemapFace face,
                      uint32_t z, uint32_t array, size_t *stride);

    void * getPointerUnchecked(uint32_t x, uint32_t y,
                               uint32_t z = 0, uint32_t lod = 0,
                               RsAllocationCubemapFace face = RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X,
                               uint32_t a1 = 0, uint32_t a2 = 0, uint32_t a3 = 0, uint32_t a4 = 0) const {

        uint8_t * p = (uint8_t *) mHal.drvState.lod[lod].mallocPtr;
        p += x * getType()->getElementSizeBytes();
        p += y * mHal.drvState.lod[lod].stride;
        p += z * mHal.drvState.lod[lod].stride * mHal.drvState.lod[lod].dimY;

        // Todo: arrays

        return p;
    }

    bool hasSameDims(const Allocation *Other) const;

protected:
    std::vector<const Program *> mToDirtyList;
    ObjectBaseRef<const Type> mType;
    void setType(const Type *t) {
        mType.set(t);
        mHal.state.type = t;
    }

#ifndef RS_COMPATIBILITY_LIB
    GrallocConsumer *mGrallocConsumer = nullptr;
    bool mBufferQueueInited = false;
    uint32_t mCurrentIdx;
#endif


private:
    void freeChildrenUnlocked();
    Allocation(Context *rsc, const Type *, uint32_t usages, RsAllocationMipmapControl mc, void *ptr);
    Allocation(Context *rsc, const Allocation *, const Type *);

    uint32_t getPackedSize() const;
    static void writePackedData(Context *rsc, const Type *type, uint8_t *dst,
                                const uint8_t *src, bool dstPadded);
    void unpackVec3Allocation(Context *rsc, const void *data, size_t dataSize);
    void packVec3Allocation(Context *rsc, OStream *stream) const;
};

} // namespace renderscript
} // namespace android
#endif
