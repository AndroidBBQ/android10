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

#include "HelpCommand.h"

#include "Lshal.h"

namespace android {
namespace lshal {

std::string HelpCommand::GetName() {
    return "help";
}

std::string HelpCommand::getSimpleDescription() const {
    return "Print help message.";
}

Status HelpCommand::main(const Arg &arg) {
    if (optind >= arg.argc) {
        // `lshal help` prints global usage.
        mLshal.usage();
        return OK;
    }
    (void)usageOfCommand(arg.argv[optind]);
    return OK;
}

Status HelpCommand::usageOfCommand(const std::string& c) const {
    if (c.empty()) {
        mLshal.usage();
        return USAGE;
    }
    auto command = mLshal.selectCommand(c);
    if (command == nullptr) {
        // from HelpCommand::main, `lshal help unknown`
        mLshal.usage();
        return USAGE;
    }

    command->usage();
    return USAGE;

}

void HelpCommand::usage() const {
    mLshal.err()
            << "help:" << std::endl
            << "    lshal -h" << std::endl
            << "    lshal --help" << std::endl
            << "    lshal help" << std::endl
            << "        Print this help message" << std::endl;
    mLshal.forEachCommand([&](const Command* e) {
        mLshal.err() << "    lshal help " << e->getName() << std::endl
                     << "        Print help message for " << e->getName() << std::endl;
    });

}

}  // namespace lshal
}  // namespace android

