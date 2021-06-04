/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_AUDIOTRACK_H
#define ANDROID_AUDIOTRACK_H

#include <cutils/sched_policy.h>
#include <media/AudioSystem.h>
#include <media/AudioTimestamp.h>
#include <media/IAudioTrack.h>
#include <media/AudioResamplerPublic.h>
#include <media/MediaAnalyticsItem.h>
#include <media/Modulo.h>
#include <utils/threads.h>

namespace android {

// ----------------------------------------------------------------------------

struct audio_track_cblk_t;
class AudioTrackClientProxy;
class StaticAudioTrackClientProxy;

// ----------------------------------------------------------------------------

class AudioTrack : public AudioSystem::AudioDeviceCallback
{
public:

    /* Events used by AudioTrack callback function (callback_t).
     * Keep in sync with frameworks/base/media/java/android/media/AudioTrack.java NATIVE_EVENT_*.
     */
    enum event_type {
        EVENT_MORE_DATA = 0,        // Request to write more data to buffer.
                                    // This event only occurs for TRANSFER_CALLBACK.
                                    // If this event is delivered but the callback handler
                                    // does not want to write more data, the handler must
                                    // ignore the event by setting frameCount to zero.
                                    // This might occur, for example, if the application is
                                    // waiting for source data or is at the end of stream.
                                    //
                                    // For data filling, it is preferred that the callback
                                    // does not block and instead returns a short count on
                                    // the amount of data actually delivered
                                    // (or 0, if no data is currently available).
        EVENT_UNDERRUN = 1,         // Buffer underrun occurred. This will not occur for
                                    // static tracks.
        EVENT_LOOP_END = 2,         // Sample loop end was reached; playback restarted from
                                    // loop start if loop count was not 0 for a static track.
        EVENT_MARKER = 3,           // Playback head is at the specified marker position
                                    // (See setMarkerPosition()).
        EVENT_NEW_POS = 4,          // Playback head is at a new position
                                    // (See setPositionUpdatePeriod()).
        EVENT_BUFFER_END = 5,       // Playback has completed for a static track.
        EVENT_NEW_IAUDIOTRACK = 6,  // IAudioTrack was re-created, either due to re-routing and
                                    // voluntary invalidation by mediaserver, or mediaserver crash.
        EVENT_STREAM_END = 7,       // Sent after all the buffers queued in AF and HW are played
                                    // back (after stop is called) for an offloaded track.
#if 0   // FIXME not yet implemented
        EVENT_NEW_TIMESTAMP = 8,    // Delivered periodically and when there's a significant change
                                    // in the mapping from frame position to presentation time.
                                    // See AudioTimestamp for the information included with event.
#endif
        EVENT_CAN_WRITE_MORE_DATA = 9,// Notification that more data can be given by write()
                                    // This event only occurs for TRANSFER_SYNC_NOTIF_CALLBACK.
    };

    /* Client should declare a Buffer and pass the address to obtainBuffer()
     * and releaseBuffer().  See also callback_t for EVENT_MORE_DATA.
     */

    class Buffer
    {
    public:
        // FIXME use m prefix
        size_t      frameCount;   // number of sample frames corresponding to size;
                                  // on input to obtainBuffer() it is the number of frames desired,
                                  // on output from obtainBuffer() it is the number of available
                                  //    [empty slots for] frames to be filled
                                  // on input to releaseBuffer() it is currently ignored

        size_t      size;         // input/output in bytes == frameCount * frameSize
                                  // on input to obtainBuffer() it is ignored
                                  // on output from obtainBuffer() it is the number of available
                                  //    [empty slots for] bytes to be filled,
                                  //    which is frameCount * frameSize
                                  // on input to releaseBuffer() it is the number of bytes to
                                  //    release
                                  // FIXME This is redundant with respect to frameCount.  Consider
                                  //    removing size and making frameCount the primary field.

        union {
            void*       raw;
            int16_t*    i16;      // signed 16-bit
            int8_t*     i8;       // unsigned 8-bit, offset by 0x80
        };                        // input to obtainBuffer(): unused, output: pointer to buffer

        uint32_t    sequence;       // IAudioTrack instance sequence number, as of obtainBuffer().
                                    // It is set by obtainBuffer() and confirmed by releaseBuffer().
                                    // Not "user-serviceable".
                                    // TODO Consider sp<IMemory> instead, or in addition to this.
    };

    /* As a convenience, if a callback is supplied, a handler thread
     * is automatically created with the appropriate priority. This thread
     * invokes the callback when a new buffer becomes available or various conditions occur.
     * Parameters:
     *
     * event:   type of event notified (see enum AudioTrack::event_type).
     * user:    Pointer to context for use by the callback receiver.
     * info:    Pointer to optional parameter according to event type:
     *          - EVENT_MORE_DATA: pointer to AudioTrack::Buffer struct. The callback must not write
     *            more bytes than indicated by 'size' field and update 'size' if fewer bytes are
     *            written.
     *          - EVENT_UNDERRUN: unused.
     *          - EVENT_LOOP_END: pointer to an int indicating the number of loops remaining.
     *          - EVENT_MARKER: pointer to const uint32_t containing the marker position in frames.
     *          - EVENT_NEW_POS: pointer to const uint32_t containing the new position in frames.
     *          - EVENT_BUFFER_END: unused.
     *          - EVENT_NEW_IAUDIOTRACK: unused.
     *          - EVENT_STREAM_END: unused.
     *          - EVENT_NEW_TIMESTAMP: pointer to const AudioTimestamp.
     */

    typedef void (*callback_t)(int event, void* user, void *info);

    /* Returns the minimum frame count required for the successful creation of
     * an AudioTrack object.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - NO_INIT: audio server or audio hardware not initialized
     *  - BAD_VALUE: unsupported configuration
     * frameCount is guaranteed to be non-zero if status is NO_ERROR,
     * and is undefined otherwise.
     * FIXME This API assumes a route, and so should be deprecated.
     */

    static status_t getMinFrameCount(size_t* frameCount,
                                     audio_stream_type_t streamType,
                                     uint32_t sampleRate);

    /* Check if direct playback is possible for the given audio configuration and attributes.
     * Return true if output is possible for the given parameters. Otherwise returns false.
     */
    static bool isDirectOutputSupported(const audio_config_base_t& config,
                                        const audio_attributes_t& attributes);

    /* How data is transferred to AudioTrack
     */
    enum transfer_type {
        TRANSFER_DEFAULT,   // not specified explicitly; determine from the other parameters
        TRANSFER_CALLBACK,  // callback EVENT_MORE_DATA
        TRANSFER_OBTAIN,    // call obtainBuffer() and releaseBuffer()
        TRANSFER_SYNC,      // synchronous write()
        TRANSFER_SHARED,    // shared memory
        TRANSFER_SYNC_NOTIF_CALLBACK, // synchronous write(), notif EVENT_CAN_WRITE_MORE_DATA
    };

