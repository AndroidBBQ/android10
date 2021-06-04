/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma once

#include <atomic>
#include <limits>

namespace android {

template<typename T>
class HandleGenerator {
  protected:
    static T getNextHandle();
};

template<typename T>
T HandleGenerator<T>::getNextHandle() {
    static std::atomic<uint32_t> mNextUniqueId(1);
    uint32_t id = mNextUniqueId++;
    while (id > std::numeric_limits<T>::max()) {
        id -= std::numeric_limits<T>::max();
    }
    return static_cast<T>(id);
}

}  // namespace android
