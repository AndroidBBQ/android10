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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_HELP_COMMAND_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_HELP_COMMAND_H_

#include <string>

#include <android-base/macros.h>

#include "Command.h"
#include "utils.h"

namespace android {
namespace lshal {

class Lshal;

class HelpCommand : public Command {
public:
    explicit HelpCommand(Lshal &lshal) : Command(lshal) {}
    ~HelpCommand() = default;
    Status main(const Arg &arg) override;
    void usage() const override;
    std::string getSimpleDescription() const override;
    std::string getName() const override { return GetName(); }
    static std::string GetName();
    Status usageOfCommand(const std::string& c) const;
};


}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_HELP_COMMAND_H_
