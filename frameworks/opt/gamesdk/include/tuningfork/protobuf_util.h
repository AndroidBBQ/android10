/*
 * Copyright 2018 The Android Open Source Project
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

#pragma once

#include "tuningfork/tuningfork.h"

#include <vector>
#include <cstdint>
#include <cstdlib>

namespace tuningfork {

template <typename T>
bool Deserialize(const std::vector<uint8_t> &ser, T &pb) {
    return pb.ParseFromArray(ser.data(), ser.size());
}
template <typename T>
bool Serialize(const T &pb, std::vector<uint8_t> &ser) {
    ser.resize(pb.ByteSize());
    return pb.SerializeToArray(ser.data(), ser.size());
}
template <typename T>
std::vector<uint8_t> Serialize(const T &pb) {
    std::vector<uint8_t> ser(pb.ByteSize());
    pb.SerializeToArray(ser.data(), ser.size());
    return ser;
}
// Serialize to a CProtobuf. The caller takes ownership of the returned serialization and must
//  call CProtobufSerialization_Free to deallocate any memory.
template <typename T>
CProtobufSerialization CProtobufSerialization_Alloc(const T &pb) {
    CProtobufSerialization cser;
    cser.bytes = (uint8_t*)::malloc(pb.ByteSize());
    cser.size = pb.ByteSize();
    cser.dealloc = ::free;
    pb.SerializeToArray(cser.bytes, cser.size);
    return cser;
}

void CProtobufSerialization_Free(CProtobufSerialization* ser);

} // namespace tuningfork {
