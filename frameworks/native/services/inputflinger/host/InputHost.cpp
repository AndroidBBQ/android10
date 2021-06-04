/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <vector>

#include "InputDriver.h"
#include "InputHost.h"

#include <utils/Log.h>
#include <utils/String8.h>

#define INDENT "  "

namespace android {

void InputHost::registerInputDriver(InputDriverInterface* driver) {
    LOG_ALWAYS_FATAL_IF(driver == nullptr, "Cannot register a nullptr as an InputDriver!");
    driver->init();
    mDrivers.push_back(driver);
}

void InputHost::dump(String8& result) {
    result.append(INDENT "Input Drivers:\n");
    for (size_t i = 0; i < mDrivers.size(); i++) {
        mDrivers[i]->dump(result);
    }
}

} // namespace android
