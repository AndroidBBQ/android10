/*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "JAudioTrack"

#include "media/JAudioAttributes.h"
#include "media/JAudioFormat.h"
#include "mediaplayer2/JAudioTrack.h"

#include <android_media_AudioErrors.h>
#include <mediaplayer2/JavaVMHelper.h>

namespace android {

// TODO: Store Java class/methodID as a member variable in the class.
// TODO: Add NULL && Exception checks after every JNI call.
JAudioTrack::JAudioTrack(                             // < Usages of the arguments are below >
        uint32_t sampleRate,                          // AudioFormat && bufferSizeInBytes
        audio_format_t format,                        // AudioFormat && bufferSizeInBytes
        audio_channel_mask_t channelMask,             // AudioFormat && bufferSizeInBytes
        callback_t cbf,                               // Offload
        void* user,                                   // Offload
        size_t frameCount,                            // bufferSizeInBytes
        int32_t sessionId,                            // AudioTrack
        const jobject attributes,                     // AudioAttributes
        float maxRequiredSpeed) {                     // bufferSizeInBytes

    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jclass jAudioTrackCls = env->FindClass("android/media/AudioTrack");
    mAudioTrackCls = reinterpret_cast<jclass>(env->NewGlobalRef(jAudioTrackCls));
    env->DeleteLocalRef(jAudioTrackCls);

    maxRequiredSpeed = std::min(std::max(maxRequiredSpeed, 1.0f), AUDIO_TIMESTRETCH_SPEED_MAX);

    int bufferSizeInBytes = 0;
    if (sampleRate == 0 || frameCount > 0) {
        // Manually calculate buffer size.
        bufferSizeInBytes = audio_channel_count_from_out_mask(channelMask)
                * audio_bytes_per_sample(format) * (frameCount > 0 ? frameCount : 1);
    } else if (sampleRate > 0) {
        // Call Java AudioTrack::getMinBufferSize().
        jmethodID jGetMinBufferSize =
                env->GetStaticMethodID(mAudioTrackCls, "getMinBufferSize", "(III)I");
        bufferSizeInBytes = env->CallStaticIntMethod(mAudioTrackCls, jGetMinBufferSize,
                sampleRate, outChannelMaskFromNative(channelMask), audioFormatFromNative(format));
    }
    bufferSizeInBytes = (int) (bufferSizeInBytes * maxRequiredSpeed);

    // Create a Java AudioTrack object through its Builder.
    jclass jBuilderCls = env->FindClass("android/media/AudioTrack$Builder");
    jmethodID jBuilderCtor = env->GetMethodID(jBuilderCls, "<init>", "()V");
    jobject jBuilderObj = env->NewObject(jBuilderCls, jBuilderCtor);

    {
        sp<JObjectHolder> audioAttributesObj;
        if (attributes != NULL) {
            audioAttributesObj = new JObjectHolder(attributes);
        } else {
            audioAttributesObj = new JObjectHolder(
                    JAudioAttributes::createAudioAttributesObj(env, NULL));
        }
        jmethodID jSetAudioAttributes = env->GetMethodID(jBuilderCls, "setAudioAttributes",
                "(Landroid/media/AudioAttributes;)Landroid/media/AudioTrack$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj,
                jSetAudioAttributes, audioAttributesObj->getJObject());
    }

    jmethodID jSetAudioFormat = env->GetMethodID(jBuilderCls, "setAudioFormat",
            "(Landroid/media/AudioFormat;)Landroid/media/AudioTrack$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetAudioFormat,
            JAudioFormat::createAudioFormatObj(env, sampleRate, format, channelMask));

    jmethodID jSetBufferSizeInBytes = env->GetMethodID(jBuilderCls, "setBufferSizeInBytes",
            "(I)Landroid/media/AudioTrack$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetBufferSizeInBytes, bufferSizeInBytes);

    // We only use streaming mode of Java AudioTrack.
    jfieldID jModeStream = env->GetStaticFieldID(mAudioTrackCls, "MODE_STREAM", "I");
    jint transferMode = env->GetStaticIntField(mAudioTrackCls, jModeStream);
    jmethodID jSetTransferMode = env->GetMethodID(jBuilderCls, "setTransferMode",
            "(I)Landroid/media/AudioTrack$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetTransferMode,
            transferMode /* Java AudioTrack::MODE_STREAM */);

    if (sessionId != 0) {
        jmethodID jSetSessionId = env->GetMethodID(jBuilderCls, "setSessionId",
                "(I)Landroid/media/AudioTrack$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetSessionId, sessionId);
    }

    mFlags = AUDIO_OUTPUT_FLAG_NONE;
    if (cbf != NULL) {
        jmethodID jSetOffloadedPlayback = env->GetMethodID(jBuilderCls, "setOffloadedPlayback",
                "(Z)Landroid/media/AudioTrack$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderObj, jSetOffloadedPlayback, true);
        mFlags = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    }

    jmethodID jBuild = env->GetMethodID(jBuilderCls, "build", "()Landroid/media/AudioTrack;");
    jobject jAudioTrackObj = env->CallObjectMethod(jBuilderObj, jBuild);
    mAudioTrackObj = reinterpret_cast<jobject>(env->NewGlobalRef(jAudioTrackObj));
    env->DeleteLocalRef(jBuilderObj);

    if (cbf != NULL) {
        // Set offload mode callback
        jobject jStreamEventCallbackObj = createStreamEventCallback(cbf, user);
        jobject jExecutorObj = createCallbackExecutor();
        jmethodID jSetStreamEventCallback = env->GetMethodID(
                jAudioTrackCls,
                "setStreamEventCallback",
                "(Ljava/util/concurrent/Executor;Landroid/media/AudioTrack$StreamEventCallback;)V");
        env->CallVoidMethod(
                mAudioTrackObj, jSetStreamEventCallback, jExecutorObj, jStreamEventCallbackObj);
    }
}

JAudioTrack::~JAudioTrack() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    env->DeleteGlobalRef(mAudioTrackCls);
    env->DeleteGlobalRef(mAudioTrackObj);
}

