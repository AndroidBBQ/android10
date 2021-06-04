//===- DebugString.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DEBUGSTRING_H_
#define MCLD_LD_DEBUGSTRING_H_

#include "mcld/LD/MergedStringTable.h"

#include <vector>

namespace mcld {

class LDSection;
class Relocation;
class TargetLDBackend;

/** \class DebugString
 *  \brief DebugString represents the output debug section .debug_str
 */
class DebugString {
 public:
  DebugString()
      : m_pSection(NULL) {}

  static DebugString* Create(LDSection& pSection);

  /// merge - process the strings in the given input .debug_str section and add
  /// those strings into merged string map
  void merge(LDSection& pSection);

  /// computeOffsetSize - set up the output offset of each strings and the
  /// section size
  /// @return string table size
  size_t computeOffsetSize();

  /// applyOffset - apply the relocation which refer to debug string. This
  /// should be called after finalizeStringsOffset()
  void applyOffset(Relocation& pReloc, TargetLDBackend& pBackend);

  /// emit - emit the section .debug_str
  void emit(MemoryRegion& pRegion);

  void setOutputSection(LDSection& pSection)
  { m_pSection = &pSection; }

  /// ---- observers ----- ///
  const LDSection* getSection() const { return m_pSection; }
  LDSection*       getSection()       { return m_pSection; }

 private:
  /// m_Section - the output LDSection of this .debug_str
  LDSection* m_pSection;

  MergedStringTable m_StringTable;
};

}  // namespace mcld

#endif  // MCLD_LD_DEBUGSTRING_H_

