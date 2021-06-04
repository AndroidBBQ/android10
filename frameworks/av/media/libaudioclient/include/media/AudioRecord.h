/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_AUDIORECORD_H
#define ANDROID_AUDIORECORD_H

#include <memory>
#include <vector>

#include <binder/IMemory.h>
#include <cutils/sched_policy.h>
#include <media/AudioSystem.h>
#include <media/AudioTimestamp.h>
#include <media/MediaAnalyticsItem.h>
#include <media/Modulo.h>
#include <media/MicrophoneInfo.h>
#include <media/RecordingActivityTracker.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

#include "android/media/IAudioRecord.h"

namespace android {

// ----------------------------------------------------------------------------

struct audio_track_cblk_t;
class AudioRecordClientProxy;

// ----------------------------------------------------------------------------

class AudioRecord : public AudioSystem::AudioDeviceCallback
{
public:

    /* Events used by AudioRecord callback function (callback_t).
     * Keep in sync with frameworks/base/media/java/android/media/AudioRecord.java NATIVE_EVENT_*.
     */
    enum event_type {
        EVENT_MORE_DATA = 0,        // Request to read available data from buffer.
                                    // If this event is delivered but the callback handler
                                    // does not want to read the available data, the handler must
                                    // explicitly ignore the event by setting frameCount to zero.
        EVENT_OVERRUN = 1,          // Buffer overrun occurred.
        EVENT_MARKER = 2,           // Record head is at the specified marker position
                                    // (See setMarkerPosition()).
        EVENT_NEW_POS = 3,          // Record head is at a new position
                                    // (See setPositionUpdatePeriod()).
        EVENT_NEW_IAUDIORECORD = 4, // IAudioRecord was re-created, either due to re-routing and
                                    // voluntary invalidation by mediaserver, or mediaserver crash.
    };

    /* Client should declare a Buffer and pass address to obtainBuffer()
     * and releaseBuffer().  See also callback_t for EVENT_MORE_DATA.
     */

    class Buffer
    {
    public:
        // FIXME use m prefix
        size_t      frameCount;     // number of sample frames corresponding to size;
                                    // on input to obtainBuffer() it is the number of frames desired
                                    // on output from obtainBuffer() it is the number of available
                                    //    frames to be read
                                    // on input to releaseBuffer() it is currently ignored

        size_t      size;           // input/output in bytes == frameCount * frameSize
                                    // on input to obtainBuffer() it is ignored
                                    // on output from obtainBuffer() it is the number of available
                                    //    bytes to be read, which is frameCount * frameSize
                                    // on input to releaseBuffer() it is the number of bytes to
                                    //    release
                                    // FIXME This is redundant with respect to frameCount.  Consider
                                    //    removing size and making frameCount the primary field.

        union {
            void*       raw;
            int16_t*    i16;        // signed 16-bit
            int8_t*     i8;         // unsigned 8-bit, offset by 0x80
                                    // input to obtainBuffer(): unused, output: pointer to buffer
        };

        uint32_t    sequence;       // IAudioRecord instance sequence number, as of obtainBuffer().
                                    // It is set by obtainBuffer() and confirmed by releaseBuffer().
                                    // Not "user-serviceable".
                                    // TODO Consider sp<IMemory> instead, or in addition to this.
    };

    /* As a convenience, if a callback is supplied, a handler thread
     * is automatically created with the appropriate priority. This thread
     * invokes the callback when a new buffer becomes available or various conditions occur.
     * Parameters:
     *
     * event:   type of event notified (see enum AudioRecord::event_type).
     * user:    Pointer to context for use by the callback receiver.
     * info:    Pointer to optional parameter according to event type:
     *          - EVENT_MORE_DATA: pointer to AudioRecord::Buffer struct. The callback must not read
     *                             more bytes than indicated by 'size' field and update 'size' if
     *                             fewer bytes are consumed.
     *          - EVENT_OVERRUN: unused.
     *          - EVENT_MARKER: pointer to const uint32_t containing the marker position in frames.
     *          - EVENT_NEW_POS: pointer to const uint32_t containing the new position in frames.
     *          - EVENT_NEW_IAUDIORECORD: unused.
     */

    typedef void (*callback_t)(int event, void* user, void *info);

