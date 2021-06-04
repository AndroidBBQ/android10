/*
 * Copyright (C) 2009 The Android Open Source Project
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

#define LOG_TAG "AudioPolicyService"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#undef __STRICT_ANSI__
#define __STDINT_LIMITS
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sys/time.h>

#include <audio_utils/clock.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <binder/IPCThreadState.h>
#include <binder/ActivityManager.h>
#include <binder/PermissionController.h>
#include <binder/IResultReceiver.h>
#include <utils/String16.h>
#include <utils/threads.h>
#include "AudioPolicyService.h"
#include <hardware_legacy/power.h>
#include <media/AudioEffect.h>
#include <media/AudioParameter.h>
#include <mediautils/ServiceUtilities.h>
#include <sensorprivacy/SensorPrivacyManager.h>

#include <system/audio.h>
#include <system/audio_policy.h>

namespace android {

static const char kDeadlockedString[] = "AudioPolicyService may be deadlocked\n";
static const char kCmdDeadlockedString[] = "AudioPolicyService command thread may be deadlocked\n";

static const int kDumpLockTimeoutNs = 1 * NANOS_PER_SECOND;

static const nsecs_t kAudioCommandTimeoutNs = seconds(3); // 3 seconds

static const String16 sManageAudioPolicyPermission("android.permission.MANAGE_AUDIO_POLICY");

// ----------------------------------------------------------------------------

AudioPolicyService::AudioPolicyService()
    : BnAudioPolicyService(), mpAudioPolicyDev(NULL), mpAudioPolicy(NULL),
      mAudioPolicyManager(NULL), mAudioPolicyClient(NULL), mPhoneState(AUDIO_MODE_INVALID)
{
}

void AudioPolicyService::onFirstRef()
{
    {
        Mutex::Autolock _l(mLock);

        // start audio commands thread
        mAudioCommandThread = new AudioCommandThread(String8("ApmAudio"), this);
        // start output activity command thread
        mOutputCommandThread = new AudioCommandThread(String8("ApmOutput"), this);

        mAudioPolicyClient = new AudioPolicyClient(this);
        mAudioPolicyManager = createAudioPolicyManager(mAudioPolicyClient);
    }
    // load audio processing modules
    sp<AudioPolicyEffects>audioPolicyEffects = new AudioPolicyEffects();
    {
        Mutex::Autolock _l(mLock);
        mAudioPolicyEffects = audioPolicyEffects;
    }

    mUidPolicy = new UidPolicy(this);
    mUidPolicy->registerSelf();

    mSensorPrivacyPolicy = new SensorPrivacyPolicy(this);
    mSensorPrivacyPolicy->registerSelf();
}

AudioPolicyService::~AudioPolicyService()
{
    mAudioCommandThread->exit();
    mOutputCommandThread->exit();

    destroyAudioPolicyManager(mAudioPolicyManager);
    delete mAudioPolicyClient;

    mNotificationClients.clear();
    mAudioPolicyEffects.clear();

    mUidPolicy->unregisterSelf();
    mUidPolicy.clear();

    mSensorPrivacyPolicy->unregisterSelf();
    mSensorPrivacyPolicy.clear();
}

// A notification client is always registered by AudioSystem when the client process
// connects to AudioPolicyService.
void AudioPolicyService::registerClient(const sp<IAudioPolicyServiceClient>& client)
{
    if (client == 0) {
        ALOGW("%s got NULL client", __FUNCTION__);
        return;
    }
    Mutex::Autolock _l(mNotificationClientsLock);

    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();
    int64_t token = ((int64_t)uid<<32) | pid;

    if (mNotificationClients.indexOfKey(token) < 0) {
        sp<NotificationClient> notificationClient = new NotificationClient(this,
                                                                           client,
                                                                           uid,
                                                                           pid);
        ALOGV("registerClient() client %p, uid %d pid %d", client.get(), uid, pid);

        mNotificationClients.add(token, notificationClient);

        sp<IBinder> binder = IInterface::asBinder(client);
        binder->linkToDeath(notificationClient);
    }
}

void AudioPolicyService::setAudioPortCallbacksEnabled(bool enabled)
{
    Mutex::Autolock _l(mNotificationClientsLock);

    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();
    int64_t token = ((int64_t)uid<<32) | pid;

    if (mNotificationClients.indexOfKey(token) < 0) {
        return;
    }
    mNotificationClients.valueFor(token)->setAudioPortCallbacksEnabled(enabled);
}

void AudioPolicyService::setAudioVolumeGroupCallbacksEnabled(bool enabled)
{
    Mutex::Autolock _l(mNotificationClientsLock);

    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();
    int64_t token = ((int64_t)uid<<32) | pid;

    if (mNotificationClients.indexOfKey(token) < 0) {
        return;
    }
    mNotificationClients.valueFor(token)->setAudioVolumeGroupCallbacksEnabled(enabled);
}

// removeNotificationClient() is called when the client process dies.
void AudioPolicyService::removeNotificationClient(uid_t uid, pid_t pid)
{
    {
        Mutex::Autolock _l(mNotificationClientsLock);
        int64_t token = ((int64_t)uid<<32) | pid;
        mNotificationClients.removeItem(token);
    }
    {
        Mutex::Autolock _l(mLock);
        bool hasSameUid = false;
        for (size_t i = 0; i < mNotificationClients.size(); i++) {
            if (mNotificationClients.valueAt(i)->uid() == uid) {
                hasSameUid = true;
                break;
            }
        }
        if (mAudioPolicyManager && !hasSameUid) {
            // called from binder death notification: no need to clear caller identity
            mAudioPolicyManager->releaseResourcesForUid(uid);
        }
    }
}

void AudioPolicyService::onAudioPortListUpdate()
{
    mOutputCommandThread->updateAudioPortListCommand();
}

void AudioPolicyService::doOnAudioPortListUpdate()
{
    Mutex::Autolock _l(mNotificationClientsLock);
    for (size_t i = 0; i < mNotificationClients.size(); i++) {
        mNotificationClients.valueAt(i)->onAudioPortListUpdate();
    }
}

void AudioPolicyService::onAudioPatchListUpdate()
{
    mOutputCommandThread->updateAudioPatchListCommand();
}

void AudioPolicyService::doOnAudioPatchListUpdate()
{
    Mutex::Autolock _l(mNotificationClientsLock);
    for (size_t i = 0; i < mNotificationClients.size(); i++) {
        mNotificationClients.valueAt(i)->onAudioPatchListUpdate();
    }
}

void AudioPolicyService::onAudioVolumeGroupChanged(volume_group_t group, int flags)
{
    mOutputCommandThread->changeAudioVolumeGroupCommand(group, flags);
}

void AudioPolicyService::doOnAudioVolumeGroupChanged(volume_group_t group, int flags)
{
    Mutex::Autolock _l(mNotificationClientsLock);
    for (size_t i = 0; i < mNotificationClients.size(); i++) {
        mNotificationClients.valueAt(i)->onAudioVolumeGroupChanged(group, flags);
    }
}

void AudioPolicyService::onDynamicPolicyMixStateUpdate(const String8& regId, int32_t state)
{
    ALOGV("AudioPolicyService::onDynamicPolicyMixStateUpdate(%s, %d)",
            regId.string(), state);
    mOutputCommandThread->dynamicPolicyMixStateUpdateCommand(regId, state);
}

void AudioPolicyService::doOnDynamicPolicyMixStateUpdate(const String8& regId, int32_t state)
{
    Mutex::Autolock _l(mNotificationClientsLock);
    for (size_t i = 0; i < mNotificationClients.size(); i++) {
        mNotificationClients.valueAt(i)->onDynamicPolicyMixStateUpdate(regId, state);
    }
}

void AudioPolicyService::onRecordingConfigurationUpdate(
                                                    int event,
                                                    const record_client_info_t *clientInfo,
                                                    const audio_config_base_t *clientConfig,
                                                    std::vector<effect_descriptor_t> clientEffects,
                                                    const audio_config_base_t *deviceConfig,
                                                    std::vector<effect_descriptor_t> effects,
                                                    audio_patch_handle_t patchHandle,
                                                    audio_source_t source)
{
    mOutputCommandThread->recordingConfigurationUpdateCommand(event, clientInfo,
            clientConfig, clientEffects, deviceConfig, effects, patchHandle, source);
}

void AudioPolicyService::doOnRecordingConfigurationUpdate(
                                                  int event,
                                                  const record_client_info_t *clientInfo,
                                                  const audio_config_base_t *clientConfig,
                                                  std::vector<effect_descriptor_t> clientEffects,
                                                  const audio_config_base_t *deviceConfig,
                                                  std::vector<effect_descriptor_t> effects,
                                                  audio_patch_handle_t patchHandle,
                                                  audio_source_t source)
{
    Mutex::Autolock _l(mNotificationClientsLock);
    for (size_t i = 0; i < mNotificationClients.size(); i++) {
        mNotificationClients.valueAt(i)->onRecordingConfigurationUpdate(event, clientInfo,
                clientConfig, clientEffects, deviceConfig, effects, patchHandle, source);
    }
}

status_t AudioPolicyService::clientCreateAudioPatch(const struct audio_patch *patch,
                                                audio_patch_handle_t *handle,
                                                int delayMs)
{
    return mAudioCommandThread->createAudioPatchCommand(patch, handle, delayMs);
}

status_t AudioPolicyService::clientReleaseAudioPatch(audio_patch_handle_t handle,
                                                 int delayMs)
{
    return mAudioCommandThread->releaseAudioPatchCommand(handle, delayMs);
}

status_t AudioPolicyService::clientSetAudioPortConfig(const struct audio_port_config *config,
                                                      int delayMs)
{
    return mAudioCommandThread->setAudioPortConfigCommand(config, delayMs);
}

AudioPolicyService::NotificationClient::NotificationClient(const sp<AudioPolicyService>& service,
                                                     const sp<IAudioPolicyServiceClient>& client,
                                                     uid_t uid,
                                                     pid_t pid)
    : mService(service), mUid(uid), mPid(pid), mAudioPolicyServiceClient(client),
      mAudioPortCallbacksEnabled(false), mAudioVolumeGroupCallbacksEnabled(false)
{
}

AudioPolicyService::NotificationClient::~NotificationClient()
{
}

void AudioPolicyService::NotificationClient::binderDied(const wp<IBinder>& who __unused)
{
    sp<NotificationClient> keep(this);
    sp<AudioPolicyService> service = mService.promote();
    if (service != 0) {
        service->removeNotificationClient(mUid, mPid);
    }
}

void AudioPolicyService::NotificationClient::onAudioPortListUpdate()
{
    if (mAudioPolicyServiceClient != 0 && mAudioPortCallbacksEnabled) {
        mAudioPolicyServiceClient->onAudioPortListUpdate();
    }
}

void AudioPolicyService::NotificationClient::onAudioPatchListUpdate()
{
    if (mAudioPolicyServiceClient != 0 && mAudioPortCallbacksEnabled) {
        mAudioPolicyServiceClient->onAudioPatchListUpdate();
    }
}

void AudioPolicyService::NotificationClient::onAudioVolumeGroupChanged(volume_group_t group, 
                                                                      int flags)
{
    if (mAudioPolicyServiceClient != 0 && mAudioVolumeGroupCallbacksEnabled) {
        mAudioPolicyServiceClient->onAudioVolumeGroupChanged(group, flags);
    }
}


void AudioPolicyService::NotificationClient::onDynamicPolicyMixStateUpdate(
        const String8& regId, int32_t state)
{
    if (mAudioPolicyServiceClient != 0 && isServiceUid(mUid)) {
        mAudioPolicyServiceClient->onDynamicPolicyMixStateUpdate(regId, state);
    }
}

void AudioPolicyService::NotificationClient::onRecordingConfigurationUpdate(
                                            int event,
                                            const record_client_info_t *clientInfo,
                                            const audio_config_base_t *clientConfig,
                                            std::vector<effect_descriptor_t> clientEffects,
                                            const audio_config_base_t *deviceConfig,
                                            std::vector<effect_descriptor_t> effects,
                                            audio_patch_handle_t patchHandle,
                                            audio_source_t source)
{
    if (mAudioPolicyServiceClient != 0 && isServiceUid(mUid)) {
        mAudioPolicyServiceClient->onRecordingConfigurationUpdate(event, clientInfo,
                clientConfig, clientEffects, deviceConfig, effects, patchHandle, source);
    }
}

void AudioPolicyService::NotificationClient::setAudioPortCallbacksEnabled(bool enabled)
{
    mAudioPortCallbacksEnabled = enabled;
}

void AudioPolicyService::NotificationClient::setAudioVolumeGroupCallbacksEnabled(bool enabled)
{
    mAudioVolumeGroupCallbacksEnabled = enabled;
}

void AudioPolicyService::binderDied(const wp<IBinder>& who) {
    ALOGW("binderDied() %p, calling pid %d", who.unsafe_get(),
            IPCThreadState::self()->getCallingPid());
}

static bool dumpTryLock(Mutex& mutex)
{
    status_t err = mutex.timedLock(kDumpLockTimeoutNs);
    return err == NO_ERROR;
}

status_t AudioPolicyService::dumpInternals(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "AudioPolicyManager: %p\n", mAudioPolicyManager);
    result.append(buffer);
    snprintf(buffer, SIZE, "Command Thread: %p\n", mAudioCommandThread.get());
    result.append(buffer);

    write(fd, result.string(), result.size());
    return NO_ERROR;
}

void AudioPolicyService::updateUidStates()
{
    Mutex::Autolock _l(mLock);
    updateUidStates_l();
}

void AudioPolicyService::updateUidStates_l()
{
//    Go over all active clients and allow capture (does not force silence) in the
//    following cases:
//    Another client in the same UID has already been allowed to capture
//    OR The client is the assistant
//        AND an accessibility service is on TOP or a RTT call is active
//                AND the source is VOICE_RECOGNITION or HOTWORD
//            OR uses VOICE_RECOGNITION AND is on TOP
//                OR uses HOTWORD
//            AND there is no active privacy sensitive capture or call
//                OR client has CAPTURE_AUDIO_OUTPUT privileged permission
//    OR The client is an accessibility service
//        AND Is on TOP
//                AND the source is VOICE_RECOGNITION or HOTWORD
//            OR The assistant is not on TOP
//                AND there is no active privacy sensitive capture or call
//                    OR client has CAPTURE_AUDIO_OUTPUT privileged permission
//        AND is on TOP
//        AND the source is VOICE_RECOGNITION or HOTWORD
//    OR the client source is virtual (remote submix, call audio TX or RX...)
//    OR Any client
//        AND The assistant is not on TOP
//        AND is on TOP or latest started
//        AND there is no active privacy sensitive capture or call
//            OR client has CAPTURE_AUDIO_OUTPUT privileged permission

    sp<AudioRecordClient> topActive;
    sp<AudioRecordClient> latestActive;
    sp<AudioRecordClient> latestSensitiveActive;

    nsecs_t topStartNs = 0;
    nsecs_t latestStartNs = 0;
    nsecs_t latestSensitiveStartNs = 0;
    bool isA11yOnTop = mUidPolicy->isA11yOnTop();
    bool isAssistantOnTop = false;
    bool isSensitiveActive = false;
    bool isInCall = mPhoneState == AUDIO_MODE_IN_CALL;
    bool rttCallActive =
            (mPhoneState == AUDIO_MODE_IN_CALL || mPhoneState == AUDIO_MODE_IN_COMMUNICATION)
            && mUidPolicy->isRttEnabled();

    // if Sensor Privacy is enabled then all recordings should be silenced.
    if (mSensorPrivacyPolicy->isSensorPrivacyEnabled()) {
        silenceAllRecordings_l();
        return;
    }

    for (size_t i =0; i < mAudioRecordClients.size(); i++) {
        sp<AudioRecordClient> current = mAudioRecordClients[i];
        if (!current->active) {
            continue;
        }

        app_state_t appState = apmStatFromAmState(mUidPolicy->getUidState(current->uid));
        // clients which app is in IDLE state are not eligible for top active or
        // latest active
        if (appState == APP_STATE_IDLE) {
            continue;
        }

        bool isAssistant = mUidPolicy->isAssistantUid(current->uid);
        bool isAccessibility = mUidPolicy->isA11yUid(current->uid);
        if (appState == APP_STATE_TOP && !isAccessibility) {
            if (current->startTimeNs > topStartNs) {
                topActive = current;
                topStartNs = current->startTimeNs;
            }
            if (isAssistant) {
                isAssistantOnTop = true;
            }
        }
        // Assistant capturing for HOTWORD or Accessibility services not considered
        // for latest active to avoid masking regular clients started before
        if (current->startTimeNs > latestStartNs
                && !((current->attributes.source == AUDIO_SOURCE_HOTWORD
                        || isA11yOnTop || rttCallActive)
                    && isAssistant)
                && !isAccessibility) {
            latestActive = current;
            latestStartNs = current->startTimeNs;
        }
        if (isPrivacySensitiveSource(current->attributes.source)) {
            if (current->startTimeNs > latestSensitiveStartNs) {
                latestSensitiveActive = current;
                latestSensitiveStartNs = current->startTimeNs;
            }
            isSensitiveActive = true;
        }
    }

    // if no active client with UI on Top, consider latest active as top
    if (topActive == nullptr) {
        topActive = latestActive;
    }

    std::vector<uid_t> enabledUids;

    for (size_t i =0; i < mAudioRecordClients.size(); i++) {
        sp<AudioRecordClient> current = mAudioRecordClients[i];
        if (!current->active) {
            continue;
        }

        // keep capture allowed if another client with the same UID has already
        // been allowed to capture
        if (std::find(enabledUids.begin(), enabledUids.end(), current->uid)
                != enabledUids.end()) {
            continue;
        }

        audio_source_t source = current->attributes.source;
        bool isTopOrLatestActive = topActive == nullptr ? false : current->uid == topActive->uid;
        bool isLatestSensitive = latestSensitiveActive == nullptr ?
                                 false : current->uid == latestSensitiveActive->uid;

        // By default allow capture if:
        //     The assistant is not on TOP
        //     AND is on TOP or latest started
        //     AND there is no active privacy sensitive capture or call
        //             OR client has CAPTURE_AUDIO_OUTPUT privileged permission
        bool allowCapture = !isAssistantOnTop
                && ((isTopOrLatestActive && !isLatestSensitive) || isLatestSensitive)
                && !(isSensitiveActive && !(isLatestSensitive || current->canCaptureOutput))
                && !(isInCall && !current->canCaptureOutput);

        if (isVirtualSource(source)) {
            // Allow capture for virtual (remote submix, call audio TX or RX...) sources
            allowCapture = true;
        } else if (mUidPolicy->isAssistantUid(current->uid)) {
            // For assistant allow capture if:
            //     An accessibility service is on TOP or a RTT call is active
            //            AND the source is VOICE_RECOGNITION or HOTWORD
            //     OR is on TOP AND uses VOICE_RECOGNITION
            //            OR uses HOTWORD
            //         AND there is no active privacy sensitive capture or call
            //             OR client has CAPTURE_AUDIO_OUTPUT privileged permission
            if (isA11yOnTop || rttCallActive) {
                if (source == AUDIO_SOURCE_HOTWORD || source == AUDIO_SOURCE_VOICE_RECOGNITION) {
                    allowCapture = true;
                }
            } else {
                if (((isAssistantOnTop && source == AUDIO_SOURCE_VOICE_RECOGNITION) ||
                        source == AUDIO_SOURCE_HOTWORD) &&
                        (!(isSensitiveActive || isInCall) || current->canCaptureOutput)) {
                    allowCapture = true;
                }
            }
        } else if (mUidPolicy->isA11yUid(current->uid)) {
            // For accessibility service allow capture if:
            //     Is on TOP
            //          AND the source is VOICE_RECOGNITION or HOTWORD
            //     Or
            //          The assistant is not on TOP
            //          AND there is no active privacy sensitive capture or call
            //             OR client has CAPTURE_AUDIO_OUTPUT privileged permission
            if (isA11yOnTop) {
                if (source == AUDIO_SOURCE_VOICE_RECOGNITION || source == AUDIO_SOURCE_HOTWORD) {
                    allowCapture = true;
                }
            } else {
                if (!isAssistantOnTop
                        && (!(isSensitiveActive || isInCall) || current->canCaptureOutput)) {
                    allowCapture = true;
                }
            }
        }
        setAppState_l(current->uid,
                      allowCapture ? apmStatFromAmState(mUidPolicy->getUidState(current->uid)) :
                                APP_STATE_IDLE);
        if (allowCapture) {
            enabledUids.push_back(current->uid);
        }
    }
}

void AudioPolicyService::silenceAllRecordings_l() {
    for (size_t i = 0; i < mAudioRecordClients.size(); i++) {
        sp<AudioRecordClient> current = mAudioRecordClients[i];
        if (!isVirtualSource(current->attributes.source)) {
            setAppState_l(current->uid, APP_STATE_IDLE);
        }
    }
}

/* static */
app_state_t AudioPolicyService::apmStatFromAmState(int amState) {

    if (amState == ActivityManager::PROCESS_STATE_UNKNOWN) {
        return APP_STATE_IDLE;
    } else if (amState <= ActivityManager::PROCESS_STATE_TOP) {
      // include persistent services
      return APP_STATE_TOP;
    }
    return APP_STATE_FOREGROUND;
}

