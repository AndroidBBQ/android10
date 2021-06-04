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

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cutils/properties.h>
#include <cutils/sockets.h>

// This program will trigger the dumpstate service to start a call to
// dumpstate, then connect to the dumpstate local client to read the
// output. All of the dumpstate output is written to stdout, including
// any errors encountered while reading/writing the output.
int main() {

  fprintf(stderr, "=============================================================================\n");
  fprintf(stderr, "WARNING: flat bugreports are deprecated, use adb bugreport <zip_file> instead\n");
  fprintf(stderr, "=============================================================================\n\n\n");

  // Start the dumpstate service.
  property_set("ctl.start", "dumpstate");

  // Socket will not be available until service starts.
  int s;
  for (int i = 0; i < 20; i++) {
    s = socket_local_client("dumpstate", ANDROID_SOCKET_NAMESPACE_RESERVED,
                            SOCK_STREAM);
    if (s >= 0)
      break;
    // Try again in 1 second.
    sleep(1);
  }

  if (s == -1) {
    printf("Failed to connect to dumpstate service: %s\n", strerror(errno));
    return 1;
  }

  // Set a timeout so that if nothing is read in 3 minutes, we'll stop
  // reading and quit. No timeout in dumpstate is longer than 60 seconds,
  // so this gives lots of leeway in case of unforeseen time outs.
  struct timeval tv;
  tv.tv_sec = 3 * 60;
  tv.tv_usec = 0;
  if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
    printf("WARNING: Cannot set socket timeout: %s\n", strerror(errno));
  }

  while (1) {
    char buffer[65536];
    ssize_t bytes_read = TEMP_FAILURE_RETRY(read(s, buffer, sizeof(buffer)));
    if (bytes_read == 0) {
      break;
    } else if (bytes_read == -1) {
      // EAGAIN really means time out, so change the errno.
      if (errno == EAGAIN) {
        errno = ETIMEDOUT;
      }
      printf("\nBugreport read terminated abnormally (%s).\n", strerror(errno));
      break;
    }

    ssize_t bytes_to_send = bytes_read;
    ssize_t bytes_written;
    do {
      bytes_written = TEMP_FAILURE_RETRY(write(STDOUT_FILENO,
                                               buffer + bytes_read - bytes_to_send,
                                               bytes_to_send));
      if (bytes_written == -1) {
        printf("Failed to write data to stdout: read %zd, trying to send %zd (%s)\n",
               bytes_read, bytes_to_send, strerror(errno));
        return 1;
      }
      bytes_to_send -= bytes_written;
    } while (bytes_written != 0 && bytes_to_send > 0);
  }

  close(s);
  return 0;
}
