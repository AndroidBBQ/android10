/*
 * Copyright 2013 The Android Open Source Project
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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "ProgramCache.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <log/log.h>
#include <renderengine/private/Description.h>
#include <utils/String8.h>
#include <utils/Trace.h>
#include "Program.h"

ANDROID_SINGLETON_STATIC_INSTANCE(android::renderengine::gl::ProgramCache)

namespace android {
namespace renderengine {
namespace gl {

/*
 * A simple formatter class to automatically add the endl and
 * manage the indentation.
 */

class Formatter;
static Formatter& indent(Formatter& f);
static Formatter& dedent(Formatter& f);

class Formatter {
    String8 mString;
    int mIndent;
    typedef Formatter& (*FormaterManipFunc)(Formatter&);
    friend Formatter& indent(Formatter& f);
    friend Formatter& dedent(Formatter& f);

public:
    Formatter() : mIndent(0) {}

    String8 getString() const { return mString; }

    friend Formatter& operator<<(Formatter& out, const char* in) {
        for (int i = 0; i < out.mIndent; i++) {
            out.mString.append("    ");
        }
        out.mString.append(in);
        out.mString.append("\n");
        return out;
    }
    friend inline Formatter& operator<<(Formatter& out, const String8& in) {
        return operator<<(out, in.string());
    }
    friend inline Formatter& operator<<(Formatter& to, FormaterManipFunc func) {
        return (*func)(to);
    }
};
Formatter& indent(Formatter& f) {
    f.mIndent++;
    return f;
}
Formatter& dedent(Formatter& f) {
    f.mIndent--;
    return f;
}

void ProgramCache::primeCache(EGLContext context, bool useColorManagement) {
    auto& cache = mCaches[context];
    uint32_t shaderCount = 0;
    uint32_t keyMask = Key::BLEND_MASK | Key::OPACITY_MASK | Key::ALPHA_MASK | Key::TEXTURE_MASK
        | Key::ROUNDED_CORNERS_MASK;
    // Prime the cache for all combinations of the above masks,
    // leaving off the experimental color matrix mask options.

    nsecs_t timeBefore = systemTime();
    for (uint32_t keyVal = 0; keyVal <= keyMask; keyVal++) {
        Key shaderKey;
        shaderKey.set(keyMask, keyVal);
        uint32_t tex = shaderKey.getTextureTarget();
        if (tex != Key::TEXTURE_OFF && tex != Key::TEXTURE_EXT && tex != Key::TEXTURE_2D) {
            continue;
        }
        if (cache.count(shaderKey) == 0) {
            cache.emplace(shaderKey, generateProgram(shaderKey));
            shaderCount++;
        }
    }

    // Prime for sRGB->P3 conversion
    if (useColorManagement) {
        Key shaderKey;
        shaderKey.set(Key::BLEND_MASK | Key::OUTPUT_TRANSFORM_MATRIX_MASK | Key::INPUT_TF_MASK |
                              Key::OUTPUT_TF_MASK,
                      Key::BLEND_PREMULT | Key::OUTPUT_TRANSFORM_MATRIX_ON | Key::INPUT_TF_SRGB |
                              Key::OUTPUT_TF_SRGB);
        for (int i = 0; i < 16; i++) {
            shaderKey.set(Key::OPACITY_MASK,
                          (i & 1) ? Key::OPACITY_OPAQUE : Key::OPACITY_TRANSLUCENT);
            shaderKey.set(Key::ALPHA_MASK, (i & 2) ? Key::ALPHA_LT_ONE : Key::ALPHA_EQ_ONE);

            // Cache rounded corners
            shaderKey.set(Key::ROUNDED_CORNERS_MASK,
                          (i & 4) ? Key::ROUNDED_CORNERS_ON : Key::ROUNDED_CORNERS_OFF);

            // Cache texture off option for window transition
            shaderKey.set(Key::TEXTURE_MASK, (i & 8) ? Key::TEXTURE_EXT : Key::TEXTURE_OFF);
            if (cache.count(shaderKey) == 0) {
                cache.emplace(shaderKey, generateProgram(shaderKey));
                shaderCount++;
            }
        }
    }

    nsecs_t timeAfter = systemTime();
    float compileTimeMs = static_cast<float>(timeAfter - timeBefore) / 1.0E6;
    ALOGD("shader cache generated - %u shaders in %f ms\n", shaderCount, compileTimeMs);
}