/* static */
bool AudioPolicyService::isPrivacySensitiveSource(audio_source_t source)
{
    switch (source) {
        case AUDIO_SOURCE_CAMCORDER:
        case AUDIO_SOURCE_VOICE_COMMUNICATION:
            return true;
        default:
            break;
    }
    return false;
}

/* static */
bool AudioPolicyService::isVirtualSource(audio_source_t source)
{
    switch (source) {
        case AUDIO_SOURCE_VOICE_UPLINK:
        case AUDIO_SOURCE_VOICE_DOWNLINK:
        case AUDIO_SOURCE_VOICE_CALL:
        case AUDIO_SOURCE_REMOTE_SUBMIX:
        case AUDIO_SOURCE_FM_TUNER:
            return true;
        default:
            break;
    }
    return false;
}

void AudioPolicyService::setAppState_l(uid_t uid, app_state_t state)
{
    AutoCallerClear acc;

    if (mAudioPolicyManager) {
        mAudioPolicyManager->setAppState(uid, state);
    }
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af) {
        bool silenced = state == APP_STATE_IDLE;
        af->setRecordSilenced(uid, silenced);
    }
}

status_t AudioPolicyService::dump(int fd, const Vector<String16>& args __unused)
{
    if (!dumpAllowed()) {
        dumpPermissionDenial(fd);
    } else {
        bool locked = dumpTryLock(mLock);
        if (!locked) {
            String8 result(kDeadlockedString);
            write(fd, result.string(), result.size());
        }

        dumpInternals(fd);
        if (mAudioCommandThread != 0) {
            mAudioCommandThread->dump(fd);
        }

        if (mAudioPolicyManager) {
            mAudioPolicyManager->dump(fd);
        }

        mPackageManager.dump(fd);

        if (locked) mLock.unlock();
    }
    return NO_ERROR;
}

