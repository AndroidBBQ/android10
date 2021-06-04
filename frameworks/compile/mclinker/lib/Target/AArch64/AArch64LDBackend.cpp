//===- AArch64LDBackend.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64.h"
#include "AArch64CA53Erratum835769Stub.h"
#include "AArch64CA53Erratum843419Stub.h"
#include "AArch64CA53Erratum843419Stub2.h"
#include "AArch64ELFDynamic.h"
#include "AArch64GNUInfo.h"
#include "AArch64InsnHelpers.h"
#include "AArch64LDBackend.h"
#include "AArch64LongBranchStub.h"
#include "AArch64Relocator.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Fragment/AlignFragment.h"
#include "mcld/Fragment/FillFragment.h"
#include "mcld/Fragment/NullFragment.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Fragment/Stub.h"
#include "mcld/LD/BranchIslandFactory.h"
#include "mcld/LD/ELFFileFormat.h"
#include "mcld/LD/ELFSegment.h"
#include "mcld/LD/ELFSegmentFactory.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/StubFactory.h"
#include "mcld/Support/MemoryRegion.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFAttribute.h"
#include "mcld/Target/GNUInfo.h"
#include "mcld/Object/ObjectBuilder.h"

#include <llvm/ADT/Triple.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>

#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// AArch64GNULDBackend
//===----------------------------------------------------------------------===//
AArch64GNULDBackend::AArch64GNULDBackend(const LinkerConfig& pConfig,
                                         GNUInfo* pInfo)
    : GNULDBackend(pConfig, pInfo),
      m_pRelocator(NULL),
      m_pGOT(NULL),
      m_pGOTPLT(NULL),
      m_pPLT(NULL),
      m_pRelaDyn(NULL),
      m_pRelaPLT(NULL),
      m_pDynamic(NULL),
      m_pGOTSymbol(NULL) {
}

AArch64GNULDBackend::~AArch64GNULDBackend() {
  if (m_pRelocator != NULL)
    delete m_pRelocator;
  if (m_pGOT == m_pGOTPLT) {
    if (m_pGOT != NULL)
      delete m_pGOT;
  } else {
    if (m_pGOT != NULL)
      delete m_pGOT;
    if (m_pGOTPLT != NULL)
      delete m_pGOTPLT;
  }
  if (m_pPLT != NULL)
    delete m_pPLT;
  if (m_pRelaDyn != NULL)
    delete m_pRelaDyn;
  if (m_pRelaPLT != NULL)
    delete m_pRelaPLT;
  if (m_pDynamic != NULL)
    delete m_pDynamic;
}

void AArch64GNULDBackend::initTargetSections(Module& pModule,
                                             ObjectBuilder& pBuilder) {
  if (LinkerConfig::Object != config().codeGenType()) {
    ELFFileFormat* file_format = getOutputFormat();

    // initialize .got
    LDSection& got = file_format->getGOT();
    m_pGOT = new AArch64GOT(got);
    if (config().options().hasNow()) {
      // when -z now is given, there will be only one .got section (contains
      // both GOTPLT and normal GOT entries), create GOT0 for .got section and
      // set m_pGOTPLT to the same .got
      m_pGOT->createGOT0();
      m_pGOTPLT = m_pGOT;
    } else {
      // Otherwise, got should be seperated to two sections, .got and .got.plt
      // initialize .got.plt
      LDSection& gotplt = file_format->getGOTPLT();
      m_pGOTPLT = new AArch64GOT(gotplt);
      m_pGOTPLT->createGOT0();
    }

    // initialize .plt
    LDSection& plt = file_format->getPLT();
    m_pPLT = new AArch64PLT(plt, *m_pGOTPLT);

    // initialize .rela.plt
    LDSection& relaplt = file_format->getRelaPlt();
    relaplt.setLink(&plt);
    m_pRelaPLT = new OutputRelocSection(pModule, relaplt);

    // initialize .rela.dyn
    LDSection& reladyn = file_format->getRelaDyn();
    m_pRelaDyn = new OutputRelocSection(pModule, reladyn);
  }
}