    /* Constructs an uninitialized AudioTrack. No connection with
     * AudioFlinger takes place.  Use set() after this.
     */
                        AudioTrack();

    /* Creates an AudioTrack object and registers it with AudioFlinger.
     * Once created, the track needs to be started before it can be used.
     * Unspecified values are set to appropriate default values.
     *
     * Parameters:
     *
     * streamType:         Select the type of audio stream this track is attached to
     *                     (e.g. AUDIO_STREAM_MUSIC).
     * sampleRate:         Data source sampling rate in Hz.  Zero means to use the sink sample rate.
     *                     A non-zero value must be specified if AUDIO_OUTPUT_FLAG_DIRECT is set.
     *                     0 will not work with current policy implementation for direct output
     *                     selection where an exact match is needed for sampling rate.
     * format:             Audio format. For mixed tracks, any PCM format supported by server is OK.
     *                     For direct and offloaded tracks, the possible format(s) depends on the
     *                     output sink.
     * channelMask:        Channel mask, such that audio_is_output_channel(channelMask) is true.
     * frameCount:         Minimum size of track PCM buffer in frames. This defines the
     *                     application's contribution to the
     *                     latency of the track. The actual size selected by the AudioTrack could be
     *                     larger if the requested size is not compatible with current audio HAL
     *                     configuration.  Zero means to use a default value.
     * flags:              See comments on audio_output_flags_t in <system/audio.h>.
     * cbf:                Callback function. If not null, this function is called periodically
     *                     to provide new data in TRANSFER_CALLBACK mode
     *                     and inform of marker, position updates, etc.
     * user:               Context for use by the callback receiver.
     * notificationFrames: The callback function is called each time notificationFrames PCM
     *                     frames have been consumed from track input buffer by server.
     *                     Zero means to use a default value, which is typically:
     *                      - fast tracks: HAL buffer size, even if track frameCount is larger
     *                      - normal tracks: 1/2 of track frameCount
     *                     A positive value means that many frames at initial source sample rate.
     *                     A negative value for this parameter specifies the negative of the
     *                     requested number of notifications (sub-buffers) in the entire buffer.
     *                     For fast tracks, the FastMixer will process one sub-buffer at a time.
     *                     The size of each sub-buffer is determined by the HAL.
     *                     To get "double buffering", for example, one should pass -2.
     *                     The minimum number of sub-buffers is 1 (expressed as -1),
     *                     and the maximum number of sub-buffers is 8 (expressed as -8).
     *                     Negative is only permitted for fast tracks, and if frameCount is zero.
     *                     TODO It is ugly to overload a parameter in this way depending on
     *                     whether it is positive, negative, or zero.  Consider splitting apart.
     * sessionId:          Specific session ID, or zero to use default.
     * transferType:       How data is transferred to AudioTrack.
     * offloadInfo:        If not NULL, provides offload parameters for
     *                     AudioSystem::getOutputForAttr().
     * uid:                User ID of the app which initially requested this AudioTrack
     *                     for power management tracking, or -1 for current user ID.
     * pid:                Process ID of the app which initially requested this AudioTrack
     *                     for power management tracking, or -1 for current process ID.
     * pAttributes:        If not NULL, supersedes streamType for use case selection.
     * doNotReconnect:     If set to true, AudioTrack won't automatically recreate the IAudioTrack
                           binder to AudioFlinger.
                           It will return an error instead.  The application will recreate
                           the track based on offloading or different channel configuration, etc.
     * maxRequiredSpeed:   For PCM tracks, this creates an appropriate buffer size that will allow
     *                     maxRequiredSpeed playback. Values less than 1.0f and greater than
     *                     AUDIO_TIMESTRETCH_SPEED_MAX will be clamped.  For non-PCM tracks
     *                     and direct or offloaded tracks, this parameter is ignored.
     * selectedDeviceId:   Selected device id of the app which initially requested the AudioTrack
     *                     to open with a specific device.
     * threadCanCallJava:  Not present in parameter list, and so is fixed at false.
     */

                        AudioTrack( audio_stream_type_t streamType,
                                    uint32_t sampleRate,
                                    audio_format_t format,
                                    audio_channel_mask_t channelMask,
                                    size_t frameCount    = 0,
                                    audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                                    callback_t cbf       = NULL,
                                    void* user           = NULL,
                                    int32_t notificationFrames = 0,
                                    audio_session_t sessionId  = AUDIO_SESSION_ALLOCATE,
                                    transfer_type transferType = TRANSFER_DEFAULT,
                                    const audio_offload_info_t *offloadInfo = NULL,
                                    uid_t uid = AUDIO_UID_INVALID,
                                    pid_t pid = -1,
                                    const audio_attributes_t* pAttributes = NULL,
                                    bool doNotReconnect = false,
                                    float maxRequiredSpeed = 1.0f,
                                    audio_port_handle_t selectedDeviceId = AUDIO_PORT_HANDLE_NONE);

    /* Creates an audio track and registers it with AudioFlinger.
     * With this constructor, the track is configured for static buffer mode.
     * Data to be rendered is passed in a shared memory buffer
     * identified by the argument sharedBuffer, which should be non-0.
     * If sharedBuffer is zero, this constructor is equivalent to the previous constructor
     * but without the ability to specify a non-zero value for the frameCount parameter.
     * The memory should be initialized to the desired data before calling start().
     * The write() method is not supported in this case.
     * It is recommended to pass a callback function to be notified of playback end by an
     * EVENT_UNDERRUN event.
     */

                        AudioTrack( audio_stream_type_t streamType,
                                    uint32_t sampleRate,
                                    audio_format_t format,
                                    audio_channel_mask_t channelMask,
                                    const sp<IMemory>& sharedBuffer,
                                    audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                                    callback_t cbf      = NULL,
                                    void* user          = NULL,
                                    int32_t notificationFrames = 0,
                                    audio_session_t sessionId   = AUDIO_SESSION_ALLOCATE,
                                    transfer_type transferType = TRANSFER_DEFAULT,
                                    const audio_offload_info_t *offloadInfo = NULL,
                                    uid_t uid = AUDIO_UID_INVALID,
                                    pid_t pid = -1,
                                    const audio_attributes_t* pAttributes = NULL,
                                    bool doNotReconnect = false,
                                    float maxRequiredSpeed = 1.0f);

    /* Terminates the AudioTrack and unregisters it from AudioFlinger.
     * Also destroys all resources associated with the AudioTrack.
     */
protected:
                        virtual ~AudioTrack();
public:

