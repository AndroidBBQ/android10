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

#pragma once

#include <unordered_map>
#include <vector>

namespace android {
namespace bpf {

bool startTrackingUidCpuFreqTimes();
bool getUidCpuFreqTimes(unsigned int uid, std::vector<std::vector<uint64_t>> *freqTimes);
bool getUidsCpuFreqTimes(std::unordered_map<uint32_t, std::vector<std::vector<uint64_t>>> *tisMap);
bool clearUidCpuFreqTimes(unsigned int uid);

} // namespace bpf
} // namespace android
