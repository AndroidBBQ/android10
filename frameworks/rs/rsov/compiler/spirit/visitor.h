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

#ifndef VISITOR_H
#define VISITOR_H

#include <functional>

namespace android {
namespace spirit {

class Entity;
class Module;
class EntryPointDefinition;
class DebugInfoSection;
class AnnotationSection;
class GlobalSection;
class FunctionDeclaration;
class Block;
class FunctionDefinition;
class Instruction;
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS) class INST_CLASS;
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

class IVisitor {
public:
  virtual ~IVisitor() {}

  virtual void visit(Entity *) = 0;
  virtual void visit(Module *) = 0;
  virtual void visit(EntryPointDefinition *) = 0;
  virtual void visit(DebugInfoSection *) = 0;
  virtual void visit(AnnotationSection *) = 0;
  virtual void visit(GlobalSection *) = 0;
  virtual void visit(FunctionDeclaration *) = 0;
  virtual void visit(Block *) = 0;
  virtual void visit(FunctionDefinition *) = 0;
  virtual void visit(Instruction *) = 0;
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  virtual void visit(INST_CLASS *) = 0;
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION
};

class DoNothingVisitor : public IVisitor {
public:
  virtual ~DoNothingVisitor() {}

  virtual void visit(Entity *e);
  virtual void visit(Module *m);
  virtual void visit(EntryPointDefinition *);
  virtual void visit(DebugInfoSection *dinfo);
  virtual void visit(AnnotationSection *a);
  virtual void visit(GlobalSection *g);
  virtual void visit(FunctionDeclaration *fdecl);
  virtual void visit(Block *b);
  virtual void visit(FunctionDefinition *fdef);
  // This visit(Instruction *) necessary?
  virtual void visit(Instruction *inst);
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS) virtual void visit(INST_CLASS *);
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION
};

template <typename T> class InstructionVisitor : public DoNothingVisitor {
public:
  InstructionVisitor(T action) : mAction(action) {}

  virtual ~InstructionVisitor() {}

#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  void visit(INST_CLASS *inst) override { mAction((Instruction *)inst); }
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

private:
  T mAction;
};

template <typename T> static IVisitor *CreateInstructionVisitor(T action) {
  return new InstructionVisitor<decltype(action)>(action);
}

} // namespace spirit
} // namespace android

#endif // VISITOR_H
