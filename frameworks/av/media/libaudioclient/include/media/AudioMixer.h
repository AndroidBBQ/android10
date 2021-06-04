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

#ifndef ANDROID_AUDIO_MIXER_H
#define ANDROID_AUDIO_MIXER_H

#include <map>
#include <pthread.h>
#include <sstream>
#include <stdint.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

#include <android/os/IExternalVibratorService.h>
#include <media/AudioBufferProvider.h>
#include <media/AudioResampler.h>
#include <media/AudioResamplerPublic.h>
#include <media/BufferProviders.h>
#include <system/audio.h>
#include <utils/Compat.h>
#include <utils/threads.h>

// FIXME This is actually unity gain, which might not be max in future, expressed in U.12
#define MAX_GAIN_INT AudioMixer::UNITY_GAIN_INT

// This must match frameworks/av/services/audioflinger/Configuration.h
#define FLOAT_AUX

namespace android {

namespace NBLog {
class Writer;
}   // namespace NBLog

// ----------------------------------------------------------------------------

class AudioMixer
{
public:
    // Do not change these unless underlying code changes.
    // This mixer has a hard-coded upper limit of 8 channels for output.
    static constexpr uint32_t MAX_NUM_CHANNELS = FCC_8;
    static constexpr uint32_t MAX_NUM_VOLUMES = FCC_2; // stereo volume only
    // maximum number of channels supported for the content
    static const uint32_t MAX_NUM_CHANNELS_TO_DOWNMIX = AUDIO_CHANNEL_COUNT_MAX;

    static const uint16_t UNITY_GAIN_INT = 0x1000;
    static const CONSTEXPR float UNITY_GAIN_FLOAT = 1.0f;

    enum { // names
        // setParameter targets
        TRACK           = 0x3000,
        RESAMPLE        = 0x3001,
        RAMP_VOLUME     = 0x3002, // ramp to new volume
        VOLUME          = 0x3003, // don't ramp
        TIMESTRETCH     = 0x3004,

        // set Parameter names
        // for target TRACK
        CHANNEL_MASK    = 0x4000,
        FORMAT          = 0x4001,
        MAIN_BUFFER     = 0x4002,
        AUX_BUFFER      = 0x4003,
        DOWNMIX_TYPE    = 0X4004,
        MIXER_FORMAT    = 0x4005, // AUDIO_FORMAT_PCM_(FLOAT|16_BIT)
        MIXER_CHANNEL_MASK = 0x4006, // Channel mask for mixer output
        // for haptic
        HAPTIC_ENABLED  = 0x4007, // Set haptic data from this track should be played or not.
        HAPTIC_INTENSITY = 0x4008, // Set the intensity to play haptic data.
        // for target RESAMPLE
        SAMPLE_RATE     = 0x4100, // Configure sample rate conversion on this track name;
                                  // parameter 'value' is the new sample rate in Hz.
                                  // Only creates a sample rate converter the first time that
                                  // the track sample rate is different from the mix sample rate.
                                  // If the new sample rate is the same as the mix sample rate,
                                  // and a sample rate converter already exists,
                                  // then the sample rate converter remains present but is a no-op.
        RESET           = 0x4101, // Reset sample rate converter without changing sample rate.
                                  // This clears out the resampler's input buffer.
        REMOVE          = 0x4102, // Remove the sample rate converter on this track name;
                                  // the track is restored to the mix sample rate.
        // for target RAMP_VOLUME and VOLUME (8 channels max)
        // FIXME use float for these 3 to improve the dynamic range
        VOLUME0         = 0x4200,
        VOLUME1         = 0x4201,
        AUXLEVEL        = 0x4210,
        // for target TIMESTRETCH
        PLAYBACK_RATE   = 0x4300, // Configure timestretch on this track name;
                                  // parameter 'value' is a pointer to the new playback rate.
    };

