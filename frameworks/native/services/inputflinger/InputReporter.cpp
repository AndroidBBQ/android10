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

#include "InputReporterInterface.h"

namespace android {

// --- InputReporter ---

class InputReporter : public InputReporterInterface {
public:
    void reportUnhandledKey(uint32_t sequenceNum) override;
    void reportDroppedKey(uint32_t sequenceNum) override;
};

void InputReporter::reportUnhandledKey(uint32_t sequenceNum) {
  // do nothing
}

void InputReporter::reportDroppedKey(uint32_t sequenceNum) {
  // do nothing
}

sp<InputReporterInterface> createInputReporter() {
  return new InputReporter();
}

} // namespace android
