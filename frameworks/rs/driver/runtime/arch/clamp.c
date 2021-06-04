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

#include "rs_core.rsh"

typedef unsigned long long ull;
typedef unsigned long long ull2 __attribute__((ext_vector_type(2)));
typedef unsigned long long ull3 __attribute__((ext_vector_type(3)));
typedef unsigned long long ull4 __attribute__((ext_vector_type(4)));

#define S_CLAMP(T) \
extern T __attribute__((overloadable)) clamp(T amount, T low, T high) {             \
    return amount < low ? low : (amount > high ? high : amount);                    \
}

S_CLAMP(half);
//_CLAMP(float);  implemented in .ll
S_CLAMP(double);
S_CLAMP(char);
S_CLAMP(uchar);
S_CLAMP(short);
S_CLAMP(ushort);
S_CLAMP(int);
S_CLAMP(uint);
S_CLAMP(long);
S_CLAMP(ulong);

#undef S_CLAMP


                                                                                    \
#define V_CLAMP(T) \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T##2 low, T##2 high) { \
    T##2 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T##3 low, T##3 high) { \
    T##3 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    r.z = amount.z < low.z ? low.z : (amount.z > high.z ? high.z : amount.z);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T##4 low, T##4 high) { \
    T##4 r;                                                                         \
    r.x = amount.x < low.x ? low.x : (amount.x > high.x ? high.x : amount.x);       \
    r.y = amount.y < low.y ? low.y : (amount.y > high.y ? high.y : amount.y);       \
    r.z = amount.z < low.z ? low.z : (amount.z > high.z ? high.z : amount.z);       \
    r.w = amount.w < low.w ? low.w : (amount.w > high.w ? high.w : amount.w);       \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T low, T high) {       \
    T##2 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T low, T high) {       \
    T##3 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);               \
    return r;                                                                       \
}                                                                                   \
                                                                                    \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T low, T high) {       \
    T##4 r;                                                                         \
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);               \
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);               \
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);               \
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);               \
    return r;                                                                       \
}

V_CLAMP(half);
//V_CLAMP(float);  implemented in .ll
V_CLAMP(double);
V_CLAMP(char);
V_CLAMP(uchar);
V_CLAMP(short);
V_CLAMP(ushort);
#if !defined(ARCH_ARM_HAVE_NEON) && !defined (ARCH_ARM64_HAVE_NEON)
    V_CLAMP(int);  //implemented in .ll
    V_CLAMP(uint);  //implemented in .ll
#endif

V_CLAMP(long);
V_CLAMP(ulong);
V_CLAMP(ull);

#undef _CLAMP
