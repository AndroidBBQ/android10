/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <android/binder_ibinder.h>
#include "ibinder_internal.h"

#include <android/binder_status.h>
#include "parcel_internal.h"
#include "status_internal.h"

#include <android-base/logging.h>
#include <binder/IPCThreadState.h>

using DeathRecipient = ::android::IBinder::DeathRecipient;

using ::android::IBinder;
using ::android::Parcel;
using ::android::sp;
using ::android::status_t;
using ::android::String16;
using ::android::String8;
using ::android::wp;

namespace ABBinderTag {

static const void* kId = "ABBinder";
static void* kValue = static_cast<void*>(new bool{true});
void clean(const void* /*id*/, void* /*obj*/, void* /*cookie*/){/* do nothing */};

static void attach(const sp<IBinder>& binder) {
    binder->attachObject(kId, kValue, nullptr /*cookie*/, clean);
}
static bool has(const sp<IBinder>& binder) {
    return binder != nullptr && binder->findObject(kId) == kValue;
}

}  // namespace ABBinderTag

namespace ABpBinderTag {

static std::mutex gLock;
static const void* kId = "ABpBinder";
struct Value {
    wp<ABpBinder> binder;
};
void clean(const void* id, void* obj, void* cookie) {
    CHECK(id == kId) << id << " " << obj << " " << cookie;

    delete static_cast<Value*>(obj);
};

}  // namespace ABpBinderTag

AIBinder::AIBinder(const AIBinder_Class* clazz) : mClazz(clazz) {}
AIBinder::~AIBinder() {}

bool AIBinder::associateClass(const AIBinder_Class* clazz) {
    if (clazz == nullptr) return false;
    if (mClazz == clazz) return true;

    String8 newDescriptor(clazz->getInterfaceDescriptor());

    if (mClazz != nullptr) {
        String8 currentDescriptor(mClazz->getInterfaceDescriptor());
        if (newDescriptor == currentDescriptor) {
            LOG(ERROR) << __func__ << ": Class descriptors '" << currentDescriptor
                       << "' match during associateClass, but they are different class objects. "
                          "Class descriptor collision?";
        } else {
            LOG(ERROR) << __func__
                       << ": Class cannot be associated on object which already has a class. "
                          "Trying to associate to '"
                       << newDescriptor.c_str() << "' but already set to '"
                       << currentDescriptor.c_str() << "'.";
        }

        // always a failure because we know mClazz != clazz
        return false;
    }

    CHECK(asABpBinder() != nullptr);  // ABBinder always has a descriptor

    String8 descriptor(getBinder()->getInterfaceDescriptor());
    if (descriptor != newDescriptor) {
        LOG(ERROR) << __func__ << ": Expecting binder to have class '" << newDescriptor.c_str()
                   << "' but descriptor is actually '" << descriptor.c_str() << "'.";
        return false;
    }

    // if this is a local object, it's not one known to libbinder_ndk
    mClazz = clazz;

    return true;
}

ABBinder::ABBinder(const AIBinder_Class* clazz, void* userData)
    : AIBinder(clazz), BBinder(), mUserData(userData) {
    CHECK(clazz != nullptr);
}
ABBinder::~ABBinder() {
    getClass()->onDestroy(mUserData);
}

const String16& ABBinder::getInterfaceDescriptor() const {
    return getClass()->getInterfaceDescriptor();
}

status_t ABBinder::dump(int fd, const ::android::Vector<String16>& args) {
    AIBinder_onDump onDump = getClass()->onDump;

    if (onDump == nullptr) {
        return STATUS_OK;
    }

    // technically UINT32_MAX would be okay here, but INT32_MAX is expected since this may be
    // null in Java
    if (args.size() > INT32_MAX) {
        LOG(ERROR) << "ABBinder::dump received too many arguments: " << args.size();
        return STATUS_BAD_VALUE;
    }

    std::vector<String8> utf8Args;  // owns memory of utf8s
    utf8Args.reserve(args.size());
    std::vector<const char*> utf8Pointers;  // what can be passed over NDK API
    utf8Pointers.reserve(args.size());

    for (size_t i = 0; i < args.size(); i++) {
        utf8Args.push_back(String8(args[i]));
        utf8Pointers.push_back(utf8Args[i].c_str());
    }

    return onDump(this, fd, utf8Pointers.data(), utf8Pointers.size());
}

