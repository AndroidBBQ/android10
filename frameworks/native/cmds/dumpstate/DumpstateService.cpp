/**
 * Copyright (c) 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "dumpstate"

#include "DumpstateService.h"

#include <memory>

#include <android-base/stringprintf.h>
#include "android/os/BnDumpstate.h"

#include "DumpstateInternal.h"

using android::base::StringPrintf;

namespace android {
namespace os {

namespace {

struct DumpstateInfo {
  public:
    Dumpstate* ds = nullptr;
    int32_t calling_uid = -1;
    std::string calling_package;
};

static binder::Status exception(uint32_t code, const std::string& msg) {
    MYLOGE("%s (%d) ", msg.c_str(), code);
    return binder::Status::fromExceptionCode(code, String8(msg.c_str()));
}

// Creates a bugreport and exits, thus preserving the oneshot nature of the service.
// Note: takes ownership of data.
[[noreturn]] static void* dumpstate_thread_main(void* data) {
    std::unique_ptr<DumpstateInfo> ds_info(static_cast<DumpstateInfo*>(data));
    ds_info->ds->Run(ds_info->calling_uid, ds_info->calling_package);
    MYLOGD("Finished taking a bugreport. Exiting.\n");
    exit(0);
}

[[noreturn]] static void signalErrorAndExit(sp<IDumpstateListener> listener, int error_code) {
    listener->onError(error_code);
    exit(0);
}

class DumpstateToken : public BnDumpstateToken {};

}  // namespace

DumpstateService::DumpstateService() : ds_(nullptr) {
}

char const* DumpstateService::getServiceName() {
    return "dumpstate";
}

status_t DumpstateService::Start() {
    IPCThreadState::self()->disableBackgroundScheduling(true);
    status_t ret = BinderService<DumpstateService>::publish();
    if (ret != android::OK) {
        return ret;
    }
    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();
    ps->giveThreadPoolName();
    return android::OK;
}

// Note: this method is part of the old flow and is not expected to be used in combination
// with startBugreport.
binder::Status DumpstateService::setListener(const std::string& name,
                                             const sp<IDumpstateListener>& listener,
                                             bool getSectionDetails,
                                             sp<IDumpstateToken>* returned_token) {
    *returned_token = nullptr;
    if (name.empty()) {
        MYLOGE("setListener(): name not set\n");
        return binder::Status::ok();
    }
    if (listener == nullptr) {
        MYLOGE("setListener(): listener not set\n");
        return binder::Status::ok();
    }
    std::lock_guard<std::mutex> lock(lock_);
    if (ds_ == nullptr) {
        ds_ = &(Dumpstate::GetInstance());
    }
    if (ds_->listener_ != nullptr) {
        MYLOGE("setListener(%s): already set (%s)\n", name.c_str(), ds_->listener_name_.c_str());
        return binder::Status::ok();
    }

    ds_->listener_name_ = name;
    ds_->listener_ = listener;
    ds_->report_section_ = getSectionDetails;
    *returned_token = new DumpstateToken();

    return binder::Status::ok();
}

binder::Status DumpstateService::startBugreport(int32_t calling_uid,
                                                const std::string& calling_package,
                                                const android::base::unique_fd& bugreport_fd,
                                                const android::base::unique_fd& screenshot_fd,
                                                int bugreport_mode,
                                                const sp<IDumpstateListener>& listener) {
    MYLOGI("startBugreport() with mode: %d\n", bugreport_mode);

    // This is the bugreporting API flow, so ensure there is only one bugreport in progress at a
    // time.
    std::lock_guard<std::mutex> lock(lock_);
    if (ds_ != nullptr) {
        MYLOGE("Error! There is already a bugreport in progress. Returning.");
        if (listener != nullptr) {
            listener->onError(IDumpstateListener::BUGREPORT_ERROR_ANOTHER_REPORT_IN_PROGRESS);
        }
        return exception(binder::Status::EX_SERVICE_SPECIFIC,
                         "There is already a bugreport in progress");
    }

    // From here on, all conditions that indicate we are done with this incoming request should
    // result in exiting the service to free it up for next invocation.
    if (listener == nullptr) {
        MYLOGE("Invalid input: no listener");
        exit(0);
    }

    if (bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_FULL &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_INTERACTIVE &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_REMOTE &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_WEAR &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_TELEPHONY &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_WIFI &&
        bugreport_mode != Dumpstate::BugreportMode::BUGREPORT_DEFAULT) {
        MYLOGE("Invalid input: bad bugreport mode: %d", bugreport_mode);
        signalErrorAndExit(listener, IDumpstateListener::BUGREPORT_ERROR_INVALID_INPUT);
    }

    if (bugreport_fd.get() == -1 || screenshot_fd.get() == -1) {
        // TODO(b/111441001): screenshot fd should be optional
        MYLOGE("Invalid filedescriptor");
        signalErrorAndExit(listener, IDumpstateListener::BUGREPORT_ERROR_INVALID_INPUT);
    }

    std::unique_ptr<Dumpstate::DumpOptions> options = std::make_unique<Dumpstate::DumpOptions>();
    options->Initialize(static_cast<Dumpstate::BugreportMode>(bugreport_mode), bugreport_fd,
                        screenshot_fd);

    ds_ = &(Dumpstate::GetInstance());
    ds_->SetOptions(std::move(options));
    ds_->listener_ = listener;

    DumpstateInfo* ds_info = new DumpstateInfo();
    ds_info->ds = ds_;
    ds_info->calling_uid = calling_uid;
    ds_info->calling_package = calling_package;

    pthread_t thread;
    status_t err = pthread_create(&thread, nullptr, dumpstate_thread_main, ds_info);
    if (err != 0) {
        delete ds_info;
        ds_info = nullptr;
        MYLOGE("Could not create a thread");
        signalErrorAndExit(listener, IDumpstateListener::BUGREPORT_ERROR_RUNTIME_ERROR);
    }
    return binder::Status::ok();
}

binder::Status DumpstateService::cancelBugreport() {
    // This is a no-op since the cancellation is done from java side via setting sys properties.
    // See BugreportManagerServiceImpl.
    // TODO(b/111441001): maybe make native and java sides use different binder interface
    // to avoid these annoyances.
    return binder::Status::ok();
}

status_t DumpstateService::dump(int fd, const Vector<String16>&) {
    if (ds_ == nullptr) {
        dprintf(fd, "Bugreport not in progress yet");
        return NO_ERROR;
    }
    std::string destination = ds_->options_->bugreport_fd.get() != -1
                                  ? StringPrintf("[fd:%d]", ds_->options_->bugreport_fd.get())
                                  : ds_->bugreport_internal_dir_.c_str();
    dprintf(fd, "id: %d\n", ds_->id_);
    dprintf(fd, "pid: %d\n", ds_->pid_);
    dprintf(fd, "update_progress: %s\n", ds_->options_->do_progress_updates ? "true" : "false");
    dprintf(fd, "update_progress_threshold: %d\n", ds_->update_progress_threshold_);
    dprintf(fd, "last_updated_progress: %d\n", ds_->last_updated_progress_);
    dprintf(fd, "progress:\n");
    ds_->progress_->Dump(fd, "  ");
    dprintf(fd, "args: %s\n", ds_->options_->args.c_str());
    dprintf(fd, "extra_options: %s\n", ds_->options_->extra_options.c_str());
    dprintf(fd, "version: %s\n", ds_->version_.c_str());
    dprintf(fd, "bugreport_dir: %s\n", destination.c_str());
    dprintf(fd, "screenshot_path: %s\n", ds_->screenshot_path_.c_str());
    dprintf(fd, "log_path: %s\n", ds_->log_path_.c_str());
    dprintf(fd, "tmp_path: %s\n", ds_->tmp_path_.c_str());
    dprintf(fd, "path: %s\n", ds_->path_.c_str());
    dprintf(fd, "extra_options: %s\n", ds_->options_->extra_options.c_str());
    dprintf(fd, "base_name: %s\n", ds_->base_name_.c_str());
    dprintf(fd, "name: %s\n", ds_->name_.c_str());
    dprintf(fd, "now: %ld\n", ds_->now_);
    dprintf(fd, "is_zipping: %s\n", ds_->IsZipping() ? "true" : "false");
    dprintf(fd, "listener: %s\n", ds_->listener_name_.c_str());
    dprintf(fd, "notification title: %s\n", ds_->options_->notification_title.c_str());
    dprintf(fd, "notification description: %s\n", ds_->options_->notification_description.c_str());

    return NO_ERROR;
}
}  // namespace os
}  // namespace android
