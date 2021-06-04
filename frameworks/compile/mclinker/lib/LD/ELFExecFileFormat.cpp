//===- ELFExecFileFormat.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFExecFileFormat.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Object/ObjectBuilder.h"

#include <llvm/Support/ELF.h>

namespace mcld {

void ELFExecFileFormat::initObjectFormat(ObjectBuilder& pBuilder,
                                         unsigned int pBitClass) {
  // FIXME: make sure ELF executable files has these sections.
  f_pDynSymTab = pBuilder.CreateSection(".dynsym",
                                        LDFileFormat::NamePool,
                                        llvm::ELF::SHT_DYNSYM,
                                        llvm::ELF::SHF_ALLOC,
                                        pBitClass / 8);
  f_pDynStrTab = pBuilder.CreateSection(".dynstr",
                                        LDFileFormat::NamePool,
                                        llvm::ELF::SHT_STRTAB,
                                        llvm::ELF::SHF_ALLOC,
                                        0x1);
  f_pInterp = pBuilder.CreateSection(".interp",
                                     LDFileFormat::Note,
                                     llvm::ELF::SHT_PROGBITS,
                                     llvm::ELF::SHF_ALLOC,
                                     0x1);
  f_pHashTab = pBuilder.CreateSection(".hash",
                                      LDFileFormat::NamePool,
                                      llvm::ELF::SHT_HASH,
                                      llvm::ELF::SHF_ALLOC,
                                      pBitClass / 8);
  f_pDynamic =
      pBuilder.CreateSection(".dynamic",
                             LDFileFormat::NamePool,
                             llvm::ELF::SHT_DYNAMIC,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             pBitClass / 8);
  f_pRelaDyn = pBuilder.CreateSection(".rela.dyn",
                                      LDFileFormat::Relocation,
                                      llvm::ELF::SHT_RELA,
                                      llvm::ELF::SHF_ALLOC,
                                      pBitClass / 8);
  f_pRelaPlt = pBuilder.CreateSection(".rela.plt",
                                      LDFileFormat::Relocation,
                                      llvm::ELF::SHT_RELA,
                                      llvm::ELF::SHF_ALLOC,
                                      pBitClass / 8);
  f_pRelDyn = pBuilder.CreateSection(".rel.dyn",
                                     LDFileFormat::Relocation,
                                     llvm::ELF::SHT_REL,
                                     llvm::ELF::SHF_ALLOC,
                                     pBitClass / 8);
  f_pRelPlt = pBuilder.CreateSection(".rel.plt",
                                     LDFileFormat::Relocation,
                                     llvm::ELF::SHT_REL,
                                     llvm::ELF::SHF_ALLOC,
                                     pBitClass / 8);
  f_pGOT = pBuilder.CreateSection(".got",
                                  LDFileFormat::Target,
                                  llvm::ELF::SHT_PROGBITS,
                                  llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                                  pBitClass / 8);
  f_pPLT =
      pBuilder.CreateSection(".plt",
                             LDFileFormat::Target,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR,
                             pBitClass / 8);
  f_pGOTPLT =
      pBuilder.CreateSection(".got.plt",
                             LDFileFormat::Target,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             pBitClass / 8);
  f_pEhFrameHdr = pBuilder.CreateSection(".eh_frame_hdr",
                                         LDFileFormat::EhFrameHdr,
                                         llvm::ELF::SHT_PROGBITS,
                                         llvm::ELF::SHF_ALLOC,
                                         0x4);
  f_pGNUHashTab = pBuilder.CreateSection(".gnu.hash",
                                         LDFileFormat::NamePool,
                                         llvm::ELF::SHT_GNU_HASH,
                                         llvm::ELF::SHF_ALLOC,
                                         pBitClass / 8);
}

}  // namespace mcld