    typedef enum { // Haptic intensity, should keep consistent with VibratorService
        HAPTIC_SCALE_MUTE = os::IExternalVibratorService::SCALE_MUTE,
        HAPTIC_SCALE_VERY_LOW = os::IExternalVibratorService::SCALE_VERY_LOW,
        HAPTIC_SCALE_LOW = os::IExternalVibratorService::SCALE_LOW,
        HAPTIC_SCALE_NONE = os::IExternalVibratorService::SCALE_NONE,
        HAPTIC_SCALE_HIGH = os::IExternalVibratorService::SCALE_HIGH,
        HAPTIC_SCALE_VERY_HIGH = os::IExternalVibratorService::SCALE_VERY_HIGH,
    } haptic_intensity_t;
    static constexpr float HAPTIC_SCALE_VERY_LOW_RATIO = 2.0f / 3.0f;
    static constexpr float HAPTIC_SCALE_LOW_RATIO = 3.0f / 4.0f;
    static const constexpr float HAPTIC_MAX_AMPLITUDE_FLOAT = 1.0f;

    static inline bool isValidHapticIntensity(haptic_intensity_t hapticIntensity) {
        switch (hapticIntensity) {
        case HAPTIC_SCALE_MUTE:
        case HAPTIC_SCALE_VERY_LOW:
        case HAPTIC_SCALE_LOW:
        case HAPTIC_SCALE_NONE:
        case HAPTIC_SCALE_HIGH:
        case HAPTIC_SCALE_VERY_HIGH:
            return true;
        default:
            return false;
        }
    }

    AudioMixer(size_t frameCount, uint32_t sampleRate)
        : mSampleRate(sampleRate)
        , mFrameCount(frameCount) {
        pthread_once(&sOnceControl, &sInitRoutine);
    }

    // Create a new track in the mixer.
    //
    // \param name        a unique user-provided integer associated with the track.
    //                    If name already exists, the function will abort.
    // \param channelMask output channel mask.
    // \param format      PCM format
    // \param sessionId   Session id for the track. Tracks with the same
    //                    session id will be submixed together.
    //
    // \return OK        on success.
    //         BAD_VALUE if the format does not satisfy isValidFormat()
    //                   or the channelMask does not satisfy isValidChannelMask().
    status_t    create(
            int name, audio_channel_mask_t channelMask, audio_format_t format, int sessionId);

    bool        exists(int name) const {
        return mTracks.count(name) > 0;
    }

    // Free an allocated track by name.
    void        destroy(int name);

    // Enable or disable an allocated track by name
    void        enable(int name);
    void        disable(int name);

    void        setParameter(int name, int target, int param, void *value);

    void        setBufferProvider(int name, AudioBufferProvider* bufferProvider);

    void        process() {
        for (const auto &pair : mTracks) {
            // Clear contracted buffer before processing if contracted channels are saved
            const std::shared_ptr<Track> &t = pair.second;
            if (t->mKeepContractedChannels) {
                t->clearContractedBuffer();
            }
        }
        (this->*mHook)();
        processHapticData();
    }

    size_t      getUnreleasedFrames(int name) const;

    std::string trackNames() const {
        std::stringstream ss;
        for (const auto &pair : mTracks) {
            ss << pair.first << " ";
        }
        return ss.str();
    }

    void        setNBLogWriter(NBLog::Writer *logWriter) {
        mNBLogWriter = logWriter;
    }

    static inline bool isValidFormat(audio_format_t format) {
        switch (format) {
        case AUDIO_FORMAT_PCM_8_BIT:
        case AUDIO_FORMAT_PCM_16_BIT:
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        case AUDIO_FORMAT_PCM_32_BIT:
        case AUDIO_FORMAT_PCM_FLOAT:
            return true;
        default:
            return false;
        }
    }

    static inline bool isValidChannelMask(audio_channel_mask_t channelMask) {
        return audio_channel_mask_is_valid(channelMask); // the RemixBufferProvider is flexible.
    }

private:

    /* For multi-format functions (calls template functions
     * in AudioMixerOps.h).  The template parameters are as follows:
     *
     *   MIXTYPE     (see AudioMixerOps.h MIXTYPE_* enumeration)
     *   USEFLOATVOL (set to true if float volume is used)
     *   ADJUSTVOL   (set to true if volume ramp parameters needs adjustment afterwards)
     *   TO: int32_t (Q4.27) or float
     *   TI: int32_t (Q4.27) or int16_t (Q0.15) or float
     *   TA: int32_t (Q4.27)
     */

    enum {
        // FIXME this representation permits up to 8 channels
        NEEDS_CHANNEL_COUNT__MASK   = 0x00000007,
    };

