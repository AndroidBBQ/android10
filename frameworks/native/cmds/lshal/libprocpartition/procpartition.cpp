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

#include <procpartition/procpartition.h>

#include <android-base/file.h>

namespace android {
namespace procpartition {

std::ostream& operator<<(std::ostream& os, Partition p) {
    switch (p) {
        case Partition::SYSTEM: return os << "system";
        case Partition::VENDOR: return os << "vendor";
        case Partition::ODM: return os << "odm";
        case Partition::UNKNOWN: // fallthrough
        default:
            return os << "(unknown)";
    }
}

std::string getExe(pid_t pid) {
    std::string exe;
    std::string real;
    if (!android::base::Readlink("/proc/" + std::to_string(pid) + "/exe", &exe)) {
        return "";
    }
    if (!android::base::Realpath(exe, &real)) {
        return "";
    }
    return real;
}

std::string getCmdline(pid_t pid) {
    std::string content;
    if (!android::base::ReadFileToString("/proc/" + std::to_string(pid) + "/cmdline", &content,
                                         false /* follow symlinks */)) {
        return "";
    }
    return std::string{content.c_str()};
}

Partition parsePartition(const std::string& s) {
    if (s == "system") {
        return Partition::SYSTEM;
    }
    if (s == "vendor") {
        return Partition::VENDOR;
    }
    if (s == "odm") {
        return Partition::ODM;
    }
    return Partition::UNKNOWN;
}

Partition getPartitionFromRealpath(const std::string& path) {
    if (path == "/system/bin/app_process64" ||
        path == "/system/bin/app_process32") {

        return Partition::UNKNOWN; // cannot determine
    }
    size_t backslash = path.find_first_of('/', 1);
    std::string partition = (backslash != std::string::npos) ? path.substr(1, backslash - 1) : path;

    return parsePartition(partition);
}

Partition getPartitionFromCmdline(pid_t pid) {
    const auto& cmdline = getCmdline(pid);
    if (cmdline == "system_server") {
        return Partition::SYSTEM;
    }
    if (cmdline.empty() || cmdline.front() != '/') {
        return Partition::UNKNOWN;
    }
    return getPartitionFromRealpath(cmdline);
}

Partition getPartitionFromExe(pid_t pid) {
    const auto& real = getExe(pid);
    if (real.empty() || real.front() != '/') {
        return Partition::UNKNOWN;
    }
    return getPartitionFromRealpath(real);
}


Partition getPartition(pid_t pid) {
    Partition partition = getPartitionFromExe(pid);
    if (partition == Partition::UNKNOWN) {
        partition = getPartitionFromCmdline(pid);
    }
    return partition;
}

}  // namespace procpartition
}  // namespace android
