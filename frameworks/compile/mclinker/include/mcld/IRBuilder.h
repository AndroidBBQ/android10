//===- IRBuilder.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// IRBuilder is a class used as a convenient way to create MCLinker sections
// with a consistent and simplified interface.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_IRBUILDER_H_
#define MCLD_IRBUILDER_H_

#include "mcld/Fragment/FillFragment.h"
#include "mcld/Fragment/Fragment.h"
#include "mcld/Fragment/FragmentRef.h"
#include "mcld/Fragment/RegionFragment.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/MC/Input.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/Support/FileHandle.h"
#include "mcld/Support/Path.h"

namespace mcld {

class InputTree;
class LinkerConfig;
class Module;

/** \class IRBuilder
 *  \brief IRBuilder provides an uniform API for creating sections and
 *  inserting them into a input file.
 *
 *  Ahead-of-time virtual machines (VM) usually compiles an intermediate
 *  language into a system-dependent binary.  IRBuilder helps such kind of VMs
 *  to emit binaries in native object format, such as ELF or MachO.
 */
class IRBuilder {
 public:
  enum ObjectFormat { ELF, MachO, COFF };

  enum SymbolDefinePolicy { Force, AsReferred };

  enum SymbolResolvePolicy { Unresolve, Resolve };

 public:
  IRBuilder(Module& pModule, const LinkerConfig& pConfig);

  ~IRBuilder();

  const InputBuilder& getInputBuilder() const { return m_InputBuilder; }
  InputBuilder& getInputBuilder() { return m_InputBuilder; }
  const Module& getModule() const { return m_Module; }
  Module& getModule() { return m_Module; }

  /// @}
  /// @name Input Files On The Command Line
  /// @{

  /// CreateInput - To create an input file and append it to the input tree.
  /// This function is like to add an input file in the command line.
  ///
  /// There are four types of the input files:
  ///   - relocatable objects,
  ///   - shared objects,
  ///   - archives,
  ///   - and user-defined objects.
  ///
  /// If Input::Unknown type is given, MCLinker will automatically
  /// open and read the input file, and create sections of the input. Otherwise,
  /// users need to manually create sections by IRBuilder.
  ///
  /// @see mcld::Input
  ///
  /// @param pName [in] The name of the input file.
  /// @param pPath [in] The path of the input file.
  /// @param pType [in] The type of the input file. MCLinker will parse the
  ///                   input file to create sections only if pType is
  ///                   Input::Unknown.
  /// @return the created mcld::Input.
  Input* CreateInput(const std::string& pName,
                     const sys::fs::Path& pPath,
                     Input::Type pType);

  /// ReadInput - To read an input file and append it to the input tree.
  /// This function is like to add an input file in the command line.
  ///
  /// This funciton is equal to call
  ///   @ref IRBuilder::CreateInput(pName, pPath, Input::Unknown);
  ///
  /// MCLinker will automatically open and read the input file, and create
  /// sections of the input.
  ///
  /// @see mcld::Input
  ///
  /// @param pName [in] The name of the input file.
  /// @param pPath [in] The path of the input file.
  /// @return the created mcld::Input.
  Input* ReadInput(const std::string& pName, const sys::fs::Path& pPath);

  /// ReadInput - To read an input file and append it to the input tree.
  ///
  /// This function is equal to -l option. This function tells MCLinker to
  /// search for lib[pNameSpec].so or lib[pNameSpec].a in the search path.
  ///
  /// @param pNameSpec [in] The namespec of the input file.
  /// @return the created mcld::Input.
  Input* ReadInput(const std::string& pNameSpec);

  /// ReadInput - To read an input file and append it to the input tree.
  /// Another way to open file manually. Use MCLinker's mcld::FileHandle.
  Input* ReadInput(FileHandle& pFileHandle);