size_t JAudioTrack::frameCount() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetBufferSizeInFrames = env->GetMethodID(
            mAudioTrackCls, "getBufferSizeInFrames", "()I");
    return env->CallIntMethod(mAudioTrackObj, jGetBufferSizeInFrames);
}

size_t JAudioTrack::channelCount() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetChannelCount = env->GetMethodID(mAudioTrackCls, "getChannelCount", "()I");
    return env->CallIntMethod(mAudioTrackObj, jGetChannelCount);
}

uint32_t JAudioTrack::latency() {
    // TODO: Currently hard-coded as returning zero.
    return 0;
}

status_t JAudioTrack::getPosition(uint32_t *position) {
    if (position == NULL) {
        return BAD_VALUE;
    }

    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetPlaybackHeadPosition = env->GetMethodID(
            mAudioTrackCls, "getPlaybackHeadPosition", "()I");
    *position = env->CallIntMethod(mAudioTrackObj, jGetPlaybackHeadPosition);

    return NO_ERROR;
}

status_t JAudioTrack::getTimestamp(AudioTimestamp& timestamp) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jclass jAudioTimeStampCls = env->FindClass("android/media/AudioTimestamp");
    jobject jAudioTimeStampObj = env->AllocObject(jAudioTimeStampCls);

    jfieldID jFramePosition = env->GetFieldID(jAudioTimeStampCls, "framePosition", "J");
    jfieldID jNanoTime = env->GetFieldID(jAudioTimeStampCls, "nanoTime", "J");

    jmethodID jGetTimestamp = env->GetMethodID(mAudioTrackCls,
            "getTimestamp", "(Landroid/media/AudioTimestamp;)Z");
    bool success = env->CallBooleanMethod(mAudioTrackObj, jGetTimestamp, jAudioTimeStampObj);

    if (!success) {
        return NO_INIT;
    }

    long long framePosition = env->GetLongField(jAudioTimeStampObj, jFramePosition);
    long long nanoTime = env->GetLongField(jAudioTimeStampObj, jNanoTime);

    struct timespec ts;
    const long long secondToNano = 1000000000LL; // 1E9
    ts.tv_sec = nanoTime / secondToNano;
    ts.tv_nsec = nanoTime % secondToNano;
    timestamp.mTime = ts;
    timestamp.mPosition = (uint32_t) framePosition;

    return NO_ERROR;
}

status_t JAudioTrack::getTimestamp(ExtendedTimestamp *timestamp __unused) {
    // TODO: Implement this after appropriate Java AudioTrack method is available.
    return NO_ERROR;
}

