/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_TABLE_ENTRY_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_TABLE_ENTRY_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <iostream>

#include <procpartition/procpartition.h>
#include <vintf/Arch.h>
#include <vintf/Transport.h>

#include "TextTable.h"

namespace android {
namespace lshal {

using android::procpartition::Partition;
using Pids = std::vector<int32_t>;

enum class TableColumnType : unsigned int {
    INTERFACE_NAME,
    TRANSPORT,
    SERVER_PID,
    SERVER_CMD,
    SERVER_ADDR,
    CLIENT_PIDS,
    CLIENT_CMDS,
    ARCH,
    THREADS,
    RELEASED,
    HASH,
    VINTF,
    SERVICE_STATUS,
};

enum : unsigned int {
    VINTF_INFO_EMPTY = 0,
    DEVICE_MANIFEST = 1 << 0,
    DEVICE_MATRIX = 1 << 1,
    FRAMEWORK_MANIFEST = 1 << 2,
    FRAMEWORK_MATRIX = 1 << 3,
};
using VintfInfo = unsigned int;

enum {
    NO_PID = -1,
    NO_PTR = 0
};

enum class ServiceStatus {
    UNKNOWN, // For passthrough
    ALIVE,
    NON_RESPONSIVE, // registered but not respond to calls
    DECLARED, // in VINTF manifest
};
std::string to_string(ServiceStatus s);

struct TableEntry {
    std::string interfaceName{};
    vintf::Transport transport{vintf::Transport::EMPTY};
    int32_t serverPid{NO_PID};
    uint32_t threadUsage{0};
    uint32_t threadCount{0};
    std::string serverCmdline{};
    uint64_t serverObjectAddress{NO_PTR};
    Pids clientPids{};
    std::vector<std::string> clientCmdlines{};
    vintf::Arch arch{vintf::Arch::ARCH_EMPTY};
    // empty: unknown, all zeros: unreleased, otherwise: released
    std::string hash{};
    Partition partition{Partition::UNKNOWN};
    VintfInfo vintfInfo{VINTF_INFO_EMPTY};
    // true iff hwbinder and service started
    ServiceStatus serviceStatus{ServiceStatus::UNKNOWN};

    static bool sortByInterfaceName(const TableEntry &a, const TableEntry &b) {
        return a.interfaceName < b.interfaceName;
    };
    static bool sortByServerPid(const TableEntry &a, const TableEntry &b) {
        return a.serverPid < b.serverPid;
    };

    std::string getThreadUsage() const {
        if (threadCount == 0) {
            return "N/A";
        }

        return std::to_string(threadUsage) + "/" + std::to_string(threadCount);
    }

    std::string isReleased() const;

    std::string getVintfInfo() const;

    std::string getField(TableColumnType type) const;

    bool operator==(const TableEntry& other) const;
    std::string to_string() const;
};

using SelectedColumns = std::vector<TableColumnType>;

class Table {
public:
    using Entries = std::vector<TableEntry>;

    Entries::iterator begin() { return mEntries.begin(); }
    Entries::const_iterator begin() const { return mEntries.begin(); }
    Entries::iterator end() { return mEntries.end(); }
    Entries::const_iterator end() const { return mEntries.end(); }
    size_t size() const { return mEntries.size(); }

    void add(TableEntry&& entry) { mEntries.push_back(std::move(entry)); }

    void setSelectedColumns(const SelectedColumns& s) { mSelectedColumns = s; }
    const SelectedColumns& getSelectedColumns() const { return mSelectedColumns; }

    void setDescription(std::string&& d) { mDescription = std::move(d); }

    // Write table content.
    TextTable createTextTable(bool neat = true,
        const std::function<std::string(const std::string&)>& emitDebugInfo = nullptr) const;

private:
    std::string mDescription;
    Entries mEntries;
    SelectedColumns mSelectedColumns;
};

using TableEntryCompare = std::function<bool(const TableEntry &, const TableEntry &)>;

class MergedTable {
public:
    explicit MergedTable(std::vector<const Table*>&& tables) : mTables(std::move(tables)) {}
    TextTable createTextTable();
private:
    std::vector<const Table*> mTables;
};

}  // namespace lshal
}  // namespace android

#endif  // FRAMEWORK_NATIVE_CMDS_LSHAL_TABLE_ENTRY_H_
