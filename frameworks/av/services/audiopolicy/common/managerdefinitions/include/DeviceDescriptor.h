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

#include "AudioPort.h"
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/SortedVector.h>
#include <cutils/config_utils.h>
#include <system/audio.h>
#include <system/audio_policy.h>

namespace android {

class DeviceDescriptor : public AudioPort, public AudioPortConfig
{
public:
     // Note that empty name refers by convention to a generic device.
    explicit DeviceDescriptor(audio_devices_t type, const String8 &tagName = String8(""));
    DeviceDescriptor(audio_devices_t type, const FormatVector &encodedFormats,
            const String8 &tagName = String8(""));

    virtual ~DeviceDescriptor() {}

    virtual const String8 getTagName() const { return mTagName; }

    audio_devices_t type() const { return mDeviceType; }
    String8 address() const { return mAddress; }
    void setAddress(const String8 &address) { mAddress = address; }

    const FormatVector& encodedFormats() const { return mEncodedFormats; }

    audio_format_t getEncodedFormat() { return mCurrentEncodedFormat; }

    void setEncodedFormat(audio_format_t format) {
        mCurrentEncodedFormat = format;
    }

    bool equals(const sp<DeviceDescriptor>& other) const;

    bool hasCurrentEncodedFormat() const;

    bool supportsFormat(audio_format_t format);

    // AudioPortConfig
    virtual sp<AudioPort> getAudioPort() const { return (AudioPort*) this; }
    virtual void toAudioPortConfig(struct audio_port_config *dstConfig,
            const struct audio_port_config *srcConfig = NULL) const;

    // AudioPort
    virtual void attach(const sp<HwModule>& module);
    virtual void detach();

    virtual void toAudioPort(struct audio_port *port) const;
    virtual void importAudioPort(const sp<AudioPort>& port, bool force = false);

    audio_port_handle_t getId() const;
    void dump(String8 *dst, int spaces, int index, bool verbose = true) const;
    void log() const;
    std::string toString() const;

private:
    String8 mAddress{""};
    String8 mTagName; // Unique human readable identifier for a device port found in conf file.
    audio_devices_t     mDeviceType;
    FormatVector        mEncodedFormats;
    audio_port_handle_t mId = AUDIO_PORT_HANDLE_NONE;
    audio_format_t      mCurrentEncodedFormat;
};

class DeviceVector : public SortedVector<sp<DeviceDescriptor> >
{
public:
    DeviceVector() : SortedVector(), mDeviceTypes(AUDIO_DEVICE_NONE) {}
    explicit DeviceVector(const sp<DeviceDescriptor>& item) : DeviceVector()
    {
        add(item);
    }

    ssize_t add(const sp<DeviceDescriptor>& item);
    void add(const DeviceVector &devices);
    ssize_t remove(const sp<DeviceDescriptor>& item);
    void remove(const DeviceVector &devices);
    ssize_t indexOf(const sp<DeviceDescriptor>& item) const;

    audio_devices_t types() const { return mDeviceTypes; }

    // If 'address' is empty and 'codec' is AUDIO_FORMAT_DEFAULT, a device with a non-empty
    // address may be returned if there is no device with the specified 'type' and empty address.
    sp<DeviceDescriptor> getDevice(audio_devices_t type, const String8 &address,
                                   audio_format_t codec) const;
    DeviceVector getDevicesFromTypeMask(audio_devices_t types) const;

    /**
     * @brief getDeviceFromId
     * @param id of the DeviceDescriptor to seach (aka Port handle).
     * @return DeviceDescriptor associated to port id if found, nullptr otherwise. If the id is
     * equal to AUDIO_PORT_HANDLE_NONE, it also returns a nullptr.
     */
    sp<DeviceDescriptor> getDeviceFromId(audio_port_handle_t id) const;
    sp<DeviceDescriptor> getDeviceFromTagName(const String8 &tagName) const;
    DeviceVector getDevicesFromHwModule(audio_module_handle_t moduleHandle) const;
    audio_devices_t getDeviceTypesFromHwModule(audio_module_handle_t moduleHandle) const;

    bool contains(const sp<DeviceDescriptor>& item) const { return indexOf(item) >= 0; }

    /**
     * @brief containsAtLeastOne
     * @param devices vector of devices to check against.
     * @return true if the DeviceVector contains at list one of the devices from the given vector.
     */
    bool containsAtLeastOne(const DeviceVector &devices) const;

    /**
     * @brief containsAllDevices
     * @param devices vector of devices to check against.
     * @return true if the DeviceVector contains all the devices from the given vector
     */
    bool containsAllDevices(const DeviceVector &devices) const;

    /**
     * @brief filter the devices supported by this collection against another collection
     * @param devices to filter against
     * @return a filtered DeviceVector
     */
    DeviceVector filter(const DeviceVector &devices) const;

    /**
     * @brief filter the devices supported by this collection before sending
     * then to the Engine via AudioPolicyManagerObserver interface
     * @return a filtered DeviceVector
     */
    DeviceVector filterForEngine() const;

    /**
     * @brief merge two vectors. As SortedVector Implementation is buggy (it does not check the size
     * of the destination vector, only of the source, it provides a safe implementation
     * @param devices source device vector to merge with
     * @return size of the merged vector.
     */
    ssize_t merge(const DeviceVector &devices)
    {
        if (isEmpty()) {
            add(devices);
            return size();
        }
        return SortedVector::merge(devices);
    }

    /**
     * @brief operator == DeviceVector are equals if all the DeviceDescriptor can be found (aka
     * DeviceDescriptor with same type and address) and the vector has same size.
     * @param right DeviceVector to compare to.
     * @return true if right contains the same device and has the same size.
     */
    bool operator==(const DeviceVector &right) const
    {
        if (size() != right.size()) {
            return false;
        }
        for (const auto &device : *this) {
            if (right.indexOf(device) < 0) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const DeviceVector &right) const
    {
        return !operator==(right);
    }

    /**
     * @brief getFirstValidAddress
     * @return the first valid address of a list of device, "" if no device with valid address
     * found.
     * This helper function helps maintaining compatibility with legacy where we used to have a
     * devices mask and an address.
     */
    String8 getFirstValidAddress() const
    {
        for (const auto &device : *this) {
            if (device->address() != "") {
                return device->address();
            }
        }
        return String8("");
    }

    std::string toString() const;

    void dump(String8 *dst, const String8 &tag, int spaces = 0, bool verbose = true) const;

private:
    void refreshTypes();
    audio_devices_t mDeviceTypes;
};

} // namespace android
