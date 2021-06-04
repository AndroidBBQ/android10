//===- MergedStringTable.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_MERGEDSTRINGTABLE_H_
#define MCLD_LD_MERGEDSTRINGTABLE_H_

#include "mcld/Support/MemoryRegion.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

namespace mcld {

/** \class MergedStringTable
 *  \brief MergedStringTable represents the mergeable string table. The sections
 *  with flag SHF_MERGED and SHF_STRING are mergeable. Every string in
 *  MergedStringTable is unique.
 */
class MergedStringTable {
 public:
  typedef llvm::StringMap<size_t> StringMapTy;

 public:
  MergedStringTable() {}

  /// insertString - insert a string to the string table
  /// @return false if the string already exists in the map.
  bool insertString(llvm::StringRef pString);

  /// finalizeOffset - finalize the output offset of strings. After this
  /// function been called, any string should not be added to this table
  /// @return the section size
  uint64_t finalizeOffset();

  /// emit - emit the string table
  void emit(MemoryRegion& pRegion);

  /// ----- observers -----///
  /// getOutputOffset - get the output offset of the string. This should be
  /// called after finalizeOffset.
  size_t getOutputOffset(llvm::StringRef pStr);

 private:
  typedef StringMapTy::iterator string_map_iterator;
  typedef StringMapTy::const_iterator const_string_map_iterator;

 private:
  /// m_StringMap - the string pool of this section. It maps the string to the
  /// output offset. The key of this map is the string, and the value is output
  /// offset
  StringMapTy m_StringMap;
};

}  // namespace mcld

#endif  // MCLD_LD_MERGEDSTRINGTABLE_H_

