//===- Fragment.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_FRAGMENT_H_
#define MCLD_FRAGMENT_FRAGMENT_H_

#include "mcld/Support/Compiler.h"

#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/DataTypes.h>

#include <cassert>
#include <cstddef>

namespace mcld {

class SectionData;

/** \class Fragment
 *  \brief Fragment is the minimun linking unit of MCLinker.
 */
class Fragment : public llvm::ilist_node_with_parent<Fragment, SectionData> {
 public:
  enum Type { Alignment, Fillment, Region, Target, Stub, Null };

 public:
  Fragment();

  explicit Fragment(Type pKind, SectionData* pParent = NULL);

  virtual ~Fragment();

  Type getKind() const { return m_Kind; }

  const SectionData* getParent() const { return m_pParent; }
  SectionData* getParent() { return m_pParent; }

  void setParent(SectionData* pValue) { m_pParent = pValue; }

  uint64_t getOffset() const;

  void setOffset(uint64_t pOffset) { m_Offset = pOffset; }

  bool hasOffset() const;

  static bool classof(const Fragment* O) { return true; }

  virtual size_t size() const {
    assert(false && "Can not call abstract Fragment::size()!");
    return 0;
  }

 private:
  Type m_Kind;

  SectionData* m_pParent;

  uint64_t m_Offset;

 private:
  DISALLOW_COPY_AND_ASSIGN(Fragment);
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_FRAGMENT_H_
