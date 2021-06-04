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

#include "IVolumeCurves.h"
#include <policy.h>
#include <AudioPolicyManagerInterface.h>
#include <utils/RefBase.h>
#include <HandleGenerator.h>
#include <utils/String8.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <system/audio.h>
#include <cutils/config_utils.h>
#include <string>
#include <map>
#include <utility>

namespace android {

struct CurvePoint
{
    CurvePoint() {}
    CurvePoint(int index, int attenuationInMb) :
        mIndex(index), mAttenuationInMb(attenuationInMb) {}
    uint32_t mIndex;
    int mAttenuationInMb;
};

inline bool operator< (const CurvePoint &lhs, const CurvePoint &rhs)
{
    return lhs.mIndex < rhs.mIndex;
}

// A volume curve for a given use case and device category
// It contains of list of points of this curve expressing the attenuation in Millibels for
// a given volume index from 0 to 100
class VolumeCurve : public RefBase
{
public:
    VolumeCurve(device_category device) : mDeviceCategory(device) {}

    void add(const CurvePoint &point) { mCurvePoints.add(point); }

    float volIndexToDb(int indexInUi, int volIndexMin, int volIndexMax) const;

    void dump(String8 *dst, int spaces = 0, bool curvePoints = false) const;

    device_category getDeviceCategory() const { return mDeviceCategory; }

private:
    const device_category mDeviceCategory;
    SortedVector<CurvePoint> mCurvePoints;
};

// Volume Curves for a given use case indexed by device category
class VolumeCurves : public KeyedVector<device_category, sp<VolumeCurve> >,
                     public IVolumeCurves
{
public:
    VolumeCurves(int indexMin = 0, int indexMax = 100) :
        mIndexMin(indexMin), mIndexMax(indexMax)
    {
        addCurrentVolumeIndex(AUDIO_DEVICE_OUT_DEFAULT_FOR_VOLUME, 0);
    }
    status_t initVolume(int indexMin, int indexMax) override
    {
        mIndexMin = indexMin;
        mIndexMax = indexMax;
        return NO_ERROR;
    }

    sp<VolumeCurve> getCurvesFor(device_category device) const
    {
        if (indexOfKey(device) < 0) {
            return 0;
        }
        return valueFor(device);
    }

    virtual int getVolumeIndex(audio_devices_t device) const
    {
        device = Volume::getDeviceForVolume(device);
        // there is always a valid entry for AUDIO_DEVICE_OUT_DEFAULT_FOR_VOLUME
        if (mIndexCur.find(device) == end(mIndexCur)) {
            device = AUDIO_DEVICE_OUT_DEFAULT_FOR_VOLUME;
        }
        return mIndexCur.at(device);
    }

    virtual bool canBeMuted() const { return mCanBeMuted; }
    virtual void clearCurrentVolumeIndex() { mIndexCur.clear(); }
    void addCurrentVolumeIndex(audio_devices_t device, int index) override
    {
        mIndexCur[device] = index;
    }

    int getVolumeIndexMin() const { return mIndexMin; }

    int getVolumeIndexMax() const { return mIndexMax; }

    bool hasVolumeIndexForDevice(audio_devices_t device) const
    {
        device = Volume::getDeviceForVolume(device);
        return mIndexCur.find(device) != end(mIndexCur);
    }

    status_t switchCurvesFrom(const VolumeCurves &referenceCurves)
    {
        if (size() != referenceCurves.size()) {
            ALOGE("%s! device category not aligned, cannot switch", __FUNCTION__);
            return BAD_TYPE;
        }
        for (size_t index = 0; index < size(); index++) {
            device_category cat = keyAt(index);
            setVolumeCurve(cat, referenceCurves.getOriginVolumeCurve(cat));
        }
        return NO_ERROR;
    }
    status_t restoreOriginVolumeCurve()
    {
        return switchCurvesFrom(*this);
    }

    const sp<VolumeCurve> getOriginVolumeCurve(device_category deviceCategory) const
    {
        ALOG_ASSERT(mOriginVolumeCurves.indexOfKey(deviceCategory) >= 0, "Invalid device category");
        return mOriginVolumeCurves.valueFor(deviceCategory);
    }
    void setVolumeCurve(device_category deviceCategory, const sp<VolumeCurve> &volumeCurve)
    {
        ALOG_ASSERT(indexOfKey(deviceCategory) >= 0, "Invalid device category for Volume Curve");
        replaceValueFor(deviceCategory, volumeCurve);
    }

    ssize_t add(const sp<VolumeCurve> &volumeCurve)
    {
        device_category deviceCategory = volumeCurve->getDeviceCategory();
        ssize_t index = indexOfKey(deviceCategory);
        if (index < 0) {
            // Keep track of original Volume Curves per device category in order to switch curves.
            mOriginVolumeCurves.add(deviceCategory, volumeCurve);
            return KeyedVector::add(deviceCategory, volumeCurve);
        }
        return index;
    }

    virtual float volIndexToDb(device_category deviceCat, int indexInUi) const
    {
        sp<VolumeCurve> vc = getCurvesFor(deviceCat);
        if (vc != 0) {
            return vc->volIndexToDb(indexInUi, mIndexMin, mIndexMax);
        } else {
            ALOGE("Invalid device category %d for Volume Curve", deviceCat);
            return 0.0f;
        }
    }
    void addAttributes(const audio_attributes_t &attr)
    {
        mAttributes.push_back(attr);
    }
    AttributesVector getAttributes() const override { return mAttributes; }
    void addStreamType(audio_stream_type_t stream)
    {
        mStreams.push_back(stream);
    }
    StreamTypeVector getStreamTypes() const override { return mStreams; }

    void dump(String8 *dst, int spaces = 0, bool curvePoints = false) const override;

private:
    KeyedVector<device_category, sp<VolumeCurve> > mOriginVolumeCurves;
    std::map<audio_devices_t, int> mIndexCur; /**< current volume index per device. */
    int mIndexMin; /**< min volume index. */
    int mIndexMax; /**< max volume index. */
    const bool mCanBeMuted = true; /**< true is the stream can be muted. */

    AttributesVector mAttributes;
    StreamTypeVector mStreams; /**< Keep it for legacy. */
};

} // namespace android
