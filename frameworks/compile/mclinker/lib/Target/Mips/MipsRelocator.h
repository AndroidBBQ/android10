//===- MipsRelocator.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSRELOCATOR_H_
#define TARGET_MIPS_MIPSRELOCATOR_H_

#include "mcld/LD/Relocator.h"
#include "mcld/Support/GCFactory.h"
#include "mcld/Target/KeyEntryMap.h"
#include "MipsLDBackend.h"

#include <llvm/ADT/DenseMapInfo.h>

namespace mcld {

class MipsRelocationInfo;

/** \class MipsRelocator
 *  \brief MipsRelocator creates and destroys the Mips relocations.
 */
class MipsRelocator : public Relocator {
 public:
  enum ReservedEntryType {
    None = 0,        // no reserved entry
    ReserveRel = 1,  // reserve a dynamic relocation entry
    ReserveGot = 2,  // reserve a GOT entry
    ReservePLT = 4   // reserve a PLT entry
  };

  typedef KeyEntryMap<ResolveInfo, PLTEntryBase> SymPLTMap;
  typedef KeyEntryMap<ResolveInfo, Fragment> SymGOTPLTMap;

 public:
  MipsRelocator(MipsGNULDBackend& pParent, const LinkerConfig& pConfig);

  /// scanRelocation - determine the empty entries are needed or not and
  /// create the empty entries if needed.
  /// For Mips, the GOT, GP, and dynamic relocation entries are check to create.
  void scanRelocation(Relocation& pReloc,
                      IRBuilder& pBuilder,
                      Module& pModule,
                      LDSection& pSection,
                      Input& pInput);

  /// initializeScan - do initialization before scan relocations in pInput
  /// @return - return true for initialization success
  bool initializeScan(Input& pInput);

  /// finalizeScan - do finalization after scan relocations in pInput
  /// @return - return true for finalization success
  bool finalizeScan(Input& pInput);

  /// initializeApply - do initialization before apply relocations in pInput
  /// @return - return true for initialization success
  bool initializeApply(Input& pInput);

  /// finalizeApply - do finalization after apply relocations in pInput
  /// @return - return true for finalization success
  bool finalizeApply(Input& pInput);

  Result applyRelocation(Relocation& pReloc);

  /// getDebugStringOffset - get the offset from the relocation target. This is
  /// used to get the debug string offset.
  uint32_t getDebugStringOffset(Relocation& pReloc) const;

  /// applyDebugStringOffset - apply the relocation target to specific offset.
  /// This is used to set the debug string offset.
  void applyDebugStringOffset(Relocation& pReloc, uint32_t pOffset);

  const Input& getApplyingInput() const { return *m_pApplyingInput; }

  MipsGNULDBackend& getTarget() { return m_Target; }

  const MipsGNULDBackend& getTarget() const { return m_Target; }

  /// postponeRelocation - save R_MIPS_LO16 paired relocations
  /// like R_MISP_HI16 and R_MIPS_GOT16 for a future processing.
  void postponeRelocation(Relocation& pReloc);

  /// applyPostponedRelocations - apply all postponed relocations
  /// paired with the R_MIPS_LO16 one.
  void applyPostponedRelocations(MipsRelocationInfo& pLo16Reloc);

  /// isGpDisp - return true if relocation is against _gp_disp symbol.
  bool isGpDisp(const Relocation& pReloc) const;

  /// getGPAddress - return address of _gp symbol.
  Address getGPAddress();

  /// getTPOffset - return TP_OFFSET against the SHF_TLS
  /// section in the processing input.
  Address getTPOffset();

  /// getDTPOffset - return DTP_OFFSET against the SHF_TLS
  /// section in the processing input.
  Address getDTPOffset();

  /// getGP0 - the gp value used to create the relocatable objects
  /// in the processing input.
  Address getGP0();

  /// getLocalGOTEntry - initialize and return a local GOT entry
  /// for this relocation.
  Fragment& getLocalGOTEntry(MipsRelocationInfo& pReloc,
                             Relocation::DWord entryValue);

  /// getGlobalGOTEntry - initialize and return a global GOT entry
  /// for this relocation.
  Fragment& getGlobalGOTEntry(MipsRelocationInfo& pReloc);

  /// getTLSGOTEntry - initialize and return a TLS GOT entry
  /// for this relocation.
  Fragment& getTLSGOTEntry(MipsRelocationInfo& pReloc);

  /// getGOTOffset - return offset of corresponded GOT entry.
  Address getGOTOffset(MipsRelocationInfo& pReloc);

