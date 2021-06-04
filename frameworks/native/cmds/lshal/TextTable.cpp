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

#include <algorithm>
#include <iomanip>

#include "TextTable.h"

namespace android {
namespace lshal {

void TextTable::computeWidth(const std::vector<std::string>& v) {
    if (mWidths.size() < v.size()) {
        mWidths.resize(v.size());
    }
    for (size_t i = 0; i < v.size(); ++i) {
        mWidths[i] = std::max(mWidths[i], v[i].length());
    }
}

void TextTable::dump(std::ostream& out) const {
    out << std::left;
    for (const auto& row : mTable) {
        if (!row.isRow()) {
            out << row.line() << std::endl;
            continue;
        }

        for (size_t i = 0; i < row.fields().size(); ++i) {
            if (i != 0) {
                out << " ";
            }
            // last column does not std::setw to avoid printing unnecessary spaces.
            if (i < row.fields().size() - 1) {
                out << std::setw(mWidths[i]);
            }
            out << row.fields()[i];
        }
        out << std::endl;
    }
}

void TextTable::addAll(TextTable&& other) {
    for (auto&& row : other.mTable) {
        if (row.isRow()) {
            computeWidth(row.fields());
        }

        mTable.emplace_back(std::move(row));
    }
}

} // namespace lshal
} // namespace android
