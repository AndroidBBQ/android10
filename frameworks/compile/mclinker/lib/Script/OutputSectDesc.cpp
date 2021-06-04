//===- OutputSectDesc.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/OutputSectDesc.h"

#include "mcld/Script/InputSectDesc.h"
#include "mcld/Script/RpnExpr.h"
#include "mcld/Script/StringList.h"
#include "mcld/Script/StrToken.h"
#include "mcld/Support/raw_ostream.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"

#include <llvm/Support/Casting.h>

#include <cassert>

namespace mcld {

//===----------------------------------------------------------------------===//
// OutputSectDesc
//===----------------------------------------------------------------------===//
OutputSectDesc::OutputSectDesc(const std::string& pName, const Prolog& pProlog)
    : ScriptCommand(ScriptCommand::OUTPUT_SECT_DESC),
      m_Name(pName),
      m_Prolog(pProlog) {
}

OutputSectDesc::~OutputSectDesc() {
  for (iterator it = begin(), ie = end(); it != ie; ++it) {
    if (*it != NULL)
      delete *it;
  }
}

void OutputSectDesc::dump() const {
  mcld::outs() << m_Name << "\t";

  if (m_Prolog.hasVMA()) {
    m_Prolog.vma().dump();
    mcld::outs() << "\t";
  }

  switch (m_Prolog.type()) {
    case NOLOAD:
      mcld::outs() << "(NOLOAD)";
      break;
    case DSECT:
      mcld::outs() << "(DSECT)";
      break;
    case COPY:
      mcld::outs() << "(COPY)";
      break;
    case INFO:
      mcld::outs() << "(INFO)";
      break;
    case OVERLAY:
      mcld::outs() << "(OVERLAY)";
      break;
    default:
      break;
  }
  mcld::outs() << ":\n";

  if (m_Prolog.hasLMA()) {
    mcld::outs() << "\tAT ( ";
    m_Prolog.lma().dump();
    mcld::outs() << " )\n";
  }

  if (m_Prolog.hasAlign()) {
    mcld::outs() << "\tALIGN ( ";
    m_Prolog.align().dump();
    mcld::outs() << " )\n";
  }

  if (m_Prolog.hasSubAlign()) {
    mcld::outs() << "\tSUBALIGN ( ";
    m_Prolog.subAlign().dump();
    mcld::outs() << " )\n";
  }

  switch (m_Prolog.constraint()) {
    case ONLY_IF_RO:
      mcld::outs() << "\tONLY_IF_RO\n";
      break;
    case ONLY_IF_RW:
      mcld::outs() << "\tONLY_IF_RW\n";
      break;
    default:
      break;
  }

  mcld::outs() << "\t{\n";
  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    switch ((*it)->getKind()) {
      case ScriptCommand::ASSIGNMENT:
      case ScriptCommand::INPUT_SECT_DESC:
        mcld::outs() << "\t\t";
        (*it)->dump();
        break;
      default:
        assert(0);
        break;
    }
  }
  mcld::outs() << "\t}";

  if (m_Epilog.hasRegion())
    mcld::outs() << "\t>" << m_Epilog.region();
  if (m_Epilog.hasLMARegion())
    mcld::outs() << "\tAT>" << m_Epilog.lmaRegion();

  if (m_Epilog.hasPhdrs()) {
    for (StringList::const_iterator it = m_Epilog.phdrs().begin(),
                                    ie = m_Epilog.phdrs().end();
         it != ie;
         ++it) {
      assert((*it)->kind() == StrToken::String);
      mcld::outs() << ":" << (*it)->name() << " ";
    }
  }

  if (m_Epilog.hasFillExp()) {
    mcld::outs() << "= ";
    m_Epilog.fillExp().dump();
  }
  mcld::outs() << "\n";
}

void OutputSectDesc::push_back(ScriptCommand* pCommand) {
  switch (pCommand->getKind()) {
    case ScriptCommand::ASSIGNMENT:
    case ScriptCommand::INPUT_SECT_DESC:
      m_OutputSectCmds.push_back(pCommand);
      break;
    default:
      assert(0);
      break;
  }
}

void OutputSectDesc::setEpilog(const Epilog& pEpilog) {
  m_Epilog.m_pRegion = pEpilog.m_pRegion;
  m_Epilog.m_pLMARegion = pEpilog.m_pLMARegion;
  m_Epilog.m_pPhdrs = pEpilog.m_pPhdrs;
  m_Epilog.m_pFillExp = pEpilog.m_pFillExp;
}

void OutputSectDesc::activate(Module& pModule) {
  // Assignment in an output section
  OutputSectCmds assignments;

  for (const_iterator it = begin(), ie = end(); it != ie; ++it) {
    switch ((*it)->getKind()) {
      case ScriptCommand::ASSIGNMENT:
        assignments.push_back(*it);
        break;
      case ScriptCommand::INPUT_SECT_DESC: {
        (*it)->activate(pModule);

        for (iterator assign = assignments.begin(),
                      assignEnd = assignments.end();
             assign != assignEnd;
             ++assign) {
          (*assign)->activate(pModule);
        }
        assignments.clear();
        break;
      }
      default:
        assert(0);
        break;
    }
  }

  if (!assignments.empty()) {
    InputSectDesc::Spec spec;
    spec.m_pWildcardFile = NULL;
    spec.m_pExcludeFiles = NULL;
    spec.m_pWildcardSections = NULL;
    InputSectDesc inputDesc(InputSectDesc::Keep, spec, *this);
    pModule.getScript().sectionMap().insert(inputDesc, *this);

    for (iterator assign = assignments.begin(), assignEnd = assignments.end();
         assign != assignEnd;
         ++assign) {
      (*assign)->activate(pModule);
    }
    assignments.clear();
  }
}

}  // namespace mcld
