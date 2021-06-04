/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define ATRACE_TAG ATRACE_TAG_ALWAYS

#include <gui/Surface.h>
#include <gui/SurfaceControl.h>
#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <ui/Fence.h>
#include <utils/Trace.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <math.h>
#include <getopt.h>

#include "Flatland.h"
#include "GLHelper.h"

using namespace ::android;

static uint32_t g_SleepBetweenSamplesMs = 0;
static bool     g_PresentToWindow       = false;
static size_t   g_BenchmarkNameLen      = 0;

struct BenchmarkDesc {
    // The name of the test.
    const char* name;

    // The dimensions of the space in which window layers are specified.
    uint32_t width;
    uint32_t height;

    // The screen heights at which to run the test.
    uint32_t runHeights[MAX_TEST_RUNS];

    // The list of window layers.
    LayerDesc layers[MAX_NUM_LAYERS];
};

static const BenchmarkDesc benchmarks[] = {
    { "16:10 Single Static Window",
        2560, 1600, { 800, 1200, 1600, 2400 },
        {
            {   // Window
                0, staticGradient, opaque,
                0,    50,     2560,   1454,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2560,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1504,   2560,   96,
            },
        },
    },

    { "4:3 Single Static Window",
        2048, 1536, { 1536 },
        {
            {   // Window
                0, staticGradient, opaque,
                0,    50,     2048,   1440,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2048,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1440,   2048,   96,
            },
        },
    },

    { "16:10 App -> Home Transition",
        2560, 1600, { 800, 1200, 1600, 2400 },
        {
            {   // Wallpaper
                0, staticGradient, opaque,
                0,    50,     2560,   1454,
            },
            {   // Launcher
                0, staticGradient, blend,
                0,    50,     2560,   1454,
            },
            {   // Outgoing activity
                0, staticGradient, blendShrink,
                20,    70,     2520,   1414,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2560,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1504,   2560,   96,
            },
        },
    },

    { "4:3 App -> Home Transition",
        2048, 1536, { 1536 },
        {
            {   // Wallpaper
                0, staticGradient, opaque,
                0,    50,     2048,   1440,
            },
            {   // Launcher
                0, staticGradient, blend,
                0,    50,     2048,   1440,
            },
            {   // Outgoing activity
                0, staticGradient, blendShrink,
                20,    70,     2048,   1400,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2048,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1440,   2048,   96,
            },
        },
    },

    { "16:10 SurfaceView -> Home Transition",
        2560, 1600, { 800, 1200, 1600, 2400 },
        {
            {   // Wallpaper
                0, staticGradient, opaque,
                0,    50,     2560,   1454,
            },
            {   // Launcher
                0, staticGradient, blend,
                0,    50,     2560,   1454,
            },
            {   // Outgoing SurfaceView
                0, staticGradient, blendShrink,
                20,    70,     2520,   1414,
            },
            {   // Outgoing activity
                0, staticGradient, blendShrink,
                20,    70,     2520,   1414,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2560,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1504,   2560,   96,
            },
        },
    },

    { "4:3 SurfaceView -> Home Transition",
        2048, 1536, { 1536 },
        {
            {   // Wallpaper
                0, staticGradient, opaque,
                0,    50,     2048,   1440,
            },
            {   // Launcher
                0, staticGradient, blend,
                0,    50,     2048,   1440,
            },
            {   // Outgoing SurfaceView
                0, staticGradient, blendShrink,
                20,    70,     2048,   1400,
            },
            {   // Outgoing activity
                0, staticGradient, blendShrink,
                20,    70,     2048,   1400,
            },
            {   // Status bar
                0, staticGradient, opaque,
                0,    0,      2048,   50,
            },
            {   // Navigation bar
                0, staticGradient, opaque,
                0,    1440,   2048,   96,
            },
        },
    },
};

