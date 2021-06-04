//===- ELFAttribute.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Target/ELFAttribute.h"

#include "mcld/ADT/SizeTraits.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/SectionData.h"
#include "mcld/LinkerConfig.h"
#include "mcld/MC/Input.h"
#include "mcld/Support/LEB128.h"
#include "mcld/Support/MemoryArea.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/ELFAttributeValue.h"
#include "mcld/Target/GNULDBackend.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Host.h>

#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// ELFAttribute
//===----------------------------------------------------------------------===//
ELFAttribute::~ELFAttribute() {
  llvm::DeleteContainerPointers(m_Subsections);
  return;
}

bool ELFAttribute::merge(const Input& pInput, LDSection& pInputAttrSectHdr) {
  // Skip corrupt subsection
  if (pInputAttrSectHdr.size() < MinimalELFAttributeSectionSize)
    return true;

  // Obtain the region containing the attribute data. Expect exactly one
  // RegionFragment in the section data.
  const SectionData* sect_data = pInputAttrSectHdr.getSectionData();

  // FIXME: Why is 2?
  if ((sect_data->size() != 2) ||
      (!llvm::isa<RegionFragment>(sect_data->front()))) {
    return true;
  }

  const RegionFragment& region_frag =
      llvm::cast<RegionFragment>(sect_data->front());

  llvm::StringRef region = region_frag.getRegion();

  // Parse the ELF attribute section header. ARM [ABI-addenda], 2.2.3.
  //
  // <format-version: ‘A’>
  // [ <uint32: subsection-length> NTBS: vendor-name
  //   <bytes: vendor-data>
  // ]*
  const char* attribute_data = region.begin();

  // format-version
  if (attribute_data[0] != FormatVersion) {
    warning(diag::warn_unsupported_attribute_section_format)
        << pInput.name() << attribute_data[0];
    return true;
  }

  size_t subsection_offset = FormatVersionFieldSize;

  // Iterate all subsections containing in this attribute section.
  do {
    const char* subsection_data = region.begin() + subsection_offset;

    // subsection-length
    uint32_t subsection_length =
        *reinterpret_cast<const uint32_t*>(subsection_data);

    if (llvm::sys::IsLittleEndianHost != m_Config.targets().isLittleEndian())
      bswap32(subsection_length);

    // vendor-name
    const char* vendor_name = subsection_data + SubsectionLengthFieldSize;
    const size_t vendor_name_length = ::strlen(vendor_name) + 1 /* '\0' */;

    // Check the length.
    if ((vendor_name_length <= 1) ||
        (subsection_length <= (SubsectionLengthFieldSize + vendor_name_length)))
      return true;

    // Select the attribute subsection.
    Subsection* subsection = getSubsection(vendor_name);

    // Only process the subsections whose vendor can be recognized.
    if (subsection == NULL) {
      warning(diag::warn_unrecognized_vendor_subsection) << vendor_name
                                                         << pInput.name();
    } else {
      // vendor-data
      size_t vendor_data_offset =
          subsection_offset + SubsectionLengthFieldSize + vendor_name_length;
      size_t vendor_data_size =
          subsection_length - SubsectionLengthFieldSize - vendor_name_length;

      ConstAddress vendor_data =
          reinterpret_cast<ConstAddress>(region.begin()) + vendor_data_offset;

      // Merge the vendor data in the subsection.
      if (!subsection->merge(pInput, vendor_data, vendor_data_size))
        return false;
    }

    subsection_offset += subsection_length;
  } while ((subsection_offset + SubsectionLengthFieldSize) <
           pInputAttrSectHdr.size());

  return true;
}

size_t ELFAttribute::sizeOutput() const {
  size_t total_size = FormatVersionFieldSize;

  for (llvm::SmallVectorImpl<Subsection*>::const_iterator
           subsec_it = m_Subsections.begin(),
           subsec_end = m_Subsections.end();
       subsec_it != subsec_end;
       ++subsec_it) {
    total_size += (*subsec_it)->sizeOutput();
  }
  return total_size;
}

