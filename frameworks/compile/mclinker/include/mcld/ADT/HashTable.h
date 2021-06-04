//===- HashTable.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_HASHTABLE_H_
#define MCLD_ADT_HASHTABLE_H_

#include "mcld/ADT/HashBase.h"
#include "mcld/ADT/HashEntryFactory.h"
#include "mcld/ADT/HashIterator.h"
#include "mcld/ADT/TypeTraits.h"
#include "mcld/Support/Allocators.h"
#include "mcld/Support/Compiler.h"

#include <utility>

namespace mcld {

/** \class HashTable
 *  \brief HashTable is a hash table which follows boost::unordered_map, but it
 *  is open addressing and can linear probing.
 *
 *  mcld::HashTable is a linear probing hash table. It does not allocate
 *  the memory space of the entries by itself. Instead, entries are allocated
 *  outside and then emplaced into the hash table.
 */
template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy = HashEntryFactory<HashEntryTy> >
class HashTable : public HashTableImpl<HashEntryTy, HashFunctionTy> {
 private:
  typedef HashTableImpl<HashEntryTy, HashFunctionTy> BaseTy;

 public:
  typedef size_t size_type;
  typedef HashFunctionTy hasher;
  typedef HashEntryTy entry_type;
  typedef typename BaseTy::bucket_type bucket_type;
  typedef typename HashEntryTy::key_type key_type;

  typedef HashIterator<ChainIteratorBase<BaseTy>, NonConstTraits<HashEntryTy> >
      chain_iterator;
  typedef HashIterator<ChainIteratorBase<const BaseTy>,
                       ConstTraits<HashEntryTy> > const_chain_iterator;

  typedef HashIterator<EntryIteratorBase<BaseTy>, NonConstTraits<HashEntryTy> >
      entry_iterator;
  typedef HashIterator<EntryIteratorBase<const BaseTy>,
                       ConstTraits<HashEntryTy> > const_entry_iterator;

  typedef entry_iterator iterator;
  typedef const_entry_iterator const_iterator;

 public:
  // -----  constructor  ----- //
  explicit HashTable(size_type pSize = 3);
  ~HashTable();

  EntryFactoryTy& getEntryFactory() { return m_EntryFactory; }

  // -----  modifiers  ----- //
  void clear();

  /// insert - insert a new element to the container. The element is
  //  constructed in-place, i.e. no copy or move operations are performed.
  //  If the element already exists, return the element, and set pExist true.
  entry_type* insert(const key_type& pKey, bool& pExist);

  /// erase - remove the element with the same key
  size_type erase(const key_type& pKey);

  // -----  lookups  ----- //
  /// find - finds an element with key pKey
  //  If the element does not exist, return end()
  iterator find(const key_type& pKey);

  /// find - finds an element with key pKey, constant version
  //  If the element does not exist, return end()
  const_iterator find(const key_type& pKey) const;

  size_type count(const key_type& pKey) const;

  // -----  hash policy  ----- //
  float load_factor() const;

  /// rehash - if the load factor is larger than 75%, or the empty buckets is
  //  less than 12.5%, the rehash the hash table
  void rehash();

  /// rehash - immediately re-new the hash table to the size pCount, and
  //  rehash all elements.
  void rehash(size_type pCount);

  // -----  iterators  ----- //
  iterator begin();
  iterator end();

  const_entry_iterator begin() const;
  const_entry_iterator end() const;

  chain_iterator begin(const key_type& pKey);
  chain_iterator end(const key_type& pKey);
  const_chain_iterator begin(const key_type& pKey) const;
  const_chain_iterator end(const key_type& pKey) const;

 private:
  EntryFactoryTy m_EntryFactory;

 private:
  DISALLOW_COPY_AND_ASSIGN(HashTable);
};

#include "HashTable.tcc"

}  // namespace mcld

#endif  // MCLD_ADT_HASHTABLE_H_
