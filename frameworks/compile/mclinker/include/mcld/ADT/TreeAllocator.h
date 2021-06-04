//===- TreeAllocator.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_TREEALLOCATOR_H_
#define MCLD_ADT_TREEALLOCATOR_H_

#include "mcld/ADT/TreeBase.h"
#include "mcld/Support/GCFactory.h"

#include <set>

namespace mcld {

/** \class NodeFactory
 *  \brief NodeFactory manages the creation and destruction of mcld::Node.
 *
 *  NodeFactory guarantees all allocated memory are released finally. When
 *  the destructor of NodeFactory is called, all allocated memory are freed.
 *
 *  NodeFactory provides delegation of memory. Sometimes, we have to merge two
 *  NodeFactories, and NodeFactory::delegate() can move the memory from one
 *  NodeFactories to another.
 *
 *  @see LinearAllocator
 */
template <typename DataType>
class NodeFactory : public GCFactory<Node<DataType>, 64> {
 private:
  typedef GCFactory<Node<DataType>, 64> Alloc;

 public:
  typedef Node<DataType> NodeType;
  typedef typename Alloc::iterator iterator;
  typedef typename Alloc::const_iterator const_iterator;

 public:
  /// produce - produce a node, add it under control
  NodeType* produce() {
    NodeType* result = Alloc::allocate();
    Alloc::construct(result);
    return result;
  }

  /// delegate - get the control of chunks owned by the client
  //  after calling delegate(), client will renouce its control
  //  of memory space.
  void delegate(NodeFactory& pClient) {
    if (this == &pClient)
      return;

    if (pClient.empty())
      return;

    if (Alloc::empty()) {
      replace(pClient);
      pClient.renounce();
      return;
    }

    // neither me nor client is empty
    concatenate(pClient);
    pClient.renounce();
  }

 private:
  /// renounce - give up the control of all chunks
  void renounce() { Alloc::reset(); }

  /// replace - be the agent of client.
  void replace(NodeFactory& pClient) {
    Alloc::m_pRoot = pClient.Alloc::m_pRoot;
    Alloc::m_pCurrent = pClient.Alloc::m_pCurrent;
    Alloc::m_AllocatedNum = pClient.Alloc::m_AllocatedNum;
    Alloc::m_NumAllocData = pClient.Alloc::m_NumAllocData;
  }

  /// concatenate - conncet two factories
  void concatenate(NodeFactory& pClient) {
    Alloc::m_pCurrent->next = pClient.Alloc::m_pRoot;
    Alloc::m_pCurrent = pClient.Alloc::m_pCurrent;
    Alloc::m_AllocatedNum += pClient.Alloc::m_AllocatedNum;
    Alloc::m_NumAllocData += pClient.Alloc::m_NumAllocData;
  }
};

}  // namespace mcld

#endif  // MCLD_ADT_TREEALLOCATOR_H_
