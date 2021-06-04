//===- InputSectDesc.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/InputSectDesc.h"

#include "mcld/Script/WildcardPattern.h"
#include "mcld/Support/raw_ostream.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"

#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// InputSectDesc
//===----------------------------------------------------------------------===//
InputSectDesc::InputSectDesc(KeepPolicy pPolicy,
                             const Spec& pSpec,
                             const OutputSectDesc& pOutputDesc)
    : ScriptCommand(ScriptCommand::INPUT_SECT_DESC),
      m_KeepPolicy(pPolicy),
      m_Spec(pSpec),
      m_OutputSectDesc(pOutputDesc) {
}

InputSectDesc::~InputSectDesc() {
}

void InputSectDesc::dump() const {
  if (m_KeepPolicy == Keep)
    mcld::outs() << "KEEP (";

  assert(m_Spec.hasFile());
  if (m_Spec.file().sortPolicy() == WildcardPattern::SORT_BY_NAME)
    mcld::outs() << "SORT (";

  mcld::outs() << m_Spec.file().name();

  if (m_Spec.hasSections()) {
    mcld::outs() << "(";

    if (m_Spec.hasExcludeFiles()) {
      mcld::outs() << "EXCLUDE_FILE (";
      for (StringList::const_iterator it = m_Spec.excludeFiles().begin(),
                                      ie = m_Spec.excludeFiles().end();
           it != ie;
           ++it) {
        mcld::outs() << (*it)->name() << " ";
      }
      mcld::outs() << ")";
    }

    if (m_Spec.hasSections()) {
      for (StringList::const_iterator it = m_Spec.sections().begin(),
                                      ie = m_Spec.sections().end();
           it != ie;
           ++it) {
        assert((*it)->kind() == StrToken::Wildcard);
        WildcardPattern* wildcard = llvm::cast<WildcardPattern>(*it);

        switch (wildcard->sortPolicy()) {
          case WildcardPattern::SORT_BY_NAME:
            mcld::outs() << "SORT (";
            break;
          case WildcardPattern::SORT_BY_ALIGNMENT:
            mcld::outs() << "SORT_BY_ALIGNMENT (";
            break;
          case WildcardPattern::SORT_BY_NAME_ALIGNMENT:
            mcld::outs() << "SORT_BY_NAME_ALIGNMENT (";
            break;
          case WildcardPattern::SORT_BY_ALIGNMENT_NAME:
            mcld::outs() << "SORT_BY_ALIGNMENT_NAME (";
            break;
          default:
            break;
        }

        mcld::outs() << wildcard->name() << " ";

        if (wildcard->sortPolicy() != WildcardPattern::SORT_NONE)
          mcld::outs() << ")";
      }
    }
    mcld::outs() << ")";
  }

  if (m_Spec.file().sortPolicy() == WildcardPattern::SORT_BY_NAME)
    mcld::outs() << ")";

  if (m_KeepPolicy == Keep)
    mcld::outs() << ")";

  mcld::outs() << "\n";
}

void InputSectDesc::activate(Module& pModule) {
  pModule.getScript().sectionMap().insert(*this, m_OutputSectDesc);
}

}  // namespace mcld
