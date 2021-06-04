/*
**
** Copyright 2007, The Android Open Source Project
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

#define LOG_TAG "AudioMixer"
//#define LOG_NDEBUG 0

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Log.h>

#include <cutils/compiler.h>
#include <utils/Debug.h>

#include <system/audio.h>

#include <audio_utils/primitives.h>
#include <audio_utils/format.h>
#include <media/AudioMixer.h>

#include "AudioMixerOps.h"

// The FCC_2 macro refers to the Fixed Channel Count of 2 for the legacy integer mixer.
#ifndef FCC_2
#define FCC_2 2
#endif

// Look for MONO_HACK for any Mono hack involving legacy mono channel to
// stereo channel conversion.

/* VERY_VERY_VERBOSE_LOGGING will show exactly which process hook and track hook is
 * being used. This is a considerable amount of log spam, so don't enable unless you
 * are verifying the hook based code.
 */
//#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
//define ALOGVV printf  // for test-mixer.cpp
#else
#define ALOGVV(a...) do { } while (0)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

// Set kUseNewMixer to true to use the new mixer engine always. Otherwise the
// original code will be used for stereo sinks, the new mixer for multichannel.
static constexpr bool kUseNewMixer = true;

// Set kUseFloat to true to allow floating input into the mixer engine.
// If kUseNewMixer is false, this is ignored or may be overridden internally
// because of downmix/upmix support.
static constexpr bool kUseFloat = true;

#ifdef FLOAT_AUX
using TYPE_AUX = float;
static_assert(kUseNewMixer && kUseFloat,
        "kUseNewMixer and kUseFloat must be true for FLOAT_AUX option");
#else
using TYPE_AUX = int32_t; // q4.27
#endif

// Set to default copy buffer size in frames for input processing.
static const size_t kCopyBufferFrameCount = 256;

