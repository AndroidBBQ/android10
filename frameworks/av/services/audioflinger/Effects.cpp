/*
**
** Copyright 2012, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#define LOG_TAG "AudioFlinger"
//#define LOG_NDEBUG 0

#include <algorithm>

#include "Configuration.h"
#include <utils/Log.h>
#include <system/audio_effects/effect_aec.h>
#include <system/audio_effects/effect_dynamicsprocessing.h>
#include <system/audio_effects/effect_ns.h>
#include <system/audio_effects/effect_visualizer.h>
#include <audio_utils/channels.h>
#include <audio_utils/primitives.h>
#include <media/AudioEffect.h>
#include <media/audiohal/EffectHalInterface.h>
#include <media/audiohal/EffectsFactoryHalInterface.h>
#include <mediautils/ServiceUtilities.h>

#include "AudioFlinger.h"

// ----------------------------------------------------------------------------

// Note: the following macro is used for extremely verbose logging message.  In
// order to run with ALOG_ASSERT turned on, we need to have LOG_NDEBUG set to
// 0; but one side effect of this is to turn all LOGV's as well.  Some messages
// are so verbose that we want to suppress them even when we have ALOG_ASSERT
// turned on.  Do not uncomment the #def below unless you really know what you
// are doing and want to see all of the extremely verbose messages.
//#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#define DEFAULT_OUTPUT_SAMPLE_RATE 48000

namespace android {

// ----------------------------------------------------------------------------
//  EffectModule implementation
// ----------------------------------------------------------------------------

#undef LOG_TAG
#define LOG_TAG "AudioFlinger::EffectModule"

AudioFlinger::EffectModule::EffectModule(ThreadBase *thread,
                                        const wp<AudioFlinger::EffectChain>& chain,
                                        effect_descriptor_t *desc,
                                        int id,
                                        audio_session_t sessionId,
                                        bool pinned)
    : mPinned(pinned),
      mThread(thread), mChain(chain), mId(id), mSessionId(sessionId),
      mDescriptor(*desc),
      // clear mConfig to ensure consistent initial value of buffer framecount
      // in case buffers are associated by setInBuffer() or setOutBuffer()
      // prior to configure().
      mConfig{{}, {}},
      mStatus(NO_INIT), mState(IDLE),
      mMaxDisableWaitCnt(1), // set by configure(), should be >= 1
      mDisableWaitCnt(0),    // set by process() and updateState()
      mSuspended(false),
      mOffloaded(false),
      mAudioFlinger(thread->mAudioFlinger)
#ifdef FLOAT_EFFECT_CHAIN
      , mSupportsFloat(false)
#endif
{
    ALOGV("Constructor %p pinned %d", this, pinned);
    int lStatus;

    // create effect engine from effect factory
    mStatus = -ENODEV;
    sp<AudioFlinger> audioFlinger = mAudioFlinger.promote();
    if (audioFlinger != 0) {
        sp<EffectsFactoryHalInterface> effectsFactory = audioFlinger->getEffectsFactory();
        if (effectsFactory != 0) {
            mStatus = effectsFactory->createEffect(
                    &desc->uuid, sessionId, thread->id(), &mEffectInterface);
        }
    }

    if (mStatus != NO_ERROR) {
        return;
    }
    lStatus = init();
    if (lStatus < 0) {
        mStatus = lStatus;
        goto Error;
    }

    setOffloaded(thread->type() == ThreadBase::OFFLOAD, thread->id());
    ALOGV("Constructor success name %s, Interface %p", mDescriptor.name, mEffectInterface.get());

    return;
Error:
    mEffectInterface.clear();
    ALOGV("Constructor Error %d", mStatus);
}

AudioFlinger::EffectModule::~EffectModule()
{
    ALOGV("Destructor %p", this);
    if (mEffectInterface != 0) {
        char uuidStr[64];
        AudioEffect::guidToString(&mDescriptor.uuid, uuidStr, sizeof(uuidStr));
        ALOGW("EffectModule %p destructor called with unreleased interface, effect %s",
                this, uuidStr);
        release_l();
    }

}

status_t AudioFlinger::EffectModule::addHandle(EffectHandle *handle)
{
    status_t status;

    Mutex::Autolock _l(mLock);
    int priority = handle->priority();
    size_t size = mHandles.size();
    EffectHandle *controlHandle = NULL;
    size_t i;
    for (i = 0; i < size; i++) {
        EffectHandle *h = mHandles[i];
        if (h == NULL || h->disconnected()) {
            continue;
        }
        // first non destroyed handle is considered in control
        if (controlHandle == NULL) {
            controlHandle = h;
        }
        if (h->priority() <= priority) {
            break;
        }
    }
    // if inserted in first place, move effect control from previous owner to this handle
    if (i == 0) {
        bool enabled = false;
        if (controlHandle != NULL) {
            enabled = controlHandle->enabled();
            controlHandle->setControl(false/*hasControl*/, true /*signal*/, enabled /*enabled*/);
        }
        handle->setControl(true /*hasControl*/, false /*signal*/, enabled /*enabled*/);
        status = NO_ERROR;
    } else {
        status = ALREADY_EXISTS;
    }
    ALOGV("addHandle() %p added handle %p in position %zu", this, handle, i);
    mHandles.insertAt(handle, i);
    return status;
}

status_t AudioFlinger::EffectModule::updatePolicyState()
{
    status_t status = NO_ERROR;
    bool doRegister = false;
    bool registered = false;
    bool doEnable = false;
    bool enabled = false;
    audio_io_handle_t io;
    uint32_t strategy;

    {
        Mutex::Autolock _l(mLock);
        // register effect when first handle is attached and unregister when last handle is removed
        if (mPolicyRegistered != mHandles.size() > 0) {
            doRegister = true;
            mPolicyRegistered = mHandles.size() > 0;
            if (mPolicyRegistered) {
              sp <EffectChain> chain = mChain.promote();
              sp <ThreadBase> thread = mThread.promote();

              if (thread == nullptr || chain == nullptr) {
                    return INVALID_OPERATION;
                }
                io = thread->id();
                strategy = chain->strategy();
            }
        }
        // enable effect when registered according to enable state requested by controlling handle
        if (mHandles.size() > 0) {
            EffectHandle *handle = controlHandle_l();
            if (handle != nullptr && mPolicyEnabled != handle->enabled()) {
                doEnable = true;
                mPolicyEnabled = handle->enabled();
            }
        }
        registered = mPolicyRegistered;
        enabled = mPolicyEnabled;
        mPolicyLock.lock();
    }
    ALOGV("%s name %s id %d session %d doRegister %d registered %d doEnable %d enabled %d",
        __func__, mDescriptor.name, mId, mSessionId, doRegister, registered, doEnable, enabled);
    if (doRegister) {
        if (registered) {
            status = AudioSystem::registerEffect(
                &mDescriptor,
                io,
                strategy,
                mSessionId,
                mId);
        } else {
            status = AudioSystem::unregisterEffect(mId);
        }
    }
    if (registered && doEnable) {
        status = AudioSystem::setEffectEnabled(mId, enabled);
    }
    mPolicyLock.unlock();

    return status;
}


ssize_t AudioFlinger::EffectModule::removeHandle(EffectHandle *handle)
{
    Mutex::Autolock _l(mLock);
    return removeHandle_l(handle);
}

ssize_t AudioFlinger::EffectModule::removeHandle_l(EffectHandle *handle)
{
    size_t size = mHandles.size();
    size_t i;
    for (i = 0; i < size; i++) {
        if (mHandles[i] == handle) {
            break;
        }
    }
    if (i == size) {
        ALOGW("%s %p handle not found %p", __FUNCTION__, this, handle);
        return BAD_VALUE;
    }
    ALOGV("removeHandle_l() %p removed handle %p in position %zu", this, handle, i);

    mHandles.removeAt(i);
    // if removed from first place, move effect control from this handle to next in line
    if (i == 0) {
        EffectHandle *h = controlHandle_l();
        if (h != NULL) {
            h->setControl(true /*hasControl*/, true /*signal*/ , handle->enabled() /*enabled*/);
        }
    }

    // Prevent calls to process() and other functions on effect interface from now on.
    // The effect engine will be released by the destructor when the last strong reference on
    // this object is released which can happen after next process is called.
    if (mHandles.size() == 0 && !mPinned) {
        mState = DESTROYED;
        mEffectInterface->close();
    }

    return mHandles.size();
}

// must be called with EffectModule::mLock held
AudioFlinger::EffectHandle *AudioFlinger::EffectModule::controlHandle_l()
{
    // the first valid handle in the list has control over the module
    for (size_t i = 0; i < mHandles.size(); i++) {
        EffectHandle *h = mHandles[i];
        if (h != NULL && !h->disconnected()) {
            return h;
        }
    }

    return NULL;
}

// unsafe method called when the effect parent thread has been destroyed
ssize_t AudioFlinger::EffectModule::disconnectHandle(EffectHandle *handle, bool unpinIfLast)
{
    ALOGV("disconnect() %p handle %p", this, handle);
    Mutex::Autolock _l(mLock);
    ssize_t numHandles = removeHandle_l(handle);
    if ((numHandles == 0) && (!mPinned || unpinIfLast)) {
        sp<AudioFlinger> af = mAudioFlinger.promote();
        if (af != 0) {
            mLock.unlock();
            af->updateOrphanEffectChains(this);
            mLock.lock();
        }
    }
    return numHandles;
}

bool AudioFlinger::EffectModule::updateState() {
    Mutex::Autolock _l(mLock);

    bool started = false;
    switch (mState) {
    case RESTART:
        reset_l();
        FALLTHROUGH_INTENDED;

    case STARTING:
        // clear auxiliary effect input buffer for next accumulation
        if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
            memset(mConfig.inputCfg.buffer.raw,
                   0,
                   mConfig.inputCfg.buffer.frameCount*sizeof(int32_t));
        }
        if (start_l() == NO_ERROR) {
            mState = ACTIVE;
            started = true;
        } else {
            mState = IDLE;
        }
        break;
    case STOPPING:
        // volume control for offload and direct threads must take effect immediately.
        if (stop_l() == NO_ERROR
            && !(isVolumeControl() && isOffloadedOrDirect())) {
            mDisableWaitCnt = mMaxDisableWaitCnt;
        } else {
            mDisableWaitCnt = 1; // will cause immediate transition to IDLE
        }
        mState = STOPPED;
        break;
    case STOPPED:
        // mDisableWaitCnt is forced to 1 by process() when the engine indicates the end of the
        // turn off sequence.
        if (--mDisableWaitCnt == 0) {
            reset_l();
            mState = IDLE;
        }
        break;
    default: //IDLE , ACTIVE, DESTROYED
        break;
    }

    return started;
}

