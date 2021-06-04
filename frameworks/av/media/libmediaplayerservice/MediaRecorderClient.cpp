/*
 ** Copyright 2008, The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaRecorderService"
#include <utils/Log.h>

#include "MediaRecorderClient.h"
#include "MediaPlayerService.h"
#include "StagefrightRecorder.h"

#include <android/hardware/media/omx/1.0/IOmx.h>
#include <android/hardware/media/c2/1.0/IComponentStore.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>
#include <codec2/hidl/client.h>
#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get
#include <gui/IGraphicBufferProducer.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system/audio.h>
#include <utils/String16.h>

#include <dirent.h>
#include <unistd.h>
#include <string.h>

namespace android {

const char* cameraPermission = "android.permission.CAMERA";
const char* recordAudioPermission = "android.permission.RECORD_AUDIO";

static bool checkPermission(const char* permissionString) {
    if (getpid() == IPCThreadState::self()->getCallingPid()) return true;
    bool ok = checkCallingPermission(String16(permissionString));
    if (!ok) ALOGE("Request requires %s", permissionString);
    return ok;
}

status_t MediaRecorderClient::setInputSurface(const sp<PersistentSurface>& surface)
{
    ALOGV("setInputSurface");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setInputSurface(surface);
}

sp<IGraphicBufferProducer> MediaRecorderClient::querySurfaceMediaSource()
{
    ALOGV("Query SurfaceMediaSource");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NULL;
    }
    return mRecorder->querySurfaceMediaSource();
}



status_t MediaRecorderClient::setCamera(const sp<hardware::ICamera>& camera,
                                        const sp<ICameraRecordingProxy>& proxy)
{
    ALOGV("setCamera");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setCamera(camera, proxy);
}

status_t MediaRecorderClient::setPreviewSurface(const sp<IGraphicBufferProducer>& surface)
{
    ALOGV("setPreviewSurface");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setPreviewSurface(surface);
}

status_t MediaRecorderClient::setVideoSource(int vs)
{
    ALOGV("setVideoSource(%d)", vs);
    // Check camera permission for sources other than SURFACE
    if (vs != VIDEO_SOURCE_SURFACE && !checkPermission(cameraPermission)) {
        return PERMISSION_DENIED;
    }
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL)     {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setVideoSource((video_source)vs);
}

status_t MediaRecorderClient::setAudioSource(int as)
{
    ALOGV("setAudioSource(%d)", as);
    if (!checkPermission(recordAudioPermission)) {
        return PERMISSION_DENIED;
    }
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL)  {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setAudioSource((audio_source_t)as);
}

status_t MediaRecorderClient::setOutputFormat(int of)
{
    ALOGV("setOutputFormat(%d)", of);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setOutputFormat((output_format)of);
}

status_t MediaRecorderClient::setVideoEncoder(int ve)
{
    ALOGV("setVideoEncoder(%d)", ve);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setVideoEncoder((video_encoder)ve);
}

status_t MediaRecorderClient::setAudioEncoder(int ae)
{
    ALOGV("setAudioEncoder(%d)", ae);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setAudioEncoder((audio_encoder)ae);
}

status_t MediaRecorderClient::setOutputFile(int fd)
{
    ALOGV("setOutputFile(%d)", fd);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setOutputFile(fd);
}

status_t MediaRecorderClient::setNextOutputFile(int fd)
{
    ALOGV("setNextOutputFile(%d)", fd);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setNextOutputFile(fd);
}

status_t MediaRecorderClient::setVideoSize(int width, int height)
{
    ALOGV("setVideoSize(%dx%d)", width, height);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setVideoSize(width, height);
}

status_t MediaRecorderClient::setVideoFrameRate(int frames_per_second)
{
    ALOGV("setVideoFrameRate(%d)", frames_per_second);
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setVideoFrameRate(frames_per_second);
}

status_t MediaRecorderClient::setParameters(const String8& params) {
    ALOGV("setParameters(%s)", params.string());
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setParameters(params);
}

status_t MediaRecorderClient::prepare()
{
    ALOGV("prepare");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->prepare();
}


status_t MediaRecorderClient::getMaxAmplitude(int* max)
{
    ALOGV("getMaxAmplitude");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->getMaxAmplitude(max);
}

status_t MediaRecorderClient::getMetrics(Parcel* reply)
{
    ALOGV("MediaRecorderClient::getMetrics");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->getMetrics(reply);
}

status_t MediaRecorderClient::start()
{
    ALOGV("start");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->start();

}

status_t MediaRecorderClient::stop()
{
    ALOGV("stop");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->stop();
}

status_t MediaRecorderClient::pause()
{
    ALOGV("pause");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->pause();

}

status_t MediaRecorderClient::resume()
{
    ALOGV("resume");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->resume();
}

status_t MediaRecorderClient::init()
{
    ALOGV("init");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->init();
}

status_t MediaRecorderClient::close()
{
    ALOGV("close");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->close();
}


status_t MediaRecorderClient::reset()
{
    ALOGV("reset");
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->reset();
}

status_t MediaRecorderClient::release()
{
    ALOGV("release");
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        delete mRecorder;
        mRecorder = NULL;
        wp<MediaRecorderClient> client(this);
        mMediaPlayerService->removeMediaRecorderClient(client);
    }
    mDeathNotifiers.clear();
    return NO_ERROR;
}

MediaRecorderClient::MediaRecorderClient(const sp<MediaPlayerService>& service, pid_t pid,
        const String16& opPackageName)
{
    ALOGV("Client constructor");
    mPid = pid;
    mRecorder = new StagefrightRecorder(opPackageName);
    mMediaPlayerService = service;
}

MediaRecorderClient::~MediaRecorderClient()
{
    ALOGV("Client destructor");
    release();
}

MediaRecorderClient::AudioDeviceUpdatedNotifier::AudioDeviceUpdatedNotifier(
        const sp<IMediaRecorderClient>& listener) {
    mListener = listener;
}

MediaRecorderClient::AudioDeviceUpdatedNotifier::~AudioDeviceUpdatedNotifier() {
}

void MediaRecorderClient::AudioDeviceUpdatedNotifier::onAudioDeviceUpdate(
        audio_io_handle_t audioIo,
        audio_port_handle_t deviceId) {
    sp<IMediaRecorderClient> listener = mListener.promote();
    if (listener != NULL) {
        listener->notify(MEDIA_RECORDER_AUDIO_ROUTING_CHANGED, audioIo, deviceId);
    } else {
        ALOGW("listener for process %d death is gone", MEDIA_RECORDER_AUDIO_ROUTING_CHANGED);
    }
}

status_t MediaRecorderClient::setListener(const sp<IMediaRecorderClient>& listener)
{
    ALOGV("setListener");
    Mutex::Autolock lock(mLock);
    mDeathNotifiers.clear();
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    mRecorder->setListener(listener);

    sp<IServiceManager> sm = defaultServiceManager();

    // WORKAROUND: We don't know if camera exists here and getService might block for 5 seconds.
    // Use checkService for camera if we don't know it exists.
    static std::atomic<bool> sCameraChecked(false);  // once true never becomes false.
    static std::atomic<bool> sCameraVerified(false); // once true never becomes false.
    sp<IBinder> binder = (sCameraVerified || !sCameraChecked)
        ? sm->getService(String16("media.camera")) : sm->checkService(String16("media.camera"));
    // If the device does not have a camera, do not create a death listener for it.
    if (binder != NULL) {
        sCameraVerified = true;
        mDeathNotifiers.emplace_back(
                binder, [l = wp<IMediaRecorderClient>(listener)](){
            sp<IMediaRecorderClient> listener = l.promote();
            if (listener) {
                ALOGV("media.camera service died. "
                      "Sending death notification.");
                listener->notify(
                        MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED,
                        MediaPlayerService::CAMERA_PROCESS_DEATH);
            } else {
                ALOGW("media.camera service died without a death handler.");
            }
        });
    }
    sCameraChecked = true;

    {
        using ::android::hidl::base::V1_0::IBase;

        // Listen to OMX's IOmxStore/default
        {
            sp<IBase> base = ::android::hardware::media::omx::V1_0::
                    IOmx::getService();
            if (base == nullptr) {
                ALOGD("OMX service is not available");
            } else {
                mDeathNotifiers.emplace_back(
                        base, [l = wp<IMediaRecorderClient>(listener)](){
                    sp<IMediaRecorderClient> listener = l.promote();
                    if (listener) {
                        ALOGV("OMX service died. "
                              "Sending death notification.");
                        listener->notify(
                                MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED,
                                MediaPlayerService::MEDIACODEC_PROCESS_DEATH);
                    } else {
                        ALOGW("OMX service died without a death handler.");
                    }
                });
            }
        }

        // Listen to Codec2's IComponentStore instances
        {
            for (std::shared_ptr<Codec2Client> const& client :
                    Codec2Client::CreateFromAllServices()) {
                sp<IBase> base = client->getBase();
                mDeathNotifiers.emplace_back(
                        base, [l = wp<IMediaRecorderClient>(listener),
                               name = std::string(client->getServiceName())]() {
                    sp<IMediaRecorderClient> listener = l.promote();
                    if (listener) {
                        ALOGV("Codec2 service \"%s\" died. "
                              "Sending death notification",
                              name.c_str());
                        listener->notify(
                                MEDIA_ERROR, MEDIA_ERROR_SERVER_DIED,
                                MediaPlayerService::MEDIACODEC_PROCESS_DEATH);
                    } else {
                        ALOGW("Codec2 service \"%s\" died "
                              "without a death handler",
                              name.c_str());
                    }
                });
            }
        }
    }

    mAudioDeviceUpdatedNotifier = new AudioDeviceUpdatedNotifier(listener);
    mRecorder->setAudioDeviceCallback(mAudioDeviceUpdatedNotifier);

    return OK;
}

status_t MediaRecorderClient::setClientName(const String16& clientName) {
    ALOGV("setClientName(%s)", String8(clientName).string());
    Mutex::Autolock lock(mLock);
    if (mRecorder == NULL) {
        ALOGE("recorder is not initialized");
        return NO_INIT;
    }
    return mRecorder->setClientName(clientName);
}

status_t MediaRecorderClient::dump(int fd, const Vector<String16>& args) {
    if (mRecorder != NULL) {
        return mRecorder->dump(fd, args);
    }
    return OK;
}

status_t MediaRecorderClient::setInputDevice(audio_port_handle_t deviceId) {
    ALOGV("setInputDevice(%d)", deviceId);
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        return mRecorder->setInputDevice(deviceId);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::getRoutedDeviceId(audio_port_handle_t* deviceId) {
    ALOGV("getRoutedDeviceId");
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        return mRecorder->getRoutedDeviceId(deviceId);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::enableAudioDeviceCallback(bool enabled) {
    ALOGV("enableDeviceCallback: %d", enabled);
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        return mRecorder->enableAudioDeviceCallback(enabled);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::getActiveMicrophones(
        std::vector<media::MicrophoneInfo>* activeMicrophones) {
    ALOGV("getActiveMicrophones");
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        return mRecorder->getActiveMicrophones(activeMicrophones);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::setPreferredMicrophoneDirection(
            audio_microphone_direction_t direction) {
    ALOGV("setPreferredMicrophoneDirection(%d)", direction);
    if (mRecorder != NULL) {
        return mRecorder->setPreferredMicrophoneDirection(direction);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::setPreferredMicrophoneFieldDimension(float zoom) {
    ALOGV("setPreferredMicrophoneFieldDimension(%f)", zoom);
    if (mRecorder != NULL) {
        return mRecorder->setPreferredMicrophoneFieldDimension(zoom);
    }
    return NO_INIT;
}

status_t MediaRecorderClient::getPortId(audio_port_handle_t *portId) {
    ALOGV("getPortId");
    Mutex::Autolock lock(mLock);
    if (mRecorder != NULL) {
        return mRecorder->getPortId(portId);
    }
    return NO_INIT;
}
}; // namespace android
