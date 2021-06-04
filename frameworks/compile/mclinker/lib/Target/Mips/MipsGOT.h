//===- MipsGOT.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSGOT_H_
#define TARGET_MIPS_MIPSGOT_H_
#include "mcld/ADT/SizeTraits.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/Support/MemoryRegion.h"
#include "mcld/Target/GOT.h"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>

#include <map>
#include <set>
#include <vector>

namespace mcld {

class Input;
class LDSection;
class LDSymbol;
class OutputRelocSection;

/** \class MipsGOT
 *  \brief Mips Global Offset Table.
 */
class MipsGOT : public GOT {
 public:
  explicit MipsGOT(LDSection& pSection);

  /// Assign value to the GOT entry.
  virtual void setEntryValue(Fragment* entry, uint64_t pValue) = 0;

  /// Emit the global offset table.
  virtual uint64_t emit(MemoryRegion& pRegion) = 0;

  /// Address of _gp_disp symbol.
  uint64_t getGPDispAddress() const;

  void initializeScan(const Input& pInput);
  void finalizeScan(const Input& pInput);

  bool reserveLocalEntry(ResolveInfo& pInfo,
                         int reloc,
                         Relocation::DWord pAddend);
  bool reserveGlobalEntry(ResolveInfo& pInfo);
  bool reserveTLSGdEntry(ResolveInfo& pInfo);
  bool reserveTLSGotEntry(ResolveInfo& pInfo);
  bool reserveTLSLdmEntry();

  size_t getLocalNum() const;   ///< number of local symbols in primary GOT
  size_t getGlobalNum() const;  ///< total number of global symbols

  bool isPrimaryGOTConsumed();

  Fragment* consumeLocal();
  Fragment* consumeGlobal();
  Fragment* consumeTLS(Relocation::Type pType);

  uint64_t getGPAddr(const Input& pInput) const;
  uint64_t getGPRelOffset(const Input& pInput, const Fragment& pEntry) const;

  void recordGlobalEntry(const ResolveInfo* pInfo, Fragment* pEntry);
  Fragment* lookupGlobalEntry(const ResolveInfo* pInfo);

  void recordTLSEntry(const ResolveInfo* pInfo, Fragment* pEntry,
                      Relocation::Type pType);
  Fragment* lookupTLSEntry(const ResolveInfo* pInfo, Relocation::Type pType);

  void recordLocalEntry(const ResolveInfo* pInfo,
                        Relocation::DWord pAddend,
                        Fragment* pEntry);
  Fragment* lookupLocalEntry(const ResolveInfo* pInfo,
                             Relocation::DWord pAddend);

  /// hasGOT1 - return if this got section has any GOT1 entry
  bool hasGOT1() const;

  bool hasMultipleGOT() const;

  /// Create GOT entries and reserve dynrel entries.
  void finalizeScanning(OutputRelocSection& pRelDyn);

  /// Compare two symbols to define order in the .dynsym.
  bool dynSymOrderCompare(const LDSymbol* pX, const LDSymbol* pY) const;

 protected:
  /// Create GOT entry.
  virtual Fragment* createEntry(uint64_t pValue, SectionData* pParent) = 0;

  /// Size of GOT entry.
  virtual size_t getEntrySize() const = 0;

  /// Reserve GOT header entries.
  virtual void reserveHeader() = 0;

 private:
  /** \class GOTMultipart
   *  \brief GOTMultipart counts local and global entries in the GOT.
   */
  struct GOTMultipart {
    explicit GOTMultipart(size_t local = 0, size_t global = 0);

    typedef llvm::DenseSet<const Input*> InputSetType;

    size_t m_LocalNum;   ///< number of reserved local entries
    size_t m_GlobalNum;  ///< number of reserved global entries
    size_t m_TLSNum;     ///< number of reserved TLS entries
    size_t m_TLSDynNum;  ///< number of reserved TLS related dynamic relocations

    size_t m_ConsumedLocal;   ///< consumed local entries
    size_t m_ConsumedGlobal;  ///< consumed global entries
    size_t m_ConsumedTLS;     ///< consumed TLS entries

    Fragment* m_pLastLocal;   ///< the last consumed local entry
    Fragment* m_pLastGlobal;  ///< the last consumed global entry
    Fragment* m_pLastTLS;     ///< the last consumed TLS entry

    InputSetType m_Inputs;

    bool isConsumed() const;

