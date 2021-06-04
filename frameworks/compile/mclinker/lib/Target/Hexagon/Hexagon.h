//===- Hexagon.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGON_H_
#define TARGET_HEXAGON_HEXAGON_H_
#include <string>

namespace llvm {
class Target;
}  // namespace llvm

namespace mcld {

class Target;
class TargetLDBackend;

extern mcld::Target TheHexagonTarget;

TargetLDBackend* createHexagonLDBackend(const llvm::Target&,
                                        const std::string&);

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGON_H_