void AudioFlinger::EffectModule::process()
{
    Mutex::Autolock _l(mLock);

    if (mState == DESTROYED || mEffectInterface == 0 || mInBuffer == 0 || mOutBuffer == 0) {
        return;
    }

    const uint32_t inChannelCount =
            audio_channel_count_from_out_mask(mConfig.inputCfg.channels);
    const uint32_t outChannelCount =
            audio_channel_count_from_out_mask(mConfig.outputCfg.channels);
    const bool auxType =
            (mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY;

    // safeInputOutputSampleCount is 0 if the channel count between input and output
    // buffers do not match. This prevents automatic accumulation or copying between the
    // input and output effect buffers without an intermediary effect process.
    // TODO: consider implementing channel conversion.
    const size_t safeInputOutputSampleCount =
            mInChannelCountRequested != mOutChannelCountRequested ? 0
                    : mOutChannelCountRequested * std::min(
                            mConfig.inputCfg.buffer.frameCount,
                            mConfig.outputCfg.buffer.frameCount);
    const auto accumulateInputToOutput = [this, safeInputOutputSampleCount]() {
#ifdef FLOAT_EFFECT_CHAIN
        accumulate_float(
                mConfig.outputCfg.buffer.f32,
                mConfig.inputCfg.buffer.f32,
                safeInputOutputSampleCount);
#else
        accumulate_i16(
                mConfig.outputCfg.buffer.s16,
                mConfig.inputCfg.buffer.s16,
                safeInputOutputSampleCount);
#endif
    };
    const auto copyInputToOutput = [this, safeInputOutputSampleCount]() {
#ifdef FLOAT_EFFECT_CHAIN
        memcpy(
                mConfig.outputCfg.buffer.f32,
                mConfig.inputCfg.buffer.f32,
                safeInputOutputSampleCount * sizeof(*mConfig.outputCfg.buffer.f32));

#else
        memcpy(
                mConfig.outputCfg.buffer.s16,
                mConfig.inputCfg.buffer.s16,
                safeInputOutputSampleCount * sizeof(*mConfig.outputCfg.buffer.s16));
#endif
    };

    if (isProcessEnabled()) {
        int ret;
        if (isProcessImplemented()) {
            if (auxType) {
                // We overwrite the aux input buffer here and clear after processing.
                // aux input is always mono.
#ifdef FLOAT_EFFECT_CHAIN
                if (mSupportsFloat) {
#ifndef FLOAT_AUX
                    // Do in-place float conversion for auxiliary effect input buffer.
                    static_assert(sizeof(float) <= sizeof(int32_t),
                            "in-place conversion requires sizeof(float) <= sizeof(int32_t)");

                    memcpy_to_float_from_q4_27(
                            mConfig.inputCfg.buffer.f32,
                            mConfig.inputCfg.buffer.s32,
                            mConfig.inputCfg.buffer.frameCount);
#endif // !FLOAT_AUX
                } else
#endif // FLOAT_EFFECT_CHAIN
                {
#ifdef FLOAT_AUX
                    memcpy_to_i16_from_float(
                            mConfig.inputCfg.buffer.s16,
                            mConfig.inputCfg.buffer.f32,
                            mConfig.inputCfg.buffer.frameCount);
#else
                    memcpy_to_i16_from_q4_27(
                            mConfig.inputCfg.buffer.s16,
                            mConfig.inputCfg.buffer.s32,
                            mConfig.inputCfg.buffer.frameCount);
#endif
                }
            }
#ifdef FLOAT_EFFECT_CHAIN
            sp<EffectBufferHalInterface> inBuffer = mInBuffer;
            sp<EffectBufferHalInterface> outBuffer = mOutBuffer;

            if (!auxType && mInChannelCountRequested != inChannelCount) {
                adjust_channels(
                        inBuffer->audioBuffer()->f32, mInChannelCountRequested,
                        mInConversionBuffer->audioBuffer()->f32, inChannelCount,
                        sizeof(float),
                        sizeof(float)
                        * mInChannelCountRequested * mConfig.inputCfg.buffer.frameCount);
                inBuffer = mInConversionBuffer;
            }
            if (mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE
                    && mOutChannelCountRequested != outChannelCount) {
                adjust_selected_channels(
                        outBuffer->audioBuffer()->f32, mOutChannelCountRequested,
                        mOutConversionBuffer->audioBuffer()->f32, outChannelCount,
                        sizeof(float),
                        sizeof(float)
                        * mOutChannelCountRequested * mConfig.outputCfg.buffer.frameCount);
                outBuffer = mOutConversionBuffer;
            }
            if (!mSupportsFloat) { // convert input to int16_t as effect doesn't support float.
                if (!auxType) {
                    if (mInConversionBuffer.get() == nullptr) {
                        ALOGW("%s: mInConversionBuffer is null, bypassing", __func__);
                        goto data_bypass;
                    }
                    memcpy_to_i16_from_float(
                            mInConversionBuffer->audioBuffer()->s16,
                            inBuffer->audioBuffer()->f32,
                            inChannelCount * mConfig.inputCfg.buffer.frameCount);
                    inBuffer = mInConversionBuffer;
                }
                if (mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
                    if (mOutConversionBuffer.get() == nullptr) {
                        ALOGW("%s: mOutConversionBuffer is null, bypassing", __func__);
                        goto data_bypass;
                    }
                    memcpy_to_i16_from_float(
                            mOutConversionBuffer->audioBuffer()->s16,
                            outBuffer->audioBuffer()->f32,
                            outChannelCount * mConfig.outputCfg.buffer.frameCount);
                    outBuffer = mOutConversionBuffer;
                }
            }
#endif
            ret = mEffectInterface->process();
#ifdef FLOAT_EFFECT_CHAIN
            if (!mSupportsFloat) { // convert output int16_t back to float.
                sp<EffectBufferHalInterface> target =
                        mOutChannelCountRequested != outChannelCount
                        ? mOutConversionBuffer : mOutBuffer;

                memcpy_to_float_from_i16(
                        target->audioBuffer()->f32,
                        mOutConversionBuffer->audioBuffer()->s16,
                        outChannelCount * mConfig.outputCfg.buffer.frameCount);
            }
            if (mOutChannelCountRequested != outChannelCount) {
                adjust_selected_channels(mOutConversionBuffer->audioBuffer()->f32, outChannelCount,
                        mOutBuffer->audioBuffer()->f32, mOutChannelCountRequested,
                        sizeof(float),
                        sizeof(float) * outChannelCount * mConfig.outputCfg.buffer.frameCount);
            }
#endif
        } else {
#ifdef FLOAT_EFFECT_CHAIN
            data_bypass:
#endif
            if (!auxType  /* aux effects do not require data bypass */
                    && mConfig.inputCfg.buffer.raw != mConfig.outputCfg.buffer.raw) {
                if (mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
                    accumulateInputToOutput();
                } else {
                    copyInputToOutput();
                }
            }
            ret = -ENODATA;
        }

        // force transition to IDLE state when engine is ready
        if (mState == STOPPED && ret == -ENODATA) {
            mDisableWaitCnt = 1;
        }

        // clear auxiliary effect input buffer for next accumulation
        if (auxType) {
#ifdef FLOAT_AUX
            const size_t size =
                    mConfig.inputCfg.buffer.frameCount * inChannelCount * sizeof(float);
#else
            const size_t size =
                    mConfig.inputCfg.buffer.frameCount * inChannelCount * sizeof(int32_t);
#endif
            memset(mConfig.inputCfg.buffer.raw, 0, size);
        }
    } else if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_INSERT &&
                // mInBuffer->audioBuffer()->raw != mOutBuffer->audioBuffer()->raw
                mConfig.inputCfg.buffer.raw != mConfig.outputCfg.buffer.raw) {
        // If an insert effect is idle and input buffer is different from output buffer,
        // accumulate input onto output
        sp<EffectChain> chain = mChain.promote();
        if (chain.get() != nullptr && chain->activeTrackCnt() != 0) {
            // similar handling with data_bypass above.
            if (mConfig.outputCfg.accessMode == EFFECT_BUFFER_ACCESS_ACCUMULATE) {
                accumulateInputToOutput();
            } else { // EFFECT_BUFFER_ACCESS_WRITE
                copyInputToOutput();
            }
        }
    }
}

void AudioFlinger::EffectModule::reset_l()
{
    if (mStatus != NO_ERROR || mEffectInterface == 0) {
        return;
    }
    mEffectInterface->command(EFFECT_CMD_RESET, 0, NULL, 0, NULL);
}

status_t AudioFlinger::EffectModule::configure()
{
    ALOGVV("configure() started");
    status_t status;
    sp<ThreadBase> thread;
    uint32_t size;
    audio_channel_mask_t channelMask;

    if (mEffectInterface == 0) {
        status = NO_INIT;
        goto exit;
    }

    thread = mThread.promote();
    if (thread == 0) {
        status = DEAD_OBJECT;
        goto exit;
    }

    // TODO: handle configuration of effects replacing track process
    // TODO: handle configuration of input (record) SW effects above the HAL,
    // similar to output EFFECT_FLAG_TYPE_INSERT/REPLACE,
    // in which case input channel masks should be used here.
    channelMask = thread->channelMask();
    mConfig.inputCfg.channels = channelMask;
    mConfig.outputCfg.channels = channelMask;

    if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
        if (mConfig.inputCfg.channels != AUDIO_CHANNEL_OUT_MONO) {
            mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_MONO;
            ALOGV("Overriding auxiliary effect input channels %#x as MONO",
                    mConfig.inputCfg.channels);
        }
#ifndef MULTICHANNEL_EFFECT_CHAIN
        if (mConfig.outputCfg.channels != AUDIO_CHANNEL_OUT_STEREO) {
            mConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
            ALOGV("Overriding auxiliary effect output channels %#x as STEREO",
                    mConfig.outputCfg.channels);
        }
#endif
    } else {
#ifndef MULTICHANNEL_EFFECT_CHAIN
        // TODO: Update this logic when multichannel effects are implemented.
        // For offloaded tracks consider mono output as stereo for proper effect initialization
        if (channelMask == AUDIO_CHANNEL_OUT_MONO) {
            mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
            mConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
            ALOGV("Overriding effect input and output as STEREO");
        }
#endif
    }
    mInChannelCountRequested =
            audio_channel_count_from_out_mask(mConfig.inputCfg.channels);
    mOutChannelCountRequested =
            audio_channel_count_from_out_mask(mConfig.outputCfg.channels);

    mConfig.inputCfg.format = EFFECT_BUFFER_FORMAT;
    mConfig.outputCfg.format = EFFECT_BUFFER_FORMAT;

    // Don't use sample rate for thread if effect isn't offloadable.
    if ((thread->type() == ThreadBase::OFFLOAD) && !isOffloaded()) {
        mConfig.inputCfg.samplingRate = DEFAULT_OUTPUT_SAMPLE_RATE;
        ALOGV("Overriding effect input as 48kHz");
    } else {
        mConfig.inputCfg.samplingRate = thread->sampleRate();
    }
    mConfig.outputCfg.samplingRate = mConfig.inputCfg.samplingRate;
    mConfig.inputCfg.bufferProvider.cookie = NULL;
    mConfig.inputCfg.bufferProvider.getBuffer = NULL;
    mConfig.inputCfg.bufferProvider.releaseBuffer = NULL;
    mConfig.outputCfg.bufferProvider.cookie = NULL;
    mConfig.outputCfg.bufferProvider.getBuffer = NULL;
    mConfig.outputCfg.bufferProvider.releaseBuffer = NULL;
    mConfig.inputCfg.accessMode = EFFECT_BUFFER_ACCESS_READ;
    // Insert effect:
    // - in session AUDIO_SESSION_OUTPUT_MIX or AUDIO_SESSION_OUTPUT_STAGE,
    // always overwrites output buffer: input buffer == output buffer
    // - in other sessions:
    //      last effect in the chain accumulates in output buffer: input buffer != output buffer
    //      other effect: overwrites output buffer: input buffer == output buffer
    // Auxiliary effect:
    //      accumulates in output buffer: input buffer != output buffer
    // Therefore: accumulate <=> input buffer != output buffer
    if (mConfig.inputCfg.buffer.raw != mConfig.outputCfg.buffer.raw) {
        mConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_ACCUMULATE;
    } else {
        mConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_WRITE;
    }
    mConfig.inputCfg.mask = EFFECT_CONFIG_ALL;
    mConfig.outputCfg.mask = EFFECT_CONFIG_ALL;
    mConfig.inputCfg.buffer.frameCount = thread->frameCount();
    mConfig.outputCfg.buffer.frameCount = mConfig.inputCfg.buffer.frameCount;

    ALOGV("configure() %p thread %p buffer %p framecount %zu",
            this, thread.get(), mConfig.inputCfg.buffer.raw, mConfig.inputCfg.buffer.frameCount);

    status_t cmdStatus;
    size = sizeof(int);
    status = mEffectInterface->command(EFFECT_CMD_SET_CONFIG,
                                       sizeof(mConfig),
                                       &mConfig,
                                       &size,
                                       &cmdStatus);
    if (status == NO_ERROR) {
        status = cmdStatus;
    }

