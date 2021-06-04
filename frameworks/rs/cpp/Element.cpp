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

using android::RSC::Element;

android::RSC::sp<const Element> Element::getSubElement(uint32_t index) {
    if (!mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Element contains no sub-elements");
        return nullptr;
    }
    if (index >= mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Illegal sub-element index");
        return nullptr;
    }
    return mElements[mVisibleElementMap[index]];
}

const char * Element::getSubElementName(uint32_t index) {
    if (!mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Element contains no sub-elements");
        return nullptr;
    }
    if (index >= mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Illegal sub-element index");
        return nullptr;
    }
    return mElementNames[mVisibleElementMap[index]];
}

size_t Element::getSubElementArraySize(uint32_t index) {
    if (!mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Element contains no sub-elements");
        return 0;
    }
    if (index >= mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Illegal sub-element index");
        return 0;
    }
    return mArraySizes[mVisibleElementMap[index]];
}

uint32_t Element::getSubElementOffsetBytes(uint32_t index) {
    if (!mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Element contains no sub-elements");
        return 0;
    }
    if (index >= mVisibleElementMapSize) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Illegal sub-element index");
        return 0;
    }
    return mOffsetInBytes[mVisibleElementMap[index]];
}


#define CREATE_USER(N, T) android::RSC::sp<const Element> Element::N(const android::RSC::sp<RS>& rs) { \
    if (rs->mElements.N == nullptr) {                               \
        rs->mElements.N = (createUser(rs, RS_TYPE_##T));            \
    }                                                               \
    return rs->mElements.N;                                         \
    }

CREATE_USER(BOOLEAN, BOOLEAN);
CREATE_USER(U8, UNSIGNED_8);
CREATE_USER(I8, SIGNED_8);
CREATE_USER(U16, UNSIGNED_16);
CREATE_USER(I16, SIGNED_16);
CREATE_USER(U32, UNSIGNED_32);
CREATE_USER(I32, SIGNED_32);
CREATE_USER(U64, UNSIGNED_64);
CREATE_USER(I64, SIGNED_64);
CREATE_USER(F16, FLOAT_16);
CREATE_USER(F32, FLOAT_32);
CREATE_USER(F64, FLOAT_64);
CREATE_USER(ELEMENT, ELEMENT);
CREATE_USER(TYPE, TYPE);
CREATE_USER(ALLOCATION, ALLOCATION);
CREATE_USER(SAMPLER, SAMPLER);
CREATE_USER(SCRIPT, SCRIPT);
CREATE_USER(MATRIX_4X4, MATRIX_4X4);
CREATE_USER(MATRIX_3X3, MATRIX_3X3);
CREATE_USER(MATRIX_2X2, MATRIX_2X2);

#define CREATE_PIXEL(N, T, K) android::RSC::sp<const Element> Element::N(const android::RSC::sp<RS> &rs) { \
    if (rs->mElements.N == nullptr) {                                \
        rs->mElements.N = createPixel(rs, RS_TYPE_##T, RS_KIND_##K); \
    }                                                                \
    return rs->mElements.N;                                          \
}

CREATE_PIXEL(A_8, UNSIGNED_8, PIXEL_A);
CREATE_PIXEL(RGB_565, UNSIGNED_5_6_5, PIXEL_RGB);
CREATE_PIXEL(RGB_888, UNSIGNED_8, PIXEL_RGB);
CREATE_PIXEL(RGBA_4444, UNSIGNED_4_4_4_4, PIXEL_RGBA);
CREATE_PIXEL(RGBA_8888, UNSIGNED_8, PIXEL_RGBA);
CREATE_PIXEL(YUV, UNSIGNED_8, PIXEL_YUV);
CREATE_PIXEL(RGBA_5551, UNSIGNED_5_5_5_1, PIXEL_RGBA);

#define CREATE_VECTOR(N, T) android::RSC::sp<const Element> Element::N##_2(const android::RSC::sp<RS> &rs) { \
    if (rs->mElements.N##_2 == nullptr) {                                 \
        rs->mElements.N##_2 = createVector(rs, RS_TYPE_##T, 2);           \
    }                                                                     \
    return rs->mElements.N##_2;                                           \
}                                                                         \
android::RSC::sp<const Element> Element::N##_3(const android::RSC::sp<RS> &rs) { \
    if (rs->mElements.N##_3 == nullptr) {                                 \
        rs->mElements.N##_3 = createVector(rs, RS_TYPE_##T, 3);           \
    }                                                                     \
    return rs->mElements.N##_3;                                           \
} \
android::RSC::sp<const Element> Element::N##_4(const android::RSC::sp<RS> &rs) { \
    if (rs->mElements.N##_4 == nullptr) {                                 \
        rs->mElements.N##_4 = createVector(rs, RS_TYPE_##T, 4);           \
    }                                                                     \
    return rs->mElements.N##_4;                                           \
}
CREATE_VECTOR(U8, UNSIGNED_8);
CREATE_VECTOR(I8, SIGNED_8);
CREATE_VECTOR(U16, UNSIGNED_16);
CREATE_VECTOR(I16, SIGNED_16);
CREATE_VECTOR(U32, UNSIGNED_32);
CREATE_VECTOR(I32, SIGNED_32);
CREATE_VECTOR(U64, UNSIGNED_64);
CREATE_VECTOR(I64, SIGNED_64);
CREATE_VECTOR(F16, FLOAT_16);
CREATE_VECTOR(F32, FLOAT_32);
CREATE_VECTOR(F64, FLOAT_64);