  /// ReadInput - To read an input file and append it to the input tree.
  ///
  /// This function is like to add an input in the command line.
  ///
  /// This function tells MCLinker to read pRawMemory as an image of an object
  /// file. So far, MCLinekr only supports ELF object format, but it will
  /// support various object formats in the future. MCLinker relies triple to
  /// know the object format of pRawMemory.
  /// @param [in] pName      The name of the input file
  /// @param [in] pRawMemory An image of object file
  /// @param [in] pSize      The size of the memory
  /// @return The created mcld::Input
  Input* ReadInput(const std::string& pName, void* pRawMemory, size_t pSize);

  /// StartGroup - Add an opening tag of group.
  ///
  /// This function is equal to --start-group option. This function tells
  /// MCLinker to create a new archive group and to add the following archives
  /// in the created group. The archives in a group are searched repeatedly
  /// until no new undefined references are created.
  bool StartGroup();

  /// EndGroup - Add a closing tag of group.
  ///
  /// This function is equal to --end-group option. This function tells
  /// MCLinker to stop adding following archives in the created group.
  bool EndGroup();

  /// @}
  /// @name Positional Options On The Command Line
  /// @{

  /// WholeArchive - Append a --whole-archive option on the command line
  ///
  /// This function is equal to --whole-archive option. This function tells
  /// MCLinker to include every object files in the following archives.
  void WholeArchive();

  /// NoWholeArchive - Append a --no-whole-archive option on the command line.
  ///
  /// This function is equal to --no-whole-archive option. This function tells
  /// MCLinker to stop including every object files in the following archives.
  /// Only used object files in the following archives are included.
  void NoWholeArchive();

  /// AsNeeded - Append a --as-needed option on the command line.
  ///
  /// This function is equal to --as-needed option. This function tells
  /// MCLinker to not add a DT_NEEDED tag in .dynamic sections for the
  /// following shared objects that are not really used. MCLinker will add tags
  //  only for the following shared objects which is really used.
  void AsNeeded();

  /// NoAsNeeded - Append a --no-as-needed option on the command line.
  ///
  /// This function is equal to --no-as-needed option. This function tells
  /// MCLinker to add a DT_NEEDED tag in .dynamic section for every shared
  /// objects that is created after this option.
  void NoAsNeeded();

  /// CopyDTNeeded - Append a --add-needed option on the command line.
  ///
  /// This function is equal to --add-needed option. This function tells
  /// NCLinker to copy all DT_NEEDED tags of every following shared objects
  /// to the output file.
  void CopyDTNeeded();

  /// NoCopyDTNeeded - Append a --no-add-needed option on the command line.
  ///
  /// This function is equal to --no-add-needed option. This function tells
  /// MCLinker to stop copying all DT_NEEDS tags in the following shared
  /// objects to the output file.
  void NoCopyDTNeeded();

  /// AgainstShared - Append a -Bdynamic option on the command line.
  ///
  /// This function is equal to -Bdynamic option. This function tells MCLinker
  /// to search shared objects before archives for the following namespec.
  void AgainstShared();

  /// AgainstStatic - Append a -static option on the command line.
  ///
  /// This function is equal to -static option. This function tells MCLinker to
  /// search archives before shared objects for the following namespec.
  void AgainstStatic();

  /// @}
  /// @name Input Methods
  /// @{

  /// CreateELFHeader - To create and append a section header in the input file
  ///
  /// @param OF     [in]      The file format. @see ObjectFormat
  /// @param pInput [in, out] The input file.
  /// @param pName  [in]      The name of the section.
  /// @param pType  [in]      The meaning of the content in the section. The
  ///                         value is format-dependent. In ELF, the value is
  ///                         SHT_* in normal.
  /// @param pFlag  [in]      The format-dependent flag. In ELF, the value is
  ///                         SHF_* in normal.
  /// @param pAlign [in]      The alignment constraint of the section
  /// @return The created section header.
  static LDSection* CreateELFHeader(Input& pInput,
                                    const std::string& pName,
                                    uint32_t pType,
                                    uint32_t pFlag,
                                    uint32_t pAlign);

  /// CreateSectionData - To create a section data for given pSection.
  /// @param [in, out] pSection The given LDSection. It can be in either an
  ///         input or the output.
  ///         pSection.getSectionData() is set to a valid section data.
  /// @return The created section data. If the pSection already has section
  ///         data, or if the pSection's type should not have a section data
  ///         (.eh_frame or relocation data), then an assertion occurs.
  static SectionData* CreateSectionData(LDSection& pSection);

