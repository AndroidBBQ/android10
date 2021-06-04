/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef _UI_INPUT_APPLICATION_H
#define _UI_INPUT_APPLICATION_H

#include <string>

#include <binder/IBinder.h>
#include <binder/Parcel.h>

#include <input/Input.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>

namespace android {

/*
 * Describes the properties of an application that can receive input.
 */
struct InputApplicationInfo {
    sp<IBinder> token;
    std::string name;
    nsecs_t dispatchingTimeout;

    status_t write(Parcel& output) const;
    static InputApplicationInfo read(const Parcel& from);
};


/*
 * Handle for an application that can receive input.
 *
 * Used by the native input dispatcher as a handle for the window manager objects
 * that describe an application.
 */
class InputApplicationHandle : public RefBase {
public:
    inline const InputApplicationInfo* getInfo() const {
        return &mInfo;
    }

    inline std::string getName() const {
        return !mInfo.name.empty() ? mInfo.name : "<invalid>";
    }

    inline nsecs_t getDispatchingTimeout(nsecs_t defaultValue) const {
        return mInfo.token ? mInfo.dispatchingTimeout : defaultValue;
    }

    inline sp<IBinder> getApplicationToken() const {
        return mInfo.token;
    }

    /**
     * Requests that the state of this object be updated to reflect
     * the most current available information about the application.
     *
     * This method should only be called from within the input dispatcher's
     * critical section.
     *
     * Returns true on success, or false if the handle is no longer valid.
     */
    virtual bool updateInfo() = 0;
protected:
    InputApplicationHandle();
    virtual ~InputApplicationHandle();

    InputApplicationInfo mInfo;
};

} // namespace android

#endif // _UI_INPUT_APPLICATION_H