size_t ELFAttribute::emit(MemoryRegion& pRegion) const {
  // ARM [ABI-addenda], 2.2.3
  uint64_t total_size = 0;

  // Write format-version.
  char* buffer = reinterpret_cast<char*>(pRegion.begin());
  buffer[0] = FormatVersion;
  total_size += FormatVersionFieldSize;

  for (llvm::SmallVectorImpl<Subsection*>::const_iterator
           subsec_it = m_Subsections.begin(),
           subsec_end = m_Subsections.end();
       subsec_it != subsec_end;
       ++subsec_it) {
    // Write out subsection.
    total_size += (*subsec_it)->emit(buffer + total_size);
  }

  return total_size;
}

void ELFAttribute::registerAttributeData(ELFAttributeData& pAttrData) {
  assert((getSubsection(pAttrData.getVendorName()) == NULL) &&
         "Multiple attribute data for a vendor!");
  m_Subsections.push_back(new Subsection(*this, pAttrData));
  return;
}

ELFAttribute::Subsection* ELFAttribute::getSubsection(
    llvm::StringRef pVendorName) const {
  // Search m_Subsections linearly.
  for (llvm::SmallVectorImpl<Subsection*>::const_iterator
           subsec_it = m_Subsections.begin(),
           subsec_end = m_Subsections.end();
       subsec_it != subsec_end;
       ++subsec_it) {
    Subsection* const subsection = *subsec_it;
    if (subsection->isMyAttribute(pVendorName)) {
      return subsection;
    }
  }

  // Not found
  return NULL;
}

//===----------------------------------------------------------------------===//
// ELFAttribute::Subsection
//===----------------------------------------------------------------------===//
bool ELFAttribute::Subsection::merge(const Input& pInput,
                                     ConstAddress pData,
                                     size_t pSize) {
  const bool need_swap = (llvm::sys::IsLittleEndianHost !=
                          m_Parent.config().targets().isLittleEndian());
  // Read attribute sub-subsection from vendor data.
  //
  // ARM [ABI-addenda], 2.2.4:
  //
  // [   Tag_File    (=1) <uint32: byte-size> <attribute>*
  //   | Tag_Section (=2) <uint32: byte-size> <section number>* 0 <attribute>*
  //   | Tag_symbol  (=3) <unit32: byte-size> <symbol number>* 0 <attribute>*
  // ] +
  const char* subsubsection_data = reinterpret_cast<const char*>(pData);
  size_t remaining_size = pSize;

  if (!m_AttrData.preMerge(pInput)) {
    return false;
  }

  while (remaining_size > ELFAttribute::MinimalELFAttributeSubsectionSize) {
    // The tag of sub-subsection is encoded in ULEB128.
    size_t tag_size;
    uint64_t tag = leb128::decode<uint64_t>(subsubsection_data, tag_size);

    if ((tag_size + 4 /* byte-size */) >= remaining_size)
      break;

    size_t subsubsection_length =
        *reinterpret_cast<const uint32_t*>(subsubsection_data + tag_size);

    if (need_swap)
      bswap32(subsubsection_length);

    if (subsubsection_length > remaining_size) {
      // The subsubsection is corrupted. Try our best to process it.
      subsubsection_length = remaining_size;
    }

    switch (tag) {
      case ELFAttributeData::Tag_File: {
        ELFAttributeData::TagType tag;
        ELFAttributeValue in_attr;
        // The offset from the start of sub-subsection that <attribute> located
        size_t attribute_offset = tag_size + 4 /* byte-size */;

        const char* attr_buf = subsubsection_data + attribute_offset;
        size_t attr_size = subsubsection_length - attribute_offset;

        // Read attributes from the stream.
        do {
          if (!ELFAttributeData::ReadTag(tag, attr_buf, attr_size))
            break;

          ELFAttributeValue* out_attr;
          bool is_newly_created;

          std::tie(out_attr, is_newly_created) =
              m_AttrData.getOrCreateAttributeValue(tag);

          assert(out_attr != NULL);

          if (is_newly_created) {
            // Directly read the attribute value to the out_attr.
            if (!ELFAttributeData::ReadValue(*out_attr, attr_buf, attr_size))
              break;
          } else {
            // The attribute has been defined previously. Read the attribute
            // to a temporary storage in_attr and perform the merge.
            in_attr.reset();
            in_attr.setType(out_attr->type());

            // Read the attribute value.
            if (!ELFAttributeData::ReadValue(in_attr, attr_buf, attr_size))
              break;

            // Merge if the read attribute value is different than current one
            // in output.
            if ((in_attr != *out_attr) &&
                !m_AttrData.merge(m_Parent.config(), pInput, tag, in_attr)) {
              // Fail to merge the attribute.
              return false;
            }
          }
        } while (attr_size > 0);

        break;
      }
      // Skip sub-subsection tagged with Tag_Section and Tag_Symbol. They are
      // deprecated since ARM [ABI-addenda] r2.09.
      case ELFAttributeData::Tag_Section:
      case ELFAttributeData::Tag_Symbol:
      // Skip any unknown tags.
      default: { break; }
    }

    // Update subsubsection_data and remaining_size for next.
    subsubsection_data += subsubsection_length;
    remaining_size -= subsubsection_length;
  }  // while (remaining_size > ELFAttribute::MinimalELFAttributeSubsectionSize)

  return m_AttrData.postMerge(m_Parent.config(), pInput);
}