  /// CreateRelocData - To create a relocation data for given pSection.
  /// @param [in, out] pSection The given LDSection. It can be in either an
  ///         input or the output.
  ///         pSection.getRelocData() is set to a valid relocation data.
  /// @return The created relocation data. If the pSection already has
  ///         relocation data, or if the pSection's type is not
  ///         LDFileFormat::Relocation, then an assertion occurs.
  static RelocData* CreateRelocData(LDSection& pSection);

  /// CreateEhFrame - To create a eh_frame for given pSection
  /// @param [in, out] pSection The given LDSection. It can be in either an
  ///         input or the output.
  ///         pSection.getEhFrame() is set to a valid eh_frame.
  /// @return The created eh_frame. If the pSection already has eh_frame data,
  ///         or if the pSection's type is not LDFileFormat::EhFrame, then an
  ///         assertion occurs.
  static EhFrame* CreateEhFrame(LDSection& pSection);

  /// CreateDebugString - To create a debug_str for given pSection
  /// @param  pSection The given LDSection. It should be the output
  ///         .debug_str section
  ///         pSection.getDebugString() is set to a valid eh_frame.
  /// @return The created DebugString
  static DebugString* CreateDebugString(LDSection& pSection);

  /// CreateBSS - To create a bss section for given pSection
  /// @param [in, out] pSection The given LDSection. It can be in either an
  ///         input or the output.
  ///         pSection.getSectionData() is set to a valid section data and
  ///         contains a fillment fragment whose size is pSection.size().
  /// @return The create section data. It the pSection already has a section
  ///         data, or if the pSection's type is not LDFileFormat::BSS, then
  ///         an assertion occurs.
  static SectionData* CreateBSS(LDSection& pSection);

  /// CreateRegion - To create a region fragment in the input file.
  /// This function tells MCLinker to read a piece of data from the input
  /// file, and to create a region fragment that carries the data. The data
  /// will be deallocated automatically when pInput is destroyed.
  ///
  /// @param pInput  [in, out] The input file.
  /// @param pOffset [in]      The starting file offset of the data
  /// @param pLength [in]      The number of bytes of the data
  /// @return If pLength is zero or failing to request a region, return a
  ///         FillFragment.
  static Fragment* CreateRegion(Input& pInput, size_t pOffset, size_t pLength);

  /// CreateRegion - To create a region fragment wrapping the given memory.
  /// This function tells MCLinker to create a region fragment by the data
  /// directly. Since the data is given from outside, not read from the input
  /// file, users should deallocated the data manually.
  ///
  /// @param pMemory [in] The start address of the given data
  /// @param pLength [in] The number of bytes of the data
  /// @return If pLength is zero or failing to request a region, return a
  ///         FillFragment.
  static Fragment* CreateRegion(void* pMemory, size_t pLength);

  /// AppendFragment - To append pFrag to the given SectionData pSD.
  /// This function tells MCLinker to append a fragment to section data, and
  /// update size of the section header.
  ///
  /// @note In order to keep the alignment of pFrag, This function inserts an
  /// AlignFragment before pFrag if the section header's alignment is larger
  /// than 1.
  /// @note This function does not update offset of section headers.
  ///
  /// @param pFrag [in, out] The appended fragment. Its offset is set as the
  ///                        section offset in pSD.
  /// @param pSD   [in, out] The section data. Size of the header is also
  ///                        updated.
  /// @return Total size of the inserted fragments.
  static uint64_t AppendFragment(Fragment& pFrag, SectionData& pSD);

  /// AppendRelocation - To append a relocation to a relocation data.
  /// This function tells MCLinker to add a general relocation to the
  /// relocation data. This function does not update offset and size of section
  /// headers.
  ///
  /// @param pReloc [in]      The appended relocation.
  /// @param pRD    [in, out] The relocation data being appended.
  static void AppendRelocation(Relocation& pRelocation, RelocData& pRD);

