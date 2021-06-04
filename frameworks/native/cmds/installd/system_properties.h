/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef OTAPREOPT_SYSTEM_PROPERTIES_H_
#define OTAPREOPT_SYSTEM_PROPERTIES_H_

#include <fstream>
#include <string>
#include <unordered_map>

#include <file_parsing.h>

namespace android {
namespace installd {

// Helper class to read system properties into and manage as a string->string map.
class SystemProperties {
 public:
    bool Load(const std::string& strFile) {
        return ParseFile(strFile, [&](const std::string& line) {
            size_t equals_pos = line.find('=');
            if (equals_pos == std::string::npos || equals_pos == 0) {
                // Did not find equals sign, or it's the first character - isn't a valid line.
                return true;
            }

            std::string key = line.substr(0, equals_pos);
            std::string value = line.substr(equals_pos + 1,
                                            line.length() - equals_pos + 1);

            properties_.insert(std::make_pair(key, value));

            return true;
        });
    }

    // Look up the key in the map. Returns null if the key isn't mapped.
    const std::string* GetProperty(const std::string& key) const {
        auto it = properties_.find(key);
        if (it != properties_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void SetProperty(const std::string& key, const std::string& value) {
        properties_.insert(std::make_pair(key, value));
    }

 private:
    // The actual map.
    std::unordered_map<std::string, std::string> properties_;
};

}  // namespace installd
}  // namespace android

#endif  // OTAPREOPT_SYSTEM_PROPERTIES_H_
