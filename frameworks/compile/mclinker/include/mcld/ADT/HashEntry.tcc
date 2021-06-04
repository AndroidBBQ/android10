//===- HashEntry.tcc ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===--------------------------------------------------------------------===//
// template implementation of HashEntry
template <typename KeyType, typename ValueType, typename KeyCompare>
HashEntry<KeyType, ValueType, KeyCompare>::HashEntry(const KeyType& pKey)
    : m_Key(pKey) {
}

template <typename KeyType, typename ValueType, typename KeyCompare>
HashEntry<KeyType, ValueType, KeyCompare>::~HashEntry() {
}

template <typename KeyType, typename ValueType, typename KeyCompare>
bool HashEntry<KeyType, ValueType, KeyCompare>::compare(const KeyType& pKey) {
  static KeyCompare comparator;
  return comparator(m_Key, pKey);
}

//===--------------------------------------------------------------------===//
// template implementation of EntryFactory
template <typename HashEntryTy>
EntryFactory<HashEntryTy>::EntryFactory() {
}

template <typename HashEntryTy>
EntryFactory<HashEntryTy>::~EntryFactory() {
}

template <typename HashEntryTy>
void EntryFactory<HashEntryTy>::destroy(HashEntryTy* pEntry) {
  delete pEntry;
}

template <typename HashEntryTy>
HashEntryTy* EntryFactory<HashEntryTy>::produce(
    const typename EntryFactory<HashEntryTy>::key_type& pKey) {
  return new HashEntryTy(pKey);
}
