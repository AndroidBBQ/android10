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

#define LOG_TAG "dumpstate"

#include "DumpstateSectionReporter.h"

namespace android {
namespace os {
namespace dumpstate {

DumpstateSectionReporter::DumpstateSectionReporter(const std::string& title,
                                                   sp<android::os::IDumpstateListener> listener,
                                                   bool sendReport)
    : title_(title), listener_(listener), sendReport_(sendReport), status_(OK), size_(-1) {
    started_ = std::chrono::steady_clock::now();
}

DumpstateSectionReporter::~DumpstateSectionReporter() {
    if ((listener_ != nullptr) && (sendReport_)) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started_);
        listener_->onSectionComplete(title_, status_, size_, (int32_t)elapsed.count());
    }
}

}  // namespace dumpstate
}  // namespace os
}  // namespace android
