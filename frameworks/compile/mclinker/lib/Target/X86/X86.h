//===- X86.h --------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86_H_
#define TARGET_X86_X86_H_
#include <string>

namespace llvm {
class Target;
}  // namespace llvm

namespace mcld {

class Target;
class TargetLDBackend;

extern mcld::Target TheX86_32Target;
extern mcld::Target TheX86_64Target;

TargetLDBackend* createX86LDBackend(const llvm::Target&, const std::string&);

}  // namespace mcld

#endif  // TARGET_X86_X86_H_
