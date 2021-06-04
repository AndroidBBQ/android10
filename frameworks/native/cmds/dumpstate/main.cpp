/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "dumpstate"

#include <binder/IPCThreadState.h>

#include "DumpstateInternal.h"
#include "DumpstateService.h"
#include "dumpstate.h"

namespace {

// Returns true if we should start the service and wait for a listener
// to bind with bugreport options.
bool ShouldStartServiceAndWait(int argc, char* argv[]) {
    bool do_wait = false;
    int c;
    // Keep flags in sync with Dumpstate::DumpOptions::Initialize.
    while ((c = getopt(argc, argv, "wdho:svqzpPBRSV:")) != -1 && !do_wait) {
        switch (c) {
            case 'w':
                do_wait = true;
                break;
            default:
                // Ignore all other options
                break;
        }
    }

    // Reset next index used by getopt so getopt can be called called again in Dumpstate::Run to
    // parse bugreport options.
    optind = 1;
    return do_wait;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (ShouldStartServiceAndWait(argc, argv)) {
        int ret;
        if ((ret = android::os::DumpstateService::Start()) != android::OK) {
            MYLOGE("Unable to start 'dumpstate' service: %d", ret);
            exit(1);
        }
        MYLOGI("'dumpstate' service started and will wait for a call to startBugreport()");

        // Waits forever for an incoming connection.
        // TODO(b/111441001): should this time out?
        android::IPCThreadState::self()->joinThreadPool();
        return 0;
    } else {
        return run_main(argc, argv);
    }
}