    enum {
        NEEDS_CHANNEL_1             = 0x00000000,   // mono
        NEEDS_CHANNEL_2             = 0x00000001,   // stereo

        // sample format is not explicitly specified, and is assumed to be AUDIO_FORMAT_PCM_16_BIT

        NEEDS_MUTE                  = 0x00000100,
        NEEDS_RESAMPLE              = 0x00001000,
        NEEDS_AUX                   = 0x00010000,
    };

    // hook types
    enum {
        PROCESSTYPE_NORESAMPLEONETRACK, // others set elsewhere
    };

    enum {
        TRACKTYPE_NOP,
        TRACKTYPE_RESAMPLE,
        TRACKTYPE_NORESAMPLE,
        TRACKTYPE_NORESAMPLEMONO,
    };

    // process hook functionality
    using process_hook_t = void(AudioMixer::*)();

    struct Track;
    using hook_t = void(Track::*)(int32_t* output, size_t numOutFrames, int32_t* temp, int32_t* aux);

    struct Track {
        Track()
            : bufferProvider(nullptr)
        {
            // TODO: move additional initialization here.
        }

        ~Track()
        {
            // bufferProvider, mInputBufferProvider need not be deleted.
            mResampler.reset(nullptr);
            // Ensure the order of destruction of buffer providers as they
            // release the upstream provider in the destructor.
            mTimestretchBufferProvider.reset(nullptr);
            mPostDownmixReformatBufferProvider.reset(nullptr);
            mDownmixerBufferProvider.reset(nullptr);
            mReformatBufferProvider.reset(nullptr);
            mContractChannelsNonDestructiveBufferProvider.reset(nullptr);
            mAdjustChannelsBufferProvider.reset(nullptr);
        }

        bool        needsRamp() { return (volumeInc[0] | volumeInc[1] | auxInc) != 0; }
        bool        setResampler(uint32_t trackSampleRate, uint32_t devSampleRate);
        bool        doesResample() const { return mResampler.get() != nullptr; }
        void        resetResampler() { if (mResampler.get() != nullptr) mResampler->reset(); }
        void        adjustVolumeRamp(bool aux, bool useFloat = false);
        size_t      getUnreleasedFrames() const { return mResampler.get() != nullptr ?
                                                    mResampler->getUnreleasedFrames() : 0; };

        status_t    prepareForDownmix();
        void        unprepareForDownmix();
        status_t    prepareForReformat();
        void        unprepareForReformat();
        status_t    prepareForAdjustChannels();
        void        unprepareForAdjustChannels();
        status_t    prepareForAdjustChannelsNonDestructive(size_t frames);
        void        unprepareForAdjustChannelsNonDestructive();
        void        clearContractedBuffer();
        bool        setPlaybackRate(const AudioPlaybackRate &playbackRate);
        void        reconfigureBufferProviders();

        static hook_t getTrackHook(int trackType, uint32_t channelCount,
                audio_format_t mixerInFormat, audio_format_t mixerOutFormat);

        void track__nop(int32_t* out, size_t numFrames, int32_t* temp, int32_t* aux);

        template <int MIXTYPE, bool USEFLOATVOL, bool ADJUSTVOL,
            typename TO, typename TI, typename TA>
        void volumeMix(TO *out, size_t outFrames, const TI *in, TA *aux, bool ramp);

        uint32_t    needs;

        // TODO: Eventually remove legacy integer volume settings
        union {
        int16_t     volume[MAX_NUM_VOLUMES]; // U4.12 fixed point (top bit should be zero)
        int32_t     volumeRL;
        };

        int32_t     prevVolume[MAX_NUM_VOLUMES];
        int32_t     volumeInc[MAX_NUM_VOLUMES];
        int32_t     auxInc;
        int32_t     prevAuxLevel;
        int16_t     auxLevel;       // 0 <= auxLevel <= MAX_GAIN_INT, but signed for mul performance

        uint16_t    frameCount;

        uint8_t     channelCount;   // 1 or 2, redundant with (needs & NEEDS_CHANNEL_COUNT__MASK)
        uint8_t     unused_padding; // formerly format, was always 16
        uint16_t    enabled;        // actually bool
        audio_channel_mask_t channelMask;

        // actual buffer provider used by the track hooks, see DownmixerBufferProvider below
        //  for how the Track buffer provider is wrapped by another one when dowmixing is required
        AudioBufferProvider*                bufferProvider;

