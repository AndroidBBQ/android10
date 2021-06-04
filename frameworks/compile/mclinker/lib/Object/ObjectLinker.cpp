//===- ObjectLinker.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Object/ObjectLinker.h"

#include "mcld/InputTree.h"
#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/Archive.h"
#include "mcld/LD/ArchiveReader.h"
#include "mcld/LD/BinaryReader.h"
#include "mcld/LD/BranchIslandFactory.h"
#include "mcld/LD/DebugString.h"
#include "mcld/LD/DynObjReader.h"
#include "mcld/LD/GarbageCollection.h"
#include "mcld/LD/GroupReader.h"
#include "mcld/LD/IdenticalCodeFolding.h"
#include "mcld/LD/LDContext.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/ObjectReader.h"
#include "mcld/LD/ObjectWriter.h"
#include "mcld/LD/Relocator.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Object/ObjectBuilder.h"
#include "mcld/Script/Assignment.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/RpnEvaluator.h"
#include "mcld/Script/ScriptFile.h"
#include "mcld/Script/ScriptReader.h"
#include "mcld/Support/FileOutputBuffer.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/RealPath.h"
#include "mcld/Target/TargetLDBackend.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/Host.h>

#include <system_error>

namespace mcld {

//===----------------------------------------------------------------------===//
// ObjectLinker
//===----------------------------------------------------------------------===//
ObjectLinker::ObjectLinker(const LinkerConfig& pConfig,
                           TargetLDBackend& pLDBackend)
    : m_Config(pConfig),
      m_pModule(NULL),
      m_pBuilder(NULL),
      m_LDBackend(pLDBackend),
      m_pObjectReader(NULL),
      m_pDynObjReader(NULL),
      m_pArchiveReader(NULL),
      m_pGroupReader(NULL),
      m_pBinaryReader(NULL),
      m_pScriptReader(NULL),
      m_pWriter(NULL) {
}

ObjectLinker::~ObjectLinker() {
  delete m_pObjectReader;
  delete m_pDynObjReader;
  delete m_pArchiveReader;
  delete m_pGroupReader;
  delete m_pBinaryReader;
  delete m_pScriptReader;
  delete m_pWriter;
}

bool ObjectLinker::initialize(Module& pModule, IRBuilder& pBuilder) {
  m_pModule = &pModule;
  m_pBuilder = &pBuilder;

  // initialize the readers and writers
  m_pObjectReader = m_LDBackend.createObjectReader(*m_pBuilder);
  m_pArchiveReader = m_LDBackend.createArchiveReader(*m_pModule);
  m_pDynObjReader = m_LDBackend.createDynObjReader(*m_pBuilder);
  m_pBinaryReader = m_LDBackend.createBinaryReader(*m_pBuilder);
  m_pGroupReader = new GroupReader(*m_pModule,
                                   *m_pObjectReader,
                                   *m_pDynObjReader,
                                   *m_pArchiveReader,
                                   *m_pBinaryReader);
  m_pScriptReader = new ScriptReader(
      *m_pObjectReader, *m_pArchiveReader, *m_pDynObjReader, *m_pGroupReader);
  m_pWriter = m_LDBackend.createWriter();

  // initialize Relocator
  m_LDBackend.initRelocator();

  return true;
}

/// initStdSections - initialize standard sections
bool ObjectLinker::initStdSections() {
  ObjectBuilder builder(*m_pModule);

  // initialize standard sections
  if (!m_LDBackend.initStdSections(builder))
    return false;

  // initialize target-dependent sections
  m_LDBackend.initTargetSections(*m_pModule, builder);

  return true;
}

void ObjectLinker::addUndefinedSymbols() {
  // Add the symbol set by -u as an undefind global symbol into symbol pool
  GeneralOptions::const_undef_sym_iterator usym;
  GeneralOptions::const_undef_sym_iterator usymEnd =
      m_Config.options().undef_sym_end();
  for (usym = m_Config.options().undef_sym_begin(); usym != usymEnd; ++usym) {
    Resolver::Result result;
    m_pModule->getNamePool().insertSymbol(*usym,  // name
                                          false,  // isDyn
                                          ResolveInfo::NoType,
                                          ResolveInfo::Undefined,
                                          ResolveInfo::Global,
                                          0x0,  // size
                                          0x0,  // value
                                          ResolveInfo::Default,
                                          NULL,
                                          result);

    LDSymbol* output_sym = result.info->outSymbol();
    // create the output symbol if it dose not have one
    if (!result.existent || (output_sym != NULL)) {
      output_sym = LDSymbol::Create(*result.info);
      result.info->setSymPtr(output_sym);
      output_sym->setFragmentRef(FragmentRef::Null());
    }
  }
}

void ObjectLinker::normalize() {
  // -----  set up inputs  ----- //
  Module::input_iterator input, inEnd = m_pModule->input_end();
  for (input = m_pModule->input_begin(); input != inEnd; ++input) {
    // is a group node
    if (isGroup(input)) {
      getGroupReader()->readGroup(
          input, inEnd, m_pBuilder->getInputBuilder(), m_Config);
      continue;
    }

    // already got type - for example, bitcode or external OIR (object
    // intermediate representation)
    if ((*input)->type() == Input::Script ||
        (*input)->type() == Input::Archive ||
        (*input)->type() == Input::External)
      continue;

    if (Input::Object == (*input)->type()) {
      m_pModule->getObjectList().push_back(*input);
      continue;
    }

    if (Input::DynObj == (*input)->type()) {
      m_pModule->getLibraryList().push_back(*input);
      continue;
    }

    bool doContinue = false;
    // read input as a binary file
    if (getBinaryReader()->isMyFormat(**input, doContinue)) {
      (*input)->setType(Input::Object);
      getBinaryReader()->readBinary(**input);
      m_pModule->getObjectList().push_back(*input);
    } else if (doContinue &&
               getObjectReader()->isMyFormat(**input, doContinue)) {
      // is a relocatable object file
      (*input)->setType(Input::Object);
      getObjectReader()->readHeader(**input);
      getObjectReader()->readSections(**input);
      getObjectReader()->readSymbols(**input);
      m_pModule->getObjectList().push_back(*input);
    } else if (doContinue &&
               getDynObjReader()->isMyFormat(**input, doContinue)) {
      // is a shared object file
      (*input)->setType(Input::DynObj);
      getDynObjReader()->readHeader(**input);
      getDynObjReader()->readSymbols(**input);
      m_pModule->getLibraryList().push_back(*input);
    } else if (doContinue &&
               getArchiveReader()->isMyFormat(**input, doContinue)) {
      // is an archive
      (*input)->setType(Input::Archive);
      if (m_Config.options().isInExcludeLIBS(**input)) {
        (*input)->setNoExport();
      }
      Archive archive(**input, m_pBuilder->getInputBuilder());
      getArchiveReader()->readArchive(m_Config, archive);
      if (archive.numOfObjectMember() > 0) {
        m_pModule->getInputTree().merge<InputTree::Inclusive>(input,
                                                              archive.inputs());
      }
    } else if (doContinue &&
               getScriptReader()->isMyFormat(**input, doContinue)) {
      // try to parse input as a linker script
      ScriptFile script(
          ScriptFile::LDScript, **input, m_pBuilder->getInputBuilder());
      if (getScriptReader()->readScript(m_Config, script)) {
        (*input)->setType(Input::Script);
        script.activate(*m_pModule);
        if (script.inputs().size() > 0) {
          m_pModule->getInputTree().merge<InputTree::Inclusive>(
              input, script.inputs());
        }
      }
    } else {
      if (m_Config.options().warnMismatch())
        warning(diag::warn_unrecognized_input_file)
            << (*input)->path() << m_Config.targets().triple().str();
    }
  }  // end of for
}

bool ObjectLinker::linkable() const {
  // check we have input and output files
  if (m_pModule->getInputTree().empty()) {
    error(diag::err_no_inputs);
    return false;
  }

  // can not mix -static with shared objects
  Module::const_lib_iterator lib, libEnd = m_pModule->lib_end();
  for (lib = m_pModule->lib_begin(); lib != libEnd; ++lib) {
    if ((*lib)->attribute()->isStatic()) {
      error(diag::err_mixed_shared_static_objects) << (*lib)->name()
                                                   << (*lib)->path();
      return false;
    }
  }

  // --nmagic and --omagic options lead to static executable program.
  // These options turn off page alignment of sections. Because the
  // sections are not aligned to pages, these sections can not contain any
  // exported functions. Also, because the two options disable linking
  // against shared libraries, the output absolutely does not call outside
  // functions.
  if (m_Config.options().nmagic() && !m_Config.isCodeStatic()) {
    error(diag::err_nmagic_not_static);
    return false;
  }
  if (m_Config.options().omagic() && !m_Config.isCodeStatic()) {
    error(diag::err_omagic_not_static);
    return false;
  }

  return true;
}

void ObjectLinker::dataStrippingOpt() {
  if (m_Config.codeGenType() == LinkerConfig::Object) {
    return;
  }

  // Garbege collection
  if (m_Config.options().GCSections()) {
    GarbageCollection GC(m_Config, m_LDBackend, *m_pModule);
    GC.run();
  }

  // Identical code folding
  if (m_Config.options().getICFMode() != GeneralOptions::ICF::None) {
    IdenticalCodeFolding icf(m_Config, m_LDBackend, *m_pModule);
    icf.foldIdenticalCode();
  }
  return;
}

/// readRelocations - read all relocation entries
///
/// All symbols should be read and resolved before this function.
bool ObjectLinker::readRelocations() {
  // Bitcode is read by the other path. This function reads relocation sections
  // in object files.
  mcld::InputTree::bfs_iterator input,
      inEnd = m_pModule->getInputTree().bfs_end();
  for (input = m_pModule->getInputTree().bfs_begin(); input != inEnd; ++input) {
    if ((*input)->type() == Input::Object && (*input)->hasMemArea()) {
      if (!getObjectReader()->readRelocations(**input))
        return false;
    }
    // ignore the other kinds of files.
  }
  return true;
}

/// mergeSections - put allinput sections into output sections
bool ObjectLinker::mergeSections() {
  // run the target-dependent hooks before merging sections
  m_LDBackend.preMergeSections(*m_pModule);

  // Set up input/output from ldscript requirement if any
  {
    RpnEvaluator evaluator(*m_pModule, m_LDBackend);
    SectionMap::iterator out, outBegin, outEnd;
    outBegin = m_pModule->getScript().sectionMap().begin();
    outEnd = m_pModule->getScript().sectionMap().end();
    for (out = outBegin; out != outEnd; ++out) {
      uint64_t out_align = 0x0, in_align = 0x0;
      LDSection* out_sect = (*out)->getSection();
      SectionMap::Output::iterator in, inBegin, inEnd;
      inBegin = (*out)->begin();
      inEnd = (*out)->end();

      // force input alignment from ldscript if any
      if ((*out)->prolog().hasSubAlign()) {
        evaluator.eval((*out)->prolog().subAlign(), in_align);
      }

      for (in = inBegin; in != inEnd; ++in) {
        LDSection* in_sect = (*in)->getSection();
        if ((*out)->prolog().hasSubAlign())
          in_sect->setAlign(in_align);
      }  // for each input section description

      // force output alignment from ldscript if any
      if ((*out)->prolog().hasAlign()) {
        evaluator.eval((*out)->prolog().align(), out_align);
        out_sect->setAlign(out_align);
      }
    }  // for each output section description
  }

  ObjectBuilder builder(*m_pModule);
  Module::obj_iterator obj, objEnd = m_pModule->obj_end();
  for (obj = m_pModule->obj_begin(); obj != objEnd; ++obj) {
    LDContext::sect_iterator sect, sectEnd = (*obj)->context()->sectEnd();
    for (sect = (*obj)->context()->sectBegin(); sect != sectEnd; ++sect) {
      switch ((*sect)->kind()) {
        // Some *INPUT sections should not be merged.
        case LDFileFormat::Folded:
        case LDFileFormat::Ignore:
        case LDFileFormat::Null:
        case LDFileFormat::NamePool:
        case LDFileFormat::Group:
        case LDFileFormat::StackNote:
          // skip
          continue;
        case LDFileFormat::Relocation:
          if (!(*sect)->hasRelocData())
            continue;  // skip

          if ((*sect)->getLink()->kind() == LDFileFormat::Ignore ||
              (*sect)->getLink()->kind() == LDFileFormat::Folded)
            (*sect)->setKind(LDFileFormat::Ignore);
          break;
        case LDFileFormat::Target:
          if (!m_LDBackend.mergeSection(*m_pModule, **obj, **sect)) {
            error(diag::err_cannot_merge_section) << (*sect)->name()
                                                  << (*obj)->name();
            return false;
          }
          break;
        case LDFileFormat::EhFrame: {
          if (!(*sect)->hasEhFrame())
            continue;  // skip

          LDSection* out_sect = NULL;
          if ((out_sect = builder.MergeSection(**obj, **sect)) != NULL) {
            if (!m_LDBackend.updateSectionFlags(*out_sect, **sect)) {
              error(diag::err_cannot_merge_section) << (*sect)->name()
                                                    << (*obj)->name();
              return false;
            }
          }
          break;
        }
        case LDFileFormat::DebugString: {
          // FIXME: disable debug string merge when doing partial link.
          if (LinkerConfig::Object == m_Config.codeGenType())
            (*sect)->setKind(LDFileFormat::Debug);
        }
        // Fall through
        default: {
          if (!(*sect)->hasSectionData())
            continue;  // skip

          LDSection* out_sect = NULL;
          if ((out_sect = builder.MergeSection(**obj, **sect)) != NULL) {
            if (!m_LDBackend.updateSectionFlags(*out_sect, **sect)) {
              error(diag::err_cannot_merge_section) << (*sect)->name()
                                                    << (*obj)->name();
              return false;
            }
          }
          break;
        }
      }  // end of switch
    }    // for each section
  }      // for each obj

  {
    SectionMap::iterator out, outBegin, outEnd;
    outBegin = m_pModule->getScript().sectionMap().begin();
    outEnd = m_pModule->getScript().sectionMap().end();
    for (out = outBegin; out != outEnd; ++out) {
      LDSection* out_sect = (*out)->getSection();
      SectionMap::Output::iterator in, inBegin, inEnd;
      inBegin = (*out)->begin();
      inEnd = (*out)->end();

      for (in = inBegin; in != inEnd; ++in) {
        LDSection* in_sect = (*in)->getSection();
        if (builder.MoveSectionData(*in_sect->getSectionData(),
                                    *out_sect->getSectionData())) {
          builder.UpdateSectionAlign(*out_sect, *in_sect);
          m_LDBackend.updateSectionFlags(*out_sect, *in_sect);
        }
      }  // for each input section description

      if ((*out)->hasContent()) {
        LDSection* target = m_pModule->getSection((*out)->name());
        assert(target != NULL && target->hasSectionData());
        if (builder.MoveSectionData(*out_sect->getSectionData(),
                                    *target->getSectionData())) {
          builder.UpdateSectionAlign(*target, *out_sect);
          m_LDBackend.updateSectionFlags(*target, *out_sect);
        }
      }
    }  // for each output section description
  }

  // run the target-dependent hooks after merging sections
  m_LDBackend.postMergeSections(*m_pModule);

  return true;
}

void ObjectLinker::addSymbolToOutput(ResolveInfo& pInfo, Module& pModule) {
  // section symbols will be defined by linker later, we should not add section
  // symbols to output here
  if (ResolveInfo::Section == pInfo.type() || pInfo.outSymbol() == NULL)
    return;

  // if the symbols defined in the Ignore sections (e.g. discared by GC), then
  // not to put them to output
  // make sure that symbols defined in .debug_str won't add into output
  // symbol table. Since these symbols has fragRef to input fragments, which
  // will refer to input LDSection and has bad result when emitting their
  // section index. However, .debug_str actually does not need symobl in
  // shrad/executable objects, so it's fine to do so.
  if (pInfo.outSymbol()->hasFragRef() &&
      (LDFileFormat::Ignore ==
           pInfo.outSymbol()
               ->fragRef()
               ->frag()
               ->getParent()
               ->getSection()
               .kind() ||
       LDFileFormat::DebugString ==
           pInfo.outSymbol()
               ->fragRef()
               ->frag()
               ->getParent()
               ->getSection()
               .kind()))
    return;

  if (pInfo.shouldForceLocal(m_Config))
    pModule.getSymbolTable().forceLocal(*pInfo.outSymbol());
  else
    pModule.getSymbolTable().add(*pInfo.outSymbol());
}

void ObjectLinker::addSymbolsToOutput(Module& pModule) {
  // Traverse all the free ResolveInfo and add the output symobols to output
  NamePool::freeinfo_iterator free_it,
      free_end = pModule.getNamePool().freeinfo_end();
  for (free_it = pModule.getNamePool().freeinfo_begin(); free_it != free_end;
       ++free_it)
    addSymbolToOutput(**free_it, pModule);

  // Traverse all the resolveInfo and add the output symbol to output
  NamePool::syminfo_iterator info_it,
      info_end = pModule.getNamePool().syminfo_end();
  for (info_it = pModule.getNamePool().syminfo_begin(); info_it != info_end;
       ++info_it)
    addSymbolToOutput(*info_it.getEntry(), pModule);
}

/// addStandardSymbols - shared object and executable files need some
/// standard symbols
///   @return if there are some input symbols with the same name to the
///   standard symbols, return false
bool ObjectLinker::addStandardSymbols() {
  // create and add section symbols for each output section
  Module::iterator iter, iterEnd = m_pModule->end();
  for (iter = m_pModule->begin(); iter != iterEnd; ++iter) {
    m_pModule->getSectionSymbolSet().add(**iter, m_pModule->getNamePool());
  }

  return m_LDBackend.initStandardSymbols(*m_pBuilder, *m_pModule);
}

/// addTargetSymbols - some targets, such as MIPS and ARM, need some
/// target-dependent symbols
///   @return if there are some input symbols with the same name to the
///   target symbols, return false
bool ObjectLinker::addTargetSymbols() {
  m_LDBackend.initTargetSymbols(*m_pBuilder, *m_pModule);
  return true;
}

/// addScriptSymbols - define symbols from the command line option or linker
/// scripts.
bool ObjectLinker::addScriptSymbols() {
  LinkerScript& script = m_pModule->getScript();
  LinkerScript::Assignments::iterator it, ie = script.assignments().end();
  // go through the entire symbol assignments
  for (it = script.assignments().begin(); it != ie; ++it) {
    LDSymbol* symbol = NULL;
    assert((*it).second.symbol().type() == Operand::SYMBOL);
    const llvm::StringRef symName = (*it).second.symbol().name();
    ResolveInfo::Type type = ResolveInfo::NoType;
    ResolveInfo::Visibility vis = ResolveInfo::Default;
    size_t size = 0;
    ResolveInfo* old_info = m_pModule->getNamePool().findInfo(symName);
    // if the symbol does not exist, we can set type to NOTYPE
    // else we retain its type, same goes for size - 0 or retain old value
    // and visibility - Default or retain
    if (old_info != NULL) {
      type = static_cast<ResolveInfo::Type>(old_info->type());
      vis = old_info->visibility();
      size = old_info->size();
    }

    // Add symbol and refine the visibility if needed
    // FIXME: bfd linker would change the binding instead, but currently
    //        ABS is also a kind of Binding in ResolveInfo.
    switch ((*it).second.type()) {
      case Assignment::HIDDEN:
        vis = ResolveInfo::Hidden;
      // Fall through
      case Assignment::DEFAULT:
        symbol = m_pBuilder->AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
            symName,
            type,
            ResolveInfo::Define,
            ResolveInfo::Absolute,
            size,
            0x0,
            FragmentRef::Null(),
            vis);
        break;
      case Assignment::PROVIDE_HIDDEN:
        vis = ResolveInfo::Hidden;
      // Fall through
      case Assignment::PROVIDE:
        symbol =
            m_pBuilder->AddSymbol<IRBuilder::AsReferred, IRBuilder::Unresolve>(
                symName,
                type,
                ResolveInfo::Define,
                ResolveInfo::Absolute,
                size,
                0x0,
                FragmentRef::Null(),
                vis);
        break;
    }
    // Set symbol of this assignment.
    (*it).first = symbol;
  }
  return true;
}

