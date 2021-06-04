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

#ifndef FRAMEWORK_NATIVE_CMDS_LSHAL_TEXT_TABLE_H_
#define FRAMEWORK_NATIVE_CMDS_LSHAL_TEXT_TABLE_H_

#include <iostream>
#include <string>
#include <vector>

namespace android {
namespace lshal {

// An element in TextTable. This is either an actual row (an array of cells
// in this row), or a string of explanatory text.
// To see if this is an actual row, test fields().empty().
class TextTableRow {
public:
    // An empty line.
    TextTableRow() {}

    // A row of cells.
    explicit TextTableRow(std::vector<std::string>&& v) : mFields(std::move(v)) {}

    // A single comment string.
    explicit TextTableRow(std::string&& s) : mLine(std::move(s)) {}
    explicit TextTableRow(const std::string& s) : mLine(s) {}

    // Whether this row is an actual row of cells.
    bool isRow() const { return !fields().empty(); }

    // Get all cells.
    const std::vector<std::string>& fields() const { return mFields; }

    // Get the single comment string.
    const std::string& line() const { return mLine; }

private:
    std::vector<std::string> mFields;
    std::string mLine;
};

// A TextTable is a 2D array of strings.
class TextTable {
public:

    // Add a TextTableRow.
    void add() { mTable.emplace_back(); }
    void add(std::vector<std::string>&& v) {
        computeWidth(v);
        mTable.emplace_back(std::move(v));
    }
    void add(const std::string& s) { mTable.emplace_back(s); }
    void add(std::string&& s) { mTable.emplace_back(std::move(s)); }

    void addAll(TextTable&& other);

    // Prints the table to out, with column widths adjusted appropriately according
    // to the content.
    void dump(std::ostream& out) const;

private:
    void computeWidth(const std::vector<std::string>& v);
    std::vector<size_t> mWidths;
    std::vector<TextTableRow> mTable;
};

} // namespace lshal
} // namespace android

#endif // FRAMEWORK_NATIVE_CMDS_LSHAL_TEXT_TABLE_H_