namespace android {

// ----------------------------------------------------------------------------

static inline audio_format_t selectMixerInFormat(audio_format_t inputFormat __unused) {
    return kUseFloat && kUseNewMixer ? AUDIO_FORMAT_PCM_FLOAT : AUDIO_FORMAT_PCM_16_BIT;
}

status_t AudioMixer::create(
        int name, audio_channel_mask_t channelMask, audio_format_t format, int sessionId)
{
    LOG_ALWAYS_FATAL_IF(exists(name), "name %d already exists", name);

    if (!isValidChannelMask(channelMask)) {
        ALOGE("%s invalid channelMask: %#x", __func__, channelMask);
        return BAD_VALUE;
    }
    if (!isValidFormat(format)) {
        ALOGE("%s invalid format: %#x", __func__, format);
        return BAD_VALUE;
    }

    auto t = std::make_shared<Track>();
    {
        // TODO: move initialization to the Track constructor.
        // assume default parameters for the track, except where noted below
        t->needs = 0;

        // Integer volume.
        // Currently integer volume is kept for the legacy integer mixer.
        // Will be removed when the legacy mixer path is removed.
        t->volume[0] = 0;
        t->volume[1] = 0;
        t->prevVolume[0] = 0 << 16;
        t->prevVolume[1] = 0 << 16;
        t->volumeInc[0] = 0;
        t->volumeInc[1] = 0;
        t->auxLevel = 0;
        t->auxInc = 0;
        t->prevAuxLevel = 0;

        // Floating point volume.
        t->mVolume[0] = 0.f;
        t->mVolume[1] = 0.f;
        t->mPrevVolume[0] = 0.f;
        t->mPrevVolume[1] = 0.f;
        t->mVolumeInc[0] = 0.;
        t->mVolumeInc[1] = 0.;
        t->mAuxLevel = 0.;
        t->mAuxInc = 0.;
        t->mPrevAuxLevel = 0.;

        // no initialization needed
        // t->frameCount
        t->mHapticChannelMask = channelMask & AUDIO_CHANNEL_HAPTIC_ALL;
        t->mHapticChannelCount = audio_channel_count_from_out_mask(t->mHapticChannelMask);
        channelMask &= ~AUDIO_CHANNEL_HAPTIC_ALL;
        t->channelCount = audio_channel_count_from_out_mask(channelMask);
        t->enabled = false;
        ALOGV_IF(audio_channel_mask_get_bits(channelMask) != AUDIO_CHANNEL_OUT_STEREO,
                "Non-stereo channel mask: %d\n", channelMask);
        t->channelMask = channelMask;
        t->sessionId = sessionId;
        // setBufferProvider(name, AudioBufferProvider *) is required before enable(name)
        t->bufferProvider = NULL;
        t->buffer.raw = NULL;
        // no initialization needed
        // t->buffer.frameCount
        t->hook = NULL;
        t->mIn = NULL;
        t->sampleRate = mSampleRate;
        // setParameter(name, TRACK, MAIN_BUFFER, mixBuffer) is required before enable(name)
        t->mainBuffer = NULL;
        t->auxBuffer = NULL;
        t->mInputBufferProvider = NULL;
        t->mMixerFormat = AUDIO_FORMAT_PCM_16_BIT;
        t->mFormat = format;
        t->mMixerInFormat = selectMixerInFormat(format);
        t->mDownmixRequiresFormat = AUDIO_FORMAT_INVALID; // no format required
        t->mMixerChannelMask = audio_channel_mask_from_representation_and_bits(
                AUDIO_CHANNEL_REPRESENTATION_POSITION, AUDIO_CHANNEL_OUT_STEREO);
        t->mMixerChannelCount = audio_channel_count_from_out_mask(t->mMixerChannelMask);
        t->mPlaybackRate = AUDIO_PLAYBACK_RATE_DEFAULT;
        // haptic
        t->mHapticPlaybackEnabled = false;
        t->mHapticIntensity = HAPTIC_SCALE_NONE;
        t->mMixerHapticChannelMask = AUDIO_CHANNEL_NONE;
        t->mMixerHapticChannelCount = 0;
        t->mAdjustInChannelCount = t->channelCount + t->mHapticChannelCount;
        t->mAdjustOutChannelCount = t->channelCount + t->mMixerHapticChannelCount;
        t->mAdjustNonDestructiveInChannelCount = t->mAdjustOutChannelCount;
        t->mAdjustNonDestructiveOutChannelCount = t->channelCount;
        t->mKeepContractedChannels = false;
        // Check the downmixing (or upmixing) requirements.
        status_t status = t->prepareForDownmix();
        if (status != OK) {
            ALOGE("AudioMixer::getTrackName invalid channelMask (%#x)", channelMask);
            return BAD_VALUE;
        }
        // prepareForDownmix() may change mDownmixRequiresFormat
        ALOGVV("mMixerFormat:%#x  mMixerInFormat:%#x\n", t->mMixerFormat, t->mMixerInFormat);
        t->prepareForReformat();
        t->prepareForAdjustChannelsNonDestructive(mFrameCount);
        t->prepareForAdjustChannels();

        mTracks[name] = t;
        return OK;
    }
}

// Called when channel masks have changed for a track name
// TODO: Fix DownmixerBufferProvider not to (possibly) change mixer input format,
// which will simplify this logic.
bool AudioMixer::setChannelMasks(int name,
        audio_channel_mask_t trackChannelMask, audio_channel_mask_t mixerChannelMask) {
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    const std::shared_ptr<Track> &track = mTracks[name];

    if (trackChannelMask == (track->channelMask | track->mHapticChannelMask)
            && mixerChannelMask == (track->mMixerChannelMask | track->mMixerHapticChannelMask)) {
        return false;  // no need to change
    }
    const audio_channel_mask_t hapticChannelMask = trackChannelMask & AUDIO_CHANNEL_HAPTIC_ALL;
    trackChannelMask &= ~AUDIO_CHANNEL_HAPTIC_ALL;
    const audio_channel_mask_t mixerHapticChannelMask = mixerChannelMask & AUDIO_CHANNEL_HAPTIC_ALL;
    mixerChannelMask &= ~AUDIO_CHANNEL_HAPTIC_ALL;
    // always recompute for both channel masks even if only one has changed.
    const uint32_t trackChannelCount = audio_channel_count_from_out_mask(trackChannelMask);
    const uint32_t mixerChannelCount = audio_channel_count_from_out_mask(mixerChannelMask);
    const uint32_t hapticChannelCount = audio_channel_count_from_out_mask(hapticChannelMask);
    const uint32_t mixerHapticChannelCount =
            audio_channel_count_from_out_mask(mixerHapticChannelMask);

    ALOG_ASSERT((trackChannelCount <= MAX_NUM_CHANNELS_TO_DOWNMIX)
            && trackChannelCount
            && mixerChannelCount);
    track->channelMask = trackChannelMask;
    track->channelCount = trackChannelCount;
    track->mMixerChannelMask = mixerChannelMask;
    track->mMixerChannelCount = mixerChannelCount;
    track->mHapticChannelMask = hapticChannelMask;
    track->mHapticChannelCount = hapticChannelCount;
    track->mMixerHapticChannelMask = mixerHapticChannelMask;
    track->mMixerHapticChannelCount = mixerHapticChannelCount;

    if (track->mHapticChannelCount > 0) {
        track->mAdjustInChannelCount = track->channelCount + track->mHapticChannelCount;
        track->mAdjustOutChannelCount = track->channelCount + track->mMixerHapticChannelCount;
        track->mAdjustNonDestructiveInChannelCount = track->mAdjustOutChannelCount;
        track->mAdjustNonDestructiveOutChannelCount = track->channelCount;
        track->mKeepContractedChannels = track->mHapticPlaybackEnabled;
    } else {
        track->mAdjustInChannelCount = 0;
        track->mAdjustOutChannelCount = 0;
        track->mAdjustNonDestructiveInChannelCount = 0;
        track->mAdjustNonDestructiveOutChannelCount = 0;
        track->mKeepContractedChannels = false;
    }

    // channel masks have changed, does this track need a downmixer?
    // update to try using our desired format (if we aren't already using it)
    const status_t status = track->prepareForDownmix();
    ALOGE_IF(status != OK,
            "prepareForDownmix error %d, track channel mask %#x, mixer channel mask %#x",
            status, track->channelMask, track->mMixerChannelMask);

    // always do reformat since channel mask changed,
    // do it after downmix since track format may change!
    track->prepareForReformat();

    track->prepareForAdjustChannelsNonDestructive(mFrameCount);
    track->prepareForAdjustChannels();

    if (track->mResampler.get() != nullptr) {
        // resampler channels may have changed.
        const uint32_t resetToSampleRate = track->sampleRate;
        track->mResampler.reset(nullptr);
        track->sampleRate = mSampleRate; // without resampler, track rate is device sample rate.
        // recreate the resampler with updated format, channels, saved sampleRate.
        track->setResampler(resetToSampleRate /*trackSampleRate*/, mSampleRate /*devSampleRate*/);
    }
    return true;
}

void AudioMixer::Track::unprepareForDownmix() {
    ALOGV("AudioMixer::unprepareForDownmix(%p)", this);

    if (mPostDownmixReformatBufferProvider.get() != nullptr) {
        // release any buffers held by the mPostDownmixReformatBufferProvider
        // before deallocating the mDownmixerBufferProvider.
        mPostDownmixReformatBufferProvider->reset();
    }

    mDownmixRequiresFormat = AUDIO_FORMAT_INVALID;
    if (mDownmixerBufferProvider.get() != nullptr) {
        // this track had previously been configured with a downmixer, delete it
        mDownmixerBufferProvider.reset(nullptr);
        reconfigureBufferProviders();
    } else {
        ALOGV(" nothing to do, no downmixer to delete");
    }
}

status_t AudioMixer::Track::prepareForDownmix()
{
    ALOGV("AudioMixer::prepareForDownmix(%p) with mask 0x%x",
            this, channelMask);

    // discard the previous downmixer if there was one
    unprepareForDownmix();
    // MONO_HACK Only remix (upmix or downmix) if the track and mixer/device channel masks
    // are not the same and not handled internally, as mono -> stereo currently is.
    if (channelMask == mMixerChannelMask
            || (channelMask == AUDIO_CHANNEL_OUT_MONO
                    && mMixerChannelMask == AUDIO_CHANNEL_OUT_STEREO)) {
        return NO_ERROR;
    }
    // DownmixerBufferProvider is only used for position masks.
    if (audio_channel_mask_get_representation(channelMask)
                == AUDIO_CHANNEL_REPRESENTATION_POSITION
            && DownmixerBufferProvider::isMultichannelCapable()) {

        // Check if we have a float or int16 downmixer, in that order.
        for (const audio_format_t format : { AUDIO_FORMAT_PCM_FLOAT, AUDIO_FORMAT_PCM_16_BIT }) {
            mDownmixerBufferProvider.reset(new DownmixerBufferProvider(
                    channelMask, mMixerChannelMask,
                    format,
                    sampleRate, sessionId, kCopyBufferFrameCount));
            if (static_cast<DownmixerBufferProvider *>(mDownmixerBufferProvider.get())
                    ->isValid()) {
                mDownmixRequiresFormat = format;
                reconfigureBufferProviders();
                return NO_ERROR;
            }
        }
        // mDownmixerBufferProvider reset below.
    }

    // Effect downmixer does not accept the channel conversion.  Let's use our remixer.
    mDownmixerBufferProvider.reset(new RemixBufferProvider(channelMask,
            mMixerChannelMask, mMixerInFormat, kCopyBufferFrameCount));
    // Remix always finds a conversion whereas Downmixer effect above may fail.
    reconfigureBufferProviders();
    return NO_ERROR;
}

void AudioMixer::Track::unprepareForReformat() {
    ALOGV("AudioMixer::unprepareForReformat(%p)", this);
    bool requiresReconfigure = false;
    if (mReformatBufferProvider.get() != nullptr) {
        mReformatBufferProvider.reset(nullptr);
        requiresReconfigure = true;
    }
    if (mPostDownmixReformatBufferProvider.get() != nullptr) {
        mPostDownmixReformatBufferProvider.reset(nullptr);
        requiresReconfigure = true;
    }
    if (requiresReconfigure) {
        reconfigureBufferProviders();
    }
}

status_t AudioMixer::Track::prepareForReformat()
{
    ALOGV("AudioMixer::prepareForReformat(%p) with format %#x", this, mFormat);
    // discard previous reformatters
    unprepareForReformat();
    // only configure reformatters as needed
    const audio_format_t targetFormat = mDownmixRequiresFormat != AUDIO_FORMAT_INVALID
            ? mDownmixRequiresFormat : mMixerInFormat;
    bool requiresReconfigure = false;
    if (mFormat != targetFormat) {
        mReformatBufferProvider.reset(new ReformatBufferProvider(
                audio_channel_count_from_out_mask(channelMask),
                mFormat,
                targetFormat,
                kCopyBufferFrameCount));
        requiresReconfigure = true;
    } else if (mFormat == AUDIO_FORMAT_PCM_FLOAT) {
        // Input and output are floats, make sure application did not provide > 3db samples
        // that would break volume application (b/68099072)
        // TODO: add a trusted source flag to avoid the overhead
        mReformatBufferProvider.reset(new ClampFloatBufferProvider(
                audio_channel_count_from_out_mask(channelMask),
                kCopyBufferFrameCount));
        requiresReconfigure = true;
    }
    if (targetFormat != mMixerInFormat) {
        mPostDownmixReformatBufferProvider.reset(new ReformatBufferProvider(
                audio_channel_count_from_out_mask(mMixerChannelMask),
                targetFormat,
                mMixerInFormat,
                kCopyBufferFrameCount));
        requiresReconfigure = true;
    }
    if (requiresReconfigure) {
        reconfigureBufferProviders();
    }
    return NO_ERROR;
}

void AudioMixer::Track::unprepareForAdjustChannels()
{
    ALOGV("AUDIOMIXER::unprepareForAdjustChannels");
    if (mAdjustChannelsBufferProvider.get() != nullptr) {
        mAdjustChannelsBufferProvider.reset(nullptr);
        reconfigureBufferProviders();
    }
}

status_t AudioMixer::Track::prepareForAdjustChannels()
{
    ALOGV("AudioMixer::prepareForAdjustChannels(%p) with inChannelCount: %u, outChannelCount: %u",
            this, mAdjustInChannelCount, mAdjustOutChannelCount);
    unprepareForAdjustChannels();
    if (mAdjustInChannelCount != mAdjustOutChannelCount) {
        mAdjustChannelsBufferProvider.reset(new AdjustChannelsBufferProvider(
                mFormat, mAdjustInChannelCount, mAdjustOutChannelCount, kCopyBufferFrameCount));
        reconfigureBufferProviders();
    }
    return NO_ERROR;
}

void AudioMixer::Track::unprepareForAdjustChannelsNonDestructive()
{
    ALOGV("AUDIOMIXER::unprepareForAdjustChannelsNonDestructive");
    if (mContractChannelsNonDestructiveBufferProvider.get() != nullptr) {
        mContractChannelsNonDestructiveBufferProvider.reset(nullptr);
        reconfigureBufferProviders();
    }
}

status_t AudioMixer::Track::prepareForAdjustChannelsNonDestructive(size_t frames)
{
    ALOGV("AudioMixer::prepareForAdjustChannelsNonDestructive(%p) with inChannelCount: %u, "
          "outChannelCount: %u, keepContractedChannels: %d",
            this, mAdjustNonDestructiveInChannelCount, mAdjustNonDestructiveOutChannelCount,
            mKeepContractedChannels);
    unprepareForAdjustChannelsNonDestructive();
    if (mAdjustNonDestructiveInChannelCount != mAdjustNonDestructiveOutChannelCount) {
        uint8_t* buffer = mKeepContractedChannels
                ? (uint8_t*)mainBuffer + frames * audio_bytes_per_frame(
                        mMixerChannelCount, mMixerFormat)
                : NULL;
        mContractChannelsNonDestructiveBufferProvider.reset(
                new AdjustChannelsBufferProvider(
                        mFormat,
                        mAdjustNonDestructiveInChannelCount,
                        mAdjustNonDestructiveOutChannelCount,
                        frames,
                        mKeepContractedChannels ? mMixerFormat : AUDIO_FORMAT_INVALID,
                        buffer));
        reconfigureBufferProviders();
    }
    return NO_ERROR;
}

void AudioMixer::Track::clearContractedBuffer()
{
    if (mContractChannelsNonDestructiveBufferProvider.get() != nullptr) {
        static_cast<AdjustChannelsBufferProvider*>(
                mContractChannelsNonDestructiveBufferProvider.get())->clearContractedFrames();
    }
}

void AudioMixer::Track::reconfigureBufferProviders()
{
    // configure from upstream to downstream buffer providers.
    bufferProvider = mInputBufferProvider;
    if (mAdjustChannelsBufferProvider.get() != nullptr) {
        mAdjustChannelsBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mAdjustChannelsBufferProvider.get();
    }
    if (mContractChannelsNonDestructiveBufferProvider.get() != nullptr) {
        mContractChannelsNonDestructiveBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mContractChannelsNonDestructiveBufferProvider.get();
    }
    if (mReformatBufferProvider.get() != nullptr) {
        mReformatBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mReformatBufferProvider.get();
    }
    if (mDownmixerBufferProvider.get() != nullptr) {
        mDownmixerBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mDownmixerBufferProvider.get();
    }
    if (mPostDownmixReformatBufferProvider.get() != nullptr) {
        mPostDownmixReformatBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mPostDownmixReformatBufferProvider.get();
    }
    if (mTimestretchBufferProvider.get() != nullptr) {
        mTimestretchBufferProvider->setBufferProvider(bufferProvider);
        bufferProvider = mTimestretchBufferProvider.get();
    }
}

void AudioMixer::destroy(int name)
{
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    ALOGV("deleteTrackName(%d)", name);

    if (mTracks[name]->enabled) {
        invalidate();
    }
    mTracks.erase(name); // deallocate track
}

void AudioMixer::enable(int name)
{
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    const std::shared_ptr<Track> &track = mTracks[name];

    if (!track->enabled) {
        track->enabled = true;
        ALOGV("enable(%d)", name);
        invalidate();
    }
}

void AudioMixer::disable(int name)
{
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    const std::shared_ptr<Track> &track = mTracks[name];

    if (track->enabled) {
        track->enabled = false;
        ALOGV("disable(%d)", name);
        invalidate();
    }
}

/* Sets the volume ramp variables for the AudioMixer.
 *
 * The volume ramp variables are used to transition from the previous
 * volume to the set volume.  ramp controls the duration of the transition.
 * Its value is typically one state framecount period, but may also be 0,
 * meaning "immediate."
 *
 * FIXME: 1) Volume ramp is enabled only if there is a nonzero integer increment
 * even if there is a nonzero floating point increment (in that case, the volume
 * change is immediate).  This restriction should be changed when the legacy mixer
 * is removed (see #2).
 * FIXME: 2) Integer volume variables are used for Legacy mixing and should be removed
 * when no longer needed.
 *
 * @param newVolume set volume target in floating point [0.0, 1.0].
 * @param ramp number of frames to increment over. if ramp is 0, the volume
 * should be set immediately.  Currently ramp should not exceed 65535 (frames).
 * @param pIntSetVolume pointer to the U4.12 integer target volume, set on return.
 * @param pIntPrevVolume pointer to the U4.28 integer previous volume, set on return.
 * @param pIntVolumeInc pointer to the U4.28 increment per output audio frame, set on return.
 * @param pSetVolume pointer to the float target volume, set on return.
 * @param pPrevVolume pointer to the float previous volume, set on return.
 * @param pVolumeInc pointer to the float increment per output audio frame, set on return.
 * @return true if the volume has changed, false if volume is same.
 */
static inline bool setVolumeRampVariables(float newVolume, int32_t ramp,
        int16_t *pIntSetVolume, int32_t *pIntPrevVolume, int32_t *pIntVolumeInc,
        float *pSetVolume, float *pPrevVolume, float *pVolumeInc) {
    // check floating point volume to see if it is identical to the previously
    // set volume.
    // We do not use a tolerance here (and reject changes too small)
    // as it may be confusing to use a different value than the one set.
    // If the resulting volume is too small to ramp, it is a direct set of the volume.
    if (newVolume == *pSetVolume) {
        return false;
    }
    if (newVolume < 0) {
        newVolume = 0; // should not have negative volumes
    } else {
        switch (fpclassify(newVolume)) {
        case FP_SUBNORMAL:
        case FP_NAN:
            newVolume = 0;
            break;
        case FP_ZERO:
            break; // zero volume is fine
        case FP_INFINITE:
            // Infinite volume could be handled consistently since
            // floating point math saturates at infinities,
            // but we limit volume to unity gain float.
            // ramp = 0; break;
            //
            newVolume = AudioMixer::UNITY_GAIN_FLOAT;
            break;
        case FP_NORMAL:
        default:
            // Floating point does not have problems with overflow wrap
            // that integer has.  However, we limit the volume to
            // unity gain here.
            // TODO: Revisit the volume limitation and perhaps parameterize.
            if (newVolume > AudioMixer::UNITY_GAIN_FLOAT) {
                newVolume = AudioMixer::UNITY_GAIN_FLOAT;
            }
            break;
        }
    }

    // set floating point volume ramp
    if (ramp != 0) {
        // when the ramp completes, *pPrevVolume is set to *pSetVolume, so there
        // is no computational mismatch; hence equality is checked here.
        ALOGD_IF(*pPrevVolume != *pSetVolume, "previous float ramp hasn't finished,"
                " prev:%f  set_to:%f", *pPrevVolume, *pSetVolume);
        const float inc = (newVolume - *pPrevVolume) / ramp; // could be inf, nan, subnormal
        // could be inf, cannot be nan, subnormal
        const float maxv = std::max(newVolume, *pPrevVolume);

        if (isnormal(inc) // inc must be a normal number (no subnormals, infinite, nan)
                && maxv + inc != maxv) { // inc must make forward progress
            *pVolumeInc = inc;
            // ramp is set now.
            // Note: if newVolume is 0, then near the end of the ramp,
            // it may be possible that the ramped volume may be subnormal or
            // temporarily negative by a small amount or subnormal due to floating
            // point inaccuracies.
        } else {
            ramp = 0; // ramp not allowed
        }
    }

    // compute and check integer volume, no need to check negative values
    // The integer volume is limited to "unity_gain" to avoid wrapping and other
    // audio artifacts, so it never reaches the range limit of U4.28.
    // We safely use signed 16 and 32 bit integers here.
    const float scaledVolume = newVolume * AudioMixer::UNITY_GAIN_INT; // not neg, subnormal, nan
    const int32_t intVolume = (scaledVolume >= (float)AudioMixer::UNITY_GAIN_INT) ?
            AudioMixer::UNITY_GAIN_INT : (int32_t)scaledVolume;

    // set integer volume ramp
    if (ramp != 0) {
        // integer volume is U4.12 (to use 16 bit multiplies), but ramping uses U4.28.
        // when the ramp completes, *pIntPrevVolume is set to *pIntSetVolume << 16, so there
        // is no computational mismatch; hence equality is checked here.
        ALOGD_IF(*pIntPrevVolume != *pIntSetVolume << 16, "previous int ramp hasn't finished,"
                " prev:%d  set_to:%d", *pIntPrevVolume, *pIntSetVolume << 16);
        const int32_t inc = ((intVolume << 16) - *pIntPrevVolume) / ramp;

        if (inc != 0) { // inc must make forward progress
            *pIntVolumeInc = inc;
        } else {
            ramp = 0; // ramp not allowed
        }
    }

    // if no ramp, or ramp not allowed, then clear float and integer increments
    if (ramp == 0) {
        *pVolumeInc = 0;
        *pPrevVolume = newVolume;
        *pIntVolumeInc = 0;
        *pIntPrevVolume = intVolume << 16;
    }
    *pSetVolume = newVolume;
    *pIntSetVolume = intVolume;
    return true;
}

void AudioMixer::setParameter(int name, int target, int param, void *value)
{
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    const std::shared_ptr<Track> &track = mTracks[name];

    int valueInt = static_cast<int>(reinterpret_cast<uintptr_t>(value));
    int32_t *valueBuf = reinterpret_cast<int32_t*>(value);

    switch (target) {

    case TRACK:
        switch (param) {
        case CHANNEL_MASK: {
            const audio_channel_mask_t trackChannelMask =
                static_cast<audio_channel_mask_t>(valueInt);
            if (setChannelMasks(name, trackChannelMask,
                    (track->mMixerChannelMask | track->mMixerHapticChannelMask))) {
                ALOGV("setParameter(TRACK, CHANNEL_MASK, %x)", trackChannelMask);
                invalidate();
            }
            } break;
        case MAIN_BUFFER:
            if (track->mainBuffer != valueBuf) {
                track->mainBuffer = valueBuf;
                ALOGV("setParameter(TRACK, MAIN_BUFFER, %p)", valueBuf);
                if (track->mKeepContractedChannels) {
                    track->prepareForAdjustChannelsNonDestructive(mFrameCount);
                }
                invalidate();
            }
            break;
        case AUX_BUFFER:
            if (track->auxBuffer != valueBuf) {
                track->auxBuffer = valueBuf;
                ALOGV("setParameter(TRACK, AUX_BUFFER, %p)", valueBuf);
                invalidate();
            }
            break;
        case FORMAT: {
            audio_format_t format = static_cast<audio_format_t>(valueInt);
            if (track->mFormat != format) {
                ALOG_ASSERT(audio_is_linear_pcm(format), "Invalid format %#x", format);
                track->mFormat = format;
                ALOGV("setParameter(TRACK, FORMAT, %#x)", format);
                track->prepareForReformat();
                invalidate();
            }
            } break;
        // FIXME do we want to support setting the downmix type from AudioFlinger?
        //         for a specific track? or per mixer?
        /* case DOWNMIX_TYPE:
            break          */
        case MIXER_FORMAT: {
            audio_format_t format = static_cast<audio_format_t>(valueInt);
            if (track->mMixerFormat != format) {
                track->mMixerFormat = format;
                ALOGV("setParameter(TRACK, MIXER_FORMAT, %#x)", format);
                if (track->mKeepContractedChannels) {
                    track->prepareForAdjustChannelsNonDestructive(mFrameCount);
                }
            }
            } break;
        case MIXER_CHANNEL_MASK: {
            const audio_channel_mask_t mixerChannelMask =
                    static_cast<audio_channel_mask_t>(valueInt);
            if (setChannelMasks(name, track->channelMask | track->mHapticChannelMask,
                    mixerChannelMask)) {
                ALOGV("setParameter(TRACK, MIXER_CHANNEL_MASK, %#x)", mixerChannelMask);
                invalidate();
            }
            } break;
        case HAPTIC_ENABLED: {
            const bool hapticPlaybackEnabled = static_cast<bool>(valueInt);
            if (track->mHapticPlaybackEnabled != hapticPlaybackEnabled) {
                track->mHapticPlaybackEnabled = hapticPlaybackEnabled;
                track->mKeepContractedChannels = hapticPlaybackEnabled;
                track->prepareForAdjustChannelsNonDestructive(mFrameCount);
                track->prepareForAdjustChannels();
            }
            } break;
        case HAPTIC_INTENSITY: {
            const haptic_intensity_t hapticIntensity = static_cast<haptic_intensity_t>(valueInt);
            if (track->mHapticIntensity != hapticIntensity) {
                track->mHapticIntensity = hapticIntensity;
            }
            } break;
        default:
            LOG_ALWAYS_FATAL("setParameter track: bad param %d", param);
        }
        break;

    case RESAMPLE:
        switch (param) {
        case SAMPLE_RATE:
            ALOG_ASSERT(valueInt > 0, "bad sample rate %d", valueInt);
            if (track->setResampler(uint32_t(valueInt), mSampleRate)) {
                ALOGV("setParameter(RESAMPLE, SAMPLE_RATE, %u)",
                        uint32_t(valueInt));
                invalidate();
            }
            break;
        case RESET:
            track->resetResampler();
            invalidate();
            break;
        case REMOVE:
            track->mResampler.reset(nullptr);
            track->sampleRate = mSampleRate;
            invalidate();
            break;
        default:
            LOG_ALWAYS_FATAL("setParameter resample: bad param %d", param);
        }
        break;

    case RAMP_VOLUME:
    case VOLUME:
        switch (param) {
        case AUXLEVEL:
            if (setVolumeRampVariables(*reinterpret_cast<float*>(value),
                    target == RAMP_VOLUME ? mFrameCount : 0,
                    &track->auxLevel, &track->prevAuxLevel, &track->auxInc,
                    &track->mAuxLevel, &track->mPrevAuxLevel, &track->mAuxInc)) {
                ALOGV("setParameter(%s, AUXLEVEL: %04x)",
                        target == VOLUME ? "VOLUME" : "RAMP_VOLUME", track->auxLevel);
                invalidate();
            }
            break;
        default:
            if ((unsigned)param >= VOLUME0 && (unsigned)param < VOLUME0 + MAX_NUM_VOLUMES) {
                if (setVolumeRampVariables(*reinterpret_cast<float*>(value),
                        target == RAMP_VOLUME ? mFrameCount : 0,
                        &track->volume[param - VOLUME0],
                        &track->prevVolume[param - VOLUME0],
                        &track->volumeInc[param - VOLUME0],
                        &track->mVolume[param - VOLUME0],
                        &track->mPrevVolume[param - VOLUME0],
                        &track->mVolumeInc[param - VOLUME0])) {
                    ALOGV("setParameter(%s, VOLUME%d: %04x)",
                            target == VOLUME ? "VOLUME" : "RAMP_VOLUME", param - VOLUME0,
                                    track->volume[param - VOLUME0]);
                    invalidate();
                }
            } else {
                LOG_ALWAYS_FATAL("setParameter volume: bad param %d", param);
            }
        }
        break;
        case TIMESTRETCH:
            switch (param) {
            case PLAYBACK_RATE: {
                const AudioPlaybackRate *playbackRate =
                        reinterpret_cast<AudioPlaybackRate*>(value);
                ALOGW_IF(!isAudioPlaybackRateValid(*playbackRate),
                        "bad parameters speed %f, pitch %f",
                        playbackRate->mSpeed, playbackRate->mPitch);
                if (track->setPlaybackRate(*playbackRate)) {
                    ALOGV("setParameter(TIMESTRETCH, PLAYBACK_RATE, STRETCH_MODE, FALLBACK_MODE "
                            "%f %f %d %d",
                            playbackRate->mSpeed,
                            playbackRate->mPitch,
                            playbackRate->mStretchMode,
                            playbackRate->mFallbackMode);
                    // invalidate();  (should not require reconfigure)
                }
            } break;
            default:
                LOG_ALWAYS_FATAL("setParameter timestretch: bad param %d", param);
            }
            break;

    default:
        LOG_ALWAYS_FATAL("setParameter: bad target %d", target);
    }
}

bool AudioMixer::Track::setResampler(uint32_t trackSampleRate, uint32_t devSampleRate)
{
    if (trackSampleRate != devSampleRate || mResampler.get() != nullptr) {
        if (sampleRate != trackSampleRate) {
            sampleRate = trackSampleRate;
            if (mResampler.get() == nullptr) {
                ALOGV("Creating resampler from track %d Hz to device %d Hz",
                        trackSampleRate, devSampleRate);
                AudioResampler::src_quality quality;
                // force lowest quality level resampler if use case isn't music or video
                // FIXME this is flawed for dynamic sample rates, as we choose the resampler
                // quality level based on the initial ratio, but that could change later.
                // Should have a way to distinguish tracks with static ratios vs. dynamic ratios.
                if (isMusicRate(trackSampleRate)) {
                    quality = AudioResampler::DEFAULT_QUALITY;
                } else {
                    quality = AudioResampler::DYN_LOW_QUALITY;
                }

                // TODO: Remove MONO_HACK. Resampler sees #channels after the downmixer
                // but if none exists, it is the channel count (1 for mono).
                const int resamplerChannelCount = mDownmixerBufferProvider.get() != nullptr
                        ? mMixerChannelCount : channelCount;
                ALOGVV("Creating resampler:"
                        " format(%#x) channels(%d) devSampleRate(%u) quality(%d)\n",
                        mMixerInFormat, resamplerChannelCount, devSampleRate, quality);
                mResampler.reset(AudioResampler::create(
                        mMixerInFormat,
                        resamplerChannelCount,
                        devSampleRate, quality));
            }
            return true;
        }
    }
    return false;
}

bool AudioMixer::Track::setPlaybackRate(const AudioPlaybackRate &playbackRate)
{
    if ((mTimestretchBufferProvider.get() == nullptr &&
            fabs(playbackRate.mSpeed - mPlaybackRate.mSpeed) < AUDIO_TIMESTRETCH_SPEED_MIN_DELTA &&
            fabs(playbackRate.mPitch - mPlaybackRate.mPitch) < AUDIO_TIMESTRETCH_PITCH_MIN_DELTA) ||
            isAudioPlaybackRateEqual(playbackRate, mPlaybackRate)) {
        return false;
    }
    mPlaybackRate = playbackRate;
    if (mTimestretchBufferProvider.get() == nullptr) {
        // TODO: Remove MONO_HACK. Resampler sees #channels after the downmixer
        // but if none exists, it is the channel count (1 for mono).
        const int timestretchChannelCount = mDownmixerBufferProvider.get() != nullptr
                ? mMixerChannelCount : channelCount;
        mTimestretchBufferProvider.reset(new TimestretchBufferProvider(timestretchChannelCount,
                mMixerInFormat, sampleRate, playbackRate));
        reconfigureBufferProviders();
    } else {
        static_cast<TimestretchBufferProvider*>(mTimestretchBufferProvider.get())
                ->setPlaybackRate(playbackRate);
    }
    return true;
}

/* Checks to see if the volume ramp has completed and clears the increment
 * variables appropriately.
 *
 * FIXME: There is code to handle int/float ramp variable switchover should it not
 * complete within a mixer buffer processing call, but it is preferred to avoid switchover
 * due to precision issues.  The switchover code is included for legacy code purposes
 * and can be removed once the integer volume is removed.
 *
 * It is not sufficient to clear only the volumeInc integer variable because
 * if one channel requires ramping, all channels are ramped.
 *
 * There is a bit of duplicated code here, but it keeps backward compatibility.
 */
inline void AudioMixer::Track::adjustVolumeRamp(bool aux, bool useFloat)
{
    if (useFloat) {
        for (uint32_t i = 0; i < MAX_NUM_VOLUMES; i++) {
            if ((mVolumeInc[i] > 0 && mPrevVolume[i] + mVolumeInc[i] >= mVolume[i]) ||
                     (mVolumeInc[i] < 0 && mPrevVolume[i] + mVolumeInc[i] <= mVolume[i])) {
                volumeInc[i] = 0;
                prevVolume[i] = volume[i] << 16;
                mVolumeInc[i] = 0.;
                mPrevVolume[i] = mVolume[i];
            } else {
                //ALOGV("ramp: %f %f %f", mVolume[i], mPrevVolume[i], mVolumeInc[i]);
                prevVolume[i] = u4_28_from_float(mPrevVolume[i]);
            }
        }
    } else {
        for (uint32_t i = 0; i < MAX_NUM_VOLUMES; i++) {
            if (((volumeInc[i]>0) && (((prevVolume[i]+volumeInc[i])>>16) >= volume[i])) ||
                    ((volumeInc[i]<0) && (((prevVolume[i]+volumeInc[i])>>16) <= volume[i]))) {
                volumeInc[i] = 0;
                prevVolume[i] = volume[i] << 16;
                mVolumeInc[i] = 0.;
                mPrevVolume[i] = mVolume[i];
            } else {
                //ALOGV("ramp: %d %d %d", volume[i] << 16, prevVolume[i], volumeInc[i]);
                mPrevVolume[i]  = float_from_u4_28(prevVolume[i]);
            }
        }
    }

    if (aux) {
#ifdef FLOAT_AUX
        if (useFloat) {
            if ((mAuxInc > 0.f && mPrevAuxLevel + mAuxInc >= mAuxLevel) ||
                    (mAuxInc < 0.f && mPrevAuxLevel + mAuxInc <= mAuxLevel)) {
                auxInc = 0;
                prevAuxLevel = auxLevel << 16;
                mAuxInc = 0.f;
                mPrevAuxLevel = mAuxLevel;
            }
        } else
#endif
        if ((auxInc > 0 && ((prevAuxLevel + auxInc) >> 16) >= auxLevel) ||
                (auxInc < 0 && ((prevAuxLevel + auxInc) >> 16) <= auxLevel)) {
            auxInc = 0;
            prevAuxLevel = auxLevel << 16;
            mAuxInc = 0.f;
            mPrevAuxLevel = mAuxLevel;
        }
    }
}

size_t AudioMixer::getUnreleasedFrames(int name) const
{
    const auto it = mTracks.find(name);
    if (it != mTracks.end()) {
        return it->second->getUnreleasedFrames();
    }
    return 0;
}

void AudioMixer::setBufferProvider(int name, AudioBufferProvider* bufferProvider)
{
    LOG_ALWAYS_FATAL_IF(!exists(name), "invalid name: %d", name);
    const std::shared_ptr<Track> &track = mTracks[name];

    if (track->mInputBufferProvider == bufferProvider) {
        return; // don't reset any buffer providers if identical.
    }
    // reset order from downstream to upstream buffer providers.
    if (track->mTimestretchBufferProvider.get() != nullptr) {
        track->mTimestretchBufferProvider->reset();
    } else if (track->mPostDownmixReformatBufferProvider.get() != nullptr) {
        track->mPostDownmixReformatBufferProvider->reset();
    } else if (track->mDownmixerBufferProvider != nullptr) {
        track->mDownmixerBufferProvider->reset();
    } else if (track->mReformatBufferProvider.get() != nullptr) {
        track->mReformatBufferProvider->reset();
    } else if (track->mContractChannelsNonDestructiveBufferProvider.get() != nullptr) {
        track->mContractChannelsNonDestructiveBufferProvider->reset();
    } else if (track->mAdjustChannelsBufferProvider.get() != nullptr) {
        track->mAdjustChannelsBufferProvider->reset();
    }

    track->mInputBufferProvider = bufferProvider;
    track->reconfigureBufferProviders();
}

void AudioMixer::process__validate()
{
    // TODO: fix all16BitsStereNoResample logic to
    // either properly handle muted tracks (it should ignore them)
    // or remove altogether as an obsolete optimization.
    bool all16BitsStereoNoResample = true;
    bool resampling = false;
    bool volumeRamp = false;

    mEnabled.clear();
    mGroups.clear();
    for (const auto &pair : mTracks) {
        const int name = pair.first;
        const std::shared_ptr<Track> &t = pair.second;
        if (!t->enabled) continue;

        mEnabled.emplace_back(name);  // we add to mEnabled in order of name.
        mGroups[t->mainBuffer].emplace_back(name); // mGroups also in order of name.

        uint32_t n = 0;
        // FIXME can overflow (mask is only 3 bits)
        n |= NEEDS_CHANNEL_1 + t->channelCount - 1;
        if (t->doesResample()) {
            n |= NEEDS_RESAMPLE;
        }
        if (t->auxLevel != 0 && t->auxBuffer != NULL) {
            n |= NEEDS_AUX;
        }

        if (t->volumeInc[0]|t->volumeInc[1]) {
            volumeRamp = true;
        } else if (!t->doesResample() && t->volumeRL == 0) {
            n |= NEEDS_MUTE;
        }
        t->needs = n;

        if (n & NEEDS_MUTE) {
            t->hook = &Track::track__nop;
        } else {
            if (n & NEEDS_AUX) {
                all16BitsStereoNoResample = false;
            }
            if (n & NEEDS_RESAMPLE) {
                all16BitsStereoNoResample = false;
                resampling = true;
                t->hook = Track::getTrackHook(TRACKTYPE_RESAMPLE, t->mMixerChannelCount,
                        t->mMixerInFormat, t->mMixerFormat);
                ALOGV_IF((n & NEEDS_CHANNEL_COUNT__MASK) > NEEDS_CHANNEL_2,
                        "Track %d needs downmix + resample", name);
            } else {
                if ((n & NEEDS_CHANNEL_COUNT__MASK) == NEEDS_CHANNEL_1){
                    t->hook = Track::getTrackHook(
                            (t->mMixerChannelMask == AUDIO_CHANNEL_OUT_STEREO  // TODO: MONO_HACK
                                    && t->channelMask == AUDIO_CHANNEL_OUT_MONO)
                                ? TRACKTYPE_NORESAMPLEMONO : TRACKTYPE_NORESAMPLE,
                            t->mMixerChannelCount,
                            t->mMixerInFormat, t->mMixerFormat);
                    all16BitsStereoNoResample = false;
                }
                if ((n & NEEDS_CHANNEL_COUNT__MASK) >= NEEDS_CHANNEL_2){
                    t->hook = Track::getTrackHook(TRACKTYPE_NORESAMPLE, t->mMixerChannelCount,
                            t->mMixerInFormat, t->mMixerFormat);
                    ALOGV_IF((n & NEEDS_CHANNEL_COUNT__MASK) > NEEDS_CHANNEL_2,
                            "Track %d needs downmix", name);
                }
            }
        }
    }

    // select the processing hooks
    mHook = &AudioMixer::process__nop;
    if (mEnabled.size() > 0) {
        if (resampling) {
            if (mOutputTemp.get() == nullptr) {
                mOutputTemp.reset(new int32_t[MAX_NUM_CHANNELS * mFrameCount]);
            }
            if (mResampleTemp.get() == nullptr) {
                mResampleTemp.reset(new int32_t[MAX_NUM_CHANNELS * mFrameCount]);
            }
            mHook = &AudioMixer::process__genericResampling;
        } else {
            // we keep temp arrays around.
            mHook = &AudioMixer::process__genericNoResampling;
            if (all16BitsStereoNoResample && !volumeRamp) {
                if (mEnabled.size() == 1) {
                    const std::shared_ptr<Track> &t = mTracks[mEnabled[0]];
                    if ((t->needs & NEEDS_MUTE) == 0) {
                        // The check prevents a muted track from acquiring a process hook.
                        //
                        // This is dangerous if the track is MONO as that requires
                        // special case handling due to implicit channel duplication.
                        // Stereo or Multichannel should actually be fine here.
                        mHook = getProcessHook(PROCESSTYPE_NORESAMPLEONETRACK,
                                t->mMixerChannelCount, t->mMixerInFormat, t->mMixerFormat);
                    }
                }
            }
        }
    }

    ALOGV("mixer configuration change: %zu "
        "all16BitsStereoNoResample=%d, resampling=%d, volumeRamp=%d",
        mEnabled.size(), all16BitsStereoNoResample, resampling, volumeRamp);

   process();

    // Now that the volume ramp has been done, set optimal state and
    // track hooks for subsequent mixer process
    if (mEnabled.size() > 0) {
        bool allMuted = true;

        for (const int name : mEnabled) {
            const std::shared_ptr<Track> &t = mTracks[name];
            if (!t->doesResample() && t->volumeRL == 0) {
                t->needs |= NEEDS_MUTE;
                t->hook = &Track::track__nop;
            } else {
                allMuted = false;
            }
        }
        if (allMuted) {
            mHook = &AudioMixer::process__nop;
        } else if (all16BitsStereoNoResample) {
            if (mEnabled.size() == 1) {
                //const int i = 31 - __builtin_clz(enabledTracks);
                const std::shared_ptr<Track> &t = mTracks[mEnabled[0]];
                // Muted single tracks handled by allMuted above.
                mHook = getProcessHook(PROCESSTYPE_NORESAMPLEONETRACK,
                        t->mMixerChannelCount, t->mMixerInFormat, t->mMixerFormat);
            }
        }
    }
}

void AudioMixer::Track::track__genericResample(
        int32_t* out, size_t outFrameCount, int32_t* temp, int32_t* aux)
{
    ALOGVV("track__genericResample\n");
    mResampler->setSampleRate(sampleRate);

    // ramp gain - resample to temp buffer and scale/mix in 2nd step
    if (aux != NULL) {
        // always resample with unity gain when sending to auxiliary buffer to be able
        // to apply send level after resampling
        mResampler->setVolume(UNITY_GAIN_FLOAT, UNITY_GAIN_FLOAT);
        memset(temp, 0, outFrameCount * mMixerChannelCount * sizeof(int32_t));
        mResampler->resample(temp, outFrameCount, bufferProvider);
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1]|auxInc)) {
            volumeRampStereo(out, outFrameCount, temp, aux);
        } else {
            volumeStereo(out, outFrameCount, temp, aux);
        }
    } else {
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1])) {
            mResampler->setVolume(UNITY_GAIN_FLOAT, UNITY_GAIN_FLOAT);
            memset(temp, 0, outFrameCount * MAX_NUM_CHANNELS * sizeof(int32_t));
            mResampler->resample(temp, outFrameCount, bufferProvider);
            volumeRampStereo(out, outFrameCount, temp, aux);
        }

        // constant gain
        else {
            mResampler->setVolume(mVolume[0], mVolume[1]);
            mResampler->resample(out, outFrameCount, bufferProvider);
        }
    }
}

