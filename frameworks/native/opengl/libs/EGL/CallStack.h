/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <log/log.h>
#include <backtrace/Backtrace.h>
#include <memory>

class CallStack {
public:
    // Create a callstack with the current thread's stack trace.
    // Immediately dump it to logcat using the given logtag.
    static void log(const char* logtag) noexcept {
        std::unique_ptr<Backtrace> backtrace(
                Backtrace::Create(BACKTRACE_CURRENT_PROCESS, BACKTRACE_CURRENT_THREAD));
        if (backtrace->Unwind(2)) {
            for (size_t i = 0, c = backtrace->NumFrames(); i < c; i++) {
                __android_log_print(ANDROID_LOG_DEBUG, logtag, "%s",
                        backtrace->FormatFrameData(i).c_str());
            }
        }
    }
};