size_t ELFAttribute::Subsection::sizeOutput() const {
  // ARM [ABI-addenda], 2.2.3 and 2.2.4
  return ELFAttribute::SubsectionLengthFieldSize +
         m_AttrData.getVendorName().length() /* vendor-name */ +
         1 /* NULL-terminator for vendor-name */ + 1 /* Tag_File */ +
         sizeof(uint32_t) /* length of sub-subsection */ +
         m_AttrData.sizeOutput();
}

size_t ELFAttribute::Subsection::emit(char* pBuf) const {
  // ARM [ABI-addenda], 2.2.3 and 2.2.4
  const bool need_swap = (llvm::sys::IsLittleEndianHost !=
                          m_Parent.config().targets().isLittleEndian());

  char* buffer = pBuf;

  // The subsection-length and byte-size field in sub-subsection will be patched
  // later after writing out all attribute data.
  char* subsection_length_hole = NULL;
  char* subsubsection_length_hole = NULL;

  // Reserve space for subsection-length.
  subsection_length_hole = buffer;
  buffer += 4;

  // Write vendor-name.
  const std::string& vendor_name = m_AttrData.getVendorName();
  ::memcpy(buffer, vendor_name.c_str(), vendor_name.length());
  buffer += vendor_name.length();

  // Write NULL-terminator for vendor-name.
  *buffer++ = '\0';

  // Write Tag_File (0x01).
  *buffer++ = '\x01';

  // Reserve space for byte-size for sub-subsection.
  subsubsection_length_hole = buffer;
  buffer += sizeof(uint32_t);

  // Write attribute data.
  uint32_t subsubsection_length = m_AttrData.emit(buffer);

  // Calculate value of subsection-length.
  uint32_t subsection_length = (buffer - pBuf) + subsubsection_length;

  // ARM [ABI-addenda] 2.2.4
  //
  // The byte-size in sub-subsection includes Tag_File (1-byte) and the size
  // field of itself (4-byte).
  subsubsection_length += 1 /* Tag_File */ + 4 /* size of byte-size */;

  // Patch subsubsection_length_hole.
  assert(subsubsection_length_hole != NULL);

  if (need_swap)
    bswap32(subsubsection_length);

  ::memcpy(subsubsection_length_hole, &subsubsection_length, sizeof(uint32_t));

  // Write subsection-length in subsection_length_hole.
  if (need_swap)
    bswap32(subsection_length);

  assert(subsection_length_hole != NULL);
  ::memcpy(subsection_length_hole, &subsection_length, sizeof(uint32_t));

  return subsection_length;
}

}  // namespace mcld
