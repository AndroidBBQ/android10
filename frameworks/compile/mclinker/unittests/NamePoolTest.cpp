//===- NamePoolTest.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "NamePoolTest.h"
#include "mcld/LD/NamePool.h"
#include "mcld/LD/Resolver.h"
#include "mcld/LD/StaticResolver.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/LDSymbol.h"
#include <llvm/ADT/StringRef.h>
#include <string>
#include <cstdio>

using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
NamePoolTest::NamePoolTest() {
  // create testee. modify it if need
  StaticResolver resolver;
  m_pTestee = new NamePool(resolver, 10);
}

// Destructor can do clean-up work that doesn't throw exceptions here.
NamePoolTest::~NamePoolTest() {
  delete m_pTestee;
}

// SetUp() will be called immediately before each test.
void NamePoolTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void NamePoolTest::TearDown() {
}

//==========================================================================//
// Testcases
//

TEST_F(NamePoolTest, insertString) {
  const char* s1 = "Hello MCLinker";
  llvm::StringRef result1 = m_pTestee->insertString(s1);
  EXPECT_NE(s1, result1.data());
  EXPECT_STREQ(s1, result1.data());
}

TEST_F(NamePoolTest, insertSameString) {
  const char* s1 = "Hello MCLinker";
  std::string s2(s1);
  llvm::StringRef result1 = m_pTestee->insertString(s1);
  llvm::StringRef result2 = m_pTestee->insertString(s2.c_str());
  EXPECT_STREQ(s1, result1.data());
  EXPECT_STREQ(s2.c_str(), result2.data());
  EXPECT_EQ(result1.data(), result2.data());
}

TEST_F(NamePoolTest, insert_local_defined_Symbol) {
  const char* name = "Hello MCLinker";
  bool isDyn = false;
  ResolveInfo::Type type = ResolveInfo::Function;
  ResolveInfo::Desc desc = ResolveInfo::Define;
  ResolveInfo::Binding binding = ResolveInfo::Local;
  uint64_t value = 0;
  uint64_t size = 0;
  ResolveInfo::Visibility other = ResolveInfo::Default;
  Resolver::Result result1;
  m_pTestee->insertSymbol(
      name, isDyn, type, desc, binding, size, other, NULL, result1);

  EXPECT_NE(name, result1.info->name());
  EXPECT_STREQ(name, result1.info->name());
  EXPECT_EQ(isDyn, result1.info->isDyn());
  EXPECT_EQ(type, result1.info->type());
  EXPECT_EQ(desc, result1.info->desc());
  EXPECT_EQ(binding, result1.info->binding());
  EXPECT_EQ(size, result1.info->size());
  EXPECT_EQ(other, result1.info->visibility());

  Resolver::Result result2;
  m_pTestee->insertSymbol(
      name, isDyn, type, desc, binding, size, other, NULL, result2);

  EXPECT_NE(name, result1.info->name());
  EXPECT_STREQ(name, result1.info->name());
  EXPECT_EQ(isDyn, result1.info->isDyn());
  EXPECT_EQ(type, result1.info->type());
  EXPECT_EQ(desc, result1.info->desc());
  EXPECT_EQ(binding, result1.info->binding());
  EXPECT_EQ(size, result1.info->size());
  EXPECT_EQ(other, result1.info->visibility());

  EXPECT_NE(result1.existent, result2.existent);
}

TEST_F(NamePoolTest, insert_global_reference_Symbol) {
  const char* name = "Hello MCLinker";
  bool isDyn = false;
  ResolveInfo::Type type = ResolveInfo::NoType;
  ResolveInfo::Desc desc = ResolveInfo::Undefined;
  ResolveInfo::Binding binding = ResolveInfo::Global;
  uint64_t size = 0;
  ResolveInfo::Visibility other = ResolveInfo::Default;
  Resolver::Result result1;
  m_pTestee->insertSymbol(
      name, isDyn, type, desc, binding, size, other, NULL, result1);

  EXPECT_NE(name, result1.info->name());
  EXPECT_STREQ(name, result1.info->name());
  EXPECT_EQ(isDyn, result1.info->isDyn());
  EXPECT_EQ(type, result1.info->type());
  EXPECT_EQ(desc, result1.info->desc());
  EXPECT_EQ(binding, result1.info->binding());
  EXPECT_EQ(size, result1.info->size());
  EXPECT_EQ(other, result1.info->visibility());

  Resolver::Result result2;
  m_pTestee->insertSymbol(
      name, isDyn, type, desc, binding, size, other, NULL, result2);

  EXPECT_EQ(result1.info, result2.info);

  Resolver::Result result3;
  m_pTestee->insertSymbol("Different Symbol",
                          isDyn,
                          type,
                          desc,
                          binding,
                          size,
                          other,
                          NULL,
                          result3);

  EXPECT_NE(result1.info, result3.info);
}

TEST_F(NamePoolTest, insertSymbol_after_insert_same_string) {
  const char* name = "Hello MCLinker";
  bool isDyn = false;
  LDSymbol::Type type = LDSymbol::Defined;
  LDSymbol::Binding binding = LDSymbol::Global;
  const llvm::MCSectionData* section = 0;
  uint64_t value = 0;
  uint64_t size = 0;
  uint8_t other = 0;

  const char* result1 = m_pTestee->insertString(name);
  LDSymbol* sym = m_pTestee->insertSymbol(
      name, isDyn, type, binding, section, value, size, other);

  EXPECT_STREQ(name, sym->name());
  EXPECT_EQ(result1, sym->name());

  char s[16];
  strcpy(s, result1);
  const char* result2 = m_pTestee->insertString(result1);
  const char* result3 = m_pTestee->insertString(s);

  EXPECT_EQ(result1, result2);
  EXPECT_EQ(result1, result3);
}

TEST_F(NamePoolTest, insert_16384_weak_reference_symbols) {
  char name[16];
  bool isDyn = false;
  LDSymbol::Type type = LDSymbol::Reference;
  LDSymbol::Binding binding = LDSymbol::Weak;
  const llvm::MCSectionData* section = 0;
  uint64_t value = 0;
  uint64_t size = 0;
  uint8_t other = 0;
  strcpy(name, "Hello MCLinker");
  LDSymbol* syms[128][128];
  for (int i = 0; i < 128; ++i) {
    name[0] = i;
    for (int j = 0; j < 128; ++j) {
      name[1] = j;
      syms[i][j] = m_pTestee->insertSymbol(
          name, isDyn, type, binding, section, value, size, other);

      ASSERT_STREQ(name, syms[i][j]->name());
    }
  }
  for (int i = 127; i >= 0; --i) {
    name[0] = i;
    for (int j = 0; j < 128; ++j) {
      name[1] = j;
      LDSymbol* sym = m_pTestee->insertSymbol(
          name, isDyn, type, binding, section, value, size, other);
      ASSERT_EQ(sym, syms[i][j]);
    }
  }
  for (int i = 0; i < 128; ++i) {
    name[0] = i;
    for (int j = 0; j < 128; ++j) {
      name[1] = j;
      LDSymbol* sym = m_pTestee->insertSymbol(
          name, isDyn, type, binding, section, value, size, other);
      ASSERT_EQ(sym, syms[i][j]);
    }
  }
}
