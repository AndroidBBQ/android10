//===- Options.td ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
include "llvm/Option/OptParser.td"

//===----------------------------------------------------------------------===//
// Dynamic
//===----------------------------------------------------------------------===//
def DynamicGroup : OptionGroup<"dynamic">,
                   HelpText<"DYNAMIC OPTIONS">;

def Entry : Joined<["--"], "entry=">,
            Group<DynamicGroup>,
            HelpText<"Use the explicit symbol as the entrance of your program">;
def EntryAlias : Separate<["-"], "e">,
                 Alias<Entry>;

def Bsymbolic : Flag<["-"], "Bsymbolic">,
                Group<DynamicGroup>,
                HelpText<"Bind references within the shared library">;

def Bgroup : Flag<["-"], "Bgroup">,
             Group<DynamicGroup>,
             HelpText<"Info the dynamic linker to lookup only inside the group">;

def SOName : Joined<["-"], "soname=">,
             Group<DynamicGroup>,
             HelpText<"Set internal name of shared library">;
def SONameAlias : Separate<["-"], "soname">,
                  Group<DynamicGroup>,
                  Alias<SOName>;

def NoUndef : Flag<["--"], "no-undefined">,
              Group<DynamicGroup>,
              HelpText<"Do not allow unresolved references">;

def AllowMulDefs : Flag<["--"], "allow-multiple-definition">,
                   Group<DynamicGroup>,
                   HelpText<"Allow multiple definitions">;

def Z : Separate<["-"], "z">,
        Group<DynamicGroup>,
        HelpText<"Extensions for GNU ld compatibility">;

def Dyld : Joined<["--"], "dynamic-linker=">,
           Group<DynamicGroup>,
           HelpText<"Set the name of the dynamic linker">;
def DyldAlias : Separate<["-"], "dynamic-linker">,
                Group<DynamicGroup>,
                Alias<Dyld>;

def EnableNewDTags : Flag<["--"], "enable-new-dtags">,
                     Group<DynamicGroup>,
                     HelpText<"Enable use of DT_RUNPATH and DT_FLAGS">;

def SpareDTags: Separate<["--"], "spare-dynamic-tags">,
                Group<DynamicGroup>,
                HelpText<"Set the number of spare dyanmic tags (DT_NULL)">;

//===----------------------------------------------------------------------===//
// Optimization
//===----------------------------------------------------------------------===//
def OptimizationGroup : OptionGroup<"optimization">,
                        HelpText<"OPTIMIZATION OPTIONS">;

def GCSections : Flag<["--"], "gc-sections">,
                 Group<OptimizationGroup>,
                 HelpText<"Enable garbage collection of unused input sections">;

def NoGCSections : Flag<["--"], "no-gc-sections">,
                   Group<OptimizationGroup>,
                   HelpText<"Disable garbage collection of unused input sections">;

def PrintGCSections : Flag<["--"], "print-gc-sections">,
                      Group<OptimizationGroup>,
                      HelpText<"List all sections removed by garbage collection">;

def NoPrintGCSections : Flag<["--"], "no-print-gc-sections">,
                        Group<OptimizationGroup>,
                        HelpText<"Do not list sections removed by garbage collection">;

def LDGeneratedUnwindInfo : Flag<["--"], "ld-generated-unwind-info">,
                            Group<OptimizationGroup>,
                            HelpText<"Request creation of unwind info for linker generated code sections like PLT">;

def NoLDGeneratedUnwindInfo : Flag<["--"], "no-ld-generated-unwind-info">,
                              Group<OptimizationGroup>,
                              HelpText<"Don't create unwind info for linker generated sections to save size">;

def ICF : Joined<["--"], "icf=">,
          Group<OptimizationGroup>,
          HelpText<"Identical Code Folding">;

def ICFIters : Separate<["--"], "icf-iterations">,
               Group<OptimizationGroup>,
               HelpText<"Set number of iterations to do ICF">;

