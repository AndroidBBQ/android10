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

#ifndef KEY_FETCHER_H_
#define KEY_FETCHER_H_

#include <vector>

#include <media/stagefright/foundation/ABuffer.h>
#include <utils/Errors.h>

using namespace std;

namespace android {
namespace clearkeycas {

// Interface for classes which extract the content key from an Ecm.
class KeyFetcher {
public:
    struct KeyInfo {
        sp<ABuffer> key_bytes;
        int key_id;
    };

    KeyFetcher() {}
    virtual ~KeyFetcher() {}

    // Initializes resources set in subclass-specific calls. This must be called
    // before threads are started.
    virtual status_t Init() = 0;

    // Obtains content key(s) based on contents of |ecm|. |asset_id| is the
    // internal id of the asset, |keys| is a vector containing instances of a
    // class containing a content key and an id. |asset_id| and |keys| are
    // owned by the caller and must be non-null.
    virtual status_t ObtainKey(const sp<ABuffer>& ecm,
            uint64_t* asset_id, vector<KeyInfo>* keys) = 0;
};

}  // namespace clearkeycas
}  // namespace android

#endif  // KEY_FETCHER_H_
