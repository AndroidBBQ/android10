//===- HexagonEmulation.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Hexagon.h"
#include "mcld/LinkerScript.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFEmulation.h"

namespace mcld {

static bool MCLDEmulateHexagonELF(LinkerScript& pScript,
                                  LinkerConfig& pConfig) {
  if (!MCLDEmulateELF(pScript, pConfig))
    return false;

  // set up bitclass and endian
  pConfig.targets().setEndian(TargetOptions::Little);
  pConfig.targets().setBitClass(32);

  // set up target-dependent constraints of attributes
  pConfig.attribute().constraint().enableWholeArchive();
  pConfig.attribute().constraint().enableAsNeeded();
  pConfig.attribute().constraint().setSharedSystem();

  // set up the predefined attributes
  pConfig.attribute().predefined().unsetWholeArchive();
  pConfig.attribute().predefined().unsetAsNeeded();
  pConfig.attribute().predefined().setDynamic();
  return true;
}

//===----------------------------------------------------------------------===//
// emulateHexagonLD - the help function to emulate Hexagon ld
//===----------------------------------------------------------------------===//
bool emulateHexagonLD(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker has not supported yet");
    return false;
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker has not supported yet");
    return false;
  }

  return MCLDEmulateHexagonELF(pScript, pConfig);
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// HexagonEmulation
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeHexagonEmulation() {
  // Register the emulation
  mcld::TargetRegistry::RegisterEmulation(mcld::TheHexagonTarget,
                                          mcld::emulateHexagonLD);
}
