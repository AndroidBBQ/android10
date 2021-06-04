//===- StringEntry.tcc ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// StringEntry
template <typename DataType>
StringEntry<DataType>::StringEntry()
    : m_KeyLen(0) {
}

template <typename DataType>
StringEntry<DataType>::StringEntry(const StringEntry::key_type& pKey)
    : m_KeyLen(pKey.size()) {
}

template <typename DataType>
StringEntry<DataType>::StringEntry(const StringEntry<DataType>& pCopy)
    : m_KeyLen(pCopy.m_KeyLen), m_Value(pCopy.m_Value) {
  assert("Copy constructor of StringEntry should not be called!");
}

template <typename DataType>
StringEntry<DataType>::~StringEntry() {
}

//===----------------------------------------------------------------------===//
// StringEntryFactory
template <typename DataType>
StringEntryFactory<DataType>::StringEntryFactory() {
}

template <typename DataType>
StringEntryFactory<DataType>::~StringEntryFactory() {
}

template <typename DataType>
StringEntry<DataType>* StringEntryFactory<DataType>::produce(
    const typename StringEntryFactory<DataType>::key_type& pKey) {
  StringEntry<DataType>* result = static_cast<StringEntry<DataType>*>(
      malloc(sizeof(StringEntry<DataType>) + pKey.size() + 1));

  if (result == NULL)
    return NULL;

  size_t len = pKey.size();
  new (result) StringEntry<DataType>();
  std::memcpy(result->m_Key, pKey.data(), len);
  result->m_Key[len] = '\0';
  result->m_KeyLen = len;
  return result;
}

template <typename DataType>
void StringEntryFactory<DataType>::destroy(StringEntry<DataType>* pEntry) {
  if (pEntry != NULL) {
    pEntry->~StringEntry<DataType>();
    free(pEntry);
  }
}
