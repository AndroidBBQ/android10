//===- Relocation.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_FRAGMENT_RELOCATION_H_
#define MCLD_FRAGMENT_RELOCATION_H_

#include "mcld/Config/Config.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Support/GCFactoryListTraits.h"

#include <llvm/ADT/ilist_node.h>
#include <llvm/Support/DataTypes.h>

namespace mcld {

class ResolveInfo;
class Relocator;
class LinkerConfig;

class Relocation : public llvm::ilist_node<Relocation> {
  friend class RelocationFactory;
  friend class GCFactoryListTraits<Relocation>;
  friend class Chunk<Relocation, MCLD_RELOCATIONS_PER_INPUT>;

 public:
  typedef uint64_t Address;  // FIXME: use SizeTrait<T>::Address instead
  typedef uint64_t DWord;    // FIXME: use SizeTrait<T>::Word instead
  typedef int64_t SWord;     // FIXME: use SizeTrait<T>::SWord instead
  typedef uint32_t Type;
  typedef uint32_t Size;

 private:
  Relocation();

  Relocation(Type pType,
             FragmentRef* pTargetRef,
             Address pAddend,
             DWord pTargetData);

  ~Relocation();

 public:
  /// Initialize - set up the relocation factory
  static void SetUp(const LinkerConfig& pConfig);

  /// Clear - Clean up the relocation factory
  static void Clear();

  /// Create - produce an empty relocation entry
  static Relocation* Create();

  /// Create - produce a relocation entry
  /// @param pType    [in] the type of the relocation entry
  /// @param pFragRef [in] the place to apply the relocation
  /// @param pAddend  [in] the addend of the relocation entry
  static Relocation* Create(Type pType,
                            FragmentRef& pFragRef,
                            Address pAddend = 0);

  /// Destroy - destroy a relocation entry
  static void Destroy(Relocation*& pRelocation);

  /// type - relocation type
  Type type() const { return m_Type; }

  /// symValue - S value - the symbol address
  Address symValue() const;

  /// addend - A value
  Address addend() const { return m_Addend; }

  /// place - P value - address of the place being relocated
  Address place() const;

  /// size - the size of the relocation in bit
  Size size(Relocator& pRelocator) const;

  /// symbol info - binding, type
  const ResolveInfo* symInfo() const { return m_pSymInfo; }
  ResolveInfo* symInfo() { return m_pSymInfo; }

  /// target - the target data to relocate
  const DWord& target() const { return m_TargetData; }
  DWord& target() { return m_TargetData; }

  /// targetRef - the reference of the target data
  const FragmentRef& targetRef() const { return m_TargetAddress; }
  FragmentRef& targetRef() { return m_TargetAddress; }

  void apply(Relocator& pRelocator);

  /// updateAddend - A relocation with a section symbol must update addend
  /// before reading its value.
  void updateAddend();

  /// ----- modifiers ----- ///
  void setType(Type pType);

  void setAddend(Address pAddend);

  void setSymInfo(ResolveInfo* pSym);

 private:
  /// m_Type - the type of the relocation entries
  Type m_Type;

  /// m_TargetData - target data of the place being relocated
  DWord m_TargetData;

  /// m_pSymInfo - resolved symbol info of relocation target symbol
  ResolveInfo* m_pSymInfo;

  /// m_TargetAddress - FragmentRef of the place being relocated
  FragmentRef m_TargetAddress;

  /// m_Addend - the addend
  Address m_Addend;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_RELOCATION_H_
