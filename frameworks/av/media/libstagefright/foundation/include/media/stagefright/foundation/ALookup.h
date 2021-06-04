/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef A_LOOKUP_H_

#define A_LOOKUP_H_

#include <utility>
#include <vector>

namespace android {

template<typename T, typename U>
struct ALookup {
    ALookup(std::initializer_list<std::pair<T, U>> list);

    bool lookup(const T& from, U *to) const;
    bool rlookup(const U& from, T *to) const;

    template<typename V, typename = typename std::enable_if<!std::is_same<T, V>::value>::type>
    inline bool map(const T& from, V *to) const { return lookup(from, to); }

    template<typename V, typename = typename std::enable_if<!std::is_same<T, V>::value>::type>
    inline bool map(const V& from, T *to) const { return rlookup(from, to); }

private:
    std::vector<std::pair<T, U>> mTable;
};

template<typename T, typename U>
ALookup<T, U>::ALookup(std::initializer_list<std::pair<T, U>> list)
    : mTable(list) {
}

template<typename T, typename U>
bool ALookup<T, U>::lookup(const T& from, U *to) const {
    for (auto elem : mTable) {
        if (elem.first == from) {
            *to = elem.second;
            return true;
        }
    }
    return false;
}

template<typename T, typename U>
bool ALookup<T, U>::rlookup(const U& from, T *to) const {
    for (auto elem : mTable) {
        if (elem.second == from) {
            *to = elem.first;
            return true;
        }
    }
    return false;
}

} // namespace android

#endif  // A_UTILS_H_
