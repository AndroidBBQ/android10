//===- LDFileFormat.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFFILEFORMAT_H_
#define MCLD_LD_ELFFILEFORMAT_H_
#include "mcld/LD/LDFileFormat.h"
#include "mcld/LD/LDSection.h"

namespace mcld {

class ObjectBuilder;

/** \class ELFFileFormat
 *  \brief ELFFileFormat describes the common file formats in ELF.
 *  LDFileFormats control the formats of the output file.
 *
 *  @ref "Object Files," Ch. 4, in System V Application Binary Interface,
 *  Fourth Edition.
 *
 *  @ref "Object Format," Ch. 10, in ISO/IEC 23360 Part 1:2010(E), Linux
 *  Standard Base Core Specification 4.1.
 */
class ELFFileFormat : public LDFileFormat {
 private:
  /// initObjectFormat - initialize sections that are dependent on object
  /// formats. (executable, shared objects or relocatable objects).
  virtual void initObjectFormat(ObjectBuilder& pBuilder,
                                unsigned int pBitClass) = 0;

 public:
  ELFFileFormat();

  void initStdSections(ObjectBuilder& pBuilder, unsigned int pBitClass);

  // -----  capacity  ----- //
  /// @ref Special Sections, Ch. 4.17, System V ABI, 4th edition.
  bool hasNULLSection() const {
    return (f_pNULLSection != NULL) && (f_pNULLSection->size() != 0);
  }

  bool hasGOT() const { return (f_pGOT != NULL) && (f_pGOT->size() != 0); }

  bool hasPLT() const { return (f_pPLT != NULL) && (f_pPLT->size() != 0); }

  bool hasRelDyn() const {
    return (f_pRelDyn != NULL) && (f_pRelDyn->size() != 0);
  }

  bool hasRelPlt() const {
    return (f_pRelPlt != NULL) && (f_pRelPlt->size() != 0);
  }

  bool hasRelaDyn() const {
    return (f_pRelaDyn != NULL) && (f_pRelaDyn->size() != 0);
  }

  bool hasRelaPlt() const {
    return (f_pRelaPlt != NULL) && (f_pRelaPlt->size() != 0);
  }

  /// @ref 10.3.1.1, ISO/IEC 23360, Part 1:2010(E), p. 21.
  bool hasComment() const {
    return (f_pComment != NULL) && (f_pComment->size() != 0);
  }

  bool hasData1() const {
    return (f_pData1 != NULL) && (f_pData1->size() != 0);
  }

  bool hasDebug() const {
    return (f_pDebug != NULL) && (f_pDebug->size() != 0);
  }

  bool hasDynamic() const {
    return (f_pDynamic != NULL) && (f_pDynamic->size() != 0);
  }

  bool hasDynStrTab() const {
    return (f_pDynStrTab != NULL) && (f_pDynStrTab->size() != 0);
  }

  bool hasDynSymTab() const {
    return (f_pDynSymTab != NULL) && (f_pDynSymTab->size() != 0);
  }

  bool hasFini() const { return (f_pFini != NULL) && (f_pFini->size() != 0); }

  bool hasFiniArray() const {
    return (f_pFiniArray != NULL) && (f_pFiniArray->size() != 0);
  }

  bool hasHashTab() const {
    return (f_pHashTab != NULL) && (f_pHashTab->size() != 0);
  }

  bool hasInit() const { return (f_pInit != NULL) && (f_pInit->size() != 0); }

  bool hasInitArray() const {
    return (f_pInitArray != NULL) && (f_pInitArray->size() != 0);
  }

  bool hasInterp() const {
    return (f_pInterp != NULL) && (f_pInterp->size() != 0);
  }

  bool hasLine() const { return (f_pLine != NULL) && (f_pLine->size() != 0); }

  bool hasNote() const { return (f_pNote != NULL) && (f_pNote->size() != 0); }

  bool hasPreInitArray() const {
    return (f_pPreInitArray != NULL) && (f_pPreInitArray->size() != 0);
  }

  bool hasROData1() const {
    return (f_pROData1 != NULL) && (f_pROData1->size() != 0);
  }

  bool hasShStrTab() const {
    return (f_pShStrTab != NULL) && (f_pShStrTab->size() != 0);
  }

  bool hasStrTab() const {
    return (f_pStrTab != NULL) && (f_pStrTab->size() != 0);
  }

