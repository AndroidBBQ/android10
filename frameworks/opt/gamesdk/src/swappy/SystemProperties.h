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

#include <string>

//
// To set system properties:
// adb shell setprop swappy.property value
//

std::string getSystemPropViaGet(const char* key, std::string default_value = "");

int getSystemPropViaGetAsInt(const char* key, int default_value = 0);

int getSystemPropViaGetAsBool(const char* key, bool default_value = false);

