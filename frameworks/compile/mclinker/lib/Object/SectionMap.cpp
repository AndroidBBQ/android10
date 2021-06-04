//===- SectionMap.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Object/SectionMap.h"

#include "mcld/Fragment/NullFragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Script/Assignment.h"
#include "mcld/Script/Operand.h"
#include "mcld/Script/Operator.h"
#include "mcld/Script/RpnExpr.h"
#include "mcld/Script/StringList.h"
#include "mcld/Script/WildcardPattern.h"

#include <llvm/Support/Casting.h>

#include <cassert>
#include <cstring>
#include <climits>
#if !defined(MCLD_ON_WIN32)
#include <fnmatch.h>
#define fnmatch0(pattern, string) (fnmatch(pattern, string, 0) == 0)
#else
#include <windows.h>
#include <shlwapi.h>
#define fnmatch0(pattern, string) (PathMatchSpec(string, pattern) == true)
#endif

namespace mcld {

//===----------------------------------------------------------------------===//
// SectionMap::Input
//===----------------------------------------------------------------------===//
SectionMap::Input::Input(const std::string& pName,
                         InputSectDesc::KeepPolicy pPolicy)
    : m_Policy(pPolicy) {
  m_Spec.m_pWildcardFile =
      WildcardPattern::create("*", WildcardPattern::SORT_NONE);
  m_Spec.m_pExcludeFiles = NULL;

  StringList* sections = StringList::create();
  sections->push_back(
      WildcardPattern::create(pName, WildcardPattern::SORT_NONE));
  m_Spec.m_pWildcardSections = sections;

  m_pSection = LDSection::Create(pName, LDFileFormat::TEXT, 0, 0);
  SectionData* sd = SectionData::Create(*m_pSection);
  m_pSection->setSectionData(sd);
  new NullFragment(sd);
  new NullFragment(sd);
}

SectionMap::Input::Input(const InputSectDesc& pInputDesc)
    : m_Policy(pInputDesc.policy()) {
  m_Spec.m_pWildcardFile = pInputDesc.spec().m_pWildcardFile;
  m_Spec.m_pExcludeFiles = pInputDesc.spec().m_pExcludeFiles;
  m_Spec.m_pWildcardSections = pInputDesc.spec().m_pWildcardSections;
  m_pSection = LDSection::Create("", LDFileFormat::TEXT, 0, 0);
  SectionData* sd = SectionData::Create(*m_pSection);
  m_pSection->setSectionData(sd);
  new NullFragment(sd);
  new NullFragment(sd);
}

//===----------------------------------------------------------------------===//
// SectionMap::Output
//===----------------------------------------------------------------------===//
SectionMap::Output::Output(const std::string& pName)
    : m_Name(pName), m_Order(UINT_MAX) {
  m_Prolog.m_pVMA = NULL;
  m_Prolog.m_Type = OutputSectDesc::LOAD;
  m_Prolog.m_pLMA = NULL;
  m_Prolog.m_pAlign = NULL;
  m_Prolog.m_pSubAlign = NULL;
  m_Prolog.m_Constraint = OutputSectDesc::NO_CONSTRAINT;

  m_Epilog.m_pRegion = NULL;
  m_Epilog.m_pLMARegion = NULL;
  m_Epilog.m_pPhdrs = NULL;
  m_Epilog.m_pFillExp = NULL;

  m_pSection = LDSection::Create(pName, LDFileFormat::TEXT, 0, 0);
  SectionData* sd = SectionData::Create(*m_pSection);
  m_pSection->setSectionData(sd);

  m_bIsDiscard = pName.compare("/DISCARD/") == 0;
}

SectionMap::Output::Output(const OutputSectDesc& pOutputDesc)
    : m_Name(pOutputDesc.name()),
      m_Prolog(pOutputDesc.prolog()),
      m_Epilog(pOutputDesc.epilog()),
      m_Order(UINT_MAX) {
  m_pSection = LDSection::Create(m_Name, LDFileFormat::TEXT, 0, 0);
  SectionData* sd = SectionData::Create(*m_pSection);
  m_pSection->setSectionData(sd);

  m_bIsDiscard = m_Name.compare("/DISCARD/") == 0;
}

bool SectionMap::Output::hasContent() const {
  return m_pSection != NULL && m_pSection->size() != 0;
}

SectionMap::Output::const_dot_iterator
SectionMap::Output::find_first_explicit_dot() const {
  for (const_dot_iterator it = dot_begin(), ie = dot_end(); it != ie; ++it) {
    if ((*it).type() == Assignment::DEFAULT)
      return it;
  }
  return dot_end();
}

SectionMap::Output::dot_iterator SectionMap::Output::find_first_explicit_dot() {
  for (dot_iterator it = dot_begin(), ie = dot_end(); it != ie; ++it) {
    if ((*it).type() == Assignment::DEFAULT)
      return it;
  }
  return dot_end();
}

SectionMap::Output::const_dot_iterator
SectionMap::Output::find_last_explicit_dot() const {
  typedef DotAssignments::const_reverse_iterator CONST_RIT;
  for (CONST_RIT rit = dotAssignments().rbegin(), rie = dotAssignments().rend();
       rit != rie;
       ++rit) {
    if ((*rit).type() == Assignment::DEFAULT) {
      return dot_begin() +
             (dotAssignments().size() - (rit - dotAssignments().rbegin()) - 1);
    }
  }
  return dot_end();
}

SectionMap::Output::dot_iterator SectionMap::Output::find_last_explicit_dot() {
  typedef DotAssignments::reverse_iterator RIT;
  for (RIT rit = dotAssignments().rbegin(), rie = dotAssignments().rend();
       rit != rie;
       ++rit) {
    if ((*rit).type() == Assignment::DEFAULT) {
      return dot_begin() +
             (dotAssignments().size() - (rit - dotAssignments().rbegin()) - 1);
    }
  }
  return dot_end();
}

//===----------------------------------------------------------------------===//
// SectionMap
//===----------------------------------------------------------------------===//
SectionMap::~SectionMap() {
  iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    if (*out != NULL) {
      Output::iterator in, inBegin = (*out)->begin(), inEnd = (*out)->end();
      for (in = inBegin; in != inEnd; ++in) {
        if (*in != NULL)
          delete *in;
      }
      delete *out;
    }
  }
}