static const ShaderDesc shaders[] = {
    {
        .name="Blit",
        .vertexShader={
            "precision mediump float;",
            "",
            "attribute vec4 position;",
            "attribute vec4 uv;",
            "",
            "varying vec4 texCoords;",
            "",
            "uniform mat4 objToNdc;",
            "uniform mat4 uvToTex;",
            "",
            "void main() {",
            "    gl_Position = objToNdc * position;",
            "    texCoords = uvToTex * uv;",
            "}",
        },
        .fragmentShader={
            "#extension GL_OES_EGL_image_external : require",
            "precision mediump float;",
            "",
            "varying vec4 texCoords;",
            "",
            "uniform samplerExternalOES blitSrc;",
            "uniform vec4 modColor;",
            "",
            "void main() {",
            "    gl_FragColor = texture2D(blitSrc, texCoords.xy);",
            "    gl_FragColor *= modColor;",
            "}",
        },
    },

    {
        .name="Gradient",
        .vertexShader={
            "precision mediump float;",
            "",
            "attribute vec4 position;",
            "attribute vec4 uv;",
            "",
            "varying float interp;",
            "",
            "uniform mat4 objToNdc;",
            "uniform mat4 uvToInterp;",
            "",
            "void main() {",
            "    gl_Position = objToNdc * position;",
            "    interp = (uvToInterp * uv).x;",
            "}",
        },
        .fragmentShader={
            "precision mediump float;",
            "",
            "varying float interp;",
            "",
            "uniform vec4 color0;",
            "uniform vec4 color1;",
            "",
            "uniform sampler2D ditherKernel;",
            "uniform float invDitherKernelSize;",
            "uniform float invDitherKernelSizeSq;",
            "",
            "void main() {",
            "    float dither = texture2D(ditherKernel,",
            "            gl_FragCoord.xy * invDitherKernelSize).a;",
            "    dither *= invDitherKernelSizeSq;",
            "    vec4 color = mix(color0, color1, clamp(interp, 0.0, 1.0));",
            "    gl_FragColor = color + vec4(dither, dither, dither, 0.0);",
            "}",
        },
    },
};

class Layer {

public:

    Layer() :
        mGLHelper(nullptr),
        mSurface(EGL_NO_SURFACE) {
    }

    bool setUp(const LayerDesc& desc, GLHelper* helper) {
        bool result;

        mDesc = desc;
        mGLHelper = helper;

        result = mGLHelper->createSurfaceTexture(mDesc.width, mDesc.height,
                &mGLConsumer, &mSurface, &mTexName);
        if (!result) {
            return false;
        }

        mRenderer = desc.rendererFactory();
        result = mRenderer->setUp(helper);
        if (!result) {
            return false;
        }

        mComposer = desc.composerFactory();
        result = mComposer->setUp(desc, helper);
        if (!result) {
            return false;
        }

        return true;
    }

    void tearDown() {
        if (mComposer != nullptr) {
            mComposer->tearDown();
            delete mComposer;
            mComposer = nullptr;
        }

        if (mRenderer != nullptr) {
            mRenderer->tearDown();
            delete mRenderer;
            mRenderer = nullptr;
        }

        if (mSurface != EGL_NO_SURFACE) {
            mGLHelper->destroySurface(&mSurface);
            mGLConsumer->abandon();
        }
        mGLHelper = nullptr;
        mGLConsumer.clear();
    }

    bool render() {
        return mRenderer->render(mSurface);
    }

    bool prepareComposition() {
        status_t err;

        err = mGLConsumer->updateTexImage();
        if (err < 0) {
            fprintf(stderr, "GLConsumer::updateTexImage error: %d\n", err);
            return false;
        }

        return true;
    }

    bool compose() {
        return mComposer->compose(mTexName, mGLConsumer);
    }

private:
    LayerDesc mDesc;

    GLHelper* mGLHelper;

    GLuint mTexName;
    sp<GLConsumer> mGLConsumer;
    EGLSurface mSurface;

    Renderer* mRenderer;
    Composer* mComposer;
};

class BenchmarkRunner {

public:

    BenchmarkRunner(const BenchmarkDesc& desc, size_t instance) :
        mDesc(desc),
        mInstance(instance),
        mNumLayers(countLayers(desc)),
        mGLHelper(nullptr),
        mSurface(EGL_NO_SURFACE),
        mWindowSurface(EGL_NO_SURFACE) {
    }

    bool setUp() {
        ATRACE_CALL();

        bool result;

        float scaleFactor = float(mDesc.runHeights[mInstance]) /
            float(mDesc.height);
        uint32_t w = uint32_t(scaleFactor * float(mDesc.width));
        uint32_t h = mDesc.runHeights[mInstance];

        mGLHelper = new GLHelper();
        result = mGLHelper->setUp(shaders, NELEMS(shaders));
        if (!result) {
            return false;
        }

        GLuint texName;
        result = mGLHelper->createSurfaceTexture(w, h, &mGLConsumer, &mSurface,
                &texName);
        if (!result) {
            return false;
        }

        for (size_t i = 0; i < mNumLayers; i++) {
            // Scale the layer to match the current screen size.
            LayerDesc ld = mDesc.layers[i];
            ld.x = int32_t(scaleFactor * float(ld.x));
            ld.y = int32_t(scaleFactor * float(ld.y));
            ld.width = uint32_t(scaleFactor * float(ld.width));
            ld.height = uint32_t(scaleFactor * float(ld.height));

            // Set up the layer.
            result = mLayers[i].setUp(ld, mGLHelper);
            if (!result) {
                return false;
            }
        }

        if (g_PresentToWindow) {
            result = mGLHelper->createWindowSurface(w, h, &mSurfaceControl,
                    &mWindowSurface);
            if (!result) {
                return false;
            }

            result = doFrame(mWindowSurface);
            if (!result) {
                return false;
            }
        }

        return true;
    }

