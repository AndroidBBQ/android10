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

#ifndef ANDROID_BINDING_AUDIO_ENDPOINT_PARCELABLE_H
#define ANDROID_BINDING_AUDIO_ENDPOINT_PARCELABLE_H

#include <stdint.h>

//#include <sys/mman.h>
#include <android-base/unique_fd.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>

#include "binding/AAudioServiceDefinitions.h"
#include "binding/RingBufferParcelable.h"

using android::status_t;
using android::Parcel;
using android::Parcelable;

namespace aaudio {

/**
 * Container for information about the message queues plus
 * general stream information needed by AAudio clients.
 * It contains no addresses, just sizes, offsets and file descriptors for
 * shared memory that can be passed through Binder.
 */
class AudioEndpointParcelable : public Parcelable {
public:
    AudioEndpointParcelable();
    virtual ~AudioEndpointParcelable();

    /**
     * Add the file descriptor to the table.
     * @return index in table or negative error
     */
    int32_t addFileDescriptor(const android::base::unique_fd& fd, int32_t sizeInBytes);

    virtual status_t writeToParcel(Parcel* parcel) const override;

    virtual status_t readFromParcel(const Parcel* parcel) override;

    aaudio_result_t resolve(EndpointDescriptor *descriptor);

    aaudio_result_t close();

    void dump();

public: // TODO add getters
    // Set capacityInFrames to zero if Queue is unused.
    RingBufferParcelable    mUpMessageQueueParcelable;   // server to client
    RingBufferParcelable    mDownMessageQueueParcelable; // to server
    RingBufferParcelable    mUpDataQueueParcelable;      // eg. record, could share same queue
    RingBufferParcelable    mDownDataQueueParcelable;    // eg. playback

private:
    aaudio_result_t         validate() const;

    int32_t                 mNumSharedMemories = 0;
    SharedMemoryParcelable  mSharedMemories[MAX_SHARED_MEMORIES];
};

} /* namespace aaudio */

#endif //ANDROID_BINDING_AUDIO_ENDPOINT_PARCELABLE_H