#ifdef MULTICHANNEL_EFFECT_CHAIN
    if (status != NO_ERROR &&
            thread->isOutput() &&
            (mConfig.inputCfg.channels != AUDIO_CHANNEL_OUT_STEREO
                    || mConfig.outputCfg.channels != AUDIO_CHANNEL_OUT_STEREO)) {
        // Older effects may require exact STEREO position mask.
        if (mConfig.inputCfg.channels != AUDIO_CHANNEL_OUT_STEREO
                && (mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_AUXILIARY) {
            ALOGV("Overriding effect input channels %#x as STEREO", mConfig.inputCfg.channels);
            mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
        }
        if (mConfig.outputCfg.channels != AUDIO_CHANNEL_OUT_STEREO) {
            ALOGV("Overriding effect output channels %#x as STEREO", mConfig.outputCfg.channels);
            mConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
        }
        size = sizeof(int);
        status = mEffectInterface->command(EFFECT_CMD_SET_CONFIG,
                                           sizeof(mConfig),
                                           &mConfig,
                                           &size,
                                           &cmdStatus);
        if (status == NO_ERROR) {
            status = cmdStatus;
        }
    }
#endif

#ifdef FLOAT_EFFECT_CHAIN
    if (status == NO_ERROR) {
        mSupportsFloat = true;
    }

    if (status != NO_ERROR) {
        ALOGV("EFFECT_CMD_SET_CONFIG failed with float format, retry with int16_t.");
        mConfig.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        mConfig.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        size = sizeof(int);
        status = mEffectInterface->command(EFFECT_CMD_SET_CONFIG,
                                           sizeof(mConfig),
                                           &mConfig,
                                           &size,
                                           &cmdStatus);
        if (status == NO_ERROR) {
            status = cmdStatus;
        }
        if (status == NO_ERROR) {
            mSupportsFloat = false;
            ALOGVV("config worked with 16 bit");
        } else {
            ALOGE("%s failed %d with int16_t (as well as float)", __func__, status);
        }
    }
#endif

    if (status == NO_ERROR) {
        // Establish Buffer strategy
        setInBuffer(mInBuffer);
        setOutBuffer(mOutBuffer);

        // Update visualizer latency
        if (memcmp(&mDescriptor.type, SL_IID_VISUALIZATION, sizeof(effect_uuid_t)) == 0) {
            uint32_t buf32[sizeof(effect_param_t) / sizeof(uint32_t) + 2];
            effect_param_t *p = (effect_param_t *)buf32;

            p->psize = sizeof(uint32_t);
            p->vsize = sizeof(uint32_t);
            size = sizeof(int);
            *(int32_t *)p->data = VISUALIZER_PARAM_LATENCY;

            uint32_t latency = 0;
            PlaybackThread *pbt = thread->mAudioFlinger->checkPlaybackThread_l(thread->mId);
            if (pbt != NULL) {
                latency = pbt->latency_l();
            }

            *((int32_t *)p->data + 1)= latency;
            mEffectInterface->command(EFFECT_CMD_SET_PARAM,
                    sizeof(effect_param_t) + 8,
                    &buf32,
                    &size,
                    &cmdStatus);
        }
    }

    // mConfig.outputCfg.buffer.frameCount cannot be zero.
    mMaxDisableWaitCnt = (uint32_t)std::max(
            (uint64_t)1, // mMaxDisableWaitCnt must be greater than zero.
            (uint64_t)MAX_DISABLE_TIME_MS * mConfig.outputCfg.samplingRate
                / ((uint64_t)1000 * mConfig.outputCfg.buffer.frameCount));

exit:
    // TODO: consider clearing mConfig on error.
    mStatus = status;
    ALOGVV("configure ended");
    return status;
}

status_t AudioFlinger::EffectModule::init()
{
    Mutex::Autolock _l(mLock);
    if (mEffectInterface == 0) {
        return NO_INIT;
    }
    status_t cmdStatus;
    uint32_t size = sizeof(status_t);
    status_t status = mEffectInterface->command(EFFECT_CMD_INIT,
                                                0,
                                                NULL,
                                                &size,
                                                &cmdStatus);
    if (status == 0) {
        status = cmdStatus;
    }
    return status;
}

void AudioFlinger::EffectModule::addEffectToHal_l()
{
    if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_PRE_PROC ||
         (mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_POST_PROC) {
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0) {
            sp<StreamHalInterface> stream = thread->stream();
            if (stream != 0) {
                status_t result = stream->addEffect(mEffectInterface);
                ALOGE_IF(result != OK, "Error when adding effect: %d", result);
            }
        }
    }
}

// start() must be called with PlaybackThread::mLock or EffectChain::mLock held
status_t AudioFlinger::EffectModule::start()
{
    sp<EffectChain> chain;
    status_t status;
    {
        Mutex::Autolock _l(mLock);
        status = start_l();
        if (status == NO_ERROR) {
            chain = mChain.promote();
        }
    }
    if (chain != 0) {
        chain->resetVolume_l();
    }
    return status;
}

status_t AudioFlinger::EffectModule::start_l()
{
    if (mEffectInterface == 0) {
        return NO_INIT;
    }
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t cmdStatus;
    uint32_t size = sizeof(status_t);
    status_t status = mEffectInterface->command(EFFECT_CMD_ENABLE,
                                                0,
                                                NULL,
                                                &size,
                                                &cmdStatus);
    if (status == 0) {
        status = cmdStatus;
    }
    if (status == 0) {
        addEffectToHal_l();
    }
    return status;
}

status_t AudioFlinger::EffectModule::stop()
{
    Mutex::Autolock _l(mLock);
    return stop_l();
}

status_t AudioFlinger::EffectModule::stop_l()
{
    if (mEffectInterface == 0) {
        return NO_INIT;
    }
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t cmdStatus = NO_ERROR;
    uint32_t size = sizeof(status_t);

    if (isVolumeControl() && isOffloadedOrDirect()) {
        sp<EffectChain>chain = mChain.promote();
        // We have the EffectChain and EffectModule lock, permit a reentrant call to setVolume:
        // resetVolume_l --> setVolume_l --> EffectModule::setVolume
        mSetVolumeReentrantTid = gettid();
        chain->resetVolume_l();
        mSetVolumeReentrantTid = INVALID_PID;
    }

    status_t status = mEffectInterface->command(EFFECT_CMD_DISABLE,
                                                0,
                                                NULL,
                                                &size,
                                                &cmdStatus);
    if (status == NO_ERROR) {
        status = cmdStatus;
    }
    if (status == NO_ERROR) {
        status = remove_effect_from_hal_l();
    }
    return status;
}

// must be called with EffectChain::mLock held
void AudioFlinger::EffectModule::release_l()
{
    if (mEffectInterface != 0) {
        remove_effect_from_hal_l();
        // release effect engine
        mEffectInterface->close();
        mEffectInterface.clear();
    }
}

status_t AudioFlinger::EffectModule::remove_effect_from_hal_l()
{
    if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_PRE_PROC ||
             (mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_POST_PROC) {
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0) {
            sp<StreamHalInterface> stream = thread->stream();
            if (stream != 0) {
                status_t result = stream->removeEffect(mEffectInterface);
                ALOGE_IF(result != OK, "Error when removing effect: %d", result);
            }
        }
    }
    return NO_ERROR;
}

// round up delta valid if value and divisor are positive.
template <typename T>
static T roundUpDelta(const T &value, const T &divisor) {
    T remainder = value % divisor;
    return remainder == 0 ? 0 : divisor - remainder;
}

status_t AudioFlinger::EffectModule::command(uint32_t cmdCode,
                                             uint32_t cmdSize,
                                             void *pCmdData,
                                             uint32_t *replySize,
                                             void *pReplyData)
{
    Mutex::Autolock _l(mLock);
    ALOGVV("command(), cmdCode: %d, mEffectInterface: %p", cmdCode, mEffectInterface.get());

    if (mState == DESTROYED || mEffectInterface == 0) {
        return NO_INIT;
    }
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    if (cmdCode == EFFECT_CMD_GET_PARAM &&
            (sizeof(effect_param_t) > cmdSize ||
                    ((effect_param_t *)pCmdData)->psize > cmdSize
                                                          - sizeof(effect_param_t))) {
        android_errorWriteLog(0x534e4554, "32438594");
        android_errorWriteLog(0x534e4554, "33003822");
        return -EINVAL;
    }
    if (cmdCode == EFFECT_CMD_GET_PARAM &&
            (*replySize < sizeof(effect_param_t) ||
                    ((effect_param_t *)pCmdData)->psize > *replySize - sizeof(effect_param_t))) {
        android_errorWriteLog(0x534e4554, "29251553");
        return -EINVAL;
    }
    if (cmdCode == EFFECT_CMD_GET_PARAM &&
        (sizeof(effect_param_t) > *replySize
          || ((effect_param_t *)pCmdData)->psize > *replySize
                                                   - sizeof(effect_param_t)
          || ((effect_param_t *)pCmdData)->vsize > *replySize
                                                   - sizeof(effect_param_t)
                                                   - ((effect_param_t *)pCmdData)->psize
          || roundUpDelta(((effect_param_t *)pCmdData)->psize, (uint32_t)sizeof(int)) >
                                                   *replySize
                                                   - sizeof(effect_param_t)
                                                   - ((effect_param_t *)pCmdData)->psize
                                                   - ((effect_param_t *)pCmdData)->vsize)) {
        ALOGV("\tLVM_ERROR : EFFECT_CMD_GET_PARAM: reply size inconsistent");
                     android_errorWriteLog(0x534e4554, "32705438");
        return -EINVAL;
    }
    if ((cmdCode == EFFECT_CMD_SET_PARAM
            || cmdCode == EFFECT_CMD_SET_PARAM_DEFERRED) &&  // DEFERRED not generally used
        (sizeof(effect_param_t) > cmdSize
            || ((effect_param_t *)pCmdData)->psize > cmdSize
                                                     - sizeof(effect_param_t)
            || ((effect_param_t *)pCmdData)->vsize > cmdSize
                                                     - sizeof(effect_param_t)
                                                     - ((effect_param_t *)pCmdData)->psize
            || roundUpDelta(((effect_param_t *)pCmdData)->psize, (uint32_t)sizeof(int)) >
                                                     cmdSize
                                                     - sizeof(effect_param_t)
                                                     - ((effect_param_t *)pCmdData)->psize
                                                     - ((effect_param_t *)pCmdData)->vsize)) {
        android_errorWriteLog(0x534e4554, "30204301");
        return -EINVAL;
    }
    status_t status = mEffectInterface->command(cmdCode,
                                                cmdSize,
                                                pCmdData,
                                                replySize,
                                                pReplyData);
    if (cmdCode != EFFECT_CMD_GET_PARAM && status == NO_ERROR) {
        uint32_t size = (replySize == NULL) ? 0 : *replySize;
        for (size_t i = 1; i < mHandles.size(); i++) {
            EffectHandle *h = mHandles[i];
            if (h != NULL && !h->disconnected()) {
                h->commandExecuted(cmdCode, cmdSize, pCmdData, size, pReplyData);
            }
        }
    }
    return status;
}

status_t AudioFlinger::EffectModule::setEnabled(bool enabled)
{
    Mutex::Autolock _l(mLock);
    return setEnabled_l(enabled);
}

// must be called with EffectModule::mLock held
status_t AudioFlinger::EffectModule::setEnabled_l(bool enabled)
{

    ALOGV("setEnabled %p enabled %d", this, enabled);

    if (enabled != isEnabled()) {
        switch (mState) {
        // going from disabled to enabled
        case IDLE:
            mState = STARTING;
            break;
        case STOPPED:
            mState = RESTART;
            break;
        case STOPPING:
            mState = ACTIVE;
            break;

        // going from enabled to disabled
        case RESTART:
            mState = STOPPED;
            break;
        case STARTING:
            mState = IDLE;
            break;
        case ACTIVE:
            mState = STOPPING;
            break;
        case DESTROYED:
            return NO_ERROR; // simply ignore as we are being destroyed
        }
        for (size_t i = 1; i < mHandles.size(); i++) {
            EffectHandle *h = mHandles[i];
            if (h != NULL && !h->disconnected()) {
                h->setEnabled(enabled);
            }
        }
    }
    return NO_ERROR;
}

bool AudioFlinger::EffectModule::isEnabled() const
{
    switch (mState) {
    case RESTART:
    case STARTING:
    case ACTIVE:
        return true;
    case IDLE:
    case STOPPING:
    case STOPPED:
    case DESTROYED:
    default:
        return false;
    }
}

