//===- MipsAbiFlags.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSABIFLAGS_H_
#define TARGET_MIPS_MIPSABIFLAGS_H_

#include "mcld/Support/MemoryRegion.h"

#include <llvm/ADT/Optional.h>
#include <llvm/Object/ELFTypes.h>

namespace mcld {

class Input;
class LDSection;

/** \class MipsAbiFlags
 *  \brief Representation of .MIPS.abiflags section.
 */
class MipsAbiFlags {
 public:
  /// size of underlaid ELF section structure
  static uint64_t size();

  /// write ELF section structure to the memory region
  static uint64_t emit(const MipsAbiFlags& pInfo, MemoryRegion& pRegion);

  /// fill the structure by the data from the input section
  static bool fillBySection(const Input& pInput, const LDSection& pSection,
                            MipsAbiFlags& mipsAbi);

  /// fill the structure by the data from ELF header flags
  static bool fillByElfFlags(const Input& pInput, uint64_t elfFlags,
                             MipsAbiFlags& mipsAbi);

  /// check compatibility between two structures
  static bool isCompatible(const Input& pInput, const MipsAbiFlags& elf,
                           const MipsAbiFlags& abi);

  /// merge new abi settings to the old structure
  static bool merge(const Input& pInput, MipsAbiFlags& oldFlags,
                    const MipsAbiFlags& newFlags);

 private:
  uint32_t m_IsaLevel;
  uint32_t m_IsaRev;
  uint32_t m_IsaExt;
  uint32_t m_GprSize;
  uint32_t m_Cpr1Size;
  uint32_t m_Cpr2Size;
  uint32_t m_FpAbi;
  uint32_t m_Ases;
  uint32_t m_Flags1;
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSABIFLAGS_H_
