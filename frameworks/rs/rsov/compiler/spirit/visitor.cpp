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

#include "visitor.h"

#include "instructions.h"
#include "module.h"

namespace android {
namespace spirit {

void DoNothingVisitor::visit(Entity *e) { e->accept(this); }

void DoNothingVisitor::visit(Module *m) { m->accept(this); }

void DoNothingVisitor::visit(EntryPointDefinition *entry) {
  entry->accept(this);
}

void DoNothingVisitor::visit(DebugInfoSection *dinfo) { dinfo->accept(this); }

void DoNothingVisitor::visit(AnnotationSection *a) { a->accept(this); }

void DoNothingVisitor::visit(GlobalSection *g) { g->accept(this); }
void DoNothingVisitor::visit(FunctionDeclaration *fdecl) {
  fdecl->accept(this);
}
void DoNothingVisitor::visit(Block *b) { b->accept(this); }
void DoNothingVisitor::visit(FunctionDefinition *fdef) { fdef->accept(this); }
void DoNothingVisitor::visit(Instruction *inst) { inst->accept(this); }
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  void DoNothingVisitor::visit(INST_CLASS *) {}
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

} // namespace spirit
} // namespace android
