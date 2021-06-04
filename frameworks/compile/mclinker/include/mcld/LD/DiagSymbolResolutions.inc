DIAG(note_has_no_symtab,
     DiagnosticEngine::Note,
     "input file `%0' has no symbol table `%2'\n  path of input file: %1",
     "input file `%0' has no symbol table `%2'\n  path of input file: %1")
DIAG(fatal_cannot_read_strtab,
     DiagnosticEngine::Fatal,
     "cannot read strtab for %2 in file `%0': %1",
     "cannot read strtab for %2 in file `%0': %1")
DIAG(fail_sym_resolution,
     DiagnosticEngine::Unreachable,
     "Fails to resolve symbols [%0:%1]\nPlease reports to `%2'",
     "Fails to resolve symbols [%0:%1]\nPlease reports to `%2'")
DIAG(mark_dynamic_defined,
     DiagnosticEngine::Ignore,
     "refer to dynamic symbol %0",
     "call a external function %0")
DIAG(comm_refer_to_define,
     DiagnosticEngine::Ignore,
     "common symbol %0 is overridden by previous definition",
     "common symbol %0 is overridden by previous definition")
DIAG(redefine_common,
     DiagnosticEngine::Ignore,
     "common symbol %0 is overridden by definition",
     "common symbol %0 is overriden by definition")
DIAG(indirect_refer_to_common,
     DiagnosticEngine::Ignore,
     "indirect symbol %0 points to a common symbol",
     "indirect symbol %0 points to a common symbol")
DIAG(indirect_refer_to_inexist,
     DiagnosticEngine::Fatal,
     "indirect symbol %0 points to a undefined symbol",
     "variable %0 is undefined")
DIAG(multiple_definitions,
     DiagnosticEngine::Error,
     "multiple definition of symbol `%0'",
     "you define variable %0 twice")
DIAG(undefined_situation,
     DiagnosticEngine::Unreachable,
     "reach undefined situation, action: %0, old(%1) -> new(%2)",
     "reach undefined situation, action: %0, old(%1) -> new(%2)")
DIAG(multiple_absolute_definitions,
     DiagnosticEngine::Error,
     "inconsistent definitions of absolute symbol `%0': old(%1) -> new(%2)",
     "you defined an absolute symbol with different values")
