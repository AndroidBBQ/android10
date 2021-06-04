//===- TreeBase.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_TREEBASE_H_
#define MCLD_ADT_TREEBASE_H_

#include "mcld/ADT/TypeTraits.h"

#include <cassert>
#include <cstddef>
#include <iterator>

namespace mcld {

class NodeBase {
 public:
  NodeBase* left;
  NodeBase* right;

 public:
  NodeBase() : left(NULL), right(NULL) {}
};

class TreeIteratorBase {
 public:
  enum Direct { Leftward, Rightward };

  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;

 public:
  NodeBase* m_pNode;

 public:
  TreeIteratorBase() : m_pNode(NULL) {}

  explicit TreeIteratorBase(NodeBase* X) : m_pNode(X) {}

  virtual ~TreeIteratorBase(){};

  template <size_t DIRECT>
  void move() {
    assert(0 && "not allowed");
  }

  template <size_t DIRECT>
  void hook(NodeBase* pNode) {
    assert(0 && "not allowed");
  }

  bool isRoot() const { return (m_pNode->right == m_pNode); }

  bool hasRightChild() const {
    return ((m_pNode->right) != (m_pNode->right->right));
  }

  bool hasLeftChild() const {
    return ((m_pNode->left) != (m_pNode->left->right));
  }

  bool operator==(const TreeIteratorBase& y) const {
    return this->m_pNode == y.m_pNode;
  }

  bool operator!=(const TreeIteratorBase& y) const {
    return this->m_pNode != y.m_pNode;
  }
};

template <>
inline void TreeIteratorBase::move<TreeIteratorBase::Leftward>() {
  this->m_pNode = this->m_pNode->left;
}

template <>
inline void TreeIteratorBase::move<TreeIteratorBase::Rightward>() {
  this->m_pNode = this->m_pNode->right;
}

template <>
inline void TreeIteratorBase::hook<TreeIteratorBase::Leftward>(
    NodeBase* pOther) {
  this->m_pNode->left = pOther;
}

template <>
inline void TreeIteratorBase::hook<TreeIteratorBase::Rightward>(
    NodeBase* pOther) {
  this->m_pNode->right = pOther;
}

template <typename DataType>
class Node : public NodeBase {
 public:
  typedef DataType value_type;

 public:
  value_type* data;

 public:
  Node() : NodeBase(), data(NULL) {}

  explicit Node(const value_type& pValue) : NodeBase(), data(&pValue) {}
};

}  // namespace mcld

#endif  // MCLD_ADT_TREEBASE_H_
