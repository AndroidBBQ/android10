/*
 * Copyright (C) 2014 The Android Open Source Project
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
#ifndef MINIKIN_MACROS_H
#define MINIKIN_MACROS_H

#if defined(__clang__)
#define IGNORE_INTEGER_OVERFLOW __attribute__((no_sanitize("integer")))
#else
#define IGNORE_INTEGER_OVERFLOW  // no-op
#endif                           // __clang__

#define MINIKIN_PREVENT_COPY_AND_ASSIGN(Type) \
    Type(const Type&) = delete;               \
    Type& operator=(const Type&) = delete

#define MINIKIN_PREVENT_COPY_ASSIGN_AND_MOVE(Type) \
    Type(const Type&) = delete;                    \
    Type& operator=(const Type&) = delete;         \
    Type(Type&&) = delete;                         \
    Type& operator=(Type&&) = delete

// Following thread annotations are partially copied from Abseil thread_annotations.h file.
// https://github.com/abseil/abseil-cpp/blob/master/absl/base/thread_annotations.h

#if defined(__clang__)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op
#endif

// GUARDED_BY()
//
// Documents if a shared field or global variable needs to be protected by a
// mutex. GUARDED_BY() allows the user to specify a particular mutex that
// should be held when accessing the annotated variable.
//
// Example:
//
//   Mutex mu;
//   int p1 GUARDED_BY(mu);
#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

// EXCLUSIVE_LOCKS_REQUIRED()
//
// Documents a function that expects a mutex to be held prior to entry.
// The mutex is expected to be held both on entry to, and exit from, the
// function.
//
// Example:
//
//   Mutex mu1, mu2;
//   int a GUARDED_BY(mu1);
//   int b GUARDED_BY(mu2);
//
//   void foo() EXCLUSIVE_LOCKS_REQUIRED(mu1, mu2) { ... };
#define EXCLUSIVE_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

#endif  // MINIKIN_MACROS_H
