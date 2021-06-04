/*
 * Copyright (C) 2009-2012 The Android Open Source Project
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
#include <time.h>

namespace android {
namespace renderscript {

Script::Script(Context *rsc) : ObjectBase(rsc) {
    memset(&mEnviroment, 0, sizeof(mEnviroment));
    memset(&mHal, 0, sizeof(mHal));

    mSlots = nullptr;
    mTypes = nullptr;
    mInitialized = false;
    mHasObjectSlots = false;
    mApiLevel = 0;
}

Script::~Script() {
    if (mSlots) {
        delete [] mSlots;
        mSlots = nullptr;
    }
    if (mTypes) {
        delete [] mTypes;
        mTypes = nullptr;
    }
}

void Script::setSlot(uint32_t slot, Allocation *a) {
    //ALOGE("setSlot %i %p", slot, a);
    if (slot >= mHal.info.exportedVariableCount) {
        ALOGE("Script::setSlot unable to set allocation, invalid slot index");
        return;
    }

    if (mRSC->hadFatalError()) return;

    mSlots[slot].set(a);
    mHasObjectSlots = true;
    mRSC->mHal.funcs.script.setGlobalBind(mRSC, this, slot, a);
}

void Script::setVar(uint32_t slot, const void *val, size_t len) {
    //ALOGE("setVar %i %p %i", slot, val, len);
    if (slot >= mHal.info.exportedVariableCount) {
        ALOGE("Script::setVar unable to set allocation, invalid slot index");
        return;
    }
    if (mRSC->hadFatalError()) return;

    mRSC->mHal.funcs.script.setGlobalVar(mRSC, this, slot, (void *)val, len);
}

void Script::getVar(uint32_t slot, const void *val, size_t len) {
    //ALOGE("getVar %i %p %i", slot, val, len);
    if (slot >= mHal.info.exportedVariableCount) {
        ALOGE("Script::getVar unable to set allocation, invalid slot index: "
              "%u >= %zu", slot, mHal.info.exportedVariableCount);
        return;
    }
    if (mRSC->hadFatalError()) return;

    mRSC->mHal.funcs.script.getGlobalVar(mRSC, this, slot, (void *)val, len);
}

void Script::setVar(uint32_t slot, const void *val, size_t len, Element *e,
                    const uint32_t *dims, size_t dimLen) {
    if (slot >= mHal.info.exportedVariableCount) {
        ALOGE("Script::setVar unable to set allocation, invalid slot index: "
              "%u >= %zu", slot, mHal.info.exportedVariableCount);
        return;
    }
    if (mRSC->hadFatalError()) return;

    mRSC->mHal.funcs.script.setGlobalVarWithElemDims(mRSC, this, slot,
            (void *)val, len, e, dims, dimLen);
}

void Script::setVarObj(uint32_t slot, ObjectBase *val) {
    //ALOGE("setVarObj %i %p", slot, val);
    if (slot >= mHal.info.exportedVariableCount) {
        ALOGE("Script::setVarObj unable to set allocation, invalid slot index: "
              "%u >= %zu", slot, mHal.info.exportedVariableCount);
        return;
    }
    if (mRSC->hadFatalError()) return;

    mHasObjectSlots = true;
    mRSC->mHal.funcs.script.setGlobalObj(mRSC, this, slot, val);
}

void Script::callUpdateCacheObject(const Context *rsc, void *dstObj) const {
    if (rsc->mHal.funcs.script.updateCachedObject != nullptr) {
        rsc->mHal.funcs.script.updateCachedObject(rsc, this, (rs_script *)dstObj);
    } else {
        *((const void **)dstObj) = this;
    }
}

bool Script::freeChildren() {
    incSysRef();
    mRSC->mHal.funcs.script.invokeFreeChildren(mRSC, this);
    return decSysRef();
}

ScriptKernelID::ScriptKernelID(Context *rsc, Script *s, int slot, int sig)
        : IDBase(rsc, s, slot) {
    mHasKernelInput = (sig & 1) != 0;
    mHasKernelOutput = (sig & 2) != 0;
}

RsA3DClassID ScriptKernelID::getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_KERNEL_ID;
}

ScriptInvokeID::ScriptInvokeID(Context *rsc, Script *s, int slot)
    : IDBase(rsc, s, slot) {
}

RsA3DClassID ScriptInvokeID::getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_INVOKE_ID;
}

ScriptFieldID::ScriptFieldID(Context *rsc, Script *s, int slot) :
    IDBase(rsc, s, slot) {
}

RsA3DClassID ScriptFieldID::getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_FIELD_ID;
}


RsScriptKernelID rsi_ScriptKernelIDCreate(Context *rsc, RsScript vs, int slot, int sig) {
    ScriptKernelID *kid = new ScriptKernelID(rsc, (Script *)vs, slot, sig);
    kid->incUserRef();
    return kid;
}

RsScriptInvokeID rsi_ScriptInvokeIDCreate(Context *rsc, RsScript vs, uint32_t slot) {
    ScriptInvokeID *iid = new ScriptInvokeID(rsc, (Script *)vs, slot);
    iid->incUserRef();
    return iid;
}

RsScriptFieldID rsi_ScriptFieldIDCreate(Context *rsc, RsScript vs, int slot) {
    ScriptFieldID *fid = new ScriptFieldID(rsc, (Script *)vs, slot);
    fid->incUserRef();
    return fid;
}

void rsi_ScriptBindAllocation(Context * rsc, RsScript vs, RsAllocation va, uint32_t slot) {
    Script *s = static_cast<Script *>(vs);
    Allocation *a = static_cast<Allocation *>(va);
    s->setSlot(slot, a);
}

void rsi_ScriptSetTimeZone(Context * rsc, RsScript vs, const char * timeZone, size_t length) {
    // We unfortunately need to make a new copy of the string, since it is
    // not nullptr-terminated. We then use setenv(), which properly handles
    // freeing/duplicating the actual string for the environment.
    char *tz = (char *) malloc(length + 1);
    if (!tz) {
        ALOGE("Couldn't allocate memory for timezone buffer");
        return;
    }
    strncpy(tz, timeZone, length);
    tz[length] = '\0';
    if (setenv("TZ", tz, 1) == 0) {
        tzset();
    } else {
        ALOGE("Error setting timezone");
    }
    free(tz);
}

void rsi_ScriptForEachMulti(Context *rsc, RsScript vs, uint32_t slot,
                            RsAllocation *vains, size_t inLen,
                            RsAllocation vaout, const void *params,
                            size_t paramLen, const RsScriptCall *sc,
                            size_t scLen) {

    Script      *s    = static_cast<Script *>(vs);
    Allocation **ains = (Allocation**)(vains);

    s->runForEach(rsc, slot,
                  const_cast<const Allocation **>(ains), inLen,
                  static_cast<Allocation *>(vaout), params, paramLen, sc);

}

void rsi_ScriptForEach(Context *rsc, RsScript vs, uint32_t slot,
                       RsAllocation vain, RsAllocation vaout,
                       const void *params, size_t paramLen,
                       const RsScriptCall *sc, size_t scLen) {

    if (vain == nullptr) {
        rsi_ScriptForEachMulti(rsc, vs, slot, nullptr, 0, vaout, params, paramLen,
                               sc, scLen);
    } else {
        RsAllocation ains[1] = {vain};

        rsi_ScriptForEachMulti(rsc, vs, slot, ains,
                               sizeof(ains) / sizeof(RsAllocation), vaout,
                               params, paramLen, sc, scLen);
    }
}

void rsi_ScriptReduce(Context *rsc, RsScript vs, uint32_t slot,
                      RsAllocation *vains, size_t inLen,
                      RsAllocation vaout, const RsScriptCall *sc,
                      size_t scLen) {
  Script *s = static_cast<Script *>(vs);
  Allocation **ains = (Allocation**)(vains);

  s->runReduce(rsc, slot,
               const_cast<const Allocation **>(ains), inLen,
               static_cast<Allocation *>(vaout), sc);
}

void rsi_ScriptInvoke(Context *rsc, RsScript vs, uint32_t slot) {
    Script *s = static_cast<Script *>(vs);
    s->Invoke(rsc, slot, nullptr, 0);
}


void rsi_ScriptInvokeData(Context *rsc, RsScript vs, uint32_t slot, void *data) {
    Script *s = static_cast<Script *>(vs);
    s->Invoke(rsc, slot, nullptr, 0);
}

void rsi_ScriptInvokeV(Context *rsc, RsScript vs, uint32_t slot, const void *data, size_t len) {
    Script *s = static_cast<Script *>(vs);
    s->Invoke(rsc, slot, data, len);
}

void rsi_ScriptSetVarI(Context *rsc, RsScript vs, uint32_t slot, int value) {
    Script *s = static_cast<Script *>(vs);
    s->setVar(slot, &value, sizeof(value));
}

void rsi_ScriptSetVarObj(Context *rsc, RsScript vs, uint32_t slot, RsObjectBase value) {
    Script *s = static_cast<Script *>(vs);
    ObjectBase *o = static_cast<ObjectBase *>(value);
    s->setVarObj(slot, o);
}

void rsi_ScriptSetVarJ(Context *rsc, RsScript vs, uint32_t slot, int64_t value) {
    Script *s = static_cast<Script *>(vs);
    s->setVar(slot, &value, sizeof(value));
}

void rsi_ScriptSetVarF(Context *rsc, RsScript vs, uint32_t slot, float value) {
    Script *s = static_cast<Script *>(vs);
    s->setVar(slot, &value, sizeof(value));
}

void rsi_ScriptSetVarD(Context *rsc, RsScript vs, uint32_t slot, double value) {
    Script *s = static_cast<Script *>(vs);
    s->setVar(slot, &value, sizeof(value));
}

void rsi_ScriptSetVarV(Context *rsc, RsScript vs, uint32_t slot, const void *data, size_t len) {
    Script *s = static_cast<Script *>(vs);
    s->setVar(slot, data, len);
}

void rsi_ScriptGetVarV(Context *rsc, RsScript vs, uint32_t slot, void *data, size_t len) {
    Script *s = static_cast<Script *>(vs);
    s->getVar(slot, data, len);
}

void rsi_ScriptSetVarVE(Context *rsc, RsScript vs, uint32_t slot,
                        const void *data, size_t len, RsElement ve,
                        const uint32_t *dims, size_t dimLen) {
    Script *s = static_cast<Script *>(vs);
    Element *e = static_cast<Element *>(ve);
    s->setVar(slot, data, len, e, dims, dimLen);
}

} // namespace renderscript
} // namespace android
