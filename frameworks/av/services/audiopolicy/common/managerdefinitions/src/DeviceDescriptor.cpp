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

#define LOG_TAG "APM::Devices"
//#define LOG_NDEBUG 0

#include <audio_utils/string.h>
#include <media/TypeConverter.h>
#include <set>
#include "DeviceDescriptor.h"
#include "TypeConverter.h"
#include "AudioGain.h"
#include "HwModule.h"

namespace android {

DeviceDescriptor::DeviceDescriptor(audio_devices_t type, const String8 &tagName) :
        DeviceDescriptor(type, FormatVector{}, tagName)
{
}

DeviceDescriptor::DeviceDescriptor(audio_devices_t type, const FormatVector &encodedFormats,
        const String8 &tagName) :
    AudioPort(String8(""), AUDIO_PORT_TYPE_DEVICE,
              audio_is_output_device(type) ? AUDIO_PORT_ROLE_SINK :
                                             AUDIO_PORT_ROLE_SOURCE),
    mTagName(tagName), mDeviceType(type), mEncodedFormats(encodedFormats)
{
    mCurrentEncodedFormat = AUDIO_FORMAT_DEFAULT;
    if (audio_is_remote_submix_device(type)) {
        mAddress = String8("0");
    }
    /* If framework runs against a pre 5.0 Audio HAL, encoded formats are absent from the config.
     * FIXME: APM should know the version of the HAL and don't add the formats for V5.0.
     * For now, the workaround to remove AC3 and IEC61937 support on HDMI is to declare
     * something like 'encodedFormats="AUDIO_FORMAT_PCM_16_BIT"' on the HDMI devicePort.
     */
    if (type == AUDIO_DEVICE_OUT_HDMI && mEncodedFormats.isEmpty()) {
        mEncodedFormats.add(AUDIO_FORMAT_AC3);
        mEncodedFormats.add(AUDIO_FORMAT_IEC61937);
    }
}

audio_port_handle_t DeviceDescriptor::getId() const
{
    return mId;
}

void DeviceDescriptor::attach(const sp<HwModule>& module)
{
    AudioPort::attach(module);
    mId = getNextUniqueId();
}

void DeviceDescriptor::detach() {
    mId = AUDIO_PORT_HANDLE_NONE;
    AudioPort::detach();
}

template<typename T>
bool checkEqual(const T& f1, const T& f2)
{
    std::set<typename T::value_type> s1(f1.begin(), f1.end());
    std::set<typename T::value_type> s2(f2.begin(), f2.end());
    return s1 == s2;
}

bool DeviceDescriptor::equals(const sp<DeviceDescriptor>& other) const
{
    // Devices are considered equal if they:
    // - are of the same type (a device type cannot be AUDIO_DEVICE_NONE)
    // - have the same address
    // - have the same encodingFormats (if device supports encoding)
    if (other == 0) {
        return false;
    }

    return (mDeviceType == other->mDeviceType) && (mAddress == other->mAddress) &&
           checkEqual(mEncodedFormats, other->mEncodedFormats);
}

bool DeviceDescriptor::hasCurrentEncodedFormat() const
{
    if (!device_has_encoding_capability(type())) {
        return true;
    }
    if (mEncodedFormats.isEmpty()) {
        return true;
    }

    return (mCurrentEncodedFormat != AUDIO_FORMAT_DEFAULT);
}

bool DeviceDescriptor::supportsFormat(audio_format_t format)
{
    if (mEncodedFormats.isEmpty()) {
        return true;
    }

    for (const auto& devFormat : mEncodedFormats) {
        if (devFormat == format) {
            return true;
        }
    }
    return false;
}

void DeviceVector::refreshTypes()
{
    mDeviceTypes = AUDIO_DEVICE_NONE;
    for (size_t i = 0; i < size(); i++) {
        mDeviceTypes |= itemAt(i)->type();
    }
    ALOGV("DeviceVector::refreshTypes() mDeviceTypes %08x", mDeviceTypes);
}

ssize_t DeviceVector::indexOf(const sp<DeviceDescriptor>& item) const
{
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->equals(item)) { // item may be null sp<>, i.e. AUDIO_DEVICE_NONE
            return i;
        }
    }
    return -1;
}

void DeviceVector::add(const DeviceVector &devices)
{
    bool added = false;
    for (const auto& device : devices) {
        if (indexOf(device) < 0 && SortedVector::add(device) >= 0) {
            added = true;
        }
    }
    if (added) {
        refreshTypes();
    }
}