void AArch64GNULDBackend::initTargetSymbols(IRBuilder& pBuilder,
                                            Module& pModule) {
  // Define the symbol _GLOBAL_OFFSET_TABLE_ if there is a symbol with the
  // same name in input
  if (LinkerConfig::Object != config().codeGenType()) {
    m_pGOTSymbol =
        pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
            "_GLOBAL_OFFSET_TABLE_",
            ResolveInfo::Object,
            ResolveInfo::Define,
            ResolveInfo::Local,
            0x0,  // size
            0x0,  // value
            FragmentRef::Null(),
            ResolveInfo::Hidden);
  }
}

bool AArch64GNULDBackend::initRelocator() {
  if (m_pRelocator == NULL) {
    m_pRelocator = new AArch64Relocator(*this, config());
  }
  return true;
}

const Relocator* AArch64GNULDBackend::getRelocator() const {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

Relocator* AArch64GNULDBackend::getRelocator() {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

void AArch64GNULDBackend::defineGOTSymbol(IRBuilder& pBuilder) {
  // define symbol _GLOBAL_OFFSET_TABLE_ when .got create
  if (m_pGOTSymbol != NULL) {
    pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOTPLT->begin()), 0x0),
        ResolveInfo::Hidden);
  } else {
    m_pGOTSymbol = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOTPLT->begin()), 0x0),
        ResolveInfo::Hidden);
  }
}

void AArch64GNULDBackend::doPreLayout(IRBuilder& pBuilder) {
  // initialize .dynamic data
  if (!config().isCodeStatic() && m_pDynamic == NULL)
    m_pDynamic = new AArch64ELFDynamic(*this, config());

  if (LinkerConfig::Object != config().codeGenType()) {
    // set .got size
    if (config().options().hasNow()) {
      // when building shared object, the GOTPLT section is must
      if (LinkerConfig::DynObj == config().codeGenType() || m_pGOT->hasGOT1() ||
          m_pGOTSymbol != NULL) {
        m_pGOT->finalizeSectionSize();
        defineGOTSymbol(pBuilder);
      }
    } else {
      // when building shared object, the GOTPLT section is must
      if (LinkerConfig::DynObj == config().codeGenType() ||
          m_pGOTPLT->hasGOT1() || m_pGOTSymbol != NULL) {
        m_pGOTPLT->finalizeSectionSize();
        defineGOTSymbol(pBuilder);
      }
      if (m_pGOT->hasGOT1())
        m_pGOT->finalizeSectionSize();
    }

    // set .plt size
    if (m_pPLT->hasPLT1())
      m_pPLT->finalizeSectionSize();

    ELFFileFormat* file_format = getOutputFormat();
    // set .rela.dyn size
    if (!m_pRelaDyn->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelaDyn().setSize(m_pRelaDyn->numOfRelocs() *
                                        getRelaEntrySize());
    }

    // set .rela.plt size
    if (!m_pRelaPLT->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelaPlt().setSize(m_pRelaPLT->numOfRelocs() *
                                        getRelaEntrySize());
    }
  }
}

void AArch64GNULDBackend::doPostLayout(Module& pModule, IRBuilder& pBuilder) {
  const ELFFileFormat* file_format = getOutputFormat();

  // apply PLT
  if (file_format->hasPLT()) {
    assert(m_pPLT != NULL);
    m_pPLT->applyPLT0();
    m_pPLT->applyPLT1();
  }

  // apply GOTPLT
  if ((config().options().hasNow() && file_format->hasGOT()) ||
      file_format->hasGOTPLT()) {
    assert(m_pGOTPLT != NULL);
    if (LinkerConfig::DynObj == config().codeGenType())
      m_pGOTPLT->applyGOT0(file_format->getDynamic().addr());
    else {
      // executable file and object file? should fill with zero.
      m_pGOTPLT->applyGOT0(0);
    }
  }
}

