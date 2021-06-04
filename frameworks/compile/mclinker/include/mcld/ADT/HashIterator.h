//===- HashIterator.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_HASHITERATOR_H_
#define MCLD_ADT_HASHITERATOR_H_

#include <cstddef>

namespace mcld {

/** \class ChainIteratorBase
 *  \brief ChaintIteratorBase follows the HashEntryTy with the same hash value.
 */
template <typename HashTableImplTy>
class ChainIteratorBase {
 public:
  typedef HashTableImplTy hash_table;
  typedef typename HashTableImplTy::key_type key_type;
  typedef typename HashTableImplTy::entry_type entry_type;
  typedef typename HashTableImplTy::bucket_type bucket_type;

 public:
  ChainIteratorBase()
      : m_pHashTable(NULL), m_Index(0), m_HashValue(0), m_EndIndex(0) {}

  ChainIteratorBase(HashTableImplTy* pTable, const key_type& pKey)
      : m_pHashTable(pTable) {
    m_HashValue = pTable->hash()(pKey);
    m_EndIndex = m_Index = m_HashValue % m_pHashTable->m_NumOfBuckets;
    const unsigned int probe = 1;
    while (true) {
      bucket_type& bucket = m_pHashTable->m_Buckets[m_Index];
      if (bucket_type::getTombstone() == bucket.Entry) {
        // Ignore tombstones.
      } else if (m_HashValue == bucket.FullHashValue) {
        if (bucket.Entry->compare(pKey)) {
          m_EndIndex = m_Index;
          break;
        }
      }
      m_Index += probe;
      if (m_Index == m_pHashTable->m_NumOfBuckets)
        m_Index = 0;
      // doesn't exist
      if (m_EndIndex == m_Index) {
        reset();
        break;
      }
    }
  }

  ChainIteratorBase(const ChainIteratorBase& pCopy)
      : m_pHashTable(pCopy.m_pHashTable),
        m_Index(pCopy.m_Index),
        m_HashValue(pCopy.m_HashValue),
        m_EndIndex(pCopy.m_EndIndex) {}

  ChainIteratorBase& assign(const ChainIteratorBase& pCopy) {
    m_pHashTable = pCopy.m_pHashTable;
    m_Index = pCopy.m_Index;
    m_HashValue = pCopy.m_HashValue;
    m_EndIndex = pCopy.m_EndIndex;
    return *this;
  }

  inline bucket_type* getBucket() {
    if (m_pHashTable == NULL)
      return NULL;
    return &(m_pHashTable->m_Buckets[m_Index]);
  }

  inline const bucket_type* getBucket() const {
    if (m_pHashTable == NULL)
      return NULL;
    return &(m_pHashTable->m_Buckets[m_Index]);
  }

  inline entry_type* getEntry() {
    if (m_pHashTable == NULL)
      return NULL;
    return m_pHashTable->m_Buckets[m_Index].Entry;
  }

  inline const entry_type* getEntry() const {
    if (m_pHashTable == NULL)
      return NULL;
    return m_pHashTable->m_Buckets[m_Index].Entry;
  }

  inline void reset() {
    m_pHashTable = NULL;
    m_Index = 0;
    m_EndIndex = 0;
    m_HashValue = 0;
  }

  inline void advance() {
    if (m_pHashTable == NULL)
      return;
    const unsigned int probe = 1;
    while (true) {
      m_Index += probe;
      if (m_Index == m_pHashTable->m_NumOfBuckets)
        m_Index = 0;
      // reach end()
      if (m_Index == m_EndIndex) {
        reset();
        return;
      }

      bucket_type& bucket = m_pHashTable->m_Buckets[m_Index];

      if (bucket_type::getTombstone() == bucket.Entry ||
          bucket_type::getEmptyBucket() == bucket.Entry) {
        // Ignore tombstones.
      } else if (m_HashValue == bucket.FullHashValue) {
        return;
      }
    }
  }

  bool operator==(const ChainIteratorBase& pCopy) const {
    if (m_pHashTable == pCopy.m_pHashTable) {
      if (m_pHashTable == NULL)
        return true;
      return ((m_HashValue == pCopy.m_HashValue) &&
              (m_EndIndex == pCopy.m_EndIndex) && (m_Index == pCopy.m_Index));
    }
    return false;
  }

  bool operator!=(const ChainIteratorBase& pCopy) const {
    return !(*this == pCopy);
  }

