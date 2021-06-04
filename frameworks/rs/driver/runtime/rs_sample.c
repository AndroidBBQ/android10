#include "rs_core.rsh"
#include "rs_structs.h"


// 565 Conversion bits taken from SkBitmap
#define SK_R16_BITS     5
#define SK_G16_BITS     6
#define SK_B16_BITS     5

#define SK_R16_SHIFT    (SK_B16_BITS + SK_G16_BITS)
#define SK_G16_SHIFT    (SK_B16_BITS)
#define SK_B16_SHIFT    0

#define SK_R16_MASK     ((1 << SK_R16_BITS) - 1)
#define SK_G16_MASK     ((1 << SK_G16_BITS) - 1)
#define SK_B16_MASK     ((1 << SK_B16_BITS) - 1)

#define SkGetPackedR16(color)   (((unsigned)(color) >> SK_R16_SHIFT) & SK_R16_MASK)
#define SkGetPackedG16(color)   (((unsigned)(color) >> SK_G16_SHIFT) & SK_G16_MASK)
#define SkGetPackedB16(color)   (((unsigned)(color) >> SK_B16_SHIFT) & SK_B16_MASK)

static inline unsigned SkR16ToR32(unsigned r) {
    return (r << (8 - SK_R16_BITS)) | (r >> (2 * SK_R16_BITS - 8));
}

static inline unsigned SkG16ToG32(unsigned g) {
    return (g << (8 - SK_G16_BITS)) | (g >> (2 * SK_G16_BITS - 8));
}

static inline unsigned SkB16ToB32(unsigned b) {
    return (b << (8 - SK_B16_BITS)) | (b >> (2 * SK_B16_BITS - 8));
}

#define SkPacked16ToR32(c)      SkR16ToR32(SkGetPackedR16(c))
#define SkPacked16ToG32(c)      SkG16ToG32(SkGetPackedG16(c))
#define SkPacked16ToB32(c)      SkB16ToB32(SkGetPackedB16(c))

static float3 getFrom565(uint16_t color) {
    float3 result;
    result.x = (float)SkPacked16ToR32(color);
    result.y = (float)SkPacked16ToG32(color);
    result.z = (float)SkPacked16ToB32(color);
    return result;
}

/**
* Allocation sampling
*/
static inline float __attribute__((overloadable))
        getElementAt1(const uint8_t *p, int32_t x) {
    float r = p[x];
    return r;
}

static inline float2 __attribute__((overloadable))
        getElementAt2(const uint8_t *p, int32_t x) {
    x *= 2;
    float2 r = {p[x], p[x+1]};
    return r;
}

static inline float3 __attribute__((overloadable))
        getElementAt3(const uint8_t *p, int32_t x) {
    x *= 4;
    float3 r = {p[x], p[x+1], p[x+2]};
    return r;
}

static inline float4 __attribute__((overloadable))
        getElementAt4(const uint8_t *p, int32_t x) {
    x *= 4;
    const uchar4 *p2 = (const uchar4 *)&p[x];
    return convert_float4(p2[0]);
}

static inline float3 __attribute__((overloadable))
        getElementAt565(const uint8_t *p, int32_t x) {
    x *= 2;
    float3 r = getFrom565(((const uint16_t *)p)[0]);
    return r;
}

static inline float __attribute__((overloadable))
        getElementAt1(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    float r = p[x];
    return r;
}

static inline float2 __attribute__((overloadable))
        getElementAt2(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 2;
    float2 r = {p[x], p[x+1]};
    return r;
}

static inline float3 __attribute__((overloadable))
        getElementAt3(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 4;
    float3 r = {p[x], p[x+1], p[x+2]};
    return r;
}

static inline float4 __attribute__((overloadable))
        getElementAt4(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 4;
    float4 r = {p[x], p[x+1], p[x+2], p[x+3]};
    return r;
}

