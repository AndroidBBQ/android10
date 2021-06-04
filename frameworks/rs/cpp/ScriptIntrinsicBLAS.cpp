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


#include "RenderScript.h"
#include "rsCppInternal.h"

#define NELEM(m) (sizeof(m) / sizeof((m)[0]))

using android::RSC::Allocation;
using android::RSC::Element;
using android::RSC::RS;
using android::RSC::RS_ERROR_INVALID_ELEMENT;
using android::RSC::RS_ERROR_INVALID_PARAMETER;
using android::RSC::RS_SUCCESS;
using android::RSC::ScriptIntrinsicBLAS;
using android::RSC::sp;

// ScriptIntrinsicBLAS APIS
ScriptIntrinsicBLAS::ScriptIntrinsicBLAS(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_BLAS, e) {

}

sp<ScriptIntrinsicBLAS> ScriptIntrinsicBLAS::create(const sp<RS>& rs) {
    return new ScriptIntrinsicBLAS(rs, Element::U32(rs));
}

enum RsBlasDataType {
    SINGLE,
    DOUBLE,
    SINGLE_COMPLEX,
    DOUBLE_COMPLEX
};

static RsBlasCall
setUpBLASCall(RsBlasDataType dataType, RsBlasFunction func,
              int TransA, int TransB, int Side, int Uplo, int Diag,
              int M, int N, int K, int incX, int incY, int KL, int KU,
              float alphaF, float betaF, double alphaD, double betaD,
              float alphaCX, float alphaCY, float betaCX, float betaCY,
              double alphaZX, double alphaZY, double betaZX, double betaZY
              ) {
    RsBlasCall call;
    memset(&call, 0, sizeof(call));
    call.func = func;
    call.transA = (RsBlasTranspose)TransA;
    call.transB = (RsBlasTranspose)TransB;
    call.side = (RsBlasSide)Side;
    call.uplo = (RsBlasUplo)Uplo;
    call.diag = (RsBlasDiag)Diag;
    call.M = M;
    call.N = N;
    call.K = K;

    switch (dataType) {
        case SINGLE:
            // For Single-precision BLAS.
            call.alpha.f = alphaF;
            call.beta.f = betaF;
            break;
        case DOUBLE:
            // For Double-precision BLAS.
            call.alpha.d = alphaD;
            call.beta.d = betaD;
            break;
        case SINGLE_COMPLEX:
            // For Single-precision complex BLAS.
            call.alpha.c.r = alphaCX;
            call.alpha.c.i = alphaCY;
            call.beta.c.r = betaCX;
            call.beta.c.i = betaCY;
            break;
        case DOUBLE_COMPLEX:
            // For Double-precision complex BLAS.
            call.alpha.z.r = alphaZX;
            call.alpha.z.i = alphaZY;
            call.beta.z.r = betaZX;
            call.beta.z.i = betaZY;
            break;
        default:
            break;
    }

    call.incX = incX;
    call.incY = incY;
    call.KL = KL;
    call.KU = KU;

    return call;
}

static void
nScriptIntrinsicBLAS_Single(RS* mRS, RsContext con, RsScript id, RsBlasFunction func, int TransA,
                            int TransB, int Side, int Uplo, int Diag, int M, int N, int K,
                            float alpha, RsAllocation A, RsAllocation B,
                            float beta, RsAllocation C, int incX, int incY, int KL, int KU) {
    RsBlasCall call = setUpBLASCall(SINGLE, func, TransA, TransB, Side, Uplo, Diag,
                                    M, N, K, incX, incY, KL, KU, alpha, beta, 0.0, 0.0,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0, 0.0, 0.0, 0.0);
    RsAllocation in_allocs[3] = {A, B, C};
    tryDispatch(mRS, RS::dispatch->ScriptForEachMulti(con, id, 0, in_allocs, NELEM(in_allocs), nullptr,
                                                      &call, sizeof(call), nullptr, 0));
}


static void
nScriptIntrinsicBLAS_Double(RS* mRS, RsContext con, RsScript id, RsBlasFunction func, int TransA,
                            int TransB, int Side, int Uplo, int Diag, int M, int N, int K,
                            double alpha, RsAllocation A, RsAllocation B,
                            double beta, RsAllocation C, int incX, int incY, int KL, int KU) {
    RsBlasCall call = setUpBLASCall(DOUBLE, func, TransA, TransB, Side, Uplo, Diag,
                                    M, N, K, incX, incY, KL, KU, 0.0f, 0.0f, alpha, beta,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0, 0.0, 0.0, 0.0);
    RsAllocation in_allocs[3] = {A, B, C};
    tryDispatch(mRS, RS::dispatch->ScriptForEachMulti(con, id, 0, in_allocs, NELEM(in_allocs), nullptr,
                                                      &call, sizeof(call), nullptr, 0));
}

static void
nScriptIntrinsicBLAS_Complex(RS* mRS, RsContext con, RsScript id, RsBlasFunction func, int TransA,
                             int TransB, int Side, int Uplo, int Diag, int M, int N, int K,
                             float alphaX, float alphaY, RsAllocation A, RsAllocation B,
                             float betaX, float betaY, RsAllocation C, int incX, int incY, int KL, int KU) {
    RsBlasCall call = setUpBLASCall(SINGLE_COMPLEX, func, TransA, TransB, Side, Uplo, Diag,
                                    M, N, K, incX, incY, KL, KU, 0.0f, 0.0f, 0.0, 0.0,
                                    alphaX, alphaY, betaX, betaY, 0.0, 0.0, 0.0, 0.0);
    RsAllocation in_allocs[3] = {A, B, C};
    tryDispatch(mRS, RS::dispatch->ScriptForEachMulti(con, id, 0, in_allocs, NELEM(in_allocs), nullptr,
                                                      &call, sizeof(call), nullptr, 0));
}

static void
nScriptIntrinsicBLAS_Z(RS* mRS, RsContext con, RsScript id, RsBlasFunction func, int TransA,
                       int TransB, int Side, int Uplo, int Diag, int M, int N, int K,
                       double alphaX, double alphaY, RsAllocation A, RsAllocation B,
                       double betaX, double betaY, RsAllocation C, int incX, int incY, int KL, int KU) {
    RsBlasCall call = setUpBLASCall(DOUBLE_COMPLEX, func, TransA, TransB, Side, Uplo, Diag,
                                    M, N, K, incX, incY, KL, KU, 0.0f, 0.0f, 0.0, 0.0,
                                    0.0f, 0.0f, 0.0f, 0.0f, alphaX, alphaY, betaX, betaY);
    RsAllocation in_allocs[3] = {A, B, C};
    tryDispatch(mRS, RS::dispatch->ScriptForEachMulti(con, id, 0, in_allocs, NELEM(in_allocs), nullptr,
                                                      &call, sizeof(call), nullptr, 0));
}


