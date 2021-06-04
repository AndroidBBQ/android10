/*
**
** Copyright 2015, The Android Open Source Project
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

#include <media/audiohal/DeviceHalInterface.h>
#include <media/audiohal/StreamHalInterface.h>
#include <system/audio.h>
#include <utils/Log.h>

#include "AudioHwDevice.h"
#include "AudioStreamOut.h"

namespace android {

// ----------------------------------------------------------------------------
AudioStreamOut::AudioStreamOut(AudioHwDevice *dev, audio_output_flags_t flags)
        : audioHwDev(dev)
        , stream(NULL)
        , flags(flags)
        , mFramesWritten(0)
        , mFramesWrittenAtStandby(0)
        , mRenderPosition(0)
        , mRateMultiplier(1)
        , mHalFormatHasProportionalFrames(false)
        , mHalFrameSize(0)
{
}

AudioStreamOut::~AudioStreamOut()
{
}

sp<DeviceHalInterface> AudioStreamOut::hwDev() const
{
    return audioHwDev->hwDevice();
}

status_t AudioStreamOut::getRenderPosition(uint64_t *frames)
{
    if (stream == 0) {
        return NO_INIT;
    }

    uint32_t halPosition = 0;
    status_t status = stream->getRenderPosition(&halPosition);
    if (status != NO_ERROR) {
        return status;
    }

    // Maintain a 64-bit render position using the 32-bit result from the HAL.
    // This delta calculation relies on the arithmetic overflow behavior
    // of integers. For example (100 - 0xFFFFFFF0) = 116.
    const uint32_t truncatedPosition = (uint32_t)mRenderPosition;
    int32_t deltaHalPosition; // initialization not needed, overwitten by __builtin_sub_overflow()
    (void) __builtin_sub_overflow(halPosition, truncatedPosition, &deltaHalPosition);
    if (deltaHalPosition > 0) {
        mRenderPosition += deltaHalPosition;
    }
    // Scale from HAL sample rate to application rate.
    *frames = mRenderPosition / mRateMultiplier;

    return status;
}

// return bottom 32-bits of the render position
status_t AudioStreamOut::getRenderPosition(uint32_t *frames)
{
    uint64_t position64 = 0;
    status_t status = getRenderPosition(&position64);
    if (status == NO_ERROR) {
        *frames = (uint32_t)position64;
    }
    return status;
}

status_t AudioStreamOut::getPresentationPosition(uint64_t *frames, struct timespec *timestamp)
{
    if (stream == 0) {
        return NO_INIT;
    }

    uint64_t halPosition = 0;
    status_t status = stream->getPresentationPosition(&halPosition, timestamp);
    if (status != NO_ERROR) {
        return status;
    }

    // Adjust for standby using HAL rate frames.
    // Only apply this correction if the HAL is getting PCM frames.
    if (mHalFormatHasProportionalFrames) {
        uint64_t adjustedPosition = (halPosition <= mFramesWrittenAtStandby) ?
                0 : (halPosition - mFramesWrittenAtStandby);
        // Scale from HAL sample rate to application rate.
        *frames = adjustedPosition / mRateMultiplier;
    } else {
        // For offloaded MP3 and other compressed formats.
        *frames = halPosition;
    }

    return status;
}

status_t AudioStreamOut::open(
        audio_io_handle_t handle,
        audio_devices_t devices,
        struct audio_config *config,
        const char *address)
{
    sp<StreamOutHalInterface> outStream;

    audio_output_flags_t customFlags = (config->format == AUDIO_FORMAT_IEC61937)
                ? (audio_output_flags_t)(flags | AUDIO_OUTPUT_FLAG_IEC958_NONAUDIO)
                : flags;

    int status = hwDev()->openOutputStream(
            handle,
            devices,
            customFlags,
            config,
            address,
            &outStream);
    ALOGV("AudioStreamOut::open(), HAL returned "
            " stream %p, sampleRate %d, Format %#x, "
            "channelMask %#x, status %d",
            outStream.get(),
            config->sample_rate,
            config->format,
            config->channel_mask,
            status);

    // Some HALs may not recognize AUDIO_FORMAT_IEC61937. But if we declare
    // it as PCM then it will probably work.
    if (status != NO_ERROR && config->format == AUDIO_FORMAT_IEC61937) {
        struct audio_config customConfig = *config;
        customConfig.format = AUDIO_FORMAT_PCM_16_BIT;

        status = hwDev()->openOutputStream(
                handle,
                devices,
                customFlags,
                &customConfig,
                address,
                &outStream);
        ALOGV("AudioStreamOut::open(), treat IEC61937 as PCM, status = %d", status);
    }

    if (status == NO_ERROR) {
        stream = outStream;
        mHalFormatHasProportionalFrames = audio_has_proportional_frames(config->format);
        status = stream->getFrameSize(&mHalFrameSize);
    }

    return status;
}

audio_format_t AudioStreamOut::getFormat() const
{
    audio_format_t result;
    return stream->getFormat(&result) == OK ? result : AUDIO_FORMAT_INVALID;
}

uint32_t AudioStreamOut::getSampleRate() const
{
    uint32_t result;
    return stream->getSampleRate(&result) == OK ? result : 0;
}

audio_channel_mask_t AudioStreamOut::getChannelMask() const
{
    audio_channel_mask_t result;
    return stream->getChannelMask(&result) == OK ? result : AUDIO_CHANNEL_INVALID;
}

int AudioStreamOut::flush()
{
    mRenderPosition = 0;
    mFramesWritten = 0;
    mFramesWrittenAtStandby = 0;
    status_t result = stream->flush();
    return result != INVALID_OPERATION ? result : NO_ERROR;
}

int AudioStreamOut::standby()
{
    mRenderPosition = 0;
    mFramesWrittenAtStandby = mFramesWritten;
    return stream->standby();
}

ssize_t AudioStreamOut::write(const void *buffer, size_t numBytes)
{
    size_t bytesWritten;
    status_t result = stream->write(buffer, numBytes, &bytesWritten);
    if (result == OK && bytesWritten > 0 && mHalFrameSize > 0) {
        mFramesWritten += bytesWritten / mHalFrameSize;
    }
    return result == OK ? bytesWritten : result;
}

} // namespace android
