/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef VIEW_COMPILER_H_
#define VIEW_COMPILER_H_

namespace android {
namespace installd {

bool view_compiler(const char* apk_path, const char* package_name, const char* out_dex_file,
                   int uid);

} // namespace installd
} // namespace android

#endif // VIEW_COMPILER_H_
