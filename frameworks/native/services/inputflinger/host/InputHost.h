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

#ifndef ANDROID_INPUT_HOST_H
#define ANDROID_INPUT_HOST_H

#include <vector>

#include <hardware/input.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>

#include "InputDriver.h"

namespace android {

class InputDriverInterface;

class InputHostInterface : public virtual RefBase {
protected:
    InputHostInterface() = default;
    virtual ~InputHostInterface() = default;

public:

    virtual void registerInputDriver(InputDriverInterface* driver) = 0;

    virtual void dump(String8& result) = 0;
};

class InputHost : public InputHostInterface {
public:
    InputHost() = default;

    virtual void registerInputDriver(InputDriverInterface* driver) override;

    virtual void dump(String8& result) override;

private:
    std::vector<sp<InputDriverInterface>> mDrivers;
};

} // namespace android
#endif // ANDRIOD_INPUT_HOST_H