  bool hasSymTab() const {
    return (f_pSymTab != NULL) && (f_pSymTab->size() != 0);
  }

  bool hasTBSS() const { return (f_pTBSS != NULL) && (f_pTBSS->size() != 0); }

  bool hasTData() const {
    return (f_pTData != NULL) && (f_pTData->size() != 0);
  }

  /// @ref 10.3.1.2, ISO/IEC 23360, Part 1:2010(E), p. 24.
  bool hasCtors() const {
    return (f_pCtors != NULL) && (f_pCtors->size() != 0);
  }

  bool hasDataRelRo() const {
    return (f_pDataRelRo != NULL) && (f_pDataRelRo->size() != 0);
  }

  bool hasDtors() const {
    return (f_pDtors != NULL) && (f_pDtors->size() != 0);
  }

  bool hasEhFrame() const {
    return (f_pEhFrame != NULL) && (f_pEhFrame->size() != 0);
  }

  bool hasEhFrameHdr() const {
    return (f_pEhFrameHdr != NULL) && (f_pEhFrameHdr->size() != 0);
  }

  bool hasGCCExceptTable() const {
    return (f_pGCCExceptTable != NULL) && (f_pGCCExceptTable->size() != 0);
  }

  bool hasGNUVersion() const {
    return (f_pGNUVersion != NULL) && (f_pGNUVersion->size() != 0);
  }

  bool hasGNUVersionD() const {
    return (f_pGNUVersionD != NULL) && (f_pGNUVersionD->size() != 0);
  }

  bool hasGNUVersionR() const {
    return (f_pGNUVersionR != NULL) && (f_pGNUVersionR->size() != 0);
  }

  bool hasGOTPLT() const {
    return (f_pGOTPLT != NULL) && (f_pGOTPLT->size() != 0);
  }

  bool hasJCR() const { return (f_pJCR != NULL) && (f_pJCR->size() != 0); }

  bool hasNoteABITag() const {
    return (f_pNoteABITag != NULL) && (f_pNoteABITag->size() != 0);
  }

  bool hasStab() const { return (f_pStab != NULL) && (f_pStab->size() != 0); }

  bool hasStabStr() const {
    return (f_pStabStr != NULL) && (f_pStabStr->size() != 0);
  }

  bool hasStack() const {
    return (f_pStack != NULL) && (f_pStack->size() != 0);
  }

  bool hasStackNote() const { return (f_pStackNote != NULL); }

  bool hasDataRelRoLocal() const {
    return (f_pDataRelRoLocal != NULL) && (f_pDataRelRoLocal->size() != 0);
  }

  bool hasGNUHashTab() const {
    return (f_pGNUHashTab != NULL) && (f_pGNUHashTab->size() != 0);
  }

  // -----  access functions  ----- //
  /// @ref Special Sections, Ch. 4.17, System V ABI, 4th edition.
  LDSection& getNULLSection() {
    assert(f_pNULLSection != NULL);
    return *f_pNULLSection;
  }

  const LDSection& getNULLSection() const {
    assert(f_pNULLSection != NULL);
    return *f_pNULLSection;
  }

  LDSection& getGOT() {
    assert(f_pGOT != NULL);
    return *f_pGOT;
  }

  const LDSection& getGOT() const {
    assert(f_pGOT != NULL);
    return *f_pGOT;
  }

  LDSection& getPLT() {
    assert(f_pPLT != NULL);
    return *f_pPLT;
  }

  const LDSection& getPLT() const {
    assert(f_pPLT != NULL);
    return *f_pPLT;
  }

  LDSection& getRelDyn() {
    assert(f_pRelDyn != NULL);
    return *f_pRelDyn;
  }

  const LDSection& getRelDyn() const {
    assert(f_pRelDyn != NULL);
    return *f_pRelDyn;
  }

  LDSection& getRelPlt() {
    assert(f_pRelPlt != NULL);
    return *f_pRelPlt;
  }

  const LDSection& getRelPlt() const {
    assert(f_pRelPlt != NULL);
    return *f_pRelPlt;
  }

  LDSection& getRelaDyn() {
    assert(f_pRelaDyn != NULL);
    return *f_pRelaDyn;
  }

  const LDSection& getRelaDyn() const {
    assert(f_pRelaDyn != NULL);
    return *f_pRelaDyn;
  }