status_t AudioPolicyService::dumpPermissionDenial(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "Permission Denial: "
            "can't dump AudioPolicyService from pid=%d, uid=%d\n",
            IPCThreadState::self()->getCallingPid(),
            IPCThreadState::self()->getCallingUid());
    result.append(buffer);
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioPolicyService::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    switch (code) {
        case SHELL_COMMAND_TRANSACTION: {
            int in = data.readFileDescriptor();
            int out = data.readFileDescriptor();
            int err = data.readFileDescriptor();
            int argc = data.readInt32();
            Vector<String16> args;
            for (int i = 0; i < argc && data.dataAvail() > 0; i++) {
               args.add(data.readString16());
            }
            sp<IBinder> unusedCallback;
            sp<IResultReceiver> resultReceiver;
            status_t status;
            if ((status = data.readNullableStrongBinder(&unusedCallback)) != NO_ERROR) {
                return status;
            }
            if ((status = data.readNullableStrongBinder(&resultReceiver)) != NO_ERROR) {
                return status;
            }
            status = shellCommand(in, out, err, args);
            if (resultReceiver != nullptr) {
                resultReceiver->send(status);
            }
            return NO_ERROR;
        }
    }

    return BnAudioPolicyService::onTransact(code, data, reply, flags);
}

// ------------------- Shell command implementation -------------------