    /* Returns the minimum frame count required for the successful creation of
     * an AudioRecord object.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - NO_INIT: audio server or audio hardware not initialized
     *  - BAD_VALUE: unsupported configuration
     * frameCount is guaranteed to be non-zero if status is NO_ERROR,
     * and is undefined otherwise.
     * FIXME This API assumes a route, and so should be deprecated.
     */

     static status_t getMinFrameCount(size_t* frameCount,
                                      uint32_t sampleRate,
                                      audio_format_t format,
                                      audio_channel_mask_t channelMask);

    /* How data is transferred from AudioRecord
     */
    enum transfer_type {
        TRANSFER_DEFAULT,   // not specified explicitly; determine from the other parameters
        TRANSFER_CALLBACK,  // callback EVENT_MORE_DATA
        TRANSFER_OBTAIN,    // call obtainBuffer() and releaseBuffer()
        TRANSFER_SYNC,      // synchronous read()
    };

    /* Constructs an uninitialized AudioRecord. No connection with
     * AudioFlinger takes place.  Use set() after this.
     *
     * Parameters:
     *
     * opPackageName:      The package name used for app ops.
     */
                        AudioRecord(const String16& opPackageName);

    /* Creates an AudioRecord object and registers it with AudioFlinger.
     * Once created, the track needs to be started before it can be used.
     * Unspecified values are set to appropriate default values.
     *
     * Parameters:
     *
     * inputSource:        Select the audio input to record from (e.g. AUDIO_SOURCE_DEFAULT).
     * sampleRate:         Data sink sampling rate in Hz.  Zero means to use the source sample rate.
     * format:             Audio format (e.g AUDIO_FORMAT_PCM_16_BIT for signed
     *                     16 bits per sample).
     * channelMask:        Channel mask, such that audio_is_input_channel(channelMask) is true.
     * opPackageName:      The package name used for app ops.
     * frameCount:         Minimum size of track PCM buffer in frames. This defines the
     *                     application's contribution to the
     *                     latency of the track.  The actual size selected by the AudioRecord could
     *                     be larger if the requested size is not compatible with current audio HAL
     *                     latency.  Zero means to use a default value.
     * cbf:                Callback function. If not null, this function is called periodically
     *                     to consume new data in TRANSFER_CALLBACK mode
     *                     and inform of marker, position updates, etc.
     * user:               Context for use by the callback receiver.
     * notificationFrames: The callback function is called each time notificationFrames PCM
     *                     frames are ready in record track output buffer.
     * sessionId:          Not yet supported.
     * transferType:       How data is transferred from AudioRecord.
     * flags:              See comments on audio_input_flags_t in <system/audio.h>
     * pAttributes:        If not NULL, supersedes inputSource for use case selection.
     * threadCanCallJava:  Not present in parameter list, and so is fixed at false.
     */

                        AudioRecord(audio_source_t inputSource,
                                    uint32_t sampleRate,
                                    audio_format_t format,
                                    audio_channel_mask_t channelMask,
                                    const String16& opPackageName,
                                    size_t frameCount = 0,
                                    callback_t cbf = NULL,
                                    void* user = NULL,
                                    uint32_t notificationFrames = 0,
                                    audio_session_t sessionId = AUDIO_SESSION_ALLOCATE,
                                    transfer_type transferType = TRANSFER_DEFAULT,
                                    audio_input_flags_t flags = AUDIO_INPUT_FLAG_NONE,
                                    uid_t uid = AUDIO_UID_INVALID,
                                    pid_t pid = -1,
                                    const audio_attributes_t* pAttributes = NULL,
                                    audio_port_handle_t selectedDeviceId = AUDIO_PORT_HANDLE_NONE,
                                    audio_microphone_direction_t
                                        selectedMicDirection = MIC_DIRECTION_UNSPECIFIED,
                                    float selectedMicFieldDimension = MIC_FIELD_DIMENSION_DEFAULT);

    /* Terminates the AudioRecord and unregisters it from AudioFlinger.
     * Also destroys all resources associated with the AudioRecord.
     */
protected:
                        virtual ~AudioRecord();
public:

    /* Initialize an AudioRecord that was created using the AudioRecord() constructor.
     * Don't call set() more than once, or after an AudioRecord() constructor that takes parameters.
     * set() is not multi-thread safe.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful intialization
     *  - INVALID_OPERATION: AudioRecord is already initialized or record device is already in use
     *  - BAD_VALUE: invalid parameter (channelMask, format, sampleRate...)
     *  - NO_INIT: audio server or audio hardware not initialized
     *  - PERMISSION_DENIED: recording is not allowed for the requesting process
     * If status is not equal to NO_ERROR, don't call any other APIs on this AudioRecord.
     *
     * Parameters not listed in the AudioRecord constructors above:
     *
     * threadCanCallJava:  Whether callbacks are made from an attached thread and thus can call JNI.
     */
            status_t    set(audio_source_t inputSource,
                            uint32_t sampleRate,
                            audio_format_t format,
                            audio_channel_mask_t channelMask,
                            size_t frameCount = 0,
                            callback_t cbf = NULL,
                            void* user = NULL,
                            uint32_t notificationFrames = 0,
                            bool threadCanCallJava = false,
                            audio_session_t sessionId = AUDIO_SESSION_ALLOCATE,
                            transfer_type transferType = TRANSFER_DEFAULT,
                            audio_input_flags_t flags = AUDIO_INPUT_FLAG_NONE,
                            uid_t uid = AUDIO_UID_INVALID,
                            pid_t pid = -1,
                            const audio_attributes_t* pAttributes = NULL,
                            audio_port_handle_t selectedDeviceId = AUDIO_PORT_HANDLE_NONE,
                            audio_microphone_direction_t
                                selectedMicDirection = MIC_DIRECTION_UNSPECIFIED,
                            float selectedMicFieldDimension = MIC_FIELD_DIMENSION_DEFAULT);

    /* Result of constructing the AudioRecord. This must be checked for successful initialization
     * before using any AudioRecord API (except for set()), because using
     * an uninitialized AudioRecord produces undefined results.
     * See set() method above for possible return codes.
     */
            status_t    initCheck() const   { return mStatus; }

    /* Returns this track's estimated latency in milliseconds.
     * This includes the latency due to AudioRecord buffer size, resampling if applicable,
     * and audio hardware driver.
     */
            uint32_t    latency() const     { return mLatency; }

   /* getters, see constructor and set() */

            audio_format_t format() const   { return mFormat; }
            uint32_t    channelCount() const    { return mChannelCount; }
            size_t      frameCount() const  { return mFrameCount; }
            size_t      frameSize() const   { return mFrameSize; }
            audio_source_t inputSource() const  { return mAttributes.source; }

    /*
     * Return the period of the notification callback in frames.
     * This value is set when the AudioRecord is constructed.
     * It can be modified if the AudioRecord is rerouted.
     */
            uint32_t    getNotificationPeriodInFrames() const { return mNotificationFramesAct; }

    /*
     * return metrics information for the current instance.
     */
            status_t getMetrics(MediaAnalyticsItem * &item);

    /* After it's created the track is not active. Call start() to
     * make it active. If set, the callback will start being called.
     * If event is not AudioSystem::SYNC_EVENT_NONE, the capture start will be delayed until
     * the specified event occurs on the specified trigger session.
     */
            status_t    start(AudioSystem::sync_event_t event = AudioSystem::SYNC_EVENT_NONE,
                              audio_session_t triggerSession = AUDIO_SESSION_NONE);

    /* Stop a track.  The callback will cease being called.  Note that obtainBuffer() still
     * works and will drain buffers until the pool is exhausted, and then will return WOULD_BLOCK.
     */
            void        stop();
            bool        stopped() const;

    /* Return the sink sample rate for this record track in Hz.
     * If specified as zero in constructor or set(), this will be the source sample rate.
     * Unlike AudioTrack, the sample rate is const after initialization, so doesn't need a lock.
     */
            uint32_t    getSampleRate() const   { return mSampleRate; }

    /* Sets marker position. When record reaches the number of frames specified,
     * a callback with event type EVENT_MARKER is called. Calling setMarkerPosition
     * with marker == 0 cancels marker notification callback.
     * To set a marker at a position which would compute as 0,
     * a workaround is to set the marker at a nearby position such as ~0 or 1.
     * If the AudioRecord has been opened with no callback function associated,
     * the operation will fail.
     *
     * Parameters:
     *
     * marker:   marker position expressed in wrapping (overflow) frame units,
     *           like the return value of getPosition().
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioRecord has no callback installed.
     */
            status_t    setMarkerPosition(uint32_t marker);
            status_t    getMarkerPosition(uint32_t *marker) const;

