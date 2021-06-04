//===- EhFrame.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_EHFRAME_H_
#define MCLD_LD_EHFRAME_H_

#include "mcld/Config/Config.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Support/Allocators.h"
#include "mcld/Support/Compiler.h"

#include <llvm/ADT/StringRef.h>

#include <list>
#include <map>
#include <vector>

namespace mcld {

class Input;
class LDSection;
class Relocation;

/** \class EhFrame
 *  \brief EhFrame represents .eh_frame section
 */
class EhFrame {
 private:
  friend class Chunk<EhFrame, MCLD_SECTIONS_PER_INPUT>;

  EhFrame();
  explicit EhFrame(LDSection& pSection);

  ~EhFrame();

 public:
  enum RecordType { RECORD_UNKNOWN, RECORD_INPUT, RECORD_GENERATED };

  class CIE;
  class FDE;

  typedef std::vector<CIE*> CIEList;
  typedef CIEList::iterator cie_iterator;
  typedef CIEList::const_iterator const_cie_iterator;

  typedef std::list<FDE*> FDEList;
  typedef FDEList::iterator fde_iterator;
  typedef FDEList::const_iterator const_fde_iterator;

  typedef std::map</*offset*/ size_t, CIE*> CIEMap;

  // A super class of CIE and FDE, containing the same part
  class Record : public RegionFragment {
   public:
    explicit Record(llvm::StringRef pRegion);
    virtual ~Record();

    const llvm::StringRef getRegion() const {
      return RegionFragment::getRegion();
    }
    llvm::StringRef getRegion() { return RegionFragment::getRegion(); }
    virtual RecordType getRecordType() const { return RECORD_UNKNOWN; }

   private:
    DISALLOW_COPY_AND_ASSIGN(Record);
  };

  /** \class CIE
   *  \brief Common Information Entry.
   *  The CIE structure refers to LSB Core Spec 4.1, chap.10.6. Exception
   * Frames.
   */
  class CIE : public Record {
   public:
    explicit CIE(llvm::StringRef pRegion);
    ~CIE();

    virtual RecordType getRecordType() const { return RECORD_INPUT; }

    void setFDEEncode(uint8_t pEncode) { m_FDEEncode = pEncode; }
    uint8_t getFDEEncode() const { return m_FDEEncode; }

    void setMergeable(bool pVal = true) { m_Mergeable = pVal; }
    virtual bool getMergeable() const { return m_Mergeable; }

    void setRelocation(const Relocation& pReloc) { m_pReloc = &pReloc; }
    const Relocation* getRelocation() const { return m_pReloc; }

    void setPersonalityOffset(uint64_t pOffset) {
      m_PersonalityOffset = pOffset;
    }
    uint64_t getPersonalityOffset() const { return m_PersonalityOffset; }

    void setPersonalityName(const std::string& pStr) {
      m_PersonalityName = pStr;
    }
    const std::string& getPersonalityName() const { return m_PersonalityName; }

    void setAugmentationData(const std::string& pStr) {
      m_AugmentationData = pStr;
    }
    const std::string& getAugmentationData() const {
      return m_AugmentationData;
    }

    void add(FDE& pFDE) { m_FDEs.push_back(&pFDE); }
    void remove(FDE& pFDE) { m_FDEs.remove(&pFDE); }
    void clearFDEs() { m_FDEs.clear(); }
    size_t numOfFDEs() const { return m_FDEs.size(); }

    const_fde_iterator begin() const { return m_FDEs.begin(); }
    fde_iterator begin() { return m_FDEs.begin(); }
    const_fde_iterator end() const { return m_FDEs.end(); }
    fde_iterator end() { return m_FDEs.end(); }

   private:
    uint8_t m_FDEEncode;
    bool m_Mergeable;
    const Relocation* m_pReloc;
    uint64_t m_PersonalityOffset;
    std::string m_PersonalityName;
    std::string m_AugmentationData;
    FDEList m_FDEs;
  };

  /** \class FDE
   *  \brief Frame Description Entry
   *  The FDE structure refers to LSB Core Spec 4.1, chap.10.6. Exception
   * Frames.
   */
  class FDE : public Record {
   public:
    FDE(llvm::StringRef pRegion, CIE& pCIE);
    ~FDE();

