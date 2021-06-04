/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXOBSERVER_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXOBSERVER_H

#include <list>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <media/IOMX.h>

#include <android/hardware/media/omx/1.0/IOmxObserver.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::media::omx::V1_0::IOmxObserver;
using ::android::hardware::media::omx::V1_0::Message;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::android::IOMXObserver;
using ::android::BnOMXObserver;
using ::android::omx_message;

/**
 * Wrapper classes for conversion
 * ==============================
 *
 * Naming convention:
 * - LW = Legacy Wrapper --- It wraps a Treble object inside a legacy object.
 * - TW = Treble Wrapper --- It wraps a legacy object inside a Treble object.
 */

struct LWOmxObserver : public BnOMXObserver {
    sp<IOmxObserver> mBase;
    LWOmxObserver(sp<IOmxObserver> const& base);
    void onMessages(std::list<omx_message> const& lMessages) override;
};

struct TWOmxObserver : public IOmxObserver {
    sp<IOMXObserver> mBase;
    TWOmxObserver(sp<IOMXObserver> const& base);
    Return<void> onMessages(const hidl_vec<Message>& tMessages) override;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_WOMXOBSERVER_H