    /* Sets position update period. Every time the number of frames specified has been recorded,
     * a callback with event type EVENT_NEW_POS is called.
     * Calling setPositionUpdatePeriod with updatePeriod == 0 cancels new position notification
     * callback.
     * If the AudioRecord has been opened with no callback function associated,
     * the operation will fail.
     * Extremely small values may be rounded up to a value the implementation can support.
     *
     * Parameters:
     *
     * updatePeriod:  position update notification period expressed in frames.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioRecord has no callback installed.
     */
            status_t    setPositionUpdatePeriod(uint32_t updatePeriod);
            status_t    getPositionUpdatePeriod(uint32_t *updatePeriod) const;

    /* Return the total number of frames recorded since recording started.
     * The counter will wrap (overflow) periodically, e.g. every ~27 hours at 44.1 kHz.
     * It is reset to zero by stop().
     *
     * Parameters:
     *
     *  position:  Address where to return record head position.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - BAD_VALUE:  position is NULL
     */
            status_t    getPosition(uint32_t *position) const;

    /* Return the record timestamp.
     *
     * Parameters:
     *  timestamp: A pointer to the timestamp to be filled.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - BAD_VALUE: timestamp is NULL
     */
            status_t getTimestamp(ExtendedTimestamp *timestamp);

    /**
     * @param transferType
     * @return text string that matches the enum name
     */
    static const char * convertTransferToText(transfer_type transferType);

    /* Returns a handle on the audio input used by this AudioRecord.
     *
     * Parameters:
     *  none.
     *
     * Returned value:
     *  handle on audio hardware input
     */
// FIXME The only known public caller is frameworks/opt/net/voip/src/jni/rtp/AudioGroup.cpp
            audio_io_handle_t    getInput() const __attribute__((__deprecated__))
                                                { return getInputPrivate(); }
private:
            audio_io_handle_t    getInputPrivate() const;
public:

    /* Returns the audio session ID associated with this AudioRecord.
     *
     * Parameters:
     *  none.
     *
     * Returned value:
     *  AudioRecord session ID.
     *
     * No lock needed because session ID doesn't change after first set().
     */
            audio_session_t getSessionId() const { return mSessionId; }

    /* Public API for TRANSFER_OBTAIN mode.
     * Obtains a buffer of up to "audioBuffer->frameCount" full frames.
     * After draining these frames of data, the caller should release them with releaseBuffer().
     * If the track buffer is not empty, obtainBuffer() returns as many contiguous
     * full frames as are available immediately.
     *
     * If nonContig is non-NULL, it is an output parameter that will be set to the number of
     * additional non-contiguous frames that are predicted to be available immediately,
     * if the client were to release the first frames and then call obtainBuffer() again.
     * This value is only a prediction, and needs to be confirmed.
     * It will be set to zero for an error return.
     *
     * If the track buffer is empty and track is stopped, obtainBuffer() returns WOULD_BLOCK
     * regardless of the value of waitCount.
     * If the track buffer is empty and track is not stopped, obtainBuffer() blocks with a
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
     *  frameCount  number of frames requested
     *  size        ignored
     *  raw         ignored
     *  sequence    ignored
     * After error return:
     *  frameCount  0
     *  size        0
     *  raw         undefined
     *  sequence    undefined
     * After successful return:
     *  frameCount  actual number of frames available, <= number requested
     *  size        actual number of bytes available
     *  raw         pointer to the buffer
     *  sequence    IAudioRecord instance sequence number, as of obtainBuffer()
     */

            status_t    obtainBuffer(Buffer* audioBuffer, int32_t waitCount,
                                size_t *nonContig = NULL);

            // Explicit Routing
    /**
     * TODO Document this method.
     */
            status_t setInputDevice(audio_port_handle_t deviceId);

    /**
     * TODO Document this method.
     */
            audio_port_handle_t getInputDevice();