bool ObjectLinker::scanRelocations() {
  // apply all relocations of all inputs
  Module::obj_iterator input, inEnd = m_pModule->obj_end();
  for (input = m_pModule->obj_begin(); input != inEnd; ++input) {
    m_LDBackend.getRelocator()->initializeScan(**input);
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      // bypass the reloc section if
      // 1. its section kind is changed to Ignore. (The target section is a
      // discarded group section.)
      // 2. it has no reloc data. (All symbols in the input relocs are in the
      // discarded group sections)
      if (LDFileFormat::Ignore == (*rs)->kind() || !(*rs)->hasRelocData())
        continue;
      RelocData::iterator reloc, rEnd = (*rs)->getRelocData()->end();
      for (reloc = (*rs)->getRelocData()->begin(); reloc != rEnd; ++reloc) {
        Relocation* relocation = llvm::cast<Relocation>(reloc);

        // bypass the reloc if the symbol is in the discarded input section
        ResolveInfo* info = relocation->symInfo();
        if (!info->outSymbol()->hasFragRef() &&
            ResolveInfo::Section == info->type() &&
            ResolveInfo::Undefined == info->desc())
          continue;

        // scan relocation
        if (LinkerConfig::Object != m_Config.codeGenType()) {
          m_LDBackend.getRelocator()->scanRelocation(
              *relocation, *m_pBuilder, *m_pModule, **rs, **input);
        } else {
          m_LDBackend.getRelocator()->partialScanRelocation(
              *relocation, *m_pModule);
        }
      }  // for all relocations
    }    // for all relocation section
    m_LDBackend.getRelocator()->finalizeScan(**input);
  }  // for all inputs
  return true;
}

