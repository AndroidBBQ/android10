/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef FRAMEWORK_NATIVE_CMD_DUMPSYS_H_
#define FRAMEWORK_NATIVE_CMD_DUMPSYS_H_

#include <thread>

#include <android-base/unique_fd.h>
#include <binder/IServiceManager.h>

namespace android {

class Dumpsys {
  public:
    explicit Dumpsys(android::IServiceManager* sm) : sm_(sm) {
    }
    /**
     * Main entry point into dumpsys.
     */
    int main(int argc, char* const argv[]);

    /**
     * Returns a list of services.
     * @param priorityFlags filter services by specified priorities
     * @param supportsProto filter services that support proto dumps
     * @return list of services
     */
    Vector<String16> listServices(int priorityFlags, bool supportsProto) const;

    /**
     * Modifies @{code args} to add additional arguments  to indicate if the service
     * must dump as proto or dump to a certian priority bucket.
     * @param args initial list of arguments to pass to service dump method.
     * @param asProto dump service as proto by passing an additional --proto arg
     * @param priorityFlags indicates priority of dump by passing additional priority args
     * to the service
     */
    static void setServiceArgs(Vector<String16>& args, bool asProto, int priorityFlags);

    /**
     * Starts a thread to connect to a service and get its dump output. The thread redirects
     * the output to a pipe. Thread must be stopped by a subsequent callto {@code
     * stopDumpThread}.
     * @param serviceName
     * @param args list of arguments to pass to service dump method.
     * @return {@code OK} thread is started successfully.
     *         {@code NAME_NOT_FOUND} service could not be found.
     *         {@code != OK} error
     */
    status_t startDumpThread(const String16& serviceName, const Vector<String16>& args);

    /**
     * Writes a section header to a file descriptor.
     * @param fd file descriptor to write data
     * @param serviceName
     * @param priorityFlags dump priority specified
     */
    void writeDumpHeader(int fd, const String16& serviceName, int priorityFlags) const;

    /**
     * Redirects service dump to a file descriptor. This requires
     * {@code startDumpThread} to be called successfully otherwise the function will
     * return {@code INVALID_OPERATION}.
     * @param fd file descriptor to write data
     * @param serviceName
     * @param timeout timeout to terminate the dump if not completed
     * @param asProto used to supresses additional output to the fd such as timeout
     * error messages
     * @param elapsedDuration returns elapsed time in seconds
     * @param bytesWritten returns number of bytes written
     * @return {@code OK} if successful
     *         {@code TIMED_OUT} dump timed out
     *         {@code INVALID_OPERATION} invalid state
     *         {@code != OK} error
     */
    status_t writeDump(int fd, const String16& serviceName, std::chrono::milliseconds timeout,
                       bool asProto, std::chrono::duration<double>& elapsedDuration,
                       size_t& bytesWritten) const;

    /**
     * Writes a section footer to a file descriptor with duration info.
     * @param fd file descriptor to write data
     * @param serviceName
     * @param elapsedDuration duration of dump
     */
    void writeDumpFooter(int fd, const String16& serviceName,
                         const std::chrono::duration<double>& elapsedDuration) const;

    /**
     * Terminates dump thread.
     * @param dumpComplete If {@code true}, indicates the dump was successfully completed and
     * tries to join the thread. Otherwise thread is detached.
     */
    void stopDumpThread(bool dumpComplete);

    /**
     * Returns file descriptor of the pipe used to dump service data. This assumes
     * {@code startDumpThread} was called successfully.
     */
    int getDumpFd() const {
        return redirectFd_.get();
    }

  private:
    android::IServiceManager* sm_;
    std::thread activeThread_;
    mutable android::base::unique_fd redirectFd_;
};
}

#endif  // FRAMEWORK_NATIVE_CMD_DUMPSYS_H_
