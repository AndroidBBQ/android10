/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include <sensor/ISensorEventConnection.h>

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <sensor/BitTube.h>

namespace android {
// ----------------------------------------------------------------------------

enum {
    GET_SENSOR_CHANNEL = IBinder::FIRST_CALL_TRANSACTION,
    ENABLE_DISABLE,
    SET_EVENT_RATE,
    FLUSH_SENSOR,
    CONFIGURE_CHANNEL,
    DESTROY,
};

class BpSensorEventConnection : public BpInterface<ISensorEventConnection>
{
public:
    explicit BpSensorEventConnection(const sp<IBinder>& impl)
        : BpInterface<ISensorEventConnection>(impl)
    {
    }

    virtual ~BpSensorEventConnection();

    virtual sp<BitTube> getSensorChannel() const
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
        remote()->transact(GET_SENSOR_CHANNEL, data, &reply);
        return new BitTube(reply);
    }

    virtual status_t enableDisable(int handle, bool enabled, nsecs_t samplingPeriodNs,
                                   nsecs_t maxBatchReportLatencyNs, int reservedFlags)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
        data.writeInt32(handle);
        data.writeInt32(enabled);
        data.writeInt64(samplingPeriodNs);
        data.writeInt64(maxBatchReportLatencyNs);
        data.writeInt32(reservedFlags);
        remote()->transact(ENABLE_DISABLE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setEventRate(int handle, nsecs_t ns)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
        data.writeInt32(handle);
        data.writeInt64(ns);
        remote()->transact(SET_EVENT_RATE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t flush() {
        Parcel data, reply;
        data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
        remote()->transact(FLUSH_SENSOR, data, &reply);
        return reply.readInt32();
    }

    virtual int32_t configureChannel(int32_t handle, int32_t rateLevel) {
        Parcel data, reply;
        data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
        data.writeInt32(handle);
        data.writeInt32(rateLevel);
        remote()->transact(CONFIGURE_CHANNEL, data, &reply);
        return reply.readInt32();
    }

    virtual void onLastStrongRef(const void* id) {
        destroy();
        BpInterface<ISensorEventConnection>::onLastStrongRef(id);
    }

protected:
    virtual void destroy() {
        Parcel data, reply;
        remote()->transact(DESTROY, data, &reply);
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this
// translation unit (see clang warning -Wweak-vtables)
BpSensorEventConnection::~BpSensorEventConnection() { }

IMPLEMENT_META_INTERFACE(SensorEventConnection, "android.gui.SensorEventConnection");

// ----------------------------------------------------------------------------

status_t BnSensorEventConnection::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_SENSOR_CHANNEL: {
            CHECK_INTERFACE(ISensorEventConnection, data, reply);
            sp<BitTube> channel(getSensorChannel());
            channel->writeToParcel(reply);
            return NO_ERROR;
        }
        case ENABLE_DISABLE: {
            CHECK_INTERFACE(ISensorEventConnection, data, reply);
            int handle = data.readInt32();
            int enabled = data.readInt32();
            nsecs_t samplingPeriodNs = data.readInt64();
            nsecs_t maxBatchReportLatencyNs = data.readInt64();
            int reservedFlags = data.readInt32();
            status_t result = enableDisable(handle, enabled, samplingPeriodNs,
                                            maxBatchReportLatencyNs, reservedFlags);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case SET_EVENT_RATE: {
            CHECK_INTERFACE(ISensorEventConnection, data, reply);
            int handle = data.readInt32();
            nsecs_t ns = data.readInt64();
            status_t result = setEventRate(handle, ns);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case FLUSH_SENSOR: {
            CHECK_INTERFACE(ISensorEventConnection, data, reply);
            status_t result = flush();
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case CONFIGURE_CHANNEL: {
            CHECK_INTERFACE(ISensorEventConnection, data, reply);
            int handle = data.readInt32();
            int rateLevel = data.readInt32();
            status_t result = configureChannel(handle, rateLevel);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case DESTROY: {
            destroy();
            return NO_ERROR;
        }

    }
    return BBinder::onTransact(code, data, reply, flags);
}

// ----------------------------------------------------------------------------
}; // namespace android
