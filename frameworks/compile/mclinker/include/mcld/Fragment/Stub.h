//===- Stub.h -------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCLD_FRAGMENT_STUB_H_
#define MCLD_FRAGMENT_STUB_H_

#include "mcld/Fragment/Fragment.h"
#include "mcld/Fragment/Relocation.h"

#include <llvm/Support/DataTypes.h>

#include <string>
#include <vector>

namespace mcld {

class BranchIsland;
class IRBuilder;
class Relocation;
class ResolveInfo;

class Stub : public Fragment {
 public:
  typedef Relocation::DWord DWord;
  typedef Relocation::SWord SWord;
  typedef Relocation::Type Type;

  class Fixup {
   public:
    Fixup(DWord pOffset, SWord pAddend, Type pType)
        : m_Offset(pOffset), m_Addend(pAddend), m_Type(pType) {}

    ~Fixup() {}

    DWord offset() const { return m_Offset; }

    SWord addend() const { return m_Addend; }

    Type type() const { return m_Type; }

   private:
    DWord m_Offset;
    SWord m_Addend;
    Type m_Type;
  };

 public:
  typedef std::vector<Fixup*> FixupListType;
  typedef FixupListType::iterator fixup_iterator;
  typedef FixupListType::const_iterator const_fixup_iterator;

 public:
  Stub();

  virtual ~Stub();

  /// clone - clone function for stub factory to create the corresponding stub
  Stub* clone() { return doClone(); }

  /// isMyDuty - return true when the pReloc is problematic and the stub is able
  /// to fix it!
  virtual bool isMyDuty(const Relocation& pReloc,
                        uint64_t pSource,
                        uint64_t pTargetSymValue) const {
    return false;
  }

  virtual bool isMyDuty(const FragmentRef& pFragRef) const {
    return false;
  }

  /// name - name of this stub
  virtual const std::string& name() const = 0;

  /// getContent - content of the stub
  virtual const uint8_t* getContent() const = 0;

  /// size - size of the stub
  virtual size_t size() const = 0;

  /// alignment - alignment of the stub
  virtual size_t alignment() const = 0;

  /// symInfo - ResolveInfo of this Stub
  ResolveInfo* symInfo() { return m_pSymInfo; }

  const ResolveInfo* symInfo() const { return m_pSymInfo; }

  /// symValue - initial value for stub's symbol
  virtual uint64_t initSymValue() const { return 0x0; }

  ///  -----  Fixup  -----  ///
  fixup_iterator fixup_begin() { return m_FixupList.begin(); }

  const_fixup_iterator fixup_begin() const { return m_FixupList.begin(); }

  fixup_iterator fixup_end() { return m_FixupList.end(); }

  const_fixup_iterator fixup_end() const { return m_FixupList.end(); }

  size_t fixup_size() const { return m_FixupList.size(); }

  virtual void applyFixup(Relocation& pSrcReloc,
                          IRBuilder& pBuilder,
                          BranchIsland& pIsland);

  virtual void applyFixup(FragmentRef& pSrcFragRef,
                          IRBuilder& pBuilder,
                          BranchIsland& pIsland);

  /// ----- modifiers ----- ///
  void setSymInfo(ResolveInfo* pSymInfo);

  // Stub is a kind of Fragment with type of Stub
  static bool classof(const Fragment* F) {
    return F->getKind() == Fragment::Stub;
  }

  static bool classof(const Stub*) { return true; }

 protected:
  /// addFixup - add a fixup for this stub to build a relocation
  void addFixup(DWord pOffset, SWord pAddend, Type pType);

  /// addFixup - add a fixup from a existing fixup of the prototype
  void addFixup(const Fixup& pFixup);

  const FixupListType& getFixupList() const { return m_FixupList; }
  FixupListType&       getFixupList()       { return m_FixupList; }

 private:
  /// doClone - when adding a backend stub, we should implement this function
  virtual Stub* doClone() = 0;

 private:
  ResolveInfo* m_pSymInfo;
  FixupListType m_FixupList;
};

}  // namespace mcld

#endif  // MCLD_FRAGMENT_STUB_H_
