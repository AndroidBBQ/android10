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

#define LOG_TAG "AudioPolicyIntefaceImpl"
//#define LOG_NDEBUG 0

#include "AudioPolicyService.h"
#include "TypeConverter.h"
#include <media/MediaAnalyticsItem.h>
#include <media/AudioPolicy.h>
#include <utils/Log.h>

namespace android {


// ----------------------------------------------------------------------------

status_t AudioPolicyService::setDeviceConnectionState(audio_devices_t device,
                                                  audio_policy_dev_state_t state,
                                                  const char *device_address,
                                                  const char *device_name,
                                                  audio_format_t encodedFormat)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    if (state != AUDIO_POLICY_DEVICE_STATE_AVAILABLE &&
            state != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE) {
        return BAD_VALUE;
    }

    ALOGV("setDeviceConnectionState()");
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setDeviceConnectionState(device, state,
                                                         device_address, device_name, encodedFormat);
}

audio_policy_dev_state_t AudioPolicyService::getDeviceConnectionState(
                                                              audio_devices_t device,
                                                              const char *device_address)
{
    if (mAudioPolicyManager == NULL) {
        return AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->getDeviceConnectionState(device,
                                                      device_address);
}

status_t AudioPolicyService::handleDeviceConfigChange(audio_devices_t device,
                                                  const char *device_address,
                                                  const char *device_name,
                                                  audio_format_t encodedFormat)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }

    ALOGV("handleDeviceConfigChange()");
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->handleDeviceConfigChange(device, device_address,
                                                         device_name, encodedFormat);
}

status_t AudioPolicyService::setPhoneState(audio_mode_t state)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    if (uint32_t(state) >= AUDIO_MODE_CNT) {
        return BAD_VALUE;
    }

    ALOGV("setPhoneState()");

    // acquire lock before calling setMode() so that setMode() + setPhoneState() are an atomic
    // operation from policy manager standpoint (no other operation (e.g track start or stop)
    // can be interleaved).
    Mutex::Autolock _l(mLock);
    // TODO: check if it is more appropriate to do it in platform specific policy manager
    AudioSystem::setMode(state);

    AutoCallerClear acc;
    mAudioPolicyManager->setPhoneState(state);
    mPhoneState = state;
    return NO_ERROR;
}

audio_mode_t AudioPolicyService::getPhoneState()
{
    Mutex::Autolock _l(mLock);
    return mPhoneState;
}

status_t AudioPolicyService::setForceUse(audio_policy_force_use_t usage,
                                         audio_policy_forced_cfg_t config)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }

    if (!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }

    if (usage < 0 || usage >= AUDIO_POLICY_FORCE_USE_CNT) {
        return BAD_VALUE;
    }
    if (config < 0 || config >= AUDIO_POLICY_FORCE_CFG_CNT) {
        return BAD_VALUE;
    }
    ALOGV("setForceUse()");
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    mAudioPolicyManager->setForceUse(usage, config);
    return NO_ERROR;
}

audio_policy_forced_cfg_t AudioPolicyService::getForceUse(audio_policy_force_use_t usage)
{
    if (mAudioPolicyManager == NULL) {
        return AUDIO_POLICY_FORCE_NONE;
    }
    if (usage < 0 || usage >= AUDIO_POLICY_FORCE_USE_CNT) {
        return AUDIO_POLICY_FORCE_NONE;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->getForceUse(usage);
}

audio_io_handle_t AudioPolicyService::getOutput(audio_stream_type_t stream)
{
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return AUDIO_IO_HANDLE_NONE;
    }
    if (mAudioPolicyManager == NULL) {
        return AUDIO_IO_HANDLE_NONE;
    }
    ALOGV("getOutput()");
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getOutput(stream);
}