status_t JAudioTrack::setPlaybackRate(const AudioPlaybackRate &playbackRate) {
    // TODO: existing native AudioTrack returns INVALID_OPERATION on offload/direct/fast tracks.
    // Should we do the same thing?
    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jclass jPlaybackParamsCls = env->FindClass("android/media/PlaybackParams");
    jmethodID jPlaybackParamsCtor = env->GetMethodID(jPlaybackParamsCls, "<init>", "()V");
    jobject jPlaybackParamsObj = env->NewObject(jPlaybackParamsCls, jPlaybackParamsCtor);

    jmethodID jSetAudioFallbackMode = env->GetMethodID(
            jPlaybackParamsCls, "setAudioFallbackMode", "(I)Landroid/media/PlaybackParams;");
    jPlaybackParamsObj = env->CallObjectMethod(
            jPlaybackParamsObj, jSetAudioFallbackMode, playbackRate.mFallbackMode);

    jmethodID jSetAudioStretchMode = env->GetMethodID(
                jPlaybackParamsCls, "setAudioStretchMode", "(I)Landroid/media/PlaybackParams;");
    jPlaybackParamsObj = env->CallObjectMethod(
            jPlaybackParamsObj, jSetAudioStretchMode, playbackRate.mStretchMode);

    jmethodID jSetPitch = env->GetMethodID(
            jPlaybackParamsCls, "setPitch", "(F)Landroid/media/PlaybackParams;");
    jPlaybackParamsObj = env->CallObjectMethod(jPlaybackParamsObj, jSetPitch, playbackRate.mPitch);

    jmethodID jSetSpeed = env->GetMethodID(
            jPlaybackParamsCls, "setSpeed", "(F)Landroid/media/PlaybackParams;");
    jPlaybackParamsObj = env->CallObjectMethod(jPlaybackParamsObj, jSetSpeed, playbackRate.mSpeed);


    // Set this Java PlaybackParams object into Java AudioTrack.
    jmethodID jSetPlaybackParams = env->GetMethodID(
            mAudioTrackCls, "setPlaybackParams", "(Landroid/media/PlaybackParams;)V");
    env->CallVoidMethod(mAudioTrackObj, jSetPlaybackParams, jPlaybackParamsObj);
    // TODO: Should we catch the Java IllegalArgumentException?

    return NO_ERROR;
}

const AudioPlaybackRate JAudioTrack::getPlaybackRate() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jmethodID jGetPlaybackParams = env->GetMethodID(
            mAudioTrackCls, "getPlaybackParams", "()Landroid/media/PlaybackParams;");
    jobject jPlaybackParamsObj = env->CallObjectMethod(mAudioTrackObj, jGetPlaybackParams);

    AudioPlaybackRate playbackRate;
    jclass jPlaybackParamsCls = env->FindClass("android/media/PlaybackParams");

    jmethodID jGetAudioFallbackMode = env->GetMethodID(
            jPlaybackParamsCls, "getAudioFallbackMode", "()I");
    // TODO: Should we enable passing AUDIO_TIMESTRETCH_FALLBACK_CUT_REPEAT?
    //       The enum is internal only, so it is not defined in PlaybackParmas.java.
    // TODO: Is this right way to convert an int to an enum?
    playbackRate.mFallbackMode = static_cast<AudioTimestretchFallbackMode>(
            env->CallIntMethod(jPlaybackParamsObj, jGetAudioFallbackMode));

    jmethodID jGetAudioStretchMode = env->GetMethodID(
            jPlaybackParamsCls, "getAudioStretchMode", "()I");
    playbackRate.mStretchMode = static_cast<AudioTimestretchStretchMode>(
            env->CallIntMethod(jPlaybackParamsObj, jGetAudioStretchMode));

    jmethodID jGetPitch = env->GetMethodID(jPlaybackParamsCls, "getPitch", "()F");
    playbackRate.mPitch = env->CallFloatMethod(jPlaybackParamsObj, jGetPitch);

    jmethodID jGetSpeed = env->GetMethodID(jPlaybackParamsCls, "getSpeed", "()F");
    playbackRate.mSpeed = env->CallFloatMethod(jPlaybackParamsObj, jGetSpeed);

    return playbackRate;
}

media::VolumeShaper::Status JAudioTrack::applyVolumeShaper(
        const sp<media::VolumeShaper::Configuration>& configuration,
        const sp<media::VolumeShaper::Operation>& operation) {

    jobject jConfigurationObj = createVolumeShaperConfigurationObj(configuration);
    jobject jOperationObj = createVolumeShaperOperationObj(operation);

    if (jConfigurationObj == NULL || jOperationObj == NULL) {
        return media::VolumeShaper::Status(BAD_VALUE);
    }

    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jmethodID jCreateVolumeShaper = env->GetMethodID(mAudioTrackCls, "createVolumeShaper",
            "(Landroid/media/VolumeShaper$Configuration;)Landroid/media/VolumeShaper;");
    jobject jVolumeShaperObj = env->CallObjectMethod(
            mAudioTrackObj, jCreateVolumeShaper, jConfigurationObj);

    jclass jVolumeShaperCls = env->FindClass("android/media/VolumeShaper");
    jmethodID jApply = env->GetMethodID(jVolumeShaperCls, "apply",
            "(Landroid/media/VolumeShaper$Operation;)V");
    env->CallVoidMethod(jVolumeShaperObj, jApply, jOperationObj);

    return media::VolumeShaper::Status(NO_ERROR);
}