    void tearDown() {
        ATRACE_CALL();

        for (size_t i = 0; i < mNumLayers; i++) {
            mLayers[i].tearDown();
        }

        if (mGLHelper != nullptr) {
            if (mWindowSurface != EGL_NO_SURFACE) {
                mGLHelper->destroySurface(&mWindowSurface);
            }
            mGLHelper->destroySurface(&mSurface);
            mGLConsumer->abandon();
            mGLConsumer.clear();
            mSurfaceControl.clear();
            mGLHelper->tearDown();
            delete mGLHelper;
            mGLHelper = nullptr;
        }
    }

    nsecs_t run(uint32_t warmUpFrames, uint32_t totalFrames) {
        ATRACE_CALL();

        bool result;

        resetColorGenerator();

        // Do the warm-up frames.
        for (uint32_t i = 0; i < warmUpFrames; i++) {
            result = doFrame(mSurface);
            if (!result) {
                return -1;
            }
        }

        // Grab the fence for the start timestamp.
        sp<Fence> startFence = mGLConsumer->getCurrentFence();

        //  the timed frames.
        for (uint32_t i = warmUpFrames; i < totalFrames; i++) {
            result = doFrame(mSurface);
            if (!result) {
                return -1;
            }
        }

        // Grab the fence for the end timestamp.
        sp<Fence> endFence = mGLConsumer->getCurrentFence();

        // Keep doing frames until the end fence has signaled.
        while (endFence->wait(0) == -ETIME) {
            result = doFrame(mSurface);
            if (!result) {
                return -1;
            }
        }

        // Compute the time delta.
        nsecs_t startTime = startFence->getSignalTime();
        nsecs_t endTime = endFence->getSignalTime();

        return endTime - startTime;
    }

private:

    bool doFrame(EGLSurface surface) {
        bool result;
        status_t err;

        for (size_t i = 0; i < mNumLayers; i++) {
            result = mLayers[i].render();
            if (!result) {
                return false;
            }
        }

        for (size_t i = 0; i < mNumLayers; i++) {
            result = mLayers[i].prepareComposition();
            if (!result) {
                return false;
            }
        }

        result = mGLHelper->makeCurrent(surface);
        if (!result) {
            return false;
        }

        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < mNumLayers; i++) {
            result = mLayers[i].compose();
            if (!result) {
                return false;
            }
        }

        result = mGLHelper->swapBuffers(surface);
        if (!result) {
            return false;
        }

        err = mGLConsumer->updateTexImage();
        if (err < 0) {
            fprintf(stderr, "GLConsumer::updateTexImage error: %d\n", err);
            return false;
        }

        return true;
    }

    static size_t countLayers(const BenchmarkDesc& desc) {
        size_t i;
        for (i = 0; i < MAX_NUM_LAYERS; i++) {
            if (desc.layers[i].rendererFactory == nullptr) {
                break;
            }
        }
        return i;
    }

    const BenchmarkDesc& mDesc;
    const size_t mInstance;
    const size_t mNumLayers;

    GLHelper* mGLHelper;

    // The surface into which layers are composited
    sp<GLConsumer> mGLConsumer;
    EGLSurface mSurface;

    // Used for displaying the surface to a window.
    EGLSurface mWindowSurface;
    sp<SurfaceControl> mSurfaceControl;

    Layer mLayers[MAX_NUM_LAYERS];
};

static int cmpDouble(const double* lhs, const double* rhs) {
    if (*lhs < *rhs) {
        return -1;
    } else if (*rhs < *lhs) {
        return 1;
    }
    return 0;
}

