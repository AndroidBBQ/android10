ProgramStoreCreate {
    direct
    param bool colorMaskR
    param bool colorMaskG
    param bool colorMaskB
    param bool colorMaskA
        param bool depthMask
        param bool ditherEnable
    param RsBlendSrcFunc srcFunc
    param RsBlendDstFunc destFunc
        param RsDepthFunc depthFunc
    ret RsProgramStore
    }

ProgramRasterCreate {
    direct
    param bool pointSprite
    param RsCullMode cull
    ret RsProgramRaster
}

ProgramBindConstants {
    param RsProgram vp
    param uint32_t slot
    param RsAllocation constants
    }


ProgramBindTexture {
    param RsProgramFragment pf
    param uint32_t slot
    param RsAllocation a
    }

ProgramBindSampler {
    param RsProgramFragment pf
    param uint32_t slot
    param RsSampler s
    }

ProgramFragmentCreate {
    direct
    param const char * shaderText
    param const char ** textureNames
    param const uintptr_t * params
    ret RsProgramFragment
    }

ProgramVertexCreate {
    direct
    param const char * shaderText
    param const char ** textureNames
    param const uintptr_t * params
    ret RsProgramVertex
    }

FontCreateFromFile {
    param const char *name
    param float fontSize
    param uint32_t dpi
    ret RsFont
    }

FontCreateFromMemory {
    param const char *name
    param float fontSize
    param uint32_t dpi
    param const void *data
    ret RsFont
    }

MeshCreate {
    param RsAllocation *vtx
    param RsAllocation *idx
    param uint32_t *primType
    ret RsMesh
    }

ContextBindProgramStore {
    param RsProgramStore pgm
    }

ContextBindProgramFragment {
    param RsProgramFragment pgm
    }

ContextBindProgramVertex {
    param RsProgramVertex pgm
    }

ContextBindProgramRaster {
    param RsProgramRaster pgm
    }

ContextBindFont {
    param RsFont pgm
    }

ContextSetSurface {
    param uint32_t width
    param uint32_t height
    param RsNativeWindow sur
        sync
    }

ContextBindRootScript {
    param RsScript sampler
    }

ContextPause {
    }

ContextResume {
    }
