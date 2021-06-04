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

#ifndef FRAMEWORKS_NATIVE_CMDS_LSHAL_PIPE_RELAY_H_

#define FRAMEWORKS_NATIVE_CMDS_LSHAL_PIPE_RELAY_H_

#include <android-base/macros.h>
#include <ostream>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {
namespace lshal {

/* Creates an AF_UNIX socketpair and spawns a thread that relays any data
 * written to the "write"-end of the pair to the specified output stream "os".
 */
struct PipeRelay {
    explicit PipeRelay(std::ostream &os);
    ~PipeRelay();

    status_t initCheck() const;

    // Returns the file descriptor corresponding to the "write"-end of the
    // connection.
    int fd() const;

private:
    struct RelayThread;

    status_t mInitCheck;
    int mFds[2];
    sp<RelayThread> mThread;

    static void CloseFd(int *fd);

    DISALLOW_COPY_AND_ASSIGN(PipeRelay);
};

}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORKS_NATIVE_CMDS_LSHAL_PIPE_RELAY_H_

