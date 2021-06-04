DIAG(warn_unsupported_exception,
     DiagnosticEngine::Warning,
     "Exception handling has not been fully supported yet.\nsection `%0'.",
     "Exception handling has not been fully supported yet.\nsection `%0'.")
DIAG(warn_unsupported_symbolic_versioning,
     DiagnosticEngine::Warning,
     "Symbolic versioning has not been fully supported yet.\nsection `%0'.",
     "Symbolic versioning has not been fully supported yet.\nsection `%0'")
DIAG(err_section_not_laid_out,
     DiagnosticEngine::Unreachable,
     "section %0 has not been laid out. Developers may use an output LDSection "
     "in Layout::getFragmentRef",
     "section %0 has not been laid out. Developers may use an output LDSection "
     "in Layout::getFragmentRef")
DIAG(warn_duplicate_std_sectmap,
     DiagnosticEngine::Warning,
     "Duplicated definition of section map \"from %0 to %0\".",
     "Duplicated definition of section map \"from %0 to %0\".")
DIAG(warn_rules_check_failed,
     DiagnosticEngine::Warning,
     "Illegal section mapping rule: %0 -> %1. (conflict with %2 -> %3)",
     "Illegal section mapping rule: %0 -> %1. (conflict with %2 -> %3)")
DIAG(err_cannot_merge_section,
     DiagnosticEngine::Error,
     "Cannot merge section %0 of %1",
     "Cannot merge section %0 of %1")
DIAG(debug_icf_iterations,
     DiagnosticEngine::Debug,
     "ICF converged after `%0' iteration(s).",
     "ICF converged after `%0' iteration(s).")
DIAG(debug_icf_folded_section,
     DiagnosticEngine::Debug,
     "ICF folding section `%0' of `%1' into `%2' of `%3'",
     "ICF folding section `%0' of `%1' into `%2' of `%3'")
DIAG(err_no_space_to_place_stubs,
     DiagnosticEngine::Error,
     "There is no space left to place stubs. Current stub group size: %0\n"
     "Use --stub-group-size option to increase the group size.",
     "There is no space left to place stubs. Current stub group size: %0\n"
     "Use --stub-group-size option to increase the group size.")
