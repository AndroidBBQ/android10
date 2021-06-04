//===- IRBuilder.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/IRBuilder.h"

#include "mcld/Fragment/FragmentRef.h"
#include "mcld/LinkerScript.h"
#include "mcld/LD/DebugString.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/ELFReader.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Support/ELF.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/ADT/StringRef.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Helper Functions
//===----------------------------------------------------------------------===//
LDFileFormat::Kind GetELFSectionKind(uint32_t pType,
                                     const char* pName,
                                     uint32_t pFlag) {
  if (pFlag & llvm::ELF::SHF_EXCLUDE)
    return LDFileFormat::Exclude;

  if (pFlag & llvm::ELF::SHF_MASKPROC)
    return LDFileFormat::Target;

  // name rules
  llvm::StringRef name(pName);
  if (name.startswith(".debug") || name.startswith(".zdebug") ||
      name.startswith(".line") || name.startswith(".stab")) {
    if (name.startswith(".debug_str"))
      return LDFileFormat::DebugString;
    return LDFileFormat::Debug;
  }
  if (name.startswith(".comment"))
    return LDFileFormat::MetaData;
  if (name.startswith(".interp") || name.startswith(".dynamic"))
    return LDFileFormat::Note;
  if (name.startswith(".eh_frame"))
    return LDFileFormat::EhFrame;
  if (name.startswith(".eh_frame_hdr"))
    return LDFileFormat::EhFrameHdr;
  if (name.startswith(".gcc_except_table"))
    return LDFileFormat::GCCExceptTable;
  if (name.startswith(".note.GNU-stack"))
    return LDFileFormat::StackNote;
  if (name.startswith(".gnu.linkonce"))
    return LDFileFormat::LinkOnce;

  // type rules
  switch (pType) {
    case llvm::ELF::SHT_NULL:
      return LDFileFormat::Null;
    case llvm::ELF::SHT_INIT_ARRAY:
    case llvm::ELF::SHT_FINI_ARRAY:
    case llvm::ELF::SHT_PREINIT_ARRAY:
    case llvm::ELF::SHT_PROGBITS: {
      if ((pFlag & llvm::ELF::SHF_EXECINSTR) != 0)
        return LDFileFormat::TEXT;
      else
        return LDFileFormat::DATA;
    }
    case llvm::ELF::SHT_SYMTAB:
    case llvm::ELF::SHT_DYNSYM:
    case llvm::ELF::SHT_STRTAB:
    case llvm::ELF::SHT_HASH:
    case llvm::ELF::SHT_DYNAMIC:
    case llvm::ELF::SHT_SYMTAB_SHNDX:
      return LDFileFormat::NamePool;
    case llvm::ELF::SHT_RELA:
    case llvm::ELF::SHT_REL:
      return LDFileFormat::Relocation;
    case llvm::ELF::SHT_NOBITS:
      return LDFileFormat::BSS;
    case llvm::ELF::SHT_NOTE:
      return LDFileFormat::Note;
    case llvm::ELF::SHT_GROUP:
      return LDFileFormat::Group;
    case llvm::ELF::SHT_GNU_versym:
    case llvm::ELF::SHT_GNU_verdef:
    case llvm::ELF::SHT_GNU_verneed:
      return LDFileFormat::Version;
    case llvm::ELF::SHT_SHLIB:
      return LDFileFormat::Target;
    default:
      if ((pType >= llvm::ELF::SHT_LOPROC && pType <= llvm::ELF::SHT_HIPROC) ||
          (pType >= llvm::ELF::SHT_LOOS && pType <= llvm::ELF::SHT_HIOS) ||
          (pType >= llvm::ELF::SHT_LOUSER && pType <= llvm::ELF::SHT_HIUSER))
        return LDFileFormat::Target;
      fatal(diag::err_unsupported_section) << pName << pType;
  }
  return LDFileFormat::MetaData;
}

