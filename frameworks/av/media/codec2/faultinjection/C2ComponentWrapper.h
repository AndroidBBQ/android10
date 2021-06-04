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

#ifndef C2_COMPONENT_WRAPPER_H_
#define C2_COMPONENT_WRAPPER_H_

#include <C2Component.h>

#include "SimpleMethodState.h"

namespace android {

/**
 * Creates a Wrapper around the class C2Component and its methods. The wrapper is used to
 * simulate errors in the android media components by fault injection technique.
 * This is done to check how the framework handles the error situation.
 */
class C2ComponentWrapper
        : public C2Component, public std::enable_shared_from_this<C2ComponentWrapper> {
public:
    class Injecter {
    public:
        explicit Injecter(C2ComponentWrapper *thiz);

        SimpleMethodState::Injecter start();
    private:
        C2ComponentWrapper *const mThiz;
    };

    /**
     * A wrapper around the listener class inside C2Component class.
     */
    class Listener : public C2Component::Listener {
    public:
        explicit Listener(const std::shared_ptr<C2Component::Listener> &listener);
        virtual ~Listener() = default;

        void onWorkDone_nb(std::weak_ptr<C2Component> component,
                           std::list<std::unique_ptr<C2Work>> workItems) override;
        void onTripped_nb(std::weak_ptr<C2Component> component,
                          std::vector<std::shared_ptr<C2SettingResult>> settingResult) override;
        void onError_nb(std::weak_ptr<C2Component> component, uint32_t errorCode) override;

    private:
        std::shared_ptr<C2Component::Listener> mListener;
    };

    explicit C2ComponentWrapper(const std::shared_ptr<C2Component> &comp);
    virtual ~C2ComponentWrapper() = default;

    virtual c2_status_t setListener_vb(
            const std::shared_ptr<C2Component::Listener> &listener,
            c2_blocking_t mayBlock) override;
    virtual c2_status_t queue_nb(std::list<std::unique_ptr<C2Work>>* const items) override;
    virtual c2_status_t announce_nb(const std::vector<C2WorkOutline> &items) override;
    virtual c2_status_t flush_sm(
            flush_mode_t mode, std::list<std::unique_ptr<C2Work>>* const flushedWork) override;
    virtual c2_status_t drain_nb(drain_mode_t mode) override;
    virtual c2_status_t start() override;
    virtual c2_status_t stop() override;
    virtual c2_status_t reset() override;
    virtual c2_status_t release() override;
    virtual std::shared_ptr<C2ComponentInterface> intf() override;

    Injecter inject();

private:
    std::shared_ptr<Listener> mListener;
    std::shared_ptr<C2Component> mComp;

    SimpleMethodState mStartState;
};

}  // namespace android

#endif // C2_COMPONENT_WRAPPER_H_
