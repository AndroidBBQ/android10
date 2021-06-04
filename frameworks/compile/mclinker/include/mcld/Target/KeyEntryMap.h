//===- KeyEntryMap.h ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_KEYENTRYMAP_H_
#define MCLD_TARGET_KEYENTRYMAP_H_

#include <list>
#include <vector>

namespace mcld {

/** \class KeyEntryMap
 *  \brief KeyEntryMap is a <const KeyType*, ENTRY*> map.
 */
template <typename KEY, typename ENTRY>
class KeyEntryMap {
 public:
  typedef KEY KeyType;
  typedef ENTRY EntryType;

 private:
  struct EntryPair {
    EntryPair(EntryType* pEntry1, EntryType* pEntry2)
        : entry1(pEntry1), entry2(pEntry2) {}

    EntryType* entry1;
    EntryType* entry2;
  };

  /// EntryOrPair - A key may mapping to a signal entry or a pair of entries,
  /// user is responsible for the type of Mapping.entry
  union EntryOrPair {
    EntryType* entry_ptr;
    EntryPair* pair_ptr;
  };

  struct Mapping {
    const KeyType* key;
    EntryOrPair entry;
  };

  typedef std::vector<Mapping> KeyEntryPool;
  typedef std::list<EntryPair> PairListType;

 public:
  typedef typename KeyEntryPool::iterator iterator;
  typedef typename KeyEntryPool::const_iterator const_iterator;

 public:
  /// lookUp - look up the entry mapping to pKey
  const EntryType* lookUp(const KeyType& pKey) const;
  EntryType* lookUp(const KeyType& pKey);

  /// lookUpFirstEntry - look up the first entry mapping to pKey
  const EntryType* lookUpFirstEntry(const KeyType& pKey) const;
  EntryType* lookUpFirstEntry(const KeyType& pKey);

  /// lookUpSecondEntry - look up the second entry mapping to pKey
  const EntryType* lookUpSecondEntry(const KeyType& pKey) const;
  EntryType* lookUpSecondEntry(const KeyType& pKey);

  void record(const KeyType& pKey, EntryType& pEntry);
  void record(const KeyType& pKey, EntryType& pEntry1, EntryType& pEntry2);

  bool empty() const { return m_Pool.empty(); }
  size_t size() const { return m_Pool.size(); }

  const_iterator begin() const { return m_Pool.begin(); }
  iterator begin() { return m_Pool.begin(); }
  const_iterator end() const { return m_Pool.end(); }
  iterator end() { return m_Pool.end(); }

  void reserve(size_t pSize) { m_Pool.reserve(pSize); }

 private:
  KeyEntryPool m_Pool;

  /// m_Pairs - the EntryPairs
  PairListType m_Pairs;
};

template <typename KeyType, typename EntryType>
const EntryType* KeyEntryMap<KeyType, EntryType>::lookUp(
    const KeyType& pKey) const {
  const_iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.entry_ptr;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
EntryType* KeyEntryMap<KeyType, EntryType>::lookUp(const KeyType& pKey) {
  iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.entry_ptr;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
const EntryType* KeyEntryMap<KeyType, EntryType>::lookUpFirstEntry(
    const KeyType& pKey) const {
  const_iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.pair_ptr->entry1;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
EntryType* KeyEntryMap<KeyType, EntryType>::lookUpFirstEntry(
    const KeyType& pKey) {
  const_iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.pair_ptr->entry1;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
const EntryType* KeyEntryMap<KeyType, EntryType>::lookUpSecondEntry(
    const KeyType& pKey) const {
  const_iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.pair_ptr->entry2;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
EntryType* KeyEntryMap<KeyType, EntryType>::lookUpSecondEntry(
    const KeyType& pKey) {
  const_iterator mapping, mEnd = m_Pool.end();
  for (mapping = m_Pool.begin(); mapping != mEnd; ++mapping) {
    if (mapping->key == &pKey) {
      return mapping->entry.pair_ptr->entry2;
    }
  }

  return NULL;
}

template <typename KeyType, typename EntryType>
void KeyEntryMap<KeyType, EntryType>::record(const KeyType& pKey,
                                             EntryType& pEntry) {
  Mapping mapping;
  mapping.key = &pKey;
  mapping.entry.entry_ptr = &pEntry;
  m_Pool.push_back(mapping);
}

template <typename KeyType, typename EntryType>
void KeyEntryMap<KeyType, EntryType>::record(const KeyType& pKey,
                                             EntryType& pEntry1,
                                             EntryType& pEntry2) {
  Mapping mapping;
  mapping.key = &pKey;
  m_Pairs.push_back(EntryPair(&pEntry1, &pEntry2));
  mapping.entry.pair_ptr = &m_Pairs.back();
  m_Pool.push_back(mapping);
}

}  // namespace mcld

#endif  // MCLD_TARGET_KEYENTRYMAP_H_