ProgramCache::Key ProgramCache::computeKey(const Description& description) {
    Key needs;
    needs.set(Key::TEXTURE_MASK,
              !description.textureEnabled
                      ? Key::TEXTURE_OFF
                      : description.texture.getTextureTarget() == GL_TEXTURE_EXTERNAL_OES
                              ? Key::TEXTURE_EXT
                              : description.texture.getTextureTarget() == GL_TEXTURE_2D
                                      ? Key::TEXTURE_2D
                                      : Key::TEXTURE_OFF)
            .set(Key::ALPHA_MASK, (description.color.a < 1) ? Key::ALPHA_LT_ONE : Key::ALPHA_EQ_ONE)
            .set(Key::BLEND_MASK,
                 description.isPremultipliedAlpha ? Key::BLEND_PREMULT : Key::BLEND_NORMAL)
            .set(Key::OPACITY_MASK,
                 description.isOpaque ? Key::OPACITY_OPAQUE : Key::OPACITY_TRANSLUCENT)
            .set(Key::Key::INPUT_TRANSFORM_MATRIX_MASK,
                 description.hasInputTransformMatrix()
                         ? Key::INPUT_TRANSFORM_MATRIX_ON : Key::INPUT_TRANSFORM_MATRIX_OFF)
            .set(Key::Key::OUTPUT_TRANSFORM_MATRIX_MASK,
                 description.hasOutputTransformMatrix() || description.hasColorMatrix()
                         ? Key::OUTPUT_TRANSFORM_MATRIX_ON
                         : Key::OUTPUT_TRANSFORM_MATRIX_OFF)
            .set(Key::ROUNDED_CORNERS_MASK,
                 description.cornerRadius > 0
                         ? Key::ROUNDED_CORNERS_ON : Key::ROUNDED_CORNERS_OFF);

    needs.set(Key::Y410_BT2020_MASK,
              description.isY410BT2020 ? Key::Y410_BT2020_ON : Key::Y410_BT2020_OFF);

    if (needs.hasTransformMatrix() ||
        (description.inputTransferFunction != description.outputTransferFunction)) {
        switch (description.inputTransferFunction) {
            case Description::TransferFunction::LINEAR:
            default:
                needs.set(Key::INPUT_TF_MASK, Key::INPUT_TF_LINEAR);
                break;
            case Description::TransferFunction::SRGB:
                needs.set(Key::INPUT_TF_MASK, Key::INPUT_TF_SRGB);
                break;
            case Description::TransferFunction::ST2084:
                needs.set(Key::INPUT_TF_MASK, Key::INPUT_TF_ST2084);
                break;
            case Description::TransferFunction::HLG:
                needs.set(Key::INPUT_TF_MASK, Key::INPUT_TF_HLG);
                break;
        }

        switch (description.outputTransferFunction) {
            case Description::TransferFunction::LINEAR:
            default:
                needs.set(Key::OUTPUT_TF_MASK, Key::OUTPUT_TF_LINEAR);
                break;
            case Description::TransferFunction::SRGB:
                needs.set(Key::OUTPUT_TF_MASK, Key::OUTPUT_TF_SRGB);
                break;
            case Description::TransferFunction::ST2084:
                needs.set(Key::OUTPUT_TF_MASK, Key::OUTPUT_TF_ST2084);
                break;
            case Description::TransferFunction::HLG:
                needs.set(Key::OUTPUT_TF_MASK, Key::OUTPUT_TF_HLG);
                break;
        }
    }

    return needs;
}

