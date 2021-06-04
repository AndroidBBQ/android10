/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef RS_SPIRV_WRITER_H
#define RS_SPIRV_WRITER_H

#include <string>

namespace llvm {
class Module;
class raw_ostream;
} // namespace llvm

namespace bcinfo {
class MetadataExtractor;
} // namespace bcinfo

namespace rs2spirv {

class Context;

bool WriteSPIRV(rs2spirv::Context &Ctxt, llvm::Module *M,
                llvm::raw_ostream &OS, std::string &ErrMsg);

} // namespace rs2spirv

#endif