// NOTE: This is a remote API - make sure all args are validated
status_t AudioPolicyService::shellCommand(int in, int out, int err, Vector<String16>& args) {
    if (!checkCallingPermission(sManageAudioPolicyPermission, nullptr, nullptr)) {
        return PERMISSION_DENIED;
    }
    if (in == BAD_TYPE || out == BAD_TYPE || err == BAD_TYPE) {
        return BAD_VALUE;
    }
    if (args.size() >= 3 && args[0] == String16("set-uid-state")) {
        return handleSetUidState(args, err);
    } else if (args.size() >= 2 && args[0] == String16("reset-uid-state")) {
        return handleResetUidState(args, err);
    } else if (args.size() >= 2 && args[0] == String16("get-uid-state")) {
        return handleGetUidState(args, out, err);
    } else if (args.size() == 1 && args[0] == String16("help")) {
        printHelp(out);
        return NO_ERROR;
    }
    printHelp(err);
    return BAD_VALUE;
}

static status_t getUidForPackage(String16 packageName, int userId, /*inout*/uid_t& uid, int err) {
    if (userId < 0) {
        ALOGE("Invalid user: %d", userId);
        dprintf(err, "Invalid user: %d\n", userId);
        return BAD_VALUE;
    }

    PermissionController pc;
    uid = pc.getPackageUid(packageName, 0);
    if (uid <= 0) {
        ALOGE("Unknown package: '%s'", String8(packageName).string());
        dprintf(err, "Unknown package: '%s'\n", String8(packageName).string());
        return BAD_VALUE;
    }

    uid = multiuser_get_uid(userId, uid);
    return NO_ERROR;
}