def PrintICFSections : Flag<["--"], "print-icf-sections">,
                       Group<OptimizationGroup>,
                       HelpText<"List all sections folded by ICF">;

def NoPrintICFSections : Flag<["--"], "no-print-icf-sections">,
                         Group<OptimizationGroup>,
                         HelpText<"Do not list sections folded by ICF">;

//===----------------------------------------------------------------------===//
// Output
//===----------------------------------------------------------------------===//
def OutputGroup : OptionGroup<"output">,
                  HelpText<"OUTPUT OPTIONS">;

def Output : Separate<["-"], "o">,
             Group<OutputGroup>,
             HelpText<"Output filename">;
def OutputAlias : Joined<["--"], "output=">,
                  Group<OutputGroup>,
                  Alias<Output>;

def OutputFormat: Joined<["--"], "oformat=">,
                  Group<OutputGroup>,
                  HelpText<"Output format">;

def Shared : Flag<["-"], "shared">,
             Group<OutputGroup>,
             HelpText<"Create a shared library">;
def Bshareable : Flag<["-"], "Bshareable">,
                 Group<OutputGroup>,
                 Alias<Shared>;

def Bdynamic : Flag<["-"], "Bdynamic">,
               Group<OutputGroup>,
               HelpText<"Link against dynamic library">;
def BdynamicAlias1 : Flag<["-"], "dy">,
                     Group<OutputGroup>,
                     Alias<Bdynamic>;
def BdynamicAlias2 : Flag<["-"], "call_shared">,
                     Group<OutputGroup>,
                     Alias<Bdynamic>;

def Bstatic : Flag<["-"], "Bstatic">,
              Group<OutputGroup>,
              HelpText<"Link against static library">;
def BstaticAlias1 : Flag<["-"], "dn">,
                    Group<OutputGroup>,
                    Alias<Bstatic>;
def BstaticAlias2 : Flag<["-"], "non_shared">,
                    Group<OutputGroup>,
                    Alias<Bstatic>;
def BstaticAlias3 : Flag<["-"], "static">,
                    Group<OutputGroup>,
                    Alias<Bstatic>;

def PIE : Flag<["-"], "pie">,
          Group<OutputGroup>,
          HelpText<"Emit a position-independent executable file">;
def PICExec : Flag<["--"], "pic-executable">,
              Group<OutputGroup>,
              Alias<PIE>;

def Relocatable : Flag<["--"], "relocatable">,
                  Group<OutputGroup>,
                  HelpText<"Generate relocatable output">;
def RelocatableAlias : Flag<["-"], "r">,
                       Group<OutputGroup>,
                       Alias<Relocatable>;

def InputFormat : Joined<["--"], "format=">,
                  Group<OutputGroup>,
                  HelpText<"Specify the binary format for input object files that follow this option on the command line">;
def InputFormatAlias : Separate<["-"], "b">,
                       Group<OutputGroup>,
                       Alias<InputFormat>;

def StripDebug : Flag<["--"], "strip-debug">,
                 Group<OutputGroup>,
                 HelpText<"Omit debugger symbol information from the output file">;
def StripDebugAlias : Flag<["-"], "S">,
                      Group<OutputGroup>,
                      Alias<StripDebug>;

def StripAll : Flag<["--"], "strip-all">,
               Group<OutputGroup>,
               HelpText<"Omit all symbol information from the output file">;
def StripAllAlias : Flag<["-"], "s">,
                    Group<OutputGroup>,
                    Alias<StripAll>;

def DiscardLocals : Flag<["--"], "discard-debug">,
                    Group<OutputGroup>,
                    HelpText<"Discard all temporary local symbols">;
def DiscardLocalsAlias : Flag<["-"], "X">,
                         Group<OutputGroup>,
                         Alias<DiscardLocals>;

