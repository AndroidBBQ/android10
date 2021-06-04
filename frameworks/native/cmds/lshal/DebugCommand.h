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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_DEBUG_COMMAND_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_DEBUG_COMMAND_H_

#include <string>

#include <android-base/macros.h>

#include "Command.h"
#include "utils.h"

namespace android {
namespace lshal {

class Lshal;

class DebugCommand : public Command {
public:
    explicit DebugCommand(Lshal &lshal) : Command(lshal) {}
    ~DebugCommand() = default;
    Status main(const Arg &arg) override;
    void usage() const override;
    std::string getSimpleDescription() const override;
    std::string getName() const override;
private:
    Status parseArgs(const Arg &arg);

    std::string mInterfaceName;
    std::vector<std::string> mOptions;

    // Outputs the actual descriptor of a hal instead of the debug output
    // if the arguments provided are a superclass of the actual hal impl.
    bool mExcludesParentInstances;

    DISALLOW_COPY_AND_ASSIGN(DebugCommand);
};


}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_DEBUG_COMMAND_H_
