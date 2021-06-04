DIAG(err_cannot_open_input,
     DiagnosticEngine::Error,
     "can not open input file `%0': %1",
     "can not open input file `%0' : %1")
DIAG(err_cannot_open_output_file,
     DiagnosticEngine::Fatal,
     "cannot open output file `%0': %1",
     "cannot open output file `%0': %1")
DIAG(warn_cannot_open_search_dir,
     DiagnosticEngine::Warning,
     "can not open search directory `-L%0'",
     "can not open search directory `-L%0'")
DIAG(err_no_inputs, DiagnosticEngine::Error, "no inputs", "no inputs")
DIAG(err_empty_input,
     DiagnosticEngine::Error,
     "Empty input file `%0' : %1",
     "Empty input file `%0' : %1")
DIAG(warn_unrecognized_input_file,
     DiagnosticEngine::Warning,
     "cannot recognize the format of file `%0'.\n  object format or given "
     "target machine (%1) is incompatible.",
     "cannot recognize the format of file `%0'.\n  object format or given "
     "target machine (%1) is incompatible.")
DIAG(err_cannot_find_namespec,
     DiagnosticEngine::Fatal,
     "cannot recognize namespec -l%0",
     "cannot recognize namespec -l%0")
DIAG(err_cannot_identify_option,
     DiagnosticEngine::Fatal,
     "unknown command line argument `%0' at %1",
     "unknown command line argument `%0' at %1")
DIAG(err_mixed_shared_static_objects,
     DiagnosticEngine::Error,
     "cannot link shared objects with -static option.\nShared object `%0': %1",
     "cannot link shared objects with -static option.\nShared object `%0': %1")
DIAG(unrecognized_output_file,
     DiagnosticEngine::Unreachable,
     "unsupported output file format: `%0'",
     "unsupported output file format: `%0'")
DIAG(unrecognized_output_sectoin,
     DiagnosticEngine::Unreachable,
     "Unable to emit section `%0'\nPlease report to `%1'",
     "Unable to emit section `%0'\nPlease report to `%1'")
DIAG(duplicated_wrap,
     DiagnosticEngine::Warning,
     "wrapped symbol `%0' had been claimed",
     "wrapped symbol `%0' had been claimed")
DIAG(rewrap,
     DiagnosticEngine::Warning,
     "There are duplicated --wrap `%0' on the command line\nsymbol `%1' had "
     "been claimed",
     "There are duplicated --wrap `%0' on the command line]\nsymbol `%1' had "
     "been claimed.")
DIAG(err_cannot_read_symbol,
     DiagnosticEngine::Fatal,
     "can not read symbol[%0] in file %1",
     "can not read symbol[%0] in file %1")
DIAG(err_cannot_read_section,
     DiagnosticEngine::Fatal,
     "can not read section `%0'.",
     "can not read section `%0'.")
DIAG(err_cannot_read_target_section,
     DiagnosticEngine::Fatal,
     "can not read target-dependent section `%0'.",
     "can not read target-dependent section `%0'.")
DIAG(err_cannot_read_relocated_section,
     DiagnosticEngine::Fatal,
     "can not read the section being relocated in file %0.\ninvalid sh_info: "
     "%1\nrelocation section: %2",
     "can not read the section being relocated in file %0.\ninvalid sh_info: "
     "%1\nrelocation section: %2")
DIAG(err_unsupported_section,
     DiagnosticEngine::Fatal,
     "unsupported section `%0' (type %1)",
     "unsupported section `%0' (type %1)")
DIAG(unreachable_invalid_section_idx,
     DiagnosticEngine::Unreachable,
     "section[%0] is invalid in file %1",
     "section[%0] is invalid in file %1")
DIAG(err_unsupported_whole_archive,
     DiagnosticEngine::Error,
     "Target does not support --whole-archive",
     "Target does not support --whole-archive")
DIAG(err_unsupported_as_needed,
     DiagnosticEngine::Error,
     "Target does not support --as-needed",
     "Target does not support --as-needed")
DIAG(err_unsupported_add_needed,
     DiagnosticEngine::Error,
     "Target doest not support --add-needed",
     "Target does not support --add-needed")
DIAG(err_unsupported_Bdynamic,
     DiagnosticEngine::Error,
     "Target does not support --Bdynamic",
     "Target does not support --Bdynamic")
DIAG(err_enable_as_needed_on_static_system,
     DiagnosticEngine::Warning,
     "can not enable --as-needed on the target which does not support shared "
     "objects",
     "can not enable --as-needed on the target which does not support shared "
     "objects")
DIAG(err_mix_static_as_needed,
     DiagnosticEngine::Warning,
     "cannot mix --static with --as-needed",
     "cannot mix --static with --as-needed")
DIAG(err_cannot_change_file_size,
     DiagnosticEngine::Error,
     "cannot truncate file `%0' to size %1",
     "cannot truncate ffile `%0' to size %1")
DIAG(err_cannot_open_file,
     DiagnosticEngine::Error,
     "cannot open file `%0': %1.",
     "cannot open file `%0': %1.")
DIAG(err_cannot_close_file,
     DiagnosticEngine::Error,
     "cannot close file `%0': %1.",
     "cannot close file `%0': %1.")
