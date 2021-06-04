/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Timers.h>
#include <utils/KeyedVector.h>
#include <system/audio.h>
#include "AudioIODescriptorInterface.h"
#include "AudioPort.h"
#include "ClientDescriptor.h"
#include "DeviceDescriptor.h"
#include <vector>

namespace android {

class IOProfile;
class AudioPolicyMix;
class AudioPolicyClientInterface;

class ActivityTracking
{
public:
    virtual ~ActivityTracking() = default;
    bool isActive(uint32_t inPastMs = 0, nsecs_t sysTime = 0) const
    {
        if (mActivityCount > 0) {
            return true;
        }
        if (inPastMs == 0) {
            return false;
        }
        if (sysTime == 0) {
            sysTime = systemTime();
        }
        if (ns2ms(sysTime - mStopTime) < inPastMs) {
            return true;
        }
        return false;
    }
    void changeActivityCount(int delta)
    {
        if ((delta + (int)mActivityCount) < 0) {
            LOG_ALWAYS_FATAL("%s: invalid delta %d, refCount %d", __func__, delta, mActivityCount);
        }
        mActivityCount += delta;
        if (!mActivityCount) {
            setStopTime(systemTime());
        }
    }
    uint32_t getActivityCount() const { return mActivityCount; }
    nsecs_t getStopTime() const { return mStopTime; }
    void setStopTime(nsecs_t stopTime) { mStopTime = stopTime; }

    virtual void dump(String8 *dst, int spaces) const
    {
        dst->appendFormat("%*s- ActivityCount: %d, StopTime: %" PRId64 ", ", spaces, "",
                          getActivityCount(), getStopTime());
    }
private:
    uint32_t mActivityCount = 0;
    nsecs_t mStopTime = 0;
};

/**
 * @brief VolumeActivity: it tracks the activity for volume policy (volume index, mute,
 * memorize previous stop, and store mute if incompatible device with another strategy.
 */
class VolumeActivity : public ActivityTracking
{
public:
    bool isMuted() const { return mMuteCount > 0; }
    int getMuteCount() const { return mMuteCount; }
    int incMuteCount() { return ++mMuteCount; }
    int decMuteCount() { return mMuteCount > 0 ? --mMuteCount : -1; }

    void dump(String8 *dst, int spaces) const override
    {
        ActivityTracking::dump(dst, spaces);
        dst->appendFormat(", Volume: %.03f, MuteCount: %02d\n", mCurVolumeDb, mMuteCount);
    }
    void setVolume(float volumeDb) { mCurVolumeDb = volumeDb; }
    float getVolume() const { return mCurVolumeDb; }

private:
    int mMuteCount = 0; /**< mute request counter */
    float mCurVolumeDb = NAN; /**< current volume in dB. */
};
/**
 * Note: volume activities shall be indexed by CurvesId if we want to allow multiple
 * curves per volume source, inferring a mute management or volume balancing between HW and SW is
 * done
 */
using VolumeActivities = std::map<VolumeSource, VolumeActivity>;

/**
 * @brief The Activity class: it tracks the activity for volume policy (volume index, mute,
 * memorize previous stop, and store mute if incompatible device with another strategy.
 * Having this class prevents from looping on all attributes (legacy streams) of the strategy
 */
class RoutingActivity : public ActivityTracking
{
public:
    void setMutedByDevice( bool isMuted) { mIsMutedByDevice = isMuted; }
    bool isMutedByDevice() const { return mIsMutedByDevice; }

