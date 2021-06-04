//===- BinTree.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_BINTREE_H_
#define MCLD_ADT_BINTREE_H_

#include "mcld/ADT/TreeAllocator.h"
#include "mcld/ADT/TreeBase.h"
#include "mcld/Support/Compiler.h"

#include <cstddef>
#include <iterator>
#include <memory>
#include <queue>
#include <stack>

namespace mcld {

template <class DataType>
class BinaryTree;

class DFSIterator : public TreeIteratorBase {
 public:
  DFSIterator() : TreeIteratorBase() {}

  explicit DFSIterator(NodeBase* X) : TreeIteratorBase(X) {
    if (hasRightChild())
      m_Stack.push(m_pNode->right);
    if (hasLeftChild())
      m_Stack.push(m_pNode->left);
  }

  virtual ~DFSIterator() {}

  void advance() {
    if (m_Stack.empty()) {       // reach the end
      m_pNode = m_pNode->right;  // should be root
      return;
    }
    m_pNode = m_Stack.top();
    m_Stack.pop();
    if (hasRightChild())
      m_Stack.push(m_pNode->right);
    if (hasLeftChild())
      m_Stack.push(m_pNode->left);
  }

 private:
  std::stack<NodeBase*> m_Stack;
};

class BFSIterator : public TreeIteratorBase {
 public:
  BFSIterator() : TreeIteratorBase() {}

  explicit BFSIterator(NodeBase* X) : TreeIteratorBase(X) {
    if (hasRightChild())
      m_Queue.push(m_pNode->right);
    if (hasLeftChild())
      m_Queue.push(m_pNode->left);
  }

  virtual ~BFSIterator() {}

  void advance() {
    if (m_Queue.empty()) {       // reach the end
      m_pNode = m_pNode->right;  // should be root
      return;
    }
    m_pNode = m_Queue.front();
    m_Queue.pop();
    if (hasRightChild())
      m_Queue.push(m_pNode->right);
    if (hasLeftChild())
      m_Queue.push(m_pNode->left);
  }

 private:
  std::queue<NodeBase*> m_Queue;
};

template <class DataType, class Traits, class IteratorType>
class PolicyIteratorBase : public IteratorType {
 public:
  typedef DataType value_type;
  typedef Traits traits;
  typedef typename traits::pointer pointer;
  typedef typename traits::reference reference;

  typedef PolicyIteratorBase<value_type, Traits, IteratorType> Self;
  typedef Node<value_type> node_type;

  typedef typename traits::nonconst_traits nonconst_traits;
  typedef typename traits::const_traits const_traits;

  typedef PolicyIteratorBase<value_type, nonconst_traits, IteratorType>
      iterator;
  typedef PolicyIteratorBase<value_type, const_traits, IteratorType>
      const_iterator;

  typedef std::forward_iterator_tag iterator_category;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 public:
  PolicyIteratorBase() : IteratorType() {}

  PolicyIteratorBase(const iterator& X) : IteratorType(X.m_pNode) {}

  explicit PolicyIteratorBase(NodeBase* X) : IteratorType(X) {}

  virtual ~PolicyIteratorBase() {}

  // -----  operators  ----- //
  pointer operator*() const {
    return static_cast<node_type*>(IteratorType::m_pNode)->data;
  }

  reference operator->() const {
    return *static_cast<node_type*>(IteratorType::m_pNode)->data;
  }

  bool hasData() const {
    return (!IteratorType::isRoot() &&
            (0 != static_cast<node_type*>(IteratorType::m_pNode)->data));
  }
};

template <class DataType, class Traits, class IteratorType>
class PolicyIterator
    : public PolicyIteratorBase<DataType, Traits, IteratorType> {
 public:
  typedef PolicyIterator<DataType, Traits, IteratorType> Self;
  typedef PolicyIteratorBase<DataType, Traits, IteratorType> Base;
  typedef PolicyIterator<DataType,
                         typename Traits::nonconst_traits,
                         IteratorType> iterator;
  typedef PolicyIterator<DataType, typename Traits::const_traits, IteratorType>
      const_iterator;

 public:
  PolicyIterator() : Base() {}

  PolicyIterator(const iterator& X) : Base(X.m_pNode) {}

  explicit PolicyIterator(NodeBase* X) : Base(X) {}

  virtual ~PolicyIterator() {}

  Self& operator++() {
    IteratorType::advance();
    return *this;
  }

  Self operator++(int) {
    Self tmp = *this;
    IteratorType::advance();
    return tmp;
  }
};

template <class DataType>
class BinaryTree;

/** \class TreeIterator
 *  \brief TreeIterator provides full functions of binary tree's iterator.
 *
 *  TreeIterator is designed to compatible with STL iterators.
 *  TreeIterator is bi-directional. Incremental direction means to move
 *  rightward, and decremental direction is leftward.
 *
 *  @see TreeIteratorBase
 */
template <class DataType, class Traits>
struct TreeIterator : public TreeIteratorBase {
 public:
  typedef DataType value_type;
  typedef Traits traits;
  typedef typename traits::pointer pointer;
  typedef typename traits::reference reference;