status_t AudioPolicyService::getOutputForAttr(audio_attributes_t *attr,
                                              audio_io_handle_t *output,
                                              audio_session_t session,
                                              audio_stream_type_t *stream,
                                              pid_t pid,
                                              uid_t uid,
                                              const audio_config_t *config,
                                              audio_output_flags_t flags,
                                              audio_port_handle_t *selectedDeviceId,
                                              audio_port_handle_t *portId,
                                              std::vector<audio_io_handle_t> *secondaryOutputs)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    ALOGV("getOutputForAttr()");
    Mutex::Autolock _l(mLock);

    const uid_t callingUid = IPCThreadState::self()->getCallingUid();
    if (!isAudioServerOrMediaServerUid(callingUid) || uid == (uid_t)-1) {
        ALOGW_IF(uid != (uid_t)-1 && uid != callingUid,
                "%s uid %d tried to pass itself off as %d", __FUNCTION__, callingUid, uid);
        uid = callingUid;
    }
    if (!mPackageManager.allowPlaybackCapture(uid)) {
        attr->flags |= AUDIO_FLAG_NO_MEDIA_PROJECTION;
    }
    if (((attr->flags & (AUDIO_FLAG_BYPASS_INTERRUPTION_POLICY|AUDIO_FLAG_BYPASS_MUTE)) != 0)
            && !bypassInterruptionPolicyAllowed(pid, uid)) {
        attr->flags &= ~(AUDIO_FLAG_BYPASS_INTERRUPTION_POLICY|AUDIO_FLAG_BYPASS_MUTE);
    }
    audio_output_flags_t originalFlags = flags;
    AutoCallerClear acc;
    status_t result = mAudioPolicyManager->getOutputForAttr(attr, output, session, stream, uid,
                                                 config,
                                                 &flags, selectedDeviceId, portId,
                                                 secondaryOutputs);

    // FIXME: Introduce a way to check for the the telephony device before opening the output
    if ((result == NO_ERROR) &&
        (flags & AUDIO_OUTPUT_FLAG_INCALL_MUSIC) &&
        !modifyPhoneStateAllowed(pid, uid)) {
        // If the app tries to play music through the telephony device and doesn't have permission
        // the fallback to the default output device.
        mAudioPolicyManager->releaseOutput(*portId);
        flags = originalFlags;
        *selectedDeviceId = AUDIO_PORT_HANDLE_NONE;
        *portId = AUDIO_PORT_HANDLE_NONE;
        secondaryOutputs->clear();
        result = mAudioPolicyManager->getOutputForAttr(attr, output, session, stream, uid, config,
                                                       &flags, selectedDeviceId, portId,
                                                       secondaryOutputs);
    }

    if (result == NO_ERROR) {
        sp <AudioPlaybackClient> client =
            new AudioPlaybackClient(*attr, *output, uid, pid, session, *selectedDeviceId, *stream);
        mAudioPlaybackClients.add(*portId, client);
    }
    return result;
}

void AudioPolicyService::getPlaybackClientAndEffects(audio_port_handle_t portId,
                                                     sp<AudioPlaybackClient>& client,
                                                     sp<AudioPolicyEffects>& effects,
                                                     const char *context)
{
    Mutex::Autolock _l(mLock);
    const ssize_t index = mAudioPlaybackClients.indexOfKey(portId);
    if (index < 0) {
        ALOGE("%s AudioTrack client not found for portId %d", context, portId);
        return;
    }
    client = mAudioPlaybackClients.valueAt(index);
    effects = mAudioPolicyEffects;
}

status_t AudioPolicyService::startOutput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    ALOGV("startOutput()");
    sp<AudioPlaybackClient> client;
    sp<AudioPolicyEffects>audioPolicyEffects;

    getPlaybackClientAndEffects(portId, client, audioPolicyEffects, __func__);

    if (audioPolicyEffects != 0) {
        // create audio processors according to stream
        status_t status = audioPolicyEffects->addOutputSessionEffects(
            client->io, client->stream, client->session);
        if (status != NO_ERROR && status != ALREADY_EXISTS) {
            ALOGW("Failed to add effects on session %d", client->session);
        }
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    status_t status = mAudioPolicyManager->startOutput(portId);
    if (status == NO_ERROR) {
        client->active = true;
    }
    return status;
}

status_t AudioPolicyService::stopOutput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    ALOGV("stopOutput()");
    mOutputCommandThread->stopOutputCommand(portId);
    return NO_ERROR;
}

status_t  AudioPolicyService::doStopOutput(audio_port_handle_t portId)
{
    ALOGV("doStopOutput");
    sp<AudioPlaybackClient> client;
    sp<AudioPolicyEffects>audioPolicyEffects;

    getPlaybackClientAndEffects(portId, client, audioPolicyEffects, __func__);

    if (audioPolicyEffects != 0) {
        // release audio processors from the stream
        status_t status = audioPolicyEffects->releaseOutputSessionEffects(
            client->io, client->stream, client->session);
        if (status != NO_ERROR && status != ALREADY_EXISTS) {
            ALOGW("Failed to release effects on session %d", client->session);
        }
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    status_t status = mAudioPolicyManager->stopOutput(portId);
    if (status == NO_ERROR) {
        client->active = false;
    }
    return status;
}

void AudioPolicyService::releaseOutput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return;
    }
    ALOGV("releaseOutput()");
    mOutputCommandThread->releaseOutputCommand(portId);
}

