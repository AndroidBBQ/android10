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

#ifndef ANDROID_EXTERNAL_VIBRATION_H
#define ANDROID_EXTERNAL_VIBRATION_H

#include <android/os/IExternalVibrationController.h>
#include <binder/Binder.h>
#include <binder/IBinder.h>
#include <binder/Parcelable.h>
#include <system/audio.h>
#include <utils/RefBase.h>

namespace android {
namespace os {

class ExternalVibration : public Parcelable, public virtual RefBase {
public :
    ExternalVibration() = default;
    ExternalVibration(int32_t uid, std::string pkg, const audio_attributes_t& attrs,
            sp<IExternalVibrationController> controller);
    virtual ~ExternalVibration() = default;
    ExternalVibration(const ExternalVibration&) = default;

    bool operator==(const ExternalVibration&) const;

    status_t writeToParcel(Parcel* parcel) const override;
    status_t readFromParcel(const Parcel* parcel) override;

    int32_t getUid() const { return mUid; }
    std::string getPackage() const { return mPkg; }
    audio_attributes_t getAudioAttributes() const { return mAttrs; }
    sp<IExternalVibrationController> getController() { return mController; }


private:
    int32_t mUid;
    std::string mPkg;
    audio_attributes_t mAttrs;
    sp<IExternalVibrationController> mController;
    sp<IBinder> mToken = new BBinder();
};

} // namespace android
} // namespace os

#endif // ANDROID_EXTERNAL_VIBRATION_H
