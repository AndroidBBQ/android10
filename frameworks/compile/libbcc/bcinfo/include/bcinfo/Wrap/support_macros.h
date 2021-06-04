/*
 * Copyright 2012, The Android Open Source Project
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

// Define support macros for defining classes, etc.

#ifndef LLVM_SUPPORT_SUPPORT_MACROS_H__
#define LLVM_SUPPORT_SUPPORT_MACROS_H__

// Define macro, to use within a class declaration,  to disallow constructor
// copy. Defines copy constructor declaration under the assumption that it
// is never defined.
// NOLINT: Do not add parentheses around 'class_name'.
#define DISALLOW_CLASS_COPY(class_name) \
  class_name(class_name& arg)  // NOLINT, Do not implement

// Define macro, to use within a class declaration,  to disallow assignment.
// Defines assignment operation declaration under the assumption that it
// is never defined.
#define DISALLOW_CLASS_ASSIGN(class_name) \
  void operator=(class_name& arg)  // NOLINT, Do not implement

// Define macro to add copy and assignment declarations to a class file,
// for which no bodies will be defined, effectively disallowing these from
// being defined in the class.
#define DISALLOW_CLASS_COPY_AND_ASSIGN(class_name) \
  DISALLOW_CLASS_COPY(class_name); \
  DISALLOW_CLASS_ASSIGN(class_name)

#endif  // LLVM_SUPPORT_SUPPORT_MACROS_H__