static void
nScriptIntrinsicBLAS_BNNM(RS* mRS, RsContext con, RsScript id, int M, int N, int K,
                          RsAllocation A, int a_offset, RsAllocation B, int b_offset,
                          RsAllocation C, int c_offset, int c_mult_int) {
    RsBlasCall call;
    memset(&call, 0, sizeof(call));
    call.func = RsBlas_bnnm;
    call.M = M;
    call.N = N;
    call.K = K;
    call.a_offset = a_offset & 0xFF;
    call.b_offset = b_offset & 0xFF;
    call.c_offset = c_offset;
    call.c_mult_int = c_mult_int;

    RsAllocation in_allocs[3] = {A, B, C};
    tryDispatch(mRS, RS::dispatch->ScriptForEachMulti(con, id, 0, in_allocs, NELEM(in_allocs), nullptr,
                                                      &call, sizeof(call), nullptr, 0));
}

/**
 * Level 2 BLAS
 */
static void validateGEMV(RS* mRS, const sp<const Element>& e, RsBlasTranspose TransA, const sp<Allocation>& A,
                         const sp<Allocation>& X, int incX, const sp<Allocation>& Y, int incY) {
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = -1, expectedYDim = -1;
    if (TransA == RsBlasNoTrans) {
        expectedXDim = 1 + (N - 1) * incX;
        expectedYDim = 1 + (M - 1) * incY;
    } else {
        expectedXDim = 1 + (M - 1) * incX;
        expectedYDim = 1 + (N - 1) * incY;
    }
    if ((int)X->getType()->getX() != expectedXDim ||
        (int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for GEMV");
    }
}

void ScriptIntrinsicBLAS::SGEMV(RsBlasTranspose TransA, float alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, float beta, const sp<Allocation>& Y, int incY) {
    validateGEMV(mRS, Element::F32(mRS), TransA, A, X, incX, Y, incY);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sgemv,
                                TransA, 0, 0, 0, 0, M, N, 0,
                                alpha, A->getID(), X->getID(),
                                beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DGEMV(RsBlasTranspose TransA, double alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, double beta, const sp<Allocation>& Y, int incY) {
    validateGEMV(mRS, Element::F64(mRS), TransA, A, X, incX, Y, incY);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dgemv,
                                TransA, 0, 0, 0, 0, M, N, 0,
                                alpha, A->getID(), X->getID(),
                                beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CGEMV(RsBlasTranspose TransA, Float2 alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, Float2 beta, const sp<Allocation>& Y, int incY) {
    validateGEMV(mRS, Element::F32_2(mRS), TransA, A, X, incX, Y, incY);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cgemv,
                                 TransA, 0, 0, 0, 0, M, N, 0,
                                 alpha.x, alpha.y, A->getID(), X->getID(),
                                 beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZGEMV(RsBlasTranspose TransA, Double2 alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, Double2 beta, const sp<Allocation>& Y, int incY) {
    validateGEMV(mRS, Element::F64_2(mRS), TransA, A, X, incX, Y, incY);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zgemv,
                           TransA, 0, 0, 0, 0, M, N, 0,
                           alpha.x, alpha.y, A->getID(), X->getID(),
                           beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SGBMV(RsBlasTranspose TransA, int KL, int KU, float alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, float beta, const sp<Allocation>& Y, int incY) {
    // GBMV has the same validation requirements as GEMV + KL and KU >= 0
    validateGEMV(mRS, Element::F32(mRS), TransA, A, X, incX, Y, incY);
    if (KL < 0 || KU < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "KL and KU must be greater than or equal to 0");
    }
    int M = A->getType()->getY();
    int N = A->getType()->getX();

    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sgbmv,
                                TransA, 0, 0, 0, 0, M, N, 0,
                                alpha, A->getID(), X->getID(),
                                beta, Y->getID(), incX, incY, KL, KU);
}

void ScriptIntrinsicBLAS::DGBMV(RsBlasTranspose TransA, int KL, int KU, double alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, double beta, const sp<Allocation>& Y, int incY) {
    // GBMV has the same validation requirements as GEMV + KL and KU >= 0
    validateGEMV(mRS, Element::F64(mRS), TransA, A, X, incX, Y, incY);
    if (KL < 0 || KU < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "KL and KU must be greater than or equal to 0");
    }
    int M = A->getType()->getY();
    int N = A->getType()->getX();

    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dgbmv,
                                TransA, 0, 0, 0, 0, M, N, 0,
                                alpha, A->getID(), X->getID(),
                                beta, Y->getID(), incX, incY, KL, KU);
}

void ScriptIntrinsicBLAS::CGBMV(RsBlasTranspose TransA, int KL, int KU, Float2 alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, Float2 beta, const sp<Allocation>& Y, int incY) {
    // GBMV has the same validation requirements as GEMV + KL and KU >= 0
    validateGEMV(mRS, Element::F32_2(mRS), TransA, A, X, incX, Y, incY);
    if (KL < 0 || KU < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "KL and KU must be greater than or equal to 0");
    }
    int M = A->getType()->getY();
    int N = A->getType()->getX();

    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cgbmv,
                                 TransA, 0, 0, 0, 0, M, N, 0,
                                 alpha.x, alpha.y, A->getID(), X->getID(),
                                 beta.x, beta.y, Y->getID(), incX, incY, KL, KU);
}

void ScriptIntrinsicBLAS::ZGBMV(RsBlasTranspose TransA, int KL, int KU, Double2 alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, Double2 beta, const sp<Allocation>& Y, int incY) {
    // GBMV has the same validation requirements as GEMV + KL and KU >= 0
    validateGEMV(mRS, Element::F64_2(mRS), TransA, A, X, incX, Y, incY);
    if (KL < 0 || KU < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "KL and KU must be greater than or equal to 0");
    }
    int M = A->getType()->getY();
    int N = A->getType()->getX();

    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zgbmv,
                           TransA, 0, 0, 0, 0, M, N, 0,
                           alpha.x, alpha.y, A->getID(), X->getID(),
                           beta.x, beta.y, Y->getID(), incX, incY, KL, KU);
}

static void validateTRMV(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo, RsBlasTranspose TransA,
                         RsBlasDiag Diag, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    int N = A->getType()->getY();
    if ((int)A->getType()->getX() != N) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "A must be a square matrix for TRMV");
    }
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (incX <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for TRMV");
    }
}

