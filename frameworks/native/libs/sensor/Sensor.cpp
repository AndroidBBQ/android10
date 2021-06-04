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

#include <sensor/Sensor.h>

#include <inttypes.h>

#include <binder/AppOpsManager.h>
#include <binder/IPermissionController.h>
#include <binder/IServiceManager.h>

/*
 * The permission to use for activity recognition sensors (like step counter).
 * See sensor types for more details on what sensors should require this
 * permission.
 */
#define SENSOR_PERMISSION_ACTIVITY_RECOGNITION "android.permission.ACTIVITY_RECOGNITION"

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

Sensor::Sensor(const char * name) :
        mName(name), mHandle(0), mType(0),
        mMinValue(0), mMaxValue(0), mResolution(0),
        mPower(0), mMinDelay(0), mVersion(0), mFifoReservedEventCount(0),
        mFifoMaxEventCount(0), mRequiredAppOp(-1),
        mMaxDelay(0), mFlags(0) {
}

Sensor::Sensor(struct sensor_t const* hwSensor, int halVersion) :
        Sensor(*hwSensor, uuid_t(), halVersion) {
}

Sensor::Sensor(struct sensor_t const& hwSensor, const uuid_t& uuid, int halVersion) :
        Sensor("") {
    mName = hwSensor.name;
    mVendor = hwSensor.vendor;
    mVersion = hwSensor.version;
    mHandle = hwSensor.handle;
    mType = hwSensor.type;
    mMinValue = 0;                      // FIXME: minValue
    mMaxValue = hwSensor.maxRange;      // FIXME: maxValue
    mResolution = hwSensor.resolution;
    mPower = hwSensor.power;
    mMinDelay = hwSensor.minDelay;
    mFlags = 0;
    mUuid = uuid;

    // Set fifo event count zero for older devices which do not support batching. Fused
    // sensors also have their fifo counts set to zero.
    if (halVersion > SENSORS_DEVICE_API_VERSION_1_0) {
        mFifoReservedEventCount = hwSensor.fifoReservedEventCount;
        mFifoMaxEventCount = hwSensor.fifoMaxEventCount;
    } else {
        mFifoReservedEventCount = 0;
        mFifoMaxEventCount = 0;
    }

    if (halVersion >= SENSORS_DEVICE_API_VERSION_1_3) {
        if (hwSensor.maxDelay > INT_MAX) {
            // Max delay is declared as a 64 bit integer for 64 bit architectures. But it should
            // always fit in a 32 bit integer, log error and cap it to INT_MAX.
            ALOGE("Sensor maxDelay overflow error %s %" PRId64, mName.string(),
                  static_cast<int64_t>(hwSensor.maxDelay));
            mMaxDelay = INT_MAX;
        } else {
            mMaxDelay = static_cast<int32_t>(hwSensor.maxDelay);
        }
    } else {
        // For older hals set maxDelay to 0.
        mMaxDelay = 0;
    }

    // Ensure existing sensors have correct string type, required permissions and reporting mode.
    // Set reportingMode for all android defined sensor types, set wake-up flag only for proximity
    // sensor, significant motion, tilt, pick_up gesture, wake gesture and glance gesture on older
    // HALs. Newer HALs can define both wake-up and non wake-up proximity sensors.
    // All the OEM defined defined sensors have flags set to whatever is provided by the HAL.
    switch (mType) {
    case SENSOR_TYPE_ACCELEROMETER:
        mStringType = SENSOR_STRING_TYPE_ACCELEROMETER;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_AMBIENT_TEMPERATURE:
        mStringType = SENSOR_STRING_TYPE_AMBIENT_TEMPERATURE;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        break;
    case SENSOR_TYPE_GAME_ROTATION_VECTOR:
        mStringType = SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
        mStringType = SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_GRAVITY:
        mStringType = SENSOR_STRING_TYPE_GRAVITY;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_GYROSCOPE:
        mStringType = SENSOR_STRING_TYPE_GYROSCOPE;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
        mStringType = SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_HEART_RATE: {
        mStringType = SENSOR_STRING_TYPE_HEART_RATE;
        mRequiredPermission = SENSOR_PERMISSION_BODY_SENSORS;
        AppOpsManager appOps;
        mRequiredAppOp = appOps.permissionToOpCode(String16(mRequiredPermission));
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        } break;
    case SENSOR_TYPE_LIGHT:
        mStringType = SENSOR_STRING_TYPE_LIGHT;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        break;
    case SENSOR_TYPE_LINEAR_ACCELERATION:
        mStringType = SENSOR_STRING_TYPE_LINEAR_ACCELERATION;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_MAGNETIC_FIELD:
        mStringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
        mStringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_ORIENTATION:
        mStringType = SENSOR_STRING_TYPE_ORIENTATION;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_PRESSURE:
        mStringType = SENSOR_STRING_TYPE_PRESSURE;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_PROXIMITY:
        mStringType = SENSOR_STRING_TYPE_PROXIMITY;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_RELATIVE_HUMIDITY:
        mStringType = SENSOR_STRING_TYPE_RELATIVE_HUMIDITY;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        break;
    case SENSOR_TYPE_ROTATION_VECTOR:
        mStringType = SENSOR_STRING_TYPE_ROTATION_VECTOR;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_SIGNIFICANT_MOTION:
        mStringType = SENSOR_STRING_TYPE_SIGNIFICANT_MOTION;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_STEP_COUNTER: {
        mStringType = SENSOR_STRING_TYPE_STEP_COUNTER;
        mRequiredPermission = SENSOR_PERMISSION_ACTIVITY_RECOGNITION;
        AppOpsManager appOps;
        mRequiredAppOp =
                appOps.permissionToOpCode(String16(mRequiredPermission));
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        } break;
    case SENSOR_TYPE_STEP_DETECTOR: {
        mStringType = SENSOR_STRING_TYPE_STEP_DETECTOR;
        mRequiredPermission = SENSOR_PERMISSION_ACTIVITY_RECOGNITION;
        AppOpsManager appOps;
        mRequiredAppOp =
                appOps.permissionToOpCode(String16(mRequiredPermission));
        mFlags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE;
        } break;
    case SENSOR_TYPE_TEMPERATURE:
        mStringType = SENSOR_STRING_TYPE_TEMPERATURE;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        break;
    case SENSOR_TYPE_TILT_DETECTOR:
        mStringType = SENSOR_STRING_TYPE_TILT_DETECTOR;
        mFlags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_WAKE_GESTURE:
        mStringType = SENSOR_STRING_TYPE_WAKE_GESTURE;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_GLANCE_GESTURE:
        mStringType = SENSOR_STRING_TYPE_GLANCE_GESTURE;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_PICK_UP_GESTURE:
        mStringType = SENSOR_STRING_TYPE_PICK_UP_GESTURE;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT:
        mStringType = SENSOR_STRING_TYPE_LOW_LATENCY_OFFBODY_DETECT;
        mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
        break;
    case SENSOR_TYPE_WRIST_TILT_GESTURE:
        mStringType = SENSOR_STRING_TYPE_WRIST_TILT_GESTURE;
        mFlags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_DYNAMIC_SENSOR_META:
        mStringType = SENSOR_STRING_TYPE_DYNAMIC_SENSOR_META;
        mFlags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE; // special trigger
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_POSE_6DOF:
        mStringType = SENSOR_STRING_TYPE_POSE_6DOF;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    case SENSOR_TYPE_STATIONARY_DETECT:
        mStringType = SENSOR_STRING_TYPE_STATIONARY_DETECT;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_MOTION_DETECT:
        mStringType = SENSOR_STRING_TYPE_MOTION_DETECT;
        mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
        if (halVersion < SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags |= SENSOR_FLAG_WAKE_UP;
        }
        break;
    case SENSOR_TYPE_HEART_BEAT:
        mStringType = SENSOR_STRING_TYPE_HEART_BEAT;
        mFlags |= SENSOR_FLAG_SPECIAL_REPORTING_MODE;
        break;

    // TODO:  Placeholder for LLOB sensor type


    case SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED:
        mStringType = SENSOR_STRING_TYPE_ACCELEROMETER_UNCALIBRATED;
        mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
        break;
    default:
        // Only pipe the stringType, requiredPermission and flags for custom sensors.
        if (halVersion > SENSORS_DEVICE_API_VERSION_1_0 && hwSensor.stringType) {
            mStringType = hwSensor.stringType;
        }
        if (halVersion > SENSORS_DEVICE_API_VERSION_1_0 && hwSensor.requiredPermission) {
            mRequiredPermission = hwSensor.requiredPermission;
            if (!strcmp(mRequiredPermission, SENSOR_PERMISSION_BODY_SENSORS)) {
                AppOpsManager appOps;
                mRequiredAppOp = appOps.permissionToOpCode(String16(SENSOR_PERMISSION_BODY_SENSORS));
            }
        }

        if (halVersion >= SENSORS_DEVICE_API_VERSION_1_3) {
            mFlags = static_cast<uint32_t>(hwSensor.flags);
        } else {
            // This is an OEM defined sensor on an older HAL. Use minDelay to determine the
            // reporting mode of the sensor.
            if (mMinDelay > 0) {
                mFlags |= SENSOR_FLAG_CONTINUOUS_MODE;
            } else if (mMinDelay == 0) {
                mFlags |= SENSOR_FLAG_ON_CHANGE_MODE;
            } else if (mMinDelay < 0) {
                mFlags |= SENSOR_FLAG_ONE_SHOT_MODE;
            }
        }
        break;
    }

    if (halVersion >= SENSORS_DEVICE_API_VERSION_1_3) {
        // Wake-up flag of HAL 1.3 and above is set here
        mFlags |= (hwSensor.flags & SENSOR_FLAG_WAKE_UP);

        // Log error if the reporting mode is not as expected, but respect HAL setting.
        int actualReportingMode = (hwSensor.flags & REPORTING_MODE_MASK) >> REPORTING_MODE_SHIFT;
        int expectedReportingMode = (mFlags & REPORTING_MODE_MASK) >> REPORTING_MODE_SHIFT;
        if (actualReportingMode != expectedReportingMode) {
            ALOGE("Reporting Mode incorrect: sensor %s handle=%#010" PRIx32 " type=%" PRId32 " "
                   "actual=%d expected=%d",
                   mName.string(), mHandle, mType, actualReportingMode, expectedReportingMode);
        }
    }

    // Feature flags
    // Set DYNAMIC_SENSOR_MASK and ADDITIONAL_INFO_MASK flag here. Compatible with HAL 1_3.
    if (halVersion >= SENSORS_DEVICE_API_VERSION_1_3) {
        mFlags |= hwSensor.flags & (DYNAMIC_SENSOR_MASK | ADDITIONAL_INFO_MASK);
    }
    // Set DIRECT_REPORT_MASK and DIRECT_CHANNEL_MASK flags. Compatible with HAL 1_3.
    if (halVersion >= SENSORS_DEVICE_API_VERSION_1_3) {
        // only on continuous sensors direct report mode is defined
        if ((mFlags & REPORTING_MODE_MASK) == SENSOR_FLAG_CONTINUOUS_MODE) {
            mFlags |= hwSensor.flags
                & (SENSOR_FLAG_MASK_DIRECT_REPORT | SENSOR_FLAG_MASK_DIRECT_CHANNEL);
        }
    }
    // Set DATA_INJECTION flag here. Defined in HAL 1_4.
    if (halVersion >= SENSORS_DEVICE_API_VERSION_1_4) {
        mFlags |= (hwSensor.flags & DATA_INJECTION_MASK);
    }

    if (mRequiredPermission.length() > 0) {
        // If the sensor is protected by a permission we need to know if it is
        // a runtime one to determine whether we can use the permission cache.
        sp<IBinder> binder = defaultServiceManager()->getService(String16("permission"));
        if (binder != nullptr) {
            sp<IPermissionController> permCtrl = interface_cast<IPermissionController>(binder);
            mRequiredPermissionRuntime = permCtrl->isRuntimePermission(
                    String16(mRequiredPermission));
        }
    }
}

