//===- ARMLDBackend.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ARM.h"
#include "ARMGNUInfo.h"
#include "ARMELFAttributeData.h"
#include "ARMELFDynamic.h"
#include "ARMException.h"
#include "ARMLDBackend.h"
#include "ARMRelocator.h"
#include "ARMToARMStub.h"
#include "ARMToTHMStub.h"
#include "THMToTHMStub.h"
#include "THMToARMStub.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/ADT/ilist_sort.h"
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
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MemoryRegion.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Target/ELFAttribute.h"
#include "mcld/Target/GNUInfo.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>

#include <cstring>
#include <vector>

namespace mcld {

/// Fragment data for EXIDX_CANTUNWIND.
static const char g_CantUnwindEntry[8] = {
  // Relocation to text section.
  0, 0, 0, 0,
  // EXIDX_CANTUNWIND (little endian.)
  1, 0, 0, 0,
};

/// Helper function to create a local symbol at the end of the fragment.
static mcld::ResolveInfo*
CreateLocalSymbolToFragmentEnd(mcld::Module& pModule, mcld::Fragment& pFrag) {
  // Create and add symbol to the name pool.
  mcld::ResolveInfo* resolveInfo =
      pModule.getNamePool().createSymbol(/* pName */"",
                                         /* pIsDyn */false,
                                         mcld::ResolveInfo::Section,
                                         mcld::ResolveInfo::Define,
                                         mcld::ResolveInfo::Local,
                                         /* pSize */0,
                                         mcld::ResolveInfo::Hidden);
  if (resolveInfo == nullptr) {
    return nullptr;
  }

  // Create input symbol.
  mcld::LDSymbol* inputSym = mcld::LDSymbol::Create(*resolveInfo);
  if (inputSym == nullptr) {
    return nullptr;
  }

  inputSym->setFragmentRef(mcld::FragmentRef::Create(pFrag, pFrag.size()));
  inputSym->setValue(/* pValue */0);

  // The output symbol is simply an alias to the input symbol.
  resolveInfo->setSymPtr(inputSym);

  return resolveInfo;
}

/// Comparator to sort .ARM.exidx fragments according to the address of the
/// corresponding .text fragment.
class ExIdxFragmentComparator {
 private:
  const ARMExData& m_pExData;

 public:
  explicit ExIdxFragmentComparator(const ARMExData& pExData)
      : m_pExData(pExData) {
  }