    /* Initialize an AudioTrack that was created using the AudioTrack() constructor.
     * Don't call set() more than once, or after the AudioTrack() constructors that take parameters.
     * set() is not multi-thread safe.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful initialization
     *  - INVALID_OPERATION: AudioTrack is already initialized
     *  - BAD_VALUE: invalid parameter (channelMask, format, sampleRate...)
     *  - NO_INIT: audio server or audio hardware not initialized
     * If status is not equal to NO_ERROR, don't call any other APIs on this AudioTrack.
     * If sharedBuffer is non-0, the frameCount parameter is ignored and
     * replaced by the shared buffer's total allocated size in frame units.
     *
     * Parameters not listed in the AudioTrack constructors above:
     *
     * threadCanCallJava:  Whether callbacks are made from an attached thread and thus can call JNI.
     *      Only set to true when AudioTrack object is used for a java android.media.AudioTrack
     *      in its JNI code.
     *
     * Internal state post condition:
     *      (mStreamType == AUDIO_STREAM_DEFAULT) implies this AudioTrack has valid attributes
     */
            status_t    set(audio_stream_type_t streamType,
                            uint32_t sampleRate,
                            audio_format_t format,
                            audio_channel_mask_t channelMask,
                            size_t frameCount   = 0,
                            audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                            callback_t cbf      = NULL,
                            void* user          = NULL,
                            int32_t notificationFrames = 0,
                            const sp<IMemory>& sharedBuffer = 0,
                            bool threadCanCallJava = false,
                            audio_session_t sessionId  = AUDIO_SESSION_ALLOCATE,
                            transfer_type transferType = TRANSFER_DEFAULT,
                            const audio_offload_info_t *offloadInfo = NULL,
                            uid_t uid = AUDIO_UID_INVALID,
                            pid_t pid = -1,
                            const audio_attributes_t* pAttributes = NULL,
                            bool doNotReconnect = false,
                            float maxRequiredSpeed = 1.0f,
                            audio_port_handle_t selectedDeviceId = AUDIO_PORT_HANDLE_NONE);

    /* Result of constructing the AudioTrack. This must be checked for successful initialization
     * before using any AudioTrack API (except for set()), because using
     * an uninitialized AudioTrack produces undefined results.
     * See set() method above for possible return codes.
     */
            status_t    initCheck() const   { return mStatus; }

    /* Returns this track's estimated latency in milliseconds.
     * This includes the latency due to AudioTrack buffer size, AudioMixer (if any)
     * and audio hardware driver.
     */
            uint32_t    latency();

    /* Returns the number of application-level buffer underruns
     * since the AudioTrack was created.
     */
            uint32_t    getUnderrunCount() const;

    /* getters, see constructors and set() */

            audio_stream_type_t streamType() const;
            audio_format_t format() const   { return mFormat; }

    /* Return frame size in bytes, which for linear PCM is
     * channelCount * (bit depth per channel / 8).
     * channelCount is determined from channelMask, and bit depth comes from format.
     * For non-linear formats, the frame size is typically 1 byte.
     */
            size_t      frameSize() const   { return mFrameSize; }

            uint32_t    channelCount() const { return mChannelCount; }
            size_t      frameCount() const  { return mFrameCount; }

    /*
     * Return the period of the notification callback in frames.
     * This value is set when the AudioTrack is constructed.
     * It can be modified if the AudioTrack is rerouted.
     */
            uint32_t    getNotificationPeriodInFrames() const { return mNotificationFramesAct; }

    /* Return effective size of audio buffer that an application writes to
     * or a negative error if the track is uninitialized.
     */
            ssize_t     getBufferSizeInFrames();

    /* Returns the buffer duration in microseconds at current playback rate.
     */
            status_t    getBufferDurationInUs(int64_t *duration);

    /* Set the effective size of audio buffer that an application writes to.
     * This is used to determine the amount of available room in the buffer,
     * which determines when a write will block.
     * This allows an application to raise and lower the audio latency.
     * The requested size may be adjusted so that it is
     * greater or equal to the absolute minimum and
     * less than or equal to the getBufferCapacityInFrames().
     * It may also be adjusted slightly for internal reasons.
     *
     * Return the final size or a negative error if the track is unitialized
     * or does not support variable sizes.
     */
            ssize_t     setBufferSizeInFrames(size_t size);

    /* Return the static buffer specified in constructor or set(), or 0 for streaming mode */
            sp<IMemory> sharedBuffer() const { return mSharedBuffer; }

    /*
     * return metrics information for the current track.
     */
            status_t getMetrics(MediaAnalyticsItem * &item);

    /* After it's created the track is not active. Call start() to
     * make it active. If set, the callback will start being called.
     * If the track was previously paused, volume is ramped up over the first mix buffer.
     */
            status_t        start();

    /* Stop a track.
     * In static buffer mode, the track is stopped immediately.
     * In streaming mode, the callback will cease being called.  Note that obtainBuffer() still
     * works and will fill up buffers until the pool is exhausted, and then will return WOULD_BLOCK.
     * In streaming mode the stop does not occur immediately: any data remaining in the buffer
     * is first drained, mixed, and output, and only then is the track marked as stopped.
     */
            void        stop();
            bool        stopped() const;

    /* Flush a stopped or paused track. All previously buffered data is discarded immediately.
     * This has the effect of draining the buffers without mixing or output.
     * Flush is intended for streaming mode, for example before switching to non-contiguous content.
     * This function is a no-op if the track is not stopped or paused, or uses a static buffer.
     */
            void        flush();

    /* Pause a track. After pause, the callback will cease being called and
     * obtainBuffer returns WOULD_BLOCK. Note that obtainBuffer() still works
     * and will fill up buffers until the pool is exhausted.
     * Volume is ramped down over the next mix buffer following the pause request,
     * and then the track is marked as paused.  It can be resumed with ramp up by start().
     */
            void        pause();

    /* Set volume for this track, mostly used for games' sound effects
     * left and right volumes. Levels must be >= 0.0 and <= 1.0.
     * This is the older API.  New applications should use setVolume(float) when possible.
     */
            status_t    setVolume(float left, float right);

    /* Set volume for all channels.  This is the preferred API for new applications,
     * especially for multi-channel content.
     */
            status_t    setVolume(float volume);

    /* Set the send level for this track. An auxiliary effect should be attached
     * to the track with attachEffect(). Level must be >= 0.0 and <= 1.0.
     */
            status_t    setAuxEffectSendLevel(float level);
            void        getAuxEffectSendLevel(float* level) const;

    /* Set source sample rate for this track in Hz, mostly used for games' sound effects.
     * Zero is not permitted.
     */
            status_t    setSampleRate(uint32_t sampleRate);