status_t JAudioTrack::setAuxEffectSendLevel(float level) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jSetAuxEffectSendLevel = env->GetMethodID(
            mAudioTrackCls, "setAuxEffectSendLevel", "(F)I");
    int result = env->CallIntMethod(mAudioTrackObj, jSetAuxEffectSendLevel, level);
    return javaToNativeStatus(result);
}

status_t JAudioTrack::attachAuxEffect(int effectId) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jAttachAuxEffect = env->GetMethodID(mAudioTrackCls, "attachAuxEffect", "(I)I");
    int result = env->CallIntMethod(mAudioTrackObj, jAttachAuxEffect, effectId);
    return javaToNativeStatus(result);
}

status_t JAudioTrack::setVolume(float left, float right) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    // TODO: Java setStereoVolume is deprecated. Do we really need this method?
    jmethodID jSetStereoVolume = env->GetMethodID(mAudioTrackCls, "setStereoVolume", "(FF)I");
    int result = env->CallIntMethod(mAudioTrackObj, jSetStereoVolume, left, right);
    return javaToNativeStatus(result);
}

status_t JAudioTrack::setVolume(float volume) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jSetVolume = env->GetMethodID(mAudioTrackCls, "setVolume", "(F)I");
    int result = env->CallIntMethod(mAudioTrackObj, jSetVolume, volume);
    return javaToNativeStatus(result);
}

status_t JAudioTrack::start() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jPlay = env->GetMethodID(mAudioTrackCls, "play", "()V");
    // TODO: Should we catch the Java IllegalStateException from play()?
    env->CallVoidMethod(mAudioTrackObj, jPlay);
    return NO_ERROR;
}

ssize_t JAudioTrack::write(const void* buffer, size_t size, bool blocking) {
    if (buffer == NULL) {
        return BAD_VALUE;
    }

    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jbyteArray jAudioData = env->NewByteArray(size);
    env->SetByteArrayRegion(jAudioData, 0, size, (jbyte *) buffer);

    jclass jByteBufferCls = env->FindClass("java/nio/ByteBuffer");
    jmethodID jWrap = env->GetStaticMethodID(jByteBufferCls, "wrap", "([B)Ljava/nio/ByteBuffer;");
    jobject jByteBufferObj = env->CallStaticObjectMethod(jByteBufferCls, jWrap, jAudioData);

    int writeMode = 0;
    if (blocking) {
        jfieldID jWriteBlocking = env->GetStaticFieldID(mAudioTrackCls, "WRITE_BLOCKING", "I");
        writeMode = env->GetStaticIntField(mAudioTrackCls, jWriteBlocking);
    } else {
        jfieldID jWriteNonBlocking = env->GetStaticFieldID(
                mAudioTrackCls, "WRITE_NON_BLOCKING", "I");
        writeMode = env->GetStaticIntField(mAudioTrackCls, jWriteNonBlocking);
    }

    jmethodID jWrite = env->GetMethodID(mAudioTrackCls, "write", "(Ljava/nio/ByteBuffer;II)I");
    int result = env->CallIntMethod(mAudioTrackObj, jWrite, jByteBufferObj, size, writeMode);

    if (result >= 0) {
        return result;
    } else {
        return javaToNativeStatus(result);
    }
}

void JAudioTrack::stop() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jStop = env->GetMethodID(mAudioTrackCls, "stop", "()V");
    env->CallVoidMethod(mAudioTrackObj, jStop);
    // TODO: Should we catch IllegalStateException?
}

// TODO: Is the right implementation?
bool JAudioTrack::stopped() const {
    return !isPlaying();
}

void JAudioTrack::flush() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jFlush = env->GetMethodID(mAudioTrackCls, "flush", "()V");
    env->CallVoidMethod(mAudioTrackObj, jFlush);
}

void JAudioTrack::pause() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jPause = env->GetMethodID(mAudioTrackCls, "pause", "()V");
    env->CallVoidMethod(mAudioTrackObj, jPause);
    // TODO: Should we catch IllegalStateException?
}