// Generate EOTF that converts signal values to relative display light,
// both normalized to [0, 1].
void ProgramCache::generateEOTF(Formatter& fs, const Key& needs) {
    switch (needs.getInputTF()) {
        case Key::INPUT_TF_SRGB:
            fs << R"__SHADER__(
                float EOTF_sRGB(float srgb) {
                    return srgb <= 0.04045 ? srgb / 12.92 : pow((srgb + 0.055) / 1.055, 2.4);
                }

                vec3 EOTF_sRGB(const vec3 srgb) {
                    return vec3(EOTF_sRGB(srgb.r), EOTF_sRGB(srgb.g), EOTF_sRGB(srgb.b));
                }

                vec3 EOTF(const vec3 srgb) {
                    return sign(srgb.rgb) * EOTF_sRGB(abs(srgb.rgb));
                }
            )__SHADER__";
            break;
        case Key::INPUT_TF_ST2084:
            fs << R"__SHADER__(
                vec3 EOTF(const highp vec3 color) {
                    const highp float m1 = (2610.0 / 4096.0) / 4.0;
                    const highp float m2 = (2523.0 / 4096.0) * 128.0;
                    const highp float c1 = (3424.0 / 4096.0);
                    const highp float c2 = (2413.0 / 4096.0) * 32.0;
                    const highp float c3 = (2392.0 / 4096.0) * 32.0;

                    highp vec3 tmp = pow(clamp(color, 0.0, 1.0), 1.0 / vec3(m2));
                    tmp = max(tmp - c1, 0.0) / (c2 - c3 * tmp);
                    return pow(tmp, 1.0 / vec3(m1));
                }
            )__SHADER__";
            break;
        case Key::INPUT_TF_HLG:
            fs << R"__SHADER__(
                highp float EOTF_channel(const highp float channel) {
                    const highp float a = 0.17883277;
                    const highp float b = 0.28466892;
                    const highp float c = 0.55991073;
                    return channel <= 0.5 ? channel * channel / 3.0 :
                            (exp((channel - c) / a) + b) / 12.0;
                }

                vec3 EOTF(const highp vec3 color) {
                    return vec3(EOTF_channel(color.r), EOTF_channel(color.g),
                            EOTF_channel(color.b));
                }
            )__SHADER__";
            break;
        default:
            fs << R"__SHADER__(
                vec3 EOTF(const vec3 linear) {
                    return linear;
                }
            )__SHADER__";
            break;
    }
}