  LDSection& getRelaPlt() {
    assert(f_pRelaPlt != NULL);
    return *f_pRelaPlt;
  }

  const LDSection& getRelaPlt() const {
    assert(f_pRelaPlt != NULL);
    return *f_pRelaPlt;
  }

  LDSection& getComment() {
    assert(f_pComment != NULL);
    return *f_pComment;
  }

  /// @ref 10.3.1.1, ISO/IEC 23360, Part 1:2010(E), p. 21.
  const LDSection& getComment() const {
    assert(f_pComment != NULL);
    return *f_pComment;
  }

  LDSection& getData1() {
    assert(f_pData1 != NULL);
    return *f_pData1;
  }

  const LDSection& getData1() const {
    assert(f_pData1 != NULL);
    return *f_pData1;
  }

  LDSection& getDebug() {
    assert(f_pDebug != NULL);
    return *f_pDebug;
  }

  const LDSection& getDebug() const {
    assert(f_pDebug != NULL);
    return *f_pDebug;
  }

  LDSection& getDynamic() {
    assert(f_pDynamic != NULL);
    return *f_pDynamic;
  }

  const LDSection& getDynamic() const {
    assert(f_pDynamic != NULL);
    return *f_pDynamic;
  }

  LDSection& getDynStrTab() {
    assert(f_pDynStrTab != NULL);
    return *f_pDynStrTab;
  }

  const LDSection& getDynStrTab() const {
    assert(f_pDynStrTab != NULL);
    return *f_pDynStrTab;
  }

  LDSection& getDynSymTab() {
    assert(f_pDynSymTab != NULL);
    return *f_pDynSymTab;
  }

  const LDSection& getDynSymTab() const {
    assert(f_pDynSymTab != NULL);
    return *f_pDynSymTab;
  }

  LDSection& getFini() {
    assert(f_pFini != NULL);
    return *f_pFini;
  }

  const LDSection& getFini() const {
    assert(f_pFini != NULL);
    return *f_pFini;
  }

  LDSection& getFiniArray() {
    assert(f_pFiniArray != NULL);
    return *f_pFiniArray;
  }

  const LDSection& getFiniArray() const {
    assert(f_pFiniArray != NULL);
    return *f_pFiniArray;
  }

  LDSection& getHashTab() {
    assert(f_pHashTab != NULL);
    return *f_pHashTab;
  }

  const LDSection& getHashTab() const {
    assert(f_pHashTab != NULL);
    return *f_pHashTab;
  }

  LDSection& getInit() {
    assert(f_pInit != NULL);
    return *f_pInit;
  }

  const LDSection& getInit() const {
    assert(f_pInit != NULL);
    return *f_pInit;
  }

  LDSection& getInitArray() {
    assert(f_pInitArray != NULL);
    return *f_pInitArray;
  }

  const LDSection& getInitArray() const {
    assert(f_pInitArray != NULL);
    return *f_pInitArray;
  }

  LDSection& getInterp() {
    assert(f_pInterp != NULL);
    return *f_pInterp;
  }

  const LDSection& getInterp() const {
    assert(f_pInterp != NULL);
    return *f_pInterp;
  }

  LDSection& getLine() {
    assert(f_pLine != NULL);
    return *f_pLine;
  }

  const LDSection& getLine() const {
    assert(f_pLine != NULL);
    return *f_pLine;
  }

  LDSection& getNote() {
    assert(f_pNote != NULL);
    return *f_pNote;
  }

  const LDSection& getNote() const {
    assert(f_pNote != NULL);
    return *f_pNote;
  }

  LDSection& getPreInitArray() {
    assert(f_pPreInitArray != NULL);
    return *f_pPreInitArray;
  }

  const LDSection& getPreInitArray() const {
    assert(f_pPreInitArray != NULL);
    return *f_pPreInitArray;
  }

  LDSection& getROData1() {
    assert(f_pROData1 != NULL);
    return *f_pROData1;
  }

  const LDSection& getROData1() const {
    assert(f_pROData1 != NULL);
    return *f_pROData1;
  }

  LDSection& getShStrTab() {
    assert(f_pShStrTab != NULL);
    return *f_pShStrTab;
  }

  const LDSection& getShStrTab() const {
    assert(f_pShStrTab != NULL);
    return *f_pShStrTab;
  }

