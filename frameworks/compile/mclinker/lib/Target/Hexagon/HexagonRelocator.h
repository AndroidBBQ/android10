//===-  HexagonRelocator.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_HEXAGON_HEXAGONRELOCATOR_H_
#define TARGET_HEXAGON_HEXAGONRELOCATOR_H_

#include "mcld/LD/Relocator.h"
#include "mcld/Target/GOT.h"
#include "mcld/Target/PLT.h"
#include "mcld/Target/KeyEntryMap.h"
#include "HexagonLDBackend.h"

namespace mcld {

class LinkerConfig;
class ResolveInfo;

/** \class HexagonRelocator
 *  \brief HexagonRelocator creates and destroys the Hexagon relocations.
 *
 */
class HexagonRelocator : public Relocator {
 public:
  typedef KeyEntryMap<ResolveInfo, PLTEntryBase> SymPLTMap;
  typedef KeyEntryMap<ResolveInfo, HexagonGOTEntry> SymGOTMap;
  typedef KeyEntryMap<ResolveInfo, HexagonGOTEntry> SymGOTPLTMap;
  typedef KeyEntryMap<Relocation, Relocation> RelRelMap;

 public:
  /** \enum ReservedEntryType
   *  \brief The reserved entry type of reserved space in ResolveInfo.
   *
   *  This is used for sacnRelocation to record what kinds of entries are
   *  reserved for this resolved symbol. In Hexagon, there are three kinds
   *  of entries, GOT, PLT, and dynamic relocation.
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

  HexagonRelocator(HexagonLDBackend& pParent, const LinkerConfig& pConfig);
  ~HexagonRelocator();

  Result applyRelocation(Relocation& pRelocation);

  /// scanRelocation - determine the empty entries are needed or not and create
  /// the empty entries if needed.
  /// For Hexagon, following entries are check to create:
  /// - GOT entry (for .got and .got.plt sections)
  /// - PLT entry (for .plt section)
  /// - dynamin relocation entries (for .rel.plt and .rel.dyn sections)
  void scanRelocation(Relocation& pReloc,
                      IRBuilder& pBuilder,
                      Module& pModule,
                      LDSection& pSection,
                      Input& pInput);

  // Handle partial linking
  void partialScanRelocation(Relocation& pReloc,
                             Module& pModule);

  HexagonLDBackend& getTarget() { return m_Target; }

  const HexagonLDBackend& getTarget() const { return m_Target; }

  const char* getName(Relocation::Type pType) const;

  Size getSize(Relocation::Type pType) const;

  const SymPLTMap& getSymPLTMap() const { return m_SymPLTMap; }
  SymPLTMap& getSymPLTMap() { return m_SymPLTMap; }

  const SymGOTMap& getSymGOTMap() const { return m_SymGOTMap; }
  SymGOTMap& getSymGOTMap() { return m_SymGOTMap; }

  const SymGOTPLTMap& getSymGOTPLTMap() const { return m_SymGOTPLTMap; }
  SymGOTPLTMap& getSymGOTPLTMap() { return m_SymGOTPLTMap; }

  const RelRelMap& getRelRelMap() const { return m_RelRelMap; }
  RelRelMap& getRelRelMap() { return m_RelRelMap; }

  /// getDebugStringOffset - get the offset from the relocation target. This is
  /// used to get the debug string offset.
  uint32_t getDebugStringOffset(Relocation& pReloc) const { return 0; }

  /// applyDebugStringOffset - apply the relocation target to specific offset.
  /// This is used to set the debug string offset.
  void applyDebugStringOffset(Relocation& pReloc, uint32_t pOffset) {}

 protected:
  /// addCopyReloc - add a copy relocation into .rela.dyn for pSym
  /// @param pSym - A resolved copy symbol that defined in BSS section
  void addCopyReloc(ResolveInfo& pSym, HexagonLDBackend& pTarget);

  /// defineSymbolforCopyReloc - allocate a space in BSS section and
  /// and force define the copy of pSym to BSS section
  /// @return the output LDSymbol of the copy symbol
  LDSymbol& defineSymbolforCopyReloc(IRBuilder& pLinker,
                                     const ResolveInfo& pSym,
                                     HexagonLDBackend& pTarget);

 private:
  virtual void scanLocalReloc(Relocation& pReloc,
                              IRBuilder& pBuilder,
                              Module& pModule,
                              LDSection& pSection);

  virtual void scanGlobalReloc(Relocation& pReloc,
                               IRBuilder& pBuilder,
                               Module& pModule,
                               LDSection& pSection);

  HexagonLDBackend& m_Target;
  SymPLTMap m_SymPLTMap;
  SymGOTMap m_SymGOTMap;
  SymGOTPLTMap m_SymGOTPLTMap;
  RelRelMap m_RelRelMap;
};

}  // namespace mcld

#endif  // TARGET_HEXAGON_HEXAGONRELOCATOR_H_