    void dump(String8 *dst, int spaces) const override {
        ActivityTracking::dump(dst, spaces);
        dst->appendFormat("\n");
    }
private:
    /**
     * strategies muted because of incompatible device selection.
     * See AudioPolicyManager::checkDeviceMuteStrategies()
     */
    bool mIsMutedByDevice = false;
};
using RoutingActivities = std::map<product_strategy_t, RoutingActivity>;

// descriptor for audio outputs. Used to maintain current configuration of each opened audio output
// and keep track of the usage of this output by each audio stream type.
class AudioOutputDescriptor: public AudioPortConfig, public AudioIODescriptorInterface
    , public ClientMapHandler<TrackClientDescriptor>
{
public:
    AudioOutputDescriptor(const sp<AudioPort>& port,
                          AudioPolicyClientInterface *clientInterface);
    virtual ~AudioOutputDescriptor() {}

    void dump(String8 *dst) const override;
    void        log(const char* indent);

    audio_port_handle_t getId() const;
    virtual DeviceVector devices() const { return mDevices; }
    bool sharesHwModuleWith(const sp<AudioOutputDescriptor>& outputDesc);
    virtual DeviceVector supportedDevices() const  { return mDevices; }
    virtual bool isDuplicated() const { return false; }
    virtual uint32_t latency() { return 0; }
    virtual bool isFixedVolume(audio_devices_t device);
    virtual bool setVolume(float volumeDb,
                           VolumeSource volumeSource, const StreamTypeVector &streams,
                           audio_devices_t device,
                           uint32_t delayMs,
                           bool force);

    /**
     * @brief setStopTime set the stop time due to the client stoppage or a re routing of this
     * client
     * @param client to be considered
     * @param sysTime when the client stopped/was rerouted
     */
    void setStopTime(const sp<TrackClientDescriptor>& client, nsecs_t sysTime);

    /**
     * Changes the client->active() state and the output descriptor's global active count,
     * along with the stream active count and mActiveClients.
     * The client must be previously added by the base class addClient().
     * In case of duplicating thread, client shall be added on the duplicated thread, not on the
     * involved outputs but setClientActive will be called on all output to track strategy and
     * active client for a given output.
     * Active ref count of the client will be incremented/decremented through setActive API
     */
    virtual void setClientActive(const sp<TrackClientDescriptor>& client, bool active);

    bool isActive(uint32_t inPastMs) const;
    bool isActive(VolumeSource volumeSource = VOLUME_SOURCE_NONE,
                  uint32_t inPastMs = 0,
                  nsecs_t sysTime = 0) const;
    bool isAnyActive(VolumeSource volumeSourceToIgnore) const;

    std::vector<VolumeSource> getActiveVolumeSources() const {
        std::vector<VolumeSource> activeList;
        for (const auto &iter : mVolumeActivities) {
            if (iter.second.isActive()) {
                activeList.push_back(iter.first);
            }
        }
        return activeList;
    }
    uint32_t getActivityCount(VolumeSource vs) const
    {
        return mVolumeActivities.find(vs) != std::end(mVolumeActivities)?
                    mVolumeActivities.at(vs).getActivityCount() : 0;
    }
    bool isMuted(VolumeSource vs) const
    {
        return mVolumeActivities.find(vs) != std::end(mVolumeActivities)?
                    mVolumeActivities.at(vs).isMuted() : false;
    }
    int getMuteCount(VolumeSource vs) const
    {
        return mVolumeActivities.find(vs) != std::end(mVolumeActivities)?
                    mVolumeActivities.at(vs).getMuteCount() : 0;
    }
    int incMuteCount(VolumeSource vs)
    {
        return mVolumeActivities[vs].incMuteCount();
    }
    int decMuteCount(VolumeSource vs)
    {
        return mVolumeActivities[vs].decMuteCount();
    }
    void setCurVolume(VolumeSource vs, float volumeDb)
    {
        // Even if not activity for this source registered, need to create anyway
        mVolumeActivities[vs].setVolume(volumeDb);
    }
    float getCurVolume(VolumeSource vs) const
    {
        return mVolumeActivities.find(vs) != std::end(mVolumeActivities) ?
                    mVolumeActivities.at(vs).getVolume() : NAN;
    }

    bool isStrategyActive(product_strategy_t ps, uint32_t inPastMs = 0, nsecs_t sysTime = 0) const
    {
        return mRoutingActivities.find(ps) != std::end(mRoutingActivities)?
                    mRoutingActivities.at(ps).isActive(inPastMs, sysTime) : false;
    }
    bool isStrategyMutedByDevice(product_strategy_t ps) const
    {
        return mRoutingActivities.find(ps) != std::end(mRoutingActivities)?
                    mRoutingActivities.at(ps).isMutedByDevice() : false;
    }
    void setStrategyMutedByDevice(product_strategy_t ps, bool isMuted)
    {
        mRoutingActivities[ps].setMutedByDevice(isMuted);
    }

    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
                           const struct audio_port_config *srcConfig = NULL) const;
    virtual sp<AudioPort> getAudioPort() const { return mPort; }
    virtual void toAudioPort(struct audio_port *port) const;