ssize_t DeviceVector::add(const sp<DeviceDescriptor>& item)
{
    ssize_t ret = indexOf(item);

    if (ret < 0) {
        ret = SortedVector::add(item);
        if (ret >= 0) {
            refreshTypes();
        }
    } else {
        ALOGW("DeviceVector::add device %08x already in", item->type());
        ret = -1;
    }
    return ret;
}

ssize_t DeviceVector::remove(const sp<DeviceDescriptor>& item)
{
    ssize_t ret = indexOf(item);

    if (ret < 0) {
        ALOGW("DeviceVector::remove device %08x not in", item->type());
    } else {
        ret = SortedVector::removeAt(ret);
        if (ret >= 0) {
            refreshTypes();
        }
    }
    return ret;
}

void DeviceVector::remove(const DeviceVector &devices)
{
    for (const auto& device : devices) {
        remove(device);
    }
}

DeviceVector DeviceVector::getDevicesFromHwModule(audio_module_handle_t moduleHandle) const
{
    DeviceVector devices;
    for (const auto& device : *this) {
        if (device->getModuleHandle() == moduleHandle) {
            devices.add(device);
        }
    }
    return devices;
}

audio_devices_t DeviceVector::getDeviceTypesFromHwModule(audio_module_handle_t moduleHandle) const
{
    audio_devices_t deviceTypes = AUDIO_DEVICE_NONE;
    for (const auto& device : *this) {
        if (device->getModuleHandle() == moduleHandle) {
            deviceTypes |= device->type();
        }
    }
    return deviceTypes;
}

sp<DeviceDescriptor> DeviceVector::getDevice(audio_devices_t type, const String8& address,
                                             audio_format_t format) const
{
    sp<DeviceDescriptor> device;
    for (size_t i = 0; i < size(); i++) {
        if (itemAt(i)->type() == type) {
            // If format is specified, match it and ignore address
            // Otherwise if address is specified match it
            // Otherwise always match
            if (((address == "" || itemAt(i)->address() == address) &&
                 format == AUDIO_FORMAT_DEFAULT) ||
                (itemAt(i)->supportsFormat(format) && format != AUDIO_FORMAT_DEFAULT)) {
                device = itemAt(i);
                if (itemAt(i)->address() == address) {
                    break;
                }
            }
        }
    }
    ALOGV("DeviceVector::%s() for type %08x address \"%s\" found %p format %08x",
            __func__, type, address.string(), device.get(), format);
    return device;
}

sp<DeviceDescriptor> DeviceVector::getDeviceFromId(audio_port_handle_t id) const
{
    if (id != AUDIO_PORT_HANDLE_NONE) {
        for (const auto& device : *this) {
            if (device->getId() == id) {
                return device;
            }
        }
    }
    return nullptr;
}

DeviceVector DeviceVector::getDevicesFromTypeMask(audio_devices_t type) const
{
    DeviceVector devices;
    bool isOutput = audio_is_output_devices(type);
    type &= ~AUDIO_DEVICE_BIT_IN;
    for (size_t i = 0; (i < size()) && (type != AUDIO_DEVICE_NONE); i++) {
        bool curIsOutput = audio_is_output_devices(itemAt(i)->type());
        audio_devices_t curType = itemAt(i)->type() & ~AUDIO_DEVICE_BIT_IN;
        if ((isOutput == curIsOutput) && ((type & curType) != 0)) {
            devices.add(itemAt(i));
            type &= ~curType;
            ALOGV("DeviceVector::%s() for type %08x found %p",
                    __func__, itemAt(i)->type(), itemAt(i).get());
        }
    }
    return devices;
}

sp<DeviceDescriptor> DeviceVector::getDeviceFromTagName(const String8 &tagName) const
{
    for (const auto& device : *this) {
        if (device->getTagName() == tagName) {
            return device;
        }
    }
    return nullptr;
}

void DeviceVector::dump(String8 *dst, const String8 &tag, int spaces, bool verbose) const
{
    if (isEmpty()) {
        return;
    }
    dst->appendFormat("%*s- %s devices:\n", spaces, "", tag.string());
    for (size_t i = 0; i < size(); i++) {
        itemAt(i)->dump(dst, spaces + 2, i, verbose);
    }
}