Sensor::~Sensor() {
}

const String8& Sensor::getName() const {
    return mName;
}

const String8& Sensor::getVendor() const {
    return mVendor;
}

int32_t Sensor::getHandle() const {
    return mHandle;
}

int32_t Sensor::getType() const {
    return mType;
}

float Sensor::getMinValue() const {
    return mMinValue;
}

float Sensor::getMaxValue() const {
    return mMaxValue;
}

float Sensor::getResolution() const {
    return mResolution;
}

float Sensor::getPowerUsage() const {
    return mPower;
}

int32_t Sensor::getMinDelay() const {
    return mMinDelay;
}

nsecs_t Sensor::getMinDelayNs() const {
    return getMinDelay() * 1000;
}

int32_t Sensor::getVersion() const {
    return mVersion;
}

uint32_t Sensor::getFifoReservedEventCount() const {
    return mFifoReservedEventCount;
}

uint32_t Sensor::getFifoMaxEventCount() const {
    return mFifoMaxEventCount;
}

const String8& Sensor::getStringType() const {
    return mStringType;
}

const String8& Sensor::getRequiredPermission() const {
    return mRequiredPermission;
}

bool Sensor::isRequiredPermissionRuntime() const {
    return mRequiredPermissionRuntime;
}

int32_t Sensor::getRequiredAppOp() const {
    return mRequiredAppOp;
}