/// initStubs - initialize stub-related stuff.
bool ObjectLinker::initStubs() {
  // initialize BranchIslandFactory
  m_LDBackend.initBRIslandFactory();

  // initialize StubFactory
  m_LDBackend.initStubFactory();

  // initialize target stubs
  m_LDBackend.initTargetStubs();
  return true;
}

/// allocateCommonSymobols - allocate fragments for common symbols to the
/// corresponding sections
bool ObjectLinker::allocateCommonSymbols() {
  if (LinkerConfig::Object != m_Config.codeGenType() ||
      m_Config.options().isDefineCommon())
    return m_LDBackend.allocateCommonSymbols(*m_pModule);
  return true;
}

/// prelayout - help backend to do some modification before layout
bool ObjectLinker::prelayout() {
  // finalize the section symbols, set their fragment reference and push them
  // into output symbol table
  Module::iterator sect, sEnd = m_pModule->end();
  for (sect = m_pModule->begin(); sect != sEnd; ++sect) {
    m_pModule->getSectionSymbolSet().finalize(
        **sect,
        m_pModule->getSymbolTable(),
        m_Config.codeGenType() == LinkerConfig::Object);
  }

  m_LDBackend.preLayout(*m_pModule, *m_pBuilder);

  /// check program interpreter - computer the name size of the runtime dyld
  if (!m_Config.isCodeStatic() &&
      (LinkerConfig::Exec == m_Config.codeGenType() ||
       m_Config.options().isPIE() || m_Config.options().hasDyld()))
    m_LDBackend.sizeInterp();

  /// measure NamePools - compute the size of name pool sections
  /// In ELF, will compute  the size of.symtab, .strtab, .dynsym, .dynstr,
  /// .hash and .shstrtab sections.
  ///
  /// dump all symbols and strings from ObjectLinker and build the
  /// format-dependent
  /// hash table.
  /// @note sizeNamePools replies on LinkerConfig::CodePosition. Must determine
  /// code position model before calling GNULDBackend::sizeNamePools()
  m_LDBackend.sizeNamePools(*m_pModule);

  // Do this after backend prelayout since it may add eh_frame entries.
  LDSection* eh_frame_sect = m_pModule->getSection(".eh_frame");
  if (eh_frame_sect && eh_frame_sect->hasEhFrame())
    eh_frame_sect->getEhFrame()->computeOffsetSize();
  m_LDBackend.createAndSizeEhFrameHdr(*m_pModule);

  // size debug string table and set up the debug string offset
  // we set the .debug_str size here so that there won't be a section symbol for
  // .debug_str. While actually it doesn't matter that .debug_str has section
  // symbol or not.
  // FIXME: disable debug string merge when doing partial link.
  if (LinkerConfig::Object != m_Config.codeGenType()) {
    LDSection* debug_str_sect = m_pModule->getSection(".debug_str");
    if (debug_str_sect && debug_str_sect->hasDebugString())
      debug_str_sect->getDebugString()->computeOffsetSize();
  }
  return true;
}