void Element::updateVisibleSubElements() {
    if (!mElementsCount) {
        return;
    }
    if (mVisibleElementMapSize) {
        free(mVisibleElementMap);
        mVisibleElementMapSize = 0;
    }
    mVisibleElementMap = (uint32_t*)calloc(mElementsCount, sizeof(uint32_t));

    int noPaddingFieldCount = 0;
    size_t fieldCount = mElementsCount;
    // Find out how many elements are not padding.
    for (size_t ct = 0; ct < fieldCount; ct ++) {
        if (mElementNames[ct][0] != '#') {
            noPaddingFieldCount ++;
        }
    }

    // Make a map that points us at non-padding elements.
    size_t i = 0;
    for (size_t ct = 0; ct < fieldCount; ct ++) {
        if (mElementNames[ct][0] != '#') {
            mVisibleElementMap[i++] = (uint32_t)ct;
        }
    }
    mVisibleElementMapSize = i;
}

Element::Element(void *id, android::RSC::sp<RS> rs,
                 android::RSC::sp<const Element> * elements,
                 size_t elementCount,
                 const char ** elementNames,
                 size_t * elementNameLengths,
                 uint32_t * arraySizes) : BaseObj(id, rs) {
    mSizeBytes = 0;
    mVectorSize = 1;
    mElementsCount = elementCount;
    mVisibleElementMap = nullptr;
    mVisibleElementMapSize = 0;

    mElements = (android::RSC::sp<const Element> *)calloc(mElementsCount, sizeof(android::RSC::sp<const Element>));
    mElementNames = (char **)calloc(mElementsCount, sizeof(char *));
    mElementNameLengths = (size_t*)calloc(mElementsCount, sizeof(size_t));
    mArraySizes = (uint32_t*)calloc(mElementsCount, sizeof(uint32_t));
    mOffsetInBytes = (uint32_t*)calloc(mElementsCount, sizeof(uint32_t));

    memcpy(mElements, elements, mElementsCount * sizeof(android::RSC::sp<Element>));
    memcpy(mArraySizes, arraySizes, mElementsCount * sizeof(uint32_t));

    // Copy strings (char array).
    memcpy(mElementNameLengths, elementNameLengths, mElementsCount * sizeof(size_t));
    for (size_t ct = 0; ct < mElementsCount; ct++ ) {
        size_t elemNameLen = mElementNameLengths[ct];
        mElementNames[ct] = (char *)calloc(elemNameLen, sizeof(char));
        memcpy(mElementNames[ct], elementNames[ct], elemNameLen);
    }

    mType = RS_TYPE_NONE;
    mKind = RS_KIND_USER;

    for (size_t ct = 0; ct < mElementsCount; ct++ ) {
        mOffsetInBytes[ct] = mSizeBytes;
        mSizeBytes += mElements[ct]->mSizeBytes * mArraySizes[ct];
    }
    updateVisibleSubElements();
}

