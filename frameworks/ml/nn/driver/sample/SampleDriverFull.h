/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_FULL_H
#define ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_FULL_H

#include "SampleDriver.h"

namespace android {
namespace nn {
namespace sample_driver {

class SampleDriverFull : public SampleDriver {
   public:
    SampleDriverFull(const char* name, PerformanceInfo perf) : SampleDriver(name), mPerf(perf) {}
    Return<void> getCapabilities_1_2(getCapabilities_1_2_cb cb) override;
    Return<void> getSupportedOperations_1_2(const V1_2::Model& model,
                                            getSupportedOperations_1_2_cb cb) override;

   private:
    PerformanceInfo mPerf;
};

}  // namespace sample_driver
}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_FULL_H