    /* Return current source sample rate in Hz.
     * If specified as zero in constructor or set(), this will be the sink sample rate.
     */
            uint32_t    getSampleRate() const;

    /* Return the original source sample rate in Hz. This corresponds to the sample rate
     * if playback rate had normal speed and pitch.
     */
            uint32_t    getOriginalSampleRate() const;

    /* Set source playback rate for timestretch
     * 1.0 is normal speed: < 1.0 is slower, > 1.0 is faster
     * 1.0 is normal pitch: < 1.0 is lower pitch, > 1.0 is higher pitch
     *
     * AUDIO_TIMESTRETCH_SPEED_MIN <= speed <= AUDIO_TIMESTRETCH_SPEED_MAX
     * AUDIO_TIMESTRETCH_PITCH_MIN <= pitch <= AUDIO_TIMESTRETCH_PITCH_MAX
     *
     * Speed increases the playback rate of media, but does not alter pitch.
     * Pitch increases the "tonal frequency" of media, but does not affect the playback rate.
     */
            status_t    setPlaybackRate(const AudioPlaybackRate &playbackRate);

    /* Return current playback rate */
            const AudioPlaybackRate& getPlaybackRate() const;

    /* Enables looping and sets the start and end points of looping.
     * Only supported for static buffer mode.
     *
     * Parameters:
     *
     * loopStart:   loop start in frames relative to start of buffer.
     * loopEnd:     loop end in frames relative to start of buffer.
     * loopCount:   number of loops to execute. Calling setLoop() with loopCount == 0 cancels any
     *              pending or active loop. loopCount == -1 means infinite looping.
     *
     * For proper operation the following condition must be respected:
     *      loopCount != 0 implies 0 <= loopStart < loopEnd <= frameCount().
     *
     * If the loop period (loopEnd - loopStart) is too small for the implementation to support,
     * setLoop() will return BAD_VALUE.  loopCount must be >= -1.
     *
     */
            status_t    setLoop(uint32_t loopStart, uint32_t loopEnd, int loopCount);

    /* Sets marker position. When playback reaches the number of frames specified, a callback with
     * event type EVENT_MARKER is called. Calling setMarkerPosition with marker == 0 cancels marker
     * notification callback.  To set a marker at a position which would compute as 0,
     * a workaround is to set the marker at a nearby position such as ~0 or 1.
     * If the AudioTrack has been opened with no callback function associated, the operation will
     * fail.
     *
     * Parameters:
     *
     * marker:   marker position expressed in wrapping (overflow) frame units,
     *           like the return value of getPosition().
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioTrack has no callback installed.
     */
            status_t    setMarkerPosition(uint32_t marker);
            status_t    getMarkerPosition(uint32_t *marker) const;

    /* Sets position update period. Every time the number of frames specified has been played,
     * a callback with event type EVENT_NEW_POS is called.
     * Calling setPositionUpdatePeriod with updatePeriod == 0 cancels new position notification
     * callback.
     * If the AudioTrack has been opened with no callback function associated, the operation will
     * fail.
     * Extremely small values may be rounded up to a value the implementation can support.
     *
     * Parameters:
     *
     * updatePeriod:  position update notification period expressed in frames.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioTrack has no callback installed.
     */
            status_t    setPositionUpdatePeriod(uint32_t updatePeriod);
            status_t    getPositionUpdatePeriod(uint32_t *updatePeriod) const;

    /* Sets playback head position.
     * Only supported for static buffer mode.
     *
     * Parameters:
     *
     * position:  New playback head position in frames relative to start of buffer.
     *            0 <= position <= frameCount().  Note that end of buffer is permitted,
     *            but will result in an immediate underrun if started.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioTrack is not stopped or paused, or is streaming mode.
     *  - BAD_VALUE: The specified position is beyond the number of frames present in AudioTrack
     *               buffer
     */
            status_t    setPosition(uint32_t position);

    /* Return the total number of frames played since playback start.
     * The counter will wrap (overflow) periodically, e.g. every ~27 hours at 44.1 kHz.
     * It is reset to zero by flush(), reload(), and stop().
     *
     * Parameters:
     *
     *  position:  Address where to return play head position.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - BAD_VALUE:  position is NULL
     */
            status_t    getPosition(uint32_t *position);

    /* For static buffer mode only, this returns the current playback position in frames
     * relative to start of buffer.  It is analogous to the position units used by
     * setLoop() and setPosition().  After underrun, the position will be at end of buffer.
     */
            status_t    getBufferPosition(uint32_t *position);

    /* Forces AudioTrack buffer full condition. When playing a static buffer, this method avoids
     * rewriting the buffer before restarting playback after a stop.
     * This method must be called with the AudioTrack in paused or stopped state.
     * Not allowed in streaming mode.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioTrack is not stopped or paused, or is streaming mode.
     */
            status_t    reload();

    /**
     * @param transferType
     * @return text string that matches the enum name
     */
            static const char * convertTransferToText(transfer_type transferType);

    /* Returns a handle on the audio output used by this AudioTrack.
     *
     * Parameters:
     *  none.
     *
     * Returned value:
     *  handle on audio hardware output, or AUDIO_IO_HANDLE_NONE if the
     *  track needed to be re-created but that failed
     */
private:
            audio_io_handle_t    getOutput() const;
public:

    /* Selects the audio device to use for output of this AudioTrack. A value of
     * AUDIO_PORT_HANDLE_NONE indicates default (AudioPolicyManager) routing.
     *
     * Parameters:
     *  The device ID of the selected device (as returned by the AudioDevicesManager API).
     *
     * Returned value:
     *  - NO_ERROR: successful operation
     *    TODO: what else can happen here?
     */
            status_t    setOutputDevice(audio_port_handle_t deviceId);

    /* Returns the ID of the audio device selected for this AudioTrack.
     * A value of AUDIO_PORT_HANDLE_NONE indicates default (AudioPolicyManager) routing.
     *
     * Parameters:
     *  none.
     */
     audio_port_handle_t getOutputDevice();

     /* Returns the ID of the audio device actually used by the output to which this AudioTrack is
      * attached.
      * When the AudioTrack is inactive, the device ID returned can be either:
      * - AUDIO_PORT_HANDLE_NONE if the AudioTrack is not attached to any output.
      * - The device ID used before paused or stopped.
      * - The device ID selected by audio policy manager of setOutputDevice() if the AudioTrack
      * has not been started yet.
      *
      * Parameters:
      *  none.
      */
     audio_port_handle_t getRoutedDeviceId();