void AudioMixer::Track::track__nop(int32_t* out __unused,
        size_t outFrameCount __unused, int32_t* temp __unused, int32_t* aux __unused)
{
}

void AudioMixer::Track::volumeRampStereo(
        int32_t* out, size_t frameCount, int32_t* temp, int32_t* aux)
{
    int32_t vl = prevVolume[0];
    int32_t vr = prevVolume[1];
    const int32_t vlInc = volumeInc[0];
    const int32_t vrInc = volumeInc[1];

    //ALOGD("[0] %p: inc=%f, v0=%f, v1=%d, final=%f, count=%d",
    //        t, vlInc/65536.0f, vl/65536.0f, volume[0],
    //       (vl + vlInc*frameCount)/65536.0f, frameCount);

    // ramp volume
    if (CC_UNLIKELY(aux != NULL)) {
        int32_t va = prevAuxLevel;
        const int32_t vaInc = auxInc;
        int32_t l;
        int32_t r;

        do {
            l = (*temp++ >> 12);
            r = (*temp++ >> 12);
            *out++ += (vl >> 16) * l;
            *out++ += (vr >> 16) * r;
            *aux++ += (va >> 17) * (l + r);
            vl += vlInc;
            vr += vrInc;
            va += vaInc;
        } while (--frameCount);
        prevAuxLevel = va;
    } else {
        do {
            *out++ += (vl >> 16) * (*temp++ >> 12);
            *out++ += (vr >> 16) * (*temp++ >> 12);
            vl += vlInc;
            vr += vrInc;
        } while (--frameCount);
    }
    prevVolume[0] = vl;
    prevVolume[1] = vr;
    adjustVolumeRamp(aux != NULL);
}

