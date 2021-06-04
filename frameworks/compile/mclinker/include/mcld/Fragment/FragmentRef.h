//===- FragmentRef.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_FRAGMENTREF_H_
#define MCLD_FRAGMENT_FRAGMENTREF_H_

#include "mcld/ADT/SizeTraits.h"
#include "mcld/ADT/TypeTraits.h"
#include "mcld/Config/Config.h"
#include "mcld/Support/Allocators.h"

namespace mcld {

class Fragment;
class LDSection;
class Layout;

/** \class FragmentRef
 *  \brief FragmentRef is a reference of a Fragment's contetnt.
 *
 */
class FragmentRef {
 public:
  typedef uint64_t Offset;  // FIXME: use SizeTraits<T>::Offset
  typedef NonConstTraits<unsigned char>::pointer Address;
  typedef ConstTraits<unsigned char>::pointer ConstAddress;

 public:
  /// Create - create a fragment reference for a given fragment.
  ///
  /// @param pFrag - the given fragment
  /// @param pOffset - the offset, can be larger than the fragment, but can not
  ///                  be larger than the section size.
  /// @return if the offset is legal, return the fragment reference. Otherwise,
  /// return NULL.
  static FragmentRef* Create(Fragment& pFrag, uint64_t pOffset);

  static FragmentRef* Create(LDSection& pSection, uint64_t pOffset);

  /// Clear - clear all generated FragmentRef in the system.
  static void Clear();

  static FragmentRef* Null();

  // -----  modifiers  ----- //
  FragmentRef& assign(const FragmentRef& pCopy);

  FragmentRef& assign(Fragment& pFrag, Offset pOffset = 0);

  /// memcpy - copy memory
  /// copy memory from the fragment to the pDesc.
  /// @pDest - the destination address
  /// @pNBytes - copies pNBytes from the fragment[offset()+pOffset]
  /// @pOffset - additional offset.
  ///            the start address offset from fragment[offset()]
  void memcpy(void* pDest, size_t pNBytes, Offset pOffset = 0) const;

  // -----  observers  ----- //
  bool isNull() const { return (this == Null()); }

  Fragment* frag() { return m_pFragment; }

  const Fragment* frag() const { return m_pFragment; }

  Offset offset() const { return m_Offset; }

  Offset getOutputOffset() const;

 private:
  friend FragmentRef& NullFragmentRef();
  friend class Chunk<FragmentRef, MCLD_SECTIONS_PER_INPUT>;
  friend class Relocation;

  FragmentRef();

  explicit FragmentRef(Fragment& pFrag, Offset pOffset = 0);

 private:
  Fragment* m_pFragment;

  Offset m_Offset;

  static FragmentRef g_NullFragmentRef;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_FRAGMENTREF_H_
