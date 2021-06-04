/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_EFFECT_HAL_HIDL_H
#define ANDROID_HARDWARE_EFFECT_HAL_HIDL_H

#include PATH(android/hardware/audio/effect/FILE_VERSION/IEffect.h)
#include <media/audiohal/EffectHalInterface.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>
#include <system/audio_effect.h>

using ::android::hardware::EventFlag;
using ::android::hardware::MessageQueue;

namespace android {
namespace effect {
namespace CPP_VERSION {

using namespace ::android::hardware::audio::effect::CPP_VERSION;

class EffectHalHidl : public EffectHalInterface
{
  public:
    // Set the input buffer.
    virtual status_t setInBuffer(const sp<EffectBufferHalInterface>& buffer);

    // Set the output buffer.
    virtual status_t setOutBuffer(const sp<EffectBufferHalInterface>& buffer);

    // Effect process function.
    virtual status_t process();

    // Process reverse stream function. This function is used to pass
    // a reference stream to the effect engine.
    virtual status_t processReverse();

    // Send a command and receive a response to/from effect engine.
    virtual status_t command(uint32_t cmdCode, uint32_t cmdSize, void *pCmdData,
            uint32_t *replySize, void *pReplyData);

    // Returns the effect descriptor.
    virtual status_t getDescriptor(effect_descriptor_t *pDescriptor);

    // Free resources on the remote side.
    virtual status_t close();

    // Whether it's a local implementation.
    virtual bool isLocal() const { return false; }

    virtual status_t dump(int fd);

    uint64_t effectId() const { return mEffectId; }

    static void effectDescriptorToHal(
            const EffectDescriptor& descriptor, effect_descriptor_t* halDescriptor);

  private:
    friend class EffectsFactoryHalHidl;
    typedef MessageQueue<Result, hardware::kSynchronizedReadWrite> StatusMQ;

    sp<IEffect> mEffect;
    const uint64_t mEffectId;
    sp<EffectBufferHalInterface> mInBuffer;
    sp<EffectBufferHalInterface> mOutBuffer;
    bool mBuffersChanged;
    std::unique_ptr<StatusMQ> mStatusMQ;
    EventFlag* mEfGroup;

    static status_t analyzeResult(const Result& result);
    static void effectBufferConfigFromHal(
            const buffer_config_t& halConfig, EffectBufferConfig* config);
    static void effectBufferConfigToHal(
            const EffectBufferConfig& config, buffer_config_t* halConfig);
    static void effectConfigFromHal(const effect_config_t& halConfig, EffectConfig* config);
    static void effectConfigToHal(const EffectConfig& config, effect_config_t* halConfig);

    // Can not be constructed directly by clients.
    EffectHalHidl(const sp<IEffect>& effect, uint64_t effectId);

    // The destructor automatically releases the effect.
    virtual ~EffectHalHidl();

    status_t getConfigImpl(uint32_t cmdCode, uint32_t *replySize, void *pReplyData);
    status_t prepareForProcessing();
    bool needToResetBuffers();
    status_t processImpl(uint32_t mqFlag);
    status_t setConfigImpl(
            uint32_t cmdCode, uint32_t cmdSize, void *pCmdData,
            uint32_t *replySize, void *pReplyData);
    status_t setProcessBuffers();
};

} // namespace CPP_VERSION
} // namespace effect
} // namespace android

#endif // ANDROID_HARDWARE_EFFECT_HAL_HIDL_H
