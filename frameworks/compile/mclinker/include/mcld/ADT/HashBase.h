//===- HashBase.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_HASHBASE_H_
#define MCLD_ADT_HASHBASE_H_

#include <llvm/ADT/StringRef.h>

#include <cstdlib>

namespace mcld {

/** forward declaration **/
template <typename HashTableImplTy>
class ChainIteratorBase;

template <typename HashTableImplTy>
class EntryIteratorBase;

/** \class HashBucket
 *  \brief HashBucket is an entry in the hash table.
 */
template <typename HashEntryTy>
class HashBucket {
 public:
  typedef HashEntryTy entry_type;

 public:
  unsigned int FullHashValue;
  entry_type* Entry;

 public:
  static entry_type* getEmptyBucket();
  static entry_type* getTombstone();
};

/** \class HashTableImpl
 *  \brief HashTableImpl is the base class of HashTable.
 *
 *  HashTableImpl uses open-addressing, linear probing hash table.
 *  linear probing hash table obviously has high performance when the
 *  load factor is less than 0.7.
 *  The drawback is that the number of the stored items can notbe more
 *  than the size of the hash table.
 *
 *  MCLinker tries to merge every things in the same HashEntry. It can
 *  keep every thing in the same cache line and improve the locality
 *  efficiently. HashTableImpl provides a template argument to change the
 *  definition of HashEntries.
 *
 *  HashEntryTy must provide getKey() and getKenLength() functions.
 *
 *  Different environments have different demands of HashFunctions. For
 *  example, on-device linkers needs a more light-weight hash function
 *  than static linkers. HashTableImpl also provides a template argument to
 *  change the hash functions.
 */
template <typename HashEntryTy, typename HashFunctionTy>
class HashTableImpl {
 private:
  static const unsigned int NumOfInitBuckets = 16;

 public:
  typedef size_t size_type;
  typedef HashFunctionTy hasher;
  typedef HashEntryTy entry_type;
  typedef typename HashEntryTy::key_type key_type;
  typedef HashBucket<HashEntryTy> bucket_type;
  typedef HashTableImpl<HashEntryTy, HashFunctionTy> Self;

 public:
  HashTableImpl();
  explicit HashTableImpl(unsigned int pInitSize);
  virtual ~HashTableImpl();

  // -----  observers  ----- //
  bool empty() const;

  size_t numOfBuckets() const { return m_NumOfBuckets; }

  size_t numOfEntries() const { return m_NumOfEntries; }

  hasher& hash() { return m_Hasher; }

  const hasher& hash() const { return m_Hasher; }

 protected:
  /// initialize the hash table.
  void init(unsigned int pInitSize);

  void clear();

  /// lookUpBucketFor - search the index of bucket whose key is p>ey
  //  @return the index of the found bucket
  unsigned int lookUpBucketFor(const key_type& pKey);

  /// findKey - finds an element with key pKey
  //  return the index of the element, or -1 when the element does not exist.
  int findKey(const key_type& pKey) const;

  /// mayRehash - check the load_factor, compute the new size, and then doRehash
  void mayRehash();

  /// doRehash - re-new the hash table, and rehash all elements into the new
  /// buckets
  void doRehash(unsigned int pNewSize);

  friend class ChainIteratorBase<Self>;
  friend class ChainIteratorBase<const Self>;
  friend class EntryIteratorBase<Self>;
  friend class EntryIteratorBase<const Self>;

 protected:
  // Array of Buckets
  bucket_type* m_Buckets;
  unsigned int m_NumOfBuckets;
  unsigned int m_NumOfEntries;
  unsigned int m_NumOfTombstones;
  hasher m_Hasher;
};

#include "HashBase.tcc"

}  // namespace mcld

#endif  // MCLD_ADT_HASHBASE_H_
