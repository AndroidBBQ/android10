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
#include "rsMesh.h"

using android::renderscript::Mesh;

void rsaMeshGetVertexBufferCount(RsContext con, RsMesh mv, int32_t *numVtx) {
    Mesh *sm = static_cast<Mesh *>(mv);
    *numVtx = sm->mHal.state.vertexBuffersCount;
}

void rsaMeshGetIndexCount(RsContext con, RsMesh mv, int32_t *numIdx) {
    Mesh *sm = static_cast<Mesh *>(mv);
    *numIdx = sm->mHal.state.primitivesCount;
}

void rsaMeshGetVertices(RsContext con, RsMesh mv, RsAllocation *vtxData, uint32_t vtxDataCount) {
    Mesh *sm = static_cast<Mesh *>(mv);
    rsAssert(vtxDataCount == sm->mHal.state.vertexBuffersCount);

    for (uint32_t ct = 0; ct < vtxDataCount; ct ++) {
        vtxData[ct] = sm->mHal.state.vertexBuffers[ct];
        sm->mHal.state.vertexBuffers[ct]->incUserRef();
    }
}

void rsaMeshGetIndices(RsContext con, RsMesh mv, RsAllocation *va, uint32_t *primType, uint32_t idxDataCount) {
    Mesh *sm = static_cast<Mesh *>(mv);
    rsAssert(idxDataCount == sm->mHal.state.primitivesCount);

    for (uint32_t ct = 0; ct < idxDataCount; ct ++) {
        va[ct] = sm->mHal.state.indexBuffers[ct];
        primType[ct] = sm->mHal.state.primitives[ct];
        if (sm->mHal.state.indexBuffers[ct]) {
            sm->mHal.state.indexBuffers[ct]->incUserRef();
        }
    }
}
