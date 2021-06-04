//===- AArch64Diagnostic.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DWARFLineInfo.h"
#include "mcld/Support/TargetRegistry.h"
#include "AArch64.h"

namespace mcld {
//===----------------------------------------------------------------------===//
// createAArch64Diagnostic - the help function to create corresponding
// AArch64Diagnostic
//===----------------------------------------------------------------------===//
DiagnosticLineInfo* createAArch64DiagLineInfo(const mcld::Target& pTarget,
                                              const std::string& pTriple) {
  return new DWARFLineInfo();
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// InitializeAArch64Diagnostic
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeAArch64DiagnosticLineInfo() {
  // Register the linker frontend
  mcld::TargetRegistry::RegisterDiagnosticLineInfo(
      mcld::TheAArch64Target, mcld::createAArch64DiagLineInfo);
}
