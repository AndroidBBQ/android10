//===- MCLDAttribute.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/Attribute.h"

#include "mcld/MC/AttributeSet.h"
#include "mcld/Support/MsgHandling.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// AttrConstraint
//===----------------------------------------------------------------------===//
bool AttrConstraint::isLegal(const Attribute& pAttr) const {
  if (!isWholeArchive() && pAttr.isWholeArchive()) {
    error(diag::err_unsupported_whole_archive);
    return false;
  }
  if (!isAsNeeded() && pAttr.isAsNeeded()) {
    error(diag::err_unsupported_as_needed);
    return false;
  }
  if (!isAddNeeded() && pAttr.isAddNeeded()) {
    error(diag::err_unsupported_add_needed);
    return false;
  }
  if (isStaticSystem() && pAttr.isDynamic()) {
    error(diag::err_unsupported_Bdynamic);
    return false;
  }
  if (isStaticSystem() && pAttr.isAsNeeded()) {
    warning(diag::err_enable_as_needed_on_static_system);
    return true;
  }
  // FIXME: may be it's legal, but ignored by GNU ld.
  if (pAttr.isAsNeeded() && pAttr.isStatic()) {
    warning(diag::err_mix_static_as_needed);
    return true;
  }
  return true;
}

//===----------------------------------------------------------------------===//
// AttributeProxy
//===----------------------------------------------------------------------===//
AttributeProxy::AttributeProxy(AttributeSet& pParent,
                               const Attribute& pBase,
                               const AttrConstraint& pConstraint)
    : m_AttrPool(pParent), m_pBase(&pBase), m_Constraint(pConstraint) {
}

AttributeProxy::~AttributeProxy() {
}

bool AttributeProxy::isWholeArchive() const {
  if (m_Constraint.isWholeArchive())
    return m_pBase->isWholeArchive();
  else
    return false;
}

bool AttributeProxy::isAsNeeded() const {
  if (m_Constraint.isAsNeeded())
    return m_pBase->isAsNeeded();
  else
    return false;
}

bool AttributeProxy::isAddNeeded() const {
  if (m_Constraint.isAddNeeded())
    return m_pBase->isAddNeeded();
  else
    return false;
}

bool AttributeProxy::isStatic() const {
  if (m_Constraint.isSharedSystem())
    return m_pBase->isStatic();
  else
    return true;
}

bool AttributeProxy::isDynamic() const {
  if (m_Constraint.isSharedSystem())
    return m_pBase->isDynamic();
  else
    return false;
}

static inline void ReplaceOrRecord(AttributeSet& pParent,
                                   const Attribute*& pBase,
                                   Attribute*& pCopy) {
  Attribute* result = pParent.exists(*pCopy);
  if (result == NULL) {  // can not find
    pParent.record(*pCopy);
    pBase = pCopy;
  } else {  // find
    delete pCopy;
    pBase = result;
  }
}

void AttributeProxy::setWholeArchive() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->setWholeArchive();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::unsetWholeArchive() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->unsetWholeArchive();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::setAsNeeded() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->setAsNeeded();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::unsetAsNeeded() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->unsetAsNeeded();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::setAddNeeded() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->setAddNeeded();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::unsetAddNeeded() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->unsetAddNeeded();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::setStatic() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->setStatic();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

void AttributeProxy::setDynamic() {
  Attribute* copy = new Attribute(*m_pBase);
  copy->setDynamic();
  ReplaceOrRecord(m_AttrPool, m_pBase, copy);
}

AttributeProxy& AttributeProxy::assign(Attribute* pBase) {
  m_pBase = pBase;
  return *this;
}

}  // namespace mcld
