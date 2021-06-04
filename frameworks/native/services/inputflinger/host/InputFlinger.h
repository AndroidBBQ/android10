/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_INPUT_FLINGER_H
#define ANDROID_INPUT_FLINGER_H

#include <stdint.h>
#include <sys/types.h>

#include "InputHost.h"

#include <cutils/compiler.h>
#include <input/IInputFlinger.h>
#include <input/ISetInputWindowsListener.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/StrongPointer.h>

namespace android {

class InputFlinger : public BnInputFlinger {
public:
    static char const* getServiceName() ANDROID_API {
        return "inputflinger";
    }

    InputFlinger() ANDROID_API;

    virtual status_t dump(int fd, const Vector<String16>& args);
    void setInputWindows(const std::vector<InputWindowInfo>&,
            const sp<ISetInputWindowsListener>&) {}
    void registerInputChannel(const sp<InputChannel>&) {}
    void unregisterInputChannel(const sp<InputChannel>&) {}

private:
    virtual ~InputFlinger();

    void dumpInternal(String8& result);

    sp<InputHostInterface> mHost;
};

} // namespace android

#endif // ANDROID_INPUT_FLINGER_H
