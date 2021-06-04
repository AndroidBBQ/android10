//===- MipsDiagnostic.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DWARFLineInfo.h"
#include "mcld/Support/TargetRegistry.h"
#include "Mips.h"

namespace {

//===----------------------------------------------------------------------===//
// createMipsDiagnostic - the help function to create corresponding
// MipsDiagnostic
//===----------------------------------------------------------------------===//
mcld::DiagnosticLineInfo* createMipsDiagLineInfo(const mcld::Target& pTarget,
                                                 const std::string& pTriple) {
  return new mcld::DWARFLineInfo();
}

}  // anonymous namespace

//===----------------------------------------------------------------------===//
// InitializeMipsDiagnostic
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeMipsDiagnosticLineInfo() {
  mcld::TargetRegistry::RegisterDiagnosticLineInfo(mcld::TheMipselTarget,
                                                   createMipsDiagLineInfo);
  mcld::TargetRegistry::RegisterDiagnosticLineInfo(mcld::TheMips64elTarget,
                                                   createMipsDiagLineInfo);
}