    /* Returns the unique session ID associated with this track.
     *
     * Parameters:
     *  none.
     *
     * Returned value:
     *  AudioTrack session ID.
     */
            audio_session_t getSessionId() const { return mSessionId; }

    /* Attach track auxiliary output to specified effect. Use effectId = 0
     * to detach track from effect.
     *
     * Parameters:
     *
     * effectId:  effectId obtained from AudioEffect::id().
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the effect is not an auxiliary effect.
     *  - BAD_VALUE: The specified effect ID is invalid
     */
            status_t    attachAuxEffect(int effectId);

    /* Public API for TRANSFER_OBTAIN mode.
     * Obtains a buffer of up to "audioBuffer->frameCount" empty slots for frames.
     * After filling these slots with data, the caller should release them with releaseBuffer().
     * If the track buffer is not full, obtainBuffer() returns as many contiguous
     * [empty slots for] frames as are available immediately.
     *
     * If nonContig is non-NULL, it is an output parameter that will be set to the number of
     * additional non-contiguous frames that are predicted to be available immediately,
     * if the client were to release the first frames and then call obtainBuffer() again.
     * This value is only a prediction, and needs to be confirmed.
     * It will be set to zero for an error return.
     *
     * If the track buffer is full and track is stopped, obtainBuffer() returns WOULD_BLOCK
     * regardless of the value of waitCount.
     * If the track buffer is full and track is not stopped, obtainBuffer() blocks with a
     * maximum timeout based on waitCount; see chart below.
     * Buffers will be returned until the pool
     * is exhausted, at which point obtainBuffer() will either block
     * or return WOULD_BLOCK depending on the value of the "waitCount"
     * parameter.
     *
     * Interpretation of waitCount:
     *  +n  limits wait time to n * WAIT_PERIOD_MS,
     *  -1  causes an (almost) infinite wait time,
     *   0  non-blocking.
     *
     * Buffer fields
     * On entry:
     *  frameCount  number of [empty slots for] frames requested
     *  size        ignored
     *  raw         ignored
     *  sequence    ignored
     * After error return:
     *  frameCount  0
     *  size        0
     *  raw         undefined
     *  sequence    undefined
     * After successful return:
     *  frameCount  actual number of [empty slots for] frames available, <= number requested
     *  size        actual number of bytes available
     *  raw         pointer to the buffer
     *  sequence    IAudioTrack instance sequence number, as of obtainBuffer()
     */
            status_t    obtainBuffer(Buffer* audioBuffer, int32_t waitCount,
                                size_t *nonContig = NULL);

private:
    /* If nonContig is non-NULL, it is an output parameter that will be set to the number of
     * additional non-contiguous frames that are predicted to be available immediately,
     * if the client were to release the first frames and then call obtainBuffer() again.
     * This value is only a prediction, and needs to be confirmed.
     * It will be set to zero for an error return.
     * FIXME We could pass an array of Buffers instead of only one Buffer to obtainBuffer(),
     * in case the requested amount of frames is in two or more non-contiguous regions.
     * FIXME requested and elapsed are both relative times.  Consider changing to absolute time.
     */
            status_t    obtainBuffer(Buffer* audioBuffer, const struct timespec *requested,
                                     struct timespec *elapsed = NULL, size_t *nonContig = NULL);
public:

    /* Public API for TRANSFER_OBTAIN mode.
     * Release a filled buffer of frames for AudioFlinger to process.
     *
     * Buffer fields:
     *  frameCount  currently ignored but recommend to set to actual number of frames filled
     *  size        actual number of bytes filled, must be multiple of frameSize
     *  raw         ignored
     */
            void        releaseBuffer(const Buffer* audioBuffer);

    /* As a convenience we provide a write() interface to the audio buffer.
     * Input parameter 'size' is in byte units.
     * This is implemented on top of obtainBuffer/releaseBuffer. For best
     * performance use callbacks. Returns actual number of bytes written >= 0,
     * or one of the following negative status codes:
     *      INVALID_OPERATION   AudioTrack is configured for static buffer or streaming mode
     *      BAD_VALUE           size is invalid
     *      WOULD_BLOCK         when obtainBuffer() returns same, or
     *                          AudioTrack was stopped during the write
     *      DEAD_OBJECT         when AudioFlinger dies or the output device changes and
     *                          the track cannot be automatically restored.
     *                          The application needs to recreate the AudioTrack
     *                          because the audio device changed or AudioFlinger died.
     *                          This typically occurs for direct or offload tracks
     *                          or if mDoNotReconnect is true.
     *      or any other error code returned by IAudioTrack::start() or restoreTrack_l().
     * Default behavior is to only return when all data has been transferred. Set 'blocking' to
     * false for the method to return immediately without waiting to try multiple times to write
     * the full content of the buffer.
     */
            ssize_t     write(const void* buffer, size_t size, bool blocking = true);

    /*
     * Dumps the state of an audio track.
     * Not a general-purpose API; intended only for use by media player service to dump its tracks.
     */
            status_t    dump(int fd, const Vector<String16>& args) const;

    /*
     * Return the total number of frames which AudioFlinger desired but were unavailable,
     * and thus which resulted in an underrun.  Reset to zero by stop().
     */
            uint32_t    getUnderrunFrames() const;

    /* Get the flags */
            audio_output_flags_t getFlags() const { AutoMutex _l(mLock); return mFlags; }

    /* Set parameters - only possible when using direct output */
            status_t    setParameters(const String8& keyValuePairs);

    /* Sets the volume shaper object */
            media::VolumeShaper::Status applyVolumeShaper(
                    const sp<media::VolumeShaper::Configuration>& configuration,
                    const sp<media::VolumeShaper::Operation>& operation);

    /* Gets the volume shaper state */
            sp<media::VolumeShaper::State> getVolumeShaperState(int id);

    /* Selects the presentation (if available) */
            status_t    selectPresentation(int presentationId, int programId);

    /* Get parameters */
            String8     getParameters(const String8& keys);

    /* Poll for a timestamp on demand.
     * Use if EVENT_NEW_TIMESTAMP is not delivered often enough for your needs,
     * or if you need to get the most recent timestamp outside of the event callback handler.
     * Caution: calling this method too often may be inefficient;
     * if you need a high resolution mapping between frame position and presentation time,
     * consider implementing that at application level, based on the low resolution timestamps.
     * Returns NO_ERROR    if timestamp is valid.
     *         WOULD_BLOCK if called in STOPPED or FLUSHED state, or if called immediately after
     *                     start/ACTIVE, when the number of frames consumed is less than the
     *                     overall hardware latency to physical output. In WOULD_BLOCK cases,
     *                     one might poll again, or use getPosition(), or use 0 position and
     *                     current time for the timestamp.
     *         DEAD_OBJECT if AudioFlinger dies or the output device changes and
     *                     the track cannot be automatically restored.
     *                     The application needs to recreate the AudioTrack
     *                     because the audio device changed or AudioFlinger died.
     *                     This typically occurs for direct or offload tracks
     *                     or if mDoNotReconnect is true.
     *         INVALID_OPERATION  wrong state, or some other error.
     *
     * The timestamp parameter is undefined on return, if status is not NO_ERROR.
     */
            status_t    getTimestamp(AudioTimestamp& timestamp);
private:
            status_t    getTimestamp_l(AudioTimestamp& timestamp);
public:

