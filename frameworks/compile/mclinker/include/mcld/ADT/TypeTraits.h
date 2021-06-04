//===- TypeTraits.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_TYPETRAITS_H_
#define MCLD_ADT_TYPETRAITS_H_

#include <cstdlib>

namespace mcld {

template <typename DataType>
struct NonConstTraits;

template <typename DataType>
struct ConstTraits {
  typedef DataType value_type;
  typedef const DataType* pointer;
  typedef const DataType& reference;
  typedef size_t size_type;
  typedef ConstTraits<DataType> const_traits;
  typedef NonConstTraits<DataType> nonconst_traits;
};

template <typename DataType>
struct NonConstTraits {
  typedef DataType value_type;
  typedef DataType* pointer;
  typedef DataType& reference;
  typedef size_t size_type;
  typedef ConstTraits<DataType> const_traits;
  typedef NonConstTraits<DataType> nonconst_traits;
};

template <typename DataType>
struct ConstIteratorTraits {
  typedef DataType value_type;
  typedef const DataType* pointer;
  typedef const DataType& reference;
  typedef size_t size_type;
  typedef ConstTraits<DataType> const_traits;
  typedef NonConstTraits<DataType> nonconst_traits;
  typedef typename DataType::const_iterator iterator;
};

template <typename DataType>
struct NonConstIteratorTraits {
  typedef DataType value_type;
  typedef DataType* pointer;
  typedef DataType& reference;
  typedef size_t size_type;
  typedef ConstTraits<DataType> const_traits;
  typedef NonConstTraits<DataType> nonconst_traits;
  typedef typename DataType::iterator iterator;
};

}  // namespace mcld

#endif  // MCLD_ADT_TYPETRAITS_H_
