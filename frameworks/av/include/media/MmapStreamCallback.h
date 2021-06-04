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

#ifndef ANDROID_AUDIO_MMAP_STREAM_CALLBACK_H
#define ANDROID_AUDIO_MMAP_STREAM_CALLBACK_H

#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {


class MmapStreamCallback : public virtual RefBase {
  public:

    /**
     * The mmap stream should be torn down because conditions that permitted its creation with
     * the requested parameters have changed and do not allow it to operate with the requested
     * constraints any more.
     * \param[in] handle handle for the client stream to tear down.
     */
    virtual void onTearDown(audio_port_handle_t handle) = 0;

    /**
     * The volume to be applied to the use case specified when opening the stream has changed
     * \param[in] channels a channel mask containing all channels the volume should be applied to.
     * \param[in] values the volume values to be applied to each channel. The size of the vector
     *                   should correspond to the channel count retrieved with
     *                   audio_channel_count_from_in_mask() or audio_channel_count_from_out_mask()
     */
    virtual void onVolumeChanged(audio_channel_mask_t channels, Vector<float> values) = 0;

    /**
     * The device the stream is routed to/from has changed
     * \param[in] onRoutingChanged the unique device ID of the new device.
     */
    virtual void onRoutingChanged(audio_port_handle_t deviceId) = 0;

  protected:
    MmapStreamCallback() {}
    virtual ~MmapStreamCallback() {}
};


} // namespace android

#endif // ANDROID_AUDIO_MMAP_STREAM_CALLBACK_H
