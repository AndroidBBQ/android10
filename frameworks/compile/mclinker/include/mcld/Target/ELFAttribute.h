//===- ELFAttribute.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_ELFATTRIBUTE_H_
#define MCLD_TARGET_ELFATTRIBUTE_H_

#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/ELFAttributeData.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>

namespace mcld {

class ELFAttributeData;
class GNULDBackend;
class Input;
class LDSection;
class LinkerConfig;

/** \class ELFAttribute
 *  \brief ELFAttribute is the attribute section in an ELF file.
 */
class ELFAttribute {
 public:
  // ARM [ABI-addenda], 2.2.3.
  static const char FormatVersion = 'A';
  static const size_t FormatVersionFieldSize = sizeof(FormatVersion);  // a byte
  static const size_t SubsectionLengthFieldSize = 4;  // a 4-byte integer

  // MinimalELFAttributeSubsectionSize is the minimal number of bytes a valid
  // subsection in ELF attribute section should have.
  static const size_t MinimalELFAttributeSubsectionSize =
      1 /* Tag_File, see ARM [ABI-addenda], 2.2.4 */ +
      4 /* byte-size, see ARM [ABI-addenda], 2.2.4 */;

  // MinimalELFAttributeSectionSize is the minimal number of bytes a valid ELF
  // attribute section should have.
  static const size_t MinimalELFAttributeSectionSize =
      FormatVersionFieldSize + SubsectionLengthFieldSize +
      2 /* vendor-name, a char plus '\0', see ARM [ABI-addenda], 2.2.3 */ +
      1 * MinimalELFAttributeSubsectionSize;

 public:
  ELFAttribute(const GNULDBackend& pBackend, const LinkerConfig& pConfig)
      : m_Backend(pBackend), m_Config(pConfig) {}

  ~ELFAttribute();

 public:
  /// merge - merge attributes from input (attribute) section
  bool merge(const Input& pInput, LDSection& pInputAttrSectHdr);

  /// sizeOutput - calculate the number of bytes required to encode this
  /// attribute data section
  size_t sizeOutput() const;

  /// emit - encode and write out this attribute section
  size_t emit(MemoryRegion& pRegion) const;

  inline const GNULDBackend& backend() const { return m_Backend; }

  inline const LinkerConfig& config() const { return m_Config; }

  // Place vendor's attribute data under the management.
  void registerAttributeData(ELFAttributeData& pAttrData);

 private:
  /** \class Subsection
   *  \brief A helper class to wrap ELFAttributeData and to provide general
   *  interfaces for ELFAttribute to operate on
   */
  class Subsection {
   public:
    Subsection(ELFAttribute& pParent, ELFAttributeData& pAttrData)
        : m_Parent(pParent), m_AttrData(pAttrData) {}

   public:
    bool isMyAttribute(llvm::StringRef pVendorName) const {
      return (m_AttrData.getVendorName() == pVendorName);
    }

    /// merge -  Merge the attributes from the section in the input data.
    bool merge(const Input& pInput, ConstAddress pData, size_t pSize);

    /// sizeOutput - calculate the number of bytes required to encode this
    /// subsection
    size_t sizeOutput() const;

    /// emit - write out this attribute subsection to the buffer.
    size_t emit(char* pBuf) const;

   private:
    // The attribute section this subsection belongs to
    ELFAttribute& m_Parent;

    // The attribute data containing in this subsection
    ELFAttributeData& m_AttrData;
  };

  // Obtain the corresponding subsection of the specified vendor
  Subsection* getSubsection(llvm::StringRef pVendorName) const;

 private:
  const GNULDBackend& m_Backend;

  const LinkerConfig& m_Config;

  // There is at most two subsections ("aeabi" and "gnu") in most cases.
  llvm::SmallVector<Subsection*, 2> m_Subsections;
};

}  // namespace mcld

#endif  // MCLD_TARGET_ELFATTRIBUTE_H_
