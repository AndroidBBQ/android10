//===- AArch64GOT.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64GOT_H_
#define TARGET_AARCH64_AARCH64GOT_H_

#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/GOT.h"

#include <llvm/ADT/DenseMap.h>

#include <vector>

namespace mcld {

class LDSection;

/** \class AArch64GOTEntry
 *  \brief GOT Entry with size of 8 bytes
 */
class AArch64GOTEntry : public GOT::Entry<8> {
 public:
  AArch64GOTEntry(uint64_t pContent, SectionData* pParent)
      : GOT::Entry<8>(pContent, pParent) {}
};

/** \class AArch64GOT
 *  \brief AArch64 Global Offset Table.
 *
 *  AArch64 GOT integrates traditional .got.plt and .got sections into one.
 *  Traditional .got.plt is placed in the front part of GOT (PLTGOT), and
 *  traditional .got is placed in the rear part of GOT (GOT). When -z now and
 *  -z relro are given, the got section layout will be as below. Otherwise,
 *  there will be two seperated sections, .got and .got.plt.
 *
 *  This class may be used as .got (with no GOTPLT entry), .got.plt (with only
 *  GOTPLT entries) or .got (with GOTPLT and normal GOT entries)
 *
 *  AArch64 .got
 *            +--------------+
 *            |    GOT0      |
 *            +--------------+
 *            |    GOTPLT    |
 *            +--------------+
 *            |    GOT       |
 *            +--------------+
 *
 */
class AArch64GOT : public GOT {
 public:
  explicit AArch64GOT(LDSection& pSection);

  ~AArch64GOT();

  /// createGOT0 - create the defualt GOT0 entries. This function called when
  /// it's a .got section (with GOTPLT entries and normal GOT entry) or it's a
  /// .got.plt section
  void createGOT0();

  AArch64GOTEntry* createGOT();
  AArch64GOTEntry* createGOTPLT();

  void finalizeSectionSize();

  uint64_t emit(MemoryRegion& pRegion);

  void applyGOT0(uint64_t pAddress);

  void applyGOTPLT(uint64_t pPLTBase);

  bool hasGOT1() const;

 private:
  typedef std::vector<AArch64GOTEntry*> EntryListType;
  typedef EntryListType::iterator entry_iterator;
  typedef EntryListType::const_iterator const_entry_iterator;

 private:
  AArch64GOTEntry* m_pGOTPLTFront;
  AArch64GOTEntry* m_pGOTFront;

  /// m_GOTPLTEntries - a list of gotplt entries
  EntryListType m_GOTPLT;

  /// m_GOTEntris - a list of got entries
  EntryListType m_GOT;
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64GOT_H_
