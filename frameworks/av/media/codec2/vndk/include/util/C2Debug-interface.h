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

#ifndef C2UTILS_DEBUG_INTERFACE_H_
#define C2UTILS_DEBUG_INTERFACE_H_

#include <util/C2Debug-base.h>
#include <util/C2InterfaceUtils.h>

#include <iostream>

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2SupportedRange<T> &i);

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<T> &i);

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<T> &i);

#endif  // C2UTILS_DEBUG_INTERFACE_H_
