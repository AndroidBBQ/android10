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

#ifndef ANDROID_UTILS_PRIORITYDUMPER_H
#define ANDROID_UTILS_PRIORITYDUMPER_H

#include <utils/Errors.h>
#include <utils/String16.h>
#include <utils/Vector.h>

namespace android {

// Helper class to parse common arguments responsible for splitting dumps into
// various priority buckets and changing the output format of the dump.
class PriorityDumper {
public:
    static const char16_t PRIORITY_ARG[];
    static const char16_t PRIORITY_ARG_CRITICAL[];
    static const char16_t PRIORITY_ARG_HIGH[];
    static const char16_t PRIORITY_ARG_NORMAL[];
    static const char16_t PROTO_ARG[];

    // Parses the argument list searching for --dump_priority with a priority type
    // (HIGH, CRITICAL or NORMAL) and --proto. Matching arguments are stripped.
    // If a valid priority type is found, the associated PriorityDumper
    // method is called otherwise all supported sections are dumped.
    // If --proto is found, the dumpAsProto flag is set to dump sections in proto
    // format.
    status_t priorityDump(int fd, const Vector<String16>& args);

    // Dumps CRITICAL priority sections.
    virtual status_t dumpCritical(int /*fd*/, const Vector<String16>& /*args*/, bool /*asProto*/) {
        return OK;
    }

    // Dumps HIGH priority sections.
    virtual status_t dumpHigh(int /*fd*/, const Vector<String16>& /*args*/, bool /*asProto*/) {
        return OK;
    }

    // Dumps normal priority sections.
    virtual status_t dumpNormal(int /*fd*/, const Vector<String16>& /*args*/, bool /*asProto*/) {
        return OK;
    }

    // Dumps all sections.
    // This method is called when priorityDump is called without priority
    // arguments. By default, it calls all three dump methods.
    virtual status_t dumpAll(int fd, const Vector<String16>& args, bool asProto);
    virtual ~PriorityDumper() = default;
};

} // namespace android

#endif // ANDROID_UTILS_PRIORITYDUMPER_H
