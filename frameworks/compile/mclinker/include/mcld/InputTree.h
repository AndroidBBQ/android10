//===- InputTree.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_INPUTTREE_H_
#define MCLD_INPUTTREE_H_

#include "mcld/ADT/BinTree.h"
#include "mcld/ADT/TypeTraits.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/Path.h"

#include <string>

namespace mcld {

/** \class template<typename Traits, typename Iterator>
 * PolicyIterator<mcld::Input>
 *  \brief PolicyIterator<mcld::Input> is a partially specific PolicyIterator
 */
template <typename Traits, typename IteratorType>
class PolicyIterator<mcld::Input, Traits, IteratorType>
    : public PolicyIteratorBase<Input, Traits, IteratorType> {
 public:
  typedef PolicyIterator<Input, Traits, IteratorType> Self;
  typedef PolicyIteratorBase<Input, Traits, IteratorType> Base;
  typedef PolicyIterator<Input, typename Traits::nonconst_traits, IteratorType>
      iterator;
  typedef PolicyIterator<Input, typename Traits::const_traits, IteratorType>
      const_iterator;

 public:
  PolicyIterator() : Base() {}

  PolicyIterator(const iterator& X) : Base(X.m_pNode) {}

  explicit PolicyIterator(NodeBase* X) : Base(X) {}

  virtual ~PolicyIterator() {}

  bool isGroup() const { return !Base::hasData() && !Base::isRoot(); }

  Self& operator++() {
    IteratorType::advance();
    // skip the Group node
    while (isGroup())
      IteratorType::advance();
    return *this;
  }

  Self operator++(int) {
    Self tmp(*this);
    IteratorType::advance();
    // skip the Group node
    while (isGroup())
      IteratorType::advance();
    return tmp;
  }
};

template <>
class BinaryTree<Input> : public BinaryTreeBase<Input> {
 public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef Input value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef const value_type* const_pointer;
  typedef const value_type& const_reference;

  typedef BinaryTree<Input> Self;
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
  BinaryTree() : BinaryTreeBase<Input>() {}

  ~BinaryTree() {}

  // -----  iterators  ----- //
  bfs_iterator bfs_begin() {
    bfs_iterator it = bfs_iterator(BinaryTreeBase<Input>::m_Root.node.left);
    if (it.isGroup())
      ++it;
    return it;
  }

  bfs_iterator bfs_end() {
    return bfs_iterator(BinaryTreeBase<Input>::m_Root.node.right);
  }

  const_bfs_iterator bfs_begin() const {
    const_bfs_iterator it =
        const_bfs_iterator(BinaryTreeBase<Input>::m_Root.node.left);
    if (it.isGroup())
      ++it;
    return it;
  }

  const_bfs_iterator bfs_end() const {
    return const_bfs_iterator(BinaryTreeBase<Input>::m_Root.node.right);
  }

  dfs_iterator dfs_begin() {
    dfs_iterator it = dfs_iterator(BinaryTreeBase<Input>::m_Root.node.left);
    if (it.isGroup())
      ++it;
    return it;
  }

  dfs_iterator dfs_end() {
    return dfs_iterator(BinaryTreeBase<Input>::m_Root.node.right);
  }

  const_dfs_iterator dfs_begin() const {
    const_dfs_iterator it =
        const_dfs_iterator(BinaryTreeBase<Input>::m_Root.node.left);
    if (it.isGroup())
      ++it;
    return it;
  }

  const_dfs_iterator dfs_end() const {
    return const_dfs_iterator(BinaryTreeBase<Input>::m_Root.node.right);
  }

  iterator root() { return iterator(&(BinaryTreeBase<Input>::m_Root.node)); }

  const_iterator root() const {
    // FIXME: provide the iterater constructors for constant NodeBase instead of
    // using const_cast
    return const_iterator(
        const_cast<NodeBase*>(&BinaryTreeBase<Input>::m_Root.node));
  }

  iterator begin() {
    iterator it = iterator(BinaryTreeBase<Input>::m_Root.node.left);
    return it;
  }

  iterator end() { return iterator(BinaryTreeBase<Input>::m_Root.node.right); }

  const_iterator begin() const {
    return const_iterator(BinaryTreeBase<Input>::m_Root.node.left);
  }

  const_iterator end() const {
    return const_iterator(BinaryTreeBase<Input>::m_Root.node.right);
  }

