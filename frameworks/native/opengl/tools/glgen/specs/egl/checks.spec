eglInitialize nullAllowed major nullAllowed minor check major 1 check minor 1
eglGetConfigs nullAllowed configs check configs config_size
eglChooseConfig nullAllowed configs check configs config_size check num_config 1 sentinel attrib_list EGL_NONE
eglGetConfigAttrib check value 1
//STUB function: //eglCreateWindowSurface nullAllowed attrib_list sentinel attrib_list EGL_NONE
eglCreatePbufferSurface nullAllowed attrib_list sentinel attrib_list EGL_NONE
//unsupported: eglCreatePixmapSurface nullAllowed attrib_list sentinel attrib_list EGL_NONE
eglCreatePixmapSurface unsupported
eglCopyBuffers unsupported
eglQuerySurface check value 1
//STUB function: eglCreatePbufferFromClientBuffer nullAllowed attrib_list sentinel attrib_list EGL_NONE
eglCreateContext sentinel attrib_list EGL_NONE
eglQueryContext check value 1
//unsupported: eglCreatePlatformPixmapSurface nullAllowed attrib_list sentinel attrib_list EGL_NONE
eglCreatePlatformPixmapSurface unsupported
