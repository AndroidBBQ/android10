//===- Linker.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Linker.h"

#include "mcld/IRBuilder.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Module.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/ObjectWriter.h"
#include "mcld/LD/RelocData.h"
#include "mcld/LD/SectionData.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/Object/ObjectLinker.h"
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/FileOutputBuffer.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/TargetRegistry.h"
#include "mcld/Support/raw_ostream.h"
#include "mcld/Target/TargetLDBackend.h"

#include <cassert>

namespace mcld {

Linker::Linker()
    : m_pConfig(NULL),
      m_pIRBuilder(NULL),
      m_pTarget(NULL),
      m_pBackend(NULL),
      m_pObjLinker(NULL) {
}

Linker::~Linker() {
  reset();
}

/// emulate - To set up target-dependent options and default linker script.
/// Follow GNU ld quirks.
bool Linker::emulate(LinkerScript& pScript, LinkerConfig& pConfig) {
  m_pConfig = &pConfig;

  if (!initTarget())
    return false;

  if (!initBackend())
    return false;

  if (!initOStream())
    return false;

  if (!initEmulator(pScript))
    return false;

  return true;
}

bool Linker::link(Module& pModule, IRBuilder& pBuilder) {
  if (!normalize(pModule, pBuilder))
    return false;

  if (!resolve(pModule))
    return false;

  return layout();
}

/// normalize - to convert the command line language to the input tree.
bool Linker::normalize(Module& pModule, IRBuilder& pBuilder) {
  assert(m_pConfig != NULL);

  m_pIRBuilder = &pBuilder;

  m_pObjLinker = new ObjectLinker(*m_pConfig, *m_pBackend);

  // 2. - initialize ObjectLinker
  if (!m_pObjLinker->initialize(pModule, pBuilder))
    return false;

  // 3. - initialize output's standard sections
  if (!m_pObjLinker->initStdSections())
    return false;

  if (!Diagnose())
    return false;

  // 4.a - add undefined symbols
  //   before reading the inputs, we should add undefined symbols set by -u to
  //   ensure that correspoding objects (e.g. in an archive) will be included
  m_pObjLinker->addUndefinedSymbols();

  // 4.b - normalize the input tree
  //   read out sections and symbol/string tables (from the files) and
  //   set them in Module. When reading out the symbol, resolve their symbols
  //   immediately and set their ResolveInfo (i.e., Symbol Resolution).
  m_pObjLinker->normalize();

  if (m_pConfig->options().trace()) {
    static int counter = 0;
    mcld::outs() << "** name\ttype\tpath\tsize ("
                 << pModule.getInputTree().size() << ")\n";

    InputTree::const_dfs_iterator input,
        inEnd = pModule.getInputTree().dfs_end();
    for (input = pModule.getInputTree().dfs_begin(); input != inEnd; ++input) {
      mcld::outs() << counter++ << " *  " << (*input)->name();
      switch ((*input)->type()) {
        case Input::Archive:
          mcld::outs() << "\tarchive\t(";
          break;
        case Input::Object:
          mcld::outs() << "\tobject\t(";
          break;
        case Input::DynObj:
          mcld::outs() << "\tshared\t(";
          break;
        case Input::Script:
          mcld::outs() << "\tscript\t(";
          break;
        case Input::External:
          mcld::outs() << "\textern\t(";
          break;
        default:
          unreachable(diag::err_cannot_trace_file)
              << (*input)->type() << (*input)->name() << (*input)->path();
      }
      mcld::outs() << (*input)->path() << ")\n";
    }
  }

  // 5. - set up code position
  if (LinkerConfig::DynObj == m_pConfig->codeGenType() ||
      m_pConfig->options().isPIE()) {
    m_pConfig->setCodePosition(LinkerConfig::Independent);
  } else if (pModule.getLibraryList().empty()) {
    // If the output is dependent on its loaded address, and it does not need
    // to call outside functions, then we can treat the output static dependent
    // and perform better optimizations.
    m_pConfig->setCodePosition(LinkerConfig::StaticDependent);

    if (LinkerConfig::Exec == m_pConfig->codeGenType()) {
      // Since the output is static dependent, there should not have any
      // undefined
      // references in the output module.
      m_pConfig->options().setNoUndefined();
    }
  } else {
    m_pConfig->setCodePosition(LinkerConfig::DynamicDependent);
  }

  if (!m_pObjLinker->linkable())
    return Diagnose();

  return true;
}

bool Linker::resolve(Module& pModule) {
  assert(m_pConfig != NULL);
  assert(m_pObjLinker != NULL);

  // 6. - read all relocation entries from input files
  //   For all relocation sections of each input file (in the tree),
  //   read out reloc entry info from the object file and accordingly
  //   initiate their reloc entries in SectOrRelocData of LDSection.
  //
  //   To collect all edges in the reference graph.
  m_pObjLinker->readRelocations();

  // 7. - data stripping optimizations
  m_pObjLinker->dataStrippingOpt();

  // 8. - merge all sections
  //   Push sections into Module's SectionTable.
  //   Merge sections that have the same name.
  //   Maintain them as fragments in the section.
  //
  //   To merge nodes of the reference graph.
  if (!m_pObjLinker->mergeSections())
    return false;

  // 9.a - add symbols to output
  //  After all input symbols have been resolved, add them to output symbol
  //  table at once
  m_pObjLinker->addSymbolsToOutput(pModule);

  // 9.b - allocateCommonSymbols
  //   Allocate fragments for common symbols to the corresponding sections.
  if (!m_pObjLinker->allocateCommonSymbols())
    return false;

  return true;
}

bool Linker::layout() {
  assert(m_pConfig != NULL && m_pObjLinker != NULL);

  // 10. - add standard symbols, target-dependent symbols and script symbols
  if (!m_pObjLinker->addStandardSymbols() ||
      !m_pObjLinker->addTargetSymbols() || !m_pObjLinker->addScriptSymbols())
    return false;

  // 11. - scan all relocation entries by output symbols.
  //   reserve GOT space for layout.
  //   the space info is needed by pre-layout to compute the section size
  m_pObjLinker->scanRelocations();

  // 12.a - init relaxation stuff.
  m_pObjLinker->initStubs();

  // 12.b - pre-layout
  m_pObjLinker->prelayout();

  // 12.c - linear layout
  //   Decide which sections will be left in. Sort the sections according to
  //   a given order. Then, create program header accordingly.
  //   Finally, set the offset for sections (@ref LDSection)
  //   according to the new order.
  m_pObjLinker->layout();

  // 12.d - post-layout (create segment, instruction relaxing)
  m_pObjLinker->postlayout();

  // 13. - finalize symbol value
  m_pObjLinker->finalizeSymbolValue();

  // 14. - apply relocations
  m_pObjLinker->relocation();

  if (!Diagnose())
    return false;
  return true;
}

bool Linker::emit(FileOutputBuffer& pOutput) {
  // 15. - write out output
  m_pObjLinker->emitOutput(pOutput);

  // 16. - post processing
  m_pObjLinker->postProcessing(pOutput);

  if (!Diagnose())
    return false;

  return true;
}

bool Linker::emit(const Module& pModule, const std::string& pPath) {
  FileHandle file;
  FileHandle::OpenMode open_mode(
      FileHandle::ReadWrite | FileHandle::Truncate | FileHandle::Create);
  FileHandle::Permission permission;
  switch (m_pConfig->codeGenType()) {
    case mcld::LinkerConfig::Unknown:
    case mcld::LinkerConfig::Object:
      permission = FileHandle::Permission(0x644);
      break;
    case mcld::LinkerConfig::DynObj:
    case mcld::LinkerConfig::Exec:
    case mcld::LinkerConfig::Binary:
      permission = FileHandle::Permission(0x755);
      break;
    default:
      assert(0 && "Unknown file type");
  }

  bool result = file.open(sys::fs::Path(pPath), open_mode, permission);
  if (!result) {
    error(diag::err_cannot_open_output_file) << "Linker::emit()" << pPath;
    return false;
  }

  std::unique_ptr<FileOutputBuffer> output;
  FileOutputBuffer::create(
      file, m_pObjLinker->getWriter()->getOutputSize(pModule), output);

  result = emit(*output);
  file.close();
  return result;
}

bool Linker::emit(const Module& pModule, int pFileDescriptor) {
  FileHandle file;
  file.delegate(pFileDescriptor);

  std::unique_ptr<FileOutputBuffer> output;
  FileOutputBuffer::create(
      file, m_pObjLinker->getWriter()->getOutputSize(pModule), output);

  return emit(*output);
}

bool Linker::reset() {
  m_pConfig = NULL;
  m_pIRBuilder = NULL;
  m_pTarget = NULL;

  // Because llvm::iplist will touch the removed node, we must clear
  // RelocData before deleting target backend.
  RelocData::Clear();
  SectionData::Clear();
  EhFrame::Clear();

  delete m_pBackend;
  m_pBackend = NULL;

  delete m_pObjLinker;
  m_pObjLinker = NULL;

  LDSection::Clear();
  LDSymbol::Clear();
  FragmentRef::Clear();
  Relocation::Clear();
  return true;
}

bool Linker::initTarget() {
  assert(m_pConfig != NULL);

  std::string error;
  llvm::Triple triple(m_pConfig->targets().triple());

  m_pTarget = mcld::TargetRegistry::lookupTarget(
      m_pConfig->targets().getArch(), triple, error);
  m_pConfig->targets().setTriple(triple);

  if (m_pTarget == NULL) {
    fatal(diag::fatal_cannot_init_target) << triple.str() << error;
    return false;
  }
  return true;
}

bool Linker::initBackend() {
  assert(m_pTarget != NULL);
  m_pBackend = m_pTarget->createLDBackend(*m_pConfig);
  if (m_pBackend == NULL) {
    fatal(diag::fatal_cannot_init_backend)
        << m_pConfig->targets().triple().str();
    return false;
  }
  return true;
}

bool Linker::initOStream() {
  assert(m_pConfig != NULL);

  mcld::outs().setColor(m_pConfig->options().color());
  mcld::errs().setColor(m_pConfig->options().color());

  return true;
}

bool Linker::initEmulator(LinkerScript& pScript) {
  assert(m_pTarget != NULL && m_pConfig != NULL);
  return m_pTarget->emulate(pScript, *m_pConfig);
}

}  // namespace mcld