void AudioMixer::Track::volumeStereo(
        int32_t* out, size_t frameCount, int32_t* temp, int32_t* aux)
{
    const int16_t vl = volume[0];
    const int16_t vr = volume[1];

    if (CC_UNLIKELY(aux != NULL)) {
        const int16_t va = auxLevel;
        do {
            int16_t l = (int16_t)(*temp++ >> 12);
            int16_t r = (int16_t)(*temp++ >> 12);
            out[0] = mulAdd(l, vl, out[0]);
            int16_t a = (int16_t)(((int32_t)l + r) >> 1);
            out[1] = mulAdd(r, vr, out[1]);
            out += 2;
            aux[0] = mulAdd(a, va, aux[0]);
            aux++;
        } while (--frameCount);
    } else {
        do {
            int16_t l = (int16_t)(*temp++ >> 12);
            int16_t r = (int16_t)(*temp++ >> 12);
            out[0] = mulAdd(l, vl, out[0]);
            out[1] = mulAdd(r, vr, out[1]);
            out += 2;
        } while (--frameCount);
    }
}

void AudioMixer::Track::track__16BitsStereo(
        int32_t* out, size_t frameCount, int32_t* temp __unused, int32_t* aux)
{
    ALOGVV("track__16BitsStereo\n");
    const int16_t *in = static_cast<const int16_t *>(mIn);

    if (CC_UNLIKELY(aux != NULL)) {
        int32_t l;
        int32_t r;
        // ramp gain
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1]|auxInc)) {
            int32_t vl = prevVolume[0];
            int32_t vr = prevVolume[1];
            int32_t va = prevAuxLevel;
            const int32_t vlInc = volumeInc[0];
            const int32_t vrInc = volumeInc[1];
            const int32_t vaInc = auxInc;
            // ALOGD("[1] %p: inc=%f, v0=%f, v1=%d, final=%f, count=%d",
            //        t, vlInc/65536.0f, vl/65536.0f, volume[0],
            //        (vl + vlInc*frameCount)/65536.0f, frameCount);

            do {
                l = (int32_t)*in++;
                r = (int32_t)*in++;
                *out++ += (vl >> 16) * l;
                *out++ += (vr >> 16) * r;
                *aux++ += (va >> 17) * (l + r);
                vl += vlInc;
                vr += vrInc;
                va += vaInc;
            } while (--frameCount);

            prevVolume[0] = vl;
            prevVolume[1] = vr;
            prevAuxLevel = va;
            adjustVolumeRamp(true);
        }

        // constant gain
        else {
            const uint32_t vrl = volumeRL;
            const int16_t va = (int16_t)auxLevel;
            do {
                uint32_t rl = *reinterpret_cast<const uint32_t *>(in);
                int16_t a = (int16_t)(((int32_t)in[0] + in[1]) >> 1);
                in += 2;
                out[0] = mulAddRL(1, rl, vrl, out[0]);
                out[1] = mulAddRL(0, rl, vrl, out[1]);
                out += 2;
                aux[0] = mulAdd(a, va, aux[0]);
                aux++;
            } while (--frameCount);
        }
    } else {
        // ramp gain
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1])) {
            int32_t vl = prevVolume[0];
            int32_t vr = prevVolume[1];
            const int32_t vlInc = volumeInc[0];
            const int32_t vrInc = volumeInc[1];

            // ALOGD("[1] %p: inc=%f, v0=%f, v1=%d, final=%f, count=%d",
            //        t, vlInc/65536.0f, vl/65536.0f, volume[0],
            //        (vl + vlInc*frameCount)/65536.0f, frameCount);

            do {
                *out++ += (vl >> 16) * (int32_t) *in++;
                *out++ += (vr >> 16) * (int32_t) *in++;
                vl += vlInc;
                vr += vrInc;
            } while (--frameCount);

            prevVolume[0] = vl;
            prevVolume[1] = vr;
            adjustVolumeRamp(false);
        }

        // constant gain
        else {
            const uint32_t vrl = volumeRL;
            do {
                uint32_t rl = *reinterpret_cast<const uint32_t *>(in);
                in += 2;
                out[0] = mulAddRL(1, rl, vrl, out[0]);
                out[1] = mulAddRL(0, rl, vrl, out[1]);
                out += 2;
            } while (--frameCount);
        }
    }
    mIn = in;
}

