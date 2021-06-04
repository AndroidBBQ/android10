//===- HashTable.tcc ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// template implementation of HashTable
template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::HashTable(
    size_type pSize)
    : HashTableImpl<HashEntryTy, HashFunctionTy>(pSize), m_EntryFactory() {
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::~HashTable() {
  if (BaseTy::empty())
    return;

  /** clean up **/
  for (unsigned int i = 0; i < BaseTy::m_NumOfBuckets; ++i) {
    if (bucket_type::getEmptyBucket() != BaseTy::m_Buckets[i].Entry &&
        bucket_type::getTombstone() != BaseTy::m_Buckets[i].Entry) {
      m_EntryFactory.destroy(BaseTy::m_Buckets[i].Entry);
    }
  }
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
void HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::clear() {
  if (BaseTy::empty())
    return;

  /** clean up **/
  for (unsigned int i = 0; i < BaseTy::m_NumOfBuckets; ++i) {
    if (bucket_type::getEmptyBucket() != BaseTy::m_Buckets[i].Entry) {
      if (bucket_type::getTombstone() != BaseTy::m_Buckets[i].Entry) {
        m_EntryFactory.destroy(BaseTy::m_Buckets[i].Entry);
      }
      BaseTy::m_Buckets[i].Entry = bucket_type::getEmptyBucket();
    }
  }

  BaseTy::clear();
}

/// insert - insert a new element to the container. If the element already
//  exist, return the element.
template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::entry_type*
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::insert(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey,
    bool& pExist) {
  unsigned int index = BaseTy::lookUpBucketFor(pKey);
  bucket_type& bucket = BaseTy::m_Buckets[index];
  entry_type* entry = bucket.Entry;
  if (bucket_type::getEmptyBucket() != entry &&
      bucket_type::getTombstone() != entry) {
    // Already exist in the table
    pExist = true;
    return entry;
  }

  // find a tombstone
  if (bucket_type::getTombstone() == entry)
    --BaseTy::m_NumOfTombstones;

  entry = bucket.Entry = m_EntryFactory.produce(pKey);
  ++BaseTy::m_NumOfEntries;
  BaseTy::mayRehash();
  pExist = false;
  return entry;
}

/// erase - remove the elements with the pKey
//  @return the number of removed elements.
template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::size_type
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::erase(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) {
  int index;
  if ((index = BaseTy::findKey(pKey)) == -1)
    return 0;

  bucket_type& bucket = BaseTy::m_Buckets[index];
  m_EntryFactory.destroy(bucket.Entry);
  bucket.Entry = bucket_type::getTombstone();

  --BaseTy::m_NumOfEntries;
  ++BaseTy::m_NumOfTombstones;
  BaseTy::mayRehash();
  return 1;
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::find(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) {
  int index;
  if ((index = BaseTy::findKey(pKey)) == -1)
    return end();
  return iterator(this, index);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::const_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::find(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) const {
  int index;
  if ((index = BaseTy::findKey(pKey)) == -1)
    return end();
  return const_iterator(this, index);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::size_type
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::count(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) const {
  const_chain_iterator bucket, bEnd = end(pKey);
  size_type count = 0;
  for (bucket = begin(pKey); bucket != bEnd; ++bucket)
    ++count;
  return count;
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
float HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::load_factor()
    const {
  return ((float)BaseTy::m_NumOfEntries / (float)BaseTy::m_NumOfBuckets);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
void HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::rehash() {
  BaseTy::mayRehash();
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
void HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::rehash(
    typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::size_type
        pCount) {
  BaseTy::doRehash(pCount);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::begin() {
  if (BaseTy::empty())
    return end();
  unsigned int index = 0;
  while (bucket_type::getTombstone() == BaseTy::m_Buckets[index].Entry ||
         bucket_type::getEmptyBucket() == BaseTy::m_Buckets[index].Entry) {
    ++index;
  }
  return iterator(this, index);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::end() {
  return iterator(NULL, 0);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::const_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::begin() const {
  if (BaseTy::empty())
    return end();
  unsigned int index = 0;
  while (bucket_type::getTombstone() == BaseTy::m_Buckets[index].Entry ||
         bucket_type::getEmptyBucket() == BaseTy::m_Buckets[index].Entry) {
    ++index;
  }
  return const_iterator(this, index);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::const_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::end() const {
  return const_iterator(NULL, 0);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::chain_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::begin(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) {
  return chain_iterator(this, pKey, 0x0);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::chain_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::end(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) {
  return chain_iterator();
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy,
                   HashFunctionTy,
                   EntryFactoryTy>::const_chain_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::begin(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) const {
  return const_chain_iterator(this, pKey, 0x0);
}

template <typename HashEntryTy,
          typename HashFunctionTy,
          typename EntryFactoryTy>
typename HashTable<HashEntryTy,
                   HashFunctionTy,
                   EntryFactoryTy>::const_chain_iterator
HashTable<HashEntryTy, HashFunctionTy, EntryFactoryTy>::end(
    const typename HashTable<HashEntryTy,
                             HashFunctionTy,
                             EntryFactoryTy>::key_type& pKey) const {
  return const_chain_iterator();
}