status_t ABBinder::onTransact(transaction_code_t code, const Parcel& data, Parcel* reply,
                              binder_flags_t flags) {
    if (isUserCommand(code)) {
        if (!data.checkInterface(this)) {
            return STATUS_BAD_TYPE;
        }

        const AParcel in = AParcel::readOnly(this, &data);
        AParcel out = AParcel(this, reply, false /*owns*/);

        binder_status_t status = getClass()->onTransact(this, code, &in, &out);
        return PruneStatusT(status);
    } else {
        return BBinder::onTransact(code, data, reply, flags);
    }
}

ABpBinder::ABpBinder(const ::android::sp<::android::IBinder>& binder)
    : AIBinder(nullptr /*clazz*/), BpRefBase(binder) {
    CHECK(binder != nullptr);
}
ABpBinder::~ABpBinder() {}

void ABpBinder::onLastStrongRef(const void* id) {
    {
        std::lock_guard<std::mutex> lock(ABpBinderTag::gLock);
        // Since ABpBinder is OBJECT_LIFETIME_WEAK, we must remove this weak reference in order for
        // the ABpBinder to be deleted. Since a strong reference to this ABpBinder object should no
        // longer be able to exist at the time of this method call, there is no longer a need to
        // recover it.

        ABpBinderTag::Value* value =
                static_cast<ABpBinderTag::Value*>(remote()->findObject(ABpBinderTag::kId));
        if (value != nullptr) {
            value->binder = nullptr;
        }
    }

    BpRefBase::onLastStrongRef(id);
}

sp<AIBinder> ABpBinder::lookupOrCreateFromBinder(const ::android::sp<::android::IBinder>& binder) {
    if (binder == nullptr) {
        return nullptr;
    }
    if (ABBinderTag::has(binder)) {
        return static_cast<ABBinder*>(binder.get());
    }

    // The following code ensures that for a given binder object (remote or local), if it is not an
    // ABBinder then at most one ABpBinder object exists in a given process representing it.
    std::lock_guard<std::mutex> lock(ABpBinderTag::gLock);

    ABpBinderTag::Value* value =
            static_cast<ABpBinderTag::Value*>(binder->findObject(ABpBinderTag::kId));
    if (value == nullptr) {
        value = new ABpBinderTag::Value;
        binder->attachObject(ABpBinderTag::kId, static_cast<void*>(value), nullptr /*cookie*/,
                             ABpBinderTag::clean);
    }

    sp<ABpBinder> ret = value->binder.promote();
    if (ret == nullptr) {
        ret = new ABpBinder(binder);
        value->binder = ret;
    }

    return ret;
}

struct AIBinder_Weak {
    wp<AIBinder> binder;
};
AIBinder_Weak* AIBinder_Weak_new(AIBinder* binder) {
    if (binder == nullptr) {
        return nullptr;
    }

    return new AIBinder_Weak{wp<AIBinder>(binder)};
}
void AIBinder_Weak_delete(AIBinder_Weak* weakBinder) {
    delete weakBinder;
}
AIBinder* AIBinder_Weak_promote(AIBinder_Weak* weakBinder) {
    if (weakBinder == nullptr) {
        return nullptr;
    }

    sp<AIBinder> binder = weakBinder->binder.promote();
    AIBinder_incStrong(binder.get());
    return binder.get();
}

