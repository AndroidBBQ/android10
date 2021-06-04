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

#ifndef MINIKIN_HASHER_H
#define MINIKIN_HASHER_H

#include <cstdint>

#include <string>

#include "minikin/Macros.h"

namespace minikin {

// Provides a Jenkins hash implementation.
class Hasher {
public:
    Hasher() : mHash(0u) {}

    IGNORE_INTEGER_OVERFLOW inline Hasher& update(uint32_t data) {
        mHash += data;
        mHash += (mHash << 10);
        mHash ^= (mHash >> 6);
        return *this;
    }

    inline Hasher& updateShorts(const uint16_t* data, uint32_t length) {
        update(length);
        uint32_t i;
        for (i = 0; i < (length & -2); i += 2) {
            update((uint32_t)data[i] | ((uint32_t)data[i + 1] << 16));
        }
        if (length & 1) {
            update((uint32_t)data[i]);
        }
        return *this;
    }

    inline Hasher& updateString(const std::string& str) {
        uint32_t size = str.size();
        update(size);
        uint32_t i;
        for (i = 0; i < (size & -4); i += 4) {
            update((uint32_t)str[i] | ((uint32_t)str[i + 1] << 8) | ((uint32_t)str[i + 2] << 16) |
                   ((uint32_t)str[i + 3] << 24));
        }
        if (size & 3) {
            uint32_t data = str[i];
            data |= ((size & 3) > 1) ? ((uint32_t)str[i + 1] << 8) : 0;
            data |= ((size & 3) > 2) ? ((uint32_t)str[i + 2] << 16) : 0;
            update(data);
        }
        return *this;
    }

    IGNORE_INTEGER_OVERFLOW inline uint32_t hash() {
        uint32_t hash = mHash;
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        return hash;
    }

private:
    uint32_t mHash;
};

}  // namespace minikin

#endif  // MINIKIN_HASHER_H
