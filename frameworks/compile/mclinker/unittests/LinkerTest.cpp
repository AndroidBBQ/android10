//===- LinkerTest.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "LinkerTest.h"

#include "mcld/Environment.h"
#include "mcld/Module.h"
#include "mcld/InputTree.h"
#include "mcld/IRBuilder.h"
#include "mcld/Linker.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"

#include "mcld/Support/Path.h"

#include <llvm/Support/ELF.h>

using namespace mcld;
using namespace mcld::test;
using namespace mcld::sys::fs;

// Constructor can do set-up work for all test here.
LinkerTest::LinkerTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
LinkerTest::~LinkerTest() {
}

// SetUp() will be called immediately before each test.
void LinkerTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void LinkerTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(LinkerTest, set_up_n_clean_up) {
  Initialize();
  LinkerConfig config("arm-none-linux-gnueabi");
  LinkerScript script;
  Module module("test", script);
  config.setCodeGenType(LinkerConfig::DynObj);

  Linker linker;
  linker.emulate(script, config);

  IRBuilder builder(module, config);
  // create inputs here
  //   builder.CreateInput("./test.o");

  if (linker.link(module, builder))
    linker.emit(module, "./test.so");

  Finalize();
}

// %MCLinker --shared -soname=libplasma.so -Bsymbolic
// -mtriple="armv7-none-linux-gnueabi"
// -L=%p/../../../libs/ARM/Android/android-14
// %p/../../../libs/ARM/Android/android-14/crtbegin_so.o
// %p/plasma.o
// -lm -llog -ljnigraphics -lc
// %p/../../../libs/ARM/Android/android-14/crtend_so.o
// -o libplasma.so
TEST_F(LinkerTest, plasma) {
  Initialize();
  Linker linker;
  LinkerScript script;

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config("armv7-none-linux-gnueabi");

  /// -L=${TOPDIR}/test/libs/ARM/Android/android-14
  Path search_dir(TOPDIR);
  search_dir.append("test/libs/ARM/Android/android-14");
  script.directories().insert(search_dir);

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script, config);

  config.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config.options().setSOName("libplasma.so");   ///< --soname=libplasma.so
  config.options().setBsymbolic();              ///< -Bsymbolic

  Module module("libplasma.so", script);
  IRBuilder builder(module, config);

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtbegin_so.o
  Path crtbegin(search_dir);
  crtbegin.append("crtbegin_so.o");
  builder.ReadInput("crtbegin", crtbegin);

  /// ${TOPDIR}/test/Android/Plasma/ARM/plasma.o
  Path plasma(TOPDIR);
  plasma.append("test/Android/Plasma/ARM/plasma.o");
  builder.ReadInput("plasma", plasma);

  // -lm -llog -ljnigraphics -lc
  builder.ReadInput("m");
  builder.ReadInput("log");
  builder.ReadInput("jnigraphics");
  builder.ReadInput("c");

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtend_so.o
  Path crtend(search_dir);
  crtend.append("crtend_so.o");
  builder.ReadInput("crtend", crtend);

  if (linker.link(module, builder)) {
    linker.emit(module, "libplasma.so");  ///< -o libplasma.so
  }

  Finalize();
}