bool JAudioTrack::isPlaying() const {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetPlayState = env->GetMethodID(mAudioTrackCls, "getPlayState", "()I");
    int currentPlayState = env->CallIntMethod(mAudioTrackObj, jGetPlayState);

    // TODO: In Java AudioTrack, there is no STOPPING state.
    // This means while stopping, isPlaying() will return different value in two class.
    //  - in existing native AudioTrack: true
    //  - in JAudioTrack: false
    // If not okay, also modify the implementation of stopped().
    jfieldID jPlayStatePlaying = env->GetStaticFieldID(mAudioTrackCls, "PLAYSTATE_PLAYING", "I");
    int statePlaying = env->GetStaticIntField(mAudioTrackCls, jPlayStatePlaying);
    return currentPlayState == statePlaying;
}

uint32_t JAudioTrack::getSampleRate() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetSampleRate = env->GetMethodID(mAudioTrackCls, "getSampleRate", "()I");
    return env->CallIntMethod(mAudioTrackObj, jGetSampleRate);
}

status_t JAudioTrack::getBufferDurationInUs(int64_t *duration) {
    if (duration == nullptr) {
        return BAD_VALUE;
    }

    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetBufferSizeInFrames = env->GetMethodID(
            mAudioTrackCls, "getBufferSizeInFrames", "()I");
    int bufferSizeInFrames = env->CallIntMethod(mAudioTrackObj, jGetBufferSizeInFrames);

    const double secondToMicro = 1000000LL; // 1E6
    int sampleRate = JAudioTrack::getSampleRate();
    float speed = JAudioTrack::getPlaybackRate().mSpeed;

    *duration = (int64_t) (bufferSizeInFrames * secondToMicro / (sampleRate * speed));
    return NO_ERROR;
}

audio_format_t JAudioTrack::format() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetAudioFormat = env->GetMethodID(mAudioTrackCls, "getAudioFormat", "()I");
    int javaFormat = env->CallIntMethod(mAudioTrackObj, jGetAudioFormat);
    return audioFormatToNative(javaFormat);
}

size_t JAudioTrack::frameSize() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetFormat = env->GetMethodID(mAudioTrackCls,
            "getFormat", "()Landroid/media/AudioFormat;");
    jobject jAudioFormatObj = env->CallObjectMethod(mAudioTrackObj, jGetFormat);

    jclass jAudioFormatCls = env->FindClass("android/media/AudioFormat");
    jmethodID jGetFrameSizeInBytes = env->GetMethodID(
            jAudioFormatCls, "getFrameSizeInBytes", "()I");
    jint javaFrameSizeInBytes = env->CallIntMethod(jAudioFormatObj, jGetFrameSizeInBytes);

    return (size_t)javaFrameSizeInBytes;
}

status_t JAudioTrack::dump(int fd, const Vector<String16>& args __unused) const
{
    String8 result;

    result.append(" JAudioTrack::dump\n");

    // TODO: Remove logs that includes unavailable information from below.
//    result.appendFormat("  status(%d), state(%d), session Id(%d), flags(%#x)\n",
//                        mStatus, mState, mSessionId, mFlags);
//    result.appendFormat("  format(%#x), channel mask(%#x), channel count(%u)\n",
//                  format(), mChannelMask, channelCount());
//    result.appendFormat("  sample rate(%u), original sample rate(%u), speed(%f)\n",
//            getSampleRate(), mOriginalSampleRate, mPlaybackRate.mSpeed);
//    result.appendFormat("  frame count(%zu), req. frame count(%zu)\n",
//                  frameCount(), mReqFrameCount);
//    result.appendFormat("  notif. frame count(%u), req. notif. frame count(%u),"
//            " req. notif. per buff(%u)\n",
//             mNotificationFramesAct, mNotificationFramesReq, mNotificationsPerBufferReq);
//    result.appendFormat("  latency (%d), selected device Id(%d), routed device Id(%d)\n",
//                        latency(), mSelectedDeviceId, getRoutedDeviceId());
//    result.appendFormat("  output(%d) AF latency (%u) AF frame count(%zu) AF SampleRate(%u)\n",
//                        mOutput, mAfLatency, mAfFrameCount, mAfSampleRate);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

jobject JAudioTrack::getRoutedDevice() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetRoutedDevice = env->GetMethodID(mAudioTrackCls, "getRoutedDevice",
            "()Landroid/media/AudioDeviceInfo;");
    return env->CallObjectMethod(mAudioTrackObj, jGetRoutedDevice);
}

