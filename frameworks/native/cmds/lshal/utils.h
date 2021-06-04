/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_UTILS_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_UTILS_H_

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

namespace android {
namespace lshal {

enum : unsigned int {
    OK                                      = 0,
    // Return to Lshal::main to print help info.
    USAGE                                   = 1 << 0,
    // no service managers
    NO_BINDERIZED_MANAGER                   = 1 << 1,
    NO_PASSTHROUGH_MANAGER                  = 1 << 2,
    // general error in getting information from the three sources
    DUMP_BINDERIZED_ERROR                   = 1 << 3,
    DUMP_PASSTHROUGH_ERROR                  = 1 << 4,
    DUMP_ALL_LIBS_ERROR                     = 1 << 5,
    // I/O error in reading files
    IO_ERROR                                = 1 << 6,
    // Interface does not exist (IServiceManager::get fails)
    NO_INTERFACE                            = 1 << 7,
    // Transaction error from hwbinder transactions
    TRANSACTION_ERROR                       = 1 << 8,
    // No transaction error, but return value is unexpected.
    BAD_IMPL                                = 1 << 9,
    // Cannot fetch VINTF data.
    VINTF_ERROR                             = 1 << 10,
};
using Status = unsigned int;

struct Arg {
    int argc;
    char **argv;
};

template <typename A>
std::string join(const A &components, const std::string &separator) {
    std::stringstream out;
    bool first = true;
    for (const auto &component : components) {
        if (!first) {
            out << separator;
        }
        out << component;

        first = false;
    }
    return out.str();
}

std::string toHexString(uint64_t t);

template<typename String>
std::pair<String, String> splitFirst(const String &s, char c) {
    const char *pos = strchr(s.c_str(), c);
    if (pos == nullptr) {
        return {s, {}};
    }
    return {String(s.c_str(), pos - s.c_str()), String(pos + 1)};
}

std::vector<std::string> split(const std::string &s, char c);

void replaceAll(std::string *s, char from, char to);

}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_UTILS_H_
