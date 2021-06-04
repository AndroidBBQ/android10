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
#define LOG_TAG "Codec2-InputSurface"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/InputSurface.h>
#include <codec2/hidl/1.0/InputSurfaceConnection.h>

#include <C2Component.h>
#include <C2Config.h>

#include <memory>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;

// Derived class of C2InterfaceHelper
class InputSurface::Interface : public C2InterfaceHelper {
public:
    explicit Interface(
            const std::shared_ptr<C2ReflectorHelper> &helper)
        : C2InterfaceHelper(helper) {

        setDerivedInstance(this);

        addParameter(
                DefineParam(mEos, C2_PARAMKEY_INPUT_SURFACE_EOS)
                .withDefault(new C2InputSurfaceEosTuning(false))
                .withFields({C2F(mEos, value).oneOf({true, false})})
                .withSetter(EosSetter)
                .build());
    }

    static C2R EosSetter(bool mayBlock, C2P<C2InputSurfaceEosTuning> &me) {
        (void)mayBlock;
        return me.F(me.v.value).validatePossible(me.v.value);
    }

    bool eos() const { return mEos->value; }

private:
    std::shared_ptr<C2InputSurfaceEosTuning> mEos;
};

// Derived class of ConfigurableC2Intf
class InputSurface::ConfigurableIntf : public ConfigurableC2Intf {
public:
    ConfigurableIntf(
            const std::shared_ptr<InputSurface::Interface> &intf,
            const sp<GraphicBufferSource> &source)
        : ConfigurableC2Intf("input-surface", 0),
          mIntf(intf),
          mSource(source) {
    }

    virtual ~ConfigurableIntf() override = default;

    virtual c2_status_t query(
            const std::vector<C2Param::Index> &indices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const params
            ) const override {
        return mIntf->query({}, indices, mayBlock, params);
    }

    virtual c2_status_t config(
            const std::vector<C2Param*> &params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures
            ) override {
        c2_status_t err = mIntf->config(params, mayBlock, failures);
        if (mIntf->eos()) {
            sp<GraphicBufferSource> source = mSource.promote();
            if (source == nullptr || source->signalEndOfInputStream() != OK) {
                // TODO: put something in |failures|
                err = C2_BAD_VALUE;
            }
            // TODO: reset eos?
        }
        return err;
    }

    virtual c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>>* const params
            ) const override {
        return mIntf->querySupportedParams(params);
    }

    virtual c2_status_t querySupportedValues(
            std::vector<C2FieldSupportedValuesQuery>& fields,
            c2_blocking_t mayBlock) const override {
        return mIntf->querySupportedValues(fields, mayBlock);
    }

private:
    const std::shared_ptr<InputSurface::Interface> mIntf;
    wp<GraphicBufferSource> mSource;
};

Return<sp<InputSurface::HGraphicBufferProducer>> InputSurface::getGraphicBufferProducer() {
    return mProducer;
}

Return<sp<IConfigurable>> InputSurface::getConfigurable() {
    return mConfigurable;
}

Return<void> InputSurface::connect(
        const sp<IInputSink>& sink,
        connect_cb _hidl_cb) {
    Status status;
    sp<InputSurfaceConnection> connection;
    if (!sink) {
        _hidl_cb(Status::BAD_VALUE, nullptr);
        return Void();
    }
    std::shared_ptr<C2Component> comp = Component::findLocalComponent(sink);
    if (comp) {
        connection = new InputSurfaceConnection(mSource, comp, mStore);
    } else {
        connection = new InputSurfaceConnection(mSource, sink, mStore);
    }
    if (!connection->init()) {
        connection = nullptr;
        status = Status::BAD_VALUE;
    } else {
        status = Status::OK;
    }
    _hidl_cb(status, connection);
    return Void();
}

// Constructor is exclusive to ComponentStore.
InputSurface::InputSurface(
        const sp<ComponentStore>& store,
        const std::shared_ptr<C2ReflectorHelper>& reflector,
        const sp<HGraphicBufferProducer>& producer,
        const sp<GraphicBufferSource>& source)
      : mStore{store},
        mProducer{producer},
        mSource{source},
        mIntf{std::make_shared<Interface>(reflector)},
        mConfigurable{new CachedConfigurable(
                std::make_unique<ConfigurableIntf>(
                    mIntf, source))} {

    mConfigurable->init(store.get());
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