  /// getTLSGOTOffset - return offset of corresponded TLS GOT entry.
  Address getTLSGOTOffset(MipsRelocationInfo& pReloc);

  /// createDynRel - initialize dynamic relocation for the relocation.
  void createDynRel(MipsRelocationInfo& pReloc);

  /// calcAHL - calculate combined addend used
  /// by R_MIPS_HI16 and R_MIPS_GOT16 relocations.
  uint64_t calcAHL(const MipsRelocationInfo& pHiReloc);

  /// isN64ABI - check current ABI
  bool isN64ABI() const;

  const char* getName(Relocation::Type pType) const;

  const SymPLTMap& getSymPLTMap() const { return m_SymPLTMap; }
  SymPLTMap& getSymPLTMap() { return m_SymPLTMap; }

  const SymGOTPLTMap& getSymGOTPLTMap() const { return m_SymGOTPLTMap; }
  SymGOTPLTMap& getSymGOTPLTMap() { return m_SymGOTPLTMap; }

 protected:
  /// setupRelDynEntry - create dynamic relocation entry.
  virtual void setupRel32DynEntry(FragmentRef& pFragRef, ResolveInfo* pSym) = 0;
  /// setupTLSDynEntry - create DTPMOD / DTPREL relocation entries
  virtual void setupTLSDynEntry(Fragment& pFrag, ResolveInfo* pSym,
                                Relocation::Type pType) = 0;

  /// isLocalReloc - handle relocation as a local symbol
  bool isLocalReloc(ResolveInfo& pSym) const;

  /// setupRelDynEntry - create dynamic relocation entry with specified type.
  void setupRelDynEntry(FragmentRef& pFragRef, ResolveInfo* pSym,
                        Relocation::Type pType);

 private:
  typedef llvm::DenseSet<Relocation*> RelocationSet;
  typedef llvm::DenseMap<const ResolveInfo*, RelocationSet> SymRelocSetMap;

 private:
  MipsGNULDBackend& m_Target;
  SymPLTMap m_SymPLTMap;
  SymGOTPLTMap m_SymGOTPLTMap;
  Input* m_pApplyingInput;
  SymRelocSetMap m_PostponedRelocs;
  MipsRelocationInfo* m_CurrentLo16Reloc;

 private:
  void scanLocalReloc(MipsRelocationInfo& pReloc,
                      IRBuilder& pBuilder,
                      const LDSection& pSection);

  void scanGlobalReloc(MipsRelocationInfo& pReloc,
                       IRBuilder& pBuilder,
                       const LDSection& pSection);

  /// isPostponed - relocation applying needs to be postponed.
  bool isPostponed(const Relocation& pReloc) const;

  /// addCopyReloc - add a copy relocation into .rel.dyn for pSym
  /// @param pSym - A resolved copy symbol that defined in BSS section
  void addCopyReloc(ResolveInfo& pSym);

  /// defineSymbolforCopyReloc - allocate a space in BSS section and
  /// and force define the copy of pSym to BSS section
  /// @return the output LDSymbol of the copy symbol
  LDSymbol& defineSymbolforCopyReloc(IRBuilder& pBuilder,
                                     const ResolveInfo& pSym);

  /// isRel - returns true if REL relocation record format is expected
  bool isRel() const;
};

/** \class Mips32Relocator
 *  \brief Mips32Relocator creates and destroys the Mips 32-bit relocations.
 */
class Mips32Relocator : public MipsRelocator {
 public:
  Mips32Relocator(Mips32GNULDBackend& pParent, const LinkerConfig& pConfig);

 private:
  // MipsRelocator
  void setupRel32DynEntry(FragmentRef& pFragRef, ResolveInfo* pSym);
  void setupTLSDynEntry(Fragment& pFrag, ResolveInfo* pSym,
                        Relocation::Type pType);
  Size getSize(Relocation::Type pType) const;
};

/** \class Mips64Relocator
 *  \brief Mips64Relocator creates and destroys the Mips 64-bit relocations.
 */
class Mips64Relocator : public MipsRelocator {
 public:
  Mips64Relocator(Mips64GNULDBackend& pParent, const LinkerConfig& pConfig);

 private:
  // MipsRelocator
  void setupRel32DynEntry(FragmentRef& pFragRef, ResolveInfo* pSym);
  void setupTLSDynEntry(Fragment& pFrag, ResolveInfo* pSym,
                        Relocation::Type pType);
  Size getSize(Relocation::Type pType) const;
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSRELOCATOR_H_