SectionMap::const_mapping SectionMap::find(
    const std::string& pInputFile,
    const std::string& pInputSection) const {
  const_iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    Output::const_iterator in, inBegin = (*out)->begin(), inEnd = (*out)->end();
    for (in = inBegin; in != inEnd; ++in) {
      if (matched(**in, pInputFile, pInputSection))
        return std::make_pair(*out, *in);
    }
  }
  return std::make_pair((const Output*)NULL, (const Input*)NULL);
}

SectionMap::mapping SectionMap::find(const std::string& pInputFile,
                                     const std::string& pInputSection) {
  iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    Output::iterator in, inBegin = (*out)->begin(), inEnd = (*out)->end();
    for (in = inBegin; in != inEnd; ++in) {
      if (matched(**in, pInputFile, pInputSection))
        return std::make_pair(*out, *in);
    }
  }
  return std::make_pair(reinterpret_cast<Output*>(NULL),
                        reinterpret_cast<Input*>(NULL));
}

SectionMap::const_iterator SectionMap::find(
    const std::string& pOutputSection) const {
  const_iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    if ((*out)->name().compare(pOutputSection) == 0)
      return out;
  }
  return outEnd;
}

SectionMap::iterator SectionMap::find(const std::string& pOutputSection) {
  iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    if ((*out)->name().compare(pOutputSection) == 0)
      return out;
  }
  return outEnd;
}

std::pair<SectionMap::mapping, bool> SectionMap::insert(
    const std::string& pInputSection,
    const std::string& pOutputSection,
    InputSectDesc::KeepPolicy pPolicy) {
  iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    if ((*out)->name().compare(pOutputSection) == 0)
      break;
  }
  if (out != end()) {
    Output::iterator in, inBegin = (*out)->begin(), inEnd = (*out)->end();
    for (in = inBegin; in != inEnd; ++in) {
      if ((*in)->getSection()->name().compare(pInputSection) == 0)
        break;
    }

    if (in != (*out)->end()) {
      return std::make_pair(std::make_pair(*out, *in), false);
    } else {
      Input* input = new Input(pInputSection, pPolicy);
      (*out)->append(input);
      return std::make_pair(std::make_pair(*out, input), true);
    }
  }

  Output* output = new Output(pOutputSection);
  m_OutputDescList.push_back(output);
  Input* input = new Input(pInputSection, pPolicy);
  output->append(input);

  return std::make_pair(std::make_pair(output, input), true);
}

