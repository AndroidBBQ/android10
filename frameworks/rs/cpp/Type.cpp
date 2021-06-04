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

#include <malloc.h>
#include <string.h>

#include "RenderScript.h"
#include "rsCppInternal.h"

// From system/graphics.h
enum {
    HAL_PIXEL_FORMAT_YV12               = 0x32315659, // YCrCb 4:2:0 Planar
    HAL_PIXEL_FORMAT_YCrCb_420_SP       = 0x11,       // NV21
};

using android::RSC::Element;
using android::RSC::RS;
using android::RSC::Type;
using android::RSC::sp;

void Type::calcElementCount() {
    bool hasLod = hasMipmaps();
    uint32_t x = getX();
    uint32_t y = getY();
    uint32_t z = getZ();
    uint32_t faces = 1;
    if (hasFaces()) {
        faces = 6;
    }
    if (x == 0) {
        x = 1;
    }
    if (y == 0) {
        y = 1;
    }
    if (z == 0) {
        z = 1;
    }

    uint32_t count = x * y * z * faces;
    while (hasLod && ((x > 1) || (y > 1) || (z > 1))) {
        if(x > 1) {
            x >>= 1;
        }
        if(y > 1) {
            y >>= 1;
        }
        if(z > 1) {
            z >>= 1;
        }

        count += x * y * z * faces;
    }
    mElementCount = count;
}


Type::Type(void *id, sp<RS> rs) : BaseObj(id, rs) {
    mDimX = 0;
    mDimY = 0;
    mDimZ = 0;
    mDimMipmaps = false;
    mDimFaces = false;
    mElement = nullptr;
    mYuvFormat = RS_YUV_NONE;
}

void Type::updateFromNative() {
    BaseObj::updateFromNative();

    /*
     * We have 6 integers / pointers (uintptr_t) to obtain from the return buffer:
     * mDimX     (buffer[0]);
     * mDimY     (buffer[1]);
     * mDimZ     (buffer[2]);
     * mDimLOD   (buffer[3]);
     * mDimFaces (buffer[4]);
     * mElement  (buffer[5]);
     */
    uintptr_t dataBuffer[6];
    RS::dispatch->TypeGetNativeData(mRS->getContext(), getID(), dataBuffer, 6);

    mDimX = (uint32_t)dataBuffer[0];
    mDimY = (uint32_t)dataBuffer[1];
    mDimZ = (uint32_t)dataBuffer[2];
    mDimMipmaps = dataBuffer[3] == 1 ? true : false;
    mDimFaces = dataBuffer[4] == 1 ? true : false;

    uintptr_t elementID = dataBuffer[5];
    if(elementID != 0) {
        // Just create a new Element and update it from native.
        sp<Element> e = new Element((void *)elementID, mRS);
        e->updateFromNative();
        mElement = e;
    }
    calcElementCount();
}

sp<const Type> Type::create(const sp<RS>& rs, const sp<const Element>& e, uint32_t dimX, uint32_t dimY, uint32_t dimZ) {
    void * id = RS::dispatch->TypeCreate(rs->getContext(), e->getID(), dimX, dimY, dimZ, false, false, 0);
    Type *t = new Type(id, rs);

    t->mElement = e;
    t->mDimX = dimX;
    t->mDimY = dimY;
    t->mDimZ = dimZ;
    t->mDimMipmaps = false;
    t->mDimFaces = false;
    t->mYuvFormat = RS_YUV_NONE;

    t->calcElementCount();

    return t;
}

Type::Builder::Builder(sp<RS> rs, sp<const Element> e) {
    mRS = rs.get();
    mElement = e;
    mDimX = 0;
    mDimY = 0;
    mDimZ = 0;
    mDimMipmaps = false;
    mDimFaces = false;
    mYuvFormat = RS_YUV_NONE;
}

void Type::Builder::setX(uint32_t value) {
    if(value < 1) {
        ALOGE("Values of less than 1 for Dimension X are not valid.");
    }
    mDimX = value;
}

void Type::Builder::setY(uint32_t value) {
    if(value < 1) {
        ALOGE("Values of less than 1 for Dimension Y are not valid.");
    }
    mDimY = value;
}

void Type::Builder::setZ(uint32_t value) {
    if(value < 1) {
        ALOGE("Values of less than 1 for Dimension Z are not valid.");
    }
    mDimZ = value;
}

void Type::Builder::setYuvFormat(RsYuvFormat format) {
    if (format != RS_YUV_NONE && !(mElement->isCompatible(Element::YUV(mRS)))) {
        ALOGE("Invalid element for use with YUV.");
        return;
    }

    if (format != RS_YUV_NONE &&
        format != RS_YUV_YV12 &&
        format != RS_YUV_NV21 &&
        format != RS_YUV_420_888) {
        ALOGE("Invalid YUV format.");
        return;
    }
    mYuvFormat = format;
}


void Type::Builder::setMipmaps(bool value) {
    mDimMipmaps = value;
}

void Type::Builder::setFaces(bool value) {
    mDimFaces = value;
}

sp<const Type> Type::Builder::create() {
    if (mDimZ > 0) {
        if ((mDimX < 1) || (mDimY < 1)) {
            ALOGE("Both X and Y dimension required when Z is present.");
            return nullptr;
        }
        if (mDimFaces) {
            ALOGE("Cube maps not supported with 3D types.");
            return nullptr;
        }
    }
    if (mDimY > 0) {
        if (mDimX < 1) {
            ALOGE("X dimension required when Y is present.");
            return nullptr;
        }
    }
    if (mDimFaces) {
        if (mDimY < 1) {
            ALOGE("Cube maps require 2D Types.");
            return nullptr;
        }
    }

    if (mYuvFormat != RS_YUV_NONE) {
        if (mDimZ || mDimFaces || mDimMipmaps) {
            ALOGE("YUV only supports basic 2D.");
            return nullptr;
        }
    }

    if (mYuvFormat == RS_YUV_420_888) {
        ALOGE("YUV_420_888 not supported.");
        return nullptr;
    }

    void * id = RS::dispatch->TypeCreate(mRS->getContext(), mElement->getID(), mDimX, mDimY, mDimZ,
                                         mDimMipmaps, mDimFaces, mYuvFormat);
    Type *t = new Type(id, mRS);
    t->mElement = mElement;
    t->mDimX = mDimX;
    t->mDimY = mDimY;
    t->mDimZ = mDimZ;
    t->mDimMipmaps = mDimMipmaps;
    t->mDimFaces = mDimFaces;
    t->mYuvFormat = mYuvFormat;

    t->calcElementCount();
    return t;
}

