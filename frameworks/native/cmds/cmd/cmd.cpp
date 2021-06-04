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

#define LOG_TAG "cmd"

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IResultReceiver.h>
#include <binder/IServiceManager.h>
#include <binder/IShellCallback.h>
#include <binder/TextOutput.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <memory>

#include "selinux/selinux.h"
#include "selinux/android.h"

#include "cmd.h"

#define DEBUG 0

using namespace android;

static int sort_func(const String16* lhs, const String16* rhs)
{
    return lhs->compare(*rhs);
}

struct SecurityContext_Delete {
    void operator()(security_context_t p) const {
        freecon(p);
    }
};
typedef std::unique_ptr<char[], SecurityContext_Delete> Unique_SecurityContext;

class MyShellCallback : public BnShellCallback
{
public:
    TextOutput& mErrorLog;
    bool mActive = true;

    MyShellCallback(TextOutput& errorLog) : mErrorLog(errorLog) {}

    virtual int openFile(const String16& path, const String16& seLinuxContext,
            const String16& mode) {
        String8 path8(path);
        char cwd[256];
        getcwd(cwd, 256);
        String8 fullPath(cwd);
        fullPath.appendPath(path8);
        if (!mActive) {
            mErrorLog << "Open attempt after active for: " << fullPath << endl;
            return -EPERM;
        }
#if DEBUG
        ALOGD("openFile: %s, full=%s", path8.string(), fullPath.string());
#endif
        int flags = 0;
        bool checkRead = false;
        bool checkWrite = false;
        if (mode == u"w") {
            flags = O_WRONLY|O_CREAT|O_TRUNC;
            checkWrite = true;
        } else if (mode == u"w+") {
            flags = O_RDWR|O_CREAT|O_TRUNC;
            checkRead = checkWrite = true;
        } else if (mode == u"r") {
            flags = O_RDONLY;
            checkRead = true;
        } else if (mode == u"r+") {
            flags = O_RDWR;
            checkRead = checkWrite = true;
        } else {
            mErrorLog << "Invalid mode requested: " << mode.string() << endl;
            return -EINVAL;
        }
        int fd = open(fullPath.string(), flags, S_IRWXU|S_IRWXG);
#if DEBUG
        ALOGD("openFile: fd=%d", fd);
#endif
        if (fd < 0) {
            return fd;
        }
        if (is_selinux_enabled() && seLinuxContext.size() > 0) {
            String8 seLinuxContext8(seLinuxContext);
            security_context_t tmp = nullptr;
            getfilecon(fullPath.string(), &tmp);
            Unique_SecurityContext context(tmp);
            if (checkWrite) {
                int accessGranted = selinux_check_access(seLinuxContext8.string(), context.get(),
                        "file", "write", nullptr);
                if (accessGranted != 0) {
#if DEBUG
                    ALOGD("openFile: failed selinux write check!");
#endif
                    close(fd);
                    mErrorLog << "System server has no access to write file context " << context.get() << " (from path " << fullPath.string() << ", context " << seLinuxContext8.string() << ")" << endl;
                    return -EPERM;
                }
            }
            if (checkRead) {
                int accessGranted = selinux_check_access(seLinuxContext8.string(), context.get(),
                        "file", "read", nullptr);
                if (accessGranted != 0) {
#if DEBUG
                    ALOGD("openFile: failed selinux read check!");
#endif
                    close(fd);
                    mErrorLog << "System server has no access to read file context " << context.get() << " (from path " << fullPath.string() << ", context " << seLinuxContext8.string() << ")" << endl;
                    return -EPERM;
                }
            }
        }
        return fd;
    }
};

class MyResultReceiver : public BnResultReceiver
{
public:
    Mutex mMutex;
    Condition mCondition;
    bool mHaveResult = false;
    int32_t mResult = 0;

    virtual void send(int32_t resultCode) {
        AutoMutex _l(mMutex);
        mResult = resultCode;
        mHaveResult = true;
        mCondition.signal();
    }

    int32_t waitForResult() {
        AutoMutex _l(mMutex);
        while (!mHaveResult) {
            mCondition.wait(mMutex);
        }
        return mResult;
    }
};

int cmdMain(const std::vector<std::string_view>& argv, TextOutput& outputLog, TextOutput& errorLog,
            int in, int out, int err, RunMode runMode) {
    sp<ProcessState> proc = ProcessState::self();
    proc->startThreadPool();

#if DEBUG
    ALOGD("cmd: starting");
#endif
    sp<IServiceManager> sm = defaultServiceManager();
    if (runMode == RunMode::kStandalone) {
        fflush(stdout);
    }
    if (sm == nullptr) {
        ALOGW("Unable to get default service manager!");
        errorLog << "cmd: Unable to get default service manager!" << endl;
        return 20;
    }

    int argc = argv.size();

    if (argc == 0) {
        errorLog << "cmd: No service specified; use -l to list all services" << endl;
        return 20;
    }

    if ((argc == 1) && (argv[0] == "-l")) {
        Vector<String16> services = sm->listServices();
        services.sort(sort_func);
        outputLog << "Currently running services:" << endl;

        for (size_t i=0; i<services.size(); i++) {
            sp<IBinder> service = sm->checkService(services[i]);
            if (service != nullptr) {
                outputLog << "  " << services[i] << endl;
            }
        }
        return 0;
    }

    const auto cmd = argv[0];

    Vector<String16> args;
    String16 serviceName = String16(cmd.data(), cmd.size());
    for (int i = 1; i < argc; i++) {
        args.add(String16(argv[i].data(), argv[i].size()));
    }
    sp<IBinder> service = sm->checkService(serviceName);
    if (service == nullptr) {
        if (runMode == RunMode::kStandalone) {
            ALOGW("Can't find service %.*s", static_cast<int>(cmd.size()), cmd.data());
        }
        errorLog << "cmd: Can't find service: " << cmd << endl;
        return 20;
    }

    sp<MyShellCallback> cb = new MyShellCallback(errorLog);
    sp<MyResultReceiver> result = new MyResultReceiver();

#if DEBUG
    ALOGD("cmd: Invoking %s in=%d, out=%d, err=%d", cmd, in, out, err);
#endif

    // TODO: block until a result is returned to MyResultReceiver.
    status_t error = IBinder::shellCommand(service, in, out, err, args, cb, result);
    if (error < 0) {
        const char* errstr;
        switch (error) {
            case BAD_TYPE: errstr = "Bad type"; break;
            case FAILED_TRANSACTION: errstr = "Failed transaction"; break;
            case FDS_NOT_ALLOWED: errstr = "File descriptors not allowed"; break;
            case UNEXPECTED_NULL: errstr = "Unexpected null"; break;
            default: errstr = strerror(-error); break;
        }
        if (runMode == RunMode::kStandalone) {
            ALOGW("Failure calling service %.*s: %s (%d)", static_cast<int>(cmd.size()), cmd.data(),
                  errstr, -error);
        }
        outputLog << "cmd: Failure calling service " << cmd << ": " << errstr << " (" << (-error)
                  << ")" << endl;
        return error;
    }

    cb->mActive = false;
    status_t res = result->waitForResult();
#if DEBUG
    ALOGD("result=%d", (int)res);
#endif
    return res;
}
