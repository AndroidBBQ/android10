// Copyright (C) 2019 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <iostream>

#define NO_ERROR 0
#define ERROR_BAD_USAGE -1
#define ERROR_CANT_FIND_FILE -2
#define ERROR_NO_DEV_PROTO -3
#define ERROR_CANT_READ_PROTO -4
#define ERROR_CANT_UNCOMPRESS_PROTO -5
#define ERROR_GETTING_ANNOTATION -6
#define ERROR_GETTING_FIDELITYPARAMS -7
#define ERROR_ANNOTATION_TOO_COMPLEX -8
#define ERROR_ANNOTATION_ONLY_ENUMS -9
#define ERROR_FIDELITYPARAMS_TOO_COMPLEX -10
#define ERROR_FIDELITYPARAMS_BAD_TYPE -11
#define ERROR_GETTING_SETTINGS -12
#define ERROR_BAD_MAX_INSTRUMENTATION_KEYS -13
#define ERROR_BAD_ANNOTATION_ENUM_SIZE -14
#define ERROR_NO_DEV_FIDELITYPARAMS -15
#define ERROR(CODE, MSG) { std::cerr << MSG << std::endl; return CODE; }