DIAG(err_cannot_read_file,
     DiagnosticEngine::Error,
     "cannot read file %0 from offset %1 to length %2.",
     "cannot read file %0 from offset %1 to length %2.")
DIAG(err_cannot_read_small_file,
     DiagnosticEngine::Fatal,
     "file %0 is too small to read.\n  file size is %1.\n  read from %2.",
     "file %0 is too small to read.\n  file size is %1.\n  read from %2.")
DIAG(err_cannot_mmap_file,
     DiagnosticEngine::Error,
     "cannot open memory mapped file %0 from offset %1 to length %2.",
     "cannot open memoory mpped file %0 from offset %1 to length %2.")
DIAG(err_cannot_munmap_file,
     DiagnosticEngine::Error,
     "cannot remove the mapped memory of file %0.",
     "cannot remove the mapped memory of file %0.")
DIAG(err_cannot_write_file,
     DiagnosticEngine::Error,
     "cannot write file %0 from offset %1 to length %2.",
     "cannot write file %0 from offset %1 to length %2.")
DIAG(warn_illegal_input_section,
     DiagnosticEngine::Warning,
     "section `%0' should not appear in input file `%1': %2",
     "section `%0' should not appear in input file `%1': %2")
DIAG(err_cannot_trace_file,
     DiagnosticEngine::Unreachable,
     "cannot identify the type (%0) of input file `%1'.\n  %2",
     "cannot identify the type (%0) of input file `%1'.\n  %2")
DIAG(err_out_of_range_region,
     DiagnosticEngine::Unreachable,
     "requested memory region [%0, %1] is out of range.",
     "requested memory region [%0, %1] is out of range.")
DIAG(debug_eh_unsupport,
     DiagnosticEngine::Debug,
     "unsupported .eh_frame section in input: %0",
     "unsupported .eh_frame section in input: %0")
DIAG(note_eh_cie,
     DiagnosticEngine::Note,
     "CIE length: %0, aug_string: %1, fde_encodeing: %2",
     "CIE length: %0, aug_string: %1, fde_encodeing: %2")
DIAG(note_eh_fde,
     DiagnosticEngine::Note,
     "FDE length: %0, offset of PC Begin: %1",
     "FDE length: %0, offset of PC Begin: %1")
DIAG(fatal_cannot_init_target,
     DiagnosticEngine::Fatal,
     "Cannot initialize mcld::Target for given triple '%0'.\n(Detail: %1)",
     "Cannot initialize mcld::Target for given triple '%0'.\n(Detail: %1)")
DIAG(fatal_cannot_init_lineinfo,
     DiagnosticEngine::Fatal,
     "Cannot initialize mcld::DiagnosticLineInfo for given triple '%0'",
     "Cannot initialize mcld::DiagnosticLineInfo for given triple '%0'")
DIAG(fatal_cannot_init_backend,
     DiagnosticEngine::Fatal,
     "Cannot initialize mcld::TargetLDBackend for given triple '%0'.",
     "Cannot initialize mcld::TargetLDBackend for given triple '%0'.")
DIAG(fatal_forbid_nest_group,
     DiagnosticEngine::Fatal,
     "not matched --start-group and --end-group",
     "not matched --start-group and --end-group")
DIAG(fatal_unwritable_output,
     DiagnosticEngine::Fatal,
     "unable to write output file %0",
     "unable to write output file %0")
DIAG(warn_unsupported_option,
     DiagnosticEngine::Warning,
     "%0: unsupported option",
     "%0: unsupported option")
DIAG(warn_shared_textrel,
     DiagnosticEngine::Warning,
     "Add DT_TEXTREL in a shared object!",
     "Add DT_TEXTREL in a shared object.")
DIAG(fatal_illegal_codegen_type,
     DiagnosticEngine::Fatal,
     "illegal output format of output %0",
     "illegal output format of output %0")
DIAG(err_nmagic_not_static,
     DiagnosticEngine::Error,
     "cannot mix -nmagic option with -shared",
     "cannot mix -nmagic option with -shared")
DIAG(err_omagic_not_static,
     DiagnosticEngine::Error,
     "cannot mix -omagic option with -shared",
     "cannot mix -omagic option with -shared")
DIAG(err_invalid_emulation,
     DiagnosticEngine::Error,
     "Invalid target emulation: `%0'.",
     "Invalid target emulation: `%0'.")
DIAG(err_cannot_find_scriptfile,
     DiagnosticEngine::Fatal,
     "cannot open %0 file %1",
     "cannot open %0 file %1")
DIAG(err_unsupported_archive,
     DiagnosticEngine::Error,
     "Unsupported archive type.",
     "Unsupported archive type.")
DIAG(unexpected_frag_type,
     DiagnosticEngine::Unreachable,
     "Unexpected fragment type `%0' when constructing FG",
     "Unexpected fragment type `%0' when constructing FG")
DIAG(debug_print_gc_sections,
     DiagnosticEngine::Debug,
     "removing unused section from '%0' in file '%1'",
     "removing unused section from '%0' in file '%1'")
DIAG(eh_missing_text_section,
     DiagnosticEngine::Fatal,
     "missing text section for '%0' in file '%1'",
     "missing text section for '%0' in file '%1'")
