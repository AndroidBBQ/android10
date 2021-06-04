/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/TextOutput.h>
#include <serviceutils/PriorityDumper.h>
#include <utils/Log.h>
#include <utils/Vector.h>

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "dumpsys.h"

using namespace android;
using ::android::base::StringAppendF;
using ::android::base::StringPrintf;
using ::android::base::unique_fd;
using ::android::base::WriteFully;
using ::android::base::WriteStringToFd;

static int sort_func(const String16* lhs, const String16* rhs)
{
    return lhs->compare(*rhs);
}

static void usage() {
    fprintf(stderr,
            "usage: dumpsys\n"
            "         To dump all services.\n"
            "or:\n"
            "       dumpsys [-t TIMEOUT] [--priority LEVEL] [--help | -l | --skip SERVICES | "
            "SERVICE [ARGS]]\n"
            "         --help: shows this help\n"
            "         -l: only list services, do not dump them\n"
            "         -t TIMEOUT_SEC: TIMEOUT to use in seconds instead of default 10 seconds\n"
            "         -T TIMEOUT_MS: TIMEOUT to use in milliseconds instead of default 10 seconds\n"
            "         --proto: filter services that support dumping data in proto format. Dumps\n"
            "               will be in proto format.\n"
            "         --priority LEVEL: filter services based on specified priority\n"
            "               LEVEL must be one of CRITICAL | HIGH | NORMAL\n"
            "         --skip SERVICES: dumps all services but SERVICES (comma-separated list)\n"
            "         SERVICE [ARGS]: dumps only service SERVICE, optionally passing ARGS to it\n");
}

static bool IsSkipped(const Vector<String16>& skipped, const String16& service) {
    for (const auto& candidate : skipped) {
        if (candidate == service) {
            return true;
        }
    }
    return false;
}

static bool ConvertPriorityTypeToBitmask(const String16& type, int& bitmask) {
    if (type == PriorityDumper::PRIORITY_ARG_CRITICAL) {
        bitmask = IServiceManager::DUMP_FLAG_PRIORITY_CRITICAL;
        return true;
    }
    if (type == PriorityDumper::PRIORITY_ARG_HIGH) {
        bitmask = IServiceManager::DUMP_FLAG_PRIORITY_HIGH;
        return true;
    }
    if (type == PriorityDumper::PRIORITY_ARG_NORMAL) {
        bitmask = IServiceManager::DUMP_FLAG_PRIORITY_NORMAL;
        return true;
    }
    return false;
}

String16 ConvertBitmaskToPriorityType(int bitmask) {
    if (bitmask == IServiceManager::DUMP_FLAG_PRIORITY_CRITICAL) {
        return String16(PriorityDumper::PRIORITY_ARG_CRITICAL);
    }
    if (bitmask == IServiceManager::DUMP_FLAG_PRIORITY_HIGH) {
        return String16(PriorityDumper::PRIORITY_ARG_HIGH);
    }
    if (bitmask == IServiceManager::DUMP_FLAG_PRIORITY_NORMAL) {
        return String16(PriorityDumper::PRIORITY_ARG_NORMAL);
    }
    return String16("");
}

