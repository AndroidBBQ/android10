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

#pragma once

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/**
 * This creates a threadpool for incoming binder transactions if it has not already been created.
 */
void ABinderProcess_startThreadPool();
/**
 * This sets the maximum number of threads that can be started in the threadpool. By default, after
 * startThreadPool is called, this is one. If it is called additional times, it will only prevent
 * the kernel from starting new threads and will not delete already existing threads.
 */
bool ABinderProcess_setThreadPoolMaxThreadCount(uint32_t numThreads);
/**
 * This adds the current thread to the threadpool. This may cause the threadpool to exceed the
 * maximum size.
 */
void ABinderProcess_joinThreadPool();

__END_DECLS