AIBinder_Class::AIBinder_Class(const char* interfaceDescriptor, AIBinder_Class_onCreate onCreate,
                               AIBinder_Class_onDestroy onDestroy,
                               AIBinder_Class_onTransact onTransact)
    : onCreate(onCreate),
      onDestroy(onDestroy),
      onTransact(onTransact),
      mInterfaceDescriptor(interfaceDescriptor) {}

AIBinder_Class* AIBinder_Class_define(const char* interfaceDescriptor,
                                      AIBinder_Class_onCreate onCreate,
                                      AIBinder_Class_onDestroy onDestroy,
                                      AIBinder_Class_onTransact onTransact) {
    if (interfaceDescriptor == nullptr || onCreate == nullptr || onDestroy == nullptr ||
        onTransact == nullptr) {
        return nullptr;
    }

    return new AIBinder_Class(interfaceDescriptor, onCreate, onDestroy, onTransact);
}

void AIBinder_Class_setOnDump(AIBinder_Class* clazz, AIBinder_onDump onDump) {
    CHECK(clazz != nullptr) << "setOnDump requires non-null clazz";

    // this is required to be called before instances are instantiated
    clazz->onDump = onDump;
}

void AIBinder_DeathRecipient::TransferDeathRecipient::binderDied(const wp<IBinder>& who) {
    CHECK(who == mWho);

    mOnDied(mCookie);

    sp<AIBinder_DeathRecipient> recipient = mParentRecipient.promote();
    sp<IBinder> strongWho = who.promote();

    // otherwise this will be cleaned up later with pruneDeadTransferEntriesLocked
    if (recipient != nullptr && strongWho != nullptr) {
        status_t result = recipient->unlinkToDeath(strongWho, mCookie);
        if (result != ::android::DEAD_OBJECT) {
            LOG(WARNING) << "Unlinking to dead binder resulted in: " << result;
        }
    }

    mWho = nullptr;
}

AIBinder_DeathRecipient::AIBinder_DeathRecipient(AIBinder_DeathRecipient_onBinderDied onDied)
    : mOnDied(onDied) {
    CHECK(onDied != nullptr);
}

void AIBinder_DeathRecipient::pruneDeadTransferEntriesLocked() {
    mDeathRecipients.erase(std::remove_if(mDeathRecipients.begin(), mDeathRecipients.end(),
                                          [](const sp<TransferDeathRecipient>& tdr) {
                                              return tdr->getWho() == nullptr;
                                          }),
                           mDeathRecipients.end());
}

binder_status_t AIBinder_DeathRecipient::linkToDeath(sp<IBinder> binder, void* cookie) {
    CHECK(binder != nullptr);

    std::lock_guard<std::mutex> l(mDeathRecipientsMutex);

    sp<TransferDeathRecipient> recipient =
            new TransferDeathRecipient(binder, cookie, this, mOnDied);

    status_t status = binder->linkToDeath(recipient, cookie, 0 /*flags*/);
    if (status != STATUS_OK) {
        return PruneStatusT(status);
    }

    mDeathRecipients.push_back(recipient);

    pruneDeadTransferEntriesLocked();
    return STATUS_OK;
}

binder_status_t AIBinder_DeathRecipient::unlinkToDeath(sp<IBinder> binder, void* cookie) {
    CHECK(binder != nullptr);

    std::lock_guard<std::mutex> l(mDeathRecipientsMutex);

    for (auto it = mDeathRecipients.rbegin(); it != mDeathRecipients.rend(); ++it) {
        sp<TransferDeathRecipient> recipient = *it;

        if (recipient->getCookie() == cookie && recipient->getWho() == binder) {
            mDeathRecipients.erase(it.base() - 1);

            status_t status = binder->unlinkToDeath(recipient, cookie, 0 /*flags*/);
            if (status != ::android::OK) {
                LOG(ERROR) << __func__
                           << ": removed reference to death recipient but unlink failed.";
            }
            return PruneStatusT(status);
        }
    }

    return STATUS_NAME_NOT_FOUND;
}

// start of C-API methods

