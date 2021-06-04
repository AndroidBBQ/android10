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

#ifndef OTAPREOPT_FILE_PARSING_H_
#define OTAPREOPT_FILE_PARSING_H_

#include <fstream>
#include <functional>
#include <string>

namespace android {
namespace installd {

bool ParseFile(const std::string& strFile, std::function<bool (const std::string&)> parse) {
    std::ifstream input_stream(strFile);

    if (!input_stream.is_open()) {
        return false;
    }

    while (!input_stream.eof()) {
        // Read the next line.
        std::string line;
        getline(input_stream, line);

        // Is the line empty? Simplifies the next check.
        if (line.empty()) {
            continue;
        }

        // Is this a comment (starts with pound)?
        if (line[0] == '#') {
            continue;
        }

        if (!parse(line)) {
            return false;
        }
    }

    return true;
}

}  // namespace installd
}  // namespace android

#endif  // OTAPREOPT_FILE_PARSING_H_
