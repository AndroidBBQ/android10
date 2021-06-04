//===-  AArch64Relocator.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64RELOCATOR_H_
#define TARGET_AARCH64_AARCH64RELOCATOR_H_

#include "mcld/LD/Relocator.h"
#include "mcld/Target/GOT.h"
#include "mcld/Target/KeyEntryMap.h"
#include "AArch64LDBackend.h"

namespace mcld {

/** \class AArch64Relocator
 *  \brief AArch64Relocator creates and destroys the AArch64 relocations.
 *
 */
class AArch64Relocator : public Relocator {
 public:
  typedef KeyEntryMap<ResolveInfo, AArch64GOTEntry> SymGOTMap;
  typedef KeyEntryMap<ResolveInfo, AArch64PLT1> SymPLTMap;
  typedef KeyEntryMap<Relocation, Relocation> RelRelMap;

  /** \enum ReservedEntryType
   *  \brief The reserved entry type of reserved space in ResolveInfo.
   *
   *  This is used for sacnRelocation to record what kinds of entries are
   *  reserved for this resolved symbol In AArch64, there are three kinds of
   *  entries, GOT, PLT, and dynamic reloction.
   *
   *  bit:  3     2     1     0
   *   |    | PLT | GOT | Rel |
   *
   *  value    Name         - Description
   *
   *  0000     None         - no reserved entry
   *  0001     ReserveRel   - reserve an dynamic relocation entry
   *  0010     ReserveGOT   - reserve an GOT entry
   *  0100     ReservePLT   - reserve an PLT entry and the corresponding GOT,
   *
   */
  enum ReservedEntryType {
    None = 0,
    ReserveRel = 1,
    ReserveGOT = 2,
    ReservePLT = 4,
  };

  /** \enum EntryValue
   *  \brief The value of the entries. The symbol value will be decided at after
   *  layout, so we mark the entry during scanRelocation and fill up the actual
   *  value when applying relocations.
   */
  enum EntryValue { Default = 0, SymVal = 1 };

  enum {
    // mcld internal relocation to rewrite an instruction.
    R_AARCH64_REWRITE_INSN = 1,
  };

 public:
  AArch64Relocator(AArch64GNULDBackend& pParent, const LinkerConfig& pConfig);
  ~AArch64Relocator();

  Result applyRelocation(Relocation& pRelocation);

  AArch64GNULDBackend& getTarget() { return m_Target; }

  const AArch64GNULDBackend& getTarget() const { return m_Target; }

  const char* getName(Relocation::Type pType) const;

  Size getSize(Relocation::Type pType) const;

  const SymGOTMap& getSymGOTMap() const { return m_SymGOTMap; }
  SymGOTMap& getSymGOTMap() { return m_SymGOTMap; }

  const SymPLTMap& getSymPLTMap() const { return m_SymPLTMap; }
  SymPLTMap& getSymPLTMap() { return m_SymPLTMap; }

  const SymGOTMap& getSymGOTPLTMap() const { return m_SymGOTPLTMap; }
  SymGOTMap& getSymGOTPLTMap() { return m_SymGOTPLTMap; }

  const RelRelMap& getRelRelMap() const { return m_RelRelMap; }
  RelRelMap& getRelRelMap() { return m_RelRelMap; }

  /// scanRelocation - determine the empty entries are needed or not and create
  /// the empty entries if needed.
  /// For AArch64, following entries are check to create:
  /// - GOT entry (for .got section)
  /// - PLT entry (for .plt section)
  /// - dynamin relocation entries (for .rel.plt and .rel.dyn sections)
  void scanRelocation(Relocation& pReloc,
                      IRBuilder& pBuilder,
                      Module& pModule,
                      LDSection& pSection,
                      Input& pInput);

  /// mayHaveFunctionPointerAccess - check if the given reloc would possibly
  /// access a function pointer.
  virtual bool mayHaveFunctionPointerAccess(const Relocation& pReloc) const;

  /// getDebugStringOffset - get the offset from the relocation target. This is
  /// used to get the debug string offset.
  uint32_t getDebugStringOffset(Relocation& pReloc) const;

  /// applyDebugStringOffset - apply the relocation target to specific offset.
  /// This is used to set the debug string offset.
  void applyDebugStringOffset(Relocation& pReloc, uint32_t pOffset);

 private:
  void scanLocalReloc(Relocation& pReloc, const LDSection& pSection);

  void scanGlobalReloc(Relocation& pReloc,
                       IRBuilder& pBuilder,
                       const LDSection& pSection);

  /// addCopyReloc - add a copy relocation into .rel.dyn for pSym
  /// @param pSym - A resolved copy symbol that defined in BSS section
  void addCopyReloc(ResolveInfo& pSym);

  /// defineSymbolforCopyReloc - allocate a space in BSS section and
  /// and force define the copy of pSym to BSS section
  /// @return the output LDSymbol of the copy symbol
  LDSymbol& defineSymbolforCopyReloc(IRBuilder& pLinker,
                                     const ResolveInfo& pSym);

 private:
  AArch64GNULDBackend& m_Target;
  SymGOTMap m_SymGOTMap;
  SymPLTMap m_SymPLTMap;
  SymGOTMap m_SymGOTPLTMap;
  RelRelMap m_RelRelMap;
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64RELOCATOR_H_