  bool operator()(const Fragment& a, const Fragment& b) {
    ARMExSectionTuple* tupleA = m_pExData.getTupleByExIdx(&a);
    ARMExSectionTuple* tupleB = m_pExData.getTupleByExIdx(&b);

    Fragment* textFragA = tupleA->getTextFragment();
    Fragment* textFragB = tupleB->getTextFragment();

    uint64_t addrA = textFragA->getParent()->getSection().addr() +
                     textFragA->getOffset();
    uint64_t addrB = textFragB->getParent()->getSection().addr() +
                     textFragB->getOffset();
    return (addrA < addrB);
  }
};

//===----------------------------------------------------------------------===//
// ARMGNULDBackend
//===----------------------------------------------------------------------===//
ARMGNULDBackend::ARMGNULDBackend(const LinkerConfig& pConfig, GNUInfo* pInfo)
    : GNULDBackend(pConfig, pInfo),
      m_pRelocator(NULL),
      m_pGOT(NULL),
      m_pPLT(NULL),
      m_pRelDyn(NULL),
      m_pRelPLT(NULL),
      m_pAttrData(NULL),
      m_pDynamic(NULL),
      m_pGOTSymbol(NULL),
      m_pEXIDXStart(NULL),
      m_pEXIDXEnd(NULL),
      m_pEXIDX(NULL),
      m_pEXTAB(NULL),
      m_pAttributes(NULL) {
}

ARMGNULDBackend::~ARMGNULDBackend() {
  delete m_pRelocator;
  delete m_pGOT;
  delete m_pPLT;
  delete m_pRelDyn;
  delete m_pRelPLT;
  delete m_pDynamic;
  delete m_pAttrData;
}

void ARMGNULDBackend::initTargetSections(Module& pModule,
                                         ObjectBuilder& pBuilder) {
  // FIXME: Currently we set exidx and extab to "Exception" and directly emit
  // them from input
  m_pEXIDX =
      pBuilder.CreateSection(".ARM.exidx",
                             LDFileFormat::Target,
                             llvm::ELF::SHT_ARM_EXIDX,
                             llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_LINK_ORDER,
                             config().targets().bitclass() / 8);
  m_pEXTAB = pBuilder.CreateSection(".ARM.extab",
                                    LDFileFormat::Target,
                                    llvm::ELF::SHT_PROGBITS,
                                    llvm::ELF::SHF_ALLOC,
                                    0x1);
  m_pAttributes = pBuilder.CreateSection(".ARM.attributes",
                                         LDFileFormat::Target,
                                         llvm::ELF::SHT_ARM_ATTRIBUTES,
                                         0x0,
                                         0x1);

  // initialize "aeabi" attributes subsection
  m_pAttrData = new ARMELFAttributeData();
  attribute().registerAttributeData(*m_pAttrData);

  if (LinkerConfig::Object != config().codeGenType()) {
    ELFFileFormat* file_format = getOutputFormat();

    // initialize .got
    LDSection& got = file_format->getGOT();
    m_pGOT = new ARMGOT(got);

    // initialize .plt
    LDSection& plt = file_format->getPLT();
    m_pPLT = new ARMPLT(plt, *m_pGOT);

    // initialize .rel.plt
    LDSection& relplt = file_format->getRelPlt();
    relplt.setLink(&plt);
    // create SectionData and ARMRelDynSection
    m_pRelPLT = new OutputRelocSection(pModule, relplt);

    // initialize .rel.dyn
    LDSection& reldyn = file_format->getRelDyn();
    m_pRelDyn = new OutputRelocSection(pModule, reldyn);
  }
}

void ARMGNULDBackend::initTargetSymbols(IRBuilder& pBuilder, Module& pModule) {
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
  if (m_pEXIDX != NULL && m_pEXIDX->size() != 0x0) {
    FragmentRef* exidx_start =
        FragmentRef::Create(m_pEXIDX->getSectionData()->front(), 0x0);
    FragmentRef* exidx_end = FragmentRef::Create(
        m_pEXIDX->getSectionData()->front(), m_pEXIDX->size());
    m_pEXIDXStart =
        pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
            "__exidx_start",
            ResolveInfo::Object,
            ResolveInfo::Define,
            ResolveInfo::Local,
            0x0,          // size
            0x0,          // value
            exidx_start,  // FragRef
            ResolveInfo::Default);

    m_pEXIDXEnd = pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
        "__exidx_end",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,        // size
        0x0,        // value
        exidx_end,  // FragRef
        ResolveInfo::Default);
    // change __exidx_start/_end to local dynamic category
    if (m_pEXIDXStart != NULL)
      pModule.getSymbolTable().changeToDynamic(*m_pEXIDXStart);
    if (m_pEXIDXEnd != NULL)
      pModule.getSymbolTable().changeToDynamic(*m_pEXIDXEnd);
  } else {
    m_pEXIDXStart =
        pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
            "__exidx_start",
            ResolveInfo::NoType,
            ResolveInfo::Define,
            ResolveInfo::Absolute,
            0x0,  // size
            0x0,  // value
            FragmentRef::Null(),
            ResolveInfo::Default);

    m_pEXIDXEnd = pBuilder.AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
        "__exidx_end",
        ResolveInfo::NoType,
        ResolveInfo::Define,
        ResolveInfo::Absolute,
        0x0,  // size
        0x0,  // value
        FragmentRef::Null(),
        ResolveInfo::Default);
  }
}

bool ARMGNULDBackend::initRelocator() {
  if (m_pRelocator == NULL) {
    m_pRelocator = new ARMRelocator(*this, config());
  }
  return true;
}

