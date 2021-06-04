/*
 * Copyright 2012, The Android Open Source Project
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

#ifndef BCC_CONTEXT_H
#define BCC_CONTEXT_H

namespace llvm {
  class LLVMContext;
}

namespace bcc {

class BCCContextImpl;
class Source;

/*
 * class BCCContext manages the global data across the libbcc infrastructure.
 */
class BCCContext {
public:
  BCCContextImpl *const mImpl;

  BCCContext();
  ~BCCContext();

  llvm::LLVMContext &getLLVMContext();
  const llvm::LLVMContext &getLLVMContext() const;

  void addSource(Source &pSource);
  void removeSource(Source &pSource);

  // Global BCCContext
  static BCCContext *GetOrCreateGlobalContext();
  static void DestroyGlobalContext();
};

} // namespace bcc

#endif  // BCC_CONTEXT_H
