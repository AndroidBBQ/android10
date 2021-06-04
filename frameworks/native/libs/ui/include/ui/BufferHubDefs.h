/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_BUFFER_HUB_DEFS_H_
#define ANDROID_BUFFER_HUB_DEFS_H_

#include <atomic>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpacked"
// TODO(b/118893702): remove dependency once DvrNativeBufferMetadata moved out of libdvr
#include <dvr/dvr_api.h>
#pragma clang diagnostic pop

namespace android {

namespace BufferHubDefs {

// Single buffer clients (up to 16) ownership signal.
// 32-bit atomic unsigned int.
// Each client takes 2 bits. The first bit locates in the first 16 bits of
// bufferState; the second bit locates in the last 16 bits of bufferState.
// Client states:
// Gained state 11. Exclusive write state.
// Posted state 10.
// Acquired state 01. Shared read state.
// Released state 00.
//
//  MSB                        LSB
//   |                          |
//   v                          v
// [C15|...|C1|C0|C15| ... |C1|C0]

// Maximum number of clients a buffer can have.
static constexpr int kMaxNumberOfClients = 16;

// Definition of bit masks.
//  MSB                            LSB
//   | kHighBitsMask | kLowbitsMask |
//   v               v              v
// [b31|   ...   |b16|b15|   ...  |b0]

// The location of lower 16 bits in the 32-bit buffer state.
static constexpr uint32_t kLowbitsMask = (1U << kMaxNumberOfClients) - 1U;

// The location of higher 16 bits in the 32-bit buffer state.
static constexpr uint32_t kHighBitsMask = ~kLowbitsMask;

// The client bit mask of the first client.
static constexpr uint32_t kFirstClientBitMask = (1U << kMaxNumberOfClients) + 1U;

// Returns true if any of the client is in gained state.
static inline bool isAnyClientGained(uint32_t state) {
    uint32_t highBits = state >> kMaxNumberOfClients;
    uint32_t lowBits = state & kLowbitsMask;
    return highBits == lowBits && lowBits != 0U;
}

// Returns true if the input client is in gained state.
static inline bool isClientGained(uint32_t state, uint32_t client_bit_mask) {
    return state == client_bit_mask;
}

// Returns true if any of the client is in posted state.
static inline bool isAnyClientPosted(uint32_t state) {
    uint32_t highBits = state >> kMaxNumberOfClients;
    uint32_t lowBits = state & kLowbitsMask;
    uint32_t postedOrAcquired = highBits ^ lowBits;
    return postedOrAcquired & highBits;
}

// Returns true if the input client is in posted state.
static inline bool isClientPosted(uint32_t state, uint32_t client_bit_mask) {
    uint32_t clientBits = state & client_bit_mask;
    if (clientBits == 0U) return false;
    uint32_t lowBits = clientBits & kLowbitsMask;
    return lowBits == 0U;
}

// Return true if any of the client is in acquired state.
static inline bool isAnyClientAcquired(uint32_t state) {
    uint32_t highBits = state >> kMaxNumberOfClients;
    uint32_t lowBits = state & kLowbitsMask;
    uint32_t postedOrAcquired = highBits ^ lowBits;
    return postedOrAcquired & lowBits;
}

// Return true if the input client is in acquired state.
static inline bool isClientAcquired(uint32_t state, uint32_t client_bit_mask) {
    uint32_t clientBits = state & client_bit_mask;
    if (clientBits == 0U) return false;
    uint32_t highBits = clientBits & kHighBitsMask;
    return highBits == 0U;
}

// Returns true if the input client is in released state.
static inline bool isClientReleased(uint32_t state, uint32_t client_bit_mask) {
    return (state & client_bit_mask) == 0U;
}

// Returns the next available buffer client's client_state_masks.
// @params union_bits. Union of all existing clients' client_state_masks.
static inline uint32_t findNextAvailableClientStateMask(uint32_t union_bits) {
    uint32_t lowUnion = union_bits & kLowbitsMask;
    if (lowUnion == kLowbitsMask) return 0U;
    uint32_t incremented = lowUnion + 1U;
    uint32_t difference = incremented ^ lowUnion;
    uint32_t newLowBit = (difference + 1U) >> 1;
    return newLowBit + (newLowBit << kMaxNumberOfClients);
}

struct __attribute__((aligned(8))) MetadataHeader {
    // Internal data format, which can be updated as long as the size, padding and field alignment
    // of the struct is consistent within the same ABI. As this part is subject for future updates,
    // it's not stable cross Android version, so don't have it visible from outside of the Android
    // platform (include Apps and vendor HAL).

    // Every client takes up one bit from the higher 32 bits and one bit from the lower 32 bits in
    // bufferState.
    std::atomic<uint32_t> bufferState;

    // Every client takes up one bit in fenceState. Only the lower 32 bits are valid. The upper 32
    // bits are there for easier manipulation, but the value should be ignored.
    std::atomic<uint32_t> fenceState;

    // Every client takes up one bit from the higher 32 bits and one bit from the lower 32 bits in
    // activeClientsBitMask.
    std::atomic<uint32_t> activeClientsBitMask;

    // Explicit padding 4 bytes.
    uint32_t padding;

    // The index of the buffer queue where the buffer belongs to.
    uint64_t queueIndex;

    // Public data format, which should be updated with caution. See more details in dvr_api.h
    DvrNativeBufferMetadata metadata;
};

static_assert(sizeof(MetadataHeader) == 128, "Unexpected MetadataHeader size");
static constexpr size_t kMetadataHeaderSize = sizeof(MetadataHeader);

/**
 * android.frameworks.bufferhub@1.0::BufferTraits.bufferInfo is an opaque handle. See
 * https://cs.corp.google.com/android/frameworks/hardware/interfaces/bufferhub/1.0/types.hal for
 * more details about android.frameworks.bufferhub@1.0::BufferTraits.
 *
 * This definition could be changed, but implementation of BufferHubService::buildBufferInfo
 * (frameworks/native/services/bufferhub), VtsHalBufferHubV1_0TargetTest
 * (frameworks/hardware/interfaces/bufferhub) and BufferHubBuffer::readBufferTraits (libui) will
 * also need to be updated.
 *
 * It's definition should follow the following format:
 * {
 *   NumFds = 2,
 *   NumInts = 3,
 *   data[0] = Ashmem fd for BufferHubMetadata,
 *   data[1] = event fd,
 *   data[2] = buffer id,
 *   data[3] = client state bit mask,
 *   data[4] = user metadata size,
 * }
 */
static constexpr int kBufferInfoNumFds = 2;
static constexpr int kBufferInfoNumInts = 3;

} // namespace BufferHubDefs

} // namespace android

#endif // ANDROID_BUFFER_HUB_DEFS_H_