Element::Element(void *id, android::RSC::sp<RS> rs) :
    BaseObj(id, rs) {
}

static uint32_t GetSizeInBytesForType(RsDataType dt) {
    switch(dt) {
    case RS_TYPE_NONE:
        return 0;
    case RS_TYPE_SIGNED_8:
    case RS_TYPE_UNSIGNED_8:
    case RS_TYPE_BOOLEAN:
        return 1;

    case RS_TYPE_FLOAT_16:
    case RS_TYPE_SIGNED_16:
    case RS_TYPE_UNSIGNED_16:
    case RS_TYPE_UNSIGNED_5_6_5:
    case RS_TYPE_UNSIGNED_5_5_5_1:
    case RS_TYPE_UNSIGNED_4_4_4_4:
        return 2;

    case RS_TYPE_FLOAT_32:
    case RS_TYPE_SIGNED_32:
    case RS_TYPE_UNSIGNED_32:
        return 4;

    case RS_TYPE_FLOAT_64:
    case RS_TYPE_SIGNED_64:
    case RS_TYPE_UNSIGNED_64:
        return 8;

    case RS_TYPE_MATRIX_4X4:
        return 16 * 4;
    case RS_TYPE_MATRIX_3X3:
        return 9 * 4;
    case RS_TYPE_MATRIX_2X2:
        return 4 * 4;

    case RS_TYPE_TYPE:
    case RS_TYPE_ALLOCATION:
    case RS_TYPE_SAMPLER:
    case RS_TYPE_SCRIPT:
    case RS_TYPE_MESH:
    case RS_TYPE_PROGRAM_FRAGMENT:
    case RS_TYPE_PROGRAM_VERTEX:
    case RS_TYPE_PROGRAM_RASTER:
    case RS_TYPE_PROGRAM_STORE:
        return 4;

    default:
        break;
    }

    ALOGE("Missing type %i", dt);
    return 0;
}

Element::Element(void *id, android::RSC::sp<RS> rs,
                 RsDataType dt, RsDataKind dk, bool norm, uint32_t size) :
    BaseObj(id, rs)
{
    uint32_t tsize = GetSizeInBytesForType(dt);
    if ((dt != RS_TYPE_UNSIGNED_5_6_5) &&
        (dt != RS_TYPE_UNSIGNED_4_4_4_4) &&
        (dt != RS_TYPE_UNSIGNED_5_5_5_1)) {
        if (size == 3) {
            mSizeBytes = tsize * 4;
        } else {
            mSizeBytes = tsize * size;
        }
    } else {
        mSizeBytes = tsize;
    }
    mType = dt;
    mKind = dk;
    mNormalized = norm;
    mVectorSize = size;
    mElementsCount = 0;
    mVisibleElementMap = 0;
}

Element::~Element() {
    if (mElementsCount) {
        free(mElements);
        for (size_t ct = 0; ct < mElementsCount; ct++ ) {
            free(mElementNames[ct]);
        }
        free(mElementNames);
        free(mElementNameLengths);
        free(mArraySizes);
        free(mOffsetInBytes);
    }
    if (mVisibleElementMapSize) {
        free(mVisibleElementMap);
    }
}

