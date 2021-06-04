/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef BUFFERQUEUE_SOURCE_H_
#define BUFFERQUEUE_SOURCE_H_

#include <media/DataSource.h>

// number of SLuint32 fields to store a buffer event message in an item, by mapping each
//   to the item key (SLuint32), the item size (SLuint32), and the item data (mask on SLuint32)
#define NB_BUFFEREVENT_ITEM_FIELDS 3

namespace android {

// a Stagefright DataSource that pulls data from an AndroidBufferQueue

class BufferQueueSource : public DataSource {
public:

    // store an item structure to indicate a processed buffer
    static const SLuint32 kItemProcessed[NB_BUFFEREVENT_ITEM_FIELDS];

    explicit BufferQueueSource(IAndroidBufferQueue *androidBufferQueue);

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);

    virtual ~BufferQueueSource();

private:
    // the Android Buffer Queue from which data is consumed
    IAndroidBufferQueue* const mAndroidBufferQueueSource;

    // a monotonically increasing offset used to translate an offset from the beginning
    // of the stream, to an offset in each buffer from the buffer queue source
    off64_t mStreamToBqOffset;

    // indicates whether an EOS command has been reached when consuming the buffers in the queue
    bool mEosReached;

    BufferQueueSource(const BufferQueueSource &);
    BufferQueueSource &operator=(const BufferQueueSource &);
};

}  // namespace android

#endif  // BUFFERQUEUE_SOURCE_H_

