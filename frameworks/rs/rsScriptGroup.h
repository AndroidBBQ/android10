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

#ifndef ANDROID_RS_SCRIPT_GROUP_H
#define ANDROID_RS_SCRIPT_GROUP_H

#include "rsScriptGroupBase.h"

#include <vector>

// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {

class Allocation;
class Context;
class ProgramVertex;
class ProgramFragment;
class ProgramRaster;
class ProgramStore;
class Script;
class ScriptFieldID;
class ScriptKernelID;
class Type;

class ScriptGroup : public ScriptGroupBase {
public:
    virtual SG_API_Version getApiVersion() const { return SG_V1; }
    virtual void execute(Context *rsc);

    std::vector<ObjectBaseRef<ScriptKernelID> > mKernels;

    class Link {
    public:
        ObjectBaseRef<const ScriptKernelID> mSource;
        ObjectBaseRef<const ScriptKernelID> mDstKernel;
        ObjectBaseRef<const ScriptFieldID> mDstField;
        ObjectBaseRef<const Type> mType;
        ObjectBaseRef<Allocation> mAlloc;
        Link();
        ~Link();
    };

    class Node {
    public:
        explicit Node(Script *);

        std::vector<const ScriptKernelID *> mKernels;
        std::vector<Link *> mOutputs;
        std::vector<Link *> mInputs;
        bool mSeen;
        int mOrder;
        Script *mScript;
    };

    class IO {
    public:
        explicit IO(const ScriptKernelID *);

        const ScriptKernelID *mKernel;
        ObjectBaseRef<Allocation> mAlloc;
    };

    std::vector<Link *> mLinks;
    std::vector<Node *> mNodes;
    std::vector<IO *> mInputs;
    std::vector<IO *> mOutputs;

    static ScriptGroup * create(Context *rsc,
                           ScriptKernelID ** kernels, size_t kernelsSize,
                           ScriptKernelID ** src, size_t srcSize,
                           ScriptKernelID ** dstK, size_t dstKSize,
                           ScriptFieldID ** dstF, size_t dstFSize,
                           const Type ** type, size_t typeSize);

    void setInput(Context *rsc, ScriptKernelID *kid, Allocation *a);
    void setOutput(Context *rsc, ScriptKernelID *kid, Allocation *a);

protected:
    virtual ~ScriptGroup();
    bool mInitialized;


private:
    bool calcOrderRecurse(Node *n, int depth);
    bool calcOrder();
    Node * findNode(Script *s) const;
    // Check if input/output Allocations are correctly set for a ScriptGroup.
    // Send any error back to the client (app). Called before the ScriptGroup
    // executes. Skips the exeuction if validation fails.
    bool validateInputAndOutput(Context *);

    explicit ScriptGroup(Context *);
};


} // namespace renderscript
} // namespace android
#endif

