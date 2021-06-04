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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_LIST_COMMAND_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_LIST_COMMAND_H_

#include <getopt.h>
#include <stdint.h>

#include <fstream>
#include <string>
#include <vector>

#include <android-base/macros.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <hidl-util/FqInstance.h>
#include <vintf/HalManifest.h>
#include <vintf/VintfObject.h>

#include "Command.h"
#include "NullableOStream.h"
#include "TableEntry.h"
#include "TextTable.h"
#include "utils.h"

namespace android {
namespace lshal {

class Lshal;

struct PidInfo {
    std::map<uint64_t, Pids> refPids; // pids that are referenced
    uint32_t threadUsage; // number of threads in use
    uint32_t threadCount; // number of threads total
};

enum class HalType {
    BINDERIZED_SERVICES = 0,
    PASSTHROUGH_CLIENTS,
    PASSTHROUGH_LIBRARIES,
    VINTF_MANIFEST,
    LAZY_HALS,
};

class ListCommand : public Command {
public:
    explicit ListCommand(Lshal &lshal) : Command(lshal) {}
    virtual ~ListCommand() = default;
    Status main(const Arg &arg) override;
    void usage() const override;
    std::string getSimpleDescription() const override;
    std::string getName() const override { return GetName(); }

    static std::string GetName();

    struct RegisteredOption {
        // short alternative, e.g. 'v'. If '\0', no short options is available.
        char shortOption;
        // long alternative, e.g. 'init-vintf'
        std::string longOption;
        // no_argument, required_argument or optional_argument
        int hasArg;
        // value written to 'flag' by getopt_long
        int val;
        // operation when the argument is present
        std::function<Status(ListCommand* thiz, const char* arg)> op;
        // help message
        std::string help;

        const std::string& getHelpMessageForArgument() const;
    };
    // A list of acceptable command line options
    // key: value returned by getopt_long
    using RegisteredOptions = std::vector<RegisteredOption>;

    static std::string INIT_VINTF_NOTES;

protected:
    Status parseArgs(const Arg &arg);
    // Retrieve first-hand information
    Status fetch();
    // Retrieve derived information base on existing table
    virtual void postprocess();
    Status dump();
    void putEntry(HalType type, TableEntry &&entry);
    Status fetchPassthrough(const sp<::android::hidl::manager::V1_0::IServiceManager> &manager);
    Status fetchBinderized(const sp<::android::hidl::manager::V1_0::IServiceManager> &manager);
    Status fetchAllLibraries(const sp<::android::hidl::manager::V1_0::IServiceManager> &manager);
    Status fetchManifestHals();
    Status fetchLazyHals();

    Status fetchBinderizedEntry(const sp<::android::hidl::manager::V1_0::IServiceManager> &manager,
                                TableEntry *entry);

    // Get relevant information for a PID by parsing files under /d/binder.
    // It is a virtual member function so that it can be mocked.
    virtual bool getPidInfo(pid_t serverPid, PidInfo *info) const;
    // Retrieve from mCachedPidInfos and call getPidInfo if necessary.
    const PidInfo* getPidInfoCached(pid_t serverPid);

    void dumpTable(const NullableOStream<std::ostream>& out) const;
    void dumpVintf(const NullableOStream<std::ostream>& out) const;
    void addLine(TextTable *table, const std::string &interfaceName, const std::string &transport,
                 const std::string &arch, const std::string &threadUsage, const std::string &server,
                 const std::string &serverCmdline, const std::string &address,
                 const std::string &clients, const std::string &clientCmdlines) const;
    void addLine(TextTable *table, const TableEntry &entry);
    // Read and return /proc/{pid}/cmdline.
    virtual std::string parseCmdline(pid_t pid) const;
    // Return /proc/{pid}/cmdline if it exists, else empty string.
    const std::string& getCmdline(pid_t pid);
    // Call getCmdline on all pid in pids. If it returns empty string, the process might
    // have died, and the pid is removed from pids.
    void removeDeadProcesses(Pids *pids);

    virtual Partition getPartition(pid_t pid);
    Partition resolvePartition(Partition processPartition, const FqInstance &fqInstance) const;

    VintfInfo getVintfInfo(const std::string &fqInstanceName, vintf::TransportArch ta) const;
    // Allow to mock these functions for testing.
    virtual std::shared_ptr<const vintf::HalManifest> getDeviceManifest() const;
    virtual std::shared_ptr<const vintf::CompatibilityMatrix> getDeviceMatrix() const;
    virtual std::shared_ptr<const vintf::HalManifest> getFrameworkManifest() const;
    virtual std::shared_ptr<const vintf::CompatibilityMatrix> getFrameworkMatrix() const;

    void forEachTable(const std::function<void(Table &)> &f);
    void forEachTable(const std::function<void(const Table &)> &f) const;
    Table* tableForType(HalType type);
    const Table* tableForType(HalType type) const;

    NullableOStream<std::ostream> err() const;
    NullableOStream<std::ostream> out() const;

    void registerAllOptions();

    // helper functions to dumpVintf.
    bool addEntryWithInstance(const TableEntry &entry, vintf::HalManifest *manifest) const;
    bool addEntryWithoutInstance(const TableEntry &entry, const vintf::HalManifest *manifest) const;

    // Helper function. Whether to fetch entries corresponding to a given HAL type.
    bool shouldFetchHalType(const HalType &type) const;

    void initFetchTypes();

    // Helper functions ti add HALs that are listed in VINTF manifest to LAZY_HALS table.
    bool hasHwbinderEntry(const TableEntry& entry) const;
    bool hasPassthroughEntry(const TableEntry& entry) const;

    Table mServicesTable{};
    Table mPassthroughRefTable{};
    Table mImplementationsTable{};
    Table mManifestHalsTable{};
    Table mLazyHalsTable{};

    std::string mFileOutputPath;
    TableEntryCompare mSortColumn = nullptr;

    bool mEmitDebugInfo = false;

    // If true, output in VINTF format. Output only entries from the specified partition.
    bool mVintf = false;
    Partition mVintfPartition = Partition::UNKNOWN;

    // If true, explanatory text are not emitted.
    bool mNeat = false;

    // Type(s) of HAL associations to list.
    std::vector<HalType> mListTypes{};
    // Type(s) of HAL associations to fetch.
    std::set<HalType> mFetchTypes{};

    // If an entry does not exist, need to ask /proc/{pid}/cmdline to get it.
    // If an entry exist but is an empty string, process might have died.
    // If an entry exist and not empty, it contains the cached content of /proc/{pid}/cmdline.
    std::map<pid_t, std::string> mCmdlines;

    // Cache for getPidInfo.
    std::map<pid_t, PidInfo> mCachedPidInfos;

    // Cache for getPartition.
    std::map<pid_t, Partition> mPartitions;

    RegisteredOptions mOptions;
    // All selected columns
    std::vector<TableColumnType> mSelectedColumns;
    // If true, emit cmdlines instead of PIDs
    bool mEnableCmdlines = false;

private:
    DISALLOW_COPY_AND_ASSIGN(ListCommand);
};


}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_LIST_COMMAND_H_