  typedef TreeIterator<value_type, Traits> Self;
  typedef Node<value_type> node_type;

  typedef typename traits::nonconst_traits nonconst_traits;
  typedef TreeIterator<value_type, nonconst_traits> iterator;
  typedef typename traits::const_traits const_traits;
  typedef TreeIterator<value_type, const_traits> const_iterator;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 public:
  TreeIterator() : TreeIteratorBase() {}

  TreeIterator(const iterator& X) : TreeIteratorBase(X.m_pNode) {}

  ~TreeIterator() {}

  // -----  operators  ----- //
  pointer operator*() const { return static_cast<node_type*>(m_pNode)->data; }

  reference operator->() const {
    return *static_cast<node_type*>(m_pNode)->data;
  }

  bool isRoot() const { return (m_pNode->right == m_pNode); }

  bool hasData() const {
    return (!isRoot() && (0 != static_cast<node_type*>(m_pNode)->data));
  }

  Self& operator++() {
    this->move<TreeIteratorBase::Rightward>();
    return *this;
  }

  Self operator++(int) {
    Self tmp = *this;
    this->move<TreeIteratorBase::Rightward>();
    return tmp;
  }

  Self& operator--() {
    this->move<TreeIteratorBase::Leftward>();
    return *this;
  }

  Self operator--(int) {
    Self tmp = *this;
    this->move<TreeIteratorBase::Leftward>();
    return tmp;
  }

  explicit TreeIterator(NodeBase* X) : TreeIteratorBase(X) {}
};

/** \class BinaryTreeBase
 *  \brief BinaryTreeBase gives root node and memory management.
 *
 *  The memory management of nodes in is hidden by BinaryTreeBase.
 *  BinaryTreeBase also provides the basic functions for merging a tree and
 *  inserton of a node.
 *
 *  @see BinaryTree
 */
template <class DataType>
class BinaryTreeBase {
 public:
  typedef Node<DataType> NodeType;

 protected:
  /// TreeImpl - TreeImpl records the root node and the number of nodes
  //
  //    +---> Root(end) <---+
  //    |        |left      |
  //    |      begin        |
  //    |     /     \       |
  //    |  Left     Right   |
  //    +---/         \-----+
  //
  class TreeImpl : public NodeFactory<DataType> {
    typedef typename NodeFactory<DataType>::iterator iterator;
    typedef typename NodeFactory<DataType>::const_iterator const_iterator;

   public:
    NodeBase node;

   public:
    TreeImpl() : NodeFactory<DataType>() { node.left = node.right = &node; }

    ~TreeImpl() {}

    /// summon - change the final edges of pClient to our root
    void summon(TreeImpl& pClient) {
      if (this == &pClient)
        return;

      iterator data;
      iterator dEnd = pClient.end();
      for (data = pClient.begin(); data != dEnd; ++data) {
        if ((*data).left == &pClient.node)
          (*data).left = &node;
        if ((*data).right == &pClient.node)
          (*data).right = &node;
      }
    }
  };  // TreeImpl

 protected:
  /// m_Root is a special object who responses:
  //  - the pointer of root
  //  - the simple factory of nodes.
  TreeImpl m_Root;

 protected:
  NodeType* createNode() {
    NodeType* result = m_Root.produce();
    result->left = result->right = &m_Root.node;
    return result;
  }

  void destroyNode(NodeType* pNode) {
    pNode->left = pNode->right = 0;
    pNode->data = 0;
    m_Root.deallocate(pNode);
  }