void ProgramCache::generateToneMappingProcess(Formatter& fs, const Key& needs) {
    // Convert relative light to absolute light.
    switch (needs.getInputTF()) {
        case Key::INPUT_TF_ST2084:
            fs << R"__SHADER__(
                highp vec3 ScaleLuminance(highp vec3 color) {
                    return color * 10000.0;
                }
            )__SHADER__";
            break;
        case Key::INPUT_TF_HLG:
            fs << R"__SHADER__(
                highp vec3 ScaleLuminance(highp vec3 color) {
                    // The formula is:
                    // alpha * pow(Y, gamma - 1.0) * color + beta;
                    // where alpha is 1000.0, gamma is 1.2, beta is 0.0.
                    return color * 1000.0 * pow(color.y, 0.2);
                }
            )__SHADER__";
            break;
        default:
            fs << R"__SHADER__(
                highp vec3 ScaleLuminance(highp vec3 color) {
                    return color * displayMaxLuminance;
                }
            )__SHADER__";
            break;
    }

    // Tone map absolute light to display luminance range.
    switch (needs.getInputTF()) {
        case Key::INPUT_TF_ST2084:
        case Key::INPUT_TF_HLG:
            switch (needs.getOutputTF()) {
                case Key::OUTPUT_TF_HLG:
                    // Right now when mixed PQ and HLG contents are presented,
                    // HLG content will always be converted to PQ. However, for
                    // completeness, we simply clamp the value to [0.0, 1000.0].
                    fs << R"__SHADER__(
                        highp vec3 ToneMap(highp vec3 color) {
                            return clamp(color, 0.0, 1000.0);
                        }
                    )__SHADER__";
                    break;
                case Key::OUTPUT_TF_ST2084:
                    fs << R"__SHADER__(
                        highp vec3 ToneMap(highp vec3 color) {
                            return color;
                        }
                    )__SHADER__";
                    break;
                default:
                    fs << R"__SHADER__(
                        highp vec3 ToneMap(highp vec3 color) {
                            const float maxMasteringLumi = 1000.0;
                            const float maxContentLumi = 1000.0;
                            const float maxInLumi = min(maxMasteringLumi, maxContentLumi);
                            float maxOutLumi = displayMaxLuminance;

                            float nits = color.y;

                            // clamp to max input luminance
                            nits = clamp(nits, 0.0, maxInLumi);

                            // scale [0.0, maxInLumi] to [0.0, maxOutLumi]
                            if (maxInLumi <= maxOutLumi) {
                                return color * (maxOutLumi / maxInLumi);
                            } else {
                                // three control points
                                const float x0 = 10.0;
                                const float y0 = 17.0;
                                float x1 = maxOutLumi * 0.75;
                                float y1 = x1;
                                float x2 = x1 + (maxInLumi - x1) / 2.0;
                                float y2 = y1 + (maxOutLumi - y1) * 0.75;

                                // horizontal distances between the last three control points
                                float h12 = x2 - x1;
                                float h23 = maxInLumi - x2;
                                // tangents at the last three control points
                                float m1 = (y2 - y1) / h12;
                                float m3 = (maxOutLumi - y2) / h23;
                                float m2 = (m1 + m3) / 2.0;

                                if (nits < x0) {
                                    // scale [0.0, x0] to [0.0, y0] linearly
                                    float slope = y0 / x0;
                                    return color * slope;
                                } else if (nits < x1) {
                                    // scale [x0, x1] to [y0, y1] linearly
                                    float slope = (y1 - y0) / (x1 - x0);
                                    nits = y0 + (nits - x0) * slope;
                                } else if (nits < x2) {
                                    // scale [x1, x2] to [y1, y2] using Hermite interp
                                    float t = (nits - x1) / h12;
                                    nits = (y1 * (1.0 + 2.0 * t) + h12 * m1 * t) * (1.0 - t) * (1.0 - t) +
                                            (y2 * (3.0 - 2.0 * t) + h12 * m2 * (t - 1.0)) * t * t;
                                } else {
                                    // scale [x2, maxInLumi] to [y2, maxOutLumi] using Hermite interp
                                    float t = (nits - x2) / h23;
                                    nits = (y2 * (1.0 + 2.0 * t) + h23 * m2 * t) * (1.0 - t) * (1.0 - t) +
                                            (maxOutLumi * (3.0 - 2.0 * t) + h23 * m3 * (t - 1.0)) * t * t;
                                }
                            }

                            // color.y is greater than x0 and is thus non-zero
                            return color * (nits / color.y);
                        }
                    )__SHADER__";
                    break;
            }
            break;
        default:
            // inverse tone map; the output luminance can be up to maxOutLumi.
            fs << R"__SHADER__(
                highp vec3 ToneMap(highp vec3 color) {
                    const float maxOutLumi = 3000.0;

                    const float x0 = 5.0;
                    const float y0 = 2.5;
                    float x1 = displayMaxLuminance * 0.7;
                    float y1 = maxOutLumi * 0.15;
                    float x2 = displayMaxLuminance * 0.9;
                    float y2 = maxOutLumi * 0.45;
                    float x3 = displayMaxLuminance;
                    float y3 = maxOutLumi;

                    float c1 = y1 / 3.0;
                    float c2 = y2 / 2.0;
                    float c3 = y3 / 1.5;

                    float nits = color.y;

                    float scale;
                    if (nits <= x0) {
                        // scale [0.0, x0] to [0.0, y0] linearly
                        const float slope = y0 / x0;
                        return color * slope;
                    } else if (nits <= x1) {
                        // scale [x0, x1] to [y0, y1] using a curve
                        float t = (nits - x0) / (x1 - x0);
                        nits = (1.0 - t) * (1.0 - t) * y0 + 2.0 * (1.0 - t) * t * c1 + t * t * y1;
                    } else if (nits <= x2) {
                        // scale [x1, x2] to [y1, y2] using a curve
                        float t = (nits - x1) / (x2 - x1);
                        nits = (1.0 - t) * (1.0 - t) * y1 + 2.0 * (1.0 - t) * t * c2 + t * t * y2;
                    } else {
                        // scale [x2, x3] to [y2, y3] using a curve
                        float t = (nits - x2) / (x3 - x2);
                        nits = (1.0 - t) * (1.0 - t) * y2 + 2.0 * (1.0 - t) * t * c3 + t * t * y3;
                    }

                    // color.y is greater than x0 and is thus non-zero
                    return color * (nits / color.y);
                }
            )__SHADER__";
            break;
    }

    // convert absolute light to relative light.
    switch (needs.getOutputTF()) {
        case Key::OUTPUT_TF_ST2084:
            fs << R"__SHADER__(
                highp vec3 NormalizeLuminance(highp vec3 color) {
                    return color / 10000.0;
                }
            )__SHADER__";
            break;
        case Key::OUTPUT_TF_HLG:
            fs << R"__SHADER__(
                highp vec3 NormalizeLuminance(highp vec3 color) {
                    return color / 1000.0 * pow(color.y / 1000.0, -0.2 / 1.2);
                }
            )__SHADER__";
            break;
        default:
            fs << R"__SHADER__(
                highp vec3 NormalizeLuminance(highp vec3 color) {
                    return color / displayMaxLuminance;
                }
            )__SHADER__";
            break;
    }
}