int Dumpsys::main(int argc, char* const argv[]) {
    Vector<String16> services;
    Vector<String16> args;
    String16 priorityType;
    Vector<String16> skippedServices;
    Vector<String16> protoServices;
    bool showListOnly = false;
    bool skipServices = false;
    bool asProto = false;
    int timeoutArgMs = 10000;
    int priorityFlags = IServiceManager::DUMP_FLAG_PRIORITY_ALL;
    static struct option longOptions[] = {{"priority", required_argument, 0, 0},
                                          {"proto", no_argument, 0, 0},
                                          {"skip", no_argument, 0, 0},
                                          {"help", no_argument, 0, 0},
                                          {0, 0, 0, 0}};

    // Must reset optind, otherwise subsequent calls will fail (wouldn't happen on main.cpp, but
    // happens on test cases).
    optind = 1;
    while (1) {
        int c;
        int optionIndex = 0;

        c = getopt_long(argc, argv, "+t:T:l", longOptions, &optionIndex);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            if (!strcmp(longOptions[optionIndex].name, "skip")) {
                skipServices = true;
            } else if (!strcmp(longOptions[optionIndex].name, "proto")) {
                asProto = true;
            } else if (!strcmp(longOptions[optionIndex].name, "help")) {
                usage();
                return 0;
            } else if (!strcmp(longOptions[optionIndex].name, "priority")) {
                priorityType = String16(String8(optarg));
                if (!ConvertPriorityTypeToBitmask(priorityType, priorityFlags)) {
                    fprintf(stderr, "\n");
                    usage();
                    return -1;
                }
            }
            break;

        case 't':
            {
                char* endptr;
                timeoutArgMs = strtol(optarg, &endptr, 10);
                timeoutArgMs = timeoutArgMs * 1000;
                if (*endptr != '\0' || timeoutArgMs <= 0) {
                    fprintf(stderr, "Error: invalid timeout(seconds) number: '%s'\n", optarg);
                    return -1;
                }
            }
            break;

        case 'T':
            {
                char* endptr;
                timeoutArgMs = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || timeoutArgMs <= 0) {
                    fprintf(stderr, "Error: invalid timeout(milliseconds) number: '%s'\n", optarg);
                    return -1;
                }
            }
            break;

        case 'l':
            showListOnly = true;
            break;

        default:
            fprintf(stderr, "\n");
            usage();
            return -1;
        }
    }

    for (int i = optind; i < argc; i++) {
        if (skipServices) {
            skippedServices.add(String16(argv[i]));
        } else {
            if (i == optind) {
                services.add(String16(argv[i]));
            } else {
                args.add(String16(argv[i]));
            }
        }
    }

    if ((skipServices && skippedServices.empty()) ||
            (showListOnly && (!services.empty() || !skippedServices.empty()))) {
        usage();
        return -1;
    }

    if (services.empty() || showListOnly) {
        services = listServices(priorityFlags, asProto);
        setServiceArgs(args, asProto, priorityFlags);
    }

    const size_t N = services.size();
    if (N > 1) {
        // first print a list of the current services
        aout << "Currently running services:" << endl;

        for (size_t i=0; i<N; i++) {
            sp<IBinder> service = sm_->checkService(services[i]);

            if (service != nullptr) {
                bool skipped = IsSkipped(skippedServices, services[i]);
                aout << "  " << services[i] << (skipped ? " (skipped)" : "") << endl;
            }
        }
    }

    if (showListOnly) {
        return 0;
    }

    for (size_t i = 0; i < N; i++) {
        const String16& serviceName = services[i];
        if (IsSkipped(skippedServices, serviceName)) continue;

        if (startDumpThread(serviceName, args) == OK) {
            bool addSeparator = (N > 1);
            if (addSeparator) {
                writeDumpHeader(STDOUT_FILENO, serviceName, priorityFlags);
            }
            std::chrono::duration<double> elapsedDuration;
            size_t bytesWritten = 0;
            status_t status =
                writeDump(STDOUT_FILENO, serviceName, std::chrono::milliseconds(timeoutArgMs),
                          asProto, elapsedDuration, bytesWritten);

            if (status == TIMED_OUT) {
                aout << endl
                     << "*** SERVICE '" << serviceName << "' DUMP TIMEOUT (" << timeoutArgMs
                     << "ms) EXPIRED ***" << endl
                     << endl;
            }

            if (addSeparator) {
                writeDumpFooter(STDOUT_FILENO, serviceName, elapsedDuration);
            }
            bool dumpComplete = (status == OK);
            stopDumpThread(dumpComplete);
        }
    }

    return 0;
}

Vector<String16> Dumpsys::listServices(int priorityFilterFlags, bool filterByProto) const {
    Vector<String16> services = sm_->listServices(priorityFilterFlags);
    services.sort(sort_func);
    if (filterByProto) {
        Vector<String16> protoServices = sm_->listServices(IServiceManager::DUMP_FLAG_PROTO);
        protoServices.sort(sort_func);
        Vector<String16> intersection;
        std::set_intersection(services.begin(), services.end(), protoServices.begin(),
                              protoServices.end(), std::back_inserter(intersection));
        services = std::move(intersection);
    }
    return services;
}

void Dumpsys::setServiceArgs(Vector<String16>& args, bool asProto, int priorityFlags) {
    // Add proto flag if dumping service as proto.
    if (asProto) {
        args.insertAt(String16(PriorityDumper::PROTO_ARG), 0);
    }

    // Add -a (dump all) flag if dumping all services, dumping normal services or
    // services not explicitly registered to a priority bucket (default services).
    if ((priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_ALL) ||
        (priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_NORMAL) ||
        (priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_DEFAULT)) {
        args.insertAt(String16("-a"), 0);
    }

    // Add priority flags when dumping services registered to a specific priority bucket.
    if ((priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_CRITICAL) ||
        (priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_HIGH) ||
        (priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_NORMAL)) {
        String16 priorityType = ConvertBitmaskToPriorityType(priorityFlags);
        args.insertAt(String16(PriorityDumper::PRIORITY_ARG), 0);
        args.insertAt(priorityType, 1);
    }
}

