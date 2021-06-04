//===- ELF.h --------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_ELF_H_
#define MCLD_SUPPORT_ELF_H_

namespace mcld {
namespace ELF {

// Section flags
enum SHF {
  // Indicates this section requires ordering in relation to
  // other sections of the same type.
  SHF_ORDERED = 0x40000000,

  // Section with data that is GP relative addressable.
  SHF_MIPS_GPREL = 0x10000000
};  // enum SHF

}  // namespace ELF
}  // namespace mcld

#endif  // MCLD_SUPPORT_ELF_H_
