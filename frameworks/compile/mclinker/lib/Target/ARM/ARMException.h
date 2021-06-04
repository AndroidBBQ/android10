//===- ARMException.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMEXCEPTION_H_
#define TARGET_ARM_ARMEXCEPTION_H_

#include "mcld/LD/LDSection.h"

#include <llvm/ADT/PointerUnion.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ELF.h>

#include <map>
#include <memory>
#include <string>

namespace mcld {

class Fragment;
class Input;
class LDSection;
class Module;
class RegionFragment;
class RelocData;

/// ARMExSectionTuple - Tuple of associated exception handling sections
class ARMExSectionTuple {
 public:
  ARMExSectionTuple()
      : m_pTextSection(NULL),
        m_pExIdxSection(NULL) {
  }

  LDSection* getTextSection() const {
    return m_pTextSection;
  }

  LDSection* getExIdxSection() const {
    return m_pExIdxSection;
  }

  void setTextSection(LDSection* pSection) {
    m_pTextSection = pSection;
  }

  void setExIdxSection(LDSection* pSection) {
    m_pExIdxSection = pSection;
  }

  RegionFragment* getTextFragment() const {
    return m_pTextFragment;
  }

  RegionFragment* getExIdxFragment() const {
    return m_pExIdxFragment;
  }

  void setTextFragment(RegionFragment* pFragment) {
    m_pTextFragment = pFragment;
  }

  void setExIdxFragment(RegionFragment* pFragment) {
    m_pExIdxFragment = pFragment;
  }

 private:
  // .text section
  union {
    LDSection*      m_pTextSection;
    RegionFragment* m_pTextFragment;
  };

  // .ARM.exidx section
  union {
    LDSection*      m_pExIdxSection;
    RegionFragment* m_pExIdxFragment;
  };
};

/// ARMInputExMap - ARM exception handling section mapping of a mcld::Input.
class ARMInputExMap {
 public:
  typedef std::map<LDSection*, std::unique_ptr<ARMExSectionTuple> > SectMap;
  typedef SectMap::iterator iterator;
  typedef SectMap::const_iterator const_iterator;

 public:
  // create - Build the exception handling section mapping of a mcld::Input.
  static std::unique_ptr<ARMInputExMap> create(Input &input);

  /// getByExSection - Get the ARMExSectionTuple by the address of the
  /// .ARM.exidx section.
  ARMExSectionTuple* getByExSection(LDSection &pSect) const {
    assert(pSect.type() == llvm::ELF::SHT_ARM_EXIDX);
    SectMap::const_iterator it = m_SectToExData.find(&pSect);
    if (it == m_SectToExData.end()) {
      return NULL;
    }
    return it->second.get();
  }

  /// getOrCreate - Get an existing or create a new ARMExSectionTuple which is
  /// associated with the address of the .ARM.exidx section.
  ARMExSectionTuple* getOrCreateByExSection(LDSection &pSect) {
    assert(pSect.type() == llvm::ELF::SHT_ARM_EXIDX);
    std::unique_ptr<ARMExSectionTuple>& result = m_SectToExData[&pSect];
    if (!result) {
      result.reset(new ARMExSectionTuple());
    }
    return result.get();
  }

  /// begin - return the iterator to the begin of the map
  iterator       begin()       { return m_SectToExData.begin(); }
  const_iterator begin() const { return m_SectToExData.begin(); }

  /// end - return the iterator to the end of the map
  iterator       end()       { return m_SectToExData.end(); }
  const_iterator end() const { return m_SectToExData.end(); }

  /// erase - remove an entry from the map
  void erase(iterator it) { m_SectToExData.erase(it); }

 private:
  ARMInputExMap() = default;

 private:
  SectMap m_SectToExData;
};

/// ARMExData - ARM exception handling data of a mcld::Module.
class ARMExData {
 private:
  typedef std::map<Input*, std::unique_ptr<ARMInputExMap> > InputMap;

  typedef std::map<const Fragment*, ARMExSectionTuple*> ExIdxMap;

 public:
  // create - Build the exception handling section mapping of a mcld::Module.
  static std::unique_ptr<ARMExData> create(Module &module);

  // addInputMap - register the ARMInputExMap with associated pInput
  void addInputMap(Input* pInput,
                   std::unique_ptr<ARMInputExMap> pExMap);

  // getInputMap - get the ARMInputExMap corresponding to pInput
  ARMInputExMap* getInputMap(Input* pInput) const {
    InputMap::const_iterator it = m_Inputs.find(pInput);
    if (it == m_Inputs.end()) {
      return NULL;
    }
    return it->second.get();
  }

  // getTupleByExIdx - get the ARMExSectionTuple corresponding to pExIdxFragment
  ARMExSectionTuple* getTupleByExIdx(const Fragment* pExIdxFragment) const {
    ExIdxMap::const_iterator it = m_ExIdxToTuple.find(pExIdxFragment);
    if (it == m_ExIdxToTuple.end()) {
      return NULL;
    }
    return it->second;
  }

 private:
  ARMExData() = default;

 private:
  // Map from Input to ARMInputExMap
  InputMap m_Inputs;

  // Map from .ARM.exidx RegionFragment to ARMExSectionTuple
  ExIdxMap m_ExIdxToTuple;
};

}  // namespace mcld

#endif  // TARGET_ARM_ARMEXCEPTION_H_
