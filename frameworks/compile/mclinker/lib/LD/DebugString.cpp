//===- DebugString.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DebugString.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Fragment/Fragment.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/Target/TargetLDBackend.h"
#include "mcld/LD/Relocator.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ManagedStatic.h>

namespace mcld {

// DebugString represents the output .debug_str section, which is at most on
// in each linking
static llvm::ManagedStatic<DebugString> g_DebugString;

static inline size_t string_length(const char* pStr) {
  const char* p = pStr;
  size_t len = 0;
  for (; *p != 0; ++p)
    ++len;
  return len;
}

//==========================
// DebugString
void DebugString::merge(LDSection& pSection) {
  // get the fragment contents
  llvm::StringRef strings;
  SectionData::iterator it, end = pSection.getSectionData()->end();
  for (it = pSection.getSectionData()->begin(); it != end; ++it) {
    if ((*it).getKind() == Fragment::Region) {
      RegionFragment* frag = llvm::cast<RegionFragment>(&(*it));
      strings = frag->getRegion().data();
    }
  }

  // get the debug strings and add them into merged string table
  const char* str = strings.data();
  const char* str_end = str + pSection.size();
  while (str < str_end) {
    size_t len = string_length(str);
    m_StringTable.insertString(llvm::StringRef(str, len));
    str = str + len + 1;
  }
}

size_t DebugString::computeOffsetSize() {
  size_t size = m_StringTable.finalizeOffset();
  m_pSection->setSize(size);
  return size;
}

void DebugString::applyOffset(Relocation& pReloc, TargetLDBackend& pBackend) {
  // get the refered string
  ResolveInfo* info = pReloc.symInfo();
  // the symbol should point to the first region fragment in the debug
  // string section, get the input .debut_str region
  llvm::StringRef d_str;
  if (info->outSymbol()->fragRef()->frag()->getKind() == Fragment::Region) {
    RegionFragment* frag =
        llvm::cast<RegionFragment>(info->outSymbol()->fragRef()->frag());
    d_str = frag->getRegion();
  }
  uint32_t offset = pBackend.getRelocator()->getDebugStringOffset(pReloc);
  const char* str = d_str.data() + offset;

  // apply the relocation
  pBackend.getRelocator()->applyDebugStringOffset(pReloc,
      m_StringTable.getOutputOffset(llvm::StringRef(str, string_length(str))));
}

void DebugString::emit(MemoryRegion& pRegion) {
  return m_StringTable.emit(pRegion);
}

DebugString* DebugString::Create(LDSection& pSection) {
  g_DebugString->setOutputSection(pSection);
  return &(*g_DebugString);
}

}  // namespace mcld
