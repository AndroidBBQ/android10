/*
 * Copyright (C) 2008-2012 The Android Open Source Project
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

#include <malloc.h>

#include "RenderScript.h"
#include "rsCppInternal.h"

using android::RSC::ScriptIntrinsic;
using android::RSC::ScriptIntrinsic3DLUT;
using android::RSC::ScriptIntrinsicBlend;
using android::RSC::ScriptIntrinsicBlur;
using android::RSC::ScriptIntrinsicColorMatrix;
using android::RSC::ScriptIntrinsicConvolve3x3;
using android::RSC::ScriptIntrinsicConvolve5x5;
using android::RSC::ScriptIntrinsicHistogram;
using android::RSC::ScriptIntrinsicLUT;
using android::RSC::ScriptIntrinsicResize;
using android::RSC::ScriptIntrinsicYuvToRGB;
using android::RSC::sp;

ScriptIntrinsic::ScriptIntrinsic(sp<RS> rs, int id, sp<const Element> e)
    : Script(nullptr, rs) {
    mID = createDispatch(rs, RS::dispatch->ScriptIntrinsicCreate(rs->getContext(), id,
                         e != nullptr ? e->getID() : 0));
    mElement = e;
}

ScriptIntrinsic::~ScriptIntrinsic() {

}

sp<ScriptIntrinsic3DLUT> ScriptIntrinsic3DLUT::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (e->isCompatible(Element::U8_4(rs)) == false) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Element not supported for intrinsic");
        return nullptr;
    }
    return new ScriptIntrinsic3DLUT(rs, e);
}

ScriptIntrinsic3DLUT::ScriptIntrinsic3DLUT(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_3DLUT, e) {

}
void ScriptIntrinsic3DLUT::forEach(const sp<Allocation>& ain, const sp<Allocation>& aout) {
    if (ain->getType()->getElement()->isCompatible(mElement) == false ||
        aout->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "3DLUT forEach element mismatch");
        return;
    }
    Script::forEach(0, ain, aout, nullptr, 0);
}
void ScriptIntrinsic3DLUT::setLUT(const sp<Allocation>& lut) {
    sp<const Type> t = lut->getType();
    if (!t->getElement()->isCompatible(mElement)) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "setLUT element does not match");
        return;
    }
    if (t->getZ() == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "setLUT Allocation must be 3D");
        return;
    }

    Script::setVar(0, lut);
}

sp<ScriptIntrinsicBlend> ScriptIntrinsicBlend::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (e->isCompatible(Element::U8_4(rs)) == false) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Element not supported for intrinsic");
        return nullptr;
    }
    return new ScriptIntrinsicBlend(rs, e);
}

ScriptIntrinsicBlend::ScriptIntrinsicBlend(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_BLEND, e) {
}

void ScriptIntrinsicBlend::forEachClear(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(0, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSrc(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(1, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachDst(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(2, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSrcOver(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(3, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachDstOver(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(4, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSrcIn(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(5, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachDstIn(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(6, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSrcOut(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(7, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachDstOut(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(8, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSrcAtop(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(9, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachDstAtop(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(10, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachXor(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(11, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachMultiply(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(14, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachAdd(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(34, in, out, nullptr, 0);
}

void ScriptIntrinsicBlend::forEachSubtract(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (in->getType()->getElement()->isCompatible(mElement) == false ||
        out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blend");
    }
    Script::forEach(35, in, out, nullptr, 0);
}




sp<ScriptIntrinsicBlur> ScriptIntrinsicBlur::create(const sp<RS>& rs, const sp<const Element>& e) {
    if ((e->isCompatible(Element::U8_4(rs)) == false) &&
        (e->isCompatible(Element::U8(rs)) == false)) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blur");
        return nullptr;
    }
    return new ScriptIntrinsicBlur(rs, e);
}

ScriptIntrinsicBlur::ScriptIntrinsicBlur(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_BLUR, e) {

}

void ScriptIntrinsicBlur::setInput(const sp<Allocation>& in) {
    if (in->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blur input");
        return;
    }
    Script::setVar(1, in);
}

void ScriptIntrinsicBlur::forEach(const sp<Allocation>& out) {
    if (out->getType()->getElement()->isCompatible(mElement) == false) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element in blur output");
        return;
    }
    Script::forEach(0, nullptr, out, nullptr, 0);
}

void ScriptIntrinsicBlur::setRadius(float radius) {
    if (radius > 0.f && radius <= 25.f) {
        Script::setVar(0, &radius, sizeof(float));
    } else {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Blur radius out of 0-25 pixel bound");
    }
}



sp<ScriptIntrinsicColorMatrix> ScriptIntrinsicColorMatrix::create(const sp<RS>& rs) {
    return new ScriptIntrinsicColorMatrix(rs, Element::RGBA_8888(rs));
}

ScriptIntrinsicColorMatrix::ScriptIntrinsicColorMatrix(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_COLOR_MATRIX, e) {
    float add[4] = {0.f, 0.f, 0.f, 0.f};
    setAdd(add);

}

void ScriptIntrinsicColorMatrix::forEach(const sp<Allocation>& in, const sp<Allocation>& out) {
    if (!(in->getType()->getElement()->isCompatible(Element::U8(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::U8_2(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::U8_3(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::U8_4(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::F32(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::F32_2(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::F32_3(mRS))) &&
        !(in->getType()->getElement()->isCompatible(Element::F32_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for ColorMatrix");
        return;
    }

    if (!(out->getType()->getElement()->isCompatible(Element::U8(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U8_2(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U8_3(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U8_4(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::F32(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::F32_2(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::F32_3(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::F32_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for ColorMatrix");
        return;
    }

    Script::forEach(0, in, out, nullptr, 0);
}

void ScriptIntrinsicColorMatrix::setAdd(float* add) {
    Script::setVar(1, (void*)add, sizeof(float) * 4);
}

void ScriptIntrinsicColorMatrix::setColorMatrix3(float* m) {
    float temp[16];
    temp[0] = m[0];
    temp[1] = m[1];
    temp[2] = m[2];
    temp[3] = 0.f;

    temp[4] = m[3];
    temp[5] = m[4];
    temp[6] = m[5];
    temp[7] = 0.f;

    temp[8] = m[6];
    temp[9] = m[7];
    temp[10] = m[8];
    temp[11] = 0.f;

    temp[12] = 0.f;
    temp[13] = 0.f;
    temp[14] = 0.f;
    temp[15] = 1.f;

    setColorMatrix4(temp);
}


void ScriptIntrinsicColorMatrix::setColorMatrix4(float* m) {
    Script::setVar(0, (void*)m, sizeof(float) * 16);
}


void ScriptIntrinsicColorMatrix::setGreyscale() {
    float matrix[] = {0.299f, 0.299f, 0.299f,0.587f,0.587f,0.587f,0.114f,0.114f, 0.114f};
    setColorMatrix3(matrix);
}


void ScriptIntrinsicColorMatrix::setRGBtoYUV() {
    float matrix[] = { 0.299f, -0.14713f, 0.615f, 0.587f, -0.28886f, -0.51499f, 0.114f, 0.436f, -0.10001f};
    setColorMatrix3(matrix);
}


void ScriptIntrinsicColorMatrix::setYUVtoRGB() {
    float matrix[] = {1.f, 1.f, 1.f, 0.f, -0.39465f, 2.03211f, 1.13983f, -0.5806f, 0.f};
    setColorMatrix3(matrix);
}



sp<ScriptIntrinsicConvolve3x3> ScriptIntrinsicConvolve3x3::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (!(e->isCompatible(Element::U8(rs))) &&
        !(e->isCompatible(Element::U8_2(rs))) &&
        !(e->isCompatible(Element::U8_3(rs))) &&
        !(e->isCompatible(Element::U8_4(rs))) &&
        !(e->isCompatible(Element::F32(rs))) &&
        !(e->isCompatible(Element::F32_2(rs))) &&
        !(e->isCompatible(Element::F32_3(rs))) &&
        !(e->isCompatible(Element::F32_4(rs)))) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for Convolve3x3");
        return nullptr;
    }

    return new ScriptIntrinsicConvolve3x3(rs, e);
}

ScriptIntrinsicConvolve3x3::ScriptIntrinsicConvolve3x3(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_3x3, e) {

}

void ScriptIntrinsicConvolve3x3::setInput(const sp<Allocation>& in) {
    if (!(in->getType()->getElement()->isCompatible(mElement))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Element mismatch in Convolve3x3");
        return;
    }
    Script::setVar(1, in);
}

void ScriptIntrinsicConvolve3x3::forEach(const sp<Allocation>& out) {
    if (!(out->getType()->getElement()->isCompatible(mElement))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Element mismatch in Convolve3x3");
        return;
    }
    Script::forEach(0, nullptr, out, nullptr, 0);
}

void ScriptIntrinsicConvolve3x3::setCoefficients(float* v) {
    Script::setVar(0, (void*)v, sizeof(float) * 9);
}

sp<ScriptIntrinsicConvolve5x5> ScriptIntrinsicConvolve5x5::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (!(e->isCompatible(Element::U8(rs))) &&
        !(e->isCompatible(Element::U8_2(rs))) &&
        !(e->isCompatible(Element::U8_3(rs))) &&
        !(e->isCompatible(Element::U8_4(rs))) &&
        !(e->isCompatible(Element::F32(rs))) &&
        !(e->isCompatible(Element::F32_2(rs))) &&
        !(e->isCompatible(Element::F32_3(rs))) &&
        !(e->isCompatible(Element::F32_4(rs)))) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for Convolve5x5");
        return nullptr;
    }

    return new ScriptIntrinsicConvolve5x5(rs, e);
}

ScriptIntrinsicConvolve5x5::ScriptIntrinsicConvolve5x5(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_5x5, e) {

}

void ScriptIntrinsicConvolve5x5::setInput(const sp<Allocation>& in) {
    if (!(in->getType()->getElement()->isCompatible(mElement))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Element mismatch in Convolve5x5 input");
        return;
    }
    Script::setVar(1, in);
}

void ScriptIntrinsicConvolve5x5::forEach(const sp<Allocation>& out) {
    if (!(out->getType()->getElement()->isCompatible(mElement))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Element mismatch in Convolve5x5 output");
        return;
    }

    Script::forEach(0, nullptr, out, nullptr, 0);
}

void ScriptIntrinsicConvolve5x5::setCoefficients(float* v) {
    Script::setVar(0, (void*)v, sizeof(float) * 25);
}

sp<ScriptIntrinsicHistogram> ScriptIntrinsicHistogram::create(const sp<RS>& rs, const sp<const Element>& e) {
    return new ScriptIntrinsicHistogram(rs, e);
}

ScriptIntrinsicHistogram::ScriptIntrinsicHistogram(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_HISTOGRAM, e) {

}

void ScriptIntrinsicHistogram::setOutput(const sp<Allocation>& out) {
    if (!(out->getType()->getElement()->isCompatible(Element::U32(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U32_2(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U32_3(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::U32_4(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::I32(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::I32_2(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::I32_3(mRS))) &&
        !(out->getType()->getElement()->isCompatible(Element::I32_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for Histogram output");
        return;
    }

    if (out->getType()->getX() != 256 ||
        out->getType()->getY() != 0 ||
        out->getType()->hasMipmaps()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Invalid Allocation type for Histogram output");
        return;
    }
    mOut = out;
    Script::setVar(1, out);
}

void ScriptIntrinsicHistogram::setDotCoefficients(float r, float g, float b, float a) {
    if ((r < 0.f) || (g < 0.f) || (b < 0.f) || (a < 0.f)) {
        return;
    }
    if ((r + g + b + a) > 1.f) {
        return;
    }

    FieldPacker fp(16);
    fp.add(r);
    fp.add(g);
    fp.add(b);
    fp.add(a);
    Script::setVar(0, fp.getData(), fp.getLength());

}

void ScriptIntrinsicHistogram::forEach(const sp<Allocation>& ain) {
    if (ain->getType()->getElement()->getVectorSize() <
        mOut->getType()->getElement()->getVectorSize()) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER,
                        "Input vector size must be >= output vector size");
        return;
    }

    if (!(ain->getType()->getElement()->isCompatible(Element::U8(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::U8_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT,
                        "Input allocation to Histogram must be U8 or U8_4");
        return;
    }

    Script::forEach(0, ain, nullptr, nullptr, 0);
}


void ScriptIntrinsicHistogram::forEach_dot(const sp<Allocation>& ain) {
    if (mOut->getType()->getElement()->getVectorSize() != 1) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER,
                        "Output Histogram allocation must have vector size of 1 " \
                        "when used with forEach_dot");
        return;
    }
    if (!(ain->getType()->getElement()->isCompatible(Element::U8(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::U8_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT,
                        "Input allocation to Histogram must be U8 or U8_4");
        return;
    }

    Script::forEach(1, ain, nullptr, nullptr, 0);
}

sp<ScriptIntrinsicLUT> ScriptIntrinsicLUT::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (!(e->isCompatible(Element::U8_4(rs)))) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for LUT");
        return nullptr;
    }
    return new ScriptIntrinsicLUT(rs, e);
}

ScriptIntrinsicLUT::ScriptIntrinsicLUT(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_LUT, e), mDirty(true) {
    LUT = Allocation::createSized(rs, Element::U8(rs), 1024);
    for (int i = 0; i < 256; i++) {
        mCache[i] = i;
        mCache[i+256] = i;
        mCache[i+512] = i;
        mCache[i+768] = i;
    }
    setVar(0, LUT);
}

void ScriptIntrinsicLUT::forEach(const sp<Allocation>& ain, const sp<Allocation>& aout) {
    if (mDirty) {
        LUT->copy1DFrom((void*)mCache);
        mDirty = false;
    }
    if (!(ain->getType()->getElement()->isCompatible(Element::U8_4(mRS))) ||
        !(aout->getType()->getElement()->isCompatible(Element::U8_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for LUT");
        return;
    }
    Script::forEach(0, ain, aout, nullptr, 0);

}

void ScriptIntrinsicLUT::setTable(unsigned int offset, unsigned char base, unsigned int length, unsigned char* lutValues) {
    if ((base + length) > 256 || length == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "LUT out of range");
        return;
    }
    mDirty = true;
    for (unsigned int i = 0; i < length; i++) {
        mCache[offset + base + i] = lutValues[i];
    }
}

void ScriptIntrinsicLUT::setRed(unsigned char base, unsigned int length, unsigned char* lutValues) {
    setTable(0, base, length, lutValues);
}

void ScriptIntrinsicLUT::setGreen(unsigned char base, unsigned int length, unsigned char* lutValues) {
    setTable(256, base, length, lutValues);
}

void ScriptIntrinsicLUT::setBlue(unsigned char base, unsigned int length, unsigned char* lutValues) {
    setTable(512, base, length, lutValues);
}

void ScriptIntrinsicLUT::setAlpha(unsigned char base, unsigned int length, unsigned char* lutValues) {
    setTable(768, base, length, lutValues);
}

ScriptIntrinsicLUT::~ScriptIntrinsicLUT() {

}

sp<ScriptIntrinsicResize> ScriptIntrinsicResize::create(const sp<RS>& rs) {
    return new ScriptIntrinsicResize(rs, nullptr);
}

ScriptIntrinsicResize::ScriptIntrinsicResize(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_RESIZE, e) {

}
void ScriptIntrinsicResize::forEach_bicubic(const sp<Allocation>& aout) {
    if (aout == mInput) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Resize Input and Ouput cannot be the same");
    }

    if (!(mInput->getType()->getElement()->isCompatible(aout->getType()->getElement()))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Resize forEach element mismatch");
        return;
    }
    Script::forEach(0, nullptr, aout, nullptr, 0);
}
void ScriptIntrinsicResize::setInput(const sp<Allocation>& ain) {
    if (!(ain->getType()->getElement()->isCompatible(Element::U8(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::U8_2(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::U8_3(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::U8_4(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::F32(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::F32_2(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::F32_3(mRS))) &&
        !(ain->getType()->getElement()->isCompatible(Element::F32_4(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for Resize Input");
        return;
    }

    mInput = ain;
    Script::setVar(0, ain);
}


sp<ScriptIntrinsicYuvToRGB> ScriptIntrinsicYuvToRGB::create(const sp<RS>& rs, const sp<const Element>& e) {
    if (!(e->isCompatible(Element::U8_4(rs)))) {
        rs->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for YuvToRGB");
        return nullptr;
    }
    return new ScriptIntrinsicYuvToRGB(rs, e);
}

ScriptIntrinsicYuvToRGB::ScriptIntrinsicYuvToRGB(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_YUV_TO_RGB, e) {

}

void ScriptIntrinsicYuvToRGB::setInput(const sp<Allocation>& in) {
    if (!(in->getType()->getElement()->isCompatible(Element::YUV(mRS)))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for input in YuvToRGB");
        return;
    }
    Script::setVar(0, in);
}

void ScriptIntrinsicYuvToRGB::forEach(const sp<Allocation>& out) {
    if (!(out->getType()->getElement()->isCompatible(mElement))) {
        mRS->throwError(RS_ERROR_INVALID_ELEMENT, "Invalid element for output in YuvToRGB");
        return;
    }

    Script::forEach(0, nullptr, out, nullptr, 0);
}