int32_t Sensor::getMaxDelay() const {
    return mMaxDelay;
}

uint32_t Sensor::getFlags() const {
    return mFlags;
}

bool Sensor::isWakeUpSensor() const {
    return (mFlags & SENSOR_FLAG_WAKE_UP) != 0;
}

bool Sensor::isDynamicSensor() const {
    return (mFlags & SENSOR_FLAG_DYNAMIC_SENSOR) != 0;
}

bool Sensor::isDataInjectionSupported() const {
    return (mFlags & SENSOR_FLAG_DATA_INJECTION) != 0;
}

bool Sensor::hasAdditionalInfo() const {
    return (mFlags & SENSOR_FLAG_ADDITIONAL_INFO) != 0;
}

int32_t Sensor::getHighestDirectReportRateLevel() const {
    return ((mFlags & SENSOR_FLAG_MASK_DIRECT_REPORT) >> SENSOR_FLAG_SHIFT_DIRECT_REPORT);
}

bool Sensor::isDirectChannelTypeSupported(int32_t sharedMemType) const {
    switch (sharedMemType) {
        case SENSOR_DIRECT_MEM_TYPE_ASHMEM:
            return mFlags & SENSOR_FLAG_DIRECT_CHANNEL_ASHMEM;
        case SENSOR_DIRECT_MEM_TYPE_GRALLOC:
            return mFlags & SENSOR_FLAG_DIRECT_CHANNEL_GRALLOC;
        default:
            return false;
    }
}