  LDSection& getStrTab() {
    assert(f_pStrTab != NULL);
    return *f_pStrTab;
  }

  const LDSection& getStrTab() const {
    assert(f_pStrTab != NULL);
    return *f_pStrTab;
  }

  LDSection& getSymTab() {
    assert(f_pSymTab != NULL);
    return *f_pSymTab;
  }

  const LDSection& getSymTab() const {
    assert(f_pSymTab != NULL);
    return *f_pSymTab;
  }

  LDSection& getTBSS() {
    assert(f_pTBSS != NULL);
    return *f_pTBSS;
  }

  const LDSection& getTBSS() const {
    assert(f_pTBSS != NULL);
    return *f_pTBSS;
  }

  LDSection& getTData() {
    assert(f_pTData != NULL);
    return *f_pTData;
  }

  const LDSection& getTData() const {
    assert(f_pTData != NULL);
    return *f_pTData;
  }

  /// @ref 10.3.1.2, ISO/IEC 23360, Part 1:2010(E), p. 24.
  LDSection& getCtors() {
    assert(f_pCtors != NULL);
    return *f_pCtors;
  }

  const LDSection& getCtors() const {
    assert(f_pCtors != NULL);
    return *f_pCtors;
  }

  LDSection& getDataRelRo() {
    assert(f_pDataRelRo != NULL);
    return *f_pDataRelRo;
  }

  const LDSection& getDataRelRo() const {
    assert(f_pDataRelRo != NULL);
    return *f_pDataRelRo;
  }

  LDSection& getDtors() {
    assert(f_pDtors != NULL);
    return *f_pDtors;
  }

  const LDSection& getDtors() const {
    assert(f_pDtors != NULL);
    return *f_pDtors;
  }

  LDSection& getEhFrame() {
    assert(f_pEhFrame != NULL);
    return *f_pEhFrame;
  }

  const LDSection& getEhFrame() const {
    assert(f_pEhFrame != NULL);
    return *f_pEhFrame;
  }

  LDSection& getEhFrameHdr() {
    assert(f_pEhFrameHdr != NULL);
    return *f_pEhFrameHdr;
  }

  const LDSection& getEhFrameHdr() const {
    assert(f_pEhFrameHdr != NULL);
    return *f_pEhFrameHdr;
  }

  LDSection& getGCCExceptTable() {
    assert(f_pGCCExceptTable != NULL);
    return *f_pGCCExceptTable;
  }

  const LDSection& getGCCExceptTable() const {
    assert(f_pGCCExceptTable != NULL);
    return *f_pGCCExceptTable;
  }

  LDSection& getGNUVersion() {
    assert(f_pGNUVersion != NULL);
    return *f_pGNUVersion;
  }

  const LDSection& getGNUVersion() const {
    assert(f_pGNUVersion != NULL);
    return *f_pGNUVersion;
  }

  LDSection& getGNUVersionD() {
    assert(f_pGNUVersionD != NULL);
    return *f_pGNUVersionD;
  }

  const LDSection& getGNUVersionD() const {
    assert(f_pGNUVersionD != NULL);
    return *f_pGNUVersionD;
  }

  LDSection& getGNUVersionR() {
    assert(f_pGNUVersionR != NULL);
    return *f_pGNUVersionR;
  }

  const LDSection& getGNUVersionR() const {
    assert(f_pGNUVersionR != NULL);
    return *f_pGNUVersionR;
  }

  LDSection& getGOTPLT() {
    assert(f_pGOTPLT != NULL);
    return *f_pGOTPLT;
  }

  const LDSection& getGOTPLT() const {
    assert(f_pGOTPLT != NULL);
    return *f_pGOTPLT;
  }

  LDSection& getJCR() {
    assert(f_pJCR != NULL);
    return *f_pJCR;
  }

  const LDSection& getJCR() const {
    assert(f_pJCR != NULL);
    return *f_pJCR;
  }

  LDSection& getNoteABITag() {
    assert(f_pNoteABITag != NULL);
    return *f_pNoteABITag;
  }

  const LDSection& getNoteABITag() const {
    assert(f_pNoteABITag != NULL);
    return *f_pNoteABITag;
  }

  LDSection& getStab() {
    assert(f_pStab != NULL);
    return *f_pStab;
  }

  const LDSection& getStab() const {
    assert(f_pStab != NULL);
    return *f_pStab;
  }

