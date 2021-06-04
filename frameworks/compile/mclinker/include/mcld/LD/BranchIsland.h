//===- BranchIsland.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_BRANCHISLAND_H_
#define MCLD_LD_BRANCHISLAND_H_

#include "mcld/ADT/HashEntry.h"
#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/LDSymbol.h"

#include <llvm/Support/DataTypes.h>
#include <llvm/ADT/StringRef.h>

#include <string>

namespace mcld {

class Relocation;
class Stub;

/** \class BranchIsland
 *  \brief BranchIsland is a collection of stubs
 *
 */
class BranchIsland {
 public:
  typedef SectionData::iterator iterator;
  typedef SectionData::const_iterator const_iterator;

  typedef std::vector<Relocation*> RelocationListType;
  typedef RelocationListType::iterator reloc_iterator;
  typedef RelocationListType::const_iterator const_reloc_iterator;

 public:
  /*
   *               ----------
   *  --- Entry -> | Island | -> Exit ---
   *               ----------
   */

  /// BranchIsland - constructor
  /// @param pEntryFrag - the entry fragment to the island
  /// @param pMaxSize   - the max size the island can be
  /// @param pIndex     - the inedx in the island factory
  BranchIsland(Fragment& pEntryFrag, size_t pMaxSize, size_t pIndex);

  ~BranchIsland();

  /// fragment iterators of the island
  iterator begin();

  const_iterator begin() const;

  iterator end();

  const_iterator end() const;

  /// relocation iterators of the island
  reloc_iterator reloc_begin() { return m_Relocations.begin(); }

  const_reloc_iterator reloc_begin() const { return m_Relocations.begin(); }

  reloc_iterator reloc_end() { return m_Relocations.end(); }

  const_reloc_iterator reloc_end() const { return m_Relocations.end(); }

  /// observers
  SectionData* getParent() const { return m_Entry.getParent(); }

  uint64_t offset() const;

  size_t size() const;

  size_t maxSize() const;

  const std::string& name() const;

  size_t numOfStubs() const;

  /// findStub - return true if there is a stub built from the given prototype
  ///            for the given relocation
  Stub* findStub(const Stub* pPrototype, const Relocation& pReloc);

  /// addStub - add a stub into the island
  bool addStub(const Stub* pPrototype, const Relocation& pReloc, Stub& pStub);

  void addStub(Stub& pStub);

  /// addRelocation - add a relocation into island
  bool addRelocation(Relocation& pReloc);

 private:
  /** \class Key
   *  \brief Key to recognize a stub in the island.
   *
   */
  class Key {
   public:
    Key(const Stub* pPrototype, const LDSymbol* pSymbol, Stub::SWord pAddend)
        : m_pPrototype(pPrototype), m_pSymbol(pSymbol), m_Addend(pAddend) {}

    ~Key() {}

    const Stub* prototype() const { return m_pPrototype; }

    const LDSymbol* symbol() const { return m_pSymbol; }

    Stub::SWord addend() const { return m_Addend; }

    struct Hash {
      size_t operator()(const Key& KEY) const {
        llvm::StringRef sym_name(KEY.symbol()->name());
        hash::StringHash<hash::DJB> str_hasher;
        return (size_t((uintptr_t)KEY.prototype())) ^ str_hasher(sym_name) ^
               KEY.addend();
      }
    };

    struct Compare {
      bool operator()(const Key& KEY1, const Key& KEY2) const {
        bool res = false;
        if ((KEY1.prototype() == KEY2.prototype()) &&
            (KEY1.addend() == KEY2.addend())) {
          if (KEY1.symbol() == KEY2.symbol()) {
            res = true;
          } else {
            // Folded symbols may use the existing stub.
            if (KEY1.symbol()->hasFragRef() && KEY2.symbol()->hasFragRef()) {
              const FragmentRef* ref1 = KEY1.symbol()->fragRef();
              const FragmentRef* ref2 = KEY2.symbol()->fragRef();
              if ((ref1->offset() == ref2->offset()) &&
                  (ref1->frag()->getOffset() == ref2->frag()->getOffset())) {
                res = true;
              }
            }
          }
        }
        return res;
      }
    };

   private:
    const Stub* m_pPrototype;
    const LDSymbol* m_pSymbol;
    Stub::SWord m_Addend;
  };

  typedef HashEntry<Key, Stub*, Key::Compare> StubEntryType;

  typedef HashTable<StubEntryType, Key::Hash, EntryFactory<StubEntryType> >
      StubMapType;

 private:
  Fragment& m_Entry;  // entry fragment of the island
  Fragment* m_pExit;  // exit fragment of the island
  Fragment* m_pRear;  // rear fragment of the island
  const size_t m_MaxSize;
  std::string m_Name;
  StubMapType m_StubMap;
  /// m_Relocations - list of relocations created for stubs in this island
  RelocationListType m_Relocations;
};

}  // namespace mcld

#endif  // MCLD_LD_BRANCHISLAND_H_