static int validateTPMV(RS* mRS, const sp<const Element>& e,  RsBlasUplo Uplo, RsBlasTranspose TransA,
                        RsBlasDiag Diag, const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    if (!Ap->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (Ap->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Ap must have a Y dimension of 0 or 1");
    }

    int N = sqrt((double)Ap->getType()->getX() * 2);
    if ((int)Ap->getType()->getX() != ((N * (N+1)) / 2)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid dimension for Ap");
    }
    if (incX <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for TPMV");
    }

    return N;
}


void ScriptIntrinsicBLAS::STRMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    validateTRMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_strmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTRMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    validateTRMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtrmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTRMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    validateTRMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctrmv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                                 A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTRMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    validateTRMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztrmv,
                           TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                           A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::STBMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBMV has the same requirements as TRMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    validateTRMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_stbmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, K, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTBMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBMV has the same requirements as TRMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    validateTRMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtbmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, K, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTBMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBMV has the same requirements as TRMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    validateTRMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctbmv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, K, 0, 0,
                                 A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTBMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBMV has the same requirements as TRMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    validateTRMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztbmv,
                           TransA, 0, 0, Uplo, Diag, 0, N, K, 0, 0,
                           A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::STPMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    int N = validateTPMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_stpmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                Ap->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTPMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    int N = validateTPMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtpmv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                Ap->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTPMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap,  const sp<Allocation>& X,  int incX) {
    int N = validateTPMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctpmv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                                 Ap->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTPMV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    int N = validateTPMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztpmv,
                           TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                           Ap->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::STRSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TRSV is the same as TRMV
    validateTRMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_strsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTRSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A,  const sp<Allocation>& X,  int incX) {
    // TRSV is the same as TRMV
    validateTRMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtrsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);

}

void ScriptIntrinsicBLAS::CTRSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TRSV is the same as TRMV
    validateTRMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctrsv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                                 A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);

}

void ScriptIntrinsicBLAS::ZTRSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TRSV is the same as TRMV
    validateTRMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztrsv,
                           TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                           A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);

}

void ScriptIntrinsicBLAS::STBSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBSV is the same as TRMV + K >= 0
    validateTRMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Number of diagonals must be positive");
    }
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_stbsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, K, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTBSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBSV is the same as TRMV + K >= 0
    validateTRMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Number of diagonals must be positive");
    }
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtbsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, K, 0,
                                A->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTBSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBSV is the same as TRMV + K >= 0
    validateTRMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Number of diagonals must be positive");
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctbsv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, K,
                                 0, 0, A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTBSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                int K, const sp<Allocation>& A, const sp<Allocation>& X, int incX) {
    // TBSV is the same as TRMV + K >= 0
    validateTRMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, A, X, incX);
    int N = A->getType()->getY();
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Number of diagonals must be positive");
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztbsv,
                           TransA, 0, 0, Uplo, Diag, 0, N, K, 0, 0,
                           A->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::STPSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    // TPSV is same as TPMV
    int N = validateTPMV(mRS, Element::F32(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_stpsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                Ap->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTPSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    // TPSV is same as TPMV
    int N = validateTPMV(mRS, Element::F64(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtpsv,
                                TransA, 0, 0, Uplo, Diag, 0, N, 0, 0,
                                Ap->getID(), X->getID(), 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTPSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    // TPSV is same as TPMV
    int N = validateTPMV(mRS, Element::F32_2(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctpsv,
                                 TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                                 Ap->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTPSV(RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                const sp<Allocation>& Ap, const sp<Allocation>& X, int incX) {
    // TPSV is same as TPMV
    int N = validateTPMV(mRS, Element::F64_2(mRS), Uplo, TransA, Diag, Ap, X, incX);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztpsv,
                           TransA, 0, 0, Uplo, Diag, 0, N, 0, 0, 0,
                           Ap->getID(), X->getID(), 0, 0, 0, incX, 0, 0, 0);
}

/**
 * Level 2, S and D only
 */
static int validateSYMV(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo, const sp<Allocation>& A,
                        const sp<Allocation>& X, const sp<Allocation>& Y, int incX, int incY) {
    int N = A->getType()->getY();
    if ((int)A->getType()->getX() != N) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "A must be a square matrix for SYMV");
    }
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e) ) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SYMV");
    }
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SYMV");
    }
    return N;
}
static int validateSPMV(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo, const sp<Allocation>& Ap,
                        const sp<Allocation>& X, int incX, const sp<Allocation>& Y, int incY) {
    if (!Ap->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (Ap->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Ap must have a Y dimension of 0 or 1");
    }

    int N = sqrt((double)Ap->getType()->getX() * 2);
    if ((int)Ap->getType()->getX() != ((N * (N+1)) / 2)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid dimension for Ap");
    }
    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SPMV");
    }
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SPMV");
    }

    return N;
}
static void validateGER(RS* mRS, const sp<const Element>& e, const sp<Allocation>& X, int incX,
                        const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e) ) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }

    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    int M = A->getType()->getY();
    int N = A->getType()->getX();

    if (N < 1 || M < 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "M and N must be 1 or greater for GER");
    }
    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (M - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for GER");
    }
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for GER");
    }


}
static int validateSYR(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo,
                       const sp<Allocation>& X, int incX, const sp<Allocation>& A) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }

    int N = A->getType()->getX();

    if (X->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }
    if (N != (int)A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "A must be a symmetric matrix");
    }
    if (incX <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SYR");
    }
    return N;
}
static int validateSPR(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo,
                       const sp<Allocation>& X, int incX, const sp<Allocation>& Ap) {
    if (!Ap->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (Ap->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Ap must have a Y dimension of 0 or 1");
    }

    int N = sqrt((double)Ap->getType()->getX() * 2);
    if ((int)Ap->getType()->getX() != ((N * (N+1)) / 2)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid dimension for Ap");
    }
    if (incX <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SPR");
    }

    return N;
}

static int validateSYR2(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo, const sp<Allocation>& X,
                        int incX, const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }

    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    int N = A->getType()->getX();

    if (N != (int)A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "A must be a symmetric matrix");
    }
    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)X->getType()->getX() != expectedXDim || (int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SYR");
    }
    return N;

}
static int validateSPR2(RS* mRS, const sp<const Element>& e, RsBlasUplo Uplo, const sp<Allocation>& X,
                        int incX, const sp<Allocation>& Y, int incY, const sp<Allocation>& Ap) {
    if (!Ap->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    if (Ap->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Ap must have a Y dimension of 0 or 1");
    }

    int N = sqrt((double)Ap->getType()->getX() * 2);
    if ((int)Ap->getType()->getX() != ((N * (N+1)) / 2)) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid dimension for Ap");
    }
    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (N - 1) * incX;
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)X->getType()->getX() != expectedXDim || (int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for SPR2");
    }

    return N;
}