// Run a single benchmark and print the result.
static bool runTest(const BenchmarkDesc b, size_t run) {
    bool success = true;
    double prevResult = 0.0, result = 0.0;
    Vector<double> samples;

    uint32_t runHeight = b.runHeights[run];
    uint32_t runWidth = b.width * runHeight / b.height;
    printf(" %-*s | %4d x %4d | ", static_cast<int>(g_BenchmarkNameLen), b.name,
            runWidth, runHeight);
    fflush(stdout);

    BenchmarkRunner r(b, run);
    if (!r.setUp()) {
        fprintf(stderr, "error initializing runner.\n");
        return false;
    }

    // The slowest 1/outlierFraction sample results are ignored as potential
    // outliers.
    const uint32_t outlierFraction = 16;
    const double threshold = .0025;

    uint32_t warmUpFrames = 1;
    uint32_t totalFrames = 5;

    // Find the number of frames needed to run for over 100ms.
    double runTime = 0.0;
    while (true) {
        runTime = double(r.run(warmUpFrames, totalFrames));
        if (runTime < 50e6) {
            warmUpFrames *= 2;
            totalFrames *= 2;
        } else {
            break;
        }
    }


    if (totalFrames - warmUpFrames > 16) {
        // The test runs too fast to get a stable result.  Skip it.
        printf("  fast");
        goto done;
    } else if (totalFrames == 5 && runTime > 200e6) {
        // The test runs too slow to be very useful.  Skip it.
        printf("  slow");
        goto done;
    }

    do {
        size_t newSamples = samples.size();
        if (newSamples == 0) {
            newSamples = 4*outlierFraction;
        }

        if (newSamples > 512) {
            printf("varies");
            goto done;
        }

        for (size_t i = 0; i < newSamples; i++) {
            double sample = double(r.run(warmUpFrames, totalFrames));

            if (g_SleepBetweenSamplesMs > 0) {
                usleep(g_SleepBetweenSamplesMs  * 1000);
            }

            if (sample < 0.0) {
                success = false;
                goto done;
            }

            samples.add(sample);
        }

        samples.sort(cmpDouble);

        prevResult = result;
        size_t elem = (samples.size() * (outlierFraction-1) / outlierFraction);
        result = (samples[elem-1] + samples[elem]) * 0.5;
    } while (fabs(result - prevResult) > threshold * result);

    printf("%6.3f", result / double(totalFrames - warmUpFrames) / 1e6);

done:

    printf("\n");
    fflush(stdout);
    r.tearDown();

    return success;
}

static void printResultsTableHeader() {
    const char* scenario = "Scenario";
    size_t len = strlen(scenario);
    size_t leftPad = (g_BenchmarkNameLen - len) / 2;
    size_t rightPad = g_BenchmarkNameLen - len - leftPad;
    printf(" %*s%s%*s | Resolution  | Time (ms)\n",
            static_cast<int>(leftPad), "",
            "Scenario", static_cast<int>(rightPad), "");
}

// Run ALL the benchmarks!
static bool runTests() {
    printResultsTableHeader();

    for (size_t i = 0; i < NELEMS(benchmarks); i++) {
        const BenchmarkDesc& b = benchmarks[i];
        for (size_t j = 0; j < MAX_TEST_RUNS && b.runHeights[j]; j++) {
            if (!runTest(b, j)) {
                return false;
            }
        }
    }
    return true;
}

// Return the length longest benchmark name.
static size_t maxBenchmarkNameLen() {
    size_t maxLen = 0;
    for (size_t i = 0; i < NELEMS(benchmarks); i++) {
        const BenchmarkDesc& b = benchmarks[i];
        size_t len = strlen(b.name);
        if (len > maxLen) {
            maxLen = len;
        }
    }
    return maxLen;
}

// Print the command usage help to stderr.
static void showHelp(const char *cmd) {
    fprintf(stderr, "usage: %s [options]\n", cmd);
    fprintf(stderr, "options include:\n"
                    "  -s N            sleep for N ms between samples\n"
                    "  -d              display the test frame to a window\n"
                    "  --help          print this helpful message and exit\n"
            );
}

int main(int argc, char** argv) {
    if (argc == 2 && 0 == strcmp(argv[1], "--help")) {
        showHelp(argv[0]);
        exit(0);
    }

    for (;;) {
        int ret;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",     no_argument, 0,  0 },
            {     0,               0, 0,  0 }
        };

        ret = getopt_long(argc, argv, "ds:",
                          long_options, &option_index);

        if (ret < 0) {
            break;
        }

        switch(ret) {
            case 'd':
                g_PresentToWindow = true;
            break;

            case 's':
                g_SleepBetweenSamplesMs = atoi(optarg);
            break;

            case 0:
                if (strcmp(long_options[option_index].name, "help")) {
                    showHelp(argv[0]);
                    exit(0);
                }
            break;

            default:
                showHelp(argv[0]);
                exit(2);
        }
    }

    g_BenchmarkNameLen = maxBenchmarkNameLen();

    printf(" cmdline:");
    for (int i = 0; i < argc; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");

    if (!runTests()) {
        fprintf(stderr, "exiting due to error.\n");
        return 1;
    }
}