void AudioMixer::Track::track__16BitsMono(
        int32_t* out, size_t frameCount, int32_t* temp __unused, int32_t* aux)
{
    ALOGVV("track__16BitsMono\n");
    const int16_t *in = static_cast<int16_t const *>(mIn);

    if (CC_UNLIKELY(aux != NULL)) {
        // ramp gain
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1]|auxInc)) {
            int32_t vl = prevVolume[0];
            int32_t vr = prevVolume[1];
            int32_t va = prevAuxLevel;
            const int32_t vlInc = volumeInc[0];
            const int32_t vrInc = volumeInc[1];
            const int32_t vaInc = auxInc;

            // ALOGD("[2] %p: inc=%f, v0=%f, v1=%d, final=%f, count=%d",
            //         t, vlInc/65536.0f, vl/65536.0f, volume[0],
            //         (vl + vlInc*frameCount)/65536.0f, frameCount);

            do {
                int32_t l = *in++;
                *out++ += (vl >> 16) * l;
                *out++ += (vr >> 16) * l;
                *aux++ += (va >> 16) * l;
                vl += vlInc;
                vr += vrInc;
                va += vaInc;
            } while (--frameCount);

            prevVolume[0] = vl;
            prevVolume[1] = vr;
            prevAuxLevel = va;
            adjustVolumeRamp(true);
        }
        // constant gain
        else {
            const int16_t vl = volume[0];
            const int16_t vr = volume[1];
            const int16_t va = (int16_t)auxLevel;
            do {
                int16_t l = *in++;
                out[0] = mulAdd(l, vl, out[0]);
                out[1] = mulAdd(l, vr, out[1]);
                out += 2;
                aux[0] = mulAdd(l, va, aux[0]);
                aux++;
            } while (--frameCount);
        }
    } else {
        // ramp gain
        if (CC_UNLIKELY(volumeInc[0]|volumeInc[1])) {
            int32_t vl = prevVolume[0];
            int32_t vr = prevVolume[1];
            const int32_t vlInc = volumeInc[0];
            const int32_t vrInc = volumeInc[1];

            // ALOGD("[2] %p: inc=%f, v0=%f, v1=%d, final=%f, count=%d",
            //         t, vlInc/65536.0f, vl/65536.0f, volume[0],
            //         (vl + vlInc*frameCount)/65536.0f, frameCount);

            do {
                int32_t l = *in++;
                *out++ += (vl >> 16) * l;
                *out++ += (vr >> 16) * l;
                vl += vlInc;
                vr += vrInc;
            } while (--frameCount);

            prevVolume[0] = vl;
            prevVolume[1] = vr;
            adjustVolumeRamp(false);
        }
        // constant gain
        else {
            const int16_t vl = volume[0];
            const int16_t vr = volume[1];
            do {
                int16_t l = *in++;
                out[0] = mulAdd(l, vl, out[0]);
                out[1] = mulAdd(l, vr, out[1]);
                out += 2;
            } while (--frameCount);
        }
    }
    mIn = in;
}

