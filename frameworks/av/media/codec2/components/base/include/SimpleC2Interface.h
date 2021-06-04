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

#ifndef ANDROID_SIMPLE_C2_INTERFACE_H_
#define ANDROID_SIMPLE_C2_INTERFACE_H_

#include <C2Component.h>
#include <C2Config.h>
#include <util/C2InterfaceHelper.h>

namespace android {

/**
 * Wrap a common interface object (such as Codec2Client::Interface, or C2InterfaceHelper into
 * a C2ComponentInterface.
 *
 * \param T common interface type
 */
template <typename T>
class SimpleC2Interface : public C2ComponentInterface {
public:
    SimpleC2Interface(const char *name, c2_node_id_t id, const std::shared_ptr<T> &impl)
        : mName(name),
          mId(id),
          mImpl(impl) {
    }

    ~SimpleC2Interface() override = default;

    // From C2ComponentInterface
    C2String getName() const override { return mName; }
    c2_node_id_t getId() const override { return mId; }
    c2_status_t query_vb(
            const std::vector<C2Param*> &stackParams,
            const std::vector<C2Param::Index> &heapParamIndices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const heapParams) const override {
        return mImpl->query(stackParams, heapParamIndices, mayBlock, heapParams);
    }
    c2_status_t config_vb(
            const std::vector<C2Param*> &params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures) override {
        return mImpl->config(params, mayBlock, failures);
    }
    c2_status_t createTunnel_sm(c2_node_id_t) override { return C2_OMITTED; }
    c2_status_t releaseTunnel_sm(c2_node_id_t) override { return C2_OMITTED; }
    c2_status_t querySupportedParams_nb(
            std::vector<std::shared_ptr<C2ParamDescriptor>> * const params) const override {
        return mImpl->querySupportedParams(params);
    }
    c2_status_t querySupportedValues_vb(
            std::vector<C2FieldSupportedValuesQuery> &fields,
            c2_blocking_t mayBlock) const override {
        return mImpl->querySupportedValues(fields, mayBlock);
    }

private:
    C2String mName;
    const c2_node_id_t mId;
    const std::shared_ptr<T> mImpl;
};

/**
 * Utility classes for common interfaces.
 */
template<>
class SimpleC2Interface<void> {
public:
    /**
     * Base Codec 2.0 parameters required for all components.
     */
    struct BaseParams : C2InterfaceHelper {
        explicit BaseParams(
                const std::shared_ptr<C2ReflectorHelper> &helper,
                C2String name,
                C2Component::kind_t kind,
                C2Component::domain_t domain,
                C2String mediaType,
                std::vector<C2String> aliases = std::vector<C2String>());

        /// Marks that this component has no input latency. Otherwise, component must
        /// add support for C2PortRequestedDelayTuning::input and C2PortActualDelayTuning::input.
        void noInputLatency();

        /// Marks that this component has no output latency. Otherwise, component must
        /// add support for C2PortRequestedDelayTuning::output and C2PortActualDelayTuning::output.
        void noOutputLatency();

        /// Marks that this component has no pipeline latency. Otherwise, component must
        /// add support for C2RequestedPipelineDelayTuning and C2ActualPipelineDelayTuning.
        void noPipelineLatency();

        /// Marks that this component has no need for private buffers. Otherwise, component must
        /// add support for C2MaxPrivateBufferCountTuning, C2PrivateAllocatorsTuning and
        /// C2PrivateBlockPoolsTuning.
        void noPrivateBuffers();

        /// Marks that this component holds no references to input buffers. Otherwise, component
        /// must add support for C2StreamMaxReferenceAgeTuning::input and
        /// C2StreamMaxReferenceCountTuning::input.
        void noInputReferences();

        /// Marks that this component holds no references to output buffers. Otherwise, component
        /// must add support for C2StreamMaxReferenceAgeTuning::output and
        /// C2StreamMaxReferenceCountTuning::output.
        void noOutputReferences();

        /// Marks that this component does not stretch time. Otherwise, component
        /// must add support for C2ComponentTimeStretchTuning.
        void noTimeStretch();

        std::shared_ptr<C2ApiLevelSetting> mApiLevel;
        std::shared_ptr<C2ApiFeaturesSetting> mApiFeatures;

        std::shared_ptr<C2PlatformLevelSetting> mPlatformLevel;
        std::shared_ptr<C2PlatformFeaturesSetting> mPlatformFeatures;

        std::shared_ptr<C2ComponentNameSetting> mName;
        std::shared_ptr<C2ComponentAliasesSetting> mAliases;
        std::shared_ptr<C2ComponentKindSetting> mKind;
        std::shared_ptr<C2ComponentDomainSetting> mDomain;
        std::shared_ptr<C2ComponentAttributesSetting> mAttrib;
        std::shared_ptr<C2ComponentTimeStretchTuning> mTimeStretch;