void AudioPolicyService::doReleaseOutput(audio_port_handle_t portId)
{
    ALOGV("doReleaseOutput from tid %d", gettid());
    sp<AudioPlaybackClient> client;
    sp<AudioPolicyEffects> audioPolicyEffects;

    getPlaybackClientAndEffects(portId, client, audioPolicyEffects, __func__);

    if (audioPolicyEffects != 0 && client->active) {
        // clean up effects if output was not stopped before being released
        audioPolicyEffects->releaseOutputSessionEffects(
            client->io, client->stream, client->session);
    }
    Mutex::Autolock _l(mLock);
    mAudioPlaybackClients.removeItem(portId);

    // called from internal thread: no need to clear caller identity
    mAudioPolicyManager->releaseOutput(portId);
}

status_t AudioPolicyService::getInputForAttr(const audio_attributes_t *attr,
                                             audio_io_handle_t *input,
                                             audio_unique_id_t riid,
                                             audio_session_t session,
                                             pid_t pid,
                                             uid_t uid,
                                             const String16& opPackageName,
                                             const audio_config_base_t *config,
                                             audio_input_flags_t flags,
                                             audio_port_handle_t *selectedDeviceId,
                                             audio_port_handle_t *portId)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }

    // already checked by client, but double-check in case the client wrapper is bypassed
    if ((attr->source < AUDIO_SOURCE_DEFAULT)
            || (attr->source >= AUDIO_SOURCE_CNT
                && attr->source != AUDIO_SOURCE_HOTWORD
                && attr->source != AUDIO_SOURCE_FM_TUNER
                && attr->source != AUDIO_SOURCE_ECHO_REFERENCE)) {
        return BAD_VALUE;
    }

    bool updatePid = (pid == -1);
    const uid_t callingUid = IPCThreadState::self()->getCallingUid();
    if (!isAudioServerOrMediaServerUid(callingUid)) {
        ALOGW_IF(uid != (uid_t)-1 && uid != callingUid,
                "%s uid %d tried to pass itself off as %d", __FUNCTION__, callingUid, uid);
        uid = callingUid;
        updatePid = true;
    }

    if (updatePid) {
        const pid_t callingPid = IPCThreadState::self()->getCallingPid();
        ALOGW_IF(pid != (pid_t)-1 && pid != callingPid,
                 "%s uid %d pid %d tried to pass itself off as pid %d",
                 __func__, callingUid, callingPid, pid);
        pid = callingPid;
    }

    // check calling permissions
    if (!recordingAllowed(opPackageName, pid, uid)) {
        ALOGE("%s permission denied: recording not allowed for uid %d pid %d",
                __func__, uid, pid);
        return PERMISSION_DENIED;
    }

    bool canCaptureOutput = captureAudioOutputAllowed(pid, uid);
    if ((attr->source == AUDIO_SOURCE_VOICE_UPLINK ||
        attr->source == AUDIO_SOURCE_VOICE_DOWNLINK ||
        attr->source == AUDIO_SOURCE_VOICE_CALL ||
        attr->source == AUDIO_SOURCE_ECHO_REFERENCE) &&
        !canCaptureOutput) {
        return PERMISSION_DENIED;
    }

    bool canCaptureHotword = captureHotwordAllowed(opPackageName, pid, uid);
    if ((attr->source == AUDIO_SOURCE_HOTWORD) && !canCaptureHotword) {
        return BAD_VALUE;
    }

    sp<AudioPolicyEffects>audioPolicyEffects;
    {
        status_t status;
        AudioPolicyInterface::input_type_t inputType;

        Mutex::Autolock _l(mLock);
        {
            AutoCallerClear acc;
            // the audio_in_acoustics_t parameter is ignored by get_input()
            status = mAudioPolicyManager->getInputForAttr(attr, input, riid, session, uid,
                                                         config,
                                                         flags, selectedDeviceId,
                                                         &inputType, portId);
        }
        audioPolicyEffects = mAudioPolicyEffects;

        if (status == NO_ERROR) {
            // enforce permission (if any) required for each type of input
            switch (inputType) {
            case AudioPolicyInterface::API_INPUT_MIX_PUBLIC_CAPTURE_PLAYBACK:
                // this use case has been validated in audio service with a MediaProjection token,
                // and doesn't rely on regular permissions
            case AudioPolicyInterface::API_INPUT_LEGACY:
                break;
            case AudioPolicyInterface::API_INPUT_TELEPHONY_RX:
                // FIXME: use the same permission as for remote submix for now.
            case AudioPolicyInterface::API_INPUT_MIX_CAPTURE:
                if (!canCaptureOutput) {
                    ALOGE("getInputForAttr() permission denied: capture not allowed");
                    status = PERMISSION_DENIED;
                }
                break;
            case AudioPolicyInterface::API_INPUT_MIX_EXT_POLICY_REROUTE:
                if (!modifyAudioRoutingAllowed()) {
                    ALOGE("getInputForAttr() permission denied: modify audio routing not allowed");
                    status = PERMISSION_DENIED;
                }
                break;
            case AudioPolicyInterface::API_INPUT_INVALID:
            default:
                LOG_ALWAYS_FATAL("getInputForAttr() encountered an invalid input type %d",
                        (int)inputType);
            }
        }

        if (status != NO_ERROR) {
            if (status == PERMISSION_DENIED) {
                AutoCallerClear acc;
                mAudioPolicyManager->releaseInput(*portId);
            }
            return status;
        }

        sp<AudioRecordClient> client = new AudioRecordClient(*attr, *input, uid, pid, session,
                                                             *selectedDeviceId, opPackageName,
                                                             canCaptureOutput, canCaptureHotword);
        mAudioRecordClients.add(*portId, client);
    }

    if (audioPolicyEffects != 0) {
        // create audio pre processors according to input source
        status_t status = audioPolicyEffects->addInputEffects(*input, attr->source, session);
        if (status != NO_ERROR && status != ALREADY_EXISTS) {
            ALOGW("Failed to add effects on input %d", *input);
        }
    }
    return NO_ERROR;
}

