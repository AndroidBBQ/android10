#ifndef RS_COMPATIBILITY_LIB
#include "cblas.h"
#else
#include <dlfcn.h>
/*
 * The following enums are based on cblas.h
 */
enum CBLAS_ORDER {CblasRowMajor=101, CblasColMajor=102};
enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
enum CBLAS_UPLO {CblasUpper=121, CblasLower=122};
enum CBLAS_DIAG {CblasNonUnit=131, CblasUnit=132};
enum CBLAS_SIDE {CblasLeft=141, CblasRight=142};
#endif

/*
 * ===========================================================================
 * Prototypes for level 2 BLAS
 * ===========================================================================
 */

/*
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
typedef void (*FnPtr_cblas_sgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  const float *X, const int incX, const float beta,
                                  float *Y, const int incY);
typedef void (*FnPtr_cblas_sgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const float alpha,
                                  const float *A, const int lda, const float *X,
                                  const int incX, const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_strmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *A, const int lda,
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const float *A, const int lda,
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *Ap, float *X, const int incX);
typedef void (*FnPtr_cblas_strsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *A, const int lda, float *X,
                                  const int incX);
typedef void (*FnPtr_cblas_stbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const float *A, const int lda,
                                  float *X, const int incX);
typedef void (*FnPtr_cblas_stpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const float *Ap, float *X, const int incX);

typedef void (*FnPtr_cblas_dgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  const double *X, const int incX, const double beta,
                                  double *Y, const int incY);
typedef void (*FnPtr_cblas_dgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const double alpha,
                                  const double *A, const int lda, const double *X,
                                  const int incX, const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dtrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *A, const int lda,
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const double *A, const int lda,
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *Ap, double *X, const int incX);
typedef void (*FnPtr_cblas_dtrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *A, const int lda, double *X,
                                  const int incX);
typedef void (*FnPtr_cblas_dtbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const double *A, const int lda,
                                  double *X, const int incX);
typedef void (*FnPtr_cblas_dtpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const double *Ap, double *X, const int incX);

typedef void (*FnPtr_cblas_cgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *X, const int incX, const void *beta,
                                  void *Y, const int incY);
typedef void (*FnPtr_cblas_cgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const void *alpha,
                                  const void *A, const int lda, const void *X,
                                  const int incX, const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_ctrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);
typedef void (*FnPtr_cblas_ctrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, void *X,
                                  const int incX);
typedef void (*FnPtr_cblas_ctbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ctpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);

typedef void (*FnPtr_cblas_zgemv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *X, const int incX, const void *beta,
                                  void *Y, const int incY);
typedef void (*FnPtr_cblas_zgbmv)(const enum CBLAS_ORDER order,
                                  const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                                  const int KL, const int KU, const void *alpha,
                                  const void *A, const int lda, const void *X,
                                  const int incX, const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_ztrmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);
typedef void (*FnPtr_cblas_ztrsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *A, const int lda, void *X,
                                  const int incX);
typedef void (*FnPtr_cblas_ztbsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const int K, const void *A, const int lda,
                                  void *X, const int incX);
typedef void (*FnPtr_cblas_ztpsv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
                                  const int N, const void *Ap, void *X, const int incX);


/*
 * Routines with S and D prefixes only
 */
typedef void (*FnPtr_cblas_ssymv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *A,
                                  const int lda, const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_ssbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const float alpha, const float *A,
                                  const int lda, const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_sspmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *Ap,
                                  const float *X, const int incX,
                                  const float beta, float *Y, const int incY);
typedef void (*FnPtr_cblas_sger)(const enum CBLAS_ORDER order, const int M, const int N,
                                 const float alpha, const float *X, const int incX,
                                 const float *Y, const int incY, float *A, const int lda);
typedef void (*FnPtr_cblas_ssyr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const float *X,
                                 const int incX, float *A, const int lda);
typedef void (*FnPtr_cblas_sspr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const float *X,
                                 const int incX, float *Ap);
typedef void (*FnPtr_cblas_ssyr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *X,
                                  const int incX, const float *Y, const int incY, float *A,
                                  const int lda);
typedef void (*FnPtr_cblas_sspr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const float alpha, const float *X,
                                  const int incX, const float *Y, const int incY, float *A);

typedef void (*FnPtr_cblas_dsymv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *A,
                                  const int lda, const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dsbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const double alpha, const double *A,
                                  const int lda, const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dspmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *Ap,
                                  const double *X, const int incX,
                                  const double beta, double *Y, const int incY);
typedef void (*FnPtr_cblas_dger)(const enum CBLAS_ORDER order, const int M, const int N,
                                 const double alpha, const double *X, const int incX,
                                 const double *Y, const int incY, double *A, const int lda);
typedef void (*FnPtr_cblas_dsyr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const double *X,
                                 const int incX, double *A, const int lda);
typedef void (*FnPtr_cblas_dspr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const double *X,
                                 const int incX, double *Ap);
typedef void (*FnPtr_cblas_dsyr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *X,
                                  const int incX, const double *Y, const int incY, double *A,
                                  const int lda);
typedef void (*FnPtr_cblas_dspr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const double alpha, const double *X,
                                  const int incX, const double *Y, const int incY, double *A);


/*
 * Routines with C and Z prefixes only
 */
typedef void (*FnPtr_cblas_chemv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_chbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_chpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *Ap,
                                  const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_cgeru)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_cgerc)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_cher)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const void *X, const int incX,
                                 void *A, const int lda);