  // ----- modifiers  ----- //
  /// join - create a leaf node and merge it in the tree.
  //  This version of join determines the direction on compilation time.
  //  @param DIRECT the direction of the connecting edge of the parent node.
  //  @param position the parent node
  //  @param value the value being pushed.
  template <size_t DIRECT>
  BinaryTree& join(TreeIteratorBase& pPosition, const Input& value) {
    node_type* node = BinaryTreeBase<Input>::createNode();
    node->data = const_cast<Input*>(&value);

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
      BinaryTreeBase<Input>::m_Root.summon(pTree.BinaryTreeBase<Input>::m_Root);
      BinaryTreeBase<Input>::m_Root.delegate(pTree.m_Root);
      pTree.m_Root.node.left = pTree.m_Root.node.right = &pTree.m_Root.node;
    }
    return *this;
  }
};

/** \class InputTree
 *  \brief InputTree is the input tree to contains all inputs from the
 *  command line.
 *
 *  InputTree, of course, is uncopyable.
 *
 *  @see Input
 */
class InputTree : public BinaryTree<Input> {
 private:
  typedef BinaryTree<Input> BinTreeTy;

 public:
  enum Direction {
    Inclusive = TreeIteratorBase::Leftward,
    Positional = TreeIteratorBase::Rightward
  };

  typedef BinaryTree<Input>::iterator iterator;
  typedef BinaryTree<Input>::const_iterator const_iterator;

 public:
  /** \class Mover
   *  \brief Mover provides the interface for moving iterator forward.
   *
   *  Mover is a function object (functor). @ref Mover::move moves
   *  iterator forward in certain direction. @ref Mover::connect
   *  connects two nodes of the given iterators togather.
   */
  struct Mover {
    virtual void connect(TreeIteratorBase& pFrom, NodeBase* pTo) const = 0;
    virtual void move(TreeIteratorBase& pNode) const = 0;
    virtual ~Mover() {}
  };

  /** \class Succeeder
   *  \brief class Succeeder moves the iterator afterward.
   */
  struct Succeeder : public Mover {
    void connect(TreeIteratorBase& pFrom, NodeBase* pTo) const {
      pFrom.hook<Positional>(pTo);
    }

    void move(TreeIteratorBase& pNode) const { pNode.move<Positional>(); }
  };

  /** \class Includer
   *  \brief class Includer moves the iterator downward.
   */
  struct Includer : public Mover {
    void connect(TreeIteratorBase& pFrom, NodeBase* pTo) const {
      pFrom.hook<Inclusive>(pTo);
    }

    void move(TreeIteratorBase& pNode) const { pNode.move<Inclusive>(); }
  };

 public:
  static Succeeder Afterward;
  static Includer Downward;

 public:
  using BinTreeTy::merge;

  // -----  modify  ----- //
  template <size_t DIRECT>
  InputTree& enterGroup(TreeIteratorBase pRoot);

  template <size_t DIRECT>
  InputTree& insert(TreeIteratorBase pRoot, Input& pInput);

  InputTree& merge(TreeIteratorBase pRoot,
                   const Mover& pMover,
                   InputTree& pTree);

  InputTree& insert(TreeIteratorBase pRoot, const Mover& pMover, Input& pInput);

  InputTree& enterGroup(TreeIteratorBase pRoot, const Mover& pMover);
};

bool isGroup(const InputTree::iterator& pos);
bool isGroup(const InputTree::const_iterator& pos);
bool isGroup(const InputTree::dfs_iterator& pos);
bool isGroup(const InputTree::const_dfs_iterator& pos);
bool isGroup(const InputTree::bfs_iterator& pos);
bool isGroup(const InputTree::const_bfs_iterator& pos);

}  // namespace mcld

//===----------------------------------------------------------------------===//
// template member functions
//===----------------------------------------------------------------------===//
template <size_t DIRECT>
mcld::InputTree& mcld::InputTree::enterGroup(mcld::TreeIteratorBase pRoot) {
  BinTreeTy::node_type* node = createNode();

  if (pRoot.isRoot())
    pRoot.hook<TreeIteratorBase::Leftward>(node);
  else
    pRoot.hook<DIRECT>(node);

  return *this;
}

template <size_t DIRECT>
mcld::InputTree& mcld::InputTree::insert(mcld::TreeIteratorBase pRoot,
                                         mcld::Input& pInput) {
  BinTreeTy::node_type* node = createNode();
  node->data = &pInput;

  if (pRoot.isRoot())
    pRoot.hook<TreeIteratorBase::Leftward>(node);
  else
    pRoot.hook<DIRECT>(node);

  return *this;
}

#endif  // MCLD_INPUTTREE_H_
