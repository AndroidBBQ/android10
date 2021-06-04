/* Implementations for copysign, ilogb, and nextafter for float16 based on
 * corresponding float32 implementations in
 * bionic/libm/upstream-freebsd/lib/msun/src
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "rs_core.rsh"
#include "rs_f16_util.h"

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_copysignf.c
extern half __attribute__((overloadable)) copysign(half x, half y) {
    short hx, hy;
    GET_HALF_WORD(hx, x);
    GET_HALF_WORD(hy, y);

    SET_HALF_WORD(x, (hx & 0x7fff) | (hy & 0x8000));
    return x;
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_frexpf.c
extern half __attribute__((overloadable)) frexp(half x, int *eptr) {
    short hx, ix;
    static const half two12 = 4096;

    GET_HALF_WORD(hx, x);
    ix = hx & 0x7fff;

    *eptr = 0;
    if (ix >= 0x7c00 || ix == 0) return x; // NaN, infinity or zero
    if (ix <= 0x0400) {
        // x is subnormal.  Scale it by 2^12 (and adjust eptr accordingly) so
        // that even the smallest subnormal value becomes normal.
        x *= two12;
        GET_HALF_WORD(hx, x);
        ix = hx & 0x7fff;
        *eptr = -12;
    }

    // Adjust eptr by (non-biased exponent of hx + 1).  Set the non-biased
    // exponent to be equal to -1 so that abs(hx) is between 0.5 and 1.
    *eptr += (ix >> 10) - 14;
    hx = (hx & 0x83ff) | 0x3800;
    SET_HALF_WORD(x, hx);
    return x;
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_ilogbf.c
extern int __attribute__((overloadable)) ilogb(half x) {
    const int RS_INT_MAX = 0x7fffffff;
    const int RS_INT_MIN = 0x80000000;

    short hx, ix;
    GET_HALF_WORD(hx, x);
    hx &= 0x7fff;

    if (hx < 0x0400) { // subnormal
        if (hx == 0)
            return RS_INT_MIN; // for zero
        for (hx <<= 5, ix = -14; hx > 0; hx <<= 1)
            ix -= 1;
        return ix;
    }
    else if (hx < 0x7c00) {
        return (hx >> 10) - 15;
    }
    else { // hx >= 0x7c00
        return RS_INT_MAX; // for NaN and infinity
    }
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_modff.c
extern half __attribute__((overloadable)) modf(half x, half *iptr) {
    short i0, j0;
    unsigned short i;
    GET_HALF_WORD(i0, x);
    j0 = ((i0 >> 10) & 0x1f) - 15; // exponent of x
    if (j0 < 10) {
        if (j0 < 0) { // No integral part
            SET_HALF_WORD(*iptr, i0 & 0x8000); // *iptr = +/- 0
            return x;
        }
        else {
            i = 0x03ff >> j0; // mask to check fractional parts of x
            if ((i0 & i) == 0) { // no bits set in fractional part
                *iptr = x;
                SET_HALF_WORD(x, i0 & 0x8000);
                return x;
            }
            else {
                SET_HALF_WORD(*iptr, i0 & ~i); // zero out fractional parts
                return x - *iptr;
            }
        }
    }
    else { // No fractional part
        unsigned short ix;
        *iptr = x;
        if (x != x)
            return x;
        GET_HALF_WORD(ix, x);
        SET_HALF_WORD(x, ix & 0x8000); // x = +/- 0
        return x;
    }
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_nextafterf.c
extern half __attribute__((overloadable)) nextafter(half x, half y) {
  short hx, hy, ix, iy;

  GET_HALF_WORD(hx, x);
  GET_HALF_WORD(hy, y);
  ix = hx & 0x7fff; // |x|
  iy = hy & 0x7fff; // |y|

  if ((ix > 0x7c00) || // x is nan
      (iy > 0x7c00))   // y is nan
    return x + y;      // return nan

  if (x == y) return y; // x == y.  return y
  if (ix == 0) {
    SET_HALF_WORD(x, (hy & 0x8000) | 1);
    return x;
  }

  if (hx >= 0) {  // x >= 0
    if (hx > hy)
      hx -= 1;    // x > y, x -= 1 ulp
    else
      hx += 1;    // x < y, x += 1 ulp
  }
  else {          // x < 0
    if (hy>= 0 || hx > hy)
      hx -= 1;    // x < y, x -= 1 ulp
    else
      hx += 1;
  }

  SET_HALF_WORD(x, hx);
  return x;
}