const Relocator* ARMGNULDBackend::getRelocator() const {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

Relocator* ARMGNULDBackend::getRelocator() {
  assert(m_pRelocator != NULL);
  return m_pRelocator;
}

void ARMGNULDBackend::doPreLayout(IRBuilder& pBuilder) {
  // initialize .dynamic data
  if (!config().isCodeStatic() && m_pDynamic == NULL)
    m_pDynamic = new ARMELFDynamic(*this, config());

  // set attribute section size
  m_pAttributes->setSize(attribute().sizeOutput());

  // set .got size
  // when building shared object, the .got section is must
  if (LinkerConfig::Object != config().codeGenType()) {
    if (LinkerConfig::DynObj == config().codeGenType() || m_pGOT->hasGOT1() ||
        m_pGOTSymbol != NULL) {
      m_pGOT->finalizeSectionSize();
      defineGOTSymbol(pBuilder);
    }

    // set .plt size
    if (m_pPLT->hasPLT1())
      m_pPLT->finalizeSectionSize();

    ELFFileFormat* file_format = getOutputFormat();
    // set .rel.dyn size
    if (!m_pRelDyn->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelDyn().setSize(m_pRelDyn->numOfRelocs() *
                                       getRelEntrySize());
    }

    // set .rel.plt size
    if (!m_pRelPLT->empty()) {
      assert(
          !config().isCodeStatic() &&
          "static linkage should not result in a dynamic relocation section");
      file_format->getRelPlt().setSize(m_pRelPLT->numOfRelocs() *
                                       getRelEntrySize());
    }
  }
}

void ARMGNULDBackend::doPostLayout(Module& pModule, IRBuilder& pBuilder) {
  const ELFFileFormat* file_format = getOutputFormat();

  // apply PLT
  if (file_format->hasPLT()) {
    // Since we already have the size of LDSection PLT, m_pPLT should not be
    // NULL.
    assert(m_pPLT != NULL);
    m_pPLT->applyPLT0();
    m_pPLT->applyPLT1();
  }

  // apply GOT
  if (file_format->hasGOT()) {
    // Since we already have the size of GOT, m_pGOT should not be NULL.
    assert(m_pGOT != NULL);
    if (LinkerConfig::DynObj == config().codeGenType())
      m_pGOT->applyGOT0(file_format->getDynamic().addr());
    else {
      // executable file and object file? should fill with zero.
      m_pGOT->applyGOT0(0);
    }
  }
}

/// dynamic - the dynamic section of the target machine.
/// Use co-variant return type to return its own dynamic section.
ARMELFDynamic& ARMGNULDBackend::dynamic() {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

/// dynamic - the dynamic section of the target machine.
/// Use co-variant return type to return its own dynamic section.
const ARMELFDynamic& ARMGNULDBackend::dynamic() const {
  assert(m_pDynamic != NULL);
  return *m_pDynamic;
}

void ARMGNULDBackend::defineGOTSymbol(IRBuilder& pBuilder) {
  // define symbol _GLOBAL_OFFSET_TABLE_ when .got create
  if (m_pGOTSymbol != NULL) {
    pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOT->begin()), 0x0),
        ResolveInfo::Hidden);
  } else {
    m_pGOTSymbol = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        "_GLOBAL_OFFSET_TABLE_",
        ResolveInfo::Object,
        ResolveInfo::Define,
        ResolveInfo::Local,
        0x0,  // size
        0x0,  // value
        FragmentRef::Create(*(m_pGOT->begin()), 0x0),
        ResolveInfo::Hidden);
  }
}