int32_t Sensor::getReportingMode() const {
    return ((mFlags & REPORTING_MODE_MASK) >> REPORTING_MODE_SHIFT);
}

const Sensor::uuid_t& Sensor::getUuid() const {
    return mUuid;
}

void Sensor::setId(int32_t id) {
    mUuid.i64[0] = id;
    mUuid.i64[1] = 0;
}

int32_t Sensor::getId() const {
    return int32_t(mUuid.i64[0]);
}

size_t Sensor::getFlattenedSize() const {
    size_t fixedSize =
            sizeof(mVersion) + sizeof(mHandle) + sizeof(mType) +
            sizeof(mMinValue) + sizeof(mMaxValue) + sizeof(mResolution) +
            sizeof(mPower) + sizeof(mMinDelay) + sizeof(mFifoMaxEventCount) +
            sizeof(mFifoMaxEventCount) + sizeof(mRequiredPermissionRuntime) +
            sizeof(mRequiredAppOp) + sizeof(mMaxDelay) + sizeof(mFlags) + sizeof(mUuid);

    size_t variableSize =
            sizeof(uint32_t) + FlattenableUtils::align<4>(mName.length()) +
            sizeof(uint32_t) + FlattenableUtils::align<4>(mVendor.length()) +
            sizeof(uint32_t) + FlattenableUtils::align<4>(mStringType.length()) +
            sizeof(uint32_t) + FlattenableUtils::align<4>(mRequiredPermission.length());

    return fixedSize + variableSize;
}

