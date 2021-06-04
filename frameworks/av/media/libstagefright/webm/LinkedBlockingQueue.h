/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef LINKEDBLOCKINGQUEUE_H_
#define LINKEDBLOCKINGQUEUE_H_

#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

namespace android {

template<typename T>
class LinkedBlockingQueue {
    List<T> mList;
    Mutex mLock;
    Condition mContentAvailableCondition;

    T front(bool remove) {
        Mutex::Autolock autolock(mLock);
        while (mList.empty()) {
            mContentAvailableCondition.wait(mLock);
        }
        T e = *(mList.begin());
        if (remove) {
            mList.erase(mList.begin());
        }
        return e;
    }

    DISALLOW_EVIL_CONSTRUCTORS(LinkedBlockingQueue);

public:
    LinkedBlockingQueue() {
    }

    ~LinkedBlockingQueue() {
    }

    bool empty() {
        Mutex::Autolock autolock(mLock);
        return mList.empty();
    }

    void clear() {
        Mutex::Autolock autolock(mLock);
        mList.clear();
    }

    T peek() {
        return front(false);
    }

    T take() {
        return front(true);
    }

    void push(T e) {
        Mutex::Autolock autolock(mLock);
        mList.push_back(e);
        mContentAvailableCondition.signal();
    }
};

} /* namespace android */
#endif /* LINKEDBLOCKINGQUEUE_H_ */