// Generate OOTF that modifies the relative scence light to relative display light.
void ProgramCache::generateOOTF(Formatter& fs, const ProgramCache::Key& needs) {
    if (!needs.needsToneMapping()) {
        fs << R"__SHADER__(
            highp vec3 OOTF(const highp vec3 color) {
                return color;
            }
        )__SHADER__";
    } else {
        generateToneMappingProcess(fs, needs);
        fs << R"__SHADER__(
            highp vec3 OOTF(const highp vec3 color) {
                return NormalizeLuminance(ToneMap(ScaleLuminance(color)));
            }
        )__SHADER__";
    }
}

// Generate OETF that converts relative display light to signal values,
// both normalized to [0, 1]
void ProgramCache::generateOETF(Formatter& fs, const Key& needs) {
    switch (needs.getOutputTF()) {
        case Key::OUTPUT_TF_SRGB:
            fs << R"__SHADER__(
                float OETF_sRGB(const float linear) {
                    return linear <= 0.0031308 ?
                            linear * 12.92 : (pow(linear, 1.0 / 2.4) * 1.055) - 0.055;
                }

                vec3 OETF_sRGB(const vec3 linear) {
                    return vec3(OETF_sRGB(linear.r), OETF_sRGB(linear.g), OETF_sRGB(linear.b));
                }

                vec3 OETF(const vec3 linear) {
                    return sign(linear.rgb) * OETF_sRGB(abs(linear.rgb));
                }
            )__SHADER__";
            break;
        case Key::OUTPUT_TF_ST2084:
            fs << R"__SHADER__(
                vec3 OETF(const vec3 linear) {
                    const highp float m1 = (2610.0 / 4096.0) / 4.0;
                    const highp float m2 = (2523.0 / 4096.0) * 128.0;
                    const highp float c1 = (3424.0 / 4096.0);
                    const highp float c2 = (2413.0 / 4096.0) * 32.0;
                    const highp float c3 = (2392.0 / 4096.0) * 32.0;

                    highp vec3 tmp = pow(linear, vec3(m1));
                    tmp = (c1 + c2 * tmp) / (1.0 + c3 * tmp);
                    return pow(tmp, vec3(m2));
                }
            )__SHADER__";
            break;
        case Key::OUTPUT_TF_HLG:
            fs << R"__SHADER__(
                highp float OETF_channel(const highp float channel) {
                    const highp float a = 0.17883277;
                    const highp float b = 0.28466892;
                    const highp float c = 0.55991073;
                    return channel <= 1.0 / 12.0 ? sqrt(3.0 * channel) :
                            a * log(12.0 * channel - b) + c;
                }

                vec3 OETF(const highp vec3 color) {
                    return vec3(OETF_channel(color.r), OETF_channel(color.g),
                            OETF_channel(color.b));
                }
            )__SHADER__";
            break;
        default:
            fs << R"__SHADER__(
                vec3 OETF(const vec3 linear) {
                    return linear;
                }
            )__SHADER__";
            break;
    }
}

