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

#ifndef ANDROID_OS_DUMPSTATESECTIONREPORTER_H_
#define ANDROID_OS_DUMPSTATESECTIONREPORTER_H_

#include <android/os/IDumpstateListener.h>
#include <utils/StrongPointer.h>

namespace android {
namespace os {
namespace dumpstate {


/*
 * Helper class used to report per section details to a listener.
 *
 * Typical usage:
 *
 *    DumpstateSectionReporter sectionReporter(title, listener, sendReport);
 *    sectionReporter.setSize(5000);
 *
 */
class DumpstateSectionReporter {
  public:
    DumpstateSectionReporter(const std::string& title, sp<android::os::IDumpstateListener> listener,
                             bool sendReport);

    ~DumpstateSectionReporter();

    void setStatus(status_t status) {
        status_ = status;
    }

    void setSize(int size) {
        size_ = size;
    }

  private:
    std::string title_;
    android::sp<android::os::IDumpstateListener> listener_;
    bool sendReport_;
    status_t status_;
    int size_;
    std::chrono::time_point<std::chrono::steady_clock> started_;
};

}  // namespace dumpstate
}  // namespace os
}  // namespace android

#endif  // ANDROID_OS_DUMPSTATESECTIONREPORTER_H_
