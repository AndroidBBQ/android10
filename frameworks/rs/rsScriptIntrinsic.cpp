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

#include "rsContext.h"
#include "rsScriptIntrinsic.h"
#include <time.h>

namespace android {
namespace renderscript {

ScriptIntrinsic::ScriptIntrinsic(Context *rsc) : Script(rsc) {
    mIntrinsicID = 0;
}

ScriptIntrinsic::~ScriptIntrinsic() {
    if (mIntrinsicID != 0) {
        mRSC->mHal.funcs.script.destroy(mRSC, this);
    }
}

bool ScriptIntrinsic::init(Context *rsc, RsScriptIntrinsicID iid, Element *e) {
    mIntrinsicID = iid;
    mElement.set(e);
    mSlots = new ObjectBaseRef<Allocation>[2];
    mTypes = new ObjectBaseRef<const Type>[2];

    rsc->mHal.funcs.script.initIntrinsic(rsc, this, iid, e);


    return true;
}

bool ScriptIntrinsic::freeChildren() {
    return false;
}

void ScriptIntrinsic::setupScript(Context *rsc) {
}

uint32_t ScriptIntrinsic::run(Context *rsc) {
    rsAssert(!"ScriptIntrinsic::run - should not happen");
    return 0;
}

void ScriptIntrinsic::runForEach(Context* rsc,
                         uint32_t slot,
                         const Allocation** ains,
                         size_t inLen,
                         Allocation* aout,
                         const void* usr,
                         size_t usrBytes,
                         const RsScriptCall* sc) {

    rsc->mHal.funcs.script.invokeForEachMulti(rsc, this, slot, ains, inLen,
                                              aout, usr, usrBytes, sc);
}

void ScriptIntrinsic::runReduce(Context *rsc, uint32_t slot,
                                const Allocation ** ains, size_t inLen,
                                Allocation *aout, const RsScriptCall *sc) {
}

void ScriptIntrinsic::Invoke(Context *rsc, uint32_t slot, const void *data, size_t len) {
}

void ScriptIntrinsic::serialize(Context *rsc, OStream *stream) const {
}

RsA3DClassID ScriptIntrinsic::getClassId() const {
    return (RsA3DClassID)0;
}

RsScript rsi_ScriptIntrinsicCreate(Context *rsc, uint32_t id, RsElement ve) {
    ScriptIntrinsic *si = new ScriptIntrinsic(rsc);
    if (!si->init(rsc, (RsScriptIntrinsicID)id, (Element *)ve)) {
        delete si;
        return nullptr;
    }
    si->incUserRef();
    return si;
}

} // namespace renderscript
} // namespace android