// The outputs generated without -Bsymbolic usually have more relocation
// entries than the outputs generated with -Bsymbolic. This testcase generates
// output with -Bsymbolic first, then generate the same output without
// -Bsymbolic.
// By this way, we can make sure symbols and relocations are cleaned between
// two linkings.
TEST_F(LinkerTest, plasma_twice) {
  Initialize();
  Linker linker;

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config1("armv7-none-linux-gnueabi");

  LinkerScript script1;
  /// -L=${TOPDIR}/test/libs/ARM/Android/android-14
  Path search_dir(TOPDIR);
  search_dir.append("test/libs/ARM/Android/android-14");
  script1.directories().insert(search_dir);

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script1, config1);

  config1.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config1.options().setSOName(
      "libplasma.once.so");               ///< --soname=libplasma.twice.so
  config1.options().setBsymbolic(false);  ///< -Bsymbolic

  Module module1("libplasma.once.so", script1);
  IRBuilder builder1(module1, config1);

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtbegin_so.o
  Path crtbegin(search_dir);
  crtbegin.append("crtbegin_so.o");
  builder1.ReadInput("crtbegin", crtbegin);

  /// ${TOPDIR}/test/Android/Plasma/ARM/plasma.o
  Path plasma(TOPDIR);
  plasma.append("test/Android/Plasma/ARM/plasma.o");
  builder1.ReadInput("plasma", plasma);

  // -lm -llog -ljnigraphics -lc
  builder1.ReadInput("m");
  builder1.ReadInput("log");
  builder1.ReadInput("jnigraphics");
  builder1.ReadInput("c");

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtend_so.o
  Path crtend(search_dir);
  crtend.append("crtend_so.o");
  builder1.ReadInput("crtend", crtend);

  if (linker.link(module1, builder1)) {
    linker.emit(module1, "libplasma.once.so");  ///< -o libplasma.so
  }

  Finalize();

  linker.reset();

  Initialize();

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config2("armv7-none-linux-gnueabi");

  LinkerScript script2;
  /// -L=${TOPDIR}/test/libs/ARM/Android/android-14
  script2.directories().insert(search_dir);

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script2, config2);

  config2.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config2.options().setSOName(
      "libplasma.twice.so");         ///< --soname=libplasma.twice.exe
  config2.options().setBsymbolic();  ///< -Bsymbolic

  Module module2("libplasma.so", script2);
  IRBuilder builder2(module2, config2);

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtbegin_so.o
  builder2.ReadInput("crtbegin", crtbegin);

  /// ${TOPDIR}/test/Android/Plasma/ARM/plasma.o
  builder2.ReadInput("plasma", plasma);

  // -lm -llog -ljnigraphics -lc
  builder2.ReadInput("m");
  builder2.ReadInput("log");
  builder2.ReadInput("jnigraphics");
  builder2.ReadInput("c");

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtend_so.o
  builder2.ReadInput("crtend", crtend);

  if (linker.link(module2, builder2)) {
    linker.emit(module2, "libplasma.twice.so");  ///< -o libplasma.exe
  }

  Finalize();
}

