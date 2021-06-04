/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_COMMON_TOKEN_HASHER_H
#define ANDROID_ML_NN_COMMON_TOKEN_HASHER_H

#include <cstring>
#include <vector>

#include <android-base/macros.h>
#include <openssl/sha.h>

namespace android {
namespace nn {

class TokenHasher {
    DISALLOW_COPY_AND_ASSIGN(TokenHasher);

   public:
    // Initializes the hasher with token. If token is nullptr, the hasher is set to be empty;
    // otherwise, it must be of length ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN.
    TokenHasher(const uint8_t* token);

    // Updates with a byte array. Returns false and sets to be empty on fail.
    // The client must check if the hasher is valid before invoking this method.
    bool update(const void* bytes, size_t length);

    // Updates with a string ended with '\0'. Returns false and sets to be empty on fail.
    // The client must check if the hasher is valid before invoking this method.
    bool updateFromString(const char* s) { return update(s, strlen(s)); }

    // Finishes the hasher, and writes re-hashed token to mToken.
    // Returns false and sets to be empty on fail.
    // The client must check if the hasher is valid before invoking this method.
    bool finish();

    // Returns a pointer to the transformed token if the hasher is successfully finished.
    // Returns nullptr if the hasher is initialized with a nullptr, or some call on the
    // hasher failed.
    // Aborts if the hasher is valid but finish has not been called.
    const uint8_t* getCacheToken() const;

    bool ok() const { return !mIsError; }

   private:
    // Stores the transformed token, non-empty iff the hasher is not initialized
    // with nullptr and finish is called.
    std::vector<uint8_t> mToken;
    SHA256_CTX mHasher;
    // Indicates that either the hasher is initialized with a nullptr, or some call on
    // the hasher failed.
    bool mIsError;
};

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_TOKEN_HASHER_H