status_t Dumpsys::startDumpThread(const String16& serviceName, const Vector<String16>& args) {
    sp<IBinder> service = sm_->checkService(serviceName);
    if (service == nullptr) {
        aerr << "Can't find service: " << serviceName << endl;
        return NAME_NOT_FOUND;
    }

    int sfd[2];
    if (pipe(sfd) != 0) {
        aerr << "Failed to create pipe to dump service info for " << serviceName << ": "
             << strerror(errno) << endl;
        return -errno;
    }

    redirectFd_ = unique_fd(sfd[0]);
    unique_fd remote_end(sfd[1]);
    sfd[0] = sfd[1] = -1;

    // dump blocks until completion, so spawn a thread..
    activeThread_ = std::thread([=, remote_end{std::move(remote_end)}]() mutable {
        int err = service->dump(remote_end.get(), args);

        // It'd be nice to be able to close the remote end of the socketpair before the dump
        // call returns, to terminate our reads if the other end closes their copy of the
        // file descriptor, but then hangs for some reason. There doesn't seem to be a good
        // way to do this, though.
        remote_end.reset();

        if (err != 0) {
            aerr << "Error dumping service info: (" << strerror(err) << ") "
                 << serviceName << endl;
        }
    });
    return OK;
}

void Dumpsys::stopDumpThread(bool dumpComplete) {
    if (dumpComplete) {
        activeThread_.join();
    } else {
        activeThread_.detach();
    }
    /* close read end of the dump output redirection pipe */
    redirectFd_.reset();
}

void Dumpsys::writeDumpHeader(int fd, const String16& serviceName, int priorityFlags) const {
    std::string msg(
        "----------------------------------------"
        "---------------------------------------\n");
    if (priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_ALL ||
        priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_NORMAL ||
        priorityFlags == IServiceManager::DUMP_FLAG_PRIORITY_DEFAULT) {
        StringAppendF(&msg, "DUMP OF SERVICE %s:\n", String8(serviceName).c_str());
    } else {
        String16 priorityType = ConvertBitmaskToPriorityType(priorityFlags);
        StringAppendF(&msg, "DUMP OF SERVICE %s %s:\n", String8(priorityType).c_str(),
                      String8(serviceName).c_str());
    }
    WriteStringToFd(msg, fd);
}

status_t Dumpsys::writeDump(int fd, const String16& serviceName, std::chrono::milliseconds timeout,
                            bool asProto, std::chrono::duration<double>& elapsedDuration,
                            size_t& bytesWritten) const {
    status_t status = OK;
    size_t totalBytes = 0;
    auto start = std::chrono::steady_clock::now();
    auto end = start + timeout;

    int serviceDumpFd = redirectFd_.get();
    if (serviceDumpFd == -1) {
        return INVALID_OPERATION;
    }

    struct pollfd pfd = {.fd = serviceDumpFd, .events = POLLIN};

    while (true) {
        // Wrap this in a lambda so that TEMP_FAILURE_RETRY recalculates the timeout.
        auto time_left_ms = [end]() {
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - now);
            return std::max(diff.count(), 0LL);
        };

        int rc = TEMP_FAILURE_RETRY(poll(&pfd, 1, time_left_ms()));
        if (rc < 0) {
            aerr << "Error in poll while dumping service " << serviceName << " : "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        } else if (rc == 0) {
            status = TIMED_OUT;
            break;
        }

        char buf[4096];
        rc = TEMP_FAILURE_RETRY(read(redirectFd_.get(), buf, sizeof(buf)));
        if (rc < 0) {
            aerr << "Failed to read while dumping service " << serviceName << ": "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        } else if (rc == 0) {
            // EOF.
            break;
        }

        if (!WriteFully(fd, buf, rc)) {
            aerr << "Failed to write while dumping service " << serviceName << ": "
                 << strerror(errno) << endl;
            status = -errno;
            break;
        }
        totalBytes += rc;
    }

    if ((status == TIMED_OUT) && (!asProto)) {
        std::string msg = StringPrintf("\n*** SERVICE '%s' DUMP TIMEOUT (%llums) EXPIRED ***\n\n",
                                       String8(serviceName).string(), timeout.count());
        WriteStringToFd(msg, fd);
    }

    elapsedDuration = std::chrono::steady_clock::now() - start;
    bytesWritten = totalBytes;
    return status;
}

void Dumpsys::writeDumpFooter(int fd, const String16& serviceName,
                              const std::chrono::duration<double>& elapsedDuration) const {
    using std::chrono::system_clock;
    const auto finish = system_clock::to_time_t(system_clock::now());
    std::tm finish_tm;
    localtime_r(&finish, &finish_tm);
    std::stringstream oss;
    oss << std::put_time(&finish_tm, "%Y-%m-%d %H:%M:%S");
    std::string msg =
        StringPrintf("--------- %.3fs was the duration of dumpsys %s, ending at: %s\n",
                     elapsedDuration.count(), String8(serviceName).string(), oss.str().c_str());
    WriteStringToFd(msg, fd);
}