    void setCIE(CIE& pCIE);
    const CIE& getCIE() const { return *m_pCIE; }
    CIE& getCIE() { return *m_pCIE; }

   private:
    CIE* m_pCIE;  // Referenced CIE may change when merging.
  };

  // These are created for PLT
  class GeneratedCIE : public CIE {
   public:
    explicit GeneratedCIE(llvm::StringRef pRegion);
    ~GeneratedCIE();

    virtual RecordType getRecordType() const { return RECORD_GENERATED; }
    virtual bool getMergeable() const { return true; }
  };

  class GeneratedFDE : public FDE {
   public:
    GeneratedFDE(llvm::StringRef pRegion, CIE& pCIE);
    ~GeneratedFDE();

    virtual RecordType getRecordType() const { return RECORD_GENERATED; }
  };

 public:
  static EhFrame* Create(LDSection& pSection);

  static void Destroy(EhFrame*& pSection);

  static void Clear();

  /// merge - move all data from pOther to this object.
  EhFrame& merge(const Input& pInput, EhFrame& pInFrame);

  const LDSection& getSection() const;
  LDSection& getSection();

  const SectionData* getSectionData() const { return m_pSectionData; }
  SectionData* getSectionData() { return m_pSectionData; }

  // -----  fragment  ----- //
  void addFragment(Fragment& pFrag);

  /// addCIE - add a CIE entry in EhFrame
  void addCIE(CIE& pCIE, bool pAlsoAddFragment = true);

  /// addFDE - add a FDE entry in EhFrame
  void addFDE(FDE& pFDE, bool pAlsoAddFragment = true);

  // -----  CIE  ----- //
  const_cie_iterator cie_begin() const { return m_CIEs.begin(); }
  cie_iterator cie_begin() { return m_CIEs.begin(); }
  const_cie_iterator cie_end() const { return m_CIEs.end(); }
  cie_iterator cie_end() { return m_CIEs.end(); }

  const CIE& cie_front() const { return *m_CIEs.front(); }
  CIE& cie_front() { return *m_CIEs.front(); }
  const CIE& cie_back() const { return *m_CIEs.back(); }
  CIE& cie_back() { return *m_CIEs.back(); }

  bool emptyCIEs() const { return m_CIEs.empty(); }
  size_t numOfCIEs() const { return m_CIEs.size(); }
  size_t numOfFDEs() const;

  const CIEMap& getCIEMap() const { return m_FoundCIEs; }
  CIEMap& getCIEMap() { return m_FoundCIEs; }

 public:
  size_t computeOffsetSize();

  /// getDataStartOffset - Get the offset after length and ID field.
  /// The offset is 8byte for 32b, and 16byte for 64b.
  /// We can just use "BITCLASS/4" to represent offset.
  template <size_t BITCLASS>
  static size_t getDataStartOffset() {
    return BITCLASS / 4;
  }

 private:
  // We needs to check if it is mergeable and check personality name
  // before merging them. The important note is we must do this after
  // ALL readSections done, that is the reason why we don't check this
  // immediately when reading.
  void setupAttributes(const LDSection* reloc_sect);
  void removeDiscardedFDE(CIE& pCIE, const LDSection* pRelocEhFrameSect);

 private:
  void removeAndUpdateCIEForFDE(EhFrame& pInFrame,
                                CIE& pInCIE,
                                CIE& pOutCIE,
                                const LDSection* reloc_sect);
  void moveInputFragments(EhFrame& pInFrame);
  void moveInputFragments(EhFrame& pInFrame, CIE& pInCIE, CIE* pOutCIE = 0);

 private:
  LDSection* m_pSection;
  SectionData* m_pSectionData;

  // Each eh_frame has a list of CIE, and each CIE has a list of FDE
  // pointing to the CIE itself. This is used by management when we are
  // processing eh_frame merge.
  // However, don't forget we need to handle the Fragments inside SectionData
  // correctly since they are truly used when output emission.
  CIEList m_CIEs;

  // We need this map to find the corresponding CIE for FDE. Not all FDE point
  // to the nearest CIE.
  CIEMap m_FoundCIEs;

 private:
  DISALLOW_COPY_AND_ASSIGN(EhFrame);
};

bool operator==(const EhFrame::CIE&, const EhFrame::CIE&);

}  // namespace mcld

#endif  // MCLD_LD_EHFRAME_H_
