//===- ARMELFAttributeData.h ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ARMELFAttributeData.h"

#include "mcld/LinkerConfig.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/LEB128.h"
#include "mcld/Support/MsgHandling.h"
#include <llvm/ADT/STLExtras.h>

namespace mcld {

const ELFAttributeValue* ARMELFAttributeData::getAttributeValue(
    TagType pTag) const {
  if (pTag <= Tag_Max) {
    const ELFAttributeValue& attr_value = m_Attrs[pTag];

    if (attr_value.isInitialized()) {
      return &attr_value;
    } else {
      // Don't return uninitialized attribute value.
      return NULL;
    }
  } else {
    UnknownAttrsMap::const_iterator attr_it = m_UnknownAttrs.find(pTag);

    if (attr_it == m_UnknownAttrs.end()) {
      return NULL;
    } else {
      return &attr_it->second;
    }
  }
}

std::pair<ELFAttributeValue*, bool>
ARMELFAttributeData::getOrCreateAttributeValue(TagType pTag) {
  ELFAttributeValue* attr_value = NULL;

  if (pTag <= Tag_Max) {
    attr_value = &m_Attrs[pTag];
  } else {
    // An unknown tag encounterred.
    attr_value = &m_UnknownAttrs[pTag];
  }

  assert(attr_value != NULL);

  // Setup the value type.
  if (!attr_value->isUninitialized()) {
    return std::make_pair(attr_value, false);
  } else {
    attr_value->setType(GetAttributeValueType(pTag));
    return std::make_pair(attr_value, true);
  }
}

unsigned int ARMELFAttributeData::GetAttributeValueType(TagType pTag) {
  // See ARM [ABI-addenda], 2.2.6.
  switch (pTag) {
    case Tag_compatibility: {
      return (ELFAttributeValue::Int | ELFAttributeValue::String);
    }
    case Tag_nodefaults: {
      return (ELFAttributeValue::Int | ELFAttributeValue::NoDefault);
    }
    case Tag_CPU_raw_name:
    case Tag_CPU_name: {
      return ELFAttributeValue::String;
    }
    default: {
      if (pTag < 32)
        return ELFAttributeValue::Int;
      else
        return ((pTag & 1) ? ELFAttributeValue::String
                           : ELFAttributeValue::Int);
    }
  }
  // unreachable
}

//===--------------------------------------------------------------------===//
// Helper Functions for merge()
//===--------------------------------------------------------------------===//

namespace {

/*
 * Helper function to decode value in Tag_also_compatible_with.
 *
 * @ref ARM [ABI-addenda], 2.3.7.3
 */
static int decode_secondary_compatibility_attribute(
    const ELFAttributeValue& pValue) {
  // The encoding of Tag_also_compatible_with is:
  //
  // Tag_also_compatible_with (=65), NTSB: data
  //
  // The data can be either an ULEB128-encoded number followed by a NULL byte or
  // a NULL-terminated string. Currently, only the following byte sequence in
  // data are currently defined:
  //
  // Tag_CPU_arch (=6) [The arch] 0
  assert((pValue.type() == ELFAttributeValue::String) &&
         "Value of Tag_also_compatible_with must be a string!");

  const std::string& data = pValue.getStringValue();

  // Though the integer is in LEB128 format, but they occupy only 1 byte in
  // currently defined value.
  if (data.length() < 2)
    // Must have a byte for Tag_CPU_arch (=6)
    //           a byte for specifying the CPU architecture (CPU_Arch_ARM_*)
    //
    // Currently, the 2nd byte can only be v4T (=2) or v6-M (=11).
    return -1;

  if ((static_cast<uint8_t>(data[0]) == ARMELFAttributeData::Tag_CPU_arch) &&
      ((data[1] == ARMELFAttributeData::CPU_Arch_ARM_V4T) ||
       (data[1] == ARMELFAttributeData::CPU_Arch_ARM_V6_M)))
    return static_cast<uint32_t>(data[1]);

  // Tag_also_compatible_with can be safely ignored.
  return -1;
}

/*
 * This helper array keeps the ordering of the values in attributes such as
 * Tag_ABI_align_needed which are sored as 1 > 2 > 0.
 */
static const int value_ordering_120[] = {0, 2, 1};

}  // anonymous namespace

//===--------------------------------------------------------------------===//
// End Helper Functions for merge()
//===--------------------------------------------------------------------===//

bool ARMELFAttributeData::merge(const LinkerConfig& pConfig,
                                const Input& pInput,
                                TagType pTag,
                                const ELFAttributeValue& pInAttr) {
  // Pre-condition
  //  1. The out_attr must be initailized and has value of the same type as
  //     pInAttr.
  //  2. The value helf by out_attr and pInAttr must be different.
  ELFAttributeValue& out_attr = m_Attrs[pTag];

  // Attribute in the output must have value assigned.
  assert(out_attr.isInitialized() && "No output attribute to be merged!");

  switch (pTag) {
    case Tag_CPU_arch: {
      // Need value of Tag_also_compatible_with in the input for merge.
      if (pInAttr.getIntValue() <= CPU_Arch_Max) {
        m_CPUArch = pInAttr.getIntValue();
      } else {
        error(diag::error_unknown_cpu_arch) << pInput.name();
        return false;
      }
      break;
    }
    case Tag_CPU_name: {
      // need value of Tag_CPU_arch in the input for merge
      m_CPUName = pInAttr.getStringValue();
      break;
    }
    case Tag_CPU_raw_name: {
      // need value of Tag_CPU_arch in the input for merge
      m_CPURawName = pInAttr.getStringValue();
      break;
    }
    case Tag_FP_arch: {
      // need value of Tag_HardFP_use in the input for merge
      m_FPArch = pInAttr.getIntValue();
      break;
    }
    case Tag_ABI_HardFP_use: {
      // need value of Tag_FP_arch in the input for merge
      m_HardFPUse = pInAttr.getIntValue();
      break;
    }
    case Tag_also_compatible_with: {
      // need value of Tag_CPU_arch in the input for merge
      m_SecondaryCPUArch = decode_secondary_compatibility_attribute(pInAttr);
      break;
    }
    case Tag_ABI_VFP_args: {
      // need value of Tag_ABI_FP_number_model in the input for merge
      m_VFPArgs = pInAttr.getIntValue();
      break;
    }
    // The value of these tags are integers and after merge, only the greatest
    // value held by pInAttr and out_attr goes into output.
    case Tag_ARM_ISA_use:
    case Tag_THUMB_ISA_use:
    case Tag_WMMX_arch:
    case Tag_Advanced_SIMD_arch:
    case Tag_ABI_FP_rounding:
    case Tag_ABI_FP_exceptions:
    case Tag_ABI_FP_user_exceptions:
    case Tag_ABI_FP_number_model:
    case Tag_FP_HP_extension:
    case Tag_CPU_unaligned_access:
    case Tag_T2EE_use: {
      assert((out_attr.type() == ELFAttributeValue::Int) &&
             (pInAttr.type() == ELFAttributeValue::Int) &&
             "should have integer parameeter!");
      if (pInAttr.getIntValue() > out_attr.getIntValue())
        out_attr.setIntValue(pInAttr.getIntValue());
      break;
    }
    // The value of these tags are integers and after merge, only the smallest
    // value held by pInAttr and out_attr goes into output.
    case Tag_ABI_align_preserved:
    case Tag_ABI_PCS_RO_data: {
      assert((out_attr.type() == ELFAttributeValue::Int) &&
             (pInAttr.type() == ELFAttributeValue::Int) &&
             "should have integer parameeter!");
      if (pInAttr.getIntValue() < out_attr.getIntValue())
        out_attr.setIntValue(pInAttr.getIntValue());
      break;
    }
    // The values of these attributes are sorted as 1 > 2 > 0. And the greater
    // value becomes output.
    case Tag_ABI_align_needed:
    case Tag_ABI_FP_denormal:
    case Tag_ABI_PCS_GOT_use: {
      const int in_val = pInAttr.getIntValue();
      const int out_val = out_attr.getIntValue();

      if (in_val <= 2) {
        if (out_val <= 2) {
          // Use value_ordering_120 to determine the ordering.
          if (value_ordering_120[in_val] > value_ordering_120[out_val]) {
            out_attr.setIntValue(in_val);
          }
        }
      } else {
        // input value > 2, for future-proofing
        if (in_val > out_val) {
          out_attr.setIntValue(in_val);
        }
      }
      break;
    }
    // These tags use the first value ever seen.
    case Tag_ABI_optimization_goals:
    case Tag_ABI_FP_optimization_goals: {
      break;
    }
    // Tag_CPU_arch_profile
    case Tag_CPU_arch_profile: {
      if (pInAttr.getIntValue() == Arch_Profile_None)
        return true;

      switch (out_attr.getIntValue()) {
        case Arch_Profile_None: {
          out_attr.setIntValue(pInAttr.getIntValue());
          break;
        }
        case Arch_Profile_RealOrApp: {
          if (pInAttr.getIntValue() != Arch_Profile_Microcontroller)
            out_attr.setIntValue(pInAttr.getIntValue());
          else
            warning(diag::warn_mismatch_cpu_arch_profile)
                << pInAttr.getIntValue() << pInput.name();
          break;
        }
        default: {
          // out_attr is Arch_Profile_Application or Arch_Profile_Realtime or
          // Arch_Profile_Microcontroller.
          if ((pInAttr.getIntValue() == Arch_Profile_RealOrApp) &&
              (out_attr.getIntValue() != Arch_Profile_Microcontroller)) {
            // do nothing
          } else {
            if (pConfig.options().warnMismatch())
              warning(diag::warn_mismatch_cpu_arch_profile)
                  << pInAttr.getIntValue() << pInput.name();
          }
          break;
        }
      }
      break;
    }
    // Tag_MPextension_use and Tag_MPextension_use_legacy
    case Tag_MPextension_use:
    case Tag_MPextension_use_legacy: {
      if (m_MPextensionUse < 0) {
        m_MPextensionUse = pInAttr.getIntValue();
      } else {
        if (static_cast<unsigned>(m_MPextensionUse) != pInAttr.getIntValue()) {
          warning(diag::error_mismatch_mpextension_use) << pInput.name();
        }
      }
      break;
    }
    // Tag_DIV_use
    case Tag_DIV_use: {
      if (pInAttr.getIntValue() == 2) {
        // 2 means the code was permitted to use SDIV/UDIV in anyway.
        out_attr.setIntValue(2);
      } else {
        // Merge until settling down Tag_CPU_arch.
        m_DIVUse = pInAttr.getIntValue();
      }
      break;
    }
    // Tag_ABI_enum_size
    case Tag_ABI_enum_size: {
      if ((out_attr.getIntValue() == Enum_Unused) ||
          (out_attr.getIntValue() == Enum_Containerized_As_Possible))
        out_attr.setIntValue(pInAttr.getIntValue());
      else if (pInAttr.getIntValue() != Enum_Containerized_As_Possible &&
               pConfig.options().warnMismatch())
        warning(diag::warn_mismatch_enum_size)
            << pInput.name() << pInAttr.getIntValue() << out_attr.getIntValue();
      break;
    }
    // Tag_ABI_FP_16bit_format
    case Tag_ABI_FP_16bit_format: {
      // 0: doesn't use any 16-bit FP number
      // 1: use IEEE 754 format 16-bit FP number
      // 2: use VFPv3/Advanced SIMD "alternative format" 16-bit FP number
      if (pInAttr.getIntValue() != 0) {
        if (out_attr.getIntValue() == 0) {
          out_attr.setIntValue(pInAttr.getIntValue());
        } else {
          if (pConfig.options().warnMismatch())
            warning(diag::warn_mismatch_fp16_format) << pInput.name();
        }
      }
      break;
    }
    // Tag_nodefaults
    case Tag_nodefaults: {
      // There's nothing to do for this tag. It doesn't have an actual value.
      break;
    }
    // Tag_conformance
    case Tag_conformance: {
      // Throw away the value if the attribute value doesn't match.
      if (out_attr.getStringValue() != pInAttr.getStringValue())
        out_attr.setStringValue("");
      break;
    }
    // Tag_Virtualization_use
    case Tag_Virtualization_use: {
      // 0: No use of any virtualization extension
      // 1: TrustZone
      // 2: Virtualization extension such as HVC and ERET
      // 3: TrustZone and virtualization extension are permitted
      if (pInAttr.getIntValue() != 0) {
        if (out_attr.getIntValue() == 0) {
          out_attr.setIntValue(pInAttr.getIntValue());
        } else {
          if ((out_attr.getIntValue() <= 3) && (pInAttr.getIntValue() <= 3)) {
            // Promote to 3
            out_attr.setIntValue(3);
          } else {
            warning(diag::warn_unrecognized_virtualization_use)
                << pInput.name() << pInAttr.getIntValue();
          }
        }
      }
      break;
    }
    // Tag_ABI_WMMX_args
    case Tag_ABI_WMMX_args: {
      // There's no way to merge this value (i.e., objects contain different
      // value in this tag are definitely incompatible.)
      if (pConfig.options().warnMismatch())
        warning(diag::warn_mismatch_abi_wmmx_args) << pInput.name();
      break;
    }
    // Tag_PCS_config
    case Tag_PCS_config: {
      // 0 means no standard configuration used or no information recorded.
      if (pInAttr.getIntValue() != 0) {
        if (out_attr.getIntValue() == 0)
          out_attr.setIntValue(pInAttr.getIntValue());
        else {
          // Different values in these attribute are conflict
          if (pConfig.options().warnMismatch())
            warning(diag::warn_mismatch_pcs_config) << pInput.name();
        }
      }
      break;
    }
    // Tag_ABI_PCS_R9_use
    case Tag_ABI_PCS_R9_use: {
      if (pInAttr.getIntValue() != R9_Unused) {
        if (out_attr.getIntValue() == R9_Unused)
          out_attr.setIntValue(pInAttr.getIntValue());
        else {
          if (pConfig.options().warnMismatch())
            warning(diag::warn_mismatch_r9_use) << pInput.name();
        }
      }
      break;
    }
    // Tag_ABI_PCS_RW_data
    case Tag_ABI_PCS_RW_data: {
      if (pInAttr.getIntValue() == RW_data_SB_Relative) {
        // Require using R9 as SB (global Static Base register).
        if ((out_attr.getIntValue() != R9_Unused) &&
            (out_attr.getIntValue() != R9_SB) &&
            pConfig.options().warnMismatch())
          warning(diag::warn_mismatch_r9_use) << pInput.name();
      }
      // Choose the smaller value
      if (pInAttr.getIntValue() < out_attr.getIntValue())
        out_attr.setIntValue(pInAttr.getIntValue());
      break;
    }
    // Tag_ABI_PCS_wchar_t
    case Tag_ABI_PCS_wchar_t: {
      // 0: no use of wchar_t
      // 2: sizeof(wchar_t) = 2
      // 4: sizeof(wchar_t) = 4
      if (pInAttr.getIntValue() != 0) {
        if (out_attr.getIntValue() == 0)
          out_attr.setIntValue(pInAttr.getIntValue());
        else {
          if (pConfig.options().warnMismatch())
            warning(diag::warn_mismatch_wchar_size) << pInput.name()
                                                    << pInAttr.getIntValue()
                                                    << out_attr.getIntValue();
        }
      }
      break;
    }
    default: {
      // Handle unknown attributes:
      //
      // Since we don't know how to merge the value of unknown attribute, we
      // have to ignore it. There're two rules related to the processing (See
      // ARM [ABI-addenda] 2.2.6, Coding extensibility and compatibility.):
      //
      // 1. For tag N where N >= 128, tag N has the same properties as
      //    tag N % 128.
      // 2. Tag 64-127 can be safely ignored.
      // 3. Tag 0-63 must be comprehended, therefore we cannot ignore.
      if (pConfig.options().warnMismatch()) {
        if ((pTag & 127) < 64) {
          warning(diag::warn_unknown_mandatory_attribute) << pTag
                                                          << pInput.name();
        } else {
          warning(diag::warn_unknown_attribute) << pTag << pInput.name();
        }
      }
      break;
    }
  }
  return true;
}

//===--------------------------------------------------------------------===//
// Helper Functions for postMerge()
//===--------------------------------------------------------------------===//

namespace {

/*
 * Helper function to encode value in Tag_also_compatible_with.
 *
 * @ref ARM [ABI-addenda], 2.3.7.3
 */
static void encode_secondary_compatibility_attribute(ELFAttributeValue& pValue,
                                                     int pArch) {
  if ((pArch < 0) || (pArch > ARMELFAttributeData::CPU_Arch_Max)) {
    pValue.setStringValue("");
  } else {
    char new_value[] = {
        ARMELFAttributeData::Tag_CPU_arch, static_cast<char>(pArch), 0};
    pValue.setStringValue(std::string(new_value, sizeof(new_value)));
  }
  return;
}

/*
 * Combine the main and secondary CPU arch value
 */
static int calculate_cpu_arch(int cpu_arch, int secondary_arch) {
  // short-circuit
  if ((secondary_arch < 0) ||
      ((cpu_arch + secondary_arch) != (ARMELFAttributeData::CPU_Arch_ARM_V4T +
                                       ARMELFAttributeData::CPU_Arch_ARM_V6_M)))
    return cpu_arch;

  if ((cpu_arch == ARMELFAttributeData::CPU_Arch_ARM_V4T) &&
      (secondary_arch == ARMELFAttributeData::CPU_Arch_ARM_V6_M))
    return ARMELFAttributeData::CPU_Arch_ARM_V4T_Plus_V6_M;
  else if ((cpu_arch == ARMELFAttributeData::CPU_Arch_ARM_V6_M) &&
           (secondary_arch == ARMELFAttributeData::CPU_Arch_ARM_V4T))
    return ARMELFAttributeData::CPU_Arch_ARM_V4T_Plus_V6_M;
  else
    return cpu_arch;
}

/*
 * Given a CPU arch X and a CPU arch Y in which Y is newer than X, the value in
 * cpu_compatibility_table[X][Y] is the CPU arch required to run ISA both from X
 * and Y. 0 in the table means unreachable and -1 means conflict architecture
 * profile.
 */
#define CPU(C)  ARMELFAttributeData::CPU_Arch_ARM_ ## C
static const int cpu_compatibility_table[][CPU(V4T_Plus_V6_M) + 1] = {
    /* old\new          ARM v6T2    ARM v6K   ARM v7   ARM v6-M   ARM v6S-M   ARM v7E-M    ARMv8, ARM v4t + v6-M     */  // NOLINT
    /* Pre v4     */ { CPU(V6T2),  CPU(V6K), CPU(V7),        -1,         -1,         -1,      -1,       -1           },  // NOLINT
    /* ARM v4     */ { CPU(V6T2),  CPU(V6K), CPU(V7),        -1,         -1,         -1,      -1,       -1           },  // NOLINT
    /* ARM v4T    */ { CPU(V6T2),  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V4T)           },  // NOLINT
    /* ARM v5T    */ { CPU(V6T2),  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V5T)           },  // NOLINT
    /* ARM v5TE   */ { CPU(V6T2),  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V5TE)          },  // NOLINT
    /* ARM v5TEJ  */ { CPU(V6T2),  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V5TEJ)         },  // NOLINT
    /* ARM v6     */ { CPU(V6T2),  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V6)            },  // NOLINT
    /* ARM v6KZ   */ {   CPU(V7), CPU(V6KZ), CPU(V7), CPU(V6KZ),  CPU(V6KZ), CPU(V7E_M), CPU(V8), CPU(V6KZ)          },  // NOLINT
    /* ARM v6T2   */ { CPU(V6T2),   CPU(V7), CPU(V7),   CPU(V7),    CPU(V7), CPU(V7E_M), CPU(V8), CPU(V6T2)          },  // NOLINT
    /* ARM v6K    */ {         0,  CPU(V6K), CPU(V7),  CPU(V6K),   CPU(V6K), CPU(V7E_M), CPU(V8), CPU(V6K)           },  // NOLINT
    /* ARM v7     */ {         0,         0, CPU(V7),   CPU(V7),    CPU(V7), CPU(V7E_M), CPU(V8), CPU(V7)            },  // NOLINT
    /* ARM v6-M   */ {         0,         0,       0, CPU(V6_M), CPU(V6S_M), CPU(V7E_M), CPU(V8), CPU(V6_M)          },  // NOLINT
    /* ARM v6S-M  */ {         0,         0,       0,         0, CPU(V6S_M), CPU(V7E_M), CPU(V8), CPU(V6S_M)         },  // NOLINT
    /* ARM v7E-M  */ {         0,         0,       0,         0,          0, CPU(V7E_M), CPU(V8), CPU(V7E_M)         },  // NOLINT
    /* ARM v8     */ {         0,         0,       0,         0,          0,          0, CPU(V8), CPU(V8)            },  // NOLINT
    /* v4T + v6-M */ {         0,         0,       0,         0,          0,          0,       0, CPU(V4T_Plus_V6_M) }   // NOLINT
};