bool AudioFlinger::EffectModule::isProcessEnabled() const
{
    if (mStatus != NO_ERROR) {
        return false;
    }

    switch (mState) {
    case RESTART:
    case ACTIVE:
    case STOPPING:
    case STOPPED:
        return true;
    case IDLE:
    case STARTING:
    case DESTROYED:
    default:
        return false;
    }
}

bool AudioFlinger::EffectModule::isOffloadedOrDirect() const
{
    return (mThreadType == ThreadBase::OFFLOAD || mThreadType == ThreadBase::DIRECT);
}

bool AudioFlinger::EffectModule::isVolumeControlEnabled() const
{
    return (isVolumeControl() && (isOffloadedOrDirect() ? isEnabled() : isProcessEnabled()));
}

void AudioFlinger::EffectModule::setInBuffer(const sp<EffectBufferHalInterface>& buffer) {
    ALOGVV("setInBuffer %p",(&buffer));

    // mConfig.inputCfg.buffer.frameCount may be zero if configure() is not called yet.
    if (buffer != 0) {
        mConfig.inputCfg.buffer.raw = buffer->audioBuffer()->raw;
        buffer->setFrameCount(mConfig.inputCfg.buffer.frameCount);
    } else {
        mConfig.inputCfg.buffer.raw = NULL;
    }
    mInBuffer = buffer;
    mEffectInterface->setInBuffer(buffer);

#ifdef FLOAT_EFFECT_CHAIN
    // aux effects do in place conversion to float - we don't allocate mInConversionBuffer.
    // Theoretically insert effects can also do in-place conversions (destroying
    // the original buffer) when the output buffer is identical to the input buffer,
    // but we don't optimize for it here.
    const bool auxType = (mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY;
    const uint32_t inChannelCount =
            audio_channel_count_from_out_mask(mConfig.inputCfg.channels);
    const bool formatMismatch = !mSupportsFloat || mInChannelCountRequested != inChannelCount;
    if (!auxType && formatMismatch && mInBuffer.get() != nullptr) {
        // we need to translate - create hidl shared buffer and intercept
        const size_t inFrameCount = mConfig.inputCfg.buffer.frameCount;
        // Use FCC_2 in case mInChannelCountRequested is mono and the effect is stereo.
        const uint32_t inChannels = std::max((uint32_t)FCC_2, mInChannelCountRequested);
        const size_t size = inChannels * inFrameCount * std::max(sizeof(int16_t), sizeof(float));

        ALOGV("%s: setInBuffer updating for inChannels:%d inFrameCount:%zu total size:%zu",
                __func__, inChannels, inFrameCount, size);

        if (size > 0 && (mInConversionBuffer.get() == nullptr
                || size > mInConversionBuffer->getSize())) {
            mInConversionBuffer.clear();
            ALOGV("%s: allocating mInConversionBuffer %zu", __func__, size);
            sp<AudioFlinger> audioFlinger = mAudioFlinger.promote();
            LOG_ALWAYS_FATAL_IF(audioFlinger == nullptr, "EM could not retrieved audioFlinger");
            (void)audioFlinger->mEffectsFactoryHal->allocateBuffer(size, &mInConversionBuffer);
        }
        if (mInConversionBuffer.get() != nullptr) {
            mInConversionBuffer->setFrameCount(inFrameCount);
            mEffectInterface->setInBuffer(mInConversionBuffer);
        } else if (size > 0) {
            ALOGE("%s cannot create mInConversionBuffer", __func__);
        }
    }
#endif
}

void AudioFlinger::EffectModule::setOutBuffer(const sp<EffectBufferHalInterface>& buffer) {
    ALOGVV("setOutBuffer %p",(&buffer));

    // mConfig.outputCfg.buffer.frameCount may be zero if configure() is not called yet.
    if (buffer != 0) {
        mConfig.outputCfg.buffer.raw = buffer->audioBuffer()->raw;
        buffer->setFrameCount(mConfig.outputCfg.buffer.frameCount);
    } else {
        mConfig.outputCfg.buffer.raw = NULL;
    }
    mOutBuffer = buffer;
    mEffectInterface->setOutBuffer(buffer);

#ifdef FLOAT_EFFECT_CHAIN
    // Note: Any effect that does not accumulate does not need mOutConversionBuffer and
    // can do in-place conversion from int16_t to float.  We don't optimize here.
    const uint32_t outChannelCount =
            audio_channel_count_from_out_mask(mConfig.outputCfg.channels);
    const bool formatMismatch = !mSupportsFloat || mOutChannelCountRequested != outChannelCount;
    if (formatMismatch && mOutBuffer.get() != nullptr) {
        const size_t outFrameCount = mConfig.outputCfg.buffer.frameCount;
        // Use FCC_2 in case mOutChannelCountRequested is mono and the effect is stereo.
        const uint32_t outChannels = std::max((uint32_t)FCC_2, mOutChannelCountRequested);
        const size_t size = outChannels * outFrameCount * std::max(sizeof(int16_t), sizeof(float));

        ALOGV("%s: setOutBuffer updating for outChannels:%d outFrameCount:%zu total size:%zu",
                __func__, outChannels, outFrameCount, size);

        if (size > 0 && (mOutConversionBuffer.get() == nullptr
                || size > mOutConversionBuffer->getSize())) {
            mOutConversionBuffer.clear();
            ALOGV("%s: allocating mOutConversionBuffer %zu", __func__, size);
            sp<AudioFlinger> audioFlinger = mAudioFlinger.promote();
            LOG_ALWAYS_FATAL_IF(audioFlinger == nullptr, "EM could not retrieved audioFlinger");
            (void)audioFlinger->mEffectsFactoryHal->allocateBuffer(size, &mOutConversionBuffer);
        }
        if (mOutConversionBuffer.get() != nullptr) {
            mOutConversionBuffer->setFrameCount(outFrameCount);
            mEffectInterface->setOutBuffer(mOutConversionBuffer);
        } else if (size > 0) {
            ALOGE("%s cannot create mOutConversionBuffer", __func__);
        }
    }
#endif
}

status_t AudioFlinger::EffectModule::setVolume(uint32_t *left, uint32_t *right, bool controller)
{
    AutoLockReentrant _l(mLock, mSetVolumeReentrantTid);
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t status = NO_ERROR;
    // Send volume indication if EFFECT_FLAG_VOLUME_IND is set and read back altered volume
    // if controller flag is set (Note that controller == TRUE => EFFECT_FLAG_VOLUME_CTRL set)
    if (isProcessEnabled() &&
            ((mDescriptor.flags & EFFECT_FLAG_VOLUME_MASK) == EFFECT_FLAG_VOLUME_CTRL ||
             (mDescriptor.flags & EFFECT_FLAG_VOLUME_MASK) == EFFECT_FLAG_VOLUME_IND ||
             (mDescriptor.flags & EFFECT_FLAG_VOLUME_MASK) == EFFECT_FLAG_VOLUME_MONITOR)) {
        uint32_t volume[2];
        uint32_t *pVolume = NULL;
        uint32_t size = sizeof(volume);
        volume[0] = *left;
        volume[1] = *right;
        if (controller) {
            pVolume = volume;
        }
        status = mEffectInterface->command(EFFECT_CMD_SET_VOLUME,
                                           size,
                                           volume,
                                           &size,
                                           pVolume);
        if (controller && status == NO_ERROR && size == sizeof(volume)) {
            *left = volume[0];
            *right = volume[1];
        }
    }
    return status;
}

void AudioFlinger::EffectChain::setVolumeForOutput_l(uint32_t left, uint32_t right)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0 &&
        (thread->type() == ThreadBase::OFFLOAD || thread->type() == ThreadBase::DIRECT) &&
        !isNonOffloadableEnabled_l()) {
        PlaybackThread *t = (PlaybackThread *)thread.get();
        float vol_l = (float)left / (1 << 24);
        float vol_r = (float)right / (1 << 24);
        t->setVolumeForOutput_l(vol_l, vol_r);
    }
}

status_t AudioFlinger::EffectModule::setDevice(audio_devices_t device)
{
    if (device == AUDIO_DEVICE_NONE) {
        return NO_ERROR;
    }

    Mutex::Autolock _l(mLock);
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t status = NO_ERROR;
    if ((mDescriptor.flags & EFFECT_FLAG_DEVICE_MASK) == EFFECT_FLAG_DEVICE_IND) {
        status_t cmdStatus;
        uint32_t size = sizeof(status_t);
        uint32_t cmd = audio_is_output_devices(device) ? EFFECT_CMD_SET_DEVICE :
                            EFFECT_CMD_SET_INPUT_DEVICE;
        status = mEffectInterface->command(cmd,
                                           sizeof(uint32_t),
                                           &device,
                                           &size,
                                           &cmdStatus);
    }
    return status;
}

status_t AudioFlinger::EffectModule::setMode(audio_mode_t mode)
{
    Mutex::Autolock _l(mLock);
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t status = NO_ERROR;
    if ((mDescriptor.flags & EFFECT_FLAG_AUDIO_MODE_MASK) == EFFECT_FLAG_AUDIO_MODE_IND) {
        status_t cmdStatus;
        uint32_t size = sizeof(status_t);
        status = mEffectInterface->command(EFFECT_CMD_SET_AUDIO_MODE,
                                           sizeof(audio_mode_t),
                                           &mode,
                                           &size,
                                           &cmdStatus);
        if (status == NO_ERROR) {
            status = cmdStatus;
        }
    }
    return status;
}

status_t AudioFlinger::EffectModule::setAudioSource(audio_source_t source)
{
    Mutex::Autolock _l(mLock);
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t status = NO_ERROR;
    if ((mDescriptor.flags & EFFECT_FLAG_AUDIO_SOURCE_MASK) == EFFECT_FLAG_AUDIO_SOURCE_IND) {
        uint32_t size = 0;
        status = mEffectInterface->command(EFFECT_CMD_SET_AUDIO_SOURCE,
                                           sizeof(audio_source_t),
                                           &source,
                                           &size,
                                           NULL);
    }
    return status;
}

void AudioFlinger::EffectModule::setSuspended(bool suspended)
{
    Mutex::Autolock _l(mLock);
    mSuspended = suspended;
}

bool AudioFlinger::EffectModule::suspended() const
{
    Mutex::Autolock _l(mLock);
    return mSuspended;
}

bool AudioFlinger::EffectModule::purgeHandles()
{
    bool enabled = false;
    Mutex::Autolock _l(mLock);
    EffectHandle *handle = controlHandle_l();
    if (handle != NULL) {
        enabled = handle->enabled();
    }
    mHandles.clear();
    return enabled;
}

status_t AudioFlinger::EffectModule::setOffloaded(bool offloaded, audio_io_handle_t io)
{
    Mutex::Autolock _l(mLock);
    if (mStatus != NO_ERROR) {
        return mStatus;
    }
    status_t status = NO_ERROR;
    if ((mDescriptor.flags & EFFECT_FLAG_OFFLOAD_SUPPORTED) != 0) {
        status_t cmdStatus;
        uint32_t size = sizeof(status_t);
        effect_offload_param_t cmd;

        cmd.isOffload = offloaded;
        cmd.ioHandle = io;
        status = mEffectInterface->command(EFFECT_CMD_OFFLOAD,
                                           sizeof(effect_offload_param_t),
                                           &cmd,
                                           &size,
                                           &cmdStatus);
        if (status == NO_ERROR) {
            status = cmdStatus;
        }
        mOffloaded = (status == NO_ERROR) ? offloaded : false;
    } else {
        if (offloaded) {
            status = INVALID_OPERATION;
        }
        mOffloaded = false;
    }
    ALOGV("setOffloaded() offloaded %d io %d status %d", offloaded, io, status);
    return status;
}

bool AudioFlinger::EffectModule::isOffloaded() const
{
    Mutex::Autolock _l(mLock);
    return mOffloaded;
}

