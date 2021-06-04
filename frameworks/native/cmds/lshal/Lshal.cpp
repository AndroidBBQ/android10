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

#define LOG_TAG "lshal"
#include <android-base/logging.h>

#include "Lshal.h"

#include <set>
#include <string>

#include <hidl/ServiceManagement.h>
#include <hidl/HidlTransportUtils.h>

#include "DebugCommand.h"
#include "ListCommand.h"
#include "PipeRelay.h"

namespace android {
namespace lshal {

using ::android::hidl::manager::V1_0::IServiceManager;

Lshal::Lshal()
    : Lshal(std::cout, std::cerr, ::android::hardware::defaultServiceManager(),
            ::android::hardware::getPassthroughServiceManager()) {
}

Lshal::Lshal(std::ostream &out, std::ostream &err,
            sp<hidl::manager::V1_0::IServiceManager> serviceManager,
            sp<hidl::manager::V1_0::IServiceManager> passthroughManager)
    : mOut(out), mErr(err),
      mServiceManager(serviceManager),
      mPassthroughManager(passthroughManager) {

    mRegisteredCommands.push_back({std::make_unique<ListCommand>(*this)});
    mRegisteredCommands.push_back({std::make_unique<DebugCommand>(*this)});
    mRegisteredCommands.push_back({std::make_unique<HelpCommand>(*this)});
}

void Lshal::forEachCommand(const std::function<void(const Command* c)>& f) const {
    for (const auto& e : mRegisteredCommands) f(e.get());
}

void Lshal::usage() {
    err() << "lshal: List and debug HALs." << std::endl << std::endl
          << "commands:" << std::endl;

    size_t nameMaxLength = 0;
    forEachCommand([&](const Command* e) {
        nameMaxLength = std::max(nameMaxLength, e->getName().length());
    });
    bool first = true;
    forEachCommand([&](const Command* e) {
        if (!first) err() << std::endl;
        first = false;
        err() << "    " << std::left << std::setw(nameMaxLength + 8) << e->getName()
              << e->getSimpleDescription();
    });
    err() << std::endl << "If no command is specified, `" << ListCommand::GetName()
          << "` is the default." << std::endl << std::endl;

    first = true;
    forEachCommand([&](const Command* e) {
        if (!first) err() << std::endl;
        first = false;
        e->usage();
    });
}

// A unique_ptr type using a custom deleter function.
template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T *)> >;

static hardware::hidl_vec<hardware::hidl_string> convert(const std::vector<std::string> &v) {
    hardware::hidl_vec<hardware::hidl_string> hv;
    hv.resize(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        hv[i].setToExternal(v[i].c_str(), v[i].size());
    }
    return hv;
}

Status Lshal::emitDebugInfo(
        const std::string &interfaceName,
        const std::string &instanceName,
        const std::vector<std::string> &options,
        bool excludesParentInstances,
        std::ostream &out,
        NullableOStream<std::ostream> err) const {
    using android::hidl::base::V1_0::IBase;
    using android::hardware::details::getDescriptor;

    hardware::Return<sp<IBase>> retBase = serviceManager()->get(interfaceName, instanceName);

    if (!retBase.isOk()) {
        std::string msg = "Cannot get " + interfaceName + "/" + instanceName + ": "
                + retBase.description();
        err << msg << std::endl;
        LOG(ERROR) << msg;
        return TRANSACTION_ERROR;
    }

    sp<IBase> base = retBase;
    if (base == nullptr) {
        std::string msg = interfaceName + "/" + instanceName + " does not exist, or "
                + "no permission to connect.";
        err << msg << std::endl;
        LOG(ERROR) << msg;
        return NO_INTERFACE;
    }

    if (excludesParentInstances) {
        const std::string descriptor = getDescriptor(base.get());
        if (descriptor.empty()) {
            std::string msg = interfaceName + "/" + instanceName + " getDescriptor failed";
            err << msg << std::endl;
            LOG(ERROR) << msg;
        }
        if (descriptor != interfaceName) {
            return OK;
        }
    }

    PipeRelay relay(out);

    if (relay.initCheck() != OK) {
        std::string msg = "PipeRelay::initCheck() FAILED w/ " + std::to_string(relay.initCheck());
        err << msg << std::endl;
        LOG(ERROR) << msg;
        return IO_ERROR;
    }

    deleted_unique_ptr<native_handle_t> fdHandle(
        native_handle_create(1 /* numFds */, 0 /* numInts */),
        native_handle_delete);

    fdHandle->data[0] = relay.fd();

    hardware::Return<void> ret = base->debug(fdHandle.get(), convert(options));

    if (!ret.isOk()) {
        std::string msg = "debug() FAILED on " + interfaceName + "/" + instanceName + ": "
                + ret.description();
        err << msg << std::endl;
        LOG(ERROR) << msg;
        return TRANSACTION_ERROR;
    }
    return OK;
}

Status Lshal::parseArgs(const Arg &arg) {
    optind = 1;
    if (optind >= arg.argc) {
        // no options at all.
        return OK;
    }
    mCommand = arg.argv[optind];
    if (selectCommand(mCommand) != nullptr) {
        ++optind;
        return OK; // mCommand is set correctly
    }

    if (mCommand.size() > 0 && mCommand[0] == '-') {
        // first argument is an option, set command to "" (which is recognized as "list")
        mCommand.clear();
        return OK;
    }

    err() << arg.argv[0] << ": unrecognized option `" << arg.argv[optind] << "'" << std::endl;
    return USAGE;
}

void signalHandler(int sig) {
    if (sig == SIGINT) {
        int retVal;
        pthread_exit(&retVal);
    }
}

Command* Lshal::selectCommand(const std::string& command) const {
    if (command.empty()) {
        return selectCommand(ListCommand::GetName());
    }
    for (const auto& e : mRegisteredCommands) {
        if (e->getName() == command) {
            return e.get();
        }
    }
    return nullptr;
}

Status Lshal::main(const Arg &arg) {
    // Allow SIGINT to terminate all threads.
    signal(SIGINT, signalHandler);

    Status status = parseArgs(arg);
    if (status != OK) {
        usage();
        return status;
    }
    auto c = selectCommand(mCommand);
    if (c == nullptr) {
        // unknown command, print global usage
        usage();
        return USAGE;
    }
    status = c->main(arg);
    if (status == USAGE) {
        // bad options. Run `lshal help ${mCommand}` instead.
        // For example, `lshal --unknown-option` becomes `lshal help` (prints global help)
        // and `lshal list --unknown-option` becomes `lshal help list`
        auto&& help = selectCommand(HelpCommand::GetName());
        return static_cast<HelpCommand*>(help)->usageOfCommand(mCommand);
    }

    return status;
}

NullableOStream<std::ostream> Lshal::err() const {
    return mErr;
}
NullableOStream<std::ostream> Lshal::out() const {
    return mOut;
}

const sp<IServiceManager> &Lshal::serviceManager() const {
    return mServiceManager;
}

const sp<IServiceManager> &Lshal::passthroughManager() const {
    return mPassthroughManager;
}

}  // namespace lshal
}  // namespace android
