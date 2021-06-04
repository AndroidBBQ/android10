//===- ELFFileFormat.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/Support/ELF.h>

namespace mcld {

ELFFileFormat::ELFFileFormat()
    : f_pNULLSection(NULL),
      f_pGOT(NULL),
      f_pPLT(NULL),
      f_pRelDyn(NULL),
      f_pRelPlt(NULL),
      f_pRelaDyn(NULL),
      f_pRelaPlt(NULL),
      f_pComment(NULL),
      f_pData1(NULL),
      f_pDebug(NULL),
      f_pDynamic(NULL),
      f_pDynStrTab(NULL),
      f_pDynSymTab(NULL),
      f_pFini(NULL),
      f_pFiniArray(NULL),
      f_pHashTab(NULL),
      f_pInit(NULL),
      f_pInitArray(NULL),
      f_pInterp(NULL),
      f_pLine(NULL),
      f_pNote(NULL),
      f_pPreInitArray(NULL),
      f_pROData1(NULL),
      f_pShStrTab(NULL),
      f_pStrTab(NULL),
      f_pSymTab(NULL),
      f_pTBSS(NULL),
      f_pTData(NULL),
      f_pCtors(NULL),
      f_pDataRelRo(NULL),
      f_pDtors(NULL),
      f_pEhFrame(NULL),
      f_pEhFrameHdr(NULL),
      f_pGCCExceptTable(NULL),
      f_pGNUVersion(NULL),
      f_pGNUVersionD(NULL),
      f_pGNUVersionR(NULL),
      f_pGOTPLT(NULL),
      f_pJCR(NULL),
      f_pNoteABITag(NULL),
      f_pStab(NULL),
      f_pStabStr(NULL),
      f_pStack(NULL),
      f_pStackNote(NULL),
      f_pDataRelRoLocal(NULL),
      f_pGNUHashTab(NULL) {
}

void ELFFileFormat::initStdSections(ObjectBuilder& pBuilder,
                                    unsigned int pBitClass) {
  f_pTextSection =
      pBuilder.CreateSection(".text",
                             LDFileFormat::TEXT,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR,
                             0x1);
  f_pNULLSection =
      pBuilder.CreateSection("", LDFileFormat::Null, llvm::ELF::SHT_NULL, 0x0);
  f_pReadOnlySection = pBuilder.CreateSection(".rodata",
                                              LDFileFormat::TEXT,
                                              llvm::ELF::SHT_PROGBITS,
                                              llvm::ELF::SHF_ALLOC,
                                              0x1);

  f_pBSSSection =
      pBuilder.CreateSection(".bss",
                             LDFileFormat::BSS,
                             llvm::ELF::SHT_NOBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  f_pComment = pBuilder.CreateSection(
      ".comment", LDFileFormat::MetaData, llvm::ELF::SHT_PROGBITS, 0x0, 0x1);
  f_pDataSection =
      pBuilder.CreateSection(".data",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  f_pData1 = pBuilder.CreateSection(".data1",
                                    LDFileFormat::DATA,
                                    llvm::ELF::SHT_PROGBITS,
                                    llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                                    0x1);
  f_pDebug = pBuilder.CreateSection(
      ".debug", LDFileFormat::Debug, llvm::ELF::SHT_PROGBITS, 0x0, 0x1);
  f_pInit =
      pBuilder.CreateSection(".init",
                             LDFileFormat::TEXT,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR,
                             0x1);
  f_pInitArray =
      pBuilder.CreateSection(".init_array",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_INIT_ARRAY,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  f_pFini =
      pBuilder.CreateSection(".fini",
                             LDFileFormat::TEXT,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR,
                             0x1);
  f_pFiniArray =
      pBuilder.CreateSection(".fini_array",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_FINI_ARRAY,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  f_pLine = pBuilder.CreateSection(
      ".line", LDFileFormat::Debug, llvm::ELF::SHT_PROGBITS, 0x0, 0x1);
  f_pPreInitArray =
      pBuilder.CreateSection(".preinit_array",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_PREINIT_ARRAY,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  // the definition of SHF_XXX attributes of rodata in Linux Standard Base
  // conflicts with System V standard. We follow System V standard.
  f_pROData1 = pBuilder.CreateSection(".rodata1",
                                      LDFileFormat::TEXT,
                                      llvm::ELF::SHT_PROGBITS,
                                      llvm::ELF::SHF_ALLOC,
                                      0x1);
  f_pShStrTab = pBuilder.CreateSection(
      ".shstrtab", LDFileFormat::NamePool, llvm::ELF::SHT_STRTAB, 0x0, 0x1);
  // In ELF Spec Book I, p1-16. If symbol table and string table are in
  // loadable segments, set the attribute to SHF_ALLOC bit. But in the
  // real world, this bit always turn off.
  f_pSymTab = pBuilder.CreateSection(".symtab",
                                     LDFileFormat::NamePool,
                                     llvm::ELF::SHT_SYMTAB,
                                     0x0,
                                     pBitClass / 8);

  f_pStrTab = pBuilder.CreateSection(
      ".strtab", LDFileFormat::NamePool, llvm::ELF::SHT_STRTAB, 0x0, 0x1);
  f_pTBSS = pBuilder.CreateSection(
      ".tbss",
      LDFileFormat::BSS,
      llvm::ELF::SHT_NOBITS,
      llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE | llvm::ELF::SHF_TLS,
      0x1);
  f_pTData = pBuilder.CreateSection(
      ".tdata",
      LDFileFormat::DATA,
      llvm::ELF::SHT_PROGBITS,
      llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE | llvm::ELF::SHF_TLS,
      0x1);

  /// @ref 10.3.1.2, ISO/IEC 23360, Part 1:2010(E), p. 24.
  f_pCtors = pBuilder.CreateSection(".ctors",
                                    LDFileFormat::DATA,
                                    llvm::ELF::SHT_PROGBITS,
                                    llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                                    0x1);
  f_pDataRelRo =
      pBuilder.CreateSection(".data.rel.ro",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  f_pDtors = pBuilder.CreateSection(".dtors",
                                    LDFileFormat::DATA,
                                    llvm::ELF::SHT_PROGBITS,
                                    llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                                    0x1);
  f_pEhFrame = pBuilder.CreateSection(".eh_frame",
                                      LDFileFormat::EhFrame,
                                      llvm::ELF::SHT_PROGBITS,
                                      llvm::ELF::SHF_ALLOC,
                                      0x4);
  f_pGCCExceptTable = pBuilder.CreateSection(".gcc_except_table",
                                             LDFileFormat::GCCExceptTable,
                                             llvm::ELF::SHT_PROGBITS,
                                             llvm::ELF::SHF_ALLOC,
                                             0x4);
  f_pGNUVersion = pBuilder.CreateSection(".gnu.version",
                                         LDFileFormat::Version,
                                         llvm::ELF::SHT_GNU_versym,
                                         llvm::ELF::SHF_ALLOC,
                                         0x1);
  f_pGNUVersionD = pBuilder.CreateSection(".gnu.version_d",
                                          LDFileFormat::Version,
                                          llvm::ELF::SHT_GNU_verdef,
                                          llvm::ELF::SHF_ALLOC,
                                          0x1);
  f_pGNUVersionR = pBuilder.CreateSection(".gnu.version_r",
                                          LDFileFormat::Version,
                                          llvm::ELF::SHT_GNU_verneed,
                                          llvm::ELF::SHF_ALLOC,
                                          0x1);
  f_pJCR = pBuilder.CreateSection(".jcr",
                                  LDFileFormat::DATA,
                                  llvm::ELF::SHT_PROGBITS,
                                  llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                                  0x1);
  f_pStab = pBuilder.CreateSection(
      ".stab", LDFileFormat::Debug, llvm::ELF::SHT_PROGBITS, 0x0, 0x1);
  f_pStabStr = pBuilder.CreateSection(
      ".stabstr", LDFileFormat::Debug, llvm::ELF::SHT_STRTAB, 0x0, 0x1);
  f_pStackNote = pBuilder.CreateSection(".note.GNU-stack",
                                        LDFileFormat::StackNote,
                                        llvm::ELF::SHT_PROGBITS,
                                        0x0,
                                        0x1);

  /// @ref GCC convention, see http://www.airs.com/blog/archives/189
  f_pDataRelRoLocal =
      pBuilder.CreateSection(".data.rel.ro.local",
                             LDFileFormat::DATA,
                             llvm::ELF::SHT_PROGBITS,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                             0x1);
  /// Initialize format dependent sections. (sections for executable and shared
  /// objects)
  initObjectFormat(pBuilder, pBitClass);
}

}  // namespace mcld
