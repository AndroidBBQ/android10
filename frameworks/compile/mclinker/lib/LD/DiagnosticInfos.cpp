//===- DiagnosticInfo.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/DiagnosticInfos.h"

#include "mcld/LinkerConfig.h"
#include "mcld/ADT/SizeTraits.h"
#include "mcld/LD/Diagnostic.h"
#include "mcld/LD/DiagnosticPrinter.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/DataTypes.h>

#include <algorithm>

namespace mcld {

namespace {

struct DiagStaticInfo {
 public:
  uint16_t ID;
  DiagnosticEngine::Severity Severity;
  uint16_t DescriptionLen;
  const char* DescriptionStr;

 public:
  llvm::StringRef getDescription() const {
    return llvm::StringRef(DescriptionStr, DescriptionLen);
  }

  bool operator<(const DiagStaticInfo& pRHS) const { return (ID < pRHS.ID); }
};

}  // anonymous namespace

static const DiagStaticInfo DiagCommonInfo[] = {
#define DIAG(ENUM, CLASS, ADDRDESC, LOCDESC)                    \
  { diag::ENUM, CLASS, STR_SIZE(ADDRDESC, uint16_t), ADDRDESC } \
  ,
#include "mcld/LD/DiagAttribute.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagCommonKinds.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagReaders.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagSymbolResolutions.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagRelocations.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagLayouts.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagGOTPLT.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagLDScript.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagMips.inc"  // NOLINT [build/include] [4]
#undef DIAG
    {0, DiagnosticEngine::None, 0, 0}};

static const unsigned int DiagCommonInfoSize =
    sizeof(DiagCommonInfo) / sizeof(DiagCommonInfo[0]) - 1;

static const DiagStaticInfo DiagLoCInfo[] = {
#define DIAG(ENUM, CLASS, ADDRDESC, LOCDESC)                  \
  { diag::ENUM, CLASS, STR_SIZE(LOCDESC, uint16_t), LOCDESC } \
  ,
#include "mcld/LD/DiagAttribute.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagCommonKinds.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagReaders.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagSymbolResolutions.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagRelocations.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagLayouts.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagGOTPLT.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagLDScript.inc"  // NOLINT [build/include] [4]
#include "mcld/LD/DiagMips.inc"  // NOLINT [build/include] [4]
#undef DIAG
    {0, DiagnosticEngine::None, 0, 0}};

static const unsigned int DiagLoCInfoSize =
    sizeof(DiagLoCInfo) / sizeof(DiagLoCInfo[0]) - 1;

static const DiagStaticInfo* getDiagInfo(unsigned int pID,
                                         bool pInLoC = false) {
  const DiagStaticInfo* static_info = (pInLoC) ? DiagLoCInfo : DiagCommonInfo;
  unsigned int info_size = (pInLoC) ? DiagLoCInfoSize : DiagCommonInfoSize;

  DiagStaticInfo key = {
      static_cast<uint16_t>(pID), DiagnosticEngine::None, 0, 0};

  const DiagStaticInfo* result =
      std::lower_bound(static_info, static_info + info_size, key);

  if (result == (static_info + info_size) || result->ID != pID)
    return NULL;

  return result;
}

//===----------------------------------------------------------------------===//
//  DiagnosticInfos
//===----------------------------------------------------------------------===//
DiagnosticInfos::DiagnosticInfos(const LinkerConfig& pConfig)
    : m_Config(pConfig) {
}

DiagnosticInfos::~DiagnosticInfos() {
}

llvm::StringRef DiagnosticInfos::getDescription(unsigned int pID,
                                                bool pInLoC) const {
  return getDiagInfo(pID, pInLoC)->getDescription();
}

bool DiagnosticInfos::process(DiagnosticEngine& pEngine) const {
  Diagnostic info(pEngine);

  unsigned int ID = info.getID();

  // we are not implement LineInfo, so keep pIsLoC false.
  const DiagStaticInfo* static_info = getDiagInfo(ID);

  DiagnosticEngine::Severity severity = static_info->Severity;

  switch (ID) {
    case diag::multiple_definitions: {
      if (m_Config.options().isMulDefs()) {
        severity = DiagnosticEngine::Ignore;
      }
      break;
    }
    case diag::undefined_reference:
    case diag::undefined_reference_text: {
      // we have not implement --unresolved-symbols=method yet. So far, MCLinker
      // provides the easier --allow-shlib-undefined and --no-undefined (i.e.
      // -z defs)
      switch (m_Config.codeGenType()) {
        case LinkerConfig::Object:
          if (m_Config.options().isNoUndefined())
            severity = DiagnosticEngine::Error;
          else
            severity = DiagnosticEngine::Ignore;
          break;
        case LinkerConfig::DynObj:
          if (m_Config.options().isNoUndefined())
            severity = DiagnosticEngine::Error;
          else
            severity = DiagnosticEngine::Ignore;
          break;
        default:
          severity = DiagnosticEngine::Error;
          break;
      }
      break;
    }
    case diag::debug_print_gc_sections: {
      if (!m_Config.options().getPrintGCSections())
        severity = DiagnosticEngine::Ignore;
      break;
    }
    default:
      break;
  }  // end of switch

  // If --fatal-warnings is turned on, then switch warnings and errors to fatal
  if (m_Config.options().isFatalWarnings()) {
    if (severity == DiagnosticEngine::Warning ||
        severity == DiagnosticEngine::Error) {
      severity = DiagnosticEngine::Fatal;
    }
  }

  // finally, report it.
  pEngine.getPrinter()->handleDiagnostic(severity, info);
  return true;
}

}  // namespace mcld
