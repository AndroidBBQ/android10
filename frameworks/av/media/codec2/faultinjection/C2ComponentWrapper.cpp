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

#define LOG_NDEBUG 0
#define LOG_TAG "C2ComponentWrapper"

#include <chrono>
#include <functional>
#include <thread>

#include <C2ComponentWrapper.h>
#include <C2Config.h>
#include <C2PlatformSupport.h>

namespace android {

namespace {

using namespace std::chrono_literals;

c2_status_t WrapSimpleMethod(
        std::function<c2_status_t(void)> op, const SimpleMethodState &state) {
    c2_status_t result = C2_OK;
    switch (state.getMode()) {
        case SimpleMethodState::EXECUTE:
            result = op();
            break;
        case SimpleMethodState::NO_OP:
            break;
        case SimpleMethodState::HANG:
            while (true) {
                std::this_thread::sleep_for(1s);
            }
            break;
    }
    (void)state.overrideResult(&result);
    return result;
}

}  // namespace

C2ComponentWrapper::Injecter::Injecter(C2ComponentWrapper *thiz) : mThiz(thiz) {}

SimpleMethodState::Injecter C2ComponentWrapper::Injecter::start() {
    return SimpleMethodState::Injecter(&mThiz->mStartState);
}

C2ComponentWrapper::Listener::Listener(
        const std::shared_ptr<C2Component::Listener> &listener) : mListener(listener) {}

void C2ComponentWrapper::Listener::onWorkDone_nb(std::weak_ptr<C2Component> component,
        std::list<std::unique_ptr<C2Work>> workItems) {
    mListener->onWorkDone_nb(component, std::move(workItems));
}

void C2ComponentWrapper::Listener::onTripped_nb(std::weak_ptr<C2Component> component,
        std::vector<std::shared_ptr<C2SettingResult>> settingResult) {
    mListener->onTripped_nb(component,settingResult);
}

void C2ComponentWrapper::Listener::onError_nb(
        std::weak_ptr<C2Component> component, uint32_t errorCode) {
    mListener->onError_nb(component, errorCode);
}

C2ComponentWrapper::C2ComponentWrapper(
        const std::shared_ptr<C2Component> &comp) : mComp(comp) {}

c2_status_t C2ComponentWrapper::setListener_vb(
        const std::shared_ptr<C2Component::Listener> &listener, c2_blocking_t mayBlock) {
    mListener = std::make_shared<Listener>(listener);
    return mComp->setListener_vb(mListener, mayBlock);
}

c2_status_t C2ComponentWrapper::queue_nb(std::list<std::unique_ptr<C2Work>>* const items) {
    return mComp->queue_nb(items);
}

c2_status_t C2ComponentWrapper::announce_nb(const std::vector<C2WorkOutline> &items) {
    return mComp->announce_nb(items);
}

c2_status_t C2ComponentWrapper::flush_sm(
        C2Component::flush_mode_t mode, std::list<std::unique_ptr<C2Work>>* const flushedWork) {
    return mComp->flush_sm(mode, flushedWork);
}

c2_status_t C2ComponentWrapper::drain_nb(C2Component::drain_mode_t mode) {
    return mComp->drain_nb(mode);
}

c2_status_t C2ComponentWrapper::start() {
    return WrapSimpleMethod([this] { return mComp->start(); }, mStartState);
}

c2_status_t C2ComponentWrapper::stop() {
    return mComp->stop();
}

c2_status_t C2ComponentWrapper::reset() {
    return mComp->reset();
}

c2_status_t C2ComponentWrapper::release() {
    return mComp->release();
}

std::shared_ptr<C2ComponentInterface> C2ComponentWrapper::intf(){
    return mComp->intf();
}

C2ComponentWrapper::Injecter C2ComponentWrapper::inject() {
    return Injecter(this);
}

}  // namespace android
