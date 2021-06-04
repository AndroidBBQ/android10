//===- InputSectDesc.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_INPUTSECTDESC_H_
#define MCLD_SCRIPT_INPUTSECTDESC_H_

#include "mcld/Script/ScriptCommand.h"
#include "mcld/Script/StringList.h"

#include <cassert>

namespace mcld {

class OutputSectDesc;
class WildcardPattern;

/** \class InputSectDesc
 *  \brief This class defines the interfaces to input section description.
 */

class InputSectDesc : public ScriptCommand {
 public:
  enum KeepPolicy { Keep, NoKeep };

  struct Spec {
    bool hasFile() const { return m_pWildcardFile != NULL; }
    const WildcardPattern& file() const {
      assert(hasFile());
      return *m_pWildcardFile;
    }

    bool hasExcludeFiles() const {
      return m_pExcludeFiles != NULL && !m_pExcludeFiles->empty();
    }
    const StringList& excludeFiles() const {
      assert(hasExcludeFiles());
      return *m_pExcludeFiles;
    }

    bool hasSections() const {
      return m_pWildcardSections != NULL && !m_pWildcardSections->empty();
    }
    const StringList& sections() const {
      assert(hasSections());
      return *m_pWildcardSections;
    }

    bool operator==(const Spec& pRHS) const {
      /* FIXME: currently I don't check the real content */
      if (this == &pRHS)
        return true;
      if (m_pWildcardFile != pRHS.m_pWildcardFile)
        return false;
      if (m_pExcludeFiles != pRHS.m_pExcludeFiles)
        return false;
      if (m_pWildcardSections != pRHS.m_pWildcardSections)
        return false;
      return true;
    }

    WildcardPattern* m_pWildcardFile;
    StringList* m_pExcludeFiles;
    StringList* m_pWildcardSections;
  };

 public:
  InputSectDesc(KeepPolicy pPolicy,
                const Spec& pSpec,
                const OutputSectDesc& pOutputDesc);
  ~InputSectDesc();

  KeepPolicy policy() const { return m_KeepPolicy; }

  const Spec& spec() const { return m_Spec; }

  void dump() const;

  static bool classof(const ScriptCommand* pCmd) {
    return pCmd->getKind() == ScriptCommand::INPUT_SECT_DESC;
  }

  void activate(Module& pModule);

 private:
  KeepPolicy m_KeepPolicy;
  Spec m_Spec;
  const OutputSectDesc& m_OutputSectDesc;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_INPUTSECTDESC_H_
