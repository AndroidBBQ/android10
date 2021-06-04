/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This is a helper file to apply macros to different cblas routines.
// Will be include multiple times.

#if !defined(RS_APPLY_MACRO_TO)
#error "You must define the macro RS_APPLY_MACRO_TO to include this file"
#endif

RS_APPLY_MACRO_TO(cblas_sgemv)
RS_APPLY_MACRO_TO(cblas_sgbmv)
RS_APPLY_MACRO_TO(cblas_strmv)
RS_APPLY_MACRO_TO(cblas_stbmv)
RS_APPLY_MACRO_TO(cblas_stpmv)
RS_APPLY_MACRO_TO(cblas_strsv)
RS_APPLY_MACRO_TO(cblas_stbsv)
RS_APPLY_MACRO_TO(cblas_stpsv)

RS_APPLY_MACRO_TO(cblas_dgemv)
RS_APPLY_MACRO_TO(cblas_dgbmv)
RS_APPLY_MACRO_TO(cblas_dtrmv)
RS_APPLY_MACRO_TO(cblas_dtbmv)
RS_APPLY_MACRO_TO(cblas_dtpmv)
RS_APPLY_MACRO_TO(cblas_dtrsv)
RS_APPLY_MACRO_TO(cblas_dtbsv)
RS_APPLY_MACRO_TO(cblas_dtpsv)

RS_APPLY_MACRO_TO(cblas_cgemv)
RS_APPLY_MACRO_TO(cblas_cgbmv)
RS_APPLY_MACRO_TO(cblas_ctrmv)
RS_APPLY_MACRO_TO(cblas_ctbmv)
RS_APPLY_MACRO_TO(cblas_ctpmv)
RS_APPLY_MACRO_TO(cblas_ctrsv)
RS_APPLY_MACRO_TO(cblas_ctbsv)
RS_APPLY_MACRO_TO(cblas_ctpsv)

RS_APPLY_MACRO_TO(cblas_zgemv)
RS_APPLY_MACRO_TO(cblas_zgbmv)
RS_APPLY_MACRO_TO(cblas_ztrmv)
RS_APPLY_MACRO_TO(cblas_ztbmv)
RS_APPLY_MACRO_TO(cblas_ztpmv)
RS_APPLY_MACRO_TO(cblas_ztrsv)
RS_APPLY_MACRO_TO(cblas_ztbsv)
RS_APPLY_MACRO_TO(cblas_ztpsv)

RS_APPLY_MACRO_TO(cblas_ssymv)
RS_APPLY_MACRO_TO(cblas_ssbmv)
RS_APPLY_MACRO_TO(cblas_sspmv)
RS_APPLY_MACRO_TO(cblas_sger)
RS_APPLY_MACRO_TO(cblas_ssyr)
RS_APPLY_MACRO_TO(cblas_sspr)
RS_APPLY_MACRO_TO(cblas_ssyr2)
RS_APPLY_MACRO_TO(cblas_sspr2)

RS_APPLY_MACRO_TO(cblas_dsymv)
RS_APPLY_MACRO_TO(cblas_dsbmv)
RS_APPLY_MACRO_TO(cblas_dspmv)
RS_APPLY_MACRO_TO(cblas_dger)
RS_APPLY_MACRO_TO(cblas_dsyr)
RS_APPLY_MACRO_TO(cblas_dspr)
RS_APPLY_MACRO_TO(cblas_dsyr2)
RS_APPLY_MACRO_TO(cblas_dspr2)

RS_APPLY_MACRO_TO(cblas_chemv)
RS_APPLY_MACRO_TO(cblas_chbmv)
RS_APPLY_MACRO_TO(cblas_chpmv)
RS_APPLY_MACRO_TO(cblas_cgeru)
RS_APPLY_MACRO_TO(cblas_cgerc)
RS_APPLY_MACRO_TO(cblas_cher)
RS_APPLY_MACRO_TO(cblas_chpr)
RS_APPLY_MACRO_TO(cblas_cher2)
RS_APPLY_MACRO_TO(cblas_chpr2)

RS_APPLY_MACRO_TO(cblas_zhemv)
RS_APPLY_MACRO_TO(cblas_zhbmv)
RS_APPLY_MACRO_TO(cblas_zhpmv)
RS_APPLY_MACRO_TO(cblas_zgeru)
RS_APPLY_MACRO_TO(cblas_zgerc)
RS_APPLY_MACRO_TO(cblas_zher)
RS_APPLY_MACRO_TO(cblas_zhpr)
RS_APPLY_MACRO_TO(cblas_zher2)
RS_APPLY_MACRO_TO(cblas_zhpr2)


RS_APPLY_MACRO_TO(cblas_sgemm)
RS_APPLY_MACRO_TO(cblas_ssymm)
RS_APPLY_MACRO_TO(cblas_ssyrk)
RS_APPLY_MACRO_TO(cblas_ssyr2k)
RS_APPLY_MACRO_TO(cblas_strmm)
RS_APPLY_MACRO_TO(cblas_strsm)

RS_APPLY_MACRO_TO(cblas_dgemm)
RS_APPLY_MACRO_TO(cblas_dsymm)
RS_APPLY_MACRO_TO(cblas_dsyrk)
RS_APPLY_MACRO_TO(cblas_dsyr2k)
RS_APPLY_MACRO_TO(cblas_dtrmm)
RS_APPLY_MACRO_TO(cblas_dtrsm)

RS_APPLY_MACRO_TO(cblas_cgemm)
RS_APPLY_MACRO_TO(cblas_csymm)
RS_APPLY_MACRO_TO(cblas_csyrk)
RS_APPLY_MACRO_TO(cblas_csyr2k)
RS_APPLY_MACRO_TO(cblas_ctrmm)
RS_APPLY_MACRO_TO(cblas_ctrsm)

RS_APPLY_MACRO_TO(cblas_zgemm)
RS_APPLY_MACRO_TO(cblas_zsymm)
RS_APPLY_MACRO_TO(cblas_zsyrk)
RS_APPLY_MACRO_TO(cblas_zsyr2k)
RS_APPLY_MACRO_TO(cblas_ztrmm)
RS_APPLY_MACRO_TO(cblas_ztrsm)

RS_APPLY_MACRO_TO(cblas_chemm)
RS_APPLY_MACRO_TO(cblas_cherk)
RS_APPLY_MACRO_TO(cblas_cher2k)

RS_APPLY_MACRO_TO(cblas_zhemm)
RS_APPLY_MACRO_TO(cblas_zherk)
RS_APPLY_MACRO_TO(cblas_zher2k)

// Undefine the macro so that we can include this file multiple times to generate different functionality.
#undef RS_APPLY_MACRO_TO