status_t AudioPolicyService::handleSetUidState(Vector<String16>& args, int err) {
    // Valid arg.size() is 3 or 5, args.size() is 5 with --user option.
    if (!(args.size() == 3 || args.size() == 5)) {
        printHelp(err);
        return BAD_VALUE;
    }

    bool active = false;
    if (args[2] == String16("active")) {
        active = true;
    } else if ((args[2] != String16("idle"))) {
        ALOGE("Expected active or idle but got: '%s'", String8(args[2]).string());
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() >= 5 && args[3] == String16("--user")) {
        userId = atoi(String8(args[4]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) == BAD_VALUE) {
        return BAD_VALUE;
    }

    mUidPolicy->addOverrideUid(uid, active);
    return NO_ERROR;
}

status_t AudioPolicyService::handleResetUidState(Vector<String16>& args, int err) {
    // Valid arg.size() is 2 or 4, args.size() is 4 with --user option.
    if (!(args.size() == 2 || args.size() == 4)) {
        printHelp(err);
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() >= 4 && args[2] == String16("--user")) {
        userId = atoi(String8(args[3]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) == BAD_VALUE) {
        return BAD_VALUE;
    }

    mUidPolicy->removeOverrideUid(uid);
    return NO_ERROR;
}

status_t AudioPolicyService::handleGetUidState(Vector<String16>& args, int out, int err) {
    // Valid arg.size() is 2 or 4, args.size() is 4 with --user option.
    if (!(args.size() == 2 || args.size() == 4)) {
        printHelp(err);
        return BAD_VALUE;
    }

    int userId = 0;
    if (args.size() >= 4 && args[2] == String16("--user")) {
        userId = atoi(String8(args[3]));
    }

    uid_t uid;
    if (getUidForPackage(args[1], userId, uid, err) == BAD_VALUE) {
        return BAD_VALUE;
    }

    if (mUidPolicy->isUidActive(uid)) {
        return dprintf(out, "active\n");
    } else {
        return dprintf(out, "idle\n");
    }
}

status_t AudioPolicyService::printHelp(int out) {
    return dprintf(out, "Audio policy service commands:\n"
        "  get-uid-state <PACKAGE> [--user USER_ID] gets the uid state\n"
        "  set-uid-state <PACKAGE> <active|idle> [--user USER_ID] overrides the uid state\n"
        "  reset-uid-state <PACKAGE> [--user USER_ID] clears the uid state override\n"
        "  help print this message\n");
}

// -----------  AudioPolicyService::UidPolicy implementation ----------

void AudioPolicyService::UidPolicy::registerSelf() {
    ActivityManager am;
    am.registerUidObserver(this, ActivityManager::UID_OBSERVER_GONE
            | ActivityManager::UID_OBSERVER_IDLE
            | ActivityManager::UID_OBSERVER_ACTIVE
            | ActivityManager::UID_OBSERVER_PROCSTATE,
            ActivityManager::PROCESS_STATE_UNKNOWN,
            String16("audioserver"));
    status_t res = am.linkToDeath(this);
    if (!res) {
        Mutex::Autolock _l(mLock);
        mObserverRegistered = true;
    } else {
        ALOGE("UidPolicy::registerSelf linkToDeath failed: %d", res);

        am.unregisterUidObserver(this);
    }
}

void AudioPolicyService::UidPolicy::unregisterSelf() {
    ActivityManager am;
    am.unlinkToDeath(this);
    am.unregisterUidObserver(this);
    Mutex::Autolock _l(mLock);
    mObserverRegistered = false;
}

void AudioPolicyService::UidPolicy::binderDied(__unused const wp<IBinder> &who) {
    Mutex::Autolock _l(mLock);
    mCachedUids.clear();
    mObserverRegistered = false;
}

void AudioPolicyService::UidPolicy::checkRegistered() {
    bool needToReregister = false;
    {
        Mutex::Autolock _l(mLock);
        needToReregister = !mObserverRegistered;
    }
    if (needToReregister) {
        // Looks like ActivityManager has died previously, attempt to re-register.
        registerSelf();
    }
}

bool AudioPolicyService::UidPolicy::isUidActive(uid_t uid) {
    if (isServiceUid(uid)) return true;
    checkRegistered();
    {
        Mutex::Autolock _l(mLock);
        auto overrideIter = mOverrideUids.find(uid);
        if (overrideIter != mOverrideUids.end()) {
            return overrideIter->second.first;
        }
        // In an absense of the ActivityManager, assume everything to be active.
        if (!mObserverRegistered) return true;
        auto cacheIter = mCachedUids.find(uid);
        if (cacheIter != mCachedUids.end()) {
            return cacheIter->second.first;
        }
    }
    ActivityManager am;
    bool active = am.isUidActive(uid, String16("audioserver"));
    {
        Mutex::Autolock _l(mLock);
        mCachedUids.insert(std::pair<uid_t,
                           std::pair<bool, int>>(uid, std::pair<bool, int>(active,
                                                      ActivityManager::PROCESS_STATE_UNKNOWN)));
    }
    return active;
}

int AudioPolicyService::UidPolicy::getUidState(uid_t uid) {
    if (isServiceUid(uid)) {
        return ActivityManager::PROCESS_STATE_TOP;
    }
    checkRegistered();
    {
        Mutex::Autolock _l(mLock);
        auto overrideIter = mOverrideUids.find(uid);
        if (overrideIter != mOverrideUids.end()) {
            if (overrideIter->second.first) {
                if (overrideIter->second.second != ActivityManager::PROCESS_STATE_UNKNOWN) {
                    return overrideIter->second.second;
                } else {
                    auto cacheIter = mCachedUids.find(uid);
                    if (cacheIter != mCachedUids.end()) {
                        return cacheIter->second.second;
                    }
                }
            }
            return ActivityManager::PROCESS_STATE_UNKNOWN;
        }
        // In an absense of the ActivityManager, assume everything to be active.
        if (!mObserverRegistered) {
            return ActivityManager::PROCESS_STATE_TOP;
        }
        auto cacheIter = mCachedUids.find(uid);
        if (cacheIter != mCachedUids.end()) {
            if (cacheIter->second.first) {
                return cacheIter->second.second;
            } else {
                return ActivityManager::PROCESS_STATE_UNKNOWN;
            }
        }
    }
    ActivityManager am;
    bool active = am.isUidActive(uid, String16("audioserver"));
    int state = ActivityManager::PROCESS_STATE_UNKNOWN;
    if (active) {
        state = am.getUidProcessState(uid, String16("audioserver"));
    }
    {
        Mutex::Autolock _l(mLock);
        mCachedUids.insert(std::pair<uid_t,
                           std::pair<bool, int>>(uid, std::pair<bool, int>(active, state)));
    }

    return state;
}

void AudioPolicyService::UidPolicy::onUidActive(uid_t uid) {
    updateUid(&mCachedUids, uid, true, ActivityManager::PROCESS_STATE_UNKNOWN, true);
}

void AudioPolicyService::UidPolicy::onUidGone(uid_t uid, __unused bool disabled) {
    updateUid(&mCachedUids, uid, false, ActivityManager::PROCESS_STATE_UNKNOWN, false);
}

void AudioPolicyService::UidPolicy::onUidIdle(uid_t uid, __unused bool disabled) {
    updateUid(&mCachedUids, uid, false, ActivityManager::PROCESS_STATE_UNKNOWN, true);
}

void AudioPolicyService::UidPolicy::onUidStateChanged(uid_t uid,
                                                      int32_t procState,
                                                      int64_t procStateSeq __unused) {
    if (procState != ActivityManager::PROCESS_STATE_UNKNOWN) {
        updateUid(&mCachedUids, uid, true, procState, true);
    }
}

void AudioPolicyService::UidPolicy::updateOverrideUid(uid_t uid, bool active, bool insert) {
    updateUid(&mOverrideUids, uid, active, ActivityManager::PROCESS_STATE_UNKNOWN, insert);
}

void AudioPolicyService::UidPolicy::notifyService() {
    sp<AudioPolicyService> service = mService.promote();
    if (service != nullptr) {
        service->updateUidStates();
    }
}

void AudioPolicyService::UidPolicy::updateUid(std::unordered_map<uid_t,
                                              std::pair<bool, int>> *uids,
                                              uid_t uid,
                                              bool active,
                                              int state,
                                              bool insert) {
    if (isServiceUid(uid)) {
        return;
    }
    bool wasActive = isUidActive(uid);
    int previousState = getUidState(uid);
    {
        Mutex::Autolock _l(mLock);
        updateUidLocked(uids, uid, active, state, insert);
    }
    if (wasActive != isUidActive(uid) || state != previousState) {
        notifyService();
    }
}

void AudioPolicyService::UidPolicy::updateUidLocked(std::unordered_map<uid_t,
                                                    std::pair<bool, int>> *uids,
                                                    uid_t uid,
                                                    bool active,
                                                    int state,
                                                    bool insert) {
    auto it = uids->find(uid);
    if (it != uids->end()) {
        if (insert) {
            if (state == ActivityManager::PROCESS_STATE_UNKNOWN) {
                it->second.first = active;
            }
            if (it->second.first) {
                it->second.second = state;
            } else {
                it->second.second = ActivityManager::PROCESS_STATE_UNKNOWN;
            }
        } else {
            uids->erase(it);
        }
    } else if (insert && (state == ActivityManager::PROCESS_STATE_UNKNOWN)) {
        uids->insert(std::pair<uid_t, std::pair<bool, int>>(uid,
                                      std::pair<bool, int>(active, state)));
    }
}

bool AudioPolicyService::UidPolicy::isA11yOnTop() {
    for (const auto &uid : mCachedUids) {
        std::vector<uid_t>::iterator it = find(mA11yUids.begin(), mA11yUids.end(), uid.first);
        if (it == mA11yUids.end()) {
            continue;
        }
        if (uid.second.second >= ActivityManager::PROCESS_STATE_TOP
                && uid.second.second <= ActivityManager::PROCESS_STATE_BOUND_FOREGROUND_SERVICE) {
            return true;
        }
    }
    return false;
}

bool AudioPolicyService::UidPolicy::isA11yUid(uid_t uid)
{
    std::vector<uid_t>::iterator it = find(mA11yUids.begin(), mA11yUids.end(), uid);
    return it != mA11yUids.end();
}

// -----------  AudioPolicyService::SensorPrivacyService implementation ----------
void AudioPolicyService::SensorPrivacyPolicy::registerSelf() {
    SensorPrivacyManager spm;
    mSensorPrivacyEnabled = spm.isSensorPrivacyEnabled();
    spm.addSensorPrivacyListener(this);
}

void AudioPolicyService::SensorPrivacyPolicy::unregisterSelf() {
    SensorPrivacyManager spm;
    spm.removeSensorPrivacyListener(this);
}

bool AudioPolicyService::SensorPrivacyPolicy::isSensorPrivacyEnabled() {
    return mSensorPrivacyEnabled;
}

binder::Status AudioPolicyService::SensorPrivacyPolicy::onSensorPrivacyChanged(bool enabled) {
    mSensorPrivacyEnabled = enabled;
    sp<AudioPolicyService> service = mService.promote();
    if (service != nullptr) {
        service->updateUidStates();
    }
    return binder::Status::ok();
}

// -----------  AudioPolicyService::AudioCommandThread implementation ----------

AudioPolicyService::AudioCommandThread::AudioCommandThread(String8 name,
                                                           const wp<AudioPolicyService>& service)
    : Thread(false), mName(name), mService(service)
{
}


AudioPolicyService::AudioCommandThread::~AudioCommandThread()
{
    if (!mAudioCommands.isEmpty()) {
        release_wake_lock(mName.string());
    }
    mAudioCommands.clear();
}

void AudioPolicyService::AudioCommandThread::onFirstRef()
{
    run(mName.string(), ANDROID_PRIORITY_AUDIO);
}

bool AudioPolicyService::AudioCommandThread::threadLoop()
{
    nsecs_t waitTime = -1;

    mLock.lock();
    while (!exitPending())
    {
        sp<AudioPolicyService> svc;
        while (!mAudioCommands.isEmpty() && !exitPending()) {
            nsecs_t curTime = systemTime();
            // commands are sorted by increasing time stamp: execute them from index 0 and up
            if (mAudioCommands[0]->mTime <= curTime) {
                sp<AudioCommand> command = mAudioCommands[0];
                mAudioCommands.removeAt(0);
                mLastCommand = command;

                switch (command->mCommand) {
                case SET_VOLUME: {
                    VolumeData *data = (VolumeData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set volume stream %d, \
                            volume %f, output %d", data->mStream, data->mVolume, data->mIO);
                    mLock.unlock();
                    command->mStatus = AudioSystem::setStreamVolume(data->mStream,
                                                                    data->mVolume,
                                                                    data->mIO);
                    mLock.lock();
                    }break;
                case SET_PARAMETERS: {
                    ParametersData *data = (ParametersData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set parameters string %s, io %d",
                            data->mKeyValuePairs.string(), data->mIO);
                    mLock.unlock();
                    command->mStatus = AudioSystem::setParameters(data->mIO, data->mKeyValuePairs);
                    mLock.lock();
                    }break;
                case SET_VOICE_VOLUME: {
                    VoiceVolumeData *data = (VoiceVolumeData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set voice volume volume %f",
                            data->mVolume);
                    mLock.unlock();
                    command->mStatus = AudioSystem::setVoiceVolume(data->mVolume);
                    mLock.lock();
                    }break;
                case STOP_OUTPUT: {
                    StopOutputData *data = (StopOutputData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing stop output portId %d",
                            data->mPortId);
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doStopOutput(data->mPortId);
                    mLock.lock();
                    }break;
                case RELEASE_OUTPUT: {
                    ReleaseOutputData *data = (ReleaseOutputData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing release output portId %d",
                            data->mPortId);
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doReleaseOutput(data->mPortId);
                    mLock.lock();
                    }break;
                case CREATE_AUDIO_PATCH: {
                    CreateAudioPatchData *data = (CreateAudioPatchData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing create audio patch");
                    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
                    if (af == 0) {
                        command->mStatus = PERMISSION_DENIED;
                    } else {
                        mLock.unlock();
                        command->mStatus = af->createAudioPatch(&data->mPatch, &data->mHandle);
                        mLock.lock();
                    }
                    } break;
                case RELEASE_AUDIO_PATCH: {
                    ReleaseAudioPatchData *data = (ReleaseAudioPatchData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing release audio patch");
                    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
                    if (af == 0) {
                        command->mStatus = PERMISSION_DENIED;
                    } else {
                        mLock.unlock();
                        command->mStatus = af->releaseAudioPatch(data->mHandle);
                        mLock.lock();
                    }
                    } break;
                case UPDATE_AUDIOPORT_LIST: {
                    ALOGV("AudioCommandThread() processing update audio port list");
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doOnAudioPortListUpdate();
                    mLock.lock();
                    }break;
                case UPDATE_AUDIOPATCH_LIST: {
                    ALOGV("AudioCommandThread() processing update audio patch list");
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doOnAudioPatchListUpdate();
                    mLock.lock();
                    }break;
                case CHANGED_AUDIOVOLUMEGROUP: {
                    AudioVolumeGroupData *data =
                            static_cast<AudioVolumeGroupData *>(command->mParam.get());
                    ALOGV("AudioCommandThread() processing update audio volume group");
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doOnAudioVolumeGroupChanged(data->mGroup, data->mFlags);
                    mLock.lock();
                    }break;
                case SET_AUDIOPORT_CONFIG: {
                    SetAudioPortConfigData *data = (SetAudioPortConfigData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set port config");
                    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
                    if (af == 0) {
                        command->mStatus = PERMISSION_DENIED;
                    } else {
                        mLock.unlock();
                        command->mStatus = af->setAudioPortConfig(&data->mConfig);
                        mLock.lock();
                    }
                    } break;
                case DYN_POLICY_MIX_STATE_UPDATE: {
                    DynPolicyMixStateUpdateData *data =
                            (DynPolicyMixStateUpdateData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing dyn policy mix state update %s %d",
                            data->mRegId.string(), data->mState);
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doOnDynamicPolicyMixStateUpdate(data->mRegId, data->mState);
                    mLock.lock();
                    } break;
                case RECORDING_CONFIGURATION_UPDATE: {
                    RecordingConfigurationUpdateData *data =
                            (RecordingConfigurationUpdateData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing recording configuration update");
                    svc = mService.promote();
                    if (svc == 0) {
                        break;
                    }
                    mLock.unlock();
                    svc->doOnRecordingConfigurationUpdate(data->mEvent, &data->mClientInfo,
                            &data->mClientConfig, data->mClientEffects,
                            &data->mDeviceConfig, data->mEffects,
                            data->mPatchHandle, data->mSource);
                    mLock.lock();
                    } break;
                case SET_EFFECT_SUSPENDED: {
                    SetEffectSuspendedData *data = (SetEffectSuspendedData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set effect suspended");
                    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
                    if (af != 0) {
                        mLock.unlock();
                        af->setEffectSuspended(data->mEffectId, data->mSessionId, data->mSuspended);
                        mLock.lock();
                    }
                    } break;

                default:
                    ALOGW("AudioCommandThread() unknown command %d", command->mCommand);
                }
                {
                    Mutex::Autolock _l(command->mLock);
                    if (command->mWaitStatus) {
                        command->mWaitStatus = false;
                        command->mCond.signal();
                    }
                }
                waitTime = -1;
                // release mLock before releasing strong reference on the service as
                // AudioPolicyService destructor calls AudioCommandThread::exit() which
                // acquires mLock.
                mLock.unlock();
                svc.clear();
                mLock.lock();
            } else {
                waitTime = mAudioCommands[0]->mTime - curTime;
                break;
            }
        }

        // release delayed commands wake lock if the queue is empty
        if (mAudioCommands.isEmpty()) {
            release_wake_lock(mName.string());
        }

        // At this stage we have either an empty command queue or the first command in the queue
        // has a finite delay. So unless we are exiting it is safe to wait.
        if (!exitPending()) {
            ALOGV("AudioCommandThread() going to sleep");
            if (waitTime == -1) {
                mWaitWorkCV.wait(mLock);
            } else {
                mWaitWorkCV.waitRelative(mLock, waitTime);
            }
        }
    }
    // release delayed commands wake lock before quitting
    if (!mAudioCommands.isEmpty()) {
        release_wake_lock(mName.string());
    }
    mLock.unlock();
    return false;
}

status_t AudioPolicyService::AudioCommandThread::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "AudioCommandThread %p Dump\n", this);
    result.append(buffer);
    write(fd, result.string(), result.size());

    bool locked = dumpTryLock(mLock);
    if (!locked) {
        String8 result2(kCmdDeadlockedString);
        write(fd, result2.string(), result2.size());
    }

    snprintf(buffer, SIZE, "- Commands:\n");
    result = String8(buffer);
    result.append("   Command Time        Wait pParam\n");
    for (size_t i = 0; i < mAudioCommands.size(); i++) {
        mAudioCommands[i]->dump(buffer, SIZE);
        result.append(buffer);
    }
    result.append("  Last Command\n");
    if (mLastCommand != 0) {
        mLastCommand->dump(buffer, SIZE);
        result.append(buffer);
    } else {
        result.append("     none\n");
    }

    write(fd, result.string(), result.size());

    if (locked) mLock.unlock();

    return NO_ERROR;
}

status_t AudioPolicyService::AudioCommandThread::volumeCommand(audio_stream_type_t stream,
                                                               float volume,
                                                               audio_io_handle_t output,
                                                               int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_VOLUME;
    sp<VolumeData> data = new VolumeData();
    data->mStream = stream;
    data->mVolume = volume;
    data->mIO = output;
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding set volume stream %d, volume %f, output %d",
            stream, volume, output);
    return sendCommand(command, delayMs);
}

status_t AudioPolicyService::AudioCommandThread::parametersCommand(audio_io_handle_t ioHandle,
                                                                   const char *keyValuePairs,
                                                                   int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_PARAMETERS;
    sp<ParametersData> data = new ParametersData();
    data->mIO = ioHandle;
    data->mKeyValuePairs = String8(keyValuePairs);
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding set parameter string %s, io %d ,delay %d",
            keyValuePairs, ioHandle, delayMs);
    return sendCommand(command, delayMs);
}

status_t AudioPolicyService::AudioCommandThread::voiceVolumeCommand(float volume, int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_VOICE_VOLUME;
    sp<VoiceVolumeData> data = new VoiceVolumeData();
    data->mVolume = volume;
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding set voice volume volume %f", volume);
    return sendCommand(command, delayMs);
}

void AudioPolicyService::AudioCommandThread::setEffectSuspendedCommand(int effectId,
                                                                       audio_session_t sessionId,
                                                                       bool suspended)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_EFFECT_SUSPENDED;
    sp<SetEffectSuspendedData> data = new SetEffectSuspendedData();
    data->mEffectId = effectId;
    data->mSessionId = sessionId;
    data->mSuspended = suspended;
    command->mParam = data;
    ALOGV("AudioCommandThread() adding set suspended effectId %d sessionId %d suspended %d",
        effectId, sessionId, suspended);
    sendCommand(command);
}


void AudioPolicyService::AudioCommandThread::stopOutputCommand(audio_port_handle_t portId)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = STOP_OUTPUT;
    sp<StopOutputData> data = new StopOutputData();
    data->mPortId = portId;
    command->mParam = data;
    ALOGV("AudioCommandThread() adding stop output portId %d", portId);
    sendCommand(command);
}

void AudioPolicyService::AudioCommandThread::releaseOutputCommand(audio_port_handle_t portId)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = RELEASE_OUTPUT;
    sp<ReleaseOutputData> data = new ReleaseOutputData();
    data->mPortId = portId;
    command->mParam = data;
    ALOGV("AudioCommandThread() adding release output portId %d", portId);
    sendCommand(command);
}

status_t AudioPolicyService::AudioCommandThread::createAudioPatchCommand(
                                                const struct audio_patch *patch,
                                                audio_patch_handle_t *handle,
                                                int delayMs)
{
    status_t status = NO_ERROR;

    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = CREATE_AUDIO_PATCH;
    CreateAudioPatchData *data = new CreateAudioPatchData();
    data->mPatch = *patch;
    data->mHandle = *handle;
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding create patch delay %d", delayMs);
    status = sendCommand(command, delayMs);
    if (status == NO_ERROR) {
        *handle = data->mHandle;
    }
    return status;
}

status_t AudioPolicyService::AudioCommandThread::releaseAudioPatchCommand(audio_patch_handle_t handle,
                                                 int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = RELEASE_AUDIO_PATCH;
    ReleaseAudioPatchData *data = new ReleaseAudioPatchData();
    data->mHandle = handle;
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding release patch delay %d", delayMs);
    return sendCommand(command, delayMs);
}

void AudioPolicyService::AudioCommandThread::updateAudioPortListCommand()
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = UPDATE_AUDIOPORT_LIST;
    ALOGV("AudioCommandThread() adding update audio port list");
    sendCommand(command);
}

