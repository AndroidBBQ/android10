//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "StaticResolverTest.h"
#include "mcld/Support/TargetSelect.h"
#include "mcld/LD/StaticResolver.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LinkerConfig.h"

#include "mcld/Support/FileSystem.h"

using namespace mcld;
using namespace mcldtest;

//===----------------------------------------------------------------------===//
// StaticResolverTest
//===----------------------------------------------------------------------===//
// Constructor can do set-up work for all test here.
StaticResolverTest::StaticResolverTest() : m_pResolver(NULL), m_pConfig(NULL) {
  // create testee. modify it if need
  m_pResolver = new StaticResolver();

  m_pConfig = new LinkerConfig("arm-none-linux-gnueabi");
}

// Destructor can do clean-up work that doesn't throw exceptions here.
StaticResolverTest::~StaticResolverTest() {
  delete m_pResolver;
  delete m_pConfig;
}

// SetUp() will be called immediately before each test.
void StaticResolverTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void StaticResolverTest::TearDown() {
}

//==========================================================================//
// Testcases
//
TEST_F(StaticResolverTest, MDEF) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");
  new_sym->setDesc(ResolveInfo::Define);
  old_sym->setDesc(ResolveInfo::Define);
  ASSERT_TRUE(mcld::ResolveInfo::Define == new_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Define == old_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::define_flag == new_sym->info());
  ASSERT_TRUE(mcld::ResolveInfo::define_flag == old_sym->info());
  bool override = true;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
}

TEST_F(StaticResolverTest, DynDefAfterDynUndef) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Undefined);
  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);

  old_sym->setSize(1);

  ASSERT_TRUE(mcld::ResolveInfo::Global == new_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Global == old_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Undefined == new_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Define == old_sym->desc());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
  ASSERT_TRUE(1 == old_sym->size());
}

TEST_F(StaticResolverTest, DynDefAfterDynDef) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Define);
  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);

  old_sym->setSize(1);

  ASSERT_TRUE(mcld::ResolveInfo::Global == new_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Global == old_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Define == new_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Define == old_sym->desc());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
  ASSERT_TRUE(1 == old_sym->size());
}

TEST_F(StaticResolverTest, DynUndefAfterDynUndef) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Undefined);
  old_sym->setDesc(ResolveInfo::Undefined);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);

  old_sym->setSize(1);

  ASSERT_TRUE(mcld::ResolveInfo::Global == new_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Global == old_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Undefined == new_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Undefined == old_sym->desc());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
  ASSERT_TRUE(1 == old_sym->size());
}

TEST_F(StaticResolverTest, OverrideWeakByGlobal) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setSize(0);
  old_sym->setSize(1);

  ASSERT_TRUE(mcld::ResolveInfo::Global == new_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Weak == old_sym->binding());

  ASSERT_TRUE(mcld::ResolveInfo::global_flag == new_sym->info());
  ASSERT_TRUE(mcld::ResolveInfo::weak_flag == old_sym->info());
  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_TRUE(override);
  ASSERT_TRUE(0 == old_sym->size());
}

TEST_F(StaticResolverTest, DynWeakAfterDynDef) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setBinding(ResolveInfo::Global);

  new_sym->setSource(true);
  old_sym->setSource(true);

  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setDesc(ResolveInfo::Define);

  new_sym->setSize(0);

  old_sym->setSize(1);

  ASSERT_TRUE(mcld::ResolveInfo::Weak == old_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Global == new_sym->binding());
  ASSERT_TRUE(mcld::ResolveInfo::Define == old_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Define == new_sym->desc());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
  ASSERT_TRUE(1 == old_sym->size());
}

TEST_F(StaticResolverTest, MarkByBiggerCommon) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setDesc(ResolveInfo::Common);
  old_sym->setDesc(ResolveInfo::Common);
  new_sym->setSize(999);
  old_sym->setSize(0);

  ASSERT_TRUE(mcld::ResolveInfo::Common == new_sym->desc());
  ASSERT_TRUE(mcld::ResolveInfo::Common == old_sym->desc());

  ASSERT_TRUE(mcld::ResolveInfo::common_flag == new_sym->info());
  ASSERT_TRUE(mcld::ResolveInfo::common_flag == old_sym->info());
  bool override = true;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_FALSE(override);
  ASSERT_TRUE(999 == old_sym->size());
}