/*
 * Helper function to determine the merge of two different CPU arch.
 */
static int merge_cpu_arch(int out_cpu_arch, int in_cpu_arch) {
  if (out_cpu_arch > CPU(V4T_Plus_V6_M))
    return in_cpu_arch;

  int new_cpu_arch, old_cpu_arch;
  if (out_cpu_arch > in_cpu_arch) {
    new_cpu_arch = out_cpu_arch;
    old_cpu_arch = in_cpu_arch;
  } else {
    new_cpu_arch = in_cpu_arch;
    old_cpu_arch = out_cpu_arch;
  }

  // No need to check the compatibility since the CPU architectures before
  // V6KZ add features monotonically.
  if (new_cpu_arch <= CPU(V6KZ))
    return new_cpu_arch;

  return cpu_compatibility_table[old_cpu_arch][new_cpu_arch - CPU(V6T2)];
}
#undef CPU

/*
 * Generic CPU name is used when Tag_CPU_name is unable to guess during the
 * merge of Tag_CPU_arch.
 */
static const char* generic_cpu_name_table[] = {
    /* Pre v4    */ "Pre v4",
    /* Pre v4    */ "ARM v4",
    /* ARM v4T   */ "ARM v4T",
    /* ARM v5T   */ "ARM v5T",
    /* ARM v5TE  */ "ARM v5TE",
    /* ARM v5TEJ */ "ARM v5TEJ",
    /* ARM v6    */ "ARM v6",
    /* ARM v6KZ  */ "ARM v6KZ",
    /* ARM v6T2  */ "ARM v6T2",
    /* ARM v6K   */ "ARM v6K",
    /* ARM v7    */ "ARM v7",
    /* ARM v6-M  */ "ARM v6-M",
    /* ARM v6S-M */ "ARM v6S-M",
    /* ARM v7E-M */ "ARM v7E-M",
    /* ARM v8    */ "ARM v8",
};