int32_t JAudioTrack::getAudioSessionId() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetAudioSessionId = env->GetMethodID(mAudioTrackCls, "getAudioSessionId", "()I");
    jint sessionId = env->CallIntMethod(mAudioTrackObj, jGetAudioSessionId);
    return sessionId;
}

status_t JAudioTrack::setPreferredDevice(jobject device) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jSetPreferredDeviceId = env->GetMethodID(mAudioTrackCls, "setPreferredDevice",
            "(Landroid/media/AudioDeviceInfo;)Z");
    jboolean result = env->CallBooleanMethod(mAudioTrackObj, jSetPreferredDeviceId, device);
    return result == true ? NO_ERROR : BAD_VALUE;
}

audio_stream_type_t JAudioTrack::getAudioStreamType() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jGetAudioAttributes = env->GetMethodID(mAudioTrackCls, "getAudioAttributes",
            "()Landroid/media/AudioAttributes;");
    jobject jAudioAttributes = env->CallObjectMethod(mAudioTrackObj, jGetAudioAttributes);
    jclass jAudioAttributesCls = env->FindClass("android/media/AudioAttributes");
    jmethodID jGetVolumeControlStream = env->GetMethodID(jAudioAttributesCls,
            "getVolumeControlStream", "()I");
    int javaAudioStreamType = env->CallIntMethod(jAudioAttributes, jGetVolumeControlStream);
    return (audio_stream_type_t)javaAudioStreamType;
}

status_t JAudioTrack::pendingDuration(int32_t *msec) {
    if (msec == nullptr) {
        return BAD_VALUE;
    }

    bool isPurePcmData = audio_is_linear_pcm(format()) && (getFlags() & AUDIO_FLAG_HW_AV_SYNC) == 0;
    if (!isPurePcmData) {
        return INVALID_OPERATION;
    }

    // TODO: Need to know the difference btw. client and server time.
    // If getTimestamp(ExtendedTimestamp) is ready, and un-comment below and modify appropriately.
    // (copied from AudioTrack.cpp)

//    ExtendedTimestamp ets;
//    ExtendedTimestamp::LOCATION location = ExtendedTimestamp::LOCATION_SERVER;
//    if (getTimestamp_l(&ets) == OK && ets.mTimeNs[location] > 0) {
//        int64_t diff = ets.mPosition[ExtendedTimestamp::LOCATION_CLIENT]
//                - ets.mPosition[location];
//        if (diff < 0) {
//            *msec = 0;
//        } else {
//            // ms is the playback time by frames
//            int64_t ms = (int64_t)((double)diff * 1000 /
//                    ((double)mSampleRate * mPlaybackRate.mSpeed));
//            // clockdiff is the timestamp age (negative)
//            int64_t clockdiff = (mState != STATE_ACTIVE) ? 0 :
//                    ets.mTimeNs[location]
//                    + ets.mTimebaseOffset[ExtendedTimestamp::TIMEBASE_MONOTONIC]
//                    - systemTime(SYSTEM_TIME_MONOTONIC);
//
//            //ALOGV("ms: %lld  clockdiff: %lld", (long long)ms, (long long)clockdiff);
//            static const int NANOS_PER_MILLIS = 1000000;
//            *msec = (int32_t)(ms + clockdiff / NANOS_PER_MILLIS);
//        }
//        return NO_ERROR;
//    }

    return NO_ERROR;
}

status_t JAudioTrack::addAudioDeviceCallback(jobject listener, jobject handler) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jAddOnRoutingChangedListener = env->GetMethodID(mAudioTrackCls,
            "addOnRoutingChangedListener",
            "(Landroid/media/AudioRouting$OnRoutingChangedListener;Landroid/os/Handler;)V");
    env->CallVoidMethod(mAudioTrackObj, jAddOnRoutingChangedListener, listener, handler);
    return NO_ERROR;
}

status_t JAudioTrack::removeAudioDeviceCallback(jobject listener) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jmethodID jRemoveOnRoutingChangedListener = env->GetMethodID(mAudioTrackCls,
            "removeOnRoutingChangedListener",
            "(Landroid/media/AudioRouting$OnRoutingChangedListener;)V");
    env->CallVoidMethod(mAudioTrackObj, jRemoveOnRoutingChangedListener, listener);
    return NO_ERROR;
}

void JAudioTrack::registerRoutingDelegates(
        Vector<std::pair<sp<JObjectHolder>, sp<JObjectHolder>>>& routingDelegates) {
    for (auto it = routingDelegates.begin(); it != routingDelegates.end(); it++) {
        addAudioDeviceCallback(it->second->getJObject(), getHandler(it->second->getJObject()));
    }
}