void AudioPolicyService::AudioCommandThread::updateAudioPatchListCommand()
{
    sp<AudioCommand>command = new AudioCommand();
    command->mCommand = UPDATE_AUDIOPATCH_LIST;
    ALOGV("AudioCommandThread() adding update audio patch list");
    sendCommand(command);
}

void AudioPolicyService::AudioCommandThread::changeAudioVolumeGroupCommand(volume_group_t group,
                                                                           int flags)
{
    sp<AudioCommand>command = new AudioCommand();
    command->mCommand = CHANGED_AUDIOVOLUMEGROUP;
    AudioVolumeGroupData *data= new AudioVolumeGroupData();
    data->mGroup = group;
    data->mFlags = flags;
    command->mParam = data;
    ALOGV("AudioCommandThread() adding audio volume group changed");
    sendCommand(command);
}

status_t AudioPolicyService::AudioCommandThread::setAudioPortConfigCommand(
                                            const struct audio_port_config *config, int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_AUDIOPORT_CONFIG;
    SetAudioPortConfigData *data = new SetAudioPortConfigData();
    data->mConfig = *config;
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding set port config delay %d", delayMs);
    return sendCommand(command, delayMs);
}

void AudioPolicyService::AudioCommandThread::dynamicPolicyMixStateUpdateCommand(
        const String8& regId, int32_t state)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = DYN_POLICY_MIX_STATE_UPDATE;
    DynPolicyMixStateUpdateData *data = new DynPolicyMixStateUpdateData();
    data->mRegId = regId;
    data->mState = state;
    command->mParam = data;
    ALOGV("AudioCommandThread() sending dynamic policy mix (id=%s) state update to %d",
            regId.string(), state);
    sendCommand(command);
}