TEST_F(StaticResolverTest, OverrideByBiggerCommon) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  new_sym->setDesc(ResolveInfo::Common);
  old_sym->setDesc(ResolveInfo::Common);
  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setSize(999);
  old_sym->setSize(0);

  ASSERT_TRUE(ResolveInfo::Common == new_sym->desc());
  ASSERT_TRUE(ResolveInfo::Common == old_sym->desc());
  ASSERT_TRUE(ResolveInfo::Weak == old_sym->binding());

  ASSERT_TRUE(ResolveInfo::common_flag == new_sym->info());
  ASSERT_TRUE((ResolveInfo::weak_flag | ResolveInfo::common_flag) ==
              old_sym->info());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_TRUE(override);
  ASSERT_TRUE(999 == old_sym->size());
}

TEST_F(StaticResolverTest, OverrideCommonByDefine) {
  ResolveInfo* old_sym = ResolveInfo::Create("abc");
  ResolveInfo* new_sym = ResolveInfo::Create("abc");

  old_sym->setDesc(ResolveInfo::Common);
  old_sym->setSize(0);

  new_sym->setDesc(ResolveInfo::Define);
  new_sym->setSize(999);

  ASSERT_TRUE(ResolveInfo::Define == new_sym->desc());
  ASSERT_TRUE(ResolveInfo::Common == old_sym->desc());

  ASSERT_TRUE(ResolveInfo::define_flag == new_sym->info());
  ASSERT_TRUE(ResolveInfo::common_flag == old_sym->info());

  bool override = false;
  bool result = m_pResolver->resolve(*old_sym, *new_sym, override, 0x0);
  ASSERT_TRUE(result);
  ASSERT_TRUE(override);
  ASSERT_TRUE(999 == old_sym->size());
}

TEST_F(StaticResolverTest, SetUpDesc) {
  ResolveInfo* sym = ResolveInfo::Create("abc");

  sym->setIsSymbol(true);

  //  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE(sym->isSymbol());
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setIsSymbol(false);
  ASSERT_FALSE(sym->isSymbol());
  //  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setDesc(ResolveInfo::Define);
  ASSERT_FALSE(sym->isSymbol());
  //  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_TRUE(sym->isDefine());
  ASSERT_FALSE(sym->isUndef());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(ResolveInfo::Define == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setDesc(ResolveInfo::Common);
  ASSERT_FALSE(sym->isSymbol());
  //  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_FALSE(sym->isUndef());
  ASSERT_TRUE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(ResolveInfo::Common == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setDesc(ResolveInfo::Indirect);
  ASSERT_FALSE(sym->isSymbol());
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_FALSE(sym->isUndef());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_TRUE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(ResolveInfo::Indirect == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setDesc(ResolveInfo::Undefined);
  ASSERT_FALSE(sym->isSymbol());
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());
}

TEST_F(StaticResolverTest, SetUpBinding) {
  ResolveInfo* sym = ResolveInfo::Create("abc");

  sym->setIsSymbol(true);

  //  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE(sym->isSymbol());
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(0 == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setBinding(ResolveInfo::Global);
  ASSERT_TRUE(sym->isSymbol());
  ASSERT_TRUE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(ResolveInfo::Global == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setBinding(ResolveInfo::Weak);
  ASSERT_TRUE(sym->isSymbol());
  ASSERT_FALSE(sym->isGlobal());
  ASSERT_TRUE(sym->isWeak());
  ASSERT_FALSE(sym->isLocal());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(ResolveInfo::Weak == sym->binding());
  ASSERT_TRUE(0 == sym->other());

  sym->setBinding(ResolveInfo::Local);
  ASSERT_TRUE(sym->isSymbol());
  ASSERT_FALSE(sym->isGlobal());
  ASSERT_FALSE(sym->isWeak());
  ASSERT_TRUE(sym->isLocal());
  ASSERT_FALSE(sym->isDyn());
  ASSERT_FALSE(sym->isDefine());
  ASSERT_TRUE(sym->isUndef());
  ASSERT_FALSE(sym->isCommon());
  ASSERT_FALSE(sym->isIndirect());
  ASSERT_TRUE(ResolveInfo::NoType == sym->type());
  ASSERT_TRUE(0 == sym->desc());
  ASSERT_TRUE(ResolveInfo::Local == sym->binding());
  ASSERT_TRUE(0 == sym->other());
}
