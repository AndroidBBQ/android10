//===- GNUInfo.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_GNUINFO_H_
#define MCLD_TARGET_GNUINFO_H_
#include <llvm/ADT/Triple.h>
#include <llvm/Support/ELF.h>

namespace mcld {

/** \class GNUInfo
 *  \brief GNUInfo records ELF-dependent and target-dependnet data fields
 */
class GNUInfo {
 public:
  explicit GNUInfo(const llvm::Triple& pTriple);

  virtual ~GNUInfo() {}

  /// ELFVersion - the value of e_ident[EI_VERSION]
  virtual uint8_t ELFVersion() const { return llvm::ELF::EV_CURRENT; }

  /// The return value of machine() it the same as e_machine in the ELF header
  virtual uint32_t machine() const = 0;

  /// OSABI - the value of e_ident[EI_OSABI]
  uint8_t OSABI() const;

  /// ABIVersion - the value of e_ident[EI_ABIVRESION]
  virtual uint8_t ABIVersion() const { return 0x0; }

  /// defaultTextSegmentAddr - target should specify its own default start
  /// address of the text segment. esp. for exec.
  virtual uint64_t defaultTextSegmentAddr() const { return 0x0; }

  /// flags - the value of ElfXX_Ehdr::e_flags
  virtual uint64_t flags() const = 0;

  /// entry - the symbol name of the entry point
  virtual const char* entry() const { return "_start"; }

  /// dyld - the name of the default dynamic linker
  /// target may override this function if needed.
  virtual const char* dyld() const { return "/usr/lib/libc.so.1"; }

  /// isDefaultExecStack - target should specify whether the stack is default
  /// executable. If target favors another choice, please override this function
  virtual bool isDefaultExecStack() const { return true; }

  /// commonPageSize - the common page size of the target machine, and we set it
  /// to 4K here. If target favors the different size, please override this
  virtual uint64_t commonPageSize() const { return 0x1000; }

  /// abiPageSize - the abi page size of the target machine, and we set it to 4K
  /// here. If target favors the different size, please override this function
  virtual uint64_t abiPageSize() const { return 0x1000; }

  /// stubGroupSize - the default group size to place stubs between sections.
  virtual unsigned stubGroupSize() const { return 0x10000; }

 protected:
  const llvm::Triple& m_Triple;
};

}  // namespace mcld

#endif  // MCLD_TARGET_GNUINFO_H_
