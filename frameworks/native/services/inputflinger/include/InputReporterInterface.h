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

#ifndef _UI_INPUT_REPORTER_INTERFACE_H
#define _UI_INPUT_REPORTER_INTERFACE_H

#include <utils/RefBase.h>

namespace android {

/*
 * The interface used by the InputDispatcher to report information about input events after
 * it is sent to the application, such as if a key is unhandled or dropped.
 */
class InputReporterInterface : public virtual RefBase {
protected:
    virtual ~InputReporterInterface() { }

public:
    // Report a key that was not handled by the system or apps.
    // A key event is unhandled if:
    //   - The event was not handled and there is no fallback key; or
    //   - The event was not handled and it has a fallback key,
    //       but the fallback key was not handled.
    virtual void reportUnhandledKey(uint32_t sequenceNum) = 0;

    // Report a key that was dropped by InputDispatcher.
    // A key can be dropped for several reasons. See the enum
    // InputDispatcher::DropReason for details.
    virtual void reportDroppedKey(uint32_t sequenceNum) = 0;
};

/*
 * Factory method for InputReporter.
 */
sp<InputReporterInterface> createInputReporter();

} // namespace android

#endif // _UI_INPUT_REPORTER_INTERFACE_H