std::string AudioPolicyService::getDeviceTypeStrForPortId(audio_port_handle_t portId) {
    struct audio_port port = {};
    port.id = portId;
    status_t status = mAudioPolicyManager->getAudioPort(&port);
    if (status == NO_ERROR && port.type == AUDIO_PORT_TYPE_DEVICE) {
        return toString(port.ext.device.type);
    }
    return {};
}

status_t AudioPolicyService::startInput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    sp<AudioRecordClient> client;
    {
        Mutex::Autolock _l(mLock);

        ssize_t index = mAudioRecordClients.indexOfKey(portId);
        if (index < 0) {
            return INVALID_OPERATION;
        }
        client = mAudioRecordClients.valueAt(index);
    }

    // check calling permissions
    if (!startRecording(client->opPackageName, client->pid, client->uid)) {
        ALOGE("%s permission denied: recording not allowed for uid %d pid %d",
                __func__, client->uid, client->pid);
        return PERMISSION_DENIED;
    }

    Mutex::Autolock _l(mLock);

    client->active = true;
    client->startTimeNs = systemTime();
    updateUidStates_l();

    status_t status;
    {
        AutoCallerClear acc;
        status = mAudioPolicyManager->startInput(portId);

    }

    // including successes gets very verbose
    // but once we cut over to westworld, log them all.
    if (status != NO_ERROR) {

        static constexpr char kAudioPolicy[] = "audiopolicy";

        static constexpr char kAudioPolicyStatus[] = "android.media.audiopolicy.status";
        static constexpr char kAudioPolicyRqstSrc[] = "android.media.audiopolicy.rqst.src";
        static constexpr char kAudioPolicyRqstPkg[] = "android.media.audiopolicy.rqst.pkg";
        static constexpr char kAudioPolicyRqstSession[] = "android.media.audiopolicy.rqst.session";
        static constexpr char kAudioPolicyRqstDevice[] =
                "android.media.audiopolicy.rqst.device";
        static constexpr char kAudioPolicyActiveSrc[] = "android.media.audiopolicy.active.src";
        static constexpr char kAudioPolicyActivePkg[] = "android.media.audiopolicy.active.pkg";
        static constexpr char kAudioPolicyActiveSession[] =
                "android.media.audiopolicy.active.session";
        static constexpr char kAudioPolicyActiveDevice[] =
                "android.media.audiopolicy.active.device";

        MediaAnalyticsItem *item = MediaAnalyticsItem::create(kAudioPolicy);
        if (item != NULL) {

            item->setInt32(kAudioPolicyStatus, status);

            item->setCString(kAudioPolicyRqstSrc,
                             toString(client->attributes.source).c_str());
            item->setInt32(kAudioPolicyRqstSession, client->session);
            if (client->opPackageName.size() != 0) {
                item->setCString(kAudioPolicyRqstPkg,
                                 std::string(String8(client->opPackageName).string()).c_str());
            } else {
                item->setCString(kAudioPolicyRqstPkg, std::to_string(client->uid).c_str());
            }
            item->setCString(
                    kAudioPolicyRqstDevice, getDeviceTypeStrForPortId(client->deviceId).c_str());

            int count = mAudioRecordClients.size();
            for (int i = 0; i < count ; i++) {
                if (portId == mAudioRecordClients.keyAt(i)) {
                    continue;
                }
                sp<AudioRecordClient> other = mAudioRecordClients.valueAt(i);
                if (other->active) {
                    // keeps the last of the clients marked active
                    item->setCString(kAudioPolicyActiveSrc,
                                     toString(other->attributes.source).c_str());
                    item->setInt32(kAudioPolicyActiveSession, other->session);
                    if (other->opPackageName.size() != 0) {
                        item->setCString(kAudioPolicyActivePkg,
                             std::string(String8(other->opPackageName).string()).c_str());
                    } else {
                        item->setCString(kAudioPolicyRqstPkg,
                                         std::to_string(other->uid).c_str());
                    }
                    item->setCString(kAudioPolicyActiveDevice,
                                     getDeviceTypeStrForPortId(other->deviceId).c_str());
                }
            }
            item->selfrecord();
            delete item;
            item = NULL;
        }
    }

    if (status != NO_ERROR) {
        client->active = false;
        client->startTimeNs = 0;
        updateUidStates_l();
        finishRecording(client->opPackageName, client->uid);
    }

    return status;
}

