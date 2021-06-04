//===- MipsEmulation.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Mips.h"
#include "mcld/LinkerScript.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFEmulation.h"

namespace mcld {

static bool MCLDEmulateMipsELF(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (!MCLDEmulateELF(pScript, pConfig))
    return false;

  // set up bitclass and endian
  pConfig.targets().setEndian(TargetOptions::Little);

  llvm::Triple::ArchType arch = pConfig.targets().triple().getArch();
  assert(arch == llvm::Triple::mipsel || arch == llvm::Triple::mips64el);
  unsigned bitclass = arch == llvm::Triple::mipsel ? 32 : 64;
  pConfig.targets().setBitClass(bitclass);

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
// emulateMipsLD - the help function to emulate Mips ld
//===----------------------------------------------------------------------===//
bool emulateMipsLD(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker has not supported yet");
    return false;
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker has not supported yet");
    return false;
  }

  return MCLDEmulateMipsELF(pScript, pConfig);
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// MipsEmulation
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeMipsEmulation() {
  mcld::TargetRegistry::RegisterEmulation(mcld::TheMipselTarget,
                                          mcld::emulateMipsLD);
  mcld::TargetRegistry::RegisterEmulation(mcld::TheMips64elTarget,
                                          mcld::emulateMipsLD);
}