uint64_t ARMGNULDBackend::emitSectionData(const LDSection& pSection,
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

  if (&pSection == m_pAttributes) {
    return attribute().emit(pRegion);
  }

  // FIXME: Currently Emitting .ARM.attributes, .ARM.exidx, and .ARM.extab
  // directly from the input file.
  const SectionData* sect_data = pSection.getSectionData();
  SectionData::const_iterator frag_iter, frag_end = sect_data->end();
  uint8_t* out_offset = pRegion.begin();
  for (frag_iter = sect_data->begin(); frag_iter != frag_end; ++frag_iter) {
    size_t size = frag_iter->size();
    switch (frag_iter->getKind()) {
      case Fragment::Fillment: {
        const FillFragment& fill_frag = llvm::cast<FillFragment>(*frag_iter);
        if (fill_frag.getValueSize() == 0) {
          // virtual fillment, ignore it.
          break;
        }

        memset(out_offset, fill_frag.getValue(), fill_frag.size());
        break;
      }
      case Fragment::Region: {
        const RegionFragment& region_frag =
            llvm::cast<RegionFragment>(*frag_iter);
        const char* start = region_frag.getRegion().begin();
        memcpy(out_offset, start, size);
        break;
      }
      case Fragment::Alignment: {
        const AlignFragment& align_frag = llvm::cast<AlignFragment>(*frag_iter);
        uint64_t count = size / align_frag.getValueSize();
        switch (align_frag.getValueSize()) {
          case 1u:
            std::memset(out_offset, align_frag.getValue(), count);
            break;
          default:
            llvm::report_fatal_error(
                "unsupported value size for align fragment emission yet.\n");
            break;
        }  // end switch
        break;
      }
      case Fragment::Null: {
        assert(0x0 == size);
        break;
      }
      default:
        llvm::report_fatal_error("unsupported fragment type.\n");
        break;
    }  // end switch
    out_offset += size;
  }  // end for
  return pRegion.size();
}

/// finalizeSymbol - finalize the symbol value
bool ARMGNULDBackend::finalizeTargetSymbols() {
  return true;
}


/// preMergeSections - hooks to be executed before merging sections
void ARMGNULDBackend::preMergeSections(Module& pModule) {
  // Since the link relationship between .text and .ARM.exidx will be discarded
  // after merging sections, we have to build the exception handling section
  // mapping before section merge.
  m_pExData = ARMExData::create(pModule);
}

/// postMergeSections - hooks to be executed after merging sections
void ARMGNULDBackend::postMergeSections(Module& pModule) {
  if (m_pEXIDX->hasSectionData()) {
    // Append the NullFragment so that __exidx_end can be correctly inserted.
    NullFragment* null = new NullFragment(m_pEXIDX->getSectionData());
    null->setOffset(m_pEXIDX->size());
  }
}

bool ARMGNULDBackend::mergeSection(Module& pModule,
                                   const Input& pInput,
                                   LDSection& pSection) {
  switch (pSection.type()) {
    case llvm::ELF::SHT_ARM_ATTRIBUTES: {
      return attribute().merge(pInput, pSection);
    }

    case llvm::ELF::SHT_ARM_EXIDX: {
      assert(pSection.getLink() != NULL);
      if ((pSection.getLink()->kind() == LDFileFormat::Ignore) ||
          (pSection.getLink()->kind() == LDFileFormat::Folded)) {
        // if the target section of the .ARM.exidx is Ignore, then it should be
        // ignored as well
        pSection.setKind(LDFileFormat::Ignore);
        return true;
      }

      if (!m_pEXIDX->hasSectionData()) {
        // Create SectionData for m_pEXIDX.
        SectionData* sectData = IRBuilder::CreateSectionData(*m_pEXIDX);

        // Initialize the alignment of m_pEXIDX.
        const size_t alignExIdx = 4;
        m_pEXIDX->setAlign(alignExIdx);

        // Insert an AlignFragment to the beginning of m_pEXIDX.
        AlignFragment* frag =
            new AlignFragment(/*alignment*/alignExIdx,
                              /*the filled value*/0x0,
                              /*the size of filled value*/1u,
                              /*max bytes to emit*/alignExIdx - 1);
        frag->setOffset(0);
        frag->setParent(sectData);
        sectData->getFragmentList().push_back(frag);
        m_pEXIDX->setSize(frag->size());
      }

      // Move RegionFragment from pSection to m_pEXIDX.
      uint64_t offset = m_pEXIDX->size();
      SectionData::FragmentListType& src =
          pSection.getSectionData()->getFragmentList();
      SectionData::FragmentListType& dst =
          m_pEXIDX->getSectionData()->getFragmentList();
      SectionData::FragmentListType::iterator frag = src.begin();
      SectionData::FragmentListType::iterator fragEnd = src.end();
      while (frag != fragEnd) {
        if (frag->getKind() != Fragment::Region) {
          ++frag;
        } else {
          frag->setParent(m_pEXIDX->getSectionData());
          frag->setOffset(offset);
          offset += frag->size();
          dst.splice(dst.end(), src, frag++);
        }
      }

      // Update the size of m_pEXIDX.
      m_pEXIDX->setSize(offset);
      return true;
    }

    default: {
      ObjectBuilder builder(pModule);
      builder.MergeSection(pInput, pSection);
      return true;
    }
  }  // end of switch
  return true;
}