status_t AudioPolicyService::stopInput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }

    Mutex::Autolock _l(mLock);

    ssize_t index = mAudioRecordClients.indexOfKey(portId);
    if (index < 0) {
        return INVALID_OPERATION;
    }
    sp<AudioRecordClient> client = mAudioRecordClients.valueAt(index);

    client->active = false;
    client->startTimeNs = 0;

    updateUidStates_l();

    // finish the recording app op
    finishRecording(client->opPackageName, client->uid);
    AutoCallerClear acc;
    return mAudioPolicyManager->stopInput(portId);
}

void AudioPolicyService::releaseInput(audio_port_handle_t portId)
{
    if (mAudioPolicyManager == NULL) {
        return;
    }
    sp<AudioPolicyEffects>audioPolicyEffects;
    sp<AudioRecordClient> client;
    {
        Mutex::Autolock _l(mLock);
        audioPolicyEffects = mAudioPolicyEffects;
        ssize_t index = mAudioRecordClients.indexOfKey(portId);
        if (index < 0) {
            return;
        }
        client = mAudioRecordClients.valueAt(index);

        if (client->active) {
            ALOGW("%s releasing active client portId %d", __FUNCTION__, portId);
            client->active = false;
            client->startTimeNs = 0;
            updateUidStates_l();
        }

        mAudioRecordClients.removeItem(portId);
    }
    if (client == 0) {
        return;
    }
    if (audioPolicyEffects != 0) {
        // release audio processors from the input
        status_t status = audioPolicyEffects->releaseInputEffects(client->io, client->session);
        if(status != NO_ERROR) {
            ALOGW("Failed to release effects on input %d", client->io);
        }
    }
    {
        Mutex::Autolock _l(mLock);
        AutoCallerClear acc;
        mAudioPolicyManager->releaseInput(portId);
    }
}

status_t AudioPolicyService::initStreamVolume(audio_stream_type_t stream,
                                            int indexMin,
                                            int indexMax)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    mAudioPolicyManager->initStreamVolume(stream, indexMin, indexMax);
    return NO_ERROR;
}

status_t AudioPolicyService::setStreamVolumeIndex(audio_stream_type_t stream,
                                                  int index,
                                                  audio_devices_t device)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setStreamVolumeIndex(stream,
                                                    index,
                                                    device);
}

status_t AudioPolicyService::getStreamVolumeIndex(audio_stream_type_t stream,
                                                  int *index,
                                                  audio_devices_t device)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getStreamVolumeIndex(stream,
                                                    index,
                                                    device);
}

status_t AudioPolicyService::setVolumeIndexForAttributes(const audio_attributes_t &attributes,
                                                         int index, audio_devices_t device)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setVolumeIndexForAttributes(attributes, index, device);
}

status_t AudioPolicyService::getVolumeIndexForAttributes(const audio_attributes_t &attributes,
                                                         int &index, audio_devices_t device)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getVolumeIndexForAttributes(attributes, index, device);
}

status_t AudioPolicyService::getMinVolumeIndexForAttributes(const audio_attributes_t &attributes,
                                                            int &index)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getMinVolumeIndexForAttributes(attributes, index);
}

status_t AudioPolicyService::getMaxVolumeIndexForAttributes(const audio_attributes_t &attributes,
                                                            int &index)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getMaxVolumeIndexForAttributes(attributes, index);
}