    audio_module_handle_t getModuleHandle() const;

    // implementation of AudioIODescriptorInterface
    audio_config_base_t getConfig() const override;
    audio_patch_handle_t getPatchHandle() const override;
    void setPatchHandle(audio_patch_handle_t handle) override;

    TrackClientVector clientsList(bool activeOnly = false,
                                  product_strategy_t strategy = PRODUCT_STRATEGY_NONE,
                                  bool preferredDeviceOnly = false) const;

    // override ClientMapHandler to abort when removing a client when active.
    void removeClient(audio_port_handle_t portId) override {
        auto client = getClient(portId);
        LOG_ALWAYS_FATAL_IF(client.get() == nullptr,
                "%s(%d): nonexistent client portId %d", __func__, mId, portId);
        // it is possible that when a client is removed, we could remove its
        // associated active count by calling changeStreamActiveCount(),
        // but that would be hiding a problem, so we log fatal instead.
        auto clientIter = std::find(begin(mActiveClients), end(mActiveClients), client);
        LOG_ALWAYS_FATAL_IF(clientIter != mActiveClients.end(),
                            "%s(%d) removing client portId %d which is active (count %d)",
                            __func__, mId, portId, client->getActivityCount());
        ClientMapHandler<TrackClientDescriptor>::removeClient(portId);
    }

    const TrackClientVector& getActiveClients() const {
        return mActiveClients;
    }

    bool useHwGain() const
    {
        return !devices().isEmpty() ? devices().itemAt(0)->hasGainController() : false;
    }

    DeviceVector mDevices; /**< current devices this output is routed to */
    wp<AudioPolicyMix> mPolicyMix;  // non NULL when used by a dynamic policy

protected:
    const sp<AudioPort> mPort;
    AudioPolicyClientInterface * const mClientInterface;
    uint32_t mGlobalActiveCount = 0;  // non-client-specific active count
    audio_patch_handle_t mPatchHandle = AUDIO_PATCH_HANDLE_NONE;
    audio_port_handle_t mId = AUDIO_PORT_HANDLE_NONE;

    // The ActiveClients shows the clients that contribute to the @VolumeSource counts
    // and may include upstream clients from a duplicating thread.
    // Compare with the ClientMap (mClients) which are external AudioTrack clients of the
    // output descriptor (and do not count internal PatchTracks).
    TrackClientVector mActiveClients;

    RoutingActivities mRoutingActivities; /**< track routing activity on this ouput.*/

    VolumeActivities mVolumeActivities; /**< track volume activity on this ouput.*/
};

// Audio output driven by a software mixer in audio flinger.
class SwAudioOutputDescriptor: public AudioOutputDescriptor
{
public:
    SwAudioOutputDescriptor(const sp<IOProfile>& profile,
                            AudioPolicyClientInterface *clientInterface);
    virtual ~SwAudioOutputDescriptor() {}