void Element::updateFromNative() {
    BaseObj::updateFromNative();
    updateVisibleSubElements();
}

android::RSC::sp<const Element> Element::createUser(const android::RSC::sp<RS>& rs, RsDataType dt) {
    void * id = RS::dispatch->ElementCreate(rs->getContext(), dt, RS_KIND_USER, false, 1);
    return new Element(id, rs, dt, RS_KIND_USER, false, 1);
}

android::RSC::sp<const Element> Element::createVector(const android::RSC::sp<RS>& rs, RsDataType dt, uint32_t size) {
    if (size < 2 || size > 4) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Vector size out of range 2-4.");
        return nullptr;
    }
    void *id = RS::dispatch->ElementCreate(rs->getContext(), dt, RS_KIND_USER, false, size);
    return new Element(id, rs, dt, RS_KIND_USER, false, size);
}

android::RSC::sp<const Element> Element::createPixel(const android::RSC::sp<RS>& rs, RsDataType dt, RsDataKind dk) {
    if (!(dk == RS_KIND_PIXEL_L ||
          dk == RS_KIND_PIXEL_A ||
          dk == RS_KIND_PIXEL_LA ||
          dk == RS_KIND_PIXEL_RGB ||
          dk == RS_KIND_PIXEL_RGBA ||
          dk == RS_KIND_PIXEL_DEPTH ||
          dk == RS_KIND_PIXEL_YUV)) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Unsupported DataKind");
        return nullptr;
    }
    if (!(dt == RS_TYPE_UNSIGNED_8 ||
          dt == RS_TYPE_UNSIGNED_16 ||
          dt == RS_TYPE_UNSIGNED_5_6_5 ||
          dt == RS_TYPE_UNSIGNED_4_4_4_4 ||
          dt == RS_TYPE_UNSIGNED_5_5_5_1)) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Unsupported DataType");
        return nullptr;
    }
    if (dt == RS_TYPE_UNSIGNED_5_6_5 && dk != RS_KIND_PIXEL_RGB) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Bad kind and type combo");
        return nullptr;
    }
    if (dt == RS_TYPE_UNSIGNED_5_5_5_1 && dk != RS_KIND_PIXEL_RGBA) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Bad kind and type combo");
        return nullptr;
    }
    if (dt == RS_TYPE_UNSIGNED_4_4_4_4 && dk != RS_KIND_PIXEL_RGBA) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Bad kind and type combo");
        return nullptr;
    }
    if (dt == RS_TYPE_UNSIGNED_16 && dk != RS_KIND_PIXEL_DEPTH) {
        rs->throwError(RS_ERROR_INVALID_PARAMETER, "Bad kind and type combo");
        return nullptr;
    }

    int size = 1;
    switch (dk) {
    case RS_KIND_PIXEL_LA:
        size = 2;
        break;
    case RS_KIND_PIXEL_RGB:
        size = 3;
        break;
    case RS_KIND_PIXEL_RGBA:
        size = 4;
        break;
    case RS_KIND_PIXEL_DEPTH:
        size = 2;
        break;
    default:
        break;
    }

    void * id = RS::dispatch->ElementCreate(rs->getContext(), dt, dk, true, size);
    return new Element(id, rs, dt, dk, true, size);
}

bool Element::isCompatible(const android::RSC::sp<const Element>&e) const {
    // Try strict BaseObj equality to start with.
    if (this == e.get()) {
        return true;
    }

    /*
     * Ignore mKind because it is allowed to be different (user vs. pixel).
     * We also ignore mNormalized because it can be different. The mType
     * field must be non-null since we require name equivalence for
     * user-created Elements.
     */
    return ((mSizeBytes == e->mSizeBytes) &&
            (mType != RS_TYPE_NONE) &&
            (mType == e->mType) &&
            (mVectorSize == e->mVectorSize));
}

