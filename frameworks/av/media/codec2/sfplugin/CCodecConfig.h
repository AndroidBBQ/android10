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

#ifndef C_CODEC_CONFIG_H_
#define C_CODEC_CONFIG_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <C2Component.h>
#include <codec2/hidl/client.h>

#include <utils/RefBase.h>

#include "InputSurfaceWrapper.h"
#include "ReflectedParamUpdater.h"

namespace android {

struct AMessage;
struct StandardParams;

/**
 * Struct managing the codec configuration for CCodec.
 */
struct CCodecConfig {

    /**
     * Domain consists of a bitmask divided into fields, and specifiers work by excluding other
     * values in those domains.
     *
     * Component domains are composed by or-ing the individual IS_ constants, e.g.
     * IS_DECODER | IS_AUDIO.
     *
     * Config specifiers are composed by or-ing the individual mask constants, and
     * and-ing these groups: e.g. (DECODER | ENCODER) & AUDIO.
     *
     * The naming of these constants was to limit the length of mask names as these are used more
     * commonly as masks.
     */
    enum Domain : uint32_t {
        // component domain (domain & kind)
        GUARD_BIT   = (1 << 1),   ///< this is to prevent against accidental && or || usage
        IS_AUDIO    = (1 << 2),   ///< for audio codecs
        IS_VIDEO    = (1 << 3),   ///< for video codecs
        IS_IMAGE    = (1 << 4),   ///< for image codecs
        OTHER_DOMAIN = (1 << 5),  ///< for other domains

        IS_ENCODER  = (1 << 6),   ///< for encoders
        IS_DECODER  = (1 << 7),   ///< for decoders
        OTHER_KIND  = (1 << 8),   ///< for other domains

        // config domain
        IS_PARAM    = (1 << 9),   ///< for setParameter
        IS_CONFIG   = (1 << 10),  ///< for configure
        IS_READ     = (1 << 11),  ///< for getFormat

        // port domain
        IS_INPUT    = (1 << 12),  ///< for input port (getFormat)
        IS_OUTPUT   = (1 << 13),  ///< for output port (getFormat)
        IS_RAW      = (1 << 14),  ///< for raw port (input-encoder, output-decoder)
        IS_CODED    = (1 << 15),  ///< for coded port (input-decoder, output-encoder)

        ALL     = ~0U,
        NONE    = 0,

        AUDIO   = ~(IS_IMAGE | IS_VIDEO | OTHER_DOMAIN),
        VIDEO   = ~(IS_AUDIO | IS_IMAGE | OTHER_DOMAIN),
        IMAGE   = ~(IS_AUDIO | IS_VIDEO | OTHER_DOMAIN),

        DECODER = ~(IS_ENCODER | OTHER_KIND),
        ENCODER = ~(IS_DECODER | OTHER_KIND),

        PARAM   = ~(IS_CONFIG | IS_READ),
        CONFIG  = ~(IS_PARAM  | IS_READ),
        READ    = ~(IS_CONFIG | IS_PARAM),

        INPUT   = ~(IS_OUTPUT | IS_RAW    | IS_CODED),
        OUTPUT  = ~(IS_INPUT  | IS_RAW    | IS_CODED),
        RAW     = ~(IS_INPUT  | IS_OUTPUT | IS_CODED),
        CODED   = ~(IS_INPUT  | IS_RAW    | IS_OUTPUT),
    };

    // things required to manage formats
    std::vector<std::shared_ptr<C2ParamDescriptor>> mParamDescs;
    std::shared_ptr<C2ParamReflector> mReflector;

    std::shared_ptr<ReflectedParamUpdater> mParamUpdater;

    Domain mDomain; // component domain
    Domain mInputDomain; // input port domain
    Domain mOutputDomain; // output port domain
    std::string mCodingMediaType;  // media type of the coded stream

    // standard MediaCodec to Codec 2.0 params mapping
    std::shared_ptr<StandardParams> mStandardParams;

    std::set<C2Param::Index> mSupportedIndices; ///< indices supported by the component
    std::set<C2Param::Index> mSubscribedIndices; ///< indices to subscribe to
    size_t mSubscribedIndicesSize; ///< count of currently subscribed indices