/// layout - linearly layout all output sections and reserve some space
/// for GOT/PLT
///   Because we do not support instruction relaxing in this early version,
///   if there is a branch can not jump to its target, we return false
///   directly
bool ObjectLinker::layout() {
  m_LDBackend.layout(*m_pModule);
  return true;
}

/// prelayout - help backend to do some modification after layout
bool ObjectLinker::postlayout() {
  m_LDBackend.postLayout(*m_pModule, *m_pBuilder);
  return true;
}

/// finalizeSymbolValue - finalize the resolved symbol value.
///   Before relocate(), after layout(), ObjectLinker should correct value of
///   all
///   symbol.
bool ObjectLinker::finalizeSymbolValue() {
  Module::sym_iterator symbol, symEnd = m_pModule->sym_end();
  for (symbol = m_pModule->sym_begin(); symbol != symEnd; ++symbol) {
    if ((*symbol)->resolveInfo()->isAbsolute() ||
        (*symbol)->resolveInfo()->type() == ResolveInfo::File) {
      // absolute symbols should just use its value directly (i.e., the result
      // of symbol resolution)
      continue;
    }

    if ((*symbol)->resolveInfo()->type() == ResolveInfo::ThreadLocal) {
      m_LDBackend.finalizeTLSSymbol(**symbol);
      continue;
    }

    if ((*symbol)->hasFragRef()) {
      // set the virtual address of the symbol. If the output file is
      // relocatable object file, the section's virtual address becomes zero.
      // And the symbol's value become section relative offset.
      uint64_t value = (*symbol)->fragRef()->getOutputOffset();
      assert((*symbol)->fragRef()->frag() != NULL);
      uint64_t addr =
          (*symbol)->fragRef()->frag()->getParent()->getSection().addr();
      (*symbol)->setValue(value + addr);
      continue;
    }
  }

  RpnEvaluator evaluator(*m_pModule, m_LDBackend);
  bool finalized = m_LDBackend.finalizeSymbols();
  bool scriptSymsFinalized = true;
  LinkerScript& script = m_pModule->getScript();
  LinkerScript::Assignments::iterator assign, assignEnd;
  assignEnd = script.assignments().end();
  for (assign = script.assignments().begin(); assign != assignEnd; ++assign) {
    LDSymbol* symbol = (*assign).first;
    Assignment& assignment = (*assign).second;

    if (symbol == NULL)
      continue;

    scriptSymsFinalized &= assignment.assign(evaluator);
    if (!scriptSymsFinalized)
      break;

    symbol->setValue(assignment.symbol().value());
  }  // for each script symbol assignment

  bool assertionsPassed = true;
  LinkerScript::Assertions::iterator assert, assertEnd;
  assertEnd = script.assertions().end();
  for (assert = script.assertions().begin(); assert != assertEnd; ++assert) {
    uint64_t res = 0x0;
    evaluator.eval((*assert).getRpnExpr(), res);
    if (res == 0x0)
      fatal(diag::err_assert_failed) << (*assert).message();
  }  // for each assertion in ldscript

  return finalized && scriptSymsFinalized && assertionsPassed;
}