     /* Returns the ID of the audio device actually used by the input to which this AudioRecord
      * is attached.
      * The device ID is relevant only if the AudioRecord is active.
      * When the AudioRecord is inactive, the device ID returned can be either:
      * - AUDIO_PORT_HANDLE_NONE if the AudioRecord is not attached to any output.
      * - The device ID used before paused or stopped.
      * - The device ID selected by audio policy manager of setOutputDevice() if the AudioRecord
      * has not been started yet.
      *
      * Parameters:
      *  none.
      */
     audio_port_handle_t getRoutedDeviceId();

    /* Add an AudioDeviceCallback. The caller will be notified when the audio device
     * to which this AudioRecord is routed is updated.
     * Replaces any previously installed callback.
     * Parameters:
     *  callback:  The callback interface
     * Returns NO_ERROR if successful.
     *         INVALID_OPERATION if the same callback is already installed.
     *         NO_INIT or PREMISSION_DENIED if AudioFlinger service is not reachable
     *         BAD_VALUE if the callback is NULL
     */
            status_t addAudioDeviceCallback(
                    const sp<AudioSystem::AudioDeviceCallback>& callback);

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
     * Release an emptied buffer of "audioBuffer->frameCount" frames for AudioFlinger to re-fill.
     *
     * Buffer fields:
     *  frameCount  currently ignored but recommend to set to actual number of frames consumed
     *  size        actual number of bytes consumed, must be multiple of frameSize
     *  raw         ignored
     */
            void        releaseBuffer(const Buffer* audioBuffer);

    /* As a convenience we provide a read() interface to the audio buffer.
     * Input parameter 'size' is in byte units.
     * This is implemented on top of obtainBuffer/releaseBuffer. For best
     * performance use callbacks. Returns actual number of bytes read >= 0,
     * or one of the following negative status codes:
     *      INVALID_OPERATION   AudioRecord is configured for streaming mode
     *      BAD_VALUE           size is invalid
     *      WOULD_BLOCK         when obtainBuffer() returns same, or
     *                          AudioRecord was stopped during the read
     *      or any other error code returned by IAudioRecord::start() or restoreRecord_l().
     * Default behavior is to only return when all data has been transferred. Set 'blocking' to
     * false for the method to return immediately without waiting to try multiple times to read
     * the full content of the buffer.
     */
            ssize_t     read(void* buffer, size_t size, bool blocking = true);

    /* Return the number of input frames lost in the audio driver since the last call of this
     * function.  Audio driver is expected to reset the value to 0 and restart counting upon
     * returning the current value by this function call.  Such loss typically occurs when the
     * user space process is blocked longer than the capacity of audio driver buffers.
     * Units: the number of input audio frames.
     * FIXME The side-effect of resetting the counter may be incompatible with multi-client.
     * Consider making it more like AudioTrack::getUnderrunFrames which doesn't have side effects.
     */
            uint32_t    getInputFramesLost() const;

    /* Get the flags */
            audio_input_flags_t getFlags() const { AutoMutex _l(mLock); return mFlags; }

    /* Get active microphones. A empty vector of MicrophoneInfo will be passed as a parameter,
     * the data will be filled when querying the hal.
     */
            status_t    getActiveMicrophones(std::vector<media::MicrophoneInfo>* activeMicrophones);

    /* Set the Microphone direction (for processing purposes).
     */
            status_t    setPreferredMicrophoneDirection(audio_microphone_direction_t direction);

    /* Set the Microphone zoom factor (for processing purposes).
     */
            status_t    setPreferredMicrophoneFieldDimension(float zoom);

     /* Get the unique port ID assigned to this AudioRecord instance by audio policy manager.
      * The ID is unique across all audioserver clients and can change during the life cycle
      * of a given AudioRecord instance if the connection to audioserver is restored.
      */
            audio_port_handle_t getPortId() const { return mPortId; };

     /*
      * Dumps the state of an audio record.
      */
            status_t    dump(int fd, const Vector<String16>& args) const;

private:
    /* copying audio record objects is not allowed */
                        AudioRecord(const AudioRecord& other);
            AudioRecord& operator = (const AudioRecord& other);

    /* a small internal class to handle the callback */
    class AudioRecordThread : public Thread
    {
    public:
        AudioRecordThread(AudioRecord& receiver);

        // Do not call Thread::requestExitAndWait() without first calling requestExit().
        // Thread::requestExitAndWait() is not virtual, and the implementation doesn't do enough.
        virtual void        requestExit();