String8 effectFlagsToString(uint32_t flags) {
    String8 s;

    s.append("conn. mode: ");
    switch (flags & EFFECT_FLAG_TYPE_MASK) {
    case EFFECT_FLAG_TYPE_INSERT: s.append("insert"); break;
    case EFFECT_FLAG_TYPE_AUXILIARY: s.append("auxiliary"); break;
    case EFFECT_FLAG_TYPE_REPLACE: s.append("replace"); break;
    case EFFECT_FLAG_TYPE_PRE_PROC: s.append("preproc"); break;
    case EFFECT_FLAG_TYPE_POST_PROC: s.append("postproc"); break;
    default: s.append("unknown/reserved"); break;
    }
    s.append(", ");

    s.append("insert pref: ");
    switch (flags & EFFECT_FLAG_INSERT_MASK) {
    case EFFECT_FLAG_INSERT_ANY: s.append("any"); break;
    case EFFECT_FLAG_INSERT_FIRST: s.append("first"); break;
    case EFFECT_FLAG_INSERT_LAST: s.append("last"); break;
    case EFFECT_FLAG_INSERT_EXCLUSIVE: s.append("exclusive"); break;
    default: s.append("unknown/reserved"); break;
    }
    s.append(", ");

    s.append("volume mgmt: ");
    switch (flags & EFFECT_FLAG_VOLUME_MASK) {
    case EFFECT_FLAG_VOLUME_NONE: s.append("none"); break;
    case EFFECT_FLAG_VOLUME_CTRL: s.append("implements control"); break;
    case EFFECT_FLAG_VOLUME_IND: s.append("requires indication"); break;
    case EFFECT_FLAG_VOLUME_MONITOR: s.append("monitors volume"); break;
    default: s.append("unknown/reserved"); break;
    }
    s.append(", ");

    uint32_t devind = flags & EFFECT_FLAG_DEVICE_MASK;
    if (devind) {
        s.append("device indication: ");
        switch (devind) {
        case EFFECT_FLAG_DEVICE_IND: s.append("requires updates"); break;
        default: s.append("unknown/reserved"); break;
        }
        s.append(", ");
    }

    s.append("input mode: ");
    switch (flags & EFFECT_FLAG_INPUT_MASK) {
    case EFFECT_FLAG_INPUT_DIRECT: s.append("direct"); break;
    case EFFECT_FLAG_INPUT_PROVIDER: s.append("provider"); break;
    case EFFECT_FLAG_INPUT_BOTH: s.append("direct+provider"); break;
    default: s.append("not set"); break;
    }
    s.append(", ");

    s.append("output mode: ");
    switch (flags & EFFECT_FLAG_OUTPUT_MASK) {
    case EFFECT_FLAG_OUTPUT_DIRECT: s.append("direct"); break;
    case EFFECT_FLAG_OUTPUT_PROVIDER: s.append("provider"); break;
    case EFFECT_FLAG_OUTPUT_BOTH: s.append("direct+provider"); break;
    default: s.append("not set"); break;
    }
    s.append(", ");

    uint32_t accel = flags & EFFECT_FLAG_HW_ACC_MASK;
    if (accel) {
        s.append("hardware acceleration: ");
        switch (accel) {
        case EFFECT_FLAG_HW_ACC_SIMPLE: s.append("non-tunneled"); break;
        case EFFECT_FLAG_HW_ACC_TUNNEL: s.append("tunneled"); break;
        default: s.append("unknown/reserved"); break;
        }
        s.append(", ");
    }

    uint32_t modeind = flags & EFFECT_FLAG_AUDIO_MODE_MASK;
    if (modeind) {
        s.append("mode indication: ");
        switch (modeind) {
        case EFFECT_FLAG_AUDIO_MODE_IND: s.append("required"); break;
        default: s.append("unknown/reserved"); break;
        }
        s.append(", ");
    }

    uint32_t srcind = flags & EFFECT_FLAG_AUDIO_SOURCE_MASK;
    if (srcind) {
        s.append("source indication: ");
        switch (srcind) {
        case EFFECT_FLAG_AUDIO_SOURCE_IND: s.append("required"); break;
        default: s.append("unknown/reserved"); break;
        }
        s.append(", ");
    }

    if (flags & EFFECT_FLAG_OFFLOAD_MASK) {
        s.append("offloadable, ");
    }

    int len = s.length();
    if (s.length() > 2) {
        (void) s.lockBuffer(len);
        s.unlockBuffer(len - 2);
    }
    return s;
}

static std::string dumpInOutBuffer(bool isInput, const sp<EffectBufferHalInterface> &buffer) {
    std::stringstream ss;

    if (buffer.get() == nullptr) {
        return "nullptr"; // make different than below
    } else if (buffer->externalData() != nullptr) {
        ss << (isInput ? buffer->externalData() : buffer->audioBuffer()->raw)
                << " -> "
                << (isInput ? buffer->audioBuffer()->raw : buffer->externalData());
    } else {
        ss << buffer->audioBuffer()->raw;
    }
    return ss.str();
}

void AudioFlinger::EffectModule::dump(int fd, const Vector<String16>& args __unused)
{
    String8 result;

    result.appendFormat("\tEffect ID %d:\n", mId);

    bool locked = AudioFlinger::dumpTryLock(mLock);
    // failed to lock - AudioFlinger is probably deadlocked
    if (!locked) {
        result.append("\t\tCould not lock Fx mutex:\n");
    }

    result.append("\t\tSession Status State Registered Enabled Suspended Engine:\n");
    result.appendFormat("\t\t%05d   %03d    %03d   %s          %s       %s         %p\n",
            mSessionId, mStatus, mState, mPolicyRegistered ? "y" : "n", mPolicyEnabled ? "y" : "n",
            mSuspended ? "y" : "n", mEffectInterface.get());

    result.append("\t\tDescriptor:\n");
    char uuidStr[64];
    AudioEffect::guidToString(&mDescriptor.uuid, uuidStr, sizeof(uuidStr));
    result.appendFormat("\t\t- UUID: %s\n", uuidStr);
    AudioEffect::guidToString(&mDescriptor.type, uuidStr, sizeof(uuidStr));
    result.appendFormat("\t\t- TYPE: %s\n", uuidStr);
    result.appendFormat("\t\t- apiVersion: %08X\n\t\t- flags: %08X (%s)\n",
            mDescriptor.apiVersion,
            mDescriptor.flags,
            effectFlagsToString(mDescriptor.flags).string());
    result.appendFormat("\t\t- name: %s\n",
            mDescriptor.name);

    result.appendFormat("\t\t- implementor: %s\n",
            mDescriptor.implementor);

    result.appendFormat("\t\t- data: %s\n", mSupportsFloat ? "float" : "int16");

    result.append("\t\t- Input configuration:\n");
    result.append("\t\t\tBuffer     Frames  Smp rate Channels Format\n");
    result.appendFormat("\t\t\t%p %05zu   %05d    %08x %6d (%s)\n",
            mConfig.inputCfg.buffer.raw,
            mConfig.inputCfg.buffer.frameCount,
            mConfig.inputCfg.samplingRate,
            mConfig.inputCfg.channels,
            mConfig.inputCfg.format,
            formatToString((audio_format_t)mConfig.inputCfg.format).c_str());

    result.append("\t\t- Output configuration:\n");
    result.append("\t\t\tBuffer     Frames  Smp rate Channels Format\n");
    result.appendFormat("\t\t\t%p %05zu   %05d    %08x %6d (%s)\n",
            mConfig.outputCfg.buffer.raw,
            mConfig.outputCfg.buffer.frameCount,
            mConfig.outputCfg.samplingRate,
            mConfig.outputCfg.channels,
            mConfig.outputCfg.format,
            formatToString((audio_format_t)mConfig.outputCfg.format).c_str());

#ifdef FLOAT_EFFECT_CHAIN

    result.appendFormat("\t\t- HAL buffers:\n"
            "\t\t\tIn(%s) InConversion(%s) Out(%s) OutConversion(%s)\n",
            dumpInOutBuffer(true /* isInput */, mInBuffer).c_str(),
            dumpInOutBuffer(true /* isInput */, mInConversionBuffer).c_str(),
            dumpInOutBuffer(false /* isInput */, mOutBuffer).c_str(),
            dumpInOutBuffer(false /* isInput */, mOutConversionBuffer).c_str());
#endif

    result.appendFormat("\t\t%zu Clients:\n", mHandles.size());
    result.append("\t\t\t  Pid Priority Ctrl Locked client server\n");
    char buffer[256];
    for (size_t i = 0; i < mHandles.size(); ++i) {
        EffectHandle *handle = mHandles[i];
        if (handle != NULL && !handle->disconnected()) {
            handle->dumpToBuffer(buffer, sizeof(buffer));
            result.append(buffer);
        }
    }

    write(fd, result.string(), result.length());

    if (mEffectInterface != 0) {
        dprintf(fd, "\tEffect ID %d HAL dump:\n", mId);
        (void)mEffectInterface->dump(fd);
    }

    if (locked) {
        mLock.unlock();
    }
}

// ----------------------------------------------------------------------------
//  EffectHandle implementation
// ----------------------------------------------------------------------------

#undef LOG_TAG
#define LOG_TAG "AudioFlinger::EffectHandle"

AudioFlinger::EffectHandle::EffectHandle(const sp<EffectModule>& effect,
                                        const sp<AudioFlinger::Client>& client,
                                        const sp<IEffectClient>& effectClient,
                                        int32_t priority)
    : BnEffect(),
    mEffect(effect), mEffectClient(effectClient), mClient(client), mCblk(NULL),
    mPriority(priority), mHasControl(false), mEnabled(false), mDisconnected(false)
{
    ALOGV("constructor %p", this);

    if (client == 0) {
        return;
    }
    int bufOffset = ((sizeof(effect_param_cblk_t) - 1) / sizeof(int) + 1) * sizeof(int);
    mCblkMemory = client->heap()->allocate(EFFECT_PARAM_BUFFER_SIZE + bufOffset);
    if (mCblkMemory == 0 ||
            (mCblk = static_cast<effect_param_cblk_t *>(mCblkMemory->pointer())) == NULL) {
        ALOGE("not enough memory for Effect size=%zu", EFFECT_PARAM_BUFFER_SIZE +
                sizeof(effect_param_cblk_t));
        mCblkMemory.clear();
        return;
    }
    new(mCblk) effect_param_cblk_t();
    mBuffer = (uint8_t *)mCblk + bufOffset;
}

AudioFlinger::EffectHandle::~EffectHandle()
{
    ALOGV("Destructor %p", this);
    disconnect(false);
}

status_t AudioFlinger::EffectHandle::initCheck()
{
    return mClient == 0 || mCblkMemory != 0 ? OK : NO_MEMORY;
}

status_t AudioFlinger::EffectHandle::enable()
{
    AutoMutex _l(mLock);
    ALOGV("enable %p", this);
    sp<EffectModule> effect = mEffect.promote();
    if (effect == 0 || mDisconnected) {
        return DEAD_OBJECT;
    }
    if (!mHasControl) {
        return INVALID_OPERATION;
    }

    if (mEnabled) {
        return NO_ERROR;
    }

    mEnabled = true;

    status_t status = effect->updatePolicyState();
    if (status != NO_ERROR) {
        mEnabled = false;
        return status;
    }

    sp<ThreadBase> thread = effect->thread().promote();
    if (thread != 0) {
        thread->checkSuspendOnEffectEnabled(effect, true, effect->sessionId());
    }

    // checkSuspendOnEffectEnabled() can suspend this same effect when enabled
    if (effect->suspended()) {
        return NO_ERROR;
    }

    status = effect->setEnabled(true);
    if (status != NO_ERROR) {
        if (thread != 0) {
            thread->checkSuspendOnEffectEnabled(effect, false, effect->sessionId());
        }
        mEnabled = false;
    } else {
        if (thread != 0) {
            if (thread->type() == ThreadBase::OFFLOAD || thread->type() == ThreadBase::MMAP) {
                Mutex::Autolock _l(thread->mLock);
                thread->broadcast_l();
            }
            if (!effect->isOffloadable()) {
                if (thread->type() == ThreadBase::OFFLOAD) {
                    PlaybackThread *t = (PlaybackThread *)thread.get();
                    t->invalidateTracks(AUDIO_STREAM_MUSIC);
                }
                if (effect->sessionId() == AUDIO_SESSION_OUTPUT_MIX) {
                    thread->mAudioFlinger->onNonOffloadableGlobalEffectEnable();
                }
            }
        }
    }
    return status;
}

