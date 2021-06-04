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

#ifndef BINDING_AAUDIOSERVICEDEFINITIONS_H
#define BINDING_AAUDIOSERVICEDEFINITIONS_H

#include <stdint.h>
#include <utils/RefBase.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>

#include <aaudio/AAudio.h>

using android::NO_ERROR;
using android::IBinder;

namespace android {

enum aaudio_service_commands_t {
    REGISTER_CLIENT = IBinder::FIRST_CALL_TRANSACTION,
    OPEN_STREAM,
    CLOSE_STREAM,
    GET_STREAM_DESCRIPTION,
    START_STREAM,
    PAUSE_STREAM,
    STOP_STREAM,
    FLUSH_STREAM,
    REGISTER_AUDIO_THREAD,
    UNREGISTER_AUDIO_THREAD
};

enum aaudio_client_commands_t {
    ON_STREAM_CHANGE = IBinder::FIRST_CALL_TRANSACTION
};

} // namespace android

namespace aaudio {

typedef int32_t  aaudio_handle_t;

#define AAUDIO_HANDLE_INVALID  ((aaudio_handle_t) -1)

// This must be a fixed width so it can be in shared memory.
enum RingbufferFlags : uint32_t {
    NONE = 0,
    RATE_ISOCHRONOUS = 0x0001,
    RATE_ASYNCHRONOUS = 0x0002,
    COHERENCY_DMA = 0x0004,
    COHERENCY_ACQUIRE_RELEASE = 0x0008,
    COHERENCY_AUTO = 0x0010,
};

// This is not passed through Binder.
// Client side code will convert Binder data and fill this descriptor.
typedef struct RingBufferDescriptor_s {
    uint8_t* dataAddress;       // offset from read or write block
    int64_t* writeCounterAddress;
    int64_t* readCounterAddress;
    int32_t  bytesPerFrame;     // index is in frames
    int32_t  framesPerBurst;    // for ISOCHRONOUS queues
    int32_t  capacityInFrames;  // zero if unused
    RingbufferFlags flags;
} RingBufferDescriptor;

// This is not passed through Binder.
// Client side code will convert Binder data and fill this descriptor.
typedef struct EndpointDescriptor_s {
    // Set capacityInFrames to zero if Queue is unused.
    RingBufferDescriptor upMessageQueueDescriptor;   // server to client
    RingBufferDescriptor downMessageQueueDescriptor; // client to server
    RingBufferDescriptor dataQueueDescriptor;    // playback or capture
} EndpointDescriptor;

} // namespace aaudio

#endif //BINDING_AAUDIOSERVICEDEFINITIONS_H
