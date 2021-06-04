/*
 * Copyright 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STAGEFRIGHT_FOUNDATION_MUTEXED_H_
#define STAGEFRIGHT_FOUNDATION_MUTEXED_H_

#include <utils/Mutex.h>
#include <utils/Condition.h>

namespace android {

/*
 * Wrapper class to programmatically protect a structure using a mutex.
 *
 * Mutexed<> objects contain a built-in mutex. Protection is enforced because the structure can
 * only be accessed by locking the mutex first.
 *
 * Usage:
 *
 * struct DataToProtect {
 *   State(int var1) : mVar1(var1), mVar2(0) { }
 *   int mVar1;
 *   int mVar2;
 *   Condition mCondition1;
 * };
 *
 * Mutexed<DataToProtect> mProtectedData;
 *
 * // members are inaccessible via mProtectedData directly
 *
 * void someFunction() {
 *   Mutexed<DataToProtect>::Locked data(mProtectedData); // access the protected data
 *
 *   // the mutex is locked here, so accessing the data is safe
 *
 *   if (data->mVar1 < 5) {
 *     ++data->mVar2;
 *   }
 *
 *   // if you need to temporarily unlock the mutex, you can use unlock/relock mutex locally
 *   // using the accessor object.
 *
 *   data.unlock();
 *
 *   // data is inaccessible here
 *
 *   doSomeLongOperation();
 *
 *   data.lock();
 *
 *   // data is now accessible again. Note: it may have changed since unlock().
 *
 *   // you can use the integral mutex to wait for a condition
 *
 *   data.waitForCondition(data->mCondition1);
 *
 *   helper(&data);
 * }
 *
 * void trigger() {
 *   Mutexed<DataToProtect>::Locked data(mProtectedData);
 *   data->mCondition1.signal();
 * }
 *
 * void helper(const Mutexed<DataToProtect>::Locked &data) {
 *   data->mVar1 = 3;
 * }
 *
 */

template<typename T>
class Mutexed {
public:
    /*
     * Accessor-guard of the mutex-protected structure. This can be dereferenced to
     * access the structure (using -> or * operators).
     *
     * Upon creation, the mutex is locked. You can use lock()/unlock() methods to
     * temporarily lock/unlock the mutex. Using any references to the underlying
     * structure or its members defeats the protection of this class, so don't do
     * it.
     *
     * Note: The accessor-guard itself is not thread-safe. E.g. you should not call
     * unlock() or lock() from different threads; they must be called from the thread
     * that locked the original wrapper.
     *
     * Also note: Recursive locking/unlocking is not supported by the accessor. This
     * is as intended, as it allows lenient locking/unlocking via multiple code paths.
     */
    class Locked {
    public:
        inline Locked(Mutexed<T> &mParent);
        inline Locked(Locked &&from) noexcept :
            mLock(from.mLock),
            mTreasure(from.mTreasure),
            mLocked(from.mLocked) {}
        inline ~Locked();

        // dereference the protected structure. This returns nullptr if the
        // mutex is not locked by this accessor-guard.
        inline T* operator->() const { return mLocked ? &mTreasure : nullptr; }
        inline T& operator*()  const { return mLocked ?  mTreasure : *(T*)nullptr; }

        // same as *
        inline T& get() const { return mLocked ?  mTreasure : *(T*)nullptr; }
        // sets structure. this will abort if mLocked is false.
        inline void set(T& o) const { get() = o; }

        // Wait on the condition variable using lock. Must be locked.
        inline status_t waitForCondition(Condition &cond) { return cond.wait(mLock); }

        // same with relative timeout
        inline status_t waitForConditionRelative(Condition &cond, nsecs_t reltime) {
            return cond.waitRelative(mLock, reltime);
        }

        // unlocks the integral mutex. No-op if the mutex was already unlocked.
        inline void unlock();

        // locks the integral mutex. No-op if the mutex was already locked.
        inline void lock();

    private:
        Mutex &mLock;
        T &mTreasure;
        bool mLocked;

        // disable copy constructors
        Locked(const Locked&) = delete;
        void operator=(const Locked&) = delete;
    };

    // Wrap all constructors of the underlying structure
    template<typename ...Args>
    Mutexed(Args... args) : mTreasure(args...) { }

    ~Mutexed() { }

    // Lock the mutex, and create an accessor-guard (a Locked object) to access the underlying
    // structure. This returns an object that dereferences to the wrapped structure when the mutex
    // is locked by it, or otherwise to "null".
    // This is just a shorthand for Locked() constructor to avoid specifying the template type.
    inline Locked lock() {
        return Locked(*this);
    }

private:
    friend class Locked;
    Mutex mLock;
    T mTreasure;

    // disable copy constructors
    Mutexed(const Mutexed<T>&) = delete;
    void operator=(const Mutexed<T>&) = delete;
};

template<typename T>
inline Mutexed<T>::Locked::Locked(Mutexed<T> &mParent)
    : mLock(mParent.mLock),
      mTreasure(mParent.mTreasure),
      mLocked(true) {
    mLock.lock();
}

template<typename T>
inline Mutexed<T>::Locked::~Locked() {
    if (mLocked) {
        mLock.unlock();
    }
}

template<typename T>
inline void Mutexed<T>::Locked::unlock() {
    if (mLocked) {
        mLocked = false;
        mLock.unlock();
    }
}

template<typename T>
inline void Mutexed<T>::Locked::lock() {
    if (!mLocked) {
        mLock.lock();
        mLocked = true;
    }
}

} // namespace android

#endif