void ARMGNULDBackend::setUpReachedSectionsForGC(
    const Module& pModule,
    GarbageCollection::SectionReachedListMap& pSectReachedListMap) const {
  // traverse all the input relocations to find the relocation sections applying
  // .ARM.exidx sections
  Module::const_obj_iterator input, inEnd = pModule.obj_end();
  for (input = pModule.obj_begin(); input != inEnd; ++input) {
    LDContext::const_sect_iterator rs,
        rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      // bypass the discarded relocation section
      // 1. its section kind is changed to Ignore. (The target section is a
      // discarded group section.)
      // 2. it has no reloc data. (All symbols in the input relocs are in the
      // discarded group sections)
      LDSection* reloc_sect = *rs;
      LDSection* apply_sect = reloc_sect->getLink();
      if ((LDFileFormat::Ignore == reloc_sect->kind()) ||
          (!reloc_sect->hasRelocData()))
        continue;

      if (llvm::ELF::SHT_ARM_EXIDX == apply_sect->type()) {
        // 1. set up the reference according to relocations
        bool add_first = false;
        GarbageCollection::SectionListTy* reached_sects = NULL;
        RelocData::iterator reloc_it, rEnd = reloc_sect->getRelocData()->end();
        for (reloc_it = reloc_sect->getRelocData()->begin(); reloc_it != rEnd;
             ++reloc_it) {
          Relocation* reloc = llvm::cast<Relocation>(reloc_it);
          ResolveInfo* sym = reloc->symInfo();
          // only the target symbols defined in the input fragments can make the
          // reference
          if (sym == NULL)
            continue;
          if (!sym->isDefine() || !sym->outSymbol()->hasFragRef())
            continue;

          // only the target symbols defined in the concerned sections can make
          // the reference
          const LDSection* target_sect =
              &sym->outSymbol()->fragRef()->frag()->getParent()->getSection();
          if (target_sect->kind() != LDFileFormat::TEXT &&
              target_sect->kind() != LDFileFormat::DATA &&
              target_sect->kind() != LDFileFormat::BSS)
            continue;

          // setup the reached list, if we first add the element to reached list
          // of this section, create an entry in ReachedSections map
          if (!add_first) {
            reached_sects = &pSectReachedListMap.getReachedList(*apply_sect);
            add_first = true;
          }
          reached_sects->insert(target_sect);
        }
        reached_sects = NULL;
        add_first = false;
        // 2. set up the reference from XXX to .ARM.exidx.XXX
        assert(apply_sect->getLink() != NULL);
        pSectReachedListMap.addReference(*apply_sect->getLink(), *apply_sect);
      }
    }
  }
}

bool ARMGNULDBackend::readSection(Input& pInput, SectionData& pSD) {
  Fragment* frag = NULL;
  uint32_t offset = pInput.fileOffset() + pSD.getSection().offset();
  uint32_t size = pSD.getSection().size();

  llvm::StringRef region = pInput.memArea()->request(offset, size);
  if (region.size() == 0) {
    // If the input section's size is zero, we got a NULL region.
    // use a virtual fill fragment
    frag = new FillFragment(0x0, 0, 0);
  } else {
    frag = new RegionFragment(region);
  }

  ObjectBuilder::AppendFragment(*frag, pSD);
  return true;
}

ARMGOT& ARMGNULDBackend::getGOT() {
  assert(m_pGOT != NULL && "GOT section not exist");
  return *m_pGOT;
}

const ARMGOT& ARMGNULDBackend::getGOT() const {
  assert(m_pGOT != NULL && "GOT section not exist");
  return *m_pGOT;
}

ARMPLT& ARMGNULDBackend::getPLT() {
  assert(m_pPLT != NULL && "PLT section not exist");
  return *m_pPLT;
}