// This testcase put IRBuilder in the heap
TEST_F(LinkerTest, plasma_twice_irbuilder_heap) {
  Initialize();
  Linker linker;

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config1("armv7-none-linux-gnueabi");

  LinkerScript script1;
  /// -L=${TOPDIR}/test/libs/ARM/Android/android-14
  Path search_dir(TOPDIR);
  search_dir.append("test/libs/ARM/Android/android-14");
  script1.directories().insert(search_dir);

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script1, config1);

  config1.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config1.options().setSOName(
      "libplasma.once.so");               ///< --soname=libplasma.twice.so
  config1.options().setBsymbolic(false);  ///< -Bsymbolic

  Module module1("libplasma.once.so", script1);
  IRBuilder* builder1 = new IRBuilder(module1, config1);

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtbegin_so.o
  Path crtbegin(search_dir);
  crtbegin.append("crtbegin_so.o");
  builder1->ReadInput("crtbegin", crtbegin);

  /// ${TOPDIR}/test/Android/Plasma/ARM/plasma.o
  Path plasma(TOPDIR);
  plasma.append("test/Android/Plasma/ARM/plasma.o");
  builder1->ReadInput("plasma", plasma);

  // -lm -llog -ljnigraphics -lc
  builder1->ReadInput("m");
  builder1->ReadInput("log");
  builder1->ReadInput("jnigraphics");
  builder1->ReadInput("c");

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtend_so.o
  Path crtend(search_dir);
  crtend.append("crtend_so.o");
  builder1->ReadInput("crtend", crtend);

  if (linker.link(module1, *builder1)) {
    linker.emit(module1, "libplasma.once.so");  ///< -o libplasma.so
  }

  // Can not delete builder until emit the output. Dynamic string table
  // needs the file name of the input files, and the inputs' life is
  // controlled by IRBuilder
  delete builder1;

  Finalize();

  linker.reset();

  Initialize();

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config2("armv7-none-linux-gnueabi");

  LinkerScript script2;
  /// -L=${TOPDIR}/test/libs/ARM/Android/android-14
  script2.directories().insert(search_dir);

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script2, config2);

  config2.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config2.options().setSOName(
      "libplasma.twice.so");         ///< --soname=libplasma.twice.exe
  config2.options().setBsymbolic();  ///< -Bsymbolic

  Module module2("libplasma.so", script2);
  IRBuilder* builder2 = new IRBuilder(module2, config2);

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtbegin_so.o
  builder2->ReadInput("crtbegin", crtbegin);

  /// ${TOPDIR}/test/Android/Plasma/ARM/plasma.o
  builder2->ReadInput("plasma", plasma);

  // -lm -llog -ljnigraphics -lc
  builder2->ReadInput("m");
  builder2->ReadInput("log");
  builder2->ReadInput("jnigraphics");
  builder2->ReadInput("c");

  /// ${TOPDIR}/test/libs/ARM/Android/android-14/crtend_so.o
  builder2->ReadInput("crtend", crtend);

  if (linker.link(module2, *builder2)) {
    linker.emit(module2, "libplasma.twice.so");  ///< -o libplasma.exe
  }

  delete builder2;
  Finalize();
}