  /// AppendEhFrame - To append a fragment to a EhFrame.
  /// @note In order to keep the alignment of pFrag, This function inserts an
  /// AlignFragment before pFrag if the section header's alignment is larger
  /// than 1.
  /// @note This function also update size of the section header, but does not
  /// update header's offset.
  ///
  /// @param pFrag    [in, out] The appended fragment.
  /// @param pEhFrame [in, out] The EhFrame.
  /// @return Total size of the inserted fragments.
  static uint64_t AppendEhFrame(Fragment& pFrag, EhFrame& pEhFrame);

  /// AppendEhFrame - To append a FDE to the given EhFrame pEhFram.
  /// @note In order to keep the alignment of pFrag, This function inserts an
  /// AlignFragment before pFrag if the section header's alignment is larger
  /// than 1.
  /// @note This function also update size of the section header, but does not
  /// update header's offset.
  ///
  /// @param [in, out] pFDE The appended FDE entry.
  /// @param [in, out] pEhFrame The eh_frame being appended.
  /// @return Total size of the inserted fragments.
  static uint64_t AppendEhFrame(EhFrame::FDE& pFDE, EhFrame& pEhFrame);

  /// AppendEhFrame - To append a CIE to the given EhFrame pEhFram.
  /// @note In order to keep the alignment of pFrag, This function inserts an
  /// AlignFragment before pFrag if the section header's alignment is larger
  /// than 1.
  /// @note This function also update size of the section header, but does not
  /// update header's offset.
  ///
  /// @param [in, out] pCIE The appended CIE entry.
  /// @param [in, out] pEhFrame The eh_frame being appended.
  /// @return Total size of the inserted fragments.
  static uint64_t AppendEhFrame(EhFrame::CIE& pCIE, EhFrame& pEhFrame);

  /// CreateLocalSymbol - Create a local symbol at the given FragmentRef.
  ResolveInfo* CreateLocalSymbol(FragmentRef& pFragRef);

  /// AddSymbol - To add a symbol to the input file.
  /// This function create a new symbol and insert it into the input file. If
  /// mcld::Module has another symbol with the same name, then this function
  /// resolves these two symbols and keeps one in mcld::Module by their
  /// attributes.
  ///
  /// This is a general method for all kinds of symbol.
  ///
  /// @param [in, out] pInput   The input file. Either a relocatable or dynamic
  ///                           object
  /// @param [in]      pName    The name of the symbol
  /// @param [in]      pType    What the symbol refers to. May be a object,
  ///                           function, no-type and so on. @see ResolveInfo
  /// @param [in]      pDesc    { Undefined, Define, Common, Indirect }
  /// @param [in]      pBind    { Global, Weak, Local, Absolute }
  /// @param [in]      pSize    The size of the symbol. Bigger common symbols
  ///                           overrides the smaller common symbols.
  /// @param [in]      pValue   Common symbols' value are alignment constraints
  ///                           Undefined symbols don't have value.
  ///                           The rest symbols' value are relative section
  ///                           offset.
  /// @param [in]      pSection Absolute, undefined, common symbols do not have
  ///                           pSection. Keep their pSection be NULL.
  /// @oaram [in]      pVis     The visibility of the symbol
  ///
  /// @return The added symbol. If the insertion fails due to the resoluction,
  /// return NULL.
  LDSymbol* AddSymbol(Input& pInput,
                      const std::string& pName,
                      ResolveInfo::Type pType,
                      ResolveInfo::Desc pDesc,
                      ResolveInfo::Binding pBind,
                      ResolveInfo::SizeType pSize,
                      LDSymbol::ValueType pValue = 0x0,
                      LDSection* pSection = NULL,
                      ResolveInfo::Visibility pVis = ResolveInfo::Default);