    /* Return the extended timestamp, with additional timebase info and improved drain behavior.
     *
     * This is similar to the AudioTrack.java API:
     * getTimestamp(@NonNull AudioTimestamp timestamp, @AudioTimestamp.Timebase int timebase)
     *
     * Some differences between this method and the getTimestamp(AudioTimestamp& timestamp) method
     *
     *   1. stop() by itself does not reset the frame position.
     *      A following start() resets the frame position to 0.
     *   2. flush() by itself does not reset the frame position.
     *      The frame position advances by the number of frames flushed,
     *      when the first frame after flush reaches the audio sink.
     *   3. BOOTTIME clock offsets are provided to help synchronize with
     *      non-audio streams, e.g. sensor data.
     *   4. Position is returned with 64 bits of resolution.
     *
     * Parameters:
     *  timestamp: A pointer to the caller allocated ExtendedTimestamp.
     *
     * Returns NO_ERROR    on success; timestamp is filled with valid data.
     *         BAD_VALUE   if timestamp is NULL.
     *         WOULD_BLOCK if called immediately after start() when the number
     *                     of frames consumed is less than the
     *                     overall hardware latency to physical output. In WOULD_BLOCK cases,
     *                     one might poll again, or use getPosition(), or use 0 position and
     *                     current time for the timestamp.
     *                     If WOULD_BLOCK is returned, the timestamp is still
     *                     modified with the LOCATION_CLIENT portion filled.
     *         DEAD_OBJECT if AudioFlinger dies or the output device changes and
     *                     the track cannot be automatically restored.
     *                     The application needs to recreate the AudioTrack
     *                     because the audio device changed or AudioFlinger died.
     *                     This typically occurs for direct or offloaded tracks
     *                     or if mDoNotReconnect is true.
     *         INVALID_OPERATION  if called on a offloaded or direct track.
     *                     Use getTimestamp(AudioTimestamp& timestamp) instead.
     */
            status_t getTimestamp(ExtendedTimestamp *timestamp);
private:
            status_t getTimestamp_l(ExtendedTimestamp *timestamp);
public:

    /* Add an AudioDeviceCallback. The caller will be notified when the audio device to which this
     * AudioTrack is routed is updated.
     * Replaces any previously installed callback.
     * Parameters:
     *  callback:  The callback interface
     * Returns NO_ERROR if successful.
     *         INVALID_OPERATION if the same callback is already installed.
     *         NO_INIT or PREMISSION_DENIED if AudioFlinger service is not reachable
     *         BAD_VALUE if the callback is NULL
     */
            status_t addAudioDeviceCallback(const sp<AudioSystem::AudioDeviceCallback>& callback);

    /* remove an AudioDeviceCallback.
     * Parameters:
     *  callback:  The callback interface
     * Returns NO_ERROR if successful.
     *         INVALID_OPERATION if the callback is not installed
     *         BAD_VALUE if the callback is NULL
     */
            status_t removeAudioDeviceCallback(
                    const sp<AudioSystem::AudioDeviceCallback>& callback);

            // AudioSystem::AudioDeviceCallback> virtuals
            virtual void onAudioDeviceUpdate(audio_io_handle_t audioIo,
                                             audio_port_handle_t deviceId);



    /* Obtain the pending duration in milliseconds for playback of pure PCM
     * (mixable without embedded timing) data remaining in AudioTrack.
     *
     * This is used to estimate the drain time for the client-server buffer
     * so the choice of ExtendedTimestamp::LOCATION_SERVER is default.
     * One may optionally request to find the duration to play through the HAL
     * by specifying a location ExtendedTimestamp::LOCATION_KERNEL; however,
     * INVALID_OPERATION may be returned if the kernel location is unavailable.
     *
     * Returns NO_ERROR  if successful.
     *         INVALID_OPERATION if ExtendedTimestamp::LOCATION_KERNEL cannot be obtained
     *                   or the AudioTrack does not contain pure PCM data.
     *         BAD_VALUE if msec is nullptr or location is invalid.
     */
            status_t pendingDuration(int32_t *msec,
                    ExtendedTimestamp::Location location = ExtendedTimestamp::LOCATION_SERVER);

    /* hasStarted() is used to determine if audio is now audible at the device after
     * a start() command. The underlying implementation checks a nonzero timestamp position
     * or increment for the audible assumption.
     *
     * hasStarted() returns true if the track has been started() and audio is audible
     * and no subsequent pause() or flush() has been called.  Immediately after pause() or
     * flush() hasStarted() will return false.
     *
     * If stop() has been called, hasStarted() will return true if audio is still being
     * delivered or has finished delivery (even if no audio was written) for both offloaded
     * and normal tracks. This property removes a race condition in checking hasStarted()
     * for very short clips, where stop() must be called to finish drain.
     *
     * In all cases, hasStarted() may turn false briefly after a subsequent start() is called
     * until audio becomes audible again.
     */
            bool hasStarted(); // not const

            bool isPlaying() {
                AutoMutex lock(mLock);
                return mState == STATE_ACTIVE || mState == STATE_STOPPING;
            }

    /* Get the unique port ID assigned to this AudioTrack instance by audio policy manager.
     * The ID is unique across all audioserver clients and can change during the life cycle
     * of a given AudioTrack instance if the connection to audioserver is restored.
     */
            audio_port_handle_t getPortId() const { return mPortId; };

 protected:
    /* copying audio tracks is not allowed */
                        AudioTrack(const AudioTrack& other);
            AudioTrack& operator = (const AudioTrack& other);

    /* a small internal class to handle the callback */
    class AudioTrackThread : public Thread
    {
    public:
        AudioTrackThread(AudioTrack& receiver);

        // Do not call Thread::requestExitAndWait() without first calling requestExit().
        // Thread::requestExitAndWait() is not virtual, and the implementation doesn't do enough.
        virtual void        requestExit();

                void        pause();    // suspend thread from execution at next loop boundary
                void        resume();   // allow thread to execute, if not requested to exit
                void        wake();     // wake to handle changed notification conditions.

    private:
                void        pauseInternal(nsecs_t ns = 0LL);
                                        // like pause(), but only used internally within thread