// %MCLinker --shared -soname=libgotplt.so -mtriple arm-none-linux-gnueabi
// gotplt.o -o libgotplt.so
TEST_F(LinkerTest, plasma_object) {
  Initialize();
  Linker linker;

  ///< --mtriple="armv7-none-linux-gnueabi"
  LinkerConfig config("armv7-none-linux-gnueabi");
  LinkerScript script;

  /// To configure linker before setting options. Linker::config sets up
  /// default target-dependent configuration to LinkerConfig.
  linker.emulate(script, config);

  config.setCodeGenType(LinkerConfig::DynObj);  ///< --shared
  config.options().setSOName("libgotplt.so");   ///< --soname=libgotplt.so

  Module module(script);
  IRBuilder builder(module, config);

  Path gotplt_o(TOPDIR);
  gotplt_o.append("test/PLT/gotplt.o");
  Input* input = builder.CreateInput("gotplt.o", gotplt_o, Input::Object);

  /// Sections
  /// [ 0]                   NULL            00000000 000000 000000 00      0
  /// 0  0
  builder.CreateELFHeader(
      *input, "", LDFileFormat::Null, llvm::ELF::SHT_NULL, 0x0);

  /// [ 1] .text             PROGBITS        00000000 000034 000010 00  AX  0
  /// 0  4
  LDSection* text =
      builder.CreateELFHeader(*input,
                              ".text",
                              llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR,
                              4);

  SectionData* text_data = builder.CreateSectionData(*text);
  static uint8_t text_content[] = {
      0x00, 0x48, 0x2d, 0xe9,
      0xfe, 0xff, 0xff, 0xeb,
      0x00, 0x48, 0xbd, 0xe8,
      0x0e, 0xf0, 0xa0, 0xe1
  };

  Fragment* text_frag = builder.CreateRegion(text_content, 0x10);
  builder.AppendFragment(*text_frag, *text_data);

  /// [ 2] .rel.text         REL             00000000 0002ac 000008 08      7
  /// 1  4
  LDSection* rel_text =
      builder.CreateELFHeader(*input, ".rel.text", llvm::ELF::SHT_REL, 0x0, 4);
  rel_text->setLink(text);
  builder.CreateRelocData(*rel_text);

  /// [ 3] .data             PROGBITS        00000000 000044 000000 00  WA  0
  /// 0  4
  LDSection* data =
      builder.CreateELFHeader(*input,
                              ".data",
                              llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                              4);

  /// [ 4] .bss              NOBITS          00000000 000044 000000 00  WA  0
  /// 0  4
  LDSection* bss =
      builder.CreateELFHeader(*input,
                              ".bss",
                              llvm::ELF::SHT_NOBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE,
                              4);
  builder.CreateBSS(*bss);

  /// [ 5] .ARM.attributes   ARM_ATTRIBUTES  00000000 000044 000020 00      0
  /// 0  1
  LDSection* attr = builder.CreateELFHeader(
      *input, ".ARM.attributes", llvm::ELF::SHT_ARM_ATTRIBUTES, 0x0, 1);

  SectionData* attr_data = builder.CreateSectionData(*attr);
  static uint8_t attr_content[] = {
      0x41, 0x1f, 0x00, 0x00,
      0x00, 0x61, 0x65, 0x61,
      0x62, 0x69, 0x00, 0x01,
      0x15, 0x00, 0x00, 0x00,
      0x06, 0x02, 0x08, 0x01,
      0x09, 0x01, 0x14, 0x01,
      0x15, 0x01, 0x17, 0x03,
      0x18, 0x01, 0x19, 0x01
  };

  Fragment* attr_frag = builder.CreateRegion(attr_content, 0x20);
  builder.AppendFragment(*attr_frag, *attr_data);

  /// Symbols
  /// 1: 00000000     0 FILE    LOCAL  DEFAULT  ABS Output/gotplt.bc
  builder.AddSymbol(*input,
                    "Output/gotplt.bc",
                    ResolveInfo::File,
                    ResolveInfo::Define,
                    ResolveInfo::Local,
                    0);
  /// 2: 00000000     0 SECTION LOCAL  DEFAULT    1
  builder.AddSymbol(*input,
                    ".text",
                    ResolveInfo::Section,
                    ResolveInfo::Define,
                    ResolveInfo::Local,
                    0,
                    0x0,
                    text);
  /// 3: 00000000     0 SECTION LOCAL  DEFAULT    3
  builder.AddSymbol(*input,
                    ".data",
                    ResolveInfo::Section,
                    ResolveInfo::Define,
                    ResolveInfo::Local,
                    0,
                    0x0,
                    data);
  /// 4: 00000000     0 SECTION LOCAL  DEFAULT    4
  builder.AddSymbol(*input,
                    ".bss",
                    ResolveInfo::Section,
                    ResolveInfo::Define,
                    ResolveInfo::Local,
                    0,
                    0x0,
                    bss);
  /// 5: 00000000     0 SECTION LOCAL  DEFAULT    5
  builder.AddSymbol(*input,
                    ".ARM.attributes",
                    ResolveInfo::Section,
                    ResolveInfo::Define,
                    ResolveInfo::Local,
                    0,
                    0x0,
                    attr);
  /// 6: 00000000    16 FUNC    GLOBAL DEFAULT    1 _Z1fv
  builder.AddSymbol(*input,
                    "_Z1fv",
                    ResolveInfo::Function,
                    ResolveInfo::Define,
                    ResolveInfo::Global,
                    16,
                    0x0,
                    text);

  /// 7: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND _Z1gv
  LDSymbol* z1gv = builder.AddSymbol(*input,
                                     "_Z1gv",
                                     ResolveInfo::NoType,
                                     ResolveInfo::Undefined,
                                     ResolveInfo::Global,
                                     0);

  /// Relocations
  /// Offset     Info    Type            Sym.Value  Sym. Name
  /// 00000004  0000071b R_ARM_PLT32       00000000   _Z1gv
  builder.AddRelocation(*rel_text, llvm::ELF::R_ARM_PLT32, *z1gv, 0x4);

  if (linker.link(module, builder)) {
    linker.emit(module, "libgotplt.so");  ///< -o libgotplt.so
  }

  Finalize();
}