def DiscardAll : Flag<["--"], "discard-all">,
                 Group<OutputGroup>,
                 HelpText<"Discard all local symbols">;
def DiscardAllAlias : Flag<["-"], "x">,
                      Group<OutputGroup>,
                      Alias<DiscardAll>;

def EHFrameHdr : Flag<["--"], "eh-frame-hdr">,
                 Group<OutputGroup>,
                 HelpText<"Request creation of .eh_frame_hdr section and PT_GNU_EH_FRAME segment">;

def NMagic : Flag<["--"], "nmagic">,
             Group<OutputGroup>,
             HelpText<"Do not page align data">;
def NMagicAlias : Flag<["-"], "n">,
                  Group<OutputGroup>,
                  Alias<NMagic>;

def OMagic : Flag<["--"], "omagic">,
             Group<OutputGroup>,
             HelpText<"Do not page align data, do not make text readonly">;
def OMagicAlias : Flag<["-"], "N">,
                  Group<OutputGroup>,
                  Alias<OMagic>;

def HashStyle : Joined<["--"], "hash-style=">,
                Group<OutputGroup>,
                HelpText<"Set the type of linker's hash table(s)">;

def ExportDynamic : Flag<["--"], "export-dynamic">,
                    Group<OutputGroup>,
                    HelpText<"Export all dynamic symbols">;
def ExportDynamicAlias : Flag<["-"], "E">,
                         Group<OutputGroup>,
                         Alias<ExportDynamic>;

def NoExportDynamic : Flag<["--"], "no-export-dynamic">,
                   Group<OutputGroup>,
                   HelpText<"Not export all dynamic symbols">;



def NoWarnMismatch : Flag<["--"], "no-warn-mismatch">,
                     Group<OutputGroup>,
                     HelpText<"Allow linking together mismatched input files">;

def ExcludeLibs : Separate<["--"], "exclude-libs">,
                  Group<OutputGroup>,
                  HelpText<"Allow linking together mismatched input files">;

//===----------------------------------------------------------------------===//
// Positional
//===----------------------------------------------------------------------===//
def PositionalGroup : OptionGroup<"positional">,
                      HelpText<"POSITIONAL OPTIONS">;

def Script : Joined<["--"], "script=">,
             Group<PositionalGroup>,
             HelpText<"Use the given file as the linker script">;
def ScriptAlias : Separate<["-"], "T">,
                  Group<PositionalGroup>,
                  Alias<Script>;

def Namespec : Joined<["--"], "library=">,
               Group<PositionalGroup>,
               HelpText<"Add the archive or object file specified by namespec to the list of files to link">;
def NamespecAlias : Joined<["-"], "l">,
                    Group<PositionalGroup>,
                    Alias<Namespec>;

def WholeArchive : Flag<["--"], "whole-archive">,
                   Group<PositionalGroup>,
                   HelpText<"Include every object file in the archive in the link">;

def NoWholeArchive : Flag<["--"], "no-whole-archive">,
                     Group<PositionalGroup>,
                     HelpText<"Turn off the effect of the --whole-archive option">;

def AsNeeded : Flag<["--"], "as-needed">,
               Group<PositionalGroup>,
               HelpText<"Add the dynamic libraries mentioned to DT_NEEDED where there is "
                        "a non-weak undefined symbol reference from">;

def NoAsNeeded : Flag<["--"], "no-as-needed">,
                 Group<PositionalGroup>,
                 HelpText<"Turn off the effect of the --as-needed">;

def AddNeeded : Flag<["--"], "add-needed">,
                Group<PositionalGroup>,
                HelpText<"Deprecated">;

def NoAddNeeded : Flag<["--"], "no-add-needed">,
                  Group<PositionalGroup>,
                  HelpText<"Deprecated">;

def CopyDTNeeded : Flag<["--"], "copy-dt-needed-entries">,
                   Group<PositionalGroup>,
                   HelpText<"Add the dynamic libraries mentioned to DT_NEEDED">;

