/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace android {
namespace spirit {

template <typename T> std::vector<T> readFile(const char *filename) {
  std::ifstream ifs;
  std::filebuf *fb = ifs.rdbuf();
  fb->open(filename, std::ios::in);

  if (!fb->is_open()) {
    std::cerr << "failed opening " << filename << std::endl;
    return std::vector<T>();
  }

  ifs.seekg(0, ifs.end);
  int length = ifs.tellg();
  ifs.seekg(0, ifs.beg);

  std::vector<T> ret(length / sizeof(T));

  ifs.read((char *)ret.data(), length);

  fb->close();

  return ret;
}

template <typename T> std::vector<T> readFile(const std::string &filename) {
  return readFile<T>(filename.c_str());
}

template <typename T>
void writeFile(const char *filename, const std::vector<T> &data) {
  std::ofstream ofs(filename, std::ios::out);

  ofs.write(reinterpret_cast<const char *>(data.data()),
            sizeof(T) * data.size());
}

} // namespace spirit
} // namespace android

#endif // FILE_UTILS_H