// no-op case
void AudioMixer::process__nop()
{
    ALOGVV("process__nop\n");

    for (const auto &pair : mGroups) {
        // process by group of tracks with same output buffer to
        // avoid multiple memset() on same buffer
        const auto &group = pair.second;

        const std::shared_ptr<Track> &t = mTracks[group[0]];
        memset(t->mainBuffer, 0,
                mFrameCount * audio_bytes_per_frame(
                        t->mMixerChannelCount + t->mMixerHapticChannelCount, t->mMixerFormat));

        // now consume data
        for (const int name : group) {
            const std::shared_ptr<Track> &t = mTracks[name];
            size_t outFrames = mFrameCount;
            while (outFrames) {
                t->buffer.frameCount = outFrames;
                t->bufferProvider->getNextBuffer(&t->buffer);
                if (t->buffer.raw == NULL) break;
                outFrames -= t->buffer.frameCount;
                t->bufferProvider->releaseBuffer(&t->buffer);
            }
        }
    }
}

// generic code without resampling
void AudioMixer::process__genericNoResampling()
{
    ALOGVV("process__genericNoResampling\n");
    int32_t outTemp[BLOCKSIZE * MAX_NUM_CHANNELS] __attribute__((aligned(32)));

    for (const auto &pair : mGroups) {
        // process by group of tracks with same output main buffer to
        // avoid multiple memset() on same buffer
        const auto &group = pair.second;

        // acquire buffer
        for (const int name : group) {
            const std::shared_ptr<Track> &t = mTracks[name];
            t->buffer.frameCount = mFrameCount;
            t->bufferProvider->getNextBuffer(&t->buffer);
            t->frameCount = t->buffer.frameCount;
            t->mIn = t->buffer.raw;
        }

        int32_t *out = (int *)pair.first;
        size_t numFrames = 0;
        do {
            const size_t frameCount = std::min((size_t)BLOCKSIZE, mFrameCount - numFrames);
            memset(outTemp, 0, sizeof(outTemp));
            for (const int name : group) {
                const std::shared_ptr<Track> &t = mTracks[name];
                int32_t *aux = NULL;
                if (CC_UNLIKELY(t->needs & NEEDS_AUX)) {
                    aux = t->auxBuffer + numFrames;
                }
                for (int outFrames = frameCount; outFrames > 0; ) {
                    // t->in == nullptr can happen if the track was flushed just after having
                    // been enabled for mixing.
                    if (t->mIn == nullptr) {
                        break;
                    }
                    size_t inFrames = (t->frameCount > outFrames)?outFrames:t->frameCount;
                    if (inFrames > 0) {
                        (t.get()->*t->hook)(
                                outTemp + (frameCount - outFrames) * t->mMixerChannelCount,
                                inFrames, mResampleTemp.get() /* naked ptr */, aux);
                        t->frameCount -= inFrames;
                        outFrames -= inFrames;
                        if (CC_UNLIKELY(aux != NULL)) {
                            aux += inFrames;
                        }
                    }
                    if (t->frameCount == 0 && outFrames) {
                        t->bufferProvider->releaseBuffer(&t->buffer);
                        t->buffer.frameCount = (mFrameCount - numFrames) -
                                (frameCount - outFrames);
                        t->bufferProvider->getNextBuffer(&t->buffer);
                        t->mIn = t->buffer.raw;
                        if (t->mIn == nullptr) {
                            break;
                        }
                        t->frameCount = t->buffer.frameCount;
                    }
                }
            }

            const std::shared_ptr<Track> &t1 = mTracks[group[0]];
            convertMixerFormat(out, t1->mMixerFormat, outTemp, t1->mMixerInFormat,
                    frameCount * t1->mMixerChannelCount);
            // TODO: fix ugly casting due to choice of out pointer type
            out = reinterpret_cast<int32_t*>((uint8_t*)out
                    + frameCount * t1->mMixerChannelCount
                    * audio_bytes_per_sample(t1->mMixerFormat));
            numFrames += frameCount;
        } while (numFrames < mFrameCount);

        // release each track's buffer
        for (const int name : group) {
            const std::shared_ptr<Track> &t = mTracks[name];
            t->bufferProvider->releaseBuffer(&t->buffer);
        }
    }
}

// generic code with resampling
void AudioMixer::process__genericResampling()
{
    ALOGVV("process__genericResampling\n");
    int32_t * const outTemp = mOutputTemp.get(); // naked ptr
    size_t numFrames = mFrameCount;

    for (const auto &pair : mGroups) {
        const auto &group = pair.second;
        const std::shared_ptr<Track> &t1 = mTracks[group[0]];

        // clear temp buffer
        memset(outTemp, 0, sizeof(*outTemp) * t1->mMixerChannelCount * mFrameCount);
        for (const int name : group) {
            const std::shared_ptr<Track> &t = mTracks[name];
            int32_t *aux = NULL;
            if (CC_UNLIKELY(t->needs & NEEDS_AUX)) {
                aux = t->auxBuffer;
            }

            // this is a little goofy, on the resampling case we don't
            // acquire/release the buffers because it's done by
            // the resampler.
            if (t->needs & NEEDS_RESAMPLE) {
                (t.get()->*t->hook)(outTemp, numFrames, mResampleTemp.get() /* naked ptr */, aux);
            } else {

                size_t outFrames = 0;

                while (outFrames < numFrames) {
                    t->buffer.frameCount = numFrames - outFrames;
                    t->bufferProvider->getNextBuffer(&t->buffer);
                    t->mIn = t->buffer.raw;
                    // t->mIn == nullptr can happen if the track was flushed just after having
                    // been enabled for mixing.
                    if (t->mIn == nullptr) break;

                    (t.get()->*t->hook)(
                            outTemp + outFrames * t->mMixerChannelCount, t->buffer.frameCount,
                            mResampleTemp.get() /* naked ptr */,
                            aux != nullptr ? aux + outFrames : nullptr);
                    outFrames += t->buffer.frameCount;

                    t->bufferProvider->releaseBuffer(&t->buffer);
                }
            }
        }
        convertMixerFormat(t1->mainBuffer, t1->mMixerFormat,
                outTemp, t1->mMixerInFormat, numFrames * t1->mMixerChannelCount);
    }
}

