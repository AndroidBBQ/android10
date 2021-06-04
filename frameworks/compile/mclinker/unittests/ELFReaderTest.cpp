//===- ELFReaderTest.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ELFReaderTest.h"

#include "mcld/IRBuilder.h"
#include "mcld/TargetOptions.h"
#include "mcld/LD/ELFReader.h"
#include "mcld/LD/LDContext.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/Path.h"
#include "mcld/Support/MemoryArea.h"
#include <../lib/Target/X86/X86LDBackend.h>
#include <../lib/Target/X86/X86GNUInfo.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ELF.h>

#include <cstdio>

using namespace mcld;
using namespace mcld::sys::fs;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
ELFReaderTest::ELFReaderTest() : m_pInput(NULL) {
  m_pConfig = new LinkerConfig("x86_64-linux-gnueabi");
  m_pConfig->targets().setEndian(TargetOptions::Little);
  m_pConfig->targets().setBitClass(64);
  Relocation::SetUp(*m_pConfig);

  m_pScript = new LinkerScript();
  m_pInfo = new X86_64GNUInfo(m_pConfig->targets().triple());
  m_pLDBackend = new X86_64GNULDBackend(*m_pConfig, m_pInfo);
  m_pELFReader = new ELFReader<64, true>(*m_pLDBackend);
  m_pModule = new Module(*m_pScript);
  m_pIRBuilder = new IRBuilder(*m_pModule, *m_pConfig);
  m_pELFObjReader =
      new ELFObjectReader(*m_pLDBackend, *m_pIRBuilder, *m_pConfig);
}

// Destructor can do clean-up work that doesn't throw exceptions here.
ELFReaderTest::~ELFReaderTest() {
  delete m_pConfig;
  delete m_pLDBackend;
  delete m_pELFReader;
  delete m_pScript;
  delete m_pModule;
  delete m_pIRBuilder;
  delete m_pELFObjReader;
}

// SetUp() will be called immediately before each test.
void ELFReaderTest::SetUp() {
  Path path(TOPDIR);
  path.append("unittests/test_x86_64.o");

  m_pInput = m_pIRBuilder->ReadInput("test_x86_64", path);
  ASSERT_TRUE(NULL != m_pInput);

  ASSERT_TRUE(m_pInput->hasMemArea());
  size_t hdr_size = m_pELFReader->getELFHeaderSize();
  llvm::StringRef region =
      m_pInput->memArea()->request(m_pInput->fileOffset(), hdr_size);
  const char* ELF_hdr = region.begin();
  bool shdr_result = m_pELFReader->readSectionHeaders(*m_pInput, ELF_hdr);
  ASSERT_TRUE(shdr_result);
}

// TearDown() will be called immediately after each test.
void ELFReaderTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(ELFReaderTest, read_section_headers) {
  ASSERT_EQ(m_pInput->context()->numOfSections(), 13u);
  LDContext::const_sect_iterator iter = m_pInput->context()->sectBegin();
  ++iter;  /// test section[1]
  ASSERT_EQ(".text", (*iter)->name());
  ASSERT_EQ(llvm::ELF::SHT_PROGBITS, (*iter)->type());
  ASSERT_EQ(0x40u, (*iter)->offset());
  ASSERT_EQ(0x15u, (*iter)->size());
  ASSERT_TRUE(llvm::ELF::SHF_ALLOC & (*iter)->flag());  // AX
  ASSERT_EQ(0x4u, (*iter)->align());
  ASSERT_EQ(NULL, (*iter)->getLink());
  ASSERT_EQ(0u, (*iter)->getInfo());
}

TEST_F(ELFReaderTest, read_symbol_and_rela) {
  ASSERT_TRUE(m_pInput->hasMemArea());
  ASSERT_TRUE(m_pInput->hasContext());
  m_pInput->setType(Input::Object);

  // -- read symbols
  LDSection* symtab_shdr = m_pInput->context()->getSection(".symtab");
  ASSERT_TRUE(NULL != symtab_shdr);

  LDSection* strtab_shdr = symtab_shdr->getLink();
  ASSERT_TRUE(NULL != strtab_shdr);

  llvm::StringRef symtab_region = m_pInput->memArea()->request(
      m_pInput->fileOffset() + symtab_shdr->offset(), symtab_shdr->size());

  llvm::StringRef strtab_region = m_pInput->memArea()->request(
      m_pInput->fileOffset() + strtab_shdr->offset(), strtab_shdr->size());
  const char* strtab = strtab_region.begin();
  bool result = m_pELFReader->readSymbols(
      *m_pInput, *m_pIRBuilder, symtab_region, strtab);
  ASSERT_TRUE(result);
  ASSERT_EQ("hello.c", std::string(m_pInput->context()->getSymbol(1)->name()));
  ASSERT_EQ("puts", std::string(m_pInput->context()->getSymbol(10)->name()));
  ASSERT_TRUE(NULL == m_pInput->context()->getSymbol(11));

  // -- read relocations
  MemoryArea* mem = m_pInput->memArea();
  LDContext::sect_iterator rs = m_pInput->context()->relocSectBegin();
  ASSERT_TRUE(rs != m_pInput->context()->relocSectEnd());
  ASSERT_EQ(".rela.text", (*rs)->name());

  uint64_t offset = m_pInput->fileOffset() + (*rs)->offset();
  uint64_t size = (*rs)->size();
  llvm::StringRef region = mem->request(offset, size);
  IRBuilder::CreateRelocData(**rs);  /// create relocation data for the header

  ASSERT_EQ(llvm::ELF::SHT_RELA, (*rs)->type());
  ASSERT_TRUE(m_pELFReader->readRela(*m_pInput, **rs, region));

  const RelocData::RelocationListType& rRelocs =
      (*rs)->getRelocData()->getRelocationList();
  RelocData::const_iterator rReloc = rRelocs.begin();
  ASSERT_EQ(2u, rRelocs.size());
  ASSERT_TRUE(rRelocs.end() != rReloc);
  ++rReloc;  /// test rRelocs[1]
  ASSERT_EQ("puts", std::string(rReloc->symInfo()->name()));
  ASSERT_EQ(llvm::ELF::R_X86_64_PC32, rReloc->type());
  ASSERT_EQ(0x0u, rReloc->symValue());
  ASSERT_EQ(static_cast<mcld::Relocation::Address>(-0x4), rReloc->addend());
}

TEST_F(ELFReaderTest, read_regular_sections) {
  ASSERT_TRUE(m_pELFObjReader->readSections(*m_pInput));
}

TEST_F(ELFReaderTest, is_my_format) {
  bool doContinue;
  ASSERT_TRUE(m_pELFObjReader->isMyFormat(*m_pInput, doContinue));
}