static const char* get_generic_cpu_name(int cpu_arch) {
  assert(static_cast<size_t>(cpu_arch) <
         (sizeof(generic_cpu_name_table) / sizeof(generic_cpu_name_table[0])));
  return generic_cpu_name_table[cpu_arch];
}

/*
 * Helper functions & data used in the merge of two different FP arch.
 */
static const struct fp_config_data {
  int version;
  int regs;
} fp_configs[] = {
      {0, 0},
      {1, 16},
      {2, 16},
      {3, 32},
      {3, 16},
      {4, 32},
      {4, 16},
      {8, 32},
      {8, 16},
};

static const size_t num_fp_configs =
    sizeof(fp_configs) / sizeof(fp_config_data);

// Given h(x, y) = (x * (y >> 4) + (y >> 5))
//
// fp_config_hash_table[ h(0, 0)  =  0 ] = 0
// fp_config_hash_table[ h(1, 16) =  1 ] = 1
// fp_config_hash_table[ h(2, 16) =  2 ] = 2
// fp_config_hash_table[ h(3, 32) =  7 ] = 3
// fp_config_hash_table[ h(3, 16) =  3 ] = 4
// fp_config_hash_table[ h(4, 32) =  9 ] = 5
// fp_config_hash_table[ h(4, 16) =  4 ] = 6
// fp_config_hash_table[ h(8, 32) = 17 ] = 7
// fp_config_hash_table[ h(8, 16) =  8 ] = 8
//
// h(0, 0) = 0
static const uint8_t fp_config_hash_table[] = {
#define UND static_cast<uint8_t>(-1)
    /*  0 */ 0,
    /*  1 */ 1,
    /*  2 */ 2,
    /*  3 */ 4,
    /*  4 */ 6,
    /*  5 */ UND,
    /*  6 */ UND,
    /*  7 */ 3,
    /*  8 */ 8,
    /*  9 */ 5,
    /* 10 */ UND,
    /* 11 */ UND,
    /* 12 */ UND,
    /* 13 */ UND,
    /* 14 */ UND,
    /* 15 */ UND,
    /* 16 */ UND,
    /* 17 */ 7,
#undef UND
};