        friend class AudioTrack;
        virtual bool        threadLoop();
        AudioTrack&         mReceiver;
        virtual ~AudioTrackThread();
        Mutex               mMyLock;    // Thread::mLock is private
        Condition           mMyCond;    // Thread::mThreadExitedCondition is private
        bool                mPaused;    // whether thread is requested to pause at next loop entry
        bool                mPausedInt; // whether thread internally requests pause
        nsecs_t             mPausedNs;  // if mPausedInt then associated timeout, otherwise ignored
        bool                mIgnoreNextPausedInt;   // skip any internal pause and go immediately
                                        // to processAudioBuffer() as state may have changed
                                        // since pause time calculated.
    };

            // body of AudioTrackThread::threadLoop()
            // returns the maximum amount of time before we would like to run again, where:
            //      0           immediately
            //      > 0         no later than this many nanoseconds from now
            //      NS_WHENEVER still active but no particular deadline
            //      NS_INACTIVE inactive so don't run again until re-started
            //      NS_NEVER    never again
            static const nsecs_t NS_WHENEVER = -1, NS_INACTIVE = -2, NS_NEVER = -3;
            nsecs_t processAudioBuffer();

            // caller must hold lock on mLock for all _l methods

            void updateLatency_l(); // updates mAfLatency and mLatency from AudioSystem cache

            status_t createTrack_l();

            // can only be called when mState != STATE_ACTIVE
            void flush_l();

            void setLoop_l(uint32_t loopStart, uint32_t loopEnd, int loopCount);

            // FIXME enum is faster than strcmp() for parameter 'from'
            status_t restoreTrack_l(const char *from);

            uint32_t    getUnderrunCount_l() const;

            bool     isOffloaded() const;
            bool     isDirect() const;
            bool     isOffloadedOrDirect() const;

            bool     isOffloaded_l() const
                { return (mFlags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) != 0; }

            bool     isOffloadedOrDirect_l() const
                { return (mFlags & (AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD|
                                                AUDIO_OUTPUT_FLAG_DIRECT)) != 0; }

            bool     isDirect_l() const
                { return (mFlags & AUDIO_OUTPUT_FLAG_DIRECT) != 0; }

            // pure pcm data is mixable (which excludes HW_AV_SYNC, with embedded timing)
            bool     isPurePcmData_l() const
                { return audio_is_linear_pcm(mFormat)
                        && (mAttributes.flags & AUDIO_FLAG_HW_AV_SYNC) == 0; }

            // increment mPosition by the delta of mServer, and return new value of mPosition
            Modulo<uint32_t> updateAndGetPosition_l();

            // check sample rate and speed is compatible with AudioTrack
            bool     isSampleRateSpeedAllowed_l(uint32_t sampleRate, float speed);

            void     restartIfDisabled();

            void     updateRoutedDeviceId_l();

    // Next 4 fields may be changed if IAudioTrack is re-created, but always != 0
    sp<IAudioTrack>         mAudioTrack;
    sp<IMemory>             mCblkMemory;
    audio_track_cblk_t*     mCblk;                  // re-load after mLock.unlock()
    audio_io_handle_t       mOutput = AUDIO_IO_HANDLE_NONE; // from AudioSystem::getOutputForAttr()

    sp<AudioTrackThread>    mAudioTrackThread;
    bool                    mThreadCanCallJava;

    float                   mVolume[2];
    float                   mSendLevel;
    mutable uint32_t        mSampleRate;            // mutable because getSampleRate() can update it
    uint32_t                mOriginalSampleRate;
    AudioPlaybackRate       mPlaybackRate;
    float                   mMaxRequiredSpeed;      // use PCM buffer size to allow this speed

    // Corresponds to current IAudioTrack, value is reported back by AudioFlinger to the client.
    // This allocated buffer size is maintained by the proxy.
    size_t                  mFrameCount;            // maximum size of buffer

    size_t                  mReqFrameCount;         // frame count to request the first or next time
                                                    // a new IAudioTrack is needed, non-decreasing

    // The following AudioFlinger server-side values are cached in createAudioTrack_l().
    // These values can be used for informational purposes until the track is invalidated,
    // whereupon restoreTrack_l() calls createTrack_l() to update the values.
    uint32_t                mAfLatency;             // AudioFlinger latency in ms
    size_t                  mAfFrameCount;          // AudioFlinger frame count
    uint32_t                mAfSampleRate;          // AudioFlinger sample rate

    // constant after constructor or set()
    audio_format_t          mFormat;                // as requested by client, not forced to 16-bit
    audio_stream_type_t     mStreamType;            // mStreamType == AUDIO_STREAM_DEFAULT implies
                                                    // this AudioTrack has valid attributes
    uint32_t                mChannelCount;
    audio_channel_mask_t    mChannelMask;
    sp<IMemory>             mSharedBuffer;
    transfer_type           mTransfer;
    audio_offload_info_t    mOffloadInfoCopy;
    const audio_offload_info_t* mOffloadInfo;
    audio_attributes_t      mAttributes;

    size_t                  mFrameSize;             // frame size in bytes

    status_t                mStatus;

    // can change dynamically when IAudioTrack invalidated
    uint32_t                mLatency;               // in ms

    // Indicates the current track state.  Protected by mLock.
    enum State {
        STATE_ACTIVE,
        STATE_STOPPED,
        STATE_PAUSED,
        STATE_PAUSED_STOPPING,
        STATE_FLUSHED,
        STATE_STOPPING,
    }                       mState;

    static constexpr const char *stateToString(State state)
    {
        switch (state) {
        case STATE_ACTIVE:          return "STATE_ACTIVE";
        case STATE_STOPPED:         return "STATE_STOPPED";
        case STATE_PAUSED:          return "STATE_PAUSED";
        case STATE_PAUSED_STOPPING: return "STATE_PAUSED_STOPPING";
        case STATE_FLUSHED:         return "STATE_FLUSHED";
        case STATE_STOPPING:        return "STATE_STOPPING";
        default:                    return "UNKNOWN";
        }
    }

    // for client callback handler
    callback_t              mCbf;                   // callback handler for events, or NULL
    void*                   mUserData;

    // for notification APIs

    // next 2 fields are const after constructor or set()
    uint32_t                mNotificationFramesReq; // requested number of frames between each
                                                    // notification callback,
                                                    // at initial source sample rate
    uint32_t                mNotificationsPerBufferReq;
                                                    // requested number of notifications per buffer,
                                                    // currently only used for fast tracks with
                                                    // default track buffer size

    uint32_t                mNotificationFramesAct; // actual number of frames between each
                                                    // notification callback,
                                                    // at initial source sample rate
    bool                    mRefreshRemaining;      // processAudioBuffer() should refresh
                                                    // mRemainingFrames and mRetryOnPartialBuffer