const ARMPLT& ARMGNULDBackend::getPLT() const {
  assert(m_pPLT != NULL && "PLT section not exist");
  return *m_pPLT;
}

OutputRelocSection& ARMGNULDBackend::getRelDyn() {
  assert(m_pRelDyn != NULL && ".rel.dyn section not exist");
  return *m_pRelDyn;
}

const OutputRelocSection& ARMGNULDBackend::getRelDyn() const {
  assert(m_pRelDyn != NULL && ".rel.dyn section not exist");
  return *m_pRelDyn;
}

OutputRelocSection& ARMGNULDBackend::getRelPLT() {
  assert(m_pRelPLT != NULL && ".rel.plt section not exist");
  return *m_pRelPLT;
}

const OutputRelocSection& ARMGNULDBackend::getRelPLT() const {
  assert(m_pRelPLT != NULL && ".rel.plt section not exist");
  return *m_pRelPLT;
}

ARMELFAttributeData& ARMGNULDBackend::getAttributeData() {
  assert(m_pAttrData != NULL && ".ARM.attributes section not exist");
  return *m_pAttrData;
}

const ARMELFAttributeData& ARMGNULDBackend::getAttributeData() const {
  assert(m_pAttrData != NULL && ".ARM.attributes section not exist");
  return *m_pAttrData;
}

unsigned int ARMGNULDBackend::getTargetSectionOrder(
    const LDSection& pSectHdr) const {
  const ELFFileFormat* file_format = getOutputFormat();

  if (file_format->hasGOT() && (&pSectHdr == &file_format->getGOT())) {
    if (config().options().hasNow())
      return SHO_RELRO_LAST;
    return SHO_DATA;
  }

  if (file_format->hasPLT() && (&pSectHdr == &file_format->getPLT()))
    return SHO_PLT;

  if (&pSectHdr == m_pEXIDX || &pSectHdr == m_pEXTAB) {
    // put ARM.exidx and ARM.extab in the same order of .eh_frame
    return SHO_EXCEPTION;
  }

  return SHO_UNDEFINED;
}

void ARMGNULDBackend::rewriteARMExIdxSection(Module& pModule) {
  if (!m_pEXIDX->hasSectionData()) {
    // Return if this is empty section.
    return;
  }

  SectionData* sectData = m_pEXIDX->getSectionData();
  SectionData::FragmentListType& list = sectData->getFragmentList();

  // Move the first fragment (align fragment) and last fragment (null fragment)
  // to temporary list because we would only like to sort the region fragment.
  SectionData::FragmentListType tmp;
  {
    SectionData::iterator first = sectData->begin();
    SectionData::iterator last = sectData->end();
    --last;

    assert(first->getKind() == Fragment::Alignment);
    assert(last->getKind() == Fragment::Null);

    tmp.splice(tmp.end(), list, first);
    tmp.splice(tmp.end(), list, last);
  }

  // Sort the region fragments in the .ARM.exidx output section.
  sort(list, ExIdxFragmentComparator(*m_pExData));

  // Fix the coverage of the .ARM.exidx table.
  llvm::StringRef cantUnwindRegion(g_CantUnwindEntry,
                                   sizeof(g_CantUnwindEntry));

  SectionData::FragmentListType::iterator it = list.begin();
  if (it != list.end()) {
    Fragment* prevTextFrag = m_pExData->getTupleByExIdx(&*it)->getTextFragment();
    uint64_t prevTextEnd = prevTextFrag->getParent()->getSection().addr() +
                           prevTextFrag->getOffset() +
                           prevTextFrag->size();
    ++it;
    while (it != list.end()) {
      Fragment* currTextFrag =
          m_pExData->getTupleByExIdx(&*it)->getTextFragment();
      uint64_t currTextBegin = currTextFrag->getParent()->getSection().addr() +
                               currTextFrag->getOffset();

      if (currTextBegin > prevTextEnd) {
        // Found a gap. Insert a can't unwind entry.
        RegionFragment* frag = new RegionFragment(cantUnwindRegion, nullptr);
        frag->setParent(sectData);
        list.insert(it, frag);

        // Add PREL31 reference to the beginning of the uncovered region.
        Relocation* reloc =
            Relocation::Create(static_cast<uint32_t>(llvm::ELF::R_ARM_PREL31),
                               *FragmentRef::Create(*frag, /* pOffset */0),
                               /* pAddend */0);
        reloc->setSymInfo(
            CreateLocalSymbolToFragmentEnd(pModule, *prevTextFrag));
        addExtraRelocation(reloc);
      }

      prevTextEnd = currTextBegin + currTextFrag->size();
      prevTextFrag = currTextFrag;
      ++it;
    }

    // Add a can't unwind entry to terminate .ARM.exidx section.
    RegionFragment* frag = new RegionFragment(cantUnwindRegion, nullptr);
    frag->setParent(sectData);
    list.push_back(frag);

    // Add PREL31 reference to the end of the .text section.
    Relocation* reloc =
        Relocation::Create(static_cast<uint32_t>(llvm::ELF::R_ARM_PREL31),
                           *FragmentRef::Create(*frag, /* pOffset */0),
                           /* pAddend */0);
    reloc->setSymInfo(CreateLocalSymbolToFragmentEnd(pModule, *prevTextFrag));
    addExtraRelocation(reloc);
  }

  // Add the first and the last fragment back.
  list.splice(list.begin(), tmp, tmp.begin());
  list.splice(list.end(), tmp, tmp.begin());

  // Update the fragment offsets.
  uint64_t offset = 0;
  for (SectionData::iterator it = sectData->begin(), end = sectData->end();
       it != end; ++it) {
    it->setOffset(offset);
    offset += it->size();
  }

  // Update the section size.
  m_pEXIDX->setSize(offset);

  // Rebuild the section header.
  setOutputSectionAddress(pModule);
}

