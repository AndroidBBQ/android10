//===- InputTree.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/InputTree.h"

namespace mcld {

InputTree::Succeeder InputTree::Afterward;
InputTree::Includer InputTree::Downward;

//===----------------------------------------------------------------------===//
// InputTree
//===----------------------------------------------------------------------===//
InputTree& InputTree::merge(TreeIteratorBase pRoot,
                            const InputTree::Mover& pMover,
                            InputTree& pTree) {
  if (this == &pTree)
    return *this;

  if (!pTree.empty()) {
    pMover.connect(pRoot, pTree.m_Root.node.right);
    BinaryTreeBase<Input>::m_Root.summon(pTree.BinaryTreeBase<Input>::m_Root);
    BinaryTreeBase<Input>::m_Root.delegate(pTree.m_Root);
    pTree.m_Root.node.left = pTree.m_Root.node.right = &pTree.m_Root.node;
  }
  return *this;
}

InputTree& InputTree::enterGroup(TreeIteratorBase pRoot,
                                 const InputTree::Mover& pMover) {
  NodeBase* node = createNode();
  pMover.connect(pRoot, node);
  return *this;
}

InputTree& InputTree::insert(TreeIteratorBase pRoot,
                             const InputTree::Mover& pMover,
                             Input& pInput) {
  BinaryTree<Input>::node_type* node = createNode();
  node->data = &pInput;
  pMover.connect(pRoot, node);
  return *this;
}

//===----------------------------------------------------------------------===//
// non-member functions
//===----------------------------------------------------------------------===//
bool isGroup(const InputTree::iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

bool isGroup(const InputTree::const_iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

bool isGroup(const InputTree::dfs_iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

bool isGroup(const InputTree::const_dfs_iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

bool isGroup(const InputTree::bfs_iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

bool isGroup(const InputTree::const_bfs_iterator& pos) {
  return !pos.hasData() && !pos.isRoot();
}

}  // namespace mcld
