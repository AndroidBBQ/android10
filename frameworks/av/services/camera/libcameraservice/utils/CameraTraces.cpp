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

#define LOG_TAG "CameraTraces"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0

#include "utils/CameraTraces.h"
#include <utils/ProcessCallStack.h>

#include <utils/Mutex.h>
#include <utils/List.h>

#include <utils/Log.h>
#include <cutils/trace.h>

namespace android {
namespace camera3 {

struct CameraTracesImpl {
    Mutex                    tracesLock;
    List<ProcessCallStack>   pcsList;
}; // class CameraTraces::Impl;

static CameraTracesImpl gImpl;
CameraTracesImpl& CameraTraces::sImpl = gImpl;

void CameraTraces::saveTrace() {
    ALOGV("%s: begin", __FUNCTION__);
    ATRACE_BEGIN("CameraTraces::saveTrace");
    Mutex::Autolock al(sImpl.tracesLock);

    List<ProcessCallStack>& pcsList = sImpl.pcsList;

    // Insert new ProcessCallStack, and immediately crawl all the threads
    pcsList.push_front(ProcessCallStack());
    ProcessCallStack& pcs = *pcsList.begin();
    pcs.update();

    if (pcsList.size() > MAX_TRACES) {
        // Prune list periodically and discard oldest entry
        pcsList.erase(--pcsList.end());
    }

    IF_ALOGV() {
        pcs.log(LOG_TAG, ANDROID_LOG_VERBOSE);
    }

    ALOGD("Process trace saved. Use dumpsys media.camera to view.");

    ATRACE_END();
}

status_t CameraTraces::dump(int fd, const Vector<String16> &args __attribute__((unused))) {
    ALOGV("%s: fd = %d", __FUNCTION__, fd);
    Mutex::Autolock al(sImpl.tracesLock);
    List<ProcessCallStack>& pcsList = sImpl.pcsList;

    if (fd < 0) {
        ALOGW("%s: Negative FD (%d)", __FUNCTION__, fd);
        return BAD_VALUE;
    }

    dprintf(fd, "== Camera error traces (%zu): ==\n", pcsList.size());

    if (pcsList.empty()) {
        dprintf(fd, "  No camera traces collected.\n");
    }

    // Print newest items first
    List<ProcessCallStack>::iterator it, end;
    for (it = pcsList.begin(), end = pcsList.end(); it != end; ++it) {
        const ProcessCallStack& pcs = *it;
        pcs.dump(fd, DUMP_INDENT);
    }

    return OK;
}

}; // namespace camera3
}; // namespace android
