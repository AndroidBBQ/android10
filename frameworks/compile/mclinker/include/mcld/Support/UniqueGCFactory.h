//===- UniqueGCFactory.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_UNIQUEGCFACTORY_H_
#define MCLD_SUPPORT_UNIQUEGCFACTORY_H_

#include "mcld/Support/GCFactory.h"

#include <map>
#include <utility>

namespace mcld {

/** \class UniqueGCFactoryBase
 *  \brief UniqueGCFactories are unique associative factories, meaning that
 *  no two elements have the same key.
 */
template <typename KeyType, typename DataType, size_t ChunkSize>
class UniqueGCFactoryBase
    : public GCFactoryBase<LinearAllocator<DataType, ChunkSize> > {
 protected:
  typedef GCFactoryBase<LinearAllocator<DataType, ChunkSize> > Alloc;
  typedef std::map<KeyType, DataType*> KeyMap;

 protected:
  UniqueGCFactoryBase()
      : GCFactoryBase<LinearAllocator<DataType, ChunkSize> >() {}

  explicit UniqueGCFactoryBase(size_t pNum)
      : GCFactoryBase<LinearAllocator<DataType, ChunkSize> >(pNum) {}

 public:
  virtual ~UniqueGCFactoryBase() { f_KeyMap.clear(); }

  DataType* find(const KeyType& pKey) {
    typename KeyMap::iterator dataIter = f_KeyMap.find(pKey);
    if (dataIter != f_KeyMap.end())
      return dataIter->second;
    return 0;
  }

  const DataType* find(const KeyType& pKey) const {
    typename KeyMap::const_iterator dataIter = f_KeyMap.find(pKey);
    if (dataIter != f_KeyMap.end())
      return dataIter->second;
    return 0;
  }

  DataType* produce(const KeyType& pKey, bool& pExist) {
    typename KeyMap::iterator dataIter = f_KeyMap.find(pKey);
    if (dataIter != f_KeyMap.end()) {
      pExist = true;
      return dataIter->second;
    }
    DataType* data = Alloc::allocate();
    construct(data);
    f_KeyMap.insert(std::make_pair(pKey, data));
    pExist = false;
    return data;
  }

  DataType* produce(const KeyType& pKey, const DataType& pValue, bool& pExist) {
    typename KeyMap::iterator dataIter = f_KeyMap.find(pKey);
    if (dataIter != f_KeyMap.end()) {
      pExist = true;
      return dataIter->second;
    }
    DataType* data = Alloc::allocate();
    construct(data, pValue);
    f_KeyMap.insert(std::make_pair(pKey, data));
    pExist = false;
    return data;
  }

 protected:
  KeyMap f_KeyMap;
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_UNIQUEGCFACTORY_H_
