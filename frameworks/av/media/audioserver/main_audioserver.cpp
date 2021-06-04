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

#define LOG_TAG "audioserver"
//#define LOG_NDEBUG 0

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Log.h>

// from LOCAL_C_INCLUDES
#include "aaudio/AAudioTesting.h"
#include "AudioFlinger.h"
#include "AudioPolicyService.h"
#include "AAudioService.h"
#include "utility/AAudioUtilities.h"
#include "MediaLogService.h"
#include "MediaUtils.h"
#include "SoundTriggerHwService.h"

using namespace android;

int main(int argc __unused, char **argv)
{
    // TODO: update with refined parameters
    limitProcessMemory(
        "audio.maxmem", /* "ro.audio.maxmem", property that defines limit */
        (size_t)512 * (1 << 20), /* SIZE_MAX, upper limit in bytes */
        20 /* upper limit as percentage of physical RAM */);

    signal(SIGPIPE, SIG_IGN);

    bool doLog = (bool) property_get_bool("ro.test_harness", 0);

    pid_t childPid;
    // FIXME The advantage of making the process containing media.log service the parent process of
    // the process that contains the other audio services, is that it allows us to collect more
    // detailed information such as signal numbers, stop and continue, resource usage, etc.
    // But it is also more complex.  Consider replacing this by independent processes, and using
    // binder on death notification instead.
    if (doLog && (childPid = fork()) != 0) {
        // media.log service
        //prctl(PR_SET_NAME, (unsigned long) "media.log", 0, 0, 0);
        // unfortunately ps ignores PR_SET_NAME for the main thread, so use this ugly hack
        strcpy(argv[0], "media.log");
        sp<ProcessState> proc(ProcessState::self());
        MediaLogService::instantiate();
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
        for (;;) {
            siginfo_t info;
            int ret = waitid(P_PID, childPid, &info, WEXITED | WSTOPPED | WCONTINUED);
            if (ret == EINTR) {
                continue;
            }
            if (ret < 0) {
                break;
            }
            char buffer[32];
            const char *code;
            switch (info.si_code) {
            case CLD_EXITED:
                code = "CLD_EXITED";
                break;
            case CLD_KILLED:
                code = "CLD_KILLED";
                break;
            case CLD_DUMPED:
                code = "CLD_DUMPED";
                break;
            case CLD_STOPPED:
                code = "CLD_STOPPED";
                break;
            case CLD_TRAPPED:
                code = "CLD_TRAPPED";
                break;
            case CLD_CONTINUED:
                code = "CLD_CONTINUED";
                break;
            default:
                snprintf(buffer, sizeof(buffer), "unknown (%d)", info.si_code);
                code = buffer;
                break;
            }
            struct rusage usage;
            getrusage(RUSAGE_CHILDREN, &usage);
            ALOG(LOG_ERROR, "media.log", "pid %d status %d code %s user %ld.%03lds sys %ld.%03lds",
                    info.si_pid, info.si_status, code,
                    usage.ru_utime.tv_sec, usage.ru_utime.tv_usec / 1000,
                    usage.ru_stime.tv_sec, usage.ru_stime.tv_usec / 1000);
            sp<IServiceManager> sm = defaultServiceManager();
            sp<IBinder> binder = sm->getService(String16("media.log"));
            if (binder != 0) {
                Vector<String16> args;
                binder->dump(-1, args);
            }
            switch (info.si_code) {
            case CLD_EXITED:
            case CLD_KILLED:
            case CLD_DUMPED: {
                ALOG(LOG_INFO, "media.log", "exiting");
                _exit(0);
                // not reached
                }
            default:
                break;
            }
        }
    } else {
        // all other services
        if (doLog) {
            prctl(PR_SET_PDEATHSIG, SIGKILL);   // if parent media.log dies before me, kill me also
            setpgid(0, 0);                      // but if I die first, don't kill my parent
        }
        android::hardware::configureRpcThreadpool(4, false /*callerWillJoin*/);
        sp<ProcessState> proc(ProcessState::self());
        sp<IServiceManager> sm = defaultServiceManager();
        ALOGI("ServiceManager: %p", sm.get());
        AudioFlinger::instantiate();
        AudioPolicyService::instantiate();

        // AAudioService should only be used in OC-MR1 and later.
        // And only enable the AAudioService if the system MMAP policy explicitly allows it.
        // This prevents a client from misusing AAudioService when it is not supported.
        aaudio_policy_t mmapPolicy = property_get_int32(AAUDIO_PROP_MMAP_POLICY,
                                                        AAUDIO_POLICY_NEVER);
        if (mmapPolicy == AAUDIO_POLICY_AUTO || mmapPolicy == AAUDIO_POLICY_ALWAYS) {
            AAudioService::instantiate();
        }

        SoundTriggerHwService::instantiate();
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    }
}