  LDSection& getStabStr() {
    assert(f_pStabStr != NULL);
    return *f_pStabStr;
  }

  const LDSection& getStabStr() const {
    assert(f_pStabStr != NULL);
    return *f_pStabStr;
  }

  LDSection& getStack() {
    assert(f_pStack != NULL);
    return *f_pStack;
  }

  const LDSection& getStack() const {
    assert(f_pStack != NULL);
    return *f_pStack;
  }

  LDSection& getStackNote() {
    assert(f_pStackNote != NULL);
    return *f_pStackNote;
  }

  const LDSection& getStackNote() const {
    assert(f_pStackNote != NULL);
    return *f_pStackNote;
  }

  LDSection& getDataRelRoLocal() {
    assert(f_pDataRelRoLocal != NULL);
    return *f_pDataRelRoLocal;
  }

  const LDSection& getDataRelRoLocal() const {
    assert(f_pDataRelRoLocal != NULL);
    return *f_pDataRelRoLocal;
  }

  LDSection& getGNUHashTab() {
    assert(f_pGNUHashTab != NULL);
    return *f_pGNUHashTab;
  }

  const LDSection& getGNUHashTab() const {
    assert(f_pGNUHashTab != NULL);
    return *f_pGNUHashTab;
  }

 protected:
  //         variable name         :  ELF
  /// @ref Special Sections, Ch. 4.17, System V ABI, 4th edition.
  LDSection* f_pNULLSection;
  LDSection* f_pGOT;      // .got
  LDSection* f_pPLT;      // .plt
  LDSection* f_pRelDyn;   // .rel.dyn
  LDSection* f_pRelPlt;   // .rel.plt
  LDSection* f_pRelaDyn;  // .rela.dyn
  LDSection* f_pRelaPlt;  // .rela.plt

  /// @ref 10.3.1.1, ISO/IEC 23360, Part 1:2010(E), p. 21.
  LDSection* f_pComment;       // .comment
  LDSection* f_pData1;         // .data1
  LDSection* f_pDebug;         // .debug
  LDSection* f_pDynamic;       // .dynamic
  LDSection* f_pDynStrTab;     // .dynstr
  LDSection* f_pDynSymTab;     // .dynsym
  LDSection* f_pFini;          // .fini
  LDSection* f_pFiniArray;     // .fini_array
  LDSection* f_pHashTab;       // .hash
  LDSection* f_pInit;          // .init
  LDSection* f_pInitArray;     // .init_array
  LDSection* f_pInterp;        // .interp
  LDSection* f_pLine;          // .line
  LDSection* f_pNote;          // .note
  LDSection* f_pPreInitArray;  // .preinit_array
  LDSection* f_pROData1;       // .rodata1
  LDSection* f_pShStrTab;      // .shstrtab
  LDSection* f_pStrTab;        // .strtab
  LDSection* f_pSymTab;        // .symtab
  LDSection* f_pTBSS;          // .tbss
  LDSection* f_pTData;         // .tdata

  /// @ref 10.3.1.2, ISO/IEC 23360, Part 1:2010(E), p. 24.
  LDSection* f_pCtors;           // .ctors
  LDSection* f_pDataRelRo;       // .data.rel.ro
  LDSection* f_pDtors;           // .dtors
  LDSection* f_pEhFrame;         // .eh_frame
  LDSection* f_pEhFrameHdr;      // .eh_frame_hdr
  LDSection* f_pGCCExceptTable;  // .gcc_except_table
  LDSection* f_pGNUVersion;      // .gnu.version
  LDSection* f_pGNUVersionD;     // .gnu.version_d
  LDSection* f_pGNUVersionR;     // .gnu.version_r
  LDSection* f_pGOTPLT;          // .got.plt
  LDSection* f_pJCR;             // .jcr
  LDSection* f_pNoteABITag;      // .note.ABI-tag
  LDSection* f_pStab;            // .stab
  LDSection* f_pStabStr;         // .stabstr

  /// practical
  LDSection* f_pStack;           // .stack
  LDSection* f_pStackNote;       // .note.GNU-stack
  LDSection* f_pDataRelRoLocal;  // .data.rel.ro.local
  LDSection* f_pGNUHashTab;      // .gnu.hash
};

}  // namespace mcld

#endif  // MCLD_LD_ELFFILEFORMAT_H_