static int calculate_fp_config_hash(const struct fp_config_data& pConfig) {
  int x = pConfig.version;
  int y = pConfig.regs;
  return (x * (y >> 4) + (y >> 5));
}

static int get_fp_arch_of_config(const struct fp_config_data& pConfig) {
  int hash = calculate_fp_config_hash(pConfig);
  assert(static_cast<size_t>(hash) <
         llvm::array_lengthof(fp_config_hash_table));
  return fp_config_hash_table[hash];
}

static bool is_allowed_use_of_div(int cpu_arch,
                                  int cpu_arch_profile,
                                  int div_use) {
  // 0: The code was permitted to use SDIV and UDIV in the Thumb ISA on v7-R or
  //    v7-M.
  // 1: The code was not permitted to use SDIV and UDIV.
  // 2: The code was explicitly permitted to use SDIV and UDIV.
  switch (div_use) {
    case 0: {
      if ((cpu_arch == ARMELFAttributeData::CPU_Arch_ARM_V7) &&
          ((cpu_arch_profile == 'R') || (cpu_arch_profile == 'M'))) {
        return true;
      } else {
        return (cpu_arch >= ARMELFAttributeData::CPU_Arch_ARM_V7E_M);
      }
    }
    case 1: {
      return false;
    }
    case 2:
    // For future proofing
    default: { return true; }
  }
}

}  // anonymous namespace

