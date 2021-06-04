/*
 * Copyright (C) 2016 The Android Open Source Project
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

extern "C" void rsaElementGetNativeData(RsContext con, RsElement elem,
                             uint32_t *elemData, uint32_t elemDataSize) {
    rsAssert(elemDataSize == 5);
    // we will pack mType; mKind; mNormalized; mVectorSize; NumSubElements
    Element *e = static_cast<Element *>(elem);

    (*elemData++) = (uint32_t)e->getType();
    (*elemData++) = (uint32_t)e->getKind();
    (*elemData++) = e->getComponent().getIsNormalized() ? 1 : 0;
    (*elemData++) = e->getComponent().getVectorSize();
    (*elemData++) = e->getFieldCount();
}

extern "C" void rsaElementGetSubElements(RsContext con, RsElement elem, uintptr_t *ids,
                              const char **names, size_t *arraySizes, uint32_t dataSize) {
    Element *e = static_cast<Element *>(elem);
    rsAssert(e->getFieldCount() == dataSize);

    for (uint32_t i = 0; i < dataSize; i ++) {
        e->getField(i)->incUserRef();
        ids[i] = (uintptr_t)e->getField(i);
        names[i] = e->getFieldName(i);
        arraySizes[i] = e->getFieldArraySize(i);
    }
}

} // namespace renderscript
} // namespace android
