//===- MipsELFDynamic.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LinkerConfig.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/ELFSegment.h"
#include "mcld/LD/ELFSegmentFactory.h"
#include "mcld/Target/GNULDBackend.h"
#include "MipsELFDynamic.h"
#include "MipsLDBackend.h"

#include <llvm/Support/ELF.h>

namespace mcld {

MipsELFDynamic::MipsELFDynamic(const MipsGNULDBackend& pParent,
                               const LinkerConfig& pConfig)
    : ELFDynamic(pParent, pConfig), m_pParent(pParent), m_pConfig(pConfig) {
}

void MipsELFDynamic::reserveTargetEntries(const ELFFileFormat& pFormat) {
  if (pFormat.hasGOT())
    reserveOne(llvm::ELF::DT_PLTGOT);

  reserveOne(llvm::ELF::DT_MIPS_RLD_VERSION);
  reserveOne(llvm::ELF::DT_MIPS_FLAGS);
  reserveOne(llvm::ELF::DT_MIPS_BASE_ADDRESS);
  reserveOne(llvm::ELF::DT_MIPS_LOCAL_GOTNO);
  reserveOne(llvm::ELF::DT_MIPS_SYMTABNO);
  reserveOne(llvm::ELF::DT_MIPS_GOTSYM);

  if (pFormat.hasGOTPLT())
    reserveOne(llvm::ELF::DT_MIPS_PLTGOT);
}

void MipsELFDynamic::applyTargetEntries(const ELFFileFormat& pFormat) {
  if (pFormat.hasGOT())
    applyOne(llvm::ELF::DT_PLTGOT, pFormat.getGOT().addr());

  applyOne(llvm::ELF::DT_MIPS_RLD_VERSION, 1);
  applyOne(llvm::ELF::DT_MIPS_FLAGS, llvm::ELF::RHF_NOTPOT);
  applyOne(llvm::ELF::DT_MIPS_BASE_ADDRESS, getBaseAddress());
  applyOne(llvm::ELF::DT_MIPS_LOCAL_GOTNO, getLocalGotNum(pFormat));
  applyOne(llvm::ELF::DT_MIPS_SYMTABNO, getSymTabNum(pFormat));
  applyOne(llvm::ELF::DT_MIPS_GOTSYM, getGotSym(pFormat));

  if (pFormat.hasGOTPLT())
    applyOne(llvm::ELF::DT_MIPS_PLTGOT, pFormat.getGOTPLT().addr());
}

size_t MipsELFDynamic::getSymTabNum(const ELFFileFormat& pFormat) const {
  if (!pFormat.hasDynSymTab())
    return 0;

  const LDSection& dynsym = pFormat.getDynSymTab();
  return dynsym.size() / symbolSize();
}

size_t MipsELFDynamic::getGotSym(const ELFFileFormat& pFormat) const {
  if (!pFormat.hasGOT())
    return 0;

  return getSymTabNum(pFormat) - m_pParent.getGOT().getGlobalNum();
}

size_t MipsELFDynamic::getLocalGotNum(const ELFFileFormat& pFormat) const {
  if (!pFormat.hasGOT())
    return 0;

  return m_pParent.getGOT().getLocalNum();
}

uint64_t MipsELFDynamic::getBaseAddress() {
  if (LinkerConfig::Exec != m_pConfig.codeGenType())
    return 0;

  ELFSegmentFactory::const_iterator baseSeg =
      m_pParent.elfSegmentTable().find(llvm::ELF::PT_LOAD, 0x0, 0x0);

  return m_pParent.elfSegmentTable().end() == baseSeg ? 0 : (*baseSeg)->vaddr();
}

}  // namespace mcld
