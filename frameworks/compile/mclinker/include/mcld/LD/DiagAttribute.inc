// General
DIAG(warn_unsupported_attribute_section_format,
     DiagnosticEngine::Warning,
     "unsupported format of attribute section in input %0 (version=%1).",
     "unsupported format of attribute section in input %0 (version=%1).")
DIAG(warn_unrecognized_vendor_subsection,
     DiagnosticEngine::Warning,
     "skip unrecognized private vendor subsection with name '%0' in %1.",
     "skip unrecognized private vendor subsection with name '%0' in %1.")

// ARM attributes
DIAG(error_unknown_cpu_arch,
     DiagnosticEngine::Error,
     "input %0 has unknown CPU architecture profile.",
     "input %0 has unknown CPU architecture profile.")
DIAG(warn_mismatch_cpu_arch_profile,
     DiagnosticEngine::Warning,
     "conflicting architecture profiles %0 in %1.",
     "conflicting architecture profiles %0 in %1.")
DIAG(error_mismatch_mpextension_use,
     DiagnosticEngine::Error,
     "conflicting values from Tag_MPextension_use and "
     "Tag_MPextension_use_legacy in %0",
     "conflicting values from Tag_MPextension_use and "
     "Tag_MPextension_use_legacy in %0")
DIAG(warn_mismatch_enum_size,
     DiagnosticEngine::Warning,
     "the size of enumerated data item in input %0 (value=%1) is not "
     "compatible with the output (value=%2).",
     "the size of enumerated data item in input %0 (value=%1) is not "
     "compatible with the output (value=%2).")
DIAG(warn_mismatch_fp16_format,
     DiagnosticEngine::Warning,
     "conflicting 16-bit FP number format in %0",
     "conflicting 16-bit FP number format in %0")
DIAG(warn_unrecognized_virtualization_use,
     DiagnosticEngine::Warning,
     "value of Tag_Virtualization_use cannot be recognized in %0 (value=%1).",
     "value of Tag_Virtualization_use cannot be recognized in %0 (value=%1).")
DIAG(warn_mismatch_abi_wmmx_args,
     DiagnosticEngine::Warning,
     "%0 uses different way to pass WMMX parameters and results.",
     "%0 uses different way to pass WMMX parameters and results.")
DIAG(warn_mismatch_pcs_config,
     DiagnosticEngine::Warning,
     "conflicting procedure call standard config in input %0.",
     "conflicting procedure call standard config in input %0.")
DIAG(warn_mismatch_r9_use,
     DiagnosticEngine::Warning,
     "conflicting way to use R9 in input %0.",
     "conflicting way to use R9 in input %0.")
DIAG(warn_conflict_rw_data_and_r9,
     DiagnosticEngine::Warning,
     "RW static data addressing (SB-relative) conflicts the use of R9 "
     "(Tag_ABI_PCS_R9_use) in input %0.",
     "RW static data addressing (SB-relative) conflicts the use of R9 "
     "(Tag_ABI_PCS_R9_use) in input %0.")
DIAG(warn_mismatch_wchar_size,
     DiagnosticEngine::Warning,
     "incompatible size of wchar_t in input %0 (value=%1) with the output "
     "(value=%2).",
     "incompatible size of wchar_t in input %0 (value=%1) with the output "
     "(value=%2).")
DIAG(warn_unknown_mandatory_attribute,
     DiagnosticEngine::Warning,
     "unknown mandatory attribute with tag %0 was ignored in %1.",
     "unknown mandatory attribute with tag %0 was ignored in %1.")
DIAG(warn_unknown_attribute,
     DiagnosticEngine::Warning,
     "unknown attribute with tag %0 was ignored in %1.",
     "unknown attribute with tag %0 was ignored in %1.")
DIAG(warn_mismatch_vfp_args,
     DiagnosticEngine::Warning,
     "%0 uses different way to pass the floating point parameter and results.",
     "%0 uses different way to pass the floating point parameter and results.")
