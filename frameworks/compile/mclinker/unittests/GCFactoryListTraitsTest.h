//===- GCFactoryListTraitsTest.h ------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_GC_FACTORY_LIST_TRAITS_TEST_H
#define MCLD_GC_FACTORY_LIST_TRAITS_TEST_H

#include <gtest.h>

#include "mcld/Support/GCFactoryListTraits.h"

#include <llvm/ADT/ilist_node.h>

#include "mcld/Support/GCFactory.h"

namespace mcldtest {

/** \class GCFactoryListTraitsTest
 *  \brief
 *
 *  \see GCFactoryListTraits
 */
class GCFactoryListTraitsTest : public ::testing::Test {
 public:
  /** \class GCFactoryListTraitsTest
  *   \brief Node used in the test
  *
  */
  class NodeFactory;

  class Node : public llvm::ilist_node<Node> {
    friend class NodeFactory;

   private:
    unsigned m_Init;
    unsigned m_Value;

   public:
    Node() : m_Init(0), m_Value(0) {}

    Node(unsigned pInit) : m_Init(pInit), m_Value(pInit) {}

    unsigned getInitialValue() const { return m_Init; }

    inline unsigned getValue() const { return m_Value; }

    inline void setValue(unsigned pValue) { m_Value = pValue; }
  };

  class NodeFactory : public mcld::GCFactory<Node, 0> {
   public:
    NodeFactory() : mcld::GCFactory<Node, 0>(16) {}

    Node* produce(unsigned pInit) {
      Node* result = allocate();
      new (result) Node(pInit);
      return result;
    }
  };

  // Constructor can do set-up work for all test here.
  GCFactoryListTraitsTest();

  // Destructor can do clean-up work that doesn't throw exceptions here.
  virtual ~GCFactoryListTraitsTest();

  // SetUp() will be called immediately before each test.
  virtual void SetUp();

  // TearDown() will be called immediately after each test.
  virtual void TearDown();

  const llvm::iplist<Node, mcld::GCFactoryListTraits<Node> >& getNodeList()
      const {
    return m_pNodeList;
  }

  llvm::iplist<Node, mcld::GCFactoryListTraits<Node> >& getNodeList() {
    return m_pNodeList;
  }

 protected:
  NodeFactory m_NodeFactory;
  Node** m_pNodesAlloc;

  llvm::iplist<Node, mcld::GCFactoryListTraits<Node> > m_pNodeList;
};

}  // namespace of mcldtest

#endif
