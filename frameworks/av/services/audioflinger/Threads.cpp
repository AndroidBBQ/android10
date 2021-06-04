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
#define ATRACE_TAG ATRACE_TAG_AUDIO

#include "Configuration.h"
#include <math.h>
#include <fcntl.h>
#include <memory>
#include <string>
#include <linux/futex.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <cutils/properties.h>
#include <media/AudioParameter.h>
#include <media/AudioResamplerPublic.h>
#include <media/RecordBufferConverter.h>
#include <media/TypeConverter.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <private/media/AudioTrackShared.h>
#include <private/android_filesystem_config.h>
#include <audio_utils/Balance.h>
#include <audio_utils/channels.h>
#include <audio_utils/mono_blend.h>
#include <audio_utils/primitives.h>
#include <audio_utils/format.h>
#include <audio_utils/minifloat.h>
#include <audio_utils/safe_math.h>
#include <system/audio_effects/effect_ns.h>
#include <system/audio_effects/effect_aec.h>
#include <system/audio.h>

// NBAIO implementations
#include <media/nbaio/AudioStreamInSource.h>
#include <media/nbaio/AudioStreamOutSink.h>
#include <media/nbaio/MonoPipe.h>
#include <media/nbaio/MonoPipeReader.h>
#include <media/nbaio/Pipe.h>
#include <media/nbaio/PipeReader.h>
#include <media/nbaio/SourceAudioBufferProvider.h>
#include <mediautils/BatteryNotifier.h>

#include <audiomanager/AudioManager.h>
#include <powermanager/PowerManager.h>

#include <media/audiohal/EffectsFactoryHalInterface.h>
#include <media/audiohal/StreamHalInterface.h>

#include "AudioFlinger.h"
#include "FastMixer.h"
#include "FastCapture.h"
#include <mediautils/SchedulingPolicyService.h>
#include <mediautils/ServiceUtilities.h>

#ifdef ADD_BATTERY_DATA
#include <media/IMediaPlayerService.h>
#include <media/IMediaDeathNotifier.h>
#endif

#ifdef DEBUG_CPU_USAGE
#include <audio_utils/Statistics.h>
#include <cpustats/ThreadCpuUsage.h>
#endif

#include "AutoPark.h"

#include <pthread.h>
#include "TypedLogger.h"

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

// TODO: Move these macro/inlines to a header file.
#define max(a, b) ((a) > (b) ? (a) : (b))
template <typename T>
static inline T min(const T& a, const T& b)
{
    return a < b ? a : b;
}

namespace android {

// retry counts for buffer fill timeout
// 50 * ~20msecs = 1 second
static const int8_t kMaxTrackRetries = 50;
static const int8_t kMaxTrackStartupRetries = 50;
// allow less retry attempts on direct output thread.
// direct outputs can be a scarce resource in audio hardware and should
// be released as quickly as possible.
static const int8_t kMaxTrackRetriesDirect = 2;



// don't warn about blocked writes or record buffer overflows more often than this
static const nsecs_t kWarningThrottleNs = seconds(5);

// RecordThread loop sleep time upon application overrun or audio HAL read error
static const int kRecordThreadSleepUs = 5000;

// maximum time to wait in sendConfigEvent_l() for a status to be received
static const nsecs_t kConfigEventTimeoutNs = seconds(2);

// minimum sleep time for the mixer thread loop when tracks are active but in underrun
static const uint32_t kMinThreadSleepTimeUs = 5000;
// maximum divider applied to the active sleep time in the mixer thread loop
static const uint32_t kMaxThreadSleepTimeShift = 2;

// minimum normal sink buffer size, expressed in milliseconds rather than frames
// FIXME This should be based on experimentally observed scheduling jitter
static const uint32_t kMinNormalSinkBufferSizeMs = 20;
// maximum normal sink buffer size
static const uint32_t kMaxNormalSinkBufferSizeMs = 24;

// minimum capture buffer size in milliseconds to _not_ need a fast capture thread
// FIXME This should be based on experimentally observed scheduling jitter
static const uint32_t kMinNormalCaptureBufferSizeMs = 12;

// Offloaded output thread standby delay: allows track transition without going to standby
static const nsecs_t kOffloadStandbyDelayNs = seconds(1);

// Direct output thread minimum sleep time in idle or active(underrun) state
static const nsecs_t kDirectMinSleepTimeUs = 10000;

// The universal constant for ubiquitous 20ms value. The value of 20ms seems to provide a good
// balance between power consumption and latency, and allows threads to be scheduled reliably
// by the CFS scheduler.
// FIXME Express other hardcoded references to 20ms with references to this constant and move
// it appropriately.
#define FMS_20 20

// Whether to use fast mixer
static const enum {
    FastMixer_Never,    // never initialize or use: for debugging only
    FastMixer_Always,   // always initialize and use, even if not needed: for debugging only
                        // normal mixer multiplier is 1
    FastMixer_Static,   // initialize if needed, then use all the time if initialized,
                        // multiplier is calculated based on min & max normal mixer buffer size
    FastMixer_Dynamic,  // initialize if needed, then use dynamically depending on track load,
                        // multiplier is calculated based on min & max normal mixer buffer size
    // FIXME for FastMixer_Dynamic:
    //  Supporting this option will require fixing HALs that can't handle large writes.
    //  For example, one HAL implementation returns an error from a large write,
    //  and another HAL implementation corrupts memory, possibly in the sample rate converter.
    //  We could either fix the HAL implementations, or provide a wrapper that breaks
    //  up large writes into smaller ones, and the wrapper would need to deal with scheduler.
} kUseFastMixer = FastMixer_Static;

// Whether to use fast capture
static const enum {
    FastCapture_Never,  // never initialize or use: for debugging only
    FastCapture_Always, // always initialize and use, even if not needed: for debugging only
    FastCapture_Static, // initialize if needed, then use all the time if initialized
} kUseFastCapture = FastCapture_Static;

// Priorities for requestPriority
static const int kPriorityAudioApp = 2;
static const int kPriorityFastMixer = 3;
static const int kPriorityFastCapture = 3;

// IAudioFlinger::createTrack() has an in/out parameter 'pFrameCount' for the total size of the
// track buffer in shared memory.  Zero on input means to use a default value.  For fast tracks,
// AudioFlinger derives the default from HAL buffer size and 'fast track multiplier'.

// This is the default value, if not specified by property.
static const int kFastTrackMultiplier = 2;

// The minimum and maximum allowed values
static const int kFastTrackMultiplierMin = 1;
static const int kFastTrackMultiplierMax = 2;

// The actual value to use, which can be specified per-device via property af.fast_track_multiplier.
static int sFastTrackMultiplier = kFastTrackMultiplier;

// See Thread::readOnlyHeap().
// Initially this heap is used to allocate client buffers for "fast" AudioRecord.
// Eventually it will be the single buffer that FastCapture writes into via HAL read(),
// and that all "fast" AudioRecord clients read from.  In either case, the size can be small.
static const size_t kRecordThreadReadOnlyHeapSize = 0xD000;

// ----------------------------------------------------------------------------

static pthread_once_t sFastTrackMultiplierOnce = PTHREAD_ONCE_INIT;

static void sFastTrackMultiplierInit()
{
    char value[PROPERTY_VALUE_MAX];
    if (property_get("af.fast_track_multiplier", value, NULL) > 0) {
        char *endptr;
        unsigned long ul = strtoul(value, &endptr, 0);
        if (*endptr == '\0' && kFastTrackMultiplierMin <= ul && ul <= kFastTrackMultiplierMax) {
            sFastTrackMultiplier = (int) ul;
        }
    }
}

// ----------------------------------------------------------------------------

#ifdef ADD_BATTERY_DATA
// To collect the amplifier usage
static void addBatteryData(uint32_t params) {
    sp<IMediaPlayerService> service = IMediaDeathNotifier::getMediaPlayerService();
    if (service == NULL) {
        // it already logged
        return;
    }

    service->addBatteryData(params);
}
#endif

// Track the CLOCK_BOOTTIME versus CLOCK_MONOTONIC timebase offset
struct {
    // call when you acquire a partial wakelock
    void acquire(const sp<IBinder> &wakeLockToken) {
        pthread_mutex_lock(&mLock);
        if (wakeLockToken.get() == nullptr) {
            adjustTimebaseOffset(&mBoottimeOffset, ExtendedTimestamp::TIMEBASE_BOOTTIME);
        } else {
            if (mCount == 0) {
                adjustTimebaseOffset(&mBoottimeOffset, ExtendedTimestamp::TIMEBASE_BOOTTIME);
            }
            ++mCount;
        }
        pthread_mutex_unlock(&mLock);
    }

    // call when you release a partial wakelock.
    void release(const sp<IBinder> &wakeLockToken) {
        if (wakeLockToken.get() == nullptr) {
            return;
        }
        pthread_mutex_lock(&mLock);
        if (--mCount < 0) {
            ALOGE("negative wakelock count");
            mCount = 0;
        }
        pthread_mutex_unlock(&mLock);
    }

    // retrieves the boottime timebase offset from monotonic.
    int64_t getBoottimeOffset() {
        pthread_mutex_lock(&mLock);
        int64_t boottimeOffset = mBoottimeOffset;
        pthread_mutex_unlock(&mLock);
        return boottimeOffset;
    }

    // Adjusts the timebase offset between TIMEBASE_MONOTONIC
    // and the selected timebase.
    // Currently only TIMEBASE_BOOTTIME is allowed.
    //
    // This only needs to be called upon acquiring the first partial wakelock
    // after all other partial wakelocks are released.
    //
    // We do an empirical measurement of the offset rather than parsing
    // /proc/timer_list since the latter is not a formal kernel ABI.
    static void adjustTimebaseOffset(int64_t *offset, ExtendedTimestamp::Timebase timebase) {
        int clockbase;
        switch (timebase) {
        case ExtendedTimestamp::TIMEBASE_BOOTTIME:
            clockbase = SYSTEM_TIME_BOOTTIME;
            break;
        default:
            LOG_ALWAYS_FATAL("invalid timebase %d", timebase);
            break;
        }
        // try three times to get the clock offset, choose the one
        // with the minimum gap in measurements.
        const int tries = 3;
        nsecs_t bestGap, measured;
        for (int i = 0; i < tries; ++i) {
            const nsecs_t tmono = systemTime(SYSTEM_TIME_MONOTONIC);
            const nsecs_t tbase = systemTime(clockbase);
            const nsecs_t tmono2 = systemTime(SYSTEM_TIME_MONOTONIC);
            const nsecs_t gap = tmono2 - tmono;
            if (i == 0 || gap < bestGap) {
                bestGap = gap;
                measured = tbase - ((tmono + tmono2) >> 1);
            }
        }

        // to avoid micro-adjusting, we don't change the timebase
        // unless it is significantly different.
        //
        // Assumption: It probably takes more than toleranceNs to
        // suspend and resume the device.
        static int64_t toleranceNs = 10000; // 10 us
        if (llabs(*offset - measured) > toleranceNs) {
            ALOGV("Adjusting timebase offset old: %lld  new: %lld",
                    (long long)*offset, (long long)measured);
            *offset = measured;
        }
    }

    pthread_mutex_t mLock;
    int32_t mCount;
    int64_t mBoottimeOffset;
} gBoottime = { PTHREAD_MUTEX_INITIALIZER, 0, 0 }; // static, so use POD initialization

// ----------------------------------------------------------------------------
//      CPU Stats
// ----------------------------------------------------------------------------

class CpuStats {
public:
    CpuStats();
    void sample(const String8 &title);
#ifdef DEBUG_CPU_USAGE
private:
    ThreadCpuUsage mCpuUsage;           // instantaneous thread CPU usage in wall clock ns
    audio_utils::Statistics<double> mWcStats; // statistics on thread CPU usage in wall clock ns

    audio_utils::Statistics<double> mHzStats; // statistics on thread CPU usage in cycles

    int mCpuNum;                        // thread's current CPU number
    int mCpukHz;                        // frequency of thread's current CPU in kHz
#endif
};

CpuStats::CpuStats()
#ifdef DEBUG_CPU_USAGE
    : mCpuNum(-1), mCpukHz(-1)
#endif
{
}

void CpuStats::sample(const String8 &title
#ifndef DEBUG_CPU_USAGE
                __unused
#endif
        ) {
#ifdef DEBUG_CPU_USAGE
    // get current thread's delta CPU time in wall clock ns
    double wcNs;
    bool valid = mCpuUsage.sampleAndEnable(wcNs);

    // record sample for wall clock statistics
    if (valid) {
        mWcStats.add(wcNs);
    }

    // get the current CPU number
    int cpuNum = sched_getcpu();

    // get the current CPU frequency in kHz
    int cpukHz = mCpuUsage.getCpukHz(cpuNum);

    // check if either CPU number or frequency changed
    if (cpuNum != mCpuNum || cpukHz != mCpukHz) {
        mCpuNum = cpuNum;
        mCpukHz = cpukHz;
        // ignore sample for purposes of cycles
        valid = false;
    }

    // if no change in CPU number or frequency, then record sample for cycle statistics
    if (valid && mCpukHz > 0) {
        const double cycles = wcNs * cpukHz * 0.000001;
        mHzStats.add(cycles);
    }

    const unsigned n = mWcStats.getN();
    // mCpuUsage.elapsed() is expensive, so don't call it every loop
    if ((n & 127) == 1) {
        const long long elapsed = mCpuUsage.elapsed();
        if (elapsed >= DEBUG_CPU_USAGE * 1000000000LL) {
            const double perLoop = elapsed / (double) n;
            const double perLoop100 = perLoop * 0.01;
            const double perLoop1k = perLoop * 0.001;
            const double mean = mWcStats.getMean();
            const double stddev = mWcStats.getStdDev();
            const double minimum = mWcStats.getMin();
            const double maximum = mWcStats.getMax();
            const double meanCycles = mHzStats.getMean();
            const double stddevCycles = mHzStats.getStdDev();
            const double minCycles = mHzStats.getMin();
            const double maxCycles = mHzStats.getMax();
            mCpuUsage.resetElapsed();
            mWcStats.reset();
            mHzStats.reset();
            ALOGD("CPU usage for %s over past %.1f secs\n"
                "  (%u mixer loops at %.1f mean ms per loop):\n"
                "  us per mix loop: mean=%.0f stddev=%.0f min=%.0f max=%.0f\n"
                "  %% of wall: mean=%.1f stddev=%.1f min=%.1f max=%.1f\n"
                "  MHz: mean=%.1f, stddev=%.1f, min=%.1f max=%.1f",
                    title.string(),
                    elapsed * .000000001, n, perLoop * .000001,
                    mean * .001,
                    stddev * .001,
                    minimum * .001,
                    maximum * .001,
                    mean / perLoop100,
                    stddev / perLoop100,
                    minimum / perLoop100,
                    maximum / perLoop100,
                    meanCycles / perLoop1k,
                    stddevCycles / perLoop1k,
                    minCycles / perLoop1k,
                    maxCycles / perLoop1k);

        }
    }
#endif
};

// ----------------------------------------------------------------------------
//      ThreadBase
// ----------------------------------------------------------------------------

// static
const char *AudioFlinger::ThreadBase::threadTypeToString(AudioFlinger::ThreadBase::type_t type)
{
    switch (type) {
    case MIXER:
        return "MIXER";
    case DIRECT:
        return "DIRECT";
    case DUPLICATING:
        return "DUPLICATING";
    case RECORD:
        return "RECORD";
    case OFFLOAD:
        return "OFFLOAD";
    case MMAP:
        return "MMAP";
    default:
        return "unknown";
    }
}

AudioFlinger::ThreadBase::ThreadBase(const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
        audio_devices_t outDevice, audio_devices_t inDevice, type_t type, bool systemReady)
    :   Thread(false /*canCallJava*/),
        mType(type),
        mAudioFlinger(audioFlinger),
        // mSampleRate, mFrameCount, mChannelMask, mChannelCount, mFrameSize, mFormat, mBufferSize
        // are set by PlaybackThread::readOutputParameters_l() or
        // RecordThread::readInputParameters_l()
        //FIXME: mStandby should be true here. Is this some kind of hack?
        mStandby(false), mOutDevice(outDevice), mInDevice(inDevice),
        mPrevOutDevice(AUDIO_DEVICE_NONE), mPrevInDevice(AUDIO_DEVICE_NONE),
        mAudioSource(AUDIO_SOURCE_DEFAULT), mId(id),
        // mName will be set by concrete (non-virtual) subclass
        mDeathRecipient(new PMDeathRecipient(this)),
        mSystemReady(systemReady),
        mSignalPending(false)
{
    memset(&mPatch, 0, sizeof(struct audio_patch));
}

AudioFlinger::ThreadBase::~ThreadBase()
{
    // mConfigEvents should be empty, but just in case it isn't, free the memory it owns
    mConfigEvents.clear();

    // do not lock the mutex in destructor
    releaseWakeLock_l();
    if (mPowerManager != 0) {
        sp<IBinder> binder = IInterface::asBinder(mPowerManager);
        binder->unlinkToDeath(mDeathRecipient);
    }

    sendStatistics(true /* force */);
}

status_t AudioFlinger::ThreadBase::readyToRun()
{
    status_t status = initCheck();
    if (status == NO_ERROR) {
        ALOGI("AudioFlinger's thread %p tid=%d ready to run", this, getTid());
    } else {
        ALOGE("No working audio driver found.");
    }
    return status;
}

void AudioFlinger::ThreadBase::exit()
{
    ALOGV("ThreadBase::exit");
    // do any cleanup required for exit to succeed
    preExit();
    {
        // This lock prevents the following race in thread (uniprocessor for illustration):
        //  if (!exitPending()) {
        //      // context switch from here to exit()
        //      // exit() calls requestExit(), what exitPending() observes
        //      // exit() calls signal(), which is dropped since no waiters
        //      // context switch back from exit() to here
        //      mWaitWorkCV.wait(...);
        //      // now thread is hung
        //  }
        AutoMutex lock(mLock);
        requestExit();
        mWaitWorkCV.broadcast();
    }
    // When Thread::requestExitAndWait is made virtual and this method is renamed to
    // "virtual status_t requestExitAndWait()", replace by "return Thread::requestExitAndWait();"
    requestExitAndWait();
}

status_t AudioFlinger::ThreadBase::setParameters(const String8& keyValuePairs)
{
    ALOGV("ThreadBase::setParameters() %s", keyValuePairs.string());
    Mutex::Autolock _l(mLock);

    return sendSetParameterConfigEvent_l(keyValuePairs);
}

// sendConfigEvent_l() must be called with ThreadBase::mLock held
// Can temporarily release the lock if waiting for a reply from processConfigEvents_l().
status_t AudioFlinger::ThreadBase::sendConfigEvent_l(sp<ConfigEvent>& event)
{
    status_t status = NO_ERROR;

    if (event->mRequiresSystemReady && !mSystemReady) {
        event->mWaitStatus = false;
        mPendingConfigEvents.add(event);
        return status;
    }
    mConfigEvents.add(event);
    ALOGV("sendConfigEvent_l() num events %zu event %d", mConfigEvents.size(), event->mType);
    mWaitWorkCV.signal();
    mLock.unlock();
    {
        Mutex::Autolock _l(event->mLock);
        while (event->mWaitStatus) {
            if (event->mCond.waitRelative(event->mLock, kConfigEventTimeoutNs) != NO_ERROR) {
                event->mStatus = TIMED_OUT;
                event->mWaitStatus = false;
            }
        }
        status = event->mStatus;
    }
    mLock.lock();
    return status;
}

void AudioFlinger::ThreadBase::sendIoConfigEvent(audio_io_config_event event, pid_t pid,
                                                 audio_port_handle_t portId)
{
    Mutex::Autolock _l(mLock);
    sendIoConfigEvent_l(event, pid, portId);
}

// sendIoConfigEvent_l() must be called with ThreadBase::mLock held
void AudioFlinger::ThreadBase::sendIoConfigEvent_l(audio_io_config_event event, pid_t pid,
                                                   audio_port_handle_t portId)
{
    // The audio statistics history is exponentially weighted to forget events
    // about five or more seconds in the past.  In order to have
    // crisper statistics for mediametrics, we reset the statistics on
    // an IoConfigEvent, to reflect different properties for a new device.
    mIoJitterMs.reset();
    mLatencyMs.reset();
    mProcessTimeMs.reset();
    mTimestampVerifier.discontinuity();

    sp<ConfigEvent> configEvent = (ConfigEvent *)new IoConfigEvent(event, pid, portId);
    sendConfigEvent_l(configEvent);
}

void AudioFlinger::ThreadBase::sendPrioConfigEvent(pid_t pid, pid_t tid, int32_t prio, bool forApp)
{
    Mutex::Autolock _l(mLock);
    sendPrioConfigEvent_l(pid, tid, prio, forApp);
}

// sendPrioConfigEvent_l() must be called with ThreadBase::mLock held
void AudioFlinger::ThreadBase::sendPrioConfigEvent_l(
        pid_t pid, pid_t tid, int32_t prio, bool forApp)
{
    sp<ConfigEvent> configEvent = (ConfigEvent *)new PrioConfigEvent(pid, tid, prio, forApp);
    sendConfigEvent_l(configEvent);
}

// sendSetParameterConfigEvent_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::ThreadBase::sendSetParameterConfigEvent_l(const String8& keyValuePair)
{
    sp<ConfigEvent> configEvent;
    AudioParameter param(keyValuePair);
    int value;
    if (param.getInt(String8(AudioParameter::keyMonoOutput), value) == NO_ERROR) {
        setMasterMono_l(value != 0);
        if (param.size() == 1) {
            return NO_ERROR; // should be a solo parameter - we don't pass down
        }
        param.remove(String8(AudioParameter::keyMonoOutput));
        configEvent = new SetParameterConfigEvent(param.toString());
    } else {
        configEvent = new SetParameterConfigEvent(keyValuePair);
    }
    return sendConfigEvent_l(configEvent);
}

status_t AudioFlinger::ThreadBase::sendCreateAudioPatchConfigEvent(
                                                        const struct audio_patch *patch,
                                                        audio_patch_handle_t *handle)
{
    Mutex::Autolock _l(mLock);
    sp<ConfigEvent> configEvent = (ConfigEvent *)new CreateAudioPatchConfigEvent(*patch, *handle);
    status_t status = sendConfigEvent_l(configEvent);
    if (status == NO_ERROR) {
        CreateAudioPatchConfigEventData *data =
                                        (CreateAudioPatchConfigEventData *)configEvent->mData.get();
        *handle = data->mHandle;
    }
    return status;
}

status_t AudioFlinger::ThreadBase::sendReleaseAudioPatchConfigEvent(
                                                                const audio_patch_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    sp<ConfigEvent> configEvent = (ConfigEvent *)new ReleaseAudioPatchConfigEvent(handle);
    return sendConfigEvent_l(configEvent);
}


// post condition: mConfigEvents.isEmpty()
void AudioFlinger::ThreadBase::processConfigEvents_l()
{
    bool configChanged = false;

    while (!mConfigEvents.isEmpty()) {
        ALOGV("processConfigEvents_l() remaining events %zu", mConfigEvents.size());
        sp<ConfigEvent> event = mConfigEvents[0];
        mConfigEvents.removeAt(0);
        switch (event->mType) {
        case CFG_EVENT_PRIO: {
            PrioConfigEventData *data = (PrioConfigEventData *)event->mData.get();
            // FIXME Need to understand why this has to be done asynchronously
            int err = requestPriority(data->mPid, data->mTid, data->mPrio, data->mForApp,
                    true /*asynchronous*/);
            if (err != 0) {
                ALOGW("Policy SCHED_FIFO priority %d is unavailable for pid %d tid %d; error %d",
                      data->mPrio, data->mPid, data->mTid, err);
            }
        } break;
        case CFG_EVENT_IO: {
            IoConfigEventData *data = (IoConfigEventData *)event->mData.get();
            ioConfigChanged(data->mEvent, data->mPid, data->mPortId);
        } break;
        case CFG_EVENT_SET_PARAMETER: {
            SetParameterConfigEventData *data = (SetParameterConfigEventData *)event->mData.get();
            if (checkForNewParameter_l(data->mKeyValuePairs, event->mStatus)) {
                configChanged = true;
                mLocalLog.log("CFG_EVENT_SET_PARAMETER: (%s) configuration changed",
                        data->mKeyValuePairs.string());
            }
        } break;
        case CFG_EVENT_CREATE_AUDIO_PATCH: {
            const audio_devices_t oldDevice = getDevice();
            CreateAudioPatchConfigEventData *data =
                                            (CreateAudioPatchConfigEventData *)event->mData.get();
            event->mStatus = createAudioPatch_l(&data->mPatch, &data->mHandle);
            const audio_devices_t newDevice = getDevice();
            mLocalLog.log("CFG_EVENT_CREATE_AUDIO_PATCH: old device %#x (%s) new device %#x (%s)",
                    (unsigned)oldDevice, toString(oldDevice).c_str(),
                    (unsigned)newDevice, toString(newDevice).c_str());
        } break;
        case CFG_EVENT_RELEASE_AUDIO_PATCH: {
            const audio_devices_t oldDevice = getDevice();
            ReleaseAudioPatchConfigEventData *data =
                                            (ReleaseAudioPatchConfigEventData *)event->mData.get();
            event->mStatus = releaseAudioPatch_l(data->mHandle);
            const audio_devices_t newDevice = getDevice();
            mLocalLog.log("CFG_EVENT_RELEASE_AUDIO_PATCH: old device %#x (%s) new device %#x (%s)",
                    (unsigned)oldDevice, toString(oldDevice).c_str(),
                    (unsigned)newDevice, toString(newDevice).c_str());
        } break;
        default:
            ALOG_ASSERT(false, "processConfigEvents_l() unknown event type %d", event->mType);
            break;
        }
        {
            Mutex::Autolock _l(event->mLock);
            if (event->mWaitStatus) {
                event->mWaitStatus = false;
                event->mCond.signal();
            }
        }
        ALOGV_IF(mConfigEvents.isEmpty(), "processConfigEvents_l() DONE thread %p", this);
    }

    if (configChanged) {
        cacheParameters_l();
    }
}

String8 channelMaskToString(audio_channel_mask_t mask, bool output) {
    String8 s;
    const audio_channel_representation_t representation =
            audio_channel_mask_get_representation(mask);

    switch (representation) {
    // Travel all single bit channel mask to convert channel mask to string.
    case AUDIO_CHANNEL_REPRESENTATION_POSITION: {
        if (output) {
            if (mask & AUDIO_CHANNEL_OUT_FRONT_LEFT) s.append("front-left, ");
            if (mask & AUDIO_CHANNEL_OUT_FRONT_RIGHT) s.append("front-right, ");
            if (mask & AUDIO_CHANNEL_OUT_FRONT_CENTER) s.append("front-center, ");
            if (mask & AUDIO_CHANNEL_OUT_LOW_FREQUENCY) s.append("low freq, ");
            if (mask & AUDIO_CHANNEL_OUT_BACK_LEFT) s.append("back-left, ");
            if (mask & AUDIO_CHANNEL_OUT_BACK_RIGHT) s.append("back-right, ");
            if (mask & AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER) s.append("front-left-of-center, ");
            if (mask & AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER) s.append("front-right-of-center, ");
            if (mask & AUDIO_CHANNEL_OUT_BACK_CENTER) s.append("back-center, ");
            if (mask & AUDIO_CHANNEL_OUT_SIDE_LEFT) s.append("side-left, ");
            if (mask & AUDIO_CHANNEL_OUT_SIDE_RIGHT) s.append("side-right, ");
            if (mask & AUDIO_CHANNEL_OUT_TOP_CENTER) s.append("top-center ,");
            if (mask & AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT) s.append("top-front-left, ");
            if (mask & AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER) s.append("top-front-center, ");
            if (mask & AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT) s.append("top-front-right, ");
            if (mask & AUDIO_CHANNEL_OUT_TOP_BACK_LEFT) s.append("top-back-left, ");
            if (mask & AUDIO_CHANNEL_OUT_TOP_BACK_CENTER) s.append("top-back-center, " );
            if (mask & AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT) s.append("top-back-right, " );
            if (mask & AUDIO_CHANNEL_OUT_TOP_SIDE_LEFT) s.append("top-side-left, " );
            if (mask & AUDIO_CHANNEL_OUT_TOP_SIDE_RIGHT) s.append("top-side-right, " );
            if (mask & AUDIO_CHANNEL_OUT_HAPTIC_B) s.append("haptic-B, " );
            if (mask & AUDIO_CHANNEL_OUT_HAPTIC_A) s.append("haptic-A, " );
            if (mask & ~AUDIO_CHANNEL_OUT_ALL) s.append("unknown,  ");
        } else {
            if (mask & AUDIO_CHANNEL_IN_LEFT) s.append("left, ");
            if (mask & AUDIO_CHANNEL_IN_RIGHT) s.append("right, ");
            if (mask & AUDIO_CHANNEL_IN_FRONT) s.append("front, ");
            if (mask & AUDIO_CHANNEL_IN_BACK) s.append("back, ");
            if (mask & AUDIO_CHANNEL_IN_LEFT_PROCESSED) s.append("left-processed, ");
            if (mask & AUDIO_CHANNEL_IN_RIGHT_PROCESSED) s.append("right-processed, ");
            if (mask & AUDIO_CHANNEL_IN_FRONT_PROCESSED) s.append("front-processed, ");
            if (mask & AUDIO_CHANNEL_IN_BACK_PROCESSED) s.append("back-processed, ");
            if (mask & AUDIO_CHANNEL_IN_PRESSURE) s.append("pressure, ");
            if (mask & AUDIO_CHANNEL_IN_X_AXIS) s.append("X, ");
            if (mask & AUDIO_CHANNEL_IN_Y_AXIS) s.append("Y, ");
            if (mask & AUDIO_CHANNEL_IN_Z_AXIS) s.append("Z, ");
            if (mask & AUDIO_CHANNEL_IN_BACK_LEFT) s.append("back-left, ");
            if (mask & AUDIO_CHANNEL_IN_BACK_RIGHT) s.append("back-right, ");
            if (mask & AUDIO_CHANNEL_IN_CENTER) s.append("center, ");
            if (mask & AUDIO_CHANNEL_IN_LOW_FREQUENCY) s.append("low freq, ");
            if (mask & AUDIO_CHANNEL_IN_TOP_LEFT) s.append("top-left, " );
            if (mask & AUDIO_CHANNEL_IN_TOP_RIGHT) s.append("top-right, " );
            if (mask & AUDIO_CHANNEL_IN_VOICE_UPLINK) s.append("voice-uplink, ");
            if (mask & AUDIO_CHANNEL_IN_VOICE_DNLINK) s.append("voice-dnlink, ");
            if (mask & ~AUDIO_CHANNEL_IN_ALL) s.append("unknown,  ");
        }
        const int len = s.length();
        if (len > 2) {
            (void) s.lockBuffer(len);      // needed?
            s.unlockBuffer(len - 2);       // remove trailing ", "
        }
        return s;
    }
    case AUDIO_CHANNEL_REPRESENTATION_INDEX:
        s.appendFormat("index mask, bits:%#x", audio_channel_mask_get_bits(mask));
        return s;
    default:
        s.appendFormat("unknown mask, representation:%d  bits:%#x",
                representation, audio_channel_mask_get_bits(mask));
        return s;
    }
}

void AudioFlinger::ThreadBase::dump(int fd, const Vector<String16>& args)
{
    dprintf(fd, "\n%s thread %p, name %s, tid %d, type %d (%s):\n", isOutput() ? "Output" : "Input",
            this, mThreadName, getTid(), type(), threadTypeToString(type()));

    bool locked = AudioFlinger::dumpTryLock(mLock);
    if (!locked) {
        dprintf(fd, "  Thread may be deadlocked\n");
    }

    dumpBase_l(fd, args);
    dumpInternals_l(fd, args);
    dumpTracks_l(fd, args);
    dumpEffectChains_l(fd, args);

    if (locked) {
        mLock.unlock();
    }

    dprintf(fd, "  Local log:\n");
    mLocalLog.dump(fd, "   " /* prefix */, 40 /* lines */);
}

void AudioFlinger::ThreadBase::dumpBase_l(int fd, const Vector<String16>& args __unused)
{
    dprintf(fd, "  I/O handle: %d\n", mId);
    dprintf(fd, "  Standby: %s\n", mStandby ? "yes" : "no");
    dprintf(fd, "  Sample rate: %u Hz\n", mSampleRate);
    dprintf(fd, "  HAL frame count: %zu\n", mFrameCount);
    dprintf(fd, "  HAL format: 0x%x (%s)\n", mHALFormat, formatToString(mHALFormat).c_str());
    dprintf(fd, "  HAL buffer size: %zu bytes\n", mBufferSize);
    dprintf(fd, "  Channel count: %u\n", mChannelCount);
    dprintf(fd, "  Channel mask: 0x%08x (%s)\n", mChannelMask,
            channelMaskToString(mChannelMask, mType != RECORD).string());
    dprintf(fd, "  Processing format: 0x%x (%s)\n", mFormat, formatToString(mFormat).c_str());
    dprintf(fd, "  Processing frame size: %zu bytes\n", mFrameSize);
    dprintf(fd, "  Pending config events:");
    size_t numConfig = mConfigEvents.size();
    if (numConfig) {
        const size_t SIZE = 256;
        char buffer[SIZE];
        for (size_t i = 0; i < numConfig; i++) {
            mConfigEvents[i]->dump(buffer, SIZE);
            dprintf(fd, "\n    %s", buffer);
        }
        dprintf(fd, "\n");
    } else {
        dprintf(fd, " none\n");
    }
    // Note: output device may be used by capture threads for effects such as AEC.
    dprintf(fd, "  Output device: %#x (%s)\n", mOutDevice, toString(mOutDevice).c_str());
    dprintf(fd, "  Input device: %#x (%s)\n", mInDevice, toString(mInDevice).c_str());
    dprintf(fd, "  Audio source: %d (%s)\n", mAudioSource, toString(mAudioSource).c_str());

    // Dump timestamp statistics for the Thread types that support it.
    if (mType == RECORD
            || mType == MIXER
            || mType == DUPLICATING
            || mType == DIRECT
            || mType == OFFLOAD) {
        dprintf(fd, "  Timestamp stats: %s\n", mTimestampVerifier.toString().c_str());
        dprintf(fd, "  Timestamp corrected: %s\n", isTimestampCorrectionEnabled() ? "yes" : "no");
    }

    if (mLastIoBeginNs > 0) { // MMAP may not set this
        dprintf(fd, "  Last %s occurred (msecs): %lld\n",
                isOutput() ? "write" : "read",
                (long long) (systemTime() - mLastIoBeginNs) / NANOS_PER_MILLISECOND);
    }

    if (mProcessTimeMs.getN() > 0) {
        dprintf(fd, "  Process time ms stats: %s\n", mProcessTimeMs.toString().c_str());
    }

    if (mIoJitterMs.getN() > 0) {
        dprintf(fd, "  Hal %s jitter ms stats: %s\n",
                isOutput() ? "write" : "read",
                mIoJitterMs.toString().c_str());
    }

    if (mLatencyMs.getN() > 0) {
        dprintf(fd, "  Threadloop %s latency stats: %s\n",
                isOutput() ? "write" : "read",
                mLatencyMs.toString().c_str());
    }
}

void AudioFlinger::ThreadBase::dumpEffectChains_l(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];

    size_t numEffectChains = mEffectChains.size();
    snprintf(buffer, SIZE, "  %zu Effect Chains\n", numEffectChains);
    write(fd, buffer, strlen(buffer));

    for (size_t i = 0; i < numEffectChains; ++i) {
        sp<EffectChain> chain = mEffectChains[i];
        if (chain != 0) {
            chain->dump(fd, args);
        }
    }
}

void AudioFlinger::ThreadBase::acquireWakeLock()
{
    Mutex::Autolock _l(mLock);
    acquireWakeLock_l();
}

String16 AudioFlinger::ThreadBase::getWakeLockTag()
{
    switch (mType) {
    case MIXER:
        return String16("AudioMix");
    case DIRECT:
        return String16("AudioDirectOut");
    case DUPLICATING:
        return String16("AudioDup");
    case RECORD:
        return String16("AudioIn");
    case OFFLOAD:
        return String16("AudioOffload");
    case MMAP:
        return String16("Mmap");
    default:
        ALOG_ASSERT(false);
        return String16("AudioUnknown");
    }
}

void AudioFlinger::ThreadBase::acquireWakeLock_l()
{
    getPowerManager_l();
    if (mPowerManager != 0) {
        sp<IBinder> binder = new BBinder();
        // Uses AID_AUDIOSERVER for wakelock.  updateWakeLockUids_l() updates with client uids.
        status_t status = mPowerManager->acquireWakeLock(POWERMANAGER_PARTIAL_WAKE_LOCK,
                    binder,
                    getWakeLockTag(),
                    String16("audioserver"),
                    true /* FIXME force oneway contrary to .aidl */);
        if (status == NO_ERROR) {
            mWakeLockToken = binder;
        }
        ALOGV("acquireWakeLock_l() %s status %d", mThreadName, status);
    }

    gBoottime.acquire(mWakeLockToken);
    mTimestamp.mTimebaseOffset[ExtendedTimestamp::TIMEBASE_BOOTTIME] =
            gBoottime.getBoottimeOffset();
}

void AudioFlinger::ThreadBase::releaseWakeLock()
{
    Mutex::Autolock _l(mLock);
    releaseWakeLock_l();
}

void AudioFlinger::ThreadBase::releaseWakeLock_l()
{
    gBoottime.release(mWakeLockToken);
    if (mWakeLockToken != 0) {
        ALOGV("releaseWakeLock_l() %s", mThreadName);
        if (mPowerManager != 0) {
            mPowerManager->releaseWakeLock(mWakeLockToken, 0,
                    true /* FIXME force oneway contrary to .aidl */);
        }
        mWakeLockToken.clear();
    }
}

void AudioFlinger::ThreadBase::getPowerManager_l() {
    if (mSystemReady && mPowerManager == 0) {
        // use checkService() to avoid blocking if power service is not up yet
        sp<IBinder> binder =
            defaultServiceManager()->checkService(String16("power"));
        if (binder == 0) {
            ALOGW("Thread %s cannot connect to the power manager service", mThreadName);
        } else {
            mPowerManager = interface_cast<IPowerManager>(binder);
            binder->linkToDeath(mDeathRecipient);
        }
    }
}

void AudioFlinger::ThreadBase::updateWakeLockUids_l(const SortedVector<uid_t> &uids) {
    getPowerManager_l();

#if !LOG_NDEBUG
    std::stringstream s;
    for (uid_t uid : uids) {
        s << uid << " ";
    }
    ALOGD("updateWakeLockUids_l %s uids:%s", mThreadName, s.str().c_str());
#endif

    if (mWakeLockToken == NULL) { // token may be NULL if AudioFlinger::systemReady() not called.
        if (mSystemReady) {
            ALOGE("no wake lock to update, but system ready!");
        } else {
            ALOGW("no wake lock to update, system not ready yet");
        }
        return;
    }
    if (mPowerManager != 0) {
        std::vector<int> uidsAsInt(uids.begin(), uids.end()); // powermanager expects uids as ints
        status_t status = mPowerManager->updateWakeLockUids(
                mWakeLockToken, uidsAsInt.size(), uidsAsInt.data(),
                true /* FIXME force oneway contrary to .aidl */);
        ALOGV("updateWakeLockUids_l() %s status %d", mThreadName, status);
    }
}

void AudioFlinger::ThreadBase::clearPowerManager()
{
    Mutex::Autolock _l(mLock);
    releaseWakeLock_l();
    mPowerManager.clear();
}

void AudioFlinger::ThreadBase::PMDeathRecipient::binderDied(const wp<IBinder>& who __unused)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        thread->clearPowerManager();
    }
    ALOGW("power manager service died !!!");
}

void AudioFlinger::ThreadBase::setEffectSuspended_l(
        const effect_uuid_t *type, bool suspend, audio_session_t sessionId)
{
    sp<EffectChain> chain = getEffectChain_l(sessionId);
    if (chain != 0) {
        if (type != NULL) {
            chain->setEffectSuspended_l(type, suspend);
        } else {
            chain->setEffectSuspendedAll_l(suspend);
        }
    }

    updateSuspendedSessions_l(type, suspend, sessionId);
}

void AudioFlinger::ThreadBase::checkSuspendOnAddEffectChain_l(const sp<EffectChain>& chain)
{
    ssize_t index = mSuspendedSessions.indexOfKey(chain->sessionId());
    if (index < 0) {
        return;
    }

    const KeyedVector <int, sp<SuspendedSessionDesc> >& sessionEffects =
            mSuspendedSessions.valueAt(index);

    for (size_t i = 0; i < sessionEffects.size(); i++) {
        const sp<SuspendedSessionDesc>& desc = sessionEffects.valueAt(i);
        for (int j = 0; j < desc->mRefCount; j++) {
            if (sessionEffects.keyAt(i) == EffectChain::kKeyForSuspendAll) {
                chain->setEffectSuspendedAll_l(true);
            } else {
                ALOGV("checkSuspendOnAddEffectChain_l() suspending effects %08x",
                    desc->mType.timeLow);
                chain->setEffectSuspended_l(&desc->mType, true);
            }
        }
    }
}

void AudioFlinger::ThreadBase::updateSuspendedSessions_l(const effect_uuid_t *type,
                                                         bool suspend,
                                                         audio_session_t sessionId)
{
    ssize_t index = mSuspendedSessions.indexOfKey(sessionId);

    KeyedVector <int, sp<SuspendedSessionDesc> > sessionEffects;

    if (suspend) {
        if (index >= 0) {
            sessionEffects = mSuspendedSessions.valueAt(index);
        } else {
            mSuspendedSessions.add(sessionId, sessionEffects);
        }
    } else {
        if (index < 0) {
            return;
        }
        sessionEffects = mSuspendedSessions.valueAt(index);
    }


    int key = EffectChain::kKeyForSuspendAll;
    if (type != NULL) {
        key = type->timeLow;
    }
    index = sessionEffects.indexOfKey(key);

    sp<SuspendedSessionDesc> desc;
    if (suspend) {
        if (index >= 0) {
            desc = sessionEffects.valueAt(index);
        } else {
            desc = new SuspendedSessionDesc();
            if (type != NULL) {
                desc->mType = *type;
            }
            sessionEffects.add(key, desc);
            ALOGV("updateSuspendedSessions_l() suspend adding effect %08x", key);
        }
        desc->mRefCount++;
    } else {
        if (index < 0) {
            return;
        }
        desc = sessionEffects.valueAt(index);
        if (--desc->mRefCount == 0) {
            ALOGV("updateSuspendedSessions_l() restore removing effect %08x", key);
            sessionEffects.removeItemsAt(index);
            if (sessionEffects.isEmpty()) {
                ALOGV("updateSuspendedSessions_l() restore removing session %d",
                                 sessionId);
                mSuspendedSessions.removeItem(sessionId);
            }
        }
    }
    if (!sessionEffects.isEmpty()) {
        mSuspendedSessions.replaceValueFor(sessionId, sessionEffects);
    }
}

void AudioFlinger::ThreadBase::checkSuspendOnEffectEnabled(const sp<EffectModule>& effect,
                                                            bool enabled,
                                                            audio_session_t sessionId)
{
    Mutex::Autolock _l(mLock);
    checkSuspendOnEffectEnabled_l(effect, enabled, sessionId);
}

void AudioFlinger::ThreadBase::checkSuspendOnEffectEnabled_l(const sp<EffectModule>& effect,
                                                            bool enabled,
                                                            audio_session_t sessionId)
{
    if (mType != RECORD) {
        // suspend all effects in AUDIO_SESSION_OUTPUT_MIX when enabling any effect on
        // another session. This gives the priority to well behaved effect control panels
        // and applications not using global effects.
        // Enabling post processing in AUDIO_SESSION_OUTPUT_STAGE session does not affect
        // global effects
        if ((sessionId != AUDIO_SESSION_OUTPUT_MIX) && (sessionId != AUDIO_SESSION_OUTPUT_STAGE)) {
            setEffectSuspended_l(NULL, enabled, AUDIO_SESSION_OUTPUT_MIX);
        }
    }

    sp<EffectChain> chain = getEffectChain_l(sessionId);
    if (chain != 0) {
        chain->checkSuspendOnEffectEnabled(effect, enabled);
    }
}

// checkEffectCompatibility_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::RecordThread::checkEffectCompatibility_l(
        const effect_descriptor_t *desc, audio_session_t sessionId)
{
    // No global effect sessions on record threads
    if (sessionId == AUDIO_SESSION_OUTPUT_MIX || sessionId == AUDIO_SESSION_OUTPUT_STAGE) {
        ALOGW("checkEffectCompatibility_l(): global effect %s on record thread %s",
                desc->name, mThreadName);
        return BAD_VALUE;
    }
    // only pre processing effects on record thread
    if ((desc->flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_PRE_PROC) {
        ALOGW("checkEffectCompatibility_l(): non pre processing effect %s on record thread %s",
                desc->name, mThreadName);
        return BAD_VALUE;
    }

    // always allow effects without processing load or latency
    if ((desc->flags & EFFECT_FLAG_NO_PROCESS_MASK) == EFFECT_FLAG_NO_PROCESS) {
        return NO_ERROR;
    }

    audio_input_flags_t flags = mInput->flags;
    if (hasFastCapture() || (flags & AUDIO_INPUT_FLAG_FAST)) {
        if (flags & AUDIO_INPUT_FLAG_RAW) {
            ALOGW("checkEffectCompatibility_l(): effect %s on record thread %s in raw mode",
                  desc->name, mThreadName);
            return BAD_VALUE;
        }
        if ((desc->flags & EFFECT_FLAG_HW_ACC_TUNNEL) == 0) {
            ALOGW("checkEffectCompatibility_l(): non HW effect %s on record thread %s in fast mode",
                  desc->name, mThreadName);
            return BAD_VALUE;
        }
    }
    return NO_ERROR;
}

// checkEffectCompatibility_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::PlaybackThread::checkEffectCompatibility_l(
        const effect_descriptor_t *desc, audio_session_t sessionId)
{
    // no preprocessing on playback threads
    if ((desc->flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_PRE_PROC) {
        ALOGW("checkEffectCompatibility_l(): pre processing effect %s created on playback"
                " thread %s", desc->name, mThreadName);
        return BAD_VALUE;
    }

    // always allow effects without processing load or latency
    if ((desc->flags & EFFECT_FLAG_NO_PROCESS_MASK) == EFFECT_FLAG_NO_PROCESS) {
        return NO_ERROR;
    }

    switch (mType) {
    case MIXER: {
#ifndef MULTICHANNEL_EFFECT_CHAIN
        // Reject any effect on mixer multichannel sinks.
        // TODO: fix both format and multichannel issues with effects.
        if (mChannelCount != FCC_2) {
            ALOGW("checkEffectCompatibility_l(): effect %s for multichannel(%d) on MIXER"
                    " thread %s", desc->name, mChannelCount, mThreadName);
            return BAD_VALUE;
        }
#endif
        audio_output_flags_t flags = mOutput->flags;
        if (hasFastMixer() || (flags & AUDIO_OUTPUT_FLAG_FAST)) {
            if (sessionId == AUDIO_SESSION_OUTPUT_MIX) {
                // global effects are applied only to non fast tracks if they are SW
                if ((desc->flags & EFFECT_FLAG_HW_ACC_TUNNEL) == 0) {
                    break;
                }
            } else if (sessionId == AUDIO_SESSION_OUTPUT_STAGE) {
                // only post processing on output stage session
                if ((desc->flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_POST_PROC) {
                    ALOGW("checkEffectCompatibility_l(): non post processing effect %s not allowed"
                            " on output stage session", desc->name);
                    return BAD_VALUE;
                }
            } else {
                // no restriction on effects applied on non fast tracks
                if ((hasAudioSession_l(sessionId) & ThreadBase::FAST_SESSION) == 0) {
                    break;
                }
            }

            if (flags & AUDIO_OUTPUT_FLAG_RAW) {
                ALOGW("checkEffectCompatibility_l(): effect %s on playback thread in raw mode",
                      desc->name);
                return BAD_VALUE;
            }
            if ((desc->flags & EFFECT_FLAG_HW_ACC_TUNNEL) == 0) {
                ALOGW("checkEffectCompatibility_l(): non HW effect %s on playback thread"
                        " in fast mode", desc->name);
                return BAD_VALUE;
            }
        }
    } break;
    case OFFLOAD:
        // nothing actionable on offload threads, if the effect:
        //   - is offloadable: the effect can be created
        //   - is NOT offloadable: the effect should still be created, but EffectHandle::enable()
        //     will take care of invalidating the tracks of the thread
        break;
    case DIRECT:
        // Reject any effect on Direct output threads for now, since the format of
        // mSinkBuffer is not guaranteed to be compatible with effect processing (PCM 16 stereo).
        ALOGW("checkEffectCompatibility_l(): effect %s on DIRECT output thread %s",
                desc->name, mThreadName);
        return BAD_VALUE;
    case DUPLICATING:
#ifndef MULTICHANNEL_EFFECT_CHAIN
        // Reject any effect on mixer multichannel sinks.
        // TODO: fix both format and multichannel issues with effects.
        if (mChannelCount != FCC_2) {
            ALOGW("checkEffectCompatibility_l(): effect %s for multichannel(%d)"
                    " on DUPLICATING thread %s", desc->name, mChannelCount, mThreadName);
            return BAD_VALUE;
        }
#endif
        if ((sessionId == AUDIO_SESSION_OUTPUT_STAGE) || (sessionId == AUDIO_SESSION_OUTPUT_MIX)) {
            ALOGW("checkEffectCompatibility_l(): global effect %s on DUPLICATING"
                    " thread %s", desc->name, mThreadName);
            return BAD_VALUE;
        }
        if ((desc->flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_POST_PROC) {
            ALOGW("checkEffectCompatibility_l(): post processing effect %s on"
                    " DUPLICATING thread %s", desc->name, mThreadName);
            return BAD_VALUE;
        }
        if ((desc->flags & EFFECT_FLAG_HW_ACC_TUNNEL) != 0) {
            ALOGW("checkEffectCompatibility_l(): HW tunneled effect %s on"
                    " DUPLICATING thread %s", desc->name, mThreadName);
            return BAD_VALUE;
        }
        break;
    default:
        LOG_ALWAYS_FATAL("checkEffectCompatibility_l(): wrong thread type %d", mType);
    }

    return NO_ERROR;
}

// ThreadBase::createEffect_l() must be called with AudioFlinger::mLock held
sp<AudioFlinger::EffectHandle> AudioFlinger::ThreadBase::createEffect_l(
        const sp<AudioFlinger::Client>& client,
        const sp<IEffectClient>& effectClient,
        int32_t priority,
        audio_session_t sessionId,
        effect_descriptor_t *desc,
        int *enabled,
        status_t *status,
        bool pinned)
{
    sp<EffectModule> effect;
    sp<EffectHandle> handle;
    status_t lStatus;
    sp<EffectChain> chain;
    bool chainCreated = false;
    bool effectCreated = false;
    audio_unique_id_t effectId = AUDIO_UNIQUE_ID_USE_UNSPECIFIED;

    lStatus = initCheck();
    if (lStatus != NO_ERROR) {
        ALOGW("createEffect_l() Audio driver not initialized.");
        goto Exit;
    }

    ALOGV("createEffect_l() thread %p effect %s on session %d", this, desc->name, sessionId);

    { // scope for mLock
        Mutex::Autolock _l(mLock);

        lStatus = checkEffectCompatibility_l(desc, sessionId);
        if (lStatus != NO_ERROR) {
            goto Exit;
        }

        // check for existing effect chain with the requested audio session
        chain = getEffectChain_l(sessionId);
        if (chain == 0) {
            // create a new chain for this session
            ALOGV("createEffect_l() new effect chain for session %d", sessionId);
            chain = new EffectChain(this, sessionId);
            addEffectChain_l(chain);
            chain->setStrategy(getStrategyForSession_l(sessionId));
            chainCreated = true;
        } else {
            effect = chain->getEffectFromDesc_l(desc);
        }

        ALOGV("createEffect_l() got effect %p on chain %p", effect.get(), chain.get());

        if (effect == 0) {
            effectId = mAudioFlinger->nextUniqueId(AUDIO_UNIQUE_ID_USE_EFFECT);
            // create a new effect module if none present in the chain
            lStatus = chain->createEffect_l(effect, this, desc, effectId, sessionId, pinned);
            if (lStatus != NO_ERROR) {
                goto Exit;
            }
            effectCreated = true;

            effect->setDevice(mOutDevice);
            effect->setDevice(mInDevice);
            effect->setMode(mAudioFlinger->getMode());
            effect->setAudioSource(mAudioSource);
        }
        // create effect handle and connect it to effect module
        handle = new EffectHandle(effect, client, effectClient, priority);
        lStatus = handle->initCheck();
        if (lStatus == OK) {
            lStatus = effect->addHandle(handle.get());
        }
        if (enabled != NULL) {
            *enabled = (int)effect->isEnabled();
        }
    }

Exit:
    if (lStatus != NO_ERROR && lStatus != ALREADY_EXISTS) {
        Mutex::Autolock _l(mLock);
        if (effectCreated) {
            chain->removeEffect_l(effect);
        }
        if (chainCreated) {
            removeEffectChain_l(chain);
        }
        // handle must be cleared by caller to avoid deadlock.
    }

    *status = lStatus;
    return handle;
}

void AudioFlinger::ThreadBase::disconnectEffectHandle(EffectHandle *handle,
                                                      bool unpinIfLast)
{
    bool remove = false;
    sp<EffectModule> effect;
    {
        Mutex::Autolock _l(mLock);

        effect = handle->effect().promote();
        if (effect == 0) {
            return;
        }
        // restore suspended effects if the disconnected handle was enabled and the last one.
        remove = (effect->removeHandle(handle) == 0) && (!effect->isPinned() || unpinIfLast);
        if (remove) {
            removeEffect_l(effect, true);
        }
    }
    if (remove) {
        mAudioFlinger->updateOrphanEffectChains(effect);
        if (handle->enabled()) {
            checkSuspendOnEffectEnabled(effect, false, effect->sessionId());
        }
    }
}

sp<AudioFlinger::EffectModule> AudioFlinger::ThreadBase::getEffect(audio_session_t sessionId,
        int effectId)
{
    Mutex::Autolock _l(mLock);
    return getEffect_l(sessionId, effectId);
}

sp<AudioFlinger::EffectModule> AudioFlinger::ThreadBase::getEffect_l(audio_session_t sessionId,
        int effectId)
{
    sp<EffectChain> chain = getEffectChain_l(sessionId);
    return chain != 0 ? chain->getEffectFromId_l(effectId) : 0;
}

std::vector<int> AudioFlinger::ThreadBase::getEffectIds_l(audio_session_t sessionId)
{
    sp<EffectChain> chain = getEffectChain_l(sessionId);
    return chain != nullptr ? chain->getEffectIds() : std::vector<int>{};
}

// PlaybackThread::addEffect_l() must be called with AudioFlinger::mLock and
// PlaybackThread::mLock held
status_t AudioFlinger::ThreadBase::addEffect_l(const sp<EffectModule>& effect)
{
    // check for existing effect chain with the requested audio session
    audio_session_t sessionId = effect->sessionId();
    sp<EffectChain> chain = getEffectChain_l(sessionId);
    bool chainCreated = false;

    ALOGD_IF((mType == OFFLOAD) && !effect->isOffloadable(),
             "addEffect_l() on offloaded thread %p: effect %s does not support offload flags %#x",
                    this, effect->desc().name, effect->desc().flags);

    if (chain == 0) {
        // create a new chain for this session
        ALOGV("addEffect_l() new effect chain for session %d", sessionId);
        chain = new EffectChain(this, sessionId);
        addEffectChain_l(chain);
        chain->setStrategy(getStrategyForSession_l(sessionId));
        chainCreated = true;
    }
    ALOGV("addEffect_l() %p chain %p effect %p", this, chain.get(), effect.get());

    if (chain->getEffectFromId_l(effect->id()) != 0) {
        ALOGW("addEffect_l() %p effect %s already present in chain %p",
                this, effect->desc().name, chain.get());
        return BAD_VALUE;
    }

    effect->setOffloaded(mType == OFFLOAD, mId);

    status_t status = chain->addEffect_l(effect);
    if (status != NO_ERROR) {
        if (chainCreated) {
            removeEffectChain_l(chain);
        }
        return status;
    }

    effect->setDevice(mOutDevice);
    effect->setDevice(mInDevice);
    effect->setMode(mAudioFlinger->getMode());
    effect->setAudioSource(mAudioSource);

    return NO_ERROR;
}

void AudioFlinger::ThreadBase::removeEffect_l(const sp<EffectModule>& effect, bool release) {

    ALOGV("%s %p effect %p", __FUNCTION__, this, effect.get());
    effect_descriptor_t desc = effect->desc();
    if ((desc.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
        detachAuxEffect_l(effect->id());
    }

    sp<EffectChain> chain = effect->chain().promote();
    if (chain != 0) {
        // remove effect chain if removing last effect
        if (chain->removeEffect_l(effect, release) == 0) {
            removeEffectChain_l(chain);
        }
    } else {
        ALOGW("removeEffect_l() %p cannot promote chain for effect %p", this, effect.get());
    }
}

void AudioFlinger::ThreadBase::lockEffectChains_l(
        Vector< sp<AudioFlinger::EffectChain> >& effectChains)
{
    effectChains = mEffectChains;
    for (size_t i = 0; i < mEffectChains.size(); i++) {
        mEffectChains[i]->lock();
    }
}

void AudioFlinger::ThreadBase::unlockEffectChains(
        const Vector< sp<AudioFlinger::EffectChain> >& effectChains)
{
    for (size_t i = 0; i < effectChains.size(); i++) {
        effectChains[i]->unlock();
    }
}

sp<AudioFlinger::EffectChain> AudioFlinger::ThreadBase::getEffectChain(audio_session_t sessionId)
{
    Mutex::Autolock _l(mLock);
    return getEffectChain_l(sessionId);
}

sp<AudioFlinger::EffectChain> AudioFlinger::ThreadBase::getEffectChain_l(audio_session_t sessionId)
        const
{
    size_t size = mEffectChains.size();
    for (size_t i = 0; i < size; i++) {
        if (mEffectChains[i]->sessionId() == sessionId) {
            return mEffectChains[i];
        }
    }
    return 0;
}

void AudioFlinger::ThreadBase::setMode(audio_mode_t mode)
{
    Mutex::Autolock _l(mLock);
    size_t size = mEffectChains.size();
    for (size_t i = 0; i < size; i++) {
        mEffectChains[i]->setMode_l(mode);
    }
}

void AudioFlinger::ThreadBase::toAudioPortConfig(struct audio_port_config *config)
{
    config->type = AUDIO_PORT_TYPE_MIX;
    config->ext.mix.handle = mId;
    config->sample_rate = mSampleRate;
    config->format = mFormat;
    config->channel_mask = mChannelMask;
    config->config_mask = AUDIO_PORT_CONFIG_SAMPLE_RATE|AUDIO_PORT_CONFIG_CHANNEL_MASK|
                            AUDIO_PORT_CONFIG_FORMAT;
}

void AudioFlinger::ThreadBase::systemReady()
{
    Mutex::Autolock _l(mLock);
    if (mSystemReady) {
        return;
    }
    mSystemReady = true;

    for (size_t i = 0; i < mPendingConfigEvents.size(); i++) {
        sendConfigEvent_l(mPendingConfigEvents.editItemAt(i));
    }
    mPendingConfigEvents.clear();
}

template <typename T>
ssize_t AudioFlinger::ThreadBase::ActiveTracks<T>::add(const sp<T> &track) {
    ssize_t index = mActiveTracks.indexOf(track);
    if (index >= 0) {
        ALOGW("ActiveTracks<T>::add track %p already there", track.get());
        return index;
    }
    logTrack("add", track);
    mActiveTracksGeneration++;
    mLatestActiveTrack = track;
    ++mBatteryCounter[track->uid()].second;
    mHasChanged = true;
    return mActiveTracks.add(track);
}

template <typename T>
ssize_t AudioFlinger::ThreadBase::ActiveTracks<T>::remove(const sp<T> &track) {
    ssize_t index = mActiveTracks.remove(track);
    if (index < 0) {
        ALOGW("ActiveTracks<T>::remove nonexistent track %p", track.get());
        return index;
    }
    logTrack("remove", track);
    mActiveTracksGeneration++;
    --mBatteryCounter[track->uid()].second;
    // mLatestActiveTrack is not cleared even if is the same as track.
    mHasChanged = true;
#ifdef TEE_SINK
    track->dumpTee(-1 /* fd */, "_REMOVE");
#endif
    return index;
}

template <typename T>
void AudioFlinger::ThreadBase::ActiveTracks<T>::clear() {
    for (const sp<T> &track : mActiveTracks) {
        BatteryNotifier::getInstance().noteStopAudio(track->uid());
        logTrack("clear", track);
    }
    mLastActiveTracksGeneration = mActiveTracksGeneration;
    if (!mActiveTracks.empty()) { mHasChanged = true; }
    mActiveTracks.clear();
    mLatestActiveTrack.clear();
    mBatteryCounter.clear();
}

template <typename T>
void AudioFlinger::ThreadBase::ActiveTracks<T>::updatePowerState(
        sp<ThreadBase> thread, bool force) {
    // Updates ActiveTracks client uids to the thread wakelock.
    if (mActiveTracksGeneration != mLastActiveTracksGeneration || force) {
        thread->updateWakeLockUids_l(getWakeLockUids());
        mLastActiveTracksGeneration = mActiveTracksGeneration;
    }

    // Updates BatteryNotifier uids
    for (auto it = mBatteryCounter.begin(); it != mBatteryCounter.end();) {
        const uid_t uid = it->first;
        ssize_t &previous = it->second.first;
        ssize_t &current = it->second.second;
        if (current > 0) {
            if (previous == 0) {
                BatteryNotifier::getInstance().noteStartAudio(uid);
            }
            previous = current;
            ++it;
        } else if (current == 0) {
            if (previous > 0) {
                BatteryNotifier::getInstance().noteStopAudio(uid);
            }
            it = mBatteryCounter.erase(it); // std::map<> is stable on iterator erase.
        } else /* (current < 0) */ {
            LOG_ALWAYS_FATAL("negative battery count %zd", current);
        }
    }
}

template <typename T>
bool AudioFlinger::ThreadBase::ActiveTracks<T>::readAndClearHasChanged() {
    const bool hasChanged = mHasChanged;
    mHasChanged = false;
    return hasChanged;
}

template <typename T>
void AudioFlinger::ThreadBase::ActiveTracks<T>::logTrack(
        const char *funcName, const sp<T> &track) const {
    if (mLocalLog != nullptr) {
        String8 result;
        track->appendDump(result, false /* active */);
        mLocalLog->log("AT::%-10s(%p) %s", funcName, track.get(), result.string());
    }
}

void AudioFlinger::ThreadBase::broadcast_l()
{
    // Thread could be blocked waiting for async
    // so signal it to handle state changes immediately
    // If threadLoop is currently unlocked a signal of mWaitWorkCV will
    // be lost so we also flag to prevent it blocking on mWaitWorkCV
    mSignalPending = true;
    mWaitWorkCV.broadcast();
}

// Call only from threadLoop() or when it is idle.
// Do not call from high performance code as this may do binder rpc to the MediaMetrics service.
void AudioFlinger::ThreadBase::sendStatistics(bool force)
{
    // Do not log if we have no stats.
    // We choose the timestamp verifier because it is the most likely item to be present.
    const int64_t nstats = mTimestampVerifier.getN() - mLastRecordedTimestampVerifierN;
    if (nstats == 0) {
        return;
    }

    // Don't log more frequently than once per 12 hours.
    // We use BOOTTIME to include suspend time.
    const int64_t timeNs = systemTime(SYSTEM_TIME_BOOTTIME);
    const int64_t sinceNs = timeNs - mLastRecordedTimeNs; // ok if mLastRecordedTimeNs = 0
    if (!force && sinceNs <= 12 * NANOS_PER_HOUR) {
        return;
    }

    mLastRecordedTimestampVerifierN = mTimestampVerifier.getN();
    mLastRecordedTimeNs = timeNs;

    std::unique_ptr<MediaAnalyticsItem> item(MediaAnalyticsItem::create("audiothread"));

#define MM_PREFIX "android.media.audiothread." // avoid cut-n-paste errors.

    // thread configuration
    item->setInt32(MM_PREFIX "id", (int32_t)mId); // IO handle
    // item->setInt32(MM_PREFIX "portId", (int32_t)mPortId);
    item->setCString(MM_PREFIX "type", threadTypeToString(mType));
    item->setInt32(MM_PREFIX "sampleRate", (int32_t)mSampleRate);
    item->setInt64(MM_PREFIX "channelMask", (int64_t)mChannelMask);
    item->setCString(MM_PREFIX "encoding", toString(mFormat).c_str());
    item->setInt32(MM_PREFIX "frameCount", (int32_t)mFrameCount);
    item->setCString(MM_PREFIX "outDevice", toString(mOutDevice).c_str());
    item->setCString(MM_PREFIX "inDevice", toString(mInDevice).c_str());

    // thread statistics
    if (mIoJitterMs.getN() > 0) {
        item->setDouble(MM_PREFIX "ioJitterMs.mean", mIoJitterMs.getMean());
        item->setDouble(MM_PREFIX "ioJitterMs.std", mIoJitterMs.getStdDev());
    }
    if (mProcessTimeMs.getN() > 0) {
        item->setDouble(MM_PREFIX "processTimeMs.mean", mProcessTimeMs.getMean());
        item->setDouble(MM_PREFIX "processTimeMs.std", mProcessTimeMs.getStdDev());
    }
    const auto tsjitter = mTimestampVerifier.getJitterMs();
    if (tsjitter.getN() > 0) {
        item->setDouble(MM_PREFIX "timestampJitterMs.mean", tsjitter.getMean());
        item->setDouble(MM_PREFIX "timestampJitterMs.std", tsjitter.getStdDev());
    }
    if (mLatencyMs.getN() > 0) {
        item->setDouble(MM_PREFIX "latencyMs.mean", mLatencyMs.getMean());
        item->setDouble(MM_PREFIX "latencyMs.std", mLatencyMs.getStdDev());
    }

    item->selfrecord();
}

// ----------------------------------------------------------------------------
//      Playback
// ----------------------------------------------------------------------------

AudioFlinger::PlaybackThread::PlaybackThread(const sp<AudioFlinger>& audioFlinger,
                                             AudioStreamOut* output,
                                             audio_io_handle_t id,
                                             audio_devices_t device,
                                             type_t type,
                                             bool systemReady)
    :   ThreadBase(audioFlinger, id, device, AUDIO_DEVICE_NONE, type, systemReady),
        mNormalFrameCount(0), mSinkBuffer(NULL),
        mMixerBufferEnabled(AudioFlinger::kEnableExtendedPrecision),
        mMixerBuffer(NULL),
        mMixerBufferSize(0),
        mMixerBufferFormat(AUDIO_FORMAT_INVALID),
        mMixerBufferValid(false),
        mEffectBufferEnabled(AudioFlinger::kEnableExtendedPrecision),
        mEffectBuffer(NULL),
        mEffectBufferSize(0),
        mEffectBufferFormat(AUDIO_FORMAT_INVALID),
        mEffectBufferValid(false),
        mSuspended(0), mBytesWritten(0),
        mFramesWritten(0),
        mSuspendedFrames(0),
        mActiveTracks(&this->mLocalLog),
        // mStreamTypes[] initialized in constructor body
        mTracks(type == MIXER),
        mOutput(output),
        mNumWrites(0), mNumDelayedWrites(0), mInWrite(false),
        mMixerStatus(MIXER_IDLE),
        mMixerStatusIgnoringFastTracks(MIXER_IDLE),
        mStandbyDelayNs(AudioFlinger::mStandbyTimeInNsecs),
        mBytesRemaining(0),
        mCurrentWriteLength(0),
        mUseAsyncWrite(false),
        mWriteAckSequence(0),
        mDrainSequence(0),
        mScreenState(AudioFlinger::mScreenState),
        // index 0 is reserved for normal mixer's submix
        mFastTrackAvailMask(((1 << FastMixerState::sMaxFastTracks) - 1) & ~1),
        mHwSupportsPause(false), mHwPaused(false), mFlushPending(false),
        mLeftVolFloat(-1.0), mRightVolFloat(-1.0)
{
    snprintf(mThreadName, kThreadNameLength, "AudioOut_%X", id);
    mNBLogWriter = audioFlinger->newWriter_l(kLogSize, mThreadName);

    // Assumes constructor is called by AudioFlinger with it's mLock held, but
    // it would be safer to explicitly pass initial masterVolume/masterMute as
    // parameter.
    //
    // If the HAL we are using has support for master volume or master mute,
    // then do not attenuate or mute during mixing (just leave the volume at 1.0
    // and the mute set to false).
    mMasterVolume = audioFlinger->masterVolume_l();
    mMasterMute = audioFlinger->masterMute_l();
    if (mOutput && mOutput->audioHwDev) {
        if (mOutput->audioHwDev->canSetMasterVolume()) {
            mMasterVolume = 1.0;
        }

        if (mOutput->audioHwDev->canSetMasterMute()) {
            mMasterMute = false;
        }
        mIsMsdDevice = strcmp(
                mOutput->audioHwDev->moduleName(), AUDIO_HARDWARE_MODULE_ID_MSD) == 0;
    }

    readOutputParameters_l();

    // TODO: We may also match on address as well as device type for
    // AUDIO_DEVICE_OUT_BUS, AUDIO_DEVICE_OUT_ALL_A2DP, AUDIO_DEVICE_OUT_REMOTE_SUBMIX
    if (type == MIXER || type == DIRECT) {
        mTimestampCorrectedDevices = (audio_devices_t)property_get_int64(
                "audio.timestamp.corrected_output_devices",
                (int64_t)(mIsMsdDevice ? AUDIO_DEVICE_OUT_BUS // turn on by default for MSD
                                       : AUDIO_DEVICE_NONE));
    }

    // ++ operator does not compile
    for (audio_stream_type_t stream = AUDIO_STREAM_MIN; stream < AUDIO_STREAM_FOR_POLICY_CNT;
            stream = (audio_stream_type_t) (stream + 1)) {
        mStreamTypes[stream].volume = 0.0f;
        mStreamTypes[stream].mute = mAudioFlinger->streamMute_l(stream);
    }
    // Audio patch volume is always max
    mStreamTypes[AUDIO_STREAM_PATCH].volume = 1.0f;
    mStreamTypes[AUDIO_STREAM_PATCH].mute = false;
}

AudioFlinger::PlaybackThread::~PlaybackThread()
{
    mAudioFlinger->unregisterWriter(mNBLogWriter);
    free(mSinkBuffer);
    free(mMixerBuffer);
    free(mEffectBuffer);
}

// Thread virtuals

void AudioFlinger::PlaybackThread::onFirstRef()
{
    run(mThreadName, ANDROID_PRIORITY_URGENT_AUDIO);
}

// ThreadBase virtuals
void AudioFlinger::PlaybackThread::preExit()
{
    ALOGV("  preExit()");
    // FIXME this is using hard-coded strings but in the future, this functionality will be
    //       converted to use audio HAL extensions required to support tunneling
    status_t result = mOutput->stream->setParameters(String8("exiting=1"));
    ALOGE_IF(result != OK, "Error when setting parameters on exit: %d", result);
}

void AudioFlinger::PlaybackThread::dumpTracks_l(int fd, const Vector<String16>& args __unused)
{
    String8 result;

    result.appendFormat("  Stream volumes in dB: ");
    for (int i = 0; i < AUDIO_STREAM_CNT; ++i) {
        const stream_type_t *st = &mStreamTypes[i];
        if (i > 0) {
            result.appendFormat(", ");
        }
        result.appendFormat("%d:%.2g", i, 20.0 * log10(st->volume));
        if (st->mute) {
            result.append("M");
        }
    }
    result.append("\n");
    write(fd, result.string(), result.length());
    result.clear();

    // These values are "raw"; they will wrap around.  See prepareTracks_l() for a better way.
    FastTrackUnderruns underruns = getFastTrackUnderruns(0);
    dprintf(fd, "  Normal mixer raw underrun counters: partial=%u empty=%u\n",
            underruns.mBitFields.mPartial, underruns.mBitFields.mEmpty);

    size_t numtracks = mTracks.size();
    size_t numactive = mActiveTracks.size();
    dprintf(fd, "  %zu Tracks", numtracks);
    size_t numactiveseen = 0;
    const char *prefix = "    ";
    if (numtracks) {
        dprintf(fd, " of which %zu are active\n", numactive);
        result.append(prefix);
        mTracks[0]->appendDumpHeader(result);
        for (size_t i = 0; i < numtracks; ++i) {
            sp<Track> track = mTracks[i];
            if (track != 0) {
                bool active = mActiveTracks.indexOf(track) >= 0;
                if (active) {
                    numactiveseen++;
                }
                result.append(prefix);
                track->appendDump(result, active);
            }
        }
    } else {
        result.append("\n");
    }
    if (numactiveseen != numactive) {
        // some tracks in the active list were not in the tracks list
        result.append("  The following tracks are in the active list but"
                " not in the track list\n");
        result.append(prefix);
        mActiveTracks[0]->appendDumpHeader(result);
        for (size_t i = 0; i < numactive; ++i) {
            sp<Track> track = mActiveTracks[i];
            if (mTracks.indexOf(track) < 0) {
                result.append(prefix);
                track->appendDump(result, true /* active */);
            }
        }
    }

    write(fd, result.string(), result.size());
}

void AudioFlinger::PlaybackThread::dumpInternals_l(int fd, const Vector<String16>& args __unused)
{
    dprintf(fd, "  Master mute: %s\n", mMasterMute ? "on" : "off");
    if (mHapticChannelMask != AUDIO_CHANNEL_NONE) {
        dprintf(fd, "  Haptic channel mask: %#x (%s)\n", mHapticChannelMask,
                channelMaskToString(mHapticChannelMask, true /* output */).c_str());
    }
    dprintf(fd, "  Normal frame count: %zu\n", mNormalFrameCount);
    dprintf(fd, "  Total writes: %d\n", mNumWrites);
    dprintf(fd, "  Delayed writes: %d\n", mNumDelayedWrites);
    dprintf(fd, "  Blocked in write: %s\n", mInWrite ? "yes" : "no");
    dprintf(fd, "  Suspend count: %d\n", mSuspended);
    dprintf(fd, "  Sink buffer : %p\n", mSinkBuffer);
    dprintf(fd, "  Mixer buffer: %p\n", mMixerBuffer);
    dprintf(fd, "  Effect buffer: %p\n", mEffectBuffer);
    dprintf(fd, "  Fast track availMask=%#x\n", mFastTrackAvailMask);
    dprintf(fd, "  Standby delay ns=%lld\n", (long long)mStandbyDelayNs);
    AudioStreamOut *output = mOutput;
    audio_output_flags_t flags = output != NULL ? output->flags : AUDIO_OUTPUT_FLAG_NONE;
    dprintf(fd, "  AudioStreamOut: %p flags %#x (%s)\n",
            output, flags, toString(flags).c_str());
    dprintf(fd, "  Frames written: %lld\n", (long long)mFramesWritten);
    dprintf(fd, "  Suspended frames: %lld\n", (long long)mSuspendedFrames);
    if (mPipeSink.get() != nullptr) {
        dprintf(fd, "  PipeSink frames written: %lld\n", (long long)mPipeSink->framesWritten());
    }
    if (output != nullptr) {
        dprintf(fd, "  Hal stream dump:\n");
        (void)output->stream->dump(fd);
    }
}

// PlaybackThread::createTrack_l() must be called with AudioFlinger::mLock held
sp<AudioFlinger::PlaybackThread::Track> AudioFlinger::PlaybackThread::createTrack_l(
        const sp<AudioFlinger::Client>& client,
        audio_stream_type_t streamType,
        const audio_attributes_t& attr,
        uint32_t *pSampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t *pFrameCount,
        size_t *pNotificationFrameCount,
        uint32_t notificationsPerBuffer,
        float speed,
        const sp<IMemory>& sharedBuffer,
        audio_session_t sessionId,
        audio_output_flags_t *flags,
        pid_t creatorPid,
        pid_t tid,
        uid_t uid,
        status_t *status,
        audio_port_handle_t portId)
{
    size_t frameCount = *pFrameCount;
    size_t notificationFrameCount = *pNotificationFrameCount;
    sp<Track> track;
    status_t lStatus;
    audio_output_flags_t outputFlags = mOutput->flags;
    audio_output_flags_t requestedFlags = *flags;
    uint32_t sampleRate;

    if (sharedBuffer != 0 && checkIMemory(sharedBuffer) != NO_ERROR) {
        lStatus = BAD_VALUE;
        goto Exit;
    }

    if (*pSampleRate == 0) {
        *pSampleRate = mSampleRate;
    }
    sampleRate = *pSampleRate;

    // special case for FAST flag considered OK if fast mixer is present
    if (hasFastMixer()) {
        outputFlags = (audio_output_flags_t)(outputFlags | AUDIO_OUTPUT_FLAG_FAST);
    }

    // Check if requested flags are compatible with output stream flags
    if ((*flags & outputFlags) != *flags) {
        ALOGW("createTrack_l(): mismatch between requested flags (%08x) and output flags (%08x)",
              *flags, outputFlags);
        *flags = (audio_output_flags_t)(*flags & outputFlags);
    }

    // client expresses a preference for FAST, but we get the final say
    if (*flags & AUDIO_OUTPUT_FLAG_FAST) {
      if (
            // PCM data
            audio_is_linear_pcm(format) &&
            // TODO: extract as a data library function that checks that a computationally
            // expensive downmixer is not required: isFastOutputChannelConversion()
            (channelMask == (mChannelMask | mHapticChannelMask) ||
                    mChannelMask != AUDIO_CHANNEL_OUT_STEREO ||
                    (channelMask == AUDIO_CHANNEL_OUT_MONO
                            /* && mChannelMask == AUDIO_CHANNEL_OUT_STEREO */)) &&
            // hardware sample rate
            (sampleRate == mSampleRate) &&
            // normal mixer has an associated fast mixer
            hasFastMixer() &&
            // there are sufficient fast track slots available
            (mFastTrackAvailMask != 0)
            // FIXME test that MixerThread for this fast track has a capable output HAL
            // FIXME add a permission test also?
        ) {
        // static tracks can have any nonzero framecount, streaming tracks check against minimum.
        if (sharedBuffer == 0) {
            // read the fast track multiplier property the first time it is needed
            int ok = pthread_once(&sFastTrackMultiplierOnce, sFastTrackMultiplierInit);
            if (ok != 0) {
                ALOGE("%s pthread_once failed: %d", __func__, ok);
            }
            frameCount = max(frameCount, mFrameCount * sFastTrackMultiplier); // incl framecount 0
        }

        // check compatibility with audio effects.
        { // scope for mLock
            Mutex::Autolock _l(mLock);
            for (audio_session_t session : {
                    AUDIO_SESSION_OUTPUT_STAGE,
                    AUDIO_SESSION_OUTPUT_MIX,
                    sessionId,
                }) {
                sp<EffectChain> chain = getEffectChain_l(session);
                if (chain.get() != nullptr) {
                    audio_output_flags_t old = *flags;
                    chain->checkOutputFlagCompatibility(flags);
                    if (old != *flags) {
                        ALOGV("AUDIO_OUTPUT_FLAGS denied by effect, session=%d old=%#x new=%#x",
                                (int)session, (int)old, (int)*flags);
                    }
                }
            }
        }
        ALOGV_IF((*flags & AUDIO_OUTPUT_FLAG_FAST) != 0,
                 "AUDIO_OUTPUT_FLAG_FAST accepted: frameCount=%zu mFrameCount=%zu",
                 frameCount, mFrameCount);
      } else {
        ALOGV("AUDIO_OUTPUT_FLAG_FAST denied: sharedBuffer=%p frameCount=%zu "
                "mFrameCount=%zu format=%#x mFormat=%#x isLinear=%d channelMask=%#x "
                "sampleRate=%u mSampleRate=%u "
                "hasFastMixer=%d tid=%d fastTrackAvailMask=%#x",
                sharedBuffer.get(), frameCount, mFrameCount, format, mFormat,
                audio_is_linear_pcm(format),
                channelMask, sampleRate, mSampleRate, hasFastMixer(), tid, mFastTrackAvailMask);
        *flags = (audio_output_flags_t)(*flags & ~AUDIO_OUTPUT_FLAG_FAST);
      }
    }

    if (!audio_has_proportional_frames(format)) {
        if (sharedBuffer != 0) {
            // Same comment as below about ignoring frameCount parameter for set()
            frameCount = sharedBuffer->size();
        } else if (frameCount == 0) {
            frameCount = mNormalFrameCount;
        }
        if (notificationFrameCount != frameCount) {
            notificationFrameCount = frameCount;
        }
    } else if (sharedBuffer != 0) {
        // FIXME: Ensure client side memory buffers need
        // not have additional alignment beyond sample
        // (e.g. 16 bit stereo accessed as 32 bit frame).
        size_t alignment = audio_bytes_per_sample(format);
        if (alignment & 1) {
            // for AUDIO_FORMAT_PCM_24_BIT_PACKED (not exposed through Java).
            alignment = 1;
        }
        uint32_t channelCount = audio_channel_count_from_out_mask(channelMask);
        size_t frameSize = channelCount * audio_bytes_per_sample(format);
        if (channelCount > 1) {
            // More than 2 channels does not require stronger alignment than stereo
            alignment <<= 1;
        }
        if (((uintptr_t)sharedBuffer->pointer() & (alignment - 1)) != 0) {
            ALOGE("Invalid buffer alignment: address %p, channel count %u",
                  sharedBuffer->pointer(), channelCount);
            lStatus = BAD_VALUE;
            goto Exit;
        }

        // When initializing a shared buffer AudioTrack via constructors,
        // there's no frameCount parameter.
        // But when initializing a shared buffer AudioTrack via set(),
        // there _is_ a frameCount parameter.  We silently ignore it.
        frameCount = sharedBuffer->size() / frameSize;
    } else {
        size_t minFrameCount = 0;
        // For fast tracks we try to respect the application's request for notifications per buffer.
        if (*flags & AUDIO_OUTPUT_FLAG_FAST) {
            if (notificationsPerBuffer > 0) {
                // Avoid possible arithmetic overflow during multiplication.
                if (notificationsPerBuffer > SIZE_MAX / mFrameCount) {
                    ALOGE("Requested notificationPerBuffer=%u ignored for HAL frameCount=%zu",
                          notificationsPerBuffer, mFrameCount);
                } else {
                    minFrameCount = mFrameCount * notificationsPerBuffer;
                }
            }
        } else {
            // For normal PCM streaming tracks, update minimum frame count.
            // Buffer depth is forced to be at least 2 x the normal mixer frame count and
            // cover audio hardware latency.
            // This is probably too conservative, but legacy application code may depend on it.
            // If you change this calculation, also review the start threshold which is related.
            uint32_t latencyMs = latency_l();
            if (latencyMs == 0) {
                ALOGE("Error when retrieving output stream latency");
                lStatus = UNKNOWN_ERROR;
                goto Exit;
            }

            minFrameCount = AudioSystem::calculateMinFrameCount(latencyMs, mNormalFrameCount,
                                mSampleRate, sampleRate, speed /*, 0 mNotificationsPerBufferReq*/);

        }
        if (frameCount < minFrameCount) {
            frameCount = minFrameCount;
        }
    }

    // Make sure that application is notified with sufficient margin before underrun.
    // The client can divide the AudioTrack buffer into sub-buffers,
    // and expresses its desire to server as the notification frame count.
    if (sharedBuffer == 0 && audio_is_linear_pcm(format)) {
        size_t maxNotificationFrames;
        if (*flags & AUDIO_OUTPUT_FLAG_FAST) {
            // notify every HAL buffer, regardless of the size of the track buffer
            maxNotificationFrames = mFrameCount;
        } else {
            // Triple buffer the notification period for a triple buffered mixer period;
            // otherwise, double buffering for the notification period is fine.
            //
            // TODO: This should be moved to AudioTrack to modify the notification period
            // on AudioTrack::setBufferSizeInFrames() changes.
            const int nBuffering =
                    (uint64_t{frameCount} * mSampleRate)
                            / (uint64_t{mNormalFrameCount} * sampleRate) == 3 ? 3 : 2;

            maxNotificationFrames = frameCount / nBuffering;
            // If client requested a fast track but this was denied, then use the smaller maximum.
            if (requestedFlags & AUDIO_OUTPUT_FLAG_FAST) {
                size_t maxNotificationFramesFastDenied = FMS_20 * sampleRate / 1000;
                if (maxNotificationFrames > maxNotificationFramesFastDenied) {
                    maxNotificationFrames = maxNotificationFramesFastDenied;
                }
            }
        }
        if (notificationFrameCount == 0 || notificationFrameCount > maxNotificationFrames) {
            if (notificationFrameCount == 0) {
                ALOGD("Client defaulted notificationFrames to %zu for frameCount %zu",
                    maxNotificationFrames, frameCount);
            } else {
                ALOGW("Client adjusted notificationFrames from %zu to %zu for frameCount %zu",
                      notificationFrameCount, maxNotificationFrames, frameCount);
            }
            notificationFrameCount = maxNotificationFrames;
        }
    }

    *pFrameCount = frameCount;
    *pNotificationFrameCount = notificationFrameCount;

    switch (mType) {

    case DIRECT:
        if (audio_is_linear_pcm(format)) { // TODO maybe use audio_has_proportional_frames()?
            if (sampleRate != mSampleRate || format != mFormat || channelMask != mChannelMask) {
                ALOGE("createTrack_l() Bad parameter: sampleRate %u format %#x, channelMask 0x%08x "
                        "for output %p with format %#x",
                        sampleRate, format, channelMask, mOutput, mFormat);
                lStatus = BAD_VALUE;
                goto Exit;
            }
        }
        break;

    case OFFLOAD:
        if (sampleRate != mSampleRate || format != mFormat || channelMask != mChannelMask) {
            ALOGE("createTrack_l() Bad parameter: sampleRate %d format %#x, channelMask 0x%08x \""
                    "for output %p with format %#x",
                    sampleRate, format, channelMask, mOutput, mFormat);
            lStatus = BAD_VALUE;
            goto Exit;
        }
        break;

    default:
        if (!audio_is_linear_pcm(format)) {
                ALOGE("createTrack_l() Bad parameter: format %#x \""
                        "for output %p with format %#x",
                        format, mOutput, mFormat);
                lStatus = BAD_VALUE;
                goto Exit;
        }
        if (sampleRate > mSampleRate * AUDIO_RESAMPLER_DOWN_RATIO_MAX) {
            ALOGE("Sample rate out of range: %u mSampleRate %u", sampleRate, mSampleRate);
            lStatus = BAD_VALUE;
            goto Exit;
        }
        break;

    }

    lStatus = initCheck();
    if (lStatus != NO_ERROR) {
        ALOGE("createTrack_l() audio driver not initialized");
        goto Exit;
    }

    { // scope for mLock
        Mutex::Autolock _l(mLock);

        // all tracks in same audio session must share the same routing strategy otherwise
        // conflicts will happen when tracks are moved from one output to another by audio policy
        // manager
        uint32_t strategy = AudioSystem::getStrategyForStream(streamType);
        for (size_t i = 0; i < mTracks.size(); ++i) {
            sp<Track> t = mTracks[i];
            if (t != 0 && t->isExternalTrack()) {
                uint32_t actual = AudioSystem::getStrategyForStream(t->streamType());
                if (sessionId == t->sessionId() && strategy != actual) {
                    ALOGE("createTrack_l() mismatched strategy; expected %u but found %u",
                            strategy, actual);
                    lStatus = BAD_VALUE;
                    goto Exit;
                }
            }
        }

        track = new Track(this, client, streamType, attr, sampleRate, format,
                          channelMask, frameCount,
                          nullptr /* buffer */, (size_t)0 /* bufferSize */, sharedBuffer,
                          sessionId, creatorPid, uid, *flags, TrackBase::TYPE_DEFAULT, portId);

        lStatus = track != 0 ? track->initCheck() : (status_t) NO_MEMORY;
        if (lStatus != NO_ERROR) {
            ALOGE("createTrack_l() initCheck failed %d; no control block?", lStatus);
            // track must be cleared from the caller as the caller has the AF lock
            goto Exit;
        }
        mTracks.add(track);

        sp<EffectChain> chain = getEffectChain_l(sessionId);
        if (chain != 0) {
            ALOGV("createTrack_l() setting main buffer %p", chain->inBuffer());
            track->setMainBuffer(chain->inBuffer());
            chain->setStrategy(AudioSystem::getStrategyForStream(track->streamType()));
            chain->incTrackCnt();
        }

        if ((*flags & AUDIO_OUTPUT_FLAG_FAST) && (tid != -1)) {
            pid_t callingPid = IPCThreadState::self()->getCallingPid();
            // we don't have CAP_SYS_NICE, nor do we want to have it as it's too powerful,
            // so ask activity manager to do this on our behalf
            sendPrioConfigEvent_l(callingPid, tid, kPriorityAudioApp, true /*forApp*/);
        }
    }

    lStatus = NO_ERROR;

Exit:
    *status = lStatus;
    return track;
}

template<typename T>
ssize_t AudioFlinger::PlaybackThread::Tracks<T>::remove(const sp<T> &track)
{
    const int trackId = track->id();
    const ssize_t index = mTracks.remove(track);
    if (index >= 0) {
        if (mSaveDeletedTrackIds) {
            // We can't directly access mAudioMixer since the caller may be outside of threadLoop.
            // Instead, we add to mDeletedTrackIds which is solely used for mAudioMixer update,
            // to be handled when MixerThread::prepareTracks_l() next changes mAudioMixer.
            mDeletedTrackIds.emplace(trackId);
        }
    }
    return index;
}

uint32_t AudioFlinger::PlaybackThread::correctLatency_l(uint32_t latency) const
{
    return latency;
}

uint32_t AudioFlinger::PlaybackThread::latency() const
{
    Mutex::Autolock _l(mLock);
    return latency_l();
}
uint32_t AudioFlinger::PlaybackThread::latency_l() const
{
    uint32_t latency;
    if (initCheck() == NO_ERROR && mOutput->stream->getLatency(&latency) == OK) {
        return correctLatency_l(latency);
    }
    return 0;
}

void AudioFlinger::PlaybackThread::setMasterVolume(float value)
{
    Mutex::Autolock _l(mLock);
    // Don't apply master volume in SW if our HAL can do it for us.
    if (mOutput && mOutput->audioHwDev &&
        mOutput->audioHwDev->canSetMasterVolume()) {
        mMasterVolume = 1.0;
    } else {
        mMasterVolume = value;
    }
}

void AudioFlinger::PlaybackThread::setMasterBalance(float balance)
{
    mMasterBalance.store(balance);
}

void AudioFlinger::PlaybackThread::setMasterMute(bool muted)
{
    if (isDuplicating()) {
        return;
    }
    Mutex::Autolock _l(mLock);
    // Don't apply master mute in SW if our HAL can do it for us.
    if (mOutput && mOutput->audioHwDev &&
        mOutput->audioHwDev->canSetMasterMute()) {
        mMasterMute = false;
    } else {
        mMasterMute = muted;
    }
}

void AudioFlinger::PlaybackThread::setStreamVolume(audio_stream_type_t stream, float value)
{
    Mutex::Autolock _l(mLock);
    mStreamTypes[stream].volume = value;
    broadcast_l();
}

void AudioFlinger::PlaybackThread::setStreamMute(audio_stream_type_t stream, bool muted)
{
    Mutex::Autolock _l(mLock);
    mStreamTypes[stream].mute = muted;
    broadcast_l();
}

float AudioFlinger::PlaybackThread::streamVolume(audio_stream_type_t stream) const
{
    Mutex::Autolock _l(mLock);
    return mStreamTypes[stream].volume;
}

void AudioFlinger::PlaybackThread::setVolumeForOutput_l(float left, float right) const
{
    mOutput->stream->setVolume(left, right);
}

// addTrack_l() must be called with ThreadBase::mLock held
status_t AudioFlinger::PlaybackThread::addTrack_l(const sp<Track>& track)
{
    status_t status = ALREADY_EXISTS;

    if (mActiveTracks.indexOf(track) < 0) {
        // the track is newly added, make sure it fills up all its
        // buffers before playing. This is to ensure the client will
        // effectively get the latency it requested.
        if (track->isExternalTrack()) {
            TrackBase::track_state state = track->mState;
            mLock.unlock();
            status = AudioSystem::startOutput(track->portId());
            mLock.lock();
            // abort track was stopped/paused while we released the lock
            if (state != track->mState) {
                if (status == NO_ERROR) {
                    mLock.unlock();
                    AudioSystem::stopOutput(track->portId());
                    mLock.lock();
                }
                return INVALID_OPERATION;
            }
            // abort if start is rejected by audio policy manager
            if (status != NO_ERROR) {
                return PERMISSION_DENIED;
            }
#ifdef ADD_BATTERY_DATA
            // to track the speaker usage
            addBatteryData(IMediaPlayerService::kBatteryDataAudioFlingerStart);
#endif
            sendIoConfigEvent_l(AUDIO_CLIENT_STARTED, track->creatorPid(), track->portId());
        }

        // set retry count for buffer fill
        if (track->isOffloaded()) {
            if (track->isStopping_1()) {
                track->mRetryCount = kMaxTrackStopRetriesOffload;
            } else {
                track->mRetryCount = kMaxTrackStartupRetriesOffload;
            }
            track->mFillingUpStatus = mStandby ? Track::FS_FILLING : Track::FS_FILLED;
        } else {
            track->mRetryCount = kMaxTrackStartupRetries;
            track->mFillingUpStatus =
                    track->sharedBuffer() != 0 ? Track::FS_FILLED : Track::FS_FILLING;
        }

        if ((track->channelMask() & AUDIO_CHANNEL_HAPTIC_ALL) != AUDIO_CHANNEL_NONE
                && mHapticChannelMask != AUDIO_CHANNEL_NONE) {
            // Unlock due to VibratorService will lock for this call and will
            // call Tracks.mute/unmute which also require thread's lock.
            mLock.unlock();
            const int intensity = AudioFlinger::onExternalVibrationStart(
                    track->getExternalVibration());
            mLock.lock();
            track->setHapticIntensity(static_cast<AudioMixer::haptic_intensity_t>(intensity));
            // Haptic playback should be enabled by vibrator service.
            if (track->getHapticPlaybackEnabled()) {
                // Disable haptic playback of all active track to ensure only
                // one track playing haptic if current track should play haptic.
                for (const auto &t : mActiveTracks) {
                    t->setHapticPlaybackEnabled(false);
                }
            }
        }

        track->mResetDone = false;
        track->mPresentationCompleteFrames = 0;
        mActiveTracks.add(track);
        sp<EffectChain> chain = getEffectChain_l(track->sessionId());
        if (chain != 0) {
            ALOGV("addTrack_l() starting track on chain %p for session %d", chain.get(),
                    track->sessionId());
            chain->incActiveTrackCnt();
        }

        status = NO_ERROR;
    }

    onAddNewTrack_l();
    return status;
}

bool AudioFlinger::PlaybackThread::destroyTrack_l(const sp<Track>& track)
{
    track->terminate();
    // active tracks are removed by threadLoop()
    bool trackActive = (mActiveTracks.indexOf(track) >= 0);
    track->mState = TrackBase::STOPPED;
    if (!trackActive) {
        removeTrack_l(track);
    } else if (track->isFastTrack() || track->isOffloaded() || track->isDirect()) {
        track->mState = TrackBase::STOPPING_1;
    }

    return trackActive;
}

void AudioFlinger::PlaybackThread::removeTrack_l(const sp<Track>& track)
{
    track->triggerEvents(AudioSystem::SYNC_EVENT_PRESENTATION_COMPLETE);

    String8 result;
    track->appendDump(result, false /* active */);
    mLocalLog.log("removeTrack_l (%p) %s", track.get(), result.string());

    mTracks.remove(track);
    if (track->isFastTrack()) {
        int index = track->mFastIndex;
        ALOG_ASSERT(0 < index && index < (int)FastMixerState::sMaxFastTracks);
        ALOG_ASSERT(!(mFastTrackAvailMask & (1 << index)));
        mFastTrackAvailMask |= 1 << index;
        // redundant as track is about to be destroyed, for dumpsys only
        track->mFastIndex = -1;
    }
    sp<EffectChain> chain = getEffectChain_l(track->sessionId());
    if (chain != 0) {
        chain->decTrackCnt();
    }
}

String8 AudioFlinger::PlaybackThread::getParameters(const String8& keys)
{
    Mutex::Autolock _l(mLock);
    String8 out_s8;
    if (initCheck() == NO_ERROR && mOutput->stream->getParameters(keys, &out_s8) == OK) {
        return out_s8;
    }
    return String8();
}

status_t AudioFlinger::DirectOutputThread::selectPresentation(int presentationId, int programId) {
    Mutex::Autolock _l(mLock);
    if (mOutput == nullptr || mOutput->stream == nullptr) {
        return NO_INIT;
    }
    return mOutput->stream->selectPresentation(presentationId, programId);
}

void AudioFlinger::PlaybackThread::ioConfigChanged(audio_io_config_event event, pid_t pid,
                                                   audio_port_handle_t portId) {
    sp<AudioIoDescriptor> desc = new AudioIoDescriptor();
    ALOGV("PlaybackThread::ioConfigChanged, thread %p, event %d", this, event);

    desc->mIoHandle = mId;

    switch (event) {
    case AUDIO_OUTPUT_OPENED:
    case AUDIO_OUTPUT_REGISTERED:
    case AUDIO_OUTPUT_CONFIG_CHANGED:
        desc->mPatch = mPatch;
        desc->mChannelMask = mChannelMask;
        desc->mSamplingRate = mSampleRate;
        desc->mFormat = mFormat;
        desc->mFrameCount = mNormalFrameCount; // FIXME see
                                             // AudioFlinger::frameCount(audio_io_handle_t)
        desc->mFrameCountHAL = mFrameCount;
        desc->mLatency = latency_l();
        break;
    case AUDIO_CLIENT_STARTED:
        desc->mPatch = mPatch;
        desc->mPortId = portId;
        break;
    case AUDIO_OUTPUT_CLOSED:
    default:
        break;
    }
    mAudioFlinger->ioConfigChanged(event, desc, pid);
}

void AudioFlinger::PlaybackThread::onWriteReady()
{
    mCallbackThread->resetWriteBlocked();
}

void AudioFlinger::PlaybackThread::onDrainReady()
{
    mCallbackThread->resetDraining();
}

void AudioFlinger::PlaybackThread::onError()
{
    mCallbackThread->setAsyncError();
}

void AudioFlinger::PlaybackThread::resetWriteBlocked(uint32_t sequence)
{
    Mutex::Autolock _l(mLock);
    // reject out of sequence requests
    if ((mWriteAckSequence & 1) && (sequence == mWriteAckSequence)) {
        mWriteAckSequence &= ~1;
        mWaitWorkCV.signal();
    }
}

void AudioFlinger::PlaybackThread::resetDraining(uint32_t sequence)
{
    Mutex::Autolock _l(mLock);
    // reject out of sequence requests
    if ((mDrainSequence & 1) && (sequence == mDrainSequence)) {
        // Register discontinuity when HW drain is completed because that can cause
        // the timestamp frame position to reset to 0 for direct and offload threads.
        // (Out of sequence requests are ignored, since the discontinuity would be handled
        // elsewhere, e.g. in flush).
        mTimestampVerifier.discontinuity();
        mDrainSequence &= ~1;
        mWaitWorkCV.signal();
    }
}

void AudioFlinger::PlaybackThread::readOutputParameters_l()
{
    // unfortunately we have no way of recovering from errors here, hence the LOG_ALWAYS_FATAL
    mSampleRate = mOutput->getSampleRate();
    mChannelMask = mOutput->getChannelMask();
    if (!audio_is_output_channel(mChannelMask)) {
        LOG_ALWAYS_FATAL("HAL channel mask %#x not valid for output", mChannelMask);
    }
    if ((mType == MIXER || mType == DUPLICATING)
            && !isValidPcmSinkChannelMask(mChannelMask)) {
        LOG_ALWAYS_FATAL("HAL channel mask %#x not supported for mixed output",
                mChannelMask);
    }
    mChannelCount = audio_channel_count_from_out_mask(mChannelMask);
    mBalance.setChannelMask(mChannelMask);

    // Get actual HAL format.
    status_t result = mOutput->stream->getFormat(&mHALFormat);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error when retrieving output stream format: %d", result);
    // Get format from the shim, which will be different than the HAL format
    // if playing compressed audio over HDMI passthrough.
    mFormat = mOutput->getFormat();
    if (!audio_is_valid_format(mFormat)) {
        LOG_ALWAYS_FATAL("HAL format %#x not valid for output", mFormat);
    }
    if ((mType == MIXER || mType == DUPLICATING)
            && !isValidPcmSinkFormat(mFormat)) {
        LOG_FATAL("HAL format %#x not supported for mixed output",
                mFormat);
    }
    mFrameSize = mOutput->getFrameSize();
    result = mOutput->stream->getBufferSize(&mBufferSize);
    LOG_ALWAYS_FATAL_IF(result != OK,
            "Error when retrieving output stream buffer size: %d", result);
    mFrameCount = mBufferSize / mFrameSize;
    if (mFrameCount & 15) {
        ALOGW("HAL output buffer size is %zu frames but AudioMixer requires multiples of 16 frames",
                mFrameCount);
    }

    if (mOutput->flags & AUDIO_OUTPUT_FLAG_NON_BLOCKING) {
        if (mOutput->stream->setCallback(this) == OK) {
            mUseAsyncWrite = true;
            mCallbackThread = new AudioFlinger::AsyncCallbackThread(this);
        }
    }

    mHwSupportsPause = false;
    if (mOutput->flags & AUDIO_OUTPUT_FLAG_DIRECT) {
        bool supportsPause = false, supportsResume = false;
        if (mOutput->stream->supportsPauseAndResume(&supportsPause, &supportsResume) == OK) {
            if (supportsPause && supportsResume) {
                mHwSupportsPause = true;
            } else if (supportsPause) {
                ALOGW("direct output implements pause but not resume");
            } else if (supportsResume) {
                ALOGW("direct output implements resume but not pause");
            }
        }
    }
    if (!mHwSupportsPause && mOutput->flags & AUDIO_OUTPUT_FLAG_HW_AV_SYNC) {
        LOG_ALWAYS_FATAL("HW_AV_SYNC requested but HAL does not implement pause and resume");
    }

    if (mType == DUPLICATING && mMixerBufferEnabled && mEffectBufferEnabled) {
        // For best precision, we use float instead of the associated output
        // device format (typically PCM 16 bit).

        mFormat = AUDIO_FORMAT_PCM_FLOAT;
        mFrameSize = mChannelCount * audio_bytes_per_sample(mFormat);
        mBufferSize = mFrameSize * mFrameCount;

        // TODO: We currently use the associated output device channel mask and sample rate.
        // (1) Perhaps use the ORed channel mask of all downstream MixerThreads
        // (if a valid mask) to avoid premature downmix.
        // (2) Perhaps use the maximum sample rate of all downstream MixerThreads
        // instead of the output device sample rate to avoid loss of high frequency information.
        // This may need to be updated as MixerThread/OutputTracks are added and not here.
    }

    // Calculate size of normal sink buffer relative to the HAL output buffer size
    double multiplier = 1.0;
    if (mType == MIXER && (kUseFastMixer == FastMixer_Static ||
            kUseFastMixer == FastMixer_Dynamic)) {
        size_t minNormalFrameCount = (kMinNormalSinkBufferSizeMs * mSampleRate) / 1000;
        size_t maxNormalFrameCount = (kMaxNormalSinkBufferSizeMs * mSampleRate) / 1000;

        // round up minimum and round down maximum to nearest 16 frames to satisfy AudioMixer
        minNormalFrameCount = (minNormalFrameCount + 15) & ~15;
        maxNormalFrameCount = maxNormalFrameCount & ~15;
        if (maxNormalFrameCount < minNormalFrameCount) {
            maxNormalFrameCount = minNormalFrameCount;
        }
        multiplier = (double) minNormalFrameCount / (double) mFrameCount;
        if (multiplier <= 1.0) {
            multiplier = 1.0;
        } else if (multiplier <= 2.0) {
            if (2 * mFrameCount <= maxNormalFrameCount) {
                multiplier = 2.0;
            } else {
                multiplier = (double) maxNormalFrameCount / (double) mFrameCount;
            }
        } else {
            multiplier = floor(multiplier);
        }
    }
    mNormalFrameCount = multiplier * mFrameCount;
    // round up to nearest 16 frames to satisfy AudioMixer
    if (mType == MIXER || mType == DUPLICATING) {
        mNormalFrameCount = (mNormalFrameCount + 15) & ~15;
    }
    ALOGI("HAL output buffer size %zu frames, normal sink buffer size %zu frames", mFrameCount,
            mNormalFrameCount);

    // Check if we want to throttle the processing to no more than 2x normal rate
    mThreadThrottle = property_get_bool("af.thread.throttle", true /* default_value */);
    mThreadThrottleTimeMs = 0;
    mThreadThrottleEndMs = 0;
    mHalfBufferMs = mNormalFrameCount * 1000 / (2 * mSampleRate);

    // mSinkBuffer is the sink buffer.  Size is always multiple-of-16 frames.
    // Originally this was int16_t[] array, need to remove legacy implications.
    free(mSinkBuffer);
    mSinkBuffer = NULL;
    // For sink buffer size, we use the frame size from the downstream sink to avoid problems
    // with non PCM formats for compressed music, e.g. AAC, and Offload threads.
    const size_t sinkBufferSize = mNormalFrameCount * mFrameSize;
    (void)posix_memalign(&mSinkBuffer, 32, sinkBufferSize);

    // We resize the mMixerBuffer according to the requirements of the sink buffer which
    // drives the output.
    free(mMixerBuffer);
    mMixerBuffer = NULL;
    if (mMixerBufferEnabled) {
        mMixerBufferFormat = AUDIO_FORMAT_PCM_FLOAT; // no longer valid: AUDIO_FORMAT_PCM_16_BIT.
        mMixerBufferSize = mNormalFrameCount * mChannelCount
                * audio_bytes_per_sample(mMixerBufferFormat);
        (void)posix_memalign(&mMixerBuffer, 32, mMixerBufferSize);
    }
    free(mEffectBuffer);
    mEffectBuffer = NULL;
    if (mEffectBufferEnabled) {
        mEffectBufferFormat = EFFECT_BUFFER_FORMAT;
        mEffectBufferSize = mNormalFrameCount * mChannelCount
                * audio_bytes_per_sample(mEffectBufferFormat);
        (void)posix_memalign(&mEffectBuffer, 32, mEffectBufferSize);
    }

    mHapticChannelMask = mChannelMask & AUDIO_CHANNEL_HAPTIC_ALL;
    mChannelMask &= ~mHapticChannelMask;
    mHapticChannelCount = audio_channel_count_from_out_mask(mHapticChannelMask);
    mChannelCount -= mHapticChannelCount;

    // force reconfiguration of effect chains and engines to take new buffer size and audio
    // parameters into account
    // Note that mLock is not held when readOutputParameters_l() is called from the constructor
    // but in this case nothing is done below as no audio sessions have effect yet so it doesn't
    // matter.
    // create a copy of mEffectChains as calling moveEffectChain_l() can reorder some effect chains
    Vector< sp<EffectChain> > effectChains = mEffectChains;
    for (size_t i = 0; i < effectChains.size(); i ++) {
        mAudioFlinger->moveEffectChain_l(effectChains[i]->sessionId(),
            this/* srcThread */, this/* dstThread */);
    }
}

void AudioFlinger::PlaybackThread::updateMetadata_l()
{
    if (mOutput == nullptr || mOutput->stream == nullptr ) {
        return; // That should not happen
    }
    bool hasChanged = mActiveTracks.readAndClearHasChanged();
    for (const sp<Track> &track : mActiveTracks) {
        // Do not short-circuit as all hasChanged states must be reset
        // as all the metadata are going to be sent
        hasChanged |= track->readAndClearHasChanged();
    }
    if (!hasChanged) {
        return; // nothing to do
    }
    StreamOutHalInterface::SourceMetadata metadata;
    auto backInserter = std::back_inserter(metadata.tracks);
    for (const sp<Track> &track : mActiveTracks) {
        // No track is invalid as this is called after prepareTrack_l in the same critical section
        track->copyMetadataTo(backInserter);
    }
    sendMetadataToBackend_l(metadata);
}

void AudioFlinger::PlaybackThread::sendMetadataToBackend_l(
        const StreamOutHalInterface::SourceMetadata& metadata)
{
    mOutput->stream->updateSourceMetadata(metadata);
};

status_t AudioFlinger::PlaybackThread::getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames)
{
    if (halFrames == NULL || dspFrames == NULL) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    if (initCheck() != NO_ERROR) {
        return INVALID_OPERATION;
    }
    int64_t framesWritten = mBytesWritten / mFrameSize;
    *halFrames = framesWritten;

    if (isSuspended()) {
        // return an estimation of rendered frames when the output is suspended
        size_t latencyFrames = (latency_l() * mSampleRate) / 1000;
        *dspFrames = (uint32_t)
                (framesWritten >= (int64_t)latencyFrames ? framesWritten - latencyFrames : 0);
        return NO_ERROR;
    } else {
        status_t status;
        uint32_t frames;
        status = mOutput->getRenderPosition(&frames);
        *dspFrames = (size_t)frames;
        return status;
    }
}

uint32_t AudioFlinger::PlaybackThread::getStrategyForSession_l(audio_session_t sessionId)
{
    // session AUDIO_SESSION_OUTPUT_MIX is placed in same strategy as MUSIC stream so that
    // it is moved to correct output by audio policy manager when A2DP is connected or disconnected
    if (sessionId == AUDIO_SESSION_OUTPUT_MIX) {
        return AudioSystem::getStrategyForStream(AUDIO_STREAM_MUSIC);
    }
    for (size_t i = 0; i < mTracks.size(); i++) {
        sp<Track> track = mTracks[i];
        if (sessionId == track->sessionId() && !track->isInvalid()) {
            return AudioSystem::getStrategyForStream(track->streamType());
        }
    }
    return AudioSystem::getStrategyForStream(AUDIO_STREAM_MUSIC);
}


AudioStreamOut* AudioFlinger::PlaybackThread::getOutput() const
{
    Mutex::Autolock _l(mLock);
    return mOutput;
}

AudioStreamOut* AudioFlinger::PlaybackThread::clearOutput()
{
    Mutex::Autolock _l(mLock);
    AudioStreamOut *output = mOutput;
    mOutput = NULL;
    // FIXME FastMixer might also have a raw ptr to mOutputSink;
    //       must push a NULL and wait for ack
    mOutputSink.clear();
    mPipeSink.clear();
    mNormalSink.clear();
    return output;
}

// this method must always be called either with ThreadBase mLock held or inside the thread loop
sp<StreamHalInterface> AudioFlinger::PlaybackThread::stream() const
{
    if (mOutput == NULL) {
        return NULL;
    }
    return mOutput->stream;
}

uint32_t AudioFlinger::PlaybackThread::activeSleepTimeUs() const
{
    return (uint32_t)((uint32_t)((mNormalFrameCount * 1000) / mSampleRate) * 1000);
}

status_t AudioFlinger::PlaybackThread::setSyncEvent(const sp<SyncEvent>& event)
{
    if (!isValidSyncEvent(event)) {
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    for (size_t i = 0; i < mTracks.size(); ++i) {
        sp<Track> track = mTracks[i];
        if (event->triggerSession() == track->sessionId()) {
            (void) track->setSyncEvent(event);
            return NO_ERROR;
        }
    }

    return NAME_NOT_FOUND;
}

bool AudioFlinger::PlaybackThread::isValidSyncEvent(const sp<SyncEvent>& event) const
{
    return event->type() == AudioSystem::SYNC_EVENT_PRESENTATION_COMPLETE;
}

void AudioFlinger::PlaybackThread::threadLoop_removeTracks(
        const Vector< sp<Track> >& tracksToRemove)
{
    // Miscellaneous track cleanup when removed from the active list,
    // called without Thread lock but synchronized with threadLoop processing.
#ifdef ADD_BATTERY_DATA
    for (const auto& track : tracksToRemove) {
        if (track->isExternalTrack()) {
            // to track the speaker usage
            addBatteryData(IMediaPlayerService::kBatteryDataAudioFlingerStop);
        }
    }
#else
    (void)tracksToRemove; // suppress unused warning
#endif
}

void AudioFlinger::PlaybackThread::checkSilentMode_l()
{
    if (!mMasterMute) {
        char value[PROPERTY_VALUE_MAX];
        if (mOutDevice == AUDIO_DEVICE_OUT_REMOTE_SUBMIX) {
            ALOGD("ro.audio.silent will be ignored for threads on AUDIO_DEVICE_OUT_REMOTE_SUBMIX");
            return;
        }
        if (property_get("ro.audio.silent", value, "0") > 0) {
            char *endptr;
            unsigned long ul = strtoul(value, &endptr, 0);
            if (*endptr == '\0' && ul != 0) {
                ALOGD("Silence is golden");
                // The setprop command will not allow a property to be changed after
                // the first time it is set, so we don't have to worry about un-muting.
                setMasterMute_l(true);
            }
        }
    }
}

// shared by MIXER and DIRECT, overridden by DUPLICATING
ssize_t AudioFlinger::PlaybackThread::threadLoop_write()
{
    LOG_HIST_TS();
    mInWrite = true;
    ssize_t bytesWritten;
    const size_t offset = mCurrentWriteLength - mBytesRemaining;

    // If an NBAIO sink is present, use it to write the normal mixer's submix
    if (mNormalSink != 0) {

        const size_t count = mBytesRemaining / mFrameSize;

        ATRACE_BEGIN("write");
        // update the setpoint when AudioFlinger::mScreenState changes
        uint32_t screenState = AudioFlinger::mScreenState;
        if (screenState != mScreenState) {
            mScreenState = screenState;
            MonoPipe *pipe = (MonoPipe *)mPipeSink.get();
            if (pipe != NULL) {
                pipe->setAvgFrames((mScreenState & 1) ?
                        (pipe->maxFrames() * 7) / 8 : mNormalFrameCount * 2);
            }
        }
        ssize_t framesWritten = mNormalSink->write((char *)mSinkBuffer + offset, count);
        ATRACE_END();
        if (framesWritten > 0) {
            bytesWritten = framesWritten * mFrameSize;
#ifdef TEE_SINK
            mTee.write((char *)mSinkBuffer + offset, framesWritten);
#endif
        } else {
            bytesWritten = framesWritten;
        }
    // otherwise use the HAL / AudioStreamOut directly
    } else {
        // Direct output and offload threads

        if (mUseAsyncWrite) {
            ALOGW_IF(mWriteAckSequence & 1, "threadLoop_write(): out of sequence write request");
            mWriteAckSequence += 2;
            mWriteAckSequence |= 1;
            ALOG_ASSERT(mCallbackThread != 0);
            mCallbackThread->setWriteBlocked(mWriteAckSequence);
        }
        // FIXME We should have an implementation of timestamps for direct output threads.
        // They are used e.g for multichannel PCM playback over HDMI.
        bytesWritten = mOutput->write((char *)mSinkBuffer + offset, mBytesRemaining);

        if (mUseAsyncWrite &&
                ((bytesWritten < 0) || (bytesWritten == (ssize_t)mBytesRemaining))) {
            // do not wait for async callback in case of error of full write
            mWriteAckSequence &= ~1;
            ALOG_ASSERT(mCallbackThread != 0);
            mCallbackThread->setWriteBlocked(mWriteAckSequence);
        }
    }

    mNumWrites++;
    mInWrite = false;
    mStandby = false;
    return bytesWritten;
}

void AudioFlinger::PlaybackThread::threadLoop_drain()
{
    bool supportsDrain = false;
    if (mOutput->stream->supportsDrain(&supportsDrain) == OK && supportsDrain) {
        ALOGV("draining %s", (mMixerStatus == MIXER_DRAIN_TRACK) ? "early" : "full");
        if (mUseAsyncWrite) {
            ALOGW_IF(mDrainSequence & 1, "threadLoop_drain(): out of sequence drain request");
            mDrainSequence |= 1;
            ALOG_ASSERT(mCallbackThread != 0);
            mCallbackThread->setDraining(mDrainSequence);
        }
        status_t result = mOutput->stream->drain(mMixerStatus == MIXER_DRAIN_TRACK);
        ALOGE_IF(result != OK, "Error when draining stream: %d", result);
    }
}

void AudioFlinger::PlaybackThread::threadLoop_exit()
{
    {
        Mutex::Autolock _l(mLock);
        for (size_t i = 0; i < mTracks.size(); i++) {
            sp<Track> track = mTracks[i];
            track->invalidate();
        }
        // Clear ActiveTracks to update BatteryNotifier in case active tracks remain.
        // After we exit there are no more track changes sent to BatteryNotifier
        // because that requires an active threadLoop.
        // TODO: should we decActiveTrackCnt() of the cleared track effect chain?
        mActiveTracks.clear();
    }
}

/*
The derived values that are cached:
 - mSinkBufferSize from frame count * frame size
 - mActiveSleepTimeUs from activeSleepTimeUs()
 - mIdleSleepTimeUs from idleSleepTimeUs()
 - mStandbyDelayNs from mActiveSleepTimeUs (DIRECT only) or forced to at least
   kDefaultStandbyTimeInNsecs when connected to an A2DP device.
 - maxPeriod from frame count and sample rate (MIXER only)

The parameters that affect these derived values are:
 - frame count
 - frame size
 - sample rate
 - device type: A2DP or not
 - device latency
 - format: PCM or not
 - active sleep time
 - idle sleep time
*/

void AudioFlinger::PlaybackThread::cacheParameters_l()
{
    mSinkBufferSize = mNormalFrameCount * mFrameSize;
    mActiveSleepTimeUs = activeSleepTimeUs();
    mIdleSleepTimeUs = idleSleepTimeUs();

    // make sure standby delay is not too short when connected to an A2DP sink to avoid
    // truncating audio when going to standby.
    mStandbyDelayNs = AudioFlinger::mStandbyTimeInNsecs;
    if ((mOutDevice & AUDIO_DEVICE_OUT_ALL_A2DP) != 0) {
        if (mStandbyDelayNs < kDefaultStandbyTimeInNsecs) {
            mStandbyDelayNs = kDefaultStandbyTimeInNsecs;
        }
    }
}

bool AudioFlinger::PlaybackThread::invalidateTracks_l(audio_stream_type_t streamType)
{
    ALOGV("MixerThread::invalidateTracks() mixer %p, streamType %d, mTracks.size %zu",
            this,  streamType, mTracks.size());
    bool trackMatch = false;
    size_t size = mTracks.size();
    for (size_t i = 0; i < size; i++) {
        sp<Track> t = mTracks[i];
        if (t->streamType() == streamType && t->isExternalTrack()) {
            t->invalidate();
            trackMatch = true;
        }
    }
    return trackMatch;
}

void AudioFlinger::PlaybackThread::invalidateTracks(audio_stream_type_t streamType)
{
    Mutex::Autolock _l(mLock);
    invalidateTracks_l(streamType);
}

status_t AudioFlinger::PlaybackThread::addEffectChain_l(const sp<EffectChain>& chain)
{
    audio_session_t session = chain->sessionId();
    sp<EffectBufferHalInterface> halInBuffer, halOutBuffer;
    status_t result = mAudioFlinger->mEffectsFactoryHal->mirrorBuffer(
            mEffectBufferEnabled ? mEffectBuffer : mSinkBuffer,
            mEffectBufferEnabled ? mEffectBufferSize : mSinkBufferSize,
            &halInBuffer);
    if (result != OK) return result;
    halOutBuffer = halInBuffer;
    effect_buffer_t *buffer = reinterpret_cast<effect_buffer_t*>(halInBuffer->externalData());
    ALOGV("addEffectChain_l() %p on thread %p for session %d", chain.get(), this, session);
    if (session > AUDIO_SESSION_OUTPUT_MIX) {
        // Only one effect chain can be present in direct output thread and it uses
        // the sink buffer as input
        if (mType != DIRECT) {
            size_t numSamples = mNormalFrameCount * (mChannelCount + mHapticChannelCount);
            status_t result = mAudioFlinger->mEffectsFactoryHal->allocateBuffer(
                    numSamples * sizeof(effect_buffer_t),
                    &halInBuffer);
            if (result != OK) return result;
#ifdef FLOAT_EFFECT_CHAIN
            buffer = halInBuffer->audioBuffer()->f32;
#else
            buffer = halInBuffer->audioBuffer()->s16;
#endif
            ALOGV("addEffectChain_l() creating new input buffer %p session %d",
                    buffer, session);
        }

        // Attach all tracks with same session ID to this chain.
        for (size_t i = 0; i < mTracks.size(); ++i) {
            sp<Track> track = mTracks[i];
            if (session == track->sessionId()) {
                ALOGV("addEffectChain_l() track->setMainBuffer track %p buffer %p", track.get(),
                        buffer);
                track->setMainBuffer(buffer);
                chain->incTrackCnt();
            }
        }

        // indicate all active tracks in the chain
        for (const sp<Track> &track : mActiveTracks) {
            if (session == track->sessionId()) {
                ALOGV("addEffectChain_l() activating track %p on session %d", track.get(), session);
                chain->incActiveTrackCnt();
            }
        }
    }
    chain->setThread(this);
    chain->setInBuffer(halInBuffer);
    chain->setOutBuffer(halOutBuffer);
    // Effect chain for session AUDIO_SESSION_OUTPUT_STAGE is inserted at end of effect
    // chains list in order to be processed last as it contains output stage effects.
    // Effect chain for session AUDIO_SESSION_OUTPUT_MIX is inserted before
    // session AUDIO_SESSION_OUTPUT_STAGE to be processed
    // after track specific effects and before output stage.
    // It is therefore mandatory that AUDIO_SESSION_OUTPUT_MIX == 0 and
    // that AUDIO_SESSION_OUTPUT_STAGE < AUDIO_SESSION_OUTPUT_MIX.
    // Effect chain for other sessions are inserted at beginning of effect
    // chains list to be processed before output mix effects. Relative order between other
    // sessions is not important.
    static_assert(AUDIO_SESSION_OUTPUT_MIX == 0 &&
            AUDIO_SESSION_OUTPUT_STAGE < AUDIO_SESSION_OUTPUT_MIX,
            "audio_session_t constants misdefined");
    size_t size = mEffectChains.size();
    size_t i = 0;
    for (i = 0; i < size; i++) {
        if (mEffectChains[i]->sessionId() < session) {
            break;
        }
    }
    mEffectChains.insertAt(chain, i);
    checkSuspendOnAddEffectChain_l(chain);

    return NO_ERROR;
}

size_t AudioFlinger::PlaybackThread::removeEffectChain_l(const sp<EffectChain>& chain)
{
    audio_session_t session = chain->sessionId();

    ALOGV("removeEffectChain_l() %p from thread %p for session %d", chain.get(), this, session);

    for (size_t i = 0; i < mEffectChains.size(); i++) {
        if (chain == mEffectChains[i]) {
            mEffectChains.removeAt(i);
            // detach all active tracks from the chain
            for (const sp<Track> &track : mActiveTracks) {
                if (session == track->sessionId()) {
                    ALOGV("removeEffectChain_l(): stopping track on chain %p for session Id: %d",
                            chain.get(), session);
                    chain->decActiveTrackCnt();
                }
            }

            // detach all tracks with same session ID from this chain
            for (size_t i = 0; i < mTracks.size(); ++i) {
                sp<Track> track = mTracks[i];
                if (session == track->sessionId()) {
                    track->setMainBuffer(reinterpret_cast<effect_buffer_t*>(mSinkBuffer));
                    chain->decTrackCnt();
                }
            }
            break;
        }
    }
    return mEffectChains.size();
}

status_t AudioFlinger::PlaybackThread::attachAuxEffect(
        const sp<AudioFlinger::PlaybackThread::Track>& track, int EffectId)
{
    Mutex::Autolock _l(mLock);
    return attachAuxEffect_l(track, EffectId);
}

status_t AudioFlinger::PlaybackThread::attachAuxEffect_l(
        const sp<AudioFlinger::PlaybackThread::Track>& track, int EffectId)
{
    status_t status = NO_ERROR;

    if (EffectId == 0) {
        track->setAuxBuffer(0, NULL);
    } else {
        // Auxiliary effects are always in audio session AUDIO_SESSION_OUTPUT_MIX
        sp<EffectModule> effect = getEffect_l(AUDIO_SESSION_OUTPUT_MIX, EffectId);
        if (effect != 0) {
            if ((effect->desc().flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
                track->setAuxBuffer(EffectId, (int32_t *)effect->inBuffer());
            } else {
                status = INVALID_OPERATION;
            }
        } else {
            status = BAD_VALUE;
        }
    }
    return status;
}

void AudioFlinger::PlaybackThread::detachAuxEffect_l(int effectId)
{
    for (size_t i = 0; i < mTracks.size(); ++i) {
        sp<Track> track = mTracks[i];
        if (track->auxEffectId() == effectId) {
            attachAuxEffect_l(track, 0);
        }
    }
}

bool AudioFlinger::PlaybackThread::threadLoop()
{
    tlNBLogWriter = mNBLogWriter.get();

    Vector< sp<Track> > tracksToRemove;

    mStandbyTimeNs = systemTime();
    int64_t lastLoopCountWritten = -2; // never matches "previous" loop, when loopCount = 0.
    int64_t lastFramesWritten = -1;    // track changes in timestamp server frames written

    // MIXER
    nsecs_t lastWarning = 0;

    // DUPLICATING
    // FIXME could this be made local to while loop?
    writeFrames = 0;

    cacheParameters_l();
    mSleepTimeUs = mIdleSleepTimeUs;

    if (mType == MIXER) {
        sleepTimeShift = 0;
    }

    CpuStats cpuStats;
    const String8 myName(String8::format("thread %p type %d TID %d", this, mType, gettid()));

    acquireWakeLock();

    // mNBLogWriter logging APIs can only be called by a single thread, typically the
    // thread associated with this PlaybackThread.
    // If you want to share the mNBLogWriter with other threads (for example, binder threads)
    // then all such threads must agree to hold a common mutex before logging.
    // So if you need to log when mutex is unlocked, set logString to a non-NULL string,
    // and then that string will be logged at the next convenient opportunity.
    // See reference to logString below.
    const char *logString = NULL;

    // Estimated time for next buffer to be written to hal. This is used only on
    // suspended mode (for now) to help schedule the wait time until next iteration.
    nsecs_t timeLoopNextNs = 0;

    checkSilentMode_l();

    // DIRECT and OFFLOAD threads should reset frame count to zero on stop/flush
    // TODO: add confirmation checks:
    // 1) DIRECT threads and linear PCM format really resets to 0?
    // 2) Is frame count really valid if not linear pcm?
    // 3) Are all 64 bits of position returned, not just lowest 32 bits?
    if (mType == OFFLOAD || mType == DIRECT) {
        mTimestampVerifier.setDiscontinuityMode(mTimestampVerifier.DISCONTINUITY_MODE_ZERO);
    }
    audio_patch_handle_t lastDownstreamPatchHandle = AUDIO_PATCH_HANDLE_NONE;

    // loopCount is used for statistics and diagnostics.
    for (int64_t loopCount = 0; !exitPending(); ++loopCount)
    {
        // Log merge requests are performed during AudioFlinger binder transactions, but
        // that does not cover audio playback. It's requested here for that reason.
        mAudioFlinger->requestLogMerge();

        cpuStats.sample(myName);

        Vector< sp<EffectChain> > effectChains;
        audio_session_t activeHapticSessionId = AUDIO_SESSION_NONE;
        std::vector<sp<Track>> activeTracks;

        // If the device is AUDIO_DEVICE_OUT_BUS, check for downstream latency.
        //
        // Note: we access outDevice() outside of mLock.
        if (isMsdDevice() && (outDevice() & AUDIO_DEVICE_OUT_BUS) != 0) {
            // Here, we try for the AF lock, but do not block on it as the latency
            // is more informational.
            if (mAudioFlinger->mLock.tryLock() == NO_ERROR) {
                std::vector<PatchPanel::SoftwarePatch> swPatches;
                double latencyMs;
                status_t status = INVALID_OPERATION;
                audio_patch_handle_t downstreamPatchHandle = AUDIO_PATCH_HANDLE_NONE;
                if (mAudioFlinger->mPatchPanel.getDownstreamSoftwarePatches(id(), &swPatches) == OK
                        && swPatches.size() > 0) {
                        status = swPatches[0].getLatencyMs_l(&latencyMs);
                        downstreamPatchHandle = swPatches[0].getPatchHandle();
                }
                if (downstreamPatchHandle != lastDownstreamPatchHandle) {
                    mDownstreamLatencyStatMs.reset();
                    lastDownstreamPatchHandle = downstreamPatchHandle;
                }
                if (status == OK) {
                    // verify downstream latency (we assume a max reasonable
                    // latency of 5 seconds).
                    const double minLatency = 0., maxLatency = 5000.;
                    if (latencyMs >= minLatency && latencyMs <= maxLatency) {
                        ALOGV("new downstream latency %lf ms", latencyMs);
                    } else {
                        ALOGD("out of range downstream latency %lf ms", latencyMs);
                        if (latencyMs < minLatency) latencyMs = minLatency;
                        else if (latencyMs > maxLatency) latencyMs = maxLatency;
                    }
                    mDownstreamLatencyStatMs.add(latencyMs);
                }
                mAudioFlinger->mLock.unlock();
            }
        } else {
            if (lastDownstreamPatchHandle != AUDIO_PATCH_HANDLE_NONE) {
                // our device is no longer AUDIO_DEVICE_OUT_BUS, reset patch handle and stats.
                mDownstreamLatencyStatMs.reset();
                lastDownstreamPatchHandle = AUDIO_PATCH_HANDLE_NONE;
            }
        }

        { // scope for mLock

            Mutex::Autolock _l(mLock);

            processConfigEvents_l();

            // See comment at declaration of logString for why this is done under mLock
            if (logString != NULL) {
                mNBLogWriter->logTimestamp();
                mNBLogWriter->log(logString);
                logString = NULL;
            }

            // Collect timestamp statistics for the Playback Thread types that support it.
            if (mType == MIXER
                    || mType == DUPLICATING
                    || mType == DIRECT
                    || mType == OFFLOAD) { // no indentation
            // Gather the framesReleased counters for all active tracks,
            // and associate with the sink frames written out.  We need
            // this to convert the sink timestamp to the track timestamp.
            bool kernelLocationUpdate = false;
            ExtendedTimestamp timestamp; // use private copy to fetch
            if (mStandby) {
                mTimestampVerifier.discontinuity();
            } else if (threadloop_getHalTimestamp_l(&timestamp) == OK) {
                mTimestampVerifier.add(timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL],
                        timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL],
                        mSampleRate);

                if (isTimestampCorrectionEnabled()) {
                    ALOGV("TS_BEFORE: %d %lld %lld", id(),
                            (long long)timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL],
                            (long long)timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]);
                    auto correctedTimestamp = mTimestampVerifier.getLastCorrectedTimestamp();
                    timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]
                            = correctedTimestamp.mFrames;
                    timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL]
                            = correctedTimestamp.mTimeNs;
                    ALOGV("TS_AFTER: %d %lld %lld", id(),
                            (long long)timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL],
                            (long long)timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]);

                    // Note: Downstream latency only added if timestamp correction enabled.
                    if (mDownstreamLatencyStatMs.getN() > 0) { // we have latency info.
                        const int64_t newPosition =
                                timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]
                                - int64_t(mDownstreamLatencyStatMs.getMean() * mSampleRate * 1e-3);
                        // prevent retrograde
                        timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] = max(
                                newPosition,
                                (mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]
                                        - mSuspendedFrames));
                    }
                }

                // We always fetch the timestamp here because often the downstream
                // sink will block while writing.

                // We keep track of the last valid kernel position in case we are in underrun
                // and the normal mixer period is the same as the fast mixer period, or there
                // is some error from the HAL.
                if (mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] >= 0) {
                    mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK] =
                            mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL];
                    mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK] =
                            mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];

                    mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK] =
                            mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER];
                    mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK] =
                            mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER];
                }

                if (timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] >= 0) {
                    kernelLocationUpdate = true;
                } else {
                    ALOGVV("getTimestamp error - no valid kernel position");
                }

                // copy over kernel info
                mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] =
                        timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL]
                        + mSuspendedFrames; // add frames discarded when suspended
                mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] =
                        timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];
            } else {
                mTimestampVerifier.error();
            }

            // mFramesWritten for non-offloaded tracks are contiguous
            // even after standby() is called. This is useful for the track frame
            // to sink frame mapping.
            bool serverLocationUpdate = false;
            if (mFramesWritten != lastFramesWritten) {
                serverLocationUpdate = true;
                lastFramesWritten = mFramesWritten;
            }
            // Only update timestamps if there is a meaningful change.
            // Either the kernel timestamp must be valid or we have written something.
            if (kernelLocationUpdate || serverLocationUpdate) {
                if (serverLocationUpdate) {
                    // use the time before we called the HAL write - it is a bit more accurate
                    // to when the server last read data than the current time here.
                    //
                    // If we haven't written anything, mLastIoBeginNs will be -1
                    // and we use systemTime().
                    mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER] = mFramesWritten;
                    mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] = mLastIoBeginNs == -1
                            ? systemTime() : mLastIoBeginNs;
                }

                for (const sp<Track> &t : mActiveTracks) {
                    if (!t->isFastTrack()) {
                        t->updateTrackFrameInfo(
                                t->mAudioTrackServerProxy->framesReleased(),
                                mFramesWritten,
                                mSampleRate,
                                mTimestamp);
                    }
                }
            }

            if (audio_has_proportional_frames(mFormat)) {
                const double latencyMs = mTimestamp.getOutputServerLatencyMs(mSampleRate);
                if (latencyMs != 0.) { // note 0. means timestamp is empty.
                    mLatencyMs.add(latencyMs);
                }
            }

            } // if (mType ... ) { // no indentation
#if 0
            // logFormat example
            if (z % 100 == 0) {
                timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                LOGT("This is an integer %d, this is a float %f, this is my "
                    "pid %p %% %s %t", 42, 3.14, "and this is a timestamp", ts);
                LOGT("A deceptive null-terminated string %\0");
            }
            ++z;
#endif
            saveOutputTracks();
            if (mSignalPending) {
                // A signal was raised while we were unlocked
                mSignalPending = false;
            } else if (waitingAsyncCallback_l()) {
                if (exitPending()) {
                    break;
                }
                bool released = false;
                if (!keepWakeLock()) {
                    releaseWakeLock_l();
                    released = true;
                }

                const int64_t waitNs = computeWaitTimeNs_l();
                ALOGV("wait async completion (wait time: %lld)", (long long)waitNs);
                status_t status = mWaitWorkCV.waitRelative(mLock, waitNs);
                if (status == TIMED_OUT) {
                    mSignalPending = true; // if timeout recheck everything
                }
                ALOGV("async completion/wake");
                if (released) {
                    acquireWakeLock_l();
                }
                mStandbyTimeNs = systemTime() + mStandbyDelayNs;
                mSleepTimeUs = 0;

                continue;
            }
            if ((mActiveTracks.isEmpty() && systemTime() > mStandbyTimeNs) ||
                                   isSuspended()) {
                // put audio hardware into standby after short delay
                if (shouldStandby_l()) {

                    threadLoop_standby();

                    // This is where we go into standby
                    if (!mStandby) {
                        LOG_AUDIO_STATE();
                    }
                    mStandby = true;
                    sendStatistics(false /* force */);
                }

                if (mActiveTracks.isEmpty() && mConfigEvents.isEmpty()) {
                    // we're about to wait, flush the binder command buffer
                    IPCThreadState::self()->flushCommands();

                    clearOutputTracks();

                    if (exitPending()) {
                        break;
                    }

                    releaseWakeLock_l();
                    // wait until we have something to do...
                    ALOGV("%s going to sleep", myName.string());
                    mWaitWorkCV.wait(mLock);
                    ALOGV("%s waking up", myName.string());
                    acquireWakeLock_l();

                    mMixerStatus = MIXER_IDLE;
                    mMixerStatusIgnoringFastTracks = MIXER_IDLE;
                    mBytesWritten = 0;
                    mBytesRemaining = 0;
                    checkSilentMode_l();

                    mStandbyTimeNs = systemTime() + mStandbyDelayNs;
                    mSleepTimeUs = mIdleSleepTimeUs;
                    if (mType == MIXER) {
                        sleepTimeShift = 0;
                    }

                    continue;
                }
            }
            // mMixerStatusIgnoringFastTracks is also updated internally
            mMixerStatus = prepareTracks_l(&tracksToRemove);

            mActiveTracks.updatePowerState(this);

            updateMetadata_l();

            // prevent any changes in effect chain list and in each effect chain
            // during mixing and effect process as the audio buffers could be deleted
            // or modified if an effect is created or deleted
            lockEffectChains_l(effectChains);

            // Determine which session to pick up haptic data.
            // This must be done under the same lock as prepareTracks_l().
            // TODO: Write haptic data directly to sink buffer when mixing.
            if (mHapticChannelCount > 0 && effectChains.size() > 0) {
                for (const auto& track : mActiveTracks) {
                    if (track->getHapticPlaybackEnabled()) {
                        activeHapticSessionId = track->sessionId();
                        break;
                    }
                }
            }

            // Acquire a local copy of active tracks with lock (release w/o lock).
            //
            // Control methods on the track acquire the ThreadBase lock (e.g. start()
            // stop(), pause(), etc.), but the threadLoop is entitled to call audio
            // data / buffer methods on tracks from activeTracks without the ThreadBase lock.
            activeTracks.insert(activeTracks.end(), mActiveTracks.begin(), mActiveTracks.end());
        } // mLock scope ends

        if (mBytesRemaining == 0) {
            mCurrentWriteLength = 0;
            if (mMixerStatus == MIXER_TRACKS_READY) {
                // threadLoop_mix() sets mCurrentWriteLength
                threadLoop_mix();
            } else if ((mMixerStatus != MIXER_DRAIN_TRACK)
                        && (mMixerStatus != MIXER_DRAIN_ALL)) {
                // threadLoop_sleepTime sets mSleepTimeUs to 0 if data
                // must be written to HAL
                threadLoop_sleepTime();
                if (mSleepTimeUs == 0) {
                    mCurrentWriteLength = mSinkBufferSize;

                    // Tally underrun frames as we are inserting 0s here.
                    for (const auto& track : activeTracks) {
                        if (track->mFillingUpStatus == Track::FS_ACTIVE) {
                            track->mAudioTrackServerProxy->tallyUnderrunFrames(mNormalFrameCount);
                        }
                    }
                }
            }
            // Either threadLoop_mix() or threadLoop_sleepTime() should have set
            // mMixerBuffer with data if mMixerBufferValid is true and mSleepTimeUs == 0.
            // Merge mMixerBuffer data into mEffectBuffer (if any effects are valid)
            // or mSinkBuffer (if there are no effects).
            //
            // This is done pre-effects computation; if effects change to
            // support higher precision, this needs to move.
            //
            // mMixerBufferValid is only set true by MixerThread::prepareTracks_l().
            // TODO use mSleepTimeUs == 0 as an additional condition.
            if (mMixerBufferValid) {
                void *buffer = mEffectBufferValid ? mEffectBuffer : mSinkBuffer;
                audio_format_t format = mEffectBufferValid ? mEffectBufferFormat : mFormat;

                // mono blend occurs for mixer threads only (not direct or offloaded)
                // and is handled here if we're going directly to the sink.
                if (requireMonoBlend() && !mEffectBufferValid) {
                    mono_blend(mMixerBuffer, mMixerBufferFormat, mChannelCount, mNormalFrameCount,
                               true /*limit*/);
                }

                if (!hasFastMixer()) {
                    // Balance must take effect after mono conversion.
                    // We do it here if there is no FastMixer.
                    // mBalance detects zero balance within the class for speed (not needed here).
                    mBalance.setBalance(mMasterBalance.load());
                    mBalance.process((float *)mMixerBuffer, mNormalFrameCount);
                }

                memcpy_by_audio_format(buffer, format, mMixerBuffer, mMixerBufferFormat,
                        mNormalFrameCount * (mChannelCount + mHapticChannelCount));

                // If we're going directly to the sink and there are haptic channels,
                // we should adjust channels as the sample data is partially interleaved
                // in this case.
                if (!mEffectBufferValid && mHapticChannelCount > 0) {
                    adjust_channels_non_destructive(buffer, mChannelCount, buffer,
                            mChannelCount + mHapticChannelCount,
                            audio_bytes_per_sample(format),
                            audio_bytes_per_frame(mChannelCount, format) * mNormalFrameCount);
                }
            }

            mBytesRemaining = mCurrentWriteLength;
            if (isSuspended()) {
                // Simulate write to HAL when suspended (e.g. BT SCO phone call).
                mSleepTimeUs = suspendSleepTimeUs(); // assumes full buffer.
                const size_t framesRemaining = mBytesRemaining / mFrameSize;
                mBytesWritten += mBytesRemaining;
                mFramesWritten += framesRemaining;
                mSuspendedFrames += framesRemaining; // to adjust kernel HAL position
                mBytesRemaining = 0;
            }

            // only process effects if we're going to write
            if (mSleepTimeUs == 0 && mType != OFFLOAD) {
                for (size_t i = 0; i < effectChains.size(); i ++) {
                    effectChains[i]->process_l();
                    // TODO: Write haptic data directly to sink buffer when mixing.
                    if (activeHapticSessionId != AUDIO_SESSION_NONE
                            && activeHapticSessionId == effectChains[i]->sessionId()) {
                        // Haptic data is active in this case, copy it directly from
                        // in buffer to out buffer.
                        const size_t audioBufferSize = mNormalFrameCount
                                * audio_bytes_per_frame(mChannelCount, EFFECT_BUFFER_FORMAT);
                        memcpy_by_audio_format(
                                (uint8_t*)effectChains[i]->outBuffer() + audioBufferSize,
                                EFFECT_BUFFER_FORMAT,
                                (const uint8_t*)effectChains[i]->inBuffer() + audioBufferSize,
                                EFFECT_BUFFER_FORMAT, mNormalFrameCount * mHapticChannelCount);
                    }
                }
            }
        }
        // Process effect chains for offloaded thread even if no audio
        // was read from audio track: process only updates effect state
        // and thus does have to be synchronized with audio writes but may have
        // to be called while waiting for async write callback
        if (mType == OFFLOAD) {
            for (size_t i = 0; i < effectChains.size(); i ++) {
                effectChains[i]->process_l();
            }
        }

        // Only if the Effects buffer is enabled and there is data in the
        // Effects buffer (buffer valid), we need to
        // copy into the sink buffer.
        // TODO use mSleepTimeUs == 0 as an additional condition.
        if (mEffectBufferValid) {
            //ALOGV("writing effect buffer to sink buffer format %#x", mFormat);

            if (requireMonoBlend()) {
                mono_blend(mEffectBuffer, mEffectBufferFormat, mChannelCount, mNormalFrameCount,
                           true /*limit*/);
            }

            if (!hasFastMixer()) {
                // Balance must take effect after mono conversion.
                // We do it here if there is no FastMixer.
                // mBalance detects zero balance within the class for speed (not needed here).
                mBalance.setBalance(mMasterBalance.load());
                mBalance.process((float *)mEffectBuffer, mNormalFrameCount);
            }

            memcpy_by_audio_format(mSinkBuffer, mFormat, mEffectBuffer, mEffectBufferFormat,
                    mNormalFrameCount * (mChannelCount + mHapticChannelCount));
            // The sample data is partially interleaved when haptic channels exist,
            // we need to adjust channels here.
            if (mHapticChannelCount > 0) {
                adjust_channels_non_destructive(mSinkBuffer, mChannelCount, mSinkBuffer,
                        mChannelCount + mHapticChannelCount,
                        audio_bytes_per_sample(mFormat),
                        audio_bytes_per_frame(mChannelCount, mFormat) * mNormalFrameCount);
            }
        }

        // enable changes in effect chain
        unlockEffectChains(effectChains);

        if (!waitingAsyncCallback()) {
            // mSleepTimeUs == 0 means we must write to audio hardware
            if (mSleepTimeUs == 0) {
                ssize_t ret = 0;
                // writePeriodNs is updated >= 0 when ret > 0.
                int64_t writePeriodNs = -1;
                if (mBytesRemaining) {
                    // FIXME rewrite to reduce number of system calls
                    const int64_t lastIoBeginNs = systemTime();
                    ret = threadLoop_write();
                    const int64_t lastIoEndNs = systemTime();
                    if (ret < 0) {
                        mBytesRemaining = 0;
                    } else if (ret > 0) {
                        mBytesWritten += ret;
                        mBytesRemaining -= ret;
                        const int64_t frames = ret / mFrameSize;
                        mFramesWritten += frames;

                        writePeriodNs = lastIoEndNs - mLastIoEndNs;
                        // process information relating to write time.
                        if (audio_has_proportional_frames(mFormat)) {
                            // we are in a continuous mixing cycle
                            if (mMixerStatus == MIXER_TRACKS_READY &&
                                    loopCount == lastLoopCountWritten + 1) {

                                const double jitterMs =
                                        TimestampVerifier<int64_t, int64_t>::computeJitterMs(
                                                {frames, writePeriodNs},
                                                {0, 0} /* lastTimestamp */, mSampleRate);
                                const double processMs =
                                       (lastIoBeginNs - mLastIoEndNs) * 1e-6;

                                Mutex::Autolock _l(mLock);
                                mIoJitterMs.add(jitterMs);
                                mProcessTimeMs.add(processMs);
                            }

                            // write blocked detection
                            const int64_t deltaWriteNs = lastIoEndNs - lastIoBeginNs;
                            if (mType == MIXER && deltaWriteNs > maxPeriod) {
                                mNumDelayedWrites++;
                                if ((lastIoEndNs - lastWarning) > kWarningThrottleNs) {
                                    ATRACE_NAME("underrun");
                                    ALOGW("write blocked for %lld msecs, "
                                            "%d delayed writes, thread %d",
                                            (long long)deltaWriteNs / NANOS_PER_MILLISECOND,
                                            mNumDelayedWrites, mId);
                                    lastWarning = lastIoEndNs;
                                }
                            }
                        }
                        // update timing info.
                        mLastIoBeginNs = lastIoBeginNs;
                        mLastIoEndNs = lastIoEndNs;
                        lastLoopCountWritten = loopCount;
                    }
                } else if ((mMixerStatus == MIXER_DRAIN_TRACK) ||
                        (mMixerStatus == MIXER_DRAIN_ALL)) {
                    threadLoop_drain();
                }
                if (mType == MIXER && !mStandby) {

                    if (mThreadThrottle
                            && mMixerStatus == MIXER_TRACKS_READY // we are mixing (active tracks)
                            && writePeriodNs > 0) {               // we have write period info
                        // Limit MixerThread data processing to no more than twice the
                        // expected processing rate.
                        //
                        // This helps prevent underruns with NuPlayer and other applications
                        // which may set up buffers that are close to the minimum size, or use
                        // deep buffers, and rely on a double-buffering sleep strategy to fill.
                        //
                        // The throttle smooths out sudden large data drains from the device,
                        // e.g. when it comes out of standby, which often causes problems with
                        // (1) mixer threads without a fast mixer (which has its own warm-up)
                        // (2) minimum buffer sized tracks (even if the track is full,
                        //     the app won't fill fast enough to handle the sudden draw).
                        //
                        // Total time spent in last processing cycle equals time spent in
                        // 1. threadLoop_write, as well as time spent in
                        // 2. threadLoop_mix (significant for heavy mixing, especially
                        //                    on low tier processors)

                        // it's OK if deltaMs is an overestimate.

                        const int32_t deltaMs = writePeriodNs / NANOS_PER_MILLISECOND;

                        const int32_t throttleMs = (int32_t)mHalfBufferMs - deltaMs;
                        if ((signed)mHalfBufferMs >= throttleMs && throttleMs > 0) {
                            usleep(throttleMs * 1000);
                            // notify of throttle start on verbose log
                            ALOGV_IF(mThreadThrottleEndMs == mThreadThrottleTimeMs,
                                    "mixer(%p) throttle begin:"
                                    " ret(%zd) deltaMs(%d) requires sleep %d ms",
                                    this, ret, deltaMs, throttleMs);
                            mThreadThrottleTimeMs += throttleMs;
                            // Throttle must be attributed to the previous mixer loop's write time
                            // to allow back-to-back throttling.
                            // This also ensures proper timing statistics.
                            mLastIoEndNs = systemTime();  // we fetch the write end time again.
                        } else {
                            uint32_t diff = mThreadThrottleTimeMs - mThreadThrottleEndMs;
                            if (diff > 0) {
                                // notify of throttle end on debug log
                                // but prevent spamming for bluetooth
                                ALOGD_IF(!audio_is_a2dp_out_device(outDevice()) &&
                                         !audio_is_hearing_aid_out_device(outDevice()),
                                        "mixer(%p) throttle end: throttle time(%u)", this, diff);
                                mThreadThrottleEndMs = mThreadThrottleTimeMs;
                            }
                        }
                    }
                }

            } else {
                ATRACE_BEGIN("sleep");
                Mutex::Autolock _l(mLock);
                // suspended requires accurate metering of sleep time.
                if (isSuspended()) {
                    // advance by expected sleepTime
                    timeLoopNextNs += microseconds((nsecs_t)mSleepTimeUs);
                    const nsecs_t nowNs = systemTime();

                    // compute expected next time vs current time.
                    // (negative deltas are treated as delays).
                    nsecs_t deltaNs = timeLoopNextNs - nowNs;
                    if (deltaNs < -kMaxNextBufferDelayNs) {
                        // Delays longer than the max allowed trigger a reset.
                        ALOGV("DelayNs: %lld, resetting timeLoopNextNs", (long long) deltaNs);
                        deltaNs = microseconds((nsecs_t)mSleepTimeUs);
                        timeLoopNextNs = nowNs + deltaNs;
                    } else if (deltaNs < 0) {
                        // Delays within the max delay allowed: zero the delta/sleepTime
                        // to help the system catch up in the next iteration(s)
                        ALOGV("DelayNs: %lld, catching-up", (long long) deltaNs);
                        deltaNs = 0;
                    }
                    // update sleep time (which is >= 0)
                    mSleepTimeUs = deltaNs / 1000;
                }
                if (!mSignalPending && mConfigEvents.isEmpty() && !exitPending()) {
                    mWaitWorkCV.waitRelative(mLock, microseconds((nsecs_t)mSleepTimeUs));
                }
                ATRACE_END();
            }
        }

        // Finally let go of removed track(s), without the lock held
        // since we can't guarantee the destructors won't acquire that
        // same lock.  This will also mutate and push a new fast mixer state.
        threadLoop_removeTracks(tracksToRemove);
        tracksToRemove.clear();

        // FIXME I don't understand the need for this here;
        //       it was in the original code but maybe the
        //       assignment in saveOutputTracks() makes this unnecessary?
        clearOutputTracks();

        // Effect chains will be actually deleted here if they were removed from
        // mEffectChains list during mixing or effects processing
        effectChains.clear();

        // FIXME Note that the above .clear() is no longer necessary since effectChains
        // is now local to this block, but will keep it for now (at least until merge done).
    }

    threadLoop_exit();

    if (!mStandby) {
        threadLoop_standby();
        mStandby = true;
    }

    releaseWakeLock();

    ALOGV("Thread %p type %d exiting", this, mType);
    return false;
}

// removeTracks_l() must be called with ThreadBase::mLock held
void AudioFlinger::PlaybackThread::removeTracks_l(const Vector< sp<Track> >& tracksToRemove)
{
    for (const auto& track : tracksToRemove) {
        mActiveTracks.remove(track);
        ALOGV("%s(%d): removing track on session %d", __func__, track->id(), track->sessionId());
        sp<EffectChain> chain = getEffectChain_l(track->sessionId());
        if (chain != 0) {
            ALOGV("%s(%d): stopping track on chain %p for session Id: %d",
                    __func__, track->id(), chain.get(), track->sessionId());
            chain->decActiveTrackCnt();
        }
        // If an external client track, inform APM we're no longer active, and remove if needed.
        // We do this under lock so that the state is consistent if the Track is destroyed.
        if (track->isExternalTrack()) {
            AudioSystem::stopOutput(track->portId());
            if (track->isTerminated()) {
                AudioSystem::releaseOutput(track->portId());
            }
        }
        if (track->isTerminated()) {
            // remove from our tracks vector
            removeTrack_l(track);
        }
        if ((track->channelMask() & AUDIO_CHANNEL_HAPTIC_ALL) != AUDIO_CHANNEL_NONE
                && mHapticChannelCount > 0) {
            mLock.unlock();
            // Unlock due to VibratorService will lock for this call and will
            // call Tracks.mute/unmute which also require thread's lock.
            AudioFlinger::onExternalVibrationStop(track->getExternalVibration());
            mLock.lock();
        }
    }
}

status_t AudioFlinger::PlaybackThread::getTimestamp_l(AudioTimestamp& timestamp)
{
    if (mNormalSink != 0) {
        ExtendedTimestamp ets;
        status_t status = mNormalSink->getTimestamp(ets);
        if (status == NO_ERROR) {
            status = ets.getBestTimestamp(&timestamp);
        }
        return status;
    }
    if ((mType == OFFLOAD || mType == DIRECT) && mOutput != NULL) {
        uint64_t position64;
        if (mOutput->getPresentationPosition(&position64, &timestamp.mTime) == OK) {
            timestamp.mPosition = (uint32_t)position64;
            if (mDownstreamLatencyStatMs.getN() > 0) {
                const uint32_t positionOffset =
                    (uint32_t)(mDownstreamLatencyStatMs.getMean() * mSampleRate * 1e-3);
                if (positionOffset > timestamp.mPosition) {
                    timestamp.mPosition = 0;
                } else {
                    timestamp.mPosition -= positionOffset;
                }
            }
            return NO_ERROR;
        }
    }
    return INVALID_OPERATION;
}

// For dedicated VoIP outputs, let the HAL apply the stream volume. Track volume is
// still applied by the mixer.
// All tracks attached to a mixer with flag VOIP_RX are tied to the same
// stream type STREAM_VOICE_CALL so this will only change the HAL volume once even
// if more than one track are active
status_t AudioFlinger::PlaybackThread::handleVoipVolume_l(float *volume)
{
    status_t result = NO_ERROR;
    if ((mOutput->flags & AUDIO_OUTPUT_FLAG_VOIP_RX) != 0) {
        if (*volume != mLeftVolFloat) {
            result = mOutput->stream->setVolume(*volume, *volume);
            ALOGE_IF(result != OK,
                     "Error when setting output stream volume: %d", result);
            if (result == NO_ERROR) {
                mLeftVolFloat = *volume;
            }
        }
        // if stream volume was successfully sent to the HAL, mLeftVolFloat == v here and we
        // remove stream volume contribution from software volume.
        if (mLeftVolFloat == *volume) {
            *volume = 1.0f;
        }
    }
    return result;
}

status_t AudioFlinger::MixerThread::createAudioPatch_l(const struct audio_patch *patch,
                                                          audio_patch_handle_t *handle)
{
    status_t status;
    if (property_get_bool("af.patch_park", false /* default_value */)) {
        // Park FastMixer to avoid potential DOS issues with writing to the HAL
        // or if HAL does not properly lock against access.
        AutoPark<FastMixer> park(mFastMixer);
        status = PlaybackThread::createAudioPatch_l(patch, handle);
    } else {
        status = PlaybackThread::createAudioPatch_l(patch, handle);
    }
    return status;
}

status_t AudioFlinger::PlaybackThread::createAudioPatch_l(const struct audio_patch *patch,
                                                          audio_patch_handle_t *handle)
{
    status_t status = NO_ERROR;

    // store new device and send to effects
    audio_devices_t type = AUDIO_DEVICE_NONE;
    for (unsigned int i = 0; i < patch->num_sinks; i++) {
        type |= patch->sinks[i].ext.device.type;
    }

    audio_port_handle_t sinkPortId = patch->sinks[0].id;
#ifdef ADD_BATTERY_DATA
    // when changing the audio output device, call addBatteryData to notify
    // the change
    if (mOutDevice != type) {
        uint32_t params = 0;
        // check whether speaker is on
        if (type & AUDIO_DEVICE_OUT_SPEAKER) {
            params |= IMediaPlayerService::kBatteryDataSpeakerOn;
        }

        audio_devices_t deviceWithoutSpeaker
            = AUDIO_DEVICE_OUT_ALL & ~AUDIO_DEVICE_OUT_SPEAKER;
        // check if any other device (except speaker) is on
        if (type & deviceWithoutSpeaker) {
            params |= IMediaPlayerService::kBatteryDataOtherAudioDeviceOn;
        }

        if (params != 0) {
            addBatteryData(params);
        }
    }
#endif

    for (size_t i = 0; i < mEffectChains.size(); i++) {
        mEffectChains[i]->setDevice_l(type);
    }

    // mPrevOutDevice is the latest device set by createAudioPatch_l(). It is not set when
    // the thread is created so that the first patch creation triggers an ioConfigChanged callback
    bool configChanged = (mPrevOutDevice != type) || (mDeviceId != sinkPortId);
    mOutDevice = type;
    mPatch = *patch;

    if (mOutput->audioHwDev->supportsAudioPatches()) {
        sp<DeviceHalInterface> hwDevice = mOutput->audioHwDev->hwDevice();
        status = hwDevice->createAudioPatch(patch->num_sources,
                                            patch->sources,
                                            patch->num_sinks,
                                            patch->sinks,
                                            handle);
    } else {
        char *address;
        if (strcmp(patch->sinks[0].ext.device.address, "") != 0) {
            //FIXME: we only support address on first sink with HAL version < 3.0
            address = audio_device_address_to_parameter(
                                                        patch->sinks[0].ext.device.type,
                                                        patch->sinks[0].ext.device.address);
        } else {
            address = (char *)calloc(1, 1);
        }
        AudioParameter param = AudioParameter(String8(address));
        free(address);
        param.addInt(String8(AudioParameter::keyRouting), (int)type);
        status = mOutput->stream->setParameters(param.toString());
        *handle = AUDIO_PATCH_HANDLE_NONE;
    }
    if (configChanged) {
        mPrevOutDevice = type;
        mDeviceId = sinkPortId;
        sendIoConfigEvent_l(AUDIO_OUTPUT_CONFIG_CHANGED);
    }
    return status;
}

status_t AudioFlinger::MixerThread::releaseAudioPatch_l(const audio_patch_handle_t handle)
{
    status_t status;
    if (property_get_bool("af.patch_park", false /* default_value */)) {
        // Park FastMixer to avoid potential DOS issues with writing to the HAL
        // or if HAL does not properly lock against access.
        AutoPark<FastMixer> park(mFastMixer);
        status = PlaybackThread::releaseAudioPatch_l(handle);
    } else {
        status = PlaybackThread::releaseAudioPatch_l(handle);
    }
    return status;
}

status_t AudioFlinger::PlaybackThread::releaseAudioPatch_l(const audio_patch_handle_t handle)
{
    status_t status = NO_ERROR;

    mOutDevice = AUDIO_DEVICE_NONE;

    if (mOutput->audioHwDev->supportsAudioPatches()) {
        sp<DeviceHalInterface> hwDevice = mOutput->audioHwDev->hwDevice();
        status = hwDevice->releaseAudioPatch(handle);
    } else {
        AudioParameter param;
        param.addInt(String8(AudioParameter::keyRouting), 0);
        status = mOutput->stream->setParameters(param.toString());
    }
    return status;
}

void AudioFlinger::PlaybackThread::addPatchTrack(const sp<PatchTrack>& track)
{
    Mutex::Autolock _l(mLock);
    mTracks.add(track);
}

void AudioFlinger::PlaybackThread::deletePatchTrack(const sp<PatchTrack>& track)
{
    Mutex::Autolock _l(mLock);
    destroyTrack_l(track);
}

void AudioFlinger::PlaybackThread::toAudioPortConfig(struct audio_port_config *config)
{
    ThreadBase::toAudioPortConfig(config);
    config->role = AUDIO_PORT_ROLE_SOURCE;
    config->ext.mix.hw_module = mOutput->audioHwDev->handle();
    config->ext.mix.usecase.stream = AUDIO_STREAM_DEFAULT;
    if (mOutput && mOutput->flags != AUDIO_OUTPUT_FLAG_NONE) {
        config->config_mask |= AUDIO_PORT_CONFIG_FLAGS;
        config->flags.output = mOutput->flags;
    }
}

// ----------------------------------------------------------------------------

AudioFlinger::MixerThread::MixerThread(const sp<AudioFlinger>& audioFlinger, AudioStreamOut* output,
        audio_io_handle_t id, audio_devices_t device, bool systemReady, type_t type)
    :   PlaybackThread(audioFlinger, output, id, device, type, systemReady),
        // mAudioMixer below
        // mFastMixer below
        mFastMixerFutex(0),
        mMasterMono(false)
        // mOutputSink below
        // mPipeSink below
        // mNormalSink below
{
    setMasterBalance(audioFlinger->getMasterBalance_l());
    ALOGV("MixerThread() id=%d device=%#x type=%d", id, device, type);
    ALOGV("mSampleRate=%u, mChannelMask=%#x, mChannelCount=%u, mFormat=%#x, mFrameSize=%zu, "
            "mFrameCount=%zu, mNormalFrameCount=%zu",
            mSampleRate, mChannelMask, mChannelCount, mFormat, mFrameSize, mFrameCount,
            mNormalFrameCount);
    mAudioMixer = new AudioMixer(mNormalFrameCount, mSampleRate);

    if (type == DUPLICATING) {
        // The Duplicating thread uses the AudioMixer and delivers data to OutputTracks
        // (downstream MixerThreads) in DuplicatingThread::threadLoop_write().
        // Do not create or use mFastMixer, mOutputSink, mPipeSink, or mNormalSink.
        return;
    }
    // create an NBAIO sink for the HAL output stream, and negotiate
    mOutputSink = new AudioStreamOutSink(output->stream);
    size_t numCounterOffers = 0;
    const NBAIO_Format offers[1] = {Format_from_SR_C(
            mSampleRate, mChannelCount + mHapticChannelCount, mFormat)};
#if !LOG_NDEBUG
    ssize_t index =
#else
    (void)
#endif
            mOutputSink->negotiate(offers, 1, NULL, numCounterOffers);
    ALOG_ASSERT(index == 0);

    // initialize fast mixer depending on configuration
    bool initFastMixer;
    switch (kUseFastMixer) {
    case FastMixer_Never:
        initFastMixer = false;
        break;
    case FastMixer_Always:
        initFastMixer = true;
        break;
    case FastMixer_Static:
    case FastMixer_Dynamic:
        // FastMixer was designed to operate with a HAL that pulls at a regular rate,
        // where the period is less than an experimentally determined threshold that can be
        // scheduled reliably with CFS. However, the BT A2DP HAL is
        // bursty (does not pull at a regular rate) and so cannot operate with FastMixer.
        initFastMixer = mFrameCount < mNormalFrameCount
                && (mOutDevice & AUDIO_DEVICE_OUT_ALL_A2DP) == 0;
        break;
    }
    ALOGW_IF(initFastMixer == false && mFrameCount < mNormalFrameCount,
            "FastMixer is preferred for this sink as frameCount %zu is less than threshold %zu",
            mFrameCount, mNormalFrameCount);
    if (initFastMixer) {
        audio_format_t fastMixerFormat;
        if (mMixerBufferEnabled && mEffectBufferEnabled) {
            fastMixerFormat = AUDIO_FORMAT_PCM_FLOAT;
        } else {
            fastMixerFormat = AUDIO_FORMAT_PCM_16_BIT;
        }
        if (mFormat != fastMixerFormat) {
            // change our Sink format to accept our intermediate precision
            mFormat = fastMixerFormat;
            free(mSinkBuffer);
            mFrameSize = audio_bytes_per_frame(mChannelCount + mHapticChannelCount, mFormat);
            const size_t sinkBufferSize = mNormalFrameCount * mFrameSize;
            (void)posix_memalign(&mSinkBuffer, 32, sinkBufferSize);
        }

        // create a MonoPipe to connect our submix to FastMixer
        NBAIO_Format format = mOutputSink->format();

        // adjust format to match that of the Fast Mixer
        ALOGV("format changed from %#x to %#x", format.mFormat, fastMixerFormat);
        format.mFormat = fastMixerFormat;
        format.mFrameSize = audio_bytes_per_sample(format.mFormat) * format.mChannelCount;

        // This pipe depth compensates for scheduling latency of the normal mixer thread.
        // When it wakes up after a maximum latency, it runs a few cycles quickly before
        // finally blocking.  Note the pipe implementation rounds up the request to a power of 2.
        MonoPipe *monoPipe = new MonoPipe(mNormalFrameCount * 4, format, true /*writeCanBlock*/);
        const NBAIO_Format offers[1] = {format};
        size_t numCounterOffers = 0;
#if !LOG_NDEBUG
        ssize_t index =
#else
        (void)
#endif
                monoPipe->negotiate(offers, 1, NULL, numCounterOffers);
        ALOG_ASSERT(index == 0);
        monoPipe->setAvgFrames((mScreenState & 1) ?
                (monoPipe->maxFrames() * 7) / 8 : mNormalFrameCount * 2);
        mPipeSink = monoPipe;

        // create fast mixer and configure it initially with just one fast track for our submix
        mFastMixer = new FastMixer(mId);
        FastMixerStateQueue *sq = mFastMixer->sq();
#ifdef STATE_QUEUE_DUMP
        sq->setObserverDump(&mStateQueueObserverDump);
        sq->setMutatorDump(&mStateQueueMutatorDump);
#endif
        FastMixerState *state = sq->begin();
        FastTrack *fastTrack = &state->mFastTracks[0];
        // wrap the source side of the MonoPipe to make it an AudioBufferProvider
        fastTrack->mBufferProvider = new SourceAudioBufferProvider(new MonoPipeReader(monoPipe));
        fastTrack->mVolumeProvider = NULL;
        fastTrack->mChannelMask = mChannelMask | mHapticChannelMask; // mPipeSink channel mask for
                                                                     // audio to FastMixer
        fastTrack->mFormat = mFormat; // mPipeSink format for audio to FastMixer
        fastTrack->mHapticPlaybackEnabled = mHapticChannelMask != AUDIO_CHANNEL_NONE;
        fastTrack->mHapticIntensity = AudioMixer::HAPTIC_SCALE_NONE;
        fastTrack->mGeneration++;
        state->mFastTracksGen++;
        state->mTrackMask = 1;
        // fast mixer will use the HAL output sink
        state->mOutputSink = mOutputSink.get();
        state->mOutputSinkGen++;
        state->mFrameCount = mFrameCount;
        // specify sink channel mask when haptic channel mask present as it can not
        // be calculated directly from channel count
        state->mSinkChannelMask = mHapticChannelMask == AUDIO_CHANNEL_NONE
                ? AUDIO_CHANNEL_NONE : mChannelMask | mHapticChannelMask;
        state->mCommand = FastMixerState::COLD_IDLE;
        // already done in constructor initialization list
        //mFastMixerFutex = 0;
        state->mColdFutexAddr = &mFastMixerFutex;
        state->mColdGen++;
        state->mDumpState = &mFastMixerDumpState;
        mFastMixerNBLogWriter = audioFlinger->newWriter_l(kFastMixerLogSize, "FastMixer");
        state->mNBLogWriter = mFastMixerNBLogWriter.get();
        sq->end();
        sq->push(FastMixerStateQueue::BLOCK_UNTIL_PUSHED);

        NBLog::thread_info_t info;
        info.id = mId;
        info.type = NBLog::FASTMIXER;
        mFastMixerNBLogWriter->log<NBLog::EVENT_THREAD_INFO>(info);

        // start the fast mixer
        mFastMixer->run("FastMixer", PRIORITY_URGENT_AUDIO);
        pid_t tid = mFastMixer->getTid();
        sendPrioConfigEvent(getpid(), tid, kPriorityFastMixer, false /*forApp*/);
        stream()->setHalThreadPriority(kPriorityFastMixer);

#ifdef AUDIO_WATCHDOG
        // create and start the watchdog
        mAudioWatchdog = new AudioWatchdog();
        mAudioWatchdog->setDump(&mAudioWatchdogDump);
        mAudioWatchdog->run("AudioWatchdog", PRIORITY_URGENT_AUDIO);
        tid = mAudioWatchdog->getTid();
        sendPrioConfigEvent(getpid(), tid, kPriorityFastMixer, false /*forApp*/);
#endif
    } else {
#ifdef TEE_SINK
        // Only use the MixerThread tee if there is no FastMixer.
        mTee.set(mOutputSink->format(), NBAIO_Tee::TEE_FLAG_OUTPUT_THREAD);
        mTee.setId(std::string("_") + std::to_string(mId) + "_M");
#endif
    }

    switch (kUseFastMixer) {
    case FastMixer_Never:
    case FastMixer_Dynamic:
        mNormalSink = mOutputSink;
        break;
    case FastMixer_Always:
        mNormalSink = mPipeSink;
        break;
    case FastMixer_Static:
        mNormalSink = initFastMixer ? mPipeSink : mOutputSink;
        break;
    }
}

AudioFlinger::MixerThread::~MixerThread()
{
    if (mFastMixer != 0) {
        FastMixerStateQueue *sq = mFastMixer->sq();
        FastMixerState *state = sq->begin();
        if (state->mCommand == FastMixerState::COLD_IDLE) {
            int32_t old = android_atomic_inc(&mFastMixerFutex);
            if (old == -1) {
                (void) syscall(__NR_futex, &mFastMixerFutex, FUTEX_WAKE_PRIVATE, 1);
            }
        }
        state->mCommand = FastMixerState::EXIT;
        sq->end();
        sq->push(FastMixerStateQueue::BLOCK_UNTIL_PUSHED);
        mFastMixer->join();
        // Though the fast mixer thread has exited, it's state queue is still valid.
        // We'll use that extract the final state which contains one remaining fast track
        // corresponding to our sub-mix.
        state = sq->begin();
        ALOG_ASSERT(state->mTrackMask == 1);
        FastTrack *fastTrack = &state->mFastTracks[0];
        ALOG_ASSERT(fastTrack->mBufferProvider != NULL);
        delete fastTrack->mBufferProvider;
        sq->end(false /*didModify*/);
        mFastMixer.clear();
#ifdef AUDIO_WATCHDOG
        if (mAudioWatchdog != 0) {
            mAudioWatchdog->requestExit();
            mAudioWatchdog->requestExitAndWait();
            mAudioWatchdog.clear();
        }
#endif
    }
    mAudioFlinger->unregisterWriter(mFastMixerNBLogWriter);
    delete mAudioMixer;
}


uint32_t AudioFlinger::MixerThread::correctLatency_l(uint32_t latency) const
{
    if (mFastMixer != 0) {
        MonoPipe *pipe = (MonoPipe *)mPipeSink.get();
        latency += (pipe->getAvgFrames() * 1000) / mSampleRate;
    }
    return latency;
}

ssize_t AudioFlinger::MixerThread::threadLoop_write()
{
    // FIXME we should only do one push per cycle; confirm this is true
    // Start the fast mixer if it's not already running
    if (mFastMixer != 0) {
        FastMixerStateQueue *sq = mFastMixer->sq();
        FastMixerState *state = sq->begin();
        if (state->mCommand != FastMixerState::MIX_WRITE &&
                (kUseFastMixer != FastMixer_Dynamic || state->mTrackMask > 1)) {
            if (state->mCommand == FastMixerState::COLD_IDLE) {

                // FIXME workaround for first HAL write being CPU bound on some devices
                ATRACE_BEGIN("write");
                mOutput->write((char *)mSinkBuffer, 0);
                ATRACE_END();

                int32_t old = android_atomic_inc(&mFastMixerFutex);
                if (old == -1) {
                    (void) syscall(__NR_futex, &mFastMixerFutex, FUTEX_WAKE_PRIVATE, 1);
                }
#ifdef AUDIO_WATCHDOG
                if (mAudioWatchdog != 0) {
                    mAudioWatchdog->resume();
                }
#endif
            }
            state->mCommand = FastMixerState::MIX_WRITE;
#ifdef FAST_THREAD_STATISTICS
            mFastMixerDumpState.increaseSamplingN(mAudioFlinger->isLowRamDevice() ?
                FastThreadDumpState::kSamplingNforLowRamDevice : FastThreadDumpState::kSamplingN);
#endif
            sq->end();
            sq->push(FastMixerStateQueue::BLOCK_UNTIL_PUSHED);
            if (kUseFastMixer == FastMixer_Dynamic) {
                mNormalSink = mPipeSink;
            }
        } else {
            sq->end(false /*didModify*/);
        }
    }
    return PlaybackThread::threadLoop_write();
}

void AudioFlinger::MixerThread::threadLoop_standby()
{
    // Idle the fast mixer if it's currently running
    if (mFastMixer != 0) {
        FastMixerStateQueue *sq = mFastMixer->sq();
        FastMixerState *state = sq->begin();
        if (!(state->mCommand & FastMixerState::IDLE)) {
            // Report any frames trapped in the Monopipe
            MonoPipe *monoPipe = (MonoPipe *)mPipeSink.get();
            const long long pipeFrames = monoPipe->maxFrames() - monoPipe->availableToWrite();
            mLocalLog.log("threadLoop_standby: framesWritten:%lld  suspendedFrames:%lld  "
                    "monoPipeWritten:%lld  monoPipeLeft:%lld",
                    (long long)mFramesWritten, (long long)mSuspendedFrames,
                    (long long)mPipeSink->framesWritten(), pipeFrames);
            mLocalLog.log("threadLoop_standby: %s", mTimestamp.toString().c_str());

            state->mCommand = FastMixerState::COLD_IDLE;
            state->mColdFutexAddr = &mFastMixerFutex;
            state->mColdGen++;
            mFastMixerFutex = 0;
            sq->end();
            // BLOCK_UNTIL_PUSHED would be insufficient, as we need it to stop doing I/O now
            sq->push(FastMixerStateQueue::BLOCK_UNTIL_ACKED);
            if (kUseFastMixer == FastMixer_Dynamic) {
                mNormalSink = mOutputSink;
            }
#ifdef AUDIO_WATCHDOG
            if (mAudioWatchdog != 0) {
                mAudioWatchdog->pause();
            }
#endif
        } else {
            sq->end(false /*didModify*/);
        }
    }
    PlaybackThread::threadLoop_standby();
}

bool AudioFlinger::PlaybackThread::waitingAsyncCallback_l()
{
    return false;
}

bool AudioFlinger::PlaybackThread::shouldStandby_l()
{
    return !mStandby;
}

bool AudioFlinger::PlaybackThread::waitingAsyncCallback()
{
    Mutex::Autolock _l(mLock);
    return waitingAsyncCallback_l();
}

// shared by MIXER and DIRECT, overridden by DUPLICATING
void AudioFlinger::PlaybackThread::threadLoop_standby()
{
    ALOGV("Audio hardware entering standby, mixer %p, suspend count %d", this, mSuspended);
    mOutput->standby();
    if (mUseAsyncWrite != 0) {
        // discard any pending drain or write ack by incrementing sequence
        mWriteAckSequence = (mWriteAckSequence + 2) & ~1;
        mDrainSequence = (mDrainSequence + 2) & ~1;
        ALOG_ASSERT(mCallbackThread != 0);
        mCallbackThread->setWriteBlocked(mWriteAckSequence);
        mCallbackThread->setDraining(mDrainSequence);
    }
    mHwPaused = false;
}

void AudioFlinger::PlaybackThread::onAddNewTrack_l()
{
    ALOGV("signal playback thread");
    broadcast_l();
}

void AudioFlinger::PlaybackThread::onAsyncError()
{
    for (int i = AUDIO_STREAM_SYSTEM; i < (int)AUDIO_STREAM_CNT; i++) {
        invalidateTracks((audio_stream_type_t)i);
    }
}

void AudioFlinger::MixerThread::threadLoop_mix()
{
    // mix buffers...
    mAudioMixer->process();
    mCurrentWriteLength = mSinkBufferSize;
    // increase sleep time progressively when application underrun condition clears.
    // Only increase sleep time if the mixer is ready for two consecutive times to avoid
    // that a steady state of alternating ready/not ready conditions keeps the sleep time
    // such that we would underrun the audio HAL.
    if ((mSleepTimeUs == 0) && (sleepTimeShift > 0)) {
        sleepTimeShift--;
    }
    mSleepTimeUs = 0;
    mStandbyTimeNs = systemTime() + mStandbyDelayNs;
    //TODO: delay standby when effects have a tail

}

void AudioFlinger::MixerThread::threadLoop_sleepTime()
{
    // If no tracks are ready, sleep once for the duration of an output
    // buffer size, then write 0s to the output
    if (mSleepTimeUs == 0) {
        if (mMixerStatus == MIXER_TRACKS_ENABLED) {
            if (mPipeSink.get() != nullptr && mPipeSink == mNormalSink) {
                // Using the Monopipe availableToWrite, we estimate the
                // sleep time to retry for more data (before we underrun).
                MonoPipe *monoPipe = static_cast<MonoPipe *>(mPipeSink.get());
                const ssize_t availableToWrite = mPipeSink->availableToWrite();
                const size_t pipeFrames = monoPipe->maxFrames();
                const size_t framesLeft = pipeFrames - max(availableToWrite, 0);
                // HAL_framecount <= framesDelay ~ framesLeft / 2 <= Normal_Mixer_framecount
                const size_t framesDelay = std::min(
                        mNormalFrameCount, max(framesLeft / 2, mFrameCount));
                ALOGV("pipeFrames:%zu framesLeft:%zu framesDelay:%zu",
                        pipeFrames, framesLeft, framesDelay);
                mSleepTimeUs = framesDelay * MICROS_PER_SECOND / mSampleRate;
            } else {
                mSleepTimeUs = mActiveSleepTimeUs >> sleepTimeShift;
                if (mSleepTimeUs < kMinThreadSleepTimeUs) {
                    mSleepTimeUs = kMinThreadSleepTimeUs;
                }
                // reduce sleep time in case of consecutive application underruns to avoid
                // starving the audio HAL. As activeSleepTimeUs() is larger than a buffer
                // duration we would end up writing less data than needed by the audio HAL if
                // the condition persists.
                if (sleepTimeShift < kMaxThreadSleepTimeShift) {
                    sleepTimeShift++;
                }
            }
        } else {
            mSleepTimeUs = mIdleSleepTimeUs;
        }
    } else if (mBytesWritten != 0 || (mMixerStatus == MIXER_TRACKS_ENABLED)) {
        // clear out mMixerBuffer or mSinkBuffer, to ensure buffers are cleared
        // before effects processing or output.
        if (mMixerBufferValid) {
            memset(mMixerBuffer, 0, mMixerBufferSize);
        } else {
            memset(mSinkBuffer, 0, mSinkBufferSize);
        }
        mSleepTimeUs = 0;
        ALOGV_IF(mBytesWritten == 0 && (mMixerStatus == MIXER_TRACKS_ENABLED),
                "anticipated start");
    }
    // TODO add standby time extension fct of effect tail
}

// prepareTracks_l() must be called with ThreadBase::mLock held
AudioFlinger::PlaybackThread::mixer_state AudioFlinger::MixerThread::prepareTracks_l(
        Vector< sp<Track> > *tracksToRemove)
{
    // clean up deleted track ids in AudioMixer before allocating new tracks
    (void)mTracks.processDeletedTrackIds([this](int trackId) {
        // for each trackId, destroy it in the AudioMixer
        if (mAudioMixer->exists(trackId)) {
            mAudioMixer->destroy(trackId);
        }
    });
    mTracks.clearDeletedTrackIds();

    mixer_state mixerStatus = MIXER_IDLE;
    // find out which tracks need to be processed
    size_t count = mActiveTracks.size();
    size_t mixedTracks = 0;
    size_t tracksWithEffect = 0;
    // counts only _active_ fast tracks
    size_t fastTracks = 0;
    uint32_t resetMask = 0; // bit mask of fast tracks that need to be reset

    float masterVolume = mMasterVolume;
    bool masterMute = mMasterMute;

    if (masterMute) {
        masterVolume = 0;
    }
    // Delegate master volume control to effect in output mix effect chain if needed
    sp<EffectChain> chain = getEffectChain_l(AUDIO_SESSION_OUTPUT_MIX);
    if (chain != 0) {
        uint32_t v = (uint32_t)(masterVolume * (1 << 24));
        chain->setVolume_l(&v, &v);
        masterVolume = (float)((v + (1 << 23)) >> 24);
        chain.clear();
    }

    // prepare a new state to push
    FastMixerStateQueue *sq = NULL;
    FastMixerState *state = NULL;
    bool didModify = false;
    FastMixerStateQueue::block_t block = FastMixerStateQueue::BLOCK_UNTIL_PUSHED;
    bool coldIdle = false;
    if (mFastMixer != 0) {
        sq = mFastMixer->sq();
        state = sq->begin();
        coldIdle = state->mCommand == FastMixerState::COLD_IDLE;
    }

    mMixerBufferValid = false;  // mMixerBuffer has no valid data until appropriate tracks found.
    mEffectBufferValid = false; // mEffectBuffer has no valid data until tracks found.

    // DeferredOperations handles statistics after setting mixerStatus.
    class DeferredOperations {
    public:
        DeferredOperations(mixer_state *mixerStatus)
            : mMixerStatus(mixerStatus) { }

        // when leaving scope, tally frames properly.
        ~DeferredOperations() {
            // Tally underrun frames only if we are actually mixing (MIXER_TRACKS_READY)
            // because that is when the underrun occurs.
            // We do not distinguish between FastTracks and NormalTracks here.
            if (*mMixerStatus == MIXER_TRACKS_READY) {
                for (const auto &underrun : mUnderrunFrames) {
                    underrun.first->mAudioTrackServerProxy->tallyUnderrunFrames(
                            underrun.second);
                }
            }
        }

        // tallyUnderrunFrames() is called to update the track counters
        // with the number of underrun frames for a particular mixer period.
        // We defer tallying until we know the final mixer status.
        void tallyUnderrunFrames(sp<Track> track, size_t underrunFrames) {
            mUnderrunFrames.emplace_back(track, underrunFrames);
        }

    private:
        const mixer_state * const mMixerStatus;
        std::vector<std::pair<sp<Track>, size_t>> mUnderrunFrames;
    } deferredOperations(&mixerStatus); // implicit nested scope for variable capture

    bool noFastHapticTrack = true;
    for (size_t i=0 ; i<count ; i++) {
        const sp<Track> t = mActiveTracks[i];

        // this const just means the local variable doesn't change
        Track* const track = t.get();

        // process fast tracks
        if (track->isFastTrack()) {
            LOG_ALWAYS_FATAL_IF(mFastMixer.get() == nullptr,
                    "%s(%d): FastTrack(%d) present without FastMixer",
                     __func__, id(), track->id());

            if (track->getHapticPlaybackEnabled()) {
                noFastHapticTrack = false;
            }

            // It's theoretically possible (though unlikely) for a fast track to be created
            // and then removed within the same normal mix cycle.  This is not a problem, as
            // the track never becomes active so it's fast mixer slot is never touched.
            // The converse, of removing an (active) track and then creating a new track
            // at the identical fast mixer slot within the same normal mix cycle,
            // is impossible because the slot isn't marked available until the end of each cycle.
            int j = track->mFastIndex;
            ALOG_ASSERT(0 < j && j < (int)FastMixerState::sMaxFastTracks);
            ALOG_ASSERT(!(mFastTrackAvailMask & (1 << j)));
            FastTrack *fastTrack = &state->mFastTracks[j];

            // Determine whether the track is currently in underrun condition,
            // and whether it had a recent underrun.
            FastTrackDump *ftDump = &mFastMixerDumpState.mTracks[j];
            FastTrackUnderruns underruns = ftDump->mUnderruns;
            uint32_t recentFull = (underruns.mBitFields.mFull -
                    track->mObservedUnderruns.mBitFields.mFull) & UNDERRUN_MASK;
            uint32_t recentPartial = (underruns.mBitFields.mPartial -
                    track->mObservedUnderruns.mBitFields.mPartial) & UNDERRUN_MASK;
            uint32_t recentEmpty = (underruns.mBitFields.mEmpty -
                    track->mObservedUnderruns.mBitFields.mEmpty) & UNDERRUN_MASK;
            uint32_t recentUnderruns = recentPartial + recentEmpty;
            track->mObservedUnderruns = underruns;
            // don't count underruns that occur while stopping or pausing
            // or stopped which can occur when flush() is called while active
            size_t underrunFrames = 0;
            if (!(track->isStopping() || track->isPausing() || track->isStopped()) &&
                    recentUnderruns > 0) {
                // FIXME fast mixer will pull & mix partial buffers, but we count as a full underrun
                underrunFrames = recentUnderruns * mFrameCount;
            }
            // Immediately account for FastTrack underruns.
            track->mAudioTrackServerProxy->tallyUnderrunFrames(underrunFrames);

            // This is similar to the state machine for normal tracks,
            // with a few modifications for fast tracks.
            bool isActive = true;
            switch (track->mState) {
            case TrackBase::STOPPING_1:
                // track stays active in STOPPING_1 state until first underrun
                if (recentUnderruns > 0 || track->isTerminated()) {
                    track->mState = TrackBase::STOPPING_2;
                }
                break;
            case TrackBase::PAUSING:
                // ramp down is not yet implemented
                track->setPaused();
                break;
            case TrackBase::RESUMING:
                // ramp up is not yet implemented
                track->mState = TrackBase::ACTIVE;
                break;
            case TrackBase::ACTIVE:
                if (recentFull > 0 || recentPartial > 0) {
                    // track has provided at least some frames recently: reset retry count
                    track->mRetryCount = kMaxTrackRetries;
                }
                if (recentUnderruns == 0) {
                    // no recent underruns: stay active
                    break;
                }
                // there has recently been an underrun of some kind
                if (track->sharedBuffer() == 0) {
                    // were any of the recent underruns "empty" (no frames available)?
                    if (recentEmpty == 0) {
                        // no, then ignore the partial underruns as they are allowed indefinitely
                        break;
                    }
                    // there has recently been an "empty" underrun: decrement the retry counter
                    if (--(track->mRetryCount) > 0) {
                        break;
                    }
                    // indicate to client process that the track was disabled because of underrun;
                    // it will then automatically call start() when data is available
                    track->disable();
                    // remove from active list, but state remains ACTIVE [confusing but true]
                    isActive = false;
                    break;
                }
                FALLTHROUGH_INTENDED;
            case TrackBase::STOPPING_2:
            case TrackBase::PAUSED:
            case TrackBase::STOPPED:
            case TrackBase::FLUSHED:   // flush() while active
                // Check for presentation complete if track is inactive
                // We have consumed all the buffers of this track.
                // This would be incomplete if we auto-paused on underrun
                {
                    uint32_t latency = 0;
                    status_t result = mOutput->stream->getLatency(&latency);
                    ALOGE_IF(result != OK,
                            "Error when retrieving output stream latency: %d", result);
                    size_t audioHALFrames = (latency * mSampleRate) / 1000;
                    int64_t framesWritten = mBytesWritten / mFrameSize;
                    if (!(mStandby || track->presentationComplete(framesWritten, audioHALFrames))) {
                        // track stays in active list until presentation is complete
                        break;
                    }
                }
                if (track->isStopping_2()) {
                    track->mState = TrackBase::STOPPED;
                }
                if (track->isStopped()) {
                    // Can't reset directly, as fast mixer is still polling this track
                    //   track->reset();
                    // So instead mark this track as needing to be reset after push with ack
                    resetMask |= 1 << i;
                }
                isActive = false;
                break;
            case TrackBase::IDLE:
            default:
                LOG_ALWAYS_FATAL("unexpected track state %d", track->mState);
            }

            if (isActive) {
                // was it previously inactive?
                if (!(state->mTrackMask & (1 << j))) {
                    ExtendedAudioBufferProvider *eabp = track;
                    VolumeProvider *vp = track;
                    fastTrack->mBufferProvider = eabp;
                    fastTrack->mVolumeProvider = vp;
                    fastTrack->mChannelMask = track->mChannelMask;
                    fastTrack->mFormat = track->mFormat;
                    fastTrack->mHapticPlaybackEnabled = track->getHapticPlaybackEnabled();
                    fastTrack->mHapticIntensity = track->getHapticIntensity();
                    fastTrack->mGeneration++;
                    state->mTrackMask |= 1 << j;
                    didModify = true;
                    // no acknowledgement required for newly active tracks
                }
                sp<AudioTrackServerProxy> proxy = track->mAudioTrackServerProxy;
                float volume;
                if (track->isPlaybackRestricted() || mStreamTypes[track->streamType()].mute) {
                    volume = 0.f;
                } else {
                    volume = masterVolume * mStreamTypes[track->streamType()].volume;
                }

                handleVoipVolume_l(&volume);

                // cache the combined master volume and stream type volume for fast mixer; this
                // lacks any synchronization or barrier so VolumeProvider may read a stale value
                const float vh = track->getVolumeHandler()->getVolume(
                    proxy->framesReleased()).first;
                volume *= vh;
                track->mCachedVolume = volume;
                gain_minifloat_packed_t vlr = proxy->getVolumeLR();
                float vlf = volume * float_from_gain(gain_minifloat_unpack_left(vlr));
                float vrf = volume * float_from_gain(gain_minifloat_unpack_right(vlr));

                track->setFinalVolume((vlf + vrf) / 2.f);
                ++fastTracks;
            } else {
                // was it previously active?
                if (state->mTrackMask & (1 << j)) {
                    fastTrack->mBufferProvider = NULL;
                    fastTrack->mGeneration++;
                    state->mTrackMask &= ~(1 << j);
                    didModify = true;
                    // If any fast tracks were removed, we must wait for acknowledgement
                    // because we're about to decrement the last sp<> on those tracks.
                    block = FastMixerStateQueue::BLOCK_UNTIL_ACKED;
                } else {
                    // ALOGW rather than LOG_ALWAYS_FATAL because it seems there are cases where an
                    // AudioTrack may start (which may not be with a start() but with a write()
                    // after underrun) and immediately paused or released.  In that case the
                    // FastTrack state hasn't had time to update.
                    // TODO Remove the ALOGW when this theory is confirmed.
                    ALOGW("fast track %d should have been active; "
                            "mState=%d, mTrackMask=%#x, recentUnderruns=%u, isShared=%d",
                            j, track->mState, state->mTrackMask, recentUnderruns,
                            track->sharedBuffer() != 0);
                    // Since the FastMixer state already has the track inactive, do nothing here.
                }
                tracksToRemove->add(track);
                // Avoids a misleading display in dumpsys
                track->mObservedUnderruns.mBitFields.mMostRecent = UNDERRUN_FULL;
            }
            if (fastTrack->mHapticPlaybackEnabled != track->getHapticPlaybackEnabled()) {
                fastTrack->mHapticPlaybackEnabled = track->getHapticPlaybackEnabled();
                didModify = true;
            }
            continue;
        }

        {   // local variable scope to avoid goto warning

        audio_track_cblk_t* cblk = track->cblk();

        // The first time a track is added we wait
        // for all its buffers to be filled before processing it
        const int trackId = track->id();

        // if an active track doesn't exist in the AudioMixer, create it.
        // use the trackId as the AudioMixer name.
        if (!mAudioMixer->exists(trackId)) {
            status_t status = mAudioMixer->create(
                    trackId,
                    track->mChannelMask,
                    track->mFormat,
                    track->mSessionId);
            if (status != OK) {
                ALOGW("%s(): AudioMixer cannot create track(%d)"
                        " mask %#x, format %#x, sessionId %d",
                        __func__, trackId,
                        track->mChannelMask, track->mFormat, track->mSessionId);
                tracksToRemove->add(track);
                track->invalidate(); // consider it dead.
                continue;
            }
        }

        // make sure that we have enough frames to mix one full buffer.
        // enforce this condition only once to enable draining the buffer in case the client
        // app does not call stop() and relies on underrun to stop:
        // hence the test on (mMixerStatus == MIXER_TRACKS_READY) meaning the track was mixed
        // during last round
        size_t desiredFrames;
        const uint32_t sampleRate = track->mAudioTrackServerProxy->getSampleRate();
        AudioPlaybackRate playbackRate = track->mAudioTrackServerProxy->getPlaybackRate();

        desiredFrames = sourceFramesNeededWithTimestretch(
                sampleRate, mNormalFrameCount, mSampleRate, playbackRate.mSpeed);
        // TODO: ONLY USED FOR LEGACY RESAMPLERS, remove when they are removed.
        // add frames already consumed but not yet released by the resampler
        // because mAudioTrackServerProxy->framesReady() will include these frames
        desiredFrames += mAudioMixer->getUnreleasedFrames(trackId);

        uint32_t minFrames = 1;
        if ((track->sharedBuffer() == 0) && !track->isStopped() && !track->isPausing() &&
                (mMixerStatusIgnoringFastTracks == MIXER_TRACKS_READY)) {
            minFrames = desiredFrames;
        }

        size_t framesReady = track->framesReady();
        if (ATRACE_ENABLED()) {
            // I wish we had formatted trace names
            std::string traceName("nRdy");
            traceName += std::to_string(trackId);
            ATRACE_INT(traceName.c_str(), framesReady);
        }
        if ((framesReady >= minFrames) && track->isReady() &&
                !track->isPaused() && !track->isTerminated())
        {
            ALOGVV("track(%d) s=%08x [OK] on thread %p", trackId, cblk->mServer, this);

            mixedTracks++;

            // track->mainBuffer() != mSinkBuffer or mMixerBuffer means
            // there is an effect chain connected to the track
            chain.clear();
            if (track->mainBuffer() != mSinkBuffer &&
                    track->mainBuffer() != mMixerBuffer) {
                if (mEffectBufferEnabled) {
                    mEffectBufferValid = true; // Later can set directly.
                }
                chain = getEffectChain_l(track->sessionId());
                // Delegate volume control to effect in track effect chain if needed
                if (chain != 0) {
                    tracksWithEffect++;
                } else {
                    ALOGW("prepareTracks_l(): track(%d) attached to effect but no chain found on "
                            "session %d",
                            trackId, track->sessionId());
                }
            }


            int param = AudioMixer::VOLUME;
            if (track->mFillingUpStatus == Track::FS_FILLED) {
                // no ramp for the first volume setting
                track->mFillingUpStatus = Track::FS_ACTIVE;
                if (track->mState == TrackBase::RESUMING) {
                    track->mState = TrackBase::ACTIVE;
                    // If a new track is paused immediately after start, do not ramp on resume.
                    if (cblk->mServer != 0) {
                        param = AudioMixer::RAMP_VOLUME;
                    }
                }
                mAudioMixer->setParameter(trackId, AudioMixer::RESAMPLE, AudioMixer::RESET, NULL);
                mLeftVolFloat = -1.0;
            // FIXME should not make a decision based on mServer
            } else if (cblk->mServer != 0) {
                // If the track is stopped before the first frame was mixed,
                // do not apply ramp
                param = AudioMixer::RAMP_VOLUME;
            }

            // compute volume for this track
            uint32_t vl, vr;       // in U8.24 integer format
            float vlf, vrf, vaf;   // in [0.0, 1.0] float format
            // read original volumes with volume control
            float v = masterVolume * mStreamTypes[track->streamType()].volume;
            // Always fetch volumeshaper volume to ensure state is updated.
            const sp<AudioTrackServerProxy> proxy = track->mAudioTrackServerProxy;
            const float vh = track->getVolumeHandler()->getVolume(
                    track->mAudioTrackServerProxy->framesReleased()).first;

            if (mStreamTypes[track->streamType()].mute || track->isPlaybackRestricted()) {
                v = 0;
            }

            handleVoipVolume_l(&v);

            if (track->isPausing()) {
                vl = vr = 0;
                vlf = vrf = vaf = 0.;
                track->setPaused();
            } else {
                gain_minifloat_packed_t vlr = proxy->getVolumeLR();
                vlf = float_from_gain(gain_minifloat_unpack_left(vlr));
                vrf = float_from_gain(gain_minifloat_unpack_right(vlr));
                // track volumes come from shared memory, so can't be trusted and must be clamped
                if (vlf > GAIN_FLOAT_UNITY) {
                    ALOGV("Track left volume out of range: %.3g", vlf);
                    vlf = GAIN_FLOAT_UNITY;
                }
                if (vrf > GAIN_FLOAT_UNITY) {
                    ALOGV("Track right volume out of range: %.3g", vrf);
                    vrf = GAIN_FLOAT_UNITY;
                }
                // now apply the master volume and stream type volume and shaper volume
                vlf *= v * vh;
                vrf *= v * vh;
                // assuming master volume and stream type volume each go up to 1.0,
                // then derive vl and vr as U8.24 versions for the effect chain
                const float scaleto8_24 = MAX_GAIN_INT * MAX_GAIN_INT;
                vl = (uint32_t) (scaleto8_24 * vlf);
                vr = (uint32_t) (scaleto8_24 * vrf);
                // vl and vr are now in U8.24 format
                uint16_t sendLevel = proxy->getSendLevel_U4_12();
                // send level comes from shared memory and so may be corrupt
                if (sendLevel > MAX_GAIN_INT) {
                    ALOGV("Track send level out of range: %04X", sendLevel);
                    sendLevel = MAX_GAIN_INT;
                }
                // vaf is represented as [0.0, 1.0] float by rescaling sendLevel
                vaf = v * sendLevel * (1. / MAX_GAIN_INT);
            }

            track->setFinalVolume((vrf + vlf) / 2.f);

            // Delegate volume control to effect in track effect chain if needed
            if (chain != 0 && chain->setVolume_l(&vl, &vr)) {
                // Do not ramp volume if volume is controlled by effect
                param = AudioMixer::VOLUME;
                // Update remaining floating point volume levels
                vlf = (float)vl / (1 << 24);
                vrf = (float)vr / (1 << 24);
                track->mHasVolumeController = true;
            } else {
                // force no volume ramp when volume controller was just disabled or removed
                // from effect chain to avoid volume spike
                if (track->mHasVolumeController) {
                    param = AudioMixer::VOLUME;
                }
                track->mHasVolumeController = false;
            }

            // XXX: these things DON'T need to be done each time
            mAudioMixer->setBufferProvider(trackId, track);
            mAudioMixer->enable(trackId);

            mAudioMixer->setParameter(trackId, param, AudioMixer::VOLUME0, &vlf);
            mAudioMixer->setParameter(trackId, param, AudioMixer::VOLUME1, &vrf);
            mAudioMixer->setParameter(trackId, param, AudioMixer::AUXLEVEL, &vaf);
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::FORMAT, (void *)track->format());
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::CHANNEL_MASK, (void *)(uintptr_t)track->channelMask());
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::MIXER_CHANNEL_MASK,
                (void *)(uintptr_t)(mChannelMask | mHapticChannelMask));
            // limit track sample rate to 2 x output sample rate, which changes at re-configuration
            uint32_t maxSampleRate = mSampleRate * AUDIO_RESAMPLER_DOWN_RATIO_MAX;
            uint32_t reqSampleRate = proxy->getSampleRate();
            if (reqSampleRate == 0) {
                reqSampleRate = mSampleRate;
            } else if (reqSampleRate > maxSampleRate) {
                reqSampleRate = maxSampleRate;
            }
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::RESAMPLE,
                AudioMixer::SAMPLE_RATE,
                (void *)(uintptr_t)reqSampleRate);

            AudioPlaybackRate playbackRate = proxy->getPlaybackRate();
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TIMESTRETCH,
                AudioMixer::PLAYBACK_RATE,
                &playbackRate);

            /*
             * Select the appropriate output buffer for the track.
             *
             * Tracks with effects go into their own effects chain buffer
             * and from there into either mEffectBuffer or mSinkBuffer.
             *
             * Other tracks can use mMixerBuffer for higher precision
             * channel accumulation.  If this buffer is enabled
             * (mMixerBufferEnabled true), then selected tracks will accumulate
             * into it.
             *
             */
            if (mMixerBufferEnabled
                    && (track->mainBuffer() == mSinkBuffer
                            || track->mainBuffer() == mMixerBuffer)) {
                mAudioMixer->setParameter(
                        trackId,
                        AudioMixer::TRACK,
                        AudioMixer::MIXER_FORMAT, (void *)mMixerBufferFormat);
                mAudioMixer->setParameter(
                        trackId,
                        AudioMixer::TRACK,
                        AudioMixer::MAIN_BUFFER, (void *)mMixerBuffer);
                // TODO: override track->mainBuffer()?
                mMixerBufferValid = true;
            } else {
                mAudioMixer->setParameter(
                        trackId,
                        AudioMixer::TRACK,
                        AudioMixer::MIXER_FORMAT, (void *)EFFECT_BUFFER_FORMAT);
                mAudioMixer->setParameter(
                        trackId,
                        AudioMixer::TRACK,
                        AudioMixer::MAIN_BUFFER, (void *)track->mainBuffer());
            }
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::AUX_BUFFER, (void *)track->auxBuffer());
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::HAPTIC_ENABLED, (void *)(uintptr_t)track->getHapticPlaybackEnabled());
            mAudioMixer->setParameter(
                trackId,
                AudioMixer::TRACK,
                AudioMixer::HAPTIC_INTENSITY, (void *)(uintptr_t)track->getHapticIntensity());

            // reset retry count
            track->mRetryCount = kMaxTrackRetries;

            // If one track is ready, set the mixer ready if:
            //  - the mixer was not ready during previous round OR
            //  - no other track is not ready
            if (mMixerStatusIgnoringFastTracks != MIXER_TRACKS_READY ||
                    mixerStatus != MIXER_TRACKS_ENABLED) {
                mixerStatus = MIXER_TRACKS_READY;
            }
        } else {
            size_t underrunFrames = 0;
            if (framesReady < desiredFrames && !track->isStopped() && !track->isPaused()) {
                ALOGV("track(%d) underrun,  framesReady(%zu) < framesDesired(%zd)",
                        trackId, framesReady, desiredFrames);
                underrunFrames = desiredFrames;
            }
            deferredOperations.tallyUnderrunFrames(track, underrunFrames);

            // clear effect chain input buffer if an active track underruns to avoid sending
            // previous audio buffer again to effects
            chain = getEffectChain_l(track->sessionId());
            if (chain != 0) {
                chain->clearInputBuffer();
            }

            ALOGVV("track(%d) s=%08x [NOT READY] on thread %p", trackId, cblk->mServer, this);
            if ((track->sharedBuffer() != 0) || track->isTerminated() ||
                    track->isStopped() || track->isPaused()) {
                // We have consumed all the buffers of this track.
                // Remove it from the list of active tracks.
                // TODO: use actual buffer filling status instead of latency when available from
                // audio HAL
                size_t audioHALFrames = (latency_l() * mSampleRate) / 1000;
                int64_t framesWritten = mBytesWritten / mFrameSize;
                if (mStandby || track->presentationComplete(framesWritten, audioHALFrames)) {
                    if (track->isStopped()) {
                        track->reset();
                    }
                    tracksToRemove->add(track);
                }
            } else {
                // No buffers for this track. Give it a few chances to
                // fill a buffer, then remove it from active list.
                if (--(track->mRetryCount) <= 0) {
                    ALOGI("BUFFER TIMEOUT: remove(%d) from active list on thread %p",
                            trackId, this);
                    tracksToRemove->add(track);
                    // indicate to client process that the track was disabled because of underrun;
                    // it will then automatically call start() when data is available
                    track->disable();
                // If one track is not ready, mark the mixer also not ready if:
                //  - the mixer was ready during previous round OR
                //  - no other track is ready
                } else if (mMixerStatusIgnoringFastTracks == MIXER_TRACKS_READY ||
                                mixerStatus != MIXER_TRACKS_READY) {
                    mixerStatus = MIXER_TRACKS_ENABLED;
                }
            }
            mAudioMixer->disable(trackId);
        }

        }   // local variable scope to avoid goto warning

    }

    if (mHapticChannelMask != AUDIO_CHANNEL_NONE && sq != NULL) {
        // When there is no fast track playing haptic and FastMixer exists,
        // enabling the first FastTrack, which provides mixed data from normal
        // tracks, to play haptic data.
        FastTrack *fastTrack = &state->mFastTracks[0];
        if (fastTrack->mHapticPlaybackEnabled != noFastHapticTrack) {
            fastTrack->mHapticPlaybackEnabled = noFastHapticTrack;
            didModify = true;
        }
    }

    // Push the new FastMixer state if necessary
    bool pauseAudioWatchdog = false;
    if (didModify) {
        state->mFastTracksGen++;
        // if the fast mixer was active, but now there are no fast tracks, then put it in cold idle
        if (kUseFastMixer == FastMixer_Dynamic &&
                state->mCommand == FastMixerState::MIX_WRITE && state->mTrackMask <= 1) {
            state->mCommand = FastMixerState::COLD_IDLE;
            state->mColdFutexAddr = &mFastMixerFutex;
            state->mColdGen++;
            mFastMixerFutex = 0;
            if (kUseFastMixer == FastMixer_Dynamic) {
                mNormalSink = mOutputSink;
            }
            // If we go into cold idle, need to wait for acknowledgement
            // so that fast mixer stops doing I/O.
            block = FastMixerStateQueue::BLOCK_UNTIL_ACKED;
            pauseAudioWatchdog = true;
        }
    }
    if (sq != NULL) {
        sq->end(didModify);
        // No need to block if the FastMixer is in COLD_IDLE as the FastThread
        // is not active. (We BLOCK_UNTIL_ACKED when entering COLD_IDLE
        // when bringing the output sink into standby.)
        //
        // We will get the latest FastMixer state when we come out of COLD_IDLE.
        //
        // This occurs with BT suspend when we idle the FastMixer with
        // active tracks, which may be added or removed.
        sq->push(coldIdle ? FastMixerStateQueue::BLOCK_NEVER : block);
    }
#ifdef AUDIO_WATCHDOG
    if (pauseAudioWatchdog && mAudioWatchdog != 0) {
        mAudioWatchdog->pause();
    }
#endif

    // Now perform the deferred reset on fast tracks that have stopped
    while (resetMask != 0) {
        size_t i = __builtin_ctz(resetMask);
        ALOG_ASSERT(i < count);
        resetMask &= ~(1 << i);
        sp<Track> track = mActiveTracks[i];
        ALOG_ASSERT(track->isFastTrack() && track->isStopped());
        track->reset();
    }

    // Track destruction may occur outside of threadLoop once it is removed from active tracks.
    // Ensure the AudioMixer doesn't have a raw "buffer provider" pointer to the track if
    // it ceases to be active, to allow safe removal from the AudioMixer at the start
    // of prepareTracks_l(); this releases any outstanding buffer back to the track.
    // See also the implementation of destroyTrack_l().
    for (const auto &track : *tracksToRemove) {
        const int trackId = track->id();
        if (mAudioMixer->exists(trackId)) { // Normal tracks here, fast tracks in FastMixer.
            mAudioMixer->setBufferProvider(trackId, nullptr /* bufferProvider */);
        }
    }

    // remove all the tracks that need to be...
    removeTracks_l(*tracksToRemove);

    if (getEffectChain_l(AUDIO_SESSION_OUTPUT_MIX) != 0) {
        mEffectBufferValid = true;
    }

    if (mEffectBufferValid) {
        // as long as there are effects we should clear the effects buffer, to avoid
        // passing a non-clean buffer to the effect chain
        memset(mEffectBuffer, 0, mEffectBufferSize);
    }
    // sink or mix buffer must be cleared if all tracks are connected to an
    // effect chain as in this case the mixer will not write to the sink or mix buffer
    // and track effects will accumulate into it
    if ((mBytesRemaining == 0) && ((mixedTracks != 0 && mixedTracks == tracksWithEffect) ||
            (mixedTracks == 0 && fastTracks > 0))) {
        // FIXME as a performance optimization, should remember previous zero status
        if (mMixerBufferValid) {
            memset(mMixerBuffer, 0, mMixerBufferSize);
            // TODO: In testing, mSinkBuffer below need not be cleared because
            // the PlaybackThread::threadLoop() copies mMixerBuffer into mSinkBuffer
            // after mixing.
            //
            // To enforce this guarantee:
            // ((mixedTracks != 0 && mixedTracks == tracksWithEffect) ||
            // (mixedTracks == 0 && fastTracks > 0))
            // must imply MIXER_TRACKS_READY.
            // Later, we may clear buffers regardless, and skip much of this logic.
        }
        // FIXME as a performance optimization, should remember previous zero status
        memset(mSinkBuffer, 0, mNormalFrameCount * mFrameSize);
    }

    // if any fast tracks, then status is ready
    mMixerStatusIgnoringFastTracks = mixerStatus;
    if (fastTracks > 0) {
        mixerStatus = MIXER_TRACKS_READY;
    }
    return mixerStatus;
}

// trackCountForUid_l() must be called with ThreadBase::mLock held
uint32_t AudioFlinger::PlaybackThread::trackCountForUid_l(uid_t uid) const
{
    uint32_t trackCount = 0;
    for (size_t i = 0; i < mTracks.size() ; i++) {
        if (mTracks[i]->uid() == uid) {
            trackCount++;
        }
    }
    return trackCount;
}

// isTrackAllowed_l() must be called with ThreadBase::mLock held
bool AudioFlinger::MixerThread::isTrackAllowed_l(
        audio_channel_mask_t channelMask, audio_format_t format,
        audio_session_t sessionId, uid_t uid) const
{
    if (!PlaybackThread::isTrackAllowed_l(channelMask, format, sessionId, uid)) {
        return false;
    }
    // Check validity as we don't call AudioMixer::create() here.
    if (!AudioMixer::isValidFormat(format)) {
        ALOGW("%s: invalid format: %#x", __func__, format);
        return false;
    }
    if (!AudioMixer::isValidChannelMask(channelMask)) {
        ALOGW("%s: invalid channelMask: %#x", __func__, channelMask);
        return false;
    }
    return true;
}

// checkForNewParameter_l() must be called with ThreadBase::mLock held
bool AudioFlinger::MixerThread::checkForNewParameter_l(const String8& keyValuePair,
                                                       status_t& status)
{
    bool reconfig = false;
    bool a2dpDeviceChanged = false;

    status = NO_ERROR;

    AutoPark<FastMixer> park(mFastMixer);

    AudioParameter param = AudioParameter(keyValuePair);
    int value;
    if (param.getInt(String8(AudioParameter::keySamplingRate), value) == NO_ERROR) {
        reconfig = true;
    }
    if (param.getInt(String8(AudioParameter::keyFormat), value) == NO_ERROR) {
        if (!isValidPcmSinkFormat((audio_format_t) value)) {
            status = BAD_VALUE;
        } else {
            // no need to save value, since it's constant
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyChannels), value) == NO_ERROR) {
        if (!isValidPcmSinkChannelMask((audio_channel_mask_t) value)) {
            status = BAD_VALUE;
        } else {
            // no need to save value, since it's constant
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyFrameCount), value) == NO_ERROR) {
        // do not accept frame count changes if tracks are open as the track buffer
        // size depends on frame count and correct behavior would not be guaranteed
        // if frame count is changed after track creation
        if (!mTracks.isEmpty()) {
            status = INVALID_OPERATION;
        } else {
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyRouting), value) == NO_ERROR) {
#ifdef ADD_BATTERY_DATA
        // when changing the audio output device, call addBatteryData to notify
        // the change
        if (mOutDevice != value) {
            uint32_t params = 0;
            // check whether speaker is on
            if (value & AUDIO_DEVICE_OUT_SPEAKER) {
                params |= IMediaPlayerService::kBatteryDataSpeakerOn;
            }

            audio_devices_t deviceWithoutSpeaker
                = AUDIO_DEVICE_OUT_ALL & ~AUDIO_DEVICE_OUT_SPEAKER;
            // check if any other device (except speaker) is on
            if (value & deviceWithoutSpeaker) {
                params |= IMediaPlayerService::kBatteryDataOtherAudioDeviceOn;
            }

            if (params != 0) {
                addBatteryData(params);
            }
        }
#endif

        // forward device change to effects that have requested to be
        // aware of attached audio device.
        if (value != AUDIO_DEVICE_NONE) {
            a2dpDeviceChanged =
                    (mOutDevice & AUDIO_DEVICE_OUT_ALL_A2DP) != (value & AUDIO_DEVICE_OUT_ALL_A2DP);
            mOutDevice = value;
            for (size_t i = 0; i < mEffectChains.size(); i++) {
                mEffectChains[i]->setDevice_l(mOutDevice);
            }
        }
    }

    if (status == NO_ERROR) {
        status = mOutput->stream->setParameters(keyValuePair);
        if (!mStandby && status == INVALID_OPERATION) {
            mOutput->standby();
            mStandby = true;
            mBytesWritten = 0;
            status = mOutput->stream->setParameters(keyValuePair);
        }
        if (status == NO_ERROR && reconfig) {
            readOutputParameters_l();
            delete mAudioMixer;
            mAudioMixer = new AudioMixer(mNormalFrameCount, mSampleRate);
            for (const auto &track : mTracks) {
                const int trackId = track->id();
                status_t status = mAudioMixer->create(
                        trackId,
                        track->mChannelMask,
                        track->mFormat,
                        track->mSessionId);
                ALOGW_IF(status != NO_ERROR,
                        "%s(): AudioMixer cannot create track(%d)"
                        " mask %#x, format %#x, sessionId %d",
                        __func__,
                        trackId, track->mChannelMask, track->mFormat, track->mSessionId);
            }
            sendIoConfigEvent_l(AUDIO_OUTPUT_CONFIG_CHANGED);
        }
    }

    return reconfig || a2dpDeviceChanged;
}


void AudioFlinger::MixerThread::dumpInternals_l(int fd, const Vector<String16>& args)
{
    PlaybackThread::dumpInternals_l(fd, args);
    dprintf(fd, "  Thread throttle time (msecs): %u\n", mThreadThrottleTimeMs);
    dprintf(fd, "  AudioMixer tracks: %s\n", mAudioMixer->trackNames().c_str());
    dprintf(fd, "  Master mono: %s\n", mMasterMono ? "on" : "off");
    dprintf(fd, "  Master balance: %f (%s)\n", mMasterBalance.load(),
            (hasFastMixer() ? std::to_string(mFastMixer->getMasterBalance())
                            : mBalance.toString()).c_str());
    if (hasFastMixer()) {
        dprintf(fd, "  FastMixer thread %p tid=%d", mFastMixer.get(), mFastMixer->getTid());

        // Make a non-atomic copy of fast mixer dump state so it won't change underneath us
        // while we are dumping it.  It may be inconsistent, but it won't mutate!
        // This is a large object so we place it on the heap.
        // FIXME 25972958: Need an intelligent copy constructor that does not touch unused pages.
        const std::unique_ptr<FastMixerDumpState> copy =
                std::make_unique<FastMixerDumpState>(mFastMixerDumpState);
        copy->dump(fd);

#ifdef STATE_QUEUE_DUMP
        // Similar for state queue
        StateQueueObserverDump observerCopy = mStateQueueObserverDump;
        observerCopy.dump(fd);
        StateQueueMutatorDump mutatorCopy = mStateQueueMutatorDump;
        mutatorCopy.dump(fd);
#endif

#ifdef AUDIO_WATCHDOG
        if (mAudioWatchdog != 0) {
            // Make a non-atomic copy of audio watchdog dump so it won't change underneath us
            AudioWatchdogDump wdCopy = mAudioWatchdogDump;
            wdCopy.dump(fd);
        }
#endif

    } else {
        dprintf(fd, "  No FastMixer\n");
    }
}

uint32_t AudioFlinger::MixerThread::idleSleepTimeUs() const
{
    return (uint32_t)(((mNormalFrameCount * 1000) / mSampleRate) * 1000) / 2;
}

uint32_t AudioFlinger::MixerThread::suspendSleepTimeUs() const
{
    return (uint32_t)(((mNormalFrameCount * 1000) / mSampleRate) * 1000);
}

void AudioFlinger::MixerThread::cacheParameters_l()
{
    PlaybackThread::cacheParameters_l();

    // FIXME: Relaxed timing because of a certain device that can't meet latency
    // Should be reduced to 2x after the vendor fixes the driver issue
    // increase threshold again due to low power audio mode. The way this warning
    // threshold is calculated and its usefulness should be reconsidered anyway.
    maxPeriod = seconds(mNormalFrameCount) / mSampleRate * 15;
}

// ----------------------------------------------------------------------------

AudioFlinger::DirectOutputThread::DirectOutputThread(const sp<AudioFlinger>& audioFlinger,
        AudioStreamOut* output, audio_io_handle_t id, audio_devices_t device,
        ThreadBase::type_t type, bool systemReady)
    :   PlaybackThread(audioFlinger, output, id, device, type, systemReady)
{
    setMasterBalance(audioFlinger->getMasterBalance_l());
}

AudioFlinger::DirectOutputThread::~DirectOutputThread()
{
}

void AudioFlinger::DirectOutputThread::dumpInternals_l(int fd, const Vector<String16>& args)
{
    PlaybackThread::dumpInternals_l(fd, args);
    dprintf(fd, "  Master balance: %f  Left: %f  Right: %f\n",
            mMasterBalance.load(), mMasterBalanceLeft, mMasterBalanceRight);
}

void AudioFlinger::DirectOutputThread::setMasterBalance(float balance)
{
    Mutex::Autolock _l(mLock);
    if (mMasterBalance != balance) {
        mMasterBalance.store(balance);
        mBalance.computeStereoBalance(balance, &mMasterBalanceLeft, &mMasterBalanceRight);
        broadcast_l();
    }
}

void AudioFlinger::DirectOutputThread::processVolume_l(Track *track, bool lastTrack)
{
    float left, right;

    // Ensure volumeshaper state always advances even when muted.
    const sp<AudioTrackServerProxy> proxy = track->mAudioTrackServerProxy;
    const auto [shaperVolume, shaperActive] = track->getVolumeHandler()->getVolume(
            proxy->framesReleased());
    mVolumeShaperActive = shaperActive;

    if (mMasterMute || mStreamTypes[track->streamType()].mute || track->isPlaybackRestricted()) {
        left = right = 0;
    } else {
        float typeVolume = mStreamTypes[track->streamType()].volume;
        const float v = mMasterVolume * typeVolume * shaperVolume;

        gain_minifloat_packed_t vlr = proxy->getVolumeLR();
        left = float_from_gain(gain_minifloat_unpack_left(vlr));
        if (left > GAIN_FLOAT_UNITY) {
            left = GAIN_FLOAT_UNITY;
        }
        left *= v * mMasterBalanceLeft; // DirectOutputThread balance applied as track volume
        right = float_from_gain(gain_minifloat_unpack_right(vlr));
        if (right > GAIN_FLOAT_UNITY) {
            right = GAIN_FLOAT_UNITY;
        }
        right *= v * mMasterBalanceRight;
    }

    if (lastTrack) {
        track->setFinalVolume((left + right) / 2.f);
        if (left != mLeftVolFloat || right != mRightVolFloat) {
            mLeftVolFloat = left;
            mRightVolFloat = right;

            // Delegate volume control to effect in track effect chain if needed
            // only one effect chain can be present on DirectOutputThread, so if
            // there is one, the track is connected to it
            if (!mEffectChains.isEmpty()) {
                // if effect chain exists, volume is handled by it.
                // Convert volumes from float to 8.24
                uint32_t vl = (uint32_t)(left * (1 << 24));
                uint32_t vr = (uint32_t)(right * (1 << 24));
                // Direct/Offload effect chains set output volume in setVolume_l().
                (void)mEffectChains[0]->setVolume_l(&vl, &vr);
            } else {
                // otherwise we directly set the volume.
                setVolumeForOutput_l(left, right);
            }
        }
    }
}

void AudioFlinger::DirectOutputThread::onAddNewTrack_l()
{
    sp<Track> previousTrack = mPreviousTrack.promote();
    sp<Track> latestTrack = mActiveTracks.getLatest();

    if (previousTrack != 0 && latestTrack != 0) {
        if (mType == DIRECT) {
            if (previousTrack.get() != latestTrack.get()) {
                mFlushPending = true;
            }
        } else /* mType == OFFLOAD */ {
            if (previousTrack->sessionId() != latestTrack->sessionId()) {
                mFlushPending = true;
            }
        }
    } else if (previousTrack == 0) {
        // there could be an old track added back during track transition for direct
        // output, so always issues flush to flush data of the previous track if it
        // was already destroyed with HAL paused, then flush can resume the playback
        mFlushPending = true;
    }
    PlaybackThread::onAddNewTrack_l();
}

AudioFlinger::PlaybackThread::mixer_state AudioFlinger::DirectOutputThread::prepareTracks_l(
    Vector< sp<Track> > *tracksToRemove
)
{
    size_t count = mActiveTracks.size();
    mixer_state mixerStatus = MIXER_IDLE;
    bool doHwPause = false;
    bool doHwResume = false;

    // find out which tracks need to be processed
    for (const sp<Track> &t : mActiveTracks) {
        if (t->isInvalid()) {
            ALOGW("An invalidated track shouldn't be in active list");
            tracksToRemove->add(t);
            continue;
        }

        Track* const track = t.get();
#ifdef VERY_VERY_VERBOSE_LOGGING
        audio_track_cblk_t* cblk = track->cblk();
#endif
        // Only consider last track started for volume and mixer state control.
        // In theory an older track could underrun and restart after the new one starts
        // but as we only care about the transition phase between two tracks on a
        // direct output, it is not a problem to ignore the underrun case.
        sp<Track> l = mActiveTracks.getLatest();
        bool last = l.get() == track;

        if (track->isPausing()) {
            track->setPaused();
            if (mHwSupportsPause && last && !mHwPaused) {
                doHwPause = true;
                mHwPaused = true;
            }
        } else if (track->isFlushPending()) {
            track->flushAck();
            if (last) {
                mFlushPending = true;
            }
        } else if (track->isResumePending()) {
            track->resumeAck();
            if (last) {
                mLeftVolFloat = mRightVolFloat = -1.0;
                if (mHwPaused) {
                    doHwResume = true;
                    mHwPaused = false;
                }
            }
        }

        // The first time a track is added we wait
        // for all its buffers to be filled before processing it.
        // Allow draining the buffer in case the client
        // app does not call stop() and relies on underrun to stop:
        // hence the test on (track->mRetryCount > 1).
        // If retryCount<=1 then track is about to underrun and be removed.
        // Do not use a high threshold for compressed audio.
        uint32_t minFrames;
        if ((track->sharedBuffer() == 0) && !track->isStopping_1() && !track->isPausing()
            && (track->mRetryCount > 1) && audio_has_proportional_frames(mFormat)) {
            minFrames = mNormalFrameCount;
        } else {
            minFrames = 1;
        }

        if ((track->framesReady() >= minFrames) && track->isReady() && !track->isPaused() &&
                !track->isStopping_2() && !track->isStopped())
        {
            ALOGVV("track(%d) s=%08x [OK]", track->id(), cblk->mServer);

            if (track->mFillingUpStatus == Track::FS_FILLED) {
                track->mFillingUpStatus = Track::FS_ACTIVE;
                if (last) {
                    // make sure processVolume_l() will apply new volume even if 0
                    mLeftVolFloat = mRightVolFloat = -1.0;
                }
                if (!mHwSupportsPause) {
                    track->resumeAck();
                }
            }

            // compute volume for this track
            processVolume_l(track, last);
            if (last) {
                sp<Track> previousTrack = mPreviousTrack.promote();
                if (previousTrack != 0) {
                    if (track != previousTrack.get()) {
                        // Flush any data still being written from last track
                        mBytesRemaining = 0;
                        // Invalidate previous track to force a seek when resuming.
                        previousTrack->invalidate();
                    }
                }
                mPreviousTrack = track;

                // reset retry count
                track->mRetryCount = kMaxTrackRetriesDirect;
                mActiveTrack = t;
                mixerStatus = MIXER_TRACKS_READY;
                if (mHwPaused) {
                    doHwResume = true;
                    mHwPaused = false;
                }
            }
        } else {
            // clear effect chain input buffer if the last active track started underruns
            // to avoid sending previous audio buffer again to effects
            if (!mEffectChains.isEmpty() && last) {
                mEffectChains[0]->clearInputBuffer();
            }
            if (track->isStopping_1()) {
                track->mState = TrackBase::STOPPING_2;
                if (last && mHwPaused) {
                     doHwResume = true;
                     mHwPaused = false;
                 }
            }
            if ((track->sharedBuffer() != 0) || track->isStopped() ||
                    track->isStopping_2() || track->isPaused()) {
                // We have consumed all the buffers of this track.
                // Remove it from the list of active tracks.
                size_t audioHALFrames;
                if (audio_has_proportional_frames(mFormat)) {
                    audioHALFrames = (latency_l() * mSampleRate) / 1000;
                } else {
                    audioHALFrames = 0;
                }

                int64_t framesWritten = mBytesWritten / mFrameSize;
                if (mStandby || !last ||
                        track->presentationComplete(framesWritten, audioHALFrames) ||
                        track->isPaused()) {
                    if (track->isStopping_2()) {
                        track->mState = TrackBase::STOPPED;
                    }
                    if (track->isStopped()) {
                        track->reset();
                    }
                    tracksToRemove->add(track);
                }
            } else {
                // No buffers for this track. Give it a few chances to
                // fill a buffer, then remove it from active list.
                // Only consider last track started for mixer state control
                if (--(track->mRetryCount) <= 0) {
                    ALOGV("BUFFER TIMEOUT: remove track(%d) from active list", track->id());
                    tracksToRemove->add(track);
                    // indicate to client process that the track was disabled because of underrun;
                    // it will then automatically call start() when data is available
                    track->disable();
                } else if (last) {
                    ALOGW("pause because of UNDERRUN, framesReady = %zu,"
                            "minFrames = %u, mFormat = %#x",
                            track->framesReady(), minFrames, mFormat);
                    mixerStatus = MIXER_TRACKS_ENABLED;
                    if (mHwSupportsPause && !mHwPaused && !mStandby) {
                        doHwPause = true;
                        mHwPaused = true;
                    }
                }
            }
        }
    }

    // if an active track did not command a flush, check for pending flush on stopped tracks
    if (!mFlushPending) {
        for (size_t i = 0; i < mTracks.size(); i++) {
            if (mTracks[i]->isFlushPending()) {
                mTracks[i]->flushAck();
                mFlushPending = true;
            }
        }
    }

    // make sure the pause/flush/resume sequence is executed in the right order.
    // If a flush is pending and a track is active but the HW is not paused, force a HW pause
    // before flush and then resume HW. This can happen in case of pause/flush/resume
    // if resume is received before pause is executed.
    if (mHwSupportsPause && !mStandby &&
            (doHwPause || (mFlushPending && !mHwPaused && (count != 0)))) {
        status_t result = mOutput->stream->pause();
        ALOGE_IF(result != OK, "Error when pausing output stream: %d", result);
    }
    if (mFlushPending) {
        flushHw_l();
    }
    if (mHwSupportsPause && !mStandby && doHwResume) {
        status_t result = mOutput->stream->resume();
        ALOGE_IF(result != OK, "Error when resuming output stream: %d", result);
    }
    // remove all the tracks that need to be...
    removeTracks_l(*tracksToRemove);

    return mixerStatus;
}

void AudioFlinger::DirectOutputThread::threadLoop_mix()
{
    size_t frameCount = mFrameCount;
    int8_t *curBuf = (int8_t *)mSinkBuffer;
    // output audio to hardware
    while (frameCount) {
        AudioBufferProvider::Buffer buffer;
        buffer.frameCount = frameCount;
        status_t status = mActiveTrack->getNextBuffer(&buffer);
        if (status != NO_ERROR || buffer.raw == NULL) {
            // no need to pad with 0 for compressed audio
            if (audio_has_proportional_frames(mFormat)) {
                memset(curBuf, 0, frameCount * mFrameSize);
            }
            break;
        }
        memcpy(curBuf, buffer.raw, buffer.frameCount * mFrameSize);
        frameCount -= buffer.frameCount;
        curBuf += buffer.frameCount * mFrameSize;
        mActiveTrack->releaseBuffer(&buffer);
    }
    mCurrentWriteLength = curBuf - (int8_t *)mSinkBuffer;
    mSleepTimeUs = 0;
    mStandbyTimeNs = systemTime() + mStandbyDelayNs;
    mActiveTrack.clear();
}

void AudioFlinger::DirectOutputThread::threadLoop_sleepTime()
{
    // do not write to HAL when paused
    if (mHwPaused || (usesHwAvSync() && mStandby)) {
        mSleepTimeUs = mIdleSleepTimeUs;
        return;
    }
    if (mSleepTimeUs == 0) {
        if (mMixerStatus == MIXER_TRACKS_ENABLED) {
            mSleepTimeUs = mActiveSleepTimeUs;
        } else {
            mSleepTimeUs = mIdleSleepTimeUs;
        }
    } else if (mBytesWritten != 0 && audio_has_proportional_frames(mFormat)) {
        memset(mSinkBuffer, 0, mFrameCount * mFrameSize);
        mSleepTimeUs = 0;
    }
}

void AudioFlinger::DirectOutputThread::threadLoop_exit()
{
    {
        Mutex::Autolock _l(mLock);
        for (size_t i = 0; i < mTracks.size(); i++) {
            if (mTracks[i]->isFlushPending()) {
                mTracks[i]->flushAck();
                mFlushPending = true;
            }
        }
        if (mFlushPending) {
            flushHw_l();
        }
    }
    PlaybackThread::threadLoop_exit();
}

// must be called with thread mutex locked
bool AudioFlinger::DirectOutputThread::shouldStandby_l()
{
    bool trackPaused = false;
    bool trackStopped = false;

    if ((mType == DIRECT) && audio_is_linear_pcm(mFormat) && !usesHwAvSync()) {
        return !mStandby;
    }

    // do not put the HAL in standby when paused. AwesomePlayer clear the offloaded AudioTrack
    // after a timeout and we will enter standby then.
    if (mTracks.size() > 0) {
        trackPaused = mTracks[mTracks.size() - 1]->isPaused();
        trackStopped = mTracks[mTracks.size() - 1]->isStopped() ||
                           mTracks[mTracks.size() - 1]->mState == TrackBase::IDLE;
    }

    return !mStandby && !(trackPaused || (mHwPaused && !trackStopped));
}

// checkForNewParameter_l() must be called with ThreadBase::mLock held
bool AudioFlinger::DirectOutputThread::checkForNewParameter_l(const String8& keyValuePair,
                                                              status_t& status)
{
    bool reconfig = false;
    bool a2dpDeviceChanged = false;

    status = NO_ERROR;

    AudioParameter param = AudioParameter(keyValuePair);
    int value;
    if (param.getInt(String8(AudioParameter::keyRouting), value) == NO_ERROR) {
        // forward device change to effects that have requested to be
        // aware of attached audio device.
        if (value != AUDIO_DEVICE_NONE) {
            a2dpDeviceChanged =
                    (mOutDevice & AUDIO_DEVICE_OUT_ALL_A2DP) != (value & AUDIO_DEVICE_OUT_ALL_A2DP);
            mOutDevice = value;
            for (size_t i = 0; i < mEffectChains.size(); i++) {
                mEffectChains[i]->setDevice_l(mOutDevice);
            }
        }
    }
    if (param.getInt(String8(AudioParameter::keyFrameCount), value) == NO_ERROR) {
        // do not accept frame count changes if tracks are open as the track buffer
        // size depends on frame count and correct behavior would not be garantied
        // if frame count is changed after track creation
        if (!mTracks.isEmpty()) {
            status = INVALID_OPERATION;
        } else {
            reconfig = true;
        }
    }
    if (status == NO_ERROR) {
        status = mOutput->stream->setParameters(keyValuePair);
        if (!mStandby && status == INVALID_OPERATION) {
            mOutput->standby();
            mStandby = true;
            mBytesWritten = 0;
            status = mOutput->stream->setParameters(keyValuePair);
        }
        if (status == NO_ERROR && reconfig) {
            readOutputParameters_l();
            sendIoConfigEvent_l(AUDIO_OUTPUT_CONFIG_CHANGED);
        }
    }

    return reconfig || a2dpDeviceChanged;
}

uint32_t AudioFlinger::DirectOutputThread::activeSleepTimeUs() const
{
    uint32_t time;
    if (audio_has_proportional_frames(mFormat)) {
        time = PlaybackThread::activeSleepTimeUs();
    } else {
        time = kDirectMinSleepTimeUs;
    }
    return time;
}

uint32_t AudioFlinger::DirectOutputThread::idleSleepTimeUs() const
{
    uint32_t time;
    if (audio_has_proportional_frames(mFormat)) {
        time = (uint32_t)(((mFrameCount * 1000) / mSampleRate) * 1000) / 2;
    } else {
        time = kDirectMinSleepTimeUs;
    }
    return time;
}

uint32_t AudioFlinger::DirectOutputThread::suspendSleepTimeUs() const
{
    uint32_t time;
    if (audio_has_proportional_frames(mFormat)) {
        time = (uint32_t)(((mFrameCount * 1000) / mSampleRate) * 1000);
    } else {
        time = kDirectMinSleepTimeUs;
    }
    return time;
}

void AudioFlinger::DirectOutputThread::cacheParameters_l()
{
    PlaybackThread::cacheParameters_l();

    // use shorter standby delay as on normal output to release
    // hardware resources as soon as possible
    // no delay on outputs with HW A/V sync
    if (usesHwAvSync()) {
        mStandbyDelayNs = 0;
    } else if ((mType == OFFLOAD) && !audio_has_proportional_frames(mFormat)) {
        mStandbyDelayNs = kOffloadStandbyDelayNs;
    } else {
        mStandbyDelayNs = microseconds(mActiveSleepTimeUs*2);
    }
}

void AudioFlinger::DirectOutputThread::flushHw_l()
{
    mOutput->flush();
    mHwPaused = false;
    mFlushPending = false;
    mTimestampVerifier.discontinuity(); // DIRECT and OFFLOADED flush resets frame count.
}

int64_t AudioFlinger::DirectOutputThread::computeWaitTimeNs_l() const {
    // If a VolumeShaper is active, we must wake up periodically to update volume.
    const int64_t NS_PER_MS = 1000000;
    return mVolumeShaperActive ?
            kMinNormalSinkBufferSizeMs * NS_PER_MS : PlaybackThread::computeWaitTimeNs_l();
}

// ----------------------------------------------------------------------------

AudioFlinger::AsyncCallbackThread::AsyncCallbackThread(
        const wp<AudioFlinger::PlaybackThread>& playbackThread)
    :   Thread(false /*canCallJava*/),
        mPlaybackThread(playbackThread),
        mWriteAckSequence(0),
        mDrainSequence(0),
        mAsyncError(false)
{
}

AudioFlinger::AsyncCallbackThread::~AsyncCallbackThread()
{
}

void AudioFlinger::AsyncCallbackThread::onFirstRef()
{
    run("Offload Cbk", ANDROID_PRIORITY_URGENT_AUDIO);
}

bool AudioFlinger::AsyncCallbackThread::threadLoop()
{
    while (!exitPending()) {
        uint32_t writeAckSequence;
        uint32_t drainSequence;
        bool asyncError;

        {
            Mutex::Autolock _l(mLock);
            while (!((mWriteAckSequence & 1) ||
                     (mDrainSequence & 1) ||
                     mAsyncError ||
                     exitPending())) {
                mWaitWorkCV.wait(mLock);
            }

            if (exitPending()) {
                break;
            }
            ALOGV("AsyncCallbackThread mWriteAckSequence %d mDrainSequence %d",
                  mWriteAckSequence, mDrainSequence);
            writeAckSequence = mWriteAckSequence;
            mWriteAckSequence &= ~1;
            drainSequence = mDrainSequence;
            mDrainSequence &= ~1;
            asyncError = mAsyncError;
            mAsyncError = false;
        }
        {
            sp<AudioFlinger::PlaybackThread> playbackThread = mPlaybackThread.promote();
            if (playbackThread != 0) {
                if (writeAckSequence & 1) {
                    playbackThread->resetWriteBlocked(writeAckSequence >> 1);
                }
                if (drainSequence & 1) {
                    playbackThread->resetDraining(drainSequence >> 1);
                }
                if (asyncError) {
                    playbackThread->onAsyncError();
                }
            }
        }
    }
    return false;
}

void AudioFlinger::AsyncCallbackThread::exit()
{
    ALOGV("AsyncCallbackThread::exit");
    Mutex::Autolock _l(mLock);
    requestExit();
    mWaitWorkCV.broadcast();
}

void AudioFlinger::AsyncCallbackThread::setWriteBlocked(uint32_t sequence)
{
    Mutex::Autolock _l(mLock);
    // bit 0 is cleared
    mWriteAckSequence = sequence << 1;
}

void AudioFlinger::AsyncCallbackThread::resetWriteBlocked()
{
    Mutex::Autolock _l(mLock);
    // ignore unexpected callbacks
    if (mWriteAckSequence & 2) {
        mWriteAckSequence |= 1;
        mWaitWorkCV.signal();
    }
}

void AudioFlinger::AsyncCallbackThread::setDraining(uint32_t sequence)
{
    Mutex::Autolock _l(mLock);
    // bit 0 is cleared
    mDrainSequence = sequence << 1;
}

void AudioFlinger::AsyncCallbackThread::resetDraining()
{
    Mutex::Autolock _l(mLock);
    // ignore unexpected callbacks
    if (mDrainSequence & 2) {
        mDrainSequence |= 1;
        mWaitWorkCV.signal();
    }
}

void AudioFlinger::AsyncCallbackThread::setAsyncError()
{
    Mutex::Autolock _l(mLock);
    mAsyncError = true;
    mWaitWorkCV.signal();
}


// ----------------------------------------------------------------------------
AudioFlinger::OffloadThread::OffloadThread(const sp<AudioFlinger>& audioFlinger,
        AudioStreamOut* output, audio_io_handle_t id, uint32_t device, bool systemReady)
    :   DirectOutputThread(audioFlinger, output, id, device, OFFLOAD, systemReady),
        mPausedWriteLength(0), mPausedBytesRemaining(0), mKeepWakeLock(true),
        mOffloadUnderrunPosition(~0LL)
{
    //FIXME: mStandby should be set to true by ThreadBase constructo
    mStandby = true;
    mKeepWakeLock = property_get_bool("ro.audio.offload_wakelock", true /* default_value */);
}

void AudioFlinger::OffloadThread::threadLoop_exit()
{
    if (mFlushPending || mHwPaused) {
        // If a flush is pending or track was paused, just discard buffered data
        flushHw_l();
    } else {
        mMixerStatus = MIXER_DRAIN_ALL;
        threadLoop_drain();
    }
    if (mUseAsyncWrite) {
        ALOG_ASSERT(mCallbackThread != 0);
        mCallbackThread->exit();
    }
    PlaybackThread::threadLoop_exit();
}

AudioFlinger::PlaybackThread::mixer_state AudioFlinger::OffloadThread::prepareTracks_l(
    Vector< sp<Track> > *tracksToRemove
)
{
    size_t count = mActiveTracks.size();

    mixer_state mixerStatus = MIXER_IDLE;
    bool doHwPause = false;
    bool doHwResume = false;

    ALOGV("OffloadThread::prepareTracks_l active tracks %zu", count);

    // find out which tracks need to be processed
    for (const sp<Track> &t : mActiveTracks) {
        Track* const track = t.get();
#ifdef VERY_VERY_VERBOSE_LOGGING
        audio_track_cblk_t* cblk = track->cblk();
#endif
        // Only consider last track started for volume and mixer state control.
        // In theory an older track could underrun and restart after the new one starts
        // but as we only care about the transition phase between two tracks on a
        // direct output, it is not a problem to ignore the underrun case.
        sp<Track> l = mActiveTracks.getLatest();
        bool last = l.get() == track;

        if (track->isInvalid()) {
            ALOGW("An invalidated track shouldn't be in active list");
            tracksToRemove->add(track);
            continue;
        }

        if (track->mState == TrackBase::IDLE) {
            ALOGW("An idle track shouldn't be in active list");
            continue;
        }

        if (track->isPausing()) {
            track->setPaused();
            if (last) {
                if (mHwSupportsPause && !mHwPaused) {
                    doHwPause = true;
                    mHwPaused = true;
                }
                // If we were part way through writing the mixbuffer to
                // the HAL we must save this until we resume
                // BUG - this will be wrong if a different track is made active,
                // in that case we want to discard the pending data in the
                // mixbuffer and tell the client to present it again when the
                // track is resumed
                mPausedWriteLength = mCurrentWriteLength;
                mPausedBytesRemaining = mBytesRemaining;
                mBytesRemaining = 0;    // stop writing
            }
            tracksToRemove->add(track);
        } else if (track->isFlushPending()) {
            if (track->isStopping_1()) {
                track->mRetryCount = kMaxTrackStopRetriesOffload;
            } else {
                track->mRetryCount = kMaxTrackRetriesOffload;
            }
            track->flushAck();
            if (last) {
                mFlushPending = true;
            }
        } else if (track->isResumePending()){
            track->resumeAck();
            if (last) {
                if (mPausedBytesRemaining) {
                    // Need to continue write that was interrupted
                    mCurrentWriteLength = mPausedWriteLength;
                    mBytesRemaining = mPausedBytesRemaining;
                    mPausedBytesRemaining = 0;
                }
                if (mHwPaused) {
                    doHwResume = true;
                    mHwPaused = false;
                    // threadLoop_mix() will handle the case that we need to
                    // resume an interrupted write
                }
                // enable write to audio HAL
                mSleepTimeUs = 0;

                mLeftVolFloat = mRightVolFloat = -1.0;

                // Do not handle new data in this iteration even if track->framesReady()
                mixerStatus = MIXER_TRACKS_ENABLED;
            }
        }  else if (track->framesReady() && track->isReady() &&
                !track->isPaused() && !track->isTerminated() && !track->isStopping_2()) {
            ALOGVV("OffloadThread: track(%d) s=%08x [OK]", track->id(), cblk->mServer);
            if (track->mFillingUpStatus == Track::FS_FILLED) {
                track->mFillingUpStatus = Track::FS_ACTIVE;
                if (last) {
                    // make sure processVolume_l() will apply new volume even if 0
                    mLeftVolFloat = mRightVolFloat = -1.0;
                }
            }

            if (last) {
                sp<Track> previousTrack = mPreviousTrack.promote();
                if (previousTrack != 0) {
                    if (track != previousTrack.get()) {
                        // Flush any data still being written from last track
                        mBytesRemaining = 0;
                        if (mPausedBytesRemaining) {
                            // Last track was paused so we also need to flush saved
                            // mixbuffer state and invalidate track so that it will
                            // re-submit that unwritten data when it is next resumed
                            mPausedBytesRemaining = 0;
                            // Invalidate is a bit drastic - would be more efficient
                            // to have a flag to tell client that some of the
                            // previously written data was lost
                            previousTrack->invalidate();
                        }
                        // flush data already sent to the DSP if changing audio session as audio
                        // comes from a different source. Also invalidate previous track to force a
                        // seek when resuming.
                        if (previousTrack->sessionId() != track->sessionId()) {
                            previousTrack->invalidate();
                        }
                    }
                }
                mPreviousTrack = track;
                // reset retry count
                if (track->isStopping_1()) {
                    track->mRetryCount = kMaxTrackStopRetriesOffload;
                } else {
                    track->mRetryCount = kMaxTrackRetriesOffload;
                }
                mActiveTrack = t;
                mixerStatus = MIXER_TRACKS_READY;
            }
        } else {
            ALOGVV("OffloadThread: track(%d) s=%08x [NOT READY]", track->id(), cblk->mServer);
            if (track->isStopping_1()) {
                if (--(track->mRetryCount) <= 0) {
                    // Hardware buffer can hold a large amount of audio so we must
                    // wait for all current track's data to drain before we say
                    // that the track is stopped.
                    if (mBytesRemaining == 0) {
                        // Only start draining when all data in mixbuffer
                        // has been written
                        ALOGV("OffloadThread: underrun and STOPPING_1 -> draining, STOPPING_2");
                        track->mState = TrackBase::STOPPING_2; // so presentation completes after
                        // drain do not drain if no data was ever sent to HAL (mStandby == true)
                        if (last && !mStandby) {
                            // do not modify drain sequence if we are already draining. This happens
                            // when resuming from pause after drain.
                            if ((mDrainSequence & 1) == 0) {
                                mSleepTimeUs = 0;
                                mStandbyTimeNs = systemTime() + mStandbyDelayNs;
                                mixerStatus = MIXER_DRAIN_TRACK;
                                mDrainSequence += 2;
                            }
                            if (mHwPaused) {
                                // It is possible to move from PAUSED to STOPPING_1 without
                                // a resume so we must ensure hardware is running
                                doHwResume = true;
                                mHwPaused = false;
                            }
                        }
                    }
                } else if (last) {
                    ALOGV("stopping1 underrun retries left %d", track->mRetryCount);
                    mixerStatus = MIXER_TRACKS_ENABLED;
                }
            } else if (track->isStopping_2()) {
                // Drain has completed or we are in standby, signal presentation complete
                if (!(mDrainSequence & 1) || !last || mStandby) {
                    track->mState = TrackBase::STOPPED;
                    uint32_t latency = 0;
                    status_t result = mOutput->stream->getLatency(&latency);
                    ALOGE_IF(result != OK,
                            "Error when retrieving output stream latency: %d", result);
                    size_t audioHALFrames = (latency * mSampleRate) / 1000;
                    int64_t framesWritten =
                            mBytesWritten / mOutput->getFrameSize();
                    track->presentationComplete(framesWritten, audioHALFrames);
                    track->reset();
                    tracksToRemove->add(track);
                    // DIRECT and OFFLOADED stop resets frame counts.
                    if (!mUseAsyncWrite) {
                        // If we don't get explicit drain notification we must
                        // register discontinuity regardless of whether this is
                        // the previous (!last) or the upcoming (last) track
                        // to avoid skipping the discontinuity.
                        mTimestampVerifier.discontinuity();
                    }
                }
            } else {
                // No buffers for this track. Give it a few chances to
                // fill a buffer, then remove it from active list.
                if (--(track->mRetryCount) <= 0) {
                    bool running = false;
                    uint64_t position = 0;
                    struct timespec unused;
                    // The running check restarts the retry counter at least once.
                    status_t ret = mOutput->stream->getPresentationPosition(&position, &unused);
                    if (ret == NO_ERROR && position != mOffloadUnderrunPosition) {
                        running = true;
                        mOffloadUnderrunPosition = position;
                    }
                    if (ret == NO_ERROR) {
                        ALOGVV("underrun counter, running(%d): %lld vs %lld", running,
                                (long long)position, (long long)mOffloadUnderrunPosition);
                    }
                    if (running) { // still running, give us more time.
                        track->mRetryCount = kMaxTrackRetriesOffload;
                    } else {
                        ALOGV("OffloadThread: BUFFER TIMEOUT: remove track(%d) from active list",
                                track->id());
                        tracksToRemove->add(track);
                        // tell client process that the track was disabled because of underrun;
                        // it will then automatically call start() when data is available
                        track->disable();
                    }
                } else if (last){
                    mixerStatus = MIXER_TRACKS_ENABLED;
                }
            }
        }
        // compute volume for this track
        processVolume_l(track, last);
    }

    // make sure the pause/flush/resume sequence is executed in the right order.
    // If a flush is pending and a track is active but the HW is not paused, force a HW pause
    // before flush and then resume HW. This can happen in case of pause/flush/resume
    // if resume is received before pause is executed.
    if (!mStandby && (doHwPause || (mFlushPending && !mHwPaused && (count != 0)))) {
        status_t result = mOutput->stream->pause();
        ALOGE_IF(result != OK, "Error when pausing output stream: %d", result);
    }
    if (mFlushPending) {
        flushHw_l();
    }
    if (!mStandby && doHwResume) {
        status_t result = mOutput->stream->resume();
        ALOGE_IF(result != OK, "Error when resuming output stream: %d", result);
    }

    // remove all the tracks that need to be...
    removeTracks_l(*tracksToRemove);

    return mixerStatus;
}

// must be called with thread mutex locked
bool AudioFlinger::OffloadThread::waitingAsyncCallback_l()
{
    ALOGVV("waitingAsyncCallback_l mWriteAckSequence %d mDrainSequence %d",
          mWriteAckSequence, mDrainSequence);
    if (mUseAsyncWrite && ((mWriteAckSequence & 1) || (mDrainSequence & 1))) {
        return true;
    }
    return false;
}

bool AudioFlinger::OffloadThread::waitingAsyncCallback()
{
    Mutex::Autolock _l(mLock);
    return waitingAsyncCallback_l();
}

void AudioFlinger::OffloadThread::flushHw_l()
{
    DirectOutputThread::flushHw_l();
    // Flush anything still waiting in the mixbuffer
    mCurrentWriteLength = 0;
    mBytesRemaining = 0;
    mPausedWriteLength = 0;
    mPausedBytesRemaining = 0;
    // reset bytes written count to reflect that DSP buffers are empty after flush.
    mBytesWritten = 0;
    mOffloadUnderrunPosition = ~0LL;

    if (mUseAsyncWrite) {
        // discard any pending drain or write ack by incrementing sequence
        mWriteAckSequence = (mWriteAckSequence + 2) & ~1;
        mDrainSequence = (mDrainSequence + 2) & ~1;
        ALOG_ASSERT(mCallbackThread != 0);
        mCallbackThread->setWriteBlocked(mWriteAckSequence);
        mCallbackThread->setDraining(mDrainSequence);
    }
}

void AudioFlinger::OffloadThread::invalidateTracks(audio_stream_type_t streamType)
{
    Mutex::Autolock _l(mLock);
    if (PlaybackThread::invalidateTracks_l(streamType)) {
        mFlushPending = true;
    }
}

// ----------------------------------------------------------------------------

AudioFlinger::DuplicatingThread::DuplicatingThread(const sp<AudioFlinger>& audioFlinger,
        AudioFlinger::MixerThread* mainThread, audio_io_handle_t id, bool systemReady)
    :   MixerThread(audioFlinger, mainThread->getOutput(), id, mainThread->outDevice(),
                    systemReady, DUPLICATING),
        mWaitTimeMs(UINT_MAX)
{
    addOutputTrack(mainThread);
}

AudioFlinger::DuplicatingThread::~DuplicatingThread()
{
    for (size_t i = 0; i < mOutputTracks.size(); i++) {
        mOutputTracks[i]->destroy();
    }
}

void AudioFlinger::DuplicatingThread::threadLoop_mix()
{
    // mix buffers...
    if (outputsReady(outputTracks)) {
        mAudioMixer->process();
    } else {
        if (mMixerBufferValid) {
            memset(mMixerBuffer, 0, mMixerBufferSize);
        } else {
            memset(mSinkBuffer, 0, mSinkBufferSize);
        }
    }
    mSleepTimeUs = 0;
    writeFrames = mNormalFrameCount;
    mCurrentWriteLength = mSinkBufferSize;
    mStandbyTimeNs = systemTime() + mStandbyDelayNs;
}

void AudioFlinger::DuplicatingThread::threadLoop_sleepTime()
{
    if (mSleepTimeUs == 0) {
        if (mMixerStatus == MIXER_TRACKS_ENABLED) {
            mSleepTimeUs = mActiveSleepTimeUs;
        } else {
            mSleepTimeUs = mIdleSleepTimeUs;
        }
    } else if (mBytesWritten != 0) {
        if (mMixerStatus == MIXER_TRACKS_ENABLED) {
            writeFrames = mNormalFrameCount;
            memset(mSinkBuffer, 0, mSinkBufferSize);
        } else {
            // flush remaining overflow buffers in output tracks
            writeFrames = 0;
        }
        mSleepTimeUs = 0;
    }
}

ssize_t AudioFlinger::DuplicatingThread::threadLoop_write()
{
    for (size_t i = 0; i < outputTracks.size(); i++) {
        const ssize_t actualWritten = outputTracks[i]->write(mSinkBuffer, writeFrames);

        // Consider the first OutputTrack for timestamp and frame counting.

        // The threadLoop() generally assumes writing a full sink buffer size at a time.
        // Here, we correct for writeFrames of 0 (a stop) or underruns because
        // we always claim success.
        if (i == 0) {
            const ssize_t correction = mSinkBufferSize / mFrameSize - actualWritten;
            ALOGD_IF(correction != 0 && writeFrames != 0,
                    "%s: writeFrames:%u  actualWritten:%zd  correction:%zd  mFramesWritten:%lld",
                    __func__, writeFrames, actualWritten, correction, (long long)mFramesWritten);
            mFramesWritten -= correction;
        }

        // TODO: Report correction for the other output tracks and show in the dump.
    }
    mStandby = false;
    return (ssize_t)mSinkBufferSize;
}

void AudioFlinger::DuplicatingThread::threadLoop_standby()
{
    // DuplicatingThread implements standby by stopping all tracks
    for (size_t i = 0; i < outputTracks.size(); i++) {
        outputTracks[i]->stop();
    }
}

void AudioFlinger::DuplicatingThread::dumpInternals_l(int fd, const Vector<String16>& args __unused)
{
    MixerThread::dumpInternals_l(fd, args);

    std::stringstream ss;
    const size_t numTracks = mOutputTracks.size();
    ss << "  " << numTracks << " OutputTracks";
    if (numTracks > 0) {
        ss << ":";
        for (const auto &track : mOutputTracks) {
            const sp<ThreadBase> thread = track->thread().promote();
            ss << " (" << track->id() << " : ";
            if (thread.get() != nullptr) {
                ss << thread.get() << ", " << thread->id();
            } else {
                ss << "null";
            }
            ss << ")";
        }
    }
    ss << "\n";
    std::string result = ss.str();
    write(fd, result.c_str(), result.size());
}

void AudioFlinger::DuplicatingThread::saveOutputTracks()
{
    outputTracks = mOutputTracks;
}

void AudioFlinger::DuplicatingThread::clearOutputTracks()
{
    outputTracks.clear();
}

void AudioFlinger::DuplicatingThread::addOutputTrack(MixerThread *thread)
{
    Mutex::Autolock _l(mLock);
    // The downstream MixerThread consumes thread->frameCount() amount of frames per mix pass.
    // Adjust for thread->sampleRate() to determine minimum buffer frame count.
    // Then triple buffer because Threads do not run synchronously and may not be clock locked.
    const size_t frameCount =
            3 * sourceFramesNeeded(mSampleRate, thread->frameCount(), thread->sampleRate());
    // TODO: Consider asynchronous sample rate conversion to handle clock disparity
    // from different OutputTracks and their associated MixerThreads (e.g. one may
    // nearly empty and the other may be dropping data).

    sp<OutputTrack> outputTrack = new OutputTrack(thread,
                                            this,
                                            mSampleRate,
                                            mFormat,
                                            mChannelMask,
                                            frameCount,
                                            IPCThreadState::self()->getCallingUid());
    status_t status = outputTrack != 0 ? outputTrack->initCheck() : (status_t) NO_MEMORY;
    if (status != NO_ERROR) {
        ALOGE("addOutputTrack() initCheck failed %d", status);
        return;
    }
    thread->setStreamVolume(AUDIO_STREAM_PATCH, 1.0f);
    mOutputTracks.add(outputTrack);
    ALOGV("addOutputTrack() track %p, on thread %p", outputTrack.get(), thread);
    updateWaitTime_l();
}

void AudioFlinger::DuplicatingThread::removeOutputTrack(MixerThread *thread)
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mOutputTracks.size(); i++) {
        if (mOutputTracks[i]->thread() == thread) {
            mOutputTracks[i]->destroy();
            mOutputTracks.removeAt(i);
            updateWaitTime_l();
            if (thread->getOutput() == mOutput) {
                mOutput = NULL;
            }
            return;
        }
    }
    ALOGV("removeOutputTrack(): unknown thread: %p", thread);
}

// caller must hold mLock
void AudioFlinger::DuplicatingThread::updateWaitTime_l()
{
    mWaitTimeMs = UINT_MAX;
    for (size_t i = 0; i < mOutputTracks.size(); i++) {
        sp<ThreadBase> strong = mOutputTracks[i]->thread().promote();
        if (strong != 0) {
            uint32_t waitTimeMs = (strong->frameCount() * 2 * 1000) / strong->sampleRate();
            if (waitTimeMs < mWaitTimeMs) {
                mWaitTimeMs = waitTimeMs;
            }
        }
    }
}


bool AudioFlinger::DuplicatingThread::outputsReady(
        const SortedVector< sp<OutputTrack> > &outputTracks)
{
    for (size_t i = 0; i < outputTracks.size(); i++) {
        sp<ThreadBase> thread = outputTracks[i]->thread().promote();
        if (thread == 0) {
            ALOGW("DuplicatingThread::outputsReady() could not promote thread on output track %p",
                    outputTracks[i].get());
            return false;
        }
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
        // see note at standby() declaration
        if (playbackThread->standby() && !playbackThread->isSuspended()) {
            ALOGV("DuplicatingThread output track %p on thread %p Not Ready", outputTracks[i].get(),
                    thread.get());
            return false;
        }
    }
    return true;
}

void AudioFlinger::DuplicatingThread::sendMetadataToBackend_l(
        const StreamOutHalInterface::SourceMetadata& metadata)
{
    for (auto& outputTrack : outputTracks) { // not mOutputTracks
        outputTrack->setMetadatas(metadata.tracks);
    }
}

uint32_t AudioFlinger::DuplicatingThread::activeSleepTimeUs() const
{
    return (mWaitTimeMs * 1000) / 2;
}

void AudioFlinger::DuplicatingThread::cacheParameters_l()
{
    // updateWaitTime_l() sets mWaitTimeMs, which affects activeSleepTimeUs(), so call it first
    updateWaitTime_l();

    MixerThread::cacheParameters_l();
}


// ----------------------------------------------------------------------------
//      Record
// ----------------------------------------------------------------------------

AudioFlinger::RecordThread::RecordThread(const sp<AudioFlinger>& audioFlinger,
                                         AudioStreamIn *input,
                                         audio_io_handle_t id,
                                         audio_devices_t outDevice,
                                         audio_devices_t inDevice,
                                         bool systemReady
                                         ) :
    ThreadBase(audioFlinger, id, outDevice, inDevice, RECORD, systemReady),
    mInput(input),
    mActiveTracks(&this->mLocalLog),
    mRsmpInBuffer(NULL),
    // mRsmpInFrames, mRsmpInFramesP2, and mRsmpInFramesOA are set by readInputParameters_l()
    mRsmpInRear(0)
    , mReadOnlyHeap(new MemoryDealer(kRecordThreadReadOnlyHeapSize,
            "RecordThreadRO", MemoryHeapBase::READ_ONLY))
    // mFastCapture below
    , mFastCaptureFutex(0)
    // mInputSource
    // mPipeSink
    // mPipeSource
    , mPipeFramesP2(0)
    // mPipeMemory
    // mFastCaptureNBLogWriter
    , mFastTrackAvail(false)
    , mBtNrecSuspended(false)
{
    snprintf(mThreadName, kThreadNameLength, "AudioIn_%X", id);
    mNBLogWriter = audioFlinger->newWriter_l(kLogSize, mThreadName);

    if (mInput != nullptr && mInput->audioHwDev != nullptr) {
        mIsMsdDevice = strcmp(
                mInput->audioHwDev->moduleName(), AUDIO_HARDWARE_MODULE_ID_MSD) == 0;
    }

    readInputParameters_l();

    // TODO: We may also match on address as well as device type for
    // AUDIO_DEVICE_IN_BUS, AUDIO_DEVICE_IN_BLUETOOTH_A2DP, AUDIO_DEVICE_IN_REMOTE_SUBMIX
    mTimestampCorrectedDevices = (audio_devices_t)property_get_int64(
            "audio.timestamp.corrected_input_devices",
            (int64_t)(mIsMsdDevice ? AUDIO_DEVICE_IN_BUS // turn on by default for MSD
                                   : AUDIO_DEVICE_NONE));

    // create an NBAIO source for the HAL input stream, and negotiate
    mInputSource = new AudioStreamInSource(input->stream);
    size_t numCounterOffers = 0;
    const NBAIO_Format offers[1] = {Format_from_SR_C(mSampleRate, mChannelCount, mFormat)};
#if !LOG_NDEBUG
    ssize_t index =
#else
    (void)
#endif
            mInputSource->negotiate(offers, 1, NULL, numCounterOffers);
    ALOG_ASSERT(index == 0);

    // initialize fast capture depending on configuration
    bool initFastCapture;
    switch (kUseFastCapture) {
    case FastCapture_Never:
        initFastCapture = false;
        ALOGV("%p kUseFastCapture = Never, initFastCapture = false", this);
        break;
    case FastCapture_Always:
        initFastCapture = true;
        ALOGV("%p kUseFastCapture = Always, initFastCapture = true", this);
        break;
    case FastCapture_Static:
        initFastCapture = (mFrameCount * 1000) / mSampleRate < kMinNormalCaptureBufferSizeMs;
        ALOGV("%p kUseFastCapture = Static, (%lld * 1000) / %u vs %u, initFastCapture = %d",
                this, (long long)mFrameCount, mSampleRate, kMinNormalCaptureBufferSizeMs,
                initFastCapture);
        break;
    // case FastCapture_Dynamic:
    }

    if (initFastCapture) {
        // create a Pipe for FastCapture to write to, and for us and fast tracks to read from
        NBAIO_Format format = mInputSource->format();
        // quadruple-buffering of 20 ms each; this ensures we can sleep for 20ms in RecordThread
        size_t pipeFramesP2 = roundup(4 * FMS_20 * mSampleRate / 1000);
        size_t pipeSize = pipeFramesP2 * Format_frameSize(format);
        void *pipeBuffer = nullptr;
        const sp<MemoryDealer> roHeap(readOnlyHeap());
        sp<IMemory> pipeMemory;
        if ((roHeap == 0) ||
                (pipeMemory = roHeap->allocate(pipeSize)) == 0 ||
                (pipeBuffer = pipeMemory->pointer()) == nullptr) {
            ALOGE("not enough memory for pipe buffer size=%zu; "
                    "roHeap=%p, pipeMemory=%p, pipeBuffer=%p; roHeapSize: %lld",
                    pipeSize, roHeap.get(), pipeMemory.get(), pipeBuffer,
                    (long long)kRecordThreadReadOnlyHeapSize);
            goto failed;
        }
        // pipe will be shared directly with fast clients, so clear to avoid leaking old information
        memset(pipeBuffer, 0, pipeSize);
        Pipe *pipe = new Pipe(pipeFramesP2, format, pipeBuffer);
        const NBAIO_Format offers[1] = {format};
        size_t numCounterOffers = 0;
        ssize_t index = pipe->negotiate(offers, 1, NULL, numCounterOffers);
        ALOG_ASSERT(index == 0);
        mPipeSink = pipe;
        PipeReader *pipeReader = new PipeReader(*pipe);
        numCounterOffers = 0;
        index = pipeReader->negotiate(offers, 1, NULL, numCounterOffers);
        ALOG_ASSERT(index == 0);
        mPipeSource = pipeReader;
        mPipeFramesP2 = pipeFramesP2;
        mPipeMemory = pipeMemory;

        // create fast capture
        mFastCapture = new FastCapture();
        FastCaptureStateQueue *sq = mFastCapture->sq();
#ifdef STATE_QUEUE_DUMP
        // FIXME
#endif
        FastCaptureState *state = sq->begin();
        state->mCblk = NULL;
        state->mInputSource = mInputSource.get();
        state->mInputSourceGen++;
        state->mPipeSink = pipe;
        state->mPipeSinkGen++;
        state->mFrameCount = mFrameCount;
        state->mCommand = FastCaptureState::COLD_IDLE;
        // already done in constructor initialization list
        //mFastCaptureFutex = 0;
        state->mColdFutexAddr = &mFastCaptureFutex;
        state->mColdGen++;
        state->mDumpState = &mFastCaptureDumpState;
#ifdef TEE_SINK
        // FIXME
#endif
        mFastCaptureNBLogWriter = audioFlinger->newWriter_l(kFastCaptureLogSize, "FastCapture");
        state->mNBLogWriter = mFastCaptureNBLogWriter.get();
        sq->end();
        sq->push(FastCaptureStateQueue::BLOCK_UNTIL_PUSHED);

        // start the fast capture
        mFastCapture->run("FastCapture", ANDROID_PRIORITY_URGENT_AUDIO);
        pid_t tid = mFastCapture->getTid();
        sendPrioConfigEvent(getpid(), tid, kPriorityFastCapture, false /*forApp*/);
        stream()->setHalThreadPriority(kPriorityFastCapture);
#ifdef AUDIO_WATCHDOG
        // FIXME
#endif

        mFastTrackAvail = true;
    }
#ifdef TEE_SINK
    mTee.set(mInputSource->format(), NBAIO_Tee::TEE_FLAG_INPUT_THREAD);
    mTee.setId(std::string("_") + std::to_string(mId) + "_C");
#endif
failed: ;

    // FIXME mNormalSource
}

AudioFlinger::RecordThread::~RecordThread()
{
    if (mFastCapture != 0) {
        FastCaptureStateQueue *sq = mFastCapture->sq();
        FastCaptureState *state = sq->begin();
        if (state->mCommand == FastCaptureState::COLD_IDLE) {
            int32_t old = android_atomic_inc(&mFastCaptureFutex);
            if (old == -1) {
                (void) syscall(__NR_futex, &mFastCaptureFutex, FUTEX_WAKE_PRIVATE, 1);
            }
        }
        state->mCommand = FastCaptureState::EXIT;
        sq->end();
        sq->push(FastCaptureStateQueue::BLOCK_UNTIL_PUSHED);
        mFastCapture->join();
        mFastCapture.clear();
    }
    mAudioFlinger->unregisterWriter(mFastCaptureNBLogWriter);
    mAudioFlinger->unregisterWriter(mNBLogWriter);
    free(mRsmpInBuffer);
}

void AudioFlinger::RecordThread::onFirstRef()
{
    run(mThreadName, PRIORITY_URGENT_AUDIO);
}

void AudioFlinger::RecordThread::preExit()
{
    ALOGV("  preExit()");
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mTracks.size(); i++) {
        sp<RecordTrack> track = mTracks[i];
        track->invalidate();
    }
    mActiveTracks.clear();
    mStartStopCond.broadcast();
}

bool AudioFlinger::RecordThread::threadLoop()
{
    nsecs_t lastWarning = 0;

    inputStandBy();

reacquire_wakelock:
    sp<RecordTrack> activeTrack;
    {
        Mutex::Autolock _l(mLock);
        acquireWakeLock_l();
    }

    // used to request a deferred sleep, to be executed later while mutex is unlocked
    uint32_t sleepUs = 0;

    int64_t lastLoopCountRead = -2;  // never matches "previous" loop, when loopCount = 0.

    // loop while there is work to do
    for (int64_t loopCount = 0;; ++loopCount) {  // loopCount used for statistics tracking
        Vector< sp<EffectChain> > effectChains;

        // activeTracks accumulates a copy of a subset of mActiveTracks
        Vector< sp<RecordTrack> > activeTracks;

        // reference to the (first and only) active fast track
        sp<RecordTrack> fastTrack;

        // reference to a fast track which is about to be removed
        sp<RecordTrack> fastTrackToRemove;

        { // scope for mLock
            Mutex::Autolock _l(mLock);

            processConfigEvents_l();

            // check exitPending here because checkForNewParameters_l() and
            // checkForNewParameters_l() can temporarily release mLock
            if (exitPending()) {
                break;
            }

            // sleep with mutex unlocked
            if (sleepUs > 0) {
                ATRACE_BEGIN("sleepC");
                mWaitWorkCV.waitRelative(mLock, microseconds((nsecs_t)sleepUs));
                ATRACE_END();
                sleepUs = 0;
                continue;
            }

            // if no active track(s), then standby and release wakelock
            size_t size = mActiveTracks.size();
            if (size == 0) {
                standbyIfNotAlreadyInStandby();
                // exitPending() can't become true here
                releaseWakeLock_l();
                ALOGV("RecordThread: loop stopping");
                // go to sleep
                mWaitWorkCV.wait(mLock);
                ALOGV("RecordThread: loop starting");
                goto reacquire_wakelock;
            }

            bool doBroadcast = false;
            bool allStopped = true;
            for (size_t i = 0; i < size; ) {

                activeTrack = mActiveTracks[i];
                if (activeTrack->isTerminated()) {
                    if (activeTrack->isFastTrack()) {
                        ALOG_ASSERT(fastTrackToRemove == 0);
                        fastTrackToRemove = activeTrack;
                    }
                    removeTrack_l(activeTrack);
                    mActiveTracks.remove(activeTrack);
                    size--;
                    continue;
                }

                TrackBase::track_state activeTrackState = activeTrack->mState;
                switch (activeTrackState) {

                case TrackBase::PAUSING:
                    mActiveTracks.remove(activeTrack);
                    activeTrack->mState = TrackBase::PAUSED;
                    doBroadcast = true;
                    size--;
                    continue;

                case TrackBase::STARTING_1:
                    sleepUs = 10000;
                    i++;
                    allStopped = false;
                    continue;

                case TrackBase::STARTING_2:
                    doBroadcast = true;
                    mStandby = false;
                    activeTrack->mState = TrackBase::ACTIVE;
                    allStopped = false;
                    break;

                case TrackBase::ACTIVE:
                    allStopped = false;
                    break;

                case TrackBase::IDLE:    // cannot be on ActiveTracks if idle
                case TrackBase::PAUSED:  // cannot be on ActiveTracks if paused
                case TrackBase::STOPPED: // cannot be on ActiveTracks if destroyed/terminated
                default:
                    LOG_ALWAYS_FATAL("%s: Unexpected active track state:%d, id:%d, tracks:%zu",
                            __func__, activeTrackState, activeTrack->id(), size);
                }

                activeTracks.add(activeTrack);
                i++;

                if (activeTrack->isFastTrack()) {
                    ALOG_ASSERT(!mFastTrackAvail);
                    ALOG_ASSERT(fastTrack == 0);
                    fastTrack = activeTrack;
                }
            }

            mActiveTracks.updatePowerState(this);

            updateMetadata_l();

            if (allStopped) {
                standbyIfNotAlreadyInStandby();
            }
            if (doBroadcast) {
                mStartStopCond.broadcast();
            }

            // sleep if there are no active tracks to process
            if (activeTracks.isEmpty()) {
                if (sleepUs == 0) {
                    sleepUs = kRecordThreadSleepUs;
                }
                continue;
            }
            sleepUs = 0;

            lockEffectChains_l(effectChains);
        }

        // thread mutex is now unlocked, mActiveTracks unknown, activeTracks.size() > 0

        size_t size = effectChains.size();
        for (size_t i = 0; i < size; i++) {
            // thread mutex is not locked, but effect chain is locked
            effectChains[i]->process_l();
        }

        // Push a new fast capture state if fast capture is not already running, or cblk change
        if (mFastCapture != 0) {
            FastCaptureStateQueue *sq = mFastCapture->sq();
            FastCaptureState *state = sq->begin();
            bool didModify = false;
            FastCaptureStateQueue::block_t block = FastCaptureStateQueue::BLOCK_UNTIL_PUSHED;
            if (state->mCommand != FastCaptureState::READ_WRITE /* FIXME &&
                    (kUseFastMixer != FastMixer_Dynamic || state->mTrackMask > 1)*/) {
                if (state->mCommand == FastCaptureState::COLD_IDLE) {
                    int32_t old = android_atomic_inc(&mFastCaptureFutex);
                    if (old == -1) {
                        (void) syscall(__NR_futex, &mFastCaptureFutex, FUTEX_WAKE_PRIVATE, 1);
                    }
                }
                state->mCommand = FastCaptureState::READ_WRITE;
#if 0   // FIXME
                mFastCaptureDumpState.increaseSamplingN(mAudioFlinger->isLowRamDevice() ?
                        FastThreadDumpState::kSamplingNforLowRamDevice :
                        FastThreadDumpState::kSamplingN);
#endif
                didModify = true;
            }
            audio_track_cblk_t *cblkOld = state->mCblk;
            audio_track_cblk_t *cblkNew = fastTrack != 0 ? fastTrack->cblk() : NULL;
            if (cblkNew != cblkOld) {
                state->mCblk = cblkNew;
                // block until acked if removing a fast track
                if (cblkOld != NULL) {
                    block = FastCaptureStateQueue::BLOCK_UNTIL_ACKED;
                }
                didModify = true;
            }
            AudioBufferProvider* abp = (fastTrack != 0 && fastTrack->isPatchTrack()) ?
                    reinterpret_cast<AudioBufferProvider*>(fastTrack.get()) : nullptr;
            if (state->mFastPatchRecordBufferProvider != abp) {
                state->mFastPatchRecordBufferProvider = abp;
                state->mFastPatchRecordFormat = fastTrack == 0 ?
                        AUDIO_FORMAT_INVALID : fastTrack->format();
                didModify = true;
            }
            sq->end(didModify);
            if (didModify) {
                sq->push(block);
#if 0
                if (kUseFastCapture == FastCapture_Dynamic) {
                    mNormalSource = mPipeSource;
                }
#endif
            }
        }

        // now run the fast track destructor with thread mutex unlocked
        fastTrackToRemove.clear();

        // Read from HAL to keep up with fastest client if multiple active tracks, not slowest one.
        // Only the client(s) that are too slow will overrun. But if even the fastest client is too
        // slow, then this RecordThread will overrun by not calling HAL read often enough.
        // If destination is non-contiguous, first read past the nominal end of buffer, then
        // copy to the right place.  Permitted because mRsmpInBuffer was over-allocated.

        int32_t rear = mRsmpInRear & (mRsmpInFramesP2 - 1);
        ssize_t framesRead;
        const int64_t lastIoBeginNs = systemTime(); // start IO timing

        // If an NBAIO source is present, use it to read the normal capture's data
        if (mPipeSource != 0) {
            size_t framesToRead = min(mRsmpInFramesOA - rear, mRsmpInFramesP2 / 2);

            // The audio fifo read() returns OVERRUN on overflow, and advances the read pointer
            // to the full buffer point (clearing the overflow condition).  Upon OVERRUN error,
            // we immediately retry the read() to get data and prevent another overflow.
            for (int retries = 0; retries <= 2; ++retries) {
                ALOGW_IF(retries > 0, "overrun on read from pipe, retry #%d", retries);
                framesRead = mPipeSource->read((uint8_t*)mRsmpInBuffer + rear * mFrameSize,
                        framesToRead);
                if (framesRead != OVERRUN) break;
            }

            const ssize_t availableToRead = mPipeSource->availableToRead();
            if (availableToRead >= 0) {
                // PipeSource is the master clock.  It is up to the AudioRecord client to keep up.
                LOG_ALWAYS_FATAL_IF((size_t)availableToRead > mPipeFramesP2,
                        "more frames to read than fifo size, %zd > %zu",
                        availableToRead, mPipeFramesP2);
                const size_t pipeFramesFree = mPipeFramesP2 - availableToRead;
                const size_t sleepFrames = min(pipeFramesFree, mRsmpInFramesP2) / 2;
                ALOGVV("mPipeFramesP2:%zu mRsmpInFramesP2:%zu sleepFrames:%zu availableToRead:%zd",
                        mPipeFramesP2, mRsmpInFramesP2, sleepFrames, availableToRead);
                sleepUs = (sleepFrames * 1000000LL) / mSampleRate;
            }
            if (framesRead < 0) {
                status_t status = (status_t) framesRead;
                switch (status) {
                case OVERRUN:
                    ALOGW("overrun on read from pipe");
                    framesRead = 0;
                    break;
                case NEGOTIATE:
                    ALOGE("re-negotiation is needed");
                    framesRead = -1;  // Will cause an attempt to recover.
                    break;
                default:
                    ALOGE("unknown error %d on read from pipe", status);
                    break;
                }
            }
        // otherwise use the HAL / AudioStreamIn directly
        } else {
            ATRACE_BEGIN("read");
            size_t bytesRead;
            status_t result = mInput->stream->read(
                    (uint8_t*)mRsmpInBuffer + rear * mFrameSize, mBufferSize, &bytesRead);
            ATRACE_END();
            if (result < 0) {
                framesRead = result;
            } else {
                framesRead = bytesRead / mFrameSize;
            }
        }

        const int64_t lastIoEndNs = systemTime(); // end IO timing

        // Update server timestamp with server stats
        // systemTime() is optional if the hardware supports timestamps.
        mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER] += framesRead;
        mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] = lastIoEndNs;

        // Update server timestamp with kernel stats
        if (mPipeSource.get() == nullptr /* don't obtain for FastCapture, could block */) {
            int64_t position, time;
            if (mStandby) {
                mTimestampVerifier.discontinuity();
            } else if (mInput->stream->getCapturePosition(&position, &time) == NO_ERROR
                    && time > mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL]) {

                mTimestampVerifier.add(position, time, mSampleRate);

                // Correct timestamps
                if (isTimestampCorrectionEnabled()) {
                    ALOGV("TS_BEFORE: %d %lld %lld",
                            id(), (long long)time, (long long)position);
                    auto correctedTimestamp = mTimestampVerifier.getLastCorrectedTimestamp();
                    position = correctedTimestamp.mFrames;
                    time = correctedTimestamp.mTimeNs;
                    ALOGV("TS_AFTER: %d %lld %lld",
                            id(), (long long)time, (long long)position);
                }

                mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] = position;
                mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] = time;
                // Note: In general record buffers should tend to be empty in
                // a properly running pipeline.
                //
                // Also, it is not advantageous to call get_presentation_position during the read
                // as the read obtains a lock, preventing the timestamp call from executing.
            } else {
                mTimestampVerifier.error();
            }
        }

        // From the timestamp, input read latency is negative output write latency.
        const audio_input_flags_t flags = mInput != NULL ? mInput->flags : AUDIO_INPUT_FLAG_NONE;
        const double latencyMs = RecordTrack::checkServerLatencySupported(mFormat, flags)
                ? - mTimestamp.getOutputServerLatencyMs(mSampleRate) : 0.;
        if (latencyMs != 0.) { // note 0. means timestamp is empty.
            mLatencyMs.add(latencyMs);
        }

        // Use this to track timestamp information
        // ALOGD("%s", mTimestamp.toString().c_str());

        if (framesRead < 0 || (framesRead == 0 && mPipeSource == 0)) {
            ALOGE("read failed: framesRead=%zd", framesRead);
            // Force input into standby so that it tries to recover at next read attempt
            inputStandBy();
            sleepUs = kRecordThreadSleepUs;
        }
        if (framesRead <= 0) {
            goto unlock;
        }
        ALOG_ASSERT(framesRead > 0);
        mFramesRead += framesRead;

#ifdef TEE_SINK
        (void)mTee.write((uint8_t*)mRsmpInBuffer + rear * mFrameSize, framesRead);
#endif
        // If destination is non-contiguous, we now correct for reading past end of buffer.
        {
            size_t part1 = mRsmpInFramesP2 - rear;
            if ((size_t) framesRead > part1) {
                memcpy(mRsmpInBuffer, (uint8_t*)mRsmpInBuffer + mRsmpInFramesP2 * mFrameSize,
                        (framesRead - part1) * mFrameSize);
            }
        }
        rear = mRsmpInRear += framesRead;

        size = activeTracks.size();

        // loop over each active track
        for (size_t i = 0; i < size; i++) {
            activeTrack = activeTracks[i];

            // skip fast tracks, as those are handled directly by FastCapture
            if (activeTrack->isFastTrack()) {
                continue;
            }

            // TODO: This code probably should be moved to RecordTrack.
            // TODO: Update the activeTrack buffer converter in case of reconfigure.

            enum {
                OVERRUN_UNKNOWN,
                OVERRUN_TRUE,
                OVERRUN_FALSE
            } overrun = OVERRUN_UNKNOWN;

            // loop over getNextBuffer to handle circular sink
            for (;;) {

                activeTrack->mSink.frameCount = ~0;
                status_t status = activeTrack->getNextBuffer(&activeTrack->mSink);
                size_t framesOut = activeTrack->mSink.frameCount;
                LOG_ALWAYS_FATAL_IF((status == OK) != (framesOut > 0));

                // check available frames and handle overrun conditions
                // if the record track isn't draining fast enough.
                bool hasOverrun;
                size_t framesIn;
                activeTrack->mResamplerBufferProvider->sync(&framesIn, &hasOverrun);
                if (hasOverrun) {
                    overrun = OVERRUN_TRUE;
                }
                if (framesOut == 0 || framesIn == 0) {
                    break;
                }

                // Don't allow framesOut to be larger than what is possible with resampling
                // from framesIn.
                // This isn't strictly necessary but helps limit buffer resizing in
                // RecordBufferConverter.  TODO: remove when no longer needed.
                framesOut = min(framesOut,
                        destinationFramesPossible(
                                framesIn, mSampleRate, activeTrack->mSampleRate));

                if (activeTrack->isDirect()) {
                    // No RecordBufferConverter used for direct streams. Pass
                    // straight from RecordThread buffer to RecordTrack buffer.
                    AudioBufferProvider::Buffer buffer;
                    buffer.frameCount = framesOut;
                    status_t status = activeTrack->mResamplerBufferProvider->getNextBuffer(&buffer);
                    if (status == OK && buffer.frameCount != 0) {
                        ALOGV_IF(buffer.frameCount != framesOut,
                                "%s() read less than expected (%zu vs %zu)",
                                __func__, buffer.frameCount, framesOut);
                        framesOut = buffer.frameCount;
                        memcpy(activeTrack->mSink.raw, buffer.raw, buffer.frameCount * mFrameSize);
                        activeTrack->mResamplerBufferProvider->releaseBuffer(&buffer);
                    } else {
                        framesOut = 0;
                        ALOGE("%s() cannot fill request, status: %d, frameCount: %zu",
                            __func__, status, buffer.frameCount);
                    }
                } else {
                    // process frames from the RecordThread buffer provider to the RecordTrack
                    // buffer
                    framesOut = activeTrack->mRecordBufferConverter->convert(
                            activeTrack->mSink.raw,
                            activeTrack->mResamplerBufferProvider,
                            framesOut);
                }

                if (framesOut > 0 && (overrun == OVERRUN_UNKNOWN)) {
                    overrun = OVERRUN_FALSE;
                }

                if (activeTrack->mFramesToDrop == 0) {
                    if (framesOut > 0) {
                        activeTrack->mSink.frameCount = framesOut;
                        // Sanitize before releasing if the track has no access to the source data
                        // An idle UID receives silence from non virtual devices until active
                        if (activeTrack->isSilenced()) {
                            memset(activeTrack->mSink.raw, 0, framesOut * activeTrack->frameSize());
                        }
                        activeTrack->releaseBuffer(&activeTrack->mSink);
                    }
                } else {
                    // FIXME could do a partial drop of framesOut
                    if (activeTrack->mFramesToDrop > 0) {
                        activeTrack->mFramesToDrop -= (ssize_t)framesOut;
                        if (activeTrack->mFramesToDrop <= 0) {
                            activeTrack->clearSyncStartEvent();
                        }
                    } else {
                        activeTrack->mFramesToDrop += framesOut;
                        if (activeTrack->mFramesToDrop >= 0 || activeTrack->mSyncStartEvent == 0 ||
                                activeTrack->mSyncStartEvent->isCancelled()) {
                            ALOGW("Synced record %s, session %d, trigger session %d",
                                  (activeTrack->mFramesToDrop >= 0) ? "timed out" : "cancelled",
                                  activeTrack->sessionId(),
                                  (activeTrack->mSyncStartEvent != 0) ?
                                          activeTrack->mSyncStartEvent->triggerSession() :
                                          AUDIO_SESSION_NONE);
                            activeTrack->clearSyncStartEvent();
                        }
                    }
                }

                if (framesOut == 0) {
                    break;
                }
            }

            switch (overrun) {
            case OVERRUN_TRUE:
                // client isn't retrieving buffers fast enough
                if (!activeTrack->setOverflow()) {
                    nsecs_t now = systemTime();
                    // FIXME should lastWarning per track?
                    if ((now - lastWarning) > kWarningThrottleNs) {
                        ALOGW("RecordThread: buffer overflow");
                        lastWarning = now;
                    }
                }
                break;
            case OVERRUN_FALSE:
                activeTrack->clearOverflow();
                break;
            case OVERRUN_UNKNOWN:
                break;
            }

            // update frame information and push timestamp out
            activeTrack->updateTrackFrameInfo(
                    activeTrack->mServerProxy->framesReleased(),
                    mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER],
                    mSampleRate, mTimestamp);
        }

unlock:
        // enable changes in effect chain
        unlockEffectChains(effectChains);
        // effectChains doesn't need to be cleared, since it is cleared by destructor at scope end
        if (audio_has_proportional_frames(mFormat)
            && loopCount == lastLoopCountRead + 1) {
            const int64_t readPeriodNs = lastIoEndNs - mLastIoEndNs;
            const double jitterMs =
                TimestampVerifier<int64_t, int64_t>::computeJitterMs(
                    {framesRead, readPeriodNs},
                    {0, 0} /* lastTimestamp */, mSampleRate);
            const double processMs = (lastIoBeginNs - mLastIoEndNs) * 1e-6;

            Mutex::Autolock _l(mLock);
            mIoJitterMs.add(jitterMs);
            mProcessTimeMs.add(processMs);
        }
        // update timing info.
        mLastIoBeginNs = lastIoBeginNs;
        mLastIoEndNs = lastIoEndNs;
        lastLoopCountRead = loopCount;
    }

    standbyIfNotAlreadyInStandby();

    {
        Mutex::Autolock _l(mLock);
        for (size_t i = 0; i < mTracks.size(); i++) {
            sp<RecordTrack> track = mTracks[i];
            track->invalidate();
        }
        mActiveTracks.clear();
        mStartStopCond.broadcast();
    }

    releaseWakeLock();

    ALOGV("RecordThread %p exiting", this);
    return false;
}

void AudioFlinger::RecordThread::standbyIfNotAlreadyInStandby()
{
    if (!mStandby) {
        inputStandBy();
        mStandby = true;
    }
}

void AudioFlinger::RecordThread::inputStandBy()
{
    // Idle the fast capture if it's currently running
    if (mFastCapture != 0) {
        FastCaptureStateQueue *sq = mFastCapture->sq();
        FastCaptureState *state = sq->begin();
        if (!(state->mCommand & FastCaptureState::IDLE)) {
            state->mCommand = FastCaptureState::COLD_IDLE;
            state->mColdFutexAddr = &mFastCaptureFutex;
            state->mColdGen++;
            mFastCaptureFutex = 0;
            sq->end();
            // BLOCK_UNTIL_PUSHED would be insufficient, as we need it to stop doing I/O now
            sq->push(FastCaptureStateQueue::BLOCK_UNTIL_ACKED);
#if 0
            if (kUseFastCapture == FastCapture_Dynamic) {
                // FIXME
            }
#endif
#ifdef AUDIO_WATCHDOG
            // FIXME
#endif
        } else {
            sq->end(false /*didModify*/);
        }
    }
    status_t result = mInput->stream->standby();
    ALOGE_IF(result != OK, "Error when putting input stream into standby: %d", result);

    // If going into standby, flush the pipe source.
    if (mPipeSource.get() != nullptr) {
        const ssize_t flushed = mPipeSource->flush();
        if (flushed > 0) {
            ALOGV("Input standby flushed PipeSource %zd frames", flushed);
            mTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER] += flushed;
            mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] = systemTime();
        }
    }
}

// RecordThread::createRecordTrack_l() must be called with AudioFlinger::mLock held
sp<AudioFlinger::RecordThread::RecordTrack> AudioFlinger::RecordThread::createRecordTrack_l(
        const sp<AudioFlinger::Client>& client,
        const audio_attributes_t& attr,
        uint32_t *pSampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t *pFrameCount,
        audio_session_t sessionId,
        size_t *pNotificationFrameCount,
        pid_t creatorPid,
        uid_t uid,
        audio_input_flags_t *flags,
        pid_t tid,
        status_t *status,
        audio_port_handle_t portId,
        const String16& opPackageName)
{
    size_t frameCount = *pFrameCount;
    size_t notificationFrameCount = *pNotificationFrameCount;
    sp<RecordTrack> track;
    status_t lStatus;
    audio_input_flags_t inputFlags = mInput->flags;
    audio_input_flags_t requestedFlags = *flags;
    uint32_t sampleRate;

    lStatus = initCheck();
    if (lStatus != NO_ERROR) {
        ALOGE("createRecordTrack_l() audio driver not initialized");
        goto Exit;
    }

    if (!audio_is_linear_pcm(mFormat) && (*flags & AUDIO_INPUT_FLAG_DIRECT) == 0) {
        ALOGE("createRecordTrack_l() on an encoded stream requires AUDIO_INPUT_FLAG_DIRECT");
        lStatus = BAD_VALUE;
        goto Exit;
    }

    if (*pSampleRate == 0) {
        *pSampleRate = mSampleRate;
    }
    sampleRate = *pSampleRate;

    // special case for FAST flag considered OK if fast capture is present
    if (hasFastCapture()) {
        inputFlags = (audio_input_flags_t)(inputFlags | AUDIO_INPUT_FLAG_FAST);
    }

    // Check if requested flags are compatible with input stream flags
    if ((*flags & inputFlags) != *flags) {
        ALOGW("createRecordTrack_l(): mismatch between requested flags (%08x) and"
                " input flags (%08x)",
              *flags, inputFlags);
        *flags = (audio_input_flags_t)(*flags & inputFlags);
    }

    // client expresses a preference for FAST, but we get the final say
    if (*flags & AUDIO_INPUT_FLAG_FAST) {
      if (
            // we formerly checked for a callback handler (non-0 tid),
            // but that is no longer required for TRANSFER_OBTAIN mode
            //
            // Frame count is not specified (0), or is less than or equal the pipe depth.
            // It is OK to provide a higher capacity than requested.
            // We will force it to mPipeFramesP2 below.
            (frameCount <= mPipeFramesP2) &&
            // PCM data
            audio_is_linear_pcm(format) &&
            // hardware format
            (format == mFormat) &&
            // hardware channel mask
            (channelMask == mChannelMask) &&
            // hardware sample rate
            (sampleRate == mSampleRate) &&
            // record thread has an associated fast capture
            hasFastCapture() &&
            // there are sufficient fast track slots available
            mFastTrackAvail
        ) {
          // check compatibility with audio effects.
          Mutex::Autolock _l(mLock);
          // Do not accept FAST flag if the session has software effects
          sp<EffectChain> chain = getEffectChain_l(sessionId);
          if (chain != 0) {
              audio_input_flags_t old = *flags;
              chain->checkInputFlagCompatibility(flags);
              if (old != *flags) {
                  ALOGV("%p AUDIO_INPUT_FLAGS denied by effect old=%#x new=%#x",
                          this, (int)old, (int)*flags);
              }
          }
          ALOGV_IF((*flags & AUDIO_INPUT_FLAG_FAST) != 0,
                   "%p AUDIO_INPUT_FLAG_FAST accepted: frameCount=%zu mFrameCount=%zu",
                   this, frameCount, mFrameCount);
      } else {
        ALOGV("%p AUDIO_INPUT_FLAG_FAST denied: frameCount=%zu mFrameCount=%zu mPipeFramesP2=%zu "
                "format=%#x isLinear=%d mFormat=%#x channelMask=%#x sampleRate=%u mSampleRate=%u "
                "hasFastCapture=%d tid=%d mFastTrackAvail=%d",
                this, frameCount, mFrameCount, mPipeFramesP2,
                format, audio_is_linear_pcm(format), mFormat, channelMask, sampleRate, mSampleRate,
                hasFastCapture(), tid, mFastTrackAvail);
        *flags = (audio_input_flags_t)(*flags & ~AUDIO_INPUT_FLAG_FAST);
      }
    }

    // If FAST or RAW flags were corrected, ask caller to request new input from audio policy
    if ((*flags & AUDIO_INPUT_FLAG_FAST) !=
            (requestedFlags & AUDIO_INPUT_FLAG_FAST)) {
        *flags = (audio_input_flags_t) (*flags & ~(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_RAW));
        lStatus = BAD_TYPE;
        goto Exit;
    }

    // compute track buffer size in frames, and suggest the notification frame count
    if (*flags & AUDIO_INPUT_FLAG_FAST) {
        // fast track: frame count is exactly the pipe depth
        frameCount = mPipeFramesP2;
        // ignore requested notificationFrames, and always notify exactly once every HAL buffer
        notificationFrameCount = mFrameCount;
    } else {
        // not fast track: max notification period is resampled equivalent of one HAL buffer time
        //                 or 20 ms if there is a fast capture
        // TODO This could be a roundupRatio inline, and const
        size_t maxNotificationFrames = ((int64_t) (hasFastCapture() ? mSampleRate/50 : mFrameCount)
                * sampleRate + mSampleRate - 1) / mSampleRate;
        // minimum number of notification periods is at least kMinNotifications,
        // and at least kMinMs rounded up to a whole notification period (minNotificationsByMs)
        static const size_t kMinNotifications = 3;
        static const uint32_t kMinMs = 30;
        // TODO This could be a roundupRatio inline
        const size_t minFramesByMs = (sampleRate * kMinMs + 1000 - 1) / 1000;
        // TODO This could be a roundupRatio inline
        const size_t minNotificationsByMs = (minFramesByMs + maxNotificationFrames - 1) /
                maxNotificationFrames;
        const size_t minFrameCount = maxNotificationFrames *
                max(kMinNotifications, minNotificationsByMs);
        frameCount = max(frameCount, minFrameCount);
        if (notificationFrameCount == 0 || notificationFrameCount > maxNotificationFrames) {
            notificationFrameCount = maxNotificationFrames;
        }
    }
    *pFrameCount = frameCount;
    *pNotificationFrameCount = notificationFrameCount;

    { // scope for mLock
        Mutex::Autolock _l(mLock);

        track = new RecordTrack(this, client, attr, sampleRate,
                      format, channelMask, frameCount,
                      nullptr /* buffer */, (size_t)0 /* bufferSize */, sessionId, creatorPid, uid,
                      *flags, TrackBase::TYPE_DEFAULT, opPackageName, portId);

        lStatus = track->initCheck();
        if (lStatus != NO_ERROR) {
            ALOGE("createRecordTrack_l() initCheck failed %d; no control block?", lStatus);
            // track must be cleared from the caller as the caller has the AF lock
            goto Exit;
        }
        mTracks.add(track);

        if ((*flags & AUDIO_INPUT_FLAG_FAST) && (tid != -1)) {
            pid_t callingPid = IPCThreadState::self()->getCallingPid();
            // we don't have CAP_SYS_NICE, nor do we want to have it as it's too powerful,
            // so ask activity manager to do this on our behalf
            sendPrioConfigEvent_l(callingPid, tid, kPriorityAudioApp, true /*forApp*/);
        }
    }

    lStatus = NO_ERROR;

Exit:
    *status = lStatus;
    return track;
}

status_t AudioFlinger::RecordThread::start(RecordThread::RecordTrack* recordTrack,
                                           AudioSystem::sync_event_t event,
                                           audio_session_t triggerSession)
{
    ALOGV("RecordThread::start event %d, triggerSession %d", event, triggerSession);
    sp<ThreadBase> strongMe = this;
    status_t status = NO_ERROR;

    if (event == AudioSystem::SYNC_EVENT_NONE) {
        recordTrack->clearSyncStartEvent();
    } else if (event != AudioSystem::SYNC_EVENT_SAME) {
        recordTrack->mSyncStartEvent = mAudioFlinger->createSyncEvent(event,
                                       triggerSession,
                                       recordTrack->sessionId(),
                                       syncStartEventCallback,
                                       recordTrack);
        // Sync event can be cancelled by the trigger session if the track is not in a
        // compatible state in which case we start record immediately
        if (recordTrack->mSyncStartEvent->isCancelled()) {
            recordTrack->clearSyncStartEvent();
        } else {
            // do not wait for the event for more than AudioSystem::kSyncRecordStartTimeOutMs
            recordTrack->mFramesToDrop = -(ssize_t)
                    ((AudioSystem::kSyncRecordStartTimeOutMs * recordTrack->mSampleRate) / 1000);
        }
    }

    {
        // This section is a rendezvous between binder thread executing start() and RecordThread
        AutoMutex lock(mLock);
        if (recordTrack->isInvalid()) {
            recordTrack->clearSyncStartEvent();
            return INVALID_OPERATION;
        }
        if (mActiveTracks.indexOf(recordTrack) >= 0) {
            if (recordTrack->mState == TrackBase::PAUSING) {
                // We haven't stopped yet (moved to PAUSED and not in mActiveTracks)
                // so no need to startInput().
                ALOGV("active record track PAUSING -> ACTIVE");
                recordTrack->mState = TrackBase::ACTIVE;
            } else {
                ALOGV("active record track state %d", recordTrack->mState);
            }
            return status;
        }

        // TODO consider other ways of handling this, such as changing the state to :STARTING and
        //      adding the track to mActiveTracks after returning from AudioSystem::startInput(),
        //      or using a separate command thread
        recordTrack->mState = TrackBase::STARTING_1;
        mActiveTracks.add(recordTrack);
        status_t status = NO_ERROR;
        if (recordTrack->isExternalTrack()) {
            mLock.unlock();
            status = AudioSystem::startInput(recordTrack->portId());
            mLock.lock();
            if (recordTrack->isInvalid()) {
                recordTrack->clearSyncStartEvent();
                if (status == NO_ERROR && recordTrack->mState == TrackBase::STARTING_1) {
                    recordTrack->mState = TrackBase::STARTING_2;
                    // STARTING_2 forces destroy to call stopInput.
                }
                return INVALID_OPERATION;
            }
            if (recordTrack->mState != TrackBase::STARTING_1) {
                ALOGW("%s(%d): unsynchronized mState:%d change",
                    __func__, recordTrack->id(), recordTrack->mState);
                // Someone else has changed state, let them take over,
                // leave mState in the new state.
                recordTrack->clearSyncStartEvent();
                return INVALID_OPERATION;
            }
            // we're ok, but perhaps startInput has failed
            if (status != NO_ERROR) {
                ALOGW("%s(%d): startInput failed, status %d",
                    __func__, recordTrack->id(), status);
                // We are in ActiveTracks if STARTING_1 and valid, so remove from ActiveTracks,
                // leave in STARTING_1, so destroy() will not call stopInput.
                mActiveTracks.remove(recordTrack);
                recordTrack->clearSyncStartEvent();
                return status;
            }
            sendIoConfigEvent_l(
                AUDIO_CLIENT_STARTED, recordTrack->creatorPid(), recordTrack->portId());
        }
        // Catch up with current buffer indices if thread is already running.
        // This is what makes a new client discard all buffered data.  If the track's mRsmpInFront
        // was initialized to some value closer to the thread's mRsmpInFront, then the track could
        // see previously buffered data before it called start(), but with greater risk of overrun.

        recordTrack->mResamplerBufferProvider->reset();
        if (!recordTrack->isDirect()) {
            // clear any converter state as new data will be discontinuous
            recordTrack->mRecordBufferConverter->reset();
        }
        recordTrack->mState = TrackBase::STARTING_2;
        // signal thread to start
        mWaitWorkCV.broadcast();
        return status;
    }
}

void AudioFlinger::RecordThread::syncStartEventCallback(const wp<SyncEvent>& event)
{
    sp<SyncEvent> strongEvent = event.promote();

    if (strongEvent != 0) {
        sp<RefBase> ptr = strongEvent->cookie().promote();
        if (ptr != 0) {
            RecordTrack *recordTrack = (RecordTrack *)ptr.get();
            recordTrack->handleSyncStartEvent(strongEvent);
        }
    }
}

bool AudioFlinger::RecordThread::stop(RecordThread::RecordTrack* recordTrack) {
    ALOGV("RecordThread::stop");
    AutoMutex _l(mLock);
    // if we're invalid, we can't be on the ActiveTracks.
    if (mActiveTracks.indexOf(recordTrack) < 0 || recordTrack->mState == TrackBase::PAUSING) {
        return false;
    }
    // note that threadLoop may still be processing the track at this point [without lock]
    recordTrack->mState = TrackBase::PAUSING;

    // NOTE: Waiting here is important to keep stop synchronous.
    // This is needed for proper patchRecord peer release.
    while (recordTrack->mState == TrackBase::PAUSING && !recordTrack->isInvalid()) {
        mWaitWorkCV.broadcast(); // signal thread to stop
        mStartStopCond.wait(mLock);
    }

    if (recordTrack->mState == TrackBase::PAUSED) { // successful stop
        ALOGV("Record stopped OK");
        return true;
    }

    // don't handle anything - we've been invalidated or restarted and in a different state
    ALOGW_IF("%s(%d): unsynchronized stop, state: %d",
            __func__, recordTrack->id(), recordTrack->mState);
    return false;
}

bool AudioFlinger::RecordThread::isValidSyncEvent(const sp<SyncEvent>& event __unused) const
{
    return false;
}

status_t AudioFlinger::RecordThread::setSyncEvent(const sp<SyncEvent>& event __unused)
{
#if 0   // This branch is currently dead code, but is preserved in case it will be needed in future
    if (!isValidSyncEvent(event)) {
        return BAD_VALUE;
    }

    audio_session_t eventSession = event->triggerSession();
    status_t ret = NAME_NOT_FOUND;

    Mutex::Autolock _l(mLock);

    for (size_t i = 0; i < mTracks.size(); i++) {
        sp<RecordTrack> track = mTracks[i];
        if (eventSession == track->sessionId()) {
            (void) track->setSyncEvent(event);
            ret = NO_ERROR;
        }
    }
    return ret;
#else
    return BAD_VALUE;
#endif
}

status_t AudioFlinger::RecordThread::getActiveMicrophones(
        std::vector<media::MicrophoneInfo>* activeMicrophones)
{
    ALOGV("RecordThread::getActiveMicrophones");
    AutoMutex _l(mLock);
    status_t status = mInput->stream->getActiveMicrophones(activeMicrophones);
    return status;
}

status_t AudioFlinger::RecordThread::setPreferredMicrophoneDirection(
            audio_microphone_direction_t direction)
{
    ALOGV("setPreferredMicrophoneDirection(%d)", direction);
    AutoMutex _l(mLock);
    return mInput->stream->setPreferredMicrophoneDirection(direction);
}

status_t AudioFlinger::RecordThread::setPreferredMicrophoneFieldDimension(float zoom)
{
    ALOGV("setPreferredMicrophoneFieldDimension(%f)", zoom);
    AutoMutex _l(mLock);
    return mInput->stream->setPreferredMicrophoneFieldDimension(zoom);
}

void AudioFlinger::RecordThread::updateMetadata_l()
{
    if (mInput == nullptr || mInput->stream == nullptr ||
            !mActiveTracks.readAndClearHasChanged()) {
        return;
    }
    StreamInHalInterface::SinkMetadata metadata;
    for (const sp<RecordTrack> &track : mActiveTracks) {
        // No track is invalid as this is called after prepareTrack_l in the same critical section
        metadata.tracks.push_back({
                .source = track->attributes().source,
                .gain = 1, // capture tracks do not have volumes
        });
    }
    mInput->stream->updateSinkMetadata(metadata);
}

// destroyTrack_l() must be called with ThreadBase::mLock held
void AudioFlinger::RecordThread::destroyTrack_l(const sp<RecordTrack>& track)
{
    track->terminate();
    track->mState = TrackBase::STOPPED;
    // active tracks are removed by threadLoop()
    if (mActiveTracks.indexOf(track) < 0) {
        removeTrack_l(track);
    }
}

void AudioFlinger::RecordThread::removeTrack_l(const sp<RecordTrack>& track)
{
    String8 result;
    track->appendDump(result, false /* active */);
    mLocalLog.log("removeTrack_l (%p) %s", track.get(), result.string());

    mTracks.remove(track);
    // need anything related to effects here?
    if (track->isFastTrack()) {
        ALOG_ASSERT(!mFastTrackAvail);
        mFastTrackAvail = true;
    }
}

void AudioFlinger::RecordThread::dumpInternals_l(int fd, const Vector<String16>& args __unused)
{
    AudioStreamIn *input = mInput;
    audio_input_flags_t flags = input != NULL ? input->flags : AUDIO_INPUT_FLAG_NONE;
    dprintf(fd, "  AudioStreamIn: %p flags %#x (%s)\n",
            input, flags, toString(flags).c_str());
    dprintf(fd, "  Frames read: %lld\n", (long long)mFramesRead);
    if (mActiveTracks.isEmpty()) {
        dprintf(fd, "  No active record clients\n");
    }

    if (input != nullptr) {
        dprintf(fd, "  Hal stream dump:\n");
        (void)input->stream->dump(fd);
    }

    dprintf(fd, "  Fast capture thread: %s\n", hasFastCapture() ? "yes" : "no");
    dprintf(fd, "  Fast track available: %s\n", mFastTrackAvail ? "yes" : "no");

    // Make a non-atomic copy of fast capture dump state so it won't change underneath us
    // while we are dumping it.  It may be inconsistent, but it won't mutate!
    // This is a large object so we place it on the heap.
    // FIXME 25972958: Need an intelligent copy constructor that does not touch unused pages.
    const std::unique_ptr<FastCaptureDumpState> copy =
            std::make_unique<FastCaptureDumpState>(mFastCaptureDumpState);
    copy->dump(fd);
}

void AudioFlinger::RecordThread::dumpTracks_l(int fd, const Vector<String16>& args __unused)
{
    String8 result;
    size_t numtracks = mTracks.size();
    size_t numactive = mActiveTracks.size();
    size_t numactiveseen = 0;
    dprintf(fd, "  %zu Tracks", numtracks);
    const char *prefix = "    ";
    if (numtracks) {
        dprintf(fd, " of which %zu are active\n", numactive);
        result.append(prefix);
        mTracks[0]->appendDumpHeader(result);
        for (size_t i = 0; i < numtracks ; ++i) {
            sp<RecordTrack> track = mTracks[i];
            if (track != 0) {
                bool active = mActiveTracks.indexOf(track) >= 0;
                if (active) {
                    numactiveseen++;
                }
                result.append(prefix);
                track->appendDump(result, active);
            }
        }
    } else {
        dprintf(fd, "\n");
    }

    if (numactiveseen != numactive) {
        result.append("  The following tracks are in the active list but"
                " not in the track list\n");
        result.append(prefix);
        mActiveTracks[0]->appendDumpHeader(result);
        for (size_t i = 0; i < numactive; ++i) {
            sp<RecordTrack> track = mActiveTracks[i];
            if (mTracks.indexOf(track) < 0) {
                result.append(prefix);
                track->appendDump(result, true /* active */);
            }
        }

    }
    write(fd, result.string(), result.size());
}

void AudioFlinger::RecordThread::setRecordSilenced(uid_t uid, bool silenced)
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mTracks.size() ; i++) {
        sp<RecordTrack> track = mTracks[i];
        if (track != 0 && track->uid() == uid) {
            track->setSilenced(silenced);
        }
    }
}

void AudioFlinger::RecordThread::ResamplerBufferProvider::reset()
{
    sp<ThreadBase> threadBase = mRecordTrack->mThread.promote();
    RecordThread *recordThread = (RecordThread *) threadBase.get();
    mRsmpInFront = recordThread->mRsmpInRear;
    mRsmpInUnrel = 0;
}

void AudioFlinger::RecordThread::ResamplerBufferProvider::sync(
        size_t *framesAvailable, bool *hasOverrun)
{
    sp<ThreadBase> threadBase = mRecordTrack->mThread.promote();
    RecordThread *recordThread = (RecordThread *) threadBase.get();
    const int32_t rear = recordThread->mRsmpInRear;
    const int32_t front = mRsmpInFront;
    const ssize_t filled = audio_utils::safe_sub_overflow(rear, front);

    size_t framesIn;
    bool overrun = false;
    if (filled < 0) {
        // should not happen, but treat like a massive overrun and re-sync
        framesIn = 0;
        mRsmpInFront = rear;
        overrun = true;
    } else if ((size_t) filled <= recordThread->mRsmpInFrames) {
        framesIn = (size_t) filled;
    } else {
        // client is not keeping up with server, but give it latest data
        framesIn = recordThread->mRsmpInFrames;
        mRsmpInFront = /* front = */ audio_utils::safe_sub_overflow(
                rear, static_cast<int32_t>(framesIn));
        overrun = true;
    }
    if (framesAvailable != NULL) {
        *framesAvailable = framesIn;
    }
    if (hasOverrun != NULL) {
        *hasOverrun = overrun;
    }
}

// AudioBufferProvider interface
status_t AudioFlinger::RecordThread::ResamplerBufferProvider::getNextBuffer(
        AudioBufferProvider::Buffer* buffer)
{
    sp<ThreadBase> threadBase = mRecordTrack->mThread.promote();
    if (threadBase == 0) {
        buffer->frameCount = 0;
        buffer->raw = NULL;
        return NOT_ENOUGH_DATA;
    }
    RecordThread *recordThread = (RecordThread *) threadBase.get();
    int32_t rear = recordThread->mRsmpInRear;
    int32_t front = mRsmpInFront;
    ssize_t filled = audio_utils::safe_sub_overflow(rear, front);
    // FIXME should not be P2 (don't want to increase latency)
    // FIXME if client not keeping up, discard
    LOG_ALWAYS_FATAL_IF(!(0 <= filled && (size_t) filled <= recordThread->mRsmpInFrames));
    // 'filled' may be non-contiguous, so return only the first contiguous chunk
    front &= recordThread->mRsmpInFramesP2 - 1;
    size_t part1 = recordThread->mRsmpInFramesP2 - front;
    if (part1 > (size_t) filled) {
        part1 = filled;
    }
    size_t ask = buffer->frameCount;
    ALOG_ASSERT(ask > 0);
    if (part1 > ask) {
        part1 = ask;
    }
    if (part1 == 0) {
        // out of data is fine since the resampler will return a short-count.
        buffer->raw = NULL;
        buffer->frameCount = 0;
        mRsmpInUnrel = 0;
        return NOT_ENOUGH_DATA;
    }

    buffer->raw = (uint8_t*)recordThread->mRsmpInBuffer + front * recordThread->mFrameSize;
    buffer->frameCount = part1;
    mRsmpInUnrel = part1;
    return NO_ERROR;
}

// AudioBufferProvider interface
void AudioFlinger::RecordThread::ResamplerBufferProvider::releaseBuffer(
        AudioBufferProvider::Buffer* buffer)
{
    int32_t stepCount = static_cast<int32_t>(buffer->frameCount);
    if (stepCount == 0) {
        return;
    }
    ALOG_ASSERT(stepCount <= mRsmpInUnrel);
    mRsmpInUnrel -= stepCount;
    mRsmpInFront = audio_utils::safe_add_overflow(mRsmpInFront, stepCount);
    buffer->raw = NULL;
    buffer->frameCount = 0;
}

void AudioFlinger::RecordThread::checkBtNrec()
{
    Mutex::Autolock _l(mLock);
    checkBtNrec_l();
}

void AudioFlinger::RecordThread::checkBtNrec_l()
{
    // disable AEC and NS if the device is a BT SCO headset supporting those
    // pre processings
    bool suspend = audio_is_bluetooth_sco_device(mInDevice) &&
                        mAudioFlinger->btNrecIsOff();
    if (mBtNrecSuspended.exchange(suspend) != suspend) {
        for (size_t i = 0; i < mEffectChains.size(); i++) {
            setEffectSuspended_l(FX_IID_AEC, suspend, mEffectChains[i]->sessionId());
            setEffectSuspended_l(FX_IID_NS, suspend, mEffectChains[i]->sessionId());
        }
    }
}


bool AudioFlinger::RecordThread::checkForNewParameter_l(const String8& keyValuePair,
                                                        status_t& status)
{
    bool reconfig = false;

    status = NO_ERROR;

    audio_format_t reqFormat = mFormat;
    uint32_t samplingRate = mSampleRate;
    // TODO this may change if we want to support capture from HDMI PCM multi channel (e.g on TVs).
    audio_channel_mask_t channelMask = audio_channel_in_mask_from_count(mChannelCount);

    AudioParameter param = AudioParameter(keyValuePair);
    int value;

    // scope for AutoPark extends to end of method
    AutoPark<FastCapture> park(mFastCapture);

    // TODO Investigate when this code runs. Check with audio policy when a sample rate and
    //      channel count change can be requested. Do we mandate the first client defines the
    //      HAL sampling rate and channel count or do we allow changes on the fly?
    if (param.getInt(String8(AudioParameter::keySamplingRate), value) == NO_ERROR) {
        samplingRate = value;
        reconfig = true;
    }
    if (param.getInt(String8(AudioParameter::keyFormat), value) == NO_ERROR) {
        if (!audio_is_linear_pcm((audio_format_t) value)) {
            status = BAD_VALUE;
        } else {
            reqFormat = (audio_format_t) value;
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyChannels), value) == NO_ERROR) {
        audio_channel_mask_t mask = (audio_channel_mask_t) value;
        if (!audio_is_input_channel(mask) ||
                audio_channel_count_from_in_mask(mask) > FCC_8) {
            status = BAD_VALUE;
        } else {
            channelMask = mask;
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyFrameCount), value) == NO_ERROR) {
        // do not accept frame count changes if tracks are open as the track buffer
        // size depends on frame count and correct behavior would not be guaranteed
        // if frame count is changed after track creation
        if (mActiveTracks.size() > 0) {
            status = INVALID_OPERATION;
        } else {
            reconfig = true;
        }
    }
    if (param.getInt(String8(AudioParameter::keyRouting), value) == NO_ERROR) {
        // forward device change to effects that have requested to be
        // aware of attached audio device.
        for (size_t i = 0; i < mEffectChains.size(); i++) {
            mEffectChains[i]->setDevice_l(value);
        }

        // store input device and output device but do not forward output device to audio HAL.
        // Note that status is ignored by the caller for output device
        // (see AudioFlinger::setParameters()
        if (audio_is_output_devices(value)) {
            mOutDevice = value;
            status = BAD_VALUE;
        } else {
            mInDevice = value;
            if (value != AUDIO_DEVICE_NONE) {
                mPrevInDevice = value;
            }
            checkBtNrec_l();
        }
    }
    if (param.getInt(String8(AudioParameter::keyInputSource), value) == NO_ERROR &&
            mAudioSource != (audio_source_t)value) {
        // forward device change to effects that have requested to be
        // aware of attached audio device.
        for (size_t i = 0; i < mEffectChains.size(); i++) {
            mEffectChains[i]->setAudioSource_l((audio_source_t)value);
        }
        mAudioSource = (audio_source_t)value;
    }

    if (status == NO_ERROR) {
        status = mInput->stream->setParameters(keyValuePair);
        if (status == INVALID_OPERATION) {
            inputStandBy();
            status = mInput->stream->setParameters(keyValuePair);
        }
        if (reconfig) {
            if (status == BAD_VALUE) {
                uint32_t sRate;
                audio_channel_mask_t channelMask;
                audio_format_t format;
                if (mInput->stream->getAudioProperties(&sRate, &channelMask, &format) == OK &&
                        audio_is_linear_pcm(format) && audio_is_linear_pcm(reqFormat) &&
                        sRate <= (AUDIO_RESAMPLER_DOWN_RATIO_MAX * samplingRate) &&
                        audio_channel_count_from_in_mask(channelMask) <= FCC_8) {
                    status = NO_ERROR;
                }
            }
            if (status == NO_ERROR) {
                readInputParameters_l();
                sendIoConfigEvent_l(AUDIO_INPUT_CONFIG_CHANGED);
            }
        }
    }

    return reconfig;
}

String8 AudioFlinger::RecordThread::getParameters(const String8& keys)
{
    Mutex::Autolock _l(mLock);
    if (initCheck() == NO_ERROR) {
        String8 out_s8;
        if (mInput->stream->getParameters(keys, &out_s8) == OK) {
            return out_s8;
        }
    }
    return String8();
}

void AudioFlinger::RecordThread::ioConfigChanged(audio_io_config_event event, pid_t pid,
                                                 audio_port_handle_t portId) {
    sp<AudioIoDescriptor> desc = new AudioIoDescriptor();

    desc->mIoHandle = mId;

    switch (event) {
    case AUDIO_INPUT_OPENED:
    case AUDIO_INPUT_REGISTERED:
    case AUDIO_INPUT_CONFIG_CHANGED:
        desc->mPatch = mPatch;
        desc->mChannelMask = mChannelMask;
        desc->mSamplingRate = mSampleRate;
        desc->mFormat = mFormat;
        desc->mFrameCount = mFrameCount;
        desc->mFrameCountHAL = mFrameCount;
        desc->mLatency = 0;
        break;
    case AUDIO_CLIENT_STARTED:
        desc->mPatch = mPatch;
        desc->mPortId = portId;
        break;
    case AUDIO_INPUT_CLOSED:
    default:
        break;
    }
    mAudioFlinger->ioConfigChanged(event, desc, pid);
}

void AudioFlinger::RecordThread::readInputParameters_l()
{
    status_t result = mInput->stream->getAudioProperties(&mSampleRate, &mChannelMask, &mHALFormat);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving audio properties from HAL: %d", result);
    mFormat = mHALFormat;
    mChannelCount = audio_channel_count_from_in_mask(mChannelMask);
    if (audio_is_linear_pcm(mFormat)) {
        LOG_ALWAYS_FATAL_IF(mChannelCount > FCC_8, "HAL channel count %d > %d",
                mChannelCount, FCC_8);
    } else {
        // Can have more that FCC_8 channels in encoded streams.
        ALOGI("HAL format %#x is not linear pcm", mFormat);
    }
    result = mInput->stream->getFrameSize(&mFrameSize);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving frame size from HAL: %d", result);
    result = mInput->stream->getBufferSize(&mBufferSize);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving buffer size from HAL: %d", result);
    mFrameCount = mBufferSize / mFrameSize;
    ALOGV("%p RecordThread params: mChannelCount=%u, mFormat=%#x, mFrameSize=%lld, "
            "mBufferSize=%lld, mFrameCount=%lld",
            this, mChannelCount, mFormat, (long long)mFrameSize, (long long)mBufferSize,
            (long long)mFrameCount);
    // This is the formula for calculating the temporary buffer size.
    // With 7 HAL buffers, we can guarantee ability to down-sample the input by ratio of 6:1 to
    // 1 full output buffer, regardless of the alignment of the available input.
    // The value is somewhat arbitrary, and could probably be even larger.
    // A larger value should allow more old data to be read after a track calls start(),
    // without increasing latency.
    //
    // Note this is independent of the maximum downsampling ratio permitted for capture.
    mRsmpInFrames = mFrameCount * 7;
    mRsmpInFramesP2 = roundup(mRsmpInFrames);
    free(mRsmpInBuffer);
    mRsmpInBuffer = NULL;

    // TODO optimize audio capture buffer sizes ...
    // Here we calculate the size of the sliding buffer used as a source
    // for resampling.  mRsmpInFramesP2 is currently roundup(mFrameCount * 7).
    // For current HAL frame counts, this is usually 2048 = 40 ms.  It would
    // be better to have it derived from the pipe depth in the long term.
    // The current value is higher than necessary.  However it should not add to latency.

    // Over-allocate beyond mRsmpInFramesP2 to permit a HAL read past end of buffer
    mRsmpInFramesOA = mRsmpInFramesP2 + mFrameCount - 1;
    (void)posix_memalign(&mRsmpInBuffer, 32, mRsmpInFramesOA * mFrameSize);
    // if posix_memalign fails, will segv here.
    memset(mRsmpInBuffer, 0, mRsmpInFramesOA * mFrameSize);

    // AudioRecord mSampleRate and mChannelCount are constant due to AudioRecord API constraints.
    // But if thread's mSampleRate or mChannelCount changes, how will that affect active tracks?
}

uint32_t AudioFlinger::RecordThread::getInputFramesLost()
{
    Mutex::Autolock _l(mLock);
    uint32_t result;
    if (initCheck() == NO_ERROR && mInput->stream->getInputFramesLost(&result) == OK) {
        return result;
    }
    return 0;
}

KeyedVector<audio_session_t, bool> AudioFlinger::RecordThread::sessionIds() const
{
    KeyedVector<audio_session_t, bool> ids;
    Mutex::Autolock _l(mLock);
    for (size_t j = 0; j < mTracks.size(); ++j) {
        sp<RecordThread::RecordTrack> track = mTracks[j];
        audio_session_t sessionId = track->sessionId();
        if (ids.indexOfKey(sessionId) < 0) {
            ids.add(sessionId, true);
        }
    }
    return ids;
}

AudioFlinger::AudioStreamIn* AudioFlinger::RecordThread::clearInput()
{
    Mutex::Autolock _l(mLock);
    AudioStreamIn *input = mInput;
    mInput = NULL;
    return input;
}

// this method must always be called either with ThreadBase mLock held or inside the thread loop
sp<StreamHalInterface> AudioFlinger::RecordThread::stream() const
{
    if (mInput == NULL) {
        return NULL;
    }
    return mInput->stream;
}

status_t AudioFlinger::RecordThread::addEffectChain_l(const sp<EffectChain>& chain)
{
    ALOGV("addEffectChain_l() %p on thread %p", chain.get(), this);
    chain->setThread(this);
    chain->setInBuffer(NULL);
    chain->setOutBuffer(NULL);

    checkSuspendOnAddEffectChain_l(chain);

    // make sure enabled pre processing effects state is communicated to the HAL as we
    // just moved them to a new input stream.
    chain->syncHalEffectsState();

    mEffectChains.add(chain);

    return NO_ERROR;
}

size_t AudioFlinger::RecordThread::removeEffectChain_l(const sp<EffectChain>& chain)
{
    ALOGV("removeEffectChain_l() %p from thread %p", chain.get(), this);

    for (size_t i = 0; i < mEffectChains.size(); i++) {
        if (chain == mEffectChains[i]) {
            mEffectChains.removeAt(i);
            break;
        }
    }
    return mEffectChains.size();
}

status_t AudioFlinger::RecordThread::createAudioPatch_l(const struct audio_patch *patch,
                                                          audio_patch_handle_t *handle)
{
    status_t status = NO_ERROR;

    // store new device and send to effects
    mInDevice = patch->sources[0].ext.device.type;
    audio_port_handle_t deviceId = patch->sources[0].id;
    mPatch = *patch;
    for (size_t i = 0; i < mEffectChains.size(); i++) {
        mEffectChains[i]->setDevice_l(mInDevice);
    }

    checkBtNrec_l();

    // store new source and send to effects
    if (mAudioSource != patch->sinks[0].ext.mix.usecase.source) {
        mAudioSource = patch->sinks[0].ext.mix.usecase.source;
        for (size_t i = 0; i < mEffectChains.size(); i++) {
            mEffectChains[i]->setAudioSource_l(mAudioSource);
        }
    }

    if (mInput->audioHwDev->supportsAudioPatches()) {
        sp<DeviceHalInterface> hwDevice = mInput->audioHwDev->hwDevice();
        status = hwDevice->createAudioPatch(patch->num_sources,
                                            patch->sources,
                                            patch->num_sinks,
                                            patch->sinks,
                                            handle);
    } else {
        char *address;
        if (strcmp(patch->sources[0].ext.device.address, "") != 0) {
            address = audio_device_address_to_parameter(
                                                patch->sources[0].ext.device.type,
                                                patch->sources[0].ext.device.address);
        } else {
            address = (char *)calloc(1, 1);
        }
        AudioParameter param = AudioParameter(String8(address));
        free(address);
        param.addInt(String8(AudioParameter::keyRouting),
                     (int)patch->sources[0].ext.device.type);
        param.addInt(String8(AudioParameter::keyInputSource),
                                         (int)patch->sinks[0].ext.mix.usecase.source);
        status = mInput->stream->setParameters(param.toString());
        *handle = AUDIO_PATCH_HANDLE_NONE;
    }

    if ((mInDevice != mPrevInDevice) || (mDeviceId != deviceId)) {
        sendIoConfigEvent_l(AUDIO_INPUT_CONFIG_CHANGED);
        mPrevInDevice = mInDevice;
        mDeviceId = deviceId;
    }

    return status;
}

status_t AudioFlinger::RecordThread::releaseAudioPatch_l(const audio_patch_handle_t handle)
{
    status_t status = NO_ERROR;

    mInDevice = AUDIO_DEVICE_NONE;

    if (mInput->audioHwDev->supportsAudioPatches()) {
        sp<DeviceHalInterface> hwDevice = mInput->audioHwDev->hwDevice();
        status = hwDevice->releaseAudioPatch(handle);
    } else {
        AudioParameter param;
        param.addInt(String8(AudioParameter::keyRouting), 0);
        status = mInput->stream->setParameters(param.toString());
    }
    return status;
}

void AudioFlinger::RecordThread::addPatchTrack(const sp<PatchRecord>& record)
{
    Mutex::Autolock _l(mLock);
    mTracks.add(record);
}

void AudioFlinger::RecordThread::deletePatchTrack(const sp<PatchRecord>& record)
{
    Mutex::Autolock _l(mLock);
    destroyTrack_l(record);
}

void AudioFlinger::RecordThread::toAudioPortConfig(struct audio_port_config *config)
{
    ThreadBase::toAudioPortConfig(config);
    config->role = AUDIO_PORT_ROLE_SINK;
    config->ext.mix.hw_module = mInput->audioHwDev->handle();
    config->ext.mix.usecase.source = mAudioSource;
    if (mInput && mInput->flags != AUDIO_INPUT_FLAG_NONE) {
        config->config_mask |= AUDIO_PORT_CONFIG_FLAGS;
        config->flags.input = mInput->flags;
    }
}

// ----------------------------------------------------------------------------
//      Mmap
// ----------------------------------------------------------------------------

AudioFlinger::MmapThreadHandle::MmapThreadHandle(const sp<MmapThread>& thread)
    : mThread(thread)
{
    assert(thread != 0); // thread must start non-null and stay non-null
}

AudioFlinger::MmapThreadHandle::~MmapThreadHandle()
{
    mThread->disconnect();
}

status_t AudioFlinger::MmapThreadHandle::createMmapBuffer(int32_t minSizeFrames,
                                  struct audio_mmap_buffer_info *info)
{
    return mThread->createMmapBuffer(minSizeFrames, info);
}

status_t AudioFlinger::MmapThreadHandle::getMmapPosition(struct audio_mmap_position *position)
{
    return mThread->getMmapPosition(position);
}

status_t AudioFlinger::MmapThreadHandle::start(const AudioClient& client,
        audio_port_handle_t *handle)

{
    return mThread->start(client, handle);
}

status_t AudioFlinger::MmapThreadHandle::stop(audio_port_handle_t handle)
{
    return mThread->stop(handle);
}

status_t AudioFlinger::MmapThreadHandle::standby()
{
    return mThread->standby();
}


AudioFlinger::MmapThread::MmapThread(
        const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
        AudioHwDevice *hwDev, sp<StreamHalInterface> stream,
        audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady)
    : ThreadBase(audioFlinger, id, outDevice, inDevice, MMAP, systemReady),
      mSessionId(AUDIO_SESSION_NONE),
      mPortId(AUDIO_PORT_HANDLE_NONE),
      mHalStream(stream), mHalDevice(hwDev->hwDevice()), mAudioHwDev(hwDev),
      mActiveTracks(&this->mLocalLog),
      mHalVolFloat(-1.0f), // Initialize to illegal value so it always gets set properly later.
      mNoCallbackWarningCount(0)
{
    mStandby = true;
    readHalParameters_l();
}

AudioFlinger::MmapThread::~MmapThread()
{
    releaseWakeLock_l();
}

void AudioFlinger::MmapThread::onFirstRef()
{
    run(mThreadName, ANDROID_PRIORITY_URGENT_AUDIO);
}

void AudioFlinger::MmapThread::disconnect()
{
    ActiveTracks<MmapTrack> activeTracks;
    {
        Mutex::Autolock _l(mLock);
        for (const sp<MmapTrack> &t : mActiveTracks) {
            activeTracks.add(t);
        }
    }
    for (const sp<MmapTrack> &t : activeTracks) {
        stop(t->portId());
    }
    // This will decrement references and may cause the destruction of this thread.
    if (isOutput()) {
        AudioSystem::releaseOutput(mPortId);
    } else {
        AudioSystem::releaseInput(mPortId);
    }
}


void AudioFlinger::MmapThread::configure(const audio_attributes_t *attr,
                                                audio_stream_type_t streamType __unused,
                                                audio_session_t sessionId,
                                                const sp<MmapStreamCallback>& callback,
                                                audio_port_handle_t deviceId,
                                                audio_port_handle_t portId)
{
    mAttr = *attr;
    mSessionId = sessionId;
    mCallback = callback;
    mDeviceId = deviceId;
    mPortId = portId;
}

status_t AudioFlinger::MmapThread::createMmapBuffer(int32_t minSizeFrames,
                                  struct audio_mmap_buffer_info *info)
{
    if (mHalStream == 0) {
        return NO_INIT;
    }
    mStandby = true;
    acquireWakeLock();
    return mHalStream->createMmapBuffer(minSizeFrames, info);
}

status_t AudioFlinger::MmapThread::getMmapPosition(struct audio_mmap_position *position)
{
    if (mHalStream == 0) {
        return NO_INIT;
    }
    return mHalStream->getMmapPosition(position);
}

status_t AudioFlinger::MmapThread::exitStandby()
{
    status_t ret = mHalStream->start();
    if (ret != NO_ERROR) {
        ALOGE("%s: error mHalStream->start() = %d for first track", __FUNCTION__, ret);
        return ret;
    }
    mStandby = false;
    return NO_ERROR;
}

status_t AudioFlinger::MmapThread::start(const AudioClient& client,
                                         audio_port_handle_t *handle)
{
    ALOGV("%s clientUid %d mStandby %d mPortId %d *handle %d", __FUNCTION__,
          client.clientUid, mStandby, mPortId, *handle);
    if (mHalStream == 0) {
        return NO_INIT;
    }

    status_t ret;

    if (*handle == mPortId) {
        // for the first track, reuse portId and session allocated when the stream was opened
        return exitStandby();
    }

    audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE;

    audio_io_handle_t io = mId;
    if (isOutput()) {
        audio_config_t config = AUDIO_CONFIG_INITIALIZER;
        config.sample_rate = mSampleRate;
        config.channel_mask = mChannelMask;
        config.format = mFormat;
        audio_stream_type_t stream = streamType();
        audio_output_flags_t flags =
                (audio_output_flags_t)(AUDIO_OUTPUT_FLAG_MMAP_NOIRQ | AUDIO_OUTPUT_FLAG_DIRECT);
        audio_port_handle_t deviceId = mDeviceId;
        std::vector<audio_io_handle_t> secondaryOutputs;
        ret = AudioSystem::getOutputForAttr(&mAttr, &io,
                                            mSessionId,
                                            &stream,
                                            client.clientPid,
                                            client.clientUid,
                                            &config,
                                            flags,
                                            &deviceId,
                                            &portId,
                                            &secondaryOutputs);
        ALOGD_IF(!secondaryOutputs.empty(),
                 "MmapThread::start does not support secondary outputs, ignoring them");
    } else {
        audio_config_base_t config;
        config.sample_rate = mSampleRate;
        config.channel_mask = mChannelMask;
        config.format = mFormat;
        audio_port_handle_t deviceId = mDeviceId;
        ret = AudioSystem::getInputForAttr(&mAttr, &io,
                                              RECORD_RIID_INVALID,
                                              mSessionId,
                                              client.clientPid,
                                              client.clientUid,
                                              client.packageName,
                                              &config,
                                              AUDIO_INPUT_FLAG_MMAP_NOIRQ,
                                              &deviceId,
                                              &portId);
    }
    // APM should not chose a different input or output stream for the same set of attributes
    // and audo configuration
    if (ret != NO_ERROR || io != mId) {
        ALOGE("%s: error getting output or input from APM (error %d, io %d expected io %d)",
              __FUNCTION__, ret, io, mId);
        return BAD_VALUE;
    }

    if (isOutput()) {
        ret = AudioSystem::startOutput(portId);
    } else {
        ret = AudioSystem::startInput(portId);
    }

    Mutex::Autolock _l(mLock);
    // abort if start is rejected by audio policy manager
    if (ret != NO_ERROR) {
        ALOGE("%s: error start rejected by AudioPolicyManager = %d", __FUNCTION__, ret);
        if (!mActiveTracks.isEmpty()) {
            mLock.unlock();
            if (isOutput()) {
                AudioSystem::releaseOutput(portId);
            } else {
                AudioSystem::releaseInput(portId);
            }
            mLock.lock();
        } else {
            mHalStream->stop();
        }
        return PERMISSION_DENIED;
    }

    // Given that MmapThread::mAttr is mutable, should a MmapTrack have attributes ?
    sp<MmapTrack> track = new MmapTrack(this, mAttr, mSampleRate, mFormat, mChannelMask, mSessionId,
                                        isOutput(), client.clientUid, client.clientPid,
                                        IPCThreadState::self()->getCallingPid(), portId);

    if (isOutput()) {
        // force volume update when a new track is added
        mHalVolFloat = -1.0f;
    } else if (!track->isSilenced_l()) {
        for (const sp<MmapTrack> &t : mActiveTracks) {
            if (t->isSilenced_l() && t->uid() != client.clientUid)
                t->invalidate();
        }
    }


    mActiveTracks.add(track);
    sp<EffectChain> chain = getEffectChain_l(mSessionId);
    if (chain != 0) {
        chain->setStrategy(AudioSystem::getStrategyForStream(streamType()));
        chain->incTrackCnt();
        chain->incActiveTrackCnt();
    }

    *handle = portId;
    broadcast_l();

    ALOGV("%s DONE handle %d stream %p", __FUNCTION__, *handle, mHalStream.get());

    return NO_ERROR;
}

status_t AudioFlinger::MmapThread::stop(audio_port_handle_t handle)
{
    ALOGV("%s handle %d", __FUNCTION__, handle);

    if (mHalStream == 0) {
        return NO_INIT;
    }

    if (handle == mPortId) {
        mHalStream->stop();
        return NO_ERROR;
    }

    Mutex::Autolock _l(mLock);

    sp<MmapTrack> track;
    for (const sp<MmapTrack> &t : mActiveTracks) {
        if (handle == t->portId()) {
            track = t;
            break;
        }
    }
    if (track == 0) {
        return BAD_VALUE;
    }

    mActiveTracks.remove(track);

    mLock.unlock();
    if (isOutput()) {
        AudioSystem::stopOutput(track->portId());
        AudioSystem::releaseOutput(track->portId());
    } else {
        AudioSystem::stopInput(track->portId());
        AudioSystem::releaseInput(track->portId());
    }
    mLock.lock();

    sp<EffectChain> chain = getEffectChain_l(track->sessionId());
    if (chain != 0) {
        chain->decActiveTrackCnt();
        chain->decTrackCnt();
    }

    broadcast_l();

    return NO_ERROR;
}

status_t AudioFlinger::MmapThread::standby()
{
    ALOGV("%s", __FUNCTION__);

    if (mHalStream == 0) {
        return NO_INIT;
    }
    if (!mActiveTracks.isEmpty()) {
        return INVALID_OPERATION;
    }
    mHalStream->standby();
    mStandby = true;
    releaseWakeLock();
    return NO_ERROR;
}


void AudioFlinger::MmapThread::readHalParameters_l()
{
    status_t result = mHalStream->getAudioProperties(&mSampleRate, &mChannelMask, &mHALFormat);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving audio properties from HAL: %d", result);
    mFormat = mHALFormat;
    LOG_ALWAYS_FATAL_IF(!audio_is_linear_pcm(mFormat), "HAL format %#x is not linear pcm", mFormat);
    result = mHalStream->getFrameSize(&mFrameSize);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving frame size from HAL: %d", result);
    result = mHalStream->getBufferSize(&mBufferSize);
    LOG_ALWAYS_FATAL_IF(result != OK, "Error retrieving buffer size from HAL: %d", result);
    mFrameCount = mBufferSize / mFrameSize;
}

bool AudioFlinger::MmapThread::threadLoop()
{
    checkSilentMode_l();

    const String8 myName(String8::format("thread %p type %d TID %d", this, mType, gettid()));

    while (!exitPending())
    {
        Vector< sp<EffectChain> > effectChains;

        { // under Thread lock
        Mutex::Autolock _l(mLock);

        if (mSignalPending) {
            // A signal was raised while we were unlocked
            mSignalPending = false;
        } else {
            if (mConfigEvents.isEmpty()) {
                // we're about to wait, flush the binder command buffer
                IPCThreadState::self()->flushCommands();

                if (exitPending()) {
                    break;
                }

                // wait until we have something to do...
                ALOGV("%s going to sleep", myName.string());
                mWaitWorkCV.wait(mLock);
                ALOGV("%s waking up", myName.string());

                checkSilentMode_l();

                continue;
            }
        }

        processConfigEvents_l();

        processVolume_l();

        checkInvalidTracks_l();

        mActiveTracks.updatePowerState(this);

        updateMetadata_l();

        lockEffectChains_l(effectChains);
        } // release Thread lock

        for (size_t i = 0; i < effectChains.size(); i ++) {
            effectChains[i]->process_l(); // Thread is not locked, but effect chain is locked
        }

        // enable changes in effect chain, including moving to another thread.
        unlockEffectChains(effectChains);
        // Effect chains will be actually deleted here if they were removed from
        // mEffectChains list during mixing or effects processing
    }

    threadLoop_exit();

    if (!mStandby) {
        threadLoop_standby();
        mStandby = true;
    }

    ALOGV("Thread %p type %d exiting", this, mType);
    return false;
}

// checkForNewParameter_l() must be called with ThreadBase::mLock held
bool AudioFlinger::MmapThread::checkForNewParameter_l(const String8& keyValuePair,
                                                              status_t& status)
{
    AudioParameter param = AudioParameter(keyValuePair);
    int value;
    bool sendToHal = true;
    if (param.getInt(String8(AudioParameter::keyRouting), value) == NO_ERROR) {
        audio_devices_t device = (audio_devices_t)value;
        // forward device change to effects that have requested to be
        // aware of attached audio device.
        if (device != AUDIO_DEVICE_NONE) {
            for (size_t i = 0; i < mEffectChains.size(); i++) {
                mEffectChains[i]->setDevice_l(device);
            }
        }
        if (audio_is_output_devices(device)) {
            mOutDevice = device;
            if (!isOutput()) {
                sendToHal = false;
            }
        } else {
            mInDevice = device;
            if (device != AUDIO_DEVICE_NONE) {
                mPrevInDevice = value;
            }
            // TODO: implement and call checkBtNrec_l();
        }
    }
    if (sendToHal) {
        status = mHalStream->setParameters(keyValuePair);
    } else {
        status = NO_ERROR;
    }

    return false;
}

String8 AudioFlinger::MmapThread::getParameters(const String8& keys)
{
    Mutex::Autolock _l(mLock);
    String8 out_s8;
    if (initCheck() == NO_ERROR && mHalStream->getParameters(keys, &out_s8) == OK) {
        return out_s8;
    }
    return String8();
}

void AudioFlinger::MmapThread::ioConfigChanged(audio_io_config_event event, pid_t pid,
                                               audio_port_handle_t portId __unused) {
    sp<AudioIoDescriptor> desc = new AudioIoDescriptor();

    desc->mIoHandle = mId;

    switch (event) {
    case AUDIO_INPUT_OPENED:
    case AUDIO_INPUT_REGISTERED:
    case AUDIO_INPUT_CONFIG_CHANGED:
    case AUDIO_OUTPUT_OPENED:
    case AUDIO_OUTPUT_REGISTERED:
    case AUDIO_OUTPUT_CONFIG_CHANGED:
        desc->mPatch = mPatch;
        desc->mChannelMask = mChannelMask;
        desc->mSamplingRate = mSampleRate;
        desc->mFormat = mFormat;
        desc->mFrameCount = mFrameCount;
        desc->mFrameCountHAL = mFrameCount;
        desc->mLatency = 0;
        break;

    case AUDIO_INPUT_CLOSED:
    case AUDIO_OUTPUT_CLOSED:
    default:
        break;
    }
    mAudioFlinger->ioConfigChanged(event, desc, pid);
}

status_t AudioFlinger::MmapThread::createAudioPatch_l(const struct audio_patch *patch,
                                                          audio_patch_handle_t *handle)
{
    status_t status = NO_ERROR;

    // store new device and send to effects
    audio_devices_t type = AUDIO_DEVICE_NONE;
    audio_port_handle_t deviceId;
    if (isOutput()) {
        for (unsigned int i = 0; i < patch->num_sinks; i++) {
            type |= patch->sinks[i].ext.device.type;
        }
        deviceId = patch->sinks[0].id;
    } else {
        type = patch->sources[0].ext.device.type;
        deviceId = patch->sources[0].id;
    }

    for (size_t i = 0; i < mEffectChains.size(); i++) {
        mEffectChains[i]->setDevice_l(type);
    }

    if (isOutput()) {
        mOutDevice = type;
    } else {
        mInDevice = type;
        // store new source and send to effects
        if (mAudioSource != patch->sinks[0].ext.mix.usecase.source) {
            mAudioSource = patch->sinks[0].ext.mix.usecase.source;
            for (size_t i = 0; i < mEffectChains.size(); i++) {
                mEffectChains[i]->setAudioSource_l(mAudioSource);
            }
        }
    }

    if (mAudioHwDev->supportsAudioPatches()) {
        status = mHalDevice->createAudioPatch(patch->num_sources,
                                            patch->sources,
                                            patch->num_sinks,
                                            patch->sinks,
                                            handle);
    } else {
        char *address;
        if (strcmp(patch->sinks[0].ext.device.address, "") != 0) {
            //FIXME: we only support address on first sink with HAL version < 3.0
            address = audio_device_address_to_parameter(
                                                        patch->sinks[0].ext.device.type,
                                                        patch->sinks[0].ext.device.address);
        } else {
            address = (char *)calloc(1, 1);
        }
        AudioParameter param = AudioParameter(String8(address));
        free(address);
        param.addInt(String8(AudioParameter::keyRouting), (int)type);
        if (!isOutput()) {
            param.addInt(String8(AudioParameter::keyInputSource),
                                         (int)patch->sinks[0].ext.mix.usecase.source);
        }
        status = mHalStream->setParameters(param.toString());
        *handle = AUDIO_PATCH_HANDLE_NONE;
    }

    if (isOutput() && (mPrevOutDevice != mOutDevice || mDeviceId != deviceId)) {
        mPrevOutDevice = type;
        sendIoConfigEvent_l(AUDIO_OUTPUT_CONFIG_CHANGED);
        sp<MmapStreamCallback> callback = mCallback.promote();
        if (mDeviceId != deviceId && callback != 0) {
            mLock.unlock();
            callback->onRoutingChanged(deviceId);
            mLock.lock();
        }
        mDeviceId = deviceId;
    }
    if (!isOutput() && (mPrevInDevice != mInDevice || mDeviceId != deviceId)) {
        mPrevInDevice = type;
        sendIoConfigEvent_l(AUDIO_INPUT_CONFIG_CHANGED);
        sp<MmapStreamCallback> callback = mCallback.promote();
        if (mDeviceId != deviceId && callback != 0) {
            mLock.unlock();
            callback->onRoutingChanged(deviceId);
            mLock.lock();
        }
        mDeviceId = deviceId;
    }
    return status;
}

status_t AudioFlinger::MmapThread::releaseAudioPatch_l(const audio_patch_handle_t handle)
{
    status_t status = NO_ERROR;

    mInDevice = AUDIO_DEVICE_NONE;

    bool supportsAudioPatches = mHalDevice->supportsAudioPatches(&supportsAudioPatches) == OK ?
                                        supportsAudioPatches : false;

    if (supportsAudioPatches) {
        status = mHalDevice->releaseAudioPatch(handle);
    } else {
        AudioParameter param;
        param.addInt(String8(AudioParameter::keyRouting), 0);
        status = mHalStream->setParameters(param.toString());
    }
    return status;
}

void AudioFlinger::MmapThread::toAudioPortConfig(struct audio_port_config *config)
{
    ThreadBase::toAudioPortConfig(config);
    if (isOutput()) {
        config->role = AUDIO_PORT_ROLE_SOURCE;
        config->ext.mix.hw_module = mAudioHwDev->handle();
        config->ext.mix.usecase.stream = AUDIO_STREAM_DEFAULT;
    } else {
        config->role = AUDIO_PORT_ROLE_SINK;
        config->ext.mix.hw_module = mAudioHwDev->handle();
        config->ext.mix.usecase.source = mAudioSource;
    }
}

status_t AudioFlinger::MmapThread::addEffectChain_l(const sp<EffectChain>& chain)
{
    audio_session_t session = chain->sessionId();

    ALOGV("addEffectChain_l() %p on thread %p for session %d", chain.get(), this, session);
    // Attach all tracks with same session ID to this chain.
    // indicate all active tracks in the chain
    for (const sp<MmapTrack> &track : mActiveTracks) {
        if (session == track->sessionId()) {
            chain->incTrackCnt();
            chain->incActiveTrackCnt();
        }
    }

    chain->setThread(this);
    chain->setInBuffer(nullptr);
    chain->setOutBuffer(nullptr);
    chain->syncHalEffectsState();

    mEffectChains.add(chain);
    checkSuspendOnAddEffectChain_l(chain);
    return NO_ERROR;
}

size_t AudioFlinger::MmapThread::removeEffectChain_l(const sp<EffectChain>& chain)
{
    audio_session_t session = chain->sessionId();

    ALOGV("removeEffectChain_l() %p from thread %p for session %d", chain.get(), this, session);

    for (size_t i = 0; i < mEffectChains.size(); i++) {
        if (chain == mEffectChains[i]) {
            mEffectChains.removeAt(i);
            // detach all active tracks from the chain
            // detach all tracks with same session ID from this chain
            for (const sp<MmapTrack> &track : mActiveTracks) {
                if (session == track->sessionId()) {
                    chain->decActiveTrackCnt();
                    chain->decTrackCnt();
                }
            }
            break;
        }
    }
    return mEffectChains.size();
}

void AudioFlinger::MmapThread::threadLoop_standby()
{
    mHalStream->standby();
}

void AudioFlinger::MmapThread::threadLoop_exit()
{
    // Do not call callback->onTearDown() because it is redundant for thread exit
    // and because it can cause a recursive mutex lock on stop().
}

status_t AudioFlinger::MmapThread::setSyncEvent(const sp<SyncEvent>& event __unused)
{
    return BAD_VALUE;
}

bool AudioFlinger::MmapThread::isValidSyncEvent(const sp<SyncEvent>& event __unused) const
{
    return false;
}

status_t AudioFlinger::MmapThread::checkEffectCompatibility_l(
        const effect_descriptor_t *desc, audio_session_t sessionId)
{
    // No global effect sessions on mmap threads
    if (sessionId == AUDIO_SESSION_OUTPUT_MIX || sessionId == AUDIO_SESSION_OUTPUT_STAGE) {
        ALOGW("checkEffectCompatibility_l(): global effect %s on record thread %s",
                desc->name, mThreadName);
        return BAD_VALUE;
    }

    if (!isOutput() && ((desc->flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_PRE_PROC)) {
        ALOGW("checkEffectCompatibility_l(): non pre processing effect %s on capture mmap thread",
                desc->name);
        return BAD_VALUE;
    }
    if (isOutput() && ((desc->flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_PRE_PROC)) {
        ALOGW("checkEffectCompatibility_l(): pre processing effect %s created on playback mmap "
              "thread", desc->name);
        return BAD_VALUE;
    }

    // Only allow effects without processing load or latency
    if ((desc->flags & EFFECT_FLAG_NO_PROCESS_MASK) != EFFECT_FLAG_NO_PROCESS) {
        return BAD_VALUE;
    }

    return NO_ERROR;

}

void AudioFlinger::MmapThread::checkInvalidTracks_l()
{
    for (const sp<MmapTrack> &track : mActiveTracks) {
        if (track->isInvalid()) {
            sp<MmapStreamCallback> callback = mCallback.promote();
            if (callback != 0) {
                mLock.unlock();
                callback->onTearDown(track->portId());
                mLock.lock();
            } else if (mNoCallbackWarningCount < kMaxNoCallbackWarnings) {
                ALOGW("Could not notify MMAP stream tear down: no onTearDown callback!");
                mNoCallbackWarningCount++;
            }
        }
    }
}

void AudioFlinger::MmapThread::dumpInternals_l(int fd, const Vector<String16>& args __unused)
{
    dprintf(fd, "  Attributes: content type %d usage %d source %d\n",
            mAttr.content_type, mAttr.usage, mAttr.source);
    dprintf(fd, "  Session: %d port Id: %d\n", mSessionId, mPortId);
    if (mActiveTracks.isEmpty()) {
        dprintf(fd, "  No active clients\n");
    }
}

void AudioFlinger::MmapThread::dumpTracks_l(int fd, const Vector<String16>& args __unused)
{
    String8 result;
    size_t numtracks = mActiveTracks.size();
    dprintf(fd, "  %zu Tracks\n", numtracks);
    const char *prefix = "    ";
    if (numtracks) {
        result.append(prefix);
        mActiveTracks[0]->appendDumpHeader(result);
        for (size_t i = 0; i < numtracks ; ++i) {
            sp<MmapTrack> track = mActiveTracks[i];
            result.append(prefix);
            track->appendDump(result, true /* active */);
        }
    } else {
        dprintf(fd, "\n");
    }
    write(fd, result.string(), result.size());
}

AudioFlinger::MmapPlaybackThread::MmapPlaybackThread(
        const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
        AudioHwDevice *hwDev,  AudioStreamOut *output,
        audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady)
    : MmapThread(audioFlinger, id, hwDev, output->stream, outDevice, inDevice, systemReady),
      mStreamType(AUDIO_STREAM_MUSIC),
      mStreamVolume(1.0),
      mStreamMute(false),
      mOutput(output)
{
    snprintf(mThreadName, kThreadNameLength, "AudioMmapOut_%X", id);
    mChannelCount = audio_channel_count_from_out_mask(mChannelMask);
    mMasterVolume = audioFlinger->masterVolume_l();
    mMasterMute = audioFlinger->masterMute_l();
    if (mAudioHwDev) {
        if (mAudioHwDev->canSetMasterVolume()) {
            mMasterVolume = 1.0;
        }

        if (mAudioHwDev->canSetMasterMute()) {
            mMasterMute = false;
        }
    }
}

void AudioFlinger::MmapPlaybackThread::configure(const audio_attributes_t *attr,
                                                audio_stream_type_t streamType,
                                                audio_session_t sessionId,
                                                const sp<MmapStreamCallback>& callback,
                                                audio_port_handle_t deviceId,
                                                audio_port_handle_t portId)
{
    MmapThread::configure(attr, streamType, sessionId, callback, deviceId, portId);
    mStreamType = streamType;
}

AudioStreamOut* AudioFlinger::MmapPlaybackThread::clearOutput()
{
    Mutex::Autolock _l(mLock);
    AudioStreamOut *output = mOutput;
    mOutput = NULL;
    return output;
}

void AudioFlinger::MmapPlaybackThread::setMasterVolume(float value)
{
    Mutex::Autolock _l(mLock);
    // Don't apply master volume in SW if our HAL can do it for us.
    if (mAudioHwDev &&
            mAudioHwDev->canSetMasterVolume()) {
        mMasterVolume = 1.0;
    } else {
        mMasterVolume = value;
    }
}

void AudioFlinger::MmapPlaybackThread::setMasterMute(bool muted)
{
    Mutex::Autolock _l(mLock);
    // Don't apply master mute in SW if our HAL can do it for us.
    if (mAudioHwDev && mAudioHwDev->canSetMasterMute()) {
        mMasterMute = false;
    } else {
        mMasterMute = muted;
    }
}

void AudioFlinger::MmapPlaybackThread::setStreamVolume(audio_stream_type_t stream, float value)
{
    Mutex::Autolock _l(mLock);
    if (stream == mStreamType) {
        mStreamVolume = value;
        broadcast_l();
    }
}

float AudioFlinger::MmapPlaybackThread::streamVolume(audio_stream_type_t stream) const
{
    Mutex::Autolock _l(mLock);
    if (stream == mStreamType) {
        return mStreamVolume;
    }
    return 0.0f;
}

void AudioFlinger::MmapPlaybackThread::setStreamMute(audio_stream_type_t stream, bool muted)
{
    Mutex::Autolock _l(mLock);
    if (stream == mStreamType) {
        mStreamMute= muted;
        broadcast_l();
    }
}

void AudioFlinger::MmapPlaybackThread::invalidateTracks(audio_stream_type_t streamType)
{
    Mutex::Autolock _l(mLock);
    if (streamType == mStreamType) {
        for (const sp<MmapTrack> &track : mActiveTracks) {
            track->invalidate();
        }
        broadcast_l();
    }
}

void AudioFlinger::MmapPlaybackThread::processVolume_l()
{
    float volume;

    if (mMasterMute || mStreamMute) {
        volume = 0;
    } else {
        volume = mMasterVolume * mStreamVolume;
    }

    if (volume != mHalVolFloat) {

        // Convert volumes from float to 8.24
        uint32_t vol = (uint32_t)(volume * (1 << 24));

        // Delegate volume control to effect in track effect chain if needed
        // only one effect chain can be present on DirectOutputThread, so if
        // there is one, the track is connected to it
        if (!mEffectChains.isEmpty()) {
            mEffectChains[0]->setVolume_l(&vol, &vol);
            volume = (float)vol / (1 << 24);
        }
        // Try to use HW volume control and fall back to SW control if not implemented
        if (mOutput->stream->setVolume(volume, volume) == NO_ERROR) {
            mHalVolFloat = volume; // HW volume control worked, so update value.
            mNoCallbackWarningCount = 0;
        } else {
            sp<MmapStreamCallback> callback = mCallback.promote();
            if (callback != 0) {
                int channelCount;
                if (isOutput()) {
                    channelCount = audio_channel_count_from_out_mask(mChannelMask);
                } else {
                    channelCount = audio_channel_count_from_in_mask(mChannelMask);
                }
                Vector<float> values;
                for (int i = 0; i < channelCount; i++) {
                    values.add(volume);
                }
                mHalVolFloat = volume; // SW volume control worked, so update value.
                mNoCallbackWarningCount = 0;
                mLock.unlock();
                callback->onVolumeChanged(mChannelMask, values);
                mLock.lock();
            } else {
                if (mNoCallbackWarningCount < kMaxNoCallbackWarnings) {
                    ALOGW("Could not set MMAP stream volume: no volume callback!");
                    mNoCallbackWarningCount++;
                }
            }
        }
    }
}

void AudioFlinger::MmapPlaybackThread::updateMetadata_l()
{
    if (mOutput == nullptr || mOutput->stream == nullptr ||
            !mActiveTracks.readAndClearHasChanged()) {
        return;
    }
    StreamOutHalInterface::SourceMetadata metadata;
    for (const sp<MmapTrack> &track : mActiveTracks) {
        // No track is invalid as this is called after prepareTrack_l in the same critical section
        metadata.tracks.push_back({
                .usage = track->attributes().usage,
                .content_type = track->attributes().content_type,
                .gain = mHalVolFloat, // TODO: propagate from aaudio pre-mix volume
        });
    }
    mOutput->stream->updateSourceMetadata(metadata);
}

void AudioFlinger::MmapPlaybackThread::checkSilentMode_l()
{
    if (!mMasterMute) {
        char value[PROPERTY_VALUE_MAX];
        if (property_get("ro.audio.silent", value, "0") > 0) {
            char *endptr;
            unsigned long ul = strtoul(value, &endptr, 0);
            if (*endptr == '\0' && ul != 0) {
                ALOGD("Silence is golden");
                // The setprop command will not allow a property to be changed after
                // the first time it is set, so we don't have to worry about un-muting.
                setMasterMute_l(true);
            }
        }
    }
}

void AudioFlinger::MmapPlaybackThread::toAudioPortConfig(struct audio_port_config *config)
{
    MmapThread::toAudioPortConfig(config);
    if (mOutput && mOutput->flags != AUDIO_OUTPUT_FLAG_NONE) {
        config->config_mask |= AUDIO_PORT_CONFIG_FLAGS;
        config->flags.output = mOutput->flags;
    }
}

void AudioFlinger::MmapPlaybackThread::dumpInternals_l(int fd, const Vector<String16>& args)
{
    MmapThread::dumpInternals_l(fd, args);

    dprintf(fd, "  Stream type: %d Stream volume: %f HAL volume: %f Stream mute %d\n",
            mStreamType, mStreamVolume, mHalVolFloat, mStreamMute);
    dprintf(fd, "  Master volume: %f Master mute %d\n", mMasterVolume, mMasterMute);
}

AudioFlinger::MmapCaptureThread::MmapCaptureThread(
        const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
        AudioHwDevice *hwDev,  AudioStreamIn *input,
        audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady)
    : MmapThread(audioFlinger, id, hwDev, input->stream, outDevice, inDevice, systemReady),
      mInput(input)
{
    snprintf(mThreadName, kThreadNameLength, "AudioMmapIn_%X", id);
    mChannelCount = audio_channel_count_from_in_mask(mChannelMask);
}

status_t AudioFlinger::MmapCaptureThread::exitStandby()
{
    {
        // mInput might have been cleared by clearInput()
        Mutex::Autolock _l(mLock);
        if (mInput != nullptr && mInput->stream != nullptr) {
            mInput->stream->setGain(1.0f);
        }
    }
    return MmapThread::exitStandby();
}

AudioFlinger::AudioStreamIn* AudioFlinger::MmapCaptureThread::clearInput()
{
    Mutex::Autolock _l(mLock);
    AudioStreamIn *input = mInput;
    mInput = NULL;
    return input;
}


void AudioFlinger::MmapCaptureThread::processVolume_l()
{
    bool changed = false;
    bool silenced = false;

    sp<MmapStreamCallback> callback = mCallback.promote();
    if (callback == 0) {
        if (mNoCallbackWarningCount < kMaxNoCallbackWarnings) {
            ALOGW("Could not set MMAP stream silenced: no onStreamSilenced callback!");
            mNoCallbackWarningCount++;
        }
    }

    // After a change occurred in track silenced state, mute capture in audio DSP if at least one
    // track is silenced and unmute otherwise
    for (size_t i = 0; i < mActiveTracks.size() && !silenced; i++) {
        if (!mActiveTracks[i]->getAndSetSilencedNotified_l()) {
            changed = true;
            silenced = mActiveTracks[i]->isSilenced_l();
        }
    }

    if (changed) {
        mInput->stream->setGain(silenced ? 0.0f: 1.0f);
    }
}

void AudioFlinger::MmapCaptureThread::updateMetadata_l()
{
    if (mInput == nullptr || mInput->stream == nullptr ||
            !mActiveTracks.readAndClearHasChanged()) {
        return;
    }
    StreamInHalInterface::SinkMetadata metadata;
    for (const sp<MmapTrack> &track : mActiveTracks) {
        // No track is invalid as this is called after prepareTrack_l in the same critical section
        metadata.tracks.push_back({
                .source = track->attributes().source,
                .gain = 1, // capture tracks do not have volumes
        });
    }
    mInput->stream->updateSinkMetadata(metadata);
}

void AudioFlinger::MmapCaptureThread::setRecordSilenced(uid_t uid, bool silenced)
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mActiveTracks.size() ; i++) {
        if (mActiveTracks[i]->uid() == uid) {
            mActiveTracks[i]->setSilenced_l(silenced);
            broadcast_l();
        }
    }
}

void AudioFlinger::MmapCaptureThread::toAudioPortConfig(struct audio_port_config *config)
{
    MmapThread::toAudioPortConfig(config);
    if (mInput && mInput->flags != AUDIO_INPUT_FLAG_NONE) {
        config->config_mask |= AUDIO_PORT_CONFIG_FLAGS;
        config->flags.input = mInput->flags;
    }
}

} // namespace android
