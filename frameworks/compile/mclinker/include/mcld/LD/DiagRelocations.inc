DIAG(reloc_factory_has_not_config,
     DiagnosticEngine::Fatal,
     "Please call mcld::Linker::config before creating relocations",
     "Please call mcld::Linker::config before creating relocations")
DIAG(unsupported_bitclass,
     DiagnosticEngine::Fatal,
     "Only supports 32 and 64 bits targets. (Target: %0, bitclass:%1)",
     "Only supports 32 and 64 bits targets. (Target: %0, bitclass:%1)")
DIAG(undefined_reference,
     DiagnosticEngine::Fatal,
     "%1(%2+%3): undefined reference to `%0'",
     "%1(%2+%3): undefined reference to `%0'")
DIAG(undefined_reference_text,
     DiagnosticEngine::Fatal,
     "%1:%2:function %3: undefined reference to `%0'",
     "%1:%2: undefined reference to `%0'")
DIAG(non_pic_relocation,
     DiagnosticEngine::Error,
     "attempt to generate unsupported relocation `%0' for symbol `%1', "
     "recompile with -fPIC",
     "attempt to generate unsupported relocation `%0' for symbol `%1, "
     "recompile with -fPIC")
DIAG(base_relocation,
     DiagnosticEngine::Fatal,
     "relocation type `%0' is not supported for symbol `%1'\nPlease report to "
     "%2",
     "relocation type `%0' is not supported for symbol `%1'\nPlease report to "
     "%2")
DIAG(dynamic_relocation,
     DiagnosticEngine::Fatal,
     "unexpected relocation type `%0' in object file",
     "unexpected relocation type `%0' in object file")
DIAG(unsupported_relocation,
     DiagnosticEngine::Unreachable,
     "encounter unsupported relocation type `%0'\nPlease report to %1",
     "encounter unsupported relocation type `%0'\nPlease report to %1")
DIAG(unknown_relocation,
     DiagnosticEngine::Fatal,
     "encounter unknown relocation type `%0' for symbol `%1'",
     "encounter unknown relocation type `%0' for symbol `%1'")
DIAG(invalid_global_relocation,
     DiagnosticEngine::Unreachable,
     "relocation type `%0' is invalid for global symbol `%1'",
     "relocation type `%0' is invalid for global symbol `%1'")
DIAG(result_overflow,
     DiagnosticEngine::Error,
     "applying relocation `%0' causes overflow on symbol `%1'",
     "applying relocation `%0' causes overflow on symbol `%1'")
DIAG(result_badreloc,
     DiagnosticEngine::Error,
     "applying relocation `%0' encounters unexpected opcode on symbol `%1'",
     "applying relocation `%0' encounters unexpected opcode on symbol `%1'")
DIAG(invalid_tls,
     DiagnosticEngine::Error,
     "TLS relocation against invalid symbol `%0' in section `%1'",
     "TLS relocation against invalid symbol `%0' in section `%1'")
DIAG(unknown_reloc_section_type,
     DiagnosticEngine::Unreachable,
     "unknown relocation section type: `%0' in section `%1'",
     "unknown relocation section type: `%0' in section `%1'")
DIAG(unsupported_cond_branch_reloc,
     DiagnosticEngine::Error,
     "applying relocation `%0', conditional branch to PLT in THUMB-2 not "
     "supported yet",
     "applying relocation `%0', conditional branch to PLT in THUMB-2 not "
     "supported yet")
DIAG(unsupport_reloc_for_debug_string,
     DiagnosticEngine::Error,
     "applying relocation `%0' for .debug_str is not supported. "
     "Please report to %1",
     "applying relocation `%0' for .debug_str is not supported. "
     "Please report to %1")
