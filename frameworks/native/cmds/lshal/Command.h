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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_COMMAND_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_COMMAND_H_

#include "utils.h"

namespace android {
namespace lshal {

class Lshal;

// Base class for all *Commands
class Command {
public:
    explicit Command(Lshal& lshal) : mLshal(lshal) {}
    virtual ~Command() = default;
    // Expect optind to be set by Lshal::main and points to the next argument
    // to process.
    virtual Status main(const Arg &arg) = 0;

    virtual void usage() const = 0;

    // e.g. "list"
    virtual std::string getName() const = 0;

    // e.g. "list HALs"
    virtual std::string getSimpleDescription() const = 0;

protected:
    Lshal& mLshal;
};


}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_LIST_COMMAND_H_
