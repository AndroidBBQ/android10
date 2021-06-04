/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_RAW_DEC_H_
#define ANDROID_C2_SOFT_RAW_DEC_H_

#include <SimpleC2Component.h>


namespace android {

struct C2SoftRawDec : public SimpleC2Component {
    class IntfImpl;

    C2SoftRawDec(const char* name, c2_node_id_t id,
              const std::shared_ptr<IntfImpl>& intfImpl);
    virtual ~C2SoftRawDec();

    // From SimpleC2Component
    c2_status_t onInit() override;
    c2_status_t onStop() override;
    void onReset() override;
    void onRelease() override;
    c2_status_t onFlush_sm() override;
    void process(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool) override;
    c2_status_t drain(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool) override;
private:
    std::shared_ptr<IntfImpl> mIntf;
    bool mSignalledEos;

    C2_DO_NOT_COPY(C2SoftRawDec);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_RAW_DEC_H_