    sp<AMessage> mInputFormat;
    sp<AMessage> mOutputFormat;

    bool mUsingSurface; ///< using input or output surface

    std::shared_ptr<InputSurfaceWrapper> mInputSurface;
    std::unique_ptr<InputSurfaceWrapper::Config> mISConfig;

    /// the current configuration. Updated after configure() and based on configUpdate in
    /// onWorkDone
    std::map<C2Param::Index, std::unique_ptr<C2Param>> mCurrentConfig;

    typedef std::function<c2_status_t(std::unique_ptr<C2Param>&)> LocalParamValidator;

    /// Parameter indices tracked in current config that are not supported by the component.
    /// these are provided so that optional parameters can remain in the current configuration.
    /// as such, these parameters have no dependencies. TODO: use C2InterfaceHelper for this.
    /// For now support a validation function.
    std::map<C2Param::Index, LocalParamValidator> mLocalParams;

    std::set<std::string> mLastConfig;

    CCodecConfig();

    /// initializes the members required to manage the format: descriptors, reflector,
    /// reflected param helper, domain, standard params, and subscribes to standard
    /// indices.
    status_t initialize(
            const std::shared_ptr<Codec2Client> &client,
            const std::shared_ptr<Codec2Client::Component> &component);


    /**
     * Adds a locally maintained parameter. This is used for output configuration that can be
     * appended to the output buffers in case it is not supported by the component.
     */
    template<typename T>
    bool addLocalParam(
            const std::string &name,
            C2ParamDescriptor::attrib_t attrib = C2ParamDescriptor::IS_READ_ONLY,
            std::function<c2_status_t(std::unique_ptr<T>&)> validator_ =
                std::function<c2_status_t(std::unique_ptr<T>&)>()) {
        C2Param::Index index = T::PARAM_TYPE;
        if (mSupportedIndices.count(index) || mLocalParams.count(index)) {
            if (mSupportedIndices.count(index)) {
                mSubscribedIndices.emplace(index);
            }
            ALOGD("ignoring local param %s (%#x) as it is already %s",
                    name.c_str(), (uint32_t)index, mSupportedIndices.count(index) ? "supported" : "local");
            return false; // already supported by the component or already added
        }

        // wrap typed validator into untyped validator
        LocalParamValidator validator;
        if (validator_) {
            validator = [validator_](std::unique_ptr<C2Param>& p){
                c2_status_t res = C2_BAD_VALUE;
                std::unique_ptr<T> typed(static_cast<T*>(p.release()));
                // if parameter is correctly typed
                if (T::From(typed.get())) {
                    res = validator_(typed);
                    p.reset(typed.release());
                }
                return res;
            };
        }

        mLocalParams.emplace(index, validator);
        mParamUpdater->addStandardParam<T>(name, attrib);
        return true;
    }

    /**
     * Adds a locally maintained parameter with a default value.
     */
    template<typename T>
    bool addLocalParam(
            std::unique_ptr<T> default_,
            const std::string &name,
            C2ParamDescriptor::attrib_t attrib = C2ParamDescriptor::IS_READ_ONLY,
            std::function<c2_status_t(std::unique_ptr<T>&)> validator_ =
                std::function<c2_status_t(std::unique_ptr<T>&)>()) {
        if (addLocalParam<T>(name, attrib, validator_)) {
            if (validator_) {
                c2_status_t err = validator_(default_);
                if (err != C2_OK) {
                    ALOGD("default value for %s is invalid => %s", name.c_str(), asString(err));
                    return false;
                }
            }
            mCurrentConfig[T::PARAM_TYPE] = std::move(default_);
            return true;
        }
        return false;
    }

    template<typename T>
    bool addLocalParam(
            T *default_, const std::string &name,
            C2ParamDescriptor::attrib_t attrib = C2ParamDescriptor::IS_READ_ONLY,
            std::function<c2_status_t(std::unique_ptr<T>&)> validator_ =
                std::function<c2_status_t(std::unique_ptr<T>&)>()) {
        return addLocalParam(std::unique_ptr<T>(default_), name, attrib, validator_);
    }

