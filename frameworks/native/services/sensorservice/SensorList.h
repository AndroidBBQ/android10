/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_SENSOR_SERVICE_UTIL_SENSOR_LIST_H
#define ANDROID_SENSOR_SERVICE_UTIL_SENSOR_LIST_H

#include "SensorInterface.h"
#include "SensorServiceUtils.h"

#include <sensor/Sensor.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <map>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace android {
class SensorInterface;

namespace SensorServiceUtil {

class SensorList : public Dumpable {
public:
    // After SensorInterface * is added into SensorList, it can be assumed that SensorList own the
    // object it pointed to and the object should not be released elsewhere.
    bool add(int handle, SensorInterface* si, bool isForDebug = false, bool isVirtual = false);

    // After a handle is removed, the object that SensorInterface * pointing to may get deleted if
    // no more sp<> of the same object exist.
    bool remove(int handle);

    inline bool hasAnySensor() const { return mHandleMap.size() > 0;}

    //helper functions
    const Vector<Sensor> getUserSensors() const;
    const Vector<Sensor> getUserDebugSensors() const;
    const Vector<Sensor> getDynamicSensors() const;
    const Vector<Sensor> getVirtualSensors() const;

    String8 getName(int handle) const;
    sp<SensorInterface> getInterface(int handle) const;
    bool isNewHandle(int handle) const;

    // Iterate through Sensor in sensor list and perform operation f on each Sensor object.
    //
    // TF is a function with the signature:
    //    bool f(const Sensor &);
    // A return value of 'false' stops the iteration immediately.
    //
    // Note: in the function f, it is illegal to make calls to member functions of the same
    // SensorList object on which forEachSensor is invoked.
    template <typename TF>
    void forEachSensor(const TF& f) const;

    const Sensor& getNonSensor() const { return mNonSensor;}

    // Dumpable interface
    virtual std::string dump() const override;

    virtual ~SensorList();
private:
    struct Entry {
        sp<SensorInterface> si;
        const bool isForDebug;
        const bool isVirtual;
        Entry(SensorInterface* si_, bool debug_, bool virtual_) :
            si(si_), isForDebug(debug_), isVirtual(virtual_) {
        }
    };

    const static Sensor mNonSensor; //.getName() == "unknown",

    // Iterate through Entry in sensor list and perform operation f on each Entry.
    //
    // TF is a function with the signature:
    //    bool f(const Entry &);
    // A return value of 'false' stops the iteration over entries immediately.
    //
    // Note: in the function being passed in, it is illegal to make calls to member functions of the
    // same SensorList object on which forEachSensor is invoked.
    template <typename TF>
    void forEachEntry(const TF& f) const;

    template <typename T, typename TF>
    T getOne(int handle, const TF& accessor, T def = T()) const;

    mutable std::mutex mLock;
    std::map<int, Entry> mHandleMap;
    std::unordered_set<int> mUsedHandle;
};

template <typename TF>
void SensorList::forEachSensor(const TF& f) const {
    // lock happens in forEachEntry
    forEachEntry([&f] (const Entry& e) -> bool { return f(e.si->getSensor());});
}

template <typename TF>
void SensorList::forEachEntry(const TF& f) const {
    std::lock_guard<std::mutex> lk(mLock);

    for (auto&& i : mHandleMap) {
        if (!f(i.second)){
            break;
        }
    }
}

template <typename T, typename TF>
T SensorList::getOne(int handle, const TF& accessor, T def) const {
    std::lock_guard<std::mutex> lk(mLock);
    auto i = mHandleMap.find(handle);
    if (i != mHandleMap.end()) {
        return accessor(i->second);
    } else {
        return def;
    }
}

} // namespace SensorServiceUtil
} // namespace android

#endif // ANDROID_SENSOR_SERVICE_UTIL_SENSOR_LIST_H