/// relocate - applying relocation entries and create relocation
/// section in the output files
/// Create relocation section, asking TargetLDBackend to
/// read the relocation information into RelocationEntry
/// and push_back into the relocation section
bool ObjectLinker::relocation() {
  // when producing relocatables, no need to apply relocation
  if (LinkerConfig::Object == m_Config.codeGenType())
    return true;

  LDSection* debug_str_sect = m_pModule->getSection(".debug_str");

  // apply all relocations of all inputs
  Module::obj_iterator input, inEnd = m_pModule->obj_end();
  for (input = m_pModule->obj_begin(); input != inEnd; ++input) {
    m_LDBackend.getRelocator()->initializeApply(**input);
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      // bypass the reloc section if
      // 1. its section kind is changed to Ignore. (The target section is a
      // discarded group section.)
      // 2. it has no reloc data. (All symbols in the input relocs are in the
      // discarded group sections)
      if (LDFileFormat::Ignore == (*rs)->kind() || !(*rs)->hasRelocData())
        continue;
      RelocData::iterator reloc, rEnd = (*rs)->getRelocData()->end();
      for (reloc = (*rs)->getRelocData()->begin(); reloc != rEnd; ++reloc) {
        Relocation* relocation = llvm::cast<Relocation>(reloc);

        // bypass the reloc if the symbol is in the discarded input section
        ResolveInfo* info = relocation->symInfo();
        if (!info->outSymbol()->hasFragRef() &&
            ResolveInfo::Section == info->type() &&
            ResolveInfo::Undefined == info->desc())
          continue;

        // apply the relocation aginst symbol on DebugString
        if (info->outSymbol()->hasFragRef() &&
            info->outSymbol()->fragRef()->frag()->getKind()
                == Fragment::Region &&
            info->outSymbol()->fragRef()->frag()->getParent()->getSection()
                .kind() == LDFileFormat::DebugString) {
          assert(debug_str_sect != NULL);
          assert(debug_str_sect->hasDebugString());
          debug_str_sect->getDebugString()->applyOffset(*relocation,
                                                        m_LDBackend);
          continue;
        }

        relocation->apply(*m_LDBackend.getRelocator());
      }  // for all relocations
    }    // for all relocation section
    m_LDBackend.getRelocator()->finalizeApply(**input);
  }  // for all inputs

  // apply relocations created by relaxation
  BranchIslandFactory* br_factory = m_LDBackend.getBRIslandFactory();
  BranchIslandFactory::iterator facIter, facEnd = br_factory->end();
  for (facIter = br_factory->begin(); facIter != facEnd; ++facIter) {
    BranchIsland& island = *facIter;
    BranchIsland::reloc_iterator iter, iterEnd = island.reloc_end();
    for (iter = island.reloc_begin(); iter != iterEnd; ++iter)
      (*iter)->apply(*m_LDBackend.getRelocator());
  }

  // apply relocations created by LD backend
  for (TargetLDBackend::extra_reloc_iterator
       iter = m_LDBackend.extra_reloc_begin(),
       end = m_LDBackend.extra_reloc_end(); iter != end; ++iter) {
    iter->apply(*m_LDBackend.getRelocator());
  }

  return true;
}

