/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <stdio.h>

#include <utils/Log.h>

#include "Fusion.h"

namespace android {

// -----------------------------------------------------------------------

/*==================== BEGIN FUSION SENSOR PARAMETER =========================*/

/* Note:
 *   If a platform uses software fusion, it is necessary to tune the following
 *   parameters to fit the hardware sensors prior to release.
 *
 *   The DEFAULT_ parameters will be used in FUSION_9AXIS and FUSION_NOMAG mode.
 *   The GEOMAG_ parameters will be used in FUSION_NOGYRO mode.
 */

/*
 * GYRO_VAR gives the measured variance of the gyro's output per
 * Hz (or variance at 1 Hz). This is an "intrinsic" parameter of the gyro,
 * which is independent of the sampling frequency.
 *
 * The variance of gyro's output at a given sampling period can be
 * calculated as:
 *      variance(T) = GYRO_VAR / T
 *
 * The variance of the INTEGRATED OUTPUT at a given sampling period can be
 * calculated as:
 *       variance_integrate_output(T) = GYRO_VAR * T
 */
static const float DEFAULT_GYRO_VAR = 1e-7;      // (rad/s)^2 / Hz
static const float DEFAULT_GYRO_BIAS_VAR = 1e-12;  // (rad/s)^2 / s (guessed)
static const float GEOMAG_GYRO_VAR = 1e-4;      // (rad/s)^2 / Hz
static const float GEOMAG_GYRO_BIAS_VAR = 1e-8;  // (rad/s)^2 / s (guessed)

/*
 * Standard deviations of accelerometer and magnetometer
 */
static const float DEFAULT_ACC_STDEV  = 0.015f; // m/s^2 (measured 0.08 / CDD 0.05)
static const float DEFAULT_MAG_STDEV  = 0.1f;   // uT    (measured 0.7  / CDD 0.5)
static const float GEOMAG_ACC_STDEV  = 0.05f; // m/s^2 (measured 0.08 / CDD 0.05)
static const float GEOMAG_MAG_STDEV  = 0.1f;   // uT    (measured 0.7  / CDD 0.5)


/* ====================== END FUSION SENSOR PARAMETER ========================*/

static const float SYMMETRY_TOLERANCE = 1e-10f;

/*
 * Accelerometer updates will not be performed near free fall to avoid
 * ill-conditioning and div by zeros.
 * Threshhold: 10% of g, in m/s^2
 */
static const float NOMINAL_GRAVITY = 9.81f;
static const float FREE_FALL_THRESHOLD = 0.1f * (NOMINAL_GRAVITY);

/*
 * The geomagnetic-field should be between 30uT and 60uT.
 * Fields strengths greater than this likely indicate a local magnetic
 * disturbance which we do not want to update into the fused frame.
 */
static const float MAX_VALID_MAGNETIC_FIELD = 100; // uT
static const float MAX_VALID_MAGNETIC_FIELD_SQ =
        MAX_VALID_MAGNETIC_FIELD*MAX_VALID_MAGNETIC_FIELD;

/*
 * Values of the field smaller than this should be ignored in fusion to avoid
 * ill-conditioning. This state can happen with anomalous local magnetic
 * disturbances canceling the Earth field.
 */
static const float MIN_VALID_MAGNETIC_FIELD = 10; // uT
static const float MIN_VALID_MAGNETIC_FIELD_SQ =
        MIN_VALID_MAGNETIC_FIELD*MIN_VALID_MAGNETIC_FIELD;

/*
 * If the cross product of two vectors has magnitude squared less than this,
 * we reject it as invalid due to alignment of the vectors.
 * This threshold is used to check for the case where the magnetic field sample
 * is parallel to the gravity field, which can happen in certain places due
 * to magnetic field disturbances.
 */
static const float MIN_VALID_CROSS_PRODUCT_MAG = 1.0e-3;
static const float MIN_VALID_CROSS_PRODUCT_MAG_SQ =
    MIN_VALID_CROSS_PRODUCT_MAG*MIN_VALID_CROSS_PRODUCT_MAG;

static const float SQRT_3 = 1.732f;
static const float WVEC_EPS = 1e-4f/SQRT_3;
// -----------------------------------------------------------------------

template <typename TYPE, size_t C, size_t R>
static mat<TYPE, R, R> scaleCovariance(
        const mat<TYPE, C, R>& A,
        const mat<TYPE, C, C>& P) {
    // A*P*transpose(A);
    mat<TYPE, R, R> APAt;
    for (size_t r=0 ; r<R ; r++) {
        for (size_t j=r ; j<R ; j++) {
            double apat(0);
            for (size_t c=0 ; c<C ; c++) {
                double v(A[c][r]*P[c][c]*0.5);
                for (size_t k=c+1 ; k<C ; k++)
                    v += A[k][r] * P[c][k];
                apat += 2 * v * A[c][j];
            }
            APAt[j][r] = apat;
            APAt[r][j] = apat;
        }
    }
    return APAt;
}

template <typename TYPE, typename OTHER_TYPE>
static mat<TYPE, 3, 3> crossMatrix(const vec<TYPE, 3>& p, OTHER_TYPE diag) {
    mat<TYPE, 3, 3> r;
    r[0][0] = diag;
    r[1][1] = diag;
    r[2][2] = diag;
    r[0][1] = p.z;
    r[1][0] =-p.z;
    r[0][2] =-p.y;
    r[2][0] = p.y;
    r[1][2] = p.x;
    r[2][1] =-p.x;
    return r;
}


template<typename TYPE, size_t SIZE>
class Covariance {
    mat<TYPE, SIZE, SIZE> mSumXX;
    vec<TYPE, SIZE> mSumX;
    size_t mN;
public:
    Covariance() : mSumXX(0.0f), mSumX(0.0f), mN(0) { }
    void update(const vec<TYPE, SIZE>& x) {
        mSumXX += x*transpose(x);
        mSumX  += x;
        mN++;
    }
    mat<TYPE, SIZE, SIZE> operator()() const {
        const float N = 1.0f / mN;
        return mSumXX*N - (mSumX*transpose(mSumX))*(N*N);
    }
    void reset() {
        mN = 0;
        mSumXX = 0;
        mSumX = 0;
    }
    size_t getCount() const {
        return mN;
    }
};

// -----------------------------------------------------------------------

Fusion::Fusion() {
    Phi[0][1] = 0;
    Phi[1][1] = 1;

    Ba.x = 0;
    Ba.y = 0;
    Ba.z = 1;

    Bm.x = 0;
    Bm.y = 1;
    Bm.z = 0;

    x0 = 0;
    x1 = 0;

    init();
}

void Fusion::init(int mode) {
    mInitState = 0;

    mGyroRate = 0;

    mCount[0] = 0;
    mCount[1] = 0;
    mCount[2] = 0;

    mData = 0;
    mMode = mode;

    if (mMode != FUSION_NOGYRO) { //normal or game rotation
        mParam.gyroVar = DEFAULT_GYRO_VAR;
        mParam.gyroBiasVar = DEFAULT_GYRO_BIAS_VAR;
        mParam.accStdev = DEFAULT_ACC_STDEV;
        mParam.magStdev = DEFAULT_MAG_STDEV;
    } else {
        mParam.gyroVar = GEOMAG_GYRO_VAR;
        mParam.gyroBiasVar = GEOMAG_GYRO_BIAS_VAR;
        mParam.accStdev = GEOMAG_ACC_STDEV;
        mParam.magStdev = GEOMAG_MAG_STDEV;
    }
}

void Fusion::initFusion(const vec4_t& q, float dT)
{
    // initial estimate: E{ x(t0) }
    x0 = q;
    x1 = 0;

    // process noise covariance matrix: G.Q.Gt, with
    //
    //  G = | -1 0 |        Q = | q00 q10 |
    //      |  0 1 |            | q01 q11 |
    //
    // q00 = sv^2.dt + 1/3.su^2.dt^3
    // q10 = q01 = 1/2.su^2.dt^2
    // q11 = su^2.dt
    //

    const float dT2 = dT*dT;
    const float dT3 = dT2*dT;

    // variance of integrated output at 1/dT Hz (random drift)
    const float q00 = mParam.gyroVar * dT + 0.33333f * mParam.gyroBiasVar * dT3;

    // variance of drift rate ramp
    const float q11 = mParam.gyroBiasVar * dT;
    const float q10 = 0.5f * mParam.gyroBiasVar * dT2;
    const float q01 = q10;

    GQGt[0][0] =  q00;      // rad^2
    GQGt[1][0] = -q10;
    GQGt[0][1] = -q01;
    GQGt[1][1] =  q11;      // (rad/s)^2

    // initial covariance: Var{ x(t0) }
    // TODO: initialize P correctly
    P = 0;
}

bool Fusion::hasEstimate() const {
    return ((mInitState & MAG) || (mMode == FUSION_NOMAG)) &&
           ((mInitState & GYRO) || (mMode == FUSION_NOGYRO)) &&
           (mInitState & ACC);
}

bool Fusion::checkInitComplete(int what, const vec3_t& d, float dT) {
    if (hasEstimate())
        return true;

    if (what == ACC) {
        mData[0] += d * (1/length(d));
        mCount[0]++;
        mInitState |= ACC;
        if (mMode == FUSION_NOGYRO ) {
            mGyroRate = dT;
        }
    } else if (what == MAG) {
        mData[1] += d * (1/length(d));
        mCount[1]++;
        mInitState |= MAG;
    } else if (what == GYRO) {
        mGyroRate = dT;
        mData[2] += d*dT;
        mCount[2]++;
        mInitState |= GYRO;
    }

    if (hasEstimate()) {
        // Average all the values we collected so far
        mData[0] *= 1.0f/mCount[0];
        if (mMode != FUSION_NOMAG) {
            mData[1] *= 1.0f/mCount[1];
        }
        mData[2] *= 1.0f/mCount[2];

        // calculate the MRPs from the data collection, this gives us
        // a rough estimate of our initial state
        mat33_t R;
        vec3_t  up(mData[0]);
        vec3_t  east;

        if (mMode != FUSION_NOMAG) {
            east = normalize(cross_product(mData[1], up));
        } else {
            east = getOrthogonal(up);
        }

        vec3_t north(cross_product(up, east));
        R << east << north << up;
        const vec4_t q = matrixToQuat(R);

        initFusion(q, mGyroRate);
    }

    return false;
}

void Fusion::handleGyro(const vec3_t& w, float dT) {
    if (!checkInitComplete(GYRO, w, dT))
        return;

    predict(w, dT);
}

status_t Fusion::handleAcc(const vec3_t& a, float dT) {
    if (!checkInitComplete(ACC, a, dT))
        return BAD_VALUE;

    // ignore acceleration data if we're close to free-fall
    const float l = length(a);
    if (l < FREE_FALL_THRESHOLD) {
        return BAD_VALUE;
    }

    const float l_inv = 1.0f/l;

    if ( mMode == FUSION_NOGYRO ) {
        //geo mag
        vec3_t w_dummy;
        w_dummy = x1; //bias
        predict(w_dummy, dT);
    }

    if ( mMode == FUSION_NOMAG) {
        vec3_t m;
        m = getRotationMatrix()*Bm;
        update(m, Bm, mParam.magStdev);
    }

    vec3_t unityA = a * l_inv;
    const float d = sqrtf(fabsf(l- NOMINAL_GRAVITY));
    const float p = l_inv * mParam.accStdev*expf(d);

    update(unityA, Ba, p);
    return NO_ERROR;
}

status_t Fusion::handleMag(const vec3_t& m) {
    if (!checkInitComplete(MAG, m))
        return BAD_VALUE;

    // the geomagnetic-field should be between 30uT and 60uT
    // reject if too large to avoid spurious magnetic sources
    const float magFieldSq = length_squared(m);
    if (magFieldSq > MAX_VALID_MAGNETIC_FIELD_SQ) {
        return BAD_VALUE;
    } else if (magFieldSq < MIN_VALID_MAGNETIC_FIELD_SQ) {
        // Also reject if too small since we will get ill-defined (zero mag)
        // cross-products below
        return BAD_VALUE;
    }

    // Orthogonalize the magnetic field to the gravity field, mapping it into
    // tangent to Earth.
    const vec3_t up( getRotationMatrix() * Ba );
    const vec3_t east( cross_product(m, up) );

    // If the m and up vectors align, the cross product magnitude will
    // approach 0.
    // Reject this case as well to avoid div by zero problems and
    // ill-conditioning below.
    if (length_squared(east) < MIN_VALID_CROSS_PRODUCT_MAG_SQ) {
        return BAD_VALUE;
    }

    // If we have created an orthogonal magnetic field successfully,
    // then pass it in as the update.
    vec3_t north( cross_product(up, east) );

    const float l_inv = 1 / length(north);
    north *= l_inv;

    update(north, Bm,  mParam.magStdev*l_inv);
    return NO_ERROR;
}

void Fusion::checkState() {
    // P needs to stay positive semidefinite or the fusion diverges. When we
    // detect divergence, we reset the fusion.
    // TODO(braun): Instead, find the reason for the divergence and fix it.

    if (!isPositiveSemidefinite(P[0][0], SYMMETRY_TOLERANCE) ||
        !isPositiveSemidefinite(P[1][1], SYMMETRY_TOLERANCE)) {
        ALOGW("Sensor fusion diverged; resetting state.");
        P = 0;
    }
}

vec4_t Fusion::getAttitude() const {
    return x0;
}

vec3_t Fusion::getBias() const {
    return x1;
}

mat33_t Fusion::getRotationMatrix() const {
    return quatToMatrix(x0);
}

mat34_t Fusion::getF(const vec4_t& q) {
    mat34_t F;

    // This is used to compute the derivative of q
    // F = | [q.xyz]x |
    //     |  -q.xyz  |

    F[0].x = q.w;   F[1].x =-q.z;   F[2].x = q.y;
    F[0].y = q.z;   F[1].y = q.w;   F[2].y =-q.x;
    F[0].z =-q.y;   F[1].z = q.x;   F[2].z = q.w;
    F[0].w =-q.x;   F[1].w =-q.y;   F[2].w =-q.z;
    return F;
}

void Fusion::predict(const vec3_t& w, float dT) {
    const vec4_t q  = x0;
    const vec3_t b  = x1;
    vec3_t we = w - b;

    if (length(we) < WVEC_EPS) {
        we = (we[0]>0.f)?WVEC_EPS:-WVEC_EPS;
    }
    // q(k+1) = O(we)*q(k)
    // --------------------
    //
    // O(w) = | cos(0.5*||w||*dT)*I33 - [psi]x                   psi |
    //        | -psi'                              cos(0.5*||w||*dT) |
    //
    // psi = sin(0.5*||w||*dT)*w / ||w||
    //
    //
    // P(k+1) = Phi(k)*P(k)*Phi(k)' + G*Q(k)*G'
    // ----------------------------------------
    //
    // G = | -I33    0 |
    //     |    0  I33 |
    //
    //  Phi = | Phi00 Phi10 |
    //        |   0     1   |
    //
    //  Phi00 =   I33
    //          - [w]x   * sin(||w||*dt)/||w||
    //          + [w]x^2 * (1-cos(||w||*dT))/||w||^2
    //
    //  Phi10 =   [w]x   * (1        - cos(||w||*dt))/||w||^2
    //          - [w]x^2 * (||w||*dT - sin(||w||*dt))/||w||^3
    //          - I33*dT

    const mat33_t I33(1);
    const mat33_t I33dT(dT);
    const mat33_t wx(crossMatrix(we, 0));
    const mat33_t wx2(wx*wx);
    const float lwedT = length(we)*dT;
    const float hlwedT = 0.5f*lwedT;
    const float ilwe = 1.f/length(we);
    const float k0 = (1-cosf(lwedT))*(ilwe*ilwe);
    const float k1 = sinf(lwedT);
    const float k2 = cosf(hlwedT);
    const vec3_t psi(sinf(hlwedT)*ilwe*we);
    const mat33_t O33(crossMatrix(-psi, k2));
    mat44_t O;
    O[0].xyz = O33[0];  O[0].w = -psi.x;
    O[1].xyz = O33[1];  O[1].w = -psi.y;
    O[2].xyz = O33[2];  O[2].w = -psi.z;
    O[3].xyz = psi;     O[3].w = k2;

    Phi[0][0] = I33 - wx*(k1*ilwe) + wx2*k0;
    Phi[1][0] = wx*k0 - I33dT - wx2*(ilwe*ilwe*ilwe)*(lwedT-k1);

    x0 = O*q;

    if (x0.w < 0)
        x0 = -x0;

    P = Phi*P*transpose(Phi) + GQGt;

    checkState();
}

void Fusion::update(const vec3_t& z, const vec3_t& Bi, float sigma) {
    vec4_t q(x0);
    // measured vector in body space: h(p) = A(p)*Bi
    const mat33_t A(quatToMatrix(q));
    const vec3_t Bb(A*Bi);

    // Sensitivity matrix H = dh(p)/dp
    // H = [ L 0 ]
    const mat33_t L(crossMatrix(Bb, 0));

    // gain...
    // K = P*Ht / [H*P*Ht + R]
    vec<mat33_t, 2> K;
    const mat33_t R(sigma*sigma);
    const mat33_t S(scaleCovariance(L, P[0][0]) + R);
    const mat33_t Si(invert(S));
    const mat33_t LtSi(transpose(L)*Si);
    K[0] = P[0][0] * LtSi;
    K[1] = transpose(P[1][0])*LtSi;

    // update...
    // P = (I-K*H) * P
    // P -= K*H*P
    // | K0 | * | L 0 | * P = | K0*L  0 | * | P00  P10 | = | K0*L*P00  K0*L*P10 |
    // | K1 |                 | K1*L  0 |   | P01  P11 |   | K1*L*P00  K1*L*P10 |
    // Note: the Joseph form is numerically more stable and given by:
    //     P = (I-KH) * P * (I-KH)' + K*R*R'
    const mat33_t K0L(K[0] * L);
    const mat33_t K1L(K[1] * L);
    P[0][0] -= K0L*P[0][0];
    P[1][1] -= K1L*P[1][0];
    P[1][0] -= K0L*P[1][0];
    P[0][1] = transpose(P[1][0]);

    const vec3_t e(z - Bb);
    const vec3_t dq(K[0]*e);

    q += getF(q)*(0.5f*dq);
    x0 = normalize_quat(q);

    if (mMode != FUSION_NOMAG) {
        const vec3_t db(K[1]*e);
        x1 += db;
    }

    checkState();
}

vec3_t Fusion::getOrthogonal(const vec3_t &v) {
    vec3_t w;
    if (fabsf(v[0])<= fabsf(v[1]) && fabsf(v[0]) <= fabsf(v[2]))  {
        w[0]=0.f;
        w[1] = v[2];
        w[2] = -v[1];
    } else if (fabsf(v[1]) <= fabsf(v[2])) {
        w[0] = v[2];
        w[1] = 0.f;
        w[2] = -v[0];
    }else {
        w[0] = v[1];
        w[1] = -v[0];
        w[2] = 0.f;
    }
    return normalize(w);
}


// -----------------------------------------------------------------------

}; // namespace android

