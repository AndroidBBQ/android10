//===- TargetOptions.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGETOPTIONS_H_
#define MCLD_TARGETOPTIONS_H_

#include <llvm/ADT/Triple.h>

#include <string>

namespace mcld {

/** \class TargetOptions
 *  \brief TargetOptions collects the options that dependent on a target
 *  backend.
 */
class TargetOptions {
 public:
  enum Endian { Little, Big, Unknown };

 public:
  TargetOptions();

  explicit TargetOptions(const std::string& pTriple);

  ~TargetOptions();

  const llvm::Triple& triple() const { return m_Triple; }

  void setTriple(const std::string& pTriple);

  void setTriple(const llvm::Triple& pTriple);

  const std::string& getArch() const { return m_ArchName; }

  void setArch(const std::string& pArchName);

  const std::string& getTargetCPU() const { return m_TargetCPU; }

  void setTargetCPU(const std::string& pCPU);

  Endian endian() const { return m_Endian; }

  void setEndian(Endian pEndian) { m_Endian = pEndian; }

  bool isLittleEndian() const { return (Little == m_Endian); }
  bool isBigEndian() const { return (Big == m_Endian); }

  unsigned int bitclass() const { return m_BitClass; }

  void setBitClass(unsigned int pBitClass) { m_BitClass = pBitClass; }

  bool is32Bits() const { return (32 == m_BitClass); }
  bool is64Bits() const { return (64 == m_BitClass); }

  // -G, max GP size option
  void setGPSize(unsigned pGPSize) { m_GPSize = pGPSize; }

  unsigned getGPSize() const { return m_GPSize; }

  void setStubGroupSize(unsigned pSize) { m_StubGroupSize = pSize; }

  unsigned getStubGroupSize() const { return m_StubGroupSize; }

  void setFixCA53Erratum835769(bool pEnable = true) {
    m_FixCA53Erratum835769 = pEnable;
  }

  bool fixCA53Erratum835769() const { return m_FixCA53Erratum835769; }

  void setFixCA53Erratum843419(bool pEnable = true) {
    m_FixCA53Erratum843419 = pEnable;
  }

  bool fixCA53Erratum843419() const { return m_FixCA53Erratum843419; }

 private:
  llvm::Triple m_Triple;
  std::string m_ArchName;
  std::string m_TargetCPU;
  Endian m_Endian;
  unsigned int m_BitClass;
  unsigned m_GPSize;  // -G, --gpsize
  unsigned m_StubGroupSize;
  bool m_FixCA53Erratum835769 : 1;
  bool m_FixCA53Erratum843419 : 1;
};

}  // namespace mcld

#endif  // MCLD_TARGETOPTIONS_H_