            void dump(String8 *dst) const override;
    virtual DeviceVector devices() const;
    void setDevices(const DeviceVector &devices) { mDevices = devices; }
    bool sharesHwModuleWith(const sp<SwAudioOutputDescriptor>& outputDesc);
    virtual DeviceVector supportedDevices() const;
    virtual bool deviceSupportsEncodedFormats(audio_devices_t device);
    virtual uint32_t latency();
    virtual bool isDuplicated() const { return (mOutput1 != NULL && mOutput2 != NULL); }
    virtual bool isFixedVolume(audio_devices_t device);
    sp<SwAudioOutputDescriptor> subOutput1() { return mOutput1; }
    sp<SwAudioOutputDescriptor> subOutput2() { return mOutput2; }
    void setClientActive(const sp<TrackClientDescriptor>& client, bool active) override;
    void setAllClientsInactive()
    {
        for (const auto &client : clientsList(true)) {
            setClientActive(client, false);
        }
    }
    virtual bool setVolume(float volumeDb,
                           VolumeSource volumeSource, const StreamTypeVector &streams,
                           audio_devices_t device,
                           uint32_t delayMs,
                           bool force);

    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
                           const struct audio_port_config *srcConfig = NULL) const;
    virtual void toAudioPort(struct audio_port *port) const;

        status_t open(const audio_config_t *config,
                      const DeviceVector &devices,
                      audio_stream_type_t stream,
                      audio_output_flags_t flags,
                      audio_io_handle_t *output);

        // Called when a stream is about to be started
        // Note: called before setClientActive(true);
        status_t start();
        // Called after a stream is stopped.
        // Note: called after setClientActive(false);
        void stop();
        void close();
        status_t openDuplicating(const sp<SwAudioOutputDescriptor>& output1,
                                 const sp<SwAudioOutputDescriptor>& output2,
                                 audio_io_handle_t *ioHandle);

    /**
     * @brief supportsDevice
     * @param device to be checked against
     * @return true if the device is supported by type (for non bus / remote submix devices),
     *         true if the device is supported (both type and address) for bus / remote submix
     *         false otherwise
     */
    bool supportsDevice(const sp<DeviceDescriptor> &device) const;

    /**
     * @brief supportsAllDevices
     * @param devices to be checked against
     * @return true if the device is weakly supported by type (e.g. for non bus / rsubmix devices),
     *         true if the device is supported (both type and address) for bus / remote submix
     *         false otherwise
     */
    bool supportsAllDevices(const DeviceVector &devices) const;

    /**
     * @brief filterSupportedDevices takes a vector of devices and filters them according to the
     * device supported by this output (the profile from which this output derives from)
     * @param devices reference device vector to be filtered
     * @return vector of devices filtered from the supported devices of this output (weakly or not
     * depending on the device type)
     */
    DeviceVector filterSupportedDevices(const DeviceVector &devices) const;

    const sp<IOProfile> mProfile;          // I/O profile this output derives from
    audio_io_handle_t mIoHandle;           // output handle
    uint32_t mLatency;                  //
    audio_output_flags_t mFlags;   //
    sp<SwAudioOutputDescriptor> mOutput1;    // used by duplicated outputs: first output
    sp<SwAudioOutputDescriptor> mOutput2;    // used by duplicated outputs: second output
    uint32_t mDirectOpenCount; // number of clients using this output (direct outputs only)
    audio_session_t mDirectClientSession; // session id of the direct output client
};

// Audio output driven by an input device directly.
class HwAudioOutputDescriptor: public AudioOutputDescriptor
{
public:
    HwAudioOutputDescriptor(const sp<SourceClientDescriptor>& source,
                            AudioPolicyClientInterface *clientInterface);
    virtual ~HwAudioOutputDescriptor() {}

            void dump(String8 *dst) const override;

    virtual bool setVolume(float volumeDb,
                           VolumeSource volumeSource, const StreamTypeVector &streams,
                           audio_devices_t device,
                           uint32_t delayMs,
                           bool force);

    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
                           const struct audio_port_config *srcConfig = NULL) const;
    virtual void toAudioPort(struct audio_port *port) const;

    const sp<SourceClientDescriptor> mSource;

};