status_t AudioFlinger::EffectHandle::disable()
{
    ALOGV("disable %p", this);
    AutoMutex _l(mLock);
    sp<EffectModule> effect = mEffect.promote();
    if (effect == 0 || mDisconnected) {
        return DEAD_OBJECT;
    }
    if (!mHasControl) {
        return INVALID_OPERATION;
    }

    if (!mEnabled) {
        return NO_ERROR;
    }
    mEnabled = false;

    effect->updatePolicyState();

    if (effect->suspended()) {
        return NO_ERROR;
    }

    status_t status = effect->setEnabled(false);

    sp<ThreadBase> thread = effect->thread().promote();
    if (thread != 0) {
        thread->checkSuspendOnEffectEnabled(effect, false, effect->sessionId());
        if (thread->type() == ThreadBase::OFFLOAD || thread->type() == ThreadBase::MMAP) {
            Mutex::Autolock _l(thread->mLock);
            thread->broadcast_l();
        }
    }

    return status;
}

void AudioFlinger::EffectHandle::disconnect()
{
    ALOGV("%s %p", __FUNCTION__, this);
    disconnect(true);
}

void AudioFlinger::EffectHandle::disconnect(bool unpinIfLast)
{
    AutoMutex _l(mLock);
    ALOGV("disconnect(%s) %p", unpinIfLast ? "true" : "false", this);
    if (mDisconnected) {
        if (unpinIfLast) {
            android_errorWriteLog(0x534e4554, "32707507");
        }
        return;
    }
    mDisconnected = true;
    {
        sp<EffectModule> effect = mEffect.promote();
        if (effect != 0) {
            sp<ThreadBase> thread = effect->thread().promote();
            if (thread != 0) {
                thread->disconnectEffectHandle(this, unpinIfLast);
            } else if (effect->disconnectHandle(this, unpinIfLast) > 0) {
                ALOGW("%s Effect handle %p disconnected after thread destruction",
                    __func__, this);
            }
            effect->updatePolicyState();
        }
    }

    if (mClient != 0) {
        if (mCblk != NULL) {
            // unlike ~TrackBase(), mCblk is never a local new, so don't delete
            mCblk->~effect_param_cblk_t();   // destroy our shared-structure.
        }
        mCblkMemory.clear();    // free the shared memory before releasing the heap it belongs to
        // Client destructor must run with AudioFlinger client mutex locked
        Mutex::Autolock _l(mClient->audioFlinger()->mClientLock);
        mClient.clear();
    }
}

status_t AudioFlinger::EffectHandle::command(uint32_t cmdCode,
                                             uint32_t cmdSize,
                                             void *pCmdData,
                                             uint32_t *replySize,
                                             void *pReplyData)
{
    ALOGVV("command(), cmdCode: %d, mHasControl: %d, mEffect: %p",
            cmdCode, mHasControl, mEffect.unsafe_get());

    // reject commands reserved for internal use by audio framework if coming from outside
    // of audioserver
    switch(cmdCode) {
        case EFFECT_CMD_ENABLE:
        case EFFECT_CMD_DISABLE:
        case EFFECT_CMD_SET_PARAM:
        case EFFECT_CMD_SET_PARAM_DEFERRED:
        case EFFECT_CMD_SET_PARAM_COMMIT:
        case EFFECT_CMD_GET_PARAM:
            break;
        default:
            if (cmdCode >= EFFECT_CMD_FIRST_PROPRIETARY) {
                break;
            }
            android_errorWriteLog(0x534e4554, "62019992");
            return BAD_VALUE;
    }

    if (cmdCode == EFFECT_CMD_ENABLE) {
        if (*replySize < sizeof(int)) {
            android_errorWriteLog(0x534e4554, "32095713");
            return BAD_VALUE;
        }
        *(int *)pReplyData = NO_ERROR;
        *replySize = sizeof(int);
        return enable();
    } else if (cmdCode == EFFECT_CMD_DISABLE) {
        if (*replySize < sizeof(int)) {
            android_errorWriteLog(0x534e4554, "32095713");
            return BAD_VALUE;
        }
        *(int *)pReplyData = NO_ERROR;
        *replySize = sizeof(int);
        return disable();
    }

    AutoMutex _l(mLock);
    sp<EffectModule> effect = mEffect.promote();
    if (effect == 0 || mDisconnected) {
        return DEAD_OBJECT;
    }
    // only get parameter command is permitted for applications not controlling the effect
    if (!mHasControl && cmdCode != EFFECT_CMD_GET_PARAM) {
        return INVALID_OPERATION;
    }
    if (mClient == 0) {
        return INVALID_OPERATION;
    }

    // handle commands that are not forwarded transparently to effect engine
    if (cmdCode == EFFECT_CMD_SET_PARAM_COMMIT) {
        if (*replySize < sizeof(int)) {
            android_errorWriteLog(0x534e4554, "32095713");
            return BAD_VALUE;
        }
        *(int *)pReplyData = NO_ERROR;
        *replySize = sizeof(int);

        // No need to trylock() here as this function is executed in the binder thread serving a
        // particular client process:  no risk to block the whole media server process or mixer
        // threads if we are stuck here
        Mutex::Autolock _l(mCblk->lock);
        // keep local copy of index in case of client corruption b/32220769
        const uint32_t clientIndex = mCblk->clientIndex;
        const uint32_t serverIndex = mCblk->serverIndex;
        if (clientIndex > EFFECT_PARAM_BUFFER_SIZE ||
            serverIndex > EFFECT_PARAM_BUFFER_SIZE) {
            mCblk->serverIndex = 0;
            mCblk->clientIndex = 0;
            return BAD_VALUE;
        }
        status_t status = NO_ERROR;
        effect_param_t *param = NULL;
        for (uint32_t index = serverIndex; index < clientIndex;) {
            int *p = (int *)(mBuffer + index);
            const int size = *p++;
            if (size < 0
                    || size > EFFECT_PARAM_BUFFER_SIZE
                    || ((uint8_t *)p + size) > mBuffer + clientIndex) {
                ALOGW("command(): invalid parameter block size");
                status = BAD_VALUE;
                break;
            }

            // copy to local memory in case of client corruption b/32220769
            param = (effect_param_t *)realloc(param, size);
            if (param == NULL) {
                ALOGW("command(): out of memory");
                status = NO_MEMORY;
                break;
            }
            memcpy(param, p, size);

            int reply = 0;
            uint32_t rsize = sizeof(reply);
            status_t ret = effect->command(EFFECT_CMD_SET_PARAM,
                                            size,
                                            param,
                                            &rsize,
                                            &reply);

            // verify shared memory: server index shouldn't change; client index can't go back.
            if (serverIndex != mCblk->serverIndex
                    || clientIndex > mCblk->clientIndex) {
                android_errorWriteLog(0x534e4554, "32220769");
                status = BAD_VALUE;
                break;
            }

            // stop at first error encountered
            if (ret != NO_ERROR) {
                status = ret;
                *(int *)pReplyData = reply;
                break;
            } else if (reply != NO_ERROR) {
                *(int *)pReplyData = reply;
                break;
            }
            index += size;
        }
        free(param);
        mCblk->serverIndex = 0;
        mCblk->clientIndex = 0;
        return status;
    }

    return effect->command(cmdCode, cmdSize, pCmdData, replySize, pReplyData);
}

void AudioFlinger::EffectHandle::setControl(bool hasControl, bool signal, bool enabled)
{
    ALOGV("setControl %p control %d", this, hasControl);

    mHasControl = hasControl;
    mEnabled = enabled;

    if (signal && mEffectClient != 0) {
        mEffectClient->controlStatusChanged(hasControl);
    }
}

void AudioFlinger::EffectHandle::commandExecuted(uint32_t cmdCode,
                                                 uint32_t cmdSize,
                                                 void *pCmdData,
                                                 uint32_t replySize,
                                                 void *pReplyData)
{
    if (mEffectClient != 0) {
        mEffectClient->commandExecuted(cmdCode, cmdSize, pCmdData, replySize, pReplyData);
    }
}



void AudioFlinger::EffectHandle::setEnabled(bool enabled)
{
    if (mEffectClient != 0) {
        mEffectClient->enableStatusChanged(enabled);
    }
}

status_t AudioFlinger::EffectHandle::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    return BnEffect::onTransact(code, data, reply, flags);
}


void AudioFlinger::EffectHandle::dumpToBuffer(char* buffer, size_t size)
{
    bool locked = mCblk != NULL && AudioFlinger::dumpTryLock(mCblk->lock);

    snprintf(buffer, size, "\t\t\t%5d    %5d  %3s    %3s  %5u  %5u\n",
            (mClient == 0) ? getpid() : mClient->pid(),
            mPriority,
            mHasControl ? "yes" : "no",
            locked ? "yes" : "no",
            mCblk ? mCblk->clientIndex : 0,
            mCblk ? mCblk->serverIndex : 0
            );

    if (locked) {
        mCblk->lock.unlock();
    }
}

#undef LOG_TAG
#define LOG_TAG "AudioFlinger::EffectChain"

AudioFlinger::EffectChain::EffectChain(ThreadBase *thread,
                                        audio_session_t sessionId)
    : mThread(thread), mSessionId(sessionId), mActiveTrackCnt(0), mTrackCnt(0), mTailBufferCount(0),
      mVolumeCtrlIdx(-1), mLeftVolume(UINT_MAX), mRightVolume(UINT_MAX),
      mNewLeftVolume(UINT_MAX), mNewRightVolume(UINT_MAX)
{
    mStrategy = AudioSystem::getStrategyForStream(AUDIO_STREAM_MUSIC);
    if (thread == NULL) {
        return;
    }
    mMaxTailBuffers = ((kProcessTailDurationMs * thread->sampleRate()) / 1000) /
                                    thread->frameCount();
}

AudioFlinger::EffectChain::~EffectChain()
{
}

// getEffectFromDesc_l() must be called with ThreadBase::mLock held
sp<AudioFlinger::EffectModule> AudioFlinger::EffectChain::getEffectFromDesc_l(
        effect_descriptor_t *descriptor)
{
    size_t size = mEffects.size();

    for (size_t i = 0; i < size; i++) {
        if (memcmp(&mEffects[i]->desc().uuid, &descriptor->uuid, sizeof(effect_uuid_t)) == 0) {
            return mEffects[i];
        }
    }
    return 0;
}

// getEffectFromId_l() must be called with ThreadBase::mLock held
sp<AudioFlinger::EffectModule> AudioFlinger::EffectChain::getEffectFromId_l(int id)
{
    size_t size = mEffects.size();

    for (size_t i = 0; i < size; i++) {
        // by convention, return first effect if id provided is 0 (0 is never a valid id)
        if (id == 0 || mEffects[i]->id() == id) {
            return mEffects[i];
        }
    }
    return 0;
}

// getEffectFromType_l() must be called with ThreadBase::mLock held
sp<AudioFlinger::EffectModule> AudioFlinger::EffectChain::getEffectFromType_l(
        const effect_uuid_t *type)
{
    size_t size = mEffects.size();

    for (size_t i = 0; i < size; i++) {
        if (memcmp(&mEffects[i]->desc().type, type, sizeof(effect_uuid_t)) == 0) {
            return mEffects[i];
        }
    }
    return 0;
}

std::vector<int> AudioFlinger::EffectChain::getEffectIds()
{
    std::vector<int> ids;
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mEffects.size(); i++) {
        ids.push_back(mEffects[i]->id());
    }
    return ids;
}

void AudioFlinger::EffectChain::clearInputBuffer()
{
    Mutex::Autolock _l(mLock);
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        ALOGW("clearInputBuffer(): cannot promote mixer thread");
        return;
    }
    clearInputBuffer_l(thread);
}

// Must be called with EffectChain::mLock locked
void AudioFlinger::EffectChain::clearInputBuffer_l(const sp<ThreadBase>& thread)
{
    if (mInBuffer == NULL) {
        return;
    }
    const size_t frameSize =
            audio_bytes_per_sample(EFFECT_BUFFER_FORMAT) * thread->channelCount();

    memset(mInBuffer->audioBuffer()->raw, 0, thread->frameCount() * frameSize);
    mInBuffer->commit();
}