uint32_t AudioPolicyService::getStrategyForStream(audio_stream_type_t stream)
{
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return PRODUCT_STRATEGY_NONE;
    }
    if (mAudioPolicyManager == NULL) {
        return PRODUCT_STRATEGY_NONE;
    }
    // DO NOT LOCK, may be called from AudioFlinger with lock held, reaching deadlock
    AutoCallerClear acc;
    return mAudioPolicyManager->getStrategyForStream(stream);
}

//audio policy: use audio_device_t appropriately

audio_devices_t AudioPolicyService::getDevicesForStream(audio_stream_type_t stream)
{
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return AUDIO_DEVICE_NONE;
    }
    if (mAudioPolicyManager == NULL) {
        return AUDIO_DEVICE_NONE;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getDevicesForStream(stream);
}

audio_io_handle_t AudioPolicyService::getOutputForEffect(const effect_descriptor_t *desc)
{
    // FIXME change return type to status_t, and return NO_INIT here
    if (mAudioPolicyManager == NULL) {
        return 0;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getOutputForEffect(desc);
}

status_t AudioPolicyService::registerEffect(const effect_descriptor_t *desc,
                                audio_io_handle_t io,
                                uint32_t strategy,
                                audio_session_t session,
                                int id)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->registerEffect(desc, io, strategy, session, id);
}

status_t AudioPolicyService::unregisterEffect(int id)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->unregisterEffect(id);
}

status_t AudioPolicyService::setEffectEnabled(int id, bool enabled)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setEffectEnabled(id, enabled);
}

status_t AudioPolicyService::moveEffectsToIo(const std::vector<int>& ids, audio_io_handle_t io)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->moveEffectsToIo(ids, io);
}

bool AudioPolicyService::isStreamActive(audio_stream_type_t stream, uint32_t inPastMs) const
{
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return false;
    }
    if (mAudioPolicyManager == NULL) {
        return false;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->isStreamActive(stream, inPastMs);
}

bool AudioPolicyService::isStreamActiveRemotely(audio_stream_type_t stream, uint32_t inPastMs) const
{
    if (uint32_t(stream) >= AUDIO_STREAM_PUBLIC_CNT) {
        return false;
    }
    if (mAudioPolicyManager == NULL) {
        return false;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->isStreamActiveRemotely(stream, inPastMs);
}

bool AudioPolicyService::isSourceActive(audio_source_t source) const
{
    if (mAudioPolicyManager == NULL) {
        return false;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->isSourceActive(source);
}

status_t AudioPolicyService::getAudioPolicyEffects(sp<AudioPolicyEffects>& audioPolicyEffects)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    {
        Mutex::Autolock _l(mLock);
        audioPolicyEffects = mAudioPolicyEffects;
    }
    if (audioPolicyEffects == 0) {
        return NO_INIT;
    }

    return OK;
}

status_t AudioPolicyService::queryDefaultPreProcessing(audio_session_t audioSession,
                                                       effect_descriptor_t *descriptors,
                                                       uint32_t *count)
{
    sp<AudioPolicyEffects>audioPolicyEffects;
    status_t status = getAudioPolicyEffects(audioPolicyEffects);
    if (status != OK) {
        *count = 0;
        return status;
    }
    return audioPolicyEffects->queryDefaultInputEffects(
            (audio_session_t)audioSession, descriptors, count);
}

status_t AudioPolicyService::addSourceDefaultEffect(const effect_uuid_t *type,
                                                    const String16& opPackageName,
                                                    const effect_uuid_t *uuid,
                                                    int32_t priority,
                                                    audio_source_t source,
                                                    audio_unique_id_t* id)
{
    sp<AudioPolicyEffects>audioPolicyEffects;
    status_t status = getAudioPolicyEffects(audioPolicyEffects);
    if (status != OK) {
        return status;
    }
    if (!modifyDefaultAudioEffectsAllowed()) {
        return PERMISSION_DENIED;
    }
    return audioPolicyEffects->addSourceDefaultEffect(
            type, opPackageName, uuid, priority, source, id);
}

status_t AudioPolicyService::addStreamDefaultEffect(const effect_uuid_t *type,
                                                    const String16& opPackageName,
                                                    const effect_uuid_t *uuid,
                                                    int32_t priority,
                                                    audio_usage_t usage,
                                                    audio_unique_id_t* id)
{
    sp<AudioPolicyEffects>audioPolicyEffects;
    status_t status = getAudioPolicyEffects(audioPolicyEffects);
    if (status != OK) {
        return status;
    }
    if (!modifyDefaultAudioEffectsAllowed()) {
        return PERMISSION_DENIED;
    }
    return audioPolicyEffects->addStreamDefaultEffect(
            type, opPackageName, uuid, priority, usage, id);
}

status_t AudioPolicyService::removeSourceDefaultEffect(audio_unique_id_t id)
{
    sp<AudioPolicyEffects>audioPolicyEffects;
    status_t status = getAudioPolicyEffects(audioPolicyEffects);
    if (status != OK) {
        return status;
    }
    if (!modifyDefaultAudioEffectsAllowed()) {
        return PERMISSION_DENIED;
    }
    return audioPolicyEffects->removeSourceDefaultEffect(id);
}

status_t AudioPolicyService::removeStreamDefaultEffect(audio_unique_id_t id)
{
    sp<AudioPolicyEffects>audioPolicyEffects;
    status_t status = getAudioPolicyEffects(audioPolicyEffects);
    if (status != OK) {
        return status;
    }
    if (!modifyDefaultAudioEffectsAllowed()) {
        return PERMISSION_DENIED;
    }
    return audioPolicyEffects->removeStreamDefaultEffect(id);
}

status_t AudioPolicyService::setAllowedCapturePolicy(uid_t uid, audio_flags_mask_t capturePolicy) {
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        ALOGV("%s() mAudioPolicyManager == NULL", __func__);
        return NO_INIT;
    }
    return mAudioPolicyManager->setAllowedCapturePolicy(uid, capturePolicy);
}

