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

#ifndef STL_UTIL_H
#define STL_UTIL_H

namespace android {
namespace spirit {

template <class T> class ContainerDeleter {
public:
  ContainerDeleter(T &container) : mContainer(container) {}
  ~ContainerDeleter() {
    for (auto *ptr : mContainer) {
      delete ptr;
    }
  }

private:
  T &mContainer;
};

} // namespace spirit
} // namespace android

#endif // STL_UTIL_H
