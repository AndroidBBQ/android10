/*
 * Copyright 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "android.hardware.media.c2@1.0-service"

#include <codec2/hidl/1.0/ComponentStore.h>
#include <hidl/HidlTransportSupport.h>
#include <binder/ProcessState.h>
#include <minijail.h>

#include <C2Component.h>

// OmxStore is added for visibility by dumpstate.
#include <media/stagefright/omx/1.0/OmxStore.h>

// This is created by module "codec2.vendor.base.policy". This can be modified.
static constexpr char kBaseSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.base.policy";

// Additional device-specific seccomp permissions can be added in this file.
static constexpr char kExtSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.ext.policy";

class DummyC2Store : public C2ComponentStore {
public:
    DummyC2Store() = default;

    virtual ~DummyC2Store() override = default;

    virtual C2String getName() const override {
        return "default";
    }

    virtual c2_status_t createComponent(
            C2String /*name*/,
            std::shared_ptr<C2Component>* const /*component*/) override {
        return C2_NOT_FOUND;
    }

    virtual c2_status_t createInterface(
            C2String /* name */,
            std::shared_ptr<C2ComponentInterface>* const /* interface */) override {
        return C2_NOT_FOUND;
    }

    virtual std::vector<std::shared_ptr<const C2Component::Traits>>
            listComponents() override {
        return {};
    }

    virtual c2_status_t copyBuffer(
            std::shared_ptr<C2GraphicBuffer> /* src */,
            std::shared_ptr<C2GraphicBuffer> /* dst */) override {
        return C2_OMITTED;
    }

    virtual c2_status_t query_sm(
        const std::vector<C2Param*>& /* stackParams */,
        const std::vector<C2Param::Index>& /* heapParamIndices */,
        std::vector<std::unique_ptr<C2Param>>* const /* heapParams */) const override {
        return C2_OMITTED;
    }

    virtual c2_status_t config_sm(
            const std::vector<C2Param*>& /* params */,
            std::vector<std::unique_ptr<C2SettingResult>>* const /* failures */) override {
        return C2_OMITTED;
    }

    virtual std::shared_ptr<C2ParamReflector> getParamReflector() const override {
        return nullptr;
    }

    virtual c2_status_t querySupportedParams_nb(
            std::vector<std::shared_ptr<C2ParamDescriptor>>* const /* params */) const override {
        return C2_OMITTED;
    }

    virtual c2_status_t querySupportedValues_sm(
            std::vector<C2FieldSupportedValuesQuery>& /* fields */) const override {
        return C2_OMITTED;
    }
};

int main(int /* argc */, char** /* argv */) {
    ALOGD("android.hardware.media.c2@1.0-service starting...");

    signal(SIGPIPE, SIG_IGN);
    android::SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);

    // vndbinder is needed by BufferQueue.
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::ProcessState::self()->startThreadPool();

    // Extra threads may be needed to handle a stacked IPC sequence that
    // contains alternating binder and hwbinder calls. (See b/35283480.)
    android::hardware::configureRpcThreadpool(8, true /* callerWillJoin */);

    // Create IComponentStore service.
    {
        using namespace ::android::hardware::media::c2::V1_0;
        android::sp<IComponentStore> store;

        // Vendor's TODO: Replace this with
        // store = new utils::ComponentStore(
        //         /* implementation of C2ComponentStore */);
        ALOGD("Instantiating Codec2's dummy IComponentStore service...");
        store = new utils::ComponentStore(
                std::make_shared<DummyC2Store>());

        if (store == nullptr) {
            ALOGE("Cannot create Codec2's IComponentStore service.");
        } else {
            if (store->registerAsService("default") != android::OK) {
                ALOGE("Cannot register Codec2's "
                        "IComponentStore service.");
            } else {
                ALOGI("Codec2's IComponentStore service created.");
            }
        }
    }

    // Register IOmxStore service.
    {
        using namespace ::android::hardware::media::omx::V1_0;
        android::sp<IOmxStore> omxStore = new implementation::OmxStore();
        if (omxStore == nullptr) {
            ALOGE("Cannot create IOmxStore HAL service.");
        } else if (omxStore->registerAsService() != android::OK) {
            ALOGE("Cannot register IOmxStore HAL service.");
        }
    }

    android::hardware::joinRpcThreadpool();
    return 0;
}