/// emitOutput - emit the output file.
bool ObjectLinker::emitOutput(FileOutputBuffer& pOutput) {
  return std::error_code() == getWriter()->writeObject(*m_pModule, pOutput);
}

/// postProcessing - do modification after all processes
bool ObjectLinker::postProcessing(FileOutputBuffer& pOutput) {
  if (LinkerConfig::Object != m_Config.codeGenType())
    normalSyncRelocationResult(pOutput);
  else
    partialSyncRelocationResult(pOutput);

  // emit .eh_frame_hdr
  // eh_frame_hdr should be emitted after syncRelocation, because eh_frame_hdr
  // needs FDE PC value, which will be corrected at syncRelocation
  m_LDBackend.postProcessing(pOutput);
  return true;
}

void ObjectLinker::normalSyncRelocationResult(FileOutputBuffer& pOutput) {
  uint8_t* data = pOutput.getBufferStart();

  // sync all relocations of all inputs
  Module::obj_iterator input, inEnd = m_pModule->obj_end();
  for (input = m_pModule->obj_begin(); input != inEnd; ++input) {
    LDContext::sect_iterator rs, rsEnd = (*input)->context()->relocSectEnd();
    for (rs = (*input)->context()->relocSectBegin(); rs != rsEnd; ++rs) {
      // bypass the reloc section if
      // 1. its section kind is changed to Ignore. (The target section is a
      // discarded group section.)
      // 2. it has no reloc data. (All symbols in the input relocs are in the
      // discarded group sections)
      if (LDFileFormat::Ignore == (*rs)->kind() || !(*rs)->hasRelocData())
        continue;
      RelocData::iterator reloc, rEnd = (*rs)->getRelocData()->end();
      for (reloc = (*rs)->getRelocData()->begin(); reloc != rEnd; ++reloc) {
        Relocation* relocation = llvm::cast<Relocation>(reloc);

        // bypass the reloc if the symbol is in the discarded input section
        ResolveInfo* info = relocation->symInfo();
        if (!info->outSymbol()->hasFragRef() &&
            ResolveInfo::Section == info->type() &&
            ResolveInfo::Undefined == info->desc())
          continue;

        // bypass the relocation with NONE type. This is to avoid overwrite the
        // target result by NONE type relocation if there is a place which has
        // two relocations to apply to, and one of it is NONE type. The result
        // we want is the value of the other relocation result. For example,
        // in .exidx, there are usually an R_ARM_NONE and R_ARM_PREL31 apply to
        // the same place
        if (relocation->type() == 0x0)
          continue;
        writeRelocationResult(*relocation, data);
      }  // for all relocations
    }    // for all relocation section
  }      // for all inputs

  // sync relocations created by relaxation
  BranchIslandFactory* br_factory = m_LDBackend.getBRIslandFactory();
  BranchIslandFactory::iterator facIter, facEnd = br_factory->end();
  for (facIter = br_factory->begin(); facIter != facEnd; ++facIter) {
    BranchIsland& island = *facIter;
    BranchIsland::reloc_iterator iter, iterEnd = island.reloc_end();
    for (iter = island.reloc_begin(); iter != iterEnd; ++iter) {
      Relocation* reloc = *iter;
      writeRelocationResult(*reloc, data);
    }
  }

  // sync relocations created by LD backend
  for (TargetLDBackend::extra_reloc_iterator
       iter = m_LDBackend.extra_reloc_begin(),
       end = m_LDBackend.extra_reloc_end(); iter != end; ++iter) {
    writeRelocationResult(*iter, data);
  }
}