std::pair<SectionMap::mapping, bool> SectionMap::insert(
    const InputSectDesc& pInputDesc,
    const OutputSectDesc& pOutputDesc) {
  iterator out, outBegin = begin(), outEnd = end();
  for (out = outBegin; out != outEnd; ++out) {
    if ((*out)->name().compare(pOutputDesc.name()) == 0 &&
        (*out)->prolog() == pOutputDesc.prolog() &&
        (*out)->epilog() == pOutputDesc.epilog())
      break;
  }

  if (out != end()) {
    Output::iterator in, inBegin = (*out)->begin(), inEnd = (*out)->end();
    for (in = inBegin; in != inEnd; ++in) {
      if ((*in)->policy() == pInputDesc.policy() &&
          (*in)->spec() == pInputDesc.spec())
        break;
    }

    if (in != (*out)->end()) {
      return std::make_pair(std::make_pair(*out, *in), false);
    } else {
      Input* input = new Input(pInputDesc);
      (*out)->append(input);
      return std::make_pair(std::make_pair(*out, input), true);
    }
  }

  Output* output = new Output(pOutputDesc);
  m_OutputDescList.push_back(output);
  Input* input = new Input(pInputDesc);
  output->append(input);

  return std::make_pair(std::make_pair(output, input), true);
}

SectionMap::iterator SectionMap::insert(iterator pPosition,
                                        LDSection* pSection) {
  Output* output = new Output(pSection->name());
  output->append(new Input(pSection->name(), InputSectDesc::NoKeep));
  output->setSection(pSection);
  return m_OutputDescList.insert(pPosition, output);
}

bool SectionMap::matched(const SectionMap::Input& pInput,
                         const std::string& pInputFile,
                         const std::string& pInputSection) const {
  if (pInput.spec().hasFile() && !matched(pInput.spec().file(), pInputFile))
    return false;

  if (pInput.spec().hasExcludeFiles()) {
    StringList::const_iterator file, fileEnd;
    fileEnd = pInput.spec().excludeFiles().end();
    for (file = pInput.spec().excludeFiles().begin(); file != fileEnd; ++file) {
      if (matched(llvm::cast<WildcardPattern>(**file), pInputFile)) {
        return false;
      }
    }
  }

  if (pInput.spec().hasSections()) {
    StringList::const_iterator sect, sectEnd = pInput.spec().sections().end();
    for (sect = pInput.spec().sections().begin(); sect != sectEnd; ++sect) {
      if (matched(llvm::cast<WildcardPattern>(**sect), pInputSection)) {
        return true;
      }
    }
  }

  return false;
}

bool SectionMap::matched(const WildcardPattern& pPattern,
                         const std::string& pName) const {
  if (pPattern.isPrefix()) {
    llvm::StringRef name(pName);
    return name.startswith(pPattern.prefix());
  } else {
    return fnmatch0(pPattern.name().c_str(), pName.c_str());
  }
}

// fixupDotSymbols - ensure the dot symbols are valid
void SectionMap::fixupDotSymbols() {
  for (iterator it = begin() + 1, ie = end(); it != ie; ++it) {
    // fixup the 1st explicit dot assignment if needed
    if (!(*it)->dotAssignments().empty()) {
      Output::dot_iterator dot = (*it)->find_first_explicit_dot();
      if (dot != (*it)->dot_end() && (*dot).symbol().isDot() &&
          (*dot).getRpnExpr().hasDot()) {
        Assignment assign(Assignment::OUTPUT_SECTION,
                          Assignment::DEFAULT,
                          *SymOperand::create("."),
                          *RpnExpr::buildHelperExpr(it - 1));
        Output::dot_iterator ref = (*it)->dotAssignments().insert(dot, assign);
        for (RpnExpr::iterator tok = (*dot).getRpnExpr().begin(),
                               tokEnd = (*dot).getRpnExpr().end();
             tok != tokEnd;
             ++tok) {
          if ((*tok)->kind() == ExprToken::OPERAND &&
              llvm::cast<Operand>(*tok)->isDot())
            *tok = &((*ref).symbol());
        }  // for each token in the RHS expr of the dot assignment
      }
    }

    // fixup dot in output VMA if needed
    if ((*it)->prolog().hasVMA() && (*it)->prolog().vma().hasDot()) {
      Output::dot_iterator dot = (*it)->find_last_explicit_dot();
      if (dot == (*it)->dot_end()) {
        Assignment assign(Assignment::OUTPUT_SECTION,
                          Assignment::DEFAULT,
                          *SymOperand::create("."),
                          *RpnExpr::buildHelperExpr(it - 1));
        dot = (*it)->dotAssignments().insert(dot, assign);
      }
      for (RpnExpr::iterator tok = (*it)->prolog().vma().begin(),
                             tokEnd = (*it)->prolog().vma().end();
           tok != tokEnd;
           ++tok) {
        if ((*tok)->kind() == ExprToken::OPERAND &&
            llvm::cast<Operand>(*tok)->isDot())
          *tok = &((*dot).symbol());
      }  // for each token in the RHS expr of the dot assignment
    }
  }  // for each output section
}

}  // namespace mcld
