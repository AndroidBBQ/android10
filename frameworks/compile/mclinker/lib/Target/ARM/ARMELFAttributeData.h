//===- ARMELFAttributeData.h ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_ARMELFATTRIBUTEDATA_H_
#define TARGET_ARM_ARMELFATTRIBUTEDATA_H_

#include "mcld/Target/ELFAttributeData.h"
#include "mcld/Target/ELFAttributeValue.h"

#include <map>
#include <string>

namespace mcld {

/** \class ARMELFAttributeData
 *  \brief ARMELFAttributeData handles public ("aeabi") attributes subsection in
 *  ARM ELF.
 *
 */
class ARMELFAttributeData : public ELFAttributeData {
 public:
  enum Tag {
    // 0-3 are generic and are defined in ELFAttributeData.
    Tag_CPU_raw_name = 4,
    Tag_CPU_name = 5,
    Tag_CPU_arch = 6,
    Tag_CPU_arch_profile = 7,
    Tag_ARM_ISA_use = 8,
    Tag_THUMB_ISA_use = 9,
    Tag_FP_arch = 10,
    Tag_WMMX_arch = 11,
    Tag_Advanced_SIMD_arch = 12,
    Tag_PCS_config = 13,
    Tag_ABI_PCS_R9_use = 14,
    Tag_ABI_PCS_RW_data = 15,
    Tag_ABI_PCS_RO_data = 16,
    Tag_ABI_PCS_GOT_use = 17,
    Tag_ABI_PCS_wchar_t = 18,
    Tag_ABI_FP_rounding = 19,
    Tag_ABI_FP_denormal = 20,
    Tag_ABI_FP_exceptions = 21,
    Tag_ABI_FP_user_exceptions = 22,
    Tag_ABI_FP_number_model = 23,
    Tag_ABI_align_needed = 24,
    Tag_ABI_align_preserved = 25,
    Tag_ABI_enum_size = 26,
    Tag_ABI_HardFP_use = 27,
    Tag_ABI_VFP_args = 28,
    Tag_ABI_WMMX_args = 29,
    Tag_ABI_optimization_goals = 30,
    Tag_ABI_FP_optimization_goals = 31,
    Tag_compatibility = 32,

    Tag_CPU_unaligned_access = 34,

    Tag_FP_HP_extension = 36,

    Tag_ABI_FP_16bit_format = 38,

    Tag_MPextension_use = 42,

    Tag_DIV_use = 44,

    Tag_nodefaults = 64,
    Tag_also_compatible_with = 65,
    Tag_T2EE_use = 66,
    Tag_conformance = 67,
    Tag_Virtualization_use = 68,

    Tag_MPextension_use_legacy = 70,

    Tag_Max = Tag_MPextension_use_legacy,

    // Alias
    Tag_VFP_arch = Tag_FP_arch,
    Tag_ABI_align8_needed = Tag_ABI_align_needed,
    Tag_ABI_align8_preserved = Tag_ABI_align_preserved,
    Tag_VFP_HP_extension = Tag_FP_HP_extension
  };

  // For Tag_CPU_arch
  enum {
    CPU_Arch_ARM_Pre_V4,
    CPU_Arch_ARM_V4,     // e.g., SA110
    CPU_Arch_ARM_V4T,    // e.g., ARM7TDMI
    CPU_Arch_ARM_V5T,    // e.g., ARM9TDMI
    CPU_Arch_ARM_V5TE,   // e.g., ARM946E-S
    CPU_Arch_ARM_V5TEJ,  // e.g., ARM926EJ-S
    CPU_Arch_ARM_V6,     // e.g., ARM1136J-S
    CPU_Arch_ARM_V6KZ,   // e.g., ARM1176JZ-S
    CPU_Arch_ARM_V6T2,   // e.g., ARM1156T2F-S
    CPU_Arch_ARM_V6K,    // e.g., ARM1136J-S
    CPU_Arch_ARM_V7,     // e.g., Cortex A8, Cortex M3
    CPU_Arch_ARM_V6_M,   // e.g., Cortex M1
    CPU_Arch_ARM_V6S_M,  // e.g., v6-M with the value of System extensions
    CPU_Arch_ARM_V7E_M,  // e.g., v7-M with DSP extensions
    CPU_Arch_ARM_V8,

    CPU_Arch_Max = CPU_Arch_ARM_V8,

