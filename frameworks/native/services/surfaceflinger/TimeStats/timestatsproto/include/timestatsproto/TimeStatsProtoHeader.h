/*
 * Copyright (C) 2018 The Android Open Source Projectlayerproto/LayerProtoHeader.h
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

// pragma is used here to disable the warnings emitted from the protobuf
// headers. By adding #pragma before including layer.pb.h, it supresses
// protobuf warnings, but allows the rest of the files to continuing using
// the current flags.
// This file should be included instead of directly including layer.b.h
#pragma GCC system_header
#include <timestats.pb.h>