void DeviceDescriptor::toAudioPortConfig(struct audio_port_config *dstConfig,
                                         const struct audio_port_config *srcConfig) const
{
    dstConfig->config_mask = AUDIO_PORT_CONFIG_GAIN;
    if (mSamplingRate != 0) {
        dstConfig->config_mask |= AUDIO_PORT_CONFIG_SAMPLE_RATE;
    }
    if (mChannelMask != AUDIO_CHANNEL_NONE) {
        dstConfig->config_mask |= AUDIO_PORT_CONFIG_CHANNEL_MASK;
    }
    if (mFormat != AUDIO_FORMAT_INVALID) {
        dstConfig->config_mask |= AUDIO_PORT_CONFIG_FORMAT;
    }

    if (srcConfig != NULL) {
        dstConfig->config_mask |= srcConfig->config_mask;
    }

    AudioPortConfig::toAudioPortConfig(dstConfig, srcConfig);

    dstConfig->id = mId;
    dstConfig->role = audio_is_output_device(mDeviceType) ?
                        AUDIO_PORT_ROLE_SINK : AUDIO_PORT_ROLE_SOURCE;
    dstConfig->type = AUDIO_PORT_TYPE_DEVICE;
    dstConfig->ext.device.type = mDeviceType;

    //TODO Understand why this test is necessary. i.e. why at boot time does it crash
    // without the test?
    // This has been demonstrated to NOT be true (at start up)
    // ALOG_ASSERT(mModule != NULL);
    dstConfig->ext.device.hw_module = getModuleHandle();
    (void)audio_utils_strlcpy_zerofill(dstConfig->ext.device.address, mAddress.string());
}

void DeviceDescriptor::toAudioPort(struct audio_port *port) const
{
    ALOGV("DeviceDescriptor::toAudioPort() handle %d type %08x", mId, mDeviceType);
    AudioPort::toAudioPort(port);
    port->id = mId;
    toAudioPortConfig(&port->active_config);
    port->ext.device.type = mDeviceType;
    port->ext.device.hw_module = getModuleHandle();
    (void)audio_utils_strlcpy_zerofill(port->ext.device.address, mAddress.string());
}

void DeviceDescriptor::importAudioPort(const sp<AudioPort>& port, bool force) {
    if (!force && !port->hasDynamicAudioProfile()) {
        return;
    }
    AudioPort::importAudioPort(port);
    port->pickAudioProfile(mSamplingRate, mChannelMask, mFormat);
}

void DeviceDescriptor::dump(String8 *dst, int spaces, int index, bool verbose) const
{
    dst->appendFormat("%*sDevice %d:\n", spaces, "", index + 1);
    if (mId != 0) {
        dst->appendFormat("%*s- id: %2d\n", spaces, "", mId);
    }
    if (!mTagName.isEmpty()) {
        dst->appendFormat("%*s- tag name: %s\n", spaces, "", mTagName.string());
    }

    dst->appendFormat("%*s- type: %-48s\n", spaces, "", ::android::toString(mDeviceType).c_str());

    if (mAddress.size() != 0) {
        dst->appendFormat("%*s- address: %-32s\n", spaces, "", mAddress.string());
    }
    AudioPort::dump(dst, spaces, verbose);
}

std::string DeviceDescriptor::toString() const
{
    std::stringstream sstream;
    sstream << "type:0x" << std::hex << type() << ",@:" << mAddress;
    return sstream.str();
}

std::string DeviceVector::toString() const
{
    if (isEmpty()) {
        return {"AUDIO_DEVICE_NONE"};
    }
    std::string result = {"{"};
    for (const auto &device : *this) {
        if (device != *begin()) {
           result += ";";
        }
        result += device->toString();
    }
    return result + "}";
}

DeviceVector DeviceVector::filter(const DeviceVector &devices) const
{
    DeviceVector filteredDevices;
    for (const auto &device : *this) {
        if (devices.contains(device)) {
            filteredDevices.add(device);
        }
    }
    return filteredDevices;
}

bool DeviceVector::containsAtLeastOne(const DeviceVector &devices) const
{
    return !filter(devices).isEmpty();
}

bool DeviceVector::containsAllDevices(const DeviceVector &devices) const
{
    return filter(devices).size() == devices.size();
}

DeviceVector DeviceVector::filterForEngine() const
{
    DeviceVector filteredDevices;
    for (const auto &device : *this) {
        if (audio_is_remote_submix_device(device->type()) && device->address() != "0") {
            continue;
        }
        filteredDevices.add(device);
    }
    return filteredDevices;
}

void DeviceDescriptor::log() const
{
    ALOGI("Device id:%d type:0x%08X:%s, addr:%s", mId,  mDeviceType,
          ::android::toString(mDeviceType).c_str(),
          mAddress.string());

    AudioPort::log("  ");
}

} // namespace android
