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

#ifndef ANDROID_RS_SCRIPT_INTRINSIC_H
#define ANDROID_RS_SCRIPT_INTRINSIC_H

#include "rsScript.h"


// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {


class ScriptIntrinsic : public Script {
public:

    ObjectBaseRef<const Element> mElement;

    explicit ScriptIntrinsic(Context *);
    ~ScriptIntrinsic() override;

    bool init(Context *rsc, RsScriptIntrinsicID iid, Element *e);


    void serialize(Context *rsc, OStream *stream) const override;
    RsA3DClassID getClassId() const override;
    bool freeChildren() override;

    void runForEach(Context* rsc,
                    uint32_t slot,
                    const Allocation ** ains,
                    size_t inLen,
                    Allocation* aout,
                    const void* usr,
                    size_t usrBytes,
                    const RsScriptCall* sc = nullptr) override;

    void runReduce(Context *rsc, uint32_t slot,
                   const Allocation ** ains, size_t inLen,
                   Allocation *aout, const RsScriptCall *sc) override;

    void Invoke(Context *rsc, uint32_t slot, const void *data, size_t len) override;
    void setupScript(Context *rsc) override;
    uint32_t run(Context *) override;
    bool isIntrinsic() const override { return true; }
protected:
    uint32_t mIntrinsicID;

};


} // namespace renderscript
} // namespace android
#endif
