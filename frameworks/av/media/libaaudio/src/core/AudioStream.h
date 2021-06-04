/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef AAUDIO_AUDIOSTREAM_H
#define AAUDIO_AUDIOSTREAM_H

#include <atomic>
#include <mutex>
#include <stdint.h>
#include <aaudio/AAudio.h>
#include <binder/IServiceManager.h>
#include <binder/Status.h>
#include <utils/StrongPointer.h>

#include "media/VolumeShaper.h"
#include "media/PlayerBase.h"
#include "utility/AAudioUtilities.h"
#include "utility/MonotonicCounter.h"

// Cannot get android::media::VolumeShaper to compile!
#define AAUDIO_USE_VOLUME_SHAPER  0

namespace aaudio {

typedef void *(*aaudio_audio_thread_proc_t)(void *);
typedef uint32_t aaudio_stream_id_t;

class AudioStreamBuilder;

constexpr pid_t        CALLBACK_THREAD_NONE = 0;

/**
 * AAudio audio stream.
 */
class AudioStream {
public:

    AudioStream();

    virtual ~AudioStream();

protected:

    /* Asynchronous requests.
     * Use waitForStateChange() to wait for completion.
     */
    virtual aaudio_result_t requestStart() = 0;

    /**
     * Check the state to see if Pause is currently legal.
     *
     * @param result pointer to return code
     * @return true if OK to continue, if false then return result
     */
    bool checkPauseStateTransition(aaudio_result_t *result);

    virtual bool isFlushSupported() const {
        // Only implement FLUSH for OUTPUT streams.
        return false;
    }

    virtual bool isPauseSupported() const {
        // Only implement PAUSE for OUTPUT streams.
        return false;
    }

