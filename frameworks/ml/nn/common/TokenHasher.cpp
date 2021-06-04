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

#include "TokenHasher.h"

#include "NeuralNetworks.h"

#include <android-base/logging.h>
#include <openssl/sha.h>

namespace android {
namespace nn {

TokenHasher::TokenHasher(const uint8_t* token) : mIsError(token == nullptr) {
    if (mIsError) {
        return;
    }
    if (SHA256_Init(&mHasher) == 0 ||
        SHA256_Update(&mHasher, token, ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN) == 0) {
        mIsError = true;
    }
}

bool TokenHasher::update(const void* bytes, size_t length) {
    CHECK(!mIsError) << "Calling update on an token in error state";
    if (SHA256_Update(&mHasher, bytes, length) == 0) {
        mIsError = true;
        return false;
    }
    return true;
}

bool TokenHasher::finish() {
    CHECK(!mIsError) << "Calling finish on an token in error state";
    static_assert(SHA256_DIGEST_LENGTH == ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN,
                  "SHA256_DIGEST_LENGTH != ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN");
    mToken.resize(ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN);
    if (SHA256_Final(mToken.data(), &mHasher) == 0) {
        mToken.clear();
        mIsError = true;
        return false;
    }
    return true;
}

const uint8_t* TokenHasher::getCacheToken() const {
    if (mIsError) {
        return nullptr;
    } else {
        CHECK(!mToken.empty());
        return mToken.data();
    }
}

}  // namespace nn
}  // namespace android
