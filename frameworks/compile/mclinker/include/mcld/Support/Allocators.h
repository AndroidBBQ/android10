//===- Allocators.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_ALLOCATORS_H_
#define MCLD_SUPPORT_ALLOCATORS_H_
#include "mcld/ADT/TypeTraits.h"
#include "mcld/Support/Compiler.h"

#include <cstddef>
#include <cstdlib>

namespace mcld {

/** \class Chunk
 *  \brief Chunk is the basic unit of the storage of the LinearAllocator
 *
 *  @see LinearAllocator
 */
template <typename DataType, size_t ChunkSize>
class Chunk {
 public:
  typedef DataType value_type;

 public:
  Chunk() : next(NULL), bound(0) {}

  static size_t size() { return ChunkSize; }

  static void construct(value_type* pPtr) { new (pPtr) value_type(); }

  static void construct(value_type* pPtr, const value_type& pValue) {
    new (pPtr) value_type(pValue);
  }

  static void destroy(value_type* pPtr) {}

 public:
  Chunk* next;
  size_t bound;
  DataType data[ChunkSize];
};

template <typename DataType>
class Chunk<DataType, 0> {
 public:
  typedef DataType value_type;

 public:
  Chunk() : next(NULL), bound(0) {
    if (m_Size != 0)
      data = reinterpret_cast<DataType*>(malloc(sizeof(DataType) * m_Size));
    else
      data = 0;
  }

  ~Chunk() {
    if (data)
      free(data);
  }

  static size_t size() { return m_Size; }

  static void setSize(size_t pSize) { m_Size = pSize; }

  static void construct(value_type* pPtr) { new (pPtr) value_type(); }

  static void construct(value_type* pPtr, const value_type& pValue) {
    new (pPtr) value_type(pValue);
  }

  static void destroy(value_type* pPtr) { pPtr->~value_type(); }

 public:
  Chunk* next;
  size_t bound;
  DataType* data;
  static size_t m_Size;
};

template <typename DataType>
size_t Chunk<DataType, 0>::m_Size = 0;

template <typename ChunkType>
class LinearAllocatorBase {
 public:
  typedef ChunkType chunk_type;
  typedef typename ChunkType::value_type value_type;
  typedef typename ChunkType::value_type* pointer;
  typedef typename ChunkType::value_type& reference;
  typedef const typename ChunkType::value_type* const_pointer;
  typedef const typename ChunkType::value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef unsigned char byte_type;

 protected:
  LinearAllocatorBase() : m_pRoot(NULL), m_pCurrent(NULL), m_AllocatedNum(0) {}

  // LinearAllocatorBase does NOT mean to destroy the allocated memory.
  // If you want a memory allocator to release memory at destruction, please
  // use GCFactory series.
  virtual ~LinearAllocatorBase() {}

 public:
  pointer address(reference X) const { return &X; }

  const_pointer address(const_reference X) const { return &X; }

  /// standard construct - constructing an object on the location pointed by
  //  pPtr, and using its copy constructor to initialized its value to pValue.
  //
  //  @param pPtr the address where the object to be constructed
  //  @param pValue the value to be constructed
  void construct(pointer pPtr, const_reference pValue) {
    chunk_type::construct(pPtr, pValue);
  }

  /// default construct - constructing an object on the location pointed by
  //  pPtr, and using its default constructor to initialized its value to
  //  pValue.
  //
  //  @param pPtr the address where the object to be constructed
  void construct(pointer pPtr) { chunk_type::construct(pPtr); }

  /// standard destroy - destroy data on arbitrary address
  //  @para pPtr the address where the data to be destruected.
  void destroy(pointer pPtr) { chunk_type::destroy(pPtr); }

  /// allocate - allocate N data in order.
  //  - Disallow to allocate a chunk whose size is bigger than a chunk.
  //
  //  @param N the number of allocated data.
  //  @return the start address of the allocated memory
  pointer allocate(size_type N) {
    if (N == 0 || N > chunk_type::size())
      return 0;

    if (empty())
      initialize();

    size_type rest_num_elem = chunk_type::size() - m_pCurrent->bound;
    pointer result = 0;
    if (N > rest_num_elem)
      getNewChunk();
    result = m_pCurrent->data + m_pCurrent->bound;
    m_pCurrent->bound += N;
    return result;
  }

  /// allocate - clone function of allocating one datum.
  pointer allocate() {
    if (empty())
      initialize();

    pointer result = 0;
    if (chunk_type::size() == m_pCurrent->bound)
      getNewChunk();
    result = m_pCurrent->data + m_pCurrent->bound;
    ++m_pCurrent->bound;
    return result;
  }

  /// deallocate - deallocate N data from the pPtr
  //  - if we can simply release some memory, then do it. Otherwise, do
  //    nothing.
  void deallocate(pointer& pPtr, size_type N) {
    if (N == 0 || N > chunk_type::size() || m_pCurrent->bound == 0 ||
        N >= m_pCurrent->bound)
      return;
    if (!isAvailable(pPtr))
      return;
    m_pCurrent->bound -= N;
    pPtr = 0;
  }

  /// deallocate - clone function of deallocating one datum
  void deallocate(pointer& pPtr) {
    if (m_pCurrent->bound == 0)
      return;
    if (!isAvailable(pPtr))
      return;
    m_pCurrent->bound -= 1;
    pPtr = 0;
  }

  /// isIn - whether the pPtr is in the current chunk?
  bool isIn(pointer pPtr) const {
    if (pPtr >= &(m_pCurrent->data[0]) &&
        pPtr <= &(m_pCurrent->data[chunk_type::size() - 1]))
      return true;
    return false;
  }