    void consumeLocal();
    void consumeGlobal();
    void consumeTLS(Relocation::Type pType);
  };

  /** \class LocalEntry
   *  \brief LocalEntry local GOT entry descriptor.
   */
  struct LocalEntry {
    const ResolveInfo* m_pInfo;
    Relocation::DWord m_Addend;
    bool m_IsGot16;

    LocalEntry(const ResolveInfo* pInfo,
               Relocation::DWord addend,
               bool isGot16);

    bool operator<(const LocalEntry& O) const;
  };

  typedef std::vector<GOTMultipart> MultipartListType;

  // Set of global symbols.
  typedef llvm::DenseSet<const ResolveInfo*> SymbolSetType;
  // Map of symbols. If value is true, the symbol is referenced
  // in the current input only. If value is false, the symbol
  // is referenced in the other modules merged to the current GOT.
  typedef llvm::DenseMap<const ResolveInfo*, bool> SymbolUniqueMapType;

  // Set of local symbols.
  typedef std::set<LocalEntry> LocalSymbolSetType;

  MultipartListType m_MultipartList;  ///< list of GOT's descriptors
  const Input* m_pInput;              ///< current input

  // Global symbols merged to the current GOT
  // except symbols from the current input.
  SymbolSetType m_MergedGlobalSymbols;
  // Global symbols from the current input.
  SymbolUniqueMapType m_InputGlobalSymbols;
  // Set of symbols referenced by TLS GD relocations.
  SymbolSetType m_InputTLSGdSymbols;
  // Set of symbols referenced by TLS GOTTPREL relocation.
  SymbolSetType m_InputTLSGotSymbols;
  // There is a symbol referenced by TLS LDM relocations.
  bool m_HasTLSLdmSymbol;
  // Local symbols merged to the current GOT
  // except symbols from the current input.
  LocalSymbolSetType m_MergedLocalSymbols;
  // Local symbols from the current input.
  LocalSymbolSetType m_InputLocalSymbols;

  size_t m_CurrentGOTPart;

  typedef llvm::DenseMap<const LDSymbol*, unsigned> SymbolOrderMapType;
  SymbolOrderMapType m_SymbolOrderMap;

  void initGOTList();

  void changeInput();
  bool isGOTFull() const;
  void split();
  void reserve(size_t pNum);

 private:
  struct GotEntryKey {
    size_t m_GOTPage;
    const ResolveInfo* m_pInfo;
    Relocation::DWord m_Addend;

    bool operator<(const GotEntryKey& key) const {
      if (m_GOTPage != key.m_GOTPage)
        return m_GOTPage < key.m_GOTPage;

      if (m_pInfo != key.m_pInfo)
        return m_pInfo < key.m_pInfo;

      return m_Addend < key.m_Addend;
    }
  };

  typedef std::map<GotEntryKey, Fragment*> GotEntryMapType;
  GotEntryMapType m_GotLocalEntriesMap;
  GotEntryMapType m_GotGlobalEntriesMap;
  GotEntryMapType m_GotTLSGdEntriesMap;
  GotEntryMapType m_GotTLSGotEntriesMap;
  Fragment* m_GotTLSLdmEntry;
};

/** \class Mips32GOT
 *  \brief Mips 32-bit Global Offset Table.
 */
class Mips32GOT : public MipsGOT {
 public:
  explicit Mips32GOT(LDSection& pSection);

 private:
  typedef GOT::Entry<4> Mips32GOTEntry;

  // MipsGOT
  virtual void setEntryValue(Fragment* entry, uint64_t pValue);
  virtual uint64_t emit(MemoryRegion& pRegion);
  virtual Fragment* createEntry(uint64_t pValue, SectionData* pParent);
  virtual size_t getEntrySize() const;
  virtual void reserveHeader();
};

/** \class Mips64GOT
 *  \brief Mips 64-bit Global Offset Table.
 */
class Mips64GOT : public MipsGOT {
 public:
  explicit Mips64GOT(LDSection& pSection);

 private:
  typedef GOT::Entry<8> Mips64GOTEntry;

  // MipsGOT
  virtual void setEntryValue(Fragment* entry, uint64_t pValue);
  virtual uint64_t emit(MemoryRegion& pRegion);
  virtual Fragment* createEntry(uint64_t pValue, SectionData* pParent);
  virtual size_t getEntrySize() const;
  virtual void reserveHeader();
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSGOT_H_
