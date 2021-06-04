/*
 * Copyright 2019 The Android Open Source Project
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

#define LOG_TAG "IRegionSamplingListener"
//#define LOG_NDEBUG 0

#include <gui/IRegionSamplingListener.h>

namespace android {

namespace { // Anonymous

enum class Tag : uint32_t {
    ON_SAMPLE_COLLECTED = IBinder::FIRST_CALL_TRANSACTION,
    LAST = ON_SAMPLE_COLLECTED,
};

} // Anonymous namespace

class BpRegionSamplingListener : public SafeBpInterface<IRegionSamplingListener> {
public:
    explicit BpRegionSamplingListener(const sp<IBinder>& impl)
          : SafeBpInterface<IRegionSamplingListener>(impl, "BpRegionSamplingListener") {}

    ~BpRegionSamplingListener() override;

    void onSampleCollected(float medianLuma) override {
        callRemoteAsync<decltype(
                &IRegionSamplingListener::onSampleCollected)>(Tag::ON_SAMPLE_COLLECTED, medianLuma);
    }
};

// Out-of-line virtual method definitions to trigger vtable emission in this translation unit (see
// clang warning -Wweak-vtables)
BpRegionSamplingListener::~BpRegionSamplingListener() = default;

IMPLEMENT_META_INTERFACE(RegionSamplingListener, "android.gui.IRegionSamplingListener");

status_t BnRegionSamplingListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                              uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::ON_SAMPLE_COLLECTED:
            return callLocalAsync(data, reply, &IRegionSamplingListener::onSampleCollected);
    }
}

} // namespace android
