/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "rsObjectBase.h"
#include "rsContext.h"
#include "rsDebugHelper.h"

namespace android {
namespace renderscript {

pthread_mutex_t ObjectBase::gObjectInitMutex = PTHREAD_MUTEX_INITIALIZER;

ObjectBase::ObjectBase(Context *rsc) {
    mUserRefCount = 0;
    mSysRefCount = 0;
    mRSC = rsc;
    mNext = nullptr;
    mPrev = nullptr;
    mDH = nullptr;
    mName = nullptr;

    if (gDebugStacks || gDebugReferences || gDebugLeaks) {
        mDH = new DebugHelper();
    }

    rsAssert(rsc);
    add();

    if (gDebugLifetime || gDebugReferences) {
        ALOGV("ObjectBase constructed %p", this);
    }
}

ObjectBase::~ObjectBase() {
    if (gDebugLifetime || gDebugReferences) {
        ALOGV("ObjectBase destroyed %p   refs %i %i", this, mUserRefCount, mSysRefCount);
    }

    if (gDebugStacks || gDebugReferences || gDebugLeaks) {
        if (gDebugStacks || gDebugReferences) {
            mDH->dump();
        }
        delete mDH;
        mDH = nullptr;
    }

    free(const_cast<char *>(mName));

    if (mPrev || mNext) {
        // While the normal practice is to call remove before we call
        // delete.  Its possible for objects without a re-use list
        // for avoiding duplication to be created on the stack.  In those
        // cases we need to remove ourself here.
        asyncLock();
        remove();
        asyncUnlock();
    }

    rsAssert(!mUserRefCount);
    rsAssert(!mSysRefCount);
}

void ObjectBase::dumpLOGV(const char *op) const {
    if (mName) {
        ALOGV("%s RSobj %p, name %s, refs %i,%i  links %p,%p,%p",
             op, this, mName, mUserRefCount, mSysRefCount, mNext, mPrev, mRSC);
    } else {
        ALOGV("%s RSobj %p, no-name, refs %i,%i  links %p,%p,%p",
             op, this, mUserRefCount, mSysRefCount, mNext, mPrev, mRSC);
    }
}

void ObjectBase::incUserRef() const {
    __sync_fetch_and_add(&mUserRefCount, 1);
    if (gDebugReferences) {
        ALOGV("ObjectBase %p incU ref %i, %i", this, mUserRefCount, mSysRefCount);
    }
}

void ObjectBase::incSysRef() const {
    __sync_fetch_and_add(&mSysRefCount, 1);
    if (gDebugReferences) {
        ALOGV("ObjectBase %p incS ref %i, %i", this, mUserRefCount, mSysRefCount);
    }
}

void ObjectBase::preDestroy() const {
}

bool ObjectBase::freeChildren() {
    return false;
}

bool ObjectBase::checkDelete(const ObjectBase *ref) {
    if (!ref) {
        return false;
    }

    asyncLock();
    // This lock protects us against the non-RS threads changing
    // the ref counts.  At this point we should be the only thread
    // working on them.
    if (ref->mUserRefCount || ref->mSysRefCount) {
        asyncUnlock();
        return false;
    }

    ref->remove();
    // At this point we can unlock because there should be no possible way
    // for another thread to reference this object.
    ref->preDestroy();
    asyncUnlock();
    delete ref;
    return true;
}

bool ObjectBase::decUserRef() const {
    rsAssert(mUserRefCount > 0);
    if (gDebugReferences) {
        ALOGV("ObjectBase %p decU ref %i, %i", this, mUserRefCount, mSysRefCount);
        if (mUserRefCount <= 0) {
            mDH->dump();
        }
    }


    if ((__sync_fetch_and_sub(&mUserRefCount, 1) <= 1)) {
        __sync_synchronize();
        if (mSysRefCount <= 0) {
            return checkDelete(this);
        }
    }
    return false;
}

bool ObjectBase::zeroUserRef() const {
    if (gDebugReferences) {
        ALOGV("ObjectBase %p zeroU ref %i, %i", this, mUserRefCount, mSysRefCount);
    }

    __sync_and_and_fetch(&mUserRefCount, 0);
    if (mSysRefCount <= 0) {
        return checkDelete(this);
    }
    return false;
}

bool ObjectBase::decSysRef() const {
    if (gDebugReferences) {
        ALOGV("ObjectBase %p decS ref %i, %i", this, mUserRefCount, mSysRefCount);
    }

    rsAssert(mSysRefCount > 0);
    if ((__sync_fetch_and_sub(&mSysRefCount, 1) <= 1)) {
        __sync_synchronize();
        if (mUserRefCount <= 0) {
            return checkDelete(this);
        }
    }
    return false;
}

void ObjectBase::setName(const char *name) {
    mName = strdup(name);
}

void ObjectBase::setName(const char *name, uint32_t len) {
    char *c = (char*)calloc(len + 1, sizeof(char));
    rsAssert(c);
    memcpy(c, name, len);
    mName = c;
}

void ObjectBase::asyncLock() {
    pthread_mutex_lock(&gObjectInitMutex);
}

void ObjectBase::asyncUnlock() {
    pthread_mutex_unlock(&gObjectInitMutex);
}

void ObjectBase::add() const {
    asyncLock();

    rsAssert(!mNext);
    rsAssert(!mPrev);
    mNext = mRSC->mObjHead;
    if (mRSC->mObjHead) {
        mRSC->mObjHead->mPrev = this;
    }
    mRSC->mObjHead = this;

    asyncUnlock();
}

void ObjectBase::remove() const {
    if (!mRSC) {
        rsAssert(!mPrev);
        rsAssert(!mNext);
        return;
    }

    if (mRSC->mObjHead == this) {
        mRSC->mObjHead = mNext;
    }
    if (mPrev) {
        mPrev->mNext = mNext;
    }
    if (mNext) {
        mNext->mPrev = mPrev;
    }
    mPrev = nullptr;
    mNext = nullptr;
}

void ObjectBase::zeroAllUserRef(Context *rsc) {
    if (gDebugReferences || gDebugLeaks) {
        ALOGV("Forcing release of all outstanding user refs.");
    }

    // This operation can be slow, only to be called during context cleanup.
    const ObjectBase * o = rsc->mObjHead;
    while (o) {
        //ALOGE("o %p", o);
        if (o->zeroUserRef()) {
#ifdef __clang_analyzer__
            // Assure the static analyzer that we updated mObjHead. Otherwise,
            // it complains about a use-after-free here. Needed for b/27101951.
            if (o == rsc->mObjHead) {
                abort();
            }
#endif
            // deleted the object and possibly others, restart from head.
            o = rsc->mObjHead;
            //ALOGE("o head %p", o);
        } else {
            o = o->mNext;
            //ALOGE("o next %p", o);
        }
    }

    if (gDebugReferences || gDebugLeaks) {
        ALOGV("Objects remaining.");
        dumpAll(rsc);
    }
}

void ObjectBase::freeAllChildren(Context *rsc) {
    if (gDebugReferences) {
        ALOGV("Forcing release of all child objects.");
    }

    // This operation can be slow, only to be called during context cleanup.
    ObjectBase * o = (ObjectBase *)rsc->mObjHead;
    while (o) {
        if (o->freeChildren()) {
            // deleted ref to self and possibly others, restart from head.
            o = (ObjectBase *)rsc->mObjHead;
        } else {
            o = (ObjectBase *)o->mNext;
        }
    }

    if (gDebugReferences) {
        ALOGV("Objects remaining.");
        dumpAll(rsc);
    }
}

void ObjectBase::dumpAll(Context *rsc) {
    asyncLock();

    ALOGV("Dumping all objects");
    const ObjectBase * o = rsc->mObjHead;
    while (o) {
        ALOGV(" Object %p", o);
        o->dumpLOGV("  ");
        if (o->mDH != nullptr) {
            o->mDH->dump();
        }
        o = o->mNext;
    }

    asyncUnlock();
}

bool ObjectBase::isValid(const Context *rsc, const ObjectBase *obj) {
    asyncLock();

    const ObjectBase * o = rsc->mObjHead;
    while (o) {
        if (o == obj) {
            asyncUnlock();
            return true;
        }
        o = o->mNext;
    }
    asyncUnlock();
    return false;
}

void ObjectBase::callUpdateCacheObject(const Context *rsc, void *dstObj) const {
    //ALOGE("ObjectBase::callUpdateCacheObject %p  %p", this, dstObj);
    *((const void **)dstObj) = this;
}

} // namespace renderscript
} // namespace android