typedef void (*FnPtr_cblas_chpr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const float alpha, const void *X,
                                 const int incX, void *A);
typedef void (*FnPtr_cblas_cher2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_chpr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *Ap);

typedef void (*FnPtr_cblas_zhemv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zhbmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const int K, const void *alpha, const void *A,
                                  const int lda, const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zhpmv)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                  const int N, const void *alpha, const void *Ap,
                                  const void *X, const int incX,
                                  const void *beta, void *Y, const int incY);
typedef void (*FnPtr_cblas_zgeru)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zgerc)(const enum CBLAS_ORDER order, const int M, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zher)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const void *X, const int incX,
                                 void *A, const int lda);
typedef void (*FnPtr_cblas_zhpr)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                                 const int N, const double alpha, const void *X,
                                 const int incX, void *A);
typedef void (*FnPtr_cblas_zher2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *A, const int lda);
typedef void (*FnPtr_cblas_zhpr2)(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo, const int N,
                                  const void *alpha, const void *X, const int incX,
                                  const void *Y, const int incY, void *Ap);

/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */

/*
 * Routines with standard 4 prefixes (S, D, C, Z)
 */
typedef void (*FnPtr_cblas_sgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const float alpha, const float *A,
                                  const int lda, const float *B, const int ldb,
                                  const float beta, float *C, const int ldc);
typedef void (*FnPtr_cblas_ssymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  const float *B, const int ldb, const float beta,
                                  float *C, const int ldc);
typedef void (*FnPtr_cblas_ssyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const float alpha, const float *A, const int lda,
                                  const float beta, float *C, const int ldc);
typedef void (*FnPtr_cblas_ssyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const float alpha, const float *A, const int lda,
                                   const float *B, const int ldb, const float beta,
                                   float *C, const int ldc);
typedef void (*FnPtr_cblas_strmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  float *B, const int ldb);
typedef void (*FnPtr_cblas_strsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const float alpha, const float *A, const int lda,
                                  float *B, const int ldb);

typedef void (*FnPtr_cblas_dgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const double alpha, const double *A,
                                  const int lda, const double *B, const int ldb,
                                  const double beta, double *C, const int ldc);
typedef void (*FnPtr_cblas_dsymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  const double *B, const int ldb, const double beta,
                                  double *C, const int ldc);
typedef void (*FnPtr_cblas_dsyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const double alpha, const double *A, const int lda,
                                  const double beta, double *C, const int ldc);
typedef void (*FnPtr_cblas_dsyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const double alpha, const double *A, const int lda,
                                   const double *B, const int ldb, const double beta,
                                   double *C, const int ldc);
typedef void (*FnPtr_cblas_dtrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  double *B, const int ldb);
typedef void (*FnPtr_cblas_dtrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const double alpha, const double *A, const int lda,
                                  double *B, const int ldb);

typedef void (*FnPtr_cblas_cgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const void *alpha, const void *A,
                                  const int lda, const void *B, const int ldb,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_csymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_csyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const void *alpha, const void *A, const int lda,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_csyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const void *beta,
                                   void *C, const int ldc);
typedef void (*FnPtr_cblas_ctrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);
typedef void (*FnPtr_cblas_ctrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);

typedef void (*FnPtr_cblas_zgemm)(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                                  const int K, const void *alpha, const void *A,
                                  const int lda, const void *B, const int ldb,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zsymm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_zsyrk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const void *alpha, const void *A, const int lda,
                                  const void *beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zsyr2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const void *beta,
                                   void *C, const int ldc);
typedef void (*FnPtr_cblas_ztrmm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);
typedef void (*FnPtr_cblas_ztrsm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                                  const enum CBLAS_DIAG Diag, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  void *B, const int ldb);


/*
 * Routines with prefixes C and Z only
 */
typedef void (*FnPtr_cblas_chemm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_cherk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const float alpha, const void *A, const int lda,
                                  const float beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_cher2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const float beta,
                                   void *C, const int ldc);

typedef void (*FnPtr_cblas_zhemm)(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                                  const enum CBLAS_UPLO Uplo, const int M, const int N,
                                  const void *alpha, const void *A, const int lda,
                                  const void *B, const int ldb, const void *beta,
                                  void *C, const int ldc);
typedef void (*FnPtr_cblas_zherk)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                  const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                  const double alpha, const void *A, const int lda,
                                  const double beta, void *C, const int ldc);
typedef void (*FnPtr_cblas_zher2k)(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                                   const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
                                   const void *alpha, const void *A, const int lda,
                                   const void *B, const int ldb, const double beta,
                                   void *C, const int ldc);


#ifdef RS_COMPATIBILITY_LIB
// Macros to help declare our function pointers for the dispatch table.
#define RS_APPLY_MACRO_TO(x) \
    FnPtr_##x x;
#include "rsCpuBLAS.inc"

bool loadBLASLib() {
    void* handle = NULL;
    handle = dlopen("libblasV8.so", RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        return false;
    }

// Macros to help load the function pointers.
#define RS_APPLY_MACRO_TO(x) \
    x = (FnPtr_##x)dlsym(handle, #x); \
    if ((x) == nullptr) { \
        ALOGE("Failed to load " #x " for RS BLAS implementation."); \
        return false; \
    }
#include "rsCpuBLAS.inc"
    return true;
}
#endif