AIBinder* AIBinder_new(const AIBinder_Class* clazz, void* args) {
    if (clazz == nullptr) {
        LOG(ERROR) << __func__ << ": Must provide class to construct local binder.";
        return nullptr;
    }

    void* userData = clazz->onCreate(args);

    sp<AIBinder> ret = new ABBinder(clazz, userData);
    ABBinderTag::attach(ret->getBinder());

    AIBinder_incStrong(ret.get());
    return ret.get();
}

bool AIBinder_isRemote(const AIBinder* binder) {
    if (binder == nullptr) {
        return false;
    }

    return binder->isRemote();
}

bool AIBinder_isAlive(const AIBinder* binder) {
    if (binder == nullptr) {
        return false;
    }

    return const_cast<AIBinder*>(binder)->getBinder()->isBinderAlive();
}

binder_status_t AIBinder_ping(AIBinder* binder) {
    if (binder == nullptr) {
        return STATUS_UNEXPECTED_NULL;
    }

    return PruneStatusT(binder->getBinder()->pingBinder());
}

binder_status_t AIBinder_dump(AIBinder* binder, int fd, const char** args, uint32_t numArgs) {
    if (binder == nullptr) {
        return STATUS_UNEXPECTED_NULL;
    }

    ABBinder* bBinder = binder->asABBinder();
    if (bBinder != nullptr) {
        AIBinder_onDump onDump = binder->getClass()->onDump;
        if (onDump == nullptr) {
            return STATUS_OK;
        }
        return PruneStatusT(onDump(bBinder, fd, args, numArgs));
    }

    ::android::Vector<String16> utf16Args;
    utf16Args.setCapacity(numArgs);
    for (uint32_t i = 0; i < numArgs; i++) {
        utf16Args.push(String16(String8(args[i])));
    }

    status_t status = binder->getBinder()->dump(fd, utf16Args);
    return PruneStatusT(status);
}

binder_status_t AIBinder_linkToDeath(AIBinder* binder, AIBinder_DeathRecipient* recipient,
                                     void* cookie) {
    if (binder == nullptr || recipient == nullptr) {
        LOG(ERROR) << __func__ << ": Must provide binder and recipient.";
        return STATUS_UNEXPECTED_NULL;
    }

    // returns binder_status_t
    return recipient->linkToDeath(binder->getBinder(), cookie);
}

binder_status_t AIBinder_unlinkToDeath(AIBinder* binder, AIBinder_DeathRecipient* recipient,
                                       void* cookie) {
    if (binder == nullptr || recipient == nullptr) {
        LOG(ERROR) << __func__ << ": Must provide binder and recipient.";
        return STATUS_UNEXPECTED_NULL;
    }

    // returns binder_status_t
    return recipient->unlinkToDeath(binder->getBinder(), cookie);
}

uid_t AIBinder_getCallingUid() {
    return ::android::IPCThreadState::self()->getCallingUid();
}

pid_t AIBinder_getCallingPid() {
    return ::android::IPCThreadState::self()->getCallingPid();
}

void AIBinder_incStrong(AIBinder* binder) {
    if (binder == nullptr) {
        LOG(ERROR) << __func__ << ": on null binder";
        return;
    }

    binder->incStrong(nullptr);
}
void AIBinder_decStrong(AIBinder* binder) {
    if (binder == nullptr) {
        LOG(ERROR) << __func__ << ": on null binder";
        return;
    }

    binder->decStrong(nullptr);
}
int32_t AIBinder_debugGetRefCount(AIBinder* binder) {
    if (binder == nullptr) {
        LOG(ERROR) << __func__ << ": on null binder";
        return -1;
    }

    return binder->getStrongCount();
}

bool AIBinder_associateClass(AIBinder* binder, const AIBinder_Class* clazz) {
    if (binder == nullptr) {
        return false;
    }

    return binder->associateClass(clazz);
}

const AIBinder_Class* AIBinder_getClass(AIBinder* binder) {
    if (binder == nullptr) {
        return nullptr;
    }

    return binder->getClass();
}

