/*
 * Copyright (C) 2008-2012 The Android Open Source Project
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

#include "RenderScript.h"
#include "rsCppInternal.h"

using android::RSC::Script;

void Script::invoke(uint32_t slot, const void *v, size_t len) const {
    tryDispatch(mRS, RS::dispatch->ScriptInvokeV(mRS->getContext(), getID(), slot, v, len));
}

void Script::forEach(uint32_t slot, const sp<const Allocation>& ain, const sp<const Allocation>& aout,
                       const void *usr, size_t usrLen) const {
    if ((ain == nullptr) && (aout == nullptr)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "At least one of ain or aout is required to be non-null.");
    }
    void *in_id = BaseObj::getObjID(ain);
    void *out_id = BaseObj::getObjID(aout);
    tryDispatch(mRS, RS::dispatch->ScriptForEach(mRS->getContext(), getID(), slot, in_id, out_id, usr, usrLen, nullptr, 0));
}

Script::Script(void *id, sp<RS> rs) : BaseObj(id, rs) {
}


void Script::bindAllocation(const sp<Allocation>& va, uint32_t slot) const {
    tryDispatch(mRS, RS::dispatch->ScriptBindAllocation(mRS->getContext(), getID(), BaseObj::getObjID(va), slot));
}


void Script::setVar(uint32_t index, const sp<const BaseObj>& o) const {
    tryDispatch(mRS, RS::dispatch->ScriptSetVarObj(mRS->getContext(), getID(), index, (o == nullptr) ? 0 : o->getID()));
}

void Script::setVar(uint32_t index, const void *v, size_t len) const {
    tryDispatch(mRS, RS::dispatch->ScriptSetVarV(mRS->getContext(), getID(), index, v, len));
}

void Script::FieldBase::init(const sp<RS>& rs, uint32_t dimx, uint32_t usages) {
    mAllocation = Allocation::createSized(rs, mElement, dimx, RS_ALLOCATION_USAGE_SCRIPT | usages);
}
