/*
 * Copyright 2017, The Android Open Source Project
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

#include "transformer.h"

#include "module.h"

namespace android {
namespace spirit {

Module *Transformer::run(Module *module, int *error) {
  auto words = runAndSerialize(module, error);
  return Deserialize<Module>(words);
}

std::vector<uint32_t> Transformer::runAndSerialize(Module *m, int *error) {
  mModule = m;

  // Since contents in the decoration or global section may change, transform
  // and serialize the function definitions first.
  mVisit = 0;
  mShouldRecord = false;
  mStream = mStreamFunctions.get();
  m->accept(this);

  // Record in the annotation section any new annotations added
  m->consolidateAnnotations();

  // After the functions are transformed, serialize the other sections to
  // capture any changes made during the function transformation, and append
  // the new words from function serialization.

  mVisit = 1;
  mShouldRecord = true;
  mStream = mStreamFinal.get();

  // TODO fix Module::accept() to have the header serialization code there
  m->SerializeHeader(*mStream);
  m->accept(this);

  auto output = mStream->getWords();
  auto functions = mStreamFunctions->getWords();
  output.insert(output.end(), functions.begin(), functions.end());

  if (error) {
    *error = 0;
  }

  return output;
}

void Transformer::insert(Instruction *inst) {
  // TODO: warn on nullptr inst
  inst->Serialize(*mStream);
}

} // namespace spirit
} // namespace android
