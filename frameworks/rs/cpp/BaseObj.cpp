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

#include "RenderScript.h"
#include "rsCppInternal.h"

using android::RSC::BaseObj;

void * BaseObj::getID() const {
    if (mID == nullptr) {
        ALOGE("Internal error: Object id 0.");
    }
    return mID;
}

void * BaseObj::getObjID(const sp<const BaseObj>& o) {
    return o == nullptr ? nullptr : o->getID();
}


BaseObj::BaseObj(void *id, sp<RS> rs) {
    mRS = rs.get();
    mID = id;
}

void BaseObj::checkValid() {
    if (mID == 0) {
        ALOGE("Invalid object.");
    }
}

BaseObj::~BaseObj() {
    if (mRS && mRS->getContext()) {
        RS::dispatch->ObjDestroy(mRS->getContext(), mID);
    }
    mRS = nullptr;
    mID = nullptr;
}

void BaseObj::updateFromNative() {
    const char *name = nullptr;
    RS::dispatch->GetName(mRS->getContext(), mID, &name);
    mName = name;
}

bool BaseObj::equals(const sp<const BaseObj>& obj) {
    // Early-out check to see if both BaseObjs are actually the same.
    if (this == obj.get())
        return true;
    return mID == obj->mID;
}
