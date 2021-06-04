//===- GeneralOptions.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/GeneralOptions.h"
#include "mcld/MC/Input.h"
#include "mcld/MC/ZOption.h"
#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// GeneralOptions
//===----------------------------------------------------------------------===//
GeneralOptions::GeneralOptions()
    : m_Verbose(-1),
      m_MaxErrorNum(-1),
      m_MaxWarnNum(-1),
      m_NumSpareDTags(5),
      m_ExecStack(Unknown),
      m_NoUndefined(Unknown),
      m_MulDefs(Unknown),
      m_CommPageSize(0x0),
      m_MaxPageSize(0x0),
      m_bCombReloc(true),
      m_bInitFirst(false),
      m_bInterPose(false),
      m_bLoadFltr(false),
      m_bNoCopyReloc(false),
      m_bNoDefaultLib(false),
      m_bNoDelete(false),
      m_bNoDLOpen(false),
      m_bNoDump(false),
      m_bRelro(false),
      m_bNow(false),
      m_bOrigin(false),
      m_bTrace(false),
      m_Bsymbolic(false),
      m_Bgroup(false),
      m_bPIE(false),
      m_bColor(true),
      m_bCreateEhFrameHdr(false),
      m_bNMagic(false),
      m_bOMagic(false),
      m_bStripDebug(false),
      m_bExportDynamic(false),
      m_bWarnSharedTextrel(false),
      m_bBinaryInput(false),
      m_bDefineCommon(false),
      m_bFatalWarnings(false),
      m_bNewDTags(false),
      m_bNoStdlib(false),
      m_bWarnMismatch(true),
      m_bGCSections(false),
      m_bPrintGCSections(false),
      m_bGenUnwindInfo(true),
      m_bPrintICFSections(false),
      m_ICF(ICF::None),
      m_ICFIterations(2),
      m_StripSymbols(StripSymbolMode::KeepAllSymbols),
      m_HashStyle(HashStyle::SystemV) {
}

GeneralOptions::~GeneralOptions() {
}

void GeneralOptions::setSOName(const std::string& pName) {
  size_t pos = pName.find_last_of(sys::fs::separator);
  if (std::string::npos == pos)
    m_SOName = pName;
  else
    m_SOName = pName.substr(pos + 1);
}

void GeneralOptions::addZOption(const ZOption& pOption) {
  switch (pOption.kind()) {
    case ZOption::CombReloc:
      m_bCombReloc = true;
      break;
    case ZOption::NoCombReloc:
      m_bCombReloc = false;
      break;
    case ZOption::Defs:
      m_NoUndefined = YES;
      break;
    case ZOption::ExecStack:
      m_ExecStack = YES;
      break;
    case ZOption::NoExecStack:
      m_ExecStack = NO;
      break;
    case ZOption::InitFirst:
      m_bInitFirst = true;
      break;
    case ZOption::InterPose:
      m_bInterPose = true;
      break;
    case ZOption::LoadFltr:
      m_bLoadFltr = true;
      break;
    case ZOption::MulDefs:
      m_MulDefs = YES;
      break;
    case ZOption::NoCopyReloc:
      m_bNoCopyReloc = true;
      break;
    case ZOption::NoDefaultLib:
      m_bNoDefaultLib = true;
      break;
    case ZOption::NoDelete:
      m_bNoDelete = true;
      break;
    case ZOption::NoDLOpen:
      m_bNoDLOpen = true;
      break;
    case ZOption::NoDump:
      m_bNoDump = true;
      break;
    case ZOption::NoRelro:
      m_bRelro = false;
      break;
    case ZOption::Relro:
      m_bRelro = true;
      break;
    case ZOption::Lazy:
      m_bNow = false;
      break;
    case ZOption::Now:
      m_bNow = true;
      break;
    case ZOption::Origin:
      m_bOrigin = true;
      break;
    case ZOption::CommPageSize:
      m_CommPageSize = pOption.pageSize();
      break;
    case ZOption::MaxPageSize:
      m_MaxPageSize = pOption.pageSize();
      break;
    case ZOption::Unknown:
    default:
      assert(false && "Not a recognized -z option.");
      break;
  }
}

bool GeneralOptions::isInExcludeLIBS(const Input& pInput) const {
  assert(pInput.type() == Input::Archive);

  if (m_ExcludeLIBS.empty()) {
    return false;
  }

  // Specifying "--exclude-libs ALL" excludes symbols in all archive libraries
  // from automatic export.
  if (m_ExcludeLIBS.count("ALL") != 0) {
    return true;
  }

  std::string name(pInput.name());
  name.append(".a");
  if (m_ExcludeLIBS.count(name) != 0) {
    return true;
  }

  return false;
}

}  // namespace mcld
