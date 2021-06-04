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

#include "DebugCommand.h"

#include "Lshal.h"

#include <hidl-util/FQName.h>

namespace android {
namespace lshal {

std::string DebugCommand::getName() const {
    return "debug";
}

std::string DebugCommand::getSimpleDescription() const {
    return "Debug a specified HAL.";
}

Status DebugCommand::parseArgs(const Arg &arg) {
    if (optind >= arg.argc) {
        return USAGE;
    }

    // Optargs cannnot be used because the flag should not be considered set
    // if it should really be contained in mOptions.
    if (std::string(arg.argv[optind]) == "-E") {
        mExcludesParentInstances = true;
        optind++;
    }

    mInterfaceName = arg.argv[optind];
    ++optind;
    for (; optind < arg.argc; ++optind) {
        mOptions.push_back(arg.argv[optind]);
    }
    return OK;
}

Status DebugCommand::main(const Arg &arg) {
    Status status = parseArgs(arg);
    if (status != OK) {
        return status;
    }

    auto pair = splitFirst(mInterfaceName, '/');

    FQName fqName;
    if (!FQName::parse(pair.first, &fqName) || fqName.isIdentifier() || !fqName.isFullyQualified()) {
        mLshal.err() << "Invalid fully-qualified name '" << pair.first << "'\n\n";
        return USAGE;
    }

    return mLshal.emitDebugInfo(
            pair.first, pair.second.empty() ? "default" : pair.second, mOptions,
            mExcludesParentInstances,
            mLshal.out().buf(),
            mLshal.err());
}

void DebugCommand::usage() const {

    static const std::string debug =
            "debug:\n"
            "    lshal debug [-E] <interface> [options [options [...]]] \n"
            "        Print debug information of a specified interface.\n"
            "        -E: excludes debug output if HAL is actually a subclass.\n"
            "        <inteface>: Format is `android.hardware.foo@1.0::IFoo/default`.\n"
            "            If instance name is missing `default` is used.\n"
            "        options: space separated options to IBase::debug.\n";

    mLshal.err() << debug;
}

}  // namespace lshal
}  // namespace android

