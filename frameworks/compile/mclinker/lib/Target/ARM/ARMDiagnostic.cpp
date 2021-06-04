//===- ARMDiagnostic.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DWARFLineInfo.h"
#include "mcld/Support/TargetRegistry.h"
#include "ARM.h"

namespace mcld {
//===----------------------------------------------------------------------===//
// createARMDiagnostic - the help function to create corresponding ARMDiagnostic
//===----------------------------------------------------------------------===//
DiagnosticLineInfo* createARMDiagLineInfo(const mcld::Target& pTarget,
                                          const std::string& pTriple) {
  return new DWARFLineInfo();
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// InitializeARMDiagnostic
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeARMDiagnosticLineInfo() {
  // Register the linker frontend
  mcld::TargetRegistry::RegisterDiagnosticLineInfo(mcld::TheARMTarget,
                                                   mcld::createARMDiagLineInfo);
  mcld::TargetRegistry::RegisterDiagnosticLineInfo(mcld::TheThumbTarget,
                                                   mcld::createARMDiagLineInfo);
}
