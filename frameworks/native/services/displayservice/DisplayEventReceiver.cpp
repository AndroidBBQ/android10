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

#define LOG_TAG "libdisplayservicehidl"

#include <displayservice/DisplayEventReceiver.h>

#include <android-base/logging.h>
#include <android/frameworks/displayservice/1.0/BpHwEventCallback.h>

#include <thread>

namespace android {
namespace frameworks {
namespace displayservice {
namespace V1_0 {
namespace implementation {

sp<Looper> getLooper() {
    static sp<Looper> looper = []() {
        sp<Looper> looper = new Looper(false /* allowNonCallbacks */);

        std::thread{[&](){
            int pollResult = looper->pollAll(-1 /* timeout */);
            LOG(ERROR) << "Looper::pollAll returns unexpected " << pollResult;
        }}.detach();

        return looper;
    }();

    return looper;
}

DisplayEventReceiver::AttachedEvent::AttachedEvent(const sp<IEventCallback> &callback)
    : mCallback(callback)
{
    mLooperAttached = getLooper()->addFd(mFwkReceiver.getFd(),
            Looper::POLL_CALLBACK,
            Looper::EVENT_INPUT,
            this,
            nullptr);
}

DisplayEventReceiver::AttachedEvent::~AttachedEvent() {
    if (!detach()) {
        LOG(ERROR) << "Could not remove fd from looper.";
    }
}

bool DisplayEventReceiver::AttachedEvent::detach() {
    if (!valid()) {
        return true;
    }

    return getLooper()->removeFd(mFwkReceiver.getFd());
}

bool DisplayEventReceiver::AttachedEvent::valid() const {
    return mFwkReceiver.initCheck() == OK && mLooperAttached;
}

DisplayEventReceiver::FwkReceiver &DisplayEventReceiver::AttachedEvent::receiver() {
    return mFwkReceiver;
}

int DisplayEventReceiver::AttachedEvent::handleEvent(int fd, int events, void* /* data */) {
    CHECK(fd == mFwkReceiver.getFd());

    if (events & (Looper::EVENT_ERROR | Looper::EVENT_HANGUP)) {
        LOG(ERROR) << "AttachedEvent handleEvent received error or hangup:" << events;
        return 0; // remove the callback
    }

    if (!(events & Looper::EVENT_INPUT)) {
        LOG(ERROR) << "AttachedEvent handleEvent unhandled poll event:" << events;
        return 1; // keep the callback
    }

    constexpr size_t SIZE = 1;

    ssize_t n;
    FwkReceiver::Event buf[SIZE];
    while ((n = mFwkReceiver.getEvents(buf, SIZE)) > 0) {
        for (size_t i = 0; i < static_cast<size_t>(n); ++i) {
            const FwkReceiver::Event &event = buf[i];

            uint32_t type = event.header.type;
            uint64_t timestamp = event.header.timestamp;

            switch(buf[i].header.type) {
                case FwkReceiver::DISPLAY_EVENT_VSYNC: {
                    auto ret = mCallback->onVsync(timestamp, event.vsync.count);
                    if (!ret.isOk()) {
                        LOG(ERROR) << "AttachedEvent handleEvent fails on onVsync callback"
                                   << " because of " << ret.description();
                        return 0;  // remove the callback
                    }
                } break;
                case FwkReceiver::DISPLAY_EVENT_HOTPLUG: {
                    auto ret = mCallback->onHotplug(timestamp, event.hotplug.connected);
                    if (!ret.isOk()) {
                        LOG(ERROR) << "AttachedEvent handleEvent fails on onHotplug callback"
                                   << " because of " << ret.description();
                        return 0;  // remove the callback
                    }
                } break;
                default: {
                    LOG(ERROR) << "AttachedEvent handleEvent unknown type: " << type;
                }
            }
        }
    }

    return 1; // keep on going
}

Return<Status> DisplayEventReceiver::init(const sp<IEventCallback>& callback) {
    std::unique_lock<std::mutex> lock(mMutex);

    if (mAttached != nullptr || callback == nullptr) {
        return Status::BAD_VALUE;
    }

    mAttached = new AttachedEvent(callback);

    return mAttached->valid() ? Status::SUCCESS : Status::UNKNOWN;
}

Return<Status> DisplayEventReceiver::setVsyncRate(int32_t count) {
    std::unique_lock<std::mutex> lock(mMutex);

    if (mAttached == nullptr || count < 0) {
        return Status::BAD_VALUE;
    }

    bool success = OK == mAttached->receiver().setVsyncRate(count);
    return success ? Status::SUCCESS : Status::UNKNOWN;
}

Return<Status> DisplayEventReceiver::requestNextVsync() {
    std::unique_lock<std::mutex> lock(mMutex);

    if (mAttached == nullptr) {
        return Status::BAD_VALUE;
    }

    bool success = OK == mAttached->receiver().requestNextVsync();
    return success ? Status::SUCCESS : Status::UNKNOWN;
}

Return<Status> DisplayEventReceiver::close() {
    std::unique_lock<std::mutex> lock(mMutex);
    if (mAttached == nullptr) {
        return Status::BAD_VALUE;
    }

    bool success = mAttached->detach();
    mAttached = nullptr;

    return success ? Status::SUCCESS : Status::UNKNOWN;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace displayservice
}  // namespace frameworks
}  // namespace android