Element::Builder::Builder(android::RSC::sp<RS> rs) {
    mRS = rs.get();
    mSkipPadding = false;
    mElementsVecSize = 8;
    mElementsCount = 0;
    // Initialize space.
    mElements = (android::RSC::sp<const Element> *)calloc(mElementsVecSize, sizeof(android::RSC::sp<const Element>));
    mElementNames = (char **)calloc(mElementsVecSize, sizeof(char *));
    mElementNameLengths = (size_t*)calloc(mElementsVecSize, sizeof(size_t));
    mArraySizes = (uint32_t*)calloc(mElementsVecSize, sizeof(uint32_t));
}

Element::Builder::~Builder() {
    // Free allocated space.
    free(mElements);
    for (size_t ct = 0; ct < mElementsCount; ct++ ) {
        free(mElementNames[ct]);
    }
    free(mElementNameLengths);
    free(mElementNames);
    free(mArraySizes);
}

void Element::Builder::add(const android::RSC::sp<const Element>&e, const char * name, uint32_t arraySize) {
    // Skip padding fields after a vector 3 type.
    if (mSkipPadding) {
        const char *s1 = "#padding_";
        const char *s2 = name;
        size_t len = strlen(s1);
        if (strlen(s2) >= len) {
            if (!memcmp(s1, s2, len)) {
                mSkipPadding = false;
                return;
            }
        }
    }

    if (e->mVectorSize == 3) {
        mSkipPadding = true;
    } else {
        mSkipPadding = false;
    }

    if (mElementsCount >= mElementsVecSize) {
        // If pre-allocated space is full, allocate a larger one.
        mElementsVecSize += 8;

        android::RSC::sp<const Element> * newElements = (android::RSC::sp<const Element> *)calloc(mElementsVecSize, sizeof(android::RSC::sp<const Element>));
        char ** newElementNames = (char **)calloc(mElementsVecSize, sizeof(char *));
        size_t * newElementNameLengths = (size_t*)calloc(mElementsVecSize, sizeof(size_t));
        uint32_t * newArraySizes = (uint32_t*)calloc(mElementsVecSize, sizeof(uint32_t));

        memcpy(newElements, mElements, mElementsCount * sizeof(android::RSC::sp<Element>));
        memcpy(newElementNames, mElementNames, mElementsCount * sizeof(char *));
        memcpy(newElementNameLengths, mElementNameLengths, mElementsCount * sizeof(size_t));
        memcpy(newArraySizes, mArraySizes, mElementsCount * sizeof(uint32_t));

        // Free the old arrays.
        free(mElements);
        free(mElementNames);
        free(mArraySizes);
        free(mElementNameLengths);

        mElements = newElements;
        mElementNames = newElementNames;
        mArraySizes = newArraySizes;
        mElementNameLengths = newElementNameLengths;
    }
    mElements[mElementsCount] = e;
    mArraySizes[mElementsCount] = arraySize;

    size_t nameLen = strlen(name);
    mElementNameLengths[mElementsCount] = nameLen + 1;
    mElementNames[mElementsCount] = (char *)calloc(nameLen + 1, sizeof(char));
    memcpy(mElementNames[mElementsCount], name, nameLen);
    mElementNames[mElementsCount][nameLen] = 0;

    mElementsCount++;
}

android::RSC::sp<const Element> Element::Builder::create() {
    size_t fieldCount = mElementsCount;
    void ** elementArray = (void **)calloc(fieldCount, sizeof(void *));

    for (size_t ct = 0; ct < fieldCount; ct++) {
        elementArray[ct] = mElements[ct]->getID();
    }

    void *id = RS::dispatch->ElementCreate2(mRS->getContext(),
                                            (RsElement *)elementArray, fieldCount,
                                            (const char **)mElementNames, fieldCount, mElementNameLengths,
                                            mArraySizes, fieldCount);
    free(elementArray);
    return new Element(id, mRS, mElements, mElementsCount, (const char **)mElementNames, mElementNameLengths, mArraySizes);
}