String8 ProgramCache::generateVertexShader(const Key& needs) {
    Formatter vs;
    if (needs.isTexturing()) {
        vs << "attribute vec4 texCoords;"
           << "varying vec2 outTexCoords;";
    }
    if (needs.hasRoundedCorners()) {
        vs << "attribute lowp vec4 cropCoords;";
        vs << "varying lowp vec2 outCropCoords;";
    }
    vs << "attribute vec4 position;"
       << "uniform mat4 projection;"
       << "uniform mat4 texture;"
       << "void main(void) {" << indent << "gl_Position = projection * position;";
    if (needs.isTexturing()) {
        vs << "outTexCoords = (texture * texCoords).st;";
    }
    if (needs.hasRoundedCorners()) {
        vs << "outCropCoords = cropCoords.st;";
    }
    vs << dedent << "}";
    return vs.getString();
}

String8 ProgramCache::generateFragmentShader(const Key& needs) {
    Formatter fs;
    if (needs.getTextureTarget() == Key::TEXTURE_EXT) {
        fs << "#extension GL_OES_EGL_image_external : require";
    }

    // default precision is required-ish in fragment shaders
    fs << "precision mediump float;";

    if (needs.getTextureTarget() == Key::TEXTURE_EXT) {
        fs << "uniform samplerExternalOES sampler;"
           << "varying vec2 outTexCoords;";
    } else if (needs.getTextureTarget() == Key::TEXTURE_2D) {
        fs << "uniform sampler2D sampler;"
           << "varying vec2 outTexCoords;";
    }

    if (needs.hasRoundedCorners()) {
        // Rounded corners implementation using a signed distance function.
        fs << R"__SHADER__(
            uniform float cornerRadius;
            uniform vec2 cropCenter;
            varying vec2 outCropCoords;

            /**
             * This function takes the current crop coordinates and calculates an alpha value based
             * on the corner radius and distance from the crop center.
             */
            float applyCornerRadius(vec2 cropCoords)
            {
                vec2 position = cropCoords - cropCenter;
                // Scale down the dist vector here, as otherwise large corner
                // radii can cause floating point issues when computing the norm
                vec2 dist = (abs(position) - cropCenter + vec2(cornerRadius)) / 16.0;
                // Once we've found the norm, then scale back up.
                float plane = length(max(dist, vec2(0.0))) * 16.0;
                return 1.0 - clamp(plane - cornerRadius, 0.0, 1.0);
            }
            )__SHADER__";
    }

    if (needs.getTextureTarget() == Key::TEXTURE_OFF || needs.hasAlpha()) {
        fs << "uniform vec4 color;";
    }

    if (needs.isY410BT2020()) {
        fs << R"__SHADER__(
            vec3 convertY410BT2020(const vec3 color) {
                const vec3 offset = vec3(0.0625, 0.5, 0.5);
                const mat3 transform = mat3(
                    vec3(1.1678,  1.1678, 1.1678),
                    vec3(   0.0, -0.1878, 2.1481),
                    vec3(1.6836, -0.6523,   0.0));
                // Y is in G, U is in R, and V is in B
                return clamp(transform * (color.grb - offset), 0.0, 1.0);
            }
            )__SHADER__";
    }

    if (needs.hasTransformMatrix() || (needs.getInputTF() != needs.getOutputTF())) {
        // Currently, display maximum luminance is needed when doing tone mapping.
        if (needs.needsToneMapping()) {
            fs << "uniform float displayMaxLuminance;";
        }

        if (needs.hasInputTransformMatrix()) {
            fs << "uniform mat4 inputTransformMatrix;";
            fs << R"__SHADER__(
                highp vec3 InputTransform(const highp vec3 color) {
                    return clamp(vec3(inputTransformMatrix * vec4(color, 1.0)), 0.0, 1.0);
                }
            )__SHADER__";
        } else {
            fs << R"__SHADER__(
                highp vec3 InputTransform(const highp vec3 color) {
                    return color;
                }
            )__SHADER__";
        }

        // the transformation from a wider colorspace to a narrower one can
        // result in >1.0 or <0.0 pixel values
        if (needs.hasOutputTransformMatrix()) {
            fs << "uniform mat4 outputTransformMatrix;";
            fs << R"__SHADER__(
                highp vec3 OutputTransform(const highp vec3 color) {
                    return clamp(vec3(outputTransformMatrix * vec4(color, 1.0)), 0.0, 1.0);
                }
            )__SHADER__";
        } else {
            fs << R"__SHADER__(
                highp vec3 OutputTransform(const highp vec3 color) {
                    return clamp(color, 0.0, 1.0);
                }
            )__SHADER__";
        }

        generateEOTF(fs, needs);
        generateOOTF(fs, needs);
        generateOETF(fs, needs);
    }

    fs << "void main(void) {" << indent;
    if (needs.isTexturing()) {
        fs << "gl_FragColor = texture2D(sampler, outTexCoords);";
        if (needs.isY410BT2020()) {
            fs << "gl_FragColor.rgb = convertY410BT2020(gl_FragColor.rgb);";
        }
    } else {
        fs << "gl_FragColor.rgb = color.rgb;";
        fs << "gl_FragColor.a = 1.0;";
    }
    if (needs.isOpaque()) {
        fs << "gl_FragColor.a = 1.0;";
    }
    if (needs.hasAlpha()) {
        // modulate the current alpha value with alpha set
        if (needs.isPremultiplied()) {
            // ... and the color too if we're premultiplied
            fs << "gl_FragColor *= color.a;";
        } else {
            fs << "gl_FragColor.a *= color.a;";
        }
    }

    if (needs.hasTransformMatrix() || (needs.getInputTF() != needs.getOutputTF())) {
        if (!needs.isOpaque() && needs.isPremultiplied()) {
            // un-premultiply if needed before linearization
            // avoid divide by 0 by adding 0.5/256 to the alpha channel
            fs << "gl_FragColor.rgb = gl_FragColor.rgb / (gl_FragColor.a + 0.0019);";
        }
        fs << "gl_FragColor.rgb = "
              "OETF(OutputTransform(OOTF(InputTransform(EOTF(gl_FragColor.rgb)))));";
        if (!needs.isOpaque() && needs.isPremultiplied()) {
            // and re-premultiply if needed after gamma correction
            fs << "gl_FragColor.rgb = gl_FragColor.rgb * (gl_FragColor.a + 0.0019);";
        }
    }

    if (needs.hasRoundedCorners()) {
        if (needs.isPremultiplied()) {
            fs << "gl_FragColor *= vec4(applyCornerRadius(outCropCoords));";
        } else {
            fs << "gl_FragColor.a *= applyCornerRadius(outCropCoords);";
        }
    }

    fs << dedent << "}";
    return fs.getString();
}

std::unique_ptr<Program> ProgramCache::generateProgram(const Key& needs) {
    ATRACE_CALL();

    // vertex shader
    String8 vs = generateVertexShader(needs);

    // fragment shader
    String8 fs = generateFragmentShader(needs);

    return std::make_unique<Program>(needs, vs.string(), fs.string());
}

void ProgramCache::useProgram(EGLContext context, const Description& description) {
    // generate the key for the shader based on the description
    Key needs(computeKey(description));

    // look-up the program in the cache
    auto& cache = mCaches[context];
    auto it = cache.find(needs);
    if (it == cache.end()) {
        // we didn't find our program, so generate one...
        nsecs_t time = systemTime();
        it = cache.emplace(needs, generateProgram(needs)).first;
        time = systemTime() - time;

        ALOGV(">>> generated new program for context %p: needs=%08X, time=%u ms (%zu programs)",
              context, needs.mKey, uint32_t(ns2ms(time)), cache.size());
    }

    // here we have a suitable program for this description
    std::unique_ptr<Program>& program = it->second;
    if (program->isValid()) {
        program->use();
        program->setUniforms(description);
    }
}

} // namespace gl
} // namespace renderengine
} // namespace android
