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

#include "BCCContextImpl.h"

#include <vector>

#include <llvm/ADT/STLExtras.h>

#include "bcc/Source.h"

using namespace bcc;

BCCContextImpl::~BCCContextImpl() {
  // Another temporary container is needed to store the Source objects that we
  // are going to destroy. Since the destruction of Source object will call
  // removeSource() and change the content of OwnSources.
  std::vector<Source *> Sources(mOwnSources.begin(), mOwnSources.end());
  llvm::DeleteContainerPointers(Sources);
}