    virtual aaudio_result_t requestPause()
    {
        // Only implement this for OUTPUT streams.
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual aaudio_result_t requestFlush() {
        // Only implement this for OUTPUT streams.
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual aaudio_result_t requestStop() = 0;

public:
    virtual aaudio_result_t getTimestamp(clockid_t clockId,
                                       int64_t *framePosition,
                                       int64_t *timeNanoseconds) = 0;


    /**
     * Update state machine.()
     * @return
     */
    virtual aaudio_result_t updateStateMachine() = 0;

    // =========== End ABSTRACT methods ===========================

    virtual aaudio_result_t waitForStateChange(aaudio_stream_state_t currentState,
                                               aaudio_stream_state_t *nextState,
                                               int64_t timeoutNanoseconds);

    /**
     * Open the stream using the parameters in the builder.
     * Allocate the necessary resources.
     */
    virtual aaudio_result_t open(const AudioStreamBuilder& builder);

    /**
     * Close the stream and deallocate any resources from the open() call.
     * It is safe to call close() multiple times.
     */
    virtual aaudio_result_t close() {
        return AAUDIO_OK;
    }

    // This is only used to identify a stream in the logs without
    // revealing any pointers.
    aaudio_stream_id_t getId() {
        return mStreamId;
    }

    virtual aaudio_result_t setBufferSize(int32_t requestedFrames) = 0;

    virtual aaudio_result_t createThread(int64_t periodNanoseconds,
                                       aaudio_audio_thread_proc_t threadProc,
                                       void *threadArg);

    aaudio_result_t joinThread(void **returnArg, int64_t timeoutNanoseconds);

    virtual aaudio_result_t registerThread() {
        return AAUDIO_OK;
    }

    virtual aaudio_result_t unregisterThread() {
        return AAUDIO_OK;
    }

    /**
     * Internal function used to call the audio thread passed by the user.
     * It is unfortunately public because it needs to be called by a static 'C' function.
     */
    void* wrapUserThread();

    // ============== Queries ===========================

    aaudio_stream_state_t getState() const {
        return mState;
    }

    virtual int32_t getBufferSize() const {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual int32_t getBufferCapacity() const {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual int32_t getFramesPerBurst() const {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual int32_t getXRunCount() const {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    bool isActive() const {
        return mState == AAUDIO_STREAM_STATE_STARTING || mState == AAUDIO_STREAM_STATE_STARTED;
    }

    virtual bool isMMap() {
        return false;
    }

    aaudio_result_t getSampleRate() const {
        return mSampleRate;
    }

    audio_format_t getFormat()  const {
        return mFormat;
    }

    aaudio_result_t getSamplesPerFrame() const {
        return mSamplesPerFrame;
    }

    virtual int32_t getPerformanceMode() const {
        return mPerformanceMode;
    }

    void setPerformanceMode(aaudio_performance_mode_t performanceMode) {
        mPerformanceMode = performanceMode;
    }

    int32_t getDeviceId() const {
        return mDeviceId;
    }

    aaudio_sharing_mode_t getSharingMode() const {
        return mSharingMode;
    }

    bool isSharingModeMatchRequired() const {
        return mSharingModeMatchRequired;
    }

    virtual aaudio_direction_t getDirection() const = 0;

    aaudio_usage_t getUsage() const {
        return mUsage;
    }

    aaudio_content_type_t getContentType() const {
        return mContentType;
    }

    aaudio_input_preset_t getInputPreset() const {
        return mInputPreset;
    }

    aaudio_allowed_capture_policy_t getAllowedCapturePolicy() const {
        return mAllowedCapturePolicy;
    }

    int32_t getSessionId() const {
        return mSessionId;
    }

    /**
     * This is only valid after setSamplesPerFrame() and setFormat() have been called.
     */
    int32_t getBytesPerFrame() const {
        return mSamplesPerFrame * getBytesPerSample();
    }

    /**
     * This is only valid after setFormat() has been called.
     */
    int32_t getBytesPerSample() const {
        return audio_bytes_per_sample(mFormat);
    }

    /**
     * This is only valid after setSamplesPerFrame() and setDeviceFormat() have been called.
     */
    int32_t getBytesPerDeviceFrame() const {
        return getSamplesPerFrame() * audio_bytes_per_sample(getDeviceFormat());
    }

    virtual int64_t getFramesWritten() = 0;

    virtual int64_t getFramesRead() = 0;

    AAudioStream_dataCallback getDataCallbackProc() const {
        return mDataCallbackProc;
    }

    AAudioStream_errorCallback getErrorCallbackProc() const {
        return mErrorCallbackProc;
    }

    aaudio_data_callback_result_t maybeCallDataCallback(void *audioData, int32_t numFrames);

    void maybeCallErrorCallback(aaudio_result_t result);

    void *getDataCallbackUserData() const {
        return mDataCallbackUserData;
    }

    void *getErrorCallbackUserData() const {
        return mErrorCallbackUserData;
    }

    int32_t getFramesPerDataCallback() const {
        return mFramesPerDataCallback;
    }

    /**
     * @return true if data callback has been specified
     */
    bool isDataCallbackSet() const {
        return mDataCallbackProc != nullptr;
    }

    /**
     * @return true if data callback has been specified and stream is running
     */
    bool isDataCallbackActive() const {
        return isDataCallbackSet() && isActive();
    }

    /**
     * @return true if called from the same thread as the callback
     */
    bool collidesWithCallback() const;

    // ============== I/O ===========================
    // A Stream will only implement read() or write() depending on its direction.
    virtual aaudio_result_t write(const void *buffer __unused,
                             int32_t numFrames __unused,
                             int64_t timeoutNanoseconds __unused) {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    virtual aaudio_result_t read(void *buffer __unused,
                            int32_t numFrames __unused,
                            int64_t timeoutNanoseconds __unused) {
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    // This is used by the AudioManager to duck and mute the stream when changing audio focus.
    void setDuckAndMuteVolume(float duckAndMuteVolume);

    float getDuckAndMuteVolume() const {
        return mDuckAndMuteVolume;
    }

    // Implement this in the output subclasses.
    virtual android::status_t doSetVolume() { return android::NO_ERROR; }

#if AAUDIO_USE_VOLUME_SHAPER
    virtual ::android::binder::Status applyVolumeShaper(
            const ::android::media::VolumeShaper::Configuration& configuration __unused,
            const ::android::media::VolumeShaper::Operation& operation __unused);
#endif

    /**
     * Register this stream's PlayerBase with the AudioManager if needed.
     * Only register output streams.
     * This should only be called for client streams and not for streams
     * that run in the service.
     */
    void registerPlayerBase() {
        if (getDirection() == AAUDIO_DIRECTION_OUTPUT) {
            mPlayerBase->registerWithAudioManager();
        }
    }

    /**
     * Unregister this stream's PlayerBase with the AudioManager.
     * This will only unregister if already registered.
     */
    void unregisterPlayerBase() {
        mPlayerBase->unregisterWithAudioManager();
    }

    aaudio_result_t systemStart();

    aaudio_result_t systemPause();

    aaudio_result_t safeFlush();

    /**
     * This is called when an app calls AAudioStream_requestStop();
     * It prevents calls from a callback.
     */
    aaudio_result_t systemStopFromApp();

    /**
     * This is called internally when an app callback returns AAUDIO_CALLBACK_RESULT_STOP.
     */
    aaudio_result_t systemStopFromCallback();

    aaudio_result_t safeClose();

protected:

    // PlayerBase allows the system to control the stream volume.
    class MyPlayerBase : public android::PlayerBase {
    public:
        explicit MyPlayerBase(AudioStream *parent);

        virtual ~MyPlayerBase();

        /**
         * Register for volume changes and remote control.
         */
        void registerWithAudioManager();

        /**
         * UnRegister.
         */
        void unregisterWithAudioManager();

        /**
         * Just calls unregisterWithAudioManager().
         */
        void destroy() override;

        void clearParentReference() { mParent = nullptr; }

        // Just a stub. The ability to start audio through PlayerBase is being deprecated.
        android::status_t playerStart() override {
            return android::NO_ERROR;
        }

        // Just a stub. The ability to pause audio through PlayerBase is being deprecated.
        android::status_t playerPause() override {
            return android::NO_ERROR;
        }

        // Just a stub. The ability to stop audio through PlayerBase is being deprecated.
        android::status_t playerStop() override {
            return android::NO_ERROR;
        }

        android::status_t playerSetVolume() override {
            // No pan and only left volume is taken into account from IPLayer interface
            mParent->setDuckAndMuteVolume(mVolumeMultiplierL  /* * mPanMultiplierL */);
            return android::NO_ERROR;
        }

#if AAUDIO_USE_VOLUME_SHAPER
        ::android::binder::Status applyVolumeShaper(
                const ::android::media::VolumeShaper::Configuration& configuration,
                const ::android::media::VolumeShaper::Operation& operation) {
            return mParent->applyVolumeShaper(configuration, operation);
        }
#endif

        aaudio_result_t getResult() {
            return mResult;
        }

    private:
        AudioStream          *mParent;
        aaudio_result_t       mResult = AAUDIO_OK;
        bool                  mRegistered = false;
    };

    /**
     * This should not be called after the open() call.
     * TODO for multiple setters: assert(mState == AAUDIO_STREAM_STATE_UNINITIALIZED)
     */
    void setSampleRate(int32_t sampleRate) {
        mSampleRate = sampleRate;
    }

    /**
     * This should not be called after the open() call.
     */
    void setSamplesPerFrame(int32_t samplesPerFrame) {
        mSamplesPerFrame = samplesPerFrame;
    }

    /**
     * This should not be called after the open() call.
     */
    void setSharingMode(aaudio_sharing_mode_t sharingMode) {
        mSharingMode = sharingMode;
    }

    /**
     * This should not be called after the open() call.
     */
    void setFormat(audio_format_t format) {
        mFormat = format;
    }

    /**
     * This should not be called after the open() call.
     */
    void setDeviceFormat(audio_format_t format) {
        mDeviceFormat = format;
    }

    audio_format_t getDeviceFormat() const {
        return mDeviceFormat;
    }

    void setState(aaudio_stream_state_t state);

    void setDeviceId(int32_t deviceId) {
        mDeviceId = deviceId;
    }

    void setSessionId(int32_t sessionId) {
        mSessionId = sessionId;
    }

    std::atomic<bool>    mCallbackEnabled{false};

    float                mDuckAndMuteVolume = 1.0f;

protected:

    /**
     * Either convert the data from device format to app format and return a pointer
     * to the conversion buffer,
     * OR just pass back the original pointer.
     *
     * Note that this is only used for the INPUT path.
     *
     * @param audioData
     * @param numFrames
     * @return original pointer or the conversion buffer
     */
    virtual const void * maybeConvertDeviceData(const void *audioData, int32_t numFrames) {
        return audioData;
    }

    void setPeriodNanoseconds(int64_t periodNanoseconds) {
        mPeriodNanoseconds.store(periodNanoseconds, std::memory_order_release);
    }

    int64_t getPeriodNanoseconds() {
        return mPeriodNanoseconds.load(std::memory_order_acquire);
    }

    /**
     * This should not be called after the open() call.
     */
    void setUsage(aaudio_usage_t usage) {
        mUsage = usage;
    }

    /**
     * This should not be called after the open() call.
     */
    void setContentType(aaudio_content_type_t contentType) {
        mContentType = contentType;
    }

    /**
     * This should not be called after the open() call.
     */
    void setInputPreset(aaudio_input_preset_t inputPreset) {
        mInputPreset = inputPreset;
    }

    /**
     * This should not be called after the open() call.
     */
    void setAllowedCapturePolicy(aaudio_allowed_capture_policy_t policy) {
        mAllowedCapturePolicy = policy;
    }

private:

    aaudio_result_t safeStop();

    std::mutex                 mStreamLock;

    const android::sp<MyPlayerBase>   mPlayerBase;

    // These do not change after open().
    int32_t                     mSamplesPerFrame = AAUDIO_UNSPECIFIED;
    int32_t                     mSampleRate = AAUDIO_UNSPECIFIED;
    int32_t                     mDeviceId = AAUDIO_UNSPECIFIED;
    aaudio_sharing_mode_t       mSharingMode = AAUDIO_SHARING_MODE_SHARED;
    bool                        mSharingModeMatchRequired = false; // must match sharing mode requested
    audio_format_t              mFormat = AUDIO_FORMAT_DEFAULT;
    aaudio_stream_state_t       mState = AAUDIO_STREAM_STATE_UNINITIALIZED;
    aaudio_performance_mode_t   mPerformanceMode = AAUDIO_PERFORMANCE_MODE_NONE;

    aaudio_usage_t              mUsage           = AAUDIO_UNSPECIFIED;
    aaudio_content_type_t       mContentType     = AAUDIO_UNSPECIFIED;
    aaudio_input_preset_t       mInputPreset     = AAUDIO_UNSPECIFIED;
    aaudio_allowed_capture_policy_t mAllowedCapturePolicy = AAUDIO_ALLOW_CAPTURE_BY_ALL;

    int32_t                     mSessionId = AAUDIO_UNSPECIFIED;

    // Sometimes the hardware is operating with a different format from the app.
    // Then we require conversion in AAudio.
    audio_format_t              mDeviceFormat = AUDIO_FORMAT_INVALID;

    // callback ----------------------------------

    AAudioStream_dataCallback   mDataCallbackProc = nullptr;  // external callback functions
    void                       *mDataCallbackUserData = nullptr;
    int32_t                     mFramesPerDataCallback = AAUDIO_UNSPECIFIED; // frames
    std::atomic<pid_t>          mDataCallbackThread{CALLBACK_THREAD_NONE};

    AAudioStream_errorCallback  mErrorCallbackProc = nullptr;
    void                       *mErrorCallbackUserData = nullptr;
    std::atomic<pid_t>          mErrorCallbackThread{CALLBACK_THREAD_NONE};

    // background thread ----------------------------------
    bool                        mHasThread = false;
    pthread_t                   mThread; // initialized in constructor

    // These are set by the application thread and then read by the audio pthread.
    std::atomic<int64_t>        mPeriodNanoseconds; // for tuning SCHED_FIFO threads
    // TODO make atomic?
    aaudio_audio_thread_proc_t  mThreadProc = nullptr;
    void                       *mThreadArg = nullptr;
    aaudio_result_t             mThreadRegistrationResult = AAUDIO_OK;

    const aaudio_stream_id_t    mStreamId;

};

} /* namespace aaudio */

#endif /* AAUDIO_AUDIOSTREAM_H */
