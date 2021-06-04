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

#ifndef ANDROID_AUDIO_MMAP_STREAM_INTERFACE_H
#define ANDROID_AUDIO_MMAP_STREAM_INTERFACE_H

#include <system/audio.h>
#include <media/AudioClient.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {

class MmapStreamCallback;

class MmapStreamInterface : public virtual RefBase
{
  public:

    /**
     * Values for direction argument passed to openMmapStream()
     */
    typedef enum {
        DIRECTION_OUTPUT = 0,  /**< open a playback mmap stream */
        DIRECTION_INPUT,       /**< open a capture mmap stream */
    } stream_direction_t;

    /**
     * Open a playback or capture stream in MMAP mode at the audio HAL.
     *
     * \note This method is implemented by AudioFlinger
     *
     * \param[in] direction open a playback or capture stream.
     * \param[in] attr audio attributes defining the main use case for this stream
     * \param[in,out] config audio parameters (sampling rate, format ...) for the stream.
     *                       Requested parameters as input,
     *                       Actual parameters as output
     * \param[in] client a AudioClient struct describing the first client using this stream.
     * \param[in,out] deviceId audio device the stream should preferably be routed to/from
     *                       Requested as input,
     *                       Actual as output
     * \param[in,out] sessionId audio sessionId for the stream
     *                       Requested as input, may be AUDIO_SESSION_ALLOCATE
     *                       Actual as output
     * \param[in] callback the MmapStreamCallback interface used by AudioFlinger to notify
     *                     condition changes affecting the stream operation
     * \param[out] interface the MmapStreamInterface interface controlling the created stream
     * \param[out] same unique handle as the one used for the first client stream started.
     * \return OK if the stream was successfully created.
     *         NO_INIT if AudioFlinger is not properly initialized
     *         BAD_VALUE if the stream cannot be opened because of invalid arguments
     *         INVALID_OPERATION if the stream cannot be opened because of platform limitations
     */
    static status_t openMmapStream(stream_direction_t direction,
                                           const audio_attributes_t *attr,
                                           audio_config_base_t *config,
                                           const AudioClient& client,
                                           audio_port_handle_t *deviceId,
                                           audio_session_t *sessionId,
                                           const sp<MmapStreamCallback>& callback,
                                           sp<MmapStreamInterface>& interface,
                                           audio_port_handle_t *handle);

    /**
     * Retrieve information on the mmap buffer used for audio samples transfer.
     * Must be called before any other method after opening the stream or entering standby.
     *
     * \param[in] min_size_frames minimum buffer size requested. The actual buffer
     *        size returned in struct audio_mmap_buffer_info can be larger.
     * \param[out] info address at which the mmap buffer information should be returned.
     *
     * \return OK if the buffer was allocated.
     *         NO_INIT in case of initialization error
     *         BAD_VALUE if the requested buffer size is too large
     *         INVALID_OPERATION if called out of sequence (e.g. buffer already allocated)
     */
    virtual status_t createMmapBuffer(int32_t minSizeFrames,
                                      struct audio_mmap_buffer_info *info) = 0;

    /**
     * Read current read/write position in the mmap buffer with associated time stamp.
     *
     * \param[out] position address at which the mmap read/write position should be returned.
     *
     * \return OK if the position is successfully returned.
     *         NO_INIT in case of initialization error
     *         NOT_ENOUGH_DATA if the position cannot be retrieved
     *         INVALID_OPERATION if called before createMmapBuffer()
     */
    virtual status_t getMmapPosition(struct audio_mmap_position *position) = 0;

    /**
     * Start a stream operating in mmap mode.
     * createMmapBuffer() must be called before calling start()
     *
     * \param[in] client a AudioClient struct describing the client starting on this stream.
     * \param[out] handle unique handle for this instance. Used with stop().
     * \return OK in case of success.
     *         NO_INIT in case of initialization error
     *         INVALID_OPERATION if called out of sequence
     */
    virtual status_t start(const AudioClient& client, audio_port_handle_t *handle) = 0;

    /**
     * Stop a stream operating in mmap mode.
     * Must be called after start()
     *
     * \param[in] handle unique handle allocated by start().
     * \return OK in case of success.
     *         NO_INIT in case of initialization error
     *         INVALID_OPERATION if called out of sequence
     */
    virtual status_t stop(audio_port_handle_t handle) = 0;

    /**
     * Put a stream operating in mmap mode into standby.
     * Must be called after createMmapBuffer(). Cannot be called if any client is active.
     * It is recommended to place a mmap stream into standby as often as possible when no client is
     * active to save power.
     *
     * \return OK in case of success.
     *         NO_INIT in case of initialization error
     *         INVALID_OPERATION if called out of sequence
     */
    virtual status_t standby() = 0;

  protected:
    // Subclasses can not be constructed directly by clients.
    MmapStreamInterface() {}

    // The destructor automatically closes the stream.
    virtual ~MmapStreamInterface() {}
};

} // namespace android

#endif // ANDROID_AUDIO_MMAP_STREAM_INTERFACE_H
