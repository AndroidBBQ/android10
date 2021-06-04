//===- InputBuilder.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/InputBuilder.h"

#include "mcld/LinkerConfig.h"
#include "mcld/Config/Config.h"
#include "mcld/MC/ContextFactory.h"
#include "mcld/MC/InputFactory.h"
#include "mcld/Support/MemoryAreaFactory.h"
#include "mcld/Support/Path.h"

namespace mcld {

InputBuilder::InputBuilder(const LinkerConfig& pConfig)
    : m_Config(pConfig),
      m_pCurrentTree(NULL),
      m_pMove(NULL),
      m_Root(),
      m_bOwnFactory(true) {
  m_pInputFactory = new InputFactory(MCLD_NUM_OF_INPUTS, pConfig);
  m_pContextFactory = new ContextFactory(MCLD_NUM_OF_INPUTS);
  m_pMemFactory = new MemoryAreaFactory(MCLD_NUM_OF_INPUTS);
}

InputBuilder::InputBuilder(const LinkerConfig& pConfig,
                           InputFactory& pInputFactory,
                           ContextFactory& pContextFactory,
                           MemoryAreaFactory& pMemoryFactory,
                           bool pDelegate)
    : m_Config(pConfig),
      m_pInputFactory(&pInputFactory),
      m_pMemFactory(&pMemoryFactory),
      m_pContextFactory(&pContextFactory),
      m_pCurrentTree(NULL),
      m_pMove(NULL),
      m_Root(),
      m_bOwnFactory(pDelegate) {
}

InputBuilder::~InputBuilder() {
  if (m_bOwnFactory) {
    delete m_pInputFactory;
    delete m_pContextFactory;
    delete m_pMemFactory;
  }
}

Input* InputBuilder::createInput(const std::string& pName,
                                 const sys::fs::Path& pPath,
                                 unsigned int pType,
                                 off_t pFileOffset) {
  return m_pInputFactory->produce(pName, pPath, pType, pFileOffset);
}

InputTree& InputBuilder::enterGroup() {
  assert(m_pCurrentTree != NULL && m_pMove != NULL);

  m_pCurrentTree->enterGroup(m_Root, *m_pMove);
  m_pMove->move(m_Root);
  m_ReturnStack.push(m_Root);
  m_pMove = &InputTree::Downward;

  return *m_pCurrentTree;
}

InputTree& InputBuilder::exitGroup() {
  assert(m_pCurrentTree != NULL && m_pMove != NULL);

  m_Root = m_ReturnStack.top();
  m_ReturnStack.pop();
  m_pMove = &InputTree::Afterward;

  return *m_pCurrentTree;
}

bool InputBuilder::isInGroup() const {
  return !m_ReturnStack.empty();
}

const InputTree& InputBuilder::getCurrentTree() const {
  assert(m_pCurrentTree != NULL && m_pMove != NULL);
  return *m_pCurrentTree;
}

InputTree& InputBuilder::getCurrentTree() {
  assert(m_pCurrentTree != NULL && m_pMove != NULL);
  return *m_pCurrentTree;
}

void InputBuilder::setCurrentTree(InputTree& pInputTree) {
  m_pCurrentTree = &pInputTree;
  m_Root = m_pCurrentTree->root();
  m_pMove = &InputTree::Downward;
}

bool InputBuilder::setContext(Input& pInput, bool pCheck) {
  // The object files in an archive have common path. Every object files in an
  // archive needs a individual context. We identify the object files in an
  // archive by its file offset. Their file offsets are not zero.
  LDContext* context = NULL;
  if (pInput.fileOffset() != 0 || !pCheck) {
    // pInput is an object in an archive file. Produce a new context in this
    // case.
    context = m_pContextFactory->produce();
  } else {
    // Using pInput.path() to avoid from creating context for identical file
    // twice.
    context = m_pContextFactory->produce(pInput.path());
  }

  pInput.setContext(context);
  return true;
}

bool InputBuilder::setMemory(Input& pInput,
                             FileHandle::OpenMode pMode,
                             FileHandle::Permission pPerm) {
  MemoryArea* memory = m_pMemFactory->produce(pInput.path(), pMode, pPerm);
  pInput.setMemArea(memory);
  return true;
}

bool InputBuilder::setMemory(Input& pInput, void* pMemBuffer, size_t pSize) {
  MemoryArea* memory = m_pMemFactory->produce(pMemBuffer, pSize);
  pInput.setMemArea(memory);
  return true;
}

const AttrConstraint& InputBuilder::getConstraint() const {
  return m_Config.attribute().constraint();
}

const AttributeProxy& InputBuilder::getAttributes() const {
  return m_pInputFactory->attr();
}

AttributeProxy& InputBuilder::getAttributes() {
  return m_pInputFactory->attr();
}

}  // namespace mcld