/////////////////////////////////////////////////////////////
///                Static methods begin                   ///
/////////////////////////////////////////////////////////////
jobject JAudioTrack::getListener(const jobject routingDelegateObj) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jclass jRoutingDelegateCls = env->FindClass("android/media/RoutingDelegate");
    jmethodID jGetListener = env->GetMethodID(jRoutingDelegateCls,
            "getListener", "()Landroid/media/AudioRouting$OnRoutingChangedListener;");
    return env->CallObjectMethod(routingDelegateObj, jGetListener);
}

jobject JAudioTrack::getHandler(const jobject routingDelegateObj) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jclass jRoutingDelegateCls = env->FindClass("android/media/RoutingDelegate");
    jmethodID jGetHandler = env->GetMethodID(jRoutingDelegateCls,
        "getHandler", "()Landroid/os/Handler;");
    return env->CallObjectMethod(routingDelegateObj, jGetHandler);
}

jobject JAudioTrack::findByKey(
        Vector<std::pair<sp<JObjectHolder>, sp<JObjectHolder>>>& mp, const jobject key) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    for (auto it = mp.begin(); it != mp.end(); it++) {
        if (env->IsSameObject(it->first->getJObject(), key)) {
            return it->second->getJObject();
        }
    }
    return nullptr;
}

void JAudioTrack::eraseByKey(
        Vector<std::pair<sp<JObjectHolder>, sp<JObjectHolder>>>& mp, const jobject key) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    for (auto it = mp.begin(); it != mp.end(); it++) {
        if (env->IsSameObject(it->first->getJObject(), key)) {
            mp.erase(it);
            return;
        }
    }
}

/////////////////////////////////////////////////////////////
///                Private method begins                  ///
/////////////////////////////////////////////////////////////

