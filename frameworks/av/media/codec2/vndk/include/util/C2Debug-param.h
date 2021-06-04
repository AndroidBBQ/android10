/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef C2UTILS_DEBUG_PARAM_H_
#define C2UTILS_DEBUG_PARAM_H_

#include <C2Param.h>
#include <util/C2Debug-base.h>

#include <iostream>

/**
 * Debug/print declarations for objects defined in C2Param.h
 */
C2_DECLARE_AS_STRING_AND_DEFINE_STREAM_OUT(C2FieldDescriptor::type_t)

std::ostream& operator<<(std::ostream& os, const C2Param::CoreIndex &i);
std::ostream& operator<<(std::ostream& os, const C2Param::Type &i);
std::ostream& operator<<(std::ostream& os, const C2Param::Index &i);
std::ostream& operator<<(std::ostream& os, const _C2FieldId &i);
std::ostream& operator<<(std::ostream& os, const C2FieldDescriptor &i);
std::ostream& operator<<(std::ostream& os, const C2ParamField &i);

#endif  // C2UTILS_DEBUG_PARAM_H_
