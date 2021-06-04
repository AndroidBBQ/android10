//===- InputToken.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/InputToken.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// InputToken
//===----------------------------------------------------------------------===//
InputToken::InputToken() : m_Type(Unknown), m_bAsNeeded(false) {
}

InputToken::InputToken(Type pType, const std::string& pName, bool pAsNeeded)
    : StrToken(StrToken::Input, pName), m_Type(pType), m_bAsNeeded(pAsNeeded) {
}

InputToken::~InputToken() {
}

}  // namespace mcld
