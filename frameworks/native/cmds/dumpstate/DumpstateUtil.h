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
#ifndef ANDROID_OS_DUMPSTATE_UTIL_H_
#define ANDROID_OS_DUMPSTATE_UTIL_H_

#include <cstdint>
#include <string>

/*
 * Converts seconds to milliseconds.
 */
#define SEC_TO_MSEC(second) (second * 1000)

/*
 * Converts milliseconds to seconds.
 */
#define MSEC_TO_SEC(millisecond) (millisecond / 1000)

namespace android {
namespace os {
namespace dumpstate {

/*
 * Defines the Linux account that should be executing a command.
 */
enum PrivilegeMode {
    /* Explicitly change the `uid` and `gid` to be `shell`.*/
    DROP_ROOT,
    /* Don't change the `uid` and `gid`. */
    DONT_DROP_ROOT,
    /* Prefix the command with `/PATH/TO/su root`. Won't work non user builds. */
    SU_ROOT
};

/*
 * Defines what should happen with the main output stream (`stdout` or fd) of a command.
 */
enum OutputMode {
    /* Don't change main output. */
    NORMAL_OUTPUT,
    /* Redirect main output to `stderr`. */
    REDIRECT_TO_STDERR
};

/*
 * Value object used to set command options.
 *
 * Typically constructed using a builder with chained setters. Examples:
 *
 *  CommandOptions::WithTimeout(20).AsRoot().Build();
 *  CommandOptions::WithTimeout(10).Always().RedirectStderr().Build();
 *
 * Although the builder could be used to dynamically set values. Example:
 *
 *  CommandOptions::CommandOptionsBuilder options =
 *  CommandOptions::WithTimeout(10);
 *  if (!is_user_build()) {
 *    options.AsRoot();
 *  }
 *  RunCommand("command", {"args"}, options.Build());
 */
class CommandOptions {
  private:
    class CommandOptionsValues {
      private:
        explicit CommandOptionsValues(int64_t timeout_ms);

        int64_t timeout_ms_;
        bool always_;
        PrivilegeMode account_mode_;
        OutputMode output_mode_;
        std::string logging_message_;

        friend class CommandOptions;
        friend class CommandOptionsBuilder;
    };

    explicit CommandOptions(const CommandOptionsValues& values);

    const CommandOptionsValues values;

  public:
    class CommandOptionsBuilder {
      public:
        /* Sets the command to always run, even on `dry-run` mode. */
        CommandOptionsBuilder& Always();
        /*
         * Sets the command's PrivilegeMode as `SU_ROOT` unless overridden by system property
         * 'dumpstate.unroot'.
         */
        CommandOptionsBuilder& AsRoot();
        /*
         * Runs AsRoot() on userdebug builds. No-op on user builds since 'su' is
         * not available. This is used for commands that return some useful information even
         * when run as shell.
         */
        CommandOptionsBuilder& AsRootIfAvailable();
        /* Sets the command's PrivilegeMode as `DROP_ROOT` */
        CommandOptionsBuilder& DropRoot();
        /* Sets the command's OutputMode as `REDIRECT_TO_STDERR` */
        CommandOptionsBuilder& RedirectStderr();
        /* When not empty, logs a message before executing the command.
         * Must contain a `%s`, which will be replaced by the full command line, and end on `\n`. */
        CommandOptionsBuilder& Log(const std::string& message);
        /* Builds the command options. */
        CommandOptions Build();

      private:
        explicit CommandOptionsBuilder(int64_t timeout_ms);
        CommandOptionsValues values;
        friend class CommandOptions;
    };

    /** Gets the command timeout in seconds. */
    int64_t Timeout() const;
    /** Gets the command timeout in milliseconds. */
    int64_t TimeoutInMs() const;
    /* Checks whether the command should always be run, even on dry-run mode. */
    bool Always() const;
    /** Gets the PrivilegeMode of the command. */
    PrivilegeMode PrivilegeMode() const;
    /** Gets the OutputMode of the command. */
    OutputMode OutputMode() const;
    /** Gets the logging message header, it any. */
    std::string LoggingMessage() const;

    /** Creates a builder with the requied timeout in seconds. */
    static CommandOptionsBuilder WithTimeout(int64_t timeout_sec);

    /** Creates a builder with the requied timeout in milliseconds. */
    static CommandOptionsBuilder WithTimeoutInMs(int64_t timeout_ms);

    // Common options.
    static CommandOptions DEFAULT;
    static CommandOptions AS_ROOT;
};

/*
 * System properties helper.
 */
class PropertiesHelper {
    friend class DumpstateBaseTest;

  public:
    /*
     * Gets whether device is running a `user` build.
     */
    static bool IsUserBuild();

    /*
     * When running in dry-run mode, skips the real dumps and just print the section headers.
     *
     * Useful when debugging dumpstate or other bugreport-related activities.
     *
     * Dry-run mode is enabled by setting the system property `dumpstate.dry_run` to true.
     */
    static bool IsDryRun();

    /**
     * Checks whether root availability should be overridden.
     *
     * Useful to verify how dumpstate would work in a device with an user build.
     */
    static bool IsUnroot();

  private:
    static std::string build_type_;
    static int dry_run_;
    static int unroot_;
};

/*
 * Forks a command, waits for it to finish, and returns its status.
 *
 * |fd| file descriptor that receives the command's 'stdout'.
 * |title| description of the command printed on `stdout` (or empty to skip
 * description).
 * |full_command| array containing the command (first entry) and its arguments.
 *                Must contain at least one element.
 * |options| optional argument defining the command's behavior.
 */
int RunCommandToFd(int fd, const std::string& title, const std::vector<std::string>& full_command,
                   const CommandOptions& options = CommandOptions::DEFAULT);

/*
 * Dumps the contents of a file into a file descriptor.
 *
 * |fd| file descriptor where the file is dumped into.
 * |title| description of the command printed on `stdout` (or empty to skip
 * description).
 * |path| location of the file to be dumped.
 */
int DumpFileToFd(int fd, const std::string& title, const std::string& path);

/*
 * Finds the process id by process name.
 * |ps_name| the process name we want to search for
 */
int GetPidByName(const std::string& ps_name);

}  // namespace dumpstate
}  // namespace os
}  // namespace android

#endif  // ANDROID_OS_DUMPSTATE_UTIL_H_
