//===- SectionMap.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_OBJECT_SECTIONMAP_H_
#define MCLD_OBJECT_SECTIONMAP_H_

#include "mcld/Script/Assignment.h"
#include "mcld/Script/InputSectDesc.h"
#include "mcld/Script/OutputSectDesc.h"

#include <llvm/Support/DataTypes.h>

#include <string>
#include <vector>

namespace mcld {

class Fragment;
class LDSection;

/** \class SectionMap
 *  \brief descirbe how to map input sections into output sections
 */
class SectionMap {
 public:
  class Input {
   public:
    typedef std::vector<std::pair<Fragment*, Assignment> > DotAssignments;
    typedef DotAssignments::const_iterator const_dot_iterator;
    typedef DotAssignments::iterator dot_iterator;

    Input(const std::string& pName, InputSectDesc::KeepPolicy pPolicy);
    explicit Input(const InputSectDesc& pInputDesc);

    InputSectDesc::KeepPolicy policy() const { return m_Policy; }

    const InputSectDesc::Spec& spec() const { return m_Spec; }

    const LDSection* getSection() const { return m_pSection; }
    LDSection* getSection() { return m_pSection; }

    const_dot_iterator dot_begin() const { return m_DotAssignments.begin(); }
    dot_iterator dot_begin() { return m_DotAssignments.begin(); }
    const_dot_iterator dot_end() const { return m_DotAssignments.end(); }
    dot_iterator dot_end() { return m_DotAssignments.end(); }

    const DotAssignments& dotAssignments() const { return m_DotAssignments; }
    DotAssignments& dotAssignments() { return m_DotAssignments; }

   private:
    InputSectDesc::KeepPolicy m_Policy;
    InputSectDesc::Spec m_Spec;
    LDSection* m_pSection;
    DotAssignments m_DotAssignments;
  };

  class Output {
   public:
    typedef std::vector<Input*> InputList;
    typedef InputList::const_iterator const_iterator;
    typedef InputList::iterator iterator;
    typedef InputList::const_reference const_reference;
    typedef InputList::reference reference;

    typedef std::vector<Assignment> DotAssignments;
    typedef DotAssignments::const_iterator const_dot_iterator;
    typedef DotAssignments::iterator dot_iterator;

    explicit Output(const std::string& pName);
    explicit Output(const OutputSectDesc& pOutputDesc);

    const std::string& name() const { return m_Name; }

    const OutputSectDesc::Prolog& prolog() const { return m_Prolog; }
    OutputSectDesc::Prolog& prolog() { return m_Prolog; }

    const OutputSectDesc::Epilog& epilog() const { return m_Epilog; }
    OutputSectDesc::Epilog& epilog() { return m_Epilog; }

    size_t order() const { return m_Order; }

    void setOrder(size_t pOrder) { m_Order = pOrder; }

    bool hasContent() const;

    const LDSection* getSection() const { return m_pSection; }
    LDSection* getSection() { return m_pSection; }

    void setSection(LDSection* pSection) { m_pSection = pSection; }

    const_iterator begin() const { return m_InputList.begin(); }
    iterator begin() { return m_InputList.begin(); }
    const_iterator end() const { return m_InputList.end(); }
    iterator end() { return m_InputList.end(); }

    const_reference front() const { return m_InputList.front(); }
    reference front() { return m_InputList.front(); }
    const_reference back() const { return m_InputList.back(); }
    reference back() { return m_InputList.back(); }

    size_t size() const { return m_InputList.size(); }

    bool empty() const { return m_InputList.empty(); }

    bool isDiscard() const { return m_bIsDiscard; }

    void append(Input* pInput) { m_InputList.push_back(pInput); }

    const_dot_iterator dot_begin() const { return m_DotAssignments.begin(); }
    dot_iterator dot_begin() { return m_DotAssignments.begin(); }
    const_dot_iterator dot_end() const { return m_DotAssignments.end(); }
    dot_iterator dot_end() { return m_DotAssignments.end(); }

    const_dot_iterator find_first_explicit_dot() const;
    dot_iterator find_first_explicit_dot();

    const_dot_iterator find_last_explicit_dot() const;
    dot_iterator find_last_explicit_dot();

    const DotAssignments& dotAssignments() const { return m_DotAssignments; }
    DotAssignments& dotAssignments() { return m_DotAssignments; }

   private:
    std::string m_Name;
    OutputSectDesc::Prolog m_Prolog;
    OutputSectDesc::Epilog m_Epilog;
    LDSection* m_pSection;
    size_t m_Order;
    bool m_bIsDiscard;
    InputList m_InputList;
    DotAssignments m_DotAssignments;
  };

  struct SHOCompare {
    bool operator()(const Output* LHS, const Output* RHS) const {
      return LHS->order() < RHS->order();
    }
  };

  typedef std::pair<const Output*, const Input*> const_mapping;
  typedef std::pair<Output*, Input*> mapping;

  typedef std::vector<Output*> OutputDescList;
  typedef OutputDescList::const_iterator const_iterator;
  typedef OutputDescList::iterator iterator;
  typedef OutputDescList::const_reference const_reference;
  typedef OutputDescList::reference reference;

  typedef OutputDescList::const_reverse_iterator const_reverse_iterator;
  typedef OutputDescList::reverse_iterator reverse_iterator;

 public:
  ~SectionMap();

  const_mapping find(const std::string& pInputFile,
                     const std::string& pInputSection) const;
  mapping find(const std::string& pInputFile, const std::string& pInputSection);

  const_iterator find(const std::string& pOutputSection) const;
  iterator find(const std::string& pOutputSection);

  std::pair<mapping, bool> insert(
      const std::string& pInputSection,
      const std::string& pOutputSection,
      InputSectDesc::KeepPolicy pPolicy = InputSectDesc::NoKeep);
  std::pair<mapping, bool> insert(const InputSectDesc& pInputDesc,
                                  const OutputSectDesc& pOutputDesc);

  bool empty() const { return m_OutputDescList.empty(); }
  size_t size() const { return m_OutputDescList.size(); }

  const_iterator begin() const { return m_OutputDescList.begin(); }
  iterator begin() { return m_OutputDescList.begin(); }
  const_iterator end() const { return m_OutputDescList.end(); }
  iterator end() { return m_OutputDescList.end(); }

  const_reference front() const { return m_OutputDescList.front(); }
  reference front() { return m_OutputDescList.front(); }
  const_reference back() const { return m_OutputDescList.back(); }
  reference back() { return m_OutputDescList.back(); }

  const_reverse_iterator rbegin() const { return m_OutputDescList.rbegin(); }
  reverse_iterator rbegin() { return m_OutputDescList.rbegin(); }
  const_reverse_iterator rend() const { return m_OutputDescList.rend(); }
  reverse_iterator rend() { return m_OutputDescList.rend(); }

  iterator insert(iterator pPosition, LDSection* pSection);

  // fixupDotSymbols - ensure the dot assignments are valid
  void fixupDotSymbols();

 private:
  bool matched(const Input& pInput,
               const std::string& pInputFile,
               const std::string& pInputSection) const;

  bool matched(const WildcardPattern& pPattern, const std::string& pName) const;

 private:
  OutputDescList m_OutputDescList;
};

}  // namespace mcld

#endif  // MCLD_OBJECT_SECTIONMAP_H_
