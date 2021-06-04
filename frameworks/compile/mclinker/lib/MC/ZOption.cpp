//===- ZOption.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/ZOption.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// ZOption
//===----------------------------------------------------------------------===//
ZOption::ZOption() : ZOption(Unknown) {
}

ZOption::ZOption(Kind pKind) : ZOption(pKind, 0x0) {
}

ZOption::ZOption(Kind pKind, uint64_t pPageSize)
    : m_Kind(pKind), m_PageSize(pPageSize) {
}

}  // namespace mcld
