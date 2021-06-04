/*
 * Copyright (C) 2009 The Android Open Source Project
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

namespace android {
namespace renderscript {

Element::Element(Context *rsc) : ObjectBase(rsc) {
    mBits = 0;
    mBitsUnpadded = 0;
    mFields = nullptr;
    mFieldCount = 0;
    mHasReference = false;
    memset(&mHal, 0, sizeof(mHal));
}

Element::~Element() {
    clear();
}

void Element::operator delete(void* ptr) {
    if (ptr) {
        Element *e = (Element*) ptr;
        e->getContext()->mHal.funcs.freeRuntimeMem(ptr);
    }
}

void Element::preDestroy() const {
    auto& elements = mRSC->mStateElement.mElements;
    for (uint32_t ct = 0; ct < elements.size(); ct++) {
        if (elements[ct] == this) {
            elements.erase(elements.begin() + ct);
            break;
        }
    }
}

void Element::clear() {
    if (mFields) {
        for (size_t i = 0; i < mFieldCount; i++) {
            delete[] mFields[i].name;
        }
        delete [] mFields;
    }
    mFields = nullptr;
    mFieldCount = 0;
    mHasReference = false;

    delete [] mHal.state.fields;
    delete [] mHal.state.fieldArraySizes;
    delete [] mHal.state.fieldNames;
    delete [] mHal.state.fieldNameLengths;
    delete [] mHal.state.fieldOffsetBytes;
}

size_t Element::getSizeBits() const {
    if (!mFieldCount) {
        return mBits;
    }

    size_t total = 0;
    for (size_t ct=0; ct < mFieldCount; ct++) {
        total += mFields[ct].e->mBits * mFields[ct].arraySize;
    }
    return total;
}

size_t Element::getSizeBitsUnpadded() const {
    if (!mFieldCount) {
        return mBitsUnpadded;
    }

    size_t total = 0;
    for (size_t ct=0; ct < mFieldCount; ct++) {
        total += mFields[ct].e->mBitsUnpadded * mFields[ct].arraySize;
    }
    return total;
}

void Element::dumpLOGV(const char *prefix) const {
    ObjectBase::dumpLOGV(prefix);
    ALOGV("%s Element: fieldCount: %zu,  size bytes: %zu", prefix, mFieldCount, getSizeBytes());
    mComponent.dumpLOGV(prefix);
    for (uint32_t ct = 0; ct < mFieldCount; ct++) {
        ALOGV("%s Element field index: %u ------------------", prefix, ct);
        ALOGV("%s name: %s, offsetBits: %u, arraySize: %u",
             prefix, mFields[ct].name, mFields[ct].offsetBits, mFields[ct].arraySize);
        mFields[ct].e->dumpLOGV(prefix);
    }
}

void Element::serialize(Context *rsc, OStream *stream) const {
    // Need to identify ourselves
    stream->addU32((uint32_t)getClassId());
    stream->addString(getName());

    mComponent.serialize(stream);

    // Now serialize all the fields
    stream->addU32(mFieldCount);
    for (uint32_t ct = 0; ct < mFieldCount; ct++) {
        stream->addString(mFields[ct].name);
        stream->addU32(mFields[ct].arraySize);
        mFields[ct].e->serialize(rsc, stream);
    }
}

Element *Element::createFromStream(Context *rsc, IStream *stream) {
    // First make sure we are reading the correct object
    RsA3DClassID classID = (RsA3DClassID)stream->loadU32();
    if (classID != RS_A3D_CLASS_ID_ELEMENT) {
        ALOGE("element loading skipped due to invalid class id\n");
        return nullptr;
    }

    const char *name = stream->loadString();

    Component component;
    component.loadFromStream(stream);

    uint32_t fieldCount = stream->loadU32();
    if (!fieldCount) {
        return (Element *)Element::create(rsc,
                                          component.getType(),
                                          component.getKind(),
                                          component.getIsNormalized(),
                                          component.getVectorSize());
    }

    const Element **subElems = new const Element *[fieldCount];
    const char **subElemNames = new const char *[fieldCount];
    size_t *subElemNamesLengths = new size_t[fieldCount];
    uint32_t *arraySizes = new uint32_t[fieldCount];

    for (uint32_t ct = 0; ct < fieldCount; ct ++) {
        subElemNames[ct] = stream->loadString();
        subElemNamesLengths[ct] = strlen(subElemNames[ct]);
        arraySizes[ct] = stream->loadU32();
        subElems[ct] = Element::createFromStream(rsc, stream);
    }

    const Element *elem = Element::create(rsc, fieldCount, subElems, subElemNames,
                                          subElemNamesLengths, arraySizes);
    for (uint32_t ct = 0; ct < fieldCount; ct ++) {
        delete [] subElemNames[ct];
        subElems[ct]->decUserRef();
    }
    delete[] name;
    delete[] subElems;
    delete[] subElemNames;
    delete[] subElemNamesLengths;
    delete[] arraySizes;

    return (Element *)elem;
}

void Element::compute() {
    mHal.state.dataType = mComponent.getType();
    mHal.state.dataKind = mComponent.getKind();
    mHal.state.vectorSize = mComponent.getVectorSize();

    if (mFieldCount == 0) {
        mBits = mComponent.getBits();
        mBitsUnpadded = mComponent.getBitsUnpadded();
        mHasReference = mComponent.isReference();

        mHal.state.elementSizeBytes = getSizeBytes();
        return;
    }

    uint32_t noPaddingFieldCount = 0;
    for (uint32_t ct = 0; ct < mFieldCount; ct ++) {
        if (mFields[ct].name[0] != '#') {
            noPaddingFieldCount ++;
        }
    }

    mHal.state.fields = new const Element*[noPaddingFieldCount];
    mHal.state.fieldArraySizes = new uint32_t[noPaddingFieldCount];
    mHal.state.fieldNames = new const char*[noPaddingFieldCount];
    mHal.state.fieldNameLengths = new uint32_t[noPaddingFieldCount];
    mHal.state.fieldOffsetBytes = new uint32_t[noPaddingFieldCount];
    mHal.state.fieldsCount = noPaddingFieldCount;

    size_t bits = 0;
    size_t bitsUnpadded = 0;
    for (size_t ct = 0, ctNoPadding = 0; ct < mFieldCount; ct++) {
        mFields[ct].offsetBits = bits;
        mFields[ct].offsetBitsUnpadded = bitsUnpadded;
        bits += mFields[ct].e->getSizeBits() * mFields[ct].arraySize;
        bitsUnpadded += mFields[ct].e->getSizeBitsUnpadded() * mFields[ct].arraySize;

        if (mFields[ct].e->mHasReference) {
            mHasReference = true;
        }

        if (mFields[ct].name[0] == '#') {
            continue;
        }

        mHal.state.fields[ctNoPadding] = mFields[ct].e.get();
        mHal.state.fieldArraySizes[ctNoPadding] = mFields[ct].arraySize;
        mHal.state.fieldNames[ctNoPadding] = mFields[ct].name;
        mHal.state.fieldNameLengths[ctNoPadding] = strlen(mFields[ct].name) + 1; // to include 0
        mHal.state.fieldOffsetBytes[ctNoPadding] = mFields[ct].offsetBits >> 3;

        ctNoPadding ++;
    }

    mBits = bits;
    mBitsUnpadded = bitsUnpadded;
    mHal.state.elementSizeBytes = getSizeBytes();
}

ObjectBaseRef<const Element> Element::createRef(Context *rsc, RsDataType dt, RsDataKind dk,
                                bool isNorm, uint32_t vecSize) {
    ObjectBaseRef<const Element> returnRef;
    // Look for an existing match.
    ObjectBase::asyncLock();
    for (uint32_t ct=0; ct < rsc->mStateElement.mElements.size(); ct++) {
        const Element *ee = rsc->mStateElement.mElements[ct];
        if (!ee->getFieldCount() &&
            (ee->getComponent().getType() == dt) &&
            (ee->getComponent().getKind() == dk) &&
            (ee->getComponent().getIsNormalized() == isNorm) &&
            (ee->getComponent().getVectorSize() == vecSize)) {
            // Match
            returnRef.set(ee);
            ObjectBase::asyncUnlock();
            return ee;
        }
    }
    ObjectBase::asyncUnlock();

    // Element objects must use allocator specified by the driver
    void* allocMem = rsc->mHal.funcs.allocRuntimeMem(sizeof(Element), 0);
    if (!allocMem) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Couldn't allocate memory for Element");
        return nullptr;
    }

    Element *e = new (allocMem) Element(rsc);
    returnRef.set(e);
    e->mComponent.set(dt, dk, isNorm, vecSize);
    e->compute();

#ifdef RS_FIND_OFFSETS
    ALOGE("pointer for element: %p", e);
    ALOGE("pointer for element.drv: %p", &e->mHal.drv);
#endif


    ObjectBase::asyncLock();
    rsc->mStateElement.mElements.push_back(e);
    ObjectBase::asyncUnlock();

    return returnRef;
}

ObjectBaseRef<const Element> Element::createRef(Context *rsc, size_t count, const Element **ein,
                            const char **nin, const size_t * lengths, const uint32_t *asin) {

    ObjectBaseRef<const Element> returnRef;
    // Look for an existing match.
    ObjectBase::asyncLock();
    for (uint32_t ct=0; ct < rsc->mStateElement.mElements.size(); ct++) {
        const Element *ee = rsc->mStateElement.mElements[ct];
        if (ee->getFieldCount() == count) {
            bool match = true;
            for (uint32_t i=0; i < count; i++) {
                size_t len;
                uint32_t asize = 1;
                if (lengths) {
                    len = lengths[i];
                } else {
                    len = strlen(nin[i]);
                }
                if (asin) {
                    asize = asin[i];
                }

                if ((ee->mFields[i].e.get() != ein[i]) ||
                    (strlen(ee->mFields[i].name) != len) ||
                    strcmp(ee->mFields[i].name, nin[i]) ||
                    (ee->mFields[i].arraySize != asize)) {
                    match = false;
                    break;
                }
            }
            if (match) {
                returnRef.set(ee);
                ObjectBase::asyncUnlock();
                return returnRef;
            }
        }
    }
    ObjectBase::asyncUnlock();

    // Element objects must use allocator specified by the driver
    void* allocMem = rsc->mHal.funcs.allocRuntimeMem(sizeof(Element), 0);
    if (!allocMem) {
        rsc->setError(RS_ERROR_FATAL_DRIVER, "Couldn't allocate memory for Element");
        return nullptr;
    }

    Element *e = new (allocMem) Element(rsc);
    returnRef.set(e);
    e->mFields = new ElementField_t [count];
    e->mFieldCount = count;
    for (size_t ct=0; ct < count; ct++) {
        size_t len;
        uint32_t asize = 1;
        if (lengths) {
            len = lengths[ct];
        } else {
            len = strlen(nin[ct]);
        }
        if (asin) {
            asize = asin[ct];
        }

        e->mFields[ct].e.set(ein[ct]);
        e->mFields[ct].name = rsuCopyString(nin[ct], len);
        e->mFields[ct].arraySize = asize;
    }
    e->compute();

    ObjectBase::asyncLock();
    rsc->mStateElement.mElements.push_back(e);
    ObjectBase::asyncUnlock();

    return returnRef;
}

void Element::incRefs(const void *ptr) const {
    if (!mFieldCount) {
        if (mComponent.isReference()) {
            ObjectBase *const*obp = static_cast<ObjectBase *const*>(ptr);
            ObjectBase *ob = obp[0];
            if (ob) ob->incSysRef();
        }
        return;
    }

    const uint8_t *p = static_cast<const uint8_t *>(ptr);
    for (uint32_t i=0; i < mFieldCount; i++) {
        if (mFields[i].e->mHasReference) {
            const uint8_t *p2 = &p[mFields[i].offsetBits >> 3];
            for (uint32_t ct=0; ct < mFields[i].arraySize; ct++) {
                mFields[i].e->incRefs(p2);
                p2 += mFields[i].e->getSizeBytes();
            }
        }
    }
}

void Element::decRefs(const void *ptr) const {
    if (!mFieldCount) {
        if (mComponent.isReference()) {
            ObjectBase *const*obp = static_cast<ObjectBase *const*>(ptr);
            ObjectBase *ob = obp[0];
            if (ob) ob->decSysRef();
        }
        return;
    }

    const uint8_t *p = static_cast<const uint8_t *>(ptr);
    for (uint32_t i=0; i < mFieldCount; i++) {
        if (mFields[i].e->mHasReference) {
            const uint8_t *p2 = &p[mFields[i].offsetBits >> 3];
            for (uint32_t ct=0; ct < mFields[i].arraySize; ct++) {
                mFields[i].e->decRefs(p2);
                p2 += mFields[i].e->getSizeBytes();
            }
        }
    }
}

void Element::callUpdateCacheObject(const Context *rsc, void *dstObj) const {
    if (rsc->mHal.funcs.element.updateCachedObject != nullptr) {
        rsc->mHal.funcs.element.updateCachedObject(rsc, this, (rs_element *)dstObj);
    } else {
        *((const void **)dstObj) = this;
    }
}

ElementState::ElementState() {
}

ElementState::~ElementState() {
    rsAssert(!mElements.size());
}

/////////////////////////////////////////
//

RsElement rsi_ElementCreate(Context *rsc,
                            RsDataType dt,
                            RsDataKind dk,
                            bool norm,
                            uint32_t vecSize) {
    return (RsElement)Element::create(rsc, dt, dk, norm, vecSize);
}


RsElement rsi_ElementCreate2(Context *rsc,
                             const RsElement * ein,
                             size_t ein_length,

                             const char ** names,
                             size_t nameLengths_length,
                             const size_t * nameLengths,

                             const uint32_t * arraySizes,
                             size_t arraySizes_length) {
    return (RsElement)Element::create(rsc, ein_length, (const Element **)ein,
                                      names, nameLengths, arraySizes);
}

} // namespace renderscript
} // namespace android