void ScriptIntrinsicBLAS::SSYMV(RsBlasUplo Uplo, float alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, float beta, const sp<Allocation>& Y, int incY) {
    int N = validateSYMV(mRS, Element::F32(mRS), Uplo, A, X, Y, incX, incY);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssymv,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                A->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SSBMV(RsBlasUplo Uplo, int K, float alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, float beta, const sp<Allocation>& Y, int incY) {
    // SBMV is the same as SYMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    int N = validateSYMV(mRS, Element::F32(mRS), Uplo, A, X, Y, incX, incY);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssbmv,
                                0, 0, 0, Uplo, 0, 0, N, K, alpha,
                                A->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SSPMV(RsBlasUplo Uplo, float alpha, const sp<Allocation>& Ap, const sp<Allocation>& X,
                                int incX, float beta, const sp<Allocation>& Y, int incY) {
    int N = validateSPMV(mRS, Element::F32(mRS), Uplo, Ap, X, incX, Y, incY);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sspmv,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                Ap->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SGER(float alpha, const sp<Allocation>& X, int incX,
                               const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    validateGER(mRS, Element::F32(mRS), X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sger,
                                0, 0, 0, 0, 0, M, N, 0, alpha,
                                X->getID(), Y->getID(), 0.f, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SSYR(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& A) {
    int N = validateSYR(mRS, Element::F32(mRS), Uplo, X, incX, A);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssyr,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), A->getID(), 0.f, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::SSPR(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& Ap) {
    int N = validateSPR(mRS, Element::F32(mRS), Uplo, X, incX, Ap);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sspr,
                                0, 0, 0, Uplo, 0, 0, N, 0,
                                alpha, X->getID(), Ap->getID(), 0.f, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::SSYR2(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    int N = validateSYR2(mRS, Element::F32(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssyr2,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), Y->getID(), 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::SSPR2(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& Ap) {
    int N = validateSPR2(mRS, Element::F32(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sspr2,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), Y->getID(), 0, Ap->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DSYMV(RsBlasUplo Uplo, double alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, double beta, const sp<Allocation>& Y, int incY) {
    int N = validateSYMV(mRS, Element::F64(mRS), Uplo, A, X, Y, incX, incY);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsymv,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                A->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DSBMV(RsBlasUplo Uplo, int K, double alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, double beta, const sp<Allocation>& Y, int incY) {
    // SBMV is the same as SYMV + K >= 0
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be greater than or equal to 0");
    }
    int N = validateSYMV(mRS, Element::F64(mRS), Uplo, A, X, Y, incX, incY);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsbmv,
                                0, 0, 0, Uplo, 0, 0, N, K, alpha,
                                A->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DSPMV(RsBlasUplo Uplo, double alpha, const sp<Allocation>& Ap, const sp<Allocation>& X,
                                int incX, double beta, const sp<Allocation>& Y, int incY) {
    int N = validateSPMV(mRS, Element::F64(mRS), Uplo, Ap, X, incX, Y, incY);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dspmv,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                Ap->getID(), X->getID(), beta, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DGER(double alpha, const sp<Allocation>& X, int incX, const sp<Allocation>& Y,
                               int incY, const sp<Allocation>& A) {
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    validateGER(mRS, Element::F64(mRS), X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dger,
                                0, 0, 0, 0, 0, M, N, 0, alpha,
                                X->getID(), Y->getID(), 0.f, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DSYR(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& A) {
    int N = validateSYR(mRS, Element::F64(mRS), Uplo, X, incX, A);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsyr,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), A->getID(), 0.f, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DSPR(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& Ap) {
    int N = validateSPR(mRS, Element::F64(mRS), Uplo, X, incX, Ap);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dspr,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), Ap->getID(), 0.f, 0, incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DSYR2(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    int N = validateSYR2(mRS, Element::F64(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsyr2,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), Y->getID(), 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::DSPR2(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& Ap) {
    int N = validateSPR2(mRS, Element::F64(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dspr2,
                                0, 0, 0, Uplo, 0, 0, N, 0, alpha,
                                X->getID(), Y->getID(), 0, Ap->getID(), incX, incY, 0, 0);
}


/**
 * Level 2, C and Z only
 */

static void validateGERU(RS* mRS, const sp<const Element>& e, const sp<Allocation>& X, int incX,
                         const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !X->getType()->getElement()->isCompatible(e) ||
        !Y->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (X->getType()->getY() > 1 || Y->getType()->getY() > 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "BLAS vectors must have Y dimension of 0 or 1");
    }

    int M = A->getType()->getY();
    int N = A->getType()->getX();
    if (incX <= 0 || incY <= 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Vector increments must be greater than 0");
    }
    int expectedXDim = 1 + (M - 1) * incX;
    if ((int)X->getType()->getX() != expectedXDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for GERU");
    }
    int expectedYDim = 1 + (N - 1) * incY;
    if ((int)Y->getType()->getX() != expectedYDim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Incorrect vector dimensions for GERU");
    }

}

void ScriptIntrinsicBLAS::CHEMV(RsBlasUplo Uplo, Float2 alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, Float2 beta, const sp<Allocation>& Y, int incY) {
    // HEMV is the same as SYR2 validation-wise
    int N = validateSYR2(mRS, Element::F32_2(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chemv,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha.x, alpha.y, A->getID(), X->getID(),
                                 beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CHBMV(RsBlasUplo Uplo, int K, Float2 alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, Float2 beta, const sp<Allocation>& Y, int incY) {
    // HBMV is the same as SYR2 validation-wise
    int N = validateSYR2(mRS, Element::F32_2(mRS), Uplo, X, incX, Y, incY, A);
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be 0 or greater for HBMV");
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chbmv,
                                 0, 0, 0, Uplo, 0, 0, N, K,
                                 alpha.x, alpha.y, A->getID(), X->getID(),
                                 beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CHPMV(RsBlasUplo Uplo, Float2 alpha, const sp<Allocation>& Ap,
                                const sp<Allocation>& X, int incX, Float2 beta, const sp<Allocation>& Y, int incY) {
    // HPMV is the same as SPR2
    int N = validateSPR2(mRS, Element::F32_2(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chpmv,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha.x, alpha.y, Ap->getID(), X->getID(),
                                 beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CGERU(Float2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    validateGERU(mRS, Element::F32_2(mRS), X, incX, Y, incY, A);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cgeru,
                                 0, 0, 0, 0, 0, M, N, 0,
                                 alpha.x, alpha.y, X->getID(), Y->getID(),
                                 0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CGERC(Float2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    // Same as GERU
    validateGERU(mRS, Element::F32_2(mRS), X, incX, Y, incY, A);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cgerc,
                                 0, 0, 0, 0, 0, M, N, 0,
                                 alpha.x, alpha.y, X->getID(), Y->getID(),
                                 0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CHER(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& A) {
    // Same as SYR
    int N = validateSYR(mRS, Element::F32_2(mRS), Uplo, X, incX, A);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cher,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha, 0, X->getID(), 0,
                                 0, 0, A->getID(), incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CHPR(RsBlasUplo Uplo, float alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& Ap) {
    // Equivalent to SPR for validation
    int N = validateSPR(mRS, Element::F32_2(mRS), Uplo, X, incX, Ap);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chpr,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha, 0, X->getID(), 0,
                                 0, 0, Ap->getID(), incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CHER2(RsBlasUplo Uplo, Float2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    // Same as SYR2
    int N = validateSYR2(mRS, Element::F32_2(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cher2,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha.x, alpha.y, X->getID(), Y->getID(),
                                 0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::CHPR2(RsBlasUplo Uplo, Float2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& Ap) {
    // Same as SPR2
    int N = validateSPR2(mRS, Element::F32_2(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chpr2,
                                 0, 0, 0, Uplo, 0, 0, N, 0,
                                 alpha.x, alpha.y, X->getID(), Y->getID(),
                                 0, 0, Ap->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZHEMV(RsBlasUplo Uplo, Double2 alpha, const sp<Allocation>& A,
                                const sp<Allocation>& X, int incX, Double2 beta, const sp<Allocation>& Y, int incY) {
    // HEMV is the same as SYR2 validation-wise
    int N = validateSYR2(mRS, Element::F64_2(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhemv,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha.x, alpha.y, A->getID(), X->getID(),
                           beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZHBMV(RsBlasUplo Uplo, int K, Double2 alpha, const sp<Allocation>& A, const sp<Allocation>& X,
                                int incX, Double2 beta, const sp<Allocation>& Y, int incY) {
    // HBMV is the same as SYR2 validation-wise
    int N = validateSYR2(mRS, Element::F64_2(mRS), Uplo, X, incX, Y, incY, A);
    if (K < 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "K must be 0 or greater for HBMV");
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhbmv,
                           0, 0, 0, Uplo, 0, 0, N, K,
                           alpha.x, alpha.y, A->getID(), X->getID(),
                           beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZHPMV(RsBlasUplo Uplo, Double2 alpha, const sp<Allocation>& Ap, const sp<Allocation>& X,
                                int incX, Double2 beta, const sp<Allocation>& Y, int incY) {
    // HPMV is the same as SPR2
    int N = validateSPR2(mRS, Element::F64_2(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhpmv,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha.x, alpha.y, Ap->getID(), X->getID(),
                           beta.x, beta.y, Y->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZGERU(Double2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    validateGERU(mRS, Element::F64_2(mRS), X, incX, Y, incY, A);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zgeru,
                           0, 0, 0, 0, 0, M, N, 0,
                           alpha.x, alpha.y, X->getID(), Y->getID(),
                           0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZGERC(Double2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    // Same as GERU
    validateGERU(mRS, Element::F64_2(mRS), X, incX, Y, incY, A);
    int M = A->getType()->getY();
    int N = A->getType()->getX();
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zgerc,
                           0, 0, 0, 0, 0, M, N, 0,
                           alpha.x, alpha.y, X->getID(), Y->getID(),
                           0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZHER(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& A) {
    // Same as SYR
    int N = validateSYR(mRS, Element::F64_2(mRS), Uplo, X, incX, A);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zher,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha, 0, X->getID(), 0,
                           0, 0, A->getID(), incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZHPR(RsBlasUplo Uplo, double alpha, const sp<Allocation>& X,
                               int incX, const sp<Allocation>& Ap) {
    // Equivalent to SPR for validation
    int N = validateSPR(mRS, Element::F64_2(mRS), Uplo, X, incX, Ap);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhpr,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha, 0, X->getID(), 0,
                           0, 0, Ap->getID(), incX, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZHER2(RsBlasUplo Uplo, Double2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& A) {
    // Same as SYR2
    int N = validateSYR2(mRS, Element::F64_2(mRS), Uplo, X, incX, Y, incY, A);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zher2,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha.x, alpha.y, X->getID(), Y->getID(),
                           0, 0, A->getID(), incX, incY, 0, 0);
}

void ScriptIntrinsicBLAS::ZHPR2(RsBlasUplo Uplo, Double2 alpha, const sp<Allocation>& X, int incX,
                                const sp<Allocation>& Y, int incY, const sp<Allocation>& Ap) {
    // Same as SPR2
    int N = validateSPR2(mRS, Element::F64_2(mRS), Uplo, X, incX, Y, incY, Ap);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhpr2,
                           0, 0, 0, Uplo, 0, 0, N, 0,
                           alpha.x, alpha.y, X->getID(), Y->getID(),
                           0, 0, Ap->getID(), incX, incY, 0, 0);
}


/**
 * Level 3 BLAS
 */

static void validateL3(RS* mRS, const sp<const Element>& e, int TransA, int TransB, int Side,
                       const sp<Allocation>& A, const sp<Allocation>& B, const sp<Allocation>& C) {
    int aM = -1, aN = -1, bM = -1, bN = -1, cM = -1, cN = -1;
    if ((A != nullptr && !A->getType()->getElement()->isCompatible(e)) ||
        (B != nullptr && !B->getType()->getElement()->isCompatible(e)) ||
        (C != nullptr && !C->getType()->getElement()->isCompatible(e))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (C == nullptr) {
        // Since matrix C is used to store the result, it cannot be null.
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Allocation C cannot be null");
    }
    cM = C->getType()->getY();
    cN = C->getType()->getX();

    if (Side == RsBlasRight) {
        if ((A == nullptr && B != nullptr) || (A != nullptr && B == nullptr)) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Provided Matrix A without Matrix B, or vice versa");
        }
        if (B != nullptr) {
            bM = A->getType()->getY();
            bN = A->getType()->getX();
        }
        if (A != nullptr) {
            aM = B->getType()->getY();
            aN = B->getType()->getX();
        }
    } else {
        if (A != nullptr) {
            if (TransA == RsBlasTrans || TransA == RsBlasConjTrans) {
                aN = A->getType()->getY();
                aM = A->getType()->getX();
            } else {
                aM = A->getType()->getY();
                aN = A->getType()->getX();
            }
        }
        if (B != nullptr) {
            if (TransB == RsBlasTrans || TransB == RsBlasConjTrans) {
                bN = B->getType()->getY();
                bM = B->getType()->getX();
            } else {
                bM = B->getType()->getY();
                bN = B->getType()->getX();
            }
        }
    }
    if (A != nullptr && B != nullptr && C != nullptr) {
        if (aN != bM || aM != cM || bN != cN) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called BLAS with invalid dimensions");
        }
    } else if (A != nullptr && C != nullptr) {
        // A and C only, for SYRK
        if (cM != cN) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Matrix C is not symmetric");
        }
        if (aM != cM) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called BLAS with invalid dimensions");
        }
    } else if (A != nullptr && B != nullptr) {
        // A and B only
        if (aN != bM) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called BLAS with invalid dimensions");
        }
    }

}

void ScriptIntrinsicBLAS::SGEMM(RsBlasTranspose TransA, RsBlasTranspose TransB, float alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, float beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F32(mRS), TransA, TransB, 0, A, B, C);

    int M = -1, N = -1, K = -1;
    if (TransA != RsBlasNoTrans) {
        M = A->getType()->getX();
        K = A->getType()->getY();
    } else {
        M = A->getType()->getY();
        K = A->getType()->getX();
    }
    if (TransB != RsBlasNoTrans) {
        N = B->getType()->getY();
    } else {
        N = B->getType()->getX();
    }
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_sgemm,
                                TransA, TransB, 0, 0, 0, M, N, K,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DGEMM(RsBlasTranspose TransA, RsBlasTranspose TransB, double alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, double beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F64(mRS), TransA, TransB, 0, A, B, C);
    int M = -1, N = -1, K = -1;
    if (TransA != RsBlasNoTrans) {
        M = A->getType()->getX();
        K = A->getType()->getY();
    } else {
        M = A->getType()->getY();
        K = A->getType()->getX();
    }
    if (TransB != RsBlasNoTrans) {
        N = B->getType()->getY();
    } else {
        N = B->getType()->getX();
    }
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dgemm,
                                TransA, TransB, 0, 0, 0, M, N, K,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CGEMM(RsBlasTranspose TransA, RsBlasTranspose TransB, Float2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Float2 beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F32_2(mRS), TransA, TransB, 0, A, B, C);
    int M = -1, N = -1, K = -1;
    if (TransA != RsBlasNoTrans) {
        M = A->getType()->getX();
        K = A->getType()->getY();
    } else {
        M = A->getType()->getY();
        K = A->getType()->getX();
    }
    if (TransB != RsBlasNoTrans) {
        N = B->getType()->getY();
    } else {
        N = B->getType()->getX();
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cgemm,
                                 TransA, TransB, 0, 0, 0, M, N, K,
                                 alpha.x, alpha.y, A->getID(), B->getID(),
                                 beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZGEMM(RsBlasTranspose TransA, RsBlasTranspose TransB, Double2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Double2 beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F64_2(mRS), TransA, TransB, 0, A, B, C);
    int M = -1, N = -1, K = -1;
    if (TransA != RsBlasNoTrans) {
        M = A->getType()->getX();
        K = A->getType()->getY();
    } else {
        M = A->getType()->getY();
        K = A->getType()->getX();
    }
    if (TransB != RsBlasNoTrans) {
        N = B->getType()->getY();
    } else {
        N = B->getType()->getX();
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zgemm,
                           TransA, TransB, 0, 0, 0, M, N, K,
                           alpha.x, alpha.y, A->getID(), B->getID(),
                           beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::SSYMM(RsBlasSide Side, RsBlasUplo Uplo, float alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, float beta, const sp<Allocation>& C) {
    //For SYMM, Matrix A should be symmetric
    if (A->getType()->getX() != A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Matrix A is not symmetric");
    }
    validateL3(mRS, Element::F32(mRS), 0, 0, Side, A, B, C);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssymm,
                                0, 0, Side, Uplo, 0, C->getType()->getY(), C->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DSYMM(RsBlasSide Side, RsBlasUplo Uplo, double alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, double beta, const sp<Allocation>& C) {
    if (A->getType()->getX() != A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Matrix A is not symmetric");
    }
    validateL3(mRS, Element::F64(mRS), 0, 0, Side, A, B, C);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsymm,
                                0, 0, Side, Uplo, 0, C->getType()->getY(), C->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CSYMM(RsBlasSide Side, RsBlasUplo Uplo, Float2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Float2 beta, const sp<Allocation>& C) {
    if (A->getType()->getX() != A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Matrix A is not symmetric");
    }
    validateL3(mRS, Element::F32_2(mRS), 0, 0, Side, A, B, C);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_csymm,
                                 0, 0, Side, Uplo, 0, C->getType()->getY(), C->getType()->getX(), 0,
                                 alpha.x, alpha.y, A->getID(), B->getID(),
                                 beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZSYMM(RsBlasSide Side, RsBlasUplo Uplo, Double2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Double2 beta, const sp<Allocation>& C) {
    if (A->getType()->getX() != A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Matrix A is not symmetric");
    }
    validateL3(mRS, Element::F64_2(mRS), 0, 0, Side, A, B, C);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zsymm,
                           0, 0, Side, Uplo, 0, C->getType()->getY(), C->getType()->getX(), 0,
                           alpha.x, alpha.y, A->getID(), B->getID(),
                           beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::SSYRK(RsBlasUplo Uplo, RsBlasTranspose Trans, float alpha,
                                const sp<Allocation>& A, float beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F32(mRS), Trans, 0, 0, A, nullptr, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssyrk,
                                Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                alpha, A->getID(), 0,
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DSYRK(RsBlasUplo Uplo, RsBlasTranspose Trans, double alpha,
                                const sp<Allocation>& A, double beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F64(mRS), Trans, 0, 0, A, nullptr, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsyrk,
                                Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                alpha, A->getID(), 0,
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CSYRK(RsBlasUplo Uplo, RsBlasTranspose Trans, Float2 alpha,
                                const sp<Allocation>& A, Float2 beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F32_2(mRS), Trans, 0, 0, A, nullptr, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_csyrk,
                                 Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                 alpha.x, alpha.y, A->getID(), 0,
                                 beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZSYRK(RsBlasUplo Uplo, RsBlasTranspose Trans, Double2 alpha,
                                const sp<Allocation>& A, Double2 beta, const sp<Allocation>& C) {
    validateL3(mRS, Element::F64_2(mRS), Trans, 0, 0, A, nullptr, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zsyrk,
                           Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                           alpha.x, alpha.y, A->getID(), 0,
                           beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

static void validateSYR2K(RS* mRS, const sp<const Element>& e, RsBlasTranspose Trans,
                          const sp<Allocation>& A, const sp<Allocation>& B, const sp<Allocation>& C) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !B->getType()->getElement()->isCompatible(e) ||
        !C->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    int Cdim = -1;
    // A is n x k if no transpose, k x n if transpose
    // C is n x n
    if (Trans == RsBlasTrans) {
        // check columns versus C
        Cdim = A->getType()->getX();
    } else {
        // check rows versus C
        Cdim = A->getType()->getY();
    }
    if ((int)C->getType()->getX() != Cdim || (int)C->getType()->getY() != Cdim) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid symmetric matrix in SYR2K");
    }
    // A dims == B dims
    if (A->getType()->getX() != B->getType()->getX() || A->getType()->getY() != B->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid A and B in SYR2K");
    }
}

void ScriptIntrinsicBLAS::SSYR2K(RsBlasUplo Uplo, RsBlasTranspose Trans, float alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, float beta, const sp<Allocation>& C) {
    validateSYR2K(mRS, Element::F32(mRS), Trans, A, B, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_ssyr2k,
                                Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DSYR2K(RsBlasUplo Uplo, RsBlasTranspose Trans, double alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, double beta, const sp<Allocation>& C) {
    validateSYR2K(mRS, Element::F64(mRS), Trans, A, B, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dsyr2k,
                                Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                alpha, A->getID(), B->getID(),
                                beta, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CSYR2K(RsBlasUplo Uplo, RsBlasTranspose Trans, Float2 alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, Float2 beta, const sp<Allocation>& C) {
    validateSYR2K(mRS, Element::F32_2(mRS), Trans, A, B, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_csyr2k,
                                 Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                                 alpha.x, alpha.y, A->getID(), B->getID(),
                                 beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZSYR2K(RsBlasUplo Uplo, RsBlasTranspose Trans, Double2 alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, Double2 beta, const sp<Allocation>& C) {
    validateSYR2K(mRS, Element::F64_2(mRS), Trans, A, B, C);
    int K = -1;
    if (Trans != RsBlasNoTrans) {
        K = A->getType()->getY();
    } else {
        K = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zsyr2k,
                           Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), K,
                           alpha.x, alpha.y, A->getID(), B->getID(),
                           beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

static void validateTRMM(RS* mRS, const sp<const Element>& e, RsBlasSide Side, RsBlasTranspose TransA,
                         const sp<Allocation>& A, const sp<Allocation>& B) {
    int aM = -1, aN = -1, bM = -1, bN = -1;
    if (!A->getType()->getElement()->isCompatible(e) ||
        !B->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }

    aM = A->getType()->getY();
    aN = A->getType()->getX();
    if (aM != aN) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRMM with a non-symmetric matrix A");
    }

    bM = B->getType()->getY();
    bN = B->getType()->getX();
    if (Side == RsBlasLeft) {
        if (aN != bM) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRMM with invalid matrices");
        }
    } else {
        if (bN != aM) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRMM with invalid matrices");
        }
    }
}

void ScriptIntrinsicBLAS::STRMM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                float alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRMM(mRS, Element::F32(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_strmm,
                                TransA, 0, Side, Uplo, Diag,\
                                B->getType()->getY(), B->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(), 0.f, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTRMM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                double alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRMM(mRS, Element::F64(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtrmm,
                                TransA, 0, Side, Uplo, Diag,
                                B->getType()->getY(), B->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTRMM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                Float2 alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRMM(mRS, Element::F32_2(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctrmm,
                                 TransA, 0, Side, Uplo, Diag,
                                 B->getType()->getY(), B->getType()->getX(), 0,
                                 alpha.x, alpha.y, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTRMM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                Double2 alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRMM(mRS, Element::F64_2(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztrmm,
                           TransA, 0, Side, Uplo, Diag,
                           B->getType()->getY(), B->getType()->getX(), 0,
                           alpha.x, alpha.y, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0, 0);
}

static void validateTRSM(RS* mRS, const sp<const Element>& e, RsBlasSide Side, RsBlasTranspose TransA,
                         const sp<Allocation>& A, const sp<Allocation>& B) {
    int adim = -1, bM = -1, bN = -1;
    if (!A->getType()->getElement()->isCompatible(e) ||
        !B->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    adim = A->getType()->getX();
    if (adim != (int)A->getType()->getY()) {
        // This may be unnecessary, the restriction could potentially be relaxed.
        // Allocation A needs to contain at least that symmetric matrix but could theoretically
        // be larger for now we assume adapters are sufficient, will reevaluate in the future.
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRSM with a non-symmetric matrix A");
    }
    bM = B->getType()->getY();
    bN = B->getType()->getX();
    if (Side == RsBlasLeft) {
        // A is M*M
        if (adim != bM) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRSM with invalid matrix dimensions");
        }
    } else {
        // A is N*N
        if (adim != bN) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called TRSM with invalid matrix dimensions");
        }
    }
}

void ScriptIntrinsicBLAS::STRSM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                float alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRSM(mRS, Element::F32(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Single(mRS, mRS->getContext(), getID(), RsBlas_strsm,
                                TransA, 0, Side, Uplo, Diag,
                                B->getType()->getY(), B->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::DTRSM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                double alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRSM(mRS, Element::F64(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Double(mRS, mRS->getContext(), getID(), RsBlas_dtrsm,
                                TransA, 0, Side, Uplo, Diag,
                                B->getType()->getY(), B->getType()->getX(), 0,
                                alpha, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::CTRSM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                Float2 alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRSM(mRS, Element::F32_2(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_ctrsm,
                                 TransA, 0, Side, Uplo, Diag,
                                 B->getType()->getY(), B->getType()->getX(), 0,
                                 alpha.x, alpha.y, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZTRSM(RsBlasSide Side, RsBlasUplo Uplo, RsBlasTranspose TransA, RsBlasDiag Diag,
                                Double2 alpha, const sp<Allocation>& A, const sp<Allocation>& B) {
    validateTRSM(mRS, Element::F64_2(mRS), Side, TransA, A, B);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_ztrsm,
                           TransA, 0, Side, Uplo, Diag,
                           B->getType()->getY(), B->getType()->getX(), 0,
                           alpha.x, alpha.y, A->getID(), B->getID(), 0, 0, 0, 0, 0, 0, 0);
}

static void validateHEMM(RS* mRS, const sp<const Element>& e, RsBlasSide Side,
                         const sp<Allocation>& A, const sp<Allocation>& B, const sp<Allocation>& C) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !B->getType()->getElement()->isCompatible(e) ||
        !C->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }

    // A must be square; can potentially be relaxed similar to TRSM
    int adim = A->getType()->getX();
    if (adim != (int)A->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HEMM with non-square A");
    }
    if ((Side == RsBlasLeft && adim != (int)B->getType()->getY()) ||
        (Side == RsBlasRight && adim != (int)B->getType()->getX())) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HEMM with invalid B");
    }
    if (B->getType()->getX() != C->getType()->getX() ||
        B->getType()->getY() != C->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HEMM with mismatched B and C");
    }
}

void ScriptIntrinsicBLAS::CHEMM(RsBlasSide Side, RsBlasUplo Uplo, Float2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Float2 beta, const sp<Allocation>& C) {
    validateHEMM(mRS, Element::F32_2(mRS), Side, A, B, C);
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_chemm,
                                 0, 0, Side, Uplo, 0,
                                 C->getType()->getY(), C->getType()->getX(), 0,
                                 alpha.x, alpha.y, A->getID(), B->getID(),
                                 beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZHEMM(RsBlasSide Side, RsBlasUplo Uplo, Double2 alpha,
                                const sp<Allocation>& A, const sp<Allocation>& B, Double2 beta, const sp<Allocation>& C) {
    validateHEMM(mRS, Element::F64_2(mRS), Side, A, B, C);
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zhemm,
                           0, 0, Side, Uplo, 0,
                           C->getType()->getY(), C->getType()->getX(), 0,
                           alpha.x, alpha.y, A->getID(), B->getID(),
                           beta.x, beta.y, C->getID(), 0, 0, 0, 0);
}

static void validateHERK(RS* mRS, const sp<const Element>& e, RsBlasTranspose Trans,
                         const sp<Allocation>& A, const sp<Allocation>& C) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !C->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (Trans != RsBlasNoTrans && Trans != RsBlasConjTrans) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Call HERK with invalid Transpose");
    }
    int cdim = C->getType()->getX();
    if (cdim != (int)C->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HERK with non-square C");
    }
    if (Trans == RsBlasNoTrans) {
        if (cdim != (int)A->getType()->getY()) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HERK with invalid A");
        }
    } else {
        if (cdim != (int)A->getType()->getX()) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HERK with invalid A");
        }
    }
}

void ScriptIntrinsicBLAS::CHERK(RsBlasUplo Uplo, RsBlasTranspose Trans, float alpha,
                                const sp<Allocation>& A, float beta, const sp<Allocation>& C) {
    validateHERK(mRS, Element::F32_2(mRS), Trans, A, C);
    int k = 0;
    if (Trans == RsBlasConjTrans) {
        k = A->getType()->getY();
    } else {
        k = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cherk,
                                 Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), k,
                                 alpha, 0, A->getID(), 0,
                                 beta, 0, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZHERK(RsBlasUplo Uplo, RsBlasTranspose Trans, double alpha,
                                const sp<Allocation>& A, double beta, const sp<Allocation>& C) {
    validateHERK(mRS, Element::F64_2(mRS), Trans, A, C);
    int k = 0;
    if (Trans == RsBlasConjTrans) {
        k = A->getType()->getY();
    } else {
        k = A->getType()->getX();
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zherk,
                           Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), k,
                           alpha, 0, A->getID(), 0,
                           beta, 0, C->getID(), 0, 0, 0, 0);
}

static void validateHER2K(RS* mRS, const sp<const Element>& e, RsBlasTranspose Trans,
                          const sp<Allocation>& A, const sp<Allocation>& B, const sp<Allocation>& C) {
    if (!A->getType()->getElement()->isCompatible(e) ||
        !B->getType()->getElement()->isCompatible(e) ||
        !C->getType()->getElement()->isCompatible(e)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Called BLAS with wrong Element type");
    }
    if (Trans != RsBlasNoTrans && Trans != RsBlasConjTrans) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Call HERK with invalid Transpose");
    }
    int cdim = C->getType()->getX();
    if (cdim != (int)C->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HER2K with non-square C");
    }
    if (Trans == RsBlasNoTrans) {
        if ((int)A->getType()->getY() != cdim) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HER2K with invalid matrices");
        }
    } else {
        if ((int)A->getType()->getX() != cdim) {
            mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HER2K with invalid matrices");
        }
    }
    if (A->getType()->getX() != B->getType()->getX() || A->getType()->getY() != B->getType()->getY()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Called HER2K with invalid A and B matrices");
    }
}

void ScriptIntrinsicBLAS::CHER2K(RsBlasUplo Uplo, RsBlasTranspose Trans, Float2 alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, float beta, const sp<Allocation>& C) {
    validateHER2K(mRS, Element::F32_2(mRS), Trans, A, B, C);
    int k = 0;
    if (Trans == RsBlasNoTrans) {
        k = A->getType()->getX();
    } else {
        k = A->getType()->getY();
    }
    nScriptIntrinsicBLAS_Complex(mRS, mRS->getContext(), getID(), RsBlas_cher2k,
                                 Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), k,
                                 alpha.x, alpha.y, A->getID(), B->getID(),
                                 beta, 0, C->getID(), 0, 0, 0, 0);
}

void ScriptIntrinsicBLAS::ZHER2K(RsBlasUplo Uplo, RsBlasTranspose Trans, Double2 alpha,
                                 const sp<Allocation>& A, const sp<Allocation>& B, double beta, const sp<Allocation>& C) {
    validateHER2K(mRS, Element::F64_2(mRS), Trans, A, B, C);
    int k = 0;
    if (Trans == RsBlasNoTrans) {
        k = A->getType()->getX();
    } else {
        k = A->getType()->getY();
    }
    nScriptIntrinsicBLAS_Z(mRS, mRS->getContext(), getID(), RsBlas_zher2k,
                           Trans, 0, 0, Uplo, 0, 0, C->getType()->getX(), k,
                           alpha.x, alpha.y, A->getID(), B->getID(),
                           beta, 0, C->getID(), 0, 0, 0, 0);
}



void ScriptIntrinsicBLAS::BNNM(const sp<Allocation>& A, int a_offset, const sp<Allocation>& B, int b_offset,
                               const sp<Allocation>& C, int c_offset, int c_mult) {
    validateL3(mRS, Element::U8(mRS), RsBlasNoTrans, RsBlasTrans, 0, A, B, C);

    if (a_offset < 0 || a_offset > 255) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid a_offset passed to BNNM");
    }
    if (b_offset < 0 || b_offset > 255) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid b_offset passed to BNNM");
    }
    int M = -1, N = -1, K = -1;
    M = A->getType()->getY();
    N = B->getType()->getY();
    K = A->getType()->getX();

    nScriptIntrinsicBLAS_BNNM(mRS, mRS->getContext(), getID(), M, N, K, A->getID(), a_offset,
                              B->getID(), b_offset, C->getID(), c_offset, c_mult);
}