status_t Sensor::flatten(void* buffer, size_t size) const {
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    flattenString8(buffer, size, mName);
    flattenString8(buffer, size, mVendor);
    FlattenableUtils::write(buffer, size, mVersion);
    FlattenableUtils::write(buffer, size, mHandle);
    FlattenableUtils::write(buffer, size, mType);
    FlattenableUtils::write(buffer, size, mMinValue);
    FlattenableUtils::write(buffer, size, mMaxValue);
    FlattenableUtils::write(buffer, size, mResolution);
    FlattenableUtils::write(buffer, size, mPower);
    FlattenableUtils::write(buffer, size, mMinDelay);
    FlattenableUtils::write(buffer, size, mFifoReservedEventCount);
    FlattenableUtils::write(buffer, size, mFifoMaxEventCount);
    flattenString8(buffer, size, mStringType);
    flattenString8(buffer, size, mRequiredPermission);
    FlattenableUtils::write(buffer, size, mRequiredPermissionRuntime);
    FlattenableUtils::write(buffer, size, mRequiredAppOp);
    FlattenableUtils::write(buffer, size, mMaxDelay);
    FlattenableUtils::write(buffer, size, mFlags);
    if (mUuid.i64[1] != 0) {
        // We should never hit this case with our current API, but we
        // could via a careless API change.  If that happens,
        // this code will keep us from leaking our UUID (while probably
        // breaking dynamic sensors).  See b/29547335.
        ALOGW("Sensor with UUID being flattened; sending 0.  Expect "
              "bad dynamic sensor behavior");
        uuid_t tmpUuid;  // default constructor makes this 0.
        FlattenableUtils::write(buffer, size, tmpUuid);
    } else {
        FlattenableUtils::write(buffer, size, mUuid);
    }
    return NO_ERROR;
}

status_t Sensor::unflatten(void const* buffer, size_t size) {
    if (!unflattenString8(buffer, size, mName)) {
        return NO_MEMORY;
    }
    if (!unflattenString8(buffer, size, mVendor)) {
        return NO_MEMORY;
    }

    size_t fixedSize1 =
            sizeof(mVersion) + sizeof(mHandle) + sizeof(mType) + sizeof(mMinValue) +
            sizeof(mMaxValue) + sizeof(mResolution) + sizeof(mPower) + sizeof(mMinDelay) +
            sizeof(mFifoMaxEventCount) + sizeof(mFifoMaxEventCount);
    if (size < fixedSize1) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, mVersion);
    FlattenableUtils::read(buffer, size, mHandle);
    FlattenableUtils::read(buffer, size, mType);
    FlattenableUtils::read(buffer, size, mMinValue);
    FlattenableUtils::read(buffer, size, mMaxValue);
    FlattenableUtils::read(buffer, size, mResolution);
    FlattenableUtils::read(buffer, size, mPower);
    FlattenableUtils::read(buffer, size, mMinDelay);
    FlattenableUtils::read(buffer, size, mFifoReservedEventCount);
    FlattenableUtils::read(buffer, size, mFifoMaxEventCount);

    if (!unflattenString8(buffer, size, mStringType)) {
        return NO_MEMORY;
    }
    if (!unflattenString8(buffer, size, mRequiredPermission)) {
        return NO_MEMORY;
    }

    size_t fixedSize2 =
            sizeof(mRequiredPermissionRuntime) + sizeof(mRequiredAppOp) + sizeof(mMaxDelay) +
            sizeof(mFlags) + sizeof(mUuid);
    if (size < fixedSize2) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, mRequiredPermissionRuntime);
    FlattenableUtils::read(buffer, size, mRequiredAppOp);
    FlattenableUtils::read(buffer, size, mMaxDelay);
    FlattenableUtils::read(buffer, size, mFlags);
    FlattenableUtils::read(buffer, size, mUuid);
    return NO_ERROR;
}

void Sensor::flattenString8(void*& buffer, size_t& size,
        const String8& string8) {
    uint32_t len = static_cast<uint32_t>(string8.length());
    FlattenableUtils::write(buffer, size, len);
    memcpy(static_cast<char*>(buffer), string8.string(), len);
    FlattenableUtils::advance(buffer, size, len);
    size -= FlattenableUtils::align<4>(buffer);
}

bool Sensor::unflattenString8(void const*& buffer, size_t& size, String8& outputString8) {
    uint32_t len;
    if (size < sizeof(len)) {
        return false;
    }
    FlattenableUtils::read(buffer, size, len);
    if (size < len) {
        return false;
    }
    outputString8.setTo(static_cast<char const*>(buffer), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    return true;
}

// ----------------------------------------------------------------------------
}; // namespace android
