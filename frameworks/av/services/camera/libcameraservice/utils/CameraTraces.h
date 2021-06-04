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

#ifndef ANDROID_SERVERS_CAMERA_TRACES_H_
#define ANDROID_SERVERS_CAMERA_TRACES_H_

#include <utils/Errors.h>
#include <utils/String16.h>
#include <utils/Vector.h>

namespace android {
namespace camera3 {

struct CameraTracesImpl;

// Collect a list of the process's stack traces
class CameraTraces {
public:
    /**
     * Save the current stack trace for each thread in the process. At most
     * MAX_TRACES will be saved, after which the oldest traces will be discarded.
     *
     * <p>Use CameraTraces::dump to print out the traces.</p>
     */
    static void     saveTrace();

    /**
     * Prints all saved traces to the specified file descriptor.
     *
     * <p>Each line is indented by DUMP_INDENT spaces.</p>
     */
    static status_t dump(int fd, const Vector<String16>& args);

private:
    enum {
        // Don't collect more than 100 traces. Discard oldest.
        MAX_TRACES = 100,

        // Insert 2 spaces when dumping the traces
        DUMP_INDENT = 2,
    };

    CameraTraces();
    ~CameraTraces();
    CameraTraces(CameraTraces& rhs);

    static CameraTracesImpl& sImpl;
}; // class CameraTraces

}; // namespace camera3
}; // namespace android

#endif // ANDROID_SERVERS_CAMERA_TRACES_H_
