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

#ifndef RSD_CPU_H
#define RSD_CPU_H

#include "rsInternalDefines.h"

typedef const char* (*RSSelectRTCallback) (const char*, size_t);

namespace android {
namespace renderscript {

class Allocation;
class Context;
class Element;
class ObjectBase;
class ScriptC;
class Script;
class ScriptGroupBase;
class ScriptKernelID;

class RsdCpuReference {
public:
    struct CpuSymbol {
        const char * name;
        void * fnPtr;
        bool threadable;
    };

    typedef const CpuSymbol * (* sym_lookup_t)(Context *, const char *name);

    struct CpuTls {
        Context *rsc;
        const ScriptC * sc;
    };

    class CpuScript {
    public:
        virtual void populateScript(Script *) = 0;
        virtual void invokeFunction(uint32_t slot, const void *params, size_t paramLength) = 0;
        virtual int invokeRoot() = 0;

        virtual void invokeForEach(uint32_t slot,
                                   const Allocation ** ains,
                                   uint32_t inLen,
                                   Allocation * aout,
                                   const void * usr,
                                   uint32_t usrLen,
                                   const RsScriptCall *sc) = 0;

        virtual void invokeReduce(uint32_t slot,
                                  const Allocation ** ains, uint32_t inLen,
                                  Allocation *aout,
                                  const RsScriptCall *sc) = 0;

        virtual void invokeInit() = 0;
        virtual void invokeFreeChildren() = 0;

        virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength) = 0;
        virtual void getGlobalVar(uint32_t slot, void *data, size_t dataLength) = 0;
        virtual void setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                      const Element *e, const uint32_t *dims, size_t dimLength) = 0;
        virtual void setGlobalBind(uint32_t slot, Allocation *data) = 0;
        virtual void setGlobalObj(uint32_t slot, ObjectBase *obj) = 0;

        virtual Allocation * getAllocationForPointer(const void *ptr) const = 0;

        // Returns number of global variables in this Script (may be 0 if
        // compiler is not configured to emit this information).
        virtual int getGlobalEntries() const = 0;
        // Returns the name of the global variable at index i.
        virtual const char * getGlobalName(int i) const = 0;
        // Returns the CPU address of the global variable at index i.
        virtual const void * getGlobalAddress(int i) const = 0;
        // Returns the size (in bytes) of the global variable at index i.
        virtual size_t getGlobalSize(int i) const = 0;
        // Returns the properties of the global variable at index i.
        virtual uint32_t getGlobalProperties(int i) const = 0;

        virtual ~CpuScript() {}
    };
    typedef CpuScript * (* script_lookup_t)(Context *, const Script *s);

    class CpuScriptGroupBase {
     public:
      virtual void execute() = 0;
      virtual ~CpuScriptGroupBase() {}
    };

    class CpuScriptGroup : public CpuScriptGroupBase {
    public:
        virtual void setInput(const ScriptKernelID *kid, Allocation *) = 0;
        virtual void setOutput(const ScriptKernelID *kid, Allocation *) = 0;
        ~CpuScriptGroup() override {};
    };

    class CpuScriptGroup2 : public CpuScriptGroupBase {
     public:
      ~CpuScriptGroup2() override {}
    };

    static Context * getTlsContext();
    static const Script * getTlsScript();
    static pthread_key_t getThreadTLSKey();

    static RsdCpuReference * create(Context *c, uint32_t version_major,
                                    uint32_t version_minor, sym_lookup_t lfn, script_lookup_t slfn
                                    , RSSelectRTCallback pSelectRTCallback = nullptr,
                                    const char *pBccPluginName = nullptr
                                    );
    virtual ~RsdCpuReference();
    virtual void setPriority(int32_t priority) = 0;

    virtual CpuScript * createScript(const ScriptC *s, char const *resName, char const *cacheDir,
                                     uint8_t const *bitcode, size_t bitcodeSize,
                                     uint32_t flags) = 0;
    virtual CpuScript * createIntrinsic(const Script *s, RsScriptIntrinsicID iid, Element *e) = 0;
    virtual void* createScriptGroup(const ScriptGroupBase *sg) = 0;
    virtual bool getInKernel() = 0;  // Is a parallel kernel execution underway?

    // Set to true if we should embed global variable information in the code.
    virtual void setEmbedGlobalInfo(bool v) = 0;

    // Returns true if we should embed global variable information in the code.
    virtual bool getEmbedGlobalInfo() const = 0;

    // Set to true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    virtual void setEmbedGlobalInfoSkipConstant(bool v) = 0;

    // Returns true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    virtual bool getEmbedGlobalInfoSkipConstant() const = 0;
};


} // namespace renderscript
} // namespace android

#endif