// one track, 16 bits stereo without resampling is the most common case
void AudioMixer::process__oneTrack16BitsStereoNoResampling()
{
    ALOGVV("process__oneTrack16BitsStereoNoResampling\n");
    LOG_ALWAYS_FATAL_IF(mEnabled.size() != 0,
            "%zu != 1 tracks enabled", mEnabled.size());
    const int name = mEnabled[0];
    const std::shared_ptr<Track> &t = mTracks[name];

    AudioBufferProvider::Buffer& b(t->buffer);

    int32_t* out = t->mainBuffer;
    float *fout = reinterpret_cast<float*>(out);
    size_t numFrames = mFrameCount;

    const int16_t vl = t->volume[0];
    const int16_t vr = t->volume[1];
    const uint32_t vrl = t->volumeRL;
    while (numFrames) {
        b.frameCount = numFrames;
        t->bufferProvider->getNextBuffer(&b);
        const int16_t *in = b.i16;

        // in == NULL can happen if the track was flushed just after having
        // been enabled for mixing.
        if (in == NULL || (((uintptr_t)in) & 3)) {
            if ( AUDIO_FORMAT_PCM_FLOAT == t->mMixerFormat ) {
                 memset((char*)fout, 0, numFrames
                         * t->mMixerChannelCount * audio_bytes_per_sample(t->mMixerFormat));
            } else {
                 memset((char*)out, 0, numFrames
                         * t->mMixerChannelCount * audio_bytes_per_sample(t->mMixerFormat));
            }
            ALOGE_IF((((uintptr_t)in) & 3),
                    "process__oneTrack16BitsStereoNoResampling: misaligned buffer"
                    " %p track %d, channels %d, needs %08x, volume %08x vfl %f vfr %f",
                    in, name, t->channelCount, t->needs, vrl, t->mVolume[0], t->mVolume[1]);
            return;
        }
        size_t outFrames = b.frameCount;

        switch (t->mMixerFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            do {
                uint32_t rl = *reinterpret_cast<const uint32_t *>(in);
                in += 2;
                int32_t l = mulRL(1, rl, vrl);
                int32_t r = mulRL(0, rl, vrl);
                *fout++ = float_from_q4_27(l);
                *fout++ = float_from_q4_27(r);
                // Note: In case of later int16_t sink output,
                // conversion and clamping is done by memcpy_to_i16_from_float().
            } while (--outFrames);
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            if (CC_UNLIKELY(uint32_t(vl) > UNITY_GAIN_INT || uint32_t(vr) > UNITY_GAIN_INT)) {
                // volume is boosted, so we might need to clamp even though
                // we process only one track.
                do {
                    uint32_t rl = *reinterpret_cast<const uint32_t *>(in);
                    in += 2;
                    int32_t l = mulRL(1, rl, vrl) >> 12;
                    int32_t r = mulRL(0, rl, vrl) >> 12;
                    // clamping...
                    l = clamp16(l);
                    r = clamp16(r);
                    *out++ = (r<<16) | (l & 0xFFFF);
                } while (--outFrames);
            } else {
                do {
                    uint32_t rl = *reinterpret_cast<const uint32_t *>(in);
                    in += 2;
                    int32_t l = mulRL(1, rl, vrl) >> 12;
                    int32_t r = mulRL(0, rl, vrl) >> 12;
                    *out++ = (r<<16) | (l & 0xFFFF);
                } while (--outFrames);
            }
            break;
        default:
            LOG_ALWAYS_FATAL("bad mixer format: %d", t->mMixerFormat);
        }
        numFrames -= b.frameCount;
        t->bufferProvider->releaseBuffer(&b);
    }
}

/*static*/ pthread_once_t AudioMixer::sOnceControl = PTHREAD_ONCE_INIT;

/*static*/ void AudioMixer::sInitRoutine()
{
    DownmixerBufferProvider::init(); // for the downmixer
}

/* TODO: consider whether this level of optimization is necessary.
 * Perhaps just stick with a single for loop.
 */

// Needs to derive a compile time constant (constexpr).  Could be targeted to go
// to a MONOVOL mixtype based on MAX_NUM_VOLUMES, but that's an unnecessary complication.
#define MIXTYPE_MONOVOL(mixtype) ((mixtype) == MIXTYPE_MULTI ? MIXTYPE_MULTI_MONOVOL : \
        (mixtype) == MIXTYPE_MULTI_SAVEONLY ? MIXTYPE_MULTI_SAVEONLY_MONOVOL : (mixtype))

/* MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27) or float
 */
template <int MIXTYPE,
        typename TO, typename TI, typename TV, typename TA, typename TAV>
