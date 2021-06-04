//===- HashEntry.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MCLD_ADT_HASHENTRY_H_
#define MCLD_ADT_HASHENTRY_H_

namespace mcld {

/** forward declaration **/
template <typename HashEntryTy>
class EntryFactory;

/** \class HashEntry
 *  \brief HashEntry is the item in the bucket of hash table.
 *
 *  mcld::HashEntry illustrates the demand from mcld::HashTable.
 *  Since HashTable can change the definition of the HashEntry by changing
 *  the template argument. class mcld::HashEntry here is used to show the
 *  basic interfaces that HashTable requests. You can define your own entry
 *  of the hash table which has no relation to mcld::HashEntry
 *
 *  Since mcld::HashEntry here is a special class whose size is changing,
 *  derive a new class from it is risky. Make sure you understand what you
 *  are doing when you let a new class inherit from mcld::HashEntry.
 */
template <typename KeyType, typename ValueType, typename KeyCompare>
class HashEntry {
 public:
  typedef KeyType key_type;
  typedef ValueType value_type;
  typedef KeyCompare key_compare;

 private:
  typedef HashEntry<KeyType, ValueType, KeyCompare> Self;
  friend class EntryFactory<Self>;

 private:
  explicit HashEntry(const KeyType& pKey);
  ~HashEntry();

 public:
  KeyType& key() { return m_Key; }

  const KeyType& key() const { return m_Key; }

  ValueType& value() { return m_Value; }

  const ValueType& value() const { return m_Value; }

  void setValue(const ValueType& pValue) { m_Value = pValue; }

  bool compare(const key_type& pKey);

 public:
  KeyType m_Key;
  ValueType m_Value;
};

template <typename HashEntryTy>
class EntryFactory {
 public:
  typedef HashEntryTy entry_type;
  typedef typename HashEntryTy::key_type key_type;
  typedef typename HashEntryTy::value_type value_type;

 public:
  EntryFactory();
  ~EntryFactory();

  HashEntryTy* produce(const key_type& pKey);
  void destroy(HashEntryTy* pEntry);
};

#include "HashEntry.tcc"

}  // namespace mcld

#endif  // MCLD_ADT_HASHENTRY_H_
