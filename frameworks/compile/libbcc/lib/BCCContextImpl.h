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

#ifndef BCC_CORE_CONTEXT_IMPL_H
#define BCC_CORE_CONTEXT_IMPL_H

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/LLVMContext.h>

namespace bcc {

class BCCContext;
class Source;

/*
 * class BCCContextImpl contains the implementation of BCCContext.
 */
class BCCContextImpl {
public:
  llvm::LLVMContext mLLVMContext;

  // The set of sources that initialized in this context. They will be destroyed
  // automatically when this context is gone.
  llvm::SmallPtrSet<Source *, 2> mOwnSources;

  explicit BCCContextImpl(BCCContext &pContext) { }
  ~BCCContextImpl();
};

} // namespace bcc

#endif  // BCC_CORE_CONTEXT_IMPL_H
