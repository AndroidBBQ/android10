/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *            http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CLEARKEY_INIT_DATA_PARSER_H_
#define CLEARKEY_INIT_DATA_PARSER_H_

#include <android/hardware/drm/1.0/types.h>

#include "ClearKeyTypes.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

using ::android::hardware::drm::V1_0::Status;

class InitDataParser {
public:
    InitDataParser() {}

    Status parse(const std::vector<uint8_t>& initData,
            const std::string& mimeType,
            V1_0::KeyType keyType,
            std::vector<uint8_t>* licenseRequest);

private:
    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(InitDataParser);

    Status parsePssh(const std::vector<uint8_t>& initData,
            std::vector<const uint8_t*>* keyIds);

    std::string generateRequest(V1_0::KeyType keyType,
            const std::vector<const uint8_t*>& keyIds);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif // CLEARKEY_INIT_DATA_PARSER_H_