bool AudioPolicyService::isOffloadSupported(const audio_offload_info_t& info)
{
    if (mAudioPolicyManager == NULL) {
        ALOGV("mAudioPolicyManager == NULL");
        return false;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->isOffloadSupported(info);
}

bool AudioPolicyService::isDirectOutputSupported(const audio_config_base_t& config,
                                                 const audio_attributes_t& attributes) {
    if (mAudioPolicyManager == NULL) {
        ALOGV("mAudioPolicyManager == NULL");
        return false;
    }
    Mutex::Autolock _l(mLock);
    return mAudioPolicyManager->isDirectOutputSupported(config, attributes);
}


status_t AudioPolicyService::listAudioPorts(audio_port_role_t role,
                                            audio_port_type_t type,
                                            unsigned int *num_ports,
                                            struct audio_port *ports,
                                            unsigned int *generation)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->listAudioPorts(role, type, num_ports, ports, generation);
}

status_t AudioPolicyService::getAudioPort(struct audio_port *port)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->getAudioPort(port);
}

status_t AudioPolicyService::createAudioPatch(const struct audio_patch *patch,
        audio_patch_handle_t *handle)
{
    Mutex::Autolock _l(mLock);
    if(!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->createAudioPatch(patch, handle,
                                                  IPCThreadState::self()->getCallingUid());
}

status_t AudioPolicyService::releaseAudioPatch(audio_patch_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    if(!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->releaseAudioPatch(handle,
                                                     IPCThreadState::self()->getCallingUid());
}

status_t AudioPolicyService::listAudioPatches(unsigned int *num_patches,
        struct audio_patch *patches,
        unsigned int *generation)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->listAudioPatches(num_patches, patches, generation);
}

status_t AudioPolicyService::setAudioPortConfig(const struct audio_port_config *config)
{
    Mutex::Autolock _l(mLock);
    if(!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->setAudioPortConfig(config);
}

status_t AudioPolicyService::acquireSoundTriggerSession(audio_session_t *session,
                                       audio_io_handle_t *ioHandle,
                                       audio_devices_t *device)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->acquireSoundTriggerSession(session, ioHandle, device);
}

status_t AudioPolicyService::releaseSoundTriggerSession(audio_session_t session)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->releaseSoundTriggerSession(session);
}

status_t AudioPolicyService::registerPolicyMixes(const Vector<AudioMix>& mixes, bool registration)
{
    Mutex::Autolock _l(mLock);

    // loopback|render only need a MediaProjection (checked in caller AudioService.java)
    bool needModifyAudioRouting = std::any_of(mixes.begin(), mixes.end(), [](auto& mix) {
            return !is_mix_loopback_render(mix.mRouteFlags); });
    if (needModifyAudioRouting && !modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }

    bool needCaptureMediaOutput = std::any_of(mixes.begin(), mixes.end(), [](auto& mix) {
            return mix.mAllowPrivilegedPlaybackCapture; });
    const uid_t callingUid = IPCThreadState::self()->getCallingUid();
    const pid_t callingPid = IPCThreadState::self()->getCallingPid();
    if (needCaptureMediaOutput && !captureMediaOutputAllowed(callingPid, callingUid)) {
        return PERMISSION_DENIED;
    }

    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    if (registration) {
        return mAudioPolicyManager->registerPolicyMixes(mixes);
    } else {
        return mAudioPolicyManager->unregisterPolicyMixes(mixes);
    }
}