//===----------------------------------------------------------------------===//
// IRBuilder
//===----------------------------------------------------------------------===//
IRBuilder::IRBuilder(Module& pModule, const LinkerConfig& pConfig)
    : m_Module(pModule), m_Config(pConfig), m_InputBuilder(pConfig) {
  m_InputBuilder.setCurrentTree(m_Module.getInputTree());

  // FIXME: where to set up Relocation?
  Relocation::SetUp(m_Config);
}

IRBuilder::~IRBuilder() {
}

/// CreateInput - To create an input file and append it to the input tree.
Input* IRBuilder::CreateInput(const std::string& pName,
                              const sys::fs::Path& pPath,
                              Input::Type pType) {
  if (Input::Unknown == pType)
    return ReadInput(pName, pPath);

  m_InputBuilder.createNode<InputTree::Positional>(pName, pPath, pType);
  Input* input = *m_InputBuilder.getCurrentNode();

  if (!input->hasContext())
    m_InputBuilder.setContext(*input, false);

  return input;
}

/// ReadInput - To read an input file and append it to the input tree.
Input* IRBuilder::ReadInput(const std::string& pName,
                            const sys::fs::Path& pPath) {
  m_InputBuilder.createNode<InputTree::Positional>(
      pName, pPath, Input::Unknown);
  Input* input = *m_InputBuilder.getCurrentNode();

  if (!input->hasContext())
    m_InputBuilder.setContext(*input);

  if (!input->hasMemArea())
    m_InputBuilder.setMemory(*input, FileHandle::OpenMode(FileHandle::ReadOnly),
                             FileHandle::Permission(FileHandle::System));

  return input;
}

/// ReadInput - To read an input file and append it to the input tree.
Input* IRBuilder::ReadInput(const std::string& pNameSpec) {
  const sys::fs::Path* path = NULL;
  // find out the real path of the namespec.
  if (m_InputBuilder.getConstraint().isSharedSystem()) {
    // In the system with shared object support, we can find both archive
    // and shared object.

    if (m_InputBuilder.getAttributes().isStatic()) {
      // with --static, we must search an archive.
      path = m_Module.getScript().directories().find(pNameSpec, Input::Archive);
    } else {
      // otherwise, with --Bdynamic, we can find either an archive or a
      // shared object.
      path = m_Module.getScript().directories().find(pNameSpec, Input::DynObj);
    }
  } else {
    // In the system without shared object support, we only look for an archive
    path = m_Module.getScript().directories().find(pNameSpec, Input::Archive);
  }

  if (path == NULL) {
    fatal(diag::err_cannot_find_namespec) << pNameSpec;
    return NULL;
  }

  m_InputBuilder.createNode<InputTree::Positional>(pNameSpec, *path);
  Input* input = *m_InputBuilder.getCurrentNode();

  if (!input->hasContext())
    m_InputBuilder.setContext(*input);

  if (!input->hasMemArea())
    m_InputBuilder.setMemory(*input, FileHandle::OpenMode(FileHandle::ReadOnly),
                             FileHandle::Permission(FileHandle::System));

  return input;
}

/// ReadInput - To read an input file and append it to the input tree.
Input* IRBuilder::ReadInput(FileHandle& pFileHandle) {
  m_InputBuilder.createNode<InputTree::Positional>("file handler",
                                                   pFileHandle.path());

  Input* input = *m_InputBuilder.getCurrentNode();
  if (pFileHandle.path().empty()) {
    m_InputBuilder.setContext(*input, false);
  } else {
    m_InputBuilder.setContext(*input, true);
  }
  m_InputBuilder.setMemory(*input, FileHandle::OpenMode(FileHandle::ReadOnly),
                           FileHandle::Permission(FileHandle::System));

  return input;
}

/// ReadInput - To read an input file and append it to the input tree.
Input* IRBuilder::ReadInput(const std::string& pName,
                            void* pRawMemory,
                            size_t pSize) {
  m_InputBuilder.createNode<InputTree::Positional>(pName, sys::fs::Path("NAN"));
  Input* input = *m_InputBuilder.getCurrentNode();
  m_InputBuilder.setContext(*input, false);
  m_InputBuilder.setMemory(*input, pRawMemory, pSize);
  return input;
}

