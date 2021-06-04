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

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <vector>

#include "instructions.h"
#include "pass.h"
#include "visitor.h"
#include "word_stream.h"

namespace android {
namespace spirit {

// Transformer is the base class for a transformation that transforms a Module.
// An instance of a derived class can be added to a PassQueue and applied to a
// Module.
class Transformer : public Pass, public DoNothingVisitor {
public:
  Transformer()
      : mStreamFunctions(WordStream::Create()),
        mStreamFinal(WordStream::Create()) {}

  virtual ~Transformer() {}

  Module *run(Module *m, int *error = nullptr) override;

  std::vector<uint32_t> runAndSerialize(Module *module,
                                        int *error = nullptr) override;

  // Returns the module being transformed
  Module *getModule() const { return mModule; }

  // Inserts a new instruction before the current instruction.
  // Call this from a transform() method in a derived class.
  void insert(Instruction *);

  void visit(FunctionDefinition *fdef) override {
    mShouldRecord = (mVisit == 0);
    DoNothingVisitor::visit(fdef);
  }

  // Transforms the current instruction into a new instruction as specified by
  // the return value. If returns nullptr, deletes the current instruction.
  // Override this in a derived class for desired behavior.
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  virtual Instruction *transform(INST_CLASS *inst) {                           \
    return static_cast<Instruction *>(inst);                                   \
  }                                                                            \
  virtual void visit(INST_CLASS *inst) {                                       \
    if (!mShouldRecord) {                                                      \
      return;                                                                  \
    }                                                                          \
    if (Instruction *transformed = transform(inst)) {                          \
      transformed->Serialize(*mStream);                                        \
    }                                                                          \
  }
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

private:
  Module *mModule;
  int mVisit;
  bool mShouldRecord;
  std::unique_ptr<WordStream> mStreamFunctions;
  std::unique_ptr<WordStream> mStreamFinal;
  WordStream *mStream;
};

} // namespace spirit
} // namespace android

#endif // TRANSFORMER_H
