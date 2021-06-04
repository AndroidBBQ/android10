//===- X86PLT.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_X86_X86PLT_H_
#define TARGET_X86_X86PLT_H_

#include "mcld/Target/PLT.h"

const uint8_t x86_32_dyn_plt0[] = {
    0xff, 0xb3, 0x04, 0, 0, 0,  // pushl  0x4(%ebx)
    0xff, 0xa3, 0x08, 0, 0, 0,  // jmp    *0x8(%ebx)
    0x0f, 0x1f, 0x4,  0         // nopl   0(%eax)
};

const uint8_t x86_32_dyn_plt1[] = {
    0xff, 0xa3, 0, 0, 0, 0,  // jmp    *sym@GOT(%ebx)
    0x68, 0, 0, 0, 0,        // pushl  $offset
    0xe9, 0, 0, 0, 0         // jmp    plt0
};

const uint8_t x86_32_exec_plt0[] = {
    0xff, 0x35, 0, 0, 0, 0,  // pushl  .got + 4
    0xff, 0x25, 0, 0, 0, 0,  // jmp    *(.got + 8)
    0x0f, 0x1f, 0x4, 0       // nopl   0(%eax)
};

const uint8_t x86_32_exec_plt1[] = {
    0xff, 0x25, 0, 0, 0, 0,  // jmp    *(sym in .got)
    0x68, 0, 0, 0, 0,        // pushl  $offset
    0xe9, 0, 0, 0, 0         // jmp    plt0
};

const uint8_t x86_64_plt0[] = {
    0xff, 0x35, 0x8, 0, 0, 0,   // pushq  GOT + 8(%rip)
    0xff, 0x25, 0x16, 0, 0, 0,  // jmq    *GOT + 16(%rip)
    0x0f, 0x1f, 0x40, 0         // nopl   0(%rax)
};

const uint8_t x86_64_plt1[] = {
    0xff, 0x25, 0, 0, 0, 0,  // jmpq   *sym@GOTPCREL(%rip)
    0x68, 0, 0, 0, 0,        // pushq  $index
    0xe9, 0, 0, 0, 0         // jmpq   plt0
};

namespace mcld {

class X86_32GOTPLT;
class GOTEntry;
class LinkerConfig;

//===----------------------------------------------------------------------===//
// X86_32PLT Entry
//===----------------------------------------------------------------------===//
class X86_32DynPLT0 : public PLT::Entry<sizeof(x86_32_dyn_plt0)> {
 public:
  X86_32DynPLT0(SectionData& pParent);
};

class X86_32DynPLT1 : public PLT::Entry<sizeof(x86_32_dyn_plt1)> {
 public:
  X86_32DynPLT1(SectionData& pParent);
};

class X86_32ExecPLT0 : public PLT::Entry<sizeof(x86_32_exec_plt0)> {
 public:
  X86_32ExecPLT0(SectionData& pParent);
};

class X86_32ExecPLT1 : public PLT::Entry<sizeof(x86_32_exec_plt1)> {
 public:
  X86_32ExecPLT1(SectionData& pParent);
};

//===----------------------------------------------------------------------===//
// X86_64PLT Entry
//===----------------------------------------------------------------------===//
class X86_64PLT0 : public PLT::Entry<sizeof(x86_64_plt0)> {
 public:
  X86_64PLT0(SectionData& pParent);
};

class X86_64PLT1 : public PLT::Entry<sizeof(x86_64_plt1)> {
 public:
  X86_64PLT1(SectionData& pParent);
};

//===----------------------------------------------------------------------===//
// X86PLT
//===----------------------------------------------------------------------===//
/** \class X86PLT
 *  \brief X86 Procedure Linkage Table
 */
class X86PLT : public PLT {
 public:
  X86PLT(LDSection& pSection, const LinkerConfig& pConfig, int got_size);
  ~X86PLT();

  // finalizeSectionSize - set LDSection size
  void finalizeSectionSize();

  // hasPLT1 - return if this PLT has any PLT1 entry
  bool hasPLT1() const;

  PLTEntryBase* create();

  virtual void applyPLT0() = 0;

  virtual void applyPLT1() = 0;

  unsigned int getPLT0Size() const { return m_PLT0Size; }
  unsigned int getPLT1Size() const { return m_PLT1Size; }

 protected:
  PLTEntryBase* getPLT0() const;

 protected:
  const uint8_t* m_PLT0;
  const uint8_t* m_PLT1;
  unsigned int m_PLT0Size;
  unsigned int m_PLT1Size;

  const LinkerConfig& m_Config;
};

//===----------------------------------------------------------------------===//
// X86_32PLT
//===----------------------------------------------------------------------===//
/** \class X86_32PLT
 *  \brief X86_32 Procedure Linkage Table
 */
class X86_32PLT : public X86PLT {
 public:
  X86_32PLT(LDSection& pSection,
            X86_32GOTPLT& pGOTPLT,
            const LinkerConfig& pConfig);

  void applyPLT0();

  void applyPLT1();

 private:
  X86_32GOTPLT& m_GOTPLT;
};

//===----------------------------------------------------------------------===//
// X86_64PLT
//===----------------------------------------------------------------------===//
/** \class X86_64PLT
 *  \brief X86_64 Procedure Linkage Table
 */
class X86_64PLT : public X86PLT {
 public:
  X86_64PLT(LDSection& pSection,
            X86_64GOTPLT& pGOTPLT,
            const LinkerConfig& pConfig);

  void applyPLT0();

  void applyPLT1();

 private:
  X86_64GOTPLT& m_GOTPLT;
};

}  // namespace mcld

#endif  // TARGET_X86_X86PLT_H_
