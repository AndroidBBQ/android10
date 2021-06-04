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

#include "bcc/BCCContext.h"

#include "BCCContextImpl.h"
#include "Log.h"
#include "bcc/Source.h"

#include <new>

using namespace bcc;

static BCCContext *GlobalContext = nullptr;

BCCContext *BCCContext::GetOrCreateGlobalContext() {
  if (GlobalContext == nullptr) {
    GlobalContext = new (std::nothrow) BCCContext();
    if (GlobalContext == nullptr) {
      ALOGE("Out of memory when allocating global BCCContext!");
    }
  }
  return GlobalContext;
}

void BCCContext::DestroyGlobalContext() {
  delete GlobalContext;
  GlobalContext = nullptr;
}

BCCContext::BCCContext() : mImpl(new BCCContextImpl(*this)) { }

BCCContext::~BCCContext() {
  delete mImpl;
  if (this == GlobalContext) {
    // We're deleting the context returned from GetOrCreateGlobalContext().
    // Reset the GlobalContext.
    GlobalContext = nullptr;
  }
}

void BCCContext::addSource(Source &pSource)
{ mImpl->mOwnSources.insert(&pSource); }

void BCCContext::removeSource(Source &pSource)
{ mImpl->mOwnSources.erase(&pSource); }

llvm::LLVMContext &BCCContext::getLLVMContext()
{ return mImpl->mLLVMContext; }

const llvm::LLVMContext &BCCContext::getLLVMContext() const
{ return mImpl->mLLVMContext; }
