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

#include "include/serviceutils/PriorityDumper.h"

namespace android {

const char16_t PriorityDumper::PROTO_ARG[] = u"--proto";
const char16_t PriorityDumper::PRIORITY_ARG[] = u"--dump-priority";
const char16_t PriorityDumper::PRIORITY_ARG_CRITICAL[] = u"CRITICAL";
const char16_t PriorityDumper::PRIORITY_ARG_HIGH[] = u"HIGH";
const char16_t PriorityDumper::PRIORITY_ARG_NORMAL[] = u"NORMAL";

enum class PriorityType { INVALID, CRITICAL, HIGH, NORMAL };

static PriorityType getPriorityType(const String16& arg) {
    if (arg == PriorityDumper::PRIORITY_ARG_CRITICAL) {
        return PriorityType::CRITICAL;
    } else if (arg == PriorityDumper::PRIORITY_ARG_HIGH) {
        return PriorityType::HIGH;
    } else if (arg == PriorityDumper::PRIORITY_ARG_NORMAL) {
        return PriorityType::NORMAL;
    }
    return PriorityType::INVALID;
}

status_t PriorityDumper::dumpAll(int fd, const Vector<String16>& args, bool asProto) {
    status_t status;
    status = dumpCritical(fd, args, asProto);
    if (status != OK) return status;
    status = dumpHigh(fd, args, asProto);
    if (status != OK) return status;
    status = dumpNormal(fd, args, asProto);
    if (status != OK) return status;
    return status;
}

status_t PriorityDumper::priorityDump(int fd, const Vector<String16>& args) {
    status_t status;
    bool asProto = false;
    PriorityType priority = PriorityType::INVALID;

    Vector<String16> strippedArgs;
    for (uint32_t argIndex = 0; argIndex < args.size(); argIndex++) {
        if (args[argIndex] == PROTO_ARG) {
            asProto = true;
        } else if (args[argIndex] == PRIORITY_ARG) {
            if (argIndex + 1 < args.size()) {
                argIndex++;
                priority = getPriorityType(args[argIndex]);
            }
        } else {
            strippedArgs.add(args[argIndex]);
        }
    }

    switch (priority) {
        case PriorityType::CRITICAL:
            status = dumpCritical(fd, strippedArgs, asProto);
            break;
        case PriorityType::HIGH:
            status = dumpHigh(fd, strippedArgs, asProto);
            break;
        case PriorityType::NORMAL:
            status = dumpNormal(fd, strippedArgs, asProto);
            break;
        default:
            status = dumpAll(fd, strippedArgs, asProto);
            break;
    }
    return status;
}
} // namespace android
