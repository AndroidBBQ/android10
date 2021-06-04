/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_RESAMPLER_FIR_GEN_H
#define ANDROID_AUDIO_RESAMPLER_FIR_GEN_H

#include "utils/Compat.h"

namespace android {

/*
 * generates a sine wave at equal steps.
 *
 * As most of our functions use sine or cosine at equal steps,
 * it is very efficient to compute them that way (single multiply and subtract),
 * rather than invoking the math library sin() or cos() each time.
 *
 * SineGen uses Goertzel's Algorithm (as a generator not a filter)
 * to calculate sine(wstart + n * wstep) or cosine(wstart + n * wstep)
 * by stepping through 0, 1, ... n.
 *
 * e^i(wstart+wstep) = 2cos(wstep) * e^i(wstart) - e^i(wstart-wstep)
 *
 * or looking at just the imaginary sine term, as the cosine follows identically:
 *
 * sin(wstart+wstep) = 2cos(wstep) * sin(wstart) - sin(wstart-wstep)
 *
 * Goertzel's algorithm is more efficient than the angle addition formula,
 * e^i(wstart+wstep) = e^i(wstart) * e^i(wstep), which takes up to
 * 4 multiplies and 2 adds (or 3* and 3+) and requires both sine and
 * cosine generation due to the complex * complex multiply (full rotation).
 *
 * See: http://en.wikipedia.org/wiki/Goertzel_algorithm
 *
 */

class SineGen {
public:
    SineGen(double wstart, double wstep, bool cosine = false) {
        if (cosine) {
            mCurrent = cos(wstart);
            mPrevious = cos(wstart - wstep);
        } else {
            mCurrent = sin(wstart);
            mPrevious = sin(wstart - wstep);
        }
        mTwoCos = 2.*cos(wstep);
    }
    SineGen(double expNow, double expPrev, double twoCosStep) {
        mCurrent = expNow;
        mPrevious = expPrev;
        mTwoCos = twoCosStep;
    }
    inline double value() const {
        return mCurrent;
    }
    inline void advance() {
        double tmp = mCurrent;
        mCurrent = mCurrent*mTwoCos - mPrevious;
        mPrevious = tmp;
    }
    inline double valueAdvance() {
        double tmp = mCurrent;
        mCurrent = mCurrent*mTwoCos - mPrevious;
        mPrevious = tmp;
        return tmp;
    }

private:
    double mCurrent; // current value of sine/cosine
    double mPrevious; // previous value of sine/cosine
    double mTwoCos; // stepping factor
};

/*
 * generates a series of sine generators, phase offset by fixed steps.
 *
 * This is used to generate polyphase sine generators, one per polyphase
 * in the filter code below.
 *
 * The SineGen returned by value() starts at innerStart = outerStart + n*outerStep;
 * increments by innerStep.
 *
 */

class SineGenGen {
public:
    SineGenGen(double outerStart, double outerStep, double innerStep, bool cosine = false)
            : mSineInnerCur(outerStart, outerStep, cosine),
              mSineInnerPrev(outerStart-innerStep, outerStep, cosine)
    {
        mTwoCos = 2.*cos(innerStep);
    }
    inline SineGen value() {
        return SineGen(mSineInnerCur.value(), mSineInnerPrev.value(), mTwoCos);
    }
    inline void advance() {
        mSineInnerCur.advance();
        mSineInnerPrev.advance();
    }
    inline SineGen valueAdvance() {
        return SineGen(mSineInnerCur.valueAdvance(), mSineInnerPrev.valueAdvance(), mTwoCos);
    }

private:
    SineGen mSineInnerCur; // generate the inner sine values (stepped by outerStep).
    SineGen mSineInnerPrev; // generate the inner sine previous values
                            // (behind by innerStep, stepped by outerStep).
    double mTwoCos; // the inner stepping factor for the returned SineGen.
};

static inline double sqr(double x) {
    return x * x;
}

/*
 * rounds a double to the nearest integer for FIR coefficients.
 *
 * One variant uses noise shaping, which must keep error history
 * to work (the err parameter, initialized to 0).
 * The other variant is a non-noise shaped version for
 * S32 coefficients (noise shaping doesn't gain much).
 *
 * Caution: No bounds saturation is applied, but isn't needed in this case.
 *
 * @param x is the value to round.
 *
 * @param maxval is the maximum integer scale factor expressed as an int64 (for headroom).
 * Typically this may be the maximum positive integer+1 (using the fact that double precision
 * FIR coefficients generated here are never that close to 1.0 to pose an overflow condition).
 *
 * @param err is the previous error (actual - rounded) for the previous rounding op.
 * For 16b coefficients this can improve stopband dB performance by up to 2dB.
 *
 * Many variants exist for the noise shaping: http://en.wikipedia.org/wiki/Noise_shaping
 *
 */

static inline int64_t toint(double x, int64_t maxval, double& err) {
    double val = x * maxval;
    double ival = floor(val + 0.5 + err*0.2);
    err = val - ival;
    return static_cast<int64_t>(ival);
}

static inline int64_t toint(double x, int64_t maxval) {
    return static_cast<int64_t>(floor(x * maxval + 0.5));
}

/*
 * Modified Bessel function of the first kind
 * http://en.wikipedia.org/wiki/Bessel_function
 *
 * The formulas are taken from Abramowitz and Stegun,
 * _Handbook of Mathematical Functions_ (links below):
 *
 * http://people.math.sfu.ca/~cbm/aands/page_375.htm
 * http://people.math.sfu.ca/~cbm/aands/page_378.htm
 *
 * http://dlmf.nist.gov/10.25
 * http://dlmf.nist.gov/10.40
 *
 * Note we assume x is nonnegative (the function is symmetric,
 * pass in the absolute value as needed).
 *
 * Constants are compile time derived with templates I0Term<> and
 * I0ATerm<> to the precision of the compiler.  The series can be expanded
 * to any precision needed, but currently set around 24b precision.
 *
 * We use a bit of template math here, constexpr would probably be
 * more appropriate for a C++11 compiler.
 *
 * For the intermediate range 3.75 < x < 15, we use minimax polynomial fit.
 *
 */

template <int N>
struct I0Term {
    static const CONSTEXPR double value = I0Term<N-1>::value / (4. * N * N);
};

template <>
struct I0Term<0> {
    static const CONSTEXPR double value = 1.;
};

template <int N>
struct I0ATerm {
    static const CONSTEXPR double value = I0ATerm<N-1>::value * (2.*N-1.) * (2.*N-1.) / (8. * N);
};

template <>
struct I0ATerm<0> { // 1/sqrt(2*PI);
    static const CONSTEXPR double value =
            0.398942280401432677939946059934381868475858631164934657665925;
};

#if USE_HORNERS_METHOD
/* Polynomial evaluation of A + Bx + Cx^2 + Dx^3 + ...
 * using Horner's Method: http://en.wikipedia.org/wiki/Horner's_method
 *
 * This has fewer multiplications than Estrin's method below, but has back to back
 * floating point dependencies.
 *
 * On ARM this appears to work slower, so USE_HORNERS_METHOD is not default enabled.
 */

inline double Poly2(double A, double B, double x) {
    return A + x * B;
}

inline double Poly4(double A, double B, double C, double D, double x) {
    return A + x * (B + x * (C + x * (D)));
}

inline double Poly7(double A, double B, double C, double D, double E, double F, double G,
        double x) {
    return A + x * (B + x * (C + x * (D + x * (E + x * (F + x * (G))))));
}

inline double Poly9(double A, double B, double C, double D, double E, double F, double G,
        double H, double I, double x) {
    return A + x * (B + x * (C + x * (D + x * (E + x * (F + x * (G + x * (H + x * (I))))))));
}

#else
/* Polynomial evaluation of A + Bx + Cx^2 + Dx^3 + ...
 * using Estrin's Method: http://en.wikipedia.org/wiki/Estrin's_scheme
 *
 * This is typically faster, perhaps gains about 5-10% overall on ARM processors
 * over Horner's method above.
 */

inline double Poly2(double A, double B, double x) {
    return A + B * x;
}

inline double Poly3(double A, double B, double C, double x, double x2) {
    return Poly2(A, B, x) + C * x2;
}

inline double Poly3(double A, double B, double C, double x) {
    return Poly2(A, B, x) + C * x * x;
}

inline double Poly4(double A, double B, double C, double D, double x, double x2) {
    return Poly2(A, B, x) + Poly2(C, D, x) * x2; // same as poly2(poly2, poly2, x2);
}

inline double Poly4(double A, double B, double C, double D, double x) {
    return Poly4(A, B, C, D, x, x * x);
}

inline double Poly7(double A, double B, double C, double D, double E, double F, double G,
        double x) {
    double x2 = x * x;
    return Poly4(A, B, C, D, x, x2) + Poly3(E, F, G, x, x2) * (x2 * x2);
}

inline double Poly8(double A, double B, double C, double D, double E, double F, double G,
        double H, double x, double x2, double x4) {
    return Poly4(A, B, C, D, x, x2) + Poly4(E, F, G, H, x, x2) * x4;
}

inline double Poly9(double A, double B, double C, double D, double E, double F, double G,
        double H, double I, double x) {
    double x2 = x * x;
#if 1
    // It does not seem faster to explicitly decompose Poly8 into Poly4, but
    // could depend on compiler floating point scheduling.
    double x4 = x2 * x2;
    return Poly8(A, B, C, D, E, F, G, H, x, x2, x4) + I * (x4 * x4);
#else
    double val = Poly4(A, B, C, D, x, x2);
    double x4 = x2 * x2;
    return val + Poly4(E, F, G, H, x, x2) * x4 + I * (x4 * x4);
#endif
}
#endif

static inline double I0(double x) {
    if (x < 3.75) {
        x *= x;
        return Poly7(I0Term<0>::value, I0Term<1>::value,
                I0Term<2>::value, I0Term<3>::value,
                I0Term<4>::value, I0Term<5>::value,
                I0Term<6>::value, x); // e < 1.6e-7
    }
    if (1) {
        /*
         * Series expansion coefs are easy to calculate, but are expanded around 0,
         * so error is unequal over the interval 0 < x < 3.75, the error being
         * significantly better near 0.
         *
         * A better solution is to use precise minimax polynomial fits.
         *
         * We use a slightly more complicated solution for 3.75 < x < 15, based on
         * the tables in Blair and Edwards, "Stable Rational Minimax Approximations
         * to the Modified Bessel Functions I0(x) and I1(x)", Chalk Hill Nuclear Laboratory,
         * AECL-4928.
         *
         * http://www.iaea.org/inis/collection/NCLCollectionStore/_Public/06/178/6178667.pdf
         *
         * See Table 11 for 0 < x < 15; e < 10^(-7.13).
         *
         * Note: Beta cannot exceed 15 (hence Stopband cannot exceed 144dB = 24b).
         *
         * This speeds up overall computation by about 40% over using the else clause below,
         * which requires sqrt and exp.
         *
         */

        x *= x;
        double num = Poly9(-0.13544938430e9, -0.33153754512e8,
                -0.19406631946e7, -0.48058318783e5,
                -0.63269783360e3, -0.49520779070e1,
                -0.24970910370e-1, -0.74741159550e-4,
                -0.18257612460e-6, x);
        double y = x - 225.; // reflection around 15 (squared)
        double den = Poly4(-0.34598737196e8, 0.23852643181e6,
                -0.70699387620e3, 0.10000000000e1, y);
        return num / den;

#if IO_EXTENDED_BETA
        /* Table 42 for x > 15; e < 10^(-8.11).
         * This is used for Beta>15, but is disabled here as
         * we never use Beta that high.
         *
         * NOTE: This should be enabled only for x > 15.
         */

        double y = 1./x;
        double z = y - (1./15);
        double num = Poly2(0.415079861746e1, -0.5149092496e1, z);
        double den = Poly3(0.103150763823e2, -0.14181687413e2,
                0.1000000000e1, z);
        return exp(x) * sqrt(y) * num / den;
#endif
    } else {
        /*
         * NOT USED, but reference for large Beta.
         *
         * Abramowitz and Stegun asymptotic formula.
         * works for x > 3.75.
         */
        double y = 1./x;
        return exp(x) * sqrt(y) *
                // note: reciprocal squareroot may be easier!
                // http://en.wikipedia.org/wiki/Fast_inverse_square_root
                Poly9(I0ATerm<0>::value, I0ATerm<1>::value,
                        I0ATerm<2>::value, I0ATerm<3>::value,
                        I0ATerm<4>::value, I0ATerm<5>::value,
                        I0ATerm<6>::value, I0ATerm<7>::value,
                        I0ATerm<8>::value, y); // (... e) < 1.9e-7
    }
}

/* A speed optimized version of the Modified Bessel I0() which incorporates
 * the sqrt and numerator multiply and denominator divide into the computation.
 * This speeds up filter computation by about 10-15%.
 */
static inline double I0SqrRat(double x2, double num, double den) {
    if (x2 < (3.75 * 3.75)) {
        return Poly7(I0Term<0>::value, I0Term<1>::value,
                I0Term<2>::value, I0Term<3>::value,
                I0Term<4>::value, I0Term<5>::value,
                I0Term<6>::value, x2) * num / den; // e < 1.6e-7
    }
    num *= Poly9(-0.13544938430e9, -0.33153754512e8,
            -0.19406631946e7, -0.48058318783e5,
            -0.63269783360e3, -0.49520779070e1,
            -0.24970910370e-1, -0.74741159550e-4,
            -0.18257612460e-6, x2); // e < 10^(-7.13).
    double y = x2 - 225.; // reflection around 15 (squared)
    den *= Poly4(-0.34598737196e8, 0.23852643181e6,
            -0.70699387620e3, 0.10000000000e1, y);
    return num / den;
}

/*
 * calculates the transition bandwidth for a Kaiser filter
 *
 * Formula 3.2.8, Vaidyanathan, _Multirate Systems and Filter Banks_, p. 48
 * Formula 7.76, Oppenheim and Schafer, _Discrete-time Signal Processing, 3e_, p. 542
 *
 * @param halfNumCoef is half the number of coefficients per filter phase.
 *
 * @param stopBandAtten is the stop band attenuation desired.
 *
 * @return the transition bandwidth in normalized frequency (0 <= f <= 0.5)
 */
static inline double firKaiserTbw(int halfNumCoef, double stopBandAtten) {
    return (stopBandAtten - 7.95)/((2.*14.36)*halfNumCoef);
}

/*
 * calculates the fir transfer response of the overall polyphase filter at w.
 *
 * Calculates the DTFT transfer coefficient H(w) for 0 <= w <= PI, utilizing the
 * fact that h[n] is symmetric (cosines only, no complex arithmetic).
 *
 * We use Goertzel's algorithm to accelerate the computation to essentially
 * a single multiply and 2 adds per filter coefficient h[].
 *
 * Be careful be careful to consider that h[n] is the overall polyphase filter,
 * with L phases, so rescaling H(w)/L is probably what you expect for "unity gain",
 * as you only use one of the polyphases at a time.
 */
template <typename T>
static inline double firTransfer(const T* coef, int L, int halfNumCoef, double w) {
    double accum = static_cast<double>(coef[0])*0.5;  // "center coefficient" from first bank
    coef += halfNumCoef;    // skip first filterbank (picked up by the last filterbank).
#if SLOW_FIRTRANSFER
    /* Original code for reference.  This is equivalent to the code below, but slower. */
    for (int i=1 ; i<=L ; ++i) {
        for (int j=0, ix=i ; j<halfNumCoef ; ++j, ix+=L) {
            accum += cos(ix*w)*static_cast<double>(*coef++);
        }
    }
#else
    /*
     * Our overall filter is stored striped by polyphases, not a contiguous h[n].
     * We could fetch coefficients in a non-contiguous fashion
     * but that will not scale to vector processing.
     *
     * We apply Goertzel's algorithm directly to each polyphase filter bank instead of
     * using cosine generation/multiplication, thereby saving one multiply per inner loop.
     *
     * See: http://en.wikipedia.org/wiki/Goertzel_algorithm
     * Also: Oppenheim and Schafer, _Discrete Time Signal Processing, 3e_, p. 720.
     *
     * We use the basic recursion to incorporate the cosine steps into real sequence x[n]:
     * s[n] = x[n] + (2cosw)*s[n-1] + s[n-2]
     *
     * y[n] = s[n] - e^(iw)s[n-1]
     *      = sum_{k=-\infty}^{n} x[k]e^(-iw(n-k))
     *      = e^(-iwn) sum_{k=0}^{n} x[k]e^(iwk)
     *
     * The summation contains the frequency steps we want multiplied by the source
     * (similar to a DTFT).
     *
     * Using symmetry, and just the real part (be careful, this must happen
     * after any internal complex multiplications), the polyphase filterbank
     * transfer function is:
     *
     * Hpp[n, w, w_0] = sum_{k=0}^{n} x[k] * cos(wk + w_0)
     *                = Re{ e^(iwn + iw_0) y[n]}
     *                = cos(wn+w_0) * s[n] - cos(w(n+1)+w_0) * s[n-1]
     *
     * using the fact that s[n] of real x[n] is real.
     *
     */
    double dcos = 2. * cos(L*w);
    int start = ((halfNumCoef)*L + 1);
    SineGen cc((start - L) * w, w, true); // cosine
    SineGen cp(start * w, w, true); // cosine
    for (int i=1 ; i<=L ; ++i) {
        double sc = 0;
        double sp = 0;
        for (int j=0 ; j<halfNumCoef ; ++j) {
            double tmp = sc;
            sc  = static_cast<double>(*coef++) + dcos*sc - sp;
            sp = tmp;
        }
        // If we are awfully clever, we can apply Goertzel's algorithm
        // again on the sc and sp sequences returned here.
        accum += cc.valueAdvance() * sc - cp.valueAdvance() * sp;
    }
#endif
    return accum*2.;
}

/*
 * evaluates the minimum and maximum |H(f)| bound in a band region.
 *
 * This is usually done with equally spaced increments in the target band in question.
 * The passband is often very small, and sampled that way. The stopband is often much
 * larger.
 *
 * We use the fact that the overall polyphase filter has an additional bank at the end
 * for interpolation; hence it is overspecified for the H(f) computation.  Thus the
 * first polyphase is never actually checked, excepting its first term.
 *
 * In this code we use the firTransfer() evaluator above, which uses Goertzel's
 * algorithm to calculate the transfer function at each point.
 *
 * TODO: An alternative with equal spacing is the FFT/DFT.  An alternative with unequal
 * spacing is a chirp transform.
 *
 * @param coef is the designed polyphase filter banks
 *
 * @param L is the number of phases (for interpolation)
 *
 * @param halfNumCoef should be half the number of coefficients for a single
 * polyphase.
 *
 * @param fstart is the normalized frequency start.
 *
 * @param fend is the normalized frequency end.
 *
 * @param steps is the number of steps to take (sampling) between frequency start and end
 *
 * @param firMin returns the minimum transfer |H(f)| found
 *
 * @param firMax returns the maximum transfer |H(f)| found
 *
 * 0 <= f <= 0.5.
 * This is used to test passband and stopband performance.
 */
template <typename T>
static void testFir(const T* coef, int L, int halfNumCoef,
        double fstart, double fend, int steps, double &firMin, double &firMax) {
    double wstart = fstart*(2.*M_PI);
    double wend = fend*(2.*M_PI);
    double wstep = (wend - wstart)/steps;
    double fmax, fmin;
    double trf = firTransfer(coef, L, halfNumCoef, wstart);
    if (trf<0) {
        trf = -trf;
    }
    fmin = fmax = trf;
    wstart += wstep;
    for (int i=1; i<steps; ++i) {
        trf = firTransfer(coef, L, halfNumCoef, wstart);
        if (trf<0) {
            trf = -trf;
        }
        if (trf>fmax) {
            fmax = trf;
        }
        else if (trf<fmin) {
            fmin = trf;
        }
        wstart += wstep;
    }
    // renormalize - this is needed for integer filter types, use 1 for float or double.
    constexpr int integralShift = std::is_integral<T>::value ? (sizeof(T) * CHAR_BIT - 1) : 0;
    const double norm = 1. / (int64_t{L} << integralShift);

    firMin = fmin * norm;
    firMax = fmax * norm;
}

/*
 * evaluates the |H(f)| lowpass band characteristics.
 *
 * This function tests the lowpass characteristics for the overall polyphase filter,
 * and is used to verify the design.
 *
 * For a polyphase filter (L > 1), typically fp should be set to the
 * passband normalized frequency from 0 to 0.5 for the overall filter (thus it
 * is the designed polyphase bank value / L).  Likewise for fs.
 * Similarly the stopSteps should be L * passSteps for equivalent accuracy.
 *
 * @param coef is the designed polyphase filter banks
 *
 * @param L is the number of phases (for interpolation)
 *
 * @param halfNumCoef should be half the number of coefficients for a single
 * polyphase.
 *
 * @param fp is the passband normalized frequency, 0 < fp < fs < 0.5.
 *
 * @param fs is the stopband normalized frequency, 0 < fp < fs < 0.5.
 *
 * @param passSteps is the number of passband sampling steps.
 *
 * @param stopSteps is the number of stopband sampling steps.
 *
 * @param passMin is the minimum value in the passband
 *
 * @param passMax is the maximum value in the passband (useful for scaling).  This should
 * be less than 1., to avoid sine wave test overflow.
 *
 * @param passRipple is the passband ripple.  Typically this should be less than 0.1 for
 * an audio filter.  Generally speaker/headphone device characteristics will dominate
 * the passband term.
 *
 * @param stopMax is the maximum value in the stopband.
 *
 * @param stopRipple is the stopband ripple, also known as stopband attenuation.
 * Typically this should be greater than ~80dB for low quality, and greater than
 * ~100dB for full 16b quality, otherwise aliasing may become noticeable.
 *
 */
template <typename T>
static void testFir(const T* coef, int L, int halfNumCoef,
        double fp, double fs, int passSteps, int stopSteps,
        double &passMin, double &passMax, double &passRipple,
        double &stopMax, double &stopRipple) {
    double fmin, fmax;
    testFir(coef, L, halfNumCoef, 0., fp, passSteps, fmin, fmax);
    double d1 = (fmax - fmin)/2.;
    passMin = fmin;
    passMax = fmax;
    passRipple = -20.*log10(1. - d1); // passband ripple
    testFir(coef, L, halfNumCoef, fs, 0.5, stopSteps, fmin, fmax);
    // fmin is really not important for the stopband.
    stopMax = fmax;
    stopRipple = -20.*log10(fmax); // stopband ripple/attenuation
}

/*
 * Estimate the windowed sinc minimum passband value.
 *
 * This is the minimum value for a windowed sinc filter in its passband,
 * which is identical to the scaling required not to cause overflow of a 0dBFS signal.
 * The actual value used to attenuate the filter amplitude should be slightly
 * smaller than this (suggest squaring) as this is just an estimate.
 *
 * As a windowed sinc has a passband ripple commensurate to the stopband attenuation
 * due to Gibb's phenomenon from truncating the sinc, we derive this value from
 * the design stopbandAttenuationDb (a positive value).
 */
static inline double computeWindowedSincMinimumPassbandValue(
        double stopBandAttenuationDb) {
    return 1. - pow(10. /* base */, stopBandAttenuationDb * (-1. / 20.));
}

/*
 * Compute the windowed sinc passband ripple from stopband attenuation.
 *
 * As a windowed sinc has an passband ripple commensurate to the stopband attenuation
 * due to Gibb's phenomenon from truncating the sinc, we derive this value from
 * the design stopbandAttenuationDb (a positive value).
 */
static inline double computeWindowedSincPassbandRippleDb(
        double stopBandAttenuationDb) {
    return -20. * log10(computeWindowedSincMinimumPassbandValue(stopBandAttenuationDb));
}

/*
 * Kaiser window Beta value
 *
 * Formula 3.2.5, 3.2.7, Vaidyanathan, _Multirate Systems and Filter Banks_, p. 48
 * Formula 7.75, Oppenheim and Schafer, _Discrete-time Signal Processing, 3e_, p. 542
 *
 * See also: http://melodi.ee.washington.edu/courses/ee518/notes/lec17.pdf
 *
 * Kaiser window and beta parameter
 *
 *         | 0.1102*(A - 8.7)                         A > 50
 *  Beta = | 0.5842*(A - 21)^0.4 + 0.07886*(A - 21)   21 < A <= 50
 *         | 0.                                       A <= 21
 *
 * with A is the desired stop-band attenuation in positive dBFS
 *
 *    30 dB    2.210
 *    40 dB    3.384
 *    50 dB    4.538
 *    60 dB    5.658
 *    70 dB    6.764
 *    80 dB    7.865
 *    90 dB    8.960
 *   100 dB   10.056
 *
 * For some values of stopBandAttenuationDb the function may be computed
 * at compile time.
 */
static inline constexpr double computeBeta(double stopBandAttenuationDb) {
    if (stopBandAttenuationDb > 50.) {
        return 0.1102 * (stopBandAttenuationDb - 8.7);
    }
    const double offset = stopBandAttenuationDb - 21.;
    if (offset > 0.) {
        return 0.5842 * pow(offset, 0.4) + 0.07886 * offset;
    }
    return 0.;
}

/*
 * Calculates the overall polyphase filter based on a windowed sinc function.
 *
 * The windowed sinc is an odd length symmetric filter of exactly L*halfNumCoef*2+1
 * taps for the entire kernel.  This is then decomposed into L+1 polyphase filterbanks.
 * The last filterbank is used for interpolation purposes (and is mostly composed
 * of the first bank shifted by one sample), and is unnecessary if one does
 * not do interpolation.
 *
 * We use the last filterbank for some transfer function calculation purposes,
 * so it needs to be generated anyways.
 *
 * @param coef is the caller allocated space for coefficients.  This should be
 * exactly (L+1)*halfNumCoef in size.
 *
 * @param L is the number of phases (for interpolation)
 *
 * @param halfNumCoef should be half the number of coefficients for a single
 * polyphase.
 *
 * @param stopBandAtten is the stopband value, should be >50dB.
 *
 * @param fcr is cutoff frequency/sampling rate (<0.5).  At this point, the energy
 * should be 6dB less. (fcr is where the amplitude drops by half).  Use the
 * firKaiserTbw() to calculate the transition bandwidth.  fcr is the midpoint
 * between the stop band and the pass band (fstop+fpass)/2.
 *
 * @param atten is the attenuation (generally slightly less than 1).
 */

template <typename T>
static inline void firKaiserGen(T* coef, int L, int halfNumCoef,
        double stopBandAtten, double fcr, double atten) {
    const int N = L * halfNumCoef; // non-negative half
    const double beta = computeBeta(stopBandAtten);
    const double xstep = (2. * M_PI) * fcr / L;
    const double xfrac = 1. / N;
    const double yscale = atten * L / (I0(beta) * M_PI);
    const double sqrbeta = sqr(beta);

    // We use sine generators, which computes sines on regular step intervals.
    // This speeds up overall computation about 40% from computing the sine directly.

    SineGenGen sgg(0., xstep, L*xstep); // generates sine generators (one per polyphase)

    for (int i=0 ; i<=L ; ++i) { // generate an extra set of coefs for interpolation

        // computation for a single polyphase of the overall filter.
        SineGen sg = sgg.valueAdvance(); // current sine generator for "j" inner loop.
        double err = 0; // for noise shaping on int16_t coefficients (over each polyphase)

        for (int j=0, ix=i ; j<halfNumCoef ; ++j, ix+=L) {
            double y;
            if (CC_LIKELY(ix)) {
                double x = static_cast<double>(ix);

                // sine generator: sg.valueAdvance() returns sin(ix*xstep);
                // y = I0(beta * sqrt(1.0 - sqr(x * xfrac))) * yscale * sg.valueAdvance() / x;
                y = I0SqrRat(sqrbeta * (1.0 - sqr(x * xfrac)), yscale * sg.valueAdvance(), x);
            } else {
                y = 2. * atten * fcr; // center of filter, sinc(0) = 1.
                sg.advance();
            }

            if (std::is_same<T, int16_t>::value) { // int16_t needs noise shaping
                *coef++ = static_cast<T>(toint(y, 1ULL<<(sizeof(T)*8-1), err));
            } else if (std::is_same<T, int32_t>::value) {
                *coef++ = static_cast<T>(toint(y, 1ULL<<(sizeof(T)*8-1)));
            } else { // assumed float or double
                *coef++ = static_cast<T>(y);
            }
        }
    }
}

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_FIR_GEN_H*/