void ObjectLinker::partialSyncRelocationResult(FileOutputBuffer& pOutput) {
  uint8_t* data = pOutput.getBufferStart();

  // traverse outputs' LDSection to get RelocData
  Module::iterator sectIter, sectEnd = m_pModule->end();
  for (sectIter = m_pModule->begin(); sectIter != sectEnd; ++sectIter) {
    if (LDFileFormat::Relocation != (*sectIter)->kind())
      continue;

    RelocData* reloc_data = (*sectIter)->getRelocData();
    RelocData::iterator relocIter, relocEnd = reloc_data->end();
    for (relocIter = reloc_data->begin(); relocIter != relocEnd; ++relocIter) {
      Relocation* reloc = llvm::cast<Relocation>(relocIter);

      // bypass the relocation with NONE type. This is to avoid overwrite the
      // target result by NONE type relocation if there is a place which has
      // two relocations to apply to, and one of it is NONE type. The result
      // we want is the value of the other relocation result. For example,
      // in .exidx, there are usually an R_ARM_NONE and R_ARM_PREL31 apply to
      // the same place
      if (reloc->type() == 0x0)
        continue;
      writeRelocationResult(*reloc, data);
    }
  }
}

void ObjectLinker::writeRelocationResult(Relocation& pReloc, uint8_t* pOutput) {
  // get output file offset
  size_t out_offset =
      pReloc.targetRef().frag()->getParent()->getSection().offset() +
      pReloc.targetRef().getOutputOffset();

  uint8_t* target_addr = pOutput + out_offset;
  // byte swapping if target and host has different endian, and then write back
  if (llvm::sys::IsLittleEndianHost != m_Config.targets().isLittleEndian()) {
    uint64_t tmp_data = 0;

    switch (pReloc.size(*m_LDBackend.getRelocator())) {
      case 8u:
        std::memcpy(target_addr, &pReloc.target(), 1);
        break;

      case 16u:
        tmp_data = mcld::bswap16(pReloc.target());
        std::memcpy(target_addr, &tmp_data, 2);
        break;

      case 32u:
        tmp_data = mcld::bswap32(pReloc.target());
        std::memcpy(target_addr, &tmp_data, 4);
        break;

      case 64u:
        tmp_data = mcld::bswap64(pReloc.target());
        std::memcpy(target_addr, &tmp_data, 8);
        break;

      default:
        break;
    }
  } else {
    std::memcpy(target_addr, &pReloc.target(),
                (pReloc.size(*m_LDBackend.getRelocator()) + 7) / 8);
  }
}

}  // namespace mcld
