/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <binder/IBatteryStats.h>

#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include <private/binder/Static.h>

namespace android {

// ----------------------------------------------------------------------

class BpBatteryStats : public BpInterface<IBatteryStats>
{
public:
    explicit BpBatteryStats(const sp<IBinder>& impl)
        : BpInterface<IBatteryStats>(impl)
    {
    }

    virtual void noteStartSensor(int uid, int sensor) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        data.writeInt32(sensor);
        remote()->transact(NOTE_START_SENSOR_TRANSACTION, data, &reply);
    }

    virtual void noteStopSensor(int uid, int sensor) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        data.writeInt32(sensor);
        remote()->transact(NOTE_STOP_SENSOR_TRANSACTION, data, &reply);
    }

    virtual void noteStartVideo(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_START_VIDEO_TRANSACTION, data, &reply);
    }

    virtual void noteStopVideo(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_STOP_VIDEO_TRANSACTION, data, &reply);
    }

    virtual void noteStartAudio(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_START_AUDIO_TRANSACTION, data, &reply);
    }

    virtual void noteStopAudio(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_STOP_AUDIO_TRANSACTION, data, &reply);
    }

    virtual void noteResetVideo() {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        remote()->transact(NOTE_RESET_VIDEO_TRANSACTION, data, &reply);
    }

    virtual void noteResetAudio() {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        remote()->transact(NOTE_RESET_AUDIO_TRANSACTION, data, &reply);
    }

    virtual void noteFlashlightOn(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_FLASHLIGHT_ON_TRANSACTION, data, &reply);
    }

    virtual void noteFlashlightOff(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_FLASHLIGHT_OFF_TRANSACTION, data, &reply);
    }

    virtual void noteStartCamera(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_START_CAMERA_TRANSACTION, data, &reply);
    }

    virtual void noteStopCamera(int uid) {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        data.writeInt32(uid);
        remote()->transact(NOTE_STOP_CAMERA_TRANSACTION, data, &reply);
    }

    virtual void noteResetCamera() {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        remote()->transact(NOTE_RESET_CAMERA_TRANSACTION, data, &reply);
    }

    virtual void noteResetFlashlight() {
        Parcel data, reply;
        data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
        remote()->transact(NOTE_RESET_FLASHLIGHT_TRANSACTION, data, &reply);
    }

};

IMPLEMENT_META_INTERFACE(BatteryStats, "com.android.internal.app.IBatteryStats");

// ----------------------------------------------------------------------

// NOLINTNEXTLINE(google-default-arguments)
status_t BnBatteryStats::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case NOTE_START_SENSOR_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            int sensor = data.readInt32();
            noteStartSensor(uid, sensor);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_STOP_SENSOR_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            int sensor = data.readInt32();
            noteStopSensor(uid, sensor);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_START_VIDEO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStartVideo(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_STOP_VIDEO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStopVideo(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_START_AUDIO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStartAudio(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_STOP_AUDIO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStopAudio(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_RESET_VIDEO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            noteResetVideo();
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_RESET_AUDIO_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            noteResetAudio();
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_FLASHLIGHT_ON_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteFlashlightOn(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_FLASHLIGHT_OFF_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteFlashlightOff(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_START_CAMERA_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStartCamera(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_STOP_CAMERA_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            int uid = data.readInt32();
            noteStopCamera(uid);
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_RESET_CAMERA_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            noteResetCamera();
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case NOTE_RESET_FLASHLIGHT_TRANSACTION: {
            CHECK_INTERFACE(IBatteryStats, data, reply);
            noteResetFlashlight();
            reply->writeNoException();
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
