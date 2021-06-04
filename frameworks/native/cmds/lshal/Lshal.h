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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_LSHAL_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_LSHAL_H_

#include <iostream>
#include <string>

#include <android-base/macros.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <utils/StrongPointer.h>

#include "Command.h"
#include "HelpCommand.h"
#include "NullableOStream.h"
#include "utils.h"

namespace android {
namespace lshal {

class Lshal {
public:
    Lshal();
    virtual ~Lshal() {}
    Lshal(std::ostream &out, std::ostream &err,
            sp<hidl::manager::V1_0::IServiceManager> serviceManager,
            sp<hidl::manager::V1_0::IServiceManager> passthroughManager);
    Status main(const Arg &arg);
    // global usage
    void usage();
    virtual NullableOStream<std::ostream> err() const;
    virtual NullableOStream<std::ostream> out() const;
    const sp<hidl::manager::V1_0::IServiceManager> &serviceManager() const;
    const sp<hidl::manager::V1_0::IServiceManager> &passthroughManager() const;

    Status emitDebugInfo(
            const std::string &interfaceName,
            const std::string &instanceName,
            const std::vector<std::string> &options,
            bool excludesParentInstances,
            std::ostream &out,
            NullableOStream<std::ostream> err) const;

    Command* selectCommand(const std::string& command) const;

    void forEachCommand(const std::function<void(const Command* c)>& f) const;

private:
    Status parseArgs(const Arg &arg);

    std::string mCommand;
    NullableOStream<std::ostream> mOut;
    NullableOStream<std::ostream> mErr;

    sp<hidl::manager::V1_0::IServiceManager> mServiceManager;
    sp<hidl::manager::V1_0::IServiceManager> mPassthroughManager;

    std::vector<std::unique_ptr<Command>> mRegisteredCommands;

    DISALLOW_COPY_AND_ASSIGN(Lshal);
};

}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_LSHAL_H_
