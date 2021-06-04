//===- ResolveInfo.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/ResolveInfo.h"

#include "mcld/LinkerConfig.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Support/GCFactory.h"

#include <llvm/Support/ManagedStatic.h>

#include <cstdlib>
#include <cstring>

namespace mcld {

/// g_NullResolveInfo - a pointer to Null ResolveInfo.
static ResolveInfo* g_NullResolveInfo = NULL;

//===----------------------------------------------------------------------===//
// ResolveInfo
//===----------------------------------------------------------------------===//
ResolveInfo::ResolveInfo() : m_Size(0), m_BitField(0) {
  m_Ptr.sym_ptr = 0;
}

ResolveInfo::~ResolveInfo() {
}

void ResolveInfo::override(const ResolveInfo& pFrom) {
  m_Size = pFrom.m_Size;
  overrideAttributes(pFrom);
  overrideVisibility(pFrom);
}

void ResolveInfo::overrideAttributes(const ResolveInfo& pFrom) {
  m_BitField &= ~RESOLVE_MASK | VISIBILITY_MASK;
  m_BitField |= (pFrom.m_BitField & (RESOLVE_MASK & ~VISIBILITY_MASK));
}

/// overrideVisibility - override the visibility
///   always use the most strict visibility
void ResolveInfo::overrideVisibility(const ResolveInfo& pFrom) {
  // The rule for combining visibility is that we always choose the
  // most constrained visibility.  In order of increasing constraint,
  // visibility goes PROTECTED, HIDDEN, INTERNAL.  This is the reverse
  // of the numeric values, so the effect is that we always want the
  // smallest non-zero value.
  //
  // enum {
  //   STV_DEFAULT = 0,
  //   STV_INTERNAL = 1,
  //   STV_HIDDEN = 2,
  //   STV_PROTECTED = 3
  // };

  Visibility from_vis = pFrom.visibility();
  Visibility cur_vis = visibility();
  if (from_vis != 0) {
    if (cur_vis == 0)
      setVisibility(from_vis);
    else if (cur_vis > from_vis)
      setVisibility(from_vis);
  }
}

void ResolveInfo::setRegular() {
  m_BitField &= (~dynamic_flag);
}

void ResolveInfo::setDynamic() {
  m_BitField |= dynamic_flag;
}

void ResolveInfo::setSource(bool pIsDyn) {
  if (pIsDyn)
    m_BitField |= dynamic_flag;
  else
    m_BitField &= (~dynamic_flag);
}

void ResolveInfo::setInDyn() {
  m_BitField |= indyn_flag;
}

void ResolveInfo::setType(uint32_t pType) {
  m_BitField &= ~TYPE_MASK;
  m_BitField |= ((pType << TYPE_OFFSET) & TYPE_MASK);
}

void ResolveInfo::setDesc(uint32_t pDesc) {
  m_BitField &= ~DESC_MASK;
  m_BitField |= ((pDesc << DESC_OFFSET) & DESC_MASK);
}

void ResolveInfo::setBinding(uint32_t pBinding) {
  m_BitField &= ~BINDING_MASK;
  if (pBinding == Local || pBinding == Absolute)
    m_BitField |= local_flag;
  if (pBinding == Weak || pBinding == Absolute)
    m_BitField |= weak_flag;
}

void ResolveInfo::setReserved(uint32_t pReserved) {
  m_BitField &= ~RESERVED_MASK;
  m_BitField |= ((pReserved << RESERVED_OFFSET) & RESERVED_MASK);
}

void ResolveInfo::setOther(uint32_t pOther) {
  setVisibility(static_cast<ResolveInfo::Visibility>(pOther & 0x3));
}

void ResolveInfo::setVisibility(ResolveInfo::Visibility pVisibility) {
  m_BitField &= ~VISIBILITY_MASK;
  m_BitField |= pVisibility << VISIBILITY_OFFSET;
}

void ResolveInfo::setIsSymbol(bool pIsSymbol) {
  if (pIsSymbol)
    m_BitField |= symbol_flag;
  else
    m_BitField &= ~symbol_flag;
}

bool ResolveInfo::isNull() const {
  return (this == Null());
}

bool ResolveInfo::isDyn() const {
  return (dynamic_flag == (m_BitField & DYN_MASK));
}

bool ResolveInfo::isUndef() const {
  return (undefine_flag == (m_BitField & DESC_MASK));
}

bool ResolveInfo::isDefine() const {
  return (define_flag == (m_BitField & DESC_MASK));
}

bool ResolveInfo::isCommon() const {
  return (common_flag == (m_BitField & DESC_MASK));
}

bool ResolveInfo::isIndirect() const {
  return (indirect_flag == (m_BitField & DESC_MASK));
}

// isGlobal - [L,W] == [0, 0]
bool ResolveInfo::isGlobal() const {
  return (global_flag == (m_BitField & BINDING_MASK));
}

// isWeak - [L,W] == [0, 1]
bool ResolveInfo::isWeak() const {
  return (weak_flag == (m_BitField & BINDING_MASK));
}

// isLocal - [L,W] == [1, 0]
bool ResolveInfo::isLocal() const {
  return (local_flag == (m_BitField & BINDING_MASK));
}

// isAbsolute - [L,W] == [1, 1]
bool ResolveInfo::isAbsolute() const {
  return (absolute_flag == (m_BitField & BINDING_MASK));
}

bool ResolveInfo::isSymbol() const {
  return (symbol_flag == (m_BitField & SYMBOL_MASK));
}

bool ResolveInfo::isString() const {
  return (string_flag == (m_BitField & SYMBOL_MASK));
}

bool ResolveInfo::isInDyn() const {
  return (indyn_flag == (m_BitField & IN_DYN_MASK));
}

uint32_t ResolveInfo::type() const {
  return (m_BitField & TYPE_MASK) >> TYPE_OFFSET;
}

uint32_t ResolveInfo::desc() const {
  return (m_BitField & DESC_MASK) >> DESC_OFFSET;
}

uint32_t ResolveInfo::binding() const {
  if (m_BitField & LOCAL_MASK) {
    if (m_BitField & GLOBAL_MASK) {
      return ResolveInfo::Absolute;
    }
    return ResolveInfo::Local;
  }
  return m_BitField & GLOBAL_MASK;
}

uint32_t ResolveInfo::reserved() const {
  return (m_BitField & RESERVED_MASK) >> RESERVED_OFFSET;
}

ResolveInfo::Visibility ResolveInfo::visibility() const {
  return static_cast<ResolveInfo::Visibility>((m_BitField & VISIBILITY_MASK) >>
                                              VISIBILITY_OFFSET);
}

bool ResolveInfo::compare(const ResolveInfo::key_type& pKey) {
  size_t length = nameSize();
  if (length != pKey.size())
    return false;
  return (std::memcmp(m_Name, pKey.data(), length) == 0);
}

bool ResolveInfo::shouldForceLocal(const LinkerConfig& pConfig) {
  // forced local symbol matches all rules:
  // 1. We are not doing incremental linking.
  // 2. The symbol is with Hidden or Internal visibility.
  // 3. The symbol should be global or weak. Otherwise, local symbol is local.
  // 4. The symbol is defined or common
  if (LinkerConfig::Object != pConfig.codeGenType() &&
      (visibility() == ResolveInfo::Hidden ||
       visibility() == ResolveInfo::Internal) &&
      (isGlobal() || isWeak()) && (isDefine() || isCommon()))
    return true;
  return false;
}
//===----------------------------------------------------------------------===//
// ResolveInfo Factory Methods
//===----------------------------------------------------------------------===//
ResolveInfo* ResolveInfo::Create(const ResolveInfo::key_type& pKey) {
  ResolveInfo* info =
      static_cast<ResolveInfo*>(malloc(sizeof(ResolveInfo) + pKey.size() + 1));
  if (info == NULL)
    return NULL;

  new (info) ResolveInfo();  // call constructor at the `result` address.
  std::memcpy(info->m_Name, pKey.data(), pKey.size());
  info->m_Name[pKey.size()] = '\0';
  info->m_BitField &= ~ResolveInfo::RESOLVE_MASK;
  info->m_BitField |= (pKey.size() << ResolveInfo::NAME_LENGTH_OFFSET);
  return info;
}

void ResolveInfo::Destroy(ResolveInfo*& pInfo) {
  if (pInfo->isNull())
    return;

  if (pInfo != NULL) {
    pInfo->~ResolveInfo();
    free(pInfo);
  }

  pInfo = NULL;
}

ResolveInfo* ResolveInfo::Null() {
  if (g_NullResolveInfo == NULL) {
    g_NullResolveInfo =
        static_cast<ResolveInfo*>(malloc(sizeof(ResolveInfo) + 1));
    new (g_NullResolveInfo) ResolveInfo();
    g_NullResolveInfo->m_Name[0] = '\0';
    g_NullResolveInfo->m_BitField = 0x0;
    g_NullResolveInfo->setBinding(Local);
  }
  return g_NullResolveInfo;
}

}  // namespace mcld