jobject JAudioTrack::createVolumeShaperConfigurationObj(
        const sp<media::VolumeShaper::Configuration>& config) {

    // TODO: Java VolumeShaper's setId() / setOptionFlags() are hidden.
    if (config == NULL || config->getType() == media::VolumeShaper::Configuration::TYPE_ID) {
        return NULL;
    }

    JNIEnv *env = JavaVMHelper::getJNIEnv();

    // Referenced "android_media_VolumeShaper.h".
    jfloatArray xarray = nullptr;
    jfloatArray yarray = nullptr;
    if (config->getType() == media::VolumeShaper::Configuration::TYPE_SCALE) {
        // convert curve arrays
        xarray = env->NewFloatArray(config->size());
        yarray = env->NewFloatArray(config->size());
        float * const x = env->GetFloatArrayElements(xarray, nullptr /* isCopy */);
        float * const y = env->GetFloatArrayElements(yarray, nullptr /* isCopy */);
        float *xptr = x, *yptr = y;
        for (const auto &pt : *config.get()) {
            *xptr++ = pt.first;
            *yptr++ = pt.second;
        }
        env->ReleaseFloatArrayElements(xarray, x, 0 /* mode */);
        env->ReleaseFloatArrayElements(yarray, y, 0 /* mode */);
    }

    jclass jBuilderCls = env->FindClass("android/media/VolumeShaper$Configuration$Builder");
    jmethodID jBuilderCtor = env->GetMethodID(jBuilderCls, "<init>", "()V");
    jobject jBuilderObj = env->NewObject(jBuilderCls, jBuilderCtor);

    jmethodID jSetDuration = env->GetMethodID(jBuilderCls, "setDuration",
            "(L)Landroid/media/VolumeShaper$Configuration$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderCls, jSetDuration, (jlong) config->getDurationMs());

    jmethodID jSetInterpolatorType = env->GetMethodID(jBuilderCls, "setInterpolatorType",
            "(I)Landroid/media/VolumeShaper$Configuration$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderCls, jSetInterpolatorType,
            config->getInterpolatorType());

    jmethodID jSetCurve = env->GetMethodID(jBuilderCls, "setCurve",
            "([F[F)Landroid/media/VolumeShaper$Configuration$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderCls, jSetCurve, xarray, yarray);

    jmethodID jBuild = env->GetMethodID(jBuilderCls, "build",
            "()Landroid/media/VolumeShaper$Configuration;");
    return env->CallObjectMethod(jBuilderObj, jBuild);
}

jobject JAudioTrack::createVolumeShaperOperationObj(
        const sp<media::VolumeShaper::Operation>& operation) {

    JNIEnv *env = JavaVMHelper::getJNIEnv();

    jclass jBuilderCls = env->FindClass("android/media/VolumeShaper$Operation$Builder");
    jmethodID jBuilderCtor = env->GetMethodID(jBuilderCls, "<init>", "()V");
    jobject jBuilderObj = env->NewObject(jBuilderCls, jBuilderCtor);

    // Set XOffset
    jmethodID jSetXOffset = env->GetMethodID(jBuilderCls, "setXOffset",
            "(F)Landroid/media/VolumeShaper$Operation$Builder;");
    jBuilderObj = env->CallObjectMethod(jBuilderCls, jSetXOffset, operation->getXOffset());

    int32_t flags = operation->getFlags();

    if (operation->getReplaceId() >= 0) {
        jmethodID jReplace = env->GetMethodID(jBuilderCls, "replace",
                "(IB)Landroid/media/VolumeShaper$Operation$Builder;");
        bool join = (flags | media::VolumeShaper::Operation::FLAG_JOIN) != 0;
        jBuilderObj = env->CallObjectMethod(jBuilderCls, jReplace, operation->getReplaceId(), join);
    }

    if (flags | media::VolumeShaper::Operation::FLAG_REVERSE) {
        jmethodID jReverse = env->GetMethodID(jBuilderCls, "reverse",
                "()Landroid/media/VolumeShaper$Operation$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderCls, jReverse);
    }

    // TODO: VolumeShaper Javadoc says "Do not call terminate() directly". Can we call this?
    if (flags | media::VolumeShaper::Operation::FLAG_TERMINATE) {
        jmethodID jTerminate = env->GetMethodID(jBuilderCls, "terminate",
                "()Landroid/media/VolumeShaper$Operation$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderCls, jTerminate);
    }

    if (flags | media::VolumeShaper::Operation::FLAG_DELAY) {
        jmethodID jDefer = env->GetMethodID(jBuilderCls, "defer",
                "()Landroid/media/VolumeShaper$Operation$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderCls, jDefer);
    }

    if (flags | media::VolumeShaper::Operation::FLAG_CREATE_IF_NECESSARY) {
        jmethodID jCreateIfNeeded = env->GetMethodID(jBuilderCls, "createIfNeeded",
                "()Landroid/media/VolumeShaper$Operation$Builder;");
        jBuilderObj = env->CallObjectMethod(jBuilderCls, jCreateIfNeeded);
    }

    // TODO: Handle error case (can it be NULL?)
    jmethodID jBuild = env->GetMethodID(jBuilderCls, "build",
            "()Landroid/media/VolumeShaper$Operation;");
    return env->CallObjectMethod(jBuilderObj, jBuild);
}

jobject JAudioTrack::createStreamEventCallback(callback_t cbf, void* user) {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jclass jCallbackCls = env->FindClass("android/media/MediaPlayer2$StreamEventCallback");
    jmethodID jCallbackCtor = env->GetMethodID(jCallbackCls, "<init>", "(JJJ)V");
    jobject jCallbackObj = env->NewObject(jCallbackCls, jCallbackCtor, this, cbf, user);
    return jCallbackObj;
}

jobject JAudioTrack::createCallbackExecutor() {
    JNIEnv *env = JavaVMHelper::getJNIEnv();
    jclass jExecutorsCls = env->FindClass("java/util/concurrent/Executors");
    jmethodID jNewSingleThreadExecutor = env->GetStaticMethodID(jExecutorsCls,
            "newSingleThreadExecutor", "()Ljava/util/concurrent/ExecutorService;");
    jobject jSingleThreadExecutorObj =
            env->CallStaticObjectMethod(jExecutorsCls, jNewSingleThreadExecutor);
    return jSingleThreadExecutorObj;
}

status_t JAudioTrack::javaToNativeStatus(int javaStatus) {
    switch (javaStatus) {
    case AUDIO_JAVA_SUCCESS:
        return NO_ERROR;
    case AUDIO_JAVA_BAD_VALUE:
        return BAD_VALUE;
    case AUDIO_JAVA_INVALID_OPERATION:
        return INVALID_OPERATION;
    case AUDIO_JAVA_PERMISSION_DENIED:
        return PERMISSION_DENIED;
    case AUDIO_JAVA_NO_INIT:
        return NO_INIT;
    case AUDIO_JAVA_WOULD_BLOCK:
        return WOULD_BLOCK;
    case AUDIO_JAVA_DEAD_OBJECT:
        return DEAD_OBJECT;
    default:
        return UNKNOWN_ERROR;
    }
}

} // namespace android
