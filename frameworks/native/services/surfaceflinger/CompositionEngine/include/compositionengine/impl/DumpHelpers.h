/*
 * Copyright 2019 The Android Open Source Project
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

#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <math/mat4.h>
#include <ui/FloatRect.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Transform.h>

namespace android::compositionengine::impl {

void dumpVal(std::string& out, const char* name, bool);
void dumpVal(std::string& out, const char* name, const void*);
void dumpVal(std::string& out, const char* name, int);
void dumpVal(std::string& out, const char* name, float);
void dumpVal(std::string& out, const char* name, uint32_t);
void dumpHex(std::string& out, const char* name, uint64_t);
void dumpVal(std::string& out, const char* name, const char* value);
void dumpVal(std::string& out, const char* name, const std::string& value);

// For enums with named values
void dumpVal(std::string& out, const char* name, const char*, int);
void dumpVal(std::string& out, const char* name, const std::string&, int);

template <typename EnumType>
void dumpVal(std::string& out, const char* name, const char* valueName, EnumType value) {
    dumpVal(out, name, valueName, static_cast<std::underlying_type_t<EnumType>>(value));
}

template <typename EnumType>
void dumpVal(std::string& out, const char* name, const std::string& valueName, EnumType value) {
    dumpVal(out, name, valueName, static_cast<std::underlying_type_t<EnumType>>(value));
}

void dumpVal(std::string& out, const char* name, const FloatRect& rect);
void dumpVal(std::string& out, const char* name, const Rect& rect);
void dumpVal(std::string& out, const char* name, const Region& region);
void dumpVal(std::string& out, const char* name, const ui::Transform&);
void dumpVal(std::string& out, const char* name, const ui::Size&);

void dumpVal(std::string& out, const char* name, const mat4& tr);

} // namespace android::compositionengine::impl