bool IRBuilder::StartGroup() {
  if (m_InputBuilder.isInGroup()) {
    fatal(diag::fatal_forbid_nest_group);
    return false;
  }
  m_InputBuilder.enterGroup();
  return true;
}

bool IRBuilder::EndGroup() {
  m_InputBuilder.exitGroup();
  return true;
}

void IRBuilder::WholeArchive() {
  m_InputBuilder.getAttributes().setWholeArchive();
}

void IRBuilder::NoWholeArchive() {
  m_InputBuilder.getAttributes().unsetWholeArchive();
}

void IRBuilder::AsNeeded() {
  m_InputBuilder.getAttributes().setAsNeeded();
}

void IRBuilder::NoAsNeeded() {
  m_InputBuilder.getAttributes().unsetAsNeeded();
}

void IRBuilder::CopyDTNeeded() {
  m_InputBuilder.getAttributes().setAddNeeded();
}

void IRBuilder::NoCopyDTNeeded() {
  m_InputBuilder.getAttributes().unsetAddNeeded();
}

void IRBuilder::AgainstShared() {
  m_InputBuilder.getAttributes().setDynamic();
}

void IRBuilder::AgainstStatic() {
  m_InputBuilder.getAttributes().setStatic();
}

LDSection* IRBuilder::CreateELFHeader(Input& pInput,
                                      const std::string& pName,
                                      uint32_t pType,
                                      uint32_t pFlag,
                                      uint32_t pAlign) {
  // Create section header
  LDFileFormat::Kind kind = GetELFSectionKind(pType, pName.c_str(), pFlag);
  LDSection* header = LDSection::Create(pName, kind, pType, pFlag);
  header->setAlign(pAlign);

  // Append section header in input
  pInput.context()->appendSection(*header);
  return header;
}

/// CreateSectionData - To create a section data for given pSection.
SectionData* IRBuilder::CreateSectionData(LDSection& pSection) {
  assert(!pSection.hasSectionData() && "pSection already has section data.");

  SectionData* sect_data = SectionData::Create(pSection);
  pSection.setSectionData(sect_data);
  return sect_data;
}

/// CreateRelocData - To create a relocation data for given pSection.
RelocData* IRBuilder::CreateRelocData(LDSection& pSection) {
  assert(!pSection.hasRelocData() && "pSection already has relocation data.");

  RelocData* reloc_data = RelocData::Create(pSection);
  pSection.setRelocData(reloc_data);
  return reloc_data;
}

/// CreateEhFrame - To create a eh_frame for given pSection
EhFrame* IRBuilder::CreateEhFrame(LDSection& pSection) {
  assert(!pSection.hasEhFrame() && "pSection already has eh_frame.");

  EhFrame* eh_frame = EhFrame::Create(pSection);
  pSection.setEhFrame(eh_frame);
  return eh_frame;
}

/// CreateDebugString - To create a DebugString for given pSection
DebugString* IRBuilder::CreateDebugString(LDSection& pSection) {
  assert(!pSection.hasDebugString() && "pSection already has debug_str.");

  DebugString* debug_str = DebugString::Create(pSection);
  pSection.setDebugString(debug_str);
  return debug_str;
}

/// CreateBSS - To create a bss section for given pSection
SectionData* IRBuilder::CreateBSS(LDSection& pSection) {
  assert(!pSection.hasSectionData() && "pSection already has section data.");
  assert((pSection.kind() == LDFileFormat::BSS) &&
         "pSection is not a BSS section.");

  SectionData* sect_data = SectionData::Create(pSection);
  pSection.setSectionData(sect_data);

  /*  value, valsize, size*/
  FillFragment* frag = new FillFragment(0x0, 1, pSection.size());

  ObjectBuilder::AppendFragment(*frag, *sect_data);
  return sect_data;
}