    /// Applies configuration updates, and updates format in the specific domain.
    /// Returns true if formats were updated
    /// \param domain input/output bitmask
    bool updateConfiguration(
            std::vector<std::unique_ptr<C2Param>> &configUpdate, Domain domain);

    /// Updates formats in the specific domain. Returns true if any of the formats have changed.
    /// \param domain input/output bitmask
    bool updateFormats(Domain domain);

    /**
     * Applies SDK configurations in a specific configuration domain.
     * Updates relevant input/output formats and subscribes to parameters specified in the
     * configuration.
     * \param domain config/setParam bitmask
     * \param blocking blocking mode to use with the component
     */
    status_t getConfigUpdateFromSdkParams(
            std::shared_ptr<Codec2Client::Component> component,
            const sp<AMessage> &sdkParams, Domain domain,
            c2_blocking_t blocking,
            std::vector<std::unique_ptr<C2Param>> *configUpdate) const;

    /**
     * Applies a configuration update to the component.
     * Updates relevant input/output formats and subscribes to parameters specified in the
     * configuration.
     * \param blocking blocking mode to use with the component
     */
    status_t setParameters(
            std::shared_ptr<Codec2Client::Component> component,
            std::vector<std::unique_ptr<C2Param>> &configUpdate,
            c2_blocking_t blocking);

    /// Queries subscribed indices (which contains all SDK-exposed values) and updates
    /// input/output formats.
    status_t queryConfiguration(
            const std::shared_ptr<Codec2Client::Component> &component);

    /// Queries a configuration parameter value. Returns nullptr if the parameter is not
    /// part of the current configuration
    const C2Param *getConfigParameterValue(C2Param::Index index) const;

    /**
     * Object that can be used to access configuration parameters and if they change.
     */
    template<typename T>
    struct Watcher {
        ~Watcher() = default;

        /// returns true if the value of this configuration has changed
        bool hasChanged() const {
            const C2Param *value = mParent->getConfigParameterValue(mIndex);
            if (value && mValue) {
                return *value != *mValue;
            } else {
                return value != mValue.get();
            }
        }

        /// updates the current value and returns it
        std::shared_ptr<const T> update() {
            const C2Param *value = mParent->getConfigParameterValue(mIndex);
            if (value) {
                mValue = std::shared_ptr<const T>(T::From(C2Param::Copy(*value).release()));
            }
            return mValue;
        }

    private:
        Watcher(C2Param::Index index, const CCodecConfig *parent)
            : mParent(parent), mIndex(index) {
            update();
        }

        friend struct CCodecConfig;

        const CCodecConfig *mParent;
        std::shared_ptr<const T> mValue;
        C2Param::Index mIndex;
    };

    /**
     * Returns a watcher object for a parameter.
     */
    template<typename T>
    Watcher<T> watch(C2Param::Index index = T::PARAM_TYPE) const {
        if (index.type() != T::PARAM_TYPE) {
            __builtin_trap();
        }
        return Watcher<T>(index, this);
    }

private:

    /// initializes the standard MediaCodec to Codec 2.0 params mapping
    void initializeStandardParams();

    /// Adds indices to the subscribed indices, and updated subscription to component
    /// \param blocking blocking mode to use with the component
    status_t subscribeToConfigUpdate(
            const std::shared_ptr<Codec2Client::Component> &component,
            const std::vector<C2Param::Index> &indices,
            c2_blocking_t blocking = C2_DONT_BLOCK);

    /// Gets SDK format from codec 2.0 reflected configuration
    /// \param domain input/output bitmask
    sp<AMessage> getSdkFormatForDomain(
            const ReflectedParamUpdater::Dict &reflected, Domain domain) const;

    /**
     * Converts a set of configuration parameters in an AMessage to a list of path-based Codec
     * 2.0 configuration parameters.
     *
     * \param domain config/setParam bitmask
     */
    ReflectedParamUpdater::Dict getReflectedFormat(
            const sp<AMessage> &config, Domain domain) const;
};

DEFINE_ENUM_OPERATORS(CCodecConfig::Domain)

}  // namespace android

#endif  // C_CODEC_H_

