/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef MIDI_IO_WRAPPER_H_
#define MIDI_IO_WRAPPER_H_

#include <libsonivox/eas_types.h>

#include <media/DataSourceBase.h>

namespace android {

struct CDataSource;
class DataSourceUnwrapper;

class MidiIoWrapper {
public:
    explicit MidiIoWrapper(const char *path);
    explicit MidiIoWrapper(int fd, off64_t offset, int64_t size);
    explicit MidiIoWrapper(CDataSource *csource);

    ~MidiIoWrapper();

    int readAt(void *buffer, int offset, int size);
    int size();

    EAS_FILE_LOCATOR getLocator();

private:
    int mFd;
    off64_t mBase;
    int64_t  mLength;
    DataSourceUnwrapper *mDataSource;
    EAS_FILE mEasFile;
};


}  // namespace android

#endif // MIDI_IO_WRAPPER_H_