        mutable AudioBufferProvider::Buffer buffer; // 8 bytes

        hook_t      hook;
        const void  *mIn;             // current location in buffer

        std::unique_ptr<AudioResampler> mResampler;
        uint32_t            sampleRate;
        int32_t*           mainBuffer;
        int32_t*           auxBuffer;

        /* Buffer providers are constructed to translate the track input data as needed.
         *
         * TODO: perhaps make a single PlaybackConverterProvider class to move
         * all pre-mixer track buffer conversions outside the AudioMixer class.
         *
         * 1) mInputBufferProvider: The AudioTrack buffer provider.
         * 2) mAdjustChannelsBufferProvider: Expands or contracts sample data from one interleaved
         *    channel format to another. Expanded channels are filled with zeros and put at the end
         *    of each audio frame. Contracted channels are copied to the end of the buffer.
         * 3) mContractChannelsNonDestructiveBufferProvider: Non-destructively contract sample data.
         *    This is currently using at audio-haptic coupled playback to separate audio and haptic
         *    data. Contracted channels could be written to given buffer.
         * 4) mReformatBufferProvider: If not NULL, performs the audio reformat to
         *    match either mMixerInFormat or mDownmixRequiresFormat, if the downmixer
         *    requires reformat. For example, it may convert floating point input to
         *    PCM_16_bit if that's required by the downmixer.
         * 5) mDownmixerBufferProvider: If not NULL, performs the channel remixing to match
         *    the number of channels required by the mixer sink.
         * 6) mPostDownmixReformatBufferProvider: If not NULL, performs reformatting from
         *    the downmixer requirements to the mixer engine input requirements.
         * 7) mTimestretchBufferProvider: Adds timestretching for playback rate
         */
        AudioBufferProvider*     mInputBufferProvider;    // externally provided buffer provider.
        // TODO: combine mAdjustChannelsBufferProvider and
        // mContractChannelsNonDestructiveBufferProvider
        std::unique_ptr<PassthruBufferProvider> mAdjustChannelsBufferProvider;
        std::unique_ptr<PassthruBufferProvider> mContractChannelsNonDestructiveBufferProvider;
        std::unique_ptr<PassthruBufferProvider> mReformatBufferProvider;
        std::unique_ptr<PassthruBufferProvider> mDownmixerBufferProvider;
        std::unique_ptr<PassthruBufferProvider> mPostDownmixReformatBufferProvider;
        std::unique_ptr<PassthruBufferProvider> mTimestretchBufferProvider;

        int32_t     sessionId;

        audio_format_t mMixerFormat;     // output mix format: AUDIO_FORMAT_PCM_(FLOAT|16_BIT)
        audio_format_t mFormat;          // input track format
        audio_format_t mMixerInFormat;   // mix internal format AUDIO_FORMAT_PCM_(FLOAT|16_BIT)
                                         // each track must be converted to this format.
        audio_format_t mDownmixRequiresFormat;  // required downmixer format
                                                // AUDIO_FORMAT_PCM_16_BIT if 16 bit necessary
                                                // AUDIO_FORMAT_INVALID if no required format

        float          mVolume[MAX_NUM_VOLUMES];     // floating point set volume
        float          mPrevVolume[MAX_NUM_VOLUMES]; // floating point previous volume
        float          mVolumeInc[MAX_NUM_VOLUMES];  // floating point volume increment

        float          mAuxLevel;                     // floating point set aux level
        float          mPrevAuxLevel;                 // floating point prev aux level
        float          mAuxInc;                       // floating point aux increment

        audio_channel_mask_t mMixerChannelMask;
        uint32_t             mMixerChannelCount;

        AudioPlaybackRate    mPlaybackRate;

        // Haptic
        bool                 mHapticPlaybackEnabled;
        haptic_intensity_t   mHapticIntensity;
        audio_channel_mask_t mHapticChannelMask;
        uint32_t             mHapticChannelCount;
        audio_channel_mask_t mMixerHapticChannelMask;
        uint32_t             mMixerHapticChannelCount;
        uint32_t             mAdjustInChannelCount;
        uint32_t             mAdjustOutChannelCount;
        uint32_t             mAdjustNonDestructiveInChannelCount;
        uint32_t             mAdjustNonDestructiveOutChannelCount;
        bool                 mKeepContractedChannels;