//===--------------------------------------------------------------------===//
// End Helper Functions for postMerge()
//===--------------------------------------------------------------------===//

bool ARMELFAttributeData::postMerge(const LinkerConfig& pConfig,
                                    const Input& pInput) {
  // Process Tag_CPU_arch, Tag_CPU_name, Tag_CPU_raw_name, and
  // Tag_also_compatible_with.
  ELFAttributeValue& out_cpu_arch_attr = m_Attrs[Tag_CPU_arch];
  ELFAttributeValue& out_secondary_compatibility_attr =
      m_Attrs[Tag_also_compatible_with];

  if ((m_CurrentCPUArch < 0) && out_cpu_arch_attr.isInitialized()) {
    // Current input initializes the value of Tag_CPU_arch. Validate it.
    int out_cpu_arch = out_cpu_arch_attr.getIntValue();

    if (out_cpu_arch > CPU_Arch_Max) {
      error(diag::error_unknown_cpu_arch) << pInput.name();
      return false;
    }

    // Initialize m_CurrentCPUArch.
    int out_secondary_arch = -1;
    if (out_secondary_compatibility_attr.isInitialized())
      out_secondary_arch = decode_secondary_compatibility_attribute(
          out_secondary_compatibility_attr);

    m_CurrentCPUArch = calculate_cpu_arch(out_cpu_arch, out_secondary_arch);
  }

  if (m_CPUArch >= 0) {
    assert(out_cpu_arch_attr.isInitialized() && "CPU arch has never set!");
    assert(m_CurrentCPUArch >= 0);

    int in_cpu_arch = calculate_cpu_arch(m_CPUArch, m_SecondaryCPUArch);
    int result_cpu_arch = merge_cpu_arch(m_CurrentCPUArch, in_cpu_arch);

    if (result_cpu_arch < 0) {
      warning(diag::warn_mismatch_cpu_arch_profile) << in_cpu_arch
                                                    << pInput.name();
    } else {
      if (result_cpu_arch != m_CurrentCPUArch) {
        // Value of Tag_CPU_arch are going to changea.
        m_CurrentCPUArch = result_cpu_arch;

        // Write the result value to the output.
        if (result_cpu_arch == CPU_Arch_ARM_V4T_Plus_V6_M) {
          out_cpu_arch_attr.setIntValue(CPU_Arch_ARM_V4T);
          encode_secondary_compatibility_attribute(
              out_secondary_compatibility_attr, CPU_Arch_ARM_V6_M);
        } else {
          out_cpu_arch_attr.setIntValue(result_cpu_arch);
          encode_secondary_compatibility_attribute(
              out_secondary_compatibility_attr, -1);
        }

        ELFAttributeValue& out_cpu_name = m_Attrs[Tag_CPU_name];
        ELFAttributeValue& out_cpu_raw_name = m_Attrs[Tag_CPU_raw_name];

        if (m_CurrentCPUArch != in_cpu_arch) {
          // Unable to guess the Tag_CPU_name. Use the generic name.
          if (out_cpu_name.isInitialized()) {
            out_cpu_name.setStringValue(get_generic_cpu_name(m_CurrentCPUArch));
          }

          // Tag_CPU_raw_name becomes unknown. Set to default value to disable
          // it.
          out_cpu_raw_name.setStringValue("");
        } else {
          // Use the value of Tag_CPU_name and Tag_CPU_raw_name from the input.
          if (!m_CPUName.empty()) {
            ELFAttributeValue& out_cpu_name = m_Attrs[Tag_CPU_name];
            assert(out_cpu_name.isInitialized() && "CPU name has never set!");
            out_cpu_name.setStringValue(m_CPUName);
          }

          if (!m_CPURawName.empty()) {
            ELFAttributeValue& out_cpu_raw_name = m_Attrs[Tag_CPU_raw_name];
            assert(out_cpu_raw_name.isInitialized() &&
                   "CPU raw name has never set!");
            out_cpu_raw_name.setStringValue(m_CPURawName);
          }
        }
      }
    }
  }  // (m_CPUArch >= 0)

  // Process Tag_ABI_VFP_args.
  if (m_VFPArgs >= 0) {
    ELFAttributeValue& out_attr = m_Attrs[Tag_ABI_VFP_args];
    ELFAttributeValue& out_float_number_model_attr =
        m_Attrs[Tag_ABI_FP_number_model];

    assert(out_attr.isInitialized() && "VFP args has never set!");

    // If the output is not permitted to use floating number, this attribute
    // is ignored (migrate the value from input directly.)
    if (out_float_number_model_attr.isUninitialized() ||
        (out_float_number_model_attr.getIntValue() == 0)) {
      // Inherit requirement from input.
      out_attr.setIntValue(m_VFPArgs);
    } else {
      if (pConfig.options().warnMismatch())
        warning(diag::warn_mismatch_vfp_args) << pInput.name();
    }
  }

  // Process Tag_FP_arch.
  ELFAttributeValue& out_fp_arch_attr = m_Attrs[Tag_FP_arch];
  if (m_FPArch >= 0) {
    assert(out_fp_arch_attr.isInitialized() && "FP arch has never set!");

    // Tag_FP_arch
    //  0: instructions requiring FP hardware are not permitted
    //  1: VFP1
    //  2: VFP2
    //  3: VFP3 D32
    //  4: VFP3 D16
    //  5: VFP4 D32
    //  6: VFP4 D16
    //  7: ARM v8-A D32
    //  8: ARM v8-A D16
    if (out_fp_arch_attr.getIntValue() == 0) {
      // Output has no constraints on FP hardware. Copy the requirement from
      // input.
      out_fp_arch_attr.setIntValue(m_FPArch);
    } else if (m_FPArch == 0) {
      // Input has no constraints on FP hardware. Do nothing.
    } else {
      // If here, both output and input contain non-zero value of Tag_FP_arch.

      // Version greater than num_fp_configs is not defined. Choose the greater
      // one for future-proofing.
      if (static_cast<unsigned>(m_FPArch) > num_fp_configs) {
        if (static_cast<unsigned>(m_FPArch) > out_fp_arch_attr.getIntValue()) {
          out_fp_arch_attr.setIntValue(m_FPArch);
        }
      } else {
        if (out_fp_arch_attr.getIntValue() < num_fp_configs) {
          const struct fp_config_data& input_fp_config = fp_configs[m_FPArch];

          const struct fp_config_data& output_fp_config =
              fp_configs[out_fp_arch_attr.getIntValue()];

          const struct fp_config_data result_fp_config = {
              /*version*/ ((output_fp_config.version > input_fp_config.version)
                               ? output_fp_config.version
                               : input_fp_config.version),
              /* regs */ ((output_fp_config.regs > input_fp_config.regs)
                              ? output_fp_config.regs
                              : input_fp_config.regs),
          };
          // Find the attribute value corresponding the result_fp_config
          out_fp_arch_attr.setIntValue(get_fp_arch_of_config(result_fp_config));
        }
      }
    }
  }  // (m_FPArch >= 0)

  // Process Tag_ABI_HardFP_use.
  ELFAttributeValue& out_hardfp_use_attr = m_Attrs[Tag_ABI_HardFP_use];

  if (!m_HardFPUseInitialized && out_hardfp_use_attr.isInitialized()) {
    m_HardFPUse = out_hardfp_use_attr.getIntValue();
    m_HardFPUseInitialized = true;
  }

  if (m_HardFPUse >= 0) {
    // Tag_ABI_HardFP_use depends on the meaning of Tag_FP_arch when it's 0.
    assert(out_hardfp_use_attr.isInitialized() && "HardFP use has never set!");

    if (out_fp_arch_attr.isUninitialized() ||
        (out_fp_arch_attr.getIntValue() == 0)) {
      // Has no constraints on FP hardware.
      out_hardfp_use_attr.setIntValue(m_HardFPUse);
    } else {
      // Both output and input contain non-zero value of Tag_FP_arch and we have
      // different Tag_ABI_HaedFP_Use settings other than 0.
      if ((out_fp_arch_attr.getIntValue() > 0) && (m_HardFPUse > 0))
        // Promote to 3 (The user permitted this entity to use both SP and DP
        // VFP instruction.)
        out_hardfp_use_attr.setIntValue(3);
    }
  }

  // Move the value of Tag_MPextension_use_legacy to Tag_MPextension_use.
  ELFAttributeValue& out_mpextension_use_legacy =
      m_Attrs[Tag_MPextension_use_legacy];

  ELFAttributeValue& out_mpextension_use = m_Attrs[Tag_MPextension_use];

  // If Tag_MPextension_use_legacy has value, it must be introduced by current
  // input since it is reset every time after the merge completed.
  if (out_mpextension_use_legacy.isInitialized()) {
    if (out_mpextension_use.isInitialized()) {
      if (m_MPextensionUse < 0) {
        // The value of Tag_MPextension_use is introduced by the current input.
        // Check whether it is consistent with the one set in legacy.
        m_MPextensionUse = out_mpextension_use.getIntValue();
      } else {
        // Current input introduces value of Tag_MPextension_use in
        // m_MPextensionUse.
      }

      // Check the consistency between m_MPextensionUse and the value of
      // Tag_MPextension_use_legacy.
      if (static_cast<unsigned>(m_MPextensionUse) !=
          out_mpextension_use_legacy.getIntValue()) {
        error(diag::error_mismatch_mpextension_use) << pInput.name();
        return false;
      }
    } else {
      if (m_MPextensionUse < 0) {
        // Tag_MPextension_use is not set. Initialize it and move the value.
        out_mpextension_use.setType(ELFAttributeValue::Int);
        out_mpextension_use.setIntValue(out_mpextension_use.getIntValue());
      } else {
        // Unreachable case since the value to unitialized attribute is directly
        // assigned in ELFAttribute::Subsection::merge().
        assert(false && "Tag_MPextension_use is uninitialized but have value?");
      }
    }

    // Reset the attribute to uninitialized so it won't be included in the
    // output.
    out_mpextension_use_legacy.setType(ELFAttributeValue::Uninitialized);
  }

  // Process Tag_MPextension_use.
  if (m_MPextensionUse > 0) {
    assert(out_mpextension_use.isInitialized());

    if (static_cast<unsigned>(m_MPextensionUse) >
        out_mpextension_use.getIntValue()) {
      out_mpextension_use.setIntValue(m_MPextensionUse);
    }
  }

  // Process Tag_DIV_use.
  ELFAttributeValue& out_div_use_attr = m_Attrs[Tag_DIV_use];

  if (!m_DIVUseInitialized && out_div_use_attr.isInitialized()) {
    // Perform the merge by reverting value of Tag_DIV_use and setup m_DIVUse.
    m_DIVUse = out_div_use_attr.getIntValue();
    out_div_use_attr.setIntValue(0);
    m_DIVUseInitialized = true;
  }

  if (m_DIVUse >= 0) {
    assert(out_div_use_attr.isInitialized());

    const ELFAttributeValue& out_cpu_arch_profile_attr =
        m_Attrs[Tag_CPU_arch_profile];

    int out_cpu_arch_profile = Arch_Profile_None;
    if (out_cpu_arch_profile_attr.isInitialized()) {
      out_cpu_arch_profile = out_cpu_arch_profile_attr.getIntValue();
    }

    if (m_DIVUse == 1) {
      // Input (=1) was not permitted to use SDIV and UDIV. See whether current
      // output was explicitly permitted the use.
      if (!is_allowed_use_of_div(m_CurrentCPUArch,
                                 out_cpu_arch_profile,
                                 out_div_use_attr.getIntValue())) {
        out_div_use_attr.setIntValue(1);
      }
    } else {
      if (out_div_use_attr.getIntValue() != 1) {
        // Output does not explicitly forbid the use of SDIV/UDIV. See whether
        // the input attribute can allow it under current CPU architecture
        // profile.
        if (is_allowed_use_of_div(
                m_CurrentCPUArch, out_cpu_arch_profile, m_DIVUse)) {
          out_div_use_attr.setIntValue(m_DIVUse);
        }
      }
    }
  }

  return true;
}