/// CreateRegion - To create a region fragment in the input file.
Fragment* IRBuilder::CreateRegion(Input& pInput,
                                  size_t pOffset,
                                  size_t pLength) {
  if (!pInput.hasMemArea()) {
    fatal(diag::fatal_cannot_read_input) << pInput.path();
    return NULL;
  }

  if (0 == pLength)
    return new FillFragment(0x0, 0, 0);

  llvm::StringRef region = pInput.memArea()->request(pOffset, pLength);
  return new RegionFragment(region);
}

/// CreateRegion - To create a region fragment wrapping the given memory
Fragment* IRBuilder::CreateRegion(void* pMemory, size_t pLength) {
  if (0 == pLength)
    return new FillFragment(0x0, 0, 0);

  llvm::StringRef region(reinterpret_cast<const char*>(pMemory), pLength);
  return new RegionFragment(region);
}

/// AppendFragment - To append pFrag to the given SectionData pSD
uint64_t IRBuilder::AppendFragment(Fragment& pFrag, SectionData& pSD) {
  uint64_t size =
      ObjectBuilder::AppendFragment(pFrag, pSD, pSD.getSection().align());
  pSD.getSection().setSize(pSD.getSection().size() + size);
  return size;
}

/// AppendRelocation - To append an relocation to the given RelocData pRD.
void IRBuilder::AppendRelocation(Relocation& pRelocation, RelocData& pRD) {
  pRD.append(pRelocation);
}

/// AppendEhFrame - To append a fragment to EhFrame.
uint64_t IRBuilder::AppendEhFrame(Fragment& pFrag, EhFrame& pEhFrame) {
  uint64_t size = ObjectBuilder::AppendFragment(
      pFrag, *pEhFrame.getSectionData(), pEhFrame.getSection().align());
  pEhFrame.getSection().setSize(pEhFrame.getSection().size() + size);
  return size;
}

/// AppendEhFrame - To append a FDE to the given EhFrame pEhFram.
uint64_t IRBuilder::AppendEhFrame(EhFrame::FDE& pFDE, EhFrame& pEhFrame) {
  pEhFrame.addFDE(pFDE);
  pEhFrame.getSection().setSize(pEhFrame.getSection().size() + pFDE.size());
  return pFDE.size();
}

/// AppendEhFrame - To append a CIE to the given EhFrame pEhFram.
uint64_t IRBuilder::AppendEhFrame(EhFrame::CIE& pCIE, EhFrame& pEhFrame) {
  pEhFrame.addCIE(pCIE);
  pEhFrame.getSection().setSize(pEhFrame.getSection().size() + pCIE.size());
  return pCIE.size();
}

/// AddSymbol - To add a symbol in the input file and resolve the symbol
/// immediately
LDSymbol* IRBuilder::AddSymbol(Input& pInput,
                               const std::string& pName,
                               ResolveInfo::Type pType,
                               ResolveInfo::Desc pDesc,
                               ResolveInfo::Binding pBind,
                               ResolveInfo::SizeType pSize,
                               LDSymbol::ValueType pValue,
                               LDSection* pSection,
                               ResolveInfo::Visibility pVis) {
  // rename symbols
  std::string name = pName;
  if (!m_Module.getScript().renameMap().empty() &&
      ResolveInfo::Undefined == pDesc) {
    // If the renameMap is not empty, some symbols should be renamed.
    // --wrap and --portable defines the symbol rename map.
    const LinkerScript& script = m_Module.getScript();
    LinkerScript::SymbolRenameMap::const_iterator renameSym =
        script.renameMap().find(pName);
    if (script.renameMap().end() != renameSym)
      name = renameSym.getEntry()->value();
  }

  // Fix up the visibility if object has no export set.
  if (pInput.noExport() && (pDesc != ResolveInfo::Undefined)) {
    if ((pVis == ResolveInfo::Default) || (pVis == ResolveInfo::Protected)) {
      pVis = ResolveInfo::Hidden;
    }
  }

  switch (pInput.type()) {
    case Input::Object: {
      FragmentRef* frag = NULL;
      if (pSection == NULL || ResolveInfo::Undefined == pDesc ||
          ResolveInfo::Common == pDesc || ResolveInfo::Absolute == pBind ||
          LDFileFormat::Ignore == pSection->kind() ||
          LDFileFormat::Group == pSection->kind())
        frag = FragmentRef::Null();
      else
        frag = FragmentRef::Create(*pSection, pValue);

      LDSymbol* input_sym = addSymbolFromObject(
          name, pType, pDesc, pBind, pSize, pValue, frag, pVis);
      pInput.context()->addSymbol(input_sym);
      return input_sym;
    }
    case Input::DynObj: {
      return addSymbolFromDynObj(
          pInput, name, pType, pDesc, pBind, pSize, pValue, pVis);
    }
    default: {
      return NULL;
      break;
    }
  }
  return NULL;
}