        float getHapticScaleGamma() const {
        // Need to keep consistent with the value in VibratorService.
        switch (mHapticIntensity) {
        case HAPTIC_SCALE_VERY_LOW:
            return 2.0f;
        case HAPTIC_SCALE_LOW:
            return 1.5f;
        case HAPTIC_SCALE_HIGH:
            return 0.5f;
        case HAPTIC_SCALE_VERY_HIGH:
            return 0.25f;
        default:
            return 1.0f;
        }
        }

        float getHapticMaxAmplitudeRatio() const {
        // Need to keep consistent with the value in VibratorService.
        switch (mHapticIntensity) {
        case HAPTIC_SCALE_VERY_LOW:
            return HAPTIC_SCALE_VERY_LOW_RATIO;
        case HAPTIC_SCALE_LOW:
            return HAPTIC_SCALE_LOW_RATIO;
        case HAPTIC_SCALE_NONE:
        case HAPTIC_SCALE_HIGH:
        case HAPTIC_SCALE_VERY_HIGH:
            return 1.0f;
        default:
            return 0.0f;
        }
        }

    private:
        // hooks
        void track__genericResample(int32_t* out, size_t numFrames, int32_t* temp, int32_t* aux);
        void track__16BitsStereo(int32_t* out, size_t numFrames, int32_t* temp, int32_t* aux);
        void track__16BitsMono(int32_t* out, size_t numFrames, int32_t* temp, int32_t* aux);

        void volumeRampStereo(int32_t* out, size_t frameCount, int32_t* temp, int32_t* aux);
        void volumeStereo(int32_t* out, size_t frameCount, int32_t* temp, int32_t* aux);

        // multi-format track hooks
        template <int MIXTYPE, typename TO, typename TI, typename TA>
        void track__Resample(TO* out, size_t frameCount, TO* temp __unused, TA* aux);
        template <int MIXTYPE, typename TO, typename TI, typename TA>
        void track__NoResample(TO* out, size_t frameCount, TO* temp __unused, TA* aux);
    };

    // TODO: remove BLOCKSIZE unit of processing - it isn't needed anymore.
    static constexpr int BLOCKSIZE = 16;

    bool setChannelMasks(int name,
            audio_channel_mask_t trackChannelMask, audio_channel_mask_t mixerChannelMask);

    // Called when track info changes and a new process hook should be determined.
    void invalidate() {
        mHook = &AudioMixer::process__validate;
    }

    void process__validate();
    void process__nop();
    void process__genericNoResampling();
    void process__genericResampling();
    void process__oneTrack16BitsStereoNoResampling();

    template <int MIXTYPE, typename TO, typename TI, typename TA>
    void process__noResampleOneTrack();

    void processHapticData();

    static process_hook_t getProcessHook(int processType, uint32_t channelCount,
            audio_format_t mixerInFormat, audio_format_t mixerOutFormat);

    static void convertMixerFormat(void *out, audio_format_t mixerOutFormat,
            void *in, audio_format_t mixerInFormat, size_t sampleCount);

    static void sInitRoutine();

    // initialization constants
    const uint32_t mSampleRate;
    const size_t mFrameCount;

    NBLog::Writer *mNBLogWriter = nullptr;   // associated NBLog::Writer

    process_hook_t mHook = &AudioMixer::process__nop;   // one of process__*, never nullptr

    // the size of the type (int32_t) should be the largest of all types supported
    // by the mixer.
    std::unique_ptr<int32_t[]> mOutputTemp;
    std::unique_ptr<int32_t[]> mResampleTemp;

    // track names grouped by main buffer, in no particular order of main buffer.
    // however names for a particular main buffer are in order (by construction).
    std::unordered_map<void * /* mainBuffer */, std::vector<int /* name */>> mGroups;

    // track names that are enabled, in increasing order (by construction).
    std::vector<int /* name */> mEnabled;

    // track smart pointers, by name, in increasing order of name.
    std::map<int /* name */, std::shared_ptr<Track>> mTracks;

    static pthread_once_t sOnceControl; // initialized in constructor by first new
};

// ----------------------------------------------------------------------------
} // namespace android

#endif // ANDROID_AUDIO_MIXER_H