size_t ARMELFAttributeData::sizeOutput() const {
  size_t result = 0;

  // Size contributed by known attributes
  for (unsigned i = 0; i <= Tag_Max; ++i) {
    TagType tag = static_cast<TagType>(i);
    const ELFAttributeValue& value = m_Attrs[tag];

    if (value.shouldEmit()) {
      result += leb128::size(static_cast<uint32_t>(tag));
      result += value.getSize();
    }
  }

  // Size contributed by unknown attributes
  for (UnknownAttrsMap::const_iterator unknown_attr_it = m_UnknownAttrs.begin(),
                                       unknown_attr_end = m_UnknownAttrs.end();
       unknown_attr_it != unknown_attr_end;
       ++unknown_attr_it) {
    TagType tag = unknown_attr_it->first;
    const ELFAttributeValue& value = unknown_attr_it->second;

    if (value.shouldEmit()) {
      result += leb128::size(static_cast<uint32_t>(tag));
      result += value.getSize();
    }
  }

  return result;
}

size_t ARMELFAttributeData::emit(char* pBuf) const {
  char* buffer = pBuf;

  // Tag_conformance "should be emitted first in a file-scope sub-subsection of
  // the first public subsection of the attribute section."
  //
  // See ARM [ABI-addenda], 2.3.7.4 Conformance tag
  const ELFAttributeValue& attr_conformance = m_Attrs[Tag_conformance];

  if (attr_conformance.shouldEmit()) {
    if (!ELFAttributeData::WriteAttribute(
            Tag_conformance, attr_conformance, buffer)) {
      return 0;
    }
  }

  // Tag_nodefaults "should be emitted before any other tag in an attribute
  // subsection other that the conformance tag"
  //
  // See ARM [ABI-addenda], 2.3.7.5 No defaults tag
  const ELFAttributeValue& attr_nodefaults = m_Attrs[Tag_nodefaults];

  if (attr_nodefaults.shouldEmit()) {
    if (!ELFAttributeData::WriteAttribute(
            Tag_nodefaults, attr_nodefaults, buffer)) {
      return 0;
    }
  }

  // Tag_conformance (=67)
  // Tag_nodefaults (=64)
  for (unsigned i = 0; i < Tag_nodefaults; ++i) {
    TagType tag = static_cast<TagType>(i);
    const ELFAttributeValue& value = m_Attrs[tag];

    if (value.shouldEmit() &&
        !ELFAttributeData::WriteAttribute(tag, value, buffer)) {
      return 0;
    }
  }

  for (unsigned i = (Tag_nodefaults + 1); i <= Tag_Max; ++i) {
    TagType tag = static_cast<TagType>(i);
    const ELFAttributeValue& value = m_Attrs[tag];

    if (value.shouldEmit() && (i != Tag_conformance) &&
        !ELFAttributeData::WriteAttribute(tag, value, buffer)) {
      return 0;
    }
  }

  for (UnknownAttrsMap::const_iterator unknown_attr_it = m_UnknownAttrs.begin(),
                                       unknown_attr_end = m_UnknownAttrs.end();
       unknown_attr_it != unknown_attr_end;
       ++unknown_attr_it) {
    TagType tag = unknown_attr_it->first;
    const ELFAttributeValue& value = unknown_attr_it->second;

    if (value.shouldEmit() &&
        !ELFAttributeData::WriteAttribute(tag, value, buffer)) {
      return 0;
    }
  }

  return (buffer - pBuf);
}

bool ARMELFAttributeData::usingThumb() const {
  int arch = m_Attrs[Tag_CPU_arch].getIntValue();
  if ((arch == CPU_Arch_ARM_V6_M) || (arch == CPU_Arch_ARM_V6S_M))
    return true;
  if ((arch != CPU_Arch_ARM_V7) && (arch != CPU_Arch_ARM_V7E_M))
    return false;

  arch = m_Attrs[Tag_CPU_arch_profile].getIntValue();
  return arch == Arch_Profile_Microcontroller;
}

bool ARMELFAttributeData::usingThumb2() const {
  int arch = m_Attrs[Tag_CPU_arch].getIntValue();
  return (arch == CPU_Arch_ARM_V6T2) || (arch == CPU_Arch_ARM_V7);
}

}  // namespace mcld
