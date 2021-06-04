//===- ELFAttributeValue.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "mcld/Target/ELFAttributeValue.h"

#include <llvm/Support/ErrorHandling.h>

#include "mcld/Support/LEB128.h"

namespace mcld {

size_t ELFAttributeValue::getSize() const {
  size_t size = 0;

  if (isIntValue())
    size += leb128::size<uint32_t>(m_IntValue);

  if (isStringValue())
    size += m_StringValue.length() + 1 /* for NULL-terminator */;

  if (size <= 0)
    // unknown type of the value
    llvm::report_fatal_error("Unknown type of attribtue value!");

  return size;
}

bool ELFAttributeValue::isDefaultValue() const {
  if (isUninitialized()) {
    // Uninitialized attribute means default value
    return true;
  } else {
    if (isIntValue() && (m_IntValue != 0))
      return false;
    if (isStringValue() && !m_StringValue.empty())
      return false;

    // The value hold in the storage is the "default value by default" (i.e., 0
    // for int type, empty string for string type), but we need to check whether
    // the "default value" is defined (that is, hasNoDefault() = false.)
    return !hasNoDefault();
  }
  // unreachable
}

bool ELFAttributeValue::equals(const ELFAttributeValue& pValue) const {
  if ((pValue.type() != m_Type) || isUninitialized())
    return false;

  if (isIntValue() && (m_IntValue != pValue.getIntValue()))
    return false;

  if (isStringValue() && (m_StringValue != pValue.getStringValue()))
    return false;

  return true;
}

}  // namespace mcld
