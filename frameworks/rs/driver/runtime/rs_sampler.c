#include "rs_core.rsh"
#include "rs_structs.h"

/**
* Sampler
*/
extern rs_sampler_value __attribute__((overloadable))
        rsSamplerGetMinification(rs_sampler s) {
    Sampler_t *prog = (Sampler_t *)s.p;
    if (prog == NULL) {
        return RS_SAMPLER_INVALID;
    }
    return prog->mHal.state.minFilter;
}

extern rs_sampler_value __attribute__((overloadable))
        rsSamplerGetMagnification(rs_sampler s) {
    Sampler_t *prog = (Sampler_t *)s.p;
    if (prog == NULL) {
        return RS_SAMPLER_INVALID;
    }
    return prog->mHal.state.magFilter;
}

extern rs_sampler_value __attribute__((overloadable))
        rsSamplerGetWrapS(rs_sampler s) {
    Sampler_t *prog = (Sampler_t *)s.p;
    if (prog == NULL) {
        return RS_SAMPLER_INVALID;
    }
    return prog->mHal.state.wrapS;
}

extern rs_sampler_value __attribute__((overloadable))
        rsSamplerGetWrapT(rs_sampler s) {
    Sampler_t *prog = (Sampler_t *)s.p;
    if (prog == NULL) {
        return RS_SAMPLER_INVALID;
    }
    return prog->mHal.state.wrapT;
}

extern float __attribute__((overloadable))
        rsSamplerGetAnisotropy(rs_sampler s) {
    Sampler_t *prog = (Sampler_t *)s.p;
    if (prog == NULL) {
        return 0.0f;
    }
    return prog->mHal.state.aniso;
}
