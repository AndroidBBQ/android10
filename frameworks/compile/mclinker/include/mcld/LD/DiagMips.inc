// Mips specific errors and warnings on object file incompatibilities
DIAG(error_Mips_incompatible_class,
     DiagnosticEngine::Fatal,
     "target '%0' is incompatible with '%1' in %2",
     "target '%0' is incompatible with '%1' in %2")
DIAG(error_Mips_inconsistent_arch,
     DiagnosticEngine::Error,
     "target arch '%0' is inconsist with the '%1' in %2",
     "target arch '%0' is inconsist with the '%1' in %2")
DIAG(error_Mips_abiflags_invalid_size,
     DiagnosticEngine::Error,
     "invalid size of .MIPS.abiflags section in %0",
     "invalid size of .MIPS.abiflags section in %0")
DIAG(error_Mips_abiflags_invalid_version,
     DiagnosticEngine::Error,
     "unexpected .MIPS.abiflags section version number '%0' in %1",
     "unexpected .MIPS.abiflags section version number '%0' in %1")
DIAG(error_Mips_inconsistent_abi,
     DiagnosticEngine::Error,
     "target ABI is incompatible with ABI in %0",
     "target ABI is incompatible with ABI in %0")
DIAG(error_Mips_inconsistent_mnan,
     DiagnosticEngine::Error,
     "target -mnan=%0 flag is incompatible with -mnan=%1 in %2",
     "target -mnan=%0 flag is incompatible with -mnan=%1 in %2")
DIAG(error_Mips_inconsistent_fp64,
     DiagnosticEngine::Error,
     "target -mfp flag is incompatible with -mfp in %0",
     "target -mfp flag is incompatible with -mfp in %0")
DIAG(error_Mips_m16_unsupported,
     DiagnosticEngine::Error,
     "MIPS16 extension is unsupported: %0",
     "MIPS16 extension is unsupported: %0")
DIAG(warn_Mips_abicalls_linking,
     DiagnosticEngine::Warning,
     "conflicting linking abicalls and non-abicalls files on %0.",
     "conflicting linking abicalls and non-abicalls files on %0.")
DIAG(warn_Mips_fp_abi_incompatible,
     DiagnosticEngine::Warning,
     "FP ABI %0 is incompatible with %1 used by %2",
     "FP ABI %0 is incompatible with %1 used by %2")
DIAG(warn_Mips_isa_incompatible,
     DiagnosticEngine::Warning,
     "inconsistent ISA between .MIPS.abiflags and ELF header e_flags field: %0",
     "inconsistent ISA between .MIPS.abiflags and ELF header e_flags field: %0")
DIAG(warn_Mips_isa_ext_incompatible,
     DiagnosticEngine::Warning,
     "inconsistent ISA extensions between .MIPS.abiflags and ELF header e_flags field: %0",
     "inconsistent ISA extensions between .MIPS.abiflags and ELF header e_flags field: %0")
DIAG(warn_Mips_ases_incompatible,
     DiagnosticEngine::Warning,
     "inconsistent ASEs between .MIPS.abiflags and ELF header e_flags field: %0",
     "inconsistent ASEs between .MIPS.abiflags and ELF header e_flags field: %0")
