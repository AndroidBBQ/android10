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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_PROCPARTITION_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_PROCPARTITION_H_

#include <sys/types.h>

#include <string>
#include <iostream>

namespace android {
namespace procpartition {

enum class Partition {
    UNKNOWN = 0,
    SYSTEM,
    VENDOR,
    ODM
};

std::ostream& operator<<(std::ostream& os, Partition p);
Partition parsePartition(const std::string& s);

// Return the path that /proc/<pid>/exe points to.
std::string getExe(pid_t pid);
// Return the content of /proc/<pid>/cmdline.
std::string getCmdline(pid_t pid);
// Infer the partition of a process from /proc/<pid>/exe and /proc/<pid>/cmdline.
Partition getPartition(pid_t pid);

}  // namespace procpartition
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_PROCPARTITION_H_