        std::shared_ptr<C2PortMediaTypeSetting::input> mInputMediaType;
        std::shared_ptr<C2PortMediaTypeSetting::output> mOutputMediaType;
        std::shared_ptr<C2StreamBufferTypeSetting::input> mInputFormat;
        std::shared_ptr<C2StreamBufferTypeSetting::output> mOutputFormat;

        std::shared_ptr<C2PortRequestedDelayTuning::input> mRequestedInputDelay;
        std::shared_ptr<C2PortRequestedDelayTuning::output> mRequestedOutputDelay;
        std::shared_ptr<C2RequestedPipelineDelayTuning> mRequestedPipelineDelay;

        std::shared_ptr<C2PortActualDelayTuning::input> mActualInputDelay;
        std::shared_ptr<C2PortActualDelayTuning::output> mActualOutputDelay;
        std::shared_ptr<C2ActualPipelineDelayTuning> mActualPipelineDelay;

        std::shared_ptr<C2StreamMaxReferenceAgeTuning::input> mMaxInputReferenceAge;
        std::shared_ptr<C2StreamMaxReferenceCountTuning::input> mMaxInputReferenceCount;
        std::shared_ptr<C2StreamMaxReferenceAgeTuning::output> mMaxOutputReferenceAge;
        std::shared_ptr<C2StreamMaxReferenceCountTuning::output> mMaxOutputReferenceCount;
        std::shared_ptr<C2MaxPrivateBufferCountTuning> mMaxPrivateBufferCount;

        std::shared_ptr<C2PortStreamCountTuning::input> mInputStreamCount;
        std::shared_ptr<C2PortStreamCountTuning::output> mOutputStreamCount;

        std::shared_ptr<C2SubscribedParamIndicesTuning> mSubscribedParamIndices;
        std::shared_ptr<C2PortSuggestedBufferCountTuning::input> mSuggestedInputBufferCount;
        std::shared_ptr<C2PortSuggestedBufferCountTuning::output> mSuggestedOutputBufferCount;

        std::shared_ptr<C2CurrentWorkTuning> mCurrentWorkOrdinal;
        std::shared_ptr<C2LastWorkQueuedTuning::input> mLastInputQueuedWorkOrdinal;
        std::shared_ptr<C2LastWorkQueuedTuning::output> mLastOutputQueuedWorkOrdinal;

        std::shared_ptr<C2PortAllocatorsTuning::input> mInputAllocators;
        std::shared_ptr<C2PortAllocatorsTuning::output> mOutputAllocators;
        std::shared_ptr<C2PrivateAllocatorsTuning> mPrivateAllocators;
        std::shared_ptr<C2PortBlockPoolsTuning::output> mOutputPoolIds;
        std::shared_ptr<C2PrivateBlockPoolsTuning> mPrivatePoolIds;

        std::shared_ptr<C2TrippedTuning> mTripped;
        std::shared_ptr<C2OutOfMemoryTuning> mOutOfMemory;

        std::shared_ptr<C2PortConfigCounterTuning::input> mInputConfigCounter;
        std::shared_ptr<C2PortConfigCounterTuning::output> mOutputConfigCounter;
        std::shared_ptr<C2ConfigCounterTuning> mDirectConfigCounter;
    };
};

template<typename T>
using SimpleInterface = SimpleC2Interface<T>;

template<typename T, typename ...Args>
std::shared_ptr<T> AllocSharedString(const Args(&... args), const char *str) {
    size_t len = strlen(str) + 1;
    std::shared_ptr<T> ret = T::AllocShared(len, args...);
    strcpy(ret->m.value, str);
    return ret;
}

template<typename T, typename ...Args>
std::shared_ptr<T> AllocSharedString(const Args(&... args), const std::string &str) {
    std::shared_ptr<T> ret = T::AllocShared(str.length() + 1, args...);
    strcpy(ret->m.value, str.c_str());
    return ret;
}

template <typename T>
struct Setter {
    typedef typename std::remove_reference<T>::type type;

    static C2R NonStrictValueWithNoDeps(
            bool mayBlock, C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        return me.F(me.v.value).validatePossible(me.v.value);
    }

    static C2R NonStrictValuesWithNoDeps(
            bool mayBlock, C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        for (size_t ix = 0; ix < me.v.flexCount(); ++ix) {
            res.plus(me.F(me.v.m.values[ix]).validatePossible(me.v.m.values[ix]));
        }
        return res;
    }

    static C2R StrictValueWithNoDeps(
            bool mayBlock,
            const C2InterfaceHelper::C2P<type> &old,
            C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        if (!me.F(me.v.value).supportsNow(me.v.value)) {
            me.set().value = old.v.value;
        }
        return me.F(me.v.value).validatePossible(me.v.value);
    }
};

}  // namespace android

#endif  // ANDROID_SIMPLE_C2_INTERFACE_H_