    // This is a pseudo-architecture to describe an architecture mixed with
    // the subset of armv4t and armv6-m. This never appears in the value of
    // Tag_CPU_arch.
    CPU_Arch_ARM_V4T_Plus_V6_M = (CPU_Arch_Max + 1),

    CPU_Arch_Plus_Pseudo_Max = CPU_Arch_ARM_V4T_Plus_V6_M,
  };

  // For Tag_CPU_arch_profile
  enum {
    Arch_Profile_None = 0,
    Arch_Profile_Application = 'A',
    Arch_Profile_Realtime = 'R',
    Arch_Profile_Microcontroller = 'M',
    Arch_Profile_RealOrApp = 'S'
  };

  // For Tag_ABI_enum_size
  enum {
    Enum_Unused,
    Enum_Smallest_Container,
    Enum_32bit_Container,
    Enum_Containerized_As_Possible
  };

  // For Tag_ABI_PCS_R9_use
  enum { R9_V6, R9_SB, R9_TLS, R9_Unused };

  // For Tag_ABI_PCS_RW_data
  enum {
    RW_data_Absolute,
    RW_data_PC_Relative,
    RW_data_SB_Relative,
    RW_data_unused
  };

 public:
  // ARM [ABI-addenda], 2.2.2: A public attributes subsection is named aeabi.
  ARMELFAttributeData()
      : ELFAttributeData("aeabi"),
        m_CurrentCPUArch(-1),
        m_DIVUseInitialized(false),
        m_HardFPUseInitialized(false) {}

 public:
  virtual const ELFAttributeValue* getAttributeValue(TagType pTag) const;

  virtual std::pair<ELFAttributeValue*, bool> getOrCreateAttributeValue(
      TagType pTag);

  virtual bool preMerge(const Input& pInput) {
    // Reset states.
    m_CPUArch = -1;
    m_CPUName.clear();
    m_CPURawName.clear();
    m_SecondaryCPUArch = -1;
    m_VFPArgs = -1;
    m_FPArch = -1;
    m_HardFPUse = -1;
    m_MPextensionUse = -1;
    m_DIVUse = -1;
    return true;
  }

  virtual bool merge(const LinkerConfig& pConfig,
                     const Input& pInput,
                     TagType pTag,
                     const ELFAttributeValue& pInAttr);

  virtual bool postMerge(const LinkerConfig& pConfig, const Input& pInput);

  virtual size_t sizeOutput() const;

  virtual size_t emit(char* pBuf) const;

  virtual bool usingThumb() const;

  virtual bool usingThumb2() const;

 private:
  /// GetAttributeValueType - obtain the value type of the indicated tag.
  static unsigned int GetAttributeValueType(TagType pTag);

 private:
  // The storage for known tags which is indexed by the tag
  ELFAttributeValue m_Attrs[Tag_Max + 1];

  // The storage for unknown tags
  typedef std::map<TagType, ELFAttributeValue> UnknownAttrsMap;
  UnknownAttrsMap m_UnknownAttrs;

  // This is a cache for the current output architecture calculate from of
  // Tag_CPU_arch and Tag_also_compatible_with.
  int m_CurrentCPUArch;

  // Value of Tag_DIV_use and Tag_ABI_HardFP_use requires further examination
  // for the every time adding to the output. These booleans are initialized to
  // false and set to true until the corresponding attribute is initialized.
  bool m_DIVUseInitialized;
  bool m_HardFPUseInitialized;

  // These attributes have dependency with each other. During the merge, we
  // record their attribute values in the associated variables as follows and
  // process them in postmerge() (when all other attributes are settled down.)

  // Record the value of input Tag_CPU_arch.
  int m_CPUArch;

  // Record the value of input Tag_CPU_name.
  std::string m_CPUName;

  // Record the value of input Tag_CPU_raw_name.
  std::string m_CPURawName;

  // Record the value of input Tag_FP_arch.
  int m_FPArch;

  // Record the value of input Tag_ABI_HardFP_use.
  int m_HardFPUse;

  // Record the value of input Tag_also_compatible_with.
  int m_SecondaryCPUArch;

  // Record the value of input Tag_ABI_VFP_args.
  int m_VFPArgs;

  // Record the value of input Tag_MPextension_use and
  // Tag_MPextension_use_legacy.
  int m_MPextensionUse;

  // Record the value of input Tag_DIV_use.
  int m_DIVUse;
};

}  // namespace mcld

#endif  // TARGET_ARM_ARMELFATTRIBUTEDATA_H_
