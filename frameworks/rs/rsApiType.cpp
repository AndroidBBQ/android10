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

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#endif

namespace android {
namespace renderscript {

extern "C" void rsaTypeGetNativeData(RsContext con, RsType type, uintptr_t *typeData, uint32_t typeDataSize) {
    rsAssert(typeDataSize == 6);
    // Pack the data in the follofing way mHal.state.dimX; mHal.state.dimY; mHal.state.dimZ;
    // mHal.state.lodCount; mHal.state.faces; mElement; into typeData
    Type *t = static_cast<Type *>(type);

    (*typeData++) = t->getDimX();
    (*typeData++) = t->getDimY();
    (*typeData++) = t->getDimZ();
    (*typeData++) = t->getDimLOD() ? 1 : 0;
    (*typeData++) = t->getDimFaces() ? 1 : 0;
    (*typeData++) = (uintptr_t)t->getElement();
    t->getElement()->incUserRef();
}

} // namespace renderscript
} // namespace android