def NoCopyDTNeeded : Flag<["--"], "no-copy-dt-needed-entries">,
                     Group<PositionalGroup>,
                     HelpText<"Turn off the effect of the --copy-dt-needed-entries">;

def StartGroup : Flag<["--"], "start-group">,
                 Group<PositionalGroup>,
                 HelpText<"Start to record a group of archives">;
def StartGroupAlias : Flag<["-"], "(">,
                      Group<PositionalGroup>,
                      Alias<StartGroup>;

def EndGroup : Flag<["--"], "end-group">,
               Group<PositionalGroup>,
               HelpText<"Stop recording a group of archives">;
def EndGroupAlias : Flag<["-"], ")">,
                    Group<PositionalGroup>,
                    Alias<EndGroup>;

//===----------------------------------------------------------------------===//
// Preference
//===----------------------------------------------------------------------===//
def PreferenceGroup : OptionGroup<"preference">,
                      HelpText<"PREFERENCE OPTIONS">;

def Color : Joined<["--"], "color=">,
            Group<PreferenceGroup>,
            HelpText<"Surround the result strings with the marker">;

def Trace : Flag<["--"], "trace">,
            Group<PreferenceGroup>,
            HelpText<"Print the names of the input files as ld processes them">;
def TraceAlias : Flag<["-"], "t">,
                 Group<PreferenceGroup>,
                 Alias<Trace>;

def Help : Flag<["-", "--"], "help">,
           Group<PreferenceGroup>,
           HelpText<"Display available options (to standard output)">;
def HelpAlias : Flag<["-"], "h">,
                Group<PreferenceGroup>,
                Alias<Help>;

def Verbose : Joined<["--"], "verbose=">,
              Group<PreferenceGroup>,
              HelpText<"Set linker diagnostic output level">;

def Version : Flag<["--"], "version">,
              Group<PreferenceGroup>,
              HelpText<"Display MCLinker version">;
def VersionAlias1 : Flag<["-"], "v">,
                    Group<PreferenceGroup>,
                    Alias<Version>;
def VersionAlias2 : Flag<["-"], "V">,
                    Group<PreferenceGroup>,
                    Alias<Version>;

def ErrorLimit : Joined<["--"], "error-limit=">,
                 Group<PreferenceGroup>,
                 HelpText<"Set the maximum limit of errors">;

def WarningLimit : Joined<["--"], "warning-limit=">,
                   Group<PreferenceGroup>,
                   HelpText<"Set the maximum limit of warnings">;

def FatalWarnings : Flag<["--"], "fatal-warnings">,
                    Group<PreferenceGroup>,
                    HelpText<"Turn all warnings into errors">;

def NoFatalWarnings : Flag<["--"], "no-fatal-warnings">,
                      Group<PreferenceGroup>,
                      HelpText<"Do not turn all warnings into errors">;

def WarnSharedTextrel : Flag<["--"], "warn-shared-textrel">,
                        Group<PreferenceGroup>,
                        HelpText<"Warn if there is a text relocation in the output shared object">;

//===----------------------------------------------------------------------===//
// Script
//===----------------------------------------------------------------------===//
def ScriptGroup : OptionGroup<"script">,
                  HelpText<"SCRIPT OPTIONS">;

def DefSym : Joined<["--"], "defsym=">,
             Group<ScriptGroup>,
             HelpText<"Define a symbol">;

def Wrap : Joined<["--"], "wrap=">,
           Group<ScriptGroup>,
           HelpText<"Use a wrap function for the symbol">;

def Portable : Joined<["--"], "portable=">,
               Group<ScriptGroup>,
               HelpText<"Use a portable function for the symbol">;

def SectionStart : Joined<["--"], "section-start=">,
                   Group<ScriptGroup>,
                   HelpText<"Locate a output section at the given absolute address">;