LDSymbol* IRBuilder::addSymbolFromObject(const std::string& pName,
                                         ResolveInfo::Type pType,
                                         ResolveInfo::Desc pDesc,
                                         ResolveInfo::Binding pBinding,
                                         ResolveInfo::SizeType pSize,
                                         LDSymbol::ValueType pValue,
                                         FragmentRef* pFragmentRef,
                                         ResolveInfo::Visibility pVisibility) {
  // Step 1. calculate a Resolver::Result
  // resolved_result is a triple <resolved_info, existent, override>
  Resolver::Result resolved_result;
  ResolveInfo old_info;  // used for arrange output symbols

  if (pBinding == ResolveInfo::Local) {
    // if the symbol is a local symbol, create a LDSymbol for input, but do not
    // resolve them.
    resolved_result.info = m_Module.getNamePool().createSymbol(
        pName, false, pType, pDesc, pBinding, pSize, pVisibility);

    // No matter if there is a symbol with the same name, insert the symbol
    // into output symbol table. So, we let the existent false.
    resolved_result.existent = false;
    resolved_result.overriden = true;
  } else {
    // if the symbol is not local, insert and resolve it immediately
    m_Module.getNamePool().insertSymbol(pName,
                                        false,
                                        pType,
                                        pDesc,
                                        pBinding,
                                        pSize,
                                        pValue,
                                        pVisibility,
                                        &old_info,
                                        resolved_result);
  }

  // the return ResolveInfo should not NULL
  assert(resolved_result.info != NULL);

  /// Step 2. create an input LDSymbol.
  // create a LDSymbol for the input file.
  LDSymbol* input_sym = LDSymbol::Create(*resolved_result.info);
  input_sym->setFragmentRef(pFragmentRef);
  input_sym->setValue(pValue);

  // Step 3. Set up corresponding output LDSymbol
  LDSymbol* output_sym = resolved_result.info->outSymbol();
  bool has_output_sym = (output_sym != NULL);
  if (!resolved_result.existent || !has_output_sym) {
    // it is a new symbol, the output_sym should be NULL.
    assert(output_sym == NULL);

    if (pType == ResolveInfo::Section) {
      // if it is a section symbol, its output LDSymbol is the input LDSymbol.
      output_sym = input_sym;
    } else {
      // if it is a new symbol, create a LDSymbol for the output
      output_sym = LDSymbol::Create(*resolved_result.info);
    }
    resolved_result.info->setSymPtr(output_sym);
  }

  if (resolved_result.overriden || !has_output_sym) {
    // symbol can be overriden only if it exists.
    assert(output_sym != NULL);

    // should override output LDSymbol
    output_sym->setFragmentRef(pFragmentRef);
    output_sym->setValue(pValue);
  }
  return input_sym;
}

