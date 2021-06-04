//===- X86Emulation.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFEmulation.h"

namespace mcld {

static bool MCLDEmulateX86ELF(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (!MCLDEmulateELF(pScript, pConfig))
    return false;

  // set up bitclass and endian
  pConfig.targets().setEndian(TargetOptions::Little);
  unsigned int bitclass;
  llvm::Triple::ArchType arch = pConfig.targets().triple().getArch();
  assert(arch == llvm::Triple::x86 || arch == llvm::Triple::x86_64);
  if (arch == llvm::Triple::x86 ||
      pConfig.targets().triple().getEnvironment() == llvm::Triple::GNUX32) {
    bitclass = 32;
  } else {
    bitclass = 64;
  }
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
// emulateX86LD - the help function to emulate X86 ld
//===----------------------------------------------------------------------===//
bool emulateX86LD(LinkerScript& pScript, LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker has not supported yet");
    return false;
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker has not supported yet");
    return false;
  }

  return MCLDEmulateX86ELF(pScript, pConfig);
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// X86Emulation
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeX86Emulation() {
  // Register the emulation
  mcld::TargetRegistry::RegisterEmulation(mcld::TheX86_32Target,
                                          mcld::emulateX86LD);
  mcld::TargetRegistry::RegisterEmulation(mcld::TheX86_64Target,
                                          mcld::emulateX86LD);
}