def Tbss : Joined<["-"], "Tbss=">,
           Group<ScriptGroup>,
           HelpText<"Set the address of the bss segment">;

def Tdata : Joined<["-"], "Tdata=">,
            Group<ScriptGroup>,
            HelpText<"Set the address of the data segment">;

def Ttext : Joined<["-"], "Ttext=">,
            Group<ScriptGroup>,
            HelpText<"Set the address of the text segment">;

//===----------------------------------------------------------------------===//
// Search Path
//===----------------------------------------------------------------------===//
def SearchpathGroup : OptionGroup<"searchpath">,
                      HelpText<"SEARCHPATH OPTIONS">;

def Sysroot : Joined<["--"], "sysroot=">,
              Group<SearchpathGroup>,
              HelpText<"Use the given directory as the location of the sysroot">;

def LibraryPath : Joined<["--"], "library-path=">,
                  Group<SearchpathGroup>,
                  HelpText<"Add the given directory to the list of search paths">;
def LibraryPathAlias : Joined<["-"], "L">,
                       Group<SearchpathGroup>,
                       Alias<LibraryPath>;

def NoStdlib : Flag<["-"], "nostdlib">,
               Group<SearchpathGroup>,
               HelpText<"Only search lib dirs explicitly specified on cmdline">;

def RPath : Joined<["-"], "rpath=">,
            Group<SearchpathGroup>,
            HelpText<"Add a directory to the runtime library search path">;

//===----------------------------------------------------------------------===//
// Symbol
//===----------------------------------------------------------------------===//
def SymbolGroup : OptionGroup<"symbol">,
                  HelpText<"SYMBOL OPTIONS">;

def Undefined : Joined<["--"], "undefined=">,
                Group<SymbolGroup>,
                HelpText<"Force symbol to be undefined in the output file">;
def UndefinedAlias : Separate<["-"], "u">,
                     Group<SymbolGroup>,
                     Alias<Undefined>;

def DefineCommon : Flag<["-"], "d">,
                   Group<SymbolGroup>,
                   HelpText<"Define common symbol">;
def DefineCommonAlias1 : Flag<["-"], "dc">,
                         Group<SymbolGroup>,
                         Alias<DefineCommon>;
def DefineCommonAlias2 : Flag<["-"], "dp">,
                         Group<SymbolGroup>,
                         Alias<DefineCommon>;

//===----------------------------------------------------------------------===//
// Target
//===----------------------------------------------------------------------===//
def TargetGroup : OptionGroup<"target">,
                  HelpText<"TARGET OPTIONS">;

def GPSize : Joined<["--"], "gpsize=">,
             Group<TargetGroup>,
             HelpText<"Set the maximum size of objects to be optimized using GP">;
def GPSizeAlias : Separate<["-"], "G">,
                  Group<TargetGroup>,
                  Alias<GPSize>;

def Triple : Joined<["-"], "mtriple=">,
             Group<TargetGroup>,
             HelpText<"Override target triple for module">;

def Arch : Joined<["-"], "march=">,
           Group<TargetGroup>,
           HelpText<"Architecture to generate code for">;

def CPU : Joined<["-"], "mcpu=">,
          Group<TargetGroup>,
          HelpText<"Set a specific cpu type">;

def Emulation : Separate<["-"], "m">,
                Group<TargetGroup>,
                HelpText<"Set GNU linker emulation">;

def StubGroupSize : Joined<["--"], "stub-group-size=">,
                    Group<TargetGroup>,
                    HelpText<"Set the group size to place stubs between sections">;

def FixCA53Erratum835769 : Flag<["--"], "fix-cortex-a53-835769">,
                           Group<TargetGroup>,
                           HelpText<"Enable fix for cortex a53 erratum 835769">;

def FixCA53Erratum843419 : Flag<["--"], "fix-cortex-a53-843419">,
                           Group<TargetGroup>,
                           HelpText<"Enable fix for cortex a53 erratum 843419">;