// Must be called with EffectChain::mLock locked
void AudioFlinger::EffectChain::process_l()
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        ALOGW("process_l(): cannot promote mixer thread");
        return;
    }
    bool isGlobalSession = (mSessionId == AUDIO_SESSION_OUTPUT_MIX) ||
            (mSessionId == AUDIO_SESSION_OUTPUT_STAGE);
    // never process effects when:
    // - on an OFFLOAD thread
    // - no more tracks are on the session and the effect tail has been rendered
    bool doProcess = (thread->type() != ThreadBase::OFFLOAD)
                  && (thread->type() != ThreadBase::MMAP);
    if (!isGlobalSession) {
        bool tracksOnSession = (trackCnt() != 0);

        if (!tracksOnSession && mTailBufferCount == 0) {
            doProcess = false;
        }

        if (activeTrackCnt() == 0) {
            // if no track is active and the effect tail has not been rendered,
            // the input buffer must be cleared here as the mixer process will not do it
            if (tracksOnSession || mTailBufferCount > 0) {
                clearInputBuffer_l(thread);
                if (mTailBufferCount > 0) {
                    mTailBufferCount--;
                }
            }
        }
    }

    size_t size = mEffects.size();
    if (doProcess) {
        // Only the input and output buffers of the chain can be external,
        // and 'update' / 'commit' do nothing for allocated buffers, thus
        // it's not needed to consider any other buffers here.
        mInBuffer->update();
        if (mInBuffer->audioBuffer()->raw != mOutBuffer->audioBuffer()->raw) {
            mOutBuffer->update();
        }
        for (size_t i = 0; i < size; i++) {
            mEffects[i]->process();
        }
        mInBuffer->commit();
        if (mInBuffer->audioBuffer()->raw != mOutBuffer->audioBuffer()->raw) {
            mOutBuffer->commit();
        }
    }
    bool doResetVolume = false;
    for (size_t i = 0; i < size; i++) {
        doResetVolume = mEffects[i]->updateState() || doResetVolume;
    }
    if (doResetVolume) {
        resetVolume_l();
    }
}

// createEffect_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::EffectChain::createEffect_l(sp<EffectModule>& effect,
                                                   ThreadBase *thread,
                                                   effect_descriptor_t *desc,
                                                   int id,
                                                   audio_session_t sessionId,
                                                   bool pinned)
{
    Mutex::Autolock _l(mLock);
    effect = new EffectModule(thread, this, desc, id, sessionId, pinned);
    status_t lStatus = effect->status();
    if (lStatus == NO_ERROR) {
        lStatus = addEffect_ll(effect);
    }
    if (lStatus != NO_ERROR) {
        effect.clear();
    }
    return lStatus;
}

// addEffect_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::EffectChain::addEffect_l(const sp<EffectModule>& effect)
{
    Mutex::Autolock _l(mLock);
    return addEffect_ll(effect);
}
// addEffect_l() must be called with ThreadBase::mLock and EffectChain::mLock held
status_t AudioFlinger::EffectChain::addEffect_ll(const sp<EffectModule>& effect)
{
    effect_descriptor_t desc = effect->desc();
    uint32_t insertPref = desc.flags & EFFECT_FLAG_INSERT_MASK;

    effect->setChain(this);
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        return NO_INIT;
    }
    effect->setThread(thread);

    if ((desc.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
        // Auxiliary effects are inserted at the beginning of mEffects vector as
        // they are processed first and accumulated in chain input buffer
        mEffects.insertAt(effect, 0);

        // the input buffer for auxiliary effect contains mono samples in
        // 32 bit format. This is to avoid saturation in AudoMixer
        // accumulation stage. Saturation is done in EffectModule::process() before
        // calling the process in effect engine
        size_t numSamples = thread->frameCount();
        sp<EffectBufferHalInterface> halBuffer;
#ifdef FLOAT_EFFECT_CHAIN
        status_t result = thread->mAudioFlinger->mEffectsFactoryHal->allocateBuffer(
                numSamples * sizeof(float), &halBuffer);
#else
        status_t result = thread->mAudioFlinger->mEffectsFactoryHal->allocateBuffer(
                numSamples * sizeof(int32_t), &halBuffer);
#endif
        if (result != OK) return result;
        effect->setInBuffer(halBuffer);
        // auxiliary effects output samples to chain input buffer for further processing
        // by insert effects
        effect->setOutBuffer(mInBuffer);
    } else {
        // Insert effects are inserted at the end of mEffects vector as they are processed
        //  after track and auxiliary effects.
        // Insert effect order as a function of indicated preference:
        //  if EFFECT_FLAG_INSERT_EXCLUSIVE, insert in first position or reject if
        //  another effect is present
        //  else if EFFECT_FLAG_INSERT_FIRST, insert in first position or after the
        //  last effect claiming first position
        //  else if EFFECT_FLAG_INSERT_LAST, insert in last position or before the
        //  first effect claiming last position
        //  else if EFFECT_FLAG_INSERT_ANY insert after first or before last
        // Reject insertion if an effect with EFFECT_FLAG_INSERT_EXCLUSIVE is
        // already present

        size_t size = mEffects.size();
        size_t idx_insert = size;
        ssize_t idx_insert_first = -1;
        ssize_t idx_insert_last = -1;

        for (size_t i = 0; i < size; i++) {
            effect_descriptor_t d = mEffects[i]->desc();
            uint32_t iMode = d.flags & EFFECT_FLAG_TYPE_MASK;
            uint32_t iPref = d.flags & EFFECT_FLAG_INSERT_MASK;
            if (iMode == EFFECT_FLAG_TYPE_INSERT) {
                // check invalid effect chaining combinations
                if (insertPref == EFFECT_FLAG_INSERT_EXCLUSIVE ||
                    iPref == EFFECT_FLAG_INSERT_EXCLUSIVE) {
                    ALOGW("addEffect_l() could not insert effect %s: exclusive conflict with %s",
                            desc.name, d.name);
                    return INVALID_OPERATION;
                }
                // remember position of first insert effect and by default
                // select this as insert position for new effect
                if (idx_insert == size) {
                    idx_insert = i;
                }
                // remember position of last insert effect claiming
                // first position
                if (iPref == EFFECT_FLAG_INSERT_FIRST) {
                    idx_insert_first = i;
                }
                // remember position of first insert effect claiming
                // last position
                if (iPref == EFFECT_FLAG_INSERT_LAST &&
                    idx_insert_last == -1) {
                    idx_insert_last = i;
                }
            }
        }

        // modify idx_insert from first position if needed
        if (insertPref == EFFECT_FLAG_INSERT_LAST) {
            if (idx_insert_last != -1) {
                idx_insert = idx_insert_last;
            } else {
                idx_insert = size;
            }
        } else {
            if (idx_insert_first != -1) {
                idx_insert = idx_insert_first + 1;
            }
        }

        // always read samples from chain input buffer
        effect->setInBuffer(mInBuffer);

        // if last effect in the chain, output samples to chain
        // output buffer, otherwise to chain input buffer
        if (idx_insert == size) {
            if (idx_insert != 0) {
                mEffects[idx_insert-1]->setOutBuffer(mInBuffer);
                mEffects[idx_insert-1]->configure();
            }
            effect->setOutBuffer(mOutBuffer);
        } else {
            effect->setOutBuffer(mInBuffer);
        }
        mEffects.insertAt(effect, idx_insert);

        ALOGV("addEffect_l() effect %p, added in chain %p at rank %zu", effect.get(), this,
                idx_insert);
    }
    effect->configure();

    return NO_ERROR;
}

// removeEffect_l() must be called with ThreadBase::mLock held
size_t AudioFlinger::EffectChain::removeEffect_l(const sp<EffectModule>& effect,
                                                 bool release)
{
    Mutex::Autolock _l(mLock);
    size_t size = mEffects.size();
    uint32_t type = effect->desc().flags & EFFECT_FLAG_TYPE_MASK;

    for (size_t i = 0; i < size; i++) {
        if (effect == mEffects[i]) {
            // calling stop here will remove pre-processing effect from the audio HAL.
            // This is safe as we hold the EffectChain mutex which guarantees that we are not in
            // the middle of a read from audio HAL
            if (mEffects[i]->state() == EffectModule::ACTIVE ||
                    mEffects[i]->state() == EffectModule::STOPPING) {
                mEffects[i]->stop();
            }
            if (release) {
                mEffects[i]->release_l();
            }

            if (type != EFFECT_FLAG_TYPE_AUXILIARY) {
                if (i == size - 1 && i != 0) {
                    mEffects[i - 1]->setOutBuffer(mOutBuffer);
                    mEffects[i - 1]->configure();
                }
            }
            mEffects.removeAt(i);
            ALOGV("removeEffect_l() effect %p, removed from chain %p at rank %zu", effect.get(),
                    this, i);

            break;
        }
    }

    return mEffects.size();
}

// setDevice_l() must be called with ThreadBase::mLock held
void AudioFlinger::EffectChain::setDevice_l(audio_devices_t device)
{
    size_t size = mEffects.size();
    for (size_t i = 0; i < size; i++) {
        mEffects[i]->setDevice(device);
    }
}

// setMode_l() must be called with ThreadBase::mLock held
void AudioFlinger::EffectChain::setMode_l(audio_mode_t mode)
{
    size_t size = mEffects.size();
    for (size_t i = 0; i < size; i++) {
        mEffects[i]->setMode(mode);
    }
}

// setAudioSource_l() must be called with ThreadBase::mLock held
void AudioFlinger::EffectChain::setAudioSource_l(audio_source_t source)
{
    size_t size = mEffects.size();
    for (size_t i = 0; i < size; i++) {
        mEffects[i]->setAudioSource(source);
    }
}

// setVolume_l() must be called with ThreadBase::mLock or EffectChain::mLock held
bool AudioFlinger::EffectChain::setVolume_l(uint32_t *left, uint32_t *right, bool force)
{
    uint32_t newLeft = *left;
    uint32_t newRight = *right;
    bool hasControl = false;
    int ctrlIdx = -1;
    size_t size = mEffects.size();

    // first update volume controller
    for (size_t i = size; i > 0; i--) {
        if (mEffects[i - 1]->isVolumeControlEnabled()) {
            ctrlIdx = i - 1;
            hasControl = true;
            break;
        }
    }

    if (!force && ctrlIdx == mVolumeCtrlIdx &&
            *left == mLeftVolume && *right == mRightVolume) {
        if (hasControl) {
            *left = mNewLeftVolume;
            *right = mNewRightVolume;
        }
        return hasControl;
    }

    mVolumeCtrlIdx = ctrlIdx;
    mLeftVolume = newLeft;
    mRightVolume = newRight;

    // second get volume update from volume controller
    if (ctrlIdx >= 0) {
        mEffects[ctrlIdx]->setVolume(&newLeft, &newRight, true);
        mNewLeftVolume = newLeft;
        mNewRightVolume = newRight;
    }
    // then indicate volume to all other effects in chain.
    // Pass altered volume to effects before volume controller
    // and requested volume to effects after controller or with volume monitor flag
    uint32_t lVol = newLeft;
    uint32_t rVol = newRight;

    for (size_t i = 0; i < size; i++) {
        if ((int)i == ctrlIdx) {
            continue;
        }
        // this also works for ctrlIdx == -1 when there is no volume controller
        if ((int)i > ctrlIdx) {
            lVol = *left;
            rVol = *right;
        }
        // Pass requested volume directly if this is volume monitor module
        if (mEffects[i]->isVolumeMonitor()) {
            mEffects[i]->setVolume(left, right, false);
        } else {
            mEffects[i]->setVolume(&lVol, &rVol, false);
        }
    }
    *left = newLeft;
    *right = newRight;

    setVolumeForOutput_l(*left, *right);

    return hasControl;
}

// resetVolume_l() must be called with ThreadBase::mLock or EffectChain::mLock held
void AudioFlinger::EffectChain::resetVolume_l()
{
    if ((mLeftVolume != UINT_MAX) && (mRightVolume != UINT_MAX)) {
        uint32_t left = mLeftVolume;
        uint32_t right = mRightVolume;
        (void)setVolume_l(&left, &right, true);
    }
}