  /// isIn - whether the pPtr is allocated, and can be constructed.
  bool isAvailable(pointer pPtr) const {
    if (pPtr >= &(m_pCurrent->data[m_pCurrent->bound]) &&
        pPtr <= &(m_pCurrent->data[chunk_type::size() - 1]))
      return true;
    return false;
  }

  void reset() {
    m_pRoot = 0;
    m_pCurrent = 0;
    m_AllocatedNum = 0;
  }

  /// clear - clear all chunks
  void clear() {
    chunk_type* cur = m_pRoot, *prev;
    while (cur != 0) {
      prev = cur;
      cur = cur->next;
      for (unsigned int idx = 0; idx != prev->bound; ++idx)
        destroy(prev->data + idx);
      delete prev;
    }
    reset();
  }

  // -----  observers  ----- //
  bool empty() const { return (m_pRoot == 0); }

  size_type max_size() const { return m_AllocatedNum; }

 protected:
  inline void initialize() {
    m_pRoot = new chunk_type();
    m_pCurrent = m_pRoot;
    m_AllocatedNum += chunk_type::size();
  }

  inline chunk_type* getNewChunk() {
    chunk_type* result = new chunk_type();
    m_pCurrent->next = result;
    m_pCurrent = result;
    m_AllocatedNum += chunk_type::size();
    return result;
  }

 protected:
  chunk_type* m_pRoot;
  chunk_type* m_pCurrent;
  size_type m_AllocatedNum;

 private:
  DISALLOW_COPY_AND_ASSIGN(LinearAllocatorBase);
};

/** \class LinearAllocator
 *  \brief LinearAllocator is another bump pointer allocator which should be
 *  limited in use of two-phase memory allocation.
 *
 *  Two-phase memory allocation clear separates the use of memory into 'claim'
 *  and 'release' phases. There are no interleaving allocation and
 *  deallocation. Interleaving 'allocate' and 'deallocate' increases the size
 *  of allocated memory, and causes bad locality.
 *
 *  The underlying concept of LinearAllocator is a memory pool. LinearAllocator
 *  is a simple implementation of boost::pool's ordered_malloc() and
 *  ordered_free().
 *
 *  template argument DataType is the DataType to be allocated
 *  template argument ChunkSize is the number of bytes of a chunk
 */
template <typename DataType, size_t ChunkSize>
class LinearAllocator
    : public LinearAllocatorBase<Chunk<DataType, ChunkSize> > {
 public:
  template <typename NewDataType>
  struct rebind {
    typedef LinearAllocator<NewDataType, ChunkSize> other;
  };

 public:
  LinearAllocator() : LinearAllocatorBase<Chunk<DataType, ChunkSize> >() {}

  virtual ~LinearAllocator() {}
};

template <typename DataType>
class LinearAllocator<DataType, 0>
    : public LinearAllocatorBase<Chunk<DataType, 0> > {
 public:
  template <typename NewDataType>
  struct rebind {
    typedef LinearAllocator<NewDataType, 0> other;
  };

 public:
  explicit LinearAllocator(size_t pNum)
      : LinearAllocatorBase<Chunk<DataType, 0> >() {
    Chunk<DataType, 0>::setSize(pNum);
  }

  virtual ~LinearAllocator() {}
};

template <typename DataType>
class MallocAllocator {
 public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef DataType* pointer;
  typedef const DataType* const_pointer;
  typedef DataType& reference;
  typedef const DataType& const_reference;
  typedef DataType value_type;

  template <typename OtherDataType>
  struct rebind {
    typedef MallocAllocator<OtherDataType> other;
  };

 public:
  MallocAllocator() throw() {}

  MallocAllocator(const MallocAllocator&) throw() {}

  ~MallocAllocator() throw() {}

  pointer address(reference X) const { return &X; }

  const_pointer address(const_reference X) const { return &X; }

  pointer allocate(size_type pNumOfElements, const void* = 0) {
    return static_cast<DataType*>(
        std::malloc(pNumOfElements * sizeof(DataType)));
  }

  void deallocate(pointer pObject, size_type) {
    std::free(static_cast<void*>(pObject));
  }

  size_type max_size() const throw() { return size_t(-1) / sizeof(DataType); }

  void construct(pointer pObject, const DataType& pValue) {
    ::new (reinterpret_cast<void*>(pObject)) value_type(pValue);
  }

  void destroy(pointer pObject) { pObject->~DataType(); }
};

template <>
class MallocAllocator<void> {
 public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void* reference;
  typedef const void* const_reference;
  typedef void* value_type;

  template <typename OtherDataType>
  struct rebind {
    typedef MallocAllocator<OtherDataType> other;
  };

 public:
  MallocAllocator() throw() {}

  MallocAllocator(const MallocAllocator&) throw() {}

  ~MallocAllocator() throw() {}

  size_type max_size() const throw() { return size_t(-1) / sizeof(void*); }

  pointer address(reference X) const { return X; }

  const_pointer address(const_reference X) const { return X; }

  template <typename DataType>
  DataType* allocate(size_type pNumOfElements, const void* = 0) {
    return static_cast<DataType*>(
        std::malloc(pNumOfElements * sizeof(DataType)));
  }

  pointer allocate(size_type pNumOfElements, const void* = 0) {
    return std::malloc(pNumOfElements);
  }

  template <typename DataType>
  void deallocate(DataType* pObject, size_type) {
    std::free(static_cast<void*>(pObject));
  }

  void deallocate(pointer pObject, size_type) { std::free(pObject); }

  template <typename DataType>
  void construct(DataType* pObject, const DataType& pValue) { /* do nothing */
  }

  void construct(pointer pObject, const_reference pValue) { /* do nothing */
  }

  template <typename DataType>
  void destroy(DataType* pObject) { /* do nothing */
  }

  void destroy(pointer pObject) { /* do nothing */
  }
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_ALLOCATORS_H_
