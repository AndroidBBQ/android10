/*
 * Copyright 2020, The Android Open Source Project
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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

int main(int argc, const char **argv) {
  // Create a copy of argv strings that we can modify, and then eventually
  // const_cast away the const-ness of the buffers to call execv().
  std::vector<std::unique_ptr<std::string>> argv_strings;
  std::vector<const char *> argv_chars;

  // Replace lld.exe with lld-bin\lld.exe instead on Windows.
  argv_strings.push_back(std::make_unique<std::string>(argv[0]));
  size_t idx = argv_strings[0]->rfind("lld.exe");
  argv_strings[0]->insert(idx, "lld-bin\\");
  argv_chars.push_back(argv_strings[0]->c_str());

  // Make a copy of every other argv entry, and map a pointer to the C string
  // buffer as argv_chars for use with execv() later.
  for (int i = 1; i < argc; ++i) {
    argv_strings.push_back(std::make_unique<std::string>(argv[i]));
    argv_chars.push_back(argv_strings[i]->c_str());
  }

  // execv() expects a nullptr to terminate the argument list for argv.
  argv_chars.push_back(nullptr);

  // We cast away the const-ness of the char buffers, but it should be safe,
  // since we own these strings.
  int status = execv(argv_chars[0], const_cast<char **>(argv_chars.data()));

  // We shouldn't get here unless we failed to execute the new binary.
  if (status != 0) {
    std::string command;
    bool first = true;
    for (auto arg : argv_chars) {
      if (arg) {
        if (!first) {
          command.append(" ");
        } else {
          first = false;
        }
        command.append(arg);
      }
    }
    fprintf(stderr, "Failed to execute command: %s\n", command.c_str());
  }
  return status;
}
