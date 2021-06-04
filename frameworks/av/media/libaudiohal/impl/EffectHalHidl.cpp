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

#define LOG_TAG "EffectHalHidl"
//#define LOG_NDEBUG 0

#include <common/all-versions/VersionUtils.h>
#include <cutils/native_handle.h>
#include <hwbinder/IPCThreadState.h>
#include <media/EffectsFactoryApi.h>
#include <utils/Log.h>

#include "EffectBufferHalHidl.h"
#include "EffectHalHidl.h"
#include "HidlUtils.h"

using ::android::hardware::audio::common::CPP_VERSION::implementation::HidlUtils;
using ::android::hardware::audio::common::utils::EnumBitfield;
using ::android::hardware::hidl_vec;
using ::android::hardware::MQDescriptorSync;
using ::android::hardware::Return;

namespace android {
namespace effect {
namespace CPP_VERSION {

using namespace ::android::hardware::audio::common::CPP_VERSION;
using namespace ::android::hardware::audio::effect::CPP_VERSION;

EffectHalHidl::EffectHalHidl(const sp<IEffect>& effect, uint64_t effectId)
        : mEffect(effect), mEffectId(effectId), mBuffersChanged(true), mEfGroup(nullptr) {
}

EffectHalHidl::~EffectHalHidl() {
    if (mEffect != 0) {
        close();
        mEffect.clear();
        hardware::IPCThreadState::self()->flushCommands();
    }
    if (mEfGroup) {
        EventFlag::deleteEventFlag(&mEfGroup);
    }
}

// static
void EffectHalHidl::effectDescriptorToHal(
        const EffectDescriptor& descriptor, effect_descriptor_t* halDescriptor) {
    HidlUtils::uuidToHal(descriptor.type, &halDescriptor->type);
    HidlUtils::uuidToHal(descriptor.uuid, &halDescriptor->uuid);
    halDescriptor->flags = static_cast<uint32_t>(descriptor.flags);
    halDescriptor->cpuLoad = descriptor.cpuLoad;
    halDescriptor->memoryUsage = descriptor.memoryUsage;
    memcpy(halDescriptor->name, descriptor.name.data(), descriptor.name.size());
    memcpy(halDescriptor->implementor,
            descriptor.implementor.data(), descriptor.implementor.size());
}

// TODO(mnaganov): These buffer conversion functions should be shared with Effect wrapper
// via HidlUtils. Move them there when hardware/interfaces will get un-frozen again.

// static
void EffectHalHidl::effectBufferConfigFromHal(
        const buffer_config_t& halConfig, EffectBufferConfig* config) {
    config->samplingRateHz = halConfig.samplingRate;
    config->channels = EnumBitfield<AudioChannelMask>(halConfig.channels);
    config->format = AudioFormat(halConfig.format);
    config->accessMode = EffectBufferAccess(halConfig.accessMode);
    config->mask = EnumBitfield<EffectConfigParameters>(halConfig.mask);
}

// static
void EffectHalHidl::effectBufferConfigToHal(
        const EffectBufferConfig& config, buffer_config_t* halConfig) {
    halConfig->buffer.frameCount = 0;
    halConfig->buffer.raw = NULL;
    halConfig->samplingRate = config.samplingRateHz;
    halConfig->channels = static_cast<uint32_t>(config.channels);
    halConfig->bufferProvider.cookie = NULL;
    halConfig->bufferProvider.getBuffer = NULL;
    halConfig->bufferProvider.releaseBuffer = NULL;
    halConfig->format = static_cast<uint8_t>(config.format);
    halConfig->accessMode = static_cast<uint8_t>(config.accessMode);
    halConfig->mask = static_cast<uint8_t>(config.mask);
}

// static
void EffectHalHidl::effectConfigFromHal(const effect_config_t& halConfig, EffectConfig* config) {
    effectBufferConfigFromHal(halConfig.inputCfg, &config->inputCfg);
    effectBufferConfigFromHal(halConfig.outputCfg, &config->outputCfg);
}

// static
void EffectHalHidl::effectConfigToHal(const EffectConfig& config, effect_config_t* halConfig) {
    effectBufferConfigToHal(config.inputCfg, &halConfig->inputCfg);
    effectBufferConfigToHal(config.outputCfg, &halConfig->outputCfg);
}

// static
status_t EffectHalHidl::analyzeResult(const Result& result) {
    switch (result) {
        case Result::OK: return OK;
        case Result::INVALID_ARGUMENTS: return BAD_VALUE;
        case Result::INVALID_STATE: return NOT_ENOUGH_DATA;
        case Result::NOT_INITIALIZED: return NO_INIT;
        case Result::NOT_SUPPORTED: return INVALID_OPERATION;
        case Result::RESULT_TOO_BIG: return NO_MEMORY;
        default: return NO_INIT;
    }
}

status_t EffectHalHidl::setInBuffer(const sp<EffectBufferHalInterface>& buffer) {
    if (!mBuffersChanged) {
        if (buffer.get() == nullptr || mInBuffer.get() == nullptr) {
            mBuffersChanged = buffer.get() != mInBuffer.get();
        } else {
            mBuffersChanged = buffer->audioBuffer() != mInBuffer->audioBuffer();
        }
    }
    mInBuffer = buffer;
    return OK;
}

status_t EffectHalHidl::setOutBuffer(const sp<EffectBufferHalInterface>& buffer) {
    if (!mBuffersChanged) {
        if (buffer.get() == nullptr || mOutBuffer.get() == nullptr) {
            mBuffersChanged = buffer.get() != mOutBuffer.get();
        } else {
            mBuffersChanged = buffer->audioBuffer() != mOutBuffer->audioBuffer();
        }
    }
    mOutBuffer = buffer;
    return OK;
}

status_t EffectHalHidl::process() {
    return processImpl(static_cast<uint32_t>(MessageQueueFlagBits::REQUEST_PROCESS));
}

status_t EffectHalHidl::processReverse() {
    return processImpl(static_cast<uint32_t>(MessageQueueFlagBits::REQUEST_PROCESS_REVERSE));
}

status_t EffectHalHidl::prepareForProcessing() {
    std::unique_ptr<StatusMQ> tempStatusMQ;
    Result retval;
    Return<void> ret = mEffect->prepareForProcessing(
            [&](Result r, const MQDescriptorSync<Result>& statusMQ) {
                retval = r;
                if (retval == Result::OK) {
                    tempStatusMQ.reset(new StatusMQ(statusMQ));
                    if (tempStatusMQ->isValid() && tempStatusMQ->getEventFlagWord()) {
                        EventFlag::createEventFlag(tempStatusMQ->getEventFlagWord(), &mEfGroup);
                    }
                }
            });
    if (!ret.isOk() || retval != Result::OK) {
        return ret.isOk() ? analyzeResult(retval) : FAILED_TRANSACTION;
    }
    if (!tempStatusMQ || !tempStatusMQ->isValid() || !mEfGroup) {
        ALOGE_IF(!tempStatusMQ, "Failed to obtain status message queue for effects");
        ALOGE_IF(tempStatusMQ && !tempStatusMQ->isValid(),
                "Status message queue for effects is invalid");
        ALOGE_IF(!mEfGroup, "Event flag creation for effects failed");
        return NO_INIT;
    }
    mStatusMQ = std::move(tempStatusMQ);
    return OK;
}

bool EffectHalHidl::needToResetBuffers() {
    if (mBuffersChanged) return true;
    bool inBufferFrameCountUpdated = mInBuffer->checkFrameCountChange();
    bool outBufferFrameCountUpdated = mOutBuffer->checkFrameCountChange();
    return inBufferFrameCountUpdated || outBufferFrameCountUpdated;
}

status_t EffectHalHidl::processImpl(uint32_t mqFlag) {
    if (mEffect == 0 || mInBuffer == 0 || mOutBuffer == 0) return NO_INIT;
    status_t status;
    if (!mStatusMQ && (status = prepareForProcessing()) != OK) {
        return status;
    }
    if (needToResetBuffers() && (status = setProcessBuffers()) != OK) {
        return status;
    }
    // The data is already in the buffers, just need to flush it and wake up the server side.
    std::atomic_thread_fence(std::memory_order_release);
    mEfGroup->wake(mqFlag);
    uint32_t efState = 0;
retry:
    status_t ret = mEfGroup->wait(
            static_cast<uint32_t>(MessageQueueFlagBits::DONE_PROCESSING), &efState);
    if (efState & static_cast<uint32_t>(MessageQueueFlagBits::DONE_PROCESSING)) {
        Result retval = Result::NOT_INITIALIZED;
        mStatusMQ->read(&retval);
        if (retval == Result::OK || retval == Result::INVALID_STATE) {
            // Sync back the changed contents of the buffer.
            std::atomic_thread_fence(std::memory_order_acquire);
        }
        return analyzeResult(retval);
    }
    if (ret == -EAGAIN || ret == -EINTR) {
        // Spurious wakeup. This normally retries no more than once.
        goto retry;
    }
    return ret;
}

status_t EffectHalHidl::setProcessBuffers() {
    Return<Result> ret = mEffect->setProcessBuffers(
            static_cast<EffectBufferHalHidl*>(mInBuffer.get())->hidlBuffer(),
            static_cast<EffectBufferHalHidl*>(mOutBuffer.get())->hidlBuffer());
    if (ret.isOk() && ret == Result::OK) {
        mBuffersChanged = false;
        return OK;
    }
    return ret.isOk() ? analyzeResult(ret) : FAILED_TRANSACTION;
}

status_t EffectHalHidl::command(uint32_t cmdCode, uint32_t cmdSize, void *pCmdData,
        uint32_t *replySize, void *pReplyData) {
    if (mEffect == 0) return NO_INIT;

    // Special cases.
    if (cmdCode == EFFECT_CMD_SET_CONFIG || cmdCode == EFFECT_CMD_SET_CONFIG_REVERSE) {
        return setConfigImpl(cmdCode, cmdSize, pCmdData, replySize, pReplyData);
    } else if (cmdCode == EFFECT_CMD_GET_CONFIG || cmdCode == EFFECT_CMD_GET_CONFIG_REVERSE) {
        return getConfigImpl(cmdCode, replySize, pReplyData);
    }

    // Common case.
    hidl_vec<uint8_t> hidlData;
    if (pCmdData != nullptr && cmdSize > 0) {
        hidlData.setToExternal(reinterpret_cast<uint8_t*>(pCmdData), cmdSize);
    }
    status_t status;
    uint32_t replySizeStub = 0;
    if (replySize == nullptr || pReplyData == nullptr) replySize = &replySizeStub;
    Return<void> ret = mEffect->command(cmdCode, hidlData, *replySize,
            [&](int32_t s, const hidl_vec<uint8_t>& result) {
                status = s;
                if (status == 0) {
                    if (*replySize > result.size()) *replySize = result.size();
                    if (pReplyData != nullptr && *replySize > 0) {
                        memcpy(pReplyData, &result[0], *replySize);
                    }
                }
            });
    return ret.isOk() ? status : FAILED_TRANSACTION;
}

status_t EffectHalHidl::getDescriptor(effect_descriptor_t *pDescriptor) {
    if (mEffect == 0) return NO_INIT;
    Result retval = Result::NOT_INITIALIZED;
    Return<void> ret = mEffect->getDescriptor(
            [&](Result r, const EffectDescriptor& result) {
                retval = r;
                if (retval == Result::OK) {
                    effectDescriptorToHal(result, pDescriptor);
                }
            });
    return ret.isOk() ? analyzeResult(retval) : FAILED_TRANSACTION;
}

status_t EffectHalHidl::close() {
    if (mEffect == 0) return NO_INIT;
    Return<Result> ret = mEffect->close();
    return ret.isOk() ? analyzeResult(ret) : FAILED_TRANSACTION;
}

status_t EffectHalHidl::dump(int fd) {
    if (mEffect == 0) return NO_INIT;
    native_handle_t* hidlHandle = native_handle_create(1, 0);
    hidlHandle->data[0] = fd;
    Return<void> ret = mEffect->debug(hidlHandle, {} /* options */);
    native_handle_delete(hidlHandle);
    return ret.isOk() ? OK : FAILED_TRANSACTION;
}

status_t EffectHalHidl::getConfigImpl(
        uint32_t cmdCode, uint32_t *replySize, void *pReplyData) {
    if (replySize == NULL || *replySize != sizeof(effect_config_t) || pReplyData == NULL) {
        return BAD_VALUE;
    }
    status_t result = FAILED_TRANSACTION;
    Return<void> ret;
    if (cmdCode == EFFECT_CMD_GET_CONFIG) {
        ret = mEffect->getConfig([&] (Result r, const EffectConfig &hidlConfig) {
            result = analyzeResult(r);
            if (r == Result::OK) {
                effectConfigToHal(hidlConfig, static_cast<effect_config_t*>(pReplyData));
            }
        });
    } else {
        ret = mEffect->getConfigReverse([&] (Result r, const EffectConfig &hidlConfig) {
            result = analyzeResult(r);
            if (r == Result::OK) {
                effectConfigToHal(hidlConfig, static_cast<effect_config_t*>(pReplyData));
            }
        });
    }
    if (!ret.isOk()) {
        result = FAILED_TRANSACTION;
    }
    return result;
}

status_t EffectHalHidl::setConfigImpl(
        uint32_t cmdCode, uint32_t cmdSize, void *pCmdData, uint32_t *replySize, void *pReplyData) {
    if (pCmdData == NULL || cmdSize != sizeof(effect_config_t) ||
            replySize == NULL || *replySize != sizeof(int32_t) || pReplyData == NULL) {
        return BAD_VALUE;
    }
    const effect_config_t *halConfig = static_cast<effect_config_t*>(pCmdData);
    if (halConfig->inputCfg.bufferProvider.getBuffer != NULL ||
            halConfig->inputCfg.bufferProvider.releaseBuffer != NULL ||
            halConfig->outputCfg.bufferProvider.getBuffer != NULL ||
            halConfig->outputCfg.bufferProvider.releaseBuffer != NULL) {
        ALOGE("Buffer provider callbacks are not supported");
    }
    EffectConfig hidlConfig;
    effectConfigFromHal(*halConfig, &hidlConfig);
    Return<Result> ret = cmdCode == EFFECT_CMD_SET_CONFIG ?
            mEffect->setConfig(hidlConfig, nullptr, nullptr) :
            mEffect->setConfigReverse(hidlConfig, nullptr, nullptr);
    status_t result = FAILED_TRANSACTION;
    if (ret.isOk()) {
        result = analyzeResult(ret);
        *static_cast<int32_t*>(pReplyData) = result;
    }
    return result;
}

} // namespace CPP_VERSION
} // namespace effect
} // namespace android