class SwAudioOutputCollection :
        public DefaultKeyedVector< audio_io_handle_t, sp<SwAudioOutputDescriptor> >
{
public:
    bool isActive(VolumeSource volumeSource, uint32_t inPastMs = 0) const;

    /**
     * return whether any source contributing to VolumeSource is playing remotely, override
     * to change the definition of
     * local/remote playback, used for instance by notification manager to not make
     * media players lose audio focus when not playing locally
     * For the base implementation, "remotely" means playing during screen mirroring which
     * uses an output for playback with a non-empty, non "0" address.
     */
    bool isActiveRemotely(VolumeSource volumeSource, uint32_t inPastMs = 0) const;

    /**
     * return whether any source contributing to VolumeSource is playing, but not on a "remote"
     * device.
     * Override to change the definition of a local/remote playback.
     * Used for instance by policy manager to alter the speaker playback ("speaker safe" behavior)
     * when media plays or not locally.
     * For the base implementation, "remotely" means playing during screen mirroring.
     */
    bool isActiveLocally(VolumeSource volumeSource, uint32_t inPastMs = 0) const;

    /**
     * @brief isStrategyActiveOnSameModule checks if the given strategy is active (or was active
     * in the past) on the given output and all the outputs belonging to the same HW Module
     * the same module than the given output
     * @param outputDesc to be considered
     * @param ps product strategy to be checked upon activity status
     * @param inPastMs if 0, check currently, otherwise, check in the past
     * @param sysTime shall be set if request is done for the past activity.
     * @return true if an output following the strategy is active on the same module than desc,
     * false otherwise
     */
    bool isStrategyActiveOnSameModule(product_strategy_t ps,
                                      const sp<SwAudioOutputDescriptor>& desc,
                                      uint32_t inPastMs = 0, nsecs_t sysTime = 0) const;

    /**
     * @brief clearSessionRoutesForDevice: when a device is disconnected, and if this device has
     * been chosen as the preferred device by any client, the policy manager shall
     * prevent from using this device any more by clearing all the session routes involving this
     * device.
     * In other words, the preferred device port id of these clients will be resetted to NONE.
     * @param disconnectedDevice device to be disconnected
     */
    void clearSessionRoutesForDevice(const sp<DeviceDescriptor> &disconnectedDevice);

    /**
     * returns the A2DP output handle if it is open or 0 otherwise
     */
    audio_io_handle_t getA2dpOutput() const;

    /**
     * returns true if primary HAL supports A2DP Offload
     */
    bool isA2dpOffloadedOnPrimary() const;

    /**
     * returns true if A2DP is supported (either via hardware offload or software encoding)
     */
    bool isA2dpSupported() const;

    sp<SwAudioOutputDescriptor> getOutputFromId(audio_port_handle_t id) const;

    sp<SwAudioOutputDescriptor> getPrimaryOutput() const;

    /**
     * @brief isAnyOutputActive checks if any output is active (aka playing) except the one(s) that
     * hold the volume source to be ignored
     * @param volumeSourceToIgnore source not to be considered in the activity detection
     * @return true if any output is active for any volume source except the one to be ignored
     */
    bool isAnyOutputActive(VolumeSource volumeSourceToIgnore) const
    {
        for (size_t i = 0; i < size(); i++) {
            const sp<AudioOutputDescriptor> &outputDesc = valueAt(i);
            if (outputDesc->isAnyActive(volumeSourceToIgnore)) {
                return true;
            }
        }
        return false;
    }

    audio_devices_t getSupportedDevices(audio_io_handle_t handle) const;

    sp<SwAudioOutputDescriptor> getOutputForClient(audio_port_handle_t portId);

    void dump(String8 *dst) const;
};

class HwAudioOutputCollection :
        public DefaultKeyedVector< audio_io_handle_t, sp<HwAudioOutputDescriptor> >
{
public:
    bool isActive(VolumeSource volumeSource, uint32_t inPastMs = 0) const;

    /**
     * @brief isAnyOutputActive checks if any output is active (aka playing) except the one(s) that
     * hold the volume source to be ignored
     * @param volumeSourceToIgnore source not to be considered in the activity detection
     * @return true if any output is active for any volume source except the one to be ignored
     */
    bool isAnyOutputActive(VolumeSource volumeSourceToIgnore) const
    {
        for (size_t i = 0; i < size(); i++) {
            const sp<AudioOutputDescriptor> &outputDesc = valueAt(i);
            if (outputDesc->isAnyActive(volumeSourceToIgnore)) {
                return true;
            }
        }
        return false;
    }

    void dump(String8 *dst) const;
};


} // namespace android