static inline float3 __attribute__((overloadable))
        getElementAt565(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 2;
    float3 r = getFrom565(((const uint16_t *)p)[0]);
    return r;
}





static float4 __attribute__((overloadable))
            getSample_A(const uint8_t *p, int32_t iPixel,
                          int32_t next, float w0, float w1) {
    float p0 = getElementAt1(p, iPixel);
    float p1 = getElementAt1(p, next);
    float r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {0.f, 0.f, 0.f, r};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_L(const uint8_t *p, int32_t iPixel,
                          int32_t next, float w0, float w1) {
    float p0 = getElementAt1(p, iPixel);
    float p1 = getElementAt1(p, next);
    float r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r, r, r, 1.f};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_LA(const uint8_t *p, int32_t iPixel,
                           int32_t next, float w0, float w1) {
    float2 p0 = getElementAt2(p, iPixel);
    float2 p1 = getElementAt2(p, next);
    float2 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.x, r.y};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_RGB(const uint8_t *p, int32_t iPixel,
                            int32_t next, float w0, float w1) {
    float3 p0 = getElementAt3(p, iPixel);
    float3 p1 = getElementAt3(p, next);
    float3 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.z, 1.f};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_565(const uint8_t *p, int32_t iPixel,
                           int32_t next, float w0, float w1) {
    float3 p0 = getElementAt565(p, iPixel);
    float3 p1 = getElementAt565(p, next);
    float3 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.z, 1.f};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_RGBA(const uint8_t *p, int32_t iPixel,
                             int32_t next, float w0, float w1) {
    float4 p0 = getElementAt4(p, iPixel);
    float4 p1 = getElementAt4(p, next);
    float4 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    return r;
}


static float4 __attribute__((overloadable))
            getSample_A(const uint8_t *p, size_t stride,
                          int locX, int locY, int nextX, int nextY,
                          float w0, float w1, float w2, float w3) {
    float p0 = getElementAt1(p, stride, locX, locY);
    float p1 = getElementAt1(p, stride, nextX, locY);
    float p2 = getElementAt1(p, stride, locX, nextY);
    float p3 = getElementAt1(p, stride, nextX, nextY);
    float r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {0.f, 0.f, 0.f, r};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_L(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float p0 = getElementAt1(p, stride, locX, locY);
    float p1 = getElementAt1(p, stride, nextX, locY);
    float p2 = getElementAt1(p, stride, locX, nextY);
    float p3 = getElementAt1(p, stride, nextX, nextY);
    float r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r, r, r, 1.f};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_LA(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float2 p0 = getElementAt2(p, stride, locX, locY);
    float2 p1 = getElementAt2(p, stride, nextX, locY);
    float2 p2 = getElementAt2(p, stride, locX, nextY);
    float2 p3 = getElementAt2(p, stride, nextX, nextY);
    float2 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.x, r.y};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_RGB(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float4 p0 = getElementAt4(p, stride, locX, locY);
    float4 p1 = getElementAt4(p, stride, nextX, locY);
    float4 p2 = getElementAt4(p, stride, locX, nextY);
    float4 p3 = getElementAt4(p, stride, nextX, nextY);
    float4 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.y, r.z, 1.f};
    return ret;
}
static float4 __attribute__((overloadable))
            getSample_RGBA(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float4 p0 = getElementAt4(p, stride, locX, locY);
    float4 p1 = getElementAt4(p, stride, nextX, locY);
    float4 p2 = getElementAt4(p, stride, locX, nextY);
    float4 p3 = getElementAt4(p, stride, nextX, nextY);
    float4 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    return r;
}
static float4 __attribute__((overloadable))
            getSample_565(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float3 p0 = getElementAt565(p, stride, locX, locY);
    float3 p1 = getElementAt565(p, stride, nextX, locY);
    float3 p2 = getElementAt565(p, stride, locX, nextY);
    float3 p3 = getElementAt565(p, stride, nextX, nextY);
    float3 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret;
    ret.rgb = r;
    ret.w = 1.f;
    return ret;
}

static float4 __attribute__((overloadable))
        getBilinearSample1D(const Allocation_t *alloc, float2 weights,
                          uint32_t iPixel, uint32_t next,
                          rs_data_kind dk, rs_data_type dt, uint32_t lod) {

     const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;

     switch(dk) {
     case RS_KIND_PIXEL_RGBA:
         return getSample_RGBA(p, iPixel, next, weights.x, weights.y);
     case RS_KIND_PIXEL_A:
         return getSample_A(p, iPixel, next, weights.x, weights.y);
     case RS_KIND_PIXEL_RGB:
         if (dt == RS_TYPE_UNSIGNED_5_6_5) {
             return getSample_565(p, iPixel, next, weights.x, weights.y);
         }
         return getSample_RGB(p, iPixel, next, weights.x, weights.y);
     case RS_KIND_PIXEL_L:
         return getSample_L(p, iPixel, next, weights.x, weights.y);
     case RS_KIND_PIXEL_LA:
         return getSample_LA(p, iPixel, next, weights.x, weights.y);

     default:
         //__builtin_unreachable();
         break;
     }

     //__builtin_unreachable();
     return 0.f;
}

static uint32_t wrapI(rs_sampler_value wrap, int32_t coord, int32_t size) {
    if (wrap == RS_SAMPLER_WRAP) {
        coord = coord % size;
        if (coord < 0) {
            coord += size;
        }
    }
    if (wrap == RS_SAMPLER_MIRRORED_REPEAT) {
        coord = coord % (size * 2);
        if (coord < 0) {
            coord = (size * 2) + coord;
        }
        if (coord >= size) {
            coord = (size * 2 - 1) - coord;
        }
    }
    return (uint32_t)max(0, min(coord, size - 1));
}

static float4 __attribute__((overloadable))
        getBilinearSample2D(const Allocation_t *alloc, float w0, float w1, float w2, float w3,
                          int lx, int ly, int nx, int ny,
                          rs_data_kind dk, rs_data_type dt, uint32_t lod) {

    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;
    size_t stride = alloc->mHal.drvState.lod[lod].stride;

    switch(dk) {
    case RS_KIND_PIXEL_RGBA:
        return getSample_RGBA(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);
    case RS_KIND_PIXEL_A:
        return getSample_A(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);
    case RS_KIND_PIXEL_LA:
        return getSample_LA(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);
    case RS_KIND_PIXEL_RGB:
        if (dt == RS_TYPE_UNSIGNED_5_6_5) {
            return getSample_565(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);
        }
        return getSample_RGB(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);
    case RS_KIND_PIXEL_L:
        return getSample_L(p, stride, lx, ly, nx, ny, w0, w1, w2, w3);

    default:
        break;
    }

    return 0.f;
}

static float4  __attribute__((overloadable))
        getNearestSample(const Allocation_t *alloc, uint32_t iPixel, rs_data_kind dk,
                         rs_data_type dt, uint32_t lod) {

    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;

    float4 result = {0.f, 0.f, 0.f, 255.f};

    switch(dk) {
    case RS_KIND_PIXEL_RGBA:
        result = getElementAt4(p, iPixel);
        break;
    case RS_KIND_PIXEL_A:
        result.w = getElementAt1(p, iPixel);
        break;
    case RS_KIND_PIXEL_LA:
        result.zw = getElementAt2(p, iPixel);
        result.xy = result.z;
        break;
    case RS_KIND_PIXEL_RGB:
        if (dt == RS_TYPE_UNSIGNED_5_6_5) {
            result.xyz = getElementAt565(p, iPixel);
        } else {
            result.xyz = getElementAt3(p, iPixel);
        }
        break;
    case RS_KIND_PIXEL_L:
        result.xyz = getElementAt1(p, iPixel);

    default:
        //__builtin_unreachable();
        break;
    }

    return result * 0.003921569f;
}

static float4  __attribute__((overloadable))
        getNearestSample(const Allocation_t *alloc, uint2 iPixel, rs_data_kind dk,
                         rs_data_type dt, uint32_t lod) {

    const uint8_t *p = (const uint8_t *)alloc->mHal.drvState.lod[lod].mallocPtr;
    size_t stride = alloc->mHal.drvState.lod[lod].stride;

    float4 result = {0.f, 0.f, 0.f, 255.f};

    switch(dk) {
    case RS_KIND_PIXEL_RGBA:
        result = getElementAt4(p, stride, iPixel.x, iPixel.y);
        break;
    case RS_KIND_PIXEL_A:
        result.w = getElementAt1(p, stride, iPixel.x, iPixel.y);
        break;
    case RS_KIND_PIXEL_LA:
        result.zw = getElementAt2(p, stride, iPixel.x, iPixel.y);
        result.xy = result.z;
        break;
    case RS_KIND_PIXEL_RGB:
        if (dt == RS_TYPE_UNSIGNED_5_6_5) {
            result.xyz = getElementAt565(p, stride, iPixel.x, iPixel.y);
        } else {
            result.xyz = getElementAt3(p, stride, iPixel.x, iPixel.y);
        }
        break;

    default:
        //__builtin_unreachable();
        break;
    }

    return result * 0.003921569f;
}

static float4 __attribute__((overloadable))
        sample_LOD_LinearPixel(const Allocation_t *alloc,
                               rs_data_kind dk, rs_data_type dt,
                               rs_sampler_value wrapS,
                               float uv, uint32_t lod) {

    int32_t sourceW = alloc->mHal.drvState.lod[lod].dimX;
    float pixelUV = uv * (float)(sourceW);
    int32_t iPixel = floor(pixelUV);
    float frac = pixelUV - (float)iPixel;

    if (frac < 0.5f) {
        iPixel -= 1;
        frac += 0.5f;
    } else {
        frac -= 0.5f;
    }

    float oneMinusFrac = 1.0f - frac;

    float2 weights;
    weights.x = oneMinusFrac;
    weights.y = frac;

    uint32_t next = wrapI(wrapS, iPixel + 1, sourceW);
    uint32_t location = wrapI(wrapS, iPixel, sourceW);

    return getBilinearSample1D(alloc, weights, location, next, dk, dt, lod);
}

static float4 __attribute__((overloadable))
        sample_LOD_NearestPixel(const Allocation_t *alloc,
                                rs_data_kind dk, rs_data_type dt,
                                rs_sampler_value wrapS,
                                float uv, uint32_t lod) {

    int32_t sourceW = alloc->mHal.drvState.lod[lod].dimX;
    int32_t iPixel = floor(uv * (float)(sourceW));
    uint32_t location = wrapI(wrapS, iPixel, sourceW);

    return getNearestSample(alloc, location, dk, dt, lod);
}

static float4 __attribute__((overloadable))
        sample_LOD_LinearPixel(const Allocation_t *alloc,
                               rs_data_kind dk, rs_data_type dt,
                               rs_sampler_value wrapS,
                               rs_sampler_value wrapT,
                               float2 uv, uint32_t lod) {

    int sourceW = alloc->mHal.drvState.lod[lod].dimX;
    int sourceH = alloc->mHal.drvState.lod[lod].dimY;

    float pixelU = uv.x * sourceW;
    float pixelV = uv.y * sourceH;
    int iPixelU = floor(pixelU);
    int iPixelV = floor(pixelV);
    float fracU = pixelU - iPixelU;
    float fracV = pixelV - iPixelV;

    if (fracU < 0.5f) {
        iPixelU -= 1;
        fracU += 0.5f;
    } else {
        fracU -= 0.5f;
    }
    if (fracV < 0.5f) {
        iPixelV -= 1;
        fracV += 0.5f;
    } else {
        fracV -= 0.5f;
    }
    float oneMinusFracU = 1.0f - fracU;
    float oneMinusFracV = 1.0f - fracV;

    float w0 = oneMinusFracU * oneMinusFracV;
    float w1 = fracU * oneMinusFracV;
    float w2 = oneMinusFracU * fracV;
    float w3 = fracU * fracV;

    int nx = wrapI(wrapS, iPixelU + 1, sourceW);
    int ny = wrapI(wrapT, iPixelV + 1, sourceH);
    int lx = wrapI(wrapS, iPixelU, sourceW);
    int ly = wrapI(wrapT, iPixelV, sourceH);

    return getBilinearSample2D(alloc, w0, w1, w2, w3, lx, ly, nx, ny, dk, dt, lod);

}

static float4 __attribute__((overloadable))
        sample_LOD_NearestPixel(const Allocation_t *alloc,
                                rs_data_kind dk, rs_data_type dt,
                                rs_sampler_value wrapS,
                                rs_sampler_value wrapT,
                                float2 uv, uint32_t lod) {
    int sourceW = alloc->mHal.drvState.lod[lod].dimX;
    int sourceH = alloc->mHal.drvState.lod[lod].dimY;

    float2 dimF;
    dimF.x = (float)(sourceW);
    dimF.y = (float)(sourceH);
    int2 iPixel = convert_int2(floor(uv * dimF));

    uint2 location;
    location.x = wrapI(wrapS, iPixel.x, sourceW);
    location.y = wrapI(wrapT, iPixel.y, sourceH);
    return getNearestSample(alloc, location, dk, dt, lod);
}

extern float4 __attribute__((overloadable))
        rsSample(rs_allocation a, rs_sampler s, float uv, float lod) {

    const Allocation_t *alloc = (const Allocation_t *)a.p;
    const Sampler_t *prog = (Sampler_t *)s.p;
    const Type_t *type = (Type_t *)alloc->mHal.state.type;
    const Element_t *elem = type->mHal.state.element;
    rs_data_kind dk = elem->mHal.state.dataKind;
    rs_data_type dt = elem->mHal.state.dataType;
    rs_sampler_value sampleMin = prog->mHal.state.minFilter;
    rs_sampler_value sampleMag = prog->mHal.state.magFilter;
    rs_sampler_value wrapS = prog->mHal.state.wrapS;

    if (!(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE)) {
        return 0.f;
    }

    if (lod <= 0.0f) {
        if (sampleMag == RS_SAMPLER_NEAREST) {
            return sample_LOD_NearestPixel(alloc, dk, dt, wrapS, uv, 0);
        }
        return sample_LOD_LinearPixel(alloc, dk, dt, wrapS, uv, 0);
    }

    if (sampleMin == RS_SAMPLER_LINEAR_MIP_NEAREST) {
        uint32_t maxLOD = type->mHal.state.lodCount - 1;
        lod = min(lod, (float)maxLOD);
        uint32_t nearestLOD = (uint32_t)round(lod);
        return sample_LOD_LinearPixel(alloc, dk, dt, wrapS, uv, nearestLOD);
    }

    if (sampleMin == RS_SAMPLER_LINEAR_MIP_LINEAR) {
        uint32_t lod0 = (uint32_t)floor(lod);
        uint32_t lod1 = (uint32_t)ceil(lod);
        uint32_t maxLOD = type->mHal.state.lodCount - 1;
        lod0 = min(lod0, maxLOD);
        lod1 = min(lod1, maxLOD);
        float4 sample0 = sample_LOD_LinearPixel(alloc, dk, dt, wrapS, uv, lod0);
        float4 sample1 = sample_LOD_LinearPixel(alloc, dk, dt, wrapS, uv, lod1);
        float frac = lod - (float)lod0;
        return sample0 * (1.0f - frac) + sample1 * frac;
    }

    return sample_LOD_NearestPixel(alloc, dk, dt, wrapS, uv, 0);
}

extern float4 __attribute__((overloadable))
        rsSample(rs_allocation a, rs_sampler s, float location) {
    return rsSample(a, s, location, 0);
}


extern float4 __attribute__((overloadable))
        rsSample(rs_allocation a, rs_sampler s, float2 uv, float lod) {

    const Allocation_t *alloc = (const Allocation_t *)a.p;
    const Sampler_t *prog = (Sampler_t *)s.p;
    const Type_t *type = (Type_t *)alloc->mHal.state.type;
    const Element_t *elem = type->mHal.state.element;
    rs_data_kind dk = elem->mHal.state.dataKind;
    rs_data_type dt = elem->mHal.state.dataType;
    rs_sampler_value sampleMin = prog->mHal.state.minFilter;
    rs_sampler_value sampleMag = prog->mHal.state.magFilter;
    rs_sampler_value wrapS = prog->mHal.state.wrapS;
    rs_sampler_value wrapT = prog->mHal.state.wrapT;

    if (!(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE)) {
        return 0.f;
    }

    if (lod <= 0.0f) {
        if (sampleMag == RS_SAMPLER_NEAREST) {
            return sample_LOD_NearestPixel(alloc, dk, dt, wrapS, wrapT, uv, 0);
        }
        return sample_LOD_LinearPixel(alloc, dk, dt, wrapS, wrapT, uv, 0);
    }

    if (sampleMin == RS_SAMPLER_LINEAR_MIP_NEAREST) {
        uint32_t maxLOD = type->mHal.state.lodCount - 1;
        lod = min(lod, (float)maxLOD);
        uint32_t nearestLOD = (uint32_t)round(lod);
        return sample_LOD_LinearPixel(alloc, dk, dt, wrapS, wrapT, uv, nearestLOD);
    }

    if (sampleMin == RS_SAMPLER_LINEAR_MIP_LINEAR) {
        uint32_t lod0 = (uint32_t)floor(lod);
        uint32_t lod1 = (uint32_t)ceil(lod);
        uint32_t maxLOD = type->mHal.state.lodCount - 1;
        lod0 = min(lod0, maxLOD);
        lod1 = min(lod1, maxLOD);
        float4 sample0 = sample_LOD_LinearPixel(alloc, dk, dt, wrapS, wrapT, uv, lod0);
        float4 sample1 = sample_LOD_LinearPixel(alloc, dk, dt, wrapS, wrapT, uv, lod1);
        float frac = lod - (float)lod0;
        return sample0 * (1.0f - frac) + sample1 * frac;
    }

    return sample_LOD_NearestPixel(alloc, dk, dt, wrapS, wrapT, uv, 0);
}

extern float4 __attribute__((overloadable))
        rsSample(rs_allocation a, rs_sampler s, float2 uv) {

    const Allocation_t *alloc = (const Allocation_t *)a.p;
    const Sampler_t *prog = (Sampler_t *)s.p;
    const Type_t *type = (Type_t *)alloc->mHal.state.type;
    const Element_t *elem = type->mHal.state.element;
    rs_data_kind dk = elem->mHal.state.dataKind;
    rs_data_type dt = elem->mHal.state.dataType;
    rs_sampler_value wrapS = prog->mHal.state.wrapS;
    rs_sampler_value wrapT = prog->mHal.state.wrapT;

    if (!(alloc->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE)) {
        return 0.f;
    }

    if (prog->mHal.state.magFilter == RS_SAMPLER_NEAREST) {
        return sample_LOD_NearestPixel(alloc, dk, dt, wrapS, wrapT, uv, 0);
    }
    return sample_LOD_LinearPixel(alloc, dk, dt, wrapS, wrapT, uv, 0);
}