void AudioPolicyService::AudioCommandThread::recordingConfigurationUpdateCommand(
                                                int event,
                                                const record_client_info_t *clientInfo,
                                                const audio_config_base_t *clientConfig,
                                                std::vector<effect_descriptor_t> clientEffects,
                                                const audio_config_base_t *deviceConfig,
                                                std::vector<effect_descriptor_t> effects,
                                                audio_patch_handle_t patchHandle,
                                                audio_source_t source)
{
    sp<AudioCommand>command = new AudioCommand();
    command->mCommand = RECORDING_CONFIGURATION_UPDATE;
    RecordingConfigurationUpdateData *data = new RecordingConfigurationUpdateData();
    data->mEvent = event;
    data->mClientInfo = *clientInfo;
    data->mClientConfig = *clientConfig;
    data->mClientEffects = clientEffects;
    data->mDeviceConfig = *deviceConfig;
    data->mEffects = effects;
    data->mPatchHandle = patchHandle;
    data->mSource = source;
    command->mParam = data;
    ALOGV("AudioCommandThread() adding recording configuration update event %d, source %d uid %u",
            event, clientInfo->source, clientInfo->uid);
    sendCommand(command);
}

status_t AudioPolicyService::AudioCommandThread::sendCommand(sp<AudioCommand>& command, int delayMs)
{
    {
        Mutex::Autolock _l(mLock);
        insertCommand_l(command, delayMs);
        mWaitWorkCV.signal();
    }
    Mutex::Autolock _l(command->mLock);
    while (command->mWaitStatus) {
        nsecs_t timeOutNs = kAudioCommandTimeoutNs + milliseconds(delayMs);
        if (command->mCond.waitRelative(command->mLock, timeOutNs) != NO_ERROR) {
            command->mStatus = TIMED_OUT;
            command->mWaitStatus = false;
        }
    }
    return command->mStatus;
}