void* AIBinder_getUserData(AIBinder* binder) {
    if (binder == nullptr) {
        return nullptr;
    }

    ABBinder* bBinder = binder->asABBinder();
    if (bBinder == nullptr) {
        return nullptr;
    }

    return bBinder->getUserData();
}

binder_status_t AIBinder_prepareTransaction(AIBinder* binder, AParcel** in) {
    if (binder == nullptr || in == nullptr) {
        LOG(ERROR) << __func__ << ": requires non-null parameters.";
        return STATUS_UNEXPECTED_NULL;
    }
    const AIBinder_Class* clazz = binder->getClass();
    if (clazz == nullptr) {
        LOG(ERROR) << __func__
                   << ": Class must be defined for a remote binder transaction. See "
                      "AIBinder_associateClass.";
        return STATUS_INVALID_OPERATION;
    }

    if (!binder->isRemote()) {
        LOG(WARNING) << "A binder object at " << binder
                     << " is being transacted on, however, this object is in the same process as "
                        "its proxy. Transacting with this binder is expensive compared to just "
                        "calling the corresponding functionality in the same process.";
    }

    *in = new AParcel(binder);
    status_t status = (*in)->get()->writeInterfaceToken(clazz->getInterfaceDescriptor());
    binder_status_t ret = PruneStatusT(status);

    if (ret != STATUS_OK) {
        delete *in;
        *in = nullptr;
    }

    return ret;
}

static void DestroyParcel(AParcel** parcel) {
    delete *parcel;
    *parcel = nullptr;
}

binder_status_t AIBinder_transact(AIBinder* binder, transaction_code_t code, AParcel** in,
                                  AParcel** out, binder_flags_t flags) {
    if (in == nullptr) {
        LOG(ERROR) << __func__ << ": requires non-null in parameter";
        return STATUS_UNEXPECTED_NULL;
    }

    using AutoParcelDestroyer = std::unique_ptr<AParcel*, void (*)(AParcel**)>;
    // This object is the input to the transaction. This function takes ownership of it and deletes
    // it.
    AutoParcelDestroyer forIn(in, DestroyParcel);

    if (!isUserCommand(code)) {
        LOG(ERROR) << __func__ << ": Only user-defined transactions can be made from the NDK.";
        return STATUS_UNKNOWN_TRANSACTION;
    }

    if ((flags & ~FLAG_ONEWAY) != 0) {
        LOG(ERROR) << __func__ << ": Unrecognized flags sent: " << flags;
        return STATUS_BAD_VALUE;
    }

    if (binder == nullptr || *in == nullptr || out == nullptr) {
        LOG(ERROR) << __func__ << ": requires non-null parameters.";
        return STATUS_UNEXPECTED_NULL;
    }

    if ((*in)->getBinder() != binder) {
        LOG(ERROR) << __func__ << ": parcel is associated with binder object " << binder
                   << " but called with " << (*in)->getBinder();
        return STATUS_BAD_VALUE;
    }

    *out = new AParcel(binder);

    status_t status = binder->getBinder()->transact(code, *(*in)->get(), (*out)->get(), flags);
    binder_status_t ret = PruneStatusT(status);

    if (ret != STATUS_OK) {
        delete *out;
        *out = nullptr;
    }

    return ret;
}

AIBinder_DeathRecipient* AIBinder_DeathRecipient_new(
        AIBinder_DeathRecipient_onBinderDied onBinderDied) {
    if (onBinderDied == nullptr) {
        LOG(ERROR) << __func__ << ": requires non-null onBinderDied parameter.";
        return nullptr;
    }
    auto ret = new AIBinder_DeathRecipient(onBinderDied);
    ret->incStrong(nullptr);
    return ret;
}

void AIBinder_DeathRecipient_delete(AIBinder_DeathRecipient* recipient) {
    if (recipient == nullptr) {
        return;
    }

    recipient->decStrong(nullptr);
}