AArch64ELFDynamic& AArch64GNULDBackend::dynamic() {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

const AArch64ELFDynamic& AArch64GNULDBackend::dynamic() const {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

uint64_t AArch64GNULDBackend::emitSectionData(const LDSection& pSection,
                                              MemoryRegion& pRegion) const {
  assert(pRegion.size() && "Size of MemoryRegion is zero!");

  const ELFFileFormat* file_format = getOutputFormat();

  if (file_format->hasPLT() && (&pSection == &(file_format->getPLT()))) {
    uint64_t result = m_pPLT->emit(pRegion);
    return result;
  }

  if (file_format->hasGOT() && (&pSection == &(file_format->getGOT()))) {
    uint64_t result = m_pGOT->emit(pRegion);
    return result;
  }

  if (file_format->hasGOTPLT() && (&pSection == &(file_format->getGOTPLT()))) {
    uint64_t result = m_pGOT->emit(pRegion);
    return result;
  }

  return pRegion.size();
}

unsigned int AArch64GNULDBackend::getTargetSectionOrder(
    const LDSection& pSectHdr) const {
  const ELFFileFormat* file_format = getOutputFormat();

  if (file_format->hasGOT() && (&pSectHdr == &file_format->getGOT())) {
    if (config().options().hasNow())
      return SHO_RELRO;
    return SHO_RELRO_LAST;
  }

  if (file_format->hasGOTPLT() && (&pSectHdr == &file_format->getGOTPLT()))
    return SHO_NON_RELRO_FIRST;

  if (file_format->hasPLT() && (&pSectHdr == &file_format->getPLT()))
    return SHO_PLT;

  return SHO_UNDEFINED;
}

void AArch64GNULDBackend::scanErrata(Module& pModule,
                                     IRBuilder& pBuilder,
                                     size_t& num_new_stubs,
                                     size_t& stubs_strlen) {
  // TODO: Implement AArch64 ErrataStubFactory to create the specific erratum
  //       stub and simplify the logics.
  for (Module::iterator sect = pModule.begin(), sectEnd = pModule.end();
       sect != sectEnd; ++sect) {
    if (((*sect)->kind() == LDFileFormat::TEXT) && (*sect)->hasSectionData()) {
      SectionData* sd = (*sect)->getSectionData();
      for (SectionData::iterator it = sd->begin(), ie = sd->end(); it != ie;
           ++it) {
        Fragment* frag = llvm::dyn_cast<RegionFragment>(it);
        if (frag != NULL) {
          FragmentRef* frag_ref = FragmentRef::Create(*frag, 0);
          for (unsigned offset = 0; offset < frag->size();
               offset += AArch64InsnHelpers::InsnSize) {
            Stub* stub = getStubFactory()->create(*frag_ref,
                                                  pBuilder,
                                                  *getBRIslandFactory());
            if (stub != NULL) {
              // A stub symbol should be local
              assert(stub->symInfo() != NULL && stub->symInfo()->isLocal());
              const AArch64CA53ErratumStub* erratum_stub =
                  reinterpret_cast<const AArch64CA53ErratumStub*>(stub);
              assert(erratum_stub != NULL);
              // Rewrite the erratum instruction as a branch to the stub.
              uint64_t offset = frag_ref->offset() +
                                erratum_stub->getErratumInsnOffset();
              Relocation* reloc =
                  Relocation::Create(llvm::ELF::R_AARCH64_JUMP26,
                                     *(FragmentRef::Create(*frag, offset)),
                                     /* pAddend */0);
              reloc->setSymInfo(stub->symInfo());
              reloc->target() = AArch64InsnHelpers::buildBranchInsn();
              addExtraRelocation(reloc);

              ++num_new_stubs;
              stubs_strlen += stub->symInfo()->nameSize() + 1;
            }

            frag_ref->assign(*frag, offset + AArch64InsnHelpers::InsnSize);
          }  // for each INSN
        }
      }  // for each FRAGMENT
    }
  }  // for each TEXT section
}

bool AArch64GNULDBackend::doRelax(Module& pModule,
                                  IRBuilder& pBuilder,
                                  bool& pFinished) {
  assert(getStubFactory() != NULL && getBRIslandFactory() != NULL);

  // Number of new stubs added
  size_t num_new_stubs = 0;
  // String lengh to hold new stub symbols
  size_t stubs_strlen = 0;

  if (config().targets().fixCA53Erratum835769() ||
      config().targets().fixCA53Erratum843419()) {
    scanErrata(pModule, pBuilder, num_new_stubs, stubs_strlen);
  }

  ELFFileFormat* file_format = getOutputFormat();
  // check branch relocs and create the related stubs if needed
  Module::obj_iterator input, inEnd = pModule.obj_end();
  for (input = pModule.obj_begin(); input != inEnd; ++input) {
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      if (LDFileFormat::Ignore == (*rs)->kind() || !(*rs)->hasRelocData())
        continue;
      RelocData::iterator reloc, rEnd = (*rs)->getRelocData()->end();
      for (reloc = (*rs)->getRelocData()->begin(); reloc != rEnd; ++reloc) {
        Relocation* relocation = llvm::cast<Relocation>(reloc);

        switch (relocation->type()) {
          case llvm::ELF::R_AARCH64_CALL26:
          case llvm::ELF::R_AARCH64_JUMP26: {
            // calculate the possible symbol value
            uint64_t sym_value = 0x0;
            LDSymbol* symbol = relocation->symInfo()->outSymbol();
            if (symbol->hasFragRef()) {
              uint64_t value = symbol->fragRef()->getOutputOffset();
              uint64_t addr =
                  symbol->fragRef()->frag()->getParent()->getSection().addr();
              sym_value = addr + value;
            }
            if ((relocation->symInfo()->reserved() &
                 AArch64Relocator::ReservePLT) != 0x0) {
              // FIXME: we need to find out the address of the specific plt
              // entry
              assert(file_format->hasPLT());
              sym_value = file_format->getPLT().addr();
            }
            Stub* stub = getStubFactory()->create(*relocation,  // relocation
                                                  sym_value,    // symbol value
                                                  pBuilder,
                                                  *getBRIslandFactory());
            if (stub != NULL) {
              // a stub symbol should be local
              assert(stub->symInfo() != NULL && stub->symInfo()->isLocal());
              // reset the branch target of the reloc to this stub instead
              relocation->setSymInfo(stub->symInfo());

              ++num_new_stubs;
              stubs_strlen += stub->symInfo()->nameSize() + 1;
            }
            break;
          }
          default: {
            break;
          }
        }  // end of switch
      }  // for all relocations
    }  // for all relocation section
  }  // for all inputs

  // Find the first fragment w/ invalid offset due to stub insertion.
  std::vector<Fragment*> invalid_frags;
  pFinished = true;
  for (BranchIslandFactory::iterator island = getBRIslandFactory()->begin(),
                                     island_end = getBRIslandFactory()->end();
       island != island_end;
       ++island) {
    if ((*island).size() > stubGroupSize()) {
      error(diag::err_no_space_to_place_stubs) << stubGroupSize();
      return false;
    }

    if ((*island).numOfStubs() == 0) {
      continue;
    }

    Fragment* exit = &*(*island).end();
    if (exit == &*(*island).begin()->getParent()->end()) {
      continue;
    }

    if (((*island).offset() + (*island).size()) > exit->getOffset()) {
      if (invalid_frags.empty() ||
          (invalid_frags.back()->getParent() != (*island).getParent())) {
        invalid_frags.push_back(exit);
        pFinished = false;
      }
      continue;
    }
  }

  // Reset the offset of invalid fragments.
  for (auto it = invalid_frags.begin(), ie = invalid_frags.end(); it != ie;
       ++it) {
    Fragment* invalid = *it;
    while (invalid != NULL) {
      invalid->setOffset(invalid->getPrevNode()->getOffset() +
                         invalid->getPrevNode()->size());
      invalid = invalid->getNextNode();
    }
  }

  // Fix up the size of .symtab, .strtab, and TEXT sections
  if (num_new_stubs == 0) {
    return false;
  } else {
    switch (config().options().getStripSymbolMode()) {
      case GeneralOptions::StripSymbolMode::StripAllSymbols:
      case GeneralOptions::StripSymbolMode::StripLocals:
        break;
      default: {
        LDSection& symtab = file_format->getSymTab();
        LDSection& strtab = file_format->getStrTab();

        symtab.setSize(symtab.size() +
                       sizeof(llvm::ELF::Elf64_Sym) * num_new_stubs);
        symtab.setInfo(symtab.getInfo() + num_new_stubs);
        strtab.setSize(strtab.size() + stubs_strlen);
      }
    }  // switch (config().options().getStripSymbolMode())

    SectionData* prev = NULL;
    for (BranchIslandFactory::iterator island = getBRIslandFactory()->begin(),
                                       island_end = getBRIslandFactory()->end();
         island != island_end;
         ++island) {
      SectionData* sd = (*island).begin()->getParent();
      if ((*island).numOfStubs() != 0) {
        if (sd != prev) {
          sd->getSection().setSize(sd->back().getOffset() + sd->back().size());
        }
      }
      prev = sd;
    }
    return true;
  }  // if (num_new_stubs == 0)
}

bool AArch64GNULDBackend::initTargetStubs() {
  StubFactory* factory = getStubFactory();
  if (factory != NULL) {
    factory->addPrototype(new AArch64LongBranchStub(config().isCodeIndep()));
    if (config().targets().fixCA53Erratum835769()) {
      factory->addPrototype(new AArch64CA53Erratum835769Stub());
    }
    if (config().targets().fixCA53Erratum843419()) {
      factory->addPrototype(new AArch64CA53Erratum843419Stub());
      factory->addPrototype(new AArch64CA53Erratum843419Stub2());
    }
    return true;
  }
  return false;
}

void AArch64GNULDBackend::doCreateProgramHdrs(Module& pModule) {
}

bool AArch64GNULDBackend::finalizeTargetSymbols() {
  return true;
}

bool AArch64GNULDBackend::mergeSection(Module& pModule,
                                       const Input& pInput,
                                       LDSection& pSection) {
  return true;
}

bool AArch64GNULDBackend::readSection(Input& pInput, SectionData& pSD) {
  return true;
}

AArch64GOT& AArch64GNULDBackend::getGOT() {
  assert(m_pGOT != NULL && "GOT section not exist");
  return *m_pGOT;
}

const AArch64GOT& AArch64GNULDBackend::getGOT() const {
  assert(m_pGOT != NULL && "GOT section not exist");
  return *m_pGOT;
}

AArch64GOT& AArch64GNULDBackend::getGOTPLT() {
  assert(m_pGOTPLT != NULL && "GOTPLT section not exist");
  return *m_pGOTPLT;
}

const AArch64GOT& AArch64GNULDBackend::getGOTPLT() const {
  assert(m_pGOTPLT != NULL && "GOTPLT section not exist");
  return *m_pGOTPLT;
}

AArch64PLT& AArch64GNULDBackend::getPLT() {
  assert(m_pPLT != NULL && "PLT section not exist");
  return *m_pPLT;
}

const AArch64PLT& AArch64GNULDBackend::getPLT() const {
  assert(m_pPLT != NULL && "PLT section not exist");
  return *m_pPLT;
}

OutputRelocSection& AArch64GNULDBackend::getRelaDyn() {
  assert(m_pRelaDyn != NULL && ".rela.dyn section not exist");
  return *m_pRelaDyn;
}

const OutputRelocSection& AArch64GNULDBackend::getRelaDyn() const {
  assert(m_pRelaDyn != NULL && ".rela.dyn section not exist");
  return *m_pRelaDyn;
}

OutputRelocSection& AArch64GNULDBackend::getRelaPLT() {
  assert(m_pRelaPLT != NULL && ".rela.plt section not exist");
  return *m_pRelaPLT;
}

const OutputRelocSection& AArch64GNULDBackend::getRelaPLT() const {
  assert(m_pRelaPLT != NULL && ".rela.plt section not exist");
  return *m_pRelaPLT;
}

//===----------------------------------------------------------------------===//
//  createAArch64LDBackend - the help funtion to create corresponding
//  AArch64LDBackend
//===----------------------------------------------------------------------===//
TargetLDBackend* createAArch64LDBackend(const LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker is not supported yet");
    /**
    return new AArch64MachOLDBackend(createAArch64MachOArchiveReader,
                                     createAArch64MachOObjectReader,
                                     createAArch64MachOObjectWriter);
    **/
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker is not supported yet");
    /**
    return new AArch64COFFLDBackend(createAArch64COFFArchiveReader,
                                    createAArch64COFFObjectReader,
                                    createAArch64COFFObjectWriter);
    **/
  }
  return new AArch64GNULDBackend(
      pConfig, new AArch64GNUInfo(pConfig.targets().triple()));
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// Force static initialization.
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeAArch64LDBackend() {
  // Register the linker backend
  mcld::TargetRegistry::RegisterTargetLDBackend(mcld::TheAArch64Target,
                                                mcld::createAArch64LDBackend);
}