                                                    // used for static track cbf and restoration
    int32_t                 mLoopCount;             // last setLoop loopCount; zero means disabled
    uint32_t                mLoopStart;             // last setLoop loopStart
    uint32_t                mLoopEnd;               // last setLoop loopEnd
    int32_t                 mLoopCountNotified;     // the last loopCount notified by callback.
                                                    // mLoopCountNotified counts down, matching
                                                    // the remaining loop count for static track
                                                    // playback.

    // These are private to processAudioBuffer(), and are not protected by a lock
    uint32_t                mRemainingFrames;       // number of frames to request in obtainBuffer()
    bool                    mRetryOnPartialBuffer;  // sleep and retry after partial obtainBuffer()
    uint32_t                mObservedSequence;      // last observed value of mSequence

    Modulo<uint32_t>        mMarkerPosition;        // in wrapping (overflow) frame units
    bool                    mMarkerReached;
    Modulo<uint32_t>        mNewPosition;           // in frames
    uint32_t                mUpdatePeriod;          // in frames, zero means no EVENT_NEW_POS

    Modulo<uint32_t>        mServer;                // in frames, last known mProxy->getPosition()
                                                    // which is count of frames consumed by server,
                                                    // reset by new IAudioTrack,
                                                    // whether it is reset by stop() is TBD
    Modulo<uint32_t>        mPosition;              // in frames, like mServer except continues
                                                    // monotonically after new IAudioTrack,
                                                    // and could be easily widened to uint64_t
    Modulo<uint32_t>        mReleased;              // count of frames released to server
                                                    // but not necessarily consumed by server,
                                                    // reset by stop() but continues monotonically
                                                    // after new IAudioTrack to restore mPosition,
                                                    // and could be easily widened to uint64_t
    int64_t                 mStartFromZeroUs;       // the start time after flush or stop,
                                                    // when position should be 0.
                                                    // only used for offloaded and direct tracks.
    int64_t                 mStartNs;               // the time when start() is called.
    ExtendedTimestamp       mStartEts;              // Extended timestamp at start for normal
                                                    // AudioTracks.
    AudioTimestamp          mStartTs;               // Timestamp at start for offloaded or direct
                                                    // AudioTracks.

    bool                    mPreviousTimestampValid;// true if mPreviousTimestamp is valid
    bool                    mTimestampStartupGlitchReported;      // reduce log spam
    bool                    mTimestampRetrogradePositionReported; // reduce log spam
    bool                    mTimestampRetrogradeTimeReported;     // reduce log spam
    bool                    mTimestampStallReported;              // reduce log spam
    bool                    mTimestampStaleTimeReported;          // reduce log spam
    AudioTimestamp          mPreviousTimestamp;     // used to detect retrograde motion
    ExtendedTimestamp::Location mPreviousLocation;  // location used for previous timestamp

    uint32_t                mUnderrunCountOffset;   // updated when restoring tracks

    int64_t                 mFramesWritten;         // total frames written. reset to zero after
                                                    // the start() following stop(). It is not
                                                    // changed after restoring the track or
                                                    // after flush.
    int64_t                 mFramesWrittenServerOffset; // An offset to server frames due to
                                                    // restoring AudioTrack, or stop/start.
                                                    // This offset is also used for static tracks.
    int64_t                 mFramesWrittenAtRestore; // Frames written at restore point (or frames
                                                    // delivered for static tracks).
                                                    // -1 indicates no previous restore point.

    audio_output_flags_t    mFlags;                 // same as mOrigFlags, except for bits that may
                                                    // be denied by client or server, such as
                                                    // AUDIO_OUTPUT_FLAG_FAST.  mLock must be
                                                    // held to read or write those bits reliably.
    audio_output_flags_t    mOrigFlags;             // as specified in constructor or set(), const

    bool                    mDoNotReconnect;

    audio_session_t         mSessionId;
    int                     mAuxEffectId;
    audio_port_handle_t     mPortId;                    // Id from Audio Policy Manager

    mutable Mutex           mLock;

    int                     mPreviousPriority;          // before start()
    SchedPolicy             mPreviousSchedulingGroup;
    bool                    mAwaitBoost;    // thread should wait for priority boost before running

    // The proxy should only be referenced while a lock is held because the proxy isn't
    // multi-thread safe, especially the SingleStateQueue part of the proxy.
    // An exception is that a blocking ClientProxy::obtainBuffer() may be called without a lock,
    // provided that the caller also holds an extra reference to the proxy and shared memory to keep
    // them around in case they are replaced during the obtainBuffer().
    sp<StaticAudioTrackClientProxy> mStaticProxy;   // for type safety only
    sp<AudioTrackClientProxy>       mProxy;         // primary owner of the memory

    bool                    mInUnderrun;            // whether track is currently in underrun state
    uint32_t                mPausedPosition;

    // For Device Selection API
    //  a value of AUDIO_PORT_HANDLE_NONE indicated default (AudioPolicyManager) routing.
    audio_port_handle_t    mSelectedDeviceId; // Device requested by the application.
    audio_port_handle_t    mRoutedDeviceId;   // Device actually selected by audio policy manager:
                                              // May not match the app selection depending on other
                                              // activity and connected devices.

    sp<media::VolumeHandler>       mVolumeHandler;

private:
    class DeathNotifier : public IBinder::DeathRecipient {
    public:
        DeathNotifier(AudioTrack* audioTrack) : mAudioTrack(audioTrack) { }
    protected:
        virtual void        binderDied(const wp<IBinder>& who);
    private:
        const wp<AudioTrack> mAudioTrack;
    };

    sp<DeathNotifier>       mDeathNotifier;
    uint32_t                mSequence;              // incremented for each new IAudioTrack attempt
    uid_t                   mClientUid;
    pid_t                   mClientPid;

    wp<AudioSystem::AudioDeviceCallback> mDeviceCallback;

private:
    class MediaMetrics {
      public:
        MediaMetrics() : mAnalyticsItem(MediaAnalyticsItem::create("audiotrack")) {
        }
        ~MediaMetrics() {
            // mAnalyticsItem alloc failure will be flagged in the constructor
            // don't log empty records
            if (mAnalyticsItem->count() > 0) {
                mAnalyticsItem->selfrecord();
            }
        }
        void gather(const AudioTrack *track);
        MediaAnalyticsItem *dup() { return mAnalyticsItem->dup(); }
      private:
        std::unique_ptr<MediaAnalyticsItem> mAnalyticsItem;
    };
    MediaMetrics mMediaMetrics;
};

}; // namespace android

#endif // ANDROID_AUDIOTRACK_H