void AudioFlinger::EffectChain::syncHalEffectsState()
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mEffects.size(); i++) {
        if (mEffects[i]->state() == EffectModule::ACTIVE ||
                mEffects[i]->state() == EffectModule::STOPPING) {
            mEffects[i]->addEffectToHal_l();
        }
    }
}

void AudioFlinger::EffectChain::dump(int fd, const Vector<String16>& args)
{
    String8 result;

    const size_t numEffects = mEffects.size();
    result.appendFormat("    %zu effects for session %d\n", numEffects, mSessionId);

    if (numEffects) {
        bool locked = AudioFlinger::dumpTryLock(mLock);
        // failed to lock - AudioFlinger is probably deadlocked
        if (!locked) {
            result.append("\tCould not lock mutex:\n");
        }

        const std::string inBufferStr = dumpInOutBuffer(true /* isInput */, mInBuffer);
        const std::string outBufferStr = dumpInOutBuffer(false /* isInput */, mOutBuffer);
        result.appendFormat("\t%-*s%-*s   Active tracks:\n",
                (int)inBufferStr.size(), "In buffer    ",
                (int)outBufferStr.size(), "Out buffer      ");
        result.appendFormat("\t%s   %s   %d\n",
                inBufferStr.c_str(), outBufferStr.c_str(), mActiveTrackCnt);
        write(fd, result.string(), result.size());

        for (size_t i = 0; i < numEffects; ++i) {
            sp<EffectModule> effect = mEffects[i];
            if (effect != 0) {
                effect->dump(fd, args);
            }
        }

        if (locked) {
            mLock.unlock();
        }
    } else {
        write(fd, result.string(), result.size());
    }
}

// must be called with ThreadBase::mLock held
void AudioFlinger::EffectChain::setEffectSuspended_l(
        const effect_uuid_t *type, bool suspend)
{
    sp<SuspendedEffectDesc> desc;
    // use effect type UUID timelow as key as there is no real risk of identical
    // timeLow fields among effect type UUIDs.
    ssize_t index = mSuspendedEffects.indexOfKey(type->timeLow);
    if (suspend) {
        if (index >= 0) {
            desc = mSuspendedEffects.valueAt(index);
        } else {
            desc = new SuspendedEffectDesc();
            desc->mType = *type;
            mSuspendedEffects.add(type->timeLow, desc);
            ALOGV("setEffectSuspended_l() add entry for %08x", type->timeLow);
        }

        if (desc->mRefCount++ == 0) {
            sp<EffectModule> effect = getEffectIfEnabled(type);
            if (effect != 0) {
                desc->mEffect = effect;
                effect->setSuspended(true);
                effect->setEnabled(false);
            }
        }
    } else {
        if (index < 0) {
            return;
        }
        desc = mSuspendedEffects.valueAt(index);
        if (desc->mRefCount <= 0) {
            ALOGW("setEffectSuspended_l() restore refcount should not be 0 %d", desc->mRefCount);
            desc->mRefCount = 0;
            return;
        }
        if (--desc->mRefCount == 0) {
            ALOGV("setEffectSuspended_l() remove entry for %08x", mSuspendedEffects.keyAt(index));
            if (desc->mEffect != 0) {
                sp<EffectModule> effect = desc->mEffect.promote();
                if (effect != 0) {
                    effect->setSuspended(false);
                    effect->lock();
                    EffectHandle *handle = effect->controlHandle_l();
                    if (handle != NULL && !handle->disconnected()) {
                        effect->setEnabled_l(handle->enabled());
                    }
                    effect->unlock();
                }
                desc->mEffect.clear();
            }
            mSuspendedEffects.removeItemsAt(index);
        }
    }
}

// must be called with ThreadBase::mLock held
void AudioFlinger::EffectChain::setEffectSuspendedAll_l(bool suspend)
{
    sp<SuspendedEffectDesc> desc;

    ssize_t index = mSuspendedEffects.indexOfKey((int)kKeyForSuspendAll);
    if (suspend) {
        if (index >= 0) {
            desc = mSuspendedEffects.valueAt(index);
        } else {
            desc = new SuspendedEffectDesc();
            mSuspendedEffects.add((int)kKeyForSuspendAll, desc);
            ALOGV("setEffectSuspendedAll_l() add entry for 0");
        }
        if (desc->mRefCount++ == 0) {
            Vector< sp<EffectModule> > effects;
            getSuspendEligibleEffects(effects);
            for (size_t i = 0; i < effects.size(); i++) {
                setEffectSuspended_l(&effects[i]->desc().type, true);
            }
        }
    } else {
        if (index < 0) {
            return;
        }
        desc = mSuspendedEffects.valueAt(index);
        if (desc->mRefCount <= 0) {
            ALOGW("setEffectSuspendedAll_l() restore refcount should not be 0 %d", desc->mRefCount);
            desc->mRefCount = 1;
        }
        if (--desc->mRefCount == 0) {
            Vector<const effect_uuid_t *> types;
            for (size_t i = 0; i < mSuspendedEffects.size(); i++) {
                if (mSuspendedEffects.keyAt(i) == (int)kKeyForSuspendAll) {
                    continue;
                }
                types.add(&mSuspendedEffects.valueAt(i)->mType);
            }
            for (size_t i = 0; i < types.size(); i++) {
                setEffectSuspended_l(types[i], false);
            }
            ALOGV("setEffectSuspendedAll_l() remove entry for %08x",
                    mSuspendedEffects.keyAt(index));
            mSuspendedEffects.removeItem((int)kKeyForSuspendAll);
        }
    }
}


// The volume effect is used for automated tests only
#ifndef OPENSL_ES_H_
static const effect_uuid_t SL_IID_VOLUME_ = { 0x09e8ede0, 0xddde, 0x11db, 0xb4f6,
                                            { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } };
const effect_uuid_t * const SL_IID_VOLUME = &SL_IID_VOLUME_;
#endif //OPENSL_ES_H_

/* static */
bool AudioFlinger::EffectChain::isEffectEligibleForBtNrecSuspend(const effect_uuid_t *type)
{
    // Only NS and AEC are suspended when BtNRec is off
    if ((memcmp(type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0) ||
        (memcmp(type, FX_IID_NS, sizeof(effect_uuid_t)) == 0)) {
        return true;
    }
    return false;
}

bool AudioFlinger::EffectChain::isEffectEligibleForSuspend(const effect_descriptor_t& desc)
{
    // auxiliary effects and visualizer are never suspended on output mix
    if ((mSessionId == AUDIO_SESSION_OUTPUT_MIX) &&
        (((desc.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) ||
         (memcmp(&desc.type, SL_IID_VISUALIZATION, sizeof(effect_uuid_t)) == 0) ||
         (memcmp(&desc.type, SL_IID_VOLUME, sizeof(effect_uuid_t)) == 0) ||
         (memcmp(&desc.type, SL_IID_DYNAMICSPROCESSING, sizeof(effect_uuid_t)) == 0))) {
        return false;
    }
    return true;
}

void AudioFlinger::EffectChain::getSuspendEligibleEffects(
        Vector< sp<AudioFlinger::EffectModule> > &effects)
{
    effects.clear();
    for (size_t i = 0; i < mEffects.size(); i++) {
        if (isEffectEligibleForSuspend(mEffects[i]->desc())) {
            effects.add(mEffects[i]);
        }
    }
}

sp<AudioFlinger::EffectModule> AudioFlinger::EffectChain::getEffectIfEnabled(
                                                            const effect_uuid_t *type)
{
    sp<EffectModule> effect = getEffectFromType_l(type);
    return effect != 0 && effect->isEnabled() ? effect : 0;
}

void AudioFlinger::EffectChain::checkSuspendOnEffectEnabled(const sp<EffectModule>& effect,
                                                            bool enabled)
{
    ssize_t index = mSuspendedEffects.indexOfKey(effect->desc().type.timeLow);
    if (enabled) {
        if (index < 0) {
            // if the effect is not suspend check if all effects are suspended
            index = mSuspendedEffects.indexOfKey((int)kKeyForSuspendAll);
            if (index < 0) {
                return;
            }
            if (!isEffectEligibleForSuspend(effect->desc())) {
                return;
            }
            setEffectSuspended_l(&effect->desc().type, enabled);
            index = mSuspendedEffects.indexOfKey(effect->desc().type.timeLow);
            if (index < 0) {
                ALOGW("checkSuspendOnEffectEnabled() Fx should be suspended here!");
                return;
            }
        }
        ALOGV("checkSuspendOnEffectEnabled() enable suspending fx %08x",
            effect->desc().type.timeLow);
        sp<SuspendedEffectDesc> desc = mSuspendedEffects.valueAt(index);
        // if effect is requested to suspended but was not yet enabled, suspend it now.
        if (desc->mEffect == 0) {
            desc->mEffect = effect;
            effect->setEnabled(false);
            effect->setSuspended(true);
        }
    } else {
        if (index < 0) {
            return;
        }
        ALOGV("checkSuspendOnEffectEnabled() disable restoring fx %08x",
            effect->desc().type.timeLow);
        sp<SuspendedEffectDesc> desc = mSuspendedEffects.valueAt(index);
        desc->mEffect.clear();
        effect->setSuspended(false);
    }
}

bool AudioFlinger::EffectChain::isNonOffloadableEnabled()
{
    Mutex::Autolock _l(mLock);
    return isNonOffloadableEnabled_l();
}

bool AudioFlinger::EffectChain::isNonOffloadableEnabled_l()
{
    size_t size = mEffects.size();
    for (size_t i = 0; i < size; i++) {
        if (mEffects[i]->isEnabled() && !mEffects[i]->isOffloadable()) {
            return true;
        }
    }
    return false;
}

void AudioFlinger::EffectChain::setThread(const sp<ThreadBase>& thread)
{
    Mutex::Autolock _l(mLock);
    mThread = thread;
    for (size_t i = 0; i < mEffects.size(); i++) {
        mEffects[i]->setThread(thread);
    }
}

void AudioFlinger::EffectChain::checkOutputFlagCompatibility(audio_output_flags_t *flags) const
{
    if ((*flags & AUDIO_OUTPUT_FLAG_RAW) != 0 && !isRawCompatible()) {
        *flags = (audio_output_flags_t)(*flags & ~AUDIO_OUTPUT_FLAG_RAW);
    }
    if ((*flags & AUDIO_OUTPUT_FLAG_FAST) != 0 && !isFastCompatible()) {
        *flags = (audio_output_flags_t)(*flags & ~AUDIO_OUTPUT_FLAG_FAST);
    }
}

void AudioFlinger::EffectChain::checkInputFlagCompatibility(audio_input_flags_t *flags) const
{
    if ((*flags & AUDIO_INPUT_FLAG_RAW) != 0 && !isRawCompatible()) {
        *flags = (audio_input_flags_t)(*flags & ~AUDIO_INPUT_FLAG_RAW);
    }
    if ((*flags & AUDIO_INPUT_FLAG_FAST) != 0 && !isFastCompatible()) {
        *flags = (audio_input_flags_t)(*flags & ~AUDIO_INPUT_FLAG_FAST);
    }
}

bool AudioFlinger::EffectChain::isRawCompatible() const
{
    Mutex::Autolock _l(mLock);
    for (const auto &effect : mEffects) {
        if (effect->isProcessImplemented()) {
            return false;
        }
    }
    // Allow effects without processing.
    return true;
}

bool AudioFlinger::EffectChain::isFastCompatible() const
{
    Mutex::Autolock _l(mLock);
    for (const auto &effect : mEffects) {
        if (effect->isProcessImplemented()
                && effect->isImplementationSoftware()) {
            return false;
        }
    }
    // Allow effects without processing or hw accelerated effects.
    return true;
}

// isCompatibleWithThread_l() must be called with thread->mLock held
bool AudioFlinger::EffectChain::isCompatibleWithThread_l(const sp<ThreadBase>& thread) const
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mEffects.size(); i++) {
        if (thread->checkEffectCompatibility_l(&(mEffects[i]->desc()), mSessionId) != NO_ERROR) {
            return false;
        }
    }
    return true;
}

} // namespace android