/// relax - the relaxation pass
bool ARMGNULDBackend::relax(Module& pModule, IRBuilder& pBuilder) {
  if (!GNULDBackend::relax(pModule, pBuilder)) {
    return false;
  }
  rewriteARMExIdxSection(pModule);
  return true;
}

/// doRelax
bool ARMGNULDBackend::doRelax(Module& pModule,
                              IRBuilder& pBuilder,
                              bool& pFinished) {
  assert(getStubFactory() != NULL && getBRIslandFactory() != NULL);

  bool isRelaxed = false;
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
          case llvm::ELF::R_ARM_PC24:
          case llvm::ELF::R_ARM_CALL:
          case llvm::ELF::R_ARM_JUMP24:
          case llvm::ELF::R_ARM_PLT32:
          case llvm::ELF::R_ARM_THM_CALL:
          case llvm::ELF::R_ARM_THM_XPC22:
          case llvm::ELF::R_ARM_THM_JUMP24:
          case llvm::ELF::R_ARM_THM_JUMP19: {
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
                 ARMRelocator::ReservePLT) != 0x0) {
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
              assert(stub->symInfo() != NULL);
              // reset the branch target of the reloc to this stub instead
              relocation->setSymInfo(stub->symInfo());

              switch (config().options().getStripSymbolMode()) {
                case GeneralOptions::StripSymbolMode::StripAllSymbols:
                case GeneralOptions::StripSymbolMode::StripLocals:
                  break;
                default: {
                  // a stub symbol should be local
                  assert(stub->symInfo() != NULL && stub->symInfo()->isLocal());
                  LDSection& symtab = file_format->getSymTab();
                  LDSection& strtab = file_format->getStrTab();

                  // increase the size of .symtab and .strtab if needed
                  symtab.setSize(symtab.size() + sizeof(llvm::ELF::Elf32_Sym));
                  symtab.setInfo(symtab.getInfo() + 1);
                  strtab.setSize(strtab.size() + stub->symInfo()->nameSize() +
                                 1);
                }
              }  // end of switch
              isRelaxed = true;
            }
            break;
          }
          case llvm::ELF::R_ARM_V4BX:
            /* FIXME: bypass R_ARM_V4BX relocation now */
            break;
          default:
            break;
        }  // end of switch
      }  // for all relocations
    }  // for all relocation section
  }  // for all inputs

  // find the first fragment w/ invalid offset due to stub insertion
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

  // reset the offset of invalid fragments
  for (auto it = invalid_frags.begin(), ie = invalid_frags.end(); it != ie;
       ++it) {
    Fragment* invalid = *it;
    while (invalid != NULL) {
      invalid->setOffset(invalid->getPrevNode()->getOffset() +
                         invalid->getPrevNode()->size());
      invalid = invalid->getNextNode();
    }
  }

  // reset the size of section that has stubs inserted.
  if (isRelaxed) {
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
  }
  return isRelaxed;
}