static void volumeRampMulti(uint32_t channels, TO* out, size_t frameCount,
        const TI* in, TA* aux, TV *vol, const TV *volinc, TAV *vola, TAV volainc)
{
    switch (channels) {
    case 1:
        volumeRampMulti<MIXTYPE, 1>(out, frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 2:
        volumeRampMulti<MIXTYPE, 2>(out, frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 3:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 3>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 4:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 4>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 5:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 5>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 6:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 6>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 7:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 7>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    case 8:
        volumeRampMulti<MIXTYPE_MONOVOL(MIXTYPE), 8>(out,
                frameCount, in, aux, vol, volinc, vola, volainc);
        break;
    }
}

/* MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27) or float
 */
template <int MIXTYPE,
        typename TO, typename TI, typename TV, typename TA, typename TAV>
static void volumeMulti(uint32_t channels, TO* out, size_t frameCount,
        const TI* in, TA* aux, const TV *vol, TAV vola)
{
    switch (channels) {
    case 1:
        volumeMulti<MIXTYPE, 1>(out, frameCount, in, aux, vol, vola);
        break;
    case 2:
        volumeMulti<MIXTYPE, 2>(out, frameCount, in, aux, vol, vola);
        break;
    case 3:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 3>(out, frameCount, in, aux, vol, vola);
        break;
    case 4:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 4>(out, frameCount, in, aux, vol, vola);
        break;
    case 5:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 5>(out, frameCount, in, aux, vol, vola);
        break;
    case 6:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 6>(out, frameCount, in, aux, vol, vola);
        break;
    case 7:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 7>(out, frameCount, in, aux, vol, vola);
        break;
    case 8:
        volumeMulti<MIXTYPE_MONOVOL(MIXTYPE), 8>(out, frameCount, in, aux, vol, vola);
        break;
    }
}

/* MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * USEFLOATVOL (set to true if float volume is used)
 * ADJUSTVOL   (set to true if volume ramp parameters needs adjustment afterwards)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27) or float
 */
template <int MIXTYPE, bool USEFLOATVOL, bool ADJUSTVOL,
    typename TO, typename TI, typename TA>
void AudioMixer::Track::volumeMix(TO *out, size_t outFrames,
        const TI *in, TA *aux, bool ramp)
{
    if (USEFLOATVOL) {
        if (ramp) {
            volumeRampMulti<MIXTYPE>(mMixerChannelCount, out, outFrames, in, aux,
                    mPrevVolume, mVolumeInc,
#ifdef FLOAT_AUX
                    &mPrevAuxLevel, mAuxInc
#else
                    &prevAuxLevel, auxInc
#endif
                );
            if (ADJUSTVOL) {
                adjustVolumeRamp(aux != NULL, true);
            }
        } else {
            volumeMulti<MIXTYPE>(mMixerChannelCount, out, outFrames, in, aux,
                    mVolume,
#ifdef FLOAT_AUX
                    mAuxLevel
#else
                    auxLevel
#endif
            );
        }
    } else {
        if (ramp) {
            volumeRampMulti<MIXTYPE>(mMixerChannelCount, out, outFrames, in, aux,
                    prevVolume, volumeInc, &prevAuxLevel, auxInc);
            if (ADJUSTVOL) {
                adjustVolumeRamp(aux != NULL);
            }
        } else {
            volumeMulti<MIXTYPE>(mMixerChannelCount, out, outFrames, in, aux,
                    volume, auxLevel);
        }
    }
}

/* This process hook is called when there is a single track without
 * aux buffer, volume ramp, or resampling.
 * TODO: Update the hook selection: this can properly handle aux and ramp.
 *
 * MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27)
 */
template <int MIXTYPE, typename TO, typename TI, typename TA>
void AudioMixer::process__noResampleOneTrack()
{
    ALOGVV("process__noResampleOneTrack\n");
    LOG_ALWAYS_FATAL_IF(mEnabled.size() != 1,
            "%zu != 1 tracks enabled", mEnabled.size());
    const std::shared_ptr<Track> &t = mTracks[mEnabled[0]];
    const uint32_t channels = t->mMixerChannelCount;
    TO* out = reinterpret_cast<TO*>(t->mainBuffer);
    TA* aux = reinterpret_cast<TA*>(t->auxBuffer);
    const bool ramp = t->needsRamp();

    for (size_t numFrames = mFrameCount; numFrames > 0; ) {
        AudioBufferProvider::Buffer& b(t->buffer);
        // get input buffer
        b.frameCount = numFrames;
        t->bufferProvider->getNextBuffer(&b);
        const TI *in = reinterpret_cast<TI*>(b.raw);

        // in == NULL can happen if the track was flushed just after having
        // been enabled for mixing.
        if (in == NULL || (((uintptr_t)in) & 3)) {
            memset(out, 0, numFrames
                    * channels * audio_bytes_per_sample(t->mMixerFormat));
            ALOGE_IF((((uintptr_t)in) & 3), "process__noResampleOneTrack: bus error: "
                    "buffer %p track %p, channels %d, needs %#x",
                    in, &t, t->channelCount, t->needs);
            return;
        }

        const size_t outFrames = b.frameCount;
        t->volumeMix<MIXTYPE, is_same<TI, float>::value /* USEFLOATVOL */, false /* ADJUSTVOL */> (
                out, outFrames, in, aux, ramp);

        out += outFrames * channels;
        if (aux != NULL) {
            aux += outFrames;
        }
        numFrames -= b.frameCount;

        // release buffer
        t->bufferProvider->releaseBuffer(&b);
    }
    if (ramp) {
        t->adjustVolumeRamp(aux != NULL, is_same<TI, float>::value);
    }
}

void AudioMixer::processHapticData()
{
    // Need to keep consistent with VibrationEffect.scale(int, float, int)
    for (const auto &pair : mGroups) {
        // process by group of tracks with same output main buffer.
        const auto &group = pair.second;
        for (const int name : group) {
            const std::shared_ptr<Track> &t = mTracks[name];
            if (t->mHapticPlaybackEnabled) {
                size_t sampleCount = mFrameCount * t->mMixerHapticChannelCount;
                float gamma = t->getHapticScaleGamma();
                float maxAmplitudeRatio = t->getHapticMaxAmplitudeRatio();
                uint8_t* buffer = (uint8_t*)pair.first + mFrameCount * audio_bytes_per_frame(
                        t->mMixerChannelCount, t->mMixerFormat);
                switch (t->mMixerFormat) {
                // Mixer format should be AUDIO_FORMAT_PCM_FLOAT.
                case AUDIO_FORMAT_PCM_FLOAT: {
                    float* fout = (float*) buffer;
                    for (size_t i = 0; i < sampleCount; i++) {
                        float mul = fout[i] >= 0 ? 1.0 : -1.0;
                        fout[i] = powf(fabsf(fout[i] / HAPTIC_MAX_AMPLITUDE_FLOAT), gamma)
                                * maxAmplitudeRatio * HAPTIC_MAX_AMPLITUDE_FLOAT * mul;
                    }
                } break;
                default:
                    LOG_ALWAYS_FATAL("bad mMixerFormat: %#x", t->mMixerFormat);
                    break;
                }
                break;
            }
        }
    }
}

/* This track hook is called to do resampling then mixing,
 * pulling from the track's upstream AudioBufferProvider.
 *
 * MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27) or float
 */
template <int MIXTYPE, typename TO, typename TI, typename TA>
void AudioMixer::Track::track__Resample(TO* out, size_t outFrameCount, TO* temp, TA* aux)
{
    ALOGVV("track__Resample\n");
    mResampler->setSampleRate(sampleRate);
    const bool ramp = needsRamp();
    if (ramp || aux != NULL) {
        // if ramp:        resample with unity gain to temp buffer and scale/mix in 2nd step.
        // if aux != NULL: resample with unity gain to temp buffer then apply send level.

        mResampler->setVolume(UNITY_GAIN_FLOAT, UNITY_GAIN_FLOAT);
        memset(temp, 0, outFrameCount * mMixerChannelCount * sizeof(TO));
        mResampler->resample((int32_t*)temp, outFrameCount, bufferProvider);

        volumeMix<MIXTYPE, is_same<TI, float>::value /* USEFLOATVOL */, true /* ADJUSTVOL */>(
                out, outFrameCount, temp, aux, ramp);

    } else { // constant volume gain
        mResampler->setVolume(mVolume[0], mVolume[1]);
        mResampler->resample((int32_t*)out, outFrameCount, bufferProvider);
    }
}

/* This track hook is called to mix a track, when no resampling is required.
 * The input buffer should be present in in.
 *
 * MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
 * TO: int32_t (Q4.27) or float
 * TI: int32_t (Q4.27) or int16_t (Q0.15) or float
 * TA: int32_t (Q4.27) or float
 */
template <int MIXTYPE, typename TO, typename TI, typename TA>
void AudioMixer::Track::track__NoResample(TO* out, size_t frameCount, TO* temp __unused, TA* aux)
{
    ALOGVV("track__NoResample\n");
    const TI *in = static_cast<const TI *>(mIn);

    volumeMix<MIXTYPE, is_same<TI, float>::value /* USEFLOATVOL */, true /* ADJUSTVOL */>(
            out, frameCount, in, aux, needsRamp());

    // MIXTYPE_MONOEXPAND reads a single input channel and expands to NCHAN output channels.
    // MIXTYPE_MULTI reads NCHAN input channels and places to NCHAN output channels.
    in += (MIXTYPE == MIXTYPE_MONOEXPAND) ? frameCount : frameCount * mMixerChannelCount;
    mIn = in;
}

/* The Mixer engine generates either int32_t (Q4_27) or float data.
 * We use this function to convert the engine buffers
 * to the desired mixer output format, either int16_t (Q.15) or float.
 */
/* static */
void AudioMixer::convertMixerFormat(void *out, audio_format_t mixerOutFormat,
        void *in, audio_format_t mixerInFormat, size_t sampleCount)
{
    switch (mixerInFormat) {
    case AUDIO_FORMAT_PCM_FLOAT:
        switch (mixerOutFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            memcpy(out, in, sampleCount * sizeof(float)); // MEMCPY. TODO optimize out
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            memcpy_to_i16_from_float((int16_t*)out, (float*)in, sampleCount);
            break;
        default:
            LOG_ALWAYS_FATAL("bad mixerOutFormat: %#x", mixerOutFormat);
            break;
        }
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
        switch (mixerOutFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            memcpy_to_float_from_q4_27((float*)out, (const int32_t*)in, sampleCount);
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            memcpy_to_i16_from_q4_27((int16_t*)out, (const int32_t*)in, sampleCount);
            break;
        default:
            LOG_ALWAYS_FATAL("bad mixerOutFormat: %#x", mixerOutFormat);
            break;
        }
        break;
    default:
        LOG_ALWAYS_FATAL("bad mixerInFormat: %#x", mixerInFormat);
        break;
    }
}

/* Returns the proper track hook to use for mixing the track into the output buffer.
 */
/* static */
AudioMixer::hook_t AudioMixer::Track::getTrackHook(int trackType, uint32_t channelCount,
        audio_format_t mixerInFormat, audio_format_t mixerOutFormat __unused)
{
    if (!kUseNewMixer && channelCount == FCC_2 && mixerInFormat == AUDIO_FORMAT_PCM_16_BIT) {
        switch (trackType) {
        case TRACKTYPE_NOP:
            return &Track::track__nop;
        case TRACKTYPE_RESAMPLE:
            return &Track::track__genericResample;
        case TRACKTYPE_NORESAMPLEMONO:
            return &Track::track__16BitsMono;
        case TRACKTYPE_NORESAMPLE:
            return &Track::track__16BitsStereo;
        default:
            LOG_ALWAYS_FATAL("bad trackType: %d", trackType);
            break;
        }
    }
    LOG_ALWAYS_FATAL_IF(channelCount > MAX_NUM_CHANNELS);
    switch (trackType) {
    case TRACKTYPE_NOP:
        return &Track::track__nop;
    case TRACKTYPE_RESAMPLE:
        switch (mixerInFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            return (AudioMixer::hook_t) &Track::track__Resample<
                    MIXTYPE_MULTI, float /*TO*/, float /*TI*/, TYPE_AUX>;
        case AUDIO_FORMAT_PCM_16_BIT:
            return (AudioMixer::hook_t) &Track::track__Resample<
                    MIXTYPE_MULTI, int32_t /*TO*/, int16_t /*TI*/, TYPE_AUX>;
        default:
            LOG_ALWAYS_FATAL("bad mixerInFormat: %#x", mixerInFormat);
            break;
        }
        break;
    case TRACKTYPE_NORESAMPLEMONO:
        switch (mixerInFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            return (AudioMixer::hook_t) &Track::track__NoResample<
                            MIXTYPE_MONOEXPAND, float /*TO*/, float /*TI*/, TYPE_AUX>;
        case AUDIO_FORMAT_PCM_16_BIT:
            return (AudioMixer::hook_t) &Track::track__NoResample<
                            MIXTYPE_MONOEXPAND, int32_t /*TO*/, int16_t /*TI*/, TYPE_AUX>;
        default:
            LOG_ALWAYS_FATAL("bad mixerInFormat: %#x", mixerInFormat);
            break;
        }
        break;
    case TRACKTYPE_NORESAMPLE:
        switch (mixerInFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            return (AudioMixer::hook_t) &Track::track__NoResample<
                    MIXTYPE_MULTI, float /*TO*/, float /*TI*/, TYPE_AUX>;
        case AUDIO_FORMAT_PCM_16_BIT:
            return (AudioMixer::hook_t) &Track::track__NoResample<
                    MIXTYPE_MULTI, int32_t /*TO*/, int16_t /*TI*/, TYPE_AUX>;
        default:
            LOG_ALWAYS_FATAL("bad mixerInFormat: %#x", mixerInFormat);
            break;
        }
        break;
    default:
        LOG_ALWAYS_FATAL("bad trackType: %d", trackType);
        break;
    }
    return NULL;
}

/* Returns the proper process hook for mixing tracks. Currently works only for
 * PROCESSTYPE_NORESAMPLEONETRACK, a mix involving one track, no resampling.
 *
 * TODO: Due to the special mixing considerations of duplicating to
 * a stereo output track, the input track cannot be MONO.  This should be
 * prevented by the caller.
 */
/* static */
AudioMixer::process_hook_t AudioMixer::getProcessHook(
        int processType, uint32_t channelCount,
        audio_format_t mixerInFormat, audio_format_t mixerOutFormat)
{
    if (processType != PROCESSTYPE_NORESAMPLEONETRACK) { // Only NORESAMPLEONETRACK
        LOG_ALWAYS_FATAL("bad processType: %d", processType);
        return NULL;
    }
    if (!kUseNewMixer && channelCount == FCC_2 && mixerInFormat == AUDIO_FORMAT_PCM_16_BIT) {
        return &AudioMixer::process__oneTrack16BitsStereoNoResampling;
    }
    LOG_ALWAYS_FATAL_IF(channelCount > MAX_NUM_CHANNELS);
    switch (mixerInFormat) {
    case AUDIO_FORMAT_PCM_FLOAT:
        switch (mixerOutFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            return &AudioMixer::process__noResampleOneTrack<
                    MIXTYPE_MULTI_SAVEONLY, float /*TO*/, float /*TI*/, TYPE_AUX>;
        case AUDIO_FORMAT_PCM_16_BIT:
            return &AudioMixer::process__noResampleOneTrack<
                    MIXTYPE_MULTI_SAVEONLY, int16_t /*TO*/, float /*TI*/, TYPE_AUX>;
        default:
            LOG_ALWAYS_FATAL("bad mixerOutFormat: %#x", mixerOutFormat);
            break;
        }
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
        switch (mixerOutFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            return &AudioMixer::process__noResampleOneTrack<
                    MIXTYPE_MULTI_SAVEONLY, float /*TO*/, int16_t /*TI*/, TYPE_AUX>;
        case AUDIO_FORMAT_PCM_16_BIT:
            return &AudioMixer::process__noResampleOneTrack<
                    MIXTYPE_MULTI_SAVEONLY, int16_t /*TO*/, int16_t /*TI*/, TYPE_AUX>;
        default:
            LOG_ALWAYS_FATAL("bad mixerOutFormat: %#x", mixerOutFormat);
            break;
        }
        break;
    default:
        LOG_ALWAYS_FATAL("bad mixerInFormat: %#x", mixerInFormat);
        break;
    }
    return NULL;
}

// ----------------------------------------------------------------------------
} // namespace android