LDSymbol* IRBuilder::addSymbolFromDynObj(Input& pInput,
                                         const std::string& pName,
                                         ResolveInfo::Type pType,
                                         ResolveInfo::Desc pDesc,
                                         ResolveInfo::Binding pBinding,
                                         ResolveInfo::SizeType pSize,
                                         LDSymbol::ValueType pValue,
                                         ResolveInfo::Visibility pVisibility) {
  // We don't need sections of dynamic objects. So we ignore section symbols.
  if (pType == ResolveInfo::Section)
    return NULL;

  // ignore symbols with local binding or that have internal or hidden
  // visibility
  if (pBinding == ResolveInfo::Local || pVisibility == ResolveInfo::Internal ||
      pVisibility == ResolveInfo::Hidden)
    return NULL;

  // A protected symbol in a shared library must be treated as a
  // normal symbol when viewed from outside the shared library.
  if (pVisibility == ResolveInfo::Protected)
    pVisibility = ResolveInfo::Default;

  // insert symbol and resolve it immediately
  // resolved_result is a triple <resolved_info, existent, override>
  Resolver::Result resolved_result;
  m_Module.getNamePool().insertSymbol(pName,
                                      true,
                                      pType,
                                      pDesc,
                                      pBinding,
                                      pSize,
                                      pValue,
                                      pVisibility,
                                      NULL,
                                      resolved_result);

  // the return ResolveInfo should not NULL
  assert(resolved_result.info != NULL);

  if (resolved_result.overriden || !resolved_result.existent)
    pInput.setNeeded();

  // create a LDSymbol for the input file.
  LDSymbol* input_sym = LDSymbol::Create(*resolved_result.info);
  input_sym->setFragmentRef(FragmentRef::Null());
  input_sym->setValue(pValue);

  // this symbol is seen in a dynamic object, set the InDyn flag
  resolved_result.info->setInDyn();

  if (!resolved_result.existent) {
    // we get a new symbol, leave it as NULL
    resolved_result.info->setSymPtr(NULL);
  }
  return input_sym;
}

/// AddRelocation - add a relocation entry
///
/// All symbols should be read and resolved before calling this function.
Relocation* IRBuilder::AddRelocation(LDSection& pSection,
                                     Relocation::Type pType,
                                     LDSymbol& pSym,
                                     uint32_t pOffset,
                                     Relocation::Address pAddend) {
  FragmentRef* frag_ref = FragmentRef::Create(*pSection.getLink(), pOffset);

  Relocation* relocation = Relocation::Create(pType, *frag_ref, pAddend);

  relocation->setSymInfo(pSym.resolveInfo());
  pSection.getRelocData()->append(*relocation);

  return relocation;
}

ResolveInfo* IRBuilder::CreateLocalSymbol(FragmentRef& pFragRef) {
  // Create and add symbol to the name pool.
  ResolveInfo* resolveInfo =
      m_Module.getNamePool().createSymbol(/* pName */"",
                                          /* pIsDyn */false,
                                          ResolveInfo::Section,
                                          ResolveInfo::Define,
                                          ResolveInfo::Local,
                                          /* pSize */0,
                                          ResolveInfo::Hidden);
  if (resolveInfo == nullptr) {
    return nullptr;
  }

  // Create input symbol.
  LDSymbol* inputSym = LDSymbol::Create(*resolveInfo);
  if (inputSym == nullptr) {
    return nullptr;
  }

  inputSym->setFragmentRef(FragmentRef::Create(*pFragRef.frag(),
                                               pFragRef.offset()));
  inputSym->setValue(/* pValue */0);

  // The output symbol is simply an alias to the input symbol.
  resolveInfo->setSymPtr(inputSym);

  return resolveInfo;
}