status_t AudioPolicyService::setUidDeviceAffinities(uid_t uid,
        const Vector<AudioDeviceTypeAddr>& devices) {
    Mutex::Autolock _l(mLock);
    if(!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->setUidDeviceAffinities(uid, devices);
}

status_t AudioPolicyService::removeUidDeviceAffinities(uid_t uid) {
    Mutex::Autolock _l(mLock);
    if(!modifyAudioRoutingAllowed()) {
        return PERMISSION_DENIED;
    }
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->removeUidDeviceAffinities(uid);
}

status_t AudioPolicyService::startAudioSource(const struct audio_port_config *source,
                                              const audio_attributes_t *attributes,
                                              audio_port_handle_t *portId)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    // startAudioSource should be created as the calling uid
    const uid_t callingUid = IPCThreadState::self()->getCallingUid();
    AutoCallerClear acc;
    return mAudioPolicyManager->startAudioSource(source, attributes, portId, callingUid);
}

status_t AudioPolicyService::stopAudioSource(audio_port_handle_t portId)
{
    Mutex::Autolock _l(mLock);
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    AutoCallerClear acc;
    return mAudioPolicyManager->stopAudioSource(portId);
}

status_t AudioPolicyService::setMasterMono(bool mono)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    if (!settingsAllowed()) {
        return PERMISSION_DENIED;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setMasterMono(mono);
}

status_t AudioPolicyService::getMasterMono(bool *mono)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getMasterMono(mono);
}


float AudioPolicyService::getStreamVolumeDB(
            audio_stream_type_t stream, int index, audio_devices_t device)
{
    if (mAudioPolicyManager == NULL) {
        return NAN;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getStreamVolumeDB(stream, index, device);
}

status_t AudioPolicyService::getSurroundFormats(unsigned int *numSurroundFormats,
                                                audio_format_t *surroundFormats,
                                                bool *surroundFormatsEnabled,
                                                bool reported)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getSurroundFormats(numSurroundFormats, surroundFormats,
                                                   surroundFormatsEnabled, reported);
}

status_t AudioPolicyService::getHwOffloadEncodingFormatsSupportedForA2DP(
                                        std::vector<audio_format_t> *formats)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->getHwOffloadEncodingFormatsSupportedForA2DP(formats);
}

status_t AudioPolicyService::setSurroundFormatEnabled(audio_format_t audioFormat, bool enabled)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->setSurroundFormatEnabled(audioFormat, enabled);
}

status_t AudioPolicyService::setAssistantUid(uid_t uid)
{
    Mutex::Autolock _l(mLock);
    mUidPolicy->setAssistantUid(uid);
    return NO_ERROR;
}

status_t AudioPolicyService::setA11yServicesUids(const std::vector<uid_t>& uids)
{
    Mutex::Autolock _l(mLock);
    mUidPolicy->setA11yUids(uids);
    return NO_ERROR;
}

bool AudioPolicyService::isHapticPlaybackSupported()
{
    if (mAudioPolicyManager == NULL) {
        ALOGW("%s, mAudioPolicyManager == NULL", __func__);
        return false;
    }
    Mutex::Autolock _l(mLock);
    AutoCallerClear acc;
    return mAudioPolicyManager->isHapticPlaybackSupported();
}

status_t AudioPolicyService::listAudioProductStrategies(AudioProductStrategyVector &strategies)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    return mAudioPolicyManager->listAudioProductStrategies(strategies);
}

status_t AudioPolicyService::getProductStrategyFromAudioAttributes(
        const AudioAttributes &aa, product_strategy_t &productStrategy)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    return mAudioPolicyManager->getProductStrategyFromAudioAttributes(aa, productStrategy);
}

status_t AudioPolicyService::listAudioVolumeGroups(AudioVolumeGroupVector &groups)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    return mAudioPolicyManager->listAudioVolumeGroups(groups);
}

status_t AudioPolicyService::getVolumeGroupFromAudioAttributes(const AudioAttributes &aa,
                                                               volume_group_t &volumeGroup)
{
    if (mAudioPolicyManager == NULL) {
        return NO_INIT;
    }
    Mutex::Autolock _l(mLock);
    return mAudioPolicyManager->getVolumeGroupFromAudioAttributes(aa, volumeGroup);
}

status_t AudioPolicyService::setRttEnabled(bool enabled)
{
    Mutex::Autolock _l(mLock);
    mUidPolicy->setRttEnabled(enabled);
    return NO_ERROR;
}

} // namespace android
