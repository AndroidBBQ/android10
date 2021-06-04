/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <string>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <utils/misc.h>

// from LOCAL_C_INCLUDES
#include "MediaExtractorService.h"
#include "MediaUtils.h"
#include "minijail.h"

using namespace android;

static const char kSystemSeccompPolicyPath[] =
        "/apex/com.android.media/etc/seccomp_policy/mediaextractor.policy";
static const char kVendorSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/mediaextractor.policy";

int main(int argc __unused, char** argv)
{
    limitProcessMemory(
        "ro.media.maxmem", /* property that defines limit */
        SIZE_MAX, /* upper limit in bytes */
        20 /* upper limit as percentage of physical RAM */);

    signal(SIGPIPE, SIG_IGN);

    //b/62255959: this forces libutis.so to dlopen vendor version of libutils.so
    //before minijail is on. This is dirty but required since some syscalls such
    //as pread64 are used by linker but aren't allowed in the minijail. By
    //calling the function before entering minijail, we can force dlopen.
    android::report_sysprop_change();

    SetUpMinijail(kSystemSeccompPolicyPath, kVendorSeccompPolicyPath);

    strcpy(argv[0], "media.extractor");
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    MediaExtractorService::instantiate();

    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}
