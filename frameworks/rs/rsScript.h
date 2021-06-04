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

#ifndef ANDROID_RS_SCRIPT_H
#define ANDROID_RS_SCRIPT_H

#include "rsAllocation.h"
#include "rsMap.h"

#include <utility>

// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
class ProgramVertex;
class ProgramFragment;
class ProgramRaster;
class ProgramStore;
#endif

class IDBase : public ObjectBase {
public:
    IDBase(Context *rsc, Script *s, int slot) :
        ObjectBase(rsc), mScript(s), mSlot(slot) {}
    virtual ~IDBase() {}

    virtual void serialize(Context *rsc, OStream *stream) const {}
    virtual RsA3DClassID getClassId() const = 0;

    Script *mScript;
    int mSlot;
};

class ScriptKernelID : public IDBase {
public:
    ScriptKernelID(Context *rsc, Script *s, int slot, int sig);
    virtual ~ScriptKernelID() {}

    virtual RsA3DClassID getClassId() const;

    bool mHasKernelInput;
    bool mHasKernelOutput;
};

class ScriptInvokeID : public IDBase {
public:
    ScriptInvokeID(Context *rsc, Script *s, int slot);
    virtual ~ScriptInvokeID() {}

    virtual RsA3DClassID getClassId() const;
};

class ScriptFieldID : public IDBase {
public:
    ScriptFieldID(Context *rsc, Script *s, int slot);
    virtual ~ScriptFieldID() {}

    virtual RsA3DClassID getClassId() const;
};

class Script : public ObjectBase {
public:

    struct Hal {
        void * drv;

        struct DriverInfo {
            int mVersionMajor;
            int mVersionMinor;

            size_t exportedVariableCount;
            size_t exportedForEachCount;
            size_t exportedReduceCount;
            size_t exportedFunctionCount;
            size_t exportedPragmaCount;
            char const **exportedPragmaKeyList;
            char const **exportedPragmaValueList;
            const Pair<const char *, uint32_t> *exportedForeachFuncList;

            int (* root)();
        };
        DriverInfo info;
    };
    Hal mHal;

    explicit Script(Context *);
    virtual ~Script();

    struct Enviroment_t {
        int64_t mStartTimeMillis;
        mutable int64_t mLastDtTime;

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        ObjectBaseRef<ProgramVertex> mVertex;
        ObjectBaseRef<ProgramFragment> mFragment;
        ObjectBaseRef<ProgramRaster> mRaster;
        ObjectBaseRef<ProgramStore> mFragmentStore;
#endif
    };
    Enviroment_t mEnviroment;

    void setSlot(uint32_t slot, Allocation *a);
    void setVar(uint32_t slot, const void *val, size_t len);
    void getVar(uint32_t slot, const void *val, size_t len);
    void setVar(uint32_t slot, const void *val, size_t len, Element *e,
                const uint32_t *dims, size_t dimLen);
    void setVarObj(uint32_t slot, ObjectBase *val);

    virtual bool freeChildren();

    virtual void runForEach(Context* rsc,
                            uint32_t slot,
                            const Allocation ** ains,
                            size_t inLen,
                            Allocation* aout,
                            const void* usr,
                            size_t usrBytes,
                            const RsScriptCall *sc = nullptr) = 0;

    virtual void runReduce(Context *rsc, uint32_t slot,
                           const Allocation **ains, size_t inLen,
                           Allocation *aout, const RsScriptCall *sc) = 0;

    virtual void Invoke(Context *rsc, uint32_t slot, const void *data, size_t len) = 0;
    virtual void setupScript(Context *rsc) = 0;
    virtual uint32_t run(Context *) = 0;
    virtual bool isIntrinsic() const { return false; }

    bool hasObjectSlots() const {
        return mHasObjectSlots;
    }
    virtual void callUpdateCacheObject(const Context *rsc, void *dstObj) const;

    uint32_t getApiLevel() const {
        return mApiLevel;
    }

protected:
    bool mInitialized;
    bool mHasObjectSlots;
    uint32_t mApiLevel;
    ObjectBaseRef<Allocation> *mSlots;
    ObjectBaseRef<const Type> *mTypes;

};


} // namespace renderscript
} // namespace android
#endif
