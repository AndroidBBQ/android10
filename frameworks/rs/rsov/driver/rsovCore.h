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

#ifndef RSOV_CORE_H
#define RSOV_CORE_H

namespace android {
namespace renderscript {

class RsdCpuReference;

namespace rsov {

class RSoVContext;

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

struct RSoVHal {
  android::renderscript::rsov::RSoVContext* mRSoV;
  android::renderscript::RsdCpuReference *mCpuRef;
};

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#endif