 private:
  HashTableImplTy* m_pHashTable;
  unsigned int m_Index;
  unsigned int m_HashValue;
  unsigned int m_EndIndex;
};

/** \class EntryIteratorBase
 *  \brief EntryIteratorBase walks over hash table by the natural layout of the
 *  buckets
 */
template <typename HashTableImplTy>
class EntryIteratorBase {
 public:
  typedef HashTableImplTy hash_table;
  typedef typename HashTableImplTy::key_type key_type;
  typedef typename HashTableImplTy::entry_type entry_type;
  typedef typename HashTableImplTy::bucket_type bucket_type;

 public:
  EntryIteratorBase() : m_pHashTable(NULL), m_Index(0) {}

  EntryIteratorBase(HashTableImplTy* pTable, unsigned int pIndex)
      : m_pHashTable(pTable), m_Index(pIndex) {}

  EntryIteratorBase(const EntryIteratorBase& pCopy)
      : m_pHashTable(pCopy.m_pHashTable), m_Index(pCopy.m_Index) {}

  EntryIteratorBase& assign(const EntryIteratorBase& pCopy) {
    m_pHashTable = pCopy.m_pHashTable;
    m_Index = pCopy.m_Index;
    return *this;
  }

  inline bucket_type* getBucket() {
    if (m_pHashTable == NULL)
      return NULL;
    return &(m_pHashTable->m_Buckets[m_Index]);
  }

  inline const bucket_type* getBucket() const {
    if (m_pHashTable == NULL)
      return NULL;
    return &(m_pHashTable->m_Buckets[m_Index]);
  }

  inline entry_type* getEntry() {
    if (m_pHashTable == NULL)
      return NULL;
    return m_pHashTable->m_Buckets[m_Index].Entry;
  }

  inline const entry_type* getEntry() const {
    if (m_pHashTable == NULL)
      return NULL;
    return m_pHashTable->m_Buckets[m_Index].Entry;
  }

  inline void reset() {
    m_pHashTable = NULL;
    m_Index = 0;
  }

  inline void advance() {
    if (m_pHashTable == NULL)
      return;
    do {
      ++m_Index;
      if (m_pHashTable->m_NumOfBuckets == m_Index) {  // to the end
        reset();
        return;
      }
    } while (bucket_type::getEmptyBucket() ==
                 m_pHashTable->m_Buckets[m_Index].Entry ||
             bucket_type::getTombstone() ==
                 m_pHashTable->m_Buckets[m_Index].Entry);
  }

  bool operator==(const EntryIteratorBase& pCopy) const {
    return ((m_pHashTable == pCopy.m_pHashTable) && (m_Index == pCopy.m_Index));
  }

  bool operator!=(const EntryIteratorBase& pCopy) const {
    return !(*this == pCopy);
  }

 private:
  HashTableImplTy* m_pHashTable;
  unsigned int m_Index;
};

/** \class HashIterator
 *  \brief HashIterator provides a policy-based iterator.
 *
 *  HashTable has two kinds of iterators. One is used to traverse buckets
 *  with the same hash value; the other is used to traverse all entries of the
 *  hash table.
 *
 *  HashIterator is a template policy-based iterator, which can change its
 *  behavior by change the template argument IteratorBase. HashTable defines
 *  above two iterators by defining HashIterators with different IteratorBase.
 */
template <typename IteratorBase, typename Traits>
class HashIterator : public IteratorBase {
 public:
  typedef Traits traits;
  typedef typename traits::pointer pointer;
  typedef typename traits::reference reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef IteratorBase Base;

  typedef HashIterator<IteratorBase, Traits> Self;

  typedef typename traits::nonconst_traits nonconst_traits;
  typedef HashIterator<IteratorBase, nonconst_traits> iterator;

  typedef typename traits::const_traits const_traits;
  typedef HashIterator<IteratorBase, const_traits> const_iterator;
  typedef std::forward_iterator_tag iterator_category;

 public:
  HashIterator() : IteratorBase() {}

  /// HashIterator - constructor for EntryIterator
  HashIterator(typename IteratorBase::hash_table* pTable, unsigned int pIndex)
      : IteratorBase(pTable, pIndex) {}

  /// HashIterator - constructor for ChainIterator
  explicit HashIterator(typename IteratorBase::hash_table* pTable,
                        const typename IteratorBase::key_type& pKey,
                        int)
      : IteratorBase(pTable, pKey) {}

  HashIterator(const HashIterator& pCopy) : IteratorBase(pCopy) {}

  ~HashIterator() {}

  HashIterator& operator=(const HashIterator& pCopy) {
    IteratorBase::assign(pCopy);
    return *this;
  }

  // -----  operators  ----- //
  Self& operator++() {
    this->Base::advance();
    return *this;
  }

  Self operator++(int) {
    Self tmp = *this;
    this->Base::advance();
    return tmp;
  }
};

}  // namespace mcld

#endif  // MCLD_ADT_HASHITERATOR_H_
