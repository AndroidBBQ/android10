/*
 * Copyright 2016 The Android Open Source Project
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

/*
 * Replayer - Main.cpp
 *
 * 1. Get flags from command line
 * 2. Commit actions or settings based on the flags
 * 3. Initalize a replayer object with the filename passed in
 * 4. Replay
 * 5. Exit successfully or print error statement
 */

#include <Replayer.h>

#include <csignal>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace android;

void printHelpMenu() {
    std::cout << "SurfaceReplayer options:\n";
    std::cout << "Usage: surfacereplayer [OPTIONS...] <TRACE FILE>\n";
    std::cout << "  File path must be absolute" << std::endl << std::endl;

    std::cout << "  -m  Stops the replayer at the start of the trace and switches ";
                 "to manual replay\n";

    std::cout << "\n  -t [Number of Threads]  Specifies the number of threads to be used while "
                 "replaying (default is " << android::DEFAULT_THREADS << ")\n";

    std::cout << "\n  -s [Timestamp]  Specify at what timestamp should the replayer switch "
                 "to manual replay\n";

    std::cout << "  -n  Ignore timestamps and run through trace as fast as possible\n";

    std::cout << "  -l  Indefinitely loop the replayer\n";

    std::cout << "  -h  Display help menu\n";

    std::cout << std::endl;
}

int main(int argc, char** argv) {
    std::string filename;
    bool loop = false;
    bool wait = true;
    bool pauseBeginning = false;
    int numThreads = DEFAULT_THREADS;
    long stopHere = -1;

    int opt = 0;
    while ((opt = getopt(argc, argv, "mt:s:nlh?")) != -1) {
        switch (opt) {
            case 'm':
                pauseBeginning = true;
                break;
            case 't':
                numThreads = atoi(optarg);
                break;
            case 's':
                stopHere = atol(optarg);
                break;
            case 'n':
                wait = false;
                break;
            case 'l':
                loop = true;
                break;
            case 'h':
            case '?':
                printHelpMenu();
                exit(0);
            default:
                std::cerr << "Invalid argument...exiting" << std::endl;
                printHelpMenu();
                exit(0);
        }
    }

    char** input = argv + optind;
    if (input[0] == nullptr) {
        std::cerr << "No trace file provided...exiting" << std::endl;
        abort();
    }
    filename.assign(input[0]);

    status_t status = NO_ERROR;
    do {
        android::Replayer r(filename, pauseBeginning, numThreads, wait, stopHere);
        status = r.replay();
    } while(loop);

    if (status == NO_ERROR) {
        std::cout << "Successfully finished replaying trace" << std::endl;
    } else {
        std::cerr << "Trace replayer returned error: " << status << std::endl;
    }

    return 0;
}
