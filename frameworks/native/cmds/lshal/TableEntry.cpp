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

#include <map>

#include <android-base/strings.h>
#include <hidl-hash/Hash.h>
#include <vintf/parse_string.h>

#include "TableEntry.h"

#include "TextTable.h"
#include "utils.h"

namespace android {
namespace lshal {

static const std::string &getArchString(vintf::Arch arch) {
    static const std::string sStr64 = "64";
    static const std::string sStr32 = "32";
    static const std::string sStrBoth = "32+64";
    static const std::string sStrUnknown = "?";
    switch (arch) {
        case vintf::Arch::ARCH_64:
            return sStr64;
        case vintf::Arch::ARCH_32:
            return sStr32;
        case vintf::Arch::ARCH_32_64:
            return sStrBoth;
        case vintf::Arch::ARCH_EMPTY: // fall through
        default:
            return sStrUnknown;
    }
}

static std::string getTitle(TableColumnType type) {
    switch (type) {
        case TableColumnType::INTERFACE_NAME:   return "Interface";
        case TableColumnType::TRANSPORT:        return "Transport";
        case TableColumnType::SERVER_PID:       return "Server";
        case TableColumnType::SERVER_CMD:       return "Server CMD";
        case TableColumnType::SERVER_ADDR:      return "PTR";
        case TableColumnType::CLIENT_PIDS:      return "Clients";
        case TableColumnType::CLIENT_CMDS:      return "Clients CMD";
        case TableColumnType::ARCH:             return "Arch";
        case TableColumnType::THREADS:          return "Thread Use";
        case TableColumnType::RELEASED:         return "R";
        case TableColumnType::HASH:             return "Hash";
        case TableColumnType::VINTF:            return "VINTF";
        case TableColumnType::SERVICE_STATUS:   return "Status";
        default:
            LOG(FATAL) << __func__ << "Should not reach here. " << static_cast<int>(type);
            return "";
    }
}

std::string TableEntry::getField(TableColumnType type) const {
    switch (type) {
        case TableColumnType::INTERFACE_NAME:
            return interfaceName;
        case TableColumnType::TRANSPORT:
            return vintf::to_string(transport);
        case TableColumnType::SERVER_PID:
            return serverPid == NO_PID ? "N/A" : std::to_string(serverPid);
        case TableColumnType::SERVER_CMD:
            return serverCmdline;
        case TableColumnType::SERVER_ADDR:
            return serverObjectAddress == NO_PTR ? "N/A" : toHexString(serverObjectAddress);
        case TableColumnType::CLIENT_PIDS:
            return join(clientPids, " ");
        case TableColumnType::CLIENT_CMDS:
            return join(clientCmdlines, ";");
        case TableColumnType::ARCH:
            return getArchString(arch);
        case TableColumnType::THREADS:
            return getThreadUsage();
        case TableColumnType::RELEASED:
            return isReleased();
        case TableColumnType::HASH:
            return hash;
        case TableColumnType::VINTF:
            return getVintfInfo();
        case TableColumnType::SERVICE_STATUS:
            return lshal::to_string(serviceStatus);
        default:
            LOG(FATAL) << __func__ << "Should not reach here. " << static_cast<int>(type);
            return "";
    }
}

std::string TableEntry::isReleased() const {
    static const std::string unreleased = Hash::hexString(Hash::kEmptyHash);

    if (hash.empty()) {
        return "?";
    }
    if (hash == unreleased) {
        return "N"; // unknown or unreleased
    }
    return "Y"; // released
}

std::string TableEntry::getVintfInfo() const {
    static const std::map<VintfInfo, std::string> values{
            {DEVICE_MANIFEST, "DM"},
            {DEVICE_MATRIX, "DC"},
            {FRAMEWORK_MANIFEST, "FM"},
            {FRAMEWORK_MATRIX, "FC"},
    };
    std::vector<std::string> ret;
    for (const auto& pair : values) {
        if (vintfInfo & pair.first) {
            ret.push_back(pair.second);
        }
    }
    auto joined = base::Join(ret, ',');
    return joined.empty() ? "X" : joined;
}

std::string to_string(ServiceStatus s) {
    switch (s) {
        case ServiceStatus::ALIVE: return "alive";
        case ServiceStatus::NON_RESPONSIVE: return "non-responsive";
        case ServiceStatus::DECLARED: return "declared";
        case ServiceStatus::UNKNOWN: return "N/A";
    }

    LOG(FATAL) << __func__ << "Should not reach here." << static_cast<int>(s);
    return "";
}

TextTable Table::createTextTable(bool neat,
    const std::function<std::string(const std::string&)>& emitDebugInfo) const {

    TextTable textTable;
    std::vector<std::string> row;
    if (!neat) {
        textTable.add(mDescription);

        row.clear();
        for (TableColumnType type : mSelectedColumns) {
            row.push_back(getTitle(type));
        }
        textTable.add(std::move(row));
    }

    for (const auto& entry : mEntries) {
        row.clear();
        for (TableColumnType type : mSelectedColumns) {
            row.push_back(entry.getField(type));
        }
        textTable.add(std::move(row));

        if (emitDebugInfo) {
            std::string debugInfo = emitDebugInfo(entry.interfaceName);
            if (!debugInfo.empty()) textTable.add(debugInfo);
        }
    }
    return textTable;
}

TextTable MergedTable::createTextTable() {
    TextTable textTable;
    for (const Table* table : mTables) {
        textTable.addAll(table->createTextTable());
    }
    return textTable;
}

bool TableEntry::operator==(const TableEntry& other) const {
    if (this == &other) {
        return true;
    }
    return interfaceName == other.interfaceName && transport == other.transport &&
        serverPid == other.serverPid && threadUsage == other.threadUsage &&
        threadCount == other.threadCount && serverCmdline == other.serverCmdline &&
        serverObjectAddress == other.serverObjectAddress && clientPids == other.clientPids &&
        clientCmdlines == other.clientCmdlines && arch == other.arch;
}

std::string TableEntry::to_string() const {
    using vintf::operator<<;
    std::stringstream ss;
    ss << "name=" << interfaceName << ";transport=" << transport << ";thread=" << getThreadUsage()
       << ";server=" << serverPid
       << "(" << serverObjectAddress << ";" << serverCmdline << ");clients=["
       << join(clientPids, ";") << "](" << join(clientCmdlines, ";") << ");arch="
       << getArchString(arch);
    return ss.str();

}

} // namespace lshal
} // namespace android