                void        pause();    // suspend thread from execution at next loop boundary
                void        resume();   // allow thread to execute, if not requested to exit
                void        wake();     // wake to handle changed notification conditions.

    private:
                void        pauseInternal(nsecs_t ns = 0LL);
                                        // like pause(), but only used internally within thread

        friend class AudioRecord;
        virtual bool        threadLoop();
        AudioRecord&        mReceiver;
        virtual ~AudioRecordThread();
        Mutex               mMyLock;    // Thread::mLock is private
        Condition           mMyCond;    // Thread::mThreadExitedCondition is private
        bool                mPaused;    // whether thread is requested to pause at next loop entry
        bool                mPausedInt; // whether thread internally requests pause
        nsecs_t             mPausedNs;  // if mPausedInt then associated timeout, otherwise ignored
        bool                mIgnoreNextPausedInt;   // skip any internal pause and go immediately
                                        // to processAudioBuffer() as state may have changed
                                        // since pause time calculated.
    };

            // body of AudioRecordThread::threadLoop()
            // returns the maximum amount of time before we would like to run again, where:
            //      0           immediately
            //      > 0         no later than this many nanoseconds from now
            //      NS_WHENEVER still active but no particular deadline
            //      NS_INACTIVE inactive so don't run again until re-started
            //      NS_NEVER    never again
            static const nsecs_t NS_WHENEVER = -1, NS_INACTIVE = -2, NS_NEVER = -3;
            nsecs_t processAudioBuffer();

            // caller must hold lock on mLock for all _l methods

            status_t createRecord_l(const Modulo<uint32_t> &epoch, const String16& opPackageName);

            // FIXME enum is faster than strcmp() for parameter 'from'
            status_t restoreRecord_l(const char *from);

            void     updateRoutedDeviceId_l();

    sp<AudioRecordThread>   mAudioRecordThread;
    mutable Mutex           mLock;

    std::unique_ptr<RecordingActivityTracker> mTracker;

    // Current client state:  false = stopped, true = active.  Protected by mLock.  If more states
    // are added, consider changing this to enum State { ... } mState as in AudioTrack.
    bool                    mActive;

    // for client callback handler
    callback_t              mCbf;                   // callback handler for events, or NULL
    void*                   mUserData;

    // for notification APIs
    uint32_t                mNotificationFramesReq; // requested number of frames between each
                                                    // notification callback
                                                    // as specified in constructor or set()
    uint32_t                mNotificationFramesAct; // actual number of frames between each
                                                    // notification callback
    bool                    mRefreshRemaining;      // processAudioBuffer() should refresh
                                                    // mRemainingFrames and mRetryOnPartialBuffer

    // These are private to processAudioBuffer(), and are not protected by a lock
    uint32_t                mRemainingFrames;       // number of frames to request in obtainBuffer()
    bool                    mRetryOnPartialBuffer;  // sleep and retry after partial obtainBuffer()
    uint32_t                mObservedSequence;      // last observed value of mSequence

    Modulo<uint32_t>        mMarkerPosition;        // in wrapping (overflow) frame units
    bool                    mMarkerReached;
    Modulo<uint32_t>        mNewPosition;           // in frames
    uint32_t                mUpdatePeriod;          // in frames, zero means no EVENT_NEW_POS

    status_t                mStatus;

    String16                mOpPackageName;         // The package name used for app ops.

    size_t                  mFrameCount;            // corresponds to current IAudioRecord, value is
                                                    // reported back by AudioFlinger to the client
    size_t                  mReqFrameCount;         // frame count to request the first or next time
                                                    // a new IAudioRecord is needed, non-decreasing

    int64_t                 mFramesRead;            // total frames read. reset to zero after
                                                    // the start() following stop(). It is not
                                                    // changed after restoring the track.
    int64_t                 mFramesReadServerOffset; // An offset to server frames read due to
                                                    // restoring AudioRecord, or stop/start.
    // constant after constructor or set()
    uint32_t                mSampleRate;
    audio_format_t          mFormat;
    uint32_t                mChannelCount;
    size_t                  mFrameSize;         // app-level frame size == AudioFlinger frame size
    uint32_t                mLatency;           // in ms
    audio_channel_mask_t    mChannelMask;

