DIAG(mips_got_symbol,
     DiagnosticEngine::Unreachable,
     "%0 is not a dynamic symbol, do not put it in global got",
     "%0 is not a dynamic symbol, do not put it in global got")
DIAG(fail_allocate_memory_got,
     DiagnosticEngine::Fatal,
     "fial to allocate memory for GOT",
     "fial to allocate memory for GOT")
DIAG(fail_allocate_memory_plt,
     DiagnosticEngine::Fatal,
     "fial to allocate memory for PLT",
     "fial to allocate memory for PLT")
DIAG(reserve_entry_number_mismatch_got,
     DiagnosticEngine::Unreachable,
     "The number of reserved entries for GOT is inconsist",
     "The number of reserved entries for GOT is inconsist")
DIAG(reserve_entry_number_mismatch_plt,
     DiagnosticEngine::Unreachable,
     "The number of reserved entries for PLT is inconsist",
     "The number of reserved entries for PLT is inconsist")
