//===- HashBase.tcc -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// internal non-member functions
//===----------------------------------------------------------------------===//
inline static unsigned int compute_bucket_count(unsigned int pNumOfBuckets) {
  static const unsigned int bucket_size[] = {
      1,     3,     17,    37,    67,    97,     197,
      419,   977,   2593,  4099,  8209,  12289,  16411,
      20483, 32771, 49157, 65537, 98317, 131101, 196613};

  const unsigned int buckets_count =
      sizeof(bucket_size) / sizeof(bucket_size[0]);
  unsigned int idx = 0;
  do {
    if (pNumOfBuckets < bucket_size[idx]) {
      return bucket_size[idx];
    }
    ++idx;
  } while (idx < buckets_count);

  return (pNumOfBuckets + 131101);  // rare case. increase constantly
}

//===----------------------------------------------------------------------===//
// template implementation of HashBucket
//===----------------------------------------------------------------------===//
template <typename DataType>
typename HashBucket<DataType>::entry_type*
HashBucket<DataType>::getEmptyBucket() {
  static entry_type* empty_bucket = reinterpret_cast<entry_type*>(0x0);
  return empty_bucket;
}

template <typename DataType>
typename HashBucket<DataType>::entry_type*
HashBucket<DataType>::getTombstone() {
  static entry_type* tombstone = reinterpret_cast<entry_type*>(0x1);
  return tombstone;
}

//===----------------------------------------------------------------------===//
// template implementation of HashTableImpl
//===----------------------------------------------------------------------===//
template <typename HashEntryTy, typename HashFunctionTy>
HashTableImpl<HashEntryTy, HashFunctionTy>::HashTableImpl()
    : m_Buckets(0),
      m_NumOfBuckets(0),
      m_NumOfEntries(0),
      m_NumOfTombstones(0),
      m_Hasher() {
}

template <typename HashEntryTy, typename HashFunctionTy>
HashTableImpl<HashEntryTy, HashFunctionTy>::HashTableImpl(
    unsigned int pInitSize)
    : m_Hasher() {
  if (pInitSize) {
    init(pInitSize);
    return;
  }

  m_Buckets = 0;
  m_NumOfBuckets = 0;
  m_NumOfEntries = 0;
  m_NumOfTombstones = 0;
}

template <typename HashEntryTy, typename HashFunctionTy>
HashTableImpl<HashEntryTy, HashFunctionTy>::~HashTableImpl() {
  clear();
}

/// empty - check if the hash table is empty
template <typename HashEntryTy, typename HashFunctionTy>
bool HashTableImpl<HashEntryTy, HashFunctionTy>::empty() const {
  return (m_NumOfEntries == 0);
}

/// init - initialize the hash table.
template <typename HashEntryTy, typename HashFunctionTy>
void HashTableImpl<HashEntryTy, HashFunctionTy>::init(unsigned int pInitSize) {
  m_NumOfBuckets =
      pInitSize ? compute_bucket_count(pInitSize) : NumOfInitBuckets;

  m_NumOfEntries = 0;
  m_NumOfTombstones = 0;

  /** calloc also set bucket.Item = bucket_type::getEmptyStone() **/
  m_Buckets = (bucket_type*)calloc(m_NumOfBuckets, sizeof(bucket_type));
}

/// clear - clear the hash table.
template <typename HashEntryTy, typename HashFunctionTy>
void HashTableImpl<HashEntryTy, HashFunctionTy>::clear() {
  free(m_Buckets);

  m_Buckets = 0;
  m_NumOfBuckets = 0;
  m_NumOfEntries = 0;
  m_NumOfTombstones = 0;
}