/// AddSymbol - define an output symbol and override it immediately
template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility) {
  ResolveInfo* info = m_Module.getNamePool().findInfo(pName);
  LDSymbol* output_sym = NULL;
  if (info == NULL) {
    // the symbol is not in the pool, create a new one.
    // create a ResolveInfo
    Resolver::Result result;
    m_Module.getNamePool().insertSymbol(pName,
                                        false,
                                        pType,
                                        pDesc,
                                        pBinding,
                                        pSize,
                                        pValue,
                                        pVisibility,
                                        NULL,
                                        result);
    assert(!result.existent);

    // create a output LDSymbol
    output_sym = LDSymbol::Create(*result.info);
    result.info->setSymPtr(output_sym);

    if (result.info->shouldForceLocal(m_Config))
      m_Module.getSymbolTable().forceLocal(*output_sym);
    else
      m_Module.getSymbolTable().add(*output_sym);
  } else {
    // the symbol is already in the pool, override it
    ResolveInfo old_info;
    old_info.override(*info);

    info->setRegular();
    info->setType(pType);
    info->setDesc(pDesc);
    info->setBinding(pBinding);
    info->setVisibility(pVisibility);
    info->setIsSymbol(true);
    info->setSize(pSize);

    output_sym = info->outSymbol();
    if (output_sym != NULL)
      m_Module.getSymbolTable().arrange(*output_sym, old_info);
    else {
      // create a output LDSymbol
      output_sym = LDSymbol::Create(*info);
      info->setSymPtr(output_sym);

      m_Module.getSymbolTable().add(*output_sym);
    }
  }

  if (output_sym != NULL) {
    output_sym->setFragmentRef(pFragmentRef);
    output_sym->setValue(pValue);
  }

  return output_sym;
}

/// AddSymbol - define an output symbol and override it immediately
template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::AsReferred, IRBuilder::Unresolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility) {
  ResolveInfo* info = m_Module.getNamePool().findInfo(pName);

  if (info == NULL || !(info->isUndef() || info->isDyn())) {
    // only undefined symbol and dynamic symbol can make a reference.
    return NULL;
  }

  // the symbol is already in the pool, override it
  ResolveInfo old_info;
  old_info.override(*info);

  info->setRegular();
  info->setType(pType);
  info->setDesc(pDesc);
  info->setBinding(pBinding);
  info->setVisibility(pVisibility);
  info->setIsSymbol(true);
  info->setSize(pSize);

  LDSymbol* output_sym = info->outSymbol();
  if (output_sym != NULL) {
    output_sym->setFragmentRef(pFragmentRef);
    output_sym->setValue(pValue);
    m_Module.getSymbolTable().arrange(*output_sym, old_info);
  } else {
    // create a output LDSymbol
    output_sym = LDSymbol::Create(*info);
    info->setSymPtr(output_sym);

    m_Module.getSymbolTable().add(*output_sym);
  }

  return output_sym;
}

/// AddSymbol - define an output symbol and resolve it
/// immediately
template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility) {
  // Result is <info, existent, override>
  Resolver::Result result;
  ResolveInfo old_info;
  m_Module.getNamePool().insertSymbol(pName,
                                      false,
                                      pType,
                                      pDesc,
                                      pBinding,
                                      pSize,
                                      pValue,
                                      pVisibility,
                                      &old_info,
                                      result);

  LDSymbol* output_sym = result.info->outSymbol();
  bool has_output_sym = (output_sym != NULL);

  if (!result.existent || !has_output_sym) {
    output_sym = LDSymbol::Create(*result.info);
    result.info->setSymPtr(output_sym);
  }

  if (result.overriden || !has_output_sym) {
    output_sym->setFragmentRef(pFragmentRef);
    output_sym->setValue(pValue);
  }

  // After symbol resolution, the visibility is changed to the most restrict.
  // arrange the output position
  if (result.info->shouldForceLocal(m_Config))
    m_Module.getSymbolTable().forceLocal(*output_sym);
  else if (has_output_sym)
    m_Module.getSymbolTable().arrange(*output_sym, old_info);
  else
    m_Module.getSymbolTable().add(*output_sym);

  return output_sym;
}

/// defineSymbol - define an output symbol and resolve it immediately.
template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility) {
  ResolveInfo* info = m_Module.getNamePool().findInfo(pName);

  if (info == NULL || !(info->isUndef() || info->isDyn())) {
    // only undefined symbol and dynamic symbol can make a reference.
    return NULL;
  }

  return AddSymbol<Force, Resolve>(
      pName, pType, pDesc, pBinding, pSize, pValue, pFragmentRef, pVisibility);
}

}  // namespace mcld
