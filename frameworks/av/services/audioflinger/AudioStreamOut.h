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

#ifndef ANDROID_AUDIO_STREAM_OUT_H
#define ANDROID_AUDIO_STREAM_OUT_H

#include <stdint.h>
#include <sys/types.h>

#include <system/audio.h>

namespace android {

class AudioHwDevice;
class DeviceHalInterface;
class StreamOutHalInterface;

/**
 * Managed access to a HAL output stream.
 */
class AudioStreamOut {
public:
// AudioStreamOut is immutable, so its fields are const.
// For emphasis, we could also make all pointers to them be "const *",
// but that would clutter the code unnecessarily.
    AudioHwDevice * const audioHwDev;
    sp<StreamOutHalInterface> stream;
    const audio_output_flags_t flags;

    sp<DeviceHalInterface> hwDev() const;

    AudioStreamOut(AudioHwDevice *dev, audio_output_flags_t flags);

    virtual status_t open(
            audio_io_handle_t handle,
            audio_devices_t devices,
            struct audio_config *config,
            const char *address);

    virtual ~AudioStreamOut();

    // Get the bottom 32-bits of the 64-bit render position.
    status_t getRenderPosition(uint32_t *frames);

    virtual status_t getRenderPosition(uint64_t *frames);

    virtual status_t getPresentationPosition(uint64_t *frames, struct timespec *timestamp);

    /**
    * Write audio buffer to driver. Returns number of bytes written, or a
    * negative status_t. If at least one frame was written successfully prior to the error,
    * it is suggested that the driver return that successful (short) byte count
    * and then return an error in the subsequent call.
    *
    * If set_callback() has previously been called to enable non-blocking mode
    * the write() is not allowed to block. It must write only the number of
    * bytes that currently fit in the driver/hardware buffer and then return
    * this byte count. If this is less than the requested write size the
    * callback function must be called when more space is available in the
    * driver/hardware buffer.
    */
    virtual ssize_t write(const void *buffer, size_t bytes);

    /**
     * @return frame size from the perspective of the application and the AudioFlinger.
     */
    virtual size_t getFrameSize() const { return mHalFrameSize; }

    /**
     * @return format from the perspective of the application and the AudioFlinger.
     */
    virtual audio_format_t getFormat() const;

    /**
     * The HAL may be running at a higher sample rate if, for example, playing wrapped EAC3.
     * @return sample rate from the perspective of the application and the AudioFlinger.
     */
    virtual uint32_t getSampleRate() const;

    /**
     * The HAL is in stereo mode when playing multi-channel compressed audio over HDMI.
     * @return channel mask from the perspective of the application and the AudioFlinger.
     */
    virtual audio_channel_mask_t getChannelMask() const;


    virtual status_t flush();
    virtual status_t standby();

protected:
    uint64_t             mFramesWritten; // reset by flush
    uint64_t             mFramesWrittenAtStandby;
    uint64_t             mRenderPosition; // reset by flush or standby
    int                  mRateMultiplier;
    bool                 mHalFormatHasProportionalFrames;
    size_t               mHalFrameSize;
};

} // namespace android

#endif // ANDROID_AUDIO_STREAM_OUT_H
