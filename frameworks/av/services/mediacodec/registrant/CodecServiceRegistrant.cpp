/*
 * Copyright 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "CodecServiceRegistrant"

#include <android-base/logging.h>

#include <C2PlatformSupport.h>
#include <codec2/hidl/1.0/ComponentStore.h>
#include <media/CodecServiceRegistrant.h>

extern "C" void RegisterCodecServices() {
    using namespace ::android::hardware::media::c2::V1_0;
    LOG(INFO) << "Creating software Codec2 service...";
    android::sp<IComponentStore> store =
        new utils::ComponentStore(
                android::GetCodec2PlatformComponentStore());
    if (store == nullptr) {
        LOG(ERROR) <<
                "Cannot create software Codec2 service.";
    } else {
        if (store->registerAsService("software") != android::OK) {
            LOG(ERROR) <<
                    "Cannot register software Codec2 service.";
        } else {
            LOG(INFO) <<
                    "Software Codec2 service created.";
        }
    }
}