  /// AddSymbol - To add a symbol in mcld::Module
  /// This function create a new symbol and insert it into mcld::Module.
  ///
  /// @tparam POLICY idicate the condition to define or not to define the
  /// symbol.
  ///   - AsRefered
  ///     - Define a symbol only if mcld::Module contains a symbol with
  ///       identical name. If mcld::Module does not have any symbol with
  ///       the same name, this function returns NULL.
  ///
  ///   - Force
  ///     - Define a symbol no matter mcld::Module has a symbol with identical
  ///       name or not.
  ///
  /// @tparam RESOLVE indicate the method to define a symbol. If we must define
  /// a symbol in mcld::Module, then how to define it.
  ///
  ///   - Resolve
  ///      - Follow the symbol resolution rule to bind the symbol references.
  ///        Resolution of the symbols with idential name depends on their
  ///        attributes.
  ///
  ///   - Unresolve
  ///      - Forcefully override the symbol in mcld::Module.  With this
  ///        argument, AddSymbol function turns a blind eye to symbol
  ///        resolution rules.
  ///
  /// @param [in] pName    The name of the symbol
  /// @param [in] pType    The type of the symbol
  /// @param [in] pDesc    The description of the symbol, Could be one of
  ///                      { Undefined, Define, Common, Indirect }
  /// @param [in] pBinding The binding of the symbol. Could be one of
  ///                      { Global, Weak, Local, Absolute }
  ///
  /// @return The symbol kept in mcld::Module.
  template <SymbolDefinePolicy POLICY, SymbolResolvePolicy RESOLVE>
  LDSymbol* AddSymbol(
      const llvm::StringRef& pName,
      ResolveInfo::Type pType,
      ResolveInfo::Desc pDesc,
      ResolveInfo::Binding pBinding,
      ResolveInfo::SizeType pSize = 0,
      LDSymbol::ValueType pValue = 0x0,
      FragmentRef * pFragmentRef = FragmentRef::Null(),
      ResolveInfo::Visibility pVisibility = ResolveInfo::Default);

  /// AddRelocation - To add a relocation entry
  ///
  /// @param [in] pSection The relocation section. pSection's link should point
  /// to
  ///                      the target section.
  /// @param [in] pType    The type of the relocation (target dependent)
  /// @param [in] pSym     The symbol should be the symbol in the input file.
  /// @param [in] pOffset  The offset of target section.
  /// @param [in] pAddend  Tthe addend value for applying relocation
  static Relocation* AddRelocation(LDSection& pSection,
                                   Relocation::Type pType,
                                   LDSymbol& pSym,
                                   uint32_t pOffset,
                                   Relocation::Address pAddend = 0);

  /// shouldForceLocal - The helper function for AddSymbol to check if the
  /// symbols should be force to local symbols
  bool shouldForceLocal(const ResolveInfo& pInfo, const LinkerConfig& pConfig);

 private:
  LDSymbol* addSymbolFromObject(const std::string& pName,
                                ResolveInfo::Type pType,
                                ResolveInfo::Desc pDesc,
                                ResolveInfo::Binding pBinding,
                                ResolveInfo::SizeType pSize,
                                LDSymbol::ValueType pValue,
                                FragmentRef* pFragmentRef,
                                ResolveInfo::Visibility pVisibility);

  LDSymbol* addSymbolFromDynObj(Input& pInput,
                                const std::string& pName,
                                ResolveInfo::Type pType,
                                ResolveInfo::Desc pDesc,
                                ResolveInfo::Binding pBinding,
                                ResolveInfo::SizeType pSize,
                                LDSymbol::ValueType pValue,
                                ResolveInfo::Visibility pVisibility);

 private:
  Module& m_Module;
  const LinkerConfig& m_Config;

  InputBuilder m_InputBuilder;
};

template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility);

template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::AsReferred, IRBuilder::Unresolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility);

template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility);

template <>
LDSymbol* IRBuilder::AddSymbol<IRBuilder::AsReferred, IRBuilder::Resolve>(
    const llvm::StringRef& pName,
    ResolveInfo::Type pType,
    ResolveInfo::Desc pDesc,
    ResolveInfo::Binding pBinding,
    ResolveInfo::SizeType pSize,
    LDSymbol::ValueType pValue,
    FragmentRef* pFragmentRef,
    ResolveInfo::Visibility pVisibility);

}  // end of namespace mcld

#endif  // MCLD_IRBUILDER_H_