    audio_input_flags_t     mFlags;                 // same as mOrigFlags, except for bits that may
                                                    // be denied by client or server, such as
                                                    // AUDIO_INPUT_FLAG_FAST.  mLock must be
                                                    // held to read or write those bits reliably.
    audio_input_flags_t     mOrigFlags;             // as specified in constructor or set(), const

    audio_session_t         mSessionId;
    audio_port_handle_t     mPortId;                    // Id from Audio Policy Manager
    transfer_type           mTransfer;

    // Next 5 fields may be changed if IAudioRecord is re-created, but always != 0
    // provided the initial set() was successful
    sp<media::IAudioRecord> mAudioRecord;
    sp<IMemory>             mCblkMemory;
    audio_track_cblk_t*     mCblk;              // re-load after mLock.unlock()
    sp<IMemory>             mBufferMemory;
    audio_io_handle_t       mInput = AUDIO_IO_HANDLE_NONE; // from AudioSystem::getInputforAttr()

    int                     mPreviousPriority;  // before start()
    SchedPolicy             mPreviousSchedulingGroup;
    bool                    mAwaitBoost;    // thread should wait for priority boost before running

    // The proxy should only be referenced while a lock is held because the proxy isn't
    // multi-thread safe.
    // An exception is that a blocking ClientProxy::obtainBuffer() may be called without a lock,
    // provided that the caller also holds an extra reference to the proxy and shared memory to keep
    // them around in case they are replaced during the obtainBuffer().
    sp<AudioRecordClientProxy> mProxy;

    bool                    mInOverrun;         // whether recorder is currently in overrun state

private:
    class DeathNotifier : public IBinder::DeathRecipient {
    public:
        DeathNotifier(AudioRecord* audioRecord) : mAudioRecord(audioRecord) { }
    protected:
        virtual void        binderDied(const wp<IBinder>& who);
    private:
        const wp<AudioRecord> mAudioRecord;
    };

    sp<DeathNotifier>       mDeathNotifier;
    uint32_t                mSequence;              // incremented for each new IAudioRecord attempt
    uid_t                   mClientUid;
    pid_t                   mClientPid;
    audio_attributes_t      mAttributes;

    // For Device Selection API
    //  a value of AUDIO_PORT_HANDLE_NONE indicated default (AudioPolicyManager) routing.
    audio_port_handle_t     mSelectedDeviceId; // Device requested by the application.
    audio_port_handle_t     mRoutedDeviceId;   // Device actually selected by audio policy manager:
                                              // May not match the app selection depending on other
                                              // activity and connected devices
    wp<AudioSystem::AudioDeviceCallback> mDeviceCallback;

    audio_microphone_direction_t mSelectedMicDirection;
    float mSelectedMicFieldDimension;

private:
    class MediaMetrics {
      public:
        MediaMetrics() : mAnalyticsItem(MediaAnalyticsItem::create("audiorecord")),
                         mCreatedNs(systemTime(SYSTEM_TIME_REALTIME)),
                         mStartedNs(0), mDurationNs(0), mCount(0),
                         mLastError(NO_ERROR) {
        }
        ~MediaMetrics() {
            // mAnalyticsItem alloc failure will be flagged in the constructor
            // don't log empty records
            if (mAnalyticsItem->count() > 0) {
                mAnalyticsItem->selfrecord();
            }
        }
        void gather(const AudioRecord *record);
        MediaAnalyticsItem *dup() { return mAnalyticsItem->dup(); }

        void logStart(nsecs_t when) { mStartedNs = when; mCount++; }
        void logStop(nsecs_t when) { mDurationNs += (when-mStartedNs); mStartedNs = 0;}
        void markError(status_t errcode, const char *func)
                 { mLastError = errcode; mLastErrorFunc = func;}
      private:
        std::unique_ptr<MediaAnalyticsItem> mAnalyticsItem;
        nsecs_t mCreatedNs;     // XXX: perhaps not worth it in production
        nsecs_t mStartedNs;
        nsecs_t mDurationNs;
        int32_t mCount;

        status_t mLastError;
        std::string mLastErrorFunc;
    };
    MediaMetrics mMediaMetrics;
};

}; // namespace android

#endif // ANDROID_AUDIORECORD_H