// insertCommand_l() must be called with mLock held
void AudioPolicyService::AudioCommandThread::insertCommand_l(sp<AudioCommand>& command, int delayMs)
{
    ssize_t i;  // not size_t because i will count down to -1
    Vector < sp<AudioCommand> > removedCommands;
    command->mTime = systemTime() + milliseconds(delayMs);

    // acquire wake lock to make sure delayed commands are processed
    if (mAudioCommands.isEmpty()) {
        acquire_wake_lock(PARTIAL_WAKE_LOCK, mName.string());
    }

    // check same pending commands with later time stamps and eliminate them
    for (i = (ssize_t)mAudioCommands.size()-1; i >= 0; i--) {
        sp<AudioCommand> command2 = mAudioCommands[i];
        // commands are sorted by increasing time stamp: no need to scan the rest of mAudioCommands
        if (command2->mTime <= command->mTime) break;

        // create audio patch or release audio patch commands are equivalent
        // with regard to filtering
        if ((command->mCommand == CREATE_AUDIO_PATCH) ||
                (command->mCommand == RELEASE_AUDIO_PATCH)) {
            if ((command2->mCommand != CREATE_AUDIO_PATCH) &&
                    (command2->mCommand != RELEASE_AUDIO_PATCH)) {
                continue;
            }
        } else if (command2->mCommand != command->mCommand) continue;

        switch (command->mCommand) {
        case SET_PARAMETERS: {
            ParametersData *data = (ParametersData *)command->mParam.get();
            ParametersData *data2 = (ParametersData *)command2->mParam.get();
            if (data->mIO != data2->mIO) break;
            ALOGV("Comparing parameter command %s to new command %s",
                    data2->mKeyValuePairs.string(), data->mKeyValuePairs.string());
            AudioParameter param = AudioParameter(data->mKeyValuePairs);
            AudioParameter param2 = AudioParameter(data2->mKeyValuePairs);
            for (size_t j = 0; j < param.size(); j++) {
                String8 key;
                String8 value;
                param.getAt(j, key, value);
                for (size_t k = 0; k < param2.size(); k++) {
                    String8 key2;
                    String8 value2;
                    param2.getAt(k, key2, value2);
                    if (key2 == key) {
                        param2.remove(key2);
                        ALOGV("Filtering out parameter %s", key2.string());
                        break;
                    }
                }
            }
            // if all keys have been filtered out, remove the command.
            // otherwise, update the key value pairs
            if (param2.size() == 0) {
                removedCommands.add(command2);
            } else {
                data2->mKeyValuePairs = param2.toString();
            }
            command->mTime = command2->mTime;
            // force delayMs to non 0 so that code below does not request to wait for
            // command status as the command is now delayed
            delayMs = 1;
        } break;

        case SET_VOLUME: {
            VolumeData *data = (VolumeData *)command->mParam.get();
            VolumeData *data2 = (VolumeData *)command2->mParam.get();
            if (data->mIO != data2->mIO) break;
            if (data->mStream != data2->mStream) break;
            ALOGV("Filtering out volume command on output %d for stream %d",
                    data->mIO, data->mStream);
            removedCommands.add(command2);
            command->mTime = command2->mTime;
            // force delayMs to non 0 so that code below does not request to wait for
            // command status as the command is now delayed
            delayMs = 1;
        } break;

        case SET_VOICE_VOLUME: {
            VoiceVolumeData *data = (VoiceVolumeData *)command->mParam.get();
            VoiceVolumeData *data2 = (VoiceVolumeData *)command2->mParam.get();
            ALOGV("Filtering out voice volume command value %f replaced by %f",
                  data2->mVolume, data->mVolume);
            removedCommands.add(command2);
            command->mTime = command2->mTime;
            // force delayMs to non 0 so that code below does not request to wait for
            // command status as the command is now delayed
            delayMs = 1;
        } break;

        case CREATE_AUDIO_PATCH:
        case RELEASE_AUDIO_PATCH: {
            audio_patch_handle_t handle;
            struct audio_patch patch;
            if (command->mCommand == CREATE_AUDIO_PATCH) {
                handle = ((CreateAudioPatchData *)command->mParam.get())->mHandle;
                patch = ((CreateAudioPatchData *)command->mParam.get())->mPatch;
            } else {
                handle = ((ReleaseAudioPatchData *)command->mParam.get())->mHandle;
                memset(&patch, 0, sizeof(patch));
            }
            audio_patch_handle_t handle2;
            struct audio_patch patch2;
            if (command2->mCommand == CREATE_AUDIO_PATCH) {
                handle2 = ((CreateAudioPatchData *)command2->mParam.get())->mHandle;
                patch2 = ((CreateAudioPatchData *)command2->mParam.get())->mPatch;
            } else {
                handle2 = ((ReleaseAudioPatchData *)command2->mParam.get())->mHandle;
                memset(&patch2, 0, sizeof(patch2));
            }
            if (handle != handle2) break;
            /* Filter CREATE_AUDIO_PATCH commands only when they are issued for
               same output. */
            if( (command->mCommand == CREATE_AUDIO_PATCH) &&
                (command2->mCommand == CREATE_AUDIO_PATCH) ) {
                bool isOutputDiff = false;
                if (patch.num_sources == patch2.num_sources) {
                    for (unsigned count = 0; count < patch.num_sources; count++) {
                        if (patch.sources[count].id != patch2.sources[count].id) {
                            isOutputDiff = true;
                            break;
                        }
                    }
                    if (isOutputDiff)
                       break;
                }
            }
            ALOGV("Filtering out %s audio patch command for handle %d",
                  (command->mCommand == CREATE_AUDIO_PATCH) ? "create" : "release", handle);
            removedCommands.add(command2);
            command->mTime = command2->mTime;
            // force delayMs to non 0 so that code below does not request to wait for
            // command status as the command is now delayed
            delayMs = 1;
        } break;

        case DYN_POLICY_MIX_STATE_UPDATE: {

        } break;

        case RECORDING_CONFIGURATION_UPDATE: {

        } break;

        default:
            break;
        }
    }

    // remove filtered commands
    for (size_t j = 0; j < removedCommands.size(); j++) {
        // removed commands always have time stamps greater than current command
        for (size_t k = i + 1; k < mAudioCommands.size(); k++) {
            if (mAudioCommands[k].get() == removedCommands[j].get()) {
                ALOGV("suppressing command: %d", mAudioCommands[k]->mCommand);
                mAudioCommands.removeAt(k);
                break;
            }
        }
    }
    removedCommands.clear();

    // Disable wait for status if delay is not 0.
    // Except for create audio patch command because the returned patch handle
    // is needed by audio policy manager
    if (delayMs != 0 && command->mCommand != CREATE_AUDIO_PATCH) {
        command->mWaitStatus = false;
    }

    // insert command at the right place according to its time stamp
    ALOGV("inserting command: %d at index %zd, num commands %zu",
            command->mCommand, i+1, mAudioCommands.size());
    mAudioCommands.insertAt(command, i + 1);
}

void AudioPolicyService::AudioCommandThread::exit()
{
    ALOGV("AudioCommandThread::exit");
    {
        AutoMutex _l(mLock);
        requestExit();
        mWaitWorkCV.signal();
    }
    // Note that we can call it from the thread loop if all other references have been released
    // but it will safely return WOULD_BLOCK in this case
    requestExitAndWait();
}

void AudioPolicyService::AudioCommandThread::AudioCommand::dump(char* buffer, size_t size)
{
    snprintf(buffer, size, "   %02d      %06d.%03d  %01u    %p\n",
            mCommand,
            (int)ns2s(mTime),
            (int)ns2ms(mTime)%1000,
            mWaitStatus,
            mParam.get());
}

/******* helpers for the service_ops callbacks defined below *********/
void AudioPolicyService::setParameters(audio_io_handle_t ioHandle,
                                       const char *keyValuePairs,
                                       int delayMs)
{
    mAudioCommandThread->parametersCommand(ioHandle, keyValuePairs,
                                           delayMs);
}

int AudioPolicyService::setStreamVolume(audio_stream_type_t stream,
                                        float volume,
                                        audio_io_handle_t output,
                                        int delayMs)
{
    return (int)mAudioCommandThread->volumeCommand(stream, volume,
                                                   output, delayMs);
}

int AudioPolicyService::setVoiceVolume(float volume, int delayMs)
{
    return (int)mAudioCommandThread->voiceVolumeCommand(volume, delayMs);
}

void AudioPolicyService::setEffectSuspended(int effectId,
                                            audio_session_t sessionId,
                                            bool suspended)
{
    mAudioCommandThread->setEffectSuspendedCommand(effectId, sessionId, suspended);
}


extern "C" {
audio_module_handle_t aps_load_hw_module(void *service __unused,
                                             const char *name);
audio_io_handle_t aps_open_output(void *service __unused,
                                         audio_devices_t *pDevices,
                                         uint32_t *pSamplingRate,
                                         audio_format_t *pFormat,
                                         audio_channel_mask_t *pChannelMask,
                                         uint32_t *pLatencyMs,
                                         audio_output_flags_t flags);

audio_io_handle_t aps_open_output_on_module(void *service __unused,
                                                   audio_module_handle_t module,
                                                   audio_devices_t *pDevices,
                                                   uint32_t *pSamplingRate,
                                                   audio_format_t *pFormat,
                                                   audio_channel_mask_t *pChannelMask,
                                                   uint32_t *pLatencyMs,
                                                   audio_output_flags_t flags,
                                                   const audio_offload_info_t *offloadInfo);
audio_io_handle_t aps_open_dup_output(void *service __unused,
                                                 audio_io_handle_t output1,
                                                 audio_io_handle_t output2);
int aps_close_output(void *service __unused, audio_io_handle_t output);
int aps_suspend_output(void *service __unused, audio_io_handle_t output);
int aps_restore_output(void *service __unused, audio_io_handle_t output);
audio_io_handle_t aps_open_input(void *service __unused,
                                        audio_devices_t *pDevices,
                                        uint32_t *pSamplingRate,
                                        audio_format_t *pFormat,
                                        audio_channel_mask_t *pChannelMask,
                                        audio_in_acoustics_t acoustics __unused);
audio_io_handle_t aps_open_input_on_module(void *service __unused,
                                                  audio_module_handle_t module,
                                                  audio_devices_t *pDevices,
                                                  uint32_t *pSamplingRate,
                                                  audio_format_t *pFormat,
                                                  audio_channel_mask_t *pChannelMask);
int aps_close_input(void *service __unused, audio_io_handle_t input);
int aps_invalidate_stream(void *service __unused, audio_stream_type_t stream);
int aps_move_effects(void *service __unused, audio_session_t session,
                                audio_io_handle_t src_output,
                                audio_io_handle_t dst_output);
char * aps_get_parameters(void *service __unused, audio_io_handle_t io_handle,
                                     const char *keys);
void aps_set_parameters(void *service, audio_io_handle_t io_handle,
                                   const char *kv_pairs, int delay_ms);
int aps_set_stream_volume(void *service, audio_stream_type_t stream,
                                     float volume, audio_io_handle_t output,
                                     int delay_ms);
int aps_set_voice_volume(void *service, float volume, int delay_ms);
};

} // namespace android