/// initTargetStubs
bool ARMGNULDBackend::initTargetStubs() {
  if (getStubFactory() != NULL) {
    getStubFactory()->addPrototype(new ARMToARMStub(config().isCodeIndep()));
    getStubFactory()->addPrototype(new ARMToTHMStub(config().isCodeIndep()));
    getStubFactory()->addPrototype(
        new THMToTHMStub(config().isCodeIndep(), m_pAttrData->usingThumb2()));
    getStubFactory()->addPrototype(
        new THMToARMStub(config().isCodeIndep(), m_pAttrData->usingThumb2()));
    return true;
  }
  return false;
}

/// maxFwdBranchOffset
int64_t ARMGNULDBackend::maxFwdBranchOffset() const {
  if (m_pAttrData->usingThumb2()) {
    return THM2_MAX_FWD_BRANCH_OFFSET;
  } else {
    return THM_MAX_FWD_BRANCH_OFFSET;
  }
}

/// maxBwdBranchOffset
int64_t ARMGNULDBackend::maxBwdBranchOffset() const {
  if (m_pAttrData->usingThumb2()) {
    return THM2_MAX_BWD_BRANCH_OFFSET;
  } else {
    return THM_MAX_BWD_BRANCH_OFFSET;
  }
}

/// doCreateProgramHdrs - backend can implement this function to create the
/// target-dependent segments
void ARMGNULDBackend::doCreateProgramHdrs(Module& pModule) {
  if (m_pEXIDX != NULL && m_pEXIDX->size() != 0x0) {
    // make PT_ARM_EXIDX
    ELFSegment* exidx_seg =
        elfSegmentTable().produce(llvm::ELF::PT_ARM_EXIDX, llvm::ELF::PF_R);
    exidx_seg->append(m_pEXIDX);
  }
}

/// mayHaveUnsafeFunctionPointerAccess - check if the section may have unsafe
/// function pointer access
bool ARMGNULDBackend::mayHaveUnsafeFunctionPointerAccess(
    const LDSection& pSection) const {
  llvm::StringRef name(pSection.name());
  return !name.startswith(".ARM.exidx") && !name.startswith(".ARM.extab") &&
         GNULDBackend::mayHaveUnsafeFunctionPointerAccess(pSection);
}

//===----------------------------------------------------------------------===//
/// createARMLDBackend - the help funtion to create corresponding ARMLDBackend
///
TargetLDBackend* createARMLDBackend(const LinkerConfig& pConfig) {
  if (pConfig.targets().triple().isOSDarwin()) {
    assert(0 && "MachO linker is not supported yet");
    /**
    return new ARMMachOLDBackend(createARMMachOArchiveReader,
                               createARMMachOObjectReader,
                               createARMMachOObjectWriter);
    **/
  }
  if (pConfig.targets().triple().isOSWindows()) {
    assert(0 && "COFF linker is not supported yet");
    /**
    return new ARMCOFFLDBackend(createARMCOFFArchiveReader,
                               createARMCOFFObjectReader,
                               createARMCOFFObjectWriter);
    **/
  }
  return new ARMGNULDBackend(pConfig,
                             new ARMGNUInfo(pConfig.targets().triple()));
}

}  // namespace mcld

//===----------------------------------------------------------------------===//
// Force static initialization.
//===----------------------------------------------------------------------===//
extern "C" void MCLDInitializeARMLDBackend() {
  // Register the linker backend
  mcld::TargetRegistry::RegisterTargetLDBackend(mcld::TheARMTarget,
                                                mcld::createARMLDBackend);
  mcld::TargetRegistry::RegisterTargetLDBackend(mcld::TheThumbTarget,
                                                mcld::createARMLDBackend);
}
