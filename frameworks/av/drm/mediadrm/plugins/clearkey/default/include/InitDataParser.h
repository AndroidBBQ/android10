/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/Vector.h>

namespace clearkeydrm {

class InitDataParser {
public:
    InitDataParser() {}

    android::status_t parse(const android::Vector<uint8_t>& initData,
            const android::String8& type,
            android::Vector<uint8_t>* licenseRequest);

private:
    DISALLOW_EVIL_CONSTRUCTORS(InitDataParser);

    android::status_t parsePssh(const android::Vector<uint8_t>& initData,
            android::Vector<const uint8_t*>* keyIds);

    android::String8 generateRequest(
            const android::Vector<const uint8_t*>& keyIds);
};

} // namespace clearkeydrm

#endif // CLEARKEY_INIT_DATA_PARSER_H_