/// lookUpBucketFor - look up the bucket whose key is pKey
template <typename HashEntryTy, typename HashFunctionTy>
unsigned int HashTableImpl<HashEntryTy, HashFunctionTy>::lookUpBucketFor(
    const typename HashTableImpl<HashEntryTy, HashFunctionTy>::key_type& pKey) {
  if (m_NumOfBuckets == 0) {
    // NumOfBuckets is changed after init(pInitSize)
    init(NumOfInitBuckets);
  }

  unsigned int full_hash = m_Hasher(pKey);
  unsigned int index = full_hash % m_NumOfBuckets;

  const unsigned int probe = 1;
  int firstTombstone = -1;

  // linear probing
  while (true) {
    bucket_type& bucket = m_Buckets[index];
    // If we found an empty bucket, this key isn't in the table yet, return it.
    if (bucket_type::getEmptyBucket() == bucket.Entry) {
      if (firstTombstone != -1) {
        m_Buckets[firstTombstone].FullHashValue = full_hash;
        return firstTombstone;
      }

      bucket.FullHashValue = full_hash;
      return index;
    }

    if (bucket_type::getTombstone() == bucket.Entry) {
      if (firstTombstone == -1) {
        firstTombstone = index;
      }
    } else if (bucket.FullHashValue == full_hash) {
      if (bucket.Entry->compare(pKey)) {
        return index;
      }
    }

    index += probe;
    if (index == m_NumOfBuckets)
      index = 0;
  }
}

template <typename HashEntryTy, typename HashFunctionTy>
int HashTableImpl<HashEntryTy, HashFunctionTy>::findKey(
    const typename HashTableImpl<HashEntryTy, HashFunctionTy>::key_type& pKey)
    const {
  if (m_NumOfBuckets == 0)
    return -1;

  unsigned int full_hash = m_Hasher(pKey);
  unsigned int index = full_hash % m_NumOfBuckets;

  const unsigned int probe = 1;
  // linear probing
  while (true) {
    bucket_type& bucket = m_Buckets[index];

    if (bucket_type::getEmptyBucket() == bucket.Entry)
      return -1;

    if (bucket_type::getTombstone() == bucket.Entry) {
      // Ignore tombstones.
    } else if (full_hash == bucket.FullHashValue) {
      // get string, compare, if match, return index
      if (bucket.Entry->compare(pKey))
        return index;
    }
    index += probe;
    if (index == m_NumOfBuckets)
      index = 0;
  }
}

template <typename HashEntryTy, typename HashFunctionTy>
void HashTableImpl<HashEntryTy, HashFunctionTy>::mayRehash() {
  unsigned int new_size;
  // If the hash table is now more than 3/4 full, or if fewer than 1/8 of
  // the buckets are empty (meaning that many are filled with tombstones),
  // grow/rehash the table.
  if ((m_NumOfEntries << 2) > m_NumOfBuckets * 3)
    new_size = compute_bucket_count(m_NumOfBuckets);
  else if (((m_NumOfBuckets - (m_NumOfEntries + m_NumOfTombstones)) << 3) <
           m_NumOfBuckets)
    new_size = m_NumOfBuckets;
  else
    return;

  doRehash(new_size);
}

template <typename HashEntryTy, typename HashFunctionTy>
void HashTableImpl<HashEntryTy, HashFunctionTy>::doRehash(
    unsigned int pNewSize) {
  bucket_type* new_table = (bucket_type*)calloc(pNewSize, sizeof(bucket_type));

  // Rehash all the items into their new buckets.  Luckily :) we already have
  // the hash values available, so we don't have to recall hash function again.
  for (bucket_type* IB = m_Buckets, * E = m_Buckets + m_NumOfBuckets; IB != E;
       ++IB) {
    if (IB->Entry != bucket_type::getEmptyBucket() &&
        IB->Entry != bucket_type::getTombstone()) {
      // Fast case, bucket available.
      unsigned full_hash = IB->FullHashValue;
      unsigned new_bucket = full_hash % pNewSize;
      if (bucket_type::getEmptyBucket() == new_table[new_bucket].Entry) {
        new_table[new_bucket].Entry = IB->Entry;
        new_table[new_bucket].FullHashValue = full_hash;
        continue;
      }

      // Otherwise probe for a spot.
      const unsigned int probe = 1;
      do {
        new_bucket += probe;
        if (new_bucket == pNewSize)
          new_bucket = 0;
      } while (new_table[new_bucket].Entry != bucket_type::getEmptyBucket());

      // Finally found a slot.  Fill it in.
      new_table[new_bucket].Entry = IB->Entry;
      new_table[new_bucket].FullHashValue = full_hash;
    }
  }

  free(m_Buckets);

  m_Buckets = new_table;
  m_NumOfBuckets = pNewSize;
  m_NumOfTombstones = 0;
}
