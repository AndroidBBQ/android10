//===- AArch64Emulation.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFEmulation.h"

namespace mcld {

static bool MCLDEmulateAArch64ELF(LinkerScript& pScript,
                                  LinkerConfig& pConfig) {
  if (!MCLDEmulateELF(pScript, pConfig))
    return false;

  // set up bitclass and endian
  pConfig.targets().setEndian(TargetOptions::Little);
  pConfig.targets().setBitClass(64);

  // set up target-dependent constraints of attributes
  pConfig.attribute().constraint().enableWholeArchive();
  pConfig.attribute().constraint().enableAsNeeded();
  pConfig.attribute().constraint().setSharedSystem();

  // set up the predefined attributes
  pConfig.attribute().predefined().unsetWholeArchive();
  pConfig.attribute().predefined().unsetAsNeeded();
  pConfig.attribute().predefined().setDynamic();

  // set up section map
  if (pConfig.options().getScriptList().empty() &&
      pConfig.codeGenType() != LinkerConfig::Object) {
    pScript.sectionMap().insert(".ARM.attributes*", ".ARM.attributes");
  }
  return true;
}

//===----------------------------------------------------------------------===//
// emulateAArch64LD - the help function to emulate AArch64 ld
//===----------------------------------------------------------------------===//
bool emulateAArch64LD(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker has not supported yet");
    return false;
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker has not supported yet");
    return false;
  }

  return MCLDEmulateAArch64ELF(pScript, pConfig);
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// AArch64Emulation
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeAArch64Emulation() {
  // Register the emulation
  mcld::TargetRegistry::RegisterEmulation(mcld::TheAArch64Target,
                                          mcld::emulateAArch64LD);
}