 public:
  BinaryTreeBase() : m_Root() {}

  virtual ~BinaryTreeBase() {}

  size_t size() const { return m_Root.size(); }

  bool empty() const { return m_Root.empty(); }

 protected:
  void clear() { m_Root.clear(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(BinaryTreeBase);
};

/** \class BinaryTree
 *  \brief An abstract data type of binary tree.
 *
 *  @see mcld::InputTree
 */
template <class DataType>
class BinaryTree : public BinaryTreeBase<DataType> {
 public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef DataType value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef const value_type* const_pointer;
  typedef const value_type& const_reference;

  typedef BinaryTree<DataType> Self;
  typedef TreeIterator<value_type, NonConstTraits<value_type> > iterator;
  typedef TreeIterator<value_type, ConstTraits<value_type> > const_iterator;

  typedef PolicyIterator<value_type, NonConstTraits<value_type>, DFSIterator>
      dfs_iterator;
  typedef PolicyIterator<value_type, ConstTraits<value_type>, DFSIterator>
      const_dfs_iterator;

  typedef PolicyIterator<value_type, NonConstTraits<value_type>, BFSIterator>
      bfs_iterator;
  typedef PolicyIterator<value_type, ConstTraits<value_type>, BFSIterator>
      const_bfs_iterator;

 protected:
  typedef Node<value_type> node_type;

 public:
  // -----  constructors and destructor  ----- //
  BinaryTree() : BinaryTreeBase<DataType>() {}

  ~BinaryTree() {}

  // -----  iterators  ----- //
  bfs_iterator bfs_begin() {
    return bfs_iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  bfs_iterator bfs_end() {
    return bfs_iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  const_bfs_iterator bfs_begin() const {
    return const_bfs_iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  const_bfs_iterator bfs_end() const {
    return const_bfs_iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  dfs_iterator dfs_begin() {
    return dfs_iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  dfs_iterator dfs_end() {
    return dfs_iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  const_dfs_iterator dfs_begin() const {
    return const_dfs_iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  const_dfs_iterator dfs_end() const {
    return const_dfs_iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  iterator root() { return iterator(&(BinaryTreeBase<DataType>::m_Root.node)); }

  const_iterator root() const {
    return const_iterator(&(BinaryTreeBase<DataType>::m_Root.node));
  }

  iterator begin() {
    return iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  iterator end() {
    return iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  const_iterator begin() const {
    return const_iterator(BinaryTreeBase<DataType>::m_Root.node.left);
  }

  const_iterator end() const {
    return const_iterator(BinaryTreeBase<DataType>::m_Root.node.right);
  }

  // ----- modifiers  ----- //
  /// join - create a leaf node and merge it in the tree.
  //  This version of join determines the direction on compilation time.
  //  @param DIRECT the direction of the connecting edge of the parent node.
  //  @param position the parent node
  //  @param value the value being pushed.
  template <size_t DIRECT>
  BinaryTree& join(TreeIteratorBase& pPosition, const DataType& pValue) {
    node_type* node = BinaryTreeBase<DataType>::createNode();
    node->data = const_cast<DataType*>(&pValue);

    if (pPosition.isRoot())
      pPosition.hook<TreeIteratorBase::Leftward>(node);
    else
      pPosition.hook<DIRECT>(node);

    return *this;
  }

  /// merge - merge the tree
  //  @param DIRECT the direction of the connecting edge of the parent node.
  //  @param position the parent node
  //  @param the tree being joined.
  //  @return the joined tree
  template <size_t DIRECT>
  BinaryTree& merge(TreeIteratorBase& pPosition, BinaryTree& pTree) {
    if (this == &pTree)
      return *this;

    if (!pTree.empty()) {
      pPosition.hook<DIRECT>(pTree.m_Root.node.left);
      BinaryTreeBase<DataType>::m_Root.summon(
          pTree.BinaryTreeBase<DataType>::m_Root);
      BinaryTreeBase<DataType>::m_Root.delegate(pTree.m_Root);
      pTree.m_Root.node.left = pTree.m_Root.node.right = &pTree.m_Root.node;
    }
    return *this;
  }
};

}  // namespace mcld

#endif  // MCLD_ADT_BINTREE_H_
